
#include <hpfeedclient.hpp>
#include <iostream>

#include <stdio.h>
#include <string.h>
#include <hpfeedbroker.hpp>

extern "C" {
#include <hpfeeds.h>
}

using namespace std;

MessageRouter HpFeedClient::_router = MessageRouter();

HpFeedClient::HpFeedClient(const Poco::Net::StreamSocket& s) :
        Poco::Net::TCPServerConnection(s),
        _sock(this->socket()),
        _logger(Poco::Logger::get("Tentacool.HpFeedClient")),
        _state(S_INIT)
{
}

inline string HpFeedClient::ip()
{
    return _sock.peerAddress().host().toString();
}

void HpFeedClient::run()
{
    //_sock = this->socket();
    poco_information_f1(_logger, "New connection from: %s", this->ip());
    bool isOpen = true;
    Poco::Timespan timeOut(10, 0);
    int nbytes;
	hpf_msg_t *msg;

    // Start the authentication phase
    msg = hpf_msg_info(_auth.genNonce(), (u_char*)HpFeedBroker::name.data(), HpFeedBroker::name.size());
    _sock.sendBytes(msg, ntohl(msg->hdr.msglen));
    hpf_msg_delete(msg);
    _state = S_AUTHENTICATION_PROCEEDING;

    while (isOpen) {
        memset(_inBuffer, 0x0, 1000);

        if (!_sock.poll(timeOut,Poco::Net::Socket::SELECT_READ) == false) {
            nbytes = -1;

            try {
                // Receiving bytes from client
                nbytes = _sock.receiveBytes(_inBuffer, sizeof(_inBuffer));
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
                switch (_state) {
                    case S_AUTHENTICATION_PROCEEDING:
                        this->authUser();
                        if (_state != S_AUTHENTICATED) {
                            return ;
                        }
                        break;
                    case S_AUTHENTICATED:
                        // Read message from socket
                        msg = (hpf_msg_t *)_inBuffer;
                        cout << "OPCODE " << int(msg->hdr.opcode) << endl;
                        switch (msg->hdr.opcode) {
                            // XXX
                            case 4:
                                poco_debug(_logger, "Subscribe incoming");
                                break;
                            default:
                                break;
                        }
                        break;
                    default:
                        poco_error_f1(_logger, "Unhandled state: %d", _state);
                        break;
                }
            }
        }
    }
    poco_debug(_logger, "Client disconnected");
}

void HpFeedClient::authUser()
{
	hpf_chunk_t *chunk;
	hpf_msg_t* msg = (hpf_msg_t *)_inBuffer;
    chunk = hpf_msg_get_chunk((u_char*)_inBuffer + sizeof(msg->hdr),
        ntohl(msg->hdr.msglen) - sizeof(msg->hdr));

    string username((char*)chunk->data, chunk->len);
    string secret(_inBuffer + sizeof(msg->hdr) + 1 + chunk->len,
        int(ntohl(msg->hdr.msglen) - sizeof(msg->hdr) - 1 - chunk->len));

    poco_debug_f1(_logger, "Getting auth request for %s", username);

    if (_auth.authenticate(username, secret) == true) {
        poco_information_f1(_logger, "User %s authenticated", username);
        _state = S_AUTHENTICATED;
    } else {
        poco_information_f1(_logger, "Authentication failed for %s", username);
        msg = hpf_msg_error((u_char*)"Authentication failed", 21);
        _sock.sendBytes(msg, ntohl(msg->hdr.msglen));
        hpf_msg_delete(msg);
        _state = S_INIT;
    }
}

HpFeedClient::~HpFeedClient()
{
}
