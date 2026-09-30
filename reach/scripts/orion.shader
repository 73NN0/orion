// orion.shader - matériaux 2D animés du banc d'essai.
// Le nom d'un bloc est le nom demandé à trap_R_RegisterShader*, sans
// extension. Un bloc de script gagne toujours sur une image du même nom.
//
// Attention : un matériau écrit en script garde le test de profondeur,
// alors qu'un matériau implicite 2D (une image seule) le désactive.
// Pour les aplats d'interface, une image suffit : voir white.tga.

// Une couleur qui respire : rgbGen wave remplace trap_R_SetColor.
// wave <forme> <base> <amplitude> <phase> <fréquence en Hz>
gfx/orion/pulse
{
	nopicmip
	{
		map $whiteimage
		rgbGen wave sin 0.5 0.5 0 0.5
	}
}

// Une texture qui défile : tcMod scroll <s par seconde> <t par seconde>.
gfx/orion/scroll
{
	nopicmip
	{
		map gfx/orion/stripes.tga
		tcMod scroll 0.5 0
	}
}
