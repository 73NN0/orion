/*
 * ui_main.c - cycle de vie de l'UI : vmMain, entrées, menus, actions.
 *
 * Règle QVM : vmMain doit être la première fonction compilée de ce
 * fichier, lui-même premier de la liste des sources. Les déclarations
 * et les données qui la précèdent ne produisent pas de code.
 */
#include "ui_local.h"

struct ui_state uis;

/* Une action est une ligne de commande, exactement comme un bind. */
struct ui_button {
    const char *label;
    float x, y, w, h;
    const char *command; /* NULL : fermer le menu */
};

static const struct ui_button main_menu[] = {
    {"START", 64, 256, 320, 48, "map orion_test\n"},
    {"QUIT", 64, 320, 320, 48, "quit\n"},
};

static const struct ui_button ingame_menu[] = {
    {"RESUME", 64, 256, 320, 48, NULL},
    {"LEAVE", 64, 320, 320, 48, "disconnect\n"},
};

static void
UI_Init(void);
static void
UI_Refresh(int realtime);
static void
UI_KeyEvent(int key, int down);
static void
UI_MouseEvent(int dx, int dy);
static void
UI_SetActiveMenu(int menu);
static qboolean
UI_IsFullscreen(void);

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
        UI_KeyEvent(arg0, arg1);
        return 0;
    case UI_MOUSE_EVENT:
        UI_MouseEvent(arg0, arg1);
        return 0;
    case UI_REFRESH:
        UI_Refresh(arg0);
        return 0;
    case UI_IS_FULLSCREEN:
        return UI_IsFullscreen();
    case UI_SET_ACTIVE_MENU:
        UI_SetActiveMenu(arg0);
        return 0;
    case UI_CONSOLE_COMMAND:
        return qfalse;
    case UI_DRAW_CONNECT_SCREEN:
        return 0;
    case UI_HASUNIQUECDKEY:
        return qtrue;
    }

    return -1;
}

static const struct ui_button *
UI_Buttons(int *count) {
    switch (uis.menu) {
    case UIMENU_MAIN:
        *count = ARRAY_LEN(main_menu);
        return main_menu;
    case UIMENU_INGAME:
        *count = ARRAY_LEN(ingame_menu);
        return ingame_menu;
    }
    *count = 0;
    return NULL;
}

static void
UI_Init(void) {
    trap_GetGlconfig(&uis.glconfig);

    uis.white = trap_R_RegisterShaderNoMip("gfx/orion/white");
    uis.charset = trap_R_RegisterShaderNoMip("gfx/orion/charset");
    uis.pulse = trap_R_RegisterShaderNoMip("gfx/orion/pulse");
    uis.scroll = trap_R_RegisterShaderNoMip("gfx/orion/scroll");
    uis.cube = trap_R_RegisterModel("models/orion/cube.md3");
    uis.click = trap_S_RegisterSound("sound/orion/click.wav", qfalse);

    uis.cursor_x = uis.glconfig.vidWidth / 2;
    uis.cursor_y = uis.glconfig.vidHeight / 2;
    uis.hot = -1;

    Com_Printf("ui: %dx%d white=%d charset=%d pulse=%d scroll=%d cube=%d\n",
        uis.glconfig.vidWidth,
        uis.glconfig.vidHeight,
        uis.white,
        uis.charset,
        uis.pulse,
        uis.scroll,
        uis.cube);
}

/*
 * Le moteur appelle UI_SET_ACTIVE_MENU : au démarrage et quand on est
 * déconnecté sans catcher (UIMENU_MAIN), sur Échap en jeu (UIMENU_INGAME),
 * et quand une cinématique ou une connexion doit reprendre la main
 * (UIMENU_NONE). Tenir le clavier fait partie de la réponse.
 */
static void
UI_SetActiveMenu(int menu) {
    uis.menu = menu;
    uis.hot = -1;

    if (menu == UIMENU_NONE) {
        trap_Key_SetCatcher(trap_Key_GetCatcher() & ~KEYCATCH_UI);
        trap_Key_ClearStates();
        return;
    }
    trap_Key_SetCatcher(KEYCATCH_UI);
}

/*
 * Plein écran = un menu est ouvert, l'UI tient le clavier, et aucune
 * partie n'est affichée. En jeu, le menu se dessine par-dessus le monde.
 */
static qboolean
UI_IsFullscreen(void) {
    uiClientState_t cs;

    if (uis.menu == UIMENU_NONE)
        return qfalse;
    if (!(trap_Key_GetCatcher() & KEYCATCH_UI))
        return qfalse;

    trap_GetClientState(&cs);
    return cs.connState != CA_ACTIVE;
}

static void
UI_Activate(const struct ui_button *b) {
    trap_S_StartLocalSound(uis.click, CHAN_LOCAL_SOUND);

    if (!b->command) {
        UI_SetActiveMenu(UIMENU_NONE);
        return;
    }
    /* EXEC_APPEND : exécuté au prochain Cbuf_Execute, pas pendant vmMain. */
    trap_Cmd_ExecuteText(EXEC_APPEND, b->command);
}

static void
UI_KeyEvent(int key, int down) {
    const struct ui_button *buttons;
    int count;

    if (!down || (key & K_CHAR_FLAG)) {
        return;
    }

    buttons = UI_Buttons(&count);
    if (!count) {
        return;
    }

    switch (key) {
    case K_MOUSE1:
    case K_ENTER:
    case K_KP_ENTER:
        if (uis.hot >= 0)
            UI_Activate(&buttons[uis.hot]);
        break;
    case K_DOWNARROW:
    case K_TAB:
        uis.hot = (uis.hot + 1) % count;
        break;
    case K_UPARROW:
        uis.hot = (uis.hot + count - 1) % count;
        break;
    case K_ESCAPE:
        if (uis.menu == UIMENU_INGAME)
            UI_SetActiveMenu(UIMENU_NONE);
        break;
    }
}

static void
UI_MouseEvent(int dx, int dy) {
    const struct ui_button *buttons;
    int count, i;

    uis.cursor_x = Com_Clamp(0, uis.glconfig.vidWidth - 1, uis.cursor_x + dx);
    uis.cursor_y = Com_Clamp(0, uis.glconfig.vidHeight - 1, uis.cursor_y + dy);

    buttons = UI_Buttons(&count);
    for (i = 0; i < count; i++) {
        const struct ui_button *b = &buttons[i];

        if (UI_PointInRect(uis.cursor_x, uis.cursor_y, b->x, b->y, b->w, b->h)) {
            uis.hot = i;
            return;
        }
    }
}

static void
UI_Refresh(int realtime) {
    static const vec4_t bg = {0.05f, 0.05f, 0.05f, 1.0f};
    static const vec4_t veil = {0.0f, 0.0f, 0.0f, 0.6f};
    static const vec4_t red = {1.0f, 0.0f, 0.0f, 1.0f};
    static const vec4_t idle = {0.18f, 0.18f, 0.20f, 1.0f};
    static const vec4_t hot = {0.55f, 0.10f, 0.10f, 1.0f};
    static const vec4_t text = {1.0f, 1.0f, 1.0f, 1.0f};
    const struct ui_button *buttons;
    int count, i;

    uis.frametime = realtime - uis.realtime;
    uis.realtime = realtime;

    buttons = UI_Buttons(&count);
    if (!count)
        return;

    if (UI_IsFullscreen())
        UI_FillRect(0, 0, uis.glconfig.vidWidth, uis.glconfig.vidHeight, bg);
    else
        UI_FillRect(0, 0, uis.glconfig.vidWidth, uis.glconfig.vidHeight, veil);

    UI_FillRect(64, 64, 320, 160, red);
    UI_DrawString(96, 128, 32, "ORION", text);
    UI_DrawCube(608, 64, 320, 304); /* le cube rouge, en 3D */

    for (i = 0; i < count; i++) {
        const struct ui_button *b = &buttons[i];

        UI_FillRect(b->x, b->y, b->w, b->h, i == uis.hot ? hot : idle);
        UI_DrawString(b->x + 16, b->y + 12, 24, b->label, text);
    }

    /* deux matériaux animés par leur script, sans une ligne de C */
    UI_DrawPic(416, 64, 160, 160, uis.pulse);
    UI_DrawPic(416, 256, 160, 112, uis.scroll);

    UI_FillRect(uis.cursor_x - 3, uis.cursor_y - 3, 6, 6, text);
}
