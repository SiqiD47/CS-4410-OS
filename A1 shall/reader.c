/* Author: Robbert van Renesse 2018
 *
 * The interface is as follows:
 *	reader_t reader_create(int fd);
 *		Create a reader that reads characters from the given file descriptor.
 *
 *	char reader_next(reader_t reader):
 *		Return the next character or -1 upon EOF (or error...)
 *
 *	void reader_free(reader_t reader):
 *		Release any memory allocated.
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <assert.h>
#include "shall.h"

struct reader {
	int fd;
	char buffer[512];	// hold up to 512 characters
	int idx;
};

reader_t reader_create(int fd){
	reader_t reader = (reader_t) calloc(1, sizeof(*reader));
	reader->fd = fd;
	reader->idx = 512;
	return reader;
}

char reader_next(reader_t reader){
	char c;
	if (reader->idx == 512) {	// buffer is full
		int n = read(reader->fd, &(reader->buffer), 512);
		if (n <= 0) {	// nothing was read
			reader->idx = -1; // re-initialize
			int n = read(reader->fd, &c, 1);
			return n == 1 ? c : EOF;
		}
		else {
			reader->idx = 1;
			return reader->buffer[0];
		}
	}
	else if (reader->idx == -1) {	// buffer is empty, read one character
		int n = read(reader->fd, &c, 1);
		return n == 1 ? c : EOF;
	}
	else {	// 0 <= idx < 512
		c = reader->buffer[reader->idx];
		if (c == EOF) reader->idx = -1;	// re-initialize if EOF
		reader->idx = reader->idx + 1;
		return c;
	}	
}

void reader_free(reader_t reader){
	free(reader);
}
