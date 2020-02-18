/* Author: Robbert van Renesse 2015
 *
 * This module contains the Shall parser.  It returns "lines" that are
 * terminated either by a newline or semicolon token.  Each line is a
 * vector of "elements", which are of the following types:
 *
 *		ELT_ARG:	an argument (essentially just a string)
 *		ELT_REDIR:	an I/O redirection
 *
 * There are three types of I/O redirection: input (<), output, (>),
 * and append (>>).  Redirection can either involve a file name or
 * a file descriptor.
 *
 * Grammar:
 *
 *	program
 *		: element* EOF
 *		| element* [ semicolon | ampersand | newline ] program
 *		;
 *
 *  element
 *		: string
 *		| fd? LT [ fd | string ]		// input redirection
 *		| fd? GT [ fd | string ]		// output redirection
 *		| fd? GT GT string				// append
 *		;
 *
 *	fd
 *		: '{' string '}'
 *		;
 *
 * The module exports two function:
 *
 *	parser_t parser_create(tokenizer_t tokenizer);
 *	element_t *parse_next(parser_t parser);
 *	void element_free(element_t);
 *	void parser_free(parser_t);
 */

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "shall.h"

/* The longest pattern (list of tokens) is:
 *
 *		'{' string '}' '>' '{' string '}'
 */
#define MAX_TOKENS		7

struct parser {
	enum {
		PARSER_NEUTRAL,
		PARSER_ERROR,
		PARSER_EOF
	} state;
	tokenizer_t tokenizer;
	token_t tokens[MAX_TOKENS];
	unsigned int ntokens;
	unsigned int line;
};

void element_free(element_t elt){
	switch (elt->type) {
	case ELEMENT_ARG:
		free(elt->u.arg.string);
		break;
	case ELEMENT_REDIR_FILE_IN:
	case ELEMENT_REDIR_FILE_OUT:
	case ELEMENT_REDIR_FILE_APPEND:
		free(elt->u.redir_file.name);
		break;
	default:
		break;
	}
	free(elt);
}

parser_t parser_create(tokenizer_t tokenizer){
	parser_t parser = (parser_t) calloc(1, sizeof(*parser));
	parser->tokenizer = tokenizer;
	parser->line = 1;
	return parser;
}

static void parser_truncate(parser_t parser){
	unsigned int i;

	for (i = 0; i < parser->ntokens; i++) {
		token_free(parser->tokens[i]);
	}
	parser->ntokens = 0;
}

static element_t element_create(enum element_type et){
	element_t elt = calloc(1, sizeof(*elt));

	elt->type = et;
	return elt;
}

/* See if the current list of tokens is a complete pattern.
 */
static element_t parser_match(parser_t parser){
	assert(parser->ntokens > 0);
	unsigned int offset, fd;
	element_t elt;

	switch (parser->tokens[0]->type) {
	case TOKEN_STRING:
		assert(parser->ntokens == 1);
		elt = element_create(ELEMENT_ARG);
		elt->u.arg.string = parser->tokens[0]->u.string;
		parser->tokens[0]->u.string = 0;
		return elt;
	case TOKEN_LT:
		fd = 0;
		offset = 0;
		break;
	case TOKEN_GT:
		fd = 1;
		offset = 0;
		break;
	case TOKEN_CB_OPEN:
		if (parser->ntokens < 2) {
			return 0;
		}
		if (parser->tokens[1]->type != TOKEN_STRING) {
			fprintf(stderr, "line %u: expected a file descriptor\n", parser->line);
			return element_create(ELEMENT_ERROR);
		}
		fd = atoi(parser->tokens[1]->u.string);
		if (parser->ntokens < 3) {
			return 0;
		}
		if (parser->tokens[2]->type != TOKEN_CB_CLOSE) {
			fprintf(stderr, "line %u: expected a '}'\n", parser->line);
			return element_create(ELEMENT_ERROR);
		}
		if (parser->ntokens < 4) {
			return 0;
		}
		switch (parser->tokens[3]->type) {
		case TOKEN_LT: case TOKEN_GT:
			offset = 3;
			break;
		default:
			fprintf(stderr, "line %u: expected a redirection character\n", parser->line);
			return element_create(ELEMENT_ERROR);
		}
		break;
	default:
		fprintf(stderr, "line %u: unexpected token\n", parser->line);
		return element_create(ELEMENT_ERROR);
	}

	/* We're in the process of matching a redirection.  The last token at
	 * 'offset' is either '<' or '>'.  Expect at least one more token.
	 */
	assert(offset < parser->ntokens);
	if (offset == parser->ntokens - 1) {
		return 0;
	}

	/* Match for '>> file'.
	 */
	if (parser->tokens[offset + 1]->type == TOKEN_GT) {
		if (parser->tokens[offset]->type != TOKEN_GT) {
			fprintf(stderr, "line %u: expected >>\n", parser->line);
			return element_create(ELEMENT_ERROR);
		}
		if (offset == parser->ntokens - 2) {
			return 0;
		}
		if (parser->tokens[offset + 2]->type != TOKEN_STRING) {
			fprintf(stderr, "line %u: expected >> string\n", parser->line);
			return element_create(ELEMENT_ERROR);
		}
		elt = element_create(ELEMENT_REDIR_FILE_APPEND);
		elt->u.redir_file.fd = fd;
		elt->u.redir_file.name = parser->tokens[offset + 2]->u.string;
		parser->tokens[offset + 2]->u.string = 0;
		return elt;
	}

	/* Next should be a file or a fd.
	 */
	switch (parser->tokens[offset + 1]->type) {
	case TOKEN_STRING:
		assert(offset == parser->ntokens - 2);
		elt = element_create(
							parser->tokens[offset]->type == TOKEN_LT
							? ELEMENT_REDIR_FILE_IN
							: ELEMENT_REDIR_FILE_OUT);
		elt->u.redir_file.fd = fd;
		elt->u.redir_file.name = parser->tokens[offset + 1]->u.string;
		parser->tokens[offset + 1]->u.string = 0;
		return elt;
	case TOKEN_CB_OPEN:
		if (parser->ntokens < offset + 3) {
			return 0;
		}
		if (parser->tokens[offset + 2]->type != TOKEN_STRING) {
			fprintf(stderr, "line %u: expected a file descriptor\n", parser->line);
			return element_create(ELEMENT_ERROR);
		}
		if (parser->ntokens < offset + 4) {
			return 0;
		}
		if (parser->tokens[offset + 3]->type != TOKEN_CB_CLOSE) {
			fprintf(stderr, "line %u: expected a '}'\n", parser->line);
			return element_create(ELEMENT_ERROR);
		}
		elt = element_create(
							parser->tokens[offset]->type == TOKEN_LT
							? ELEMENT_REDIR_FD_IN
							: ELEMENT_REDIR_FD_OUT);
		elt->u.redir_fd.fd1 = fd;
		elt->u.redir_fd.fd2 = atoi(parser->tokens[offset + 2]->u.string);
		return elt;
	default:
		fprintf(stderr, "line %u: expected file or fd\n", parser->line);
		return element_create(ELEMENT_ERROR);
	}

	return 0;
}

/* Get the next element.
 */
element_t parser_next(parser_t parser){
	for (;;) {
		token_t token = tokenizer_next(parser->tokenizer);

		switch (parser->state) {
		case PARSER_NEUTRAL:
			switch (token->type) {
			case TOKEN_EOF:
				token_free(token);
				if (parser->ntokens > 0) {
					fprintf(stderr, "Line %d: unrecognized eof\n", parser->line);
					parser_truncate(parser);
					return element_create(ELEMENT_ERROR);
				}
				else {
					return element_create(ELEMENT_EOF);
				}
			case TOKEN_EOLN:
				token_free(token);
				parser->line++;
				if (parser->ntokens == 0) {
					return element_create(ELEMENT_EOLN);
				}
				break;
			case TOKEN_SEMI:
				token_free(token);
				if (parser->ntokens == 0) {
					return element_create(ELEMENT_SEMI);
				}
				else {
					fprintf(stderr, "Line %d: unrecognized semicolon\n", parser->line);
					parser_truncate(parser);
					return element_create(ELEMENT_ERROR);
				}
				break;
			case TOKEN_AMPERSAND:
				token_free(token);
				if (parser->ntokens == 0) {
					return element_create(ELEMENT_BACKGROUND);
				}
				else {
					fprintf(stderr, "Line %d: unrecognized ampersand\n", parser->line);
					parser_truncate(parser);
					return element_create(ELEMENT_ERROR);
				}
				break;
			default:
				assert(parser->ntokens < MAX_TOKENS);
				parser->tokens[parser->ntokens++] = token;
				element_t elt = parser_match(parser);
				if (elt != 0) {
					parser_truncate(parser);
					return elt;
				}
				if (parser->ntokens == MAX_TOKENS) {
					fprintf(stderr, "Line %d: pattern too long %d\n", parser->line, token->type);
					parser_truncate(parser);
					parser->state = PARSER_ERROR;
					return element_create(ELEMENT_ERROR);
				}
			}
			break;
		case PARSER_ERROR:
			if (token->type == TOKEN_EOLN) {
				parser_truncate(parser);
				parser->state = PARSER_NEUTRAL;
			}
			token_free(token);
			break;
		case PARSER_EOF:
			assert(parser->ntokens == 0);
			token_free(token);
			return element_create(ELEMENT_EOF);
		default:
			assert(0);
		}
	}
}

void parser_free(parser_t parser){
	free(parser);
}
