#ifndef __BROKER_MANAGER__
#define __BROKER_MANAGER__

#include <Poco/Net/Socket.h>
#include <Poco/Net/TCPServerConnection.h>
#include <Poco/Logger.h>
#include <string>
#include "message_router.hpp"
#include "authenticator.hpp"
#include "data_manager.hpp"

// 1KB
#define INITIAL_CHUNK 1024

//5 is the common header for all messages (msg_lenght (4) + opcode (1))
#define HEADER 5

//10MB
#define DATA 10*1024*1024

#define MAXBUF DATA + HEADER

using namespace std;

typedef enum {
    OP_ERROR,
    OP_INFO,
    OP_AUTH,
    OP_PUBLISH,
    OP_SUBSCRIBE,
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
static map<int, uint32_t>
        message_sizes = {{OP_ERROR,MAXBUF},
                        {OP_INFO, HEADER + 1 + 256 + 20},
                        {OP_AUTH, HEADER + 1 + 256 + 20},
                        {OP_PUBLISH, MAXBUF},
                        {OP_SUBSCRIBE, HEADER + 2 + 256 * 2},
                        {OP_UNSUBSCRIBE, HEADER + 2 + 256 * 2}};

//! BrokerConnection manage the main logic of the broker
class BrokerConnection : public Poco::Net::TCPServerConnection {
    Poco::Net::StreamSocket& _sock; //!< Reference to the client socket
public:
    //! Constructor
    BrokerConnection(const Poco::Net::StreamSocket& s, DataManager* data_manager);
    //! Destructor
    ~BrokerConnection();
    //! Client management main routine
    void run();
    //! The ip of the peer
    inline std::string ip();
    //! Hpfeeds Broker server name
    static string Broker_name;
    //! Stopping request
    static bool isStopped;
private:
    //! The message router
    static MessageRouter _router;

    //! The network buffer
    vector<char> _inBuffer;

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

    //! Send an error message to the client
    void sendErrorMsg(const string msg, bool sendToClient);
};

#endif
