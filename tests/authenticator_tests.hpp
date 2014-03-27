#ifndef __AUTHENTICATORTEST__
#define __AUTHENTICATORTEST__
#include <string>
#include "authenticator.hpp"
#include <cppunit/TestListener.h>
#include <cppunit/CompilerOutputter.h>
#include <cppunit/extensions/TestFactoryRegistry.h>
#include <cppunit/TestResult.h>
#include <cppunit/TestResultCollector.h>
#include "cppunit/TestFixture.h"
#include "cppunit/TestCase.h"
#include "cppunit/TestSuite.h"
#include "cppunit/TestResult.h"
#include "cppunit/TestCaller.h"
#include <cppunit/BriefTestProgressListener.h>
#include "cppunit/TestRunner.h"
#include "cppunit/ui/text/TestRunner.h"
#include <cppunit/extensions/HelperMacros.h>

#include "Poco/SHA1Engine.h"
#include "Poco/DigestEngine.h"

using namespace CppUnit;
using namespace std;

class Authenticator_test : public TestFixture
{
private:
    Authenticator* a1;
    Authenticator* a2;
    Authenticator* a3;
    Authenticator* a4;
    //Authenticator a4 = new Authenticator(1234);
    uint32_t nonce_a1, nonce_a2;
    string not_initialized_string;
    string hash;

public:

    void setUp()
    {
      //Seed = 0
      a1 = new Authenticator();
      a2 = new Authenticator();
      a3 = new Authenticator(1234);
      a4 = new Authenticator(1234);
      //Specified Seed
      nonce_a1 = a1->nonce();
      nonce_a2 = a2->nonce();
    }

	void tearDown(){
	}

    string getHash(uint32_t nonce, string password)
    {
        //Preparing the hash to use
        Poco::SHA1Engine sha1;
        Poco::DigestEngine::Digest hash_d;
        sha1.update((unsigned char*)&nonce, 4);
        sha1.update(password.data(), password.size());
        hash_d = sha1.digest();
        string hash(hash_d.begin(),hash_d.end());
        return hash;
    }

    void testNonce()
    {
    CPPUNIT_ASSERT( a1->nonce() != a2->nonce() );
    CPPUNIT_ASSERT( a1->nonce() == a1->nonce() );
    }

    void testNonceSeed()
    {
        CPPUNIT_ASSERT( a3->nonce() == a4->nonce() );
    }

    void testAuthenticate()
    {
        hash = getHash(nonce_a1, "s3cr3t");
        string wrong_hash = hash+"12";

        CPPUNIT_ASSERT( a1->authenticate(hash,"s3cr3t") == true);
        CPPUNIT_ASSERT( a1->authenticate(hash,"wrong password") == false);
        CPPUNIT_ASSERT( a1->authenticate(not_initialized_string,"s3cr3t") == false);
        CPPUNIT_ASSERT( a1->authenticate(wrong_hash,"s3cr3t") == false);
    }

    void testAuthenticateChangeNonce()
    {
        //Set another nonce
        nonce_a1 = a1->genNonce();
        hash = getHash(nonce_a1, "s3cr3t");
        string wrong_hash = hash+"12";

        CPPUNIT_ASSERT( a1->authenticate(hash,"s3cr3t") == true);
        CPPUNIT_ASSERT( a1->authenticate(hash,"wrong password") == false);
        CPPUNIT_ASSERT( a1->authenticate(not_initialized_string,"s3cr3t") == false);
        CPPUNIT_ASSERT( a1->authenticate(wrong_hash,"s3cr3t") == false);
    }

    static Test *suite()
    {
        TestSuite *suiteOfTests = new CppUnit::TestSuite( "Authenticator_test" );
        suiteOfTests->addTest( new CppUnit::TestCaller<Authenticator_test>("testNonce",&Authenticator_test::testNonce));
        suiteOfTests->addTest( new CppUnit::TestCaller<Authenticator_test>("testNonceSeed",&Authenticator_test::testNonceSeed));
        suiteOfTests->addTest( new CppUnit::TestCaller<Authenticator_test>("testAuthenticate",&Authenticator_test::testAuthenticate));
        suiteOfTests->addTest( new CppUnit::TestCaller<Authenticator_test>("testAuthenticateChangeNonce",&Authenticator_test::testAuthenticateChangeNonce));
        return suiteOfTests;
    }
};

#endif /* __AUTHENTICATORTEST__ */

