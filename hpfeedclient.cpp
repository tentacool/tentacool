
#include <hpfeedclient.hpp>
#include <iostream>

#include <stdio.h>
#include <string.h>


using namespace std;

MessageRouter HpFeedClient::_router = MessageRouter();

HpFeedClient::HpFeedClient(const Poco::Net::StreamSocket& s) :
        Poco::Net::TCPServerConnection(s),
        _logger(Poco::Logger::get("Tentacool.HpFeedClient"))
{
}

inline string HpFeedClient::ip()
{
    return socket().peerAddress().host().toString();
}

void HpFeedClient::run()
{
    Poco::Net::StreamSocket& sock = this->socket();
    poco_information_f1(_logger, "New connection from: %s", this->ip());
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
                poco_critical_f1(_logger, "Network error: %s", exc.displayText());
                //cerr << "Network error: " << exc.displayText() << endl;
                isOpen = false;
            }

            if (nbytes == 0) {
                poco_debug(_logger, "Client closes connection!");
                isOpen = false;
            } else {
                // Main message parsing routine
                if (this->type() == "subscribe") {
                    _router.subscribe(this->channel(), &sock);
                    continue;
                }
                if (this->type() == "publish") {
                    _router.publish(this->channel(), &sock, _inBuffer, nbytes);
                    continue;
                }
            }
        }
    }
    poco_debug(_logger, "Client disconnected");
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
