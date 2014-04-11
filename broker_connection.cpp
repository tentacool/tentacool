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
        memset(_inBuffer, 0x0, MAXBUF);

        if (!_sock.poll(timeOut,Poco::Net::Socket::SELECT_READ) == false) {
            nbytes = -1;

            try {
                // Receiving bytes from client
                // Because the two messages from client are sent in a single stream
                // I've to distinguish here the auth message from the publish or subscribe
                nbytes = _sock.receiveBytes(_inBuffer, sizeof(uint32_t));
                //This is the total lenght, so it must be < MAXBUF - HEADER = DATA
                memcpy(&total_length,_inBuffer,sizeof(uint32_t));
                if(ntohl(total_length) > MAXBUF){
                    //Prevent Buffer Overflow
                    //_logger.error("Oversized Message -> Bad client");
                    sendErrorMsg("Oversized Message -> Bad client");
                    isOpen = false;
                    break;
                }
                //Receive the rest of the message
                nbytes = _sock.receiveBytes(_inBuffer+sizeof(uint32_t), ntohl(total_length) - sizeof(uint32_t));

            }catch(Poco::Exception& exc){
                // Handle your network errors.
                _logger.error("Network error: "+exc.displayText());
                isOpen = false;
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
                    case S_AUTHENTICATED:
                        msg = (hpf_msg_t *)_inBuffer;
                        cout << "OPCODE " << int(msg->hdr.opcode) << endl;

                        if(ntohl(msg->hdr.msglen)>message_sizes[int(msg->hdr.opcode)]){
                            //If the message is too long for its type
                            _logger.error("Oversized Message -> Bad client");
                            sendErrorMsg("Oversized Message -> Bad client"); //??
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
                                    hpf_chunk_t *name = hpf_msg_get_chunk((u_char*)msg->data, msg->data[0]);
                                    string s_name((char*)name->data,name->len);
                                    how_much_read += s_name.length();
                                    if(how_much_read>=ntohl(msg->hdr.msglen)){
                                        sendErrorMsg("Invalid message -> Bad Client");
                                        how_much_read = 0;
                                        break;
                                    }
                                    //Get the channelname
                                    string s_channel((char*)(name+1+name->len),
                                            ntohl(msg->hdr.msglen)- sizeof(msg->hdr)-1-name->len);

                                    if(!_data_manager->may_subscribe(s_name,s_channel)){
                                        //The client can't subscribe to the specified channel
                                        _logger.information(s_name+" cannot subscribe to "+s_channel);
                                        sendErrorMsg("accessfail");
                                        how_much_read = 0;
                                        break;
                                        // If fails to one subscribe,
                                        // I have also to unsubscribe the user to the other
                                    }
                                    _router.subscribe(s_channel,&_sock);
                                    _state = S_SUBSCRIBED;
                                    how_much_read = 0;
                                }catch(Poco::Exception& e){
                                    hpf_msg_delete(msg);
                                    _logger.error("!!!"+e.displayText());
                                    how_much_read = 0;
                                }
                                break;
                            }
                            case OP_PUBLISH:{ ////////// PUBLISH //////////
                                how_much_read = 4+1+1+1;
                                _logger.debug("I've got a publish...");
                                hpf_msg_t *pub_msg = NULL;
                                try{
                                    //Get the name
                                    hpf_chunk_t *name = hpf_msg_get_chunk((u_char*)msg->data, msg->data[0]);
                                    string s_name((char*)name->data,name->len);
                                    how_much_read += s_name.length();
                                    if(how_much_read>=ntohl(msg->hdr.msglen)){
                                        sendErrorMsg("Invalid message -> Bad Client");
                                        how_much_read = 0;
                                        break;
                                    }

                                    //Get the channelname
                                    hpf_chunk_t *channel = hpf_msg_get_chunk((u_char*)(name+1+name->len), ((u_char*)(name+1+name->len))[0]);
                                    string s_channel((char*)channel->data,channel->len);
                                    how_much_read += s_channel.length();
                                    if(how_much_read>=ntohl(msg->hdr.msglen)){
                                        sendErrorMsg("Invalid message -> Bad Client");
                                        how_much_read = 0;
                                        break;
                                    }

                                    if(!_data_manager->may_publish(s_name,s_channel)){
                                        //The client can't publish to the specified channel
                                        _logger.information(s_name+" cannot publish to "+s_channel);
                                        sendErrorMsg("accessfail");
                                        how_much_read = 0;
                                        break;
                                    }
                                    //Get the payload
                                    u_char* payload = (channel->data+channel->len);
                                    int payload_lenght = ntohl(msg->hdr.msglen)- sizeof(msg->hdr)-1-name->len-1-channel->len;
                                    //Create the hpfeeds message that we'll send to the subscribed clients
                                    pub_msg = hpf_msg_publish(s_name,s_channel,payload,payload_lenght);
                                    _router.publish(s_channel,&_sock,(u_char*)pub_msg,ntohl(pub_msg->hdr.msglen));
                                    //free pointers
                                    hpf_msg_delete(pub_msg);
                                    how_much_read = 0;
                                }catch(Poco::Exception& e){
                                    _logger.error(e.displayText());
                                    hpf_msg_delete(msg);
                                    how_much_read = 0;
                                    hpf_msg_delete(pub_msg);
                                }
                                break;
                            }
                            default:
                                sendErrorMsg("Out of sequence message");
                                break;
                        }
                        break;
                    default:
                        _logger.error("Unhandled state: "+ _state);
                        hpf_msg_delete(msg);
                        break;
                }
            }
        } //It's the _sock.poll(...) closing parenthesis
    }
}

void BrokerConnection::sendErrorMsg(string msg)
{
    _logger.error(msg);
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
        _logger.information("Unexpected message: "+ NumberFormatter::format(msg->hdr.opcode));
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
        sendErrorMsg("accessfail");
    }
}

BrokerConnection::~BrokerConnection()
{
    if(_state==S_SUBSCRIBED)_router.unsubscribe(&_sock);
    _logger.information("Closed connection from: "+this->ip());
}
