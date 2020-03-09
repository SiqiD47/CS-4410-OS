
# (C) CS 4410 Spring 2020 Staff, Cornell University
# All rights reserved

from rvr_sync_wrapper import MP, MPthread
from collections import deque
import random
import signal

signal.signal(signal.SIGINT, signal.SIG_DFL)

FILL_CAPACITY = 4

#    Complete the implementation of the EspressoCzar monitor below using
#    MP Condition variables.

################################################################################
## DO NOT WRITE OR MODIFY ANY CODE ABOVE THIS LINE #############################
################################################################################

class EspressoCzar(MP):
	""" Three years into the future...

	You are a PhD student at Cornell University and have recently been
	elected to manage the CS Department coffee supply as the Espresso Czar. In
	other words, you are now the go-to person whenever the Gates Hall espresso
	machine is empty. Congrats!

	With the CS Department's shared love of coffee and 'round-the-clock work
	ethic, your colleagues will enter the kitchen all throughout the day and
	(rather quickly) deplete the coffee supply.

	In keeping up with the demand, you order small coffee bean shipments
	throughout the day and get notified upon delivery from DeliveryGuys. When
	notified, you immediately let him try to refill the coffee machine.

	To keep track of coffee usage, you rigged the machine with a bell to notify
	you when a CoffeeDrinker tries to make a cup of espresso.

	The fill capacity of the machine is specified by the constant FILL_CAPACITY
	and it uses up the coffee beans in a FIFO manner. In technical terms,
	"refilling the machine" means to add 1 unit of coffee beans to the machine,
	and "making a cup of espresso" means to use up 1 unit of coffee beans. The
	fill tank of the espresso machine may be represented by a data structure.

	Your task is to apply your CS 4410 mastery of synchronization to efficiently
	prevent CoffeeDrinkers from making coffee when the machine is empty and
	block DeliveryGuys from refilling the machine when it is already full.

	Clarification points:
	- 1 unit of coffee beans makes 1 cup of espresso.
	- Each shipment contains 1 unit of coffee beans (also written as "refill unit")
	- Your colleagues are civilized and only make 1 cup at a time.
	- Choose your data structure carefully! Keep in mind that general correctness
		is just as important as the synchronization-safety.
	- The goal of this assignment is to ensure you understand the internals of
		thread-safe data structures, so be explicit with protection!
	"""

	def __init__(self, max_coffee_bean_units):
		""" Initializes an instance of EspressoCzar. Shared resources and locks
			get created in here.

		:arg max_coffee_bean_units: is an int, expects value FILL_CAPACITY
		"""
		MP.__init__(self)
		# TODO implement me
		# self.debug = True
		self.deque = deque()
		self.lock = self.Lock('monitor lock')
		self.empty = self.lock.Condition('machine is empty')
		self.full = self.lock.Condition('machine is full')


	def refill_espresso_machine(self, refill_unit):
		""" Adds refill unit to the espresso machine. Recall from above that
			a single data structure is sufficient in tracking the refill_units
			currently inside the espresso machine.

		:arg refill_unit: is an int, unique identifier for the refill_unit
		"""
		# TODO implement me
		with self.lock:
			while len(self.deque) == FILL_CAPACITY:
				self.empty.wait()
			self.deque.append(refill_unit)
			self.full.signal()


	def make_espresso(self):
		""" Makes a cup of coffee from the espresso machine using the oldest
			refill unit available. Also RETURNS the int identifier of the
			refill_unit used.

		"""
		# TODO implement me
		with self.lock:
			while len(self.deque) == 0:
				self.full.wait()
			refill_unit = self.deque.popleft()
			self.empty.signal()
		return refill_unit


################################################################################
## DO NOT WRITE OR MODIFY ANY CODE BELOW THIS LINE #############################
################################################################################

class DeliveryGuy(MPthread):
	def __init__(self, czar, num):
		MPthread.__init__(self, czar, num)
		self.czar = czar
		self.id = num
		self.round_prefix = num * 100


	def run(self):
		for i in range(5):
			unit_num = self.round_prefix + i
			print('DeliveryGuy {} is trying to refill coffee machine with unit {}'.format(
				self.id,
				unit_num
			))
			self.czar.refill_espresso_machine(unit_num)
			print('DeliveryGuy {} finished refilling the coffee machine with unit {}'.format(
				self.id,
				unit_num
			))


class CoffeeDrinker(MPthread):
	def __init__(self, czar, num):
		MPthread.__init__(self, czar, num)
		self.czar = czar
		self.id = num


	def run(self):
		for i in range(5):
			print('CoffeeDrinker {} is trying to make coffee'.format(
				self.id
			))
			unit_used = self.czar.make_espresso()
			print('CoffeeDrinker {} finished making coffee with unit {}'.format(
				self.id,
				unit_used
			))


if __name__ == "__main__":
	czar = EspressoCzar(max_coffee_bean_units=FILL_CAPACITY)
	for i in range(3):
		DeliveryGuy(czar, i+1).start()
		CoffeeDrinker(czar, i+1).start()

	czar.Ready()
