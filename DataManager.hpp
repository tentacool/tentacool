/*
 * DataManager.hpp
 *
 *  Created on: 14/mar/2014
 *      Author: aldo
 */

#ifndef DATAMANAGER_HPP_
#define DATAMANAGER_HPP_

#include <vector>
#include <map>
#include <string>
#include <fstream>
#include <stdio.h>
#include <iostream>
#include <Poco/Message.h>
#include <Poco/Logger.h>
#include "mongo/client/dbclient.h"

/*!
 * Manage the data reading (name, auth_keys and publish/subscribe channels from
 * a file
 */
using namespace std;
using namespace Poco;

class DataManager {

	typedef struct{
		string secret;
		vector<string> _publish_chs;
		vector<string> _subscribe_chs;
	}user_data;


	typedef std::map<string,user_data> UserMap;

public:
	DataManager( string& filename);
	DataManager(const string& mongo_ip, const string& mongodb_port,const string& mongo_db, const string& mongo_collection);
	virtual ~DataManager();
	const string& getSecretbyName(const string& name);
	bool may_publish (const string& name, const string& channel) const ;
	bool may_subscribe (const string& name, const string& channel) const ;
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

#endif /* DATAMANAGER_HPP_ */
