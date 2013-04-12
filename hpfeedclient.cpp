
#include <hpfeedclient.hpp>
#include <iostream>

#include <stdio.h>
#include <string.h>


using namespace std;

MessageRouter HpFeedClient::_router;

HpFeedClient::HpFeedClient(const Poco::Net::StreamSocket& s) :
        Poco::Net::TCPServerConnection(s)
{
}

string HpFeedClient::ip()
{
    return socket().peerAddress().host().toString();
}

void HpFeedClient::run()
{
    Poco::Net::StreamSocket& sock = this->socket();
    cout << "New connection from: " << this->ip() <<  endl << flush;
    bool isOpen = true;
    Poco::Timespan timeOut(10, 0);
    int nbytes;

    while (isOpen) {
        memset(_inBuffer, 0x0, 1000);
        if (!sock.poll(timeOut,Poco::Net::Socket::SELECT_READ) == false) {
            nbytes = -1;

            try {
                // Receiving bytes from client
                nbytes = sock.receiveBytes(_inBuffer, sizeof(_inBuffer));
            } catch (Poco::Exception& exc) {
                // Handle your network errors.
                cerr << "Network error: " << exc.displayText() << endl;
                isOpen = false;
            }

            if (nbytes == 0) {
                cout << "Client closes connection!" << endl << flush;
                isOpen = false;
            } else {
                // Main message parsing routine
                if (this->type() == "subscribe") {
                    _router.subscribe(this->channel(), &sock);
                }
                if (this->type() == "publish") {
                    _router.publish(this->channel(), &sock, _inBuffer, nbytes);
                }
            }
        }
    }
    cout << "Client disconnected" << endl << flush;
}

string HpFeedClient::type()
{
    char type[50];
    sscanf(_inBuffer, "%s", type);
    return string(type, strlen(type));
}

string HpFeedClient::channel()
{
    char channel[50];
    sscanf(_inBuffer, "%*s %s %*s", channel);
    return string(channel, strlen(channel));
}
