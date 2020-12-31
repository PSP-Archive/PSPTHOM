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

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <sys/time.h>
#include <fcntl.h>

#include <pspctrl.h>
#include <pspkernel.h>
#include <pspdebug.h>
#include <SDL.h>

#include "global.h"
#include "psp_kbd.h"
#include "psp_menu.h"
#include "psp_sdl.h"
#include "psp_danzeff.h"
#include "psp_irkeyb.h"
#include "psp_battery.h"

#include "intern/keyboard.h"
#include "to7keys.h"

# define KBD_MIN_ANALOG_TIME  150000
# define KBD_MIN_START_TIME   800000
# define KBD_MAX_EVENT_TIME   500000
# define KBD_MIN_PENDING_TIME 300000
# define KBD_MIN_HOTKEY_TIME     1000000
# define KBD_MIN_DANZEFF_TIME 150000
# define KBD_MIN_COMMAND_TIME 100000
# define KBD_MIN_BATTCHECK_TIME 90000000 
# define KBD_MIN_AUTOFIRE_TIME   1000000

 static SceCtrlData    loc_button_data;
 static unsigned int   loc_last_event_time = 0;
 static unsigned int   loc_last_hotkey_time = 0;
#ifdef USE_PSP_IRKEYB
 static unsigned int   loc_last_irkbd_event_time = 0;
#endif
 static unsigned int   loc_last_analog_time = 0;
 static long           first_time_stamp = -1;
 static long           first_time_batt_stamp = -1;
 static long           first_time_auto_stamp = -1;
 static char           loc_button_press[ KBD_MAX_BUTTONS ]; 
 static char           loc_button_release[ KBD_MAX_BUTTONS ]; 
 static unsigned int   loc_button_mask[ KBD_MAX_BUTTONS ] =
 {
   PSP_CTRL_UP         , /*  KBD_UP         */
   PSP_CTRL_RIGHT      , /*  KBD_RIGHT      */
   PSP_CTRL_DOWN       , /*  KBD_DOWN       */
   PSP_CTRL_LEFT       , /*  KBD_LEFT       */
   PSP_CTRL_TRIANGLE   , /*  KBD_TRIANGLE   */
   PSP_CTRL_CIRCLE     , /*  KBD_CIRCLE     */
   PSP_CTRL_CROSS      , /*  KBD_CROSS      */
   PSP_CTRL_SQUARE     , /*  KBD_SQUARE     */
   PSP_CTRL_SELECT     , /*  KBD_SELECT     */
   PSP_CTRL_START      , /*  KBD_START      */
   PSP_CTRL_HOME       , /*  KBD_HOME       */
   PSP_CTRL_HOLD       , /*  KBD_HOLD       */
   PSP_CTRL_LTRIGGER   , /*  KBD_LTRIGGER   */
   PSP_CTRL_RTRIGGER   , /*  KBD_RTRIGGER   */
 };

 static char loc_button_name[ KBD_ALL_BUTTONS ][20] =
 {
   "UP",
   "RIGHT",
   "DOWN",
   "LEFT",
   "TRIANGLE",
   "CIRCLE",
   "CROSS",
   "SQUARE",
   "SELECT",
   "START",
   "HOME",
   "HOLD",
   "LTRIGGER",
   "RTRIGGER",
   "JOY_UP",
   "JOY_RIGHT",
   "JOY_DOWN",
   "JOY_LEFT"
 };

 static char loc_button_name_L[ KBD_ALL_BUTTONS ][20] =
 {
   "L_UP",
   "L_RIGHT",
   "L_DOWN",
   "L_LEFT",
   "L_TRIANGLE",
   "L_CIRCLE",
   "L_CROSS",
   "L_SQUARE",
   "L_SELECT",
   "L_START",
   "L_HOME",
   "L_HOLD",
   "L_LTRIGGER",
   "L_RTRIGGER",
   "L_JOY_UP",
   "L_JOY_RIGHT",
   "L_JOY_DOWN",
   "L_JOY_LEFT"
 };
 
  static char loc_button_name_R[ KBD_ALL_BUTTONS ][20] =
 {
   "R_UP",
   "R_RIGHT",
   "R_DOWN",
   "R_LEFT",
   "R_TRIANGLE",
   "R_CIRCLE",
   "R_CROSS",
   "R_SQUARE",
   "R_SELECT",
   "R_START",
   "R_HOME",
   "R_HOLD",
   "R_LTRIGGER",
   "R_RTRIGGER",
   "R_JOY_UP",
   "R_JOY_RIGHT",
   "R_JOY_DOWN",
   "R_JOY_LEFT"
 };
 
  struct thom_key_trans psp_thom_key_info[THOMK_MAX_KEY]=
  {
    // THOMK            SHIFT            NAME 
    { THOMK_1,          TO7_KEY_1,           0,     "1" },
    { THOMK_2,          TO7_KEY_2,           0,     "2" },
    { THOMK_3,          TO7_KEY_3,           0,     "3" },
    { THOMK_4,          TO7_KEY_4,           0,     "4" },
    { THOMK_5,          TO7_KEY_5,           0,     "5" },
    { THOMK_6,          TO7_KEY_6,           0,     "6" },
    { THOMK_7,          TO7_KEY_7,           0,     "7" },
    { THOMK_8,          TO7_KEY_8,           0,     "8" },
    { THOMK_9,          TO7_KEY_9,           0,     "9" },
    { THOMK_0,          TO7_KEY_0,           0,     "0" },
    { THOMK_MINUS    ,  TO7_KEY_MINUS    ,   0,     "-" },
    { THOMK_PLUS,       TO7_KEY_PLUS,        0,     "+" },
    { THOMK_ACCENT,     TO7_KEY_ACCENT,      0,     "ACCENT" },
    { THOMK_SLASH,      TO7_KEY_SLASH,       0,     "/" },
    { THOMK_ASTERISK,   TO7_KEY_ASTERISK,    0,     "*" },
    { THOMK_COMMA,      TO7_KEY_COMMA,       0,     "," },
    { THOMK_PERIOD,     TO7_KEY_PERIOD,      0,     "." },
    { THOMK_AT,         TO7_KEY_AT,          0,     "@" },

    { THOMK_EXCLAMATN,  TO7_KEY_EXCLAMATN,   0 /* 1 */,     "!" },
    { THOMK_DBLQUOTE,   TO7_KEY_DBLQUOTE,    0 /* 1 */,     "\"" },
    { THOMK_HASH,       TO7_KEY_HASH,        0 /* 1 */,     "#" },
    { THOMK_DOLLAR,     TO7_KEY_DOLLAR,      0 /* 1 */,     "$" },
    { THOMK_PERCENT,    TO7_KEY_PERCENT,     0 /* 1 */,     "%" },
    { THOMK_AMPERSAND,  TO7_KEY_AMPERSAND,   0 /* 1 */,     "&" },
    { THOMK_QUOTE,      TO7_KEY_QUOTE,       0 /* 1 */,     "'" },
    { THOMK_LEFTPAREN,  TO7_KEY_LEFTPAREN,   0 /* 1 */,     "(" },
    { THOMK_RIGHTPAREN, TO7_KEY_RIGHTPAREN,  0 /* 1 */,     ")" },
    { THOMK_BACKQUOTE,  TO7_KEY_BACKQUOTE,   0 /* 1 */,     "`" },
    { THOMK_EQUAL,      TO7_KEY_EQUAL,       0 /* 1 */,     "=" },
    { THOMK_SEMICOLON,  TO7_KEY_SEMICOLON,   0 /* 1 */,     ";" },
    { THOMK_QUESTION,   TO7_KEY_QUESTION,    0 /* 1 */,     "?" },
    { THOMK_COLON,      TO7_KEY_COLON,       0 /* 1 */,     ":" },
    { THOMK_LESS,       TO7_KEY_LESS,        0 /* 1 */,     "<" },
    { THOMK_GREATER,    TO7_KEY_GREATER,     0 /* 1 */,     ">" },
    { THOMK_POWER,      TO7_KEY_POWER,       0 /* 1 */,     "^" },


    { THOMK_STOP,       TO7_KEY_STOP,        0,     "STOP" },
    { THOMK_CNT,        TO7_KEY_CNT   ,      0,     "CNT" },
    { THOMK_RAZ,        TO7_KEY_RAZ   ,      0,     "RAZ" },
    { THOMK_SHIFT_L,    TO7_KEY_SHIFT_L,     0,     "SHIFT_L" },
    { THOMK_SHIFT_R,    TO7_KEY_SHIFT_R,     0,     "SHIFT_R" },

    { THOMK_A,          TO7_KEY_A,           1,     "A" },
    { THOMK_B,          TO7_KEY_B,           1,     "B" },
    { THOMK_C,          TO7_KEY_C,           1,     "C" },
    { THOMK_D,          TO7_KEY_D,           1,     "D" },
    { THOMK_E,          TO7_KEY_E,           1,     "E" },
    { THOMK_F,          TO7_KEY_F,           1,     "F" },
    { THOMK_G,          TO7_KEY_G,           1,     "G" },
    { THOMK_H,          TO7_KEY_H,           1,     "H" },
    { THOMK_I,          TO7_KEY_I,           1,     "I" },
    { THOMK_J,          TO7_KEY_J,           1,     "J" },
    { THOMK_K,          TO7_KEY_K,           1,     "K" },
    { THOMK_L,          TO7_KEY_L,           1,     "L" },
    { THOMK_M,          TO7_KEY_M,           1,     "M" },
    { THOMK_N,          TO7_KEY_N,           1,     "N" },
    { THOMK_O,          TO7_KEY_O,           1,     "O" },
    { THOMK_P,          TO7_KEY_P,           1,     "P" },
    { THOMK_Q,          TO7_KEY_Q,           1,     "Q" },
    { THOMK_R,          TO7_KEY_R,           1,     "R" },
    { THOMK_S,          TO7_KEY_S,           1,     "S" },
    { THOMK_T,          TO7_KEY_T,           1,     "T" },
    { THOMK_U,          TO7_KEY_U,           1,     "U" },
    { THOMK_V,          TO7_KEY_V,           1,     "V" },
    { THOMK_W,          TO7_KEY_W,           1,     "W" },
    { THOMK_X,          TO7_KEY_X,           1,     "X" },
    { THOMK_Y,          TO7_KEY_Y,           1,     "Y" },
    { THOMK_Z,          TO7_KEY_Z,           1,     "Z" },

    { THOMK_SPACE,      TO7_KEY_SPACE,       0,     "SPACE" },
    { THOMK_RETURN,     TO7_KEY_RETURN,      0,     "RETURN" },
    { THOMK_TOP,        TO7_KEY_TOP,         0,     "TOP"    },

    { THOMK_DELETE,     TO7_KEY_DEL   ,      0,     "DELETE" },
    { THOMK_INSERT,     TO7_KEY_INS   ,      0,     "INSERT" },

    { THOMK_CAPSLOCK,   TO7_KEY_CAPSLOCK,    0,     "CAPSLOCK" }, /* SHIFT_L + SPACE */

    { THOMK_LEFT,       TO7_KEY_LEFT,        0,     "LEFT" },
    { THOMK_UP,         TO7_KEY_UP,          0,     "UP" },
    { THOMK_RIGHT,      TO7_KEY_RIGHT,       0,     "RIGHT" },
    { THOMK_DOWN,       TO7_KEY_DOWN,        0,     "DOWN" },

    { THOMK_JOY_UP,     TO7_KEY_JOY_UP,      0,     "JOY_UP" },
    { THOMK_JOY_DOWN,   TO7_KEY_JOY_DOWN,    0,     "JOY_DOWN" },
    { THOMK_JOY_LEFT,   TO7_KEY_JOY_LEFT,    0,     "JOY_LEFT" },
    { THOMK_JOY_RIGHT,  TO7_KEY_JOY_RIGHT,   0,     "JOY_RIGHT" },

    { THOMK_JOY_FIRE,  TO7_KEY_JOY_FIRE ,   0,     "JOY_FIRE" },

    { THOMC_FPS,       0, 0, "C_FPS" },
    { THOMC_JOY,       0, 0, "C_JOY" },
    { THOMC_RENDER,    0, 0, "C_RENDER" },
    { THOMC_LOAD,      0, 0, "C_LOAD" },
    { THOMC_SAVE,      0, 0, "C_SAVE" },
    { THOMC_RESET,     0, 0, "C_RESET" },
    { THOMC_AUTOFIRE,  0, 0, "C_AUTOFIRE" },
    { THOMC_INCFIRE,   0, 0, "C_INCFIRE" },
    { THOMC_DECFIRE,   0, 0, "C_DECFIRE" },
    { THOMC_INCDELTA,  0, 0, "C_INCDELTA" },
    { THOMC_DECDELTA,  0, 0, "C_DECDELTA" },
    { THOMC_SCREEN,    0, 0, "C_SCREEN" },
  };

  static int loc_default_mapping[ KBD_ALL_BUTTONS ] = {
    THOMK_UP              , /*  KBD_UP         */
    THOMK_RIGHT           , /*  KBD_RIGHT      */
    THOMK_DOWN            , /*  KBD_DOWN       */
    THOMK_LEFT            , /*  KBD_LEFT       */
    THOMK_RETURN          , /*  KBD_TRIANGLE   */
    THOMK_1               , /*  KBD_CIRCLE     */
    THOMK_JOY_FIRE        , /*  KBD_CROSS      */
    THOMK_SPACE           , /*  KBD_SQUARE     */
    -1                    , /*  KBD_SELECT     */
    -1                    , /*  KBD_START      */
    -1                    , /*  KBD_HOME       */
    -1                    , /*  KBD_HOLD       */
    KBD_LTRIGGER_MAPPING  , /*  KBD_LTRIGGER   */
    KBD_RTRIGGER_MAPPING  , /*  KBD_RTRIGGER   */
    THOMK_JOY_UP          , /*  KBD_JOY_UP     */
    THOMK_JOY_RIGHT       , /*  KBD_JOY_RIGHT  */
    THOMK_JOY_DOWN        , /*  KBD_JOY_DOWN   */
    THOMK_JOY_LEFT          /*  KBD_JOY_LEFT   */
  };

  static int loc_default_mapping_L[ KBD_ALL_BUTTONS ] = {
    THOMC_INCDELTA        , /*  KBD_UP         */
    THOMC_RENDER          , /*  KBD_RIGHT      */
    THOMC_DECDELTA        , /*  KBD_DOWN       */
    THOMC_RENDER          , /*  KBD_LEFT       */
    THOMC_LOAD            , /*  KBD_TRIANGLE   */
    THOMC_JOY             , /*  KBD_CIRCLE     */
    THOMC_SAVE            , /*  KBD_CROSS      */
    THOMC_FPS             , /*  KBD_SQUARE     */
    -1                    , /*  KBD_SELECT     */
    -1                    , /*  KBD_START      */
    -1                    , /*  KBD_HOME       */
    -1                    , /*  KBD_HOLD       */
    KBD_LTRIGGER_MAPPING  , /*  KBD_LTRIGGER   */
    KBD_RTRIGGER_MAPPING  , /*  KBD_RTRIGGER   */
    THOMK_JOY_UP          , /*  KBD_JOY_UP     */
    THOMK_JOY_RIGHT       , /*  KBD_JOY_RIGHT  */
    THOMK_JOY_DOWN        , /*  KBD_JOY_DOWN   */
    THOMK_JOY_LEFT          /*  KBD_JOY_LEFT   */
  };

  static int loc_default_mapping_R[ KBD_ALL_BUTTONS ] = {
    THOMK_UP              , /*  KBD_UP         */
    THOMC_INCFIRE         , /*  KBD_RIGHT      */
    THOMK_DOWN            , /*  KBD_DOWN       */
    THOMC_DECFIRE         , /*  KBD_LEFT       */
    THOMK_RETURN          , /*  KBD_TRIANGLE   */
    THOMK_1               , /*  KBD_CIRCLE     */
    THOMC_AUTOFIRE        , /*  KBD_CROSS      */
    THOMK_STOP            , /*  KBD_SQUARE     */
    -1                    , /*  KBD_SELECT     */
    -1                    , /*  KBD_START      */
    -1                    , /*  KBD_HOME       */
    -1                    , /*  KBD_HOLD       */
    KBD_LTRIGGER_MAPPING  , /*  KBD_LTRIGGER   */
    KBD_RTRIGGER_MAPPING  , /*  KBD_RTRIGGER   */
    THOMK_JOY_UP          , /*  KBD_JOY_UP     */
    THOMK_JOY_RIGHT       , /*  KBD_JOY_RIGHT  */
    THOMK_JOY_DOWN        , /*  KBD_JOY_DOWN   */
    THOMK_JOY_LEFT          /*  KBD_JOY_LEFT   */
  };

# define KBD_MAX_ENTRIES   102

  int kbd_layout[KBD_MAX_ENTRIES][2] = {
    /* Key            Ascii */
    { THOMK_1,          '1' },
    { THOMK_2,          '2' },
    { THOMK_3,          '3' },
    { THOMK_4,          '4' },
    { THOMK_5,          '5' },
    { THOMK_6,          '6' },
    { THOMK_7,          '7' },
    { THOMK_8,          '8' },
    { THOMK_9,          '9' },
    { THOMK_0,          '0' },
    { THOMK_MINUS,      '-' },
    { THOMK_PLUS,       '+' },
    { THOMK_ACCENT,     DANZEFF_ACCENT },
    { THOMK_SLASH,      '/' },
    { THOMK_ASTERISK,   '*' },
    { THOMK_COMMA,      ',' },
    { THOMK_PERIOD,     '.' },
    { THOMK_AT,         '@' },
    { THOMK_EXCLAMATN,  '!' },
    { THOMK_DBLQUOTE,   '"' },
    { THOMK_HASH,       '#' },
    { THOMK_DOLLAR,     '$' },
    { THOMK_PERCENT,    '%' },
    { THOMK_AMPERSAND,  '&' },
    { THOMK_QUOTE,      '\'' },
    { THOMK_LEFTPAREN,  '(' },
    { THOMK_RIGHTPAREN, ')' },
    { THOMK_BACKQUOTE,  '`' },
    { THOMK_EQUAL,      '=' },
    { THOMK_SEMICOLON,  ';' },
    { THOMK_QUESTION,   '?' },
    { THOMK_COLON,      ':' },
    { THOMK_LESS,       '<' },
    { THOMK_GREATER,    '>' },
    { THOMK_POWER,      '^' },
    { THOMK_STOP,       DANZEFF_STOP },
    { THOMK_CNT,        DANZEFF_CNT  },
    { THOMK_RAZ,        DANZEFF_RAZ  },
    { THOMK_SHIFT_L,    DANZEFF_SHIFT_L  },
    { THOMK_SHIFT_R,    DANZEFF_SHIFT_R  },
    { THOMK_A,          'A' },
    { THOMK_B,          'B' },
    { THOMK_C,          'C' },
    { THOMK_D,          'D' },
    { THOMK_E,          'E' },
    { THOMK_F,          'F' },
    { THOMK_G,          'G' },
    { THOMK_H,          'H' },
    { THOMK_I,          'I' },
    { THOMK_J,          'J' },
    { THOMK_K,          'K' },
    { THOMK_L,          'L' },
    { THOMK_M,          'M' },
    { THOMK_N,          'N' },
    { THOMK_O,          'O' },
    { THOMK_P,          'P' },
    { THOMK_Q,          'Q' },
    { THOMK_R,          'R' },
    { THOMK_S,          'S' },
    { THOMK_T,          'T' },
    { THOMK_U,          'U' },
    { THOMK_V,          'V' },
    { THOMK_W,          'W' },
    { THOMK_X,          'X' },
    { THOMK_Y,          'Y' },
    { THOMK_Z,          'Z' },
    { THOMK_SPACE,      ' '         },
    { THOMK_RETURN,     DANZEFF_RETURN   },
    { THOMK_TOP,        DANZEFF_TOP      },
    { THOMK_DELETE,     DANZEFF_DEL },
    { THOMK_INSERT,     DANZEFF_INS },
    { THOMK_CAPSLOCK,   DANZEFF_CAPSLOCK },
    { THOMK_UP,         -1  },
    { THOMK_DOWN,       -1  },
    { THOMK_LEFT,       -1  },
    { THOMK_RIGHT,      -1  }
  };

 int psp_kbd_mapping[ KBD_ALL_BUTTONS ];
 int psp_kbd_mapping_L[ KBD_ALL_BUTTONS ];
 int psp_kbd_mapping_R[ KBD_ALL_BUTTONS ];
 int psp_kbd_presses[ KBD_ALL_BUTTONS ];
 int kbd_ltrigger_mapping_active;
 int kbd_rtrigger_mapping_active;

 static int danzeff_thom_key     = 0;
 static int danzeff_thom_pending = 0;
 static int danzeff_mode        = 0;

#ifdef USE_PSP_IRKEYB
 static int irkeyb_thom_key      = 0;
 static int irkeyb_thom_pending  = 0;
# endif


       char command_keys[ 128 ];
 static int command_mode        = 0;
 static int command_index       = 0;
 static int command_size        = 0;
 static int command_thom_pending = 0;
 static int command_thom_key     = 0;

int
thom_key_event(int thom_idx, int press)
{
  int to7_id   = 0;

  if ((thom_idx >=           0) && 
      (thom_idx < THOMK_JOY_UP )) {
    to7_id = psp_thom_key_info[thom_idx].to7_id;

    if (press) {
      to7_HandleKeyPress(to7_id, 1);
    } else {
      to7_HandleKeyPress(to7_id, 0);
    }

  } else
  if ((thom_idx >= THOMK_JOY_UP) &&
      (thom_idx <= THOMK_JOY_FIRE)) {

    to7_id = psp_thom_key_info[thom_idx].to7_id;
    to7_HandleKeyPress( to7_id, press, 0);
  } else 
  if ((thom_idx >= THOMC_FPS) &&
      (thom_idx <= THOMC_SCREEN)) {

    if (press) {
      SceCtrlData c;
      myCtrlPeekBufferPositive(&c, 1);
      if ((c.TimeStamp - loc_last_hotkey_time) > KBD_MIN_HOTKEY_TIME) {
        loc_last_hotkey_time = c.TimeStamp;
        thom_treat_command_key(thom_idx);
      }
    }
  }
  return 0;
}

int 
thom_kbd_reset()
{
  to7_InputReset(0, 0);
  return 0;
}

int
thom_get_key_from_ascii(int key_ascii)
{
  int index;
  if ((key_ascii >= 'a') && (key_ascii <= 'z')) {
    key_ascii = (key_ascii - 'a') + 'A';
  }
  for (index = 0; index < KBD_MAX_ENTRIES; index++) {
   if (kbd_layout[index][1] == key_ascii) return kbd_layout[index][0];
  }
  return -1;
}

void
psp_kbd_run_command(char *Command)
{
  strncpy(command_keys, Command, 128);
  command_size  = strlen(Command);
  command_index = 0;

  command_thom_key     = 0;
  command_thom_pending = 0;
  command_mode        = 1;
}

int
psp_kbd_reset_mapping(void)
{
  memcpy(psp_kbd_mapping  , loc_default_mapping, sizeof(loc_default_mapping));
  memcpy(psp_kbd_mapping_L, loc_default_mapping_L, sizeof(loc_default_mapping_L));
  memcpy(psp_kbd_mapping_R, loc_default_mapping_R, sizeof(loc_default_mapping_R));
  return 0;
}

int
psp_kbd_reset_hotkeys(void)
{
  int index;
  int key_id;
  for (index = 0; index < KBD_ALL_BUTTONS; index++) {
    key_id = loc_default_mapping[index];
    if ((key_id >= THOMC_FPS) && (key_id <= THOMC_SCREEN)) {
      psp_kbd_mapping[index] = key_id;
    }
    key_id = loc_default_mapping_L[index];
    if ((key_id >= THOMC_FPS) && (key_id <= THOMC_SCREEN)) {
      psp_kbd_mapping_L[index] = key_id;
    }
    key_id = loc_default_mapping_R[index];
    if ((key_id >= THOMC_FPS) && (key_id <= THOMC_SCREEN)) {
      psp_kbd_mapping_R[index] = key_id;
    }
  }
  return 0;
}

int
psp_kbd_load_mapping_file(FILE *KbdFile)
{
  char     Buffer[512];
  char    *Scan;
  int      tmp_mapping[KBD_ALL_BUTTONS];
  int      tmp_mapping_L[KBD_ALL_BUTTONS];
  int      tmp_mapping_R[KBD_ALL_BUTTONS];
  int      thom_key_id = 0;
  int      kbd_id = 0;

  memcpy(tmp_mapping  , loc_default_mapping  , sizeof(loc_default_mapping));
  memcpy(tmp_mapping_L, loc_default_mapping_L, sizeof(loc_default_mapping_R));
  memcpy(tmp_mapping_R, loc_default_mapping_R, sizeof(loc_default_mapping_R));

  while (fgets(Buffer,512,KbdFile) != (char *)0) {
      
      Scan = strchr(Buffer,'\n');
      if (Scan) *Scan = '\0';
      /* For this #@$% of windows ! */
      Scan = strchr(Buffer,'\r');
      if (Scan) *Scan = '\0';
      if (Buffer[0] == '#') continue;

      Scan = strchr(Buffer,'=');
      if (! Scan) continue;
    
      *Scan = '\0';
      thom_key_id = atoi(Scan + 1);

      for (kbd_id = 0; kbd_id < KBD_ALL_BUTTONS; kbd_id++) {
        if (!strcasecmp(Buffer,loc_button_name[kbd_id])) {
          tmp_mapping[kbd_id] = thom_key_id;
          //break;
        }
      }
      for (kbd_id = 0; kbd_id < KBD_ALL_BUTTONS; kbd_id++) {
        if (!strcasecmp(Buffer,loc_button_name_L[kbd_id])) {
          tmp_mapping_L[kbd_id] = thom_key_id;
          //break;
        }
      }
      for (kbd_id = 0; kbd_id < KBD_ALL_BUTTONS; kbd_id++) {
        if (!strcasecmp(Buffer,loc_button_name_R[kbd_id])) {
          tmp_mapping_R[kbd_id] = thom_key_id;
          //break;
        }
      }
  }

  memcpy(psp_kbd_mapping, tmp_mapping, sizeof(psp_kbd_mapping));
  memcpy(psp_kbd_mapping_L, tmp_mapping_L, sizeof(psp_kbd_mapping_L));
  memcpy(psp_kbd_mapping_R, tmp_mapping_R, sizeof(psp_kbd_mapping_R));
  
  return 0;
}

int
psp_kbd_load_mapping(char *kbd_filename)
{
  FILE    *KbdFile;
  int      error = 0;

  KbdFile = fopen(kbd_filename, "r");
  error   = 1;

  if (KbdFile != (FILE*)0) {
  psp_kbd_load_mapping_file(KbdFile);
  error = 0;
    fclose(KbdFile);
  }

  kbd_ltrigger_mapping_active = 0;
  kbd_rtrigger_mapping_active = 0;
    
  return error;
}

int
psp_kbd_save_mapping(char *kbd_filename)
{
  FILE    *KbdFile;
  int      kbd_id = 0;
  int      error = 0;

  KbdFile = fopen(kbd_filename, "w");
  error   = 1;

  if (KbdFile != (FILE*)0) {

    for (kbd_id = 0; kbd_id < KBD_ALL_BUTTONS; kbd_id++)
    {
      fprintf(KbdFile, "%s=%d\n", loc_button_name[kbd_id], psp_kbd_mapping[kbd_id]);
    }
    for (kbd_id = 0; kbd_id < KBD_ALL_BUTTONS; kbd_id++)
    {
      fprintf(KbdFile, "%s=%d\n", loc_button_name_L[kbd_id], psp_kbd_mapping_L[kbd_id]);
    }
    for (kbd_id = 0; kbd_id < KBD_ALL_BUTTONS; kbd_id++)
    {
      fprintf(KbdFile, "%s=%d\n", loc_button_name_R[kbd_id], psp_kbd_mapping_R[kbd_id]);
    }
    error = 0;
    fclose(KbdFile);
  }

  return error;
}

int
psp_kbd_enter_command()
{
  SceCtrlData  c;

  unsigned int command_key = 0;
  int          thom_key     = 0;
  int          key_event   = 0;

  myCtrlPeekBufferPositive(&c, 1);

  if (command_thom_pending) 
  {
    if ((c.TimeStamp - loc_last_event_time) > KBD_MIN_COMMAND_TIME) {
      loc_last_event_time = c.TimeStamp;
      command_thom_pending = 0;
      thom_key_event(command_thom_key, 0);
    }

    return 0;
  }

  if ((c.TimeStamp - loc_last_event_time) > KBD_MIN_COMMAND_TIME) {
    loc_last_event_time = c.TimeStamp;

    if (command_index >= command_size) {

      command_mode  = 0;
      command_index = 0;
      command_size  = 0;

      command_thom_pending = 0;
      command_thom_key     = 0;

      return 0;
    }
  
    command_key = command_keys[command_index++];
    thom_key = thom_get_key_from_ascii(command_key);

    if (thom_key != -1) {
      command_thom_key     = thom_key;
      command_thom_pending = 1;
      thom_key_event(command_thom_key, 1);
    }

    return 1;
  }

  return 0;
}

int 
psp_kbd_is_danzeff_mode()
{
  return danzeff_mode;
}

int
psp_kbd_enter_danzeff()
{
  unsigned int danzeff_key = 0;
  int          thom_key     = 0;
  int          key_event   = 0;
  SceCtrlData  c;

  if (! danzeff_mode) {
    psp_init_keyboard();
    danzeff_mode = 1;
  }

  myCtrlPeekBufferPositive(&c, 1);
  c.Buttons &= PSP_ALL_BUTTON_MASK;

  if (danzeff_thom_pending) 
  {
    if ((c.TimeStamp - loc_last_event_time) > KBD_MIN_PENDING_TIME) {
      loc_last_event_time = c.TimeStamp;
      danzeff_thom_pending = 0;
      thom_key_event(danzeff_thom_key, 0);
    }
    return 0;
  }

  if ((c.TimeStamp - loc_last_event_time) > KBD_MIN_DANZEFF_TIME) {
    loc_last_event_time = c.TimeStamp;
  
    myCtrlPeekBufferPositive(&c, 1);
    c.Buttons &= PSP_ALL_BUTTON_MASK;
# ifdef USE_PSP_IRKEYB
    psp_irkeyb_set_psp_key(&c);
# endif
    danzeff_key = danzeff_readInput(c);
  }

  if (danzeff_key == DANZEFF_LEFT) {
    danzeff_key = DANZEFF_DEL;
  } else if (danzeff_key == DANZEFF_DOWN) {
    danzeff_key = DANZEFF_ENTER;
  } else if (danzeff_key == DANZEFF_RIGHT) {
  } else if (danzeff_key == DANZEFF_UP) {
  }

  if (danzeff_key > DANZEFF_START) {
    thom_key = thom_get_key_from_ascii(danzeff_key);

    if (thom_key != -1) {
      danzeff_thom_key     = thom_key;
      danzeff_thom_pending = 1;
      thom_key_event(danzeff_thom_key, 1);
    }

    return 1;

  } else if (danzeff_key == DANZEFF_START) {
    danzeff_mode       = 0;
    danzeff_thom_pending = 0;
    danzeff_thom_key     = 0;

    psp_kbd_wait_no_button();

  } else if (danzeff_key == DANZEFF_SELECT) {
    danzeff_mode       = 0;
    danzeff_thom_pending = 0;
    danzeff_thom_key     = 0;
    psp_main_menu();
    psp_init_keyboard();

    psp_kbd_wait_no_button();
  }

  return 0;
}

#ifdef USE_PSP_IRKEYB
int
psp_kbd_enter_irkeyb()
{
  int thom_key   = 0;
  int psp_irkeyb = PSP_IRKEYB_EMPTY;

  SceCtrlData  c;
  myCtrlPeekBufferPositive(&c, 1);

  if (irkeyb_thom_pending) 
  {
    if ((c.TimeStamp - loc_last_irkbd_event_time) > KBD_MIN_PENDING_TIME) {
      loc_last_irkbd_event_time = c.TimeStamp;
      irkeyb_thom_pending = 0;
      thom_key_event(irkeyb_thom_key, 0);
    }
    return 0;
  }

  psp_irkeyb = psp_irkeyb_read_key();
  if (psp_irkeyb != PSP_IRKEYB_EMPTY) {

    if (psp_irkeyb == 0x8) {
      thom_key = THOMK_DELETE;
    } else
    if (psp_irkeyb == 0x9) {
      thom_key = THOMK_RAZ;
    } else
    if (psp_irkeyb == 0xd) {
      thom_key = THOMK_RETURN;
    } else
    if (psp_irkeyb == 0x1b) {
      thom_key = THOMK_STOP;
    } else
    if (psp_irkeyb == PSP_IRKEYB_UP) {
      thom_key = THOMK_UP;
    } else
    if (psp_irkeyb == PSP_IRKEYB_DOWN) {
      thom_key = THOMK_DOWN;
    } else
    if (psp_irkeyb == PSP_IRKEYB_LEFT) {
      thom_key = THOMK_LEFT;
    } else
    if (psp_irkeyb == PSP_IRKEYB_RIGHT) {
      thom_key = THOMK_RIGHT;
    } else {
      thom_key = thom_get_key_from_ascii(psp_irkeyb);
    }
    if (thom_key != -1) {
      irkeyb_thom_key     = thom_key;
      irkeyb_thom_pending = 1;
      thom_key_event(thom_key, 1);
    }
    return 1;
  }
  return 0;
}
# endif

void
psp_kbd_display_active_mapping()
{
  if (kbd_ltrigger_mapping_active) {
    psp_sdl_fill_rectangle(0, 0, 10, 3, psp_sdl_rgb(0x0, 0x0, 0xff), 0);
  } else {
    psp_sdl_fill_rectangle(0, 0, 10, 3, 0x0, 0);
  }

  if (kbd_rtrigger_mapping_active) {
    psp_sdl_fill_rectangle(470, 0, 10, 3, psp_sdl_rgb(0x0, 0x0, 0xff), 0);
  } else {
    psp_sdl_fill_rectangle(470, 0, 10, 3, 0x0, 0);
  }
}

int
thom_decode_key(int psp_b, int button_pressed)
{
  int wake = 0;
  int reverse_analog = THOM.psp_reverse_analog;

  if (reverse_analog) {
    if ((psp_b >= KBD_JOY_UP  ) &&
        (psp_b <= KBD_JOY_LEFT)) {
      psp_b = psp_b - KBD_JOY_UP + KBD_UP;
    } else
    if ((psp_b >= KBD_UP  ) &&
        (psp_b <= KBD_LEFT)) {
      psp_b = psp_b - KBD_UP + KBD_JOY_UP;
    }
  }

  if (psp_b == KBD_START) {
     if (button_pressed) psp_kbd_enter_danzeff();
  } else
  if (psp_b == KBD_SELECT) {
    if (button_pressed) {
      psp_main_menu();
      psp_init_keyboard();
    }
  } else {
 
    if (psp_kbd_mapping[psp_b] >= 0) {
      wake = 1;
      if (button_pressed) {
        // Determine which buton to press first (ie which mapping is currently active)
        if (kbd_ltrigger_mapping_active) {
          // Use ltrigger mapping
          psp_kbd_presses[psp_b] = psp_kbd_mapping_L[psp_b];
          thom_key_event(psp_kbd_presses[psp_b], button_pressed);
        } else
        if (kbd_rtrigger_mapping_active) {
          // Use rtrigger mapping
          psp_kbd_presses[psp_b] = psp_kbd_mapping_R[psp_b];
          thom_key_event(psp_kbd_presses[psp_b], button_pressed);
        } else {
          // Use standard mapping
          psp_kbd_presses[psp_b] = psp_kbd_mapping[psp_b];
          thom_key_event(psp_kbd_presses[psp_b], button_pressed);
        }
      } else {
          // Determine which button to release (ie what was pressed before)
          thom_key_event(psp_kbd_presses[psp_b], button_pressed);
      }

    } else {
      if (psp_kbd_mapping[psp_b] == KBD_LTRIGGER_MAPPING) {
        kbd_ltrigger_mapping_active = button_pressed;
        kbd_rtrigger_mapping_active = 0;
      } else
      if (psp_kbd_mapping[psp_b] == KBD_RTRIGGER_MAPPING) {
        kbd_rtrigger_mapping_active = button_pressed;
        kbd_ltrigger_mapping_active = 0;
      }
    }
  }
  return 0;
}

# define ANALOG_THRESHOLD 60

void 
kbd_get_analog_direction(int Analog_x, int Analog_y, int *x, int *y)
{
  int DeltaX = 255;
  int DeltaY = 255;
  int DirX   = 0;
  int DirY   = 0;

  *x = 0;
  *y = 0;

  if (Analog_x <=        ANALOG_THRESHOLD)  { DeltaX = Analog_x; DirX = -1; }
  else 
  if (Analog_x >= (255 - ANALOG_THRESHOLD)) { DeltaX = 255 - Analog_x; DirX = 1; }

  if (Analog_y <=        ANALOG_THRESHOLD)  { DeltaY = Analog_y; DirY = -1; }
  else 
  if (Analog_y >= (255 - ANALOG_THRESHOLD)) { DeltaY = 255 - Analog_y; DirY = 1; }

  *x = DirX;
  *y = DirY;
}

void
kbd_change_auto_fire(int auto_fire)
{
  THOM.thom_auto_fire = auto_fire;
  if (THOM.thom_auto_fire_pressed) {
    thom_key_event(THOMK_JOY_FIRE, 0);
    THOM.thom_auto_fire_pressed = 0;
  }
}


static int 
kbd_reset_button_status(void)
{
  int b = 0;
  /* Reset Button status */
  for (b = 0; b < KBD_MAX_BUTTONS; b++) {
    loc_button_press[b]   = 0;
    loc_button_release[b] = 0;
  }
  psp_init_keyboard();
  return 0;
}

int
kbd_scan_keyboard(void)
{
  SceCtrlData c;
  long        delta_stamp;
  int         event;
  int         b;

  int new_Lx;
  int new_Ly;
  int old_Lx;
  int old_Ly;

  event = 0;
  myCtrlPeekBufferPositive( &c, 1 );
  c.Buttons &= PSP_ALL_BUTTON_MASK;

# ifdef USE_PSP_IRKEYB
  psp_irkeyb_set_psp_key(&c);
# endif

  if ((c.Buttons & (PSP_CTRL_LTRIGGER|PSP_CTRL_RTRIGGER|PSP_CTRL_START)) ==
      (PSP_CTRL_LTRIGGER|PSP_CTRL_RTRIGGER|PSP_CTRL_START)) {
    /* Exit ! */
    psp_sdl_exit(0);
  }

  delta_stamp = c.TimeStamp - first_time_stamp;
  if ((delta_stamp < 0) || (delta_stamp > KBD_MIN_BATTCHECK_TIME)) {
    first_time_stamp = c.TimeStamp;
    if (psp_is_low_battery()) {
      psp_main_menu();
      psp_init_keyboard();
      return 0;
    }
  }

  if (THOM.thom_auto_fire) {
    delta_stamp = c.TimeStamp - first_time_auto_stamp;
    if ((delta_stamp < 0) || 
        (delta_stamp > (KBD_MIN_AUTOFIRE_TIME / (1 + THOM.thom_auto_fire_period)))) {
      first_time_auto_stamp = c.TimeStamp;
      thom_key_event(THOMK_JOY_FIRE, THOM.thom_auto_fire_pressed);
      THOM.thom_auto_fire_pressed = ! THOM.thom_auto_fire_pressed;
    }
  }

  /* Check Analog Device */
  kbd_get_analog_direction(loc_button_data.Lx,loc_button_data.Ly,&old_Lx,&old_Ly);
  kbd_get_analog_direction( c.Lx, c.Ly, &new_Lx, &new_Ly);

  /* Analog device has moved */
  if (new_Lx > 0) {
    if (old_Lx <  0) thom_decode_key(KBD_JOY_LEFT , 0);
    if (old_Lx <= 0) thom_decode_key(KBD_JOY_RIGHT, 1);
  } else 
  if (new_Lx < 0) {
    if (old_Lx >  0) thom_decode_key(KBD_JOY_RIGHT, 0);
    if (old_Lx >= 0) thom_decode_key(KBD_JOY_LEFT , 1);
  } else {
    if (old_Lx >  0) thom_decode_key(KBD_JOY_RIGHT , 0);
    else
    if (old_Lx <  0) thom_decode_key(KBD_JOY_LEFT, 0);
  }

  if (new_Ly > 0) {
    if (old_Ly <  0) thom_decode_key(KBD_JOY_UP , 0);
    if (old_Ly <= 0) thom_decode_key(KBD_JOY_DOWN, 1);
  } else 
  if (new_Ly < 0) {
    if (old_Ly >  0) thom_decode_key(KBD_JOY_DOWN, 0);
    if (old_Ly >= 0) thom_decode_key(KBD_JOY_UP , 1);
  } else {
    if (old_Ly >  0) thom_decode_key(KBD_JOY_DOWN , 0);
    else
    if (old_Ly <  0) thom_decode_key(KBD_JOY_UP, 0);
  }

  for (b = 0; b < KBD_MAX_BUTTONS; b++) 
  {
    if (c.Buttons & loc_button_mask[b]) {
# if 0 // GAME MODE !
      if (!(loc_button_data.Buttons & loc_button_mask[b])) 
# endif
      {
        loc_button_press[b] = 1;
        event = 1;
      }
    } else {
      if (loc_button_data.Buttons & loc_button_mask[b]) {
        loc_button_release[b] = 1;
        loc_button_press[b] = 0;
        event = 1;
      }
    }
  }
  memcpy(&loc_button_data,&c,sizeof(SceCtrlData));

  return event;
}

void
psp_kbd_wait_start(void)
{
  while (1)
  {
    SceCtrlData c;
    myCtrlPeekBufferPositive(&c, 1);
    c.Buttons &= PSP_ALL_BUTTON_MASK;
    if (c.Buttons & PSP_CTRL_START) break;
  }
  psp_kbd_wait_no_button();
}

void
psp_init_keyboard(void)
{
  thom_kbd_reset();
  kbd_ltrigger_mapping_active = 0;
  kbd_rtrigger_mapping_active = 0;
}

void
psp_kbd_wait_no_button(void)
{
  SceCtrlData c;

  do {
   myCtrlPeekBufferPositive(&c, 1);
   c.Buttons &= PSP_ALL_BUTTON_MASK;

  } while (c.Buttons != 0);
} 

void
psp_kbd_wait_button(void)
{
  SceCtrlData c;

  do {
   myCtrlPeekBufferPositive(&c, 1);
  } while (c.Buttons == 0);
} 

int
psp_update_keys(void)
{
  int         b;

  static char first_time = 1;
  static int release_pending = 0;

  if (first_time) {

    memcpy(psp_kbd_mapping, loc_default_mapping, sizeof(loc_default_mapping));
    memcpy(psp_kbd_mapping_L, loc_default_mapping_L, sizeof(loc_default_mapping_L));
    memcpy(psp_kbd_mapping_R, loc_default_mapping_R, sizeof(loc_default_mapping_R));

    thom_kbd_load();

    SceCtrlData c;
    myCtrlPeekBufferPositive(&c, 1);
    c.Buttons &= PSP_ALL_BUTTON_MASK;

    if (first_time_stamp == -1) first_time_stamp = c.TimeStamp;
    if ((! c.Buttons) && ((c.TimeStamp - first_time_stamp) < KBD_MIN_START_TIME)) return 0;

    first_time      = 0;
    release_pending = 0;

    for (b = 0; b < KBD_MAX_BUTTONS; b++) {
      loc_button_release[b] = 0;
      loc_button_press[b] = 0;
    }
    myCtrlPeekBufferPositive(&loc_button_data, 1);
    loc_button_data.Buttons &= PSP_ALL_BUTTON_MASK;

    psp_main_menu();
    psp_init_keyboard();

    return 0;
  }

  thom_apply_cheats();

  if (command_mode) {
    return psp_kbd_enter_command();
  }

  if (danzeff_mode) {
    return psp_kbd_enter_danzeff();
  }

# ifdef USE_PSP_IRKEYB
  if (psp_kbd_enter_irkeyb()) {
    return 1;
  }
# endif

  if (release_pending)
  {
    release_pending = 0;
    for (b = 0; b < KBD_MAX_BUTTONS; b++) {
      if (loc_button_release[b]) {
        loc_button_release[b] = 0;
        loc_button_press[b] = 0;
        thom_decode_key(b, 0);
      }
    }
  }

  kbd_scan_keyboard();

  /* check press event */
  for (b = 0; b < KBD_MAX_BUTTONS; b++) {
    if (loc_button_press[b]) {
      loc_button_press[b] = 0;
      release_pending     = 0;
      thom_decode_key(b, 1);
    }
  }
  /* check release event */
  for (b = 0; b < KBD_MAX_BUTTONS; b++) {
    if (loc_button_release[b]) {
      release_pending = 1;
      break;
    } 
  }

  return 0;
}
