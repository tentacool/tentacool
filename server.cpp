
// g++ mt-server.cpp -o mt-server -lPocoNet -Wall -lPocoFoundation

#include <iostream>
#include <connection_manager.hpp>
#include <message_router.hpp>

using namespace std;

MessageRouter _router;

int main(int argc, char** argv)
{
    cout << "Starting server" << endl;

    // Create a server socket to listen.
    Poco::Net::ServerSocket svs(10000);

    // Configure some server params.
    Poco::Net::TCPServerParams* pParams = new Poco::Net::TCPServerParams();
    pParams->setMaxThreads(10);
    pParams->setMaxQueued(20);
    pParams->setThreadIdleTime(100);

    // Create your server
    Poco::Net::TCPServer myServer(new Poco::Net::TCPServerConnectionFactoryImpl<ConnectionManager>(), svs, pParams);
    myServer.start();

    while(1);

    return 0;
}
