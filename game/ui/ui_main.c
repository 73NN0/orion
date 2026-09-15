#include "ui_local.h"

uiState_t uis;

static void
UI_Init(void);
static void
UI_Refresh(int realtime);

Q_EXPORT intptr_t
vmMain(int command, int arg0, int arg1, int arg2, int arg3, int arg4, int arg5, int arg6, int arg7,
    int arg8, int arg9, int arg10, int arg11) {
    switch (command) {
    case UI_GETAPIVERSION:
        return UI_API_VERSION;

    case UI_INIT:
        UI_Init();
        return 0;

    case UI_SHUTDOWN:
        return 0;

    case UI_KEY_EVENT:
    case UI_MOUSE_EVENT:
        return 0;

    case UI_REFRESH:
        UI_Refresh(arg0);
        return 0;

    case UI_IS_FULLSCREEN:
        return qtrue;

    case UI_SET_ACTIVE_MENU:
    case UI_DRAW_CONNECT_SCREEN:
        return 0;

    case UI_CONSOLE_COMMAND:
        return qfalse;

    case UI_HASUNIQUECDKEY:
        return qtrue;
    }

    return -1;
}

static void
UI_FillRect(float x, float y, float w, float h, const vec4_t color) {
    trap_R_SetColor(color);

    trap_R_DrawStretchPic(x, y, w, h, 0, 0, 1, 1, uis.whiteShader);

    trap_R_SetColor(NULL);
}

static void
UI_Init(void) {
    trap_Print("\n \n -----------------------\n \n");
    trap_Print("UI_Init: begin\n");

    trap_GetGlconfig(&uis.glconfig);

    trap_Print("UI_Init: glconfig ok\n");

    uis.whiteShader = trap_R_RegisterShaderNoMip("gfx/orion/white");
    trap_Key_SetCatcher(KEYCATCH_UI);
    Com_Printf("video: %d x %d, shader: %d\n",
        uis.glconfig.vidWidth,
        uis.glconfig.vidHeight,
        uis.whiteShader);
    trap_Print("UI_Init: shader ok\n");
}

static void
UI_Refresh(int realtime) {
    static const vec4_t background = {0.05f, 0.05f, 0.05f, 1.0f};

    static const vec4_t panel = {1.0f, 0.0f, 0.0f, 1.0f};

    uis.frametime = realtime - uis.realtime;
    uis.realtime = realtime;

    UI_FillRect(0, 0, uis.glconfig.vidWidth, uis.glconfig.vidHeight, background);

    UI_FillRect(64, 64, 320, 160, panel);
}