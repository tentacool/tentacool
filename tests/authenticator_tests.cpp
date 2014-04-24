#include "authenticator_tests.hpp"
#include "authenticator.hpp"
#include <cppunit/TestListener.h>
#include <cppunit/CompilerOutputter.h>
#include <cppunit/extensions/TestFactoryRegistry.h>
#include <cppunit/TestResult.h>
#include <cppunit/TestResultCollector.h>
#include <cppunit/TestFixture.h>
#include <cppunit/TestCase.h>
#include <cppunit/TestSuite.h>
#include <cppunit/TestResult.h>
#include <cppunit/TestCaller.h>
#include <cppunit/BriefTestProgressListener.h>
#include <cppunit/TestRunner.h>
#include <cppunit/ui/text/TestRunner.h>
#include <cppunit/extensions/HelperMacros.h>
#include <Poco/SHA1Engine.h>
#include <Poco/DigestEngine.h>

using namespace CppUnit;
using namespace std;

void Authenticator_test::setUp()
{
    nonce_a1 = a1.nonce();
    nonce_a2 = a2.nonce();
}

void Authenticator_test::tearDown()
{
}

string Authenticator_test::getHash(uint32_t nonce, string password)
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

void Authenticator_test::testNonce()
{
    CPPUNIT_ASSERT( a1.nonce() != a2.nonce() );
    CPPUNIT_ASSERT( a1.nonce() == a1.nonce() );
}

void Authenticator_test::testNonceSeed()
{
    //Specified Seed
    Authenticator a3(1234);
    Authenticator a4(1234);

    CPPUNIT_ASSERT( a3.nonce() == a4.nonce() );
}

void Authenticator_test::testAuthenticate()
{
    hash = getHash(nonce_a1, "s3cr3t");
    string wrong_hash = hash+"12";

    CPPUNIT_ASSERT( a1.authenticate(hash,"s3cr3t") == true);
    CPPUNIT_ASSERT( a1.authenticate(hash,"wrong password") == false);
    CPPUNIT_ASSERT( a1.authenticate(empty_string,"s3cr3t") == false);
    CPPUNIT_ASSERT( a1.authenticate(wrong_hash,"s3cr3t") == false);
}

void Authenticator_test::testAuthenticateChangeNonce()
{
    //Set another nonce
    nonce_a1 = a1.genNonce();
    hash = getHash(nonce_a1, "s3cr3t");
    string wrong_hash = hash+"12";

    CPPUNIT_ASSERT( a1.authenticate(hash,"s3cr3t") == true);
    CPPUNIT_ASSERT( a1.authenticate(hash,"wrong password") == false);
    CPPUNIT_ASSERT( a1.authenticate(empty_string,"s3cr3t") == false);
    CPPUNIT_ASSERT( a1.authenticate(wrong_hash,"s3cr3t") == false);
}

Test *Authenticator_test::suite()
{
    TestSuite *suiteOfTests = new CppUnit::TestSuite( "Authenticator_test" );
    suiteOfTests->addTest(
            new CppUnit::TestCaller<Authenticator_test>("testNonce",
                    &Authenticator_test::testNonce));
    suiteOfTests->addTest(
            new CppUnit::TestCaller<Authenticator_test>("testNonceSeed",
                    &Authenticator_test::testNonceSeed));
    suiteOfTests->addTest(
            new CppUnit::TestCaller<Authenticator_test>("testAuthenticate",
                    &Authenticator_test::testAuthenticate));
    suiteOfTests->addTest(
            new CppUnit::TestCaller<Authenticator_test>(
                    "testAuthenticateChangeNonce",
                    &Authenticator_test::testAuthenticateChangeNonce));
    return suiteOfTests;
}
