
# (C) CS 4410 Fall 2020 Staff, Cornell University
# All rights reserved

from rvr_sync_wrapper import MP, MPthread
import random
import signal

signal.signal(signal.SIGINT, signal.SIG_DFL)

#   Complete the implementation of the BrokerageManager monitor using MP Locks
#   and MP Condition variables

################################################################################
## DO NOT WRITE OR MODIFY ANY CODE ABOVE THIS LINE #############################
################################################################################

class BrokerageManager(MP):
    """ A recent indsutry trend has been toward on-demand, pay-per-use resources
        philosophy in which computing resources are requested and obtained when
        needed. This is sometimes called "resource brokerage."

    Another popular architecture pattern is the use of microservices. They are
    frequently implemented as small, modular programs that run in contained
    environments and can communicate with other software entities.

    Suppose your system has 3 separate microservices running independently which
    each run some computation on a DB, notify BrokerageManager when finished,
    sleep for a period of time, and repeat. Your system has the following 3
    types of "compute" microservices: AWSLambdaTask, CronTask, and IndexingTask.

    The BrokerageManager is a "manager" microservice which does the following:
    wait for a round of AWSLambdaTask, CronTask, and IndexingTask to complete
    (1 of each), and form a request for CPU resources on some cluster to update
    a shared DB.

    You are a software engineer on the infra team assigned to complete the
    BrokerageManager monitor. With your 4410 sync knowledge, you should ensure
    that this monitor does not form a resource request without results from
    all of the 3 compute tasks, and does not attempt to form a request that
    contains more than 1 round of results from any of the compute tasks.

    The following are some concrete examples:
    Good:   (1 AWSLambdaTask, 1 CronTask, 1 IndexingTask) -> resource request

    Bad:    (1 AWSLambdaTask, 2 CronTask, 1 IndexingTask) -> resource request
            (1 AWSLambdaTask, 1 CronTask) -> resource request

    (Aside: Tasks do not need to be used in order of arrival, i.e. you do not need
    to guarantee fairness (such as FIFO or other) within tasks of the same type.)
    """

    def __init__(self):
        """ Initializes an instance of BrokerageManager. Shared resources and
            locks get created in here.

        """
        MP.__init__(self)
        # TODO implement me
        #self.debug = True
        self.lock = self.Lock('lock')
        self.AWSLambdaTask = self.Shared('number of AWSLambdaTask', 0)
        self.CronTask = self.Shared('number of CronTask', 0)
        self.IndexingTask = self.Shared('number of IndexingTask', 0)
        self.AWSLambdaTask_available = self.lock.Condition('AWSLambdaTask available')
        self.CronTask_available = self.lock.Condition('CronTask available')
        self.IndexingTask_available = self.lock.Condition('IndexingTask available')
        self.can_request = self.lock.Condition('can make a request')


    def lambda_task_arrived(self):
        """ Let BrokerageManager know that an AWSLambdaTask has completed and
            is offering its results for usage in a resource request.

        This procedure should block until 1 CronTask and 1 IndexingTask are
        available to be used in a resource request. Exiting this function
        represents using the AWSLambdaTask results in a resource request.
        """
        # TODO implement me
        with self.lock:
            self.AWSLambdaTask.inc()
            while self.CronTask.read() == 0 or self.IndexingTask.read() == 0:
                self.can_request.wait()
            self.CronTask_available.signal()
            self.IndexingTask_available.signal()
            self.CronTask.dec()
            self.IndexingTask.dec()
            self.AWSLambdaTask.dec()


    def cron_task_arrived(self):
        """ Let BrokerageManager know that a CronTask has completed and
            is offering its results for usage in a resource request.

        This procedure should block until 1 AWSLambdaTask and 1 IndexingTask are
        available to be used in a resource request. Exiting this function
        represents using the CronTask results in a resource request.
        """
        # TODO implement me
        with self.lock:
            self.CronTask.inc()
            if self.AWSLambdaTask.read() >= 1 and self.CronTask.read() >= 1 and self.IndexingTask.read() >= 1:
                self.can_request.signal()
            else:
                self.CronTask_available.wait()


    def indexing_task_arrived(self):
        """ Let BrokerageManager know that a IndexingTask has completed and
            is offering its results for usage in a resource request.

        This procedure should block until 1 AWSLambdaTask and 1 CronTask are
        available to be used in a resource request. Exiting this function
        represents using the IndexingTask results in a resource request.
        """
        # TODO implement me
        with self.lock:

            self.IndexingTask.inc()
            if self.AWSLambdaTask.read() >= 1 and self.CronTask.read() >= 1 and self.IndexingTask.read() >= 1:
                self.can_request.signal()
            else:
                self.IndexingTask_available.wait()


################################################################################
## DO NOT WRITE OR MODIFY ANY CODE BELOW THIS LINE #############################
################################################################################

class AWSLambdaTask(MPthread):
    def __init__(self, manager, id):
        MPthread.__init__(self, manager, id)
        self.manager = manager
        self.id = id


    def run(self):
        self.delay(random.uniform(0, 0.2))
        print('lambda task results {} arrived'.format(self.id))
        self.manager.lambda_task_arrived()
        print('lambda task results {} has been used'.format(self.id))


class CronTask(MPthread):
    def __init__(self, manager, id):
        MPthread.__init__(self, manager, id)
        self.manager = manager
        self.id = id


    def run(self):
        self.delay(random.uniform(0, 0.3))
        print('cron task results {} arrived'.format(self.id))
        self.manager.cron_task_arrived()
        print('cron task results {} has been used'.format(self.id))


class IndexingTask(MPthread):
    def __init__(self, manager, id):
        MPthread.__init__(self, manager, id)
        self.manager = manager
        self.id = id


    def run(self):
        self.delay(random.uniform(0, 0.2))
        print('indexing task results {} arrived'.format(self.id))
        self.manager.indexing_task_arrived()
        print('indexing task results {} has been used'.format(self.id))


if __name__ == '__main__':

    NUM_LAMBDA_TASKS = 5
    NUM_CRON_TASKS = 5
    NUM_INDEXING_TASKS = 5

    manager = BrokerageManager()
    for i in range(NUM_LAMBDA_TASKS):
        AWSLambdaTask(manager, i).start()
    for j in range(NUM_CRON_TASKS):
        CronTask(manager, j).start()
    for k in range(NUM_INDEXING_TASKS):
        IndexingTask(manager, k).start()

    manager.Ready()
