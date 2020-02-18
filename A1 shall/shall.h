/* Author: Robbert van Renesse 2015
 */

typedef struct token *token_t;
typedef struct tokenizer *tokenizer_t;
typedef struct element *element_t;
typedef struct parser *parser_t;
typedef struct reader *reader_t;
typedef struct command *command_t;

/* Tokens produced by the tokenizer.
 */
struct token {
	enum token_type {
		TOKEN_EOF,					// EOF has been reached
		TOKEN_SEMI,					// ;
		TOKEN_EOLN,					// \n
		TOKEN_STRING,				// a sequence of non-special chars
		TOKEN_AMPERSAND,			// &
		TOKEN_GT,					// >
		TOKEN_LT,					// <
		TOKEN_CB_OPEN,				// {
		TOKEN_CB_CLOSE				// }
	} type;
	union {
		char *string;
	} u;
};

/* A command is a list of elements.
 */
struct element {
	enum element_type {
		ELEMENT_ARG,						// arg
		ELEMENT_REDIR_FILE_IN,				// < file
		ELEMENT_REDIR_FILE_OUT,				// > file
		ELEMENT_REDIR_FILE_APPEND,			// >> file
		ELEMENT_REDIR_FD_IN,				// < { fd }
		ELEMENT_REDIR_FD_OUT,				// > { fd }
		ELEMENT_SEMI,						// ;
		ELEMENT_BACKGROUND,					// &
		ELEMENT_EOLN,						// newline
		ELEMENT_ERROR,
		ELEMENT_EOF
	} type;
	union {
		struct {
			char *string;
		} arg;
		struct {
			int fd;
			char *name;
		} redir_file;
		struct {
			int fd1, fd2;
		} redir_fd;
	} u;
};

/* Contains the specifics of a command.  Normal arguments and redirection
 * elements have been split into separate lists.
 */
struct command {
	/* Arguments are collected here.
	 */
	char **argv;
	int argc;		// warning: includes the 0 pointer at the end of argv

	/* Redirections are collected here.
	 */
	element_t *redirs;
	int nredirs;
};

tokenizer_t tokenizer_create(reader_t reader);
token_t tokenizer_next(tokenizer_t);
void token_free(token_t);
reader_t reader_create(int fd);
char reader_next(reader_t reader);
parser_t parser_create(tokenizer_t tokenizer);
element_t parser_next(parser_t parser);
void element_free(element_t elt);
void parser_free(parser_t parser);
void tokenizer_free(tokenizer_t tokenizer);
void reader_free(reader_t reader);
void interpret(reader_t reader, int interactive);

void interrupts_disable();
void interrupts_enable();
void interrupts_catch();
void perform(command_t command, int background);
