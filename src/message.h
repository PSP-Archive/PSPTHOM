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
 *  Module     : linux/message.h
 *  Version    : 1.5.5
 *  Créé par   : Eric Botcazou juillet 1999
 *  Modifié par:
 *
 *  Classe Message d'extension du toolkit GTK+.
 */


#ifndef LINUX_MESSAGE_H
#define LINUX_MESSAGE_H

#ifndef SCAN_DEPEND
#endif


#define MESSAGE(obj)          GTK_CHECK_CAST(obj, message_get_type(), Message)
#define MESSAGE_CLASS(klass)  GTK_CHECK_CLASS_CAST(klass, message_get_type(), MessageClass)
#define IS_MESSAGE(obj)       GTK_CHECK_TYPE(obj, message_get_type())

typedef struct _Message       Message;
typedef struct _MessageClass  MessageClass;

struct _Message {
    GtkWindow window;

    GtkWidget *label;
};

struct _MessageClass {
    GtkWindowClass parent_class;
};

extern guint      message_get_type(void);
extern GtkWidget *message_new(const gchar *);

#endif
