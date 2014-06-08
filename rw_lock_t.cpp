#include <pthread.h>
#include "rw_lock_t.hpp"

ReadWriteLock::ReadWriteLock()
    : NoOfReaders(0), NoOfWriters(0), NoOfWritersWaiting(0),
      class_mutex(PTHREAD_MUTEX_INITIALIZER),
      reader_gate(PTHREAD_COND_INITIALIZER),
      writer_gate(PTHREAD_COND_INITIALIZER)
{}

ReadWriteLock::~ReadWriteLock()
{
    pthread_mutex_destroy(&class_mutex);
    pthread_cond_destroy(&reader_gate);
    pthread_cond_destroy(&writer_gate);
}

void ReadWriteLock::r_lock()
{
    pthread_mutex_lock(&class_mutex);
    //while(NoOfWriters>0 || NoOfWritersWaiting>0) //Writer Preference
    while (NoOfWriters>0) {
        pthread_cond_wait(&reader_gate, &class_mutex);
    }
    NoOfReaders++;
    pthread_mutex_unlock(&class_mutex);
}

void ReadWriteLock::w_lock()
{
    pthread_mutex_lock(&class_mutex);
    NoOfWritersWaiting++;
    while (NoOfReaders>0 || NoOfWriters>0) {
        pthread_cond_wait(&writer_gate, &class_mutex);
    }
    NoOfWritersWaiting--; NoOfWriters++;
    pthread_mutex_unlock(&class_mutex);
}

void ReadWriteLock::r_unlock()
{
    pthread_mutex_lock(&class_mutex);
    NoOfReaders--;
    if (NoOfReaders == 0 && NoOfWritersWaiting > 0)
        pthread_cond_signal(&writer_gate);
    pthread_mutex_unlock(&class_mutex);
}

void ReadWriteLock::w_unlock()
{
    pthread_mutex_lock(&class_mutex);
    NoOfWriters--;
    if(NoOfWritersWaiting > 0)
        pthread_cond_signal(&writer_gate);
    //else //Writer Preference - don't signal readers unless no writers
    pthread_cond_broadcast(&reader_gate);
    pthread_mutex_unlock(&class_mutex);
}
