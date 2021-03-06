#ifndef __SAFE_SET__
#define __SAFE_SET__

#include <set>
#include "rw_lock_t.hpp"

using namespace std;

//! SafeSet is a template class implementing thread-safe operations on a set
template <class T> class SafeSet {

    /*static*/ ReadWriteLock _mutex;
    set<T> _unsafe_set;
    typedef std::set<T> set_t;

public:
    //! Iterator typedef
    typedef typename set_t::iterator iterator;

    SafeSet() {}

    virtual ~SafeSet() {}

    iterator find(const T& val) {
        //! thread-safe find()
        _mutex.r_lock();
        iterator i= _unsafe_set.find(val);
        _mutex.r_unlock();
        return i;
    }
    iterator begin() {
        //! thread-safe begin()
        _mutex.r_lock();
        iterator i= _unsafe_set.begin();
        _mutex.r_unlock();
        return i;
    }
    iterator end() {
        //! thread-safe end()
        _mutex.r_lock();
        iterator i= _unsafe_set.end();
        _mutex.r_unlock();
        return i;
    }
    uint32_t size() {
        //! thread-safe size()
        _mutex.r_lock();
        uint32_t size = _unsafe_set.size();
        _mutex.r_unlock();
        return size;
    }
    pair<iterator,bool> insert (const T& val) {
        //! thread-safe insert(const T& val)
        _mutex.w_lock();
        pair<iterator,bool> p = _unsafe_set.insert(val);
        _mutex.w_unlock();
        return p;
    }
    uint32_t erase (const T& val) {
        //! thread-safe erase(const T& val)
        _mutex.w_lock();
        uint32_t res = _unsafe_set.erase(val);
        _mutex.w_unlock();
        return res;
    }
};

#endif
