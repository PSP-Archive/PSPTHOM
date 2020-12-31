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
 *  Module     : to7keys.h
 *  Version    : 1.5.5
 *  Créé par   : Eric Botcazou
 *  Modifié par: Eric Botcazou 06/10/2001
 *
 *  scancodes des touches passés par le handler d'Allegro 4.
 */


#ifndef TO7KEYS_H
#define TO7KEYS_H

 enum to7_keys_emum {

    TO7_KEY_1,           
    TO7_KEY_2,           
    TO7_KEY_3,           
    TO7_KEY_4,           
    TO7_KEY_5,           
    TO7_KEY_6,           
    TO7_KEY_7,           
    TO7_KEY_8,           
    TO7_KEY_9,           
    TO7_KEY_0,           
    TO7_KEY_MINUS,       
    TO7_KEY_PLUS,        
    TO7_KEY_ACCENT,      
    TO7_KEY_SLASH,       
    TO7_KEY_ASTERISK,    
    TO7_KEY_COMMA,       
    TO7_KEY_PERIOD,      
    TO7_KEY_AT,          

    TO7_KEY_EXCLAMATN,   
    TO7_KEY_DBLQUOTE,    
    TO7_KEY_HASH,        
    TO7_KEY_DOLLAR,      
    TO7_KEY_PERCENT,     
    TO7_KEY_AMPERSAND,   
    TO7_KEY_QUOTE,       
    TO7_KEY_LEFTPAREN,   
    TO7_KEY_RIGHTPAREN,  
    TO7_KEY_BACKQUOTE,   
    TO7_KEY_EQUAL,       
    TO7_KEY_SEMICOLON,   
    TO7_KEY_QUESTION,    
    TO7_KEY_COLON,       
    TO7_KEY_LESS,        
    TO7_KEY_GREATER,     
    TO7_KEY_POWER,       


    TO7_KEY_STOP,        
    TO7_KEY_CNT   ,      
    TO7_KEY_RAZ   ,      
    TO7_KEY_SHIFT_L,     
    TO7_KEY_SHIFT_R,     

    TO7_KEY_A,           
    TO7_KEY_B,           
    TO7_KEY_C,           
    TO7_KEY_D,           
    TO7_KEY_E,           
    TO7_KEY_F,           
    TO7_KEY_G,           
    TO7_KEY_H,           
    TO7_KEY_I,           
    TO7_KEY_J,           
    TO7_KEY_K,           
    TO7_KEY_L,           
    TO7_KEY_M,           
    TO7_KEY_N,           
    TO7_KEY_O,           
    TO7_KEY_P,           
    TO7_KEY_Q,           
    TO7_KEY_R,           
    TO7_KEY_S,           
    TO7_KEY_T,           
    TO7_KEY_U,           
    TO7_KEY_V,           
    TO7_KEY_W,           
    TO7_KEY_X,           
    TO7_KEY_Y,           
    TO7_KEY_Z,           

    TO7_KEY_SPACE,       
    TO7_KEY_RETURN,      
    TO7_KEY_TOP,         

    TO7_KEY_DEL   ,      
    TO7_KEY_INS   ,      

    TO7_KEY_CAPSLOCK,    

    TO7_KEY_LEFT,        
    TO7_KEY_UP,          
    TO7_KEY_RIGHT,       
    TO7_KEY_DOWN,        

    TO7_KEY_JOY_UP,      
    TO7_KEY_JOY_DOWN,    
    TO7_KEY_JOY_LEFT,    
    TO7_KEY_JOY_RIGHT,   

    TO7_KEY_JOY_FIRE,   

    TO7_MAX_KEY
  };

#endif
