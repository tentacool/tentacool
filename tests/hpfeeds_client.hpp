#ifndef __HPFEEDS_CLIENT__
#define __HPFEEDS_CLIENT__

#include <Poco/Net/SocketStream.h>
#include "hpfeeds.hpp"
#define INITIAL_CHUNK 1024 //1KB
#define MAX_BUF 10*1024 + 5

using namespace std;
using namespace Poco;

class Hpfeeds_client
{
public:
    Hpfeeds_client();
    ~Hpfeeds_client();
    void connect();
    void send_auth_message(string name, string secret);
    void send_wrong_message(uint32_t total_length, uint8_t op_code,
                                                      string data);
    void send_subscribe_message(string name, string channel);
    void send_publish_message(string name, string channel, string message);
    void receive_info_message();
    void receive_publish_message();
    void receive_error_message();
    void disconnect();
private:
    string _broker_address;
    unsigned short _broker_port;
    Net::StreamSocket* _sock;
    string _broker_name;
    uint32_t _nonce;
    //char _inBuffer[MAX_BUF];
    vector<char> _inBuffer;
};

#endif
