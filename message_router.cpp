
#include <message_router.hpp>
#include <iostream>

using namespace std;

void MessageRouter::subscribe(Channel channel, StreamSocketPtr client)
{
    cout << "Subscribe request on channel " << channel << " for " <<
        client->peerAddress().host().toString() << endl;

    StreamSocketPtrSet& channelset = _channels[channel];

    StreamSocketPtrSet::iterator itr = channelset.find(client);
    if (itr == channelset.end()) {
        channelset.insert(client);
    }
    cout << "Channel has " << channelset.size() << " subscribers" << endl;
}

void MessageRouter::publish(Channel channel, StreamSocketPtr caller, string message)
{
    cout << "Publishing message from " << caller->peerAddress().host().toString()
        << " on " << channel << endl;

    StreamSocketPtrSet& channelset = _channels[channel];

    if (channelset.size() == 0) {
        cout << "Channel " << channel << " not present" << endl;
        return;
    }

    StreamSocketPtrSet::iterator itr;
    for (itr = channelset.begin(); itr != channelset.end(); ++itr) {
        if (*itr != caller)
            (*itr)->sendBytes(message.c_str(), message.size());
    }
}
