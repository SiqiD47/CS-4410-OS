
# (C) Robbert van Renesse & CS 4410 Spring 2020 Staff, Cornell University
# All rights reserved

from rvr import MP, MPthread

class MonitorExample(MP):
	""" An example of the Monitor implemented in rvr_sync_wrapper library.

	Note: This file is only meant to serve as an example of how to use our
		sync library's syntax. The code here does not have any meaning!
	"""

	def __init__(self):
		MP.__init__(self, None)
		self.value = self.Shared('value', 0)
		self.my_list = [] # note we do not support Shared for data structures!!

		self.lock  = self.Lock('monitor lock')

		self.gt0 = self.lock.Condition('value greater than 0')
		self.lt2 = self.lock.Condition('value less than 2')
		self.list_empty = self.lock.Condition('list is empty')

	def get_value(self):
		with self.lock:
			return self.value.read()

	def block_until_pos(self):
		with self.lock:
			while not (self.value.read() > 0):
				self.gt0.wait()

	def update(self, value):
		with self.lock:
			self.value.write(value)
			if self.value.read() > 0:
				self.gt0.signal()
			if self.value.read() < 2:
				self.lt2.broadcast()
			if len(self.my_list) == 0:
				self.list_empty.signal()
