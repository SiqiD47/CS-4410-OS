
# (C) Robbert van Renesse & CS 4410 Spring 2020 Staff, Cornell University
# All rights reserved

import sys
import time
import threading
from Queue import Queue

class Shared:
	def __init__(self, mp, name, val):
		self.mp = mp
		self.lock = threading.Lock()
		self.name = name
		self.val = val

	def read(self):
		with self.lock:
			if self.mp.debug:
				print('thread {} reading {} --> {}'.format(
					threading.current_thread().name,
					self.name,
					self.val,
				))
			return self.val

	def write(self, val):
		with self.lock:
			if self.mp.debug:
				print('thread {} writing {}: {} --> {}'.format(
					threading.current_thread().name,
					self.name,
					self.val,
					val,
				))
			self.val = val

	def tas(self, oldval, newval):
		with self.lock:
			oldval = self.val
			self.val = True
			return oldval

	def cas(self, oldval, newval):
		with self.lock:
			if self.val == oldval:
				self.val = newval
				return True
			else:
				return False

	# *NON*-atomic increment: does a read and a write.
	def inc(self, amt=1):
		self.write(self.read() + amt)

	# *NON*-atomic decrement: does a read and a write.
	def dec(self, amt=1):
		self.write(self.read() - amt)

class MPcondition:
	def __init__(self, name, parent):
		self.name = name
		self.parent = parent
		self.cond = threading.Condition(parent.lock)

	def wait(self):
		if self.parent.mp.debug:
			print('thread {} starts waiting for {}/{}'.format(
				threading.current_thread().name,
				self.parent.name,
				self.name,
			))
		self.cond.wait()
		if self.parent.mp.debug:
			print('thread {} resuming after {}/{}'.format(
				threading.current_thread().name,
				self.parent.name,
				self.name,
			))

	def signal(self):
		if self.parent.mp.debug:
			print('thread {} signaling {}/{}'.format(
				threading.current_thread().name,
				self.parent.name,
				self.name,
			))
		self.cond.notify()

	def broadcast(self):
		if self.parent.mp.debug:
			print('thread {} broadcasting {}/{}'.format(
				threading.current_thread().name,
				self.parent.name,
				self.name,
			))
		self.cond.notifyAll()

class MPlock:
	def __init__(self, mp, name):
		self.mp = mp
		self.name = name
		self.lock = threading.Lock()

	def Condition(self, name):
		return MPcondition(name, self)

	def acquire(self):
		self.lock.acquire()
		if self.mp.debug:
			print('thread {} acquired lock {}'.format(
				threading.current_thread().name,
				self.name,
			))

	def release(self):
		if self.mp.debug:
			print('thread {} releasing lock {}'.format(
				threading.current_thread().name,
				self.name,
			))
		self.lock.release()

	def __enter__(self):
		self.acquire()

	def __exit__(self, type, value, traceback):
		self.release()

class MPsema:
	def __init__(self, mp, name, value):
		self.mp = mp
		self.name = name
		self.sema = threading.Semaphore(value)

	def procure(self):
		self.sema.acquire()
		if self.mp.debug:
			print('thread {} procured {}'.format(
				threading.current_thread().name,
				self.name,
			))

	def vacate(self):
		if self.mp.debug:
			print('thread {} vacating {}'.format(
				threading.current_thread().name,
				self.name,
			))
		self.sema.release()

class MP:
	def __init__(self, seed=None):
		self.debug = False
		self.threads = Queue()

	def Lock(self, name):
		return MPlock(self, name)

	def Semaphore(self, name, value):
		return MPsema(self, name, value)

	def Shared(self, name, val):
		return Shared(self, name, val)

	def Ready(self):
		while not self.threads.empty():
			thr = self.threads.get()
			thr.join()

	def Check(self):
		pass

class MPthread(threading.Thread):
	def __init__(self, mp, name):
		threading.Thread.__init__(self, name=name)
		self.MP = mp
		mp.threads.put(self)

	def delay(self, amt=1):
		time.sleep(amt)

	def run(self):
		if self.MP.debug:
			print('thread {} starting'.format(self.name))
		self.go()
		if self.MP.debug:
			print('thread {} finished'.format(self.name))

	def MP_check(self):
		self.MP.Check()
