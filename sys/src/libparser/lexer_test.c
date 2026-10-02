#include <u.h>
#include <stdio.h>

#include <lexer.h>

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

int
main(void) {
    const char input[] = "template minimal {\n"
                         "  copy-tree reach/models/players/sarge "
                         "models/players/sarge\n"
                         "}\n";

    struct lexer lexer;
    struct token token;

    lexerInit(&lexer, input, (uvlong)(sizeof(input) - 1));

    for (;;) {
        token = lexerNext(&lexer);
        print_token(&token);

        if (token.kind == TOKEN_EOF || token.kind == TOKEN_ERROR) {
            break;
        }
    }

    return token.kind == TOKEN_ERROR;
}
