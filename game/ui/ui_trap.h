/*
 * ui_trap.h - les services du moteur que cette UI utilise, et rien d'autre.
 *
 * Chaque prototype correspond à un « equ trap_X -N » de ui_syscalls.asm
 * (QVM) et à un « case UI_X: » de CL_UISystemCalls (client/cl_ui.c).
 */
#ifndef ORION_UI_TRAP_H
#define ORION_UI_TRAP_H

void trap_Print(const char *string);
void trap_Error(const char *string);

void trap_Cmd_ExecuteText(int exec_when, const char *text);

void trap_GetGlconfig(glconfig_t *glconfig);
void trap_GetClientState(uiClientState_t *state);

qhandle_t trap_R_RegisterShaderNoMip(const char *name);
void trap_R_SetColor(const float *rgba);
void trap_R_DrawStretchPic(float x, float y, float w, float h,
			   float s1, float t1, float s2, float t2,
			   qhandle_t shader);

qhandle_t trap_R_RegisterModel(const char *name);
void trap_R_ClearScene(void);
void trap_R_AddRefEntityToScene(const refEntity_t *re);
void trap_R_AddLightToScene(const vec3_t org, float intensity,
			    float r, float g, float b);
void trap_R_RenderScene(const refdef_t *fd);

sfxHandle_t trap_S_RegisterSound(const char *sample, qboolean compressed);
void trap_S_StartLocalSound(sfxHandle_t sfx, int channel);

void trap_Key_SetCatcher(int catcher);
int trap_Key_GetCatcher(void);
void trap_Key_ClearStates(void);

#endif /* ORION_UI_TRAP_H */
