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
 *  Module     : linux/sound.c
 *  Version    : 1.5.5
 *  Créé par   : Eric Botcazou août 1999
 *  Modifié par: Eric Botcazou 03/11/2003
 *
 *  Gestion de l'émulation sonore du TO7-70.
 */


#ifndef SCAN_DEPEND
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <malloc.h>
#include "SDL.h"
#endif

#include "main.h"
#include "to7.h"

# if 0
#define SOUND_FREQ  25600
#define FRAG_EXP 9
#define SOUND_BUFFER_SIZE (1<<FRAG_EXP)
#define DEVNAME "/dev/dsp"
# else
#define SOUND_FREQ            22050
#define FRAG_EXP              9
#define SOUND_BUFFER_SIZE     (1<<FRAG_EXP)

#define SDL_SOUND_FREQ        44100
#define SDL_SOUND_BUFFER_SIZE (2<<FRAG_EXP)
# endif


static int audio_fd;
static unsigned char sound_buffer[SOUND_BUFFER_SIZE];
static unsigned char sdl_sound_buffer[SDL_SOUND_BUFFER_SIZE];

static int last_index;
static unsigned char last_data;

/* PutSoundByte:
 *  Place un octet dans le tampon de streaming audio.
 */
void 
to7_PutSoundByte(unsigned long long int clock, unsigned char data)
{
    int i;
    int index=(clock%TO7_CYCLES_PER_FRAME)*SOUND_FREQ/TO7_CPU_FREQ;

    /* Dans le cas où le nombre de cycles éxécutés pendant une frame dépasse la valeur
       théorique, on bloque l'index à sa valeur maximale */
    if (index < last_index)
	  index=SOUND_BUFFER_SIZE;

    for (i=last_index; i<index; i++) {
      sound_buffer[i]=last_data;
    }

    last_index=index;
    last_data=data;
}

void 
loc_SoundCallback(void *userdata, unsigned char *stream, int len)
{
  if (len > SDL_SOUND_BUFFER_SIZE) len = SDL_SOUND_BUFFER_SIZE;
# if 1 //LUDO:
  SDL_MixAudio(stream, (unsigned char *)sdl_sound_buffer, len, (SDL_MIX_MAXVOLUME * 2) / 3);
# else
  memcpy(stream, sdl_sound_buffer, len);
# endif
}

/* InitSound:
 *  Initialise le module de streaming audio.
 */
void 
InitSound(void)
{
  SDL_AudioSpec desired;
  SDL_AudioSpec obtained;

  memset(&obtained,0,sizeof(SDL_AudioSpec));
  memset(&desired,0,sizeof(SDL_AudioSpec));

  desired.freq     = SDL_SOUND_FREQ;
  desired.format   = AUDIO_U8;
  desired.samples  = SDL_SOUND_BUFFER_SIZE;
  desired.channels = 1;
  desired.callback = loc_SoundCallback;

  if (SDL_OpenAudio(&desired, &obtained) < 0) {
    fprintf(stderr, "Could not open audio: %s\n", SDL_GetError());
    exit(1);
  }
  
# if 0 //LUDO: FOR_DEBUG
  fprintf(stdout, "freq=%d\n", obtained.freq);
  fprintf(stdout, "format=%d\n", obtained.format);
  fprintf(stdout, "samples=%d\n", obtained.samples);
  fprintf(stdout, "channels=%d\n", obtained.channels);
# endif

  memset(sdl_sound_buffer, 0x0, SDL_SOUND_BUFFER_SIZE);

  SDL_PauseAudio(0);
}

/* PlaySoundBuffer:
 *  Envoie le tampon de streaming audio à la carte son.
 */
void 
PlaySoundBuffer(void)
{
  int i;
  /* on remplit la fin du buffer avec le dernier byte déposé */
  for (i=last_index; i<SOUND_BUFFER_SIZE; i++) {
# if 0 //LUDO:
    sound_buffer[i]=last_data;
# else
    sound_buffer[i]=0;
# endif
  }
  last_index=0;

  for (i = 0; i < SDL_SOUND_BUFFER_SIZE; i++) {
   sdl_sound_buffer[i] = sound_buffer[i >> 1];
  }
  memset(sound_buffer, 0x0, SOUND_BUFFER_SIZE);
}
