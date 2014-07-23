#ifndef __DATAMANAGERTESTS__
#define __DATAMANAGERTESTS__

#include "../data_manager.hpp"
#include <cppunit/TestFixture.h>
#include <cppunit/TestCase.h>

using namespace CppUnit;
using namespace std;

class DataManager_test : public TestFixture
{
private:
    DataManager* dm_mongodb_work;
#ifdef __WITH_MONGO__
    mongo::DBClientConnection _conn;
#endif
    string _exe_path;
    string filename;
    string nullstring;
    string too_long_file;
    string malformed_file;
    string black_rows_file;
    string not_existing_file;

public:

    DataManager_test();
    virtual ~DataManager_test();
    int getPath(char* pBuf);
    void setUp();
    void tearDown();
    void testFileMode();
    void testFile_malformedFile();
    void testFile_tooLongFile();
    void testFile_blankRows();
    void testFile_getSecret();
    void testNotExistingFile();

    void testMongoDB();
    void testMongoDB_wrongIP();
    void testMongoDB_wrongCollection();
    void testMongoDB_wrongDB();
    void testMongoDBSubscribe_present();
    void testMongoDBPublish_present();
    void testMongoDBSubscribe_user_not_present();
    void testMongoDBPublish_user_not_present();
    void testMongoDBSubscribe_channel_not_present();
    void testMongoDBPublish_channel_not_present();
    void testMongoDBSubscribe_name_null();
    void testMongoDBPublish_name_null();
    void testMongoDBSubscribe_channel_null();
    void testMongoDBPublish_channel_null();
    void testMongoDB_GetSecretByNULLname();
    void testMongoDB_GetSecretByNOTEXISTINGname();
    void testFileSubscribe_present();
    void testFilePublish_present();
    void testFileSubscribe_user_not_present();
    void testFilePublish_user_not_present();
    void testFileSubscribe_channel_not_present();
    void testFilePublish_channel_not_present();
    void testFileSubscribe_name_null();
    void testFilePublish_name_null();
    void testFileSubscribe_channel_null();
    void testFilePublish_channel_null();
    void testFile_GetSecretByNULLname();
    void testFile_GetSecretByNOTEXISTINGname();

    static Test *suite();
};

#endif
