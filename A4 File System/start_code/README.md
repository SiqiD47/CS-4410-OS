# CS4410 2020sp A4

| Question                                                          | Max points |
| ----------------------------------------------------------------- | :---------:|
| Part I: implement basic file system operations                    | 40         |
| Part II: implement read/write locks for concurrency               | 20         |
| Part III (BONUS): measure performance benefits of read/write lock | 10         |
| **Total**                                                         | **70**     |


## Part I: implement alloc/free/read/write of an inode (40 pts)

In this part, you will implement four APIs of an inode-based file system (i.e., allocate, free, read, write). The file system has the following layout:

```
|<-super block->|<-free block bitmap->|<-inode table->|<-data->|
|<--------------------------- disk --------------------------->|
```

First, read `inode_layer.h` for definitions. For example, the disk contains `BLOCK_NUM` blocks and each block is `BLOCK_SIZE` bytes. For each inode, there are `NDIRECT` direct blocks and `1` indirect block, so that an array `blocks[NDIRECT+1]` is defined in `struct inode`, the data structure representing a single inode. You should be familiar with these concepts from lecture or from your book (Chapter 40). Just scan this file and don't worry if some definitions still look unfamiliar.

Next, read the `get_inode` and `put_inode` functions in `inode_layer.cc`. These two functions give you a sense of how to implement a file system operation. Now you're ready for Part IA.

### Part IA: implement inode allocation

Read and implement the function `alloc_inode` in `inode_layer.cc`. The comments in the function provide useful information and hints. After your implementation, run the following:

```
> make
> ./part1_tester
========== begin test create and getattr ==========
...
========== pass test create and getattr ==========
========== begin test put and get ==========
...
[TEST_ERROR] : error ...
--------------------------------------------------
Part1 score is : 10/40
```

**Note:** if you meet errors during `make`, it is likely because of file permissions. Try `chmod 777 A4`.


### Part IB: implement inode read/write

Read and implement `alloc_block` in `inode_layer.cc`, which allocate data blocks and modify the bitmap respectively. Then implement `read_file` and `write_file`. After your implementation, run the same grading test as part IA:

```
> ./part1_tester
========== begin test create and getattr ==========
...
========== pass test create and getattr ==========
========== begin test put and get ==========
...
========== pass test put and get ==========
========== begin test remove ==========
...
[TEST_ERROR] : error ...
--------------------------------------------------
Part1 score is : 30/40
```

### Part IC: implement inode free/remove

Implement `free_inode` and `remove_file` in `inode_layer.cc` and run the same grading test:

```
> ./part1_tester
========== begin test create and getattr ==========
...
========== pass test create and getattr ==========
========== begin test put and get ==========
...
========== pass test put and get ==========
========== begin test remove ==========
...
========== pass test remove ==========
--------------------------------------------------
Part1 score is : 40/40
```

**Note:** if the grader shows 40/40, you will get all the points in this part. 

## Part II: implement read/write locks for concurrent inode access (20 pts)

In Part IB, you have implemented `read_file` and `write_file`. However, these functions are not thread-safe for now (i.e., concurrent execution of these functions can corrupt or crash the file system). If you run `./part2_tester`, you will get 

```
Part2 score is : 0/10 (there will also be a manual check portion of your Part2 score)
```

In this part, you will implement read/write locks in file `locks.h` and use these locks to protect concurrent executions of `read_file` and `write_file`. For your read/write lock, use conditional variables and search `pthread_cond_t` for examples of conditional variables in C/C++. For modifications to `read_file` and `write_file`, use the following code.

```C
rwlocks[inum].reader_enter();  // acquire reader lock for inode inum
rwlocks[inum].reader_exit();   // release reader lock for inode inum
rwlocks[inum].writer_enter();  // acquire writer lock for inode inum
rwlocks[inum].writer_exit();   // release writer lock for inode inum
```

Feel free to modify anything in your `read_file` and `write_file` functions. After you have done, run the grader:

```
> ./part2_tester
========== begin test concurrent #1 (fixed size file) ==========
...
========== pass test concurrent #1 (fixed size file) ==========
========== begin test concurrent #2 (varied size file) ==========
...
========== pass test concurrent #2 (varied size file) ==========
--------------------------------------------------
Part2 score is : 10/10 (there will also be a manual check portion of your Part2 score)
```

**Note:** `part2_tester` can take a few seconds to terminate. However, if `part2_tester` runs for too long, it is likely because of a deadlock in your implementation. If `part2_tester` crashes, your implementation has bugs. We will manually check your code: half of the total score you can receive for Part II of the project (i.e., up to 10 points) will depend on coding style and correctness.


## Part III (BONUS): measure performance benefits of read/write lock (10 pts)

In Part II, you have implemented a read/write lock. If you replace the read/write lock with a simple mutex lock, and replace the `enter/exit` functions with `acquire/release` of the mutex lock, the concurrency control will still be correct (i.e., readers cannot read concurrently in this case). However, this replacement reduces concurrency: this part of the assignments asks you to measure the performance difference that it causes.

Complete `part3_tester.cc` which measures the performance of extensive concurrent read/write of an inode. Hints have been provided to you in the comments. After you complete the code, run `part3_tester`:

```
> ./part3_tester
...
...
========== 400 readers and 2 writers run concurrently ==========
Average reader latency is 182 milliseconds.
Average writer latency is 946 milliseconds.
Total execution latency is 2562 milliseconds.
```

Run different experiments with `part3_tester` that (1) change the number of readers and writers (`nreader` and `nwriter` in the code); (2) change the lock implementation in `locks.h` (mutex or read/write lock). Measure the performance difference as follows:


| experiment\lock scheme     | read/write lock | mutex lock    |
| ------------- |:----------:| :--------------:|
| 400 reader / 2 writer      | average reader latency = **182ms** <br> average writer latency = **946ms** <br> experiment total latency=**2562ms** <br> *experiment1*  | average reader latency = ??s <br> average writer latency = ??s <br> experiment total latency=??s <br> *experiment2* |
| 400 reader / 50 writer     |  *experiment3*  | *experiment4* |
| 800 reader / 50 writer     |  *experiment5*  | *experiment6* |
| ...                        | ...             | ...           |


Run at least 6 experiments. Measure the *average reader latency*, *average writer latency* (the average latency between a single reader/writer starts and terminates) and *experiment total latency* (the latency between `part3_tester` starts and terminates). The code for measuring the experiment total latency has been provided to you in the `main` function.

In your answer, include this table and briefly explain the numbers you get. Your explanation should answer the following questions:

* How do you compare the numbers in experiment1 and experiment2?
* How do you compare the numbers in experiment1 and experiment3?
* How do you compare the numbers in experiment3 and experiment5?
* ...

The answer to this question depends very much on your machine's configuration: there is no standard right answer. Your charge is to justify and explain the numbers you see. 

**Submission Guidelines** Submit the relevant files on CMSx 

