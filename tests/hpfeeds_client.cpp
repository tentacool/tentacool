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
        delete _sock;
    }
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

void Hpfeeds_client::receive_error_message()
{
    uint32_t total_length;
    uint8_t op_code;
    Poco::Timespan timeOut(10, 0); //sec, usec
    memset(_inBuffer, 0x0, MAX_BUF);
    if (!_sock->poll(timeOut,Net::Socket::SELECT_READ) == false){
        _sock->receiveBytes(_inBuffer, sizeof(uint32_t) + sizeof(uint8_t));
        //This is the total length
        memcpy(&total_length,_inBuffer,sizeof(uint32_t));
        //GET OPCODE
        memcpy(&op_code,_inBuffer+sizeof(uint32_t),sizeof(uint8_t));
        //get the rest
        _sock->receiveBytes(_inBuffer + sizeof(uint32_t) +
                sizeof(uint8_t), ntohl(total_length) -
                sizeof(uint32_t) - sizeof(uint8_t));
    #ifdef DEBUG
        if(op_code==OP_ERROR){
            cout<<"Server answered with an error message: "<<
                    (_inBuffer+sizeof(uint32_t) + sizeof(uint8_t))<<endl;
        }
    #endif
    }
}
void Hpfeeds_client::receive_info_message()
{
    hpf_msg_t* msg = NULL;
    uint32_t total_length;
    uint8_t op_code;
    Poco::Timespan timeOut(10, 0); //sec, usec
    memset(_inBuffer, 0x0, MAX_BUF);
    if (!_sock->poll(timeOut,Net::Socket::SELECT_READ) == false){
        _sock->receiveBytes(_inBuffer, sizeof(uint32_t) + sizeof(uint8_t));
        //This is the total length
        memcpy(&total_length,_inBuffer,sizeof(uint32_t));
        //GET OPCODE
        memcpy(&op_code,_inBuffer+sizeof(uint32_t),sizeof(uint8_t));
        //get the rest
        _sock->receiveBytes(_inBuffer + sizeof(uint32_t) +
                sizeof(uint8_t), ntohl(total_length) -
                sizeof(uint32_t) - sizeof(uint8_t));
        //msg = hpf_msg_new();
        //TODO devo allocare data in msg
        msg = reinterpret_cast<hpf_msg_t*>(_inBuffer);
        //memcpy(msg,_inBuffer,ntohl(total_length));

        //Getting the broker name
        hpf_chunk_t *b_name = hpf_msg_get_chunk(
                reinterpret_cast<u_char*>(_inBuffer) + sizeof(msg->hdr),
                                                                msg->data[0]);
        string broker_name(reinterpret_cast<char*>(b_name->data),
                                                      b_name->len);
        _broker_name = broker_name;
        //Getting the nonce
        memcpy(&_nonce, b_name + 1 + b_name->len,
                ntohl(msg->hdr.msglen) - sizeof(msg->hdr) - 1 - b_name->len);
        msg = NULL;
    }
}
void Hpfeeds_client::receive_publish_message()
{
    hpf_msg_t* msg = NULL;
    uint32_t total_length;
    uint8_t op_code;
    Poco::Timespan timeOut(10, 0); //sec, usec
    memset(_inBuffer, 0x0, MAX_BUF);
    if (!_sock->poll(timeOut,Net::Socket::SELECT_READ) == false){
        _sock->receiveBytes(_inBuffer, sizeof(uint32_t) + sizeof(uint8_t));
        //This is the total length
        memcpy(&total_length,_inBuffer,sizeof(uint32_t));
        //GET OPCODE
        memcpy(&op_code,_inBuffer+sizeof(uint32_t),sizeof(uint8_t));
        //Receive the rest
        while (_sock->poll(timeOut,Net::Socket::SELECT_READ) == false);
        _sock->receiveBytes(_inBuffer + sizeof(uint32_t) +
                sizeof(uint8_t), ntohl(total_length) -
                sizeof(uint32_t) - sizeof(uint8_t));
        //cout<<"BYTE LETTI PUBLISH: "<<nbytes<<endl;

        //msg = hpf_msg_new();
        msg = reinterpret_cast<hpf_msg_t *>(_inBuffer);
    #ifdef DEBUG
        if(op_code==OP_ERROR){
            cout<<"Server answered with an error message: "<<msg->data<<endl;
            return;
        }
    #endif

        //Getting name
        hpf_chunk_t *name = hpf_msg_get_chunk(
                reinterpret_cast<u_char*>(msg->data), msg->data[0]);
        string s_name(reinterpret_cast<char*>(name->data), name->len);
        //Getting channel
        hpf_chunk_t *channel = hpf_msg_get_chunk(
                reinterpret_cast<u_char*>(msg->data) + 1 + name->len,
                                                msg->data[name->len + 1]);
        string s_channel(reinterpret_cast<char*>(channel->data), channel->len);
        //Get message
        string message(reinterpret_cast<char*>(msg->data) + 1 + name->len
                + 1 + channel->len, ntohl(total_length) - 5 - 1 - name->len
                - 1 - channel->len);
    #ifdef DEBUG
        //if (message.Length > 15) message = message.Left(15 - 3) + "...";
        cout<<"Message from "+s_name+" on channel "+s_channel+":"<<endl<<"=> "
                <<message<<endl;
    #endif
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
            reinterpret_cast<u_char*>(const_cast<char*>(message.data())),
                                                            message.length());
    _sock->sendBytes(pub, ntohl(pub->hdr.msglen));
    hpf_msg_delete(pub);
}

void Hpfeeds_client::send_subscribe_message(string name, string channel)
{
    hpf_msg_t * sub = hpf_msg_subscribe(name, channel);
    _sock->sendBytes(sub, ntohl(sub->hdr.msglen));
    hpf_msg_delete(sub);
}

void Hpfeeds_client::send_wrong_message(uint32_t total_length,
                                                uint8_t op_code, string data)
{
    hpf_msg_t * wrong = hpf_msg_new();
    if (!wrong)
          throw Poco::Exception("Memory allocation fault");
    wrong->hdr.opcode = op_code;
    hpf_msg_add_payload(&wrong,
            reinterpret_cast<const u_char*>(data.data()), data.length());
    uint32_t realLength = wrong->hdr.msglen;
    wrong->hdr.msglen = ntohl(total_length); //Forcing a wrong length
    _sock->sendBytes(wrong, ntohl(realLength));
    hpf_msg_delete(wrong);
}
