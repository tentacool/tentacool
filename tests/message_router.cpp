#include "message_router.hpp"
#include <Poco/NumberParser.h>
#include <Poco/NumberFormatter.h>
#include <iostream>
#include <Poco/Mutex.h>
#include "safe_set.hpp"

using namespace std;
//template<typename T>Poco::Mutex  SafeSet<T>::_mutex;

MessageRouter::MessageRouter() :
    _logger(Poco::Logger::get("HF_Broker"))
{
}

void MessageRouter::subscribe(HPChannel channel, StreamSocketPtr client)
{
    _logger.information("Subscribe request on channel "+
            channel+" for "+client->peerAddress().host().toString());

    _map_mutex.r_lock();
    StreamSocketPtrSet& channelset = _channels[channel];
    _map_mutex.r_unlock();

    StreamSocketPtrSet::iterator itr = channelset.find(client);
    if (itr == channelset.end()) { //there isn't
        _map_mutex.w_lock();
        channelset.insert(client);
        _map_mutex.w_unlock();
    }
    _logger.debug("Channel "+channel+" has "+
            Poco::NumberFormatter::format(channelset.size())+" subscribers");
}

void MessageRouter::unsubscribe(StreamSocketPtr client)
{
    _logger.debug("Unsubscribe for "+client->peerAddress().host().toString());

    _map_mutex.w_lock();

    for (ChannelMap::iterator it = _channels.begin(); it != _channels.end(); ++it)
    {
        StreamSocketPtrSet::iterator itr = (it->second).find(client);
        if (itr != (it->second).end()) { //there is!
        	(it->second).erase(client);
        }
    }
    _map_mutex.w_unlock();
}

void MessageRouter::publish(HPChannel channel, StreamSocketPtr caller,
        u_char* message, uint32_t len, bool first)
{
    if(first)
    _logger.information("Publishing message from "+
            caller->peerAddress().host().toString()+" on "+channel);
    else _logger.information(".");

    _map_mutex.r_lock();
    StreamSocketPtrSet& channelset = _channels[channel];
    _map_mutex.r_unlock();

    if (channelset.size() == 0) {
        _logger.information("Channel "+channel+" not present");
        return;
    }

    StreamSocketPtrSet::iterator itr;
    for (itr = channelset.begin(); itr != channelset.end(); ++itr) {
        if (*itr != caller){
            (*itr)->sendBytes(message, len);
        }
    }
}
