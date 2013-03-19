
#ifndef __CONNECTION_MANAGER_HPP__
#define __CONNECTION_MANAGER_HPP__

#include "Poco/Net/TCPServer.h"
#include "Poco/Net/TCPServerParams.h"
#include "Poco/Net/TCPServerConnectionFactory.h"
#include "Poco/Net/TCPServerConnection.h"
#include "Poco/Net/Socket.h"
#include <message_router.hpp>

extern MessageRouter _router;

class ConnectionManager : public Poco::Net::TCPServerConnection {
public:
    ConnectionManager(const Poco::Net::StreamSocket& s) :
        Poco::Net::TCPServerConnection(s) {}

    void run();
};

#endif
