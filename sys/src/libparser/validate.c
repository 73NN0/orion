#include <u.h>
#include <arena.h>
#include <parser.h>

struct configError {
    const char *message;
    uvlong line;
    uvlong column;
};

struct configValidation {
    struct configString name;
    struct configOperation *operations;
    uvlong operationCount;
};

struct node {
    int visited;
    struct configValidation *validations;
    uvlong validationCount;
}

int
configValidate(struct arena *scratch, const struct config *config, struct configError *error) {
    struct node nodeList[] = {0};

    for (uvlong i = 0; i < config->templateCount; i++) {
        nodeList[i].visited = 1;
        for (uvlong j = 0; j < config->templates[i].operationCount; j++) {
            nodeList[i].validations[j].operations[j] = config->templates->operations[j];
        }
    }
}