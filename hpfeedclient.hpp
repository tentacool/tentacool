
#ifndef __CONNECTION_MANAGER_HPP__
#define __CONNECTION_MANAGER_HPP__

#include "Poco/Net/Socket.h"
#include "Poco/Net/TCPServerConnection.h"
#include "Poco/Logger.h"

#include <message_router.hpp>
#include <authenticator.hpp>

typedef enum {
    S_INIT,
    S_AUTHENTICATION_PROCEEDING,
    S_AUTHENTICATED,
} hpfeeds_server_state_t;

class HpFeedClient : public Poco::Net::TCPServerConnection {
    //! Reference to the client socket
    Poco::Net::StreamSocket& _sock;

    // The message router
    static MessageRouter _router;

    // The network buffer
    char _inBuffer[1000];

    //! The Poco logger
    Poco::Logger& _logger;

    //! The authentication subsystem
    Authenticator _auth;

    //! The server FSM state
    hpfeeds_server_state_t _state;

    //! Authentication routine
    void authUser();
public:
    //! Constructor
    HpFeedClient(const Poco::Net::StreamSocket& s);

    //! Destructor
    ~HpFeedClient();

    //! Client management main routine
    void run();

    //! The ip of the peer
    inline std::string ip();
};

#endif
