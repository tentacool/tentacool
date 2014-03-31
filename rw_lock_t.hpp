#ifndef __RW_LOCK_T__
#define __RW_LOCK_T__

#include <pthread.h>

class rw_lock_t
{
    int NoOfReaders, NoOfWriters, NoOfWritersWaiting;
    pthread_mutex_t class_mutex;
    pthread_cond_t  reader_gate;
    pthread_cond_t  writer_gate;

public:
    rw_lock_t();
    virtual ~rw_lock_t();
    void r_lock();
    void w_lock();
    void r_unlock();
    void w_unlock();
};

#endif /* __RW_LOCK_T__ */
