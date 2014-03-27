
#ifndef __CONNECTION_MANAGER__
#define __CONNECTION_MANAGER__

#include "Poco/Net/Socket.h"
#include "Poco/Net/TCPServerConnection.h"
#include "Poco/Logger.h"
#include <string>
#include "message_router.hpp"
#include "authenticator.hpp"
#include "data_manager.hpp"


#define DATA 10*1024*1024 //10MB
#define MAXBUF DATA+5 //5 is the common header for all messages (msg_lenght (4) + opcode (1))

using namespace std;

typedef enum {
	 OP_ERROR ,
	 OP_INFO ,
	 OP_AUTH,
	 OP_PUBLISH ,
	 OP_SUBSCRIBE ,
	 OP_UNSUBSCRIBE
} OP_CODES;

typedef enum {
    S_INIT,
    S_AUTHENTICATION_PROCEEDING,
    S_AUTHENTICATED,
    S_SUBSCRIBED,
    S_ERROR
} hpfeeds_server_state_t;

static map<int, uint32_t> message_sizes;


class HpfeedsBrokerConnection : public Poco::Net::TCPServerConnection {
    //! Reference to the client socket
    Poco::Net::StreamSocket& _sock; ///! Reference to the client socket

    //! The message router
    static MessageRouter _router;

    //! The network buffer
    char _inBuffer[MAXBUF];

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
