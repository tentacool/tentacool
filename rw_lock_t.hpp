/*
 * rw_lock_t.hpp
 *
 *  Created on: 14/mar/2014
 *      Author: aldo
 */

#ifndef RW_LOCK_T_HPP_
#define RW_LOCK_T_HPP_

#include <pthread.h>

class rw_lock_t {

    int NoOfReaders, NoOfWriters, NoOfWritersWaiting;
    pthread_mutex_t class_mutex;
    pthread_cond_t  reader_gate;
    pthread_cond_t  writer_gate;

public:
    rw_lock_t();
    virtual ~rw_lock_t();
    void  r_lock();
    void  w_lock();
    void  r_unlock();
    void  w_unlock();
};

#endif /* RW_LOCK_T_HPP_ */
