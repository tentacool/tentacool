
#ifndef __MESSAGE_ROUTER_HPP__
#define __MESSAGE_ROUTER_HPP__

#include "Poco/Net/StreamSocket.h"
#include "Poco/Logger.h"
#include <map>
#include <string>
#include <set>
#include <memory>

typedef std::string Channel;
typedef Poco::Net::StreamSocket* StreamSocketPtr;
typedef std::set<StreamSocketPtr> StreamSocketPtrSet;
typedef std::map<Channel, StreamSocketPtrSet> ChannelMap;

class MessageRouter {
    //! Container for all client, divided by channel
    ChannelMap _channels;

    //! The Poco Logger
    Poco::Logger& _logger;
public:
    //! Constructor
    MessageRouter();

    //! Register a client to a specific channel
    void subscribe(Channel channel, StreamSocketPtr client);

    //! Publish a message from a client to subscribed clients
    void publish(Channel channel, StreamSocketPtr caller, char* message, int len);
};

#endif
