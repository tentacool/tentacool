
#include <hpfeedbroker.hpp>

#include <iostream>
#include "Poco/Net/TCPServer.h"
#include "Poco/Net/TCPServerParams.h"
#include "Poco/Net/TCPServerConnectionFactory.h"
#include <hpfeedclient.hpp>
#include "Poco/Logger.h"
#include <sstream>

using namespace std;

HpFeedBroker::HpFeedBroker(int tcp_port, int threads, int queuelen, int idletime) :
    _port(tcp_port), _threads(threads), _queuelen(queuelen), _idletime(idletime),
    _logger(Poco::Logger::get("Tentacool.HpFeedBroker"))
{
}

HpFeedBroker::~HpFeedBroker()
{
    poco_debug(_logger, "Shutting down");
}

void HpFeedBroker::run()
{
    // Create a server socket to listen.
    poco_information_f1(_logger, "Starting server on port %d", _port);
    Poco::Net::ServerSocket svs(_port);

    // Configure some server params.
    Poco::Net::TCPServerParams* pParams = new Poco::Net::TCPServerParams();
    poco_debug_f1(_logger, "Setting threads: %d", _threads);
    pParams->setMaxThreads(_threads);
    poco_debug_f1(_logger, "Setting queue len: %d", _queuelen);
    pParams->setMaxQueued(_queuelen);
    poco_debug_f1(_logger, "Setting idletime: %d", _idletime);
    pParams->setThreadIdleTime(_idletime);

    // Create your server
    Poco::Net::TCPServer
        myServer(new Poco::Net::TCPServerConnectionFactoryImpl<HpFeedClient>(),
        svs, pParams);
    myServer.start();

    while(1) {
        ;
    }
}
