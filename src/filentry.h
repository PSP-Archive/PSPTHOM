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
 *  Module     : linux/filentry.h
 *  Version    : 1.5.5
 *  Créé par   : Eric Botcazou juillet 1999
 *  Modifié par: Eric Botcazou 29/07/2000
 *
 *  Classe FileEntry d'extension du toolkit GTK+.
 */


#ifndef LINUX_FILENTRY_H
#define LINUX_FILENTRY_H

#ifndef SCAN_DEPEND
   #include <gtk/gtkhbox.h>
#endif


#define FILE_ENTRY(obj)          GTK_CHECK_CAST(obj, file_entry_get_type(), FileEntry)
#define FILE_ENTRY_CLASS(klass)  GTK_CHECK_CLASS_CAST(klass, file_entry_get_type(), FileEntryClass)
#define IS_FILE_ENTRY(obj)       GTK_CHECK_TYPE(obj, file_entry_get_type())

typedef struct _FileEntry       FileEntry;
typedef struct _FileEntryClass  FileEntryClass;

struct _FileEntry {
    GtkHBox hbox;
  
    GtkWidget *label;
    GtkWidget *entry;
    GtkWidget *button;
    GtkWidget *filesel;

#define FILENT_LENGTH  127
    gchar filename[FILENT_LENGTH+1];
};

struct _FileEntryClass {
    GtkHBoxClass parent_class;

    void (*file_selected)(FileEntry *);
};

extern guint      file_entry_get_type(void);
extern GtkWidget *file_entry_new(const gchar *);
extern void       file_entry_set_entry(FileEntry *, const gchar *);
extern void       file_entry_set_filename(FileEntry *, const gchar *);
extern gchar     *file_entry_get_filename(FileEntry *);    

#endif
