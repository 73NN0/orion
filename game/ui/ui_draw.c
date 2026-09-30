/*
 * ui_draw.c - toutes les primitives 2D sont des quads étirés.
 */
#include "ui_local.h"

void UI_FillRect(float x, float y, float w, float h, const vec4_t color)
{
	trap_R_SetColor(color);
	trap_R_DrawStretchPic(x, y, w, h, 0, 0, 1, 1, uis.white);
	trap_R_SetColor(NULL);
}

void UI_DrawPic(float x, float y, float w, float h, qhandle_t shader)
{
	trap_R_DrawStretchPic(x, y, w, h, 0, 0, 1, 1, shader);
}

/*
 * Le charset est une grille de 16 × 16 cases : le glyphe n est à la
 * ligne n >> 4 et à la colonne n & 15. Une case couvre 1/16 = 0.0625
 * de la texture dans chaque direction (même calcul que CG_DrawChar).
 */
void UI_DrawChar(float x, float y, float size, int ch)
{
	float row, col;

	ch &= 255;
	if (ch == ' ')
		return;

	row = (ch >> 4) * 0.0625f;
	col = (ch & 15) * 0.0625f;
	trap_R_DrawStretchPic(x, y, size, size, col, row,
			      col + 0.0625f, row + 0.0625f, uis.charset);
}

void UI_DrawString(float x, float y, float size, const char *s,
		   const vec4_t color)
{
	trap_R_SetColor(color);
	for (; *s; s++, x += size)
		UI_DrawChar(x, y, size, *s);
	trap_R_SetColor(NULL);
}

qboolean UI_PointInRect(float px, float py,
			float x, float y, float w, float h)
{
	return px >= x && py >= y && px < x + w && py < y + h;
}
