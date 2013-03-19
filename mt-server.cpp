
// g++ mt-server.cpp -o mt-server -lPocoNet -Wall -lPocoFoundation

#include <iostream>
#include "Poco/Net/TCPServer.h"
#include "Poco/Net/TCPServerParams.h"
#include "Poco/Net/TCPServerConnectionFactory.h"
#include "Poco/Net/TCPServerConnection.h"
#include "Poco/Net/Socket.h"

using namespace std;

class ConnectionManager : public Poco::Net::TCPServerConnection {
public:
    ConnectionManager(const Poco::Net::StreamSocket& s) :
        Poco::Net::TCPServerConnection(s) {}

    void run()
    {
        cout << "New connection from: " << socket().peerAddress().host().toString() <<  endl << flush;
        bool isOpen = true;
        Poco::Timespan timeOut(10, 0);
        unsigned char incommingBuffer[1000];
        while(isOpen) {
            if (socket().poll(timeOut,Poco::Net::Socket::SELECT_READ) == false){
                cout << "TIMEOUT!" << endl << flush;
            } else {
                cout << "RX EVENT!!! ---> " << flush;
                int nBytes = -1;

                try {
                    nBytes = socket().receiveBytes(incommingBuffer, sizeof(incommingBuffer));
                } catch (Poco::Exception& exc) {
                    //Handle your network errors.
                    cerr << "Network error: " << exc.displayText() << endl;
                    isOpen = false;
                }

                if (nBytes==0) {
                    cout << "Client closes connection!" << endl << flush;
                    isOpen = false;
                } else {
                    cout << "Receiving nBytes: " << nBytes << endl << flush;
                }
            }
        }
        cout << "Connection finished!" << endl << flush;
    }
};

int main(int argc, char** argv)
{
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
