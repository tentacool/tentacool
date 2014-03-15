/*
 * SafeSet.hpp
 *
 *  Created on: 07/mar/2014
 *      Author: aldo
 */

#ifndef SAFESET_HPP_
#define SAFESET_HPP_

#include <set>
#include "Poco/Mutex.h"
#include "Poco/ScopedLock.h"


using namespace std;
/*!
 * Template class implementing thread-safe operations on a std::set
 * by using a Poco::ScopedLock, which perform auto-unlocking at the end of the scope
 * where it was created (so perform the unlock in its destructor)
 */
template <class T> class SafeSet {

	static Poco::Mutex _mutex;
	set<T> _unsafe_set;
	typedef std::set<T> set_t;

public:
	typedef typename set_t::iterator iterator;

	SafeSet(){

	}
	virtual ~SafeSet(){

	}
	iterator find(const T& val){ /*! thread-safe find() */
		Poco::Mutex::ScopedLock lock(_mutex);
		return _unsafe_set.find(val);
	}
	iterator begin(){ /*! thread-safe begin() */
			Poco::Mutex::ScopedLock lock(_mutex);
			return _unsafe_set.begin();
		}
	iterator end(){ /*! thread-safe end() */
			Poco::Mutex::ScopedLock lock(_mutex);
			return _unsafe_set.end();
		}
	uint32_t size(){ /*! thread-safe size() */
		Poco::Mutex::ScopedLock lock(_mutex);
		return _unsafe_set.size();
	}
	pair<iterator,bool> insert (const T& val){ /*! thread-safe insert(val) */
		Poco::Mutex::ScopedLock lock(_mutex);
		return _unsafe_set.insert(val);
	}
	uint32_t erase (const T& val){ /*! thread-safe erase(val) */
		Poco::Mutex::ScopedLock lock(_mutex);
		return _unsafe_set.erase(val);
	}
};

#endif /* SAFESET_HPP_ */
