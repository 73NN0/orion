#ifndef ORION_UI_LOCAL_H
#define ORION_UI_LOCAL_H

#include "../../engine/code/qcommon/q_shared.h"
#include "../../engine/code/renderercommon/tr_types.h"
#include "../../engine/code/ui/ui_public.h"

typedef struct {
    glconfig_t glconfig;
    qhandle_t whiteShader;
    int realtime;
    int frametime;
} uiState_t;

extern uiState_t uis;

void
trap_Print(const char *string);
void
trap_Error(const char *string);
void
trap_GetGlconfig(glconfig_t *glconfig);
qhandle_t
trap_R_RegisterShaderNoMip(const char *name);
void
trap_R_SetColor(const float *rgba);
void
trap_R_DrawStretchPic(float x, float y, float w, float h, float s1, float t1, float s2, float t2,
    qhandle_t shader);

void
trap_Key_SetCatcher(int catcher);

int
trap_Key_GetCatcher(void);

#endif