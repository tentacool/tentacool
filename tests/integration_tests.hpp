#ifndef __INTEGRATION_TESTS__
#define __INTEGRATION_TESTS__

#include <pthread.h>
#include <vector>
#include "hpfeeds_client.hpp"
#include <cppunit/TestFixture.h>
#include <cppunit/TestCase.h>

using namespace CppUnit;
using namespace std;

typedef vector<string> Arguments;

class Integration_test: public TestFixture
{
public:
    void setUp();
    void tearDown();
    pthread_t startBroker(Arguments* args);
    void stopBroker(pthread_t t);
    void testHelp();
    void testConnectDisconnect();
    void testDifferentPortAndName();
    void testWithMongo();
    void testWrongOpCode();
    void testWrongTotalLength();
    void testAuthentication();
    void testSubscribe();
    void testDoubleSubscribe();
    void testSubscribeNotAllowed();
    void testPublish();
    void testPublishNoChannel();
    void testPublishNotAllowed();
    void testPublishBigMessage();
    void testPublishConcurrency();
    static Test *suite();
};

#endif
