
#ifndef __BROKER_HPP__
#define __BROKER_HPP__

#include "Poco/Logger.h"
#include <string>

class HpFeedBroker {
    int _port;
    int _threads;
    int _queuelen;
    int _idletime;
    Poco::Logger& _logger;
public:
    HpFeedBroker(int tcp_port = 10000, int threads = 10, int queuelen = 20,
        int idletime = 100);

    static std::string name;

    //! Maint broker routine
    void run();

    ~HpFeedBroker();
};

#endif
