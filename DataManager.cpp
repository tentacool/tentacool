/*
 * DataManager.cpp
 *
 *  Created on: 14/mar/2014
 *      Author: aldo
 */

#include "DataManager.hpp"
#include "rw_lock_t.hpp"
#include "Poco/StringTokenizer.h"
#include <fstream>
#include <exception>
#include <string>
#include <vector>
#include <iostream>
#include <stdio.h>


DataManager::DataManager( string& filename): _logger(Logger::get("HF_Brocker")), _mode(false), _filename(filename) {
	/*!
	 *\brief Initialize the users data structures reading from file
	 *\param filename (with absolute path)
	 */
	_input.open(_filename.c_str());
	if(!_input) throw exception();
	string line ="";
	while(getline(_input,line )){
		Poco::StringTokenizer fields(line,";",Poco::StringTokenizer::TOK_TRIM);
		user_data u;
		u.secret = fields[1];
		Poco::StringTokenizer pub_channels(fields[2],",",Poco::StringTokenizer::TOK_TRIM);
		Poco::StringTokenizer sub_channels(fields[3],",",Poco::StringTokenizer::TOK_TRIM);
		for(Poco::StringTokenizer::Iterator iter= pub_channels.begin();
				iter!=pub_channels.end();iter++)	u._publish_chs.push_back((*iter));
		for(Poco::StringTokenizer::Iterator iter2= sub_channels.begin();
				iter2!=sub_channels.end();iter2++)	u._subscribe_chs.push_back(*iter2);
		_usersMap[fields[0]] = u;
	}
	_input.close();
	_logger.debug("Data fetching from file completed!");

}
DataManager::DataManager(const string& mongo_ip, const string& mongo_port, const string& mongo_db, const string& mongo_collection)
:_logger(Logger::get("HP_Brocker")), _mode(true), _mongoip(mongo_ip), _mongoport(mongo_port), _mongo_db(mongo_db), _mongo_collection(mongo_collection)
{
	mongo::DBClientConnection _conn;
	try{
		_logger.information("Connecting to Mongodb...");
		_logger.information("Mongo IP: "+_mongoip);
		_logger.information("Mongo Port: "+_mongoport);
		_logger.information("Mongo DB: "+_mongo_db);
		_logger.information("Mongo Collection: "+_mongo_collection);
		_conn.connect(_mongoip);
	}catch(mongo::DBException& dbe){
		_logger.error("Connection to MongoDB failed!");
	}

	try{
		auto_ptr<mongo::DBClientCursor> cursor =
				_conn.query(_mongo_db+"."+_mongo_collection);

		while(cursor->more()){ //for every document
			mongo::BSONObj p = cursor->next();
			user_data u;		//new user_data
			u.secret = p.getStringField("secret");
			p.getObjectField("subscribe").Vals(u._subscribe_chs);
			p.getObjectField("publish").Vals(u._publish_chs);
			_usersMap[p.getStringField("identifier")] = u;
		}
	}catch(mongo::AssertionException& ae){
		throw new mongo::AssertionException(ae);
	}catch(exception& e){ throw new exception();}
	_logger.information("Data fetching from mongodb completed!");

}
DataManager::~DataManager() {
	// TODO Auto-generated destructor stub
}
const string& DataManager::getSecretbyName(const string& name){
	/*!
	 * Return the secret of the specified user
	 * \param Username
	 */
	if(name.compare("")==0) throw new exception();
	UserMap::iterator iter = _usersMap.find(name);
	if(iter!=_usersMap.end()){ //found it!
		return (*iter).second.secret;
	}else throw new exception();
}
bool DataManager::may_subscribe (const string& name, const string& channel) const{
	/*!
		 * Return true if the client can subscribe to the channel, false otherwise
		 * \param Username
		 * \param Channel name
		 */
	if(name.compare("")==0 || channel.compare("")==0) return false;
	UserMap::const_iterator iter = _usersMap.find(name);
	if(iter!=_usersMap.end()){ //found it!
		user_data u = (*iter).second;
		if(find(u._subscribe_chs.begin(), u._subscribe_chs.end(), channel)!=u._subscribe_chs.end()) return true;
		else return false;
	}else throw new exception();

	return false;
}

bool DataManager::may_publish (const string& name, const string& channel) const{
	/*!
	 * Return true if the client can publish data on the channel, false otherwise.
	 * \param Username
	 * \param Channel name
	 */
	if(name.compare("")==0 || channel.compare("")==0) return false;
	UserMap::const_iterator iter = _usersMap.find(name);

	if(iter!=_usersMap.end()){ //found it!
		user_data u = (*iter).second;
		if(find(u._publish_chs.begin(), u._publish_chs.end(), channel)!=u._publish_chs.end()) return true;
		else return false;
	}else throw new exception();

	return false;
}

