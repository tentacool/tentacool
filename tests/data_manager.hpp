#ifndef __DATAMANAGER__
#define __DATAMANAGER__

#include <vector>
#include <map>
#include <set>
#include <string>
#include <fstream>
#include <stdio.h>
#include <iostream>
#include <Poco/Message.h>
#include <Poco/Logger.h>

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#ifdef __WITH_MONGO__
#include "mongo/client/dbclient.h"
#endif

using namespace std;
using namespace Poco;

//! DataManager manage the data reading from a file or a MongoDB collection
class DataManager
{
    typedef struct{
        string secret;              //! User secret
        set<string> _publish_chs;   //! Publish channels associated to the user
        set<string> _subscribe_chs; //! Subscribe channels associated to the user
    }user_data;

    typedef std::map<string,user_data> UserMap;

public:
    //DataManager(); //! Dummy constructor
    DataManager(string filename);
    DataManager(const string mongo_ip, const string mongodb_port,const string mongo_db, const string mongo_collection);
    virtual ~DataManager();
    const string getSecretbyName(const string name);
    bool may_publish (const string name, const string channel) const ;
    bool may_subscribe (const string name, const string channel) const ;
private:
    Logger& _logger;
    bool _mode;
    UserMap _usersMap;
    ifstream _input;
    string _filename;
    string _mongoip;
    string _mongoport;
    string _mongo_db;
    string _mongo_collection;
};

#endif
