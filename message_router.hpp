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
typedef SafeSet<StreamSocketPtr> StreamSocketPtrSet;   //Here I'm using my own thread_safe set
typedef std::map<HPChannel, StreamSocketPtrSet> ChannelMap;

//! MessageRouter main work is deliver publish messages through channels and
//! gave it to the users who registered to them
class MessageRouter {
    //! Container for all client, divided by channel
    ChannelMap _channels;

    //! The Poco Logger
    Poco::Logger& _logger;

    //! Map ReadWriteLock mutex
    ReadWriteLock _map_mutex;

    //! Client's Mutex
    map<StreamSocketPtr, ReadWriteLock*> _map_client_mutex;
public:
    //! Constructor
    MessageRouter();

    //! Register a client to a specific channel
    void subscribe(HPChannel channel, StreamSocketPtr client);

    //! Unsubscribe a client from all channels (due to closing connection or an error)
    void unsubscribe(StreamSocketPtr client);

    //! Publish a message from a client to subscribed clients
    int publish(HPChannel channel, StreamSocketPtr caller, u_char* message, uint32_t len, bool first);

    //! Get the mutexes of the client subscribed to the specified channel
    SafeSet<ReadWriteLock*> clientLock(string channel);
};

#endif
