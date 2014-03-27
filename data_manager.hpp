#ifndef __DATAMANAGER__
#define __DATAMANAGER__

#include <vector>
#include <map>
#include <string>
#include <fstream>
#include <stdio.h>
#include <iostream>
#include <Poco/Message.h>
#include <Poco/Logger.h>
#include "mongo/client/dbclient.h"

using namespace std;
using namespace Poco;

//!
//! Manage the data reading (name, auth_keys and publish/subscribe channels) from
//! a file or a MongoDB collection
class DataManager
{

    typedef struct{
        string secret;
        set<string> _publish_chs;   //! Publish channels
        set<string> _subscribe_chs; //! Subscribe channels
    }user_data;

    typedef std::map<string,user_data> UserMap; //! Map<Channel Name, User Data>

public:
    DataManager( string filename);
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

#endif /* __DATAMANAGER__ */
