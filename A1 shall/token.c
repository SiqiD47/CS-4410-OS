/* Author: Robbert van Renesse 2015
 *
 * The tokenizer divides the input into a stream of tokens:
 *
 *	- spaces, tabs, carriage returns, and null characters separate tokens
 *	  and are otherwise ignored
 *	- special characters, each a token are:
 *		newline (\n), ';', '<', '>', '&', '{', '}',
 *	- any contiguous sequence of remaining characters are returned as
 *		string tokens
 *	- the EOF token is returned (indefinitely) once the end-of-file
 *		has been reached.
 *
 * There are two forms of escaping.  Any character can be preceded by
 * a backslash to treat it as a string token character.  Also, a sequence
 * of characters can be surrounded by single or double quotes to escape
 * all those characters.
 *
 * The interface is as follows:
 *	tokenizer_t tokenizer_create(char (*getc)(void *env), void *env):
 *		Create a tokenizer that reads characters using the provided
 *		getc(env) function.
 *
 *	token_t tokenizer_next(tokenizer_t tokenizer):
 *		Return the next token.
 *
 *	void token_free(token_t token):
 *		Release the memory allocated for a token.
 */

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "shall.h"

/* State of tokenizer.
 */
struct tokenizer {
	reader_t reader;
	enum {
		TOKENIZER_NEUTRAL,		// normal state: awaiting more input
		TOKENIZER_BUFFERED,		// character buffered for future processing
		TOKENIZER_ESC,			// after reading backslash
		TOKENIZER_SQ_STRING,	// in single quated string
		TOKENIZER_DQ_STRING,	// in double quated string
		TOKENIZER_EOF			// EOF reached
	} state;
	char buffered;				// buffered character
	char *string;				// string being read
	unsigned int strlen;		// size of string
};

/* Release a token.
 */
void token_free(token_t token){
	if (token->u.string != 0) {
		free(token->u.string);
	}
	free(token);
}

/* Allocate a fresh tokenizer, which reads tokens from standard input.
 */
tokenizer_t tokenizer_create(reader_t reader){
	tokenizer_t tokenizer;

	tokenizer = (tokenizer_t) calloc(1, sizeof(*tokenizer));
	tokenizer->reader = reader;
	return tokenizer;
}

/* Append a character to the tokenizer string.
 */
static void tokenizer_append(struct tokenizer *tokenizer, char c){
	tokenizer->string = realloc(tokenizer->string, tokenizer->strlen + 1);
	tokenizer->string[tokenizer->strlen++] = c;
}

/* Return a null-terminated string token.
 */
static token_t tokenizer_string(struct tokenizer *tokenizer){
	tokenizer_append(tokenizer, 0);
	token_t token = calloc(1, sizeof(*token));
	token->type = TOKEN_STRING;
	token->u.string = tokenizer->string;
	tokenizer->string = 0;
	tokenizer->strlen = 0;
	tokenizer->state = TOKENIZER_NEUTRAL;
	return token;
}

/* Return the given token type if there are no characters buffered.  Otherwise
 * return the string and buffer the character for future processing.
 */
static token_t tokenizer_buffer(struct tokenizer *tokenizer, char c, enum token_type tt){
	if (tokenizer->string == 0) {
		token_t token = calloc(1, sizeof(*token));
		token->type = tt;
		return token;
	}
	else {
		token_t token = tokenizer_string(tokenizer);
		tokenizer->state = TOKENIZER_BUFFERED;
		tokenizer->buffered = c;
		return token;
	}
}

/* EOF has been reached.  If there is a string buffered, return that
 * first.  Otherwise return an EOF token.
 */
static token_t tokenizer_eof(struct tokenizer *tokenizer){
	assert(tokenizer->state != TOKENIZER_EOF);
	tokenizer->state = TOKENIZER_EOF;
	if (tokenizer->string == 0) {
		token_t token = calloc(1, sizeof(*token));
		token->type = TOKEN_EOF;
		return token;
	}
	else {
		return tokenizer_string(tokenizer);
	}
}

/* Get the next token from the tokenizer.  Tokens should be released
 * with token_free().
 */
token_t tokenizer_next(tokenizer_t tokenizer){
	if (tokenizer->state == TOKENIZER_EOF) {
		token_t token = calloc(1, sizeof(*token));
		token->type = TOKEN_EOF;
		return token;
	}

	for (;;) {
		char c;

		if (tokenizer->state == TOKENIZER_BUFFERED) {
			c = tokenizer->buffered;
			tokenizer->state = TOKENIZER_NEUTRAL;
		}
		else {
			c = reader_next(tokenizer->reader);
		}

		switch (tokenizer->state) {
		case TOKENIZER_EOF:
			assert(0);
		case TOKENIZER_NEUTRAL:
			switch (c) {
			case EOF:
				return tokenizer_eof(tokenizer);
				break;
			case '<':
				return tokenizer_buffer(tokenizer, c, TOKEN_LT);
			case '>':
				return tokenizer_buffer(tokenizer, c, TOKEN_GT);
			case '&':
				return tokenizer_buffer(tokenizer, c, TOKEN_AMPERSAND);
			case '{':
				return tokenizer_buffer(tokenizer, c, TOKEN_CB_OPEN);
			case '}':
				return tokenizer_buffer(tokenizer, c, TOKEN_CB_CLOSE);
			case ';':
				return tokenizer_buffer(tokenizer, c, TOKEN_SEMI);
			case '\n':
				return tokenizer_buffer(tokenizer, c, TOKEN_EOLN);
			case ' ': case '\t': case '\r': case 0:
				if (tokenizer->string != 0) {
					return tokenizer_string(tokenizer);
				}
				break;
			case '\\':
				tokenizer->state = TOKENIZER_ESC;
				break;
			case '\'':
				tokenizer->state = TOKENIZER_SQ_STRING;
				break;
			case '"':
				tokenizer->state = TOKENIZER_DQ_STRING;
				break;
			default:
				tokenizer_append(tokenizer, c);
			}
			break;
		case TOKENIZER_ESC:
			if (c == EOF) {
				return tokenizer_eof(tokenizer);
			}
			else {
				tokenizer->state = TOKENIZER_NEUTRAL;
				tokenizer_append(tokenizer, c);
			}
			break;
		case TOKENIZER_SQ_STRING:
			switch (c) {
			case EOF:
				return tokenizer_eof(tokenizer);
			case '\'':
				tokenizer->state = TOKENIZER_NEUTRAL;
				break;
			default:
				tokenizer_append(tokenizer, c);
			}
			break;
		case TOKENIZER_DQ_STRING:
			switch (c) {
			case EOF:
				return tokenizer_eof(tokenizer);
			case '"':
				tokenizer->state = TOKENIZER_NEUTRAL;
				break;
			default:
				tokenizer_append(tokenizer, c);
			}
			break;
		default:
			fprintf(stderr, "tokenizer state = %d\n", tokenizer->state);
			assert(0);
		}
	}
}

void tokenizer_free(tokenizer_t tokenizer){
	free(tokenizer);
}
