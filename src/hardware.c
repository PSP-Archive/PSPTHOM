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
 *  Module     : hardware.c
 *  Version    : 1.5.5
 *  Créé par   : Eric Botcazou 1999
 *  Modifié par: Eric Botcazou 03/11/2003
 *
 *  Emulation de l'environnement matériel du MC6809E:
 *	- carte mémoire
 *	- circuits d'entrées/sorties du système
 *	- circuits d'entrées/sorties des périphériques
 */


#ifndef SCAN_DEPEND
   #include <stdio.h>
   #include <string.h>
#endif

#include "mc68xx/mc6809.h"
#include "mc68xx/mc6821.h"
#include "mc68xx/mc6846.h"
#include "intern/disk.h"
#include "intern/errors.h"
#include "intern/hardware.h"
#include "intern/keyboard.h"
#include "intern/k7.h"
#include "intern/mouse.h"
#include "to7.h"


/* les composants matériels de l'émulateur */
struct MC6846_PIA mc6846;      /* PIA 6846 système         */
struct MC6821_PIA pia_int;     /* PIA 6821 système         */
struct MC6821_PIA pia_ext;     /* PIA 6821 musique et jeux */
struct GATE_ARRAY lga;         /* Logic Gate Array         */
struct MEMORY_PAGER mempager;  /* carte mémoire logique    */
struct MEMORY mem;             /* carte mémoire physique   */
struct MOTHERBOARD mb;         /* (pseudo) carte mère      */

static int gamma[16]={0, 100, 127, 147, 163, 179, 191, 203, 215, 223, 231, 239, 243, 247, 251, 255};

static int palette[TO7_NCOLORS]={0, 15, 240, 255, 3840, 3855, 4080, 4095,
                                 1911, 826, 931, 938, 2611, 2618, 3815, 123}; /* palette BGR 12-bit */



/* Fonctions de commutation de l'espace mémoire:
 */
static void update_cart(void)
{
    static uint8 cart_garbage[32];

    if (mem.cart.nbank==0)
        mempager.segment[0x0]=cart_garbage;
    else
    {
        mempager.segment[0x0]=mem.cart.bank[mempager.cart.page];
        mempager.segment[0x1]=mem.cart.bank[mempager.cart.page]+0x1000;
        mempager.segment[0x2]=mem.cart.bank[mempager.cart.page]+0x2000;
        mempager.segment[0x3]=mem.cart.bank[mempager.cart.page]+0x3000;
    }
}


static void update_screen(void)
{
    mempager.segment[0x4]=mem.ram.bank[0]+mempager.screen.page*0x2000;
    mempager.segment[0x5]=mem.ram.bank[0]+mempager.screen.page*0x2000+0x1000;
}


static void update_system(void)
{
    mempager.segment[0x6]=mem.ram.bank[mempager.system.page];
    mempager.segment[0x7]=mem.ram.bank[mempager.system.page]+0x1000;
    mempager.segment[0x8]=mem.ram.bank[mempager.system.page]+0x2000;
    mempager.segment[0x9]=mem.ram.bank[mempager.system.page]+0x3000;
}


static void update_data(void)
{
    mempager.segment[0xA]=mem.ram.bank[mempager.data.page];
    mempager.segment[0xB]=mem.ram.bank[mempager.data.page]+0x1000;
    mempager.segment[0xC]=mem.ram.bank[mempager.data.page]+0x2000;
    mempager.segment[0xD]=mem.ram.bank[mempager.data.page]+0x3000;
}


static void update_mon(void)
{
    mempager.segment[0xE]=mem.mon.bank[mempager.mon.page];
    mempager.segment[0xF]=mem.mon.bank[mempager.mon.page]+0x1000;
}


/* SetDeviceRegister:
 *  Dépose un octet dans le registre du périphérique et
 *  modifie en conséquence son état.
 */
static void SetDeviceRegister(int addr,int val)
{
    switch (addr)
    {
        /* PIA 6846 système */
        case 0xE7C1:
            mc6846_WriteCommand(&mc6846, val);

            if ((mc6846.crc&0x30) == 0x30)
                to7_PutSoundByte(mc6809_clock(), mc6846.crc&8 ? 0 : 0x80);
            break;

        case 0xE7C2:
            mc6846.ddrc=val;
            break;

        case 0xE7C3:
            mc6846_WriteData(&mc6846, val);

            /* bit 0: sélection demi-page VRAM */
            mempager.screen.page = (mc6846.prc&1 ? 1 : 0);
            mempager.screen.update();

            /* bit 3: led MIN clavier */
            SetNoCapsLed(mc6846.prc&8);

            /* bit 2, bits 4-6: couleur du pourtour de l'écran */
            to7_SetBorderColor((mc6846.prc&0x70)>>4|(((~mc6846.prc)&4)<<1));
            break;

        case 0xE7C5:
            mc6846_SetTcr(&mc6846, val);
            break;

        case 0xE7C6:
            mc6846_SetTmsb(&mc6846, val);
            break;

        case 0xE7C7:
            mc6846_SetTlsb(&mc6846, val);
            break;

        /* PIA 6821 système */
        case 0xE7C8:
            mc6821_WriteData(&pia_int.porta, val);
            break;

        case 0xE7C9:
            mc6821_WriteData(&pia_int.portb, val);

            /* bits 0-2 */
            pia_int.porta.idr = key_row[mc6821_ReadPort(&pia_int.portb)&0x07];

            /* bits 3-7 */
            switch (mc6821_ReadPort(&pia_int.portb)&0xF8)
            {
                /* mémoire interne 32ko */
                case 0xF0:  /* DDRB 0x0F */
                    mempager.data.page=2;
                    break;

                case 0xE8:  /* DDRB 0x17 */
                    mempager.data.page=3;
                    break;

                /* extension mémoire 64ko */
                case 0x18:  /* DDRB 0xE7 */
                    mempager.data.page=4;
                    break;

                case 0x98:  /* DDRB 0x67 */
                    mempager.data.page=5;
                    break;

                case 0x58:  /* DDRB 0xA7 */
                    mempager.data.page=6;
                    break;
                    
                case 0xD8:  /* DDRB 0x27 */
                    mempager.data.page=7;
                    break;
            }

            mempager.data.update();
            break;

        case 0xE7CA:
            mc6821_WriteCommand(&pia_int.porta, val);
#ifdef LEP_Motor
            if ((val&0x30) == 0x30)
                LEP_Motor(pia_int.porta.cr&8 ? OFF : ON);
#endif            
            break;

        case 0xE7CB:
            mc6821_WriteCommand(&pia_int.portb, val);
            break;

        /* PIA 6821 externe musique et jeux */
        case 0xE7CC:
            mc6821_WriteData(&pia_ext.porta, val);
            break;

        case 0xE7CD:
            mc6821_WriteData(&pia_ext.portb, val);

            to7_PutSoundByte(mc6809_clock(), (mc6821_ReadPort(&pia_ext.portb)&0x3F)<<2);
            break;

        case 0xE7CE:
            mc6821_WriteCommand(&pia_ext.porta, val);
            break;

        case 0xE7CF:
            mc6821_WriteCommand(&pia_ext.portb, val);
            break;

        /* contrôleur de disquettes */
        case 0xE7D0:
            disk_ctrl_cmdr(val);
            break;

        case 0xE7D1:
            disk_ctrl_tkr(val);
            break;

        case 0xE7D2:
            disk_ctrl_secr(val);
            break;

        case 0xE7D3:
            disk_ctrl_dr(val);
            break;

        case 0xE7D8:
            disk_ctrl_drvr(val);
            break;
    }  /* end of switch */
}

#ifdef DEBUG  /* la fonction n'est pas inlinée */

void DrawGPL(int addr)
{
    int pt,col;

    if (addr>=0x1F40)
        return;

    pt =mem.ram.bank[0][addr+0x2000];
    col=mem.ram.bank[0][addr];

    to7_DrawGPL(addr, pt, col);
}

#endif



/* StoreByte:
 *  Ecrit un octet en mémoire.
 */
static void StoreByte(int addr, int val)
{
    static const int page_mask[]={0, 0x0, 0x1, 0x2, 0x2};

    int msq=addr>>12;

    switch (msq)
    {
        case 0x0: /* espace cartouche */
        case 0x1:
        case 0x2:
        case 0x3:
            if (addr<=0x1FFF)  /* commutation par latchage */
            {
                mempager.cart.page = addr&page_mask[mem.cart.nbank];
                mempager.cart.update();
            }
            break;

        case 0x4: /* espace VRAM */
        case 0x5:
            STORE_BYTE(addr, val);
            DrawGPL(addr&0x1FFF);
            break;

        case 0x6: /* espace RAM1 non commutable */
        case 0x7:
        case 0x8:
        case 0x9:
        case 0xA: /* espace RAM2 */
        case 0xB:
        case 0xC:
        case 0xD:
            STORE_BYTE(addr, val);
            break;

        case 0xE:
        case 0xF:
            if ((0xE7C0<=addr) && (addr<=0xE7FF))
                SetDeviceRegister(addr,val);
            break;
    }
}



/* StoreWord:
 *  Ecrit deux octets en mémoire.
 */
static void StoreWord(int addr, int val)
{
    StoreByte(addr, (val>>8));
    StoreByte(addr+1, val&0xFF);
}



/* LoadByte:
 *  Lit un octet en mémoire.
 */
static int LoadByte(int addr)
{
    if ((0xE7C0<=addr) && (addr<=0xE7FF))
        switch (addr)
        {
            /* PIA 6846 système */
            case 0xE7C0:
            case 0xE7C4:
                return mc6846.csr;

            case 0xE7C1:
                return mc6846.crc;

            case 0xE7C2:
                return mc6846.ddrc;

            case 0xE7C3:
                return mc6846.prc;

            case 0xE7C5:
                return mc6846_tcr(&mc6846);

            case 0xE7C6:
                return mc6846_tmsb(&mc6846);

            case 0xE7C7:
                return mc6846_tlsb(&mc6846);

            /* PIA 6821 système */
            case 0xE7C8:
                return mc6821_ReadData(&pia_int.porta);

            case 0xE7C9:
                return mc6821_ReadData(&pia_int.portb);

            case 0xE7CA:
		return mc6821_ReadCommand(&pia_int.porta);

            case 0xE7CB:
		return mc6821_ReadCommand(&pia_int.portb);

            /* PIA 6821 externe musique et jeux */
            case 0xE7CC:
                return mc6821_ReadData(&pia_ext.porta);

            case 0xE7CD:
                return mc6821_ReadData(&pia_ext.portb);

            case 0xE7CE:
		return mc6821_ReadCommand(&pia_ext.porta);

            case 0xE7CF:
		return mc6821_ReadCommand(&pia_ext.portb);

            /* contrôleur de disquettes (WD 2793) */
            case 0xE7D0:
                return disk_ctrl.str;

            case 0xE7D1:
                return disk_ctrl.tkr;

            case 0xE7D2:
                return disk_ctrl.secr;

            case 0xE7D3:
                return disk_ctrl.dr;

            case 0xE7D4:
            case 0xE7D5:
            case 0xE7D6:
            case 0xE7D7:
                return 0;

            case 0xE7D8:
                return disk_ctrl.drvr;

            /* Logic Gate Array */
            case 0xE7E4:
                return lga.lp1;

            case 0xE7E5:
                return lga.lp2;

            case 0xE7E6:
                return lga.lp3;

            case 0xE7E7:
                return lga.lp4;
        }

    return LOAD_BYTE(addr);
}



/* LoadWord:
 *  Lit deux octets en mémoire.
 */
static int LoadWord(int addr)
{
    if ((0xE7C0<=addr) && (addr<=0xE7FF))
        return (LoadByte(addr)<<8)+LoadByte(addr+1);
    else
        return LOAD_WORD(addr);
}



/* FetchInstr:
 *  Remplit le buffer de fetch du CPU.
 */
static void FetchInstr(int pc, unsigned char fetch_buffer[])
{
    register int i;

    for (i=0; i<MC6809_FETCH_BUFFER_SIZE; i++)
        *(fetch_buffer+i)=LOAD_BYTE(pc+i);
}



/* BiosCall:
 *  Appel aux routines du BIOS de gestion des périphériques.
 */
static int BiosCall(struct MC6809_REGS *regs)
{
    switch (regs->pc)
    {
        case 0xF07D:  /* routine CASS */
            DoK7Stuff(&regs->br, &regs->cc);
	    break;

        case 0xF1F3:
            DoKeyShift(&regs->br, &regs->cc);
            return 0x2A; /* BPL */

        case 0xFBD4:  /* crayon optique */
            GetLightpen(&regs->xr, &regs->yr, &regs->cc);
            break;

 	/* Contrôleur de disquettes */
        case 0xE12F:
            ResetDiskCtrl(&regs->cc);
            break;

        case 0xE17E:
            WriteSector(&regs->cc);
            break;

        case 0xE203:
            ReadSector(&regs->cc);
            break;

        case 0xE348:
            FormatDrive(&regs->cc);
            break;

        case 0xE240:        
        case 0xE30D:
            DiskNop(&regs->cc);
            break;

    } /* end of switch */
 
    return 0x12;  /* NOP */
}



/* InitHardware:
 *  Initialise la carte mémoire de l'émulateur.
 */
int InitHardware(int disk_ctrl)
{
    register int i;

    struct MC6809_INTERFACE interface={ FetchInstr,
                                        LoadByte,
                                        LoadWord,
                                        StoreByte,
                                        StoreWord,
                                        BiosCall };
    mc6809_Init(&interface);
    
    /* définition de la carte mémoire physique */
    /* pas de cartouche */
    mem.cart.nbank = 0;
    mem.cart.size = 0x4000;

    /* 128ko de RAM */
    mem.ram.nbank = 8;
    mem.ram.size = 0x4000;
    for (i=0; i<mem.ram.nbank; i++)
        mem.ram.bank[i] = NULL;

    /* 8ko de ROM moniteur */
    mem.mon.nbank = 1;
    mem.mon.size = 0x2000;
    for (i=0; i<mem.mon.nbank; i++)
        mem.mon.bank[i] = NULL;

    switch (disk_ctrl)
    {
        case TO7_NO_DISK_CTRL:
            mem.mon.filename_low[0][0] = '\0';
            break;

        case TO7_CD90_640:
            strcpy(mem.mon.filename_low[0], "cd90-640.rom");
            break;

        default:
            return ErrorMessage(TO7_UNSUPPORTED_DEVICE, NULL);
    }

    strcpy(mem.mon.filename_high[0], "to770.rom");

    /* définition de la carte mémoire logique */
    mempager.cart.update   = update_cart;
    mempager.screen.update = update_screen;
    mempager.system.update = update_system;
    mempager.data.update   = update_data;
    mempager.mon.update    = update_mon;

    /* carte mère */
    mb.exact_clock = 0;

    LOCK_VARIABLE(mc6809_irq);
    LOCK_VARIABLE(mc6846);
    LOCK_VARIABLE(pia_int);
    LOCK_VARIABLE(pia_ext);
    LOCK_VARIABLE(mempager);
    LOCK_VARIABLE(mem);

    return TO7_OK;
}

void
to7_write_ram(int addr, uint8 value)
{
  int bank_id   = addr >> 14;
  int bank_addr = addr & 0x3FFF;
  mem.ram.bank[bank_id][bank_addr] = value;
}

uint8
to7_read_ram(int addr)
{
  int bank_id   = (addr >> 14   ) & 0x7;
  int bank_addr = (addr & 0x3FFF);
  return mem.ram.bank[bank_id][bank_addr];
}

/**********************************/
/* partie publique                */
/**********************************/


void to7_QueryColor(int color, int *r, int *g, int *b)
{
    if ((color<0) || (color>=TO7_NCOLORS))
        return;

    *r = gamma[ palette[color]&0xF];
    *g = gamma[(palette[color]&0xF0)>>4];
    *b = gamma[(palette[color]&0xF00)>>8];
}

