
#ifndef __MESSAGE_ROUTER_HPP__
#define __MESSAGE_ROUTER_HPP__

#include <Poco/Net/StreamSocket.h>
#include <map>
#include <set>
#include <string>

typedef std::string Channel;
typedef std::set<Poco::Net::StreamSocket> StreamSocketSet;
typedef std::map<Channel, StreamSocketSet> ChannelMap;

class MessageRouter {
     ChannelMap _channels;
public:
    MessageRouter() {}

    // Register a client to a specific channel
    void subscribe(Channel channel, const Poco::Net::StreamSocket& clientsocket);

    // Route a message from a client to subscribed clients
    void route(Channel channel, std::string message);
};

#endif
