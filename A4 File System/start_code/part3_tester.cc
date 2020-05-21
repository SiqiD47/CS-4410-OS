/* part3 tester.  
 * Performance test of read/write lock vs. mutex lock.
 */

#include "fs_client.h"
#include <stdio.h>
#include <unistd.h>
#include <stdint.h>
#include <sys/types.h>
#include <sys/time.h>


#define FILE_NUM 50
#define LARGE_FILE_SIZE 512*64
int DISK_ACCESS_LATENCY = 200;
fs_client *ec;
std::string str1, str2;
fs_protocol::fsid_t id;

/*
 * helper function to measure latency
 */
static uint64_t get_usec(){
    struct timespec tmp;
    clock_gettime(CLOCK_MONOTONIC,&tmp);
    uint64_t result=tmp.tv_sec;
    result*=1000*1000;
    result+=tmp.tv_nsec/1000;
    return result;
}


/*
 * single writer function
 */
int err_in_single_writer = 0;
void *single_writer( void* arg ) {
    /* start timer */
    uint64_t* latency = (uint64_t*) arg;
    uint64_t start = get_usec();
    
    /* randomly write str1 or str2 to the id */
    int rnum = rand() % 2;
    if (rnum) {
        if (ec->put(id, str1) != fs_protocol::OK) {
            printf("error put, return not OK\n");
            err_in_single_writer = 1;
        }            
    } else {
        if (ec->put(id, str2) != fs_protocol::OK) {
            printf("error put, return not OK\n");
            err_in_single_writer = 1;
        }
    }

    /* end timer */
    uint64_t end = get_usec();
    *latency = end - start;
}

/*
 * single reader function
 */
int err_in_single_reader = 0;
void *single_reader( void * arg ) {
    /* start timer */
    uint64_t* latency = (uint64_t*) arg;
    uint64_t start = get_usec();

    std::string readbuf;
    if (ec->get(id, readbuf) != fs_protocol::OK) {
        printf("error get, return not OK\n");
        err_in_single_reader = 1;
    }

    if (str1.compare(readbuf) != 0 && str2.compare(readbuf) != 0) {
        std::cout << "error get, not consistent with put large file" << std::endl;
        err_in_single_reader = 1;
    }

    /* end timer */
    uint64_t end = get_usec();
    *latency = end - start;
}


/*
 * run concurrent readers and writers
 */
int test_concurrent() {
    int i, rnum;
    srand((unsigned)time(NULL));

    /* prepare a large file randomly */
    char *buf1 = (char *)malloc(LARGE_FILE_SIZE + 1);
    ec->create(fs_protocol::T_FILE, id);
    for (i = 0; i < LARGE_FILE_SIZE; i++) {
        rnum = rand() % 26;
        buf1[i] = 97 + rnum;
    }
    buf1[LARGE_FILE_SIZE] = 0;

    /* prepare another large file randomly */
    char *buf2 = (char *)malloc(LARGE_FILE_SIZE*2 + 1);
    for (i = 0; i < LARGE_FILE_SIZE*2; i++) {
        rnum = rand() % 26;
        buf2[i] = 97 + rnum;
    }
    buf2[LARGE_FILE_SIZE*2] = 0;
    str1 = buf1;
    str2 = buf2;

    /* write to the file for the first time */
    if (ec->put(id, str1) != fs_protocol::OK) {
        printf("error put, return not OK\n");
        return 7;
    }

    /* create nreader reader threads */
    const int nreader = 400;
    pthread_t readers[nreader];
    uint64_t reader_latencies[nreader];
    uint64_t avg_reader_latency = 0;
    for (int i = 0; i < nreader; i++)
        pthread_create(&readers[i], NULL, single_reader, (void*)(reader_latencies+i));

    const int nwriter = 2;
    pthread_t writers[nwriter];
    uint64_t writer_latencies[nwriter];
    uint64_t avg_writer_latency = 0;

    /* 
     * Your Part III code goes here.
     * step1: create nwriter writer threads; for thread i, pass writer_latencies[i] as parameter to function single_writer
     * step2: wait for all nwriter threads to terminate
     * step3: calculate average writer latency
     */

    for (int i = 0; i < nwriter; i++)
        pthread_create(&writers[i], NULL, single_writer, (void*)(writer_latencies+i));
    for (int i = 0; i < nwriter; i++)
        pthread_join( writers[i], NULL);
    for (int i = 0; i < nwriter; i++)
        avg_writer_latency += writer_latencies[i];
    avg_writer_latency /= nwriter;

    /* Your Part III code ends here */
  
    /* wait for all readers to finish */
    for (int i = 0; i < nreader; i++)
        pthread_join( readers[i], NULL);
    for (int i = 0; i < nreader; i++)
        avg_reader_latency += reader_latencies[i];
    avg_reader_latency /= nreader;
  
    printf("========== %d readers and %d writers run concurrently ==========\n", nreader, nwriter);
    printf("Average reader latency is %lu milliseconds.\n", avg_reader_latency / 1000);

    if (avg_writer_latency == 0) {
        printf("Average writer latency is not counted. Please complete the code.\n");
    } else {
        printf("Average writer latency is %lu milliseconds.\n", avg_writer_latency / 1000);
    }

    return 0;
}


int main(int argc, char *argv[])
{
    if (argc != 1) {
        printf("Usage: ./part3_tester\n");
        return 1;
    }

    /* start timer */
    uint64_t start = get_usec();

    ec = new fs_client();
    int err = test_concurrent();

    /* end timer */
    uint64_t end = get_usec();
    uint64_t time_span_ms = (end - start) / 1000;

    if (err!= 0 || err_in_single_writer != 0 || err_in_single_reader != 0)
        printf("Your code probably has a bug. Make sure that the tests in part2 have passed.");
    else {
        printf("Total execution latency is %lu milliseconds.\n", time_span_ms);
    }

    return 0;
}
