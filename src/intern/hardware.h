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
 *  Module     : hardware.h
 *  Version    : 1.5.5
 *  Créé par   : Eric Botcazou 1999
 *  Modifié par:
 *
 *  Emulation de l'environnement matériel du MC6809E:
 *	- carte mémoire
 *	- circuits d'entrées/sorties du système
 *	- circuits d'entrées/sorties des périphériques
 */


#ifndef HARDWARE_H
#define HARDWARE_H

#include "mc68xx/mc6821.h"
#include "mc68xx/mc6846.h"
#include "intern/defs.h"
#include "to7.h"


#define  LOAD_BYTE(addr)       (int) mempager.segment[((addr)>>12)&0xF][(addr)&0xFFF]
#define  LOAD_WORD(addr)       (LOAD_BYTE(addr)<<8)+LOAD_BYTE(addr+1)
#define STORE_BYTE(addr, val)  mempager.segment[((addr)>>12)&0xF][(addr)&0xFFF]=(uint8) (val)
#define STORE_WORD(addr, val)  STORE_BYTE(addr, val>>8);STORE_BYTE(addr+1, val&0xFF)

extern struct MC6846_PIA mc6846;
extern struct MC6821_PIA pia_int,pia_ext;
extern struct GATE_ARRAY lga;
extern struct MEMORY_PAGER mempager;
extern struct MEMORY mem;
extern struct MOTHERBOARD mb;


static inline void 
DrawGPL(int addr)
{ 
    int pt,col;

    if (addr>=0x1F40)
        return;

    pt =mem.ram.bank[0][addr+0x2000];
    col=mem.ram.bank[0][addr];

    to7_DrawGPL(addr, pt, col);
}

extern int InitHardware(int);

#endif

