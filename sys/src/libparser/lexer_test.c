#include <u.h>
#include <stdio.h>

#include <lexer.h>

#define NELEM(array) ((uvlong)(sizeof(array) / sizeof((array)[0])))

struct expectedToken {
    enum tokenKind kind;
    const char *text;
    uvlong line;
    uvlong column;
};

static const struct expectedToken expected[] = {
    {TOKEN_WORD, "template", 2, 1},
    {TOKEN_WORD, "minimal", 2, 10},
    {TOKEN_LBRACE, "{", 2, 18},

    {TOKEN_WORD, "copy", 3, 3},
    {TOKEN_WORD, "source", 3, 8},
    {TOKEN_WORD, "destination", 3, 15},

    {TOKEN_WORD, "copy", 4, 3},
    {TOKEN_STRING, "https://example.com/file", 4, 8},
    {TOKEN_WORD, "remote", 4, 35},

    {TOKEN_RBRACE, "}", 5, 1},
    {TOKEN_EOF, "", 6, 1},
};

static int
spanEquals(struct span span, const char *text) {
    uvlong i;

    for (i = 0; i < span.length; i++) {
        if (text[i] == '\0' || span.data[i] != text[i]) {
            return 0;
        }
    }

    return text[span.length] == '\0';
}

static const char *token_names[TOKEN_COUNT] = {
    [TOKEN_EOF] = "EOF",
    [TOKEN_WORD] = "WORD",
    [TOKEN_STRING] = "STRING",
    [TOKEN_LBRACE] = "LBRACE",
    [TOKEN_RBRACE] = "RBRACE",
    [TOKEN_ERROR] = "ERROR",
};

static void
print_token(const struct token *token) {
    uvlong i;

    printf("%-8s %llu:%llu  `", token_names[token->kind], token->line, token->column);

    for (i = 0; i < token->text.length; i++) {
        putchar(token->text.data[i]);
    }

    puts("`");
}

static const char input[] = "// configuration minimale\n"
                            "template minimal { // ouverture\n"
                            "  copy source destination// copie simple\n"
                            "  copy \"https://example.com/file\" remote\n"
                            "} // fin\n";

int
main(void) {
    struct lexer lexer;
    struct token token;
    uvlong i;
    int failed;

    failed = 0;

    lexerInit(&lexer, input, (uvlong)(sizeof(input) - 1));

    for (i = 0; i < NELEM(expected); i++) {
        token = lexerNext(&lexer);

        if (token.kind != expected[i].kind || !spanEquals(token.text, expected[i].text)
            || token.line != expected[i].line || token.column != expected[i].column) {
            fprintf(stderr,
                "token %llu: expected %s `%s` at %llu:%llu\n",
                i,
                token_names[expected[i].kind],
                expected[i].text,
                expected[i].line,
                expected[i].column);

            fprintf(stderr, "token %llu: got      ", i);
            print_token(&token);

            failed = 1;
            break;
        }
    }

    if (failed) {
        return 1;
    }

    puts("lexer: ok");
    return 0;
}
