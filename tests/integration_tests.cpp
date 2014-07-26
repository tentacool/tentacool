#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "integration_tests.hpp"
#include "hpfeeds_client.hpp"
#include "../broker.hpp"
#include <Poco/Random.h>
#include <Poco/Path.h>
#include <Poco/NumberFormatter.h>
#include <pthread.h>
#include <signal.h>
#include <unistd.h>
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

void *run(void * args)
{
    Arguments* a = reinterpret_cast<Arguments*>(args);
#ifdef DEBUG
    cout <<endl<<"Starting broker thread..." << endl;
#else
    //Redirect the output to /dev/null
    ofstream redirect_output("/dev/null");

    // save output buffer of cout
    streambuf * strm_buffer = cout.rdbuf();

    // redirect output into file
    cout.rdbuf(redirect_output.rdbuf());
#endif

    try {
        BrokerApplication app;
        app.run(a->size(), reinterpret_cast<char**>(a->data()));
    } catch (Poco::Exception& exc) {
        cerr << exc.displayText() << endl;
        a = NULL;
        //Permit the while exiting despite of the error
        //_brokerStarted = true;
    }
#ifndef DEBUG
    // restore old buffer
    cout.rdbuf(strm_buffer);
#endif
    a = NULL;
    return NULL;
}

void *run_publisher(void * args)
{
    vector<string>* data = reinterpret_cast<vector<string>*>(args);
#ifdef DEBUG
    cout<<"Starting publisher: "<<data->at(0)<<endl;
#endif
    HpfeedsClient client_pub;
    try{
        client_pub.connect();
        sleep(BROKER_GENERIC_WAIT);
        client_pub.receiveInfo();
        client_pub.sendAuth(data->at(1), data->at(2));
        client_pub.sendPublish(data->at(1), data->at(3), data->at(4));
        sleep(BROKER_GENERIC_WAIT);
        client_pub.disconnect();
    } catch (Poco::Exception& exc) {
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
Integration_test::Integration_test()
{
    //Get the path of the executable
    char pBuf[PATH_MAX];
    int path_len = getPath(pBuf);
    if (path_len > 0) {
        _exe_path = string(pBuf, path_len);
    }
}

int Integration_test::getPath(char* pBuf)
{
    char szTmp[PATH_MAX];
    sprintf(szTmp, "/proc/self/exe");
    int bytes = readlink(szTmp, pBuf, PATH_MAX);
    if (bytes >= 0) {
        pBuf[bytes] = '\0';
        string line(pBuf, bytes);
        line = line.substr(0, line.find_last_of("\\/"));
        line += '/';
        sprintf(pBuf, "%s", line.c_str());
        return line.length();
    };
    return -1;
}

void Integration_test::testHelp()
{
    Arguments* a = new Arguments();
    a->insert(a->end(),"tentacool_integration_test");
    a->insert(a->end(),"-h");
    pthread_t broker_thread = startBroker(a);
    sleep(BROKER_SETUP_WAIT);
    stopBroker(broker_thread);
    sleep(BROKER_DELETE_WAIT);
    delete a;
}

void Integration_test::testVersion()
{
    Arguments* a = new Arguments();
    a->insert(a->end(),"tentacool_integration_test");
    a->insert(a->end(),"--version");
    pthread_t broker_thread = startBroker(a);
    sleep(BROKER_SETUP_WAIT);
    stopBroker(broker_thread);
    sleep(BROKER_DELETE_WAIT);
    delete a;
}

void Integration_test::testServerParams()
{
    Arguments* a = new Arguments();
    a->insert(a->end(),"tentacool_integration_test");
#ifdef DEBUG
    a->insert(a->end(),"-d");
    a->insert(a->end(),"-v");
#endif
    a->insert(a->end(),"-f"+_exe_path + "data/auth_keys.dat");
    a->insert(a->end(),"--max_thread=12");
    a->insert(a->end(),"--idletime=50");
    pthread_t broker_thread = startBroker(a);
    sleep(BROKER_SETUP_WAIT);
    stopBroker(broker_thread);
    sleep(BROKER_DELETE_WAIT);
    delete a;
}

void Integration_test::testDebugSettings()
{
    //Also test the logging on file
    Arguments* a = new Arguments();
    a->insert(a->end(),"tentacool_integration_test");
    a->insert(a->end(),"-d");
    pthread_t broker_thread = startBroker(a);
    sleep(BROKER_SETUP_WAIT);
    stopBroker(broker_thread);
    sleep(BROKER_DELETE_WAIT);
    delete a;
}

void Integration_test::testWrongFile()
{
    Arguments* a = new Arguments();
    a->insert(a->end(),"tentacool_integration_test");
#ifdef DEBUG
    a->insert(a->end(),"-d");
    a->insert(a->end(),"-v");
#endif
#ifdef __WITH_MONGO__
    a->insert(a->end(),"-m");
    a->insert(a->end(),"file");
#endif
    a->insert(a->end(),"-f"+_exe_path + "data/wrong_file.dat");
    pthread_t broker_thread = startBroker(a);
    sleep(BROKER_SETUP_WAIT);
    stopBroker(broker_thread);
    sleep(BROKER_DELETE_WAIT);
    delete a;
}

void Integration_test::testConnectDisconnect()
{
    Arguments* a;
    pthread_t broker_thread;
    a = new Arguments();
    a->insert(a->end(),"tentacool_integration_test");
#ifdef DEBUG
    a->insert(a->end(),"-d");
    a->insert(a->end(),"-v");
#endif
#ifdef __WITH_MONGO__
    a->insert(a->end(),"-m");
    a->insert(a->end(),"file");
#endif
    a->insert(a->end(),"-f"+_exe_path + "data/auth_keys.dat");
    broker_thread = startBroker(a);
    sleep(BROKER_SETUP_WAIT);
    HpfeedsClient client;
    client.connect();
    sleep(BROKER_GENERIC_WAIT);
    client.disconnect();
    sleep(BROKER_OPS_COMPLETION);
    stopBroker(broker_thread);
    sleep(BROKER_DELETE_WAIT);
    delete a;
}

void Integration_test::testDifferentPortAndName()
{
    Arguments* a = new Arguments();
    a->insert(a->end(),"tentacool_integration_test");
#ifdef DEBUG
    a->insert(a->end(),"-d");
    a->insert(a->end(),"-v");
#endif
#ifdef __WITH_MONGO__
    a->insert(a->end(),"-m");
    a->insert(a->end(),"file");
#endif
    a->insert(a->end(),"-f"+_exe_path + "data/auth_keys.dat");
    a->insert(a->end(),"-p10127");
    a->insert(a->end(),"-nBroker2");
    pthread_t broker_thread = startBroker(a);
    sleep(BROKER_SETUP_WAIT);
    stopBroker(broker_thread);
    sleep(BROKER_DELETE_WAIT);
    delete a;
}

void Integration_test::testAuthentication()
{
    Arguments* a = new Arguments();
    a->insert(a->end(),"tentacool_integration_test");
#ifdef DEBUG
    a->insert(a->end(),"-d");
    a->insert(a->end(),"-v");
#endif
#ifdef __WITH_MONGO__
    a->insert(a->end(),"-m");
    a->insert(a->end(),"file");
#endif
    a->insert(a->end(),"-f"+_exe_path + "data/auth_keys.dat");
    pthread_t broker_thread = startBroker(a);
    sleep(BROKER_SETUP_WAIT);
    HpfeedsClient client;
    client.connect();
    sleep(BROKER_GENERIC_WAIT);
    client.receiveInfo();
    client.sendAuth("aldo", "s3cr3t");
    sleep(BROKER_OPS_COMPLETION);
    client.disconnect();
    stopBroker(broker_thread);
    sleep(BROKER_DELETE_WAIT);
    delete a;
}

void Integration_test::testFailAuthentication()
{
    Arguments* a = new Arguments();
    a->insert(a->end(),"tentacool_integration_test");
#ifdef DEBUG
    a->insert(a->end(),"-d");
    a->insert(a->end(),"-v");
#endif
#ifdef __WITH_MONGO__
    a->insert(a->end(),"-m");
    a->insert(a->end(),"file");
#endif
    a->insert(a->end(),"-f"+_exe_path + "data/auth_keys.dat");
    pthread_t broker_thread = startBroker(a);
    sleep(BROKER_SETUP_WAIT);
    HpfeedsClient client;
    client.connect();
    sleep(BROKER_GENERIC_WAIT);
    client.receiveInfo();
    client.sendAuth("aldo", "wrong_secret");
    sleep(BROKER_GENERIC_WAIT);
    client.receiveError();
    client.disconnect();
    stopBroker(broker_thread);
    sleep(BROKER_DELETE_WAIT);
    delete a;
}

void Integration_test::testWrongAuthenticationMsg()
{
    Arguments* a = new Arguments();
    a->insert(a->end(),"tentacool_integration_test");
#ifdef DEBUG
    a->insert(a->end(),"-d");
    a->insert(a->end(),"-v");
#endif
#ifdef __WITH_MONGO__
    a->insert(a->end(),"-m");
    a->insert(a->end(),"file");
#endif
 a->insert(a->end(),"-f"+_exe_path + "data/auth_keys.dat");
    pthread_t broker_thread = startBroker(a);
    sleep(BROKER_SETUP_WAIT);
    HpfeedsClient client;
    client.connect();
    sleep(BROKER_GENERIC_WAIT);
    client.receiveInfo();
    client.sendSubscribe("aldo", "ch1"); //Instead of an auth msg
    sleep(BROKER_GENERIC_WAIT);
    client.receiveError();
    client.disconnect();
    stopBroker(broker_thread);
    sleep(BROKER_DELETE_WAIT);
    delete a;
}

void Integration_test::testTooLongAuthenticationMsg()
{
    Arguments* a = new Arguments();
    a->insert(a->end(),"tentacool_integration_test");
#ifdef DEBUG
    a->insert(a->end(),"-d");
    a->insert(a->end(),"-v");
#endif
#ifdef __WITH_MONGO__
    a->insert(a->end(),"-m");
    a->insert(a->end(),"file");
#endif
    a->insert(a->end(),"-f"+_exe_path + "data/auth_keys.dat");
    pthread_t broker_thread = startBroker(a);
    sleep(BROKER_SETUP_WAIT);
    HpfeedsClient client;
    client.connect();
    sleep(BROKER_GENERIC_WAIT);
    client.receiveInfo();
    client.sendWrong(300,OP_AUTH, "data");
    sleep(BROKER_GENERIC_WAIT);
    client.receiveError();
    client.disconnect();
    stopBroker(broker_thread);
    sleep(BROKER_DELETE_WAIT);
    delete a;
}

void Integration_test::testTooBigMessage()
{
    Arguments* a = new Arguments();
    a->insert(a->end(),"tentacool_integration_test");
#ifdef DEBUG
    a->insert(a->end(),"-d");
    a->insert(a->end(),"-v");
#endif
#ifdef __WITH_MONGO__
    a->insert(a->end(),"-m");
    a->insert(a->end(),"file");
#endif
 a->insert(a->end(),"-f"+_exe_path + "data/auth_keys.dat");
    pthread_t broker_thread = startBroker(a);
    sleep(BROKER_SETUP_WAIT);
    HpfeedsClient client;
    client.connect();
    sleep(BROKER_GENERIC_WAIT);
    client.receiveInfo();
    client.sendAuth("aldo", "s3cr3t");
    client.sendWrong(10*1024*1024 + 5 + 1, OP_SUBSCRIBE,
                                "This string is smaller, but doesn't matter.");
    sleep(BROKER_GENERIC_WAIT);
    client.disconnect();
    stopBroker(broker_thread);
    sleep(BROKER_DELETE_WAIT);
    delete a;
}

void Integration_test::testSubscribe()
{
    Arguments* a = new Arguments();
    a->insert(a->end(),"tentacool_integration_test");
#ifdef DEBUG
    a->insert(a->end(),"-d");
    a->insert(a->end(),"-v");
#endif
#ifdef __WITH_MONGO__
    a->insert(a->end(),"-m");
    a->insert(a->end(),"file");
#endif
 a->insert(a->end(),"-f"+_exe_path + "data/auth_keys.dat");
    pthread_t broker_thread = startBroker(a);
    sleep(BROKER_SETUP_WAIT);
    HpfeedsClient client;
    client.connect();
    sleep(BROKER_GENERIC_WAIT);
    client.receiveInfo();
    client.sendAuth("aldo", "s3cr3t");
    client.sendSubscribe("aldo", "ch1");
    sleep(BROKER_OPS_COMPLETION);
    client.disconnect();
    stopBroker(broker_thread);
    sleep(BROKER_DELETE_WAIT);
    delete a;
}

void Integration_test::testDoubleSubscribe()
{
    Arguments* a = new Arguments();
    a->insert(a->end(),"tentacool_integration_test");
#ifdef DEBUG
    a->insert(a->end(),"-d");
    a->insert(a->end(),"-v");
#endif
#ifdef __WITH_MONGO__
    a->insert(a->end(),"-m");
    a->insert(a->end(),"file");
#endif
 a->insert(a->end(),"-f"+_exe_path + "data/auth_keys.dat");
    pthread_t broker_thread = startBroker(a);
    sleep(BROKER_SETUP_WAIT);
    HpfeedsClient client;
    client.connect();
    sleep(BROKER_GENERIC_WAIT);
    client.receiveInfo();
    client.sendAuth("aldo", "s3cr3t");
    client.sendSubscribe("aldo", "ch1");
    client.sendSubscribe("aldo", "ch1");
    sleep(BROKER_OPS_COMPLETION);
    client.disconnect();
    stopBroker(broker_thread);
    sleep(BROKER_DELETE_WAIT);
    delete a;
}

void Integration_test::testSubscribeNotAllowed()
{
    Arguments* a = new Arguments();
    a->insert(a->end(),"tentacool_integration_test");
#ifdef DEBUG
    a->insert(a->end(),"-d");
    a->insert(a->end(),"-v");
#endif
#ifdef __WITH_MONGO__
    a->insert(a->end(),"-m");
    a->insert(a->end(),"file");
#endif
 a->insert(a->end(),"-f"+_exe_path + "data/auth_keys.dat");
    pthread_t broker_thread = startBroker(a);
    sleep(BROKER_SETUP_WAIT);
    HpfeedsClient client;
    client.connect();
    sleep(BROKER_GENERIC_WAIT);
    client.receiveInfo();
    client.sendAuth("aldo", "s3cr3t");
    client.sendSubscribe("aldo", "ch3");
    sleep(BROKER_GENERIC_WAIT);
    client.receiveError();
    client.disconnect();
    stopBroker(broker_thread);
    sleep(BROKER_DELETE_WAIT);
    delete a;
}

void Integration_test::testPublish()
{
    Arguments* a = new Arguments();
    a->insert(a->end(),"tentacool_integration_test");
#ifdef DEBUG
    a->insert(a->end(),"-d");
    a->insert(a->end(),"-v");
#endif
#ifdef __WITH_MONGO__
    a->insert(a->end(),"-m");
    a->insert(a->end(),"file");
#endif
 a->insert(a->end(),"-f"+_exe_path + "data/auth_keys.dat");
    pthread_t broker_thread = startBroker(a);
    sleep(BROKER_SETUP_WAIT);
    HpfeedsClient client_sub;
    HpfeedsClient client_pub;
    //SUBSCRIBER
    client_sub.connect();
    sleep(BROKER_GENERIC_WAIT);
    client_sub.receiveInfo();
    client_sub.sendAuth("aldo", "s3cr3t");
    client_sub.sendSubscribe("aldo", "ch1");
    //PUBLISHER
    client_pub.connect();
    sleep(BROKER_GENERIC_WAIT);
    client_pub.receiveInfo();
    client_pub.sendAuth("filippo", "pwd");
    client_pub.sendPublish("filippo", "ch1", "testing");
    client_pub.disconnect();
    //SUBSCRIBER RECEIVE
    sleep(BROKER_GENERIC_WAIT);
    client_sub.receivePublish();
    client_sub.disconnect();
    stopBroker(broker_thread);
    sleep(BROKER_DELETE_WAIT);
    delete a;
}

void Integration_test::testPublishNoChannel()
{
    Arguments* a = new Arguments();
    a->insert(a->end(),"tentacool_integration_test");
#ifdef DEBUG
    a->insert(a->end(),"-d");
    a->insert(a->end(),"-v");
#endif
#ifdef __WITH_MONGO__
    a->insert(a->end(),"-m");
    a->insert(a->end(),"file");
#endif
 a->insert(a->end(),"-f"+_exe_path + "data/auth_keys.dat");
    pthread_t broker_thread = startBroker(a);
    sleep(BROKER_SETUP_WAIT);
    HpfeedsClient client_sub;
    HpfeedsClient client_pub;
    //SUBSCRIBER
    client_sub.connect();
    sleep(BROKER_GENERIC_WAIT);
    client_sub.receiveInfo();
    client_sub.sendAuth("aldo", "s3cr3t");
    client_sub.sendSubscribe("aldo", "ch1");
    //PUBLISHER
    client_pub.connect();
    sleep(BROKER_GENERIC_WAIT);
    client_pub.receiveInfo();
    client_pub.sendAuth("filippo", "pwd");
    client_pub.sendPublish("filippo", "ch2", "testing");
    client_pub.disconnect();
    //SUBSCRIBER RECEIVE
    sleep(BROKER_GENERIC_WAIT);
    client_sub.receivePublish();
    client_sub.disconnect();
    stopBroker(broker_thread);
    sleep(BROKER_DELETE_WAIT);
    delete a;
}

void Integration_test::testPublishNotAllowed()
{
    Arguments* a = new Arguments();
    a->insert(a->end(),"tentacool_integration_test");
#ifdef DEBUG
    a->insert(a->end(),"-d");
    a->insert(a->end(),"-v");
#endif
#ifdef __WITH_MONGO__
    a->insert(a->end(),"-m");
    a->insert(a->end(),"file");
#endif
 a->insert(a->end(),"-f"+_exe_path + "data/auth_keys.dat");
    pthread_t broker_thread = startBroker(a);
    sleep(BROKER_SETUP_WAIT);
    HpfeedsClient client_sub;
    HpfeedsClient client_pub;
    //SUBSCRIBER
    client_sub.connect();
    sleep(BROKER_GENERIC_WAIT);
    client_sub.receiveInfo();
    client_sub.sendAuth("aldo", "s3cr3t");
    client_sub.sendSubscribe("aldo", "ch1");
    //PUBLISHER
    client_pub.connect();
    sleep(BROKER_GENERIC_WAIT);
    client_pub.receiveInfo();
    client_pub.sendAuth("filippo", "pwd");
    client_pub.sendPublish("filippo", "ch77", "testing");
    //SUBSCRIBER RECEIVE
    sleep(BROKER_GENERIC_WAIT);
    client_pub.receiveError();
    client_pub.disconnect();
    client_sub.disconnect();
    stopBroker(broker_thread);
    sleep(BROKER_DELETE_WAIT);
    delete a;
}

void Integration_test::testWrongOpCode()
{
    Arguments* a = new Arguments();
    a->insert(a->end(),"tentacool_integration_test");
#ifdef DEBUG
    a->insert(a->end(),"-d");
    a->insert(a->end(),"-v");
#endif
#ifdef __WITH_MONGO__
    a->insert(a->end(),"-m");
    a->insert(a->end(),"file");
#endif
 a->insert(a->end(),"-f"+_exe_path + "data/auth_keys.dat");
    pthread_t broker_thread = startBroker(a);
    sleep(BROKER_SETUP_WAIT);
    HpfeedsClient client_sub;
    //SUBSCRIBER
    client_sub.connect();
    sleep(BROKER_GENERIC_WAIT);
    client_sub.receiveInfo();
    client_sub.sendAuth("aldo", "s3cr3t");
    client_sub.sendWrong(13,45,"wrong message");
    //SUBSCRIBER RECEIVE
    sleep(BROKER_GENERIC_WAIT);
    client_sub.receiveError();
    client_sub.disconnect();
    stopBroker(broker_thread);
    sleep(BROKER_DELETE_WAIT);
    delete a;
}

void Integration_test::testWrongTotalLength()
{
    Arguments* a = new Arguments();
    a->insert(a->end(),"tentacool_integration_test");
#ifdef DEBUG
    a->insert(a->end(),"-d");
    a->insert(a->end(),"-v");
#endif
#ifdef __WITH_MONGO__
    a->insert(a->end(),"-m");
    a->insert(a->end(),"file");
#endif
 a->insert(a->end(),"-f"+_exe_path + "data/auth_keys.dat");
    pthread_t broker_thread = startBroker(a);
    sleep(BROKER_SETUP_WAIT);
    HpfeedsClient client_sub;
    //SUBSCRIBER
    client_sub.connect();
    sleep(BROKER_GENERIC_WAIT);
    client_sub.receiveInfo();
    client_sub.sendAuth("aldo", "s3cr3t");
    client_sub.sendWrong(40,OP_PUBLISH,"wrong message");
    //SUBSCRIBER RECEIVE
    sleep(BROKER_GENERIC_WAIT);
    client_sub.receiveError();
    client_sub.disconnect();
    stopBroker(broker_thread);
    sleep(BROKER_DELETE_WAIT);
    delete a;
}

void Integration_test::testPublishBigMessage()
{
    Arguments* a = new Arguments();
    a->insert(a->end(),"tentacool_integration_test");
#ifdef DEBUG
    a->insert(a->end(),"-d");
    a->insert(a->end(),"-v");
#endif
#ifdef __WITH_MONGO__
    a->insert(a->end(),"-m");
    a->insert(a->end(),"file");
#endif
 a->insert(a->end(),"-f"+_exe_path + "data/auth_keys.dat");
    pthread_t broker_thread = startBroker(a);
    sleep(BROKER_SETUP_WAIT);
    HpfeedsClient client_sub;
    HpfeedsClient client_pub;
    //CREATING BIG DATA
    vector<u_char> datas;
    Random _r;
    for(int i = 0; i < (3*1024); i++) {
        datas.insert(datas.end(), _r.nextChar());
    }
    string data(datas.begin(), datas.end());
    //SUBSCRIBER
    client_sub.connect();
    sleep(BROKER_GENERIC_WAIT);
    client_sub.receiveInfo();
    client_sub.sendAuth("aldo", "s3cr3t");
    client_sub.sendSubscribe("aldo", "ch1");
    //PUBLISHER
    client_pub.connect();
    sleep(BROKER_GENERIC_WAIT);
    client_pub.receiveInfo();
    client_pub.sendAuth("filippo", "pwd");
    client_pub.sendPublish("filippo", "ch1", data);
    client_pub.disconnect();
    //SUBSCRIBER RECEIVE
    sleep(BROKER_GENERIC_WAIT);
    client_sub.receivePublish();
    client_sub.disconnect();
    stopBroker(broker_thread);
    sleep(BROKER_DELETE_WAIT);
    delete a;
}

void Integration_test::testPublishConcurrency()
{
    Arguments* a = new Arguments();
    a->insert(a->end(),"tentacool_integration_test");
#ifdef DEBUG
    a->insert(a->end(),"-d");
    a->insert(a->end(),"-v");
#endif
#ifdef __WITH_MONGO__
    a->insert(a->end(),"-m");
    a->insert(a->end(),"file");
#endif
    a->insert(a->end(),"-f"+_exe_path + "data/auth_keys.dat");
    pthread_t broker_thread = startBroker(a);
    sleep(BROKER_SETUP_WAIT);
    HpfeedsClient client_sub;
    HpfeedsClient client_pub;
    //SUBSCRIBER
    client_sub.connect();
    sleep(BROKER_GENERIC_WAIT);
    client_sub.receiveInfo();
    client_sub.sendAuth("aldo", "s3cr3t");
    client_sub.sendSubscribe("aldo", "ch1");

    //START PUBLISHERS IN 3 THREADS
    vector<string> pub_data_1;
    pub_data_1.insert(pub_data_1.end(), "PUBLISHER1");
    pub_data_1.insert(pub_data_1.end(), "maria");
    pub_data_1.insert(pub_data_1.end(), "4321");
    pub_data_1.insert(pub_data_1.end(), "ch1");
    pub_data_1.insert(pub_data_1.end(), "I am Maria");
    pthread_t pub_1;

    vector<string> pub_data_2;
    pub_data_2.insert(pub_data_2.end(), "PUBLISHER2");
    pub_data_2.insert(pub_data_2.end(), "pippo");
    pub_data_2.insert(pub_data_2.end(), "1234");
    pub_data_2.insert(pub_data_2.end(), "ch1");
    pub_data_2.insert(pub_data_2.end(), "I am Pippo");
    pthread_t pub_2;

    vector<string> pub_data_3;
    pub_data_3.insert(pub_data_3.end(), "PUBLISHER3");
    pub_data_3.insert(pub_data_3.end(), "filippo");
    pub_data_3.insert(pub_data_3.end(), "pwd");
    pub_data_3.insert(pub_data_3.end(), "ch1");
    pub_data_3.insert(pub_data_3.end(), "I am Filippo");
    pthread_t pub_3;

    int res1 = pthread_create(&pub_1, NULL, run_publisher, &pub_data_1);
    if (res1) {
        cout<<"Error during "<<pub_data_1[0]<<" creation: "<<res1<<endl;
    }

    int res2 = pthread_create(&pub_2, NULL, run_publisher, &pub_data_2);
    if (res2) {
        cout<<"Error during "<<pub_data_2[0]<<" creation: "<<res2<<endl;
    }

    int res3 = pthread_create(&pub_3, NULL, run_publisher, &pub_data_3);
    if (res3) {
        cout<<"Error during "<<pub_data_3[0]<<" creation: "<<res3<<endl;
    }
    //SUBSCRIBER RECEIVE
    pthread_join(pub_1, NULL);
    pthread_join(pub_2, NULL);
    pthread_join(pub_3, NULL);
    sleep(BROKER_GENERIC_WAIT);
    client_sub.receivePublish();
    client_sub.receivePublish();
    client_sub.receivePublish();
    client_sub.disconnect();
    stopBroker(broker_thread);
    sleep(BROKER_DELETE_WAIT);
    delete a;

}

void Integration_test::testWithMongo()
{
    Arguments* a = new Arguments();
    a->insert(a->end(),"tentacool_integration_test");
#ifdef DEBUG
    a->insert(a->end(),"-d");
    a->insert(a->end(),"-v");
#endif
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
    pthread_t broker_thread = startBroker(a);
    sleep(BROKER_SETUP_WAIT);
    stopBroker(broker_thread);
    sleep(BROKER_DELETE_WAIT);
    delete a;
}

pthread_t Integration_test::startBroker(Arguments* args)
{
    pthread_attr_t attr;
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
    pthread_t t1;
    int res = pthread_create(&t1, &attr, run, (void*)args);
    if(res) {
        pthread_attr_destroy(&attr);
        cout<<"Error during broker_thread creation: "<<res<<endl;
    }
    pthread_attr_destroy(&attr);
    return t1;
}

void Integration_test::stopBroker(pthread_t t)
{
#ifdef DEBUG
    cout<<"Killing broker_thread..."<<endl;
#endif
    pthread_kill(t, SIGINT);
}

Test *Integration_test::suite()
{
    TestSuite *suiteOfTests = new CppUnit::TestSuite( "Integration_test" );

    suiteOfTests->addTest(
            new CppUnit::TestCaller<Integration_test>("testHelp",
                    &Integration_test::testHelp));
    suiteOfTests->addTest(
            new CppUnit::TestCaller<Integration_test>("testVersion",
                    &Integration_test::testVersion));
    suiteOfTests->addTest(
            new CppUnit::TestCaller<Integration_test>("testServerParams",
                    &Integration_test::testServerParams));
    suiteOfTests->addTest(
            new CppUnit::TestCaller<Integration_test>("testDebugSettings",
                    &Integration_test::testDebugSettings));
    suiteOfTests->addTest(
            new CppUnit::TestCaller<Integration_test>("testWrongFile",
                    &Integration_test::testWrongFile));
    suiteOfTests->addTest(
            new CppUnit::TestCaller<Integration_test>("testConnectDisconnect",
                    &Integration_test::testConnectDisconnect));
    suiteOfTests->addTest(
            new CppUnit::TestCaller<Integration_test>
        ("testDifferentPortAndName",
                    &Integration_test::testDifferentPortAndName));

#ifdef __WITH_MONGO__
    suiteOfTests->addTest(
            new CppUnit::TestCaller<Integration_test>("testWithMongo",
                    &Integration_test::testWithMongo));
#endif

    suiteOfTests->addTest(
            new CppUnit::TestCaller<Integration_test>("testAuthentication",
                    &Integration_test::testAuthentication));
    suiteOfTests->addTest(
            new CppUnit::TestCaller<Integration_test>("testFailAuthentication",
                    &Integration_test::testFailAuthentication));
    suiteOfTests->addTest(
                new CppUnit::TestCaller<Integration_test>
        ("testWrongAuthenticationMsg",
                    &Integration_test::testWrongAuthenticationMsg));
    suiteOfTests->addTest(
                    new CppUnit::TestCaller<Integration_test>
        ("testTooLongAuthenticationMsg",
                            &Integration_test::testTooLongAuthenticationMsg));

    suiteOfTests->addTest(
                    new CppUnit::TestCaller<Integration_test>
        ("testTooBigMessage",
                            &Integration_test::testTooBigMessage));
    suiteOfTests->addTest(
            new CppUnit::TestCaller<Integration_test>("testSubscribe",
                    &Integration_test::testSubscribe));
    suiteOfTests->addTest(
            new CppUnit::TestCaller<Integration_test>("testDoubleSubscribe",
                    &Integration_test::testDoubleSubscribe));
    suiteOfTests->addTest(
            new CppUnit::TestCaller<Integration_test>
        ("testSubscribeNotAllowed",
                    &Integration_test::testSubscribeNotAllowed));
    suiteOfTests->addTest(
            new CppUnit::TestCaller<Integration_test>("testWrongOpCode",
                    &Integration_test::testWrongOpCode));
    suiteOfTests->addTest(
            new CppUnit::TestCaller<Integration_test>("testWrongTotalLength",
                    &Integration_test::testWrongTotalLength));
    suiteOfTests->addTest(
            new CppUnit::TestCaller<Integration_test>("testPublish",
                    &Integration_test::testPublish));
    suiteOfTests->addTest(
            new CppUnit::TestCaller<Integration_test>("testPublishNoChannel",
                    &Integration_test::testPublishNoChannel));
    suiteOfTests->addTest(
            new CppUnit::TestCaller<Integration_test>("testPublishNotAllowed",
                    &Integration_test::testPublishNotAllowed));
    suiteOfTests->addTest(
            new CppUnit::TestCaller<Integration_test>("testPublishBigMessage",
                    &Integration_test::testPublishBigMessage));
    suiteOfTests->addTest(
            new CppUnit::TestCaller<Integration_test>("testPublishConcurrency",
                    &Integration_test::testPublishConcurrency));
    return suiteOfTests;
}
