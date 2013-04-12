
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
    _port(tcp_port), _threads(threads), _queuelen(queuelen), _idletime(idletime)
{
}

void HpFeedBroker::run()
{
    stringstream ss;
    Poco::Logger& logger = Poco::Logger::get("Tentacool.HpFeedBroker");
    ss << "Starting server on port " << _port;
    logger.information(ss.str());

    // Create a server socket to listen.
    Poco::Net::ServerSocket svs(_port);

    // Configure some server params.
    Poco::Net::TCPServerParams* pParams = new Poco::Net::TCPServerParams();
    pParams->setMaxThreads(_threads);
    pParams->setMaxQueued(_queuelen);
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
