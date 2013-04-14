
#ifndef __CONNECTION_MANAGER_HPP__
#define __CONNECTION_MANAGER_HPP__

#include "Poco/Net/Socket.h"
#include <message_router.hpp>
#include "Poco/Net/TCPServerConnection.h"
#include "Poco/Logger.h"

class HpFeedClient : public Poco::Net::TCPServerConnection {
    // The message router
    static MessageRouter _router;

    // The network buffer
    char _inBuffer[1000];

    std::string type();

    std::string channel();

    //! The Poco logger
    Poco::Logger& _logger;
public:
    HpFeedClient(const Poco::Net::StreamSocket& s);

    void run();

    inline std::string ip();
};

#endif
