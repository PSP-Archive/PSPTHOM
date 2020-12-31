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
 *                      l'�mulateur Thomson TO7-70
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
 *  Module     : to7dbg.h
 *  Version    : 1.5.5
 *  Cr�� par   : Eric Botcazou octobre 1999
 *  Modifi� par:
 *
 *  D�clarations d'objets priv�s n�cessaires au d�bogage.
 */


#ifndef TO7DBG_H
#define TO7DBG_H

#include "mc68xx/mc6821.h"
#include "mc68xx/mc6846.h"
#include "intern/defs.h"


#define  LOAD_BYTE(addr)       (int) mempager.segment[((addr)>>12)&0xF][(addr)&0xFFF]
#define STORE_BYTE(addr, val)  mempager.segment[((addr)>>12)&0xF][(addr)&0xFFF]=(uint8) (val)

extern struct MC6846_PIA mc6846;
extern struct MC6821_PIA pia_int,pia_ext;
extern struct GATE_ARRAY lga;
extern struct MEMORY_PAGER mempager;
extern struct MEMORY mem;

#endif
