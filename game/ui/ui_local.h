/*
 * ui_local.h - l'état privé de l'UI d'Orion.
 */
#ifndef ORION_UI_LOCAL_H
#define ORION_UI_LOCAL_H

#include "../../engine/code/qcommon/q_shared.h"
#include "../../engine/code/renderercommon/tr_types.h"
#include "../../engine/code/ui/ui_public.h"
#include "../../engine/code/client/keycodes.h"

#include "ui_trap.h"

struct ui_state {
	glconfig_t glconfig;
	qhandle_t white;	/* pinceau : blanc × couleur courante */
	qhandle_t charset;	/* grille 16 × 16 de glyphes */
	qhandle_t pulse;	/* matériau animé par son script */
	qhandle_t scroll;	/* idem, défilement de texture */
	qhandle_t cube;		/* models/orion/cube.md3 */
	sfxHandle_t click;
	int realtime;
	int frametime;
	float cursor_x;		/* le moteur n'envoie que des deltas */
	float cursor_y;
	int menu;		/* UIMENU_NONE, UIMENU_MAIN, UIMENU_INGAME */
	int hot;		/* bouton sous le curseur, -1 sinon */
};

extern struct ui_state uis;

/* ui_draw.c */
void UI_FillRect(float x, float y, float w, float h, const vec4_t color);
void UI_DrawPic(float x, float y, float w, float h, qhandle_t shader);
void UI_DrawChar(float x, float y, float size, int ch);
void UI_DrawString(float x, float y, float size, const char *s,
		   const vec4_t color);
qboolean UI_PointInRect(float px, float py,
			float x, float y, float w, float h);

/* ui_scene.c */
void UI_DrawCube(float x, float y, float w, float h);

#endif /* ORION_UI_LOCAL_H */
