
# (C) CS 4410 Fall 2020 Staff, Cornell University
# All rights reserved

from rvr_sync_wrapper import MP, MPthread
import random
import signal

signal.signal(signal.SIGINT, signal.SIG_DFL)

#    Modify the Coordinator below to fit the specification.

################################################################################
## DO NOT WRITE OR MODIFY ANY CODE ABOVE THIS LINE #############################
################################################################################

class Coordinator(MPthread):
    """ The provided program organizes car rentals for an airport.

    Each car is labeled with a number identifier and is requested by tourists.
    For the purposes of this problem, each car is represented by a MP Lock.

    When a Coordinator calls <car>.acquire(), it reserves that car for a client.
    Similarly, when it calls <car>.release(), it retrieves the car from a client.
    Upon retrieval, the car is available for rental again

    Clients generally reserve a single car at a time. But with probability 1/2,
    the client party requires more than one car.

    Each car may only be assigned to one client at a time.

    Your job here is to modify the following code to prevent deadlock. It should
    require only a few lines of code changes, additions, or removals.
    """

    def __init__(self, id, mp):
        MPthread.__init__(self, mp, 'Coordinator')
        self.id = id


    def run(self):
        while True:

            first_rental = random.randrange(CARS_IN_STOCK)
            extra_rental = random.randrange(CARS_IN_STOCK)  # may go unused

            needs_extra = random.getrandbits(1)
            if needs_extra and (extra_rental != first_rental):
                if extra_rental < first_rental:  # ordering
                    rental_cars[extra_rental].acquire()
                    rental_cars[first_rental].acquire()
                else:
                    rental_cars[first_rental].acquire()
                    rental_cars[extra_rental].acquire()
            else:
                rental_cars[first_rental].acquire()
                needs_extra = 0

            # perform the rental
            if needs_extra:
                print('coordinator {} reserved cars {}, {}'.format(
                    self.id,
                    first_rental,
                    extra_rental
                ))
            else:
                print('coordinator {} reserved car {}'.format(
                    self.id,
                    first_rental
                ))

            self.delay(0.1)

            if needs_extra:
                if extra_rental < first_rental:  # ordering
                    rental_cars[extra_rental].release()
                    rental_cars[first_rental].release()
                else:
                    rental_cars[first_rental].release()
                    rental_cars[extra_rental].release()
            else:
                rental_cars[first_rental].release()


################################################################################
## DO NOT WRITE OR MODIFY ANY CODE BELOW THIS LINE #############################
################################################################################

if __name__ == '__main__':

    CARS_IN_STOCK = 5
    manager = MP()
    rental_cars = [manager.Lock('m'+str(i)) for i in range(CARS_IN_STOCK)]
    for i in range(3):
        Coordinator(i, manager).start()
