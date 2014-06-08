#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "data_manager_tests.hpp"
#include "data_manager.hpp"
#include <memory>
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
#ifdef __WITH_MONGO__
#include <mongo/client/dbclient.h>
#endif

using namespace CppUnit;
using namespace std;

DataManager_test::DataManager_test(): dm_mongodb_work(NULL)
{
    string errmsg;
    try {
#ifdef __WITH_MONGO__
        if(!_conn.connect("localhost",errmsg))
                throw Poco::Exception("Mongodb connection fail. "
                        "Check if mongodb is running.");
        unique_ptr<mongo::DBClientCursor> cursor =
                (move(_conn.query("hpfeeds.auth_key")));
        if((*cursor).itcount() == 0){
            cout<<"hpfeeds.auth_key collection does not exists and will be"
                    "created"<<endl;
            mongo::BSONObjBuilder b1,b2,b3,b4;
            //aldo
            vector<string> publish1;
            publish1.insert(publish1.end(),"ch1");
            vector<string> subscribe1;
            subscribe1.insert(subscribe1.end(),"ch1");
            subscribe1.insert(subscribe1.end(),"ch2");
            b1.append("secret","s3cr3t");
            b1.append("identifier","aldo");
            b1.append("publish", publish1);
            b1.append("subscribe", subscribe1);
            mongo::BSONObj u1 = b1.obj();
            _conn.insert("hpfeeds.auth_key",u1);
            //filippo
            vector<string> publish2;
            publish2.insert(publish2.end(),"ch1");
            publish2.insert(publish2.end(),"ch2");
            vector<string> subscribe2;
            subscribe2.insert(subscribe2.end(),"ch1");
            b2.append("secret","pwd");
            b2.append("identifier","filippo");
            b2.append("publish", publish2);
            b2.append("subscribe", subscribe2);
            mongo::BSONObj u2 = b2.obj();
            _conn.insert("hpfeeds.auth_key",u2);
            //pippo
            vector<string> publish3;
            publish3.insert(publish3.end(),"ch1");
            publish3.insert(publish3.end(),"ch2");
            vector<string> subscribe3;
            subscribe3.insert(subscribe3.end(),"ch1");
            b3.append("secret","1234");
            b3.append("identifier","pippo");
            b3.append("publish", publish3);
            b3.append("subscribe", subscribe3);
            mongo::BSONObj u3 = b3.obj();
            _conn.insert("hpfeeds.auth_key",u3);
            //maria
            vector<string> publish4;
            publish4.insert(publish4.end(),"ch1");
            publish4.insert(publish4.end(),"ch2");
            vector<string> subscribe4;
            subscribe4.insert(subscribe4.end(),"ch1");
            b4.append("secret","4321");
            b4.append("identifier","maria");
            b4.append("publish", publish4);
            b4.append("subscribe", subscribe4);
            mongo::BSONObj u4 = b4.obj();
            _conn.insert("hpfeeds.auth_key",u4);
        }
#endif
    } catch(Poco::Exception& e) {
        cout<<e.displayText()<<endl;
        exit(-1);
    }
}

DataManager_test::~DataManager_test(){
#ifdef __WITH_MONGO__
    _conn.dropDatabase("hpfeeds");
#endif
}

void DataManager_test::setUp()
{
    try {
        filename = "data/auth_keys.dat";
        malformed_file = "data/malformed_file.dat";
        not_existing_file = "data/iamnotexists.boh";
        too_long_file = "data/toolong_line.dat";
        black_rows_file = "data/blank_rows.dat";
#ifdef __WITH_MONGO__
        dm_mongodb_work = new DataManager("localhost", "27017", "hpfeeds",
                "auth_key");
#endif
    } catch (Poco::Exception& e) {
        cout << e.displayText() << endl;
    }
}

void DataManager_test::tearDown()
{
#ifdef __WITH_MONGO__
    delete dm_mongodb_work;
#endif
}

void DataManager_test::testFileMode()
{
    CPPUNIT_ASSERT_NO_THROW(DataManager dm_file(filename););
}

void DataManager_test::testFile_malformedFile()
{
    CPPUNIT_ASSERT_THROW(DataManager dm_file(malformed_file);,Poco::Exception);
}

void DataManager_test::testFile_tooLongFile()
{
    CPPUNIT_ASSERT_THROW(DataManager dm_file(too_long_file);,Poco::Exception);
}

void DataManager_test::testFile_blankRows()
{
    CPPUNIT_ASSERT_NO_THROW(DataManager dm_file(black_rows_file););
}

void DataManager_test::testFile_getSecret()
{
    DataManager dm_file(filename);
    CPPUNIT_ASSERT(!dm_file.getSecretbyName("aldo").compare("s3cr3t"));
}

void DataManager_test::testNotExistingFile()
{
    CPPUNIT_ASSERT_THROW(DataManager data_manager(not_existing_file),
        Poco::Exception);
}
#ifdef __WITH_MONGO__
void DataManager_test::testMongoDB()
{
    CPPUNIT_ASSERT_NO_THROW(
        DataManager dm_mongodb("localhost","27017","hpfeeds","auth_key"));
}

void DataManager_test::testMongoDB_wrongIP()
{
    //Additional info are printed on stdout internally by mongo
    CPPUNIT_ASSERT_THROW(
        DataManager dm_mongodb_fail("wrong_ip","27017","hpfeeds","auth_key"),
        Poco::Exception);
}
void DataManager_test::testMongoDB_wrongCollection()
{
    CPPUNIT_ASSERT_THROW(
        DataManager dm_mongodb_fail("localhost","27017",
                "hpfeeds","wrong_collection"), Poco::Exception);
}

void DataManager_test::testMongoDB_wrongDB()
{
    CPPUNIT_ASSERT_THROW(
        DataManager dm_mongodb_fail("localhost","27017",
                "wrongDB","auth_key"), Poco::Exception);
}

void DataManager_test::testMongoDBSubscribe_present()
{
    DataManager dm_mongodb_work("localhost", "27017", "hpfeeds", "auth_key");
    CPPUNIT_ASSERT(dm_mongodb_work.may_subscribe("aldo", "ch1"));
}

void DataManager_test::testMongoDBPublish_present()
{
    DataManager dm_mongodb_work("localhost", "27017", "hpfeeds", "auth_key");
    CPPUNIT_ASSERT(dm_mongodb_work.may_publish("aldo", "ch1"));
}

void DataManager_test::testMongoDBSubscribe_user_not_present()
{
    DataManager dm_mongodb_work("localhost", "27017", "hpfeeds", "auth_key");
    CPPUNIT_ASSERT_THROW(dm_mongodb_work.may_subscribe("mark", "ch1"),
        Poco::Exception);
}

void DataManager_test::testMongoDBPublish_user_not_present()
{
    CPPUNIT_ASSERT_THROW(dm_mongodb_work->may_publish("joy", "ch1"),
        Poco::Exception);
}

void DataManager_test::testMongoDBSubscribe_channel_not_present()
{
    CPPUNIT_ASSERT(
        dm_mongodb_work->may_subscribe("aldo", "missingchannel") == false);
}

void DataManager_test::testMongoDBPublish_channel_not_present()
{
    CPPUNIT_ASSERT(dm_mongodb_work->may_publish("aldo", "channel?") == false);
}

void DataManager_test::testMongoDBSubscribe_name_null()
{
    CPPUNIT_ASSERT_THROW(dm_mongodb_work->may_subscribe(nullstring, "channel?"),
        Poco::Exception);
}
void DataManager_test::testMongoDBPublish_name_null()
{
    CPPUNIT_ASSERT_THROW(dm_mongodb_work->may_publish(nullstring, "channel?"),
        Poco::Exception);
}
void DataManager_test::testMongoDBSubscribe_channel_null()
{
    CPPUNIT_ASSERT_THROW(dm_mongodb_work->may_subscribe("aldo", nullstring),
        Poco::Exception);
}
void DataManager_test::testMongoDBPublish_channel_null()
{
    CPPUNIT_ASSERT_THROW(dm_mongodb_work->may_publish("aldo", nullstring),
        Poco::Exception);
}
void DataManager_test::testMongoDB_GetSecretByNULLname()
{
    CPPUNIT_ASSERT_THROW(dm_mongodb_work->getSecretbyName(nullstring),
        Poco::Exception);
}
void DataManager_test::testMongoDB_GetSecretByNOTEXISTINGname()
{
    CPPUNIT_ASSERT_THROW(dm_mongodb_work->getSecretbyName("inotexist"),
        Poco::Exception);
}
#else
void DataManager_test::testFileSubscribe_present()
{
    DataManager dm_file(filename);
    CPPUNIT_ASSERT(dm_file.may_subscribe("aldo", "ch1"));
}

void DataManager_test::testFilePublish_present()
{
    DataManager dm_file(filename);
    CPPUNIT_ASSERT(dm_file.may_publish("aldo", "ch1"));
}

void DataManager_test::testFileSubscribe_user_not_present()
{
    DataManager dm_file(filename);
    CPPUNIT_ASSERT_THROW(dm_file.may_subscribe("mark", "ch1"),
        Poco::Exception);
}

void DataManager_test::testFilePublish_user_not_present()
{
    DataManager dm_file(filename);
    CPPUNIT_ASSERT_THROW(dm_file.may_publish("joy", "ch1"),
        Poco::Exception);
}

void DataManager_test::testFileSubscribe_channel_not_present()
{
    DataManager dm_file(filename);
    CPPUNIT_ASSERT(dm_file.may_subscribe("aldo", "missingchannel") == false);
}

void DataManager_test::testFilePublish_channel_not_present()
{
    DataManager dm_file(filename);
    CPPUNIT_ASSERT(dm_file.may_publish("aldo", "channel?") == false);
}

void DataManager_test::testFileSubscribe_name_null()
{
    DataManager dm_file(filename);
    CPPUNIT_ASSERT_THROW(dm_file.may_subscribe(nullstring, "channel?"),
        Poco::Exception);
}
void DataManager_test::testFilePublish_name_null()
{
    DataManager dm_file(filename);
    CPPUNIT_ASSERT_THROW(dm_file.may_publish(nullstring, "channel?"),
        Poco::Exception);
}
void DataManager_test::testFileSubscribe_channel_null()
{
    DataManager dm_file(filename);
    CPPUNIT_ASSERT_THROW(dm_file.may_subscribe("aldo", nullstring),
        Poco::Exception);
}
void DataManager_test::testFilePublish_channel_null()
{
    DataManager dm_file(filename);
    CPPUNIT_ASSERT_THROW(dm_file.may_publish("aldo", nullstring),
        Poco::Exception);
}
void DataManager_test::testFile_GetSecretByNULLname()
{
    DataManager dm_file(filename);
    CPPUNIT_ASSERT_THROW(dm_file.getSecretbyName(nullstring),
        Poco::Exception);
}
void DataManager_test::testFile_GetSecretByNOTEXISTINGname()
{
    DataManager dm_file(filename);
    CPPUNIT_ASSERT_THROW(dm_file.getSecretbyName("inotexist"),
        Poco::Exception);
}
#endif
Test *DataManager_test::suite()
{
    TestSuite *suiteOfTests = new CppUnit::TestSuite("DataManager_test");
    suiteOfTests->addTest(
        new CppUnit::TestCaller<DataManager_test>("testFileMode",
                &DataManager_test::testFileMode));
    suiteOfTests->addTest(
        new CppUnit::TestCaller<DataManager_test>("testFile_malformedFile",
                &DataManager_test::testFile_malformedFile));
    suiteOfTests->addTest(
        new CppUnit::TestCaller<DataManager_test>("testFile_tooLongFile",
                &DataManager_test::testFile_tooLongFile));
    suiteOfTests->addTest(
        new CppUnit::TestCaller<DataManager_test>("testFile_blankRows",
                &DataManager_test::testFile_blankRows));
    suiteOfTests->addTest(
        new CppUnit::TestCaller<DataManager_test>("testFile_getSecret",
                &DataManager_test::testFile_getSecret));
    suiteOfTests->addTest(
        new CppUnit::TestCaller<DataManager_test>("testNotExistingFile",
                &DataManager_test::testNotExistingFile));
#ifdef __WITH_MONGO__
    suiteOfTests->addTest(
        new CppUnit::TestCaller<DataManager_test>("testMongoDB",
                &DataManager_test::testMongoDB));
    suiteOfTests->addTest(
        new CppUnit::TestCaller<DataManager_test>("testMongoDB_wrongIP",
                &DataManager_test::testMongoDB_wrongIP));
    suiteOfTests->addTest(
        new CppUnit::TestCaller<DataManager_test>
        ("testMongoDB_wrongCollection",
                &DataManager_test::testMongoDB_wrongCollection));
    suiteOfTests->addTest(
        new CppUnit::TestCaller<DataManager_test>("testMongoDB_wrongDB",
                &DataManager_test::testMongoDB_wrongDB));
    suiteOfTests->addTest(
        new CppUnit::TestCaller<DataManager_test>(
                "testMongoDBSubscribe_present",
                &DataManager_test::testMongoDBSubscribe_present));
    suiteOfTests->addTest(
        new CppUnit::TestCaller<DataManager_test>("testMongoDBPublish_present",
                &DataManager_test::testMongoDBPublish_present));
    suiteOfTests->addTest(
        new CppUnit::TestCaller<DataManager_test>(
                "testMongoDBSubscribe_user_not_present",
                &DataManager_test::testMongoDBSubscribe_user_not_present));
    suiteOfTests->addTest(
        new CppUnit::TestCaller<DataManager_test>(
                "testMongoDBPublish_user_not_present",
                &DataManager_test::testMongoDBPublish_user_not_present));
    suiteOfTests->addTest(
        new CppUnit::TestCaller<DataManager_test>(
                "testMongoDBSubscribe_channel_not_present",
                &DataManager_test::testMongoDBSubscribe_channel_not_present));
    suiteOfTests->addTest(
        new CppUnit::TestCaller<DataManager_test>(
                "testMongoDBPublish_channel_not_present",
                &DataManager_test::testMongoDBPublish_channel_not_present));
    suiteOfTests->addTest(
        new CppUnit::TestCaller<DataManager_test>(
                "testMongoDBSubscribe_name_null",
                &DataManager_test::testMongoDBSubscribe_name_null));
    suiteOfTests->addTest(
        new CppUnit::TestCaller<DataManager_test>(
                "testMongoDBPublish_name_null",
                &DataManager_test::testMongoDBPublish_name_null));
    suiteOfTests->addTest(
        new CppUnit::TestCaller<DataManager_test>(
                "testMongoDBSubscribe_channel_null",
                &DataManager_test::testMongoDBSubscribe_channel_null));
    suiteOfTests->addTest(
        new CppUnit::TestCaller<DataManager_test>(
                "testMongoDBPublish_channel_null",
                &DataManager_test::testMongoDBPublish_channel_null));
    suiteOfTests->addTest(
        new CppUnit::TestCaller<DataManager_test>(
                "testMongoDB_GetSecretByNULLname",
                &DataManager_test::testMongoDB_GetSecretByNULLname));
    suiteOfTests->addTest(
        new CppUnit::TestCaller<DataManager_test>(
                "testMongoDB_GetSecretByNOTEXISTINGname",
                &DataManager_test::testMongoDB_GetSecretByNOTEXISTINGname));
#else
    suiteOfTests->addTest(
        new CppUnit::TestCaller<DataManager_test>(
                "testFileSubscribe_present",
                &DataManager_test::testFileSubscribe_present));
    suiteOfTests->addTest(
        new CppUnit::TestCaller<DataManager_test>("testFilePublish_present",
                &DataManager_test::testFilePublish_present));
    suiteOfTests->addTest(
        new CppUnit::TestCaller<DataManager_test>(
                "testFileSubscribe_user_not_present",
                &DataManager_test::testFileSubscribe_user_not_present));
    suiteOfTests->addTest(
        new CppUnit::TestCaller<DataManager_test>(
                "testFilePublish_user_not_present",
                &DataManager_test::testFilePublish_user_not_present));
    suiteOfTests->addTest(
        new CppUnit::TestCaller<DataManager_test>(
                "testFileSubscribe_channel_not_present",
                &DataManager_test::testFileSubscribe_channel_not_present));
    suiteOfTests->addTest(
        new CppUnit::TestCaller<DataManager_test>(
                "testFilePublish_channel_not_present",
                &DataManager_test::testFilePublish_channel_not_present));
    suiteOfTests->addTest(
        new CppUnit::TestCaller<DataManager_test>(
                "testFileSubscribe_name_null",
                &DataManager_test::testFileSubscribe_name_null));
    suiteOfTests->addTest(
        new CppUnit::TestCaller<DataManager_test>(
                "testFilePublish_name_null",
                &DataManager_test::testFilePublish_name_null));
    suiteOfTests->addTest(
        new CppUnit::TestCaller<DataManager_test>(
                "testFileSubscribe_channel_null",
                &DataManager_test::testFileSubscribe_channel_null));
    suiteOfTests->addTest(
        new CppUnit::TestCaller<DataManager_test>(
                "testFilePublish_channel_null",
                &DataManager_test::testFilePublish_channel_null));
    suiteOfTests->addTest(
        new CppUnit::TestCaller<DataManager_test>(
                "testFile_GetSecretByNULLname",
                &DataManager_test::testFile_GetSecretByNULLname));
    suiteOfTests->addTest(
        new CppUnit::TestCaller<DataManager_test>(
                "testFile_GetSecretByNOTEXISTINGname",
                &DataManager_test::testFile_GetSecretByNOTEXISTINGname));
#endif
return suiteOfTests;
}
