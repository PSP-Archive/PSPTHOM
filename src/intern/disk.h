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
 *  Copyright (C) 1996 Sylvain Huet, 1999-2003 Eric Botcazou.
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
 *  Module     : intern/disk.h
 *  Version    : 1.5.5
 *  Créé par   : Alexandre Pukall mai 1998
 *  Modifié par: Eric Botcazou 03/11/2003
 *
 *  Gestion du format SAP 2.0: lecture et écriture disquette.
 */


#ifndef DISK_H
#define DISK_H

#include "defs.h"


extern struct DISK_CTRL disk_ctrl;

#define DISK_CTRL_SET_FUNC(name)    \
    void disk_ctrl_##name(int val)  \
    {                               \
        disk_ctrl.name = val;       \
    }

static inline DISK_CTRL_SET_FUNC(cmdr)
static inline DISK_CTRL_SET_FUNC(tkr)
static inline DISK_CTRL_SET_FUNC(secr)
static inline DISK_CTRL_SET_FUNC(dr)
static inline DISK_CTRL_SET_FUNC(drvr)


extern void InitDisk(void);
extern void ResetDiskCtrl(int *);
extern void ReadSector(int *);
extern void WriteSector(int *);
extern void FormatDrive(int *);
extern void DiskNop(int *);

#endif
