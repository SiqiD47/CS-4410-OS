Author: Robbert van Renesse 2015

This directory contains most of the code for a minimal but functional shell
called 'shall' (for "shall we execute some commands?"). All the code associating
with parsing is already included, but you have to write code for forking
processes and redirecting I/O. In addition, you should improve the code for
reading files.

Run `make` to create the executable 'shall', and then run `./shall` to run it.
You can get out of it by hitting `<ctrl>D`.

The shell syntax resembles that of the original Bourne shell or bash:


	cat exec.c
		execute program 'cat' with argument 'exec.c'.

	cat exec.c > exec.out
		execute 'cat exec.c' but write output to file exec.out,
		destroying the original contents of exec.out if it already
		existed.

	case exec.c >> exec.out
		execute 'cat exec.c' but append output to file exec.out.

	cat exec.c {2}> exec.err
		execute 'cat exec.c' but redirect error output (file descriptor 2)
		to file exec.err (for example, in case exec.c does not exist).
		Note that the format is a little different from the standard
		shell, where the command would have been 'cat exec.c 2>exec.err'.

	cat
		execute 'cat' without arguments, which will read from standard
		input until EOF (<ctrl>D).

	cat < exec.c
		execute 'cat' without arguments, but take standard input from
		file exec.c.

	cat exec.c > exec.err {2}>{1}
		write both standard input and standard output to file exec.err.
		In the standard shell, the command is 'cat exec.c > exec.err 2>&1'.

	cat exec.c &
		run 'cat exec.c' in the background (without waiting for it to
		finish).

	cat exec.c; ls -l
		run 'cat exec.c' followed by 'ls -l'.

	cd dir
		change the working directory to directory 'dir'

	source script
		read commands from file script

	exec cat exec.c
		same as 'cat exec.c', but without forking, so the 'shall' is
		replaced with 'cat exec.c' and doesn't return

	exec > exec.out
		this doesn't run anything, but redirects standard output to the
		file exec.out. Further commands that are executed now have their
		standard output redirected to file exec.out.

	exit 3
		exit 'shall' with status 3. If no status is specified, 'shall'
		exits with status 0.

	<ctrl>D
		EOF causes the 'shall' to terminate

***Some important details are as follows:***

- The 'shall' is usually in one of two modes: it is either waiting for input, or
  it is waiting for processes to terminate. It cannot wait for both at the same
  time.

- At any point in time, there is at most one foreground process. If there is a
  foreground process, the 'shall' waits for the foreground process to terminate
  before going back to waiting for input. Only input can cause the 'shall' to
  start processes.

- The 'shall' itself should catch interrupts (<cltr>C) and print a message when
  it happens, such as "got signal 2" (2 is the signal number of interrupts).

- The 'shall' should disable interrupts for commands that run in the background
  (using &). For command that run in the foreground, interrupts should generally
  cause the programs to terminate.

- When running a command in the background, the 'shall' should print its process
  identifier. For example

      process 36877 running in background

- If a command terminates normally with exit status 0, the 'shall' simply prints
  a new "prompt" ('->' in the case of 'shall'). However, if a program returns a
  non-zero status or a program terminates abnormally (say, due to an interrupt),
  the 'shall' should print a message. Examples include:

      process 36689 terminated with signal 2
      process 36889 terminated with status 1

- Also print information about background processes that terminate, even if they
  terminate normally with exit status 0:

      process 36889 terminated with status 0

  **Note that information about terminated background processes only needs to
  be printed if it is discovered while waiting for a foreground process, so it
  may not be printed until well after a background process terminates.**


***There are two projects:***

**Project 1:**
  You have to complete the missing code in file exec.c, delimited by `// BEGIN`
  and `// END` pairs. After this you should have a working 'shall' that can
  execute the examples above. You can build the 'shall' by running `make`
  followed by executing `./shall`. As an example, the code for
  `interrupts_disable()` has already been included.

**Project 2:**
  The 'reader' in file reader.c uses the `read()` system call to read one
  character at a time, which is highly ineffient. Change it so that it reads up
  to 512 characters from the source file at a time. However, you should not
  change the interface of the reader (for example, `reader_next()` should still
  return one character at a time).


***Hints:***

- for the `spawn()` function, you are expected to use the `fork()` and `wait()`
  system calls as well as macros `WIFSIGNALED()` and `WEXITSTATUS()`. You should
  invoke `interrupts_disable()` for processes that run in the background.

- use `wait()` rather than `waitpid()`, as you will want to wait for any process
  terminating, including background processes. There's no need to catch SIGCHLD.
  Note that termination of background processes will only be discovered and
  reported while waiting for a foreground process to terminate.

- to execute a command, first invoke `redir(command)` (which redirects I/O),
  then `execute(command)` (which invokes system call `execv()` to run the
  command).

- to see how to use a system call like `fork()`, run `man -s2 fork` on the
  command line. (Section 2 is the systems call section in the online Linux
  manual.)

- most of the `redir()` function is already written, but you need to fill in the
  code in functions `redir_file()` and `redir_fd()`. All of the `execute()`
  function is already written. If redirection fails, use `_exit(1)` to exit the
  forked process with a failure status of 1. In case of `redir_file()`, don't
  forget to close the original file descriptor returned by `open()`.

- for `cd()`, you are expected to use the C library function `getenv()` to
  obtain the `$HOME` environment variable in case no directory is specified in
  the `cd` command.

- for `source()`, use `open()` and `close()`. Once you have a file descriptor
  `fd`, you can use the following code to invoke the interpreter:

      reader_t reader = reader_create(fd);
      interpret(reader, 0);
      reader_free(reader);

- make sure the implement the remaining interrupt functions as well.

- for debugging, consider using gdb. For finding memory leaks, use valgrind.
  Both are usually available under Linux and easy to install if not. Under
  Mac OS X, the code should work as well but these debugging tools tend to be
  harder to install and make work.
