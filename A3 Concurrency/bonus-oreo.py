
# (C) CS 4410 Fall 2020 Staff, Cornell University
# All rights reserved

from rvr_sync_wrapper import MP, MPthread
import random
import signal

signal.signal(signal.SIGINT, signal.SIG_DFL)

# This program simulates the creation of Oreos.

# Implement the OreoFactory monitor below using MPlocks and
# MPcondition variables.

################################################################################
## DO NOT WRITE OR MODIFY ANY CODE ABOVE THIS LINE #############################
################################################################################

class OreoFactory(MP):
    """ An Oreo is made from 2 cookies and a layer of icing (each piece of
        cookie and layer of icing can be used in only one oreo).

    A thread offers a piece of ingredient by calling the appropriate method;
    the thread will block until the ingredient can be used in the making of an
    Oreo.
    """

    def __init__(self):
        """ Initializes an instance of OreoFactory. Shared resources and locks
            get created in here.

        """
        MP.__init__(self)
        # TODO implement me
        self.lock = self.Lock('lock')
        self.num_cookies = self.Shared('number of cookies', 0)
        self.num_icing = self.Shared('number of layers of icing', 0)
        self.cookies_ready = self.lock.Condition('cookies are ready')
        self.icing_ready = self.lock.Condition('icing is ready')


    def icing_arrived(self):
        """ A unit of icing has arrived at the factory.

        Block until it can be used to create an oreo. Exiting this function is
        equivalent to using the unit of icing in an oreo.
        """
        # TODO implement me
        with self.lock:
            self.num_icing.inc()
            while self.num_cookies.read() < 2:
                self.icing_ready.wait()
            self.num_icing.dec()
            self.cookies_ready.signal()
            self.cookies_ready.signal()
            self.num_cookies.dec()
            self.num_cookies.dec()


    def cookie_arrived(self):
        """ A unit of cookie has arrived at the factory.

        Block until it can be used to create an oreo. Exiting this function is
        equivalent to using the unit of cookie in an oreo.
        """
        # TODO implement me
        with self.lock:
            self.num_cookies.inc()
            if self.num_cookies.read() < 2:
                self.cookies_ready.wait()
            else:
                self.icing_ready.signal()


################################################################################
## DO NOT WRITE OR MODIFY ANY CODE BELOW THIS LINE #############################
################################################################################

class Icing(MPthread):
    def __init__(self, factory, id):
        MPthread.__init__(self, factory, id)
        self.factory = factory
        self.id = id


    def run(self):
        while True:
            print('icing {} arrived'.format(self.id))
            self.factory.icing_arrived()
            print('icing {} has been used'.format(self.id))
            self.delay(random.uniform(0, 0.25))


class Cookie(MPthread):
    def __init__(self, factory, id):
        MPthread.__init__(self, factory, id)
        self.factory = factory
        self.id = id


    def run(self):
        while True:
            print('cookie {} arrived'.format(self.id))
            self.factory.cookie_arrived()
            print('cookie {} has been used'.format(self.id))
            self.delay(random.uniform(0, 0.5))


if __name__ == '__main__':

    NUM_ICING = 4
    NUM_COOKIE = 8

    factory = OreoFactory()
    for i in range(NUM_ICING):
        Icing(factory, i).start()
    for j in range(NUM_COOKIE):
        Cookie(factory, j).start()

    factory.Ready()
