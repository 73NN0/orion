#include <u.h>
#include "lexer.h"

void
lexerInit(struct lexer *lexer, const char *input, uvlong length) {
    lexer->input = input;
    lexer->length = length;
    lexer->position = 0;
    lexer->line = 1;
    lexer->column = 1;
}

static int
lexerAtEnd(const struct lexer *lexer) {
    return lexer->position >= lexer->length;
}

static char
lexerPeek(const struct lexer *lexer) {
    if (lexerAtEnd(lexer)) {
        return '\0';
    }
    return lexer->input[lexer->position];
}

static char
lexerAdvance(struct lexer *lexer) {
    char character;

    if (lexerAtEnd(lexer)) {
        return '\0';
    }

    character = lexer->input[lexer->position++];

    if (character == '\n') {
        lexer->line++;
        lexer->column = 1;
    } else {
        lexer->column++;
    }

    return character;
}
static int
isSpace(char character) {
    return character == ' ' || character == '\t' || character == '\r' || character == '\n';
}

static void
skipSpace(struct lexer *lexer) {
    while (!lexerAtEnd(lexer) && isSpace(lexerPeek(lexer))) {
        lexerAdvance(lexer);
    }
}

static int
isWordEnd(char character) {
    return isSpace(character) || character == '{' || character == '}' || character == '"';
}

struct token
lexerNext(struct lexer *lexer) {
    struct token token = {0};
    char character;
    uvlong start;
    skipSpace(lexer);

    /*
	* memorize before advance
	*/
    start = lexer->position;

    token.line = lexer->line;
    token.column = lexer->column;
    token.text.data = lexer->input + lexer->position;

    if (lexerAtEnd(lexer)) {
        token.kind = TOKEN_EOF;
        token.text.length = 0;
        return token;
    }

    character = lexerAdvance(lexer);
    token.text.length = 1;

    switch (character) {
    case '{':
        token.kind = TOKEN_LBRACE;
        break;
    case '}':
        token.kind = TOKEN_RBRACE;
        break;
    case '"':
        /*
		 * TOKEN_STRING viendra ensuite.
		 */
        token.kind = TOKEN_ERROR;
        token.text.length = 1;
        break;

    default:
        token.kind = TOKEN_WORD;
        while (!lexerAtEnd(lexer) && !isWordEnd(lexerPeek(lexer))) {
            lexerAdvance(lexer);
        }
        token.text.length = lexer->position - start;
        break;
    }
    return token;
}
