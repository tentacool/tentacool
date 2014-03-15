/*
 * DataFileManager.cpp
 *
 *  Created on: 01/mar/2014
 *      Author: aldo
 */

#include "DataFileManager.hpp"
#include "Poco/StringTokenizer.h"
#include <fstream>
#include <exception>
#include <string>
#include <vector>
#include <iostream>
//#include <>
using namespace std;

vector<DataFileManager::user> DataFileManager::_users;
string DataFileManager::_filename;
ifstream DataFileManager::_input;

DataFileManager::DataFileManager() {
}

void DataFileManager::init(const string& file){
	/*!
	 *\brief Initialize the users data structures reading from file
	 *\param filename (with absolute path)
	 */
	_filename = file;
	_input.open(_filename.c_str());
	if(!_input) throw exception();
	string line ="";
	while(getline(_input,line )){
		Poco::StringTokenizer fields(line,";",Poco::StringTokenizer::TOK_TRIM);
		user u;
		u.name = fields[0];
		u.secret = fields[1];
		Poco::StringTokenizer pub_channels(fields[2],",",Poco::StringTokenizer::TOK_TRIM);
		Poco::StringTokenizer sub_channels(fields[3],",",Poco::StringTokenizer::TOK_TRIM);
		for(Poco::StringTokenizer::Iterator iter= pub_channels.begin();
				iter!=pub_channels.end();iter++)	u._publish_chs.push_back((*iter));
		for(Poco::StringTokenizer::Iterator iter2= sub_channels.begin();
				iter2!=sub_channels.end();iter2++)	u._subscribe_chs.push_back(*iter2);
		_users.push_back(u);
	}
	_input.close();
}
DataFileManager::~DataFileManager() {
	_input.close();
}


const string& DataFileManager::getSecretbyName(const string& name)  {
	/*!
	 * Return the secret of the specified user
	 * \param Username
	 */
	if(name.compare("")==0) throw new exception();
	vector<user>::const_iterator iter = _users.begin();
	for(;iter!=_users.end();iter++){
		if(!((user) *iter).name.compare(name)){
			return ((user) *iter).secret;
		}
	}
	if(iter==_users.end()) throw new exception();
	return "null";
}

bool DataFileManager::may_publish (const string& name, const string& channel) const {
	/*!
	 * Return true if the client can publish data on the channel, false otherwise.
	 * \param Username
	 * \param Channel name
	 */
	if(name.compare("")==0 || channel.compare("")==0) return false;
	vector<user>::const_iterator iter = _users.begin();
		for(;iter!=_users.end();iter++){
			user u = *iter;
			if(!u.name.compare(name)){
				if(find(u._publish_chs.begin(), u._publish_chs.end(), channel)!=u._publish_chs.end())
					return true;
				else return false;
			}
		}
		return false;
}
bool DataFileManager::may_subscribe (const string& name, const string& channel) const {
	/*!
	 * Return true if the client can subscribe to the channel, false otherwise
	 * \param Username
	 * \param Channel name
	 */
	if(name.compare("")==0 || channel.compare("")==0) return false;
	vector<user>::const_iterator iter = _users.begin();
		for(;iter!=_users.end();iter++){
			user u = *iter;
			if(!(u.name.compare(name))){
				//cout<<((user) *iter)._subscribe_chs[0]<<endl;
				if(find(u._subscribe_chs.begin(), u._subscribe_chs.end(), channel)!=u._subscribe_chs.end())
					return true;
				else return false;
			}
		}
		return false;
}

