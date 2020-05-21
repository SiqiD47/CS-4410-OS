#include <pthread.h>

class a4_rwlock {
    private:
        /* example of defining integers:
         *     int num_reader, num_writer;
         * example of defining conditional variables:
         *     pthread_cond_t contitional_variable;
         */
        /* Your Part II code goes here */
        int waitingReaders, waitingWriters, activeReaders, activeWriters;
    	pthread_mutex_t lock;
    	pthread_cond_t canRead, canWrite;

    public:
        a4_rwlock() {
            /* initializing the variables 
             * example: num_reader = num_writer = 0
             */
            /* Your Part II code goes here */
            waitingReaders = waitingWriters = activeReaders = activeWriters = 0;
        }

    void reader_enter() {
        /* Your Part II code goes here */
        pthread_mutex_lock( & lock);
        waitingReaders++;
        while (activeWriters > 0 || waitingWriters > 0)
            pthread_cond_wait( & canRead, & lock);
        waitingReaders--;
        activeReaders++;
        pthread_mutex_unlock( & lock);
    }

    void reader_exit() {
        /* Your Part II code goes here */
        pthread_mutex_lock( & lock);
        activeReaders--;
        if (activeReaders == 0 && waitingWriters > 0)
            pthread_cond_signal( & canWrite);
        pthread_mutex_unlock( & lock);
    }

    void writer_enter() {
        /* Your Part II code goes here */
        pthread_mutex_lock( & lock);
        waitingWriters++;
        while (activeWriters > 0 || activeReaders > 0)
            pthread_cond_wait( & canWrite, & lock);
        waitingWriters--;
        activeWriters = 1;
        pthread_mutex_unlock( & lock);
    }

    void writer_exit() {
        /* Your Part II code goes here */
        pthread_mutex_lock( & lock);
        activeWriters = 0;
        if (waitingWriters > 0)
            pthread_cond_signal( & canWrite);
        else if (waitingReaders > 0)
            pthread_cond_broadcast( & canRead);
        pthread_mutex_unlock( & lock);
    }
};
