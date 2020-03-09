
# (C) Robbert van Renesse & CS 4410 Spring 2020 Staff, Cornell University
# All rights reserved

from rvr import MP, MPthread

class SemaphoreExample(MP):
	""" An example of the Semaphore implemented in rvr_sync_wrapper library.

	Note: This file is only meant to serve as an example of how to use our
		sync library's syntax. The code here does not have any meaning!
	"""

	def __init__(self):
		MP.__init__(self, None)
		self.value = self.Shared('value', 0)
		self.valueLock = self.Semaphore('value lock', 1)

	def update(self, newValue):
		self.valueLock.procure()
		self.value.write(newValue)
		self.valueLock.vacate()
