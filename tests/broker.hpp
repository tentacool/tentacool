#ifndef __BROKER__
#define __BROKER__

#include <iostream>
#include "data_manager.hpp"
#include <Poco/String.h>
#include <Poco/Message.h>
#include <Poco/Logger.h>
#include <Poco/Util/ServerApplication.h>
#include <Poco/Util/Option.h>
#include <Poco/Util/OptionSet.h>
#include "Poco/Net/TCPServer.h"
#include "Poco/Net/TCPServerParams.h"
#include "Poco/Net/TCPServerConnectionFactory.h"
#include <vector>

using namespace std;
using namespace Poco;

//!
//!To test the BrokerApplication you can use any terminal with "telnet localhost 10000".
//!10000 is the default port.
class BrokerApplication : public Util::ServerApplication
{
public:

    BrokerApplication();
    ~BrokerApplication();
protected:
    void initialize(Util::Application& self);
    void uninitialize();
    void defineOptions(Util::OptionSet& options);
    void handleOption(const string& name, const string& value);
    void handleMode(const string& name, const string& value);
    void handlePort(const string& name, const string& value);
    void displayHelp();
    int main(const vector<string>& args);
private:
    bool m_helpRequested, _debug_mode;
    Logger& logger;
    unsigned short port;
    int num_threads;
    int queuelen;
    int idletime;
    bool _data_mode;
    bool _stdout_logging;
    string _log_file;
    string _filename;
    string _mongo_ip;
    string _mongo_port;
    string _mongo_db;
    string _mongo_collection;
    DataManager* _data_manager;
};

class TCPConnectionFactory : public Net::TCPServerConnectionFactory
{
public:
    TCPConnectionFactory(DataManager* data_m);
    ~TCPConnectionFactory();
    Net::TCPServerConnection* createConnection(const Poco::Net::StreamSocket& socket);
private:
    DataManager*    _data_m;
};

#endif
