#include "broker_connection.hpp"
#include <iostream>
#include <stdio.h>
#include <string.h>
#include <memory>
#include <Poco/NumberParser.h>
#include <Poco/Net/NetException.h>
#include <Poco/NumberFormatter.h>
#include "data_manager.hpp"
#include "hpfeeds.hpp"

using namespace std;
using namespace Poco;

string BrokerConnection::Broker_name = "@hp1";

MessageRouter BrokerConnection::_router;
ReadWriteLock BrokerConnection::_publishing_lock;

BrokerConnection::BrokerConnection(const Net::StreamSocket& s, DataManager* data_manager) :
    Net::TCPServerConnection(s),
    _sock(this->socket()),
    _logger(Poco::Logger::get("HF_Broker")),
    _data_manager(data_manager), _state(S_INIT)
{
    message_sizes[OP_ERROR] = MAXBUF;
    message_sizes[OP_INFO] = 5+1+256+20;
    message_sizes[OP_AUTH] = 5+1+256+20;
    message_sizes[OP_PUBLISH] = MAXBUF;
    message_sizes[OP_SUBSCRIBE] = 5+2+256*2;
    message_sizes[OP_UNSUBSCRIBE] = 5+2+256*2;
}

inline string BrokerConnection::ip()
{
    return _sock.peerAddress().host().toString();
}

void BrokerConnection::run()
{
    _sock = this->socket();
    _logger.information("New connection from: "+this->ip());
    uint32_t total_length;
    uint8_t op_code;
    //sleep(10);
    bool isOpen = true;
    Poco::Timespan timeOut(10, 0); //sec, usec
    int nbytes;
    uint32_t how_much_read; //total_lenght+opcode+name_lenght+channel_lenght
    hpf_msg_t *msg;

    // Start the authentication phase
    msg = hpf_msg_info(_auth.genNonce(), Broker_name);
    _logger.debug("Sending INFO message to the client...");
    _sock.sendBytes(msg, ntohl(msg->hdr.msglen));
    hpf_msg_delete(msg);
    _state = S_AUTHENTICATION_PROCEEDING;

    while (isOpen) {
        memset(_inBuffer, 0x0, CHUNK);

        if (!_sock.poll(timeOut,Poco::Net::Socket::SELECT_READ) == false) {
            nbytes = -1;

            try {
                // Receiving bytes from client
                //GET HEADER
                nbytes = _sock.receiveBytes(_inBuffer, sizeof(uint32_t) +
                                                        sizeof(uint8_t));

                if(nbytes==0){
                    isOpen = false;
                    break;
                }
                //This is the total lenght, so it must be < MAXBUF
                memcpy(&total_length,_inBuffer,sizeof(uint32_t));
                //cout<<ntohl(total_length)<<endl;
                if(ntohl(total_length) > MAXBUF){
                    //Prevent Buffer Overflow
                    sendErrorMsg("1) Oversized Message -> Bad client",false);
                    isOpen = false;
                    break;
                }
                //GET OPCODE
                memcpy(&op_code,_inBuffer+sizeof(uint32_t),sizeof(uint8_t));
                if(op_code < 0 || op_code > OP_UNSUBSCRIBE){
                    //Prevent Buffer Overflow
                    sendErrorMsg("Invalid Op_code -> Bad client", true);
                    isOpen = false;
                    break;
                }

                if(op_code!=OP_PUBLISH)
                //If not publish, receive the rest of the message
                nbytes = _sock.receiveBytes(_inBuffer + sizeof(uint32_t) +
                        sizeof(uint8_t), ntohl(total_length) -
                        sizeof(uint32_t) - sizeof(uint8_t));

            }catch(Poco::Exception& exc){
                _logger.error("Network error: "+exc.displayText());
                isOpen = false;
                break;
            }

            if(nbytes == 0){
                //if(_state != S_ERROR) _logger.error("Client closes the connection!");
                isOpen = false;
            }else{
                cout << "LEGGO DATI, state: " << _state << endl;
                switch (_state) {
                    case S_AUTHENTICATION_PROCEEDING:
                        this->authUser();
                        if (_state != S_AUTHENTICATED) {
                            isOpen = false;
                        }
                        break;
                    case S_SUBSCRIBED: //Subscribed is also authenticated
                    case S_AUTHENTICATED:
                        msg = (hpf_msg_t *)_inBuffer;
                        cout << "OPCODE " << int(msg->hdr.opcode) << endl;

                        if(ntohl(msg->hdr.msglen)>message_sizes[int(msg->hdr.opcode)]){
                            //If the message is too long for its type
                            //_logger.error("Oversized Message -> Bad client");
                            sendErrorMsg("2) Oversized Message -> Bad client", true); //??
                            isOpen = false;
                            return;
                        }
                        switch (int(msg->hdr.opcode)) {
                            case OP_SUBSCRIBE: ////////// SUBSCRIBE //////////
                            {
                                how_much_read = 4+1+1;
                                _logger.debug("I've got a subscription...");
                                try{
                                    //Get the name
                                    hpf_chunk_t *name =
                                            hpf_msg_get_chunk((u_char*)msg->data,
                                                    msg->data[0]);
                                    string s_name((char*)name->data, name->len);
                                    how_much_read += s_name.length();
                                    if(how_much_read>=ntohl(msg->hdr.msglen)){
                                        sendErrorMsg("Invalid message -> Bad Client", true);
                                        how_much_read = 0;
                                        isOpen = false;
                                        break;
                                    }
                                    //Get the channelname
                                    string s_channel(
                                            reinterpret_cast<char*>(name + 1 + name->len),
                                            ntohl(msg->hdr.msglen) -
                                            sizeof(msg->hdr) - 1 - name->len);

                                    if(!_data_manager->may_subscribe(s_name, s_channel)){
                                        //The client can't subscribe to the specified channel
                                        _logger.information(s_name +
                                                " cannot subscribe to " + s_channel);
                                        sendErrorMsg("accessfail", true);
                                        how_much_read = 0;
                                        isOpen = false;
                                        break;
                                    }
                                    _router.subscribe(s_channel,&_sock);
                                    _state = S_SUBSCRIBED;
                                    how_much_read = 0;
                                }catch(Poco::Exception& e){
                                    hpf_msg_delete(msg);
                                    _logger.error("!!!"+e.displayText());
                                    how_much_read = 0;
                                    return;
                                }
                                break;
                            }
                            case OP_PUBLISH:{ ////////// PUBLISH //////////
                                how_much_read = 4+1+1+1; //header(5) + n_l + c_l
                                uint8_t name_l, channel_l;
                                _logger.debug("I've got a publish...");
                                try{
                                    //Get the name
                                    _sock.receiveBytes(_inBuffer + HEADER,
                                                            sizeof(uint8_t));
                                    memcpy(&name_l,_inBuffer + HEADER,
                                                            sizeof(uint8_t));
                                    _sock.receiveBytes(_inBuffer + HEADER +
                                            sizeof(uint8_t), name_l);
                                    string s_name(
                                            reinterpret_cast<char*>(_inBuffer)
                                            + HEADER +
                                            sizeof(uint8_t), name_l);
                                    how_much_read += s_name.length();
                                    if(how_much_read >= ntohl(msg->hdr.msglen)){
                                        sendErrorMsg("PUBLISH: Invalid identifier -> Bad Client", true);
                                        how_much_read = 0;
                                        break;
                                    }

                                    //Get the channelname
                                    _sock.receiveBytes(_inBuffer + HEADER +
                                            sizeof(uint8_t) + name_l,
                                            sizeof(uint8_t));
                                    memcpy(&channel_l, _inBuffer + HEADER +
                                            sizeof(uint8_t) + name_l,
                                            sizeof(uint8_t));
                                    _sock.receiveBytes(_inBuffer + HEADER +
                                            sizeof(uint8_t) + name_l +
                                            sizeof(uint8_t), channel_l);
                                    string s_channel(
                                            reinterpret_cast<char*>(_inBuffer)
                                            + HEADER + sizeof(uint8_t) + name_l
                                            + sizeof(uint8_t), channel_l);
                                    how_much_read += s_channel.length();
                                    if(how_much_read >= ntohl(total_length)){
                                        sendErrorMsg("PUBLISH: Invalid channel name -> Bad Client", true);
                                        how_much_read = 0;
                                        isOpen = false;
                                        break;
                                    }

                                    if(!_data_manager->may_publish(s_name, s_channel)){
                                        //The client can't publish to the specified channel
                                        _logger.information(s_name +
                                                " cannot publish to " + s_channel);
                                        sendErrorMsg("accessfail", true);
                                        how_much_read = 0;
                                        isOpen = false;
                                        break;
                                    }
                                    uint32_t payload_length = ntohl(total_length) -
                                            HEADER - sizeof(name_l) - int(name_l) -
                                            sizeof(channel_l) - int(channel_l);
                                    //GET PUBLISHING LOCK
                                    _publishing_lock.w_lock();
                                    //_publishing_lock.writeLock();
                                    _router.publish(s_channel, &_sock,
                                            reinterpret_cast<u_char*>(_inBuffer),
                                            ntohl(total_length) - int(payload_length),
                                            true);
                                    //Get the payload chunks

                                    while(int(payload_length) >= CHUNK){
                                        //Reset _inBuffer
                                        memset(_inBuffer, 0x0, CHUNK);
                                        _sock.receiveBytes(_inBuffer, CHUNK);
                                        _router.publish(s_channel,&_sock,
                                                reinterpret_cast<u_char*>(_inBuffer),
                                                CHUNK, false);
                                        payload_length -= CHUNK;
                                    }
                                    if(int(payload_length) != 0){
                                        //Reset _inBuffer
                                        memset(_inBuffer, 0x0, CHUNK);
                                        _sock.receiveBytes(_inBuffer, int(payload_length));
                                        _router.publish(s_channel,&_sock,
                                                reinterpret_cast<u_char*>(_inBuffer),
                                                payload_length, false);
                                    }
                                    //UNLOCK
                                    _publishing_lock.w_unlock();
                                    //_publishing_lock.unlock();
                                    how_much_read = 0;
                                }catch(Poco::Exception& e){
                                    _logger.error("PUBLISH "+e.displayText());
                                    _publishing_lock.w_unlock();
                                    //_publishing_lock.unlock();
                                    //hpf_msg_delete(msg);
                                    how_much_read = 0;
                                    return;
                                }
                                break;
                            }
                            default:
                                sendErrorMsg("Out of sequence message "
                                        "- close the connection", false);
                                isOpen = false;
                                break;
                        }
                        break;
                    default:
                        _logger.error("Unhandled state: "+ _state);
                        isOpen = false;
                        hpf_msg_delete(msg);
                        break;
                }
            }
        } //It's the _sock.poll(...) closing parenthesis
    }
}

void BrokerConnection::sendErrorMsg(string msg, bool logMsg)
{
    if(logMsg) _logger.error(msg);
    hpf_msg_t *m = hpf_msg_error(msg);
    _logger.debug("Sending ERROR message to the client...");
    _sock.sendBytes(m, ntohl(m->hdr.msglen));
    hpf_msg_delete(m);
    _state = S_ERROR;
}

void BrokerConnection::authUser()
{
    hpf_chunk_t *chunk;
    hpf_msg_t* msg = (hpf_msg_t *)_inBuffer;
    uint32_t how_much_read = 4+1+1; //total_lenght + opcode+ name length

    if (msg->hdr.opcode != OP_AUTH) {
        _logger.information("Unexpected message: " +
                NumberFormatter::format(msg->hdr.opcode));
        return;
    }
    if(ntohl(msg->hdr.msglen)>message_sizes[int(msg->hdr.opcode)]){
        //If the message is too long for its type
       _logger.error("Oversized Message -> Bad client");
       _state = S_ERROR;
       hpf_msg_delete(msg);
       return;
    }

    chunk = hpf_msg_get_chunk((u_char*)_inBuffer + sizeof(msg->hdr),
        ntohl(msg->hdr.msglen) - sizeof(msg->hdr));

    string username((char*)chunk->data, chunk->len);
    how_much_read += username.length();
    if(how_much_read>=ntohl(msg->hdr.msglen)){
        _logger.error("Invalid Message -> Bad client");
        _state = S_ERROR;
        hpf_msg_delete(msg);
        return;
    }
    string hash(_inBuffer + sizeof(msg->hdr) + 1 + chunk->len,
        int(ntohl(msg->hdr.msglen) - sizeof(msg->hdr) - 1 - chunk->len));

    if(hash.length()<20){ //no valid hash
        _logger.error("Invalid Message -> Bad client");
        _state = S_ERROR;
        hpf_msg_delete(msg);
        return;
    }

    _logger.information("Getting authorization request for "+username);
    try{
        string secret = _data_manager->getSecretbyName(username);
        if (_auth.authenticate(hash, secret ) == true) {
            _logger.information("User "+username+" authenticated");
            _state = S_AUTHENTICATED;
        } else {
            _logger.information("Authentication failed for "+username);
            msg = hpf_msg_error("Authentication failed");
            _sock.sendBytes(msg, ntohl(msg->hdr.msglen));
            _state = S_INIT;
            hpf_msg_delete(msg);
        }
    }catch(Poco::Exception& e){
        _logger.error(e.displayText());
        sendErrorMsg("accessfail", true);
    }
}

BrokerConnection::~BrokerConnection()
{
    if(_state==S_SUBSCRIBED)_router.unsubscribe(&_sock);
    _logger.information("Closed connection from: " + this->ip());
}
