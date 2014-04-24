#ifndef __HPFEEDS_CLIENT__
#define __HPFEEDS_CLIENT__

#include <Poco/Net/SocketStream.h>
#include "hpfeeds.hpp"
#define CHUNK 1024

using namespace std;
using namespace Poco;

class Hpfeeds_client
{
public:
    Hpfeeds_client();
    ~Hpfeeds_client();
    void connect();
    void send_auth_message(string name, string secret);
    void receive_info_message();
    void print_data(){
        cout<<_broker_name<<", "<<_nonce<<endl;
    }
    void disconnect();
private:
    string _broker_address;
    unsigned short _broker_port;
    //Net::SocketStream* _sock;
    Net::StreamSocket* _sock;
    string _broker_name;
    uint32_t _nonce;
    char _inBuffer[CHUNK];
};

#endif
