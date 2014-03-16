
#ifndef __CONNECTION_MANAGER_HPP__
#define __CONNECTION_MANAGER_HPP__

#include "Poco/Net/Socket.h"
#include "Poco/Net/TCPServerConnection.h"
#include "Poco/Logger.h"
#include <string>
#include "message_router.hpp"
#include "Authenticator.hpp"
#include "DataManager.hpp"
using namespace std;


/*extern DataManager data_manager;*/

typedef enum {
    S_INIT,
    S_AUTHENTICATION_PROCEEDING,
    S_AUTHENTICATED,
    S_SUBSCRIBED,
    S_ERROR
} hpfeeds_server_state_t;


class HpfeedsBrokerConnection : public Poco::Net::TCPServerConnection {
    //! Reference to the client socket
    Poco::Net::StreamSocket& _sock; ///! Reference to the client socket

    //! The message router
    static MessageRouter _router;

    //! The network buffer
    char _inBuffer[1000];

    //! The Poco logger
    Poco::Logger& _logger;

    //! The authentication subsystem
    Authenticator _auth;

    //! Data
    DataManager* _data_manager;

    //! The server FSM state
    hpfeeds_server_state_t _state;

    //! Authenticate the user exploiting the Authenticator
    void authUser();
public:
    //! Constructor
    HpfeedsBrokerConnection(const Poco::Net::StreamSocket& s, DataManager* data_manager);

    //! Destructor
    ~HpfeedsBrokerConnection();

    //! Client management main routine
    void run();

    //! The ip of the peer
    inline std::string ip();

    //! Hpfeeds Broker server name
	static string Broker_name;
};

#endif
