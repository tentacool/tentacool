
#include <message_router.hpp>
#include <iostream>

using namespace std;

MessageRouter::MessageRouter() :
    _logger(Poco::Logger::get("Tentacool.MessageRouter"))
{
}

void MessageRouter::subscribe(Channel channel, StreamSocketPtr client)
{
    poco_information_f2(_logger, "Subscribe request on channel %s for %s", channel,
        client->peerAddress().host().toString());

    StreamSocketPtrSet& channelset = _channels[channel];

    StreamSocketPtrSet::iterator itr = channelset.find(client);
    if (itr == channelset.end()) {
        channelset.insert(client);
    }
    poco_debug_f1(_logger, "Channel has %d", channelset.size());
}

void MessageRouter::publish(Channel channel, StreamSocketPtr caller, char* message, int len)
{
    poco_debug_f2(_logger, "Publishing message from %s on %s",
        caller->peerAddress().host().toString(), channel);

    StreamSocketPtrSet& channelset = _channels[channel];

    if (channelset.size() == 0) {
        poco_debug_f1(_logger, "Channel %s not present", channel);
        return;
    }

    StreamSocketPtrSet::iterator itr;
    for (itr = channelset.begin(); itr != channelset.end(); ++itr) {
        if (*itr != caller)
            (*itr)->sendBytes(message, len);
    }
}
