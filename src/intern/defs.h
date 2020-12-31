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
 *  Module     : defs.h
 *  Version    : 1.5.5
 *  Créé par   : Eric Botcazou 15/12/2000
 *  Modifié par: Eric Botcazou 10/10/2001
 *
 *  Définition des structures internes.
 */


#ifndef DEFS_H
#define DEFS_H

#include "mc68xx/mc6809.h"


#ifdef DJGPP

#ifndef SCAN_DEPEND
   #include <dpmi.h>
#endif

/* from Allegro */
#define END_OF_FUNCTION(x)          void x##_end(void) { }
#define END_OF_STATIC_FUNCTION(x)   static void x##_end(void) { }
#define LOCK_DATA(d, s)             _go32_dpmi_lock_data(d, s)
#define LOCK_CODE(c, s)             _go32_dpmi_lock_code(c, s)
#define LOCK_VARIABLE(x)            LOCK_DATA((void *)&x, sizeof(x))
#define LOCK_FUNCTION(x)            LOCK_CODE(x, (long)x##_end - (long)x)

#else

#define END_OF_FUNCTION(x)
#define END_OF_STATIC_FUNCTION(x)
#define LOCK_DATA(d, s)
#define LOCK_CODE(c, s)
#define LOCK_VARIABLE(x)
#define LOCK_FUNCTION(x)

#endif


#ifndef TRUE
#define TRUE  1
#endif

#ifndef FALSE
#define FALSE  0
#endif


#define TOP_BORDER_LINES    56  /* hauteur de la bordure haute de l'écran   */
#define SCREEN_LINES        200 /* hauteur de la partie centrale de l'écran */
#define BOTTOM_BORDER_LINES 56  /* hauteur de la bordure basse de l'écran   */

#define LEFT_BORDER_CYCLES  12  /* largeur de la bordure gauche de l'écran  */
#define SCREEN_LINE_CYCLES  40  /* largeur de la partie centrale de l'écran */
#define RIGHT_BORDER_CYCLES 12  /* largeur de la bordure droite de l'écran  */


#define FILENAME_LENGTH  128


struct GATE_ARRAY {
    int lp1;
    int lp2;
    int lp3;
    int lp4;
};

struct DISK_CTRL {
    int cmdr;
    int str;
    int tkr;
    int secr;
    int dr;
    int drvr;
};

typedef unsigned char uint8;  /* unité de mémoire */

struct MEMORY {
    struct {
        int nbank;
        int size;
        uint8 *bank[4];
    } cart;
    struct {
        int nbank;
        int size;
        uint8 *bank[8];
    } ram;
    struct {
        int nbank;
        int size;
        uint8 *bank[1];
        char filename_low[1][32];
        char filename_high[1][32];
    } mon;
};

struct MEMORY_PAGER {
    struct {
        int page;  /* page de la cartouche */
        void (*update)(void);
    } cart;
    struct {
        int page;  /* page de la mémoire écran */
        void (*update)(void);
    } screen;
    struct {
        int page;
        void (*update)(void);
    } system;
    struct {
        int page; /* page de RAM de l'espace données */
        void (*update)(void);
    } data;
    struct {
        int page;
        void (*update)(void);
    } mon;
    uint8 *segment[16];  /* 16 segments de 4ko (64ko) adressables */
};

struct MOTHERBOARD {
    mc6809_clock_t exact_clock;
    /* int direct_screen_mode; */
};

#endif

