#include "hpfeeds_tests.hpp"
#include "hpfeeds.hpp"
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
#include <Poco/Random.h>
#include <Poco/NumberFormatter.h>

void Hpfeeds_test::setUp()
{

}

void Hpfeeds_test::tearDown()
{

}

uint32_t Hpfeeds_test::genNonce(int seed){
    Poco::Random _prng;
    if(seed > 0){
        _prng.seed(seed);
    }else{
        _prng.seed();
    }
    return _prng.next();
}

void Hpfeeds_test::testCreateInfoMsg()
{
    //memcmp returns 0 -> equal
    string broker_name ="@hp1";
    hpf_msg  info_1 = hpf_info(genNonce(), broker_name);
    hpf_msg  info_2 = hpf_info(genNonce(1234), broker_name);
    CPPUNIT_ASSERT(memcmp(info_1.data(), info_2.data(), info_1.size()) != 0);

//    cout<<endl;
//    for( std::vector<u_char>::const_iterator i = pub_1.begin(); i != pub_1.end(); ++i)
//        std::cout << Poco::NumberFormatter::formatHex(*i) << ' ';
//    cout<<endl;
}

void Hpfeeds_test::testMsgData(){
    string broker_name ="@hp1";
    hpf_msg info = hpf_info(genNonce(), broker_name);
    CPPUNIT_ASSERT(hpf_gettype(info) == OP_INFO && hpf_getsize(info) == 14);
}

void Hpfeeds_test::testCreateErrMsg(){
    string err_msg ="There was an error";
    hpf_msg err_1 = hpf_error(err_msg);
    hpf_msg err_2 = hpf_error(err_msg);
    CPPUNIT_ASSERT(memcmp(err_1.data(), err_2.data(), err_1.size()) == 0);

}

void Hpfeeds_test::testCreateAuthMsg()
{
    hpf_msg auth_1 = hpf_auth(1234, "aldo", "s3cr3t");
    hpf_msg auth_2 = hpf_auth(1234, "aldo","s3cr3t");
    CPPUNIT_ASSERT(memcmp(auth_1.data(), auth_2.data(), sizeof(auth_1)) == 0);
}

void Hpfeeds_test::testCreateSubscribeMsg()
{
    hpf_msg sub_1 = hpf_subscribe("aldo","ch1");
    hpf_msg sub_2 = hpf_subscribe("aldo","ch1");
    CPPUNIT_ASSERT(memcmp(sub_1.data(), sub_2.data(), sub_1.size()) == 0);
}

void Hpfeeds_test::testCreatePublishMsg(){
    string p  = "A lot of interesting data";
    vector<u_char> publish_data;
    publish_data.resize(p.length());
    copy(p.data(), p.data() + p.length(), publish_data.begin());

    hpf_msg pub_1 = hpf_publish("aldo","ch1",publish_data.data(),
                                                     publish_data.size());
    hpf_msg pub_2 = hpf_publish("aldo","ch1",publish_data.data(),
                                                     publish_data.size());
    CPPUNIT_ASSERT(memcmp(pub_1.data(), pub_2.data(), pub_1.size()) == 0);
}

void Hpfeeds_test::testCreatePublishErr()
{
    u_char* data = NULL;
    hpf_msg pub;
    CPPUNIT_ASSERT_THROW(pub = hpf_publish("aldo","ch1", data, 0),
                                                           Poco::Exception);
}

void Hpfeeds_test::testCreatePublishErr2()
{
    vector<u_char> publish_data;
    publish_data.insert(publish_data.end(),'d');
    hpf_msg pub;
    //Wrong lenght
    CPPUNIT_ASSERT_THROW(pub = hpf_publish("aldo","ch1",
                                    publish_data.data(), 0), Poco::Exception);
}

Test *Hpfeeds_test::suite()
{
    TestSuite *suiteOfTests = new CppUnit::TestSuite( "Hpfeeds_test" );
    suiteOfTests->addTest(
            new CppUnit::TestCaller<Hpfeeds_test>("testCreateInfoMsg",
                    &Hpfeeds_test::testCreateInfoMsg));

    suiteOfTests->addTest(
                new CppUnit::TestCaller<Hpfeeds_test>("testMsgData",
                        &Hpfeeds_test::testMsgData));

    suiteOfTests->addTest(
                new CppUnit::TestCaller<Hpfeeds_test>("testCreateErrMsg",
                        &Hpfeeds_test::testCreateErrMsg));

    suiteOfTests->addTest(
                new CppUnit::TestCaller<Hpfeeds_test>("testCreateAuthMsg",
                        &Hpfeeds_test::testCreateAuthMsg));

    suiteOfTests->addTest(
                new CppUnit::TestCaller<Hpfeeds_test>("testCreateSubscribeMsg",
                        &Hpfeeds_test::testCreateSubscribeMsg));

    suiteOfTests->addTest(
                new CppUnit::TestCaller<Hpfeeds_test>("testCreatePublishMsg",
                        &Hpfeeds_test::testCreatePublishMsg));

    suiteOfTests->addTest(
                new CppUnit::TestCaller<Hpfeeds_test>("testCreatePublishErr",
                        &Hpfeeds_test::testCreatePublishErr));

    suiteOfTests->addTest(
                new CppUnit::TestCaller<Hpfeeds_test>("testCreatePublishErr2",
                        &Hpfeeds_test::testCreatePublishErr2));
    return suiteOfTests;
}
