#include "integration_tests.hpp"
#include "hpfeeds_client.hpp"
#include "broker.hpp"
#include <pthread.h>
#include <signal.h>
#include <vector>
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
    }
    return NULL;
}
void *run_publisher(void * args){
    vector<string>* data = reinterpret_cast<vector<string>*>(args);
    cout<<"Starting publisher: "<<data->at(0)<<endl;
    Hpfeeds_client client_pub;
    try{
        client_pub.connect();
        sleep(1);
        client_pub.receive_info_message();
        client_pub.send_auth_message(data->at(1), data->at(2));
        client_pub.send_publish_message(data->at(1), data->at(3), data->at(4));
        sleep(1);
        client_pub.disconnect();
    }catch (Poco::Exception& exc){
        cerr << exc.displayText() << endl;
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
void Integration_test::testDifferentPortAndName()
{
    Arguments* a = new Arguments();
    a->insert(a->end(),"tentacool_integration_test");
    a->insert(a->end(),"-m");
    a->insert(a->end(),"file");
    a->insert(a->end(),"-v");
    a->insert(a->end(),"-f");
    a->insert(a->end(),"data/auth_keys.dat");
    a->insert(a->end(),"-p");
    a->insert(a->end(),"10127");
    a->insert(a->end(),"-n");
    a->insert(a->end(),"Broker2");
    pthread_t broker_thread = startBroker(a);
    sleep(2); //Wait for Broker setup
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
void Integration_test::testSubscribe()
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
    client.send_subscribe_message("aldo", "ch1");
    sleep(1);
    client.disconnect();
    stopBroker(broker_thread);
    sleep(1);
    delete a;
}
void Integration_test::testPublish()
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
    Hpfeeds_client client_sub;
    Hpfeeds_client client_pub;
    //SUBSCRIBER
    client_sub.connect();
    sleep(1);
    client_sub.receive_info_message();
    client_sub.send_auth_message("aldo", "s3cr3t");
    client_sub.send_subscribe_message("aldo", "ch1");
    //PUBLISHER
    client_pub.connect();
    sleep(1);
    client_pub.receive_info_message();
    client_pub.send_auth_message("filippo", "pwd");
    client_pub.send_publish_message("filippo", "ch1", "testing");
    client_pub.disconnect();
    //SUBSCRIBER RECEIVE
    sleep(1);
    client_sub.receive_publish_message();
    client_sub.disconnect();
    stopBroker(broker_thread);
    sleep(1);
    delete a;
}
void Integration_test::testPublishConcurrency()
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
    Hpfeeds_client client_sub;
    Hpfeeds_client client_pub;
    //SUBSCRIBER
    client_sub.connect();
    sleep(1);
    client_sub.receive_info_message();
    client_sub.send_auth_message("aldo", "s3cr3t");
    client_sub.send_subscribe_message("aldo", "ch1");

    //START PUBLISHERS IN 2 THREADS
    vector<string> pub_data_1;
    pub_data_1.insert(pub_data_1.end(), "PUBLISHER1");
    pub_data_1.insert(pub_data_1.end(), "pippo");
    pub_data_1.insert(pub_data_1.end(), "1234");
    pub_data_1.insert(pub_data_1.end(), "ch1");
    pub_data_1.insert(pub_data_1.end(), "I am Pippo");
    pthread_t pub_1;

    vector<string> pub_data_2;
    pub_data_2.insert(pub_data_2.end(), "PUBLISHER2");
    pub_data_2.insert(pub_data_2.end(), "filippo");
    pub_data_2.insert(pub_data_2.end(), "pwd");
    pub_data_2.insert(pub_data_2.end(), "ch1");
    pub_data_2.insert(pub_data_2.end(), "I am Filippo");
    pthread_t pub_2;

    int res1 = pthread_create(&pub_1, NULL, run_publisher, &pub_data_1);
    if (res1){
        cout<<"Error during "<<pub_data_1[0]<<" creation: "<<res1<<endl;
    }

    int res2 = pthread_create(&pub_2, NULL, run_publisher, &pub_data_2);
    if (res2){
        cout<<"Error during "<<pub_data_2[0]<<" creation: "<<res2<<endl;
    }

    //SUBSCRIBER RECEIVE
    pthread_join(pub_1, NULL);
    pthread_join(pub_2, NULL);
    sleep(1);
    client_sub.receive_publish_message();
    client_sub.receive_publish_message();
    client_sub.disconnect();
    stopBroker(broker_thread);
    sleep(1);
    delete a;

}
void Integration_test::testHelp()
{
    Arguments* a = new Arguments();
    a->insert(a->end(),"tentacool_integration_test");
    a->insert(a->end(),"-h");
    pthread_t broker_thread = startBroker(a);
    sleep(1); //Wait for Broker setup
    stopBroker(broker_thread);
    sleep(1);
    delete a;
}
void Integration_test::testWithMongo()
{
    Arguments* a = new Arguments();
    a->insert(a->end(),"tentacool_integration_test");
    a->insert(a->end(),"-m");
    a->insert(a->end(),"mongodb");
    a->insert(a->end(),"--mongoip");
    a->insert(a->end(),"127.0.0.1");
    a->insert(a->end(),"--mongoport");
    a->insert(a->end(),"27017");
    a->insert(a->end(),"--mongodb");
    a->insert(a->end(),"hpfeeds");
    a->insert(a->end(),"--mongocoll");
    a->insert(a->end(),"auth_key");
    a->insert(a->end(),"-v");
    a->insert(a->end(),"-d");
    pthread_t broker_thread = startBroker(a);
    sleep(2); //Wait for Broker setup
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
            new CppUnit::TestCaller<Integration_test>("testHelp",
                    &Integration_test::testHelp));
    suiteOfTests->addTest(
            new CppUnit::TestCaller<Integration_test>("testConnectDisconnect",
                    &Integration_test::testConnectDisconnect));
    suiteOfTests->addTest(
            new CppUnit::TestCaller<Integration_test>("testDifferentPortAndName",
                    &Integration_test::testDifferentPortAndName));
    suiteOfTests->addTest(
            new CppUnit::TestCaller<Integration_test>("testWithMongo",
                    &Integration_test::testWithMongo));
    suiteOfTests->addTest(
            new CppUnit::TestCaller<Integration_test>("testAuthentication",
                    &Integration_test::testAuthentication));
    suiteOfTests->addTest(
            new CppUnit::TestCaller<Integration_test>("testSubscribe",
                    &Integration_test::testSubscribe));
    suiteOfTests->addTest(
            new CppUnit::TestCaller<Integration_test>("testPublish",
                    &Integration_test::testPublish));
    suiteOfTests->addTest(
            new CppUnit::TestCaller<Integration_test>("testPublishConcurrency",
                    &Integration_test::testPublishConcurrency));
    return suiteOfTests;
}

