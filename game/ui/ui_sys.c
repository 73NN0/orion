#include "../../engine/code/qcommon/q_shared.h"

void
trap_Print(const char *string);
void
trap_Error(const char *string);

void QDECL
Com_Error(int level, const char *error, ...) {
    va_list ap;
    char text[1024];

    (void)level;

    va_start(ap, error);
    Q_vsnprintf(text, sizeof(text), error, ap);
    va_end(ap);

    trap_Error(text);
}

void QDECL
Com_Printf(const char *msg, ...) {
    va_list ap;
    char text[1024];

    va_start(ap, msg);
    Q_vsnprintf(text, sizeof(text), msg, ap);
    va_end(ap);

    trap_Print(text);
}
