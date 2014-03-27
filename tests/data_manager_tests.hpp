#ifndef __DATAMANAGERTESTS__
#define __DATAMANAGERTESTS__

#include <string>
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
    DataManager* dm_file;
    DataManager* dm_not_existing_file;
    DataManager* dm_mongodb;
    DataManager* dm_mongodb_fail;
    DataManager* dm_mongodb_work;
    string filename; string nullstring;
    string not_existing_file;

public:

    void setUp(){
        try{
            filename = "/home/aldo/workspace/HpfeedsBroker/src/auth_keys.dat";
            not_existing_file = "iamnotexists.boh";
            dm_file = new DataManager(filename);
            dm_mongodb_work = new DataManager("localhost","27017","hpfeeds","auth_key");
        }catch(Poco::Exception& e){
            cout<<e.displayText()<<endl;
        }
    }

    void tearDown(){
        delete dm_file;
        delete dm_mongodb;
        delete dm_not_existing_file;
    }

    void testExistingFile(){
		  CPPUNIT_ASSERT(dm_file->getSecretbyName("aldo") != dm_file->getSecretbyName("filippo") );
	  }
	  void testNotExistingFile(){
		  CPPUNIT_ASSERT_THROW(dm_not_existing_file = new DataManager(not_existing_file), Poco::Exception );
	  }
	  void testMongoDB(){
		  CPPUNIT_ASSERT_NO_THROW(dm_mongodb = new DataManager("localhost","27017","hpfeeds","auth_key"));
	  }

	  void testMongoDB_wrongIP(){
		  CPPUNIT_ASSERT_THROW(dm_mongodb_fail = new DataManager("wrong_ip","27017","hpfeeds","auth_key"), exception);
	  }
	  void testMongoDB_wrongCollection(){
		  CPPUNIT_ASSERT_NO_THROW(dm_mongodb_fail = new DataManager("localhost","27017","hpfeeds","wrong_collection"));
	  }

	  void testMongoDB_wrongDB(){
		  CPPUNIT_ASSERT_NO_THROW(dm_mongodb_fail = new DataManager("localhost","27017","wrongDB","auth_key"));
	  }

	  void testMongoDBSubscribe_present(){
		  CPPUNIT_ASSERT(dm_mongodb_work->may_subscribe("aldo","ch1"));
	  }

	  void testMongoDBPublish_present(){
		  CPPUNIT_ASSERT(dm_mongodb_work->may_publish("aldo","ch1"));
	  }

	  void testMongoDBSubscribe_user_not_present(){
		  CPPUNIT_ASSERT_THROW(dm_mongodb_work->may_subscribe("mark","ch1"), exception);
	  }

	  void testMongoDBPublish_user_not_present(){
		  CPPUNIT_ASSERT_THROW(dm_mongodb_work->may_publish("joy","ch1"), exception);
	  }

	  void testMongoDBSubscribe_channel_not_present(){
		  CPPUNIT_ASSERT(dm_mongodb_work->may_subscribe("aldo","missingchannel")==false);
	  }

	  void testMongoDBPublish_channel_not_present(){
		  CPPUNIT_ASSERT(dm_mongodb_work->may_publish("aldo","channel?")==false);
	  }

	  void testMongoDBSubscribe_name_null(){
		  CPPUNIT_ASSERT_THROW(dm_mongodb_work->may_subscribe(nullstring,"channel?"), exception);
	  }
	  void testMongoDBPublish_name_null(){
		  CPPUNIT_ASSERT_THROW(dm_mongodb_work->may_publish(nullstring,"channel?"), exception);
	  }
	  void testMongoDBSubscribe_channel_null(){
		  CPPUNIT_ASSERT_THROW(dm_mongodb_work->may_subscribe("aldo",nullstring), exception);
	  }
	  void testMongoDBPublish_channel_null(){
		  CPPUNIT_ASSERT_THROW(dm_mongodb_work->may_publish("aldo",nullstring), exception);
	  }
	  void testMongoDB_GetSecretByNULLname(){
		  CPPUNIT_ASSERT_THROW(dm_mongodb_work->getSecretbyName(nullstring), exception);
	  }
	  void testMongoDB_GetSecretByNOTEXISTINGname(){
		  CPPUNIT_ASSERT_THROW(dm_mongodb_work->getSecretbyName("pippo"), exception);
	  }
	  static Test *suite()
	  	  {
	  	    TestSuite *suiteOfTests = new CppUnit::TestSuite( "DataManager_test" );
	  	    suiteOfTests->addTest( new CppUnit::TestCaller<DataManager_test>("testExistingFile",&DataManager_test::testExistingFile));
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

#endif /* __DATAMANAGERTEST__ */
