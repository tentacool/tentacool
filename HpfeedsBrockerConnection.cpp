
#include "HpfeedsBrockerConnection.hpp"
#include <iostream>
#include <stdio.h>
#include <string.h>
#include <memory>
#include "Poco/NumberParser.h"
#include "Poco/NumberFormatter.h"
#include "DataManager.hpp"

extern "C" {
#include "hpfeeds.h"
}

using namespace std;
using namespace Poco;

string HpfeedsBrockerConnection::brocker_name = "@hp1";

MessageRouter HpfeedsBrockerConnection::_router;

HpfeedsBrockerConnection::HpfeedsBrockerConnection(const Poco::Net::StreamSocket& s, DataManager* data_manager) :
        Poco::Net::TCPServerConnection(s),
        _sock(this->socket()),
        _logger(Poco::Logger::get("HF_Brocker")),
        _data_manager(data_manager), _state(S_INIT)
{
}

inline string HpfeedsBrockerConnection::ip()
{
    return _sock.peerAddress().host().toString();
}

void HpfeedsBrockerConnection::run()
{
    _sock = this->socket();
    _logger.information("New connection from: "+this->ip());

    //sleep(10);
    bool isOpen = true;
    //Poco::Timespan timeOut(10, 0); //sec, usec
    int nbytes;
	hpf_msg_t *msg;

    // Start the authentication phase
    msg = hpf_msg_info(_auth.genNonce(), (u_char*) brocker_name.data(), brocker_name.size());
    _logger.debug("Sending INFO message to the client...");
    _sock.sendBytes(msg, ntohl(msg->hdr.msglen));
    hpf_msg_delete(msg);
    _state = S_AUTHENTICATION_PROCEEDING;

    while (isOpen) {
        memset(_inBuffer, 0x0, 1000);

    //    if (!_sock.poll(timeOut,Poco::Net::Socket::SELECT_READ) == false) {
     //       cout << "LETTO QUALCOSA\n";
            nbytes = -1;

            try {
                // Receiving bytes from client
                nbytes = _sock.receiveBytes(_inBuffer, sizeof(_inBuffer));
            } catch (Poco::Exception& exc) {
                // Handle your network errors.
                _logger.error("Network error: "+exc.displayText());
                isOpen = false;
            }

            if (nbytes == 0) {
            	//if(_state != S_ERROR) _logger.error("Client closes the connection!");
                isOpen = false;
            } else {
                cout << "LEGGO DATI, state: " << _state << endl;
                switch (_state) {
                    case S_AUTHENTICATION_PROCEEDING:
                        this->authUser();
                        if (_state != S_AUTHENTICATED) {
                            return;
                        }
                        break;
                    case S_AUTHENTICATED:
                        // Read message from socket
                        msg = (hpf_msg_t *)_inBuffer;
                        cout << "OPCODE " << int(msg->hdr.opcode) << endl;
                        switch (int(msg->hdr.opcode)) {
                            case OP_SUBSCRIBE: ////////// SUBSCRIBE //////////
                            {
                            	_logger.debug("I've got a subscription...");
                            	try{
									//Get the name
									hpf_chunk_t *name = hpf_msg_get_chunk((u_char*)msg->data, msg->data[0]);
									string s_name((char*)name->data,name->len);

									//Get the channelname
									string s_channel((char*)(name+1+name->len),
											ntohl(msg->hdr.msglen)- sizeof(msg->hdr)-1-name->len);

									if(!_data_manager->may_subscribe(s_name,s_channel)){ //The client can't subscribe to the specified channel
										_logger.information(s_name+" cannot subscribe to "+s_channel);
										string err = "accessfail";
										msg = hpf_msg_error((u_char*)err.data(), err.size());
										_logger.debug("Sending ERROR message to the client...");
										_sock.sendBytes(msg, ntohl(msg->hdr.msglen));
										hpf_msg_delete(msg);
										_state = S_ERROR;
										break;
										// If fails to one subscribe, I have also to unsubscribe the user to the other
									}
									_router.subscribe(s_channel,&_sock);
									_state = S_SUBSCRIBED;
                            	}catch(Poco::Exception& e){
                            		hpf_msg_delete(msg);
                            		_logger.error(e.displayText());
                            	}
                                break;
                            }
                            case OP_PUBLISH: ////////// PUBLISH //////////
                            	_logger.debug("I've got a publish...");
                            	hpf_msg_t *pub_msg;
                            	try{
                            		//Get the name
                            		hpf_chunk_t *name = hpf_msg_get_chunk((u_char*)msg->data, msg->data[0]);
                            		string s_name((char*)name->data,name->len);

                            		//Get the channelname
                            		hpf_chunk_t *channel = hpf_msg_get_chunk((u_char*)(name+1+name->len), ((u_char*)(name+1+name->len))[0]);
                            		string s_channel((char*)channel->data,channel->len);

									if(!_data_manager->may_publish(s_name,s_channel)){ //The client can't publish to the specified channel
										_logger.information(s_name+" cannot publish to "+s_channel);
										string err = "accessfail";
										msg = hpf_msg_error((u_char*)err.data(), err.size());
										_logger.debug("Sending ERROR message to the client...");
										_sock.sendBytes(msg, ntohl(msg->hdr.msglen));
										hpf_msg_delete(msg);
										_state = S_ERROR;
										break;
									}
                            		//Get the payload
                            		u_char* payload = (channel->data+channel->len);
                            		int payload_lenght = ntohl(msg->hdr.msglen)- sizeof(msg->hdr)-1-name->len-1-channel->len;
                            		//Create the hpfeeds message that we'll send to the subscribed clients
                            		pub_msg = hpf_msg_publish(name->data,name->len,channel->data,channel->len,payload,payload_lenght);
                            		_router.publish(s_channel,&_sock,(u_char*)pub_msg,ntohl(pub_msg->hdr.msglen));
                            		//free pointers
                            		hpf_msg_delete(pub_msg);
                            	}catch(Poco::Exception& e){
                            		hpf_msg_delete(msg);
                            		hpf_msg_delete(pub_msg);
                            		_logger.error(e.displayText());
                            	}
                                break;
                            default:
                        		string err = "Out of sequence message";
                            	_logger.information(err);
                        		msg = hpf_msg_error((u_char*)err.data(), err.size());
                        		_logger.debug("Sending ERROR message to the client...");
                        		_sock.sendBytes(msg, ntohl(msg->hdr.msglen));
                        		hpf_msg_delete(msg);
                        		_state = S_ERROR;
                                break;
                        }
                        break;
                    default:
                    	_logger.error("Unhandled state: "+ _state);
                    	hpf_msg_delete(msg);
                        break;
                }
            }
      //  } //It's the _sock.poll(...) closing parenthesis
    }
}

void HpfeedsBrockerConnection::authUser()
{
    hpf_chunk_t *chunk;
    hpf_msg_t* msg = (hpf_msg_t *)_inBuffer;

    if (msg->hdr.opcode != OP_AUTH) {
        _logger.information("Unexpected message: "+ NumberFormatter::format(msg->hdr.opcode));
        return;
    }

    chunk = hpf_msg_get_chunk((u_char*)_inBuffer + sizeof(msg->hdr),
        ntohl(msg->hdr.msglen) - sizeof(msg->hdr));

    string username((char*)chunk->data, chunk->len);
    string hash(_inBuffer + sizeof(msg->hdr) + 1 + chunk->len,
        int(ntohl(msg->hdr.msglen) - sizeof(msg->hdr) - 1 - chunk->len));

    _logger.information("Getting authorization request for "+username);
 try{
	 string secret = _data_manager->getSecretbyName(username);
    if (_auth.authenticate(username, hash, secret ) == true) {
    	_logger.information("User "+username+" authenticated");
        _state = S_AUTHENTICATED;
    } else {
    	_logger.information("Authentication failed for "+username);
        msg = hpf_msg_error((u_char*)"Authentication failed", 21);
        _sock.sendBytes(msg, ntohl(msg->hdr.msglen));
        _state = S_INIT;
        hpf_msg_delete(msg);
    }
 }catch(exception&  e){
	 cout<<e.what()<<endl;
	 _state = S_ERROR;
 }
}

HpfeedsBrockerConnection::~HpfeedsBrockerConnection()
{
	if(_state==S_SUBSCRIBED)_router.unsubscribe(&_sock);
	_logger.information("Closed connection from: "+this->ip());
}
