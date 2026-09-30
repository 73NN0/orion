/*
 * ui_scene.c - une scène 3D dans l'interface : un modèle, une lumière,
 * une caméra. Même protocole que cgame, sans carte (RDF_NOWORLDMODEL).
 */
#include "ui_local.h"

#define CUBE_DISTANCE	96	/* devant la caméra, sur l'axe x */

void UI_DrawCube(float x, float y, float w, float h)
{
	refdef_t rd;
	refEntity_t ent;
	vec3_t angles, light;
	float d;

	/* la caméra : un rectangle d'écran, une ouverture, une position */
	memset(&rd, 0, sizeof(rd));
	rd.rdflags = RDF_NOWORLDMODEL;
	rd.x = x;
	rd.y = y;
	rd.width = w;
	rd.height = h;
	rd.fov_x = 60;
	d = w / tan(rd.fov_x / 360 * M_PI);	/* même calcul que CG_CalcFov */
	rd.fov_y = atan2(h, d) * 360 / M_PI;
	AxisClear(rd.viewaxis);			/* regarde vers +x, z en haut */
	rd.time = uis.realtime;

	/* l'objet : un modèle, une position, trois axes */
	memset(&ent, 0, sizeof(ent));
	ent.reType = RT_MODEL;
	ent.hModel = uis.cube;
	VectorSet(ent.origin, CUBE_DISTANCE, 0, 0);
	VectorCopy(ent.origin, ent.oldorigin);
	VectorCopy(ent.origin, ent.lightingOrigin);
	ent.renderfx = RF_LIGHTING_ORIGIN | RF_NOSHADOW;
	VectorSet(angles, 25, uis.realtime * 0.05f, 0);	/* 50 degrés/s */
	AnglesToAxis(angles, ent.axis);

	VectorSet(light, CUBE_DISTANCE - 48, 40, 48);

	trap_R_ClearScene();
	trap_R_AddRefEntityToScene(&ent);
	trap_R_AddLightToScene(light, 200, 1, 1, 1);
	trap_R_RenderScene(&rd);
}
