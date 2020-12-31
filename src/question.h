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
 *  Module     : linux/question.h
 *  Version    : 1.5.5
 *  Créé par   : Eric Botcazou juillet 1999
 *  Modifié par:
 *
 *  Classe Question d'extension du toolkit GTK+.
 */


#ifndef LINUX_QUESTION_H
#define LINUX_QUESTION_H

#ifndef SCAN_DEPEND
   #include <gtk/gtkwindow.h>
#endif


#define QUESTION(obj)          GTK_CHECK_CAST(obj, question_get_type(), Question)
#define QUESTION_CLASS(klass)  GTK_CHECK_CLASS_CAST(klass, question_get_type(), QuestionClass)
#define IS_QUESTION(obj)       GTK_CHECK_TYPE(obj, question_get_type())

typedef struct _Question       Question;
typedef struct _QuestionClass  QuestionClass;

struct _Question {
    GtkWindow window;

    GtkWidget *label;
};

struct _QuestionClass {
    GtkWindowClass parent_class;

    void (*clicked_yes)(Question *);
    void (*clicked_no) (Question *);
};

extern guint      question_get_type(void);
extern GtkWidget *question_new(const gchar *);

#endif
