#ifndef __INTEGRATION_TESTS__
#define __INTEGRATION_TESTS__

#include <pthread.h>
#include <vector>
#include "hpfeeds_client.hpp"
#include <cppunit/TestFixture.h>
#include <cppunit/TestCase.h>

#define BROKER_SETUP_WAIT 1
#define BROKER_GENERIC_WAIT 1
#define BROKER_DELETE_WAIT 1
#define BROKER_OPS_COMPLETION 2

using namespace CppUnit;
using namespace std;

typedef vector<string> Arguments;

class Integration_test: public TestFixture
{
public:
    Integration_test();
    void setUp();
    void tearDown();
    int getPath(char* pBuf);
    pthread_t startBroker(Arguments* args);
    void stopBroker(pthread_t t);
    void testHelp();
    void testServerParams();
    void testDebugSettings();
    void testConnectDisconnect();
    void testDifferentPortAndName();
    void testWrongFile();
    void testWithMongo();
    void testWrongOpCode();
    void testWrongTotalLength();
    void testAuthentication();
    void testFailAuthentication();
    void testWrongAuthenticationMsg();
    void testTooLongAuthenticationMsg();
    void testTooBigMessage();
    void testSubscribe();
    void testDoubleSubscribe();
    void testSubscribeNotAllowed();
    void testPublish();
    void testPublishNoChannel();
    void testPublishNotAllowed();
    void testPublishBigMessage();
    void testPublishConcurrency();
    static Test *suite();
private:
    string _exe_path;
};
#endif
