/*
 * ui_sys.c - ce que q_shared.c attend de tout module : Com_Error et
 * Com_Printf, redirigés vers le moteur par des traps.
 */
#include "ui_local.h"

void QDECL Com_Error(int level, const char *error, ...)
{
	va_list ap;
	char text[1024];

	(void)level;

	va_start(ap, error);
	Q_vsnprintf(text, sizeof(text), error, ap);
	va_end(ap);

	trap_Error(text);
}

void QDECL Com_Printf(const char *msg, ...)
{
	va_list ap;
	char text[1024];

	va_start(ap, msg);
	Q_vsnprintf(text, sizeof(text), msg, ap);
	va_end(ap);

	trap_Print(text);
}
