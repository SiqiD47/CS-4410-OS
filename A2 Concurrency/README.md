Operating Systems Spring 2020
___

A2: Synchronization (part 1)
=====================

Overview
--------

This directory contains 5 Python files with names q01-q05. You **must** use
python 2.7.x to run them correctly. If you do not have python 2.7 on your machine,
it is available on the ugclinux machines. They come preinstalled with 2.7.12.

We highly recommend working with this code in a linux environment (either
a clean VM or the ugclinux machines that you can ssh into).

* q01, q02:
there is a CMS quiz which asks a few multiple choice
questions regarding the code in q01 and q02. You do not need to upload q01
to CMS since no code changes are required. However, you will need to upload
q02 after making a small code change according to the prompt.

* q03, q04, q05:
For each task, modify the corresponding file with an appropriate solution.
Your q03-q05 code will be filtered by an auto-grader which will detect SOME
possible errors in your code. If your code passes the auto-grader, it
will then be inspected by the course staff for correctness.

Make sure your solution makes progress whenever it is feasible to do so,
and does not violate any of the invariants provided within the prompts.

**DUE DATE**: [refer to A2 due date on course website][schedule]  

[schedule]: http://www.cs.cornell.edu/courses/cs4410/2020sp/schedule/

Commandments
------------

All of your answers should follow the
[commandments of synchronization][commandments].

[commandments]: http://www.cs.cornell.edu/courses/cs4410/2012fa/papers/commandments.pdf

**These general rules are very helpful for working with any synchronization problems,
regardless of where you may see them. We highly recommend skimming the pdf.
(~10min read)**

The CS 4410 Synchronization Library
--------------------------------

Instead of using python's built-in synchronization primitives, we are
using the primitives in the provided file [rvr_sync_wrapper.py](rvr_sync_wrapper.py).
This wrapper library facilitates helpful debugging output for you and support for
autograding for us. The documentation for this library is included in
the file [examples/rvr.md](examples/rvr.md).

It is syntactically similar to the sync libraries used in industry and out in the
wild.

**Your code must only use our wrapper library and not the builtin python threading
libraries. Any submissions that don't use our sync library will fail auto-grading
regardless if the logic is correct.**

Important note about file formats
---------------------------------
Files in this assignment have the following format:


*OUR code here*

################################################################################  
DO NOT WRITE OR MODIFY ANY CODE ABOVE THIS LINE #############################  
################################################################################

*YOUR code goes here*

################################################################################  
DO NOT WRITE OR MODIFY ANY CODE BELOW THIS LINE #############################  
################################################################################

*OUR code here*

For auto-grading, we use a parser that looks for these exact 3 lines in order to
figure out where your code starts and stops. If you modify any of these markers,
your code will likely not get extracted properly and may result in failure to pass
the auto-grader.

If you modify any of our code, it will be ignored at the time of auto-grading since
our parser will not extract it.


Updates and Clarifications
--------------------------

Any updates to this assignment will be announced in pinned posts on
Piazza.
