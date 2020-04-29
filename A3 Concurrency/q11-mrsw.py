
# (C) CS 4410 Fall 2020 Staff, Cornell University
# All rights reserved

from rvr_sync_wrapper import MP, MPthread
import random
import signal

signal.signal(signal.SIGINT, signal.SIG_DFL)

#    Modify the ReaderWriterManager below to fit the specification.

################################################################################
## DO NOT WRITE OR MODIFY ANY CODE ABOVE THIS LINE #############################
################################################################################

class ReaderWriterManager(MP):
    """ This problem simulates a variant of the classic Readers-Writers problem.

    Below, Multiple Reader Single Writer (MRSW) is implemented using Locks and
    Condition variables. The MRSW implementation below should enforce these
    invariants:

    1) At any time, multiple readers OR a single writer is allowed in the
    critical section (multiple writer may lead to corrupt data).
    2) In our case, "multiple readers" means any amount of readers.
    3) Readers and Writers may not simultaneously be in the critical section.

    The following code appears to meet these requirements. However there are
    2 issues within the implementation that make it less than ideal. Your job
    here is to modify the ReaderWriterManager to enforce safety and liveness
    for any amount of Readers and Writers efficiently.

    Some questions to help guide your modifications:
    1) Is it really necessary to broadcast everywhere?
    2) What if the ReaderWriterManager has to handle infinite streams of
    Readers and Writers? Can you think of a starvation case for this code?
         Note: Your solution does not have to be fair-to-all, but we want you to state and rationalize (in a short comment)
         the fairness policy you choose to implement.

    =======================================================================================================================

    Answer:
    Q1: It is not necessary to broadcast everywhere. Since only one writer is allowed
    in the critical section, for writers, it is enough to use 'signal()'.

    Q2: Writer starvation. All the waiting readers and writers will be notified when
    a writer or a reader exits. It is highly possible that there're still readers in the
    critical section. Therefore, some waiting writers may wait indefinitely.

    Modification: Added two more shared variables for waiting readers and writers and
    used two condition variables to control the readers and writers. This satisfies the
    safety property: (#r >= 0) and (0 <= #w <= 1) and (#r > 0) => (#w = 0) as well as
    the liveness property: all readers and writers that want to enter the critical section
    will eventually succeed. This implementation give priority to writers. It avoids the problem
    of writer starvation by preventing any new readers from acquiring the lock if there is a
    writer queued and waiting for the lock; the writer will acquire the lock as soon as all
    readers which were already holding the lock have completed.
    """

    def __init__(self):
        """ Initializes an instance of ReaderWriterManager. Shared resources and
            locks get created in here.

        """
        MP.__init__(self)
        self.monitor_lock = self.Lock('monitor lock')

        self.num_readers = self.Shared('readers currently reading', 0)
        self.num_writers = self.Shared('writers currently writing', 0)
        self.waiting_readers = self.Shared('number of waiting readers', 0)
        self.waiting_writers = self.Shared('number of waiting writers', 0)
        self.can_read = self.monitor_lock.Condition('can read')
        self.can_write = self.monitor_lock.Condition('can write')


    def reader_enter(self):
        """ Called by Reader thread whe trying to enter the critical section.

        Reaching the end of this function means that the Reader successfully
        enters the critical section.
        """
        with self.monitor_lock:
            self.waiting_readers.inc()
            while self.num_writers.read() > 0 or self.waiting_writers.read() > 0:
                self.can_read.wait()
            self.waiting_readers.dec()
            self.num_readers.inc()


    def reader_exit(self):
        """ Called by Reader thread after leaving the critical section.

        """
        with self.monitor_lock:
            self.num_readers.dec()
            if self.num_readers.read() == 0 and self.waiting_writers.read() > 0:
                self.can_write.signal()


    def writer_enter(self):
        """ Called by Writer thread when trying to enter the critical section.

        Reaching the end of this function means that the Writer successfully
        enters the critical section.
        """
        with self.monitor_lock:
            self.waiting_writers.inc()
            while self.num_readers.read() > 0 or self.num_writers.read() > 0:
                self.can_write.wait()
            self.waiting_writers.dec()
            self.num_writers.inc()


    def writer_exit(self):
        """ Called by Writer thread after leaving the critical section.

        """
        with self.monitor_lock:
            self.num_writers.dec()
            if self.waiting_writers.read() > 0:
                self.can_write.signal()
            elif self.waiting_readers.read() > 0:
                self.can_read.broadcast()


################################################################################
## DO NOT WRITE OR MODIFY ANY CODE BELOW THIS LINE #############################
################################################################################

class Reader(MPthread):
    def __init__(self, manager, num):
        MPthread.__init__(self, manager, num)
        self.manager = manager
        self.id = num


    def run(self):
        self.manager.reader_enter()

        ### BEGIN CRITICAL SECTION ###
        with self.manager.monitor_lock:
            print('reader {} inside. crit section: {} readers   | {} writers'.format(
                self.id,
                self.manager.num_readers.read(),
                self.manager.num_writers.read()
            ))
        # -> this is where reading would happen
        self.delay(random.randint(0, 1))
        ### END CRITICAL SECTION ###

        self.manager.reader_exit()


class Writer(MPthread):
    def __init__(self, manager, num):
        MPthread.__init__(self, manager, num)
        self.manager = manager
        self.id = num


    def run(self):
        self.delay(random.randint(0,1))
        self.manager.writer_enter()

        ### BEGIN CRITICAL SECTION ###
        with self.manager.monitor_lock:
            print('writer {} inside. crit section: {} readers   | {} writers'.format(
                self.id,
                self.manager.num_readers.read(),
                self.manager.num_writers.read()
            ))
        # -> this is where writing would happen
        ### END CRITICAL SECTION ###

        self.manager.writer_exit()


if __name__ == '__main__':

    manager = ReaderWriterManager()
    for i in range(10):
        Reader(manager, i).start()
        Writer(manager, i).start()

    manager.Ready()
