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
void Hpfeeds_test::testCreatePublishErr()
{
    u_char* data = NULL;
    hpf_msg_t*  pub = NULL;
    CPPUNIT_ASSERT_THROW(pub=hpf_msg_publish("aldo","ch1", data, 0),Poco::Exception);
    hpf_msg_delete(pub);
}
void Hpfeeds_test::testCreatePublishErr2()
{
    vector<u_char> publish_data;
    publish_data.insert(publish_data.end(),'d');
    hpf_msg_t*  pub = NULL;
    //Wrong lenght
    CPPUNIT_ASSERT_THROW(pub=hpf_msg_publish("aldo","ch1",
            publish_data.data(), 0),Poco::Exception);
    hpf_msg_delete(pub);
}
void Hpfeeds_test::testCreateInfoMsg()
{
    //memcmp returns 0 -> equal
    string broker_name ="@hp1";
    hpf_msg_t*  info_1 = hpf_msg_info(genNonce(), broker_name);
    hpf_msg_t*  info_2 = hpf_msg_info(genNonce(1234), broker_name);
    CPPUNIT_ASSERT(memcmp(info_1, info_2,/*sizeof(info_1)*/ ntohl(info_1->hdr.msglen))!=0);
    hpf_msg_delete(info_1);
    hpf_msg_delete(info_2);
}
void Hpfeeds_test::testMsgData(){
    string broker_name ="@hp1";
        hpf_msg_t*  info = hpf_msg_info(genNonce(), broker_name);
        CPPUNIT_ASSERT(hpf_msg_gettype(info)==OP_INFO &&
                       hpf_msg_getsize(info)==14);
        hpf_msg_delete(info);
}
void Hpfeeds_test::testCreateErrMsg(){
    //memcmp returns 0 -> equal
    string err_msg ="There was an error";
    hpf_msg_t*  err_1 = hpf_msg_error(err_msg);
    hpf_msg_t*  err_2 = hpf_msg_error(err_msg);
    CPPUNIT_ASSERT(memcmp(err_1, err_2, sizeof(err_1))==0);
    hpf_msg_delete(err_1);
    hpf_msg_delete(err_2);
}
void Hpfeeds_test::testCreatePublishMsg(){
    string p  = "A lot of interesting data";
    vector<u_char> publish_data;
    publish_data.resize(p.length());
    memcpy(&publish_data[0],p.data(),p.length());

    hpf_msg_t*  pub_1 = hpf_msg_publish("aldo","ch1",publish_data.data(),
                                                     publish_data.size());
    hpf_msg_t*  pub_2 = hpf_msg_publish("aldo","ch1",publish_data.data(),
                                                     publish_data.size());
    CPPUNIT_ASSERT(memcmp(pub_1, pub_2, sizeof(pub_1))==0);
    hpf_msg_delete(pub_1);
    hpf_msg_delete(pub_2);
}
void Hpfeeds_test::testCreateSubscribeMsg()
{
    hpf_msg_t*  sub_1 = hpf_msg_subscribe("aldo","ch1");
    hpf_msg_t*  sub_2 = hpf_msg_subscribe("aldo","ch1");
    CPPUNIT_ASSERT(memcmp(sub_1, sub_2, sizeof(sub_1))==0);
    hpf_msg_delete(sub_1);
    hpf_msg_delete(sub_2);
}
void Hpfeeds_test::testCreateAuthMsg()
{
    hpf_msg_t*  auth_1 = hpf_msg_auth(1234, "aldo", "s3cr3t");
    hpf_msg_t*  auth_2 = hpf_msg_auth(1234, "aldo","s3cr3t");
    CPPUNIT_ASSERT(memcmp(auth_1, auth_2, sizeof(auth_1))==0);
    hpf_msg_delete(auth_1);
    hpf_msg_delete(auth_2);
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
