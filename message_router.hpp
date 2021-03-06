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

typedef std::string HPChannel;
typedef Poco::Net::StreamSocket* StreamSocketPtr;
typedef SafeSet<StreamSocketPtr> StreamSocketPtrSet;
typedef std::map<HPChannel, StreamSocketPtrSet> ChannelMap;

//! MessageRouter manage the spread of a publish to the subscribers
class MessageRouter {
    //! Container for all client, divided by channel
    ChannelMap _channels;

    //! The Poco Logger
    Poco::Logger& _logger;

    //! Map ReadWriteLock mutex
    ReadWriteLock _map_mutex;
public:
    //! Constructor
    MessageRouter();

    //! Register a client to a specific channel
    void subscribe(HPChannel channel, StreamSocketPtr client);

    //! Unsubscribe a client from all channels (due to closing connection or an error)
    void unsubscribe(StreamSocketPtr client);

    //! Publish a message from a client to subscribed clients
    void publish(HPChannel channel, StreamSocketPtr caller, u_char* message, uint32_t len);
};

#endif
