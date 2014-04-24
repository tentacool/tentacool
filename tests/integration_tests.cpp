#include "integration_tests.hpp"
#include "hpfeeds_client.hpp"
#include "broker.hpp"
#include <pthread.h>
#include <signal.h>
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

using namespace std;

void *run(void * args){
    Arguments* a = reinterpret_cast<Arguments*>(args);
    cout <<endl<<"Starting broker thread..." << endl;
    try{
        BrokerApplication app;
        app.run(a->size(), reinterpret_cast<char**>(a->data()) );
    }catch (Poco::Exception& exc){
        cerr << exc.displayText() << endl;
        //Util::Application::EXIT_SOFTWARE;
    }
    return NULL;
}
void Integration_test::setUp()
{

}
void Integration_test::tearDown()
{

}
void Integration_test::testConnectDisconnect()
{
    Arguments* a = new Arguments();
    a->insert(a->end(),"tentacool_integration_test");
    a->insert(a->end(),"-m");
    a->insert(a->end(),"file");
    a->insert(a->end(),"-v");
    a->insert(a->end(),"-d");
    a->insert(a->end(),"-f");
    a->insert(a->end(),"data/auth_keys.dat");

    pthread_t broker_thread = startBroker(a);
    sleep(2); //Wait for Broker setup
    Hpfeeds_client client;
    client.connect();
    sleep(1);
    client.disconnect();
    stopBroker(broker_thread);
    sleep(1);
    delete a;
}
void Integration_test::testAuthentication()
{
    Arguments* a = new Arguments();
    a->insert(a->end(),"tentacool_integration_test");
    a->insert(a->end(),"-m");
    a->insert(a->end(),"file");
    a->insert(a->end(),"-v");
    a->insert(a->end(),"-d");
    a->insert(a->end(),"-f");
    a->insert(a->end(),"data/auth_keys.dat");
    pthread_t broker_thread = startBroker(a);
    sleep(2); //Wait for Broker setup
    Hpfeeds_client client;
    client.connect();
    sleep(1);
    client.receive_info_message();
    client.send_auth_message("aldo", "s3cr3t");
    sleep(1);
    client.disconnect();
    stopBroker(broker_thread);
    sleep(1);
    delete a;
}
pthread_t Integration_test::startBroker(Arguments* args){
    pthread_t t1;
    int res = pthread_create(&t1, NULL, run, (void*)args);
    if (res){
        cout<<"Error during broker_thread creation: "<<res<<endl;
    }
    return t1;
}
void Integration_test::stopBroker(pthread_t t){
    cout<<"Killing broker_thread..."<<endl;
    pthread_kill(t,SIGINT);
}
Test *Integration_test::suite()
{
    TestSuite *suiteOfTests = new CppUnit::TestSuite( "Integration_test" );
    suiteOfTests->addTest(
            new CppUnit::TestCaller<Integration_test>("testConnectDisconnect",
                    &Integration_test::testConnectDisconnect));
    suiteOfTests->addTest(
            new CppUnit::TestCaller<Integration_test>("testAuthentication",
                    &Integration_test::testAuthentication));
    return suiteOfTests;
}

