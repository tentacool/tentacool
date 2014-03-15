/*
 * DataFileManager.h
 *
 *  Created on: 01/mar/2014
 *      Author: aldo
 */

#ifndef DATAFILEMANAGER_HPP_
#define DATAFILEMANAGER_HPP_

#include <fstream>
#include <string>
#include <vector>
#include <Poco/Message.h>
#include <Poco/Logger.h>

using namespace std;

/*!
 * Manage the data reading (name, auth_keys and publish/subscribe channels from
 * a file
 */

class DataFileManager {

	typedef struct{
		string name;
		string secret;
		vector<string> _publish_chs;
		vector<string> _subscribe_chs;
	}user;

public:

	DataFileManager();
	static void init(const string& file);
	virtual ~DataFileManager();
	static const string& getSecretbyName(const string& name);
	bool may_publish (const string& name, const string& channel) const ;
	bool may_subscribe (const string& name, const string& channel) const ;
private:
	static vector<user> _users;/*!< It's static in order to  have a unique copy of the data */
	static ifstream _input;
	static string _filename;
};


#endif /* DATAFILEMANAGER_HPP_ */
