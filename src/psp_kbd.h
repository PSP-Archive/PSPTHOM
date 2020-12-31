/*
 *  Copyright (C) 2006 Ludovic Jacomme (ludovic.jacomme@gmail.com)
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
 *  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

# ifndef _KBD_H_
# define _KBD_H_

# define PSP_ALL_BUTTON_MASK 0xFFFF

 enum thom_keys_emum {

    THOMK_1,
    THOMK_2,
    THOMK_3,
    THOMK_4,
    THOMK_5,
    THOMK_6,
    THOMK_7,
    THOMK_8,
    THOMK_9,
    THOMK_0,
    THOMK_MINUS,
    THOMK_PLUS,
    THOMK_ACCENT,
    THOMK_SLASH,
    THOMK_ASTERISK,
    THOMK_COMMA,
    THOMK_PERIOD,
    THOMK_AT,
    THOMK_EXCLAMATN,
    THOMK_DBLQUOTE,
    THOMK_HASH,
    THOMK_DOLLAR,
    THOMK_PERCENT,
    THOMK_AMPERSAND,
    THOMK_QUOTE,
    THOMK_LEFTPAREN,
    THOMK_RIGHTPAREN,
    THOMK_BACKQUOTE,
    THOMK_EQUAL,
    THOMK_SEMICOLON,
    THOMK_QUESTION,
    THOMK_COLON,
    THOMK_LESS,
    THOMK_GREATER,
    THOMK_POWER,
    THOMK_STOP,
    THOMK_CNT,
    THOMK_RAZ,
    THOMK_SHIFT_L,
    THOMK_SHIFT_R,
    THOMK_A,
    THOMK_B,
    THOMK_C,
    THOMK_D,
    THOMK_E,
    THOMK_F,
    THOMK_G,
    THOMK_H,
    THOMK_I,
    THOMK_J,
    THOMK_K,
    THOMK_L,
    THOMK_M,
    THOMK_N,
    THOMK_O,
    THOMK_P,
    THOMK_Q,
    THOMK_R,
    THOMK_S,
    THOMK_T,
    THOMK_U,
    THOMK_V,
    THOMK_W,
    THOMK_X,
    THOMK_Y,
    THOMK_Z,
    THOMK_SPACE,
    THOMK_RETURN,
    THOMK_TOP,
    THOMK_DELETE,
    THOMK_INSERT,
    THOMK_CAPSLOCK,
    THOMK_LEFT,
    THOMK_UP,
    THOMK_RIGHT,
    THOMK_DOWN,
    THOMK_JOY_UP,
    THOMK_JOY_DOWN,
    THOMK_JOY_LEFT,
    THOMK_JOY_RIGHT,
    THOMK_JOY_FIRE,

    THOMC_FPS,
    THOMC_JOY,
    THOMC_RENDER,
    THOMC_LOAD,
    THOMC_SAVE,
    THOMC_RESET,
    THOMC_AUTOFIRE,
    THOMC_INCFIRE,
    THOMC_DECFIRE,
    THOMC_INCDELTA,
    THOMC_DECDELTA,
    THOMC_SCREEN,

    THOMK_MAX_KEY

  };

# define KBD_UP           0
# define KBD_RIGHT        1
# define KBD_DOWN         2
# define KBD_LEFT         3
# define KBD_TRIANGLE     4
# define KBD_CIRCLE       5
# define KBD_CROSS        6
# define KBD_SQUARE       7
# define KBD_SELECT       8
# define KBD_START        9
# define KBD_HOME        10
# define KBD_HOLD        11
# define KBD_LTRIGGER    12
# define KBD_RTRIGGER    13

# define KBD_MAX_BUTTONS 14

# define KBD_JOY_UP      14
# define KBD_JOY_RIGHT   15
# define KBD_JOY_DOWN    16
# define KBD_JOY_LEFT    17

# define KBD_ALL_BUTTONS 18

# define KBD_UNASSIGNED         -1

# define KBD_LTRIGGER_MAPPING   -2
# define KBD_RTRIGGER_MAPPING   -3
# define KBD_NORMAL_MAPPING     -1

 struct thom_key_trans {
   int  key;
   int  to7_id;
   int  shift;
   char name[10];
 };
  

  extern int psp_screenshot_mode;
  extern int psp_kbd_mapping[ KBD_ALL_BUTTONS ];
  extern int psp_kbd_mapping_L[ KBD_ALL_BUTTONS ];
  extern int psp_kbd_mapping_R[ KBD_ALL_BUTTONS ];
  extern int psp_kbd_presses[ KBD_ALL_BUTTONS ];
  extern int kbd_ltrigger_mapping_active;
  extern int kbd_rtrigger_mapping_active;

  extern struct thom_key_trans psp_thom_key_info[THOMK_MAX_KEY];

  extern int  psp_update_keys(void);
  extern void kbd_wait_start(void);
  extern void psp_init_keyboard(void);
  extern void psp_kbd_wait_no_button(void);
  extern int  psp_kbd_is_danzeff_mode(void);
  extern int psp_kbd_load_mapping(char *kbd_filename);
  extern int psp_kbd_save_mapping(char *kbd_filename);
  extern void psp_kbd_display_active_mapping(void);
  extern void kbd_change_auto_fire(int auto_fire);
# endif
