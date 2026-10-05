#include <u.h>
#include <arena.h>
#include "lexer.h"
#include <parser.h>

// linked list

struct opNode {
    struct configOperation op;
    struct opNode *next;
};

struct opList {
    struct opNode *first;
    struct opNode *last;
    uvlong count;
};

struct templateNode {
    struct configTemplate template;
    struct templateNode *next;
};

struct templateList {
    struct templateNode *first;
    struct templateNode *last;
    uvlong count;
};

static int
templateListAppend(struct arena *scratch, struct templateList *list, struct configTemplate tmpl) {
    struct templateNode *node = arenaPushZero(scratch, sizeof(*node));

    if (node == nil) {
        return 0;
    }

    node->template = tmpl;
    if (list->last) {
        // link the previous node to the current one
        list->last->next = node;
    } else {
        list->first = node;
    }
    // set up last of the opList to the current.
    list->last = node;
    list->count++;

    return 1;
}

static int
opListAppend(struct arena *scratch, struct opList *list, struct configOperation op) {
    struct opNode *node = arenaPushZero(scratch, sizeof(*node));

    if (node == nil) {
        return 0;
    }

    node->op = op;
    if (list->last) {
        // link the previous node to the current one
        list->last->next = node;
    } else {
        list->first = node;
    }
    // set up last of the opList to the current.
    list->last = node;
    list->count++;

    return 1;
}

struct parser {
    const struct token *tokens;
    uvlong count;
    uvlong position;
};

static void
parserInit(struct parser *parser, const struct tokenList *tokens) {
    parser->tokens = tokens->tokens;
    parser->count = tokens->count;
    parser->position = 0;
}

static const struct token *
parserPeek(const struct parser *parser) {
    return &parser->tokens[parser->position];
}

static const struct token *
parserAdvance(struct parser *parser) {
    const struct token *token;

    token = parserPeek(parser);

    if (parser->position + 1 < parser->count) {
        parser->position++;
    }

    return token;
}

static int
parserCheck(const struct parser *parser, enum tokenKind kind) {
    return parserPeek(parser)->kind == kind;
}

static int
spanEquals(struct span span, const char *text) {
    for (uvlong i = 0; i < span.length; i++) {
        if (text[i] == '\0' || span.data[i] != text[i]) {
            return 0;
        }
    }
    return text[span.length] == '\0';
}

static int
parserCheckWord(const struct parser *parser, const char *word) {
    const struct token *token;
    token = parserPeek(parser);
    return token->kind == TOKEN_WORD && spanEquals(token->text, word);
}

static int
parserFail(struct parseError *error, const struct token *token, const char *message) {
    if (error->message == nil) {
        error->message = message;
        error->line = token->line;
        error->column = token->column;
    }

    return 0;
}

static const struct token *
parserExpect(struct parser *parser, enum tokenKind kind, struct parseError *error,
    const char *message) {
    const struct token *token;

    token = parserPeek(parser);
    if (token->kind == TOKEN_ERROR) {
        parserFail(error, token, "invalid token");
        return nil;
    }

    if (token->kind != kind) {
        parserFail(error, token, message);
        return nil;
    }

    return parserAdvance(parser);
}

static const struct token *
parserExpectWord(struct parser *parser, const char *word, struct parseError *error,
    const char *message) {
    const struct token *token;

    token = parserPeek(parser);
    if (!parserCheckWord(parser, word)) {
        parserFail(error, token, token->kind == TOKEN_ERROR ? "invalid token" : message);
        return nil;
    }

    return parserAdvance(parser);
}

static const struct token *
parserExpectValue(struct parser *parser, struct parseError *error) {
    const struct token *token;

    token = parserPeek(parser);
    if (token->kind != TOKEN_WORD && token->kind != TOKEN_STRING) {
        parserFail(error,
            token,
            token->kind == TOKEN_ERROR ? "invalid token" : "expected a word or string");
        return nil;
    }

    return parserAdvance(parser);
}

static int
configStringCopy(struct arena *permanent, struct configString *destination, struct span source) {
    char *data;
    uvlong i;

    if (source.length == (uvlong)-1) {
        return 0;
    }

    data = arenaPush(permanent, source.length + 1);
    if (data == nil) {
        return 0;
    }

    for (i = 0; i < source.length; i++) {
        data[i] = source.data[i];
    }

    data[source.length] = '\0';

    destination->data = data;
    destination->length = source.length;

    return 1;
}

static int
parsePathOperation(struct parser *parser, struct arena *permanent, struct arena *scratch,
    struct opList *opList, struct parseError *error, enum configOperationKind kind) {
    const struct token *source;
    const struct token *destination;
    struct configOperation operation = {0};

    source = parserExpectValue(parser, error);
    if (source == nil)
        return 0;
    destination = parserExpectValue(parser, error);
    if (destination == nil)
        return 0;

    operation.kind = kind;

    switch (kind) {
    case CONFIG_OPERATION_COPY:
        if (!configStringCopy(permanent, &operation.data.copy.source, source->text)
            || !configStringCopy(permanent, &operation.data.copy.destination, destination->text))
            return parserFail(error, source, "out of memory");
        break;
    case CONFIG_OPERATION_COPY_TREE:
        if (!configStringCopy(permanent, &operation.data.copyTree.source, source->text)
            || !configStringCopy(permanent,
                &operation.data.copyTree.destination,
                destination->text))
            return parserFail(error, source, "out of memory");
        break;
    default:
        /* contrat violé, pas une erreur de parsing */
        return 0;
    }

    if (!opListAppend(scratch, opList, operation)) {
        return parserFail(error, source, "out of memory");
    }

    return 1;
}

static int
parseUseOperation(struct parser *parser, struct arena *permanent, struct arena *scratch,
    struct opList *opList, struct parseError *error, enum configOperationKind kind) {
    const struct token *name;
    struct configOperation operation;
    operation.kind = kind;

    name = parserExpectValue(parser, error);
    if (name == nil) {
        return 0;
    }

    if (!configStringCopy(permanent, &operation.data.use.templateName, name->text)) {
        return parserFail(error, name, "out of memory");
    }

    if (!opListAppend(scratch, opList, operation)) {
        return parserFail(error, name, "out of memory");
    }

    return 1;
}

enum argKind {
    ARG_VALUE, /* TOKEN_WORD, TOKEN_STRING */
    ARG_IDENT, /* TOKEN_WORD */
};

struct directive {
    const char *keyword;
    enum configOperationKind kind;
    int arity;
    enum argKind args[2];
};

static const struct directive directives[]
    = {{"copy", CONFIG_OPERATION_COPY, 2, {ARG_VALUE, ARG_VALUE}},
        {"copy-tree", CONFIG_OPERATION_COPY_TREE, 2, {ARG_VALUE, ARG_VALUE}},
        {"use", CONFIG_OPERATION_USE, 1, {ARG_IDENT}},
        {nil, 0, 0, {0}}};

static int
parseOperation(struct parser *parser, struct arena *permanent, struct arena *scratch,
    struct opList *opList, struct parseError *error) {
    const struct token *keyword;

    keyword = parserPeek(parser);
    // maybe later loop
    if (parserCheckWord(parser, "copy")) {
        parserAdvance(parser);
        return parsePathOperation(parser, permanent, scratch, opList, error, CONFIG_OPERATION_COPY);
    }
    if (parserCheckWord(parser, "copy-tree")) {
        parserAdvance(parser);
        return parsePathOperation(parser,
            permanent,
            scratch,
            opList,
            error,
            CONFIG_OPERATION_COPY_TREE);
    }
    if (parserCheckWord(parser, "use")) {
        parserAdvance(parser);
        return parseUseOperation(parser, permanent, scratch, opList, error, CONFIG_OPERATION_USE);
    }

    return parserFail(error,
        keyword,
        keyword->kind == TOKEN_ERROR ? "invalid token" : "expected copy, copy-tree, use, or }");
}

static int
arraySize(uvlong count, uvlong elementSize, uvlong *size) {
    if (elementSize != 0 && count > (uvlong)-1 / elementSize) {
        return 0;
    }

    *size = count * elementSize;
    return 1;
}

/*
 * parseTemplate - parse one "template NAME { ops... }" block.
 *
 * On entry, the current token is expected to be the "template" keyword.
 * On success, one configTemplate is appended to tmplist (in scratch),
 * with its name and operations array already materialized in permanent.
 *
 * Memory layout at return (success):
 *
 *     permanent arena:
 *         "minimal\0"                   <- template.name points here
 *         "src\0"  "dst\0"  ...         <- strings copied by parseOperation
 *         [ op0 | op1 | ... | opN-1 ]   <- template.operations points here
 *
 *     scratch arena:
 *         [opNode][opNode]...[opNode]   <- opList nodes, now unreferenced
 *         [templateNode]                <- newly appended by this call,
 *                                          its .template field is a copy
 *                                          of the local 'template' struct
 *
 *     caller stack (parseTemplate frame): gone after return.
 */
static int
parseTemplate(struct parser *parser, struct arena *permanent, struct arena *scratch,
    struct templateList *tmplist, struct parseError *error) {
    const struct token *name;
    struct configTemplate template = {0};

    /*
     * Local linked list, lives on the stack as a head { first, last, count }.
     * The nodes themselves will be pushed into scratch by opListAppend.
     *
     *     stack: opList { first=nil, last=nil, count=0 }
     *     scratch: (nothing yet)
     */
    struct opList opList = {0};

    if (parserExpectWord(parser, "template", error, "expected template declaration") == nil) {
        return 0;
    }

    name = parserExpect(parser, TOKEN_WORD, error, "expected template name");
    if (name == nil) {
        return 0;
    }

    if (parserExpect(parser, TOKEN_LBRACE, error, "expected { after template name") == nil) {
        return 0;
    }

    /*
     * Parse the body. Each parseOperation call appends one node to opList:
     *
     *     after 1st op:  opList.first -> [opNode A] -> nil
     *                                        ^-- opList.last
     *
     *     after 2nd op:  opList.first -> [opNode A] -> [opNode B] -> nil
     *                                                      ^-- opList.last
     *
     *     after Nth op:  opList.first -> [A] -> [B] -> ... -> [N-1] -> nil
     *                                                            ^-- opList.last
     *
     * All opNodes live in scratch. Each node holds a configOperation by value
     * whose configStrings already point into permanent (parseOperation
     * copied the token texts there via configStringCopy).
     */
    while (!parserCheck(parser, TOKEN_RBRACE)) {
        if (parserCheck(parser, TOKEN_EOF)) {
            return parserFail(error, parserPeek(parser), "expected } after template body");
        }

        if (!parseOperation(parser, permanent, scratch, &opList, error)) {
            return 0;
        }
    }
    /* consume the closing '}' */
    parserAdvance(parser);

    /*
     * Copy the template name from the token's span (which points into the
     * source buffer, probably in scratch) into permanent, so it survives.
     *
     *     before:  template.name = { data=nil, length=0 }
     *     after:   template.name = { data=<permanent addr>, length=7 }
     *                                       |
     *                                       v
     *              permanent: "minimal\0"
     */
    if (!configStringCopy(permanent, &template.name, name->text)) {
        return parserFail(error, name, "out of memory");
    }

    /*
     * Flatten the linked list into a contiguous array in permanent.
     *
     *     scratch (before):   opList.first -> [A] -> [B] -> [C] -> nil
     *
     *     permanent (after):  template.operations ---> [ A | B | C ]
     *                                                  (3 configOperation
     *                                                   structs, side by side)
     *
     * An empty template (template foo { }) is legal, so handle count == 0.
     */
    if (opList.count == 0) {
        template.operations = nil;
        template.operationCount = 0;
    } else {
        uvlong size;

        /* Guard against count * sizeof overflowing uvlong (same as arraySize
         * in the old two-pass version). For a small config this is purely
         * defensive, but it costs nothing. */
        if (!arraySize(opList.count, sizeof(struct configOperation), &size)) {
            return parserFail(error, name, "template too large");
        }

        template.operations = arenaPushZero(permanent, size);
        if (template.operations == nil) {
            return parserFail(error, name, "out of memory");
        }

        /*
         * Walk the list and copy each node's op (bit-for-bit) into the array.
         *
         *     i=0:  template.operations[0] = A->op
         *     i=1:  template.operations[1] = B->op
         *     i=2:  template.operations[2] = C->op
         *
         * The configStrings inside each op still point to permanent, so the
         * array is self-contained once the loop finishes. The opNodes
         * themselves are now unreferenced; they'll be reclaimed when the
         * caller rewinds scratch, which won't happen inside parserParse.
         */
        uvlong i = 0;
        for (struct opNode *tmp = opList.first; tmp != nil; tmp = tmp->next) {
            template.operations[i] = tmp->op;
            i++;
        }
        template.operationCount = opList.count;
    }

    /*
     * Append the completed template to the caller's templateList.
     * templateListAppend allocates a templateNode in scratch and copies
     * our local 'template' struct into it.
     *
     *     scratch grows by one templateNode:
     *         tmplist.first -> ... -> [new templateNode] -> nil
     *                                        ^-- tmplist.last
     *
     * The copy carries template.name.data and template.operations as
     * pointers; both still point into permanent, so the node is self-
     * contained.
     */
    if (!templateListAppend(scratch, tmplist, template)) {
        return parserFail(error, name, "out of memory");
    }

    return 1;
}

static int
parseDocument(struct parser *parser, struct arena *permanent, struct arena *scratch,
    struct templateList *tmpList, struct parseError *error) {
    while (!parserCheck(parser, TOKEN_EOF)) {
        if (!parseTemplate(parser, permanent, scratch, tmpList, error)) {
            return 0;
        }
    }

    return 1;
}

int
parserParse(struct arena *permanent, struct arena *scratch, const struct tokenList *tokens,
    struct config *config, struct parseError *error) {
    struct parser parser;
    struct templateList tmpList = {0};
    uvlong mark;

    config->templates = nil;
    config->templateCount = 0;
    error->message = nil;
    error->line = 0;
    error->column = 0;

    if (tokens->tokens[tokens->count - 1].kind != TOKEN_EOF
        && tokens->tokens[tokens->count - 1].kind != TOKEN_ERROR) {
        error->message = "invalid token list";
        return 0;
    }

    mark = arenaGetPos(permanent);
    parserInit(&parser, tokens);

    if (!parseDocument(&parser, permanent, scratch, &tmpList, error)) {
        goto fail;
    }

    /* Empty file is syntactically valid: no templates, no error. */
    if (tmpList.count == 0) {
        return 1;
    }

    uvlong templateSize;
    if (!arraySize(tmpList.count, sizeof(struct configTemplate), &templateSize)) {
        error->message = "configuration too large";
        goto fail;
    }

    config->templates = arenaPushZero(permanent, templateSize);
    if (config->templates == nil) {
        parserFail(error, parserPeek(&parser), "out of memory");
        goto fail;
    }

    uvlong i = 0;
    for (struct templateNode *tmp = tmpList.first; tmp != nil; tmp = tmp->next) {
        config->templates[i] = tmp->template;
        i++;
    }
    config->templateCount = tmpList.count;

    return 1;

fail:
    arenaSetPos(permanent, mark);
    config->templates = nil;
    config->templateCount = 0;
    return 0;
}
