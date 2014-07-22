#ifndef __HPFEEDS_CLIENT__
#define __HPFEEDS_CLIENT__

#include <Poco/Net/SocketStream.h>
#include "../hpfeeds.hpp"
#define INITIAL_CHUNK 1024 //1KB
#define MAX_BUF 10*1024 + 5

using namespace std;
using namespace Poco;

class HpfeedsClient
{
public:
    HpfeedsClient();
    ~HpfeedsClient();
    void connect();
    void sendAuth(string name, string secret);
    void sendWrong(uint32_t total_length, uint8_t op_code, string data);
    void sendSubscribe(string name, string channel);
    void sendPublish(string name, string channel, string message);
    void receiveInfo();
    void receivePublish();
    void receiveError();
    void disconnect();
private:
    string _broker_address;
    unsigned short _broker_port;
    Net::StreamSocket* _sock;
    string _broker_name;
    uint32_t _nonce;
    vector<char> _inBuffer;
};

#endif
