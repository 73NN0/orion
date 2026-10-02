#include <u.h>
#include <arena.h>
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
atEnd(const struct lexer *lexer) {
    return lexer->position >= lexer->length;
}

static char
peek(const struct lexer *lexer) {
    if (atEnd(lexer)) {
        return '\0';
    }
    return lexer->input[lexer->position];
}

static char
peekNext(const struct lexer *lexer) {
    if (atEnd(lexer) || lexer->length - lexer->position < 2) {
        return '\0';
    }
    return lexer->input[lexer->position + 1];
}

static char
advance(struct lexer *lexer) {
    char character;

    if (atEnd(lexer)) {
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

// static void
// skipSpace(struct lexer *lexer) {
//     while (!atEnd(lexer) && isSpace(peek(lexer))) {
//         advance(lexer);
//     }
// }

static int
isWordEnd(char character) {
    return isSpace(character) || character == '{' || character == '}' || character == '"';
}

static int
startsComment(const struct lexer *lexer) {
    return peek(lexer) == '/' && peekNext(lexer) == '/';
}

static void
skipComment(struct lexer *lexer) {
    /*
	 * Laisser le retour à la ligne être consommé comme un espace.
	 */
    while (!atEnd(lexer) && peek(lexer) != '\n' && peek(lexer) != '\r') {
        advance(lexer);
    }
}

static void
skipTrivia(struct lexer *lexer) {
    for (;;) {
        while (!atEnd(lexer) && isSpace(peek(lexer))) {
            advance(lexer);
        }

        if (startsComment(lexer)) {
            skipComment(lexer);
            continue;
        }

        break;
    }
}

struct token
lexerNext(struct lexer *lexer) {
    struct token token = {0};
    char character;
    uvlong start;
    uvlong contentStart;
    skipTrivia(lexer);

    /*
	* memorize before advance
	*/
    start = lexer->position;

    token.line = lexer->line;
    token.column = lexer->column;
    token.text.data = lexer->input + lexer->position;

    if (atEnd(lexer)) {
        token.kind = TOKEN_EOF;
        token.text.length = 0;
        return token;
    }

    character = advance(lexer);
    token.text.length = 1;

    switch (character) {
    case '{':
        token.kind = TOKEN_LBRACE;
        break;
    case '}':
        token.kind = TOKEN_RBRACE;
        break;
    case '"':
        contentStart = lexer->position;

        token.text.data = lexer->input + contentStart;

        while (!atEnd(lexer) && peek(lexer) != '"' && peek(lexer) != '\n' && peek(lexer) != '\r') {
            advance(lexer);
        }

        token.text.length = lexer->position - contentStart;

        if (atEnd(lexer) || peek(lexer) == '\n' || peek(lexer) == '\r') {
            token.kind = TOKEN_ERROR;
            break;
        }

        /*
	 * Consommer le guillemet fermant sans l'ajouter au span.
	 */
        advance(lexer);
        token.kind = TOKEN_STRING;
        break;

    default:
        token.kind = TOKEN_WORD;
        while (!atEnd(lexer) && !isWordEnd(peek(lexer)) && !startsComment(lexer)) {
            advance(lexer);
        }
        token.text.length = lexer->position - start;
        break;
    }
    return token;
}

static uvlong
countTokens(const char *input, uvlong length) {
    struct lexer lexer;
    struct token token;
    uvlong count;

    count = 0;
    lexerInit(&lexer, input, length);

    for (;;) {
        token = lexerNext(&lexer);
        count++;
        if (token.kind == TOKEN_EOF || token.kind == TOKEN_ERROR) {
            break;
        }
    }

    return count;
}

int
lexerTokenize(struct arena *scratch, const char *input, uvlong length, struct tokenList *list) {
    struct lexer lexer;
    struct token *tokens;
    uvlong mark;
    uvlong count;
    uvlong i;

    list->tokens = nil;
    list->count = 0;

    mark = arenaGetPos(scratch);
    count = countTokens(input, length);

    if (count > (uvlong)-1 / (uvlong)sizeof(*tokens)) {
        goto fail;
    }

    tokens = arenaPush(scratch, count * (uvlong)sizeof(*tokens));
    if (tokens == nil) {
        goto fail;
    }

    lexerInit(&lexer, input, length);

    for (i = 0; i < count; i++) {
        tokens[i] = lexerNext(&lexer);
    }

    list->tokens = tokens;
    list->count = count;

    return 1;

fail:
    arenaSetPos(scratch, mark);
    return 0;
}