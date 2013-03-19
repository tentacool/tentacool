
#include <message_router.hpp>
#include <iostream>

using namespace std;

void MessageRouter::subscribe(Channel channel, const Poco::Net::StreamSocket& clientsocket)
{
    StreamSocketSet& channel_set = _channels[channel];

    cout << "DEBUG " << _channels.size() << endl;

    if (channel_set.find(clientsocket) == channel_set.end()) {
        cout << "Subscribed " << clientsocket.peerAddress().host().toString() <<
            " to channel " << channel << endl;
        channel_set.insert(clientsocket);
    }
}

void MessageRouter::route(Channel channel, string message)
{
    ChannelMap::iterator m_it = _channels.find(channel);

    if (m_it == _channels.end()) {
        cerr << "No subscribers in channel " << channel << endl;
        return;
    }

    cout << "Channel " << channel << " has " << m_it->second.size() << " clients" << endl;

    for (StreamSocketSet::iterator v_it = m_it->second.begin(); v_it != m_it->second.end(); ++v_it) {
        cout << "Routing message " << message << " to " << v_it->peerAddress().host().toString() << endl;
        v_it->sendBytes(message.data(), message.size());
    }
}
