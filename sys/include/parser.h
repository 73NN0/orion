/*
 * Requires:
 *     <u.h>
 *     <arena.h>
 */

struct arena;

struct configString {
    char *data;
    uvlong length;
};

enum configOperationKind {
    CONFIG_OPERATION_COPY,
    CONFIG_OPERATION_COPY_TREE,
    CONFIG_OPERATION_USE,
    CONFIG_OPERATION_COUNT,
};

struct configPathOperation {
    struct configString source;
    struct configString destination;
};

struct configUseOperation {
    struct configString templateName;
};

struct configOperation {
    enum configOperationKind kind;

    union {
        struct configPathOperation path; /* COPY, COPY_THREE */
        struct configUseOperation use;   /* USE */
    } data;
};

struct configTemplate {
    struct configString name;
    struct configOperation *operations;
    uvlong operationCount;
};

struct config {
    struct configTemplate *templates;
    uvlong templateCount;
};

struct parseError {
    const char *message;
    uvlong line;
    uvlong column;
};

int
parserParse(struct arena *permanent, struct arena *scratch, const char *text, uvlong length,
    struct config *config, struct parseError *error);
