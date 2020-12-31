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
 *  Module     : linux/main.c
 *  Version    : 1.5.5
 *  Créé par   : Eric Botcazou juillet 1999
 *  Modifié par: Eric Botcazou 03/11/2003
 *
 *  Boucle principale de l'émulateur.
 */


#ifndef SCAN_DEPEND
   #include <locale.h>
   #include <signal.h>
   #include <stdio.h>
   #include <stdlib.h>
   #include <string.h>
   #include <unistd.h>
   #include <sys/time.h>
#endif
#include "global.h"

#include "to7.h"
#include "intern/disk.h"
#include "display.h"
#include "graphic.h"
#include "gui.h"
#include "main.h"
#include "sound.h"

#include "psp_run.h"

struct EmuTO thom={
    TRUE,
    TRUE,
    NONE
};

static int frame;           /* compteur de frame vidéo */
static volatile int tick;   /* compteur du timer */

/* RunTO7:
 *  Boucle principale de l'émulateur.
 */
static void 
RunTO7(void)
{
    frame=1;
    to7_ColdReset();

    psp_sdl_black_screen();

    do   /* boucle principale de l'émulateur */
    {
        thom.command=NONE;
        tick=frame;

        do  /* boucle d'émulation */
        {
            to7_DoFrame();

            RefreshScreen();

            HandleEvents();

            if (thom.exact_speed)  /* synchronisation sur fréquence réelle */
            {
              PlaySoundBuffer();
            }

            frame++;
        }
        while (thom.command==NONE);  /* fin de la boucle d'émulation */
          
        if (thom.command==RESET)
            to7_Reset();

        if (thom.command==COLD_RESET)
            to7_ColdReset();

    }
    while (thom.command != QUIT);  /* fin de la boucle principale */
}

/* ExitMessage:
 *  Affiche un message de sortie et sort du programme.
 */
static void ExitMessage(const char msg[])
{
    fprintf(stderr, "%s\n", msg);
    exit(EXIT_FAILURE);
}


#define IS_5_INCHES(drive) ((drive_type[drive]>0) && (drive_type[drive]<3))

/* main:
 *  Point d'entrée du programme appelé par Linux.
 */
int 
SDL_main(int argc,char *argv[])
{
    int i, x=0, y=0, user_flags=0;
    char memo_name[FILENAME_LENGTH]="";
# if 1 //LUDO:
    int disk_ctrl = TO7_CD90_640;
# else
    /* For the moment we support only k7 */
    int disk_ctrl = TO7_NO_DISK_CTRL;
# endif
    int direct_support = 0, direct_write_support = FALSE;
    int drive_type[4];

  memset(&THOM, 0, sizeof(THOM_t));
  getcwd(THOM.thom_home_dir,MAX_PATH);

  psp_sdl_init();

  thom_default_settings();
  thom_update_save_name("");
  thom_load_settings();

  THOM.thom_load_mode = THOM_LOAD_K7_MODE;
  THOM.thom_basic_128 = 1;

  scePowerSetClockFrequency(THOM.psp_cpu_clock, THOM.psp_cpu_clock, THOM.psp_cpu_clock/2);

  psp_run_load_file();

  /* Affichage du message de bienvenue du programme */

  if (to7_Init(disk_ctrl) == TO7_ERROR)
  {
# if 1 //LUDO:
    /* la ROM contrôleur disk est peut-être manquante? */
    disk_ctrl = TO7_NO_DISK_CTRL;

    if (to7_Init(disk_ctrl) == TO7_ERROR) {
      ExitMessage(to7_error_msg);
    } else {
      printf("%s\n", to7_error_msg);
    }
# else
    ExitMessage(to7_error_msg);
# endif
  }

  /* Chargement de la cartouche */
# if 0 //LUDO:
  if (memo_name[0])
  {
    to7_LoadMemo7(memo_name);
  }
  else if (disk_ctrl == TO7_NO_DISK_CTRL)
  {
    to7_LoadMemo7("./memo7/basic.m7");
  }
  else
# endif
  if (THOM.thom_basic_128) {
    to7_LoadMemo7("./memo7/basic128.m7");
  } else {
    to7_LoadMemo7("./memo7/basic.m7");
  }

  /* Initialisation des modules graphique et sonore */
  InitGraphic();
  InitSound();  
  RunTO7();

  psp_sdl_exit(0);
}
