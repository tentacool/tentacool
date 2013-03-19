
#include <connection_manager.hpp>
#include <iostream>

#include <stdio.h>
#include <string.h>

using namespace std;

void ConnectionManager::run()
{
    Poco::Net::StreamSocket& sock = socket();
    cout << "New connection from: " << sock.peerAddress().host().toString() <<  endl << flush;
    bool isOpen = true;
    Poco::Timespan timeOut(10, 0);
    unsigned char inBuffer[1000];
    char channel[100];

    while (isOpen) {
        memset(inBuffer, 0x0, 1000);
        if (sock.poll(timeOut,Poco::Net::Socket::SELECT_READ) == false) {
            cout << "TIMEOUT!" << endl << flush;
        } else {
            //cout << "RX EVENT!!! ---> " << flush;
            int nbytes = -1;

            try {
                nbytes = sock.receiveBytes(inBuffer, sizeof(inBuffer));
            } catch (Poco::Exception& exc) {
                // Handle your network errors.
                cerr << "Network error: " << exc.displayText() << endl;
                isOpen = false;
            }

            if (nbytes == 0) {
                cout << "Client closes connection!" << endl << flush;
                isOpen = false;
            } else {
                sscanf((char*)inBuffer, "%s", channel);
                _router.subscribe(channel, sock);
                //cout << "AAAA " << channel << endl;
                //cout << "Receiving nBytes: " << nbytes << endl << flush;
                //cout << "Transmitting back." << endl << flush;
                //if (sock.sendBytes(inBuffer, nbytes) != nbytes) {
                //    cerr << "Error sending bytes back" << endl << flush;
                //}
                _router.route(channel, string((char*)inBuffer, nbytes));
            }
        }
    }
    cout << "Connection finished!" << endl << flush;
}
