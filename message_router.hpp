#ifndef __MESSAGE_ROUTER__
#define __MESSAGE_ROUTER__

#include <Poco/Net/StreamSocket.h>
#include <Poco/Logger.h>
#include <map>
#include <string>
#include <set>
#include <memory>
#include "safe_set.hpp"
#include "rw_lock_t.hpp"

typedef std::string Channel;
typedef Poco::Net::StreamSocket* StreamSocketPtr;
typedef SafeSet<StreamSocketPtr> StreamSocketPtrSet;   //Here i'm using my own thread_safe set
//typedef SafeContainer<StreamSocketPtr,set<StreamSocketPtr>> StreamSocketPtrSet;   //Here i'm using my own thread_safe set
typedef std::map<Channel, StreamSocketPtrSet> ChannelMap;

class MessageRouter {
    //! Container for all client, divided by channel
    ChannelMap _channels;

    //! The Poco Logger
    Poco::Logger& _logger;

    //! Map rw_lock mutex
    rw_lock_t _map_mutex;
public:
    //! Constructor
    MessageRouter();

    //! Register a client to a specific channel
    void subscribe(Channel channel, StreamSocketPtr client);

    //! Unsubscribe a client from all channels (due to closing connection or an error)
    void unsubscribe(StreamSocketPtr client);

    //! Publish a message from a client to subscribed clients
    void publish(Channel channel, StreamSocketPtr caller, u_char* message, uint32_t len);
};

#endif
