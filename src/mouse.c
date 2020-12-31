/*
 *    TTTTTTTTTTTTTT  HH          HH  OOOOOOOOOOOOOO  MM          MM
 *    TTTTTTTTTTTTTT  HH          HH  OOOOOOOOOOOOOO  MMM        MMM
 *          TT        HH          HH  OO          OO  MMMM      MMMM 
 *          TT        HH          HH  OO          OO  MM MM    MM MM
 *          TT        HH          HH  OO          OO  MM  MM  MM  MM 
 *          TT        HHHHHHHHHHHHHH  OO          OO  MM   MMMM   MM
 *          TT        HHHHHHHHHHHHHH  OO          OO  MM    MM    MM
 *          TT        HH          HH  OO          OO  MM          MM
 *          TT        HH          HH  OO          OO  MM          MM
 *          TT        HH          HH  OO          OO  MM          MM
 *          TT        HH          HH  OOOOOOOOOOOOOO  MM          MM
 *          TT        HH          HH  OOOOOOOOOOOOOO  MM          MM 
 *
 *                      l'émulateur Thomson TO7-70
 *
 *  Copyright (C) 1996 Sylvain Huet, 1999-2002 Eric Botcazou.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */
 
/*
 *  Module     : mouse.c
 *  Version    : 1.5.5
 *  Créé par   : Eric Botcazou 1999
 *  Modifié par: Eric Botcazou 15/07/2000
 *
 *  Gestion du crayon optique du TO7-70.
 */


#include "intern/hardware.h"
#include "intern/mouse.h"  /* MacOS */
#include "to7.h"


static int mouse_x, mouse_y;



/* ResetMouse:
 *  Réinitialise les périphériques de pointage.
 */
void ResetMouse(void)
{
    mc6846.prc&=0xFD;  /* bouton crayon optique relâché */
}



/* GetLightPen:
 *  Lit l'état du crayon optique.
 */
void GetLightpen(int *xr, int *yr, int *cc)
{
    *xr=mouse_x;
    *yr=mouse_y;
    *cc&=0xFE;  /* la lecture est toujours bonne */
}



/**********************************/
/* partie publique                */
/**********************************/


/* HandleMouseMotion:
 *  Prend en compte un mouvement de la souris.
 */
void to7_HandleMouseMotion(int x_pos, int y_pos)
{
    mouse_x=x_pos;
    mouse_y=y_pos;
}

END_OF_FUNCTION(to7_HandleMouseMotion)



/* HandleMouseClick:
 *  Prend en compte un changement d'état des boutons de la souris.
 */
void to7_HandleMouseClick(int button, int release)
{
    if (button==1)  /* crayon optique */
    {
        if (release)
            mc6846.prc&=0xFD;
        else
            mc6846.prc|=2;
    }
}

END_OF_FUNCTION(to7_HandleMouseClick)




/* InitMouse:
 *  Initialise le module souris.
 */
void InitMouse(void)
{
    /* routine GETL crayon optique */
    mem.mon.bank[0][0x1BD3]=0x02;
    mem.mon.bank[0][0x1BD4]=0x39;

    LOCK_VARIABLE(mouse_x);
    LOCK_VARIABLE(mouse_y);
    LOCK_FUNCTION(to7_HandleMouseMotion);
    LOCK_FUNCTION(to7_HandleMouseClick);
}

