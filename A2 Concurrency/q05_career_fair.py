
# (C) CS 4410 Spring 2020 Staff, Cornell University
# All rights reserved

from rvr_sync_wrapper import MP, MPthread
import signal

signal.signal(signal.SIGINT, signal.SIG_DFL)

FACILITY_CAPACITY = 15

#    Complete the implementation of the Admitter monitor below using
#    MP Condition variables.

################################################################################
## DO NOT WRITE OR MODIFY ANY CODE ABOVE THIS LINE #############################
################################################################################

class Admitter(MP):
	""" You are in charge of the admission line into the Cornell Career Fair.

	The career fair is being held in Barton Hall which has a max capacity
	specified by FACILITY_CAPACITY. It is your job to manage admissions into
	the career fair.

	We will assume that the fair is attended by people who fall under 1 of
	4 categories: CS students, Econ students, AEM students, and Recruiters.

	CS students filled up the last career fair, so there is a new rule limiting
	their entry. Econ and AEM students tend to compete for the same jobs and are
	known to cut each other in line. In response, Cornell has decided AEM students
	and Econ students can't be in the career fair at the same time.

	Finally, recruiters sometimes run behind schedule and show up at Barton
	after the event begins. They should be allowed into the facility ahead of
	students.

	More technically, the rules for admission are:

	1) There are no more than FACILITY_CAPACITY people allowed in the Career Fair
		at any given time (total includes both students and recruiters).
	2) A CS student will not be allowed to take a enter if - after doing so -
		more than 60% of the people in the Career Fair are CS majors.
	3) An Econ student will not be allowed to enter if there are any AEM majors
		currently in the Career Fair.
	4) Similarly, an AEM Major will not be allowed to enter if there are any
		Econ majors currently in the Career Fair.
	5) Recruiters are always allowed priority access to the Career Fair, subject
		to condition 1. No student should be allowed to enter the Career Fair if
		a Recruiter is waiting.
	"""

	def __init__(self, n):
		MP.__init__(self)
		self.capacity = self.Shared("capacity of career fair", n)
		self.total = self.Shared("total number of people in career fair", 0)
		self.cs = self.Shared("number of cs students in career fair", 0)
		self.econ = self.Shared("number of econ students in career fair", 0)
		self.aem = self.Shared("number of aem students in career fair", 0)
		self.waiting_recruiter = self.Shared("number of waiting recruiters", 0)

		self.lock = self.Lock("monitor lock")
		self.career_fair_not_full = self.lock.Condition("career fair can accept more people")
		self.cs_le_60 = self.lock.Condition("number of cs students is <= 60%")
		self.no_econ = self.lock.Condition("there is no econ students in career fair")
		self.no_aem = self.lock.Condition("there is no aem students in career fair")
		self.no_waiting_recruiter = self.lock.Condition("there is no waiting recruiters")


	def cs_student_enter(self):
		with self.lock:
			while self.waiting_recruiter.read() > 0 or self.total.read() + 1 > self.capacity.read() or self.cs.read() + 1 > 0.6 * (self.total.read() + 1):
				if self.waiting_recruiter.read() > 0:
					self.no_waiting_recruiter.wait()
				if self.total.read() + 1 > self.capacity.read():
					self.career_fair_not_full.wait()
				if self.cs.read() + 1 > 0.6 * (self.total.read() + 1):
					self.cs_le_60.wait()
			self.cs.inc()
			self.total.inc()


	def cs_student_leave(self):
		with self.lock:
			self.cs.dec()
			self.total.dec()
			self.career_fair_not_full.broadcast()
			if self.cs.read() <= 0.6 * self.total.read():
				self.cs_le_60.broadcast()


	def econ_student_enter(self):
		with self.lock:
			while self.waiting_recruiter.read() > 0 or self.total.read() + 1 > self.capacity.read() or self.aem.read() > 0:
				if self.waiting_recruiter.read() > 0:
					self.no_waiting_recruiter.wait()
				if self.total.read() + 1 > self.capacity.read():
					self.career_fair_not_full.wait()
				if self.aem.read() > 0:
					self.no_aem.wait()
			self.econ.inc()
			self.total.inc()
			if self.cs.read() <= 0.6 * self.total.read():
				self.cs_le_60.broadcast()


	def econ_student_leave(self):
		with self.lock:
			self.total.dec()
			self.econ.dec()
			self.career_fair_not_full.broadcast()
			if self.econ.read() == 0:
				self.no_econ.broadcast()


	def aem_student_enter(self):
		with self.lock:
			while self.waiting_recruiter.read() > 0 or self.total.read() + 1 > self.capacity.read() or self.econ.read() > 0:
				if self.waiting_recruiter.read() > 0:
					self.no_waiting_recruiter.wait()
				if self.total.read() + 1 > self.capacity.read():
					self.career_fair_not_full.wait()
				if self.econ.read() > 0:
					self.no_econ.wait()
			self.aem.inc()
			self.total.inc()
			if self.cs.read() <= 0.6 * self.total.read():
				self.cs_le_60.broadcast()


	def aem_student_leave(self):
		with self.lock:
			self.total.dec()
			self.aem.dec()
			self.career_fair_not_full.broadcast()
			if self.aem.read() == 0:
				self.no_aem.broadcast()


	def recruiter_enter(self):
		with self.lock:
			self.waiting_recruiter.inc()
			while self.total.read() + 1 > self.capacity.read():
				self.career_fair_not_full.wait()
			self.waiting_recruiter.dec()
			self.total.inc()
			if self.waiting_recruiter.read() == 0:
				self.no_waiting_recruiter.broadcast()
			if self.cs.read() <= 0.6 * self.total.read():
				self.cs_le_60.broadcast()


	def recruiter_leave(self):
		with self.lock:
			self.total.dec()
			self.career_fair_not_full.broadcast()


################################################################################
## DO NOT WRITE OR MODIFY ANY CODE BELOW THIS LINE #############################
################################################################################

CS_STUDENT = 0
ECON_STUDENT = 1
AEM_STUDENT = 2
RECRUITER = 3

class Attendee(MPthread):
	def __init__(self, attendee_type, admitter, name):
		MPthread.__init__(self, admitter, name)
		self.attendee_type = attendee_type
		self.admitter = admitter


	def run(self):
		enter_funcs = [
			self.admitter.cs_student_enter,
			self.admitter.econ_student_enter,
			self.admitter.aem_student_enter,
			self.admitter.recruiter_enter
		]
		leave_funcs = [
			self.admitter.cs_student_leave,
			self.admitter.econ_student_leave,
			self.admitter.aem_student_leave,
			self.admitter.recruiter_leave
		]
		names = [
			'CsStudent',
			'EconStudent',
			'AemStudent',
			'Recruiter'
		]

		print('{} trying to enter the career fair'.format(names[self.attendee_type]))
		enter_funcs[self.attendee_type]()
		print('{} has entered the career fair'.format(names[self.attendee_type]))
		self.delay(0.1)
		print('{} leaving the career fair'.format(names[self.attendee_type]))
		leave_funcs[self.attendee_type]()
		print('{} has left'.format(names[self.attendee_type]))


if __name__ == '__main__':

	attendee_counts = [20, 35, 30, 6]
	admitter = Admitter(FACILITY_CAPACITY)

	for attendee_type in [CS_STUDENT, ECON_STUDENT, AEM_STUDENT, RECRUITER]:
		for i in range(attendee_counts[attendee_type]):
			if attendee_type == CS_STUDENT:
				Attendee(attendee_type, admitter, 'CS_STUDENT {}'.format(i)).start()
			elif attendee_type == ECON_STUDENT:
				Attendee(attendee_type, admitter, 'ECON_STUDENT {}'.format(i)).start()
			elif attendee_type == AEM_STUDENT:
				Attendee(attendee_type, admitter, 'AEM_STUDENT {}'.format(i)).start()
			elif attendee_type == RECRUITER:
				Attendee(attendee_type, admitter, 'RECRUITER {}'.format(i)).start()

	admitter.Ready()
