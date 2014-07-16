#include "data_manager.hpp"
#include "Poco/StringTokenizer.h"
#include "Poco/AutoPtr.h"
#include <fstream>
#include <exception>
#include <memory>

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#define MAX_FIELD_LENGTH 256

using namespace Poco;
using namespace std;

//! File Constructor
//! \brief Initialize the users data structures reading from file
//! \param filename The name of the file with the auth_keys
DataManager::DataManager( string filename):
    _logger(Logger::get("HF_Broker")), _mode(false), _filename(filename)
{
    _input.open(_filename.c_str());
    if(!_input) throw Poco::Exception("Error opening the file:"+filename);
    string line = "";
    while(getline(_input, line)) {
        //TODO More input Validation?
        if(line.empty()) break;
        if(line.length() > 4*MAX_FIELD_LENGTH)
            throw Poco::Exception("One or more fields are too long!");
        StringTokenizer fields(line,";",StringTokenizer::TOK_TRIM
                /*| StringTokenizer::TOK_IGNORE_EMPTY*/);
        if(fields.count() != 4)
            throw Poco::Exception("Invalid file structure!");
        user_data u;
        u.secret = fields[1];
        StringTokenizer pub_channels(fields[2],",",StringTokenizer::TOK_TRIM);
        StringTokenizer sub_channels(fields[3],",",StringTokenizer::TOK_TRIM);
        for(StringTokenizer::Iterator iter= pub_channels.begin();
                iter!=pub_channels.end();iter++)    u._publish_chs.insert((*iter));
        for(StringTokenizer::Iterator iter2= sub_channels.begin();
                iter2!=sub_channels.end();iter2++)  u._subscribe_chs.insert(*iter2);
        _usersMap[fields[0]] = u;
    }
    _input.close();
    _logger.debug("Data fetching from file completed!");
}
#ifdef __WITH_MONGO__
//!File Constructor
//!\brief Initialize the users data structures extracting datas from a Mongodb collection
//!\param Mongo IP
//!\param Mongo Port
//!\param Mongo DB name
//!\param Mongo Collection name
DataManager::DataManager(const string mongo_ip, const string mongo_port,
                            const string mongo_db, const string mongo_collection):
 _logger(Logger::get("HF_Broker")), _mode(true), _mongoip(mongo_ip),
 _mongoport(mongo_port), _mongo_db(mongo_db), _mongo_collection(mongo_collection)
{
    mongo::DBClientConnection _conn;
    string errmsg;
    _logger.debug("Connecting to Mongodb...");
    _logger.debug("Mongo IP: "+_mongoip);
    _logger.debug("Mongo Port: "+_mongoport);
    _logger.debug("Mongo DB: "+_mongo_db);
    _logger.debug("Mongo Collection: "+_mongo_collection);
    if(!_conn.connect(_mongoip,errmsg))
        throw Poco::Exception("Mongodb connection fail");
    _logger.information("Connected with mongodb");
    unique_ptr<mongo::DBClientCursor> cursor =
            (move(_conn.query(_mongo_db+"."+_mongo_collection)));

    while(cursor->more()) { //for every document
        mongo::BSONObj p = cursor->next();
        user_data u;		//new user_data
        vector<string> subscribe_vector;
        vector<string> publish_vector;
        u.secret = p.getStringField("secret");
        p.getObjectField("subscribe").Vals(subscribe_vector);
        p.getObjectField("publish").Vals(publish_vector);
        copy(subscribe_vector.begin(), subscribe_vector.end(),
                inserter(u._subscribe_chs, u._subscribe_chs.begin()));
        copy(publish_vector.begin(), publish_vector.end(),
                inserter( u._publish_chs,  u._publish_chs.begin()));
        _usersMap[p.getStringField("identifier")] = u;
    }
    if(_usersMap.empty())
        throw Poco::Exception("Mongodb connection empty or not existing!");
    _logger.information("Data fetching from mongodb completed!");
}
#endif

DataManager::~DataManager() {}

const string DataManager::getSecretbyName(const string name)
{
    //! Return the secret of the specified user
    //! \param Username the name of the user
    if(name.empty()) throw  Poco::Exception("Invalid User argument!");
    UserMap::const_iterator iter = _usersMap.find(name);
    if(iter!=_usersMap.end()) { //found it!
        return (*iter).second.secret;
    }else throw  Poco::Exception("User not present!");
}
bool DataManager::may_subscribe (const string name, const string channel) const
{
    //!
    //! Return true if the client can subscribe to the channel, false otherwise
    //! \param Username the name of the user
	//! \param Channel the name of the channel
    if(name.empty() || channel.empty()) throw  Poco::Exception("Empty name or channel!");
    UserMap::const_iterator iter = _usersMap.find(name);
    if(iter!=_usersMap.end()) { //found it!
        user_data u = (*iter).second;
        if(find(u._subscribe_chs.begin(), u._subscribe_chs.end(), channel)!=u._subscribe_chs.end())
            return true;
        else
            return false;
    }else throw  Poco::Exception("User not present!");
}

bool DataManager::may_publish (const string name, const string channel) const
{
    //! Return true if the client can publish data on the channel, false otherwise.
    //! \param Username the name of the user
    //! \param Channel the name of the channel
    if(name.empty() || channel.empty()) throw Poco::Exception("Empty name or channel!");
    UserMap::const_iterator iter = _usersMap.find(name);

    if(iter!=_usersMap.end()) { //found it!
        user_data u = (*iter).second;
        if(find(u._publish_chs.begin(), u._publish_chs.end(), channel)!=u._publish_chs.end())
            return true;
        else
            return false;
    }else throw  Poco::Exception("User not present!");
}
