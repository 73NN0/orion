struct arena;


enum tokenKind {
	TOKEN_EOF,
	TOKEN_WORD,
	TOKEN_STRING,
	TOKEN_LBRACE,
	TOKEN_RBRACE,
	TOKEN_ERROR,
	TOKEN_COUNT,
};


struct span {
const char *data;
uvlong length;
};

struct token {
	enum tokenKind kind;
	struct span text;
	uvlong line;
	uvlong column;
};

struct lexer {
	const char *input;
	uvlong length;
	uvlong position;
	uvlong line;
	uvlong column;
};

void 
lexerInit(struct lexer * /* lexer */, const char * /* input*/, uvlong /*length*/);

struct token 
lexerNext(struct lexer *);

/*
	count >= 1
	last token => TOKEN_OEF or TOKEN_ERROR
*/
struct tokenList {
	struct token *tokens;
	uvlong count;
};

int lexerTokenize(struct arena *, const char *, uvlong,
    struct tokenList *);
