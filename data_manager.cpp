#include "data_manager.hpp"
#include "Poco/StringTokenizer.h"
#include <fstream>
#include <exception>

DataManager::DataManager( string filename):
    _logger(Logger::get("HF_Broker")), _mode(false), _filename(filename) {
    //!
    //!\brief Initialize the users data structures reading from file
    //!\param filename (with absolute path)
    _input.open(_filename.c_str());
    if(!_input) throw Poco::Exception("Error opening the file");
    string line = "";
    while(getline(_input,line )){
        //TODO Input Validation
        Poco::StringTokenizer fields(line,";",Poco::StringTokenizer::TOK_TRIM);
        user_data u;
        u.secret = fields[1];
        Poco::StringTokenizer pub_channels(fields[2],",",Poco::StringTokenizer::TOK_TRIM);
        Poco::StringTokenizer sub_channels(fields[3],",",Poco::StringTokenizer::TOK_TRIM);
        for(Poco::StringTokenizer::Iterator iter= pub_channels.begin();
                iter!=pub_channels.end();iter++)	u._publish_chs.insert((*iter));
        for(Poco::StringTokenizer::Iterator iter2= sub_channels.begin();
                iter2!=sub_channels.end();iter2++)	u._subscribe_chs.insert(*iter2);
        _usersMap[fields[0]] = u;
    }
    _input.close();
    _logger.debug("Data fetching from file completed!");
}

DataManager::DataManager(const string mongo_ip, const string mongo_port,
                            const string mongo_db, const string mongo_collection):
 _logger(Logger::get("HF_Broker")), _mode(true), _mongoip(mongo_ip),
 _mongoport(mongo_port), _mongo_db(mongo_db), _mongo_collection(mongo_collection)
{
    mongo::DBClientConnection _conn;
    _logger.debug("Connecting to Mongodb...");
    _logger.debug("Mongo IP: "+_mongoip);
    _logger.debug("Mongo Port: "+_mongoport);
    _logger.debug("Mongo DB: "+_mongo_db);
    _logger.debug("Mongo Collection: "+_mongo_collection);
    _conn.connect(_mongoip);
    _logger.debug("Connected with mongodb");

    auto_ptr<mongo::DBClientCursor> cursor =
            _conn.query(_mongo_db+"."+_mongo_collection);

    while(cursor->more()){ //for every document
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
    _logger.information("Data fetching from mongodb completed!");
}

DataManager::~DataManager() {}

const string DataManager::getSecretbyName(const string name)
{
    //! Return the secret of the specified user
    //! \param Username the name of the user
    if(name.empty()) throw  Poco::Exception("User not present!");
    UserMap::const_iterator iter = _usersMap.find(name);
    if(iter!=_usersMap.end()){ //found it!
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
    if(iter!=_usersMap.end()){ //found it!
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

    if(iter!=_usersMap.end()){ //found it!
        user_data u = (*iter).second;
        if(find(u._publish_chs.begin(), u._publish_chs.end(), channel)!=u._publish_chs.end())
            return true;
        else
            return false;
    }else throw  Poco::Exception("User not present!");
}
