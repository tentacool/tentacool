#ifndef __HPFEEDS_TESTS__
#define __HPFEEDS_TESTS__

#include "hpfeeds.hpp"
#include <cppunit/TestFixture.h>
#include <cppunit/TestCase.h>

using namespace CppUnit;
using namespace std;

class Hpfeeds_test : public TestFixture
{
private:

public:
    void setUp();
    void tearDown();
    void testCreateInfoMsg();
    void testMsgData();
    void testCreateErrMsg();
    void testCreatePublishMsg();
    uint32_t genNonce(int seed = 0);
    static Test *suite();
};

#endif
