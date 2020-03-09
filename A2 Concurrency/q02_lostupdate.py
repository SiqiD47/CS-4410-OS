
# (C) CS 4410 Spring 2020 Staff, Cornell University
# All rights reserved

import time
import random

from rvr_sync_wrapper import MP, MPthread
import signal

signal.signal(signal.SIGINT, signal.SIG_DFL)

# Below are a few multiple-choice questions about the behavior of this program.
# Look for A2 - Multiple Choice on CMS and record your responses there.
# You will also need to modify the code below and submit this file.
#
# This program simulates a game between two teams. Each team presses
# their button as fast as they can. There is a counter that starts at
# zero; the red team's button increments the counter, while the blue
# team's button decrements the counter. They each get to press their
# button 10000 times. If the counter ends up positive, the read team
# wins; a negative counter means the blue team wins.
#
# This game is inherently boring; it should always end in a draw. However the
# provided implementation behaves somewhat differently. Run the script a few
# times and observe the end counter.
#
# 2.1) When both threads terminate, what is the largest possible score?
#       a) 10,000
#       b) 20,000
#       c) Could be ANY positive number
#       d) 0
#       e) 10,001
#
# 2.2) What other values can the score be when both threads have terminated?
#       a) -10,000 | 0 | 10,000
#       b) -20,000 | 0 | 20,000
#       c) Any integer between -10,000 and 10,000
#       d) Any integer between -20,000 and 20,000
#       e) Any integer
#
#   Note: "|" means "or" in this context
#
# TODO: Add appropriate synchronization such that updates to the
# counter occur in a critical section, ensuring that the end game counter
# is always 0 when the two threads terminate.
#
# Your synchronization must still allow interleaving between the two threads.

class Contest(MP):
	def __init__(self):
		MP.__init__(self)
		self.counter = self.Shared('counter', 0)
		self.counterLock = self.Semaphore('counter lock', 1)


	def pushRed(self):
		# TODO modify me
		self.counterLock.procure()
		self.counter.inc()
		self.counterLock.vacate()


	def pushBlue(self):
		# TODO modify me
		self.counterLock.procure()
		self.counter.dec()
		self.counterLock.vacate()


class RedTeam(MPthread):
	def __init__(self, contest):
		MPthread.__init__(self, contest, 'Red Team')
		self.contest = contest


	def run(self):
		for i in range(10000):
			self.contest.pushRed()


class BlueTeam(MPthread):
	def __init__(self, contest):
		MPthread.__init__(self, contest, 'Blue Team')
		self.contest = contest


	def run(self):
		for i in range(10000):
			self.contest.pushBlue()


################################################################################
## DO NOT WRITE OR MODIFY ANY CODE BELOW THIS LINE #############################
################################################################################

if __name__ == '__main__':
	contest = Contest()
	red  = RedTeam(contest)
	blue = BlueTeam(contest)

	red.start()
	blue.start()
	contest.Ready()

	print('The end counter is {}'.format(contest.counter.read()))
