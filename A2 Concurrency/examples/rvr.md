## Introduction
The 4410 Synchronization Library provides thin wrappers around the
python Semaphore, Lock, amd Condition Variable classes.  It provides
some helpful debug output, and makes grading submissions easier.  You
are required to use this library for A2.

## How to use the library

To use the library, import the MP and MPthread modules.

    from rvr import MP, MPthread

Monitors and other classes to be used for sharing data between
threads should extend the MP class:

    class MPExample(MP):
		def __init__(self):
			MP.__init__(self, None)
			...

You may find it helpful to turn debugging on.  You can do this by setting the
`debug` variable to true:
    
    class MPExample(MP):
		def __init__(self):
			MP.__init__(self, None)
			self.debug = True
			...

### Initializing Shared Variables
To create shared mutable variables, use Shared from the MP class. The
Shared class is a wrapper over the `int` primitive type. If you would
like to receive valuable feedback from the autograder, make sure to
pass in an identifier for the Shared variable that makes sense in the
first parameter of the constructor.

	class MPExample(MP):
		def __init__(self):
			MP.__init__(self, None)
			self.myVariable = self.Shared("myVariable", INITIAL_VALUE)

Shared variables should be updated by using the `read` and `write`
methods. Using these wrapper methods again provides valuable feedback
to your code as it traces the execution of the reads and writes of
your Shared variables.

### Mutating Shared Variables
Incrementing

    self.myVariable.inc()

Decrementing
    
    self.myVariable.dec()
    
Increasing or decreasing by more than 1

    self.myVariable.write(self.myVariable.read() + 5)
    self.myVariable.write(self.myVariable.read() - 5)
    
Setting a value

    self.myVariable.write(5)
    
### Using Locks and Condition Variables
See [the attached code](rvr-monitor-example.py)

Python does not have built-in support for monitors.  Instead, monitor
code must explicitly create a monitor lock, and create condition
variables that refer to that lock:

	class MonitorExample(MP):
		def __init__(self):
			MP.__init__(self, None)
			self.value = self.Shared("value", 0)
			self.lock  = self.Lock("monitor lock")
			self.gt0   = self.lock.Condition("value greater than 0")
			self.lt2   = self.lock.Condition("value less than 2")


Monitor methods should all acquire the monitor lock using the `with`
construct.  For example, using the 4410 Synchronization Library:

		def get_value(self):
			with self.lock:
				return self.value.read()

To block on a condition variable, call `wait`:

		def block_until_pos(self):
			with self.lock:
				while not (self.value.read() > 0):
					self.gt0.wait()

To notify a single blocked thread, call `signal`.  To notify all
blocked threads, call `broadcast`.  In other libraries, `signal` and
`broadcast` are sometimes called `notify` and `notifyAll`:

		def update(self, value):
			with self.lock:
				self.value.write(value)
				if self.value.read() > 0:
					self.gt0.signal()
				if self.value.read() < 2:
					self.lt2.broadcast()

### Using Semaphores

See [the attached code](rvr-semaphore-example.py).

To create a semaphore, use the `Semaphore` method of the `MP` class:

	class SemaphoreExample(MP):
		def __init__(self):
			MP.__init__(self, None)
			self.value     = self.Shared("value", 0)
			self.valueLock = self.Semaphore("value lock", 1)
			
The `P` and `V` methods are called `procure` and `vacate`:

		def update(self, newValue):
			self.valueLock.procure()
			self.value.write(newValue)
			self.valueLock.vacate()
