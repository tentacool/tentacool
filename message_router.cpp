
#include "message_router.hpp"
#include "Poco/NumberParser.h"
#include "Poco/NumberFormatter.h"
#include <iostream>
#include "Poco/Mutex.h"
#include "SafeSet.hpp"

using namespace std;
template<typename T>Poco::Mutex  SafeSet<T>::_mutex;

MessageRouter::MessageRouter() :
    _logger(Poco::Logger::get("HF_Broker"))
{
}

void MessageRouter::subscribe(Channel channel, StreamSocketPtr client)
{
	_logger.information("Subscribe request on channel "+channel+" for "+client->peerAddress().host().toString());

    StreamSocketPtrSet& channelset = _channels[channel];

    StreamSocketPtrSet::iterator itr = channelset.find(client);
    if (itr == channelset.end()) { //there isn't
        channelset.insert(client);
    }
    _logger.debug("Channel "+channel+" has "+Poco::NumberFormatter::format(channelset.size())+" subscribers");
}

void MessageRouter::unsubscribe(StreamSocketPtr client)
{
	_logger.debug("Unsubscribe for "+client->peerAddress().host().toString());

    //StreamSocketPtrSet& channelset = _channels[channel];

    for (ChannelMap::iterator it = _channels.begin(); it != _channels.end(); ++it)
    {
        StreamSocketPtrSet::iterator itr = (it->second).find(client);
        if (itr != (it->second).end()) { //there is!
        	(it->second).erase(client);
        }
    }
}

void MessageRouter::publish(Channel channel, StreamSocketPtr caller, u_char* message, uint32_t len)
{

	//cout<<"II: "+string((char*)message,len)<<endl;
	_logger.information("Publishing message from "+caller->peerAddress().host().toString()+" on "+channel);

    StreamSocketPtrSet& channelset = _channels[channel];

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
