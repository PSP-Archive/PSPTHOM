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
 *  Module     : to7.h
 *  Version    : 1.5.5
 *  Créé par   : Eric Botcazou 1999
 *  Modifié par: Eric Botcazou 27/12/2003
 *
 *  Module de pilotage de l'émulateur.
 */


#ifndef TO7_H
#define TO7_H

#define TO7_VERSION_STR "1.5.5"

#define TO7_FRAME_FREQ         50    /* Hz: fréquence de trame vidéo */
#define TO7_CYCLES_PER_FRAME   19968 /* durée exacte en nb de cycles CPU d'une frame */
#define TO7_CPU_FREQ           (TO7_FRAME_FREQ*TO7_CYCLES_PER_FRAME)  /* Hz */
#define TO7_NCOLORS            16    /* nombre de couleurs du TO7-70 */

# if 0 //LUDO:
#define TO7_WINDOW_W           320   /* largeur de la fenêtre de travail */     
#define TO7_BORDER_W           16    /* largeur de la bordure de l'écran */
#define TO7_SCREEN_W           (TO7_WINDOW_W + TO7_BORDER_W*2)  /* largeur de l'écran */
#define TO7_WINDOW_H           200   /* hauteur de la fenêtre de travail */
#define TO7_BORDER_H           16    /* hauteur de la bordure de l'écran */
#define TO7_SCREEN_H           (TO7_WINDOW_H + TO7_BORDER_H*2)  /* hauteur de l'écran */
# else
#define TO7_WINDOW_W           320   /* largeur de la fenêtre de travail */     
#define TO7_SCREEN_W           (TO7_WINDOW_W)  /* largeur de l'écran */
#define TO7_WINDOW_H           200   /* hauteur de la fenêtre de travail */
#define TO7_SCREEN_H           (TO7_WINDOW_H)  /* hauteur de l'écran */
#define SNAP_WIDTH             (TO7_SCREEN_W/2)
#define SNAP_HEIGHT            (TO7_SCREEN_H/2)
# endif

#define TO7_GPL_SIZE           8     /* taille d'un Groupe Point Ligne */
#define TO7_WINDOW_GW          (TO7_WINDOW_W/TO7_GPL_SIZE)  /* largeur de la fenêtre de travail en GPL */

#define TO7_CHAR_SIZE          8     /* taille d'un caractère */
#define TO7_WINDOW_CW          (TO7_WINDOW_W/TO7_CHAR_SIZE)  /* largeur de la fenêtre de travail en CHAR */
#define TO7_BORDER_CW          (TO7_BORDER_W/TO7_CHAR_SIZE)  /* largeur de la bordure de l'écran en CHAR */
#define TO7_SCREEN_CW          (TO7_SCREEN_W/TO7_CHAR_SIZE)  /* largeur de l'écran en CHAR */
#define TO7_WINDOW_CH          (TO7_WINDOW_H/TO7_CHAR_SIZE)  /* hauteur de la fenêtre de travail en CHAR */
#define TO7_BORDER_CH          (TO7_BORDER_H/TO7_CHAR_SIZE)  /* hauteur de la bordure de l'écran en CHAR */
#define TO7_SCREEN_CH          (TO7_SCREEN_H/TO7_CHAR_SIZE)  /* hauteur de l'écran en CHAR */

#define TO7_MEMO7_LABEL_LENGTH 25

#define TO7_SHIFT_FLAG             (1<<0)
#define TO7_CTRL_FLAG              (1<<1)
#define TO7_CAPSLOCK_FLAG          (1<<2)
#define TO7_MAX_FLAG               3

enum {
    TO7_ERROR,
    TO7_OK,
    TO7_READ_ONLY,
    TO7_READ_WRITE,
    TO7_NO_DISK_CTRL,
    TO7_CD90_640
};

/* fonctions importables requises */
extern void to7_PutSoundByte(unsigned long long int, unsigned char);

/* fonctions importables optionnelles */
extern void to7_SetBorderColor(int color);
extern void to7_SetDiskLed(int led_on);

extern int  (*to7_DirectReadSector)(int drive, int track, int sector, int nsects, unsigned char data[]);
extern int  (*to7_DirectWriteSector)(int drive, int track, int sector, int nsects, const unsigned char data[]);
extern int  (*to7_DirectFormatTrack)(int drive, int track, const unsigned char data[]);


/* variables exportables */
extern char to7_error_msg[];
extern unsigned short *to7_vram;

/* fonctions exportables */
extern int   to7_Init(int);
extern void  to7_Exit(void);
extern int   to7_LoadMemo7(const char []);
extern void  to7_Reset(void);
extern void  to7_ColdReset(void);
extern void  to7_InputReset(int, int);
extern void  to7_DoFrame(void);
extern void  to7_QueryColor(int, int *, int *, int *);
extern void  to7_HandleKeyPress(int, int);
extern void  to7_HandleMouseMotion(int, int);
extern void  to7_HandleMouseClick(int, int);
extern int   to7_LoadK7(const char []);
extern int   to7_LoadK7(const char filename[]);
extern int   to7_SetK7Mode(int mode);
extern int   to7_GetK7Counter(void);
extern void  to7_SetK7Counter(int counter);
extern int   to7_LoadDisk(int drive, const char filename[]);
extern int   to7_SetDiskMode(int drive, int mode);
extern int   to7_DirectSetDrive(int drive);

extern const char* to7_GetMemo7Label(void);
extern const char* to7_GetMemo7Filename(void);
extern const char* to7_GetK7Filename(void);
extern const char* to7_GetDiskFilename(int drive);

#endif

