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
    if (!_sock->poll(timeOut,Net::Socket::SELECT_READ) == false){
        _sock->receiveBytes(_inBuffer, sizeof(uint32_t) + sizeof(uint8_t));
        //This is the total length
        memcpy(&total_length,_inBuffer,sizeof(uint32_t));
        //GET OPCODE
        memcpy(&op_code,_inBuffer+sizeof(uint32_t),sizeof(uint8_t));

        _sock->receiveBytes(_inBuffer + sizeof(uint32_t) +
                sizeof(uint8_t), ntohl(total_length) -
                sizeof(uint32_t) - sizeof(uint8_t));

        msg = reinterpret_cast<hpf_msg_t *>(_inBuffer);

        //Getting the broker name
        hpf_chunk_t *b_name = hpf_msg_get_chunk((u_char*)msg->data, msg->data[0]);
        string broker_name(reinterpret_cast<char*>(b_name->data), b_name->len);
        _broker_name = broker_name;
        //Getting the nonce
        memcpy(&_nonce, b_name + 1 + b_name->len,
                ntohl(msg->hdr.msglen) - sizeof(msg->hdr) - 1 - b_name->len);
    }
}
void Hpfeeds_client::send_auth_message(string name, string secret)
{
    hpf_msg_t* auth = hpf_msg_auth(_nonce, name, secret);
    _sock->sendBytes(auth, ntohl(auth->hdr.msglen));
}
