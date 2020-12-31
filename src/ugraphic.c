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
 *  Module     : linux/graphic.c
 *  Version    : 1.5.5
 *  Créé par   : Eric Botcazou juillet 1999
 *  Modifié par: Eric Botcazou 03/11/2003
 *
 *  Gestion de l'affichage du TO7-70.
 */


#ifndef SCAN_DEPEND

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <psptypes.h>

#endif
#include <SDL.h>

#include "psp_sdl.h"
#include "global.h"
#include "to7.h"
#include "display.h"


int psp_screenshot_mode;
# if 0 //LUDO:
int need_border_refresh = 0;
static int border_color;
# endif

#define CELL_COLS  40
#define CELL_ROWS  25

ushort *to7_vram;

static int xcolor[TO7_NCOLORS+2];

/* SetBorderColor:
 *  Fixe la couleur du pourtour de l'écran.
 */
void 
to7_SetBorderColor(int color)
{
# if 0 //LUDO:
    if (border_color != color)
    {
        border_color=color;
        need_border_refresh=1;
    }
# endif
}



/* DrawGPL:
 *  Affiche un Groupe Point Ligne (un octet de VRAM).
 */
void 
to7_DrawGPL(int addr, int pt, int col)
{
    register int i;
    unsigned int x,y,c1,c2;

    c1=((col>>3)&7)+(((~col)&0x40)>>3);
    c2=(col&7)+(((~col)&0x80)>>4);

    x=(addr%CELL_COLS)*8;
    y=addr/CELL_COLS;

    ushort *vram = &to7_vram[x + (y * TO7_SCREEN_W)];
    for (i=0; i<8; i++) {
      *vram++ = xcolor[pt&(0x80>>i) ? c1 : c2];
    }
}


#define LED_SIZE 6

/* SetDiskLed:
 *  Allume/éteint la led du lecteur de disquettes.
 */
void 
to7_SetDiskLed(int led_on)
{
# if 0  //LUDO:
  if (led_on) {
    psp_sdl_draw_rectangle( WINDOW_WIDTH-LED_SIZE  , 0, LED_SIZE-1, LED_SIZE-1,
           xcolor[TO7_NCOLORS], 0);
    psp_sdl_fill_rectangle( WINDOW_WIDTH-LED_SIZE+1, 1, LED_SIZE-2, LED_SIZE-2,
           xcolor[TO7_NCOLORS+1], 0);
  } else {
    need_border_refresh = 1;
  }
# endif
}

void 
to7_SetNoCapsLed(int led_on)
{
# if 0 //LUDO: TO_BE_DONE !
# endif
}

void
to7_clear_vram()
{
  /* initialisation de la memoire video */
  memset(to7_vram, 0, sizeof(ushort) * TO7_SCREEN_W * TO7_SCREEN_H);
}

/* InitGraphic:
 *  Sélectionne le visual, met en place la palette de couleurs et
 *  initialise le mécanisme de bufferisation (dirty rectangles).
 */
void 
InitGraphic(void)
{
    int i,j;

   to7_vram = blit_surface->pixels;

   to7_clear_vram();

    /* récupération de la palette du TO7-70 */
    for (i=0; i<TO7_NCOLORS; i++)
    {
        int red, green, blue;
        to7_QueryColor(i, &red, &green, &blue);
        xcolor[i] = psp_sdl_rgb(red, green, blue);
    }

    /* Couleurs de la led du lecteur de disquettes */
    xcolor[TO7_NCOLORS  ] = psp_sdl_rgb(0x00, 0x00, 0x00);
    xcolor[TO7_NCOLORS+1] = psp_sdl_rgb(0x00, 0xff, 0x00);
}

static inline void
loc_split_src_rect( SDL_Rect* r )
{
  if (r->x < 0) r->x = 0;
  if (r->y < 0) r->y = 0;
  if ((r->x + r->w) > TO7_SCREEN_W) r->w = TO7_SCREEN_W - r->x;
  if ((r->y + r->h) > TO7_SCREEN_H) r->h = TO7_SCREEN_H - r->y;
}


static void
to7_display_screen_gu_normal()
{
  SDL_Rect srcRect;
  SDL_Rect dstRect;

  srcRect.w = TO7_SCREEN_W;
  srcRect.h = TO7_SCREEN_H;
  srcRect.x = 0;
  srcRect.y = 0;

  dstRect.w = TO7_SCREEN_W;
  dstRect.h = TO7_SCREEN_H;
  dstRect.x = (480 - dstRect.w) / 2;
  dstRect.y = (272 - dstRect.h) / 2;

  psp_sdl_gu_stretch(&srcRect, &dstRect);
}

static void
to7_display_screen_gu_x125()
{
  SDL_Rect srcRect;
  SDL_Rect dstRect;

  srcRect.w = TO7_SCREEN_W;
  srcRect.h = TO7_SCREEN_H;
  srcRect.x = 0;
  srcRect.y = 0;

  dstRect.w = 400;
  dstRect.h = 250;
  dstRect.x = (480 - dstRect.w) / 2;
  dstRect.y = (272 - dstRect.h) / 2;

  psp_sdl_gu_stretch(&srcRect, &dstRect);
}

static void
to7_display_screen_gu_x15()
{
  SDL_Rect srcRect;
  SDL_Rect dstRect;

  srcRect.w = TO7_SCREEN_W;
  srcRect.h = 180;
  srcRect.x = 0;
  srcRect.y = 10 + THOM.thom_delta_y;
  loc_split_src_rect( &srcRect );

  dstRect.w = 480;
  dstRect.h = 270;
  dstRect.x = 0;
  dstRect.y = 0;

  psp_sdl_gu_stretch(&srcRect, &dstRect);
}

static void
to7_display_screen_gu_x175()
{
  SDL_Rect srcRect;
  SDL_Rect dstRect;

  srcRect.w = 276;
  srcRect.h = 160;
  srcRect.x = 22;
  srcRect.y = 20 + THOM.thom_delta_y;
  loc_split_src_rect( &srcRect );

  dstRect.w = 480;
  dstRect.h = 270;
  dstRect.x = 0;
  dstRect.y = 0;

  psp_sdl_gu_stretch(&srcRect, &dstRect);
}

static void
to7_display_screen_gu_fit()
{
  SDL_Rect srcRect;
  SDL_Rect dstRect;

  srcRect.w = TO7_SCREEN_W;
  srcRect.h = TO7_SCREEN_H;
  srcRect.x = 0;
  srcRect.y = 0;

  dstRect.w = 480;
  dstRect.h = 270;
  dstRect.x = 0;
  dstRect.y = 0;

  psp_sdl_gu_stretch(&srcRect, &dstRect);
}

static void
to7_display_screen_gu_fit_h()
{
  SDL_Rect srcRect;
  SDL_Rect dstRect;

  srcRect.w = TO7_SCREEN_W;
  srcRect.h = TO7_SCREEN_H;
  srcRect.x = 0;
  srcRect.y = 0;

  dstRect.w = 435;
  dstRect.h = 270;
  dstRect.x = 22;
  dstRect.y = 0;

  psp_sdl_gu_stretch(&srcRect, &dstRect);
}

static void
to7_synchronize(void)
{
	static u32 nextclock = 1;

  if (THOM.thom_speed_limiter) {

	  if (nextclock) {
		  u32 curclock;
		  do {
        curclock = SDL_GetTicks();
		  } while (curclock < nextclock);

      nextclock = curclock + (u32)( 1000 / THOM.thom_speed_limiter);
    }
  }
}

void
to7_update_fps()
{
  static u32 next_sec_clock = 0;
  static u32 cur_num_frame = 0;
  cur_num_frame++;
  u32 curclock = SDL_GetTicks();
  if (curclock > next_sec_clock) {
    next_sec_clock = curclock + 1000;
    THOM.thom_current_fps = cur_num_frame;
    cur_num_frame = 0;
  }
}

/* RefreshScreen:
 *  Rafraîchit l'écran du TO7-70.
 */
void
psp_thom_wait_vsync()
{
# ifndef LINUX_MODE
  static int loc_pv = 0;
  int cv = sceDisplayGetVcount();
  if (loc_pv == cv) {
    sceDisplayWaitVblankCB();
  }
  loc_pv = sceDisplayGetVcount();
# endif
}

void 
RefreshScreen(void)
{ 
  if (THOM.psp_skip_cur_frame <= 0) {

    THOM.psp_skip_cur_frame = THOM.psp_skip_max_frame;

    if (THOM.thom_render_mode == THOM_RENDER_NORMAL    ) to7_display_screen_gu_normal(); 
    else
    if (THOM.thom_render_mode == THOM_RENDER_X125      ) to7_display_screen_gu_x125(); 
    else
    if (THOM.thom_render_mode == THOM_RENDER_X15       ) to7_display_screen_gu_x15(); 
    else
    if (THOM.thom_render_mode == THOM_RENDER_X175      ) to7_display_screen_gu_x175(); 
    else
    if (THOM.thom_render_mode == THOM_RENDER_FIT       ) to7_display_screen_gu_fit(); 
    else
    if (THOM.thom_render_mode == THOM_RENDER_FIT_H     ) to7_display_screen_gu_fit_h(); 

    if (psp_kbd_is_danzeff_mode()) {
      sceDisplayWaitVblankStart();
      danzeff_moveTo(-165, -50);
      danzeff_render();
    }

    if (THOM.thom_view_fps) {
      char buffer[32];
      sprintf(buffer, "%3d", (int)THOM.thom_current_fps);
      psp_sdl_fill_print(0, 0, buffer, 0xffffff, 0 );
    }

    if (THOM.psp_display_lr) {
      psp_kbd_display_active_mapping();
    }
    if (THOM.thom_vsync) {
      psp_thom_wait_vsync();
    }
    psp_sdl_flip();
  
    if (psp_screenshot_mode) {
      psp_screenshot_mode--;
      if (psp_screenshot_mode <= 0) {
        psp_sdl_save_screenshot();
        psp_screenshot_mode = 0;
      }
    }

  } else if (THOM.psp_skip_max_frame) {
    THOM.psp_skip_cur_frame--;
  }

  if (THOM.thom_speed_limiter) {
    to7_synchronize();
  }

  if (THOM.thom_view_fps) {
    to7_update_fps();
  }

}
