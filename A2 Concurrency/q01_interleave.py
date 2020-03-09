
# (C) CS 4410 Spring 2020 Staff, Cornell University
# All rights reserved

from rvr_sync_wrapper import MP, MPthread
import signal

signal.signal(signal.SIGINT, signal.SIG_DFL)

# Below are a few multiple-choice questions about the behavior of this program.
# Look for A2 - Multiple Choice on CMS and record your responses there.
# In the meantime, look over and run the following concurrent program.
#
# On bash shell:
# $ python q01_interleave.py
#
# All programs in this assignment were developed with Python 2.7.10 and do
# not require any external dependencies.
#
# 1.1) Run this concurrent program. Which of the options is true?
#       a) The outputs vary in an unpredictable way AND 2 outputs of the
#           program could never be identical
#       b) The output of the program is always exactly the same
#       c) Two outputs of the program could never be identical
#       d) The outputs vary in an unpredictable way
#
# 1.2) When code enforces no event ordering, can one rely on a particular
#   thread interleaving?
#       a) Yes
#       b) No
#
# 1.3) How many times would you have to run this program in order to observe
#   a specific interleaving?
#       a) 10 times
#       b) 100 times
#       c) 10,000 times
#       d) There's no guarantee for observing any specific interleaving of
#           threads no matter how many times
#
# 1.4) What does 1.3 imply about the effectiveness of testing to find
#   synchronization errors?
#       a) Running the code repeatedly is a good way to test for correctness.
#       b) To make sure your code is correct, you must reason about the code
#           instead of rely on observed outputs.

class Worker1(MPthread):
	def __init__(self, mp):
		MPthread.__init__(self, mp, 'Worker 1')


	def run(self):
		while True:
			print('Hello from Worker 1')


class Worker2(MPthread):
	def __init__(self, mp):
		MPthread.__init__(self, mp, 'Worker 2')


	def run(self):
		while True:
			print('Hello from Worker 2')


if __name__ == '__main__':
	mp = MP()
	w1 = Worker1(mp)
	w2 = Worker2(mp)

	w1.start()
	w2.start()
