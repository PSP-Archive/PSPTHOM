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
 *                      l'�mulateur Thomson TO7-70
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
 *  Module     : errors.c
 *  Version    : 1.5.5
 *  Cr�� par   : Eric Botcazou 15/12/2000
 *  Modifi� par: Eric Botcazou 10/10/2001
 *
 *  Gestion des erreurs g�n�r�es par l'�mulateur.
 */


#ifndef SCAN_DEPEND
   #include <stddef.h>
   #include <string.h>
#endif

#include "intern/errors.h"  /* MacOS */
#include "to7err.h"
#include "to7.h"


static const char *default_error_table[TO7_ERROR_MAX]= {
    "Erreur: initialisation multiple de l'�mulateur.",
    "Erreur: allocation d'espace impossible.",
    "Erreur: impossible de trouver ",
    "Erreur: ouverture impossible.",  
    "Erreur: mauvais format de fichier.",
    "Erreur: p�riph�rique non support�."
};

static const char **custom_error_table = NULL;



/* ErrorMessage:
 *  Renvoie une erreur g�n�r�e par l'�mulateur.
 */
int ErrorMessage(int error, const char moreinfo[])
{
    if (custom_error_table)
        strcpy(to7_error_msg, custom_error_table[error]);
    else
        strcpy(to7_error_msg, default_error_table[error]);

    if (moreinfo)
        strcat(to7_error_msg, moreinfo);
 
    return TO7_ERROR;
}



/**********************************/
/* partie publique                */
/**********************************/


char to7_error_msg[128];  /* 127 caract�res au maximum */


/* RegisterErrorTable:
 *  Inscrit une table d'erreur personnalis�e.
 */
void to7_RegisterErrorTable(const char *table[])
{
    custom_error_table = table;
}

