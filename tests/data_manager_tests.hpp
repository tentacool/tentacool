#ifndef __DATAMANAGERTESTS__
#define __DATAMANAGERTESTS__

#include "data_manager.hpp"
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

using namespace CppUnit;
using namespace std;

class DataManager_test : public TestFixture{
private:
    DataManager* dm_mongodb_work;
    string filename;
    string nullstring;
    string too_long_file;
    string malformed_file;
    string not_existing_file;

public:

    void setUp(){
        try{
            filename = "auth_keys.dat";
            malformed_file = "malformed_file.dat";
            not_existing_file = "iamnotexists.boh";
            too_long_file = "toolong_line.dat";
            dm_mongodb_work = new DataManager("localhost","27017","hpfeeds","auth_key");
        }catch(Poco::Exception& e){
            cout<<e.displayText()<<endl;
        }
    }

    void tearDown(){
        delete dm_mongodb_work;
    }

    void testFileMode()
    {
        CPPUNIT_ASSERT_NO_THROW(DataManager dm_file(filename););
    }

    void testFile_malformedFile()
    {
        CPPUNIT_ASSERT_THROW(DataManager dm_file(malformed_file);,Poco::Exception);
    }

    void testFile_tooLongFile()
    {
        CPPUNIT_ASSERT_THROW(DataManager dm_file(too_long_file);,Poco::Exception);
    }

    void testFile_getSecret()
    {
        DataManager dm_file(filename);
        CPPUNIT_ASSERT(!dm_file.getSecretbyName("aldo").compare("s3cr3t"));
    }

    void testNotExistingFile()
    {
        CPPUNIT_ASSERT_THROW(DataManager data_manager(not_existing_file), Poco::Exception );
    }

    void testMongoDB(){
        CPPUNIT_ASSERT_NO_THROW(DataManager dm_mongodb("localhost","27017","hpfeeds","auth_key"));
    }

    void testMongoDB_wrongIP()
    {
        CPPUNIT_ASSERT_THROW(DataManager dm_mongodb_fail("wrong_ip","27017","hpfeeds","auth_key"), mongo::DBException);
    }
    void testMongoDB_wrongCollection()
    {
        CPPUNIT_ASSERT_NO_THROW(DataManager dm_mongodb_fail("localhost","27017","hpfeeds","wrong_collection"));
    }

    void testMongoDB_wrongDB()
    {
        CPPUNIT_ASSERT_NO_THROW(DataManager dm_mongodb_fail("localhost","27017","wrongDB","auth_key"));
    }

    void testMongoDBSubscribe_present()
    {
        DataManager dm_mongodb_work("localhost","27017","hpfeeds","auth_key");
        CPPUNIT_ASSERT(dm_mongodb_work.may_subscribe("aldo","ch1"));
    }

    void testMongoDBPublish_present()
    {
        DataManager dm_mongodb_work("localhost","27017","hpfeeds","auth_key");
        CPPUNIT_ASSERT(dm_mongodb_work.may_publish("aldo","ch1"));
    }

    void testMongoDBSubscribe_user_not_present()
    {
        DataManager dm_mongodb_work("localhost","27017","hpfeeds","auth_key");
        CPPUNIT_ASSERT_THROW(dm_mongodb_work.may_subscribe("mark","ch1"), Poco::Exception);
    }

    void testMongoDBPublish_user_not_present(){
        CPPUNIT_ASSERT_THROW(dm_mongodb_work->may_publish("joy","ch1"), Poco::Exception);
    }

    void testMongoDBSubscribe_channel_not_present(){
        CPPUNIT_ASSERT(dm_mongodb_work->may_subscribe("aldo","missingchannel")==false);
    }

    void testMongoDBPublish_channel_not_present()
    {
        CPPUNIT_ASSERT(dm_mongodb_work->may_publish("aldo","channel?")==false);
    }

    void testMongoDBSubscribe_name_null()
    {
        CPPUNIT_ASSERT_THROW(dm_mongodb_work->may_subscribe(nullstring,"channel?"), Poco::Exception);
    }
    void testMongoDBPublish_name_null()
    {
        CPPUNIT_ASSERT_THROW(dm_mongodb_work->may_publish(nullstring,"channel?"), Poco::Exception);
    }
    void testMongoDBSubscribe_channel_null()
    {
        CPPUNIT_ASSERT_THROW(dm_mongodb_work->may_subscribe("aldo",nullstring), Poco::Exception);
    }
    void testMongoDBPublish_channel_null()
    {
        CPPUNIT_ASSERT_THROW(dm_mongodb_work->may_publish("aldo",nullstring), Poco::Exception);
    }
    void testMongoDB_GetSecretByNULLname()
    {
        CPPUNIT_ASSERT_THROW(dm_mongodb_work->getSecretbyName(nullstring), Poco::Exception);
    }
    void testMongoDB_GetSecretByNOTEXISTINGname(){
        CPPUNIT_ASSERT_THROW(dm_mongodb_work->getSecretbyName("pippo"), Poco::Exception);
    }
    static Test *suite()
    {
        TestSuite *suiteOfTests = new CppUnit::TestSuite( "DataManager_test" );
        suiteOfTests->addTest( new CppUnit::TestCaller<DataManager_test>("testFileMode",&DataManager_test::testFileMode));
        suiteOfTests->addTest( new CppUnit::TestCaller<DataManager_test>("testFile_malformedFile",&DataManager_test::testFile_malformedFile));
        suiteOfTests->addTest( new CppUnit::TestCaller<DataManager_test>("testFile_tooLongFile",&DataManager_test::testFile_tooLongFile));
        suiteOfTests->addTest( new CppUnit::TestCaller<DataManager_test>("testFile_getSecret",&DataManager_test::testFile_getSecret));
        suiteOfTests->addTest( new CppUnit::TestCaller<DataManager_test>("testNotExistingFile",&DataManager_test::testNotExistingFile));
        suiteOfTests->addTest( new CppUnit::TestCaller<DataManager_test>("testMongoDB",&DataManager_test::testMongoDB));
        suiteOfTests->addTest( new CppUnit::TestCaller<DataManager_test>("testMongoDB_wrongIP",&DataManager_test::testMongoDB_wrongIP));
        suiteOfTests->addTest( new CppUnit::TestCaller<DataManager_test>("testMongoDB_wrongCollection",&DataManager_test::testMongoDB_wrongCollection));
        suiteOfTests->addTest( new CppUnit::TestCaller<DataManager_test>("testMongoDB_wrongDB",&DataManager_test::testMongoDB_wrongDB));
        suiteOfTests->addTest( new CppUnit::TestCaller<DataManager_test>("testMongoDBSubscribe_present",&DataManager_test::testMongoDBSubscribe_present));
        suiteOfTests->addTest( new CppUnit::TestCaller<DataManager_test>("testMongoDBPublish_present",&DataManager_test::testMongoDBPublish_present));
        suiteOfTests->addTest( new CppUnit::TestCaller<DataManager_test>("testMongoDBSubscribe_user_not_present",&DataManager_test::testMongoDBSubscribe_user_not_present));
        suiteOfTests->addTest( new CppUnit::TestCaller<DataManager_test>("testMongoDBPublish_user_not_present",&DataManager_test::testMongoDBPublish_user_not_present));
        suiteOfTests->addTest( new CppUnit::TestCaller<DataManager_test>("testMongoDBSubscribe_channel_not_present",&DataManager_test::testMongoDBSubscribe_channel_not_present));
        suiteOfTests->addTest( new CppUnit::TestCaller<DataManager_test>("testMongoDBPublish_channel_not_present",&DataManager_test::testMongoDBPublish_channel_not_present));
        suiteOfTests->addTest( new CppUnit::TestCaller<DataManager_test>("testMongoDBSubscribe_name_null",&DataManager_test::testMongoDBSubscribe_name_null));
        suiteOfTests->addTest( new CppUnit::TestCaller<DataManager_test>("testMongoDBPublish_name_null",&DataManager_test::testMongoDBPublish_name_null));
        suiteOfTests->addTest( new CppUnit::TestCaller<DataManager_test>("testMongoDBSubscribe_channel_null",&DataManager_test::testMongoDBSubscribe_channel_null));
        suiteOfTests->addTest( new CppUnit::TestCaller<DataManager_test>("testMongoDBPublish_channel_null",&DataManager_test::testMongoDBPublish_channel_null));
        suiteOfTests->addTest( new CppUnit::TestCaller<DataManager_test>("testMongoDB_GetSecretByNULLname",&DataManager_test::testMongoDB_GetSecretByNULLname));
        suiteOfTests->addTest( new CppUnit::TestCaller<DataManager_test>("testMongoDB_GetSecretByNOTEXISTINGname",&DataManager_test::testMongoDB_GetSecretByNOTEXISTINGname));

        return suiteOfTests;
    }
};

#endif /* ____DATAMANAGERTEST____ */
