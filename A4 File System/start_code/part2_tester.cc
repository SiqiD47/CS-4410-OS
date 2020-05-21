/* part2 tester.  
 * Test whether read/write lock works.
 */

#include "fs_client.h"
#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>

#define FILE_NUM 50
#define LARGE_FILE_SIZE 512*64
int DISK_ACCESS_LATENCY = 0;

fs_client *ec;
int total_score = 0;


#define N_READ_WRITE_TEST 10000
std::string str1, str2;
fs_protocol::fsid_t id;
int err_in_single_repeated_writer = 0;
int err_in_single_repeated_reader = 0;


void *single_repeated_writer( void *ptr ) {
    for (int i = 0; i < N_READ_WRITE_TEST; i++) {
        // randomly write str1 or str2 to the id
        int rnum = rand() % 2;
        if (rnum) {
            if (ec->put(id, str1) != fs_protocol::OK) {
                printf("error put, return not OK\n");
                err_in_single_repeated_writer = 1;
                break;
            }            
        } else {
            if (ec->put(id, str2) != fs_protocol::OK) {
                printf("error put, return not OK\n");
                err_in_single_repeated_writer = 1;
                break;
            }
        }

        if (err_in_single_repeated_reader || err_in_single_repeated_writer)
            break;
    }

    return NULL;
}

void *single_repeated_reader( void * ptr ) {
    std::string readbuf;
    for (int i = 0; i < N_READ_WRITE_TEST; i++) {
        if (ec->get(id, readbuf) != fs_protocol::OK) {
            printf("error get, return not OK\n");
            err_in_single_repeated_reader = 1;
            break;
        }

        if (str1.compare(readbuf) != 0 && str2.compare(readbuf) != 0) {
            std::cout << "error get, not consistent with put large file" << std::endl;
            err_in_single_repeated_reader = 1;
            break;
        }

        if (err_in_single_repeated_reader || err_in_single_repeated_writer)
            break;
    }
}

int test_concurrent() {
    // concurrent read/write to fixed size file
    int i, rnum;
    char *buf = (char *)malloc(LARGE_FILE_SIZE + 1);
    srand((unsigned)time(NULL));

    printf("========== begin test concurrent #1 (fixed size file) ==========\n");

    /* prepare a large file randomly */
    ec->create(fs_protocol::T_FILE, id);
    for (i = 0; i < LARGE_FILE_SIZE; i++) {
        rnum = rand() % 26;
        buf[i] = 97 + rnum;
    }
    buf[LARGE_FILE_SIZE] = 0;

    /* write to the file for the first time */
    std::string strbuf(buf);
    if (ec->put(id, strbuf) != fs_protocol::OK) {
        printf("error put, return not OK\n");
        return 1;
    }

    /* read and compare the file */
    std::string readbuf;
    if (ec->get(id, readbuf) != fs_protocol::OK) {
        printf("error get, return not OK\n");
        return 2;
    }
    if (strbuf.compare(readbuf) != 0) {
        std::cout << "error get, not consistent with put large file" << std::endl;
        //std::cout << buf << " <-> " << buf_2 << std::endl;
        return 3;
    }

    /* start concurrent read/write test */
    /* prepare two different strings str1 and str2 */
    for (i = 0; i < LARGE_FILE_SIZE; i++) {
        rnum = rand() % 26;
        buf[i] = 97 + rnum;
    }
    str1 = strbuf;
    str2 = buf;

    // create two threads to run single_repeated_writer
    pthread_t reader1, reader2;
    int status1 = pthread_create(&reader1, NULL, single_repeated_writer, NULL);
    int status2 = pthread_create(&reader2, NULL, single_repeated_writer, NULL);
    // create two threads to run single_repeated_reader
    pthread_t writer1, writer2;
    int status3 = pthread_create(&writer1, NULL, single_repeated_reader, NULL);
    int status4 = pthread_create(&writer2, NULL, single_repeated_reader, NULL);

    if (status1 != 0 || status2 != 0 || status3 != 0 || status4 != 0) {
        std::cout << "Contact TA if you meet this error, Yunhao (yz2327@cornell.edu)" << std::endl;
        return 4;
    }

    pthread_join( reader1, NULL);
    pthread_join( reader2, NULL);
    pthread_join( writer1, NULL);
    pthread_join( writer2, NULL);

    if (err_in_single_repeated_reader || err_in_single_repeated_writer)
        return 5;
    printf("========== pass test concurrent #1 (fixed size file) ==========\n");
    total_score += 5;


    printf("========== begin test concurrent #2 (varied size file) ==========\n");

    /* start concurrent read/write test */
    /* prepare two different strings str1 and str2 */
    char *buf2 = (char *)malloc(LARGE_FILE_SIZE*2 + 1);
    for (i = 0; i < LARGE_FILE_SIZE*2; i++) {
        rnum = rand() % 26;
        buf2[i] = 97 + rnum;
    }
    buf2[LARGE_FILE_SIZE*2] = 0;
    str1 = strbuf;
    str2 = buf2;
    /* write to the file for the first time */
    if (ec->put(id, str1) != fs_protocol::OK) {
        printf("error put, return not OK\n");
        return 7;
    }

    // create two threads to run single_repeated_writer
    int status5 = pthread_create(&writer1, NULL, single_repeated_writer, NULL);
    int status6 = pthread_create(&writer2, NULL, single_repeated_writer, NULL);
    // create two threads to run single_repeated_reader
    int status7 = pthread_create(&reader1, NULL, single_repeated_reader, NULL);
    int status8 = pthread_create(&reader2, NULL, single_repeated_reader, NULL);


    if (status5 != 0 || status6 != 0 || status7 != 0 || status8 != 0) {
        std::cout << "Contact TA if you meet this error, Yunhao (yz2327@cornell.edu)" << std::endl;
        return 8;
    }

    pthread_join( writer1, NULL);
    pthread_join( writer2, NULL);
    pthread_join( reader1, NULL);
    pthread_join( reader2, NULL);

    if (err_in_single_repeated_reader || err_in_single_repeated_writer)
        return 9;
    printf("========== pass test concurrent #2 (varied size file) ==========\n");
    total_score += 5;

    return 0;
}


int main(int argc, char *argv[])
{
    if (argc != 1) {
        printf("Usage: ./part2_tester\n");
        return 1;
    }
  
    ec = new fs_client();

    int err;
    if ((err = test_concurrent()) != 0)
        goto test_finish;

test_finish:
    printf("---------------------------------\n");
    printf("Part2 score is : %d/10 (there will also be a manual check portion of your Part2 score)\n", total_score);
    return 0;
}
