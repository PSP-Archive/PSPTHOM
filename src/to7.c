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
 *  Module     : to7.c
 *  Version    : 1.5.5
 *  Créé par   : Eric Botcazou 1999
 *  Modifié par: Eric Botcazou 15/01/2002
 *
 *  Module de pilotage de l'émulateur.
 */


#ifndef SCAN_DEPEND
   #include <stdio.h>
   #include <stdlib.h>
   #include <string.h>
#endif

#include "mc68xx/mc6809.h"
#include "intern/defs.h"
#include "intern/disk.h"
#include "intern/errors.h"
#include "intern/hardware.h"
#include "intern/keyboard.h"
#include "intern/k7.h"
#include "intern/mouse.h"
#include "to7.h"
#include <zlib.h>


/* fonctions importables optionnelles */
int  (*to7_DirectReadSector)(int, int, int, int, unsigned char []) = NULL;
int  (*to7_DirectWriteSector)(int, int, int, int, const unsigned char []) = NULL;
int  (*to7_DirectFormatTrack)(int, int, const unsigned char []) = NULL;

/* variables publiques */
int to7_new_video_params;

/* variables privées */
static int to7_alive = 0;
static char memo7_label[TO7_MEMO7_LABEL_LENGTH+1];
static char memo7_filename[FILENAME_LENGTH+1];



/* LoadFile:
 *  Charge un fichier de taille donnée.
 */
static int LoadFile(char filename[], unsigned char dest[], int size)
{
    FILE *file;

    if ((file=fopen(filename,"rb")) == NULL)
        return ErrorMessage(TO7_CANNOT_FIND_FILE, filename);
        
    fread(dest, sizeof(char), size, file);
    fclose(file);

    return TO7_OK;
}



/* InitMemory:
 *  Initialisation de la carte mémoire et chargement des ROMS.
 */
static int InitMemory(void)
{
    register int i;

    /* 128 ko de RAM */
    for (i=0; i<mem.ram.nbank; i++)
        if ((mem.ram.bank[i] = calloc(mem.ram.size, sizeof(uint8))) == NULL)
            return ErrorMessage(TO7_BAD_ALLOC, NULL);        
    
    /* 8 ko de ROM moniteur */
    for (i=0; i<mem.mon.nbank; i++)
        if ((mem.mon.bank[i] = malloc(mem.mon.size*sizeof(uint8))) == NULL)
            return ErrorMessage(TO7_BAD_ALLOC, NULL);        

    for (i=0; i<mem.mon.nbank; i++)
    {
        if (mem.mon.filename_low[i][0])  /* ROM contrôleur de disquettes? */
        {
            if (LoadFile(mem.mon.filename_low[i], mem.mon.bank[i], 0x800) == TO7_ERROR)
                return TO7_ERROR;
        }

        if (LoadFile(mem.mon.filename_high[i], mem.mon.bank[i]+0x800, mem.mon.size-0x800) == TO7_ERROR)
            return TO7_ERROR;
    }

    LOCK_DATA(mem.ram.bank[0], sizeof(uint8)*mem.ram.size);
    LOCK_DATA(mem.mon.bank[0], sizeof(uint8)*mem.mon.size);

    return TO7_OK;
}



/* DoLines:
 *  Fait tourner le MC6809E en le synchronisant sur le
 *  faisceau vidéo ligne par ligne.
 */
static void DoLines(int nlines, unsigned long long int *exact_clock)
{
    register int i;

    for (i=0; i<nlines; i++)
    {
        /* bordure gauche de la ligne */
        *exact_clock+=LEFT_BORDER_CYCLES;

        mc6809_TimeExec(*exact_clock);

        /* partie centrale de la ligne */
        lga.lp3|=0x40;

        *exact_clock+=SCREEN_LINE_CYCLES;
        mc6809_TimeExec(*exact_clock);

        lga.lp3&=0xBF;

        /* bordure droite de la ligne */
        *exact_clock+=RIGHT_BORDER_CYCLES;
        mc6809_TimeExec(*exact_clock);
    }
}



/**********************************/
/* partie publique                */
/**********************************/


/* LoadMemo7:
 *  Charge une cartouche Mémo7 et extrait son label.
 */
int to7_LoadMemo7(const char filename[])
{
    register int i;
    FILE *file;
    int length, c, nbank, label_offset;

    if ((file=fopen(filename,"rb")) == NULL)
        return ErrorMessage(TO7_CANNOT_OPEN_FILE, NULL);
    
    /* test (empirique) de reconnaissance du format */
    length = 0;

    /* recherche du premier espace */
    do
    {
        c=fgetc(file);

        if ((c == EOF) || (++length>65536))
            goto bad_format;
    }
    while (c != 32);
            
    label_offset = length;
  
    /* on détermine la longueur du fichier, qui doit être
       un multiple de 4096 sans excéder 65536 octets */
    while (fgetc(file) != EOF)
        if (++length>65536)
            goto bad_format;

    if (length%4096)
        goto bad_format;
        
    /* allocation de la mémoire nécessaire */
    nbank = (length-1)/mem.cart.size + 1;

    if (mem.cart.nbank < nbank)
    {
        for (i=mem.cart.nbank; i<nbank; i++)
            if ( (mem.cart.bank[i] = malloc(mem.cart.size*sizeof(char))) == NULL)
                return ErrorMessage(TO7_BAD_ALLOC, NULL);
    }
    else if (mem.cart.nbank > nbank)
    {
        for (i=mem.cart.nbank; i>nbank; i--)
            free(mem.cart.bank[i-1]);
    }

    mem.cart.nbank = nbank;

    /* chargement de la cartouche */
    fseek(file, 0, SEEK_SET);

    for (i=0; i<mem.cart.nbank; i++)
        length = fread(mem.cart.bank[i], sizeof(char), mem.cart.size, file);

    for (i=length; i<mem.cart.size; i++)
        mem.cart.bank[mem.cart.nbank-1][i]=0;
 
    fclose(file);

    /* extraction du label */
    strncpy(memo7_label, mem.cart.bank[0]+label_offset, TO7_MEMO7_LABEL_LENGTH);

    for (i=0; i<TO7_MEMO7_LABEL_LENGTH; i++)
        if ((memo7_label[i]<32) || (memo7_label[i] == 94) || (memo7_label[i]>122))
            memo7_label[i] = 32; 

    strncpy(memo7_filename, filename, FILENAME_LENGTH);
    return TO7_READ_ONLY;

  bad_format:
    fclose(file);
    return ErrorMessage(TO7_BAD_FILE_FORMAT, NULL);
}



/* GetMemo7Label:
 *  Retourne le label extrait de la cartouche Memo7.
 */
const char *to7_GetMemo7Label(void)
{
    return memo7_label;
}



/* GetMemo7Filename:
 *  Retourne le nom du fichier utilisé comme cartouche Memo7.
 */
const char* to7_GetMemo7Filename(void)
{
    return memo7_filename;
}



/* Reset:
 *  Simule un appui sur le bouton reset du TO7.
 */
void 
to7_Reset(void)
{
  mc6809_Reset();
  to7_SetK7Counter(0);

  to7_clear_vram();
}



/* ColdReset:
 *  Simule un dé/rebranchement du TO7-70.
 */
void to7_ColdReset(void)
{
    /* initialisation du PIA 6846 système */
    mc6846_Init(&mc6846, 0, 0x81, 0x7D);
    SetNoCapsLed(0);

    /* initialisation du PIA 6821 système */
    mc6821_Init(&pia_int.porta, 0, 0xFF);

    /* les bits 3-7 sont à 1 en entrée */
    mc6821_Init(&pia_int.portb, 0, 0xFC);

    /* initialisation du PIA 6821 musique et jeux */
    mc6821_Init(&pia_ext.porta, 0xC0, 0xFF);
    mc6821_Init(&pia_ext.portb, 0xC0, 0xCF);

    /* initialisation des pages mémoire */
    mempager.cart.page = 0;
    mempager.cart.update();

    mempager.screen.page = 1;
    mempager.screen.update();

    mempager.system.page = 1;
    mempager.system.update();

    mempager.data.page = 2;
    mempager.data.update();
    
    mempager.mon.page = 0;
    mempager.mon.update();

    /* initialisation du logic gate array */
    memset(&lga, 0, sizeof(struct GATE_ARRAY));

    /* initialisation du contrôleur de disquettes */
    memset(&disk_ctrl, 0, sizeof(struct DISK_CTRL));

    /* checksum application */
    STORE_BYTE(0x60D1, 0);

    /* flag de reset à froid */
    STORE_WORD(0x60FE, 0);

    to7_clear_vram();

    mc6809_Reset();
}



/* DoFrame:
 *  Fait tourner le TO7-70 pendant une trame vidéo.
 */
void 
to7_DoFrame(void)
{
    /* début de la frame vidéo: bordure haute de l'écran */
    DoLines(TOP_BORDER_LINES, &mb.exact_clock);

    /* fenêtre centrale de l'écran */
    lga.lp4|=0x80;

    DoLines(SCREEN_LINES, &mb.exact_clock);

    /* bordure du bas de l'écran et remontée du faisceau */
    lga.lp4&=0x7F;
    DoLines(BOTTOM_BORDER_LINES, &mb.exact_clock);
}



/* InputReset:
 *  Remet à zéro les périphériques d'entrée.
 */
void 
to7_InputReset(int mask, int value)
{
    ResetKeyboard(mask, value);
    ResetMouse();
}



/* Exit:
 *  Arrête l'émulateur et restitue les ressources utilisées.
 *  (il n'est pas nécessaire de l'appeler explicitement, elle est
 *   automatiquement invoquée à la sortie du programme)
 */
void to7_Exit(void)
{
    register int i;

    if (!to7_alive)
        return;

    /* on libère la mémoire */
    for (i=0; i<mem.ram.nbank; i++)
        if (mem.ram.bank[i])
            free(mem.ram.bank[i]);
            
    for (i=0; i<mem.mon.nbank; i++)
        if (mem.mon.bank[i])
            free(mem.mon.bank[i]);

    for (i=0; i<mem.cart.nbank; i++)
        if (mem.cart.bank[i])
            free(mem.cart.bank[i]);

    to7_alive = FALSE;
}



/* Init:
 *  Initialise l'émulateur et réserve les ressources nécessaires.
 */
int to7_Init(int disk_ctrl)
{
    /* on détecte les instances multiples */
    if (to7_alive)
        return ErrorMessage(TO7_MULTIPLE_INIT, NULL);

    if (InitHardware(disk_ctrl) == TO7_ERROR)
        return TO7_ERROR;

    if (InitMemory() == TO7_ERROR)
    {
        to7_Exit();
        return TO7_ERROR;
    }

    InitKeyboard();
    InitMouse();
    InitDisk();
    InitK7();

    to7_alive = TRUE;
    atexit(to7_Exit);

    return TO7_OK;
}

#define STZ_SAVE(A) gzwrite(a_file, &(A), sizeof((A)))
#define STZ_READ(A) gzread(a_file, &(A), sizeof((A)))

int to7_LoadState(const char* filename)
{
  int index;
  MC6809_REGS mc6809_regs;
# if 0
struct MC6846_PIA mc6846;      /* PIA 6846 système         */
struct MC6821_PIA pia_int;     /* PIA 6821 système         */
struct MC6821_PIA pia_ext;     /* PIA 6821 musique et jeux */
struct GATE_ARRAY lga;         /* Logic Gate Array         */
struct MEMORY_PAGER mempager;  /* carte mémoire logique    */
struct MEMORY mem;             /* carte mémoire physique   */
struct MOTHERBOARD mb;         /* (pseudo) carte mère      */
# endif
  gzFile* a_file = gzopen( filename, "r");
  if (a_file) {
    STZ_READ(mc6846.csr);
    STZ_READ(mc6846.crc);
    STZ_READ(mc6846.ddrc);
    STZ_READ(mc6846.prc);
    STZ_READ(mc6846.w_mask);
    STZ_READ(mc6846.tcr);
    STZ_READ(mc6846.tmsb);
    STZ_READ(mc6846.tlsb);
    STZ_READ(mc6846.timer_ratio);
    STZ_READ(mc6846.timeout);
    STZ_READ(pia_int.porta.cr);
    STZ_READ(pia_int.porta.ddr);
    STZ_READ(pia_int.porta.odr);
    STZ_READ(pia_int.porta.idr);
    STZ_READ(pia_int.portb.cr);
    STZ_READ(pia_int.portb.ddr);
    STZ_READ(pia_int.portb.odr);
    STZ_READ(pia_int.portb.idr);
    STZ_READ(pia_ext.porta.cr);
    STZ_READ(pia_ext.porta.ddr);
    STZ_READ(pia_ext.porta.odr);
    STZ_READ(pia_ext.porta.idr);
    STZ_READ(pia_ext.portb.cr);
    STZ_READ(pia_ext.portb.ddr);
    STZ_READ(pia_ext.portb.odr);
    STZ_READ(pia_ext.portb.idr);
    STZ_READ(lga.lp1);
    STZ_READ(lga.lp2);
    STZ_READ(lga.lp3);
    STZ_READ(lga.lp4);
    STZ_READ(mempager.cart.page);
    STZ_READ(mempager.screen.page);
    STZ_READ(mempager.system.page);
    STZ_READ(mempager.data.page);
    STZ_READ(mempager.mon.page);
    STZ_READ(mb);
    STZ_READ(mc6809_regs.cc);
    STZ_READ(mc6809_regs.dp);
    STZ_READ(mc6809_regs.ar);
    STZ_READ(mc6809_regs.br);
    STZ_READ(mc6809_regs.xr);
    STZ_READ(mc6809_regs.yr);
    STZ_READ(mc6809_regs.ur);
    STZ_READ(mc6809_regs.sr);
    STZ_READ(mc6809_regs.pc);
    STZ_READ(mc6809_regs.cpu_clock);
    STZ_READ(mc6809_regs.cpu_timer);
    mc6809_SetRegs(&mc6809_regs, 0xFFFF);
    STZ_READ(mc6809_irq);
    STZ_READ(mem.cart.nbank);
    STZ_READ(mem.cart.size);
    STZ_READ(mem.ram.nbank);
    STZ_READ(mem.ram.size);
    STZ_READ(mem.mon.nbank);
    STZ_READ(mem.mon.size);

    for (index = 0; index < mem.cart.nbank; index++) {
      gzread(a_file, mem.cart.bank[index], mem.cart.size);
    }
    for (index = 0; index < mem.ram.nbank; index++) {
      gzread(a_file, mem.ram.bank[index], mem.ram.size);
    }
    for (index = 0; index < mem.mon.nbank; index++) {
      gzread(a_file, mem.mon.bank[index], mem.mon.size);
    }
    gzread(a_file, to7_vram, sizeof(ushort) * TO7_SCREEN_H * TO7_SCREEN_W);
    gzread(a_file, key_row, sizeof(key_row));
    gzread(a_file, j0_dir, sizeof(j0_dir));
    gzread(a_file, j1_dir, sizeof(j1_dir));

    mempager.cart.update();
    mempager.screen.update();
    mempager.system.update();
    mempager.data.update();
    mempager.mon.update();

    gzclose(a_file);
    return 1;
  }
  return 0;
}

int to7_SaveState(const char* filename)
{
  int index;

  gzFile* a_file = gzopen( filename, "w");
  if (a_file) {

    STZ_SAVE(mc6846.csr);
    STZ_SAVE(mc6846.crc);
    STZ_SAVE(mc6846.ddrc);
    STZ_SAVE(mc6846.prc);
    STZ_SAVE(mc6846.w_mask);
    STZ_SAVE(mc6846.tcr);
    STZ_SAVE(mc6846.tmsb);
    STZ_SAVE(mc6846.tlsb);
    STZ_SAVE(mc6846.timer_ratio);
    STZ_SAVE(mc6846.timeout);
    STZ_SAVE(pia_int.porta.cr);
    STZ_SAVE(pia_int.porta.ddr);
    STZ_SAVE(pia_int.porta.odr);
    STZ_SAVE(pia_int.porta.idr);
    STZ_SAVE(pia_int.portb.cr);
    STZ_SAVE(pia_int.portb.ddr);
    STZ_SAVE(pia_int.portb.odr);
    STZ_SAVE(pia_int.portb.idr);
    STZ_SAVE(pia_ext.porta.cr);
    STZ_SAVE(pia_ext.porta.ddr);
    STZ_SAVE(pia_ext.porta.odr);
    STZ_SAVE(pia_ext.porta.idr);
    STZ_SAVE(pia_ext.portb.cr);
    STZ_SAVE(pia_ext.portb.ddr);
    STZ_SAVE(pia_ext.portb.odr);
    STZ_SAVE(pia_ext.portb.idr);
    STZ_SAVE(lga.lp1);
    STZ_SAVE(lga.lp2);
    STZ_SAVE(lga.lp3);
    STZ_SAVE(lga.lp4);
    STZ_SAVE(mempager.cart.page);
    STZ_SAVE(mempager.screen.page);
    STZ_SAVE(mempager.system.page);
    STZ_SAVE(mempager.data.page);
    STZ_SAVE(mempager.mon.page);
    STZ_SAVE(mb);
    MC6809_REGS mc6809_regs;
    mc6809_GetRegs(&mc6809_regs);
    STZ_SAVE(mc6809_regs.cc);
    STZ_SAVE(mc6809_regs.dp);
    STZ_SAVE(mc6809_regs.ar);
    STZ_SAVE(mc6809_regs.br);
    STZ_SAVE(mc6809_regs.xr);
    STZ_SAVE(mc6809_regs.yr);
    STZ_SAVE(mc6809_regs.ur);
    STZ_SAVE(mc6809_regs.sr);
    STZ_SAVE(mc6809_regs.pc);
    STZ_SAVE(mc6809_regs.cpu_clock);
    STZ_SAVE(mc6809_regs.cpu_timer);
    STZ_SAVE(mc6809_irq);
    STZ_SAVE(mem.cart.nbank);
    STZ_SAVE(mem.cart.size);
    STZ_SAVE(mem.ram.nbank);
    STZ_SAVE(mem.ram.size);
    STZ_SAVE(mem.mon.nbank);
    STZ_SAVE(mem.mon.size);

    for (index = 0; index < mem.cart.nbank; index++) {
      gzwrite(a_file, mem.cart.bank[index], mem.cart.size);
    }
    for (index = 0; index < mem.ram.nbank; index++) {
      gzwrite(a_file, mem.ram.bank[index], mem.ram.size);
    }
    for (index = 0; index < mem.mon.nbank; index++) {
      gzwrite(a_file, mem.mon.bank[index], mem.mon.size);
    }
    gzwrite(a_file, to7_vram, sizeof(ushort) * TO7_SCREEN_H * TO7_SCREEN_W);
    gzwrite(a_file, key_row, sizeof(key_row));
    gzwrite(a_file, j0_dir, sizeof(j0_dir));
    gzwrite(a_file, j1_dir, sizeof(j1_dir));

    gzclose(a_file);
    return 1;
  }
  return 0;
}
