#ifndef __CONNECTION_MANAGER__
#define __CONNECTION_MANAGER__

#include <Poco/Net/Socket.h>
#include <Poco/Net/TCPServerConnection.h>
#include <Poco/Logger.h>
#include <Poco/RWLock.h>
#include <string>
#include "message_router.hpp"
#include "authenticator.hpp"
#include "data_manager.hpp"
#include "rw_lock_t.hpp"

#define DATA 10*1024*1024 //10MB
#define HEADER 5
#define MAXBUF DATA+HEADER //5 is the common header for all messages (msg_lenght (4) + opcode (1))
#define CHUNK 1024

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

//! This Map has the max size of every message type (op_code)
static map<int, uint32_t> message_sizes;

//! BrokerConnection managed the main logic of the broker
class BrokerConnection : public Poco::Net::TCPServerConnection {
    //! Reference to the client socket
    Poco::Net::StreamSocket& _sock; ///! Reference to the client socket
public:
    //! Constructor
    BrokerConnection(const Poco::Net::StreamSocket& s, DataManager* data_manager);
    //! Destructor
    ~BrokerConnection();
    void run();     //!< Client management main routine
    inline std::string ip();    //!< The ip of the peer
    static string Broker_name;  //!< Hpfeeds Broker server name
private:
    //! The message router
    static MessageRouter _router;

    //! The network buffer
    //char _inBuffer[MAXBUF];
    char _inBuffer[CHUNK];

    //! The Poco logger
    Poco::Logger& _logger;

    //! The authentication subsystem
    Authenticator _auth;

    //! Data
    DataManager* _data_manager;

    //! The server FSM state
    hpfeeds_server_state_t _state;

    //! Mutex for publishing
    static ReadWriteLock _publishing_lock;
    //static RWLock _publishing_lock;
    //! Authenticate the user exploiting the Authenticator
    void authUser();

    //! Send an error message to the client, if logMsg is true it
    //  write the error message in the logger
    void sendErrorMsg(const string msg, bool logMsg);
};

#endif
