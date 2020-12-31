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
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <time.h>

#include <zlib.h>
#include <psppower.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <pspuser.h>
#include "global.h"
#include "to7.h"
#include "psp_fmgr.h"
#include "psp_kbd.h"
#include "psp_sdl.h"

#include <pspctrl.h>
#include <pspkernel.h>
#include <pspdebug.h>
#include <pspiofilemgr.h>

//LUDO:
  THOM_t THOM;

void
thom_default_settings()
{
  THOM.thom_snd_enable     = 1;
  THOM.thom_render_mode    = THOM_RENDER_NORMAL;
  THOM.thom_vsync = 0;
  THOM.psp_reverse_analog  = 0;
  THOM.psp_cpu_clock       = 222;
  THOM.psp_display_lr      = 0;
  THOM.thom_speed_limiter  = 50;
  THOM.thom_view_fps       = 0;
  THOM.thom_auto_fire        = 0;
  THOM.thom_auto_fire_period = 6;
  THOM.thom_auto_fire_pressed = 0;
  THOM.psp_screenshot_id   = 0;
  THOM.thom_delta_y           = 0;
  THOM.psp_active_joystick = 0;

  scePowerSetClockFrequency(THOM.psp_cpu_clock, THOM.psp_cpu_clock, THOM.psp_cpu_clock/2);
}

int
loc_thom_save_settings(char *chFileName)
{
  FILE* FileDesc;
  int   error = 0;

  FileDesc = fopen(chFileName, "w");
  if (FileDesc != (FILE *)0 ) {

    fprintf(FileDesc, "psp_cpu_clock=%d\n"       , THOM.psp_cpu_clock);
    fprintf(FileDesc, "psp_reverse_analog=%d\n"  , THOM.psp_reverse_analog);
    fprintf(FileDesc, "psp_skip_max_frame=%d\n"  , THOM.psp_skip_max_frame);
    fprintf(FileDesc, "psp_display_lr=%d\n"      , THOM.psp_display_lr);
    fprintf(FileDesc, "thom_snd_enable=%d\n"     , THOM.thom_snd_enable);
    fprintf(FileDesc, "thom_render_mode=%d\n"    , THOM.thom_render_mode);
    fprintf(FileDesc, "thom_speed_limiter=%d\n"  , THOM.thom_speed_limiter);
    fprintf(FileDesc, "thom_view_fps=%d\n"       , THOM.thom_view_fps);
    fprintf(FileDesc, "thom_delta_y=%d\n"        , THOM.thom_delta_y);
    fprintf(FileDesc, "thom_vsync=%d\n"          , THOM.thom_vsync);
    fprintf(FileDesc, "thom_auto_fire_period=%d\n", THOM.thom_auto_fire_period);

    fclose(FileDesc);

  } else {
    error = 1;
  }

  return error;
}

int
thom_save_settings(void)
{
  char  FileName[MAX_PATH+1];
  int   error;

  error = 1;

  snprintf(FileName, MAX_PATH, "%s/set/%s.set", THOM.thom_home_dir, THOM.thom_save_name);
  error = loc_thom_save_settings(FileName);

  return error;
}

static int
loc_thom_load_settings(char *chFileName)
{
  char  Buffer[512];
  char *Scan;
  unsigned int Value;
  FILE* FileDesc;

  FileDesc = fopen(chFileName, "r");
  if (FileDesc == (FILE *)0 ) return 0;

  while (fgets(Buffer,512, FileDesc) != (char *)0) {

    Scan = strchr(Buffer,'\n');
    if (Scan) *Scan = '\0';
    /* For this #@$% of windows ! */
    Scan = strchr(Buffer,'\r');
    if (Scan) *Scan = '\0';
    if (Buffer[0] == '#') continue;

    Scan = strchr(Buffer,'=');
    if (! Scan) continue;

    *Scan = '\0';
    Value = atoi(Scan+1);

    if (!strcasecmp(Buffer,"psp_cpu_clock"))      THOM.psp_cpu_clock = Value;
    else
    if (!strcasecmp(Buffer,"psp_reverse_analog")) THOM.psp_reverse_analog = Value;
    else
    if (!strcasecmp(Buffer,"psp_skip_max_frame")) THOM.psp_skip_max_frame = Value;
    else
    if (!strcasecmp(Buffer,"psp_display_lr"))     THOM.psp_display_lr = Value;
    else
    if (!strcasecmp(Buffer,"thom_snd_enable"))     THOM.thom_snd_enable = Value;
    else
    if (!strcasecmp(Buffer,"thom_render_mode"))    THOM.thom_render_mode = Value;
    else
    if (!strcasecmp(Buffer,"thom_speed_limiter"))  THOM.thom_speed_limiter = Value;
    else
    if (!strcasecmp(Buffer,"thom_view_fps"))  THOM.thom_view_fps = Value;
    else
    if (!strcasecmp(Buffer,"thom_delta_y"))  THOM.thom_delta_y = Value;
    else
    if (!strcasecmp(Buffer,"thom_vsync"))  THOM.thom_vsync = Value;
    else
    if (!strcasecmp(Buffer,"thom_auto_fire_period"))  THOM.thom_auto_fire_period = Value;
  }

  fclose(FileDesc);

  scePowerSetClockFrequency(THOM.psp_cpu_clock, THOM.psp_cpu_clock, THOM.psp_cpu_clock/2);

  return 0;
}

int
thom_load_settings()
{
  char  FileName[MAX_PATH+1];
  int   error;

  error = 1;

  snprintf(FileName, MAX_PATH, "%s/set/%s.set", THOM.thom_home_dir, THOM.thom_save_name);
  error = loc_thom_load_settings(FileName);

  return error;
}

int
thom_load_file_settings(char *FileName)
{
  return loc_thom_load_settings(FileName);
}

static int 
loc_load_k7(char *TmpName)
{
  int error;

  error = to7_LoadK7(TmpName);
  if (! error) {
    THOM.thom_load_mode = THOM_LOAD_K7_MODE;
  }
  return error;
}

static int 
loc_load_sap(char *TmpName, int disk_id)
{
  int error;

  error = to7_LoadDisk(disk_id, TmpName);
  if (! error) {
    THOM.thom_load_mode = THOM_LOAD_DISK_MODE;
  }
  return error;
}

void
thom_update_save_name(char *Name)
{
  char        TmpFileName[MAX_PATH];
# ifdef LINUX_MODE
  struct stat aStat;
# else
  SceIoStat   aStat;
# endif
  int         index;
  char       *SaveName;
  char       *Scan1;
  char       *Scan2;

  SaveName = strrchr(Name,'/');
  if (SaveName != (char *)0) SaveName++;
  else                       SaveName = Name;

  if (!strncasecmp(SaveName, "sav_", 4)) {
    Scan1 = SaveName + 4;
    Scan2 = strrchr(Scan1, '_');
    if (Scan2 && (Scan2[1] >= '0') && (Scan2[1] <= '5')) {
      strncpy(THOM.thom_save_name, Scan1, MAX_PATH);
      THOM.thom_save_name[Scan2 - Scan1] = '\0';
    } else {
      strncpy(THOM.thom_save_name, SaveName, MAX_PATH);
    }
  } else {
    strncpy(THOM.thom_save_name, SaveName, MAX_PATH);
  }

  if (THOM.thom_save_name[0] == '\0') {
    strcpy(THOM.thom_save_name,"default");
  }

  for (index = 0; index < THOM_MAX_SAVE_STATE; index++) {
    THOM.thom_save_state[index].used  = 0;
    memset(&THOM.thom_save_state[index].date, 0, sizeof(ScePspDateTime));
    THOM.thom_save_state[index].thumb = 0;

    snprintf(TmpFileName, MAX_PATH, "%s/save/sav_%s_%d.stz", THOM.thom_home_dir, THOM.thom_save_name, index);
# ifdef LINUX_MODE
    if (! stat(TmpFileName, &aStat)) 
# else
    if (! sceIoGetstat(TmpFileName, &aStat))
# endif
    {
      THOM.thom_save_state[index].used = 1;
      THOM.thom_save_state[index].date = aStat.st_mtime;
      snprintf(TmpFileName, MAX_PATH, "%s/save/sav_%s_%d.png", THOM.thom_home_dir, THOM.thom_save_name, index);
# ifdef LINUX_MODE
      if (! stat(TmpFileName, &aStat)) 
# else
      if (! sceIoGetstat(TmpFileName, &aStat))
# endif
      {
        if (psp_sdl_load_thumb_png(THOM.thom_save_state[index].surface, TmpFileName)) {
          THOM.thom_save_state[index].thumb = 1;
        }
      }
    }
  }

  THOM.comment_present = 0;
  snprintf(TmpFileName, MAX_PATH, "%s/txt/%s.txt", THOM.thom_home_dir, THOM.thom_save_name);
# ifdef LINUX_MODE
  if (! stat(TmpFileName, &aStat)) 
# else
  if (! sceIoGetstat(TmpFileName, &aStat))
# endif
  {
    THOM.comment_present = 1;
  }
}

void
reset_save_name()
{
  thom_update_save_name("");
}

typedef struct thumb_list {
  struct thumb_list *next;
  char              *name;
  char              *thumb;
} thumb_list;

static thumb_list* loc_head_thumb = 0;

static void
loc_del_thumb_list()
{
  while (loc_head_thumb != 0) {
    thumb_list *del_elem = loc_head_thumb;
    loc_head_thumb = loc_head_thumb->next;
    if (del_elem->name) free( del_elem->name );
    if (del_elem->thumb) free( del_elem->thumb );
    free(del_elem);
  }
}

static void
loc_add_thumb_list(char* filename)
{
  thumb_list *new_elem;
  char tmp_filename[MAX_PATH];

  strcpy(tmp_filename, filename);
  char* save_name = tmp_filename;

  /* .png extention */
  char* Scan = strrchr(save_name, '.');
  if ((! Scan) || (strcasecmp(Scan, ".png"))) return;
  *Scan = 0;

  if (strncasecmp(save_name, "sav_", 4)) return;
  save_name += 4;

  Scan = strrchr(save_name, '_');
  if (! Scan) return;
  *Scan = 0;

  /* only one png for a given save name */
  new_elem = loc_head_thumb;
  while (new_elem != 0) {
    if (! strcasecmp(new_elem->name, save_name)) return;
    new_elem = new_elem->next;
  }

  new_elem = (thumb_list *)malloc( sizeof( thumb_list ) );
  new_elem->next = loc_head_thumb;
  loc_head_thumb = new_elem;
  new_elem->name  = strdup( save_name );
  new_elem->thumb = strdup( filename );
}

void
load_thumb_list()
{
# ifndef LINUX_MODE
  char SaveDirName[MAX_PATH];
  struct SceIoDirent a_dirent;
  int fd = 0;

  loc_del_thumb_list();

  snprintf(SaveDirName, MAX_PATH, "%s/save", THOM.thom_home_dir);
  memset(&a_dirent, 0, sizeof(a_dirent));

  fd = sceIoDopen(SaveDirName);
  if (fd < 0) return;

  while (sceIoDread(fd, &a_dirent) > 0) {
    if(a_dirent.d_name[0] == '.') continue;
    if(! FIO_S_ISDIR(a_dirent.d_stat.st_mode)) 
    {
      loc_add_thumb_list( a_dirent.d_name );
    }
  }
  sceIoDclose(fd);
# else
  char SaveDirName[MAX_PATH];
  DIR* fd = 0;

  loc_del_thumb_list();

  snprintf(SaveDirName, MAX_PATH, "%s/save", THOM.thom_home_dir);

  fd = opendir(SaveDirName);
  if (!fd) return;

  struct dirent *a_dirent;
  while ((a_dirent = readdir(fd)) != 0) {
    if(a_dirent->d_name[0] == '.') continue;
    if (a_dirent->d_type != DT_DIR) 
    {
      loc_add_thumb_list( a_dirent->d_name );
    }
  }
  closedir(fd);
# endif
}

int
load_thumb_if_exists(char *Name)
{
  char        FileName[MAX_PATH];
  char        ThumbFileName[MAX_PATH];
# ifdef LINUX_MODE
  struct stat aStat;
# else
  SceIoStat   aStat;
# endif
  char       *SaveName;
  char       *Scan;

  strcpy(FileName, Name);
  SaveName = strrchr(FileName,'/');
  if (SaveName != (char *)0) SaveName++;
  else                       SaveName = FileName;

  Scan = strrchr(SaveName,'.');
  if (Scan) *Scan = '\0';

  if (!SaveName[0]) return 0;

  thumb_list *scan_list = loc_head_thumb;
  while (scan_list != 0) {
    if (! strcasecmp( SaveName, scan_list->name)) {
      snprintf(ThumbFileName, MAX_PATH, "%s/save/%s", THOM.thom_home_dir, scan_list->thumb);
# ifdef LINUX_MODE
      if (! stat(ThumbFileName, &aStat)) 
# else
      if (! sceIoGetstat(ThumbFileName, &aStat))
# endif
      {
        if (psp_sdl_load_thumb_png(save_surface, ThumbFileName)) {
          return 1;
        }
      }
    }
    scan_list = scan_list->next;
  }
  return 0;
}

typedef struct comment_list {
  struct comment_list *next;
  char              *name;
  char              *filename;
} comment_list;

static comment_list* loc_head_comment = 0;

static void
loc_del_comment_list()
{
  while (loc_head_comment != 0) {
    comment_list *del_elem = loc_head_comment;
    loc_head_comment = loc_head_comment->next;
    if (del_elem->name) free( del_elem->name );
    if (del_elem->filename) free( del_elem->filename );
    free(del_elem);
  }
}

static void
loc_add_comment_list(char* filename)
{
  comment_list *new_elem;
  char  tmp_filename[MAX_PATH];

  strcpy(tmp_filename, filename);
  char* save_name = tmp_filename;

  /* .png extention */
  char* Scan = strrchr(save_name, '.');
  if ((! Scan) || (strcasecmp(Scan, ".txt"))) return;
  *Scan = 0;

  /* only one txt for a given save name */
  new_elem = loc_head_comment;
  while (new_elem != 0) {
    if (! strcasecmp(new_elem->name, save_name)) return;
    new_elem = new_elem->next;
  }

  new_elem = (comment_list *)malloc( sizeof( comment_list ) );
  new_elem->next = loc_head_comment;
  loc_head_comment = new_elem;
  new_elem->name  = strdup( save_name );
  new_elem->filename = strdup( filename );
}

void
load_comment_list()
{
# ifndef LINUX_MODE
  char SaveDirName[MAX_PATH];
  struct SceIoDirent a_dirent;
  int fd = 0;

  loc_del_comment_list();

  snprintf(SaveDirName, MAX_PATH, "%s/txt", THOM.thom_home_dir);
  memset(&a_dirent, 0, sizeof(a_dirent));

  fd = sceIoDopen(SaveDirName);
  if (fd < 0) return;

  while (sceIoDread(fd, &a_dirent) > 0) {
    if(a_dirent.d_name[0] == '.') continue;
    if(! FIO_S_ISDIR(a_dirent.d_stat.st_mode)) 
    {
      loc_add_comment_list( a_dirent.d_name );
    }
  }
  sceIoDclose(fd);
# else
  char SaveDirName[MAX_PATH];
  DIR* fd = 0;

  loc_del_comment_list();

  snprintf(SaveDirName, MAX_PATH, "%s/txt", THOM.thom_home_dir);

  fd = opendir(SaveDirName);
  if (!fd) return;

  struct dirent *a_dirent;
  while ((a_dirent = readdir(fd)) != 0) {
    if(a_dirent->d_name[0] == '.') continue;
    if (a_dirent->d_type != DT_DIR) 
    {
      loc_add_comment_list( a_dirent->d_name );
    }
  }
  closedir(fd);
# endif
}

char*
load_comment_if_exists(char *Name)
{
static char loc_comment_buffer[128];

  char        FileName[MAX_PATH];
  char        TmpFileName[MAX_PATH];
  FILE       *a_file;
  char       *SaveName;
  char       *Scan;

  loc_comment_buffer[0] = 0;

  strcpy(FileName, Name);
  SaveName = strrchr(FileName,'/');
  if (SaveName != (char *)0) SaveName++;
  else                       SaveName = FileName;

  Scan = strrchr(SaveName,'.');
  if (Scan) *Scan = '\0';

  if (!SaveName[0]) return 0;

  comment_list *scan_list = loc_head_comment;
  while (scan_list != 0) {
    if (! strcasecmp( SaveName, scan_list->name)) {
      snprintf(TmpFileName, MAX_PATH, "%s/txt/%s", THOM.thom_home_dir, scan_list->filename);
      a_file = fopen(TmpFileName, "r");
      if (a_file) {
        char* a_scan = 0;
        loc_comment_buffer[0] = 0;
        if (fgets(loc_comment_buffer, 60, a_file) != 0) {
          a_scan = strchr(loc_comment_buffer, '\n');
          if (a_scan) *a_scan = '\0';
          /* For this #@$% of windows ! */
          a_scan = strchr(loc_comment_buffer,'\r');
          if (a_scan) *a_scan = '\0';
          fclose(a_file);
          return loc_comment_buffer;
        }
        fclose(a_file);
        return 0;
      }
    }
    scan_list = scan_list->next;
  }
  return 0;
}


void
thom_kbd_load(void)
{
  char        TmpFileName[MAX_PATH + 1];
  struct stat aStat;

  snprintf(TmpFileName, MAX_PATH, "%s/kbd/%s.kbd", THOM.thom_home_dir, THOM.thom_save_name );
  if (! stat(TmpFileName, &aStat)) {
    psp_kbd_load_mapping(TmpFileName);
  }
}

int
thom_kbd_save(void)
{
  char TmpFileName[MAX_PATH + 1];
  snprintf(TmpFileName, MAX_PATH, "%s/kbd/%s.kbd", THOM.thom_home_dir, THOM.thom_save_name );
  return( psp_kbd_save_mapping(TmpFileName) );
}


void
emulator_soft_reset(void)
{
  to7_Reset();
  //LUDO:
  to7_SetK7Counter(0);
}

void
emulator_cold_reset(void)
{
  to7_ColdReset();
  //LUDO:
  to7_SetK7Counter(0);
}

//Load Functions

typedef struct {
   char *pchZipFile;
   char *pchExtension;
   char *pchFileNames;
   char *pchSelection;
   int iFiles;
   unsigned int dwOffset;
} t_zip_info;

t_zip_info zip_info;

typedef unsigned int    dword;
typedef unsigned char   byte;
typedef unsigned short  word;

#define ERR_FILE_NOT_FOUND       13
#define ERR_FILE_BAD_ZIP         14
#define ERR_FILE_EMPTY_ZIP       15
#define ERR_FILE_UNZIP_FAILED    16

FILE *pfileObject;
char *pbGPBuffer = NULL;

static dword
loc_get_dword(byte *buff)
{
  return ( (((dword)buff[3]) << 24) |
           (((dword)buff[2]) << 16) |
           (((dword)buff[1]) <<  8) |
           (((dword)buff[0]) <<  0) );
}

static void
loc_set_dword(byte *buff, dword value)
{
  buff[3] = (value >> 24) & 0xff;
  buff[2] = (value >> 16) & 0xff;
  buff[1] = (value >>  8) & 0xff;
  buff[0] = (value >>  0) & 0xff;
}

static word
loc_get_word(byte *buff)
{
  return( (((word)buff[1]) <<  8) |
          (((word)buff[0]) <<  0) );
}


int 
zip_dir(t_zip_info *zi)
{
   int n, iFileCount;
   long lFilePosition;
   dword dwCentralDirPosition, dwNextEntry;
   word wCentralDirEntries, wCentralDirSize, wFilenameLength;
   byte *pbPtr;
   char *pchStrPtr;
   dword dwOffset;

   iFileCount = 0;
   if ((pfileObject = fopen(zi->pchZipFile, "rb")) == NULL) {
      return ERR_FILE_NOT_FOUND;
   }

   if (pbGPBuffer == (char *)0) {
     pbGPBuffer = (char *)malloc( sizeof(byte) * 128*1024); 
   }

   wCentralDirEntries = 0;
   wCentralDirSize = 0;
   dwCentralDirPosition = 0;
   lFilePosition = -256;
   do {
      fseek(pfileObject, lFilePosition, SEEK_END);
      if (fread(pbGPBuffer, 256, 1, pfileObject) == 0) {
         fclose(pfileObject);
         return ERR_FILE_BAD_ZIP; // exit if loading of data chunck failed
      }
      pbPtr = (byte*)(pbGPBuffer + (256 - 22)); // pointer to end of central directory (under ideal conditions)
      while (pbPtr != (byte *)pbGPBuffer) {
         if (loc_get_dword(pbPtr) == 0x06054b50) { // check for end of central directory signature
            wCentralDirEntries = loc_get_word(pbPtr + 10);
            wCentralDirSize = loc_get_word(pbPtr + 12);
            dwCentralDirPosition = loc_get_dword(pbPtr + 16);
            break;
         }
         pbPtr--; // move backwards through buffer
      }
      lFilePosition -= 256; // move backwards through ZIP file
   } while (wCentralDirEntries == 0);
   if (wCentralDirSize == 0) {
      fclose(pfileObject);
      return ERR_FILE_BAD_ZIP; // exit if no central directory was found
   }
   fseek(pfileObject, dwCentralDirPosition, SEEK_SET);
   if (fread(pbGPBuffer, wCentralDirSize, 1, pfileObject) == 0) {
      fclose(pfileObject);
      return ERR_FILE_BAD_ZIP; // exit if loading of data chunck failed
   }

   pbPtr = (byte *)pbGPBuffer;
   if (zi->pchFileNames) {
      free(zi->pchFileNames); // dealloc old string table
   }
   zi->pchFileNames = (char *)malloc(wCentralDirSize); // approximate space needed by using the central directory size
   pchStrPtr = zi->pchFileNames;

   for (n = wCentralDirEntries; n; n--) {
      wFilenameLength = loc_get_word(pbPtr + 28);
      dwOffset = loc_get_dword(pbPtr + 42);
      dwNextEntry = wFilenameLength + loc_get_word(pbPtr + 30) + loc_get_word(pbPtr + 32);
      pbPtr += 46;
      char *pchThisExtension = zi->pchExtension;
      while (*pchThisExtension != '\0') { // loop for all extensions to be checked
         if (strncasecmp((char *)pbPtr + (wFilenameLength - 4), pchThisExtension, 4) == 0) {
            strncpy(pchStrPtr, (char *)pbPtr, wFilenameLength); // copy filename from zip directory
            pchStrPtr[wFilenameLength] = 0; // zero terminate string
            pchStrPtr += wFilenameLength+1;
            loc_set_dword((byte*)pchStrPtr, dwOffset);
            pchStrPtr += 4;
            iFileCount++;
            break;
         }
         pchThisExtension += 4; // advance to next extension
      }
      pbPtr += dwNextEntry;
   }
   fclose(pfileObject);

   if (iFileCount == 0) { // no files found?
      return ERR_FILE_EMPTY_ZIP;
   }

   zi->iFiles = iFileCount;
   return 0; // operation completed successfully
}

int 
zip_extract(char *pchZipFile, char *pchFileName, dword dwOffset, char *ext)
{
   int iStatus, iCount;
   dword dwSize;
   byte *pbInputBuffer, *pbOutputBuffer;
   FILE *pfileOut, *pfileIn;
   z_stream z;

   strcpy(pchFileName, THOM.thom_home_dir);
   strcat(pchFileName, "/unzip.");
   strcat(pchFileName, ext);

   if (!(pfileOut = fopen(pchFileName, "wb"))) {
      return ERR_FILE_UNZIP_FAILED; // couldn't create output file
   }
   if (pbGPBuffer == (char *)0) {
     pbGPBuffer = (char *)malloc( sizeof(byte) * 128*1024); 
   }
   pfileIn = fopen(pchZipFile, "rb"); // open ZIP file for reading
   fseek(pfileIn, dwOffset, SEEK_SET); // move file pointer to beginning of data block
   fread(pbGPBuffer, 30, 1, pfileIn); // read local header
   dwSize = loc_get_dword((byte *)(pbGPBuffer + 18)); // length of compressed data
   dwOffset += 30 + loc_get_word((byte *)(pbGPBuffer + 26)) + loc_get_word((byte *)(pbGPBuffer + 28));
   fseek(pfileIn, dwOffset, SEEK_SET); // move file pointer to start of compressed data

   pbInputBuffer = (byte *)pbGPBuffer; // space for compressed data chunck
   pbOutputBuffer = pbInputBuffer + 16384; // space for uncompressed data chunck
   z.zalloc = (alloc_func)0;
   z.zfree = (free_func)0;
   z.opaque = (voidpf)0;
   iStatus = inflateInit2(&z, -MAX_WBITS); // init zlib stream (no header)
   do {
      z.next_in = pbInputBuffer;
      if (dwSize > 16384) { // limit input size to max 16K or remaining bytes
         z.avail_in = 16384;
      } else {
         z.avail_in = dwSize;
      }
      z.avail_in = fread(pbInputBuffer, 1, z.avail_in, pfileIn); // load compressed data chunck from ZIP file
      while ((z.avail_in) && (iStatus == Z_OK)) { // loop until all data has been processed
         z.next_out = pbOutputBuffer;
         z.avail_out = 16384;
         iStatus = inflate(&z, Z_NO_FLUSH); // decompress data
         iCount = 16384 - z.avail_out;
         if (iCount) { // save data to file if output buffer is full
            fwrite(pbOutputBuffer, 1, iCount, pfileOut);
         }
      }
      dwSize -= 16384; // advance to next chunck
   } while ((dwSize > 0) && (iStatus == Z_OK)) ; // loop until done
   if (iStatus != Z_STREAM_END) {
      return ERR_FILE_UNZIP_FAILED; // abort on error
   }
   iStatus = inflateEnd(&z); // clean up
   fclose(pfileIn);
   fclose(pfileOut);

   return 0; // data was successfully decompressed
}


int 
thom_rewind_k7(void)
{
  to7_SetK7Counter(0);
  return 0;
}

int 
thom_eject_k7(void)
{
  to7_Reset();
  return 0;
}

int
thom_load_k7(char *FileName)
{
  char *scan;
  char  SaveName[MAX_PATH+1];
  int   format;
  int   error;

  error = 1;

  strncpy(SaveName,FileName,MAX_PATH);
  scan = strrchr(SaveName,'.');
  if (scan) *scan = '\0';
  thom_update_save_name(SaveName);
  error = loc_load_k7(FileName);

  if (! error ) {
    thom_kbd_load();
    thom_load_cheat();
    thom_load_settings();
    thom_set_basic_mode(0);
  }

  return error;
}

void
thom_set_basic_mode(int basic_mode)
{
  if (THOM.thom_basic_128 != basic_mode) {
    THOM.thom_basic_128 = basic_mode;
    if (THOM.thom_basic_128) {
      to7_LoadMemo7("./memo7/basic128.m7");
    } else {
      to7_LoadMemo7("./memo7/basic.m7");
    }
    emulator_cold_reset();
  } else {
    emulator_soft_reset();
  }
}

int
thom_load_sap(char *FileName, int disk_id)
{
  char *scan;
  char  SaveName[MAX_PATH+1];
  int   format;
  int   error;

  error = 1;

  strncpy(SaveName,FileName,MAX_PATH);
  scan = strrchr(SaveName,'.');
  if (scan) *scan = '\0';
  thom_update_save_name(SaveName);
  error = loc_load_sap(FileName, disk_id);

  if (! error ) {
    thom_kbd_load();
    thom_load_cheat();
    thom_load_settings();
    thom_set_basic_mode(1);
  }

  return error;
}

static int
loc_load_state(char *filename)
{
  int error;
  error = ! to7_LoadState(filename);
  return error;
}

int
thom_load_state(char *FileName)
{
  char *pchPtr;
  char *scan;
  char  SaveName[MAX_PATH+1];
  char  TmpFileName[MAX_PATH + 1];
  dword n;
  int   format;
  int   error;

  error = 1;

  to7_ColdReset();

  strncpy(SaveName,FileName,MAX_PATH);
  scan = strrchr(SaveName,'.');
  if (scan) *scan = '\0';
  thom_update_save_name(SaveName);
  error = loc_load_state(FileName);

  if (! error ) {
    thom_kbd_load();
    thom_load_cheat();
    thom_load_settings();
  }

  return error;
}

static int
loc_thom_save_state(char *filename)
{
  int error;
  error = ! to7_SaveState(filename);
  return error;
}

int
thom_snapshot_save_slot(int save_id)
{
  char      FileName[MAX_PATH+1];
# ifdef LINUX_MODE
  struct stat aStat;
# else
  SceIoStat   aStat;
# endif
  int       error;

  error = 1;

  if (save_id < THOM_MAX_SAVE_STATE) {
    snprintf(FileName, MAX_PATH, "%s/save/sav_%s_%d.stz", THOM.thom_home_dir, THOM.thom_save_name, save_id);
    error = loc_thom_save_state(FileName);
    if (! error) {
# ifdef LINUX_MODE
      if (! stat(FileName, &aStat)) 
# else
      if (! sceIoGetstat(FileName, &aStat))
# endif
      {
        THOM.thom_save_state[save_id].used  = 1;
        THOM.thom_save_state[save_id].thumb = 0;
        THOM.thom_save_state[save_id].date  = aStat.st_mtime;
        snprintf(FileName, MAX_PATH, "%s/save/sav_%s_%d.png", THOM.thom_home_dir, THOM.thom_save_name, save_id);
        if (psp_sdl_save_thumb_png(THOM.thom_save_state[save_id].surface, FileName)) {
          THOM.thom_save_state[save_id].thumb = 1;
        }
      }
    }
  }

  return error;
}

int
thom_snapshot_load_slot(int load_id)
{
  char  FileName[MAX_PATH+1];
  int   error;

  error = 1;

  if (load_id < THOM_MAX_SAVE_STATE) {
    snprintf(FileName, MAX_PATH, "%s/save/sav_%s_%d.stz", THOM.thom_home_dir, THOM.thom_save_name, load_id);
    error = loc_load_state(FileName);
  }
  return error;
}

int
thom_snapshot_del_slot(int save_id)
{
  char  FileName[MAX_PATH+1];
# ifdef LINUX_MODE
  struct stat aStat;
# else
  SceIoStat   aStat;
# endif
  int   error;

  error = 1;

  if (save_id < THOM_MAX_SAVE_STATE) {
    snprintf(FileName, MAX_PATH, "%s/save/sav_%s_%d.stz", THOM.thom_home_dir, THOM.thom_save_name, save_id);
    error = remove(FileName);
    if (! error) {
      THOM.thom_save_state[save_id].used  = 0;
      THOM.thom_save_state[save_id].thumb = 0;
      memset(&THOM.thom_save_state[save_id].date, 0, sizeof(ScePspDateTime));

      /* We keep always thumbnail with id 0, to have something to display in the file requester */ 
      if (save_id != 0) {
        snprintf(FileName, MAX_PATH, "%s/save/sav_%s_%d.png", THOM.thom_home_dir, THOM.thom_save_name, save_id);
# ifdef LINUX_MODE
        if (! stat(FileName, &aStat)) 
# else
        if (! sceIoGetstat(FileName, &aStat))
# endif
        {
          remove(FileName);
        }
      }
    }
  }

  return error;
}

static int
loc_thom_save_cheat(char *chFileName)
{
  FILE* FileDesc;
  int   cheat_num;
  int   error = 0;

  FileDesc = fopen(chFileName, "w");
  if (FileDesc != (FILE *)0 ) {

    for (cheat_num = 0; cheat_num < THOM_MAX_CHEAT; cheat_num++) {
      THOM_cheat_t* a_cheat = &THOM.thom_cheat[cheat_num];
      if (a_cheat->type != THOM_CHEAT_NONE) {
        fprintf(FileDesc, "%d,%x,%x,%s\n", 
                a_cheat->type, a_cheat->addr, a_cheat->value, a_cheat->comment);
      }
    }
    fclose(FileDesc);

  } else {
    error = 1;
  }

  return error;
}

int
thom_save_cheat(void)
{
  char  FileName[MAX_PATH+1];
  int   error;

  error = 1;

  snprintf(FileName, MAX_PATH, "%s/cht/%s.cht", THOM.thom_home_dir, THOM.thom_save_name);
  error = loc_thom_save_cheat(FileName);

  return error;
}

static int
loc_thom_load_cheat(char *chFileName)
{
  char  Buffer[512];
  char *Scan;
  char *Field;
  unsigned int  cheat_addr;
  unsigned int  cheat_value;
  unsigned int  cheat_type;
  char         *cheat_comment;
  int           cheat_num;
  FILE* FileDesc;

  memset(THOM.thom_cheat, 0, sizeof(THOM.thom_cheat));
  cheat_num = 0;

  FileDesc = fopen(chFileName, "r");
  if (FileDesc == (FILE *)0 ) return 0;

  while (fgets(Buffer,512, FileDesc) != (char *)0) {

    Scan = strchr(Buffer,'\n');
    if (Scan) *Scan = '\0';
    /* For this #@$% of windows ! */
    Scan = strchr(Buffer,'\r');
    if (Scan) *Scan = '\0';
    if (Buffer[0] == '#') continue;

    /* %d, %x, %x, %s */
    Field = Buffer;
    Scan = strchr(Field, ',');
    if (! Scan) continue;
    *Scan = 0;
    if (sscanf(Field, "%d", &cheat_type) != 1) continue;
    Field = Scan + 1;
    Scan = strchr(Field, ',');
    if (! Scan) continue;
    *Scan = 0;
    if (sscanf(Field, "%x", &cheat_addr) != 1) continue;
    Field = Scan + 1;
    Scan = strchr(Field, ',');
    if (! Scan) continue;
    *Scan = 0;
    if (sscanf(Field, "%x", &cheat_value) != 1) continue;
    Field = Scan + 1;
    cheat_comment = Field;

    if (cheat_type <= THOM_CHEAT_NONE) continue;

    THOM_cheat_t* a_cheat = &THOM.thom_cheat[cheat_num];

    a_cheat->type  = cheat_type;
    a_cheat->addr  = cheat_addr;
    a_cheat->value = cheat_value;
    strncpy(a_cheat->comment, cheat_comment, sizeof(a_cheat->comment));
    a_cheat->comment[sizeof(a_cheat->comment)-1] = 0;

    if (++cheat_num >= THOM_MAX_CHEAT) break;
  }
  fclose(FileDesc);

  return 0;
}

int
thom_load_cheat()
{
  char  FileName[MAX_PATH+1];
  int   error;

  error = 1;

  snprintf(FileName, MAX_PATH, "%s/cht/%s.cht", THOM.thom_home_dir, THOM.thom_save_name);
  error = loc_thom_load_cheat(FileName);

  return error;
}

int
thom_load_file_cheat(char *FileName)
{
  return loc_thom_load_cheat(FileName);
}

void
thom_apply_cheats()
{
  int cheat_num;
  for (cheat_num = 0; cheat_num < THOM_MAX_CHEAT; cheat_num++) {
    THOM_cheat_t* a_cheat = &THOM.thom_cheat[cheat_num];
    if (a_cheat->type == THOM_CHEAT_ENABLE) {
      to7_write_ram(a_cheat->addr, a_cheat->value);
    }
  }
}


void
thom_audio_pause()
{
  if (THOM.thom_snd_enable) {
    SDL_PauseAudio(1);
  }
}

void
thom_audio_resume()
{
  if (THOM.thom_snd_enable) {
    SDL_PauseAudio(0);
  }
}

void
thom_treat_command_key(int thom_idx)
{
  int new_render;

  switch (thom_idx) 
  {
    case THOMC_FPS: THOM.thom_view_fps = ! THOM.thom_view_fps;
    break;
    case THOMC_JOY: THOM.psp_reverse_analog = ! THOM.psp_reverse_analog;
    break;
    case THOMC_RENDER: 
      psp_sdl_black_screen();
      new_render = THOM.thom_render_mode + 1;
      if (new_render > THOM_LAST_RENDER) new_render = 0;
      THOM.thom_render_mode = new_render;
    break;
    case THOMC_LOAD: psp_main_menu_load_current();
    break;
    case THOMC_SAVE: psp_main_menu_save_current(); 
    break;
    case THOMC_RESET: 
       psp_sdl_black_screen();
       emulator_soft_reset(); 
       reset_save_name();
    break;
    case THOMC_AUTOFIRE: 
       kbd_change_auto_fire(! THOM.thom_auto_fire);
    break;
    case THOMC_DECFIRE: 
      if (THOM.thom_auto_fire_period > 0) THOM.thom_auto_fire_period--;
    break;
    case THOMC_INCFIRE: 
      if (THOM.thom_auto_fire_period < 19) THOM.thom_auto_fire_period++;
    break;
    case THOMC_DECDELTA:
      if (THOM.thom_delta_y > -20) THOM.thom_delta_y--;
    break;
    case THOMC_INCDELTA: 
      if (THOM.thom_delta_y <  20) THOM.thom_delta_y++;
    break;
    case THOMC_SCREEN: psp_screenshot_mode = 10;
    break;
  }
}

  int psp_exit_now = 0;

void
myCtrlPeekBufferPositive( SceCtrlData* pc, int count )
{
  if (psp_exit_now) psp_sdl_exit(0);
  sceCtrlPeekBufferPositive( pc, count );
}

