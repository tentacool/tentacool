
#ifndef __BROKER_HPP__
#define __BROKER_HPP__

class HpFeedBroker {
    int _port;
    int _threads;
    int _queuelen;
    int _idletime;
public:
    HpFeedBroker(int tcp_port = 10000, int threads = 10, int queuelen = 20,
        int idletime = 100);

    void run();
};

#endif
