#include "hpfeeds_client.hpp"
#include "hpfeeds.hpp"
#include <Poco/Net/SocketAddress.h>
#include <Poco/Net/StreamSocket.h>
#include <Poco/Net/SocketStream.h>
#include <Poco/StreamCopier.h>
#include <iostream>

using namespace std;
using namespace Poco;

Hpfeeds_client::Hpfeeds_client(): _broker_address("localhost"),
                                      _broker_port(10000), _sock(NULL),
                                      _broker_name("none"), _nonce(0)
{}
Hpfeeds_client::~Hpfeeds_client()
{
    if(_sock!=NULL){
        disconnect();
        delete _sock;}
}
void Hpfeeds_client::connect()
{
    Net::SocketAddress sa(_broker_address,_broker_port);
    Net::StreamSocket socket(sa);
    if(_sock!=NULL) disconnect();
    _sock = new Net::StreamSocket(socket);
}
void Hpfeeds_client::disconnect()
{
    if(_sock!=NULL) _sock->close();
}
void Hpfeeds_client::receive_info_message()
{
    hpf_msg_t* msg = NULL;
    uint32_t total_length;
    uint8_t op_code;
    Poco::Timespan timeOut(10, 0); //sec, usec
    memset(_inBuffer, 0x0, CHUNK);
    if (!_sock->poll(timeOut,Net::Socket::SELECT_READ) == false){
        _sock->receiveBytes(_inBuffer, sizeof(uint32_t) + sizeof(uint8_t));
        //This is the total length
        memcpy(&total_length,_inBuffer,sizeof(uint32_t));
        //GET OPCODE
        memcpy(&op_code,_inBuffer+sizeof(uint32_t),sizeof(uint8_t));
        //get the rest
        int nbytes = _sock->receiveBytes(_inBuffer + sizeof(uint32_t) +
                sizeof(uint8_t), ntohl(total_length) -
                sizeof(uint32_t) - sizeof(uint8_t));
        cout<<"INFO LENGTH: "<<nbytes<<endl;
        msg = hpf_msg_new();
        //msg = reinterpret_cast<hpf_msg_t *>(_inBuffer);
        memcpy(msg,_inBuffer,ntohl(total_length));

        //Getting the broker name
        hpf_chunk_t *b_name = hpf_msg_get_chunk((u_char*)msg->data, msg->data[0]);
        string broker_name(reinterpret_cast<char*>(b_name->data), b_name->len);
        _broker_name = broker_name;
        //Getting the nonce
        memcpy(&_nonce, b_name + 1 + b_name->len,
                ntohl(msg->hdr.msglen) - sizeof(msg->hdr) - 1 - b_name->len);
    }
}
void Hpfeeds_client::receive_publish_message()
{
    hpf_msg_t* msg = NULL;
    uint32_t total_length;
    uint8_t op_code;
    Poco::Timespan timeOut(10, 0); //sec, usec
    memset(_inBuffer, 0x0, CHUNK);
    if (!_sock->poll(timeOut,Net::Socket::SELECT_READ) == false){
        _sock->receiveBytes(_inBuffer, sizeof(uint32_t) + sizeof(uint8_t));
        //This is the total length
        memcpy(&total_length,_inBuffer,sizeof(uint32_t));
        cout<<"TL: "<<ntohl(total_length)<<endl;
        //GET OPCODE
        memcpy(&op_code,_inBuffer+sizeof(uint32_t),sizeof(uint8_t));
        //Receive the rest
        //do{
        while (_sock->poll(timeOut,Net::Socket::SELECT_READ) == false);
        int nbytes = _sock->receiveBytes(_inBuffer + sizeof(uint32_t) +
                sizeof(uint8_t), ntohl(total_length) -
                sizeof(uint32_t) - sizeof(uint8_t));
        cout<<"BYTE LETTI PUBLISH: "<<nbytes<<endl;

        //}while(nbytes!=0);
        msg = hpf_msg_new();
        msg = reinterpret_cast<hpf_msg_t *>(_inBuffer);
        //memcpy(msg,_inBuffer,ntohl(total_length));

        //Getting name
        hpf_chunk_t *name = hpf_msg_get_chunk((u_char*)msg->data, msg->data[0]);
        string s_name(reinterpret_cast<char*>(name->data), name->len);
        //Getting channel
        hpf_chunk_t *channel = hpf_msg_get_chunk((u_char*)msg->data
                                                + 1 + name->len , msg->data[name->len + 1]);
        string s_channel(reinterpret_cast<char*>(channel->data), channel->len);
        //Get message
        u_char* message = new u_char();
        memcpy(message, msg->data + 1 + name->len + 1 + channel->len,
                ntohl(total_length) - 5 - 1 - name->len - 1 - channel->len);
        cout<<"Message from "+s_name+" on channel "+s_channel+":"<<endl<<"=>  "
                <<message<<endl;
        delete message;
    }
}
void Hpfeeds_client::send_auth_message(string name, string secret)
{
    hpf_msg_t* auth = hpf_msg_auth(_nonce, name, secret);
    _sock->sendBytes(auth, ntohl(auth->hdr.msglen));
    hpf_msg_delete(auth);
}
void Hpfeeds_client::send_publish_message(string name,
                                               string channel, string message)
{
    hpf_msg_t* pub = hpf_msg_publish(name, channel,
            reinterpret_cast<u_char*>(const_cast<char*>(message.data())), message.length());
    _sock->sendBytes(pub, ntohl(pub->hdr.msglen));
    hpf_msg_delete(pub);
}
void Hpfeeds_client::send_subscribe_message(string name, string channel)
{
    hpf_msg_t * sub = hpf_msg_subscribe(name, channel);
    _sock->sendBytes(sub, ntohl(sub->hdr.msglen));
    hpf_msg_delete(sub);
}
