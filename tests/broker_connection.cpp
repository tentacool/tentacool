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
    //Reserve an initial chunk for the buffer
    _inBuffer.reserve(INITIAL_CHUNK);
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
        _inBuffer.clear();
        //_inBuffer.resize(INITIAL_CHUNK);

        if (!_sock.poll(timeOut,Poco::Net::Socket::SELECT_READ) == false) {
            nbytes = -1;

        try {
            // Receiving bytes from client
            // Because the two messages from client are sent in a single stream
            // I've to distinguish here the auth message from the publish or subscribe
            nbytes = _sock.receiveBytes(&_inBuffer[0], sizeof(uint32_t));
            if (nbytes <= 0) {
                _logger.debug("Connection closed by peer");
                isOpen = false;
                break;
            }
            //This is the total length, so it must be < MAXBUF - HEADER = DATA
            memcpy(&total_length,&_inBuffer[0], sizeof(uint32_t));
            //cout<<"BYTE: "+NumberFormatter::format(nbytes) + ", "
            //   "TL:" + NumberFormatter::format(ntohl(total_length))<<endl;
            if (ntohl(total_length) > MAXBUF) {
                //Prevent Buffer Overflow
                sendErrorMsg("Oversized Message -> Bad client", false);
                isOpen = false;
                break;
            }
            //Get the Opcode
            nbytes = _sock.receiveBytes(&_inBuffer[0] + sizeof(uint32_t),
                                                          sizeof(uint8_t));
            if (nbytes <= 0) {
                _logger.debug("Connection closed by peer");
                isOpen = false;
                break;
            }
            //This is the total length, so it must be < MAXBUF - HEADER = DATA
            memcpy(&op_code, &_inBuffer[0] + sizeof(uint32_t),
                                              sizeof(uint8_t));
            //cout<<"BYTE: "+NumberFormatter::format(nbytes)+","
            //        " OP:"+NumberFormatter::format(op_code)<<endl;
            //Check total length
            if (op_code > OP_UNSUBSCRIBE) {
                //Wrong OpCode
                sendErrorMsg("Wrong Opcode -> Bad client", true);
                isOpen = false;
                break;
            }
            if (ntohl(total_length) > _inBuffer.capacity()) {
                //We have to resize the buffer
                _inBuffer.resize(ntohl(total_length));
                _logger.debug("Buffer resized to " +
                        NumberFormatter::format(ntohl(total_length)));
            }
            //Receive the rest of the message
            nbytes = _sock.receiveBytes(_inBuffer.data() +
                sizeof(uint32_t) + sizeof(uint8_t),
                ntohl(total_length) - sizeof(uint32_t) - sizeof(uint8_t));
        } catch (Poco::Exception& exc) {
            // Handle your network errors.
            _logger.error("Network error: "+exc.displayText());
            isOpen = false;
            break;
        }

        //cout << "LEGGO DATI, state: " << _state << endl;
        switch (_state) {
            case S_AUTHENTICATION_PROCEEDING:
                this->authUser();
                if (_state != S_AUTHENTICATED) {
                    isOpen = false;
                }
                break;
            case S_SUBSCRIBED: //Subscribed is also authenticated
            case S_AUTHENTICATED:
                //cout << "OPCODE " << (int) op_code << endl;
                if (ntohl(total_length) > message_sizes[int(op_code)]) {
                    //If the message is too long for its type
                    //cout<<"CHECK LONG: "+NumberFormatter::format(ntohl(total_length))<<endl;
                    sendErrorMsg("Oversized Message -> Bad client", true); //??
                    isOpen = false;
                    return;
                }
                switch (int(op_code)) {
                    case OP_SUBSCRIBE: {////////// SUBSCRIBE //////////
                        how_much_read = 4+1+1;
                        _logger.debug("I've got a subscription...");
                        try{
                            //Get the name
                            hpf_chunk_t *name = hpf_msg_get_chunk(
                                  reinterpret_cast<u_char*>(&_inBuffer[5]),
                                                               _inBuffer[5]);
                            string s_name(
                                    reinterpret_cast<char*>(name->data),
                                                                name->len);
                            how_much_read += s_name.length();
                            if (how_much_read >= ntohl(total_length)) {
                                sendErrorMsg("Invalid message -> Bad Client",
                                                                       false);
                                how_much_read = 0;
                                break;
                            }
                            //Get the channelname
                            string s_channel(
                                reinterpret_cast<char*>(name+1+name->len),
                                ntohl(total_length)- HEADER - 1 - name->len);

                            if (!_data_manager->may_subscribe(s_name,
                                                              s_channel)) {
                                //The client can't subscribe to the channel
                                 _logger.information(s_name+
                                         " cannot subscribe to "+s_channel);
                                 sendErrorMsg("accessfail", true);
                                 how_much_read = 0;
                                 break;
                                 // If fails to one subscribe,
                                 // I have also to unsubscribe the user
                                 // to the other
                            }
                            _router.subscribe(s_channel,&_sock);
                            _state = S_SUBSCRIBED;
                            how_much_read = 0;
                        } catch (Poco::Exception& e) {
                            _logger.error("!!!"+e.displayText());
                            how_much_read = 0;
                        }
                        break;
                    }
                    case OP_PUBLISH: { ////////// PUBLISH //////////
                        how_much_read = 5+1+1;
                        _logger.debug("I've got a publish...");
                        hpf_msg_t *pub_msg = NULL;
                        try{
                            //Get the name
                            hpf_chunk_t *name = hpf_msg_get_chunk(
                                  reinterpret_cast<u_char*>(&_inBuffer[5]),
                                                               _inBuffer[5]);
                            string s_name(
                                    reinterpret_cast<char*>(name->data),
                                                               name->len);
                            how_much_read += s_name.length();
                            if(how_much_read >= ntohl(total_length)){
                                sendErrorMsg("Invalid message -> Bad Client",
                                                                        false);
                                how_much_read = 0;
                                break;
                            }
                            //Get the channelname
                            hpf_chunk_t *channel = hpf_msg_get_chunk(
                                reinterpret_cast<u_char*>(name+1+name->len),
                               (reinterpret_cast<u_char*>
                                                      (name+1+name->len))[0]);
                            string s_channel(
                                    reinterpret_cast<char*>(channel->data),
                                    channel->len);
                            how_much_read += s_channel.length();
                            if(how_much_read >= ntohl(total_length)){
                               sendErrorMsg("Invalid message -> Bad Client",
                                                                      false);
                               how_much_read = 0;
                               break;
                            }

                            if(!_data_manager->may_publish(s_name,s_channel)){
                                //The client can't publish to the specified channel
                                _logger.information(s_name +
                                            " cannot publish to "+s_channel);
                                sendErrorMsg("accessfail", true);
                                how_much_read = 0;
                                break;
                            }
                            //Get the payload
                            u_char* payload = (channel->data+channel->len);
                            int payload_lenght = ntohl(total_length) - 5
                                          - 1 - name->len - 1 - channel->len;
                            //Create the hpfeeds message that we'll send to
                            // the subscribed clients
                            pub_msg = hpf_msg_publish(s_name, s_channel,
                                                      payload, payload_lenght);
                            _router.publish(s_channel, &_sock,
                                    reinterpret_cast<u_char*>(pub_msg),
                                    ntohl(pub_msg->hdr.msglen));
                            hpf_msg_delete(pub_msg);
                            how_much_read = 0;
                        } catch(Poco::Exception& e) {
                            _logger.error("ERROR: " + e.displayText());
                            how_much_read = 0;
                            hpf_msg_delete(pub_msg);
                            //isOpen = false; ??
                        }
                        break;
                    }
                    default:
                        sendErrorMsg("Out of sequence message", false);
                        isOpen = false;
                        break;
                }
                break;
                case S_ERROR:
                    _logger.error("There was an error, "
                             "the communication will shut down");
                     isOpen = false;
                     break;
                default:
                     _logger.error("Unhandled state: "+ _state);
                     break;
            }
        } //It's the _sock.poll(...) closing parenthesis
    }
}

void BrokerConnection::sendErrorMsg(string msg, bool sendToClient)
{
    _logger.error(msg);
    if(sendToClient) {
        hpf_msg_t *m = hpf_msg_error(msg);
        _logger.debug("Sending ERROR message to the client...");
        _sock.sendBytes(m, ntohl(m->hdr.msglen));
        hpf_msg_delete(m);
    }
    // If it's subscribed to a channel i have to unsubscribe, so i do not
    // modify its state
    if(_state != S_SUBSCRIBED) _state = S_ERROR;
}

void BrokerConnection::authUser()
{
    hpf_chunk_t *chunk;
    hpf_msg_t* msg = reinterpret_cast<hpf_msg_t *>(_inBuffer.data());
    uint32_t how_much_read = 4+1+1; //total_lenght + opcode+ name length

    if (msg->hdr.opcode != OP_AUTH) {
        _logger.error("Unexpected message: "+
                NumberFormatter::format(msg->hdr.opcode));
        sendErrorMsg("accessfail", true);
        return;
    }
    if(ntohl(msg->hdr.msglen)>message_sizes[int(msg->hdr.opcode)]){
        //If the message is too long for its type
       sendErrorMsg("accessfail", true);
       return;
    }

    chunk = hpf_msg_get_chunk((u_char*)_inBuffer.data() + sizeof(msg->hdr),
        ntohl(msg->hdr.msglen) - sizeof(msg->hdr));

    string username((char*)chunk->data, chunk->len);
    how_much_read += username.length();
    if(how_much_read >= ntohl(msg->hdr.msglen)){
        sendErrorMsg("accessfail", true);
        return;
    }
    string hash(_inBuffer.data() + sizeof(msg->hdr) + 1 + chunk->len,
        int(ntohl(msg->hdr.msglen) - sizeof(msg->hdr) - 1 - chunk->len));

    if(hash.length() != 20){ //no valid hash
        sendErrorMsg("access fail", true);
        return;
    }

    _logger.information("Getting authorization request for "+username);
    try {
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
    } catch(Poco::Exception& e) {
        _logger.error(e.displayText());
        sendErrorMsg("accessfail", true);
    }
}

BrokerConnection::~BrokerConnection()
{
    if (_state == S_SUBSCRIBED) _router.unsubscribe(&_sock);
    else _logger.information("Closed connection from: "+this->ip());
}
