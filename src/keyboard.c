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
 *  Module     : keyboard.c
 *  Version    : 1.5.5
 *  Créé par   : Eric Botcazou 1998
 *  Modifié par: Eric Botcazou 06/10/2001
 *
 *  Gestion du clavier (et des manettes) du TO7-70.
 */


#ifndef SCAN_DEPEND
   #include <stddef.h>
#endif

#include "intern/hardware.h"
#include "intern/keyboard.h"  /* MacOS */
#include "to7keys.h"
#include "to7.h"
#include "global.h"


int key_row[NKEY_ROWS]; /* ligne de touches du clavier */

int j0_dir[2], j1_dir[2]; /* buffer de direction des manettes */

static unsigned char key_shift[19]={ 36,61,63,41,46,47,
                                     64,38,35,34,39,40,45,32,33,94,
                                     47,46,62};


/* ResetJoypad:
 *  Remet au repos le port manettes.
 */
static void ResetJoypad(void)
{
    pia_ext.porta.idr = 0xFF;
    pia_ext.portb.idr = 0xCF;
    pia_ext.porta.cr  |= 0xC0;
    pia_ext.portb.cr  |= 0xC0;

    j0_dir[0]=j0_dir[1]=0xF;
    j1_dir[0]=j1_dir[1]=0xF0;
}



/* DoKeyShift:
 *  Transpose le code des touches shiftées.
 */
void DoKeyShift(int *br, int *cc)
{
    if (*cc&8)
    {
        if (*br==37)
        {
            *br=41;
            *cc&=0xF7;
        }
        else if ((42<=*br) && (*br<=60))
        {
            *br=key_shift[*br-42];
            *cc&=0xF7;
        }
    }
}



/* ResetKeyboard:
 *  Remet à zéro le clavier et le port manette du TO7-70.
 */
void ResetKeyboard(int mask, int value)
{
    int i;

    for (i=0;i<NKEY_ROWS;i++)
        key_row[i]=0xFF;

    ResetJoypad();

    if (mask&TO7_SHIFT_FLAG)
        key_row[7] = (value&TO7_SHIFT_FLAG ? key_row[7]&0xFE : key_row[7]|0x01);

    if (mask&TO7_CTRL_FLAG)
        key_row[6] = (value&TO7_CTRL_FLAG ? key_row[6]&0xDF : key_row[6]|0x20);
 
    if (mask&TO7_CAPSLOCK_FLAG)
        mc6846.prc = (value&TO7_CAPSLOCK_FLAG ? mc6846.prc&0xF7 : mc6846.prc|0x08);
}



/* SetNoCapsLed:
 *  Allume/éteint la led Minuscules.
 */
void 
SetNoCapsLed(int new_state)
{
    static int state=1;

    if (new_state^state)
    {
        state = new_state;
                
        to7_SetNoCapsLed(state);
    }
}



/**********************************/
/* partie publique                */
/**********************************/

int key_to_row_mask[TO7_MAX_KEY][2] =
{
  /* row code */
  {  5, 128 },   /*  TO7_KEY_1   */           
  {  4, 128 },   /*  TO7_KEY_2   */           
  {  3, 128 },   /*  TO7_KEY_3   */           
  {  2, 128 },   /*  TO7_KEY_4   */           
  {  1, 128 },   /*  TO7_KEY_5   */           
  {  0, 128 },   /*  TO7_KEY_6   */           
  {  0, 64  },   /*  TO7_KEY_7   */           
  {  1, 64  },   /*  TO7_KEY_8   */           
  {  2, 64  },   /*  TO7_KEY_9   */           
  {  3, 64  },   /*  TO7_KEY_0   */           

  {  8, 128 },   /*  TO7_KEY_MINUS   */       
  {  5, 64  },   /*  TO7_KEY_PLUS   */        

//A TESTER
  {  6, 32  },   /*  TO7_KEY_ACCENT   */       /* BAD */

//A TESTER
  { 12, 16  },   /*  TO7_KEY_SLASH   */

  {  5, 16  },   /*  TO7_KEY_ASTERISK   */    
  {  1,  1  },   /*  TO7_KEY_COMMA   */       

  { 10,  1  },   /*  TO7_KEY_PERIOD   */      
  { 11, 64  },   /*  TO7_KEY_AT   */          
            
  {  9, 64  },   /*  TO7_KEY_EXCLAMATN   */   
  { 11, 128 },   /*  TO7_KEY_DBLQUOTE   */    
  { 12, 128 },   /*  TO7_KEY_HASH   */        
  { 13, 16  },   /*  TO7_KEY_DOLLAR   */      
  {  4, 64  },   /*  TO7_KEY_PERCENT   */     

  { 13, 128 },   /*  TO7_KEY_AMPERSAND   */   
  { 10, 128 },   /*  TO7_KEY_QUOTE   */       

//A TESTER
  {  9, 128 },   /*  TO7_KEY_LEFTPAREN   */
  { 12, 64  },   /*  TO7_KEY_RIGHTPAREN  */

  { 13, 32  },   /*  TO7_KEY_BACKQUOTE   */    /* BAD */

  { 13, 64  },   /*  TO7_KEY_EQUAL   */       
  {  2,  1  },   /*  TO7_KEY_SEMICOLON   */   
  {  9,  1  },   /*  TO7_KEY_QUESTION   */    

  {  4, 16  },   /*  TO7_KEY_COLON   */
  {  3,  1  },   /*  TO7_KEY_LESS   */        
  { 11,  1  },   /*  TO7_KEY_GREATER   */     

//A TESTER
  { 11, 128 },   /*  TO7_KEY_POWER   */        /* BAD */
  { -1, -1  },   /*  TO7_KEY_STOP   */        

  {  6, 32  },   /*  TO7_KEY_CNT      */      
  {  6,  8  },   /*  TO7_KEY_RAZ      */      
  {  7,  1  },   /*  TO7_KEY_SHIFT_L   */     
  {  7,  1  },   /*  TO7_KEY_SHIFT_R   */     
  {  5, 32  },   /*  TO7_KEY_A   */           
  {  4,  4  },   /*  TO7_KEY_B   */           
  {  6,  4  },   /*  TO7_KEY_C   */           
  {  3,  8  },   /*  TO7_KEY_D   */           
  {  3, 32  },   /*  TO7_KEY_E   */           
  {  2,  8  },   /*  TO7_KEY_F   */           
  {  1,  8  },   /*  TO7_KEY_G   */           
  {  0,  8  },   /*  TO7_KEY_H   */           
  {  1, 16  },   /*  TO7_KEY_I   */           
  {  0,  4  },   /*  TO7_KEY_J   */           
  {  1,  4  },   /*  TO7_KEY_K   */           
  {  2,  4  },   /*  TO7_KEY_L   */           
  {  3,  4  },   /*  TO7_KEY_M   */           
  {  0,  1  },   /*  TO7_KEY_N   */           
  {  2, 16  },   /*  TO7_KEY_O   */           
  {  3, 16  },   /*  TO7_KEY_P   */           
  {  5,  8  },   /*  TO7_KEY_Q   */           
  {  2, 32  },   /*  TO7_KEY_R   */           
  {  4,  8  },   /*  TO7_KEY_S   */           
  {  1, 32  },   /*  TO7_KEY_T   */           
  {  0, 16  },   /*  TO7_KEY_U   */           
  {  5,  4  },   /*  TO7_KEY_V   */           
  {  6,  1  },   /*  TO7_KEY_W   */           
  {  5,  1  },   /*  TO7_KEY_X   */           
  {  0, 32  },   /*  TO7_KEY_Y   */           
  {  4, 32  },   /*  TO7_KEY_Z   */           
  {  4,  1  },   /*  TO7_KEY_SPACE   */       
  {  6, 16  },   /*  TO7_KEY_RETURN   */      
  {  2, 2   },   /*  TO7_KEY_TOP   */         

  {  0,  2  },   /*  TO7_KEY_DEL      */
  {  1,  2  },   /*  TO7_KEY_INS      */      
  { 12,  1  },   /*  TO7_KEY_CAPSLOCK   */    
  {  5,  2  },   /*  TO7_KEY_LEFT   */        
  {  6,  2  },   /*  TO7_KEY_UP   */          
  {  3,  2  },   /*  TO7_KEY_RIGHT   */       
  {  4,  2  },   /*  TO7_KEY_DOWN   */        
            
  /*    Joycode */
  { -1, 0xe },   /*  TO7_KEY_JOY_UP   */      
  { -1, 0xd },   /*  TO7_KEY_JOY_DOWN   */    
  { -1, 0xb },   /*  TO7_KEY_JOY_LEFT   */    
  { -1, 0x7 },   /*  TO7_KEY_JOY_RIGHT   */   

  { -1, -1  },   /*  TO7_KEY_JOY_FIRE   */   
};

void
to7_HandleJoyPress(int joy, int press)
{
  int release = ! press;

  if (joy == TO7_KEY_JOY_FIRE) {
    if (THOM.psp_active_joystick) {
      /* Fire joystick 1 */
      if (release) {
        pia_ext.portb.idr |= 0x80;
        pia_ext.porta.cr  |= 0x80;
      } else {
        pia_ext.portb.idr &= 0x7F;
        pia_ext.porta.cr  &= 0x7F;
      }
    } else {
      /* Fire joystick 0 */
      if (release) {
        pia_ext.portb.idr |= 0x40;
        pia_ext.porta.cr  |= 0x80;
      } else {
        pia_ext.portb.idr &= 0xBF;
        pia_ext.porta.cr  &= 0x7F;
      }
    }

  } else {
    
    int code = key_to_row_mask[joy][1];

    if (! press) {

      if (THOM.psp_active_joystick) {

        /* joystick 1 */
        code = code << 4;
        if (code == j1_dir[0]) {
          j1_dir[0]=j1_dir[1];
          j1_dir[1]=0xF0;
        } else 
        if (code == j1_dir[1]) {
          j1_dir[1]=0xF0;
        }

      } else {

        /* joystick 0 */
        if (code == j0_dir[0]) {
          j0_dir[0]=j0_dir[1];
          j0_dir[1]=0xF;
        } else 
        if (code == j0_dir[1]) {
          j0_dir[1]=0xF;
        }
      }

    } else {

      if (THOM.psp_active_joystick) {

        /* joystick 1 */
        code = code << 4;
        if (code != j1_dir[0]) {
          j1_dir[1]=j1_dir[0];
          j1_dir[0]=code;
        }

      } else {

        /* joystick 0 */
        if (code != j0_dir[0]) {
          j0_dir[1]=j0_dir[0];
          j0_dir[0]=code;
        }
      }
    }

    pia_ext.porta.idr = j1_dir[0] | j0_dir[0];
  }
}


/* HandleKeyPress:
 *  Prend en compte la frappe ou le relâchement d'une touche.
 *   key: scancode de la touche frappée/relachée (voir to7keys.h pour la liste)
 *   release: flag d'enfoncement/relâchement
 */
void 
to7_HandleKeyPress(int key, int press)
{
  int release = ! press;

  /* Joystick */
  if (key >= TO7_KEY_JOY_UP) {

    to7_HandleJoyPress(key, press);

  } else {
    /* Touches du clavier */
    
    int row  = key_to_row_mask[key][0];
    int mask = key_to_row_mask[key][1];

    if (row != -1 ) {

      if (row & 8) key_row[7] ^= 0x01;

      if (release) key_row[row&7] |= mask;
		  else         key_row[row&7] &= ~mask;
    }
  }
}

/* InitKeyboard:
 *  Effectue quelques modifications mineures de la routine de
 *  lecture clavier TO7-70.
 */
void InitKeyboard(void)
{
    /* on modifie l'emplacement de 4 caractères */
    mem.mon.bank[0][0x1D17]=37;  /* _ -> % */
    mem.mon.bank[0][0x1D19]=58;  /* . -> ; */
    mem.mon.bank[0][0x1D25]=60;  /* / -> : */
    mem.mon.bank[0][0x1D2D]=59;  /* @ -> < */

    /* appel de la routine de transposition des touches shiftées */
    mem.mon.bank[0][0x11F2]=0x02;

}

