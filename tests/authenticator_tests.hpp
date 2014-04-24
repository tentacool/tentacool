#ifndef __AUTHENTICATORTESTS__
#define __AUTHENTICATORTESTS__

#include "authenticator.hpp"
#include <cppunit/TestFixture.h>
#include <cppunit/TestCase.h>

using namespace CppUnit;
using namespace std;

class Authenticator_test : public TestFixture
{
private:
    Authenticator a1;   //Seed = 0 by default
    Authenticator a2;   //Seed = 0 by default
    uint32_t nonce_a1, nonce_a2;
    string empty_string;
    string hash;

public:

    void setUp();

    void tearDown();

    string getHash(uint32_t nonce, string password);

    void testNonce();

    void testNonceSeed();

    void testAuthenticate();

    void testAuthenticateChangeNonce();

    static Test *suite();

};

#endif
