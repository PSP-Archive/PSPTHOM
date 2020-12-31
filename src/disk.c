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
 *  Module     : disk.c
 *  Version    : 1.5.5
 *  Créé par   : Alexandre Pukall mai 1998
 *  Modifié par: Eric Botcazou 03/11/2003
 *
 *  Gestion du format SAP 2.0: lecture et écriture disquette.
 */


#ifndef SCAN_DEPEND
   #include <stdio.h>
   #include <string.h>
#endif

#include "intern/disk.h"
#include "intern/errors.h"
#include "intern/hardware.h"
#include "to7.h"


/* paramètres physiques des lecteurs Thomson */
#define NBDRIVE    4
#define NBTRACK   40
#define NBSECT    16
#define SECTSIZE 256

/* contrôleur de disquettes */
struct DISK_CTRL disk_ctrl;

/* type d'un lecteur */
typedef struct {
    enum {
        NO_DISK,
        DIRECT_ACCESS,
        NORMAL_ACCESS
    } state;
    int mode;
    char filename[FILENAME_LENGTH+1];
} disk_t;

static disk_t disk[NBDRIVE];



/*****************************************/
/* émulation du contrôleur de disquettes */
/*****************************************/


#ifdef DEBUG

DISK_CTRL_SET_FUNC(cmdr)
DISK_CTRL_SET_FUNC(tkr)
DISK_CTRL_SET_FUNC(secr)
DISK_CTRL_SET_FUNC(dr)
DISK_CTRL_SET_FUNC(drvr)

#endif


/***************************************/
/* gestion du format SAP               */
/***************************************/

#define SAP_HEADER_SIZE  66
#define SAP_SECT_SIZE   262
#define SAP_MAGIC_NUM  0xB3

static const char sap_header[]="\1SYSTEME D'ARCHIVAGE PUKALL S.A.P. "
                               "(c) Alexandre PUKALL Avril 1998";

/* type d'un secteur SAP */
typedef struct {
    unsigned char format;
    unsigned char protection;
    unsigned char track;
    unsigned char sector;
    unsigned char data[SECTSIZE];
    unsigned char crc1sect;
    unsigned char crc2sect;
} sapsector_t;

/* table de calcul du CRC */
static short int crcpuk_temp;
static short int puktable[]={
   0x0000, 0x1081, 0x2102, 0x3183,
   0x4204, 0x5285, 0x6306, 0x7387,
   0x8408, 0x9489, 0xa50a, 0xb58b,
   0xc60c, 0xd68d, 0xe70e, 0xf78f
};



/* crc_pukall:
 *  Calcule le nouveau CRC à partir de la donnée c.
 */
static void crc_pukall(short int c)
{
    register short int index;

    index = (crcpuk_temp ^ c) & 0xf;
    crcpuk_temp = ((crcpuk_temp>>4) & 0xfff) ^ puktable[index];

    c >>= 4;

    index = (crcpuk_temp ^ c) & 0xf;
    crcpuk_temp = ((crcpuk_temp>>4) & 0xfff) ^ puktable[index];
}



/* do_crc:
 *  Calcule le CRC d'un secteur SAP.
 */
static void do_crc(sapsector_t *sapsector)
{
    register int i;

    crcpuk_temp = 0xffff;

    crc_pukall(sapsector->format);
    crc_pukall(sapsector->protection);
    crc_pukall(sapsector->track);
    crc_pukall(sapsector->sector);

    for (i=0;i< SECTSIZE;i++)
       crc_pukall(sapsector->data[i]);
}



/* verify_sap_lect:
 *  Vérifie l'intégrité du secteur.
 */
static int verify_sap_lect(sapsector_t *sapsector)
{
    do_crc(sapsector);

    if ((sapsector->crc1sect != ((crcpuk_temp>>8)&0xff)) || (sapsector->crc2sect != (crcpuk_temp&0xff)))
	return 8;

    if ((sapsector->format==4) || (sapsector->format==5))
	return 4;

    return 0;
}



/* verify_sap_ecri:
 *  Calcule le CRC du secteur.
 */
static int verify_sap_ecri(sapsector_t *sapsector)
{
    if ((sapsector->protection==1) || (sapsector->protection==3))
  	return 1;
  
    if (sapsector->format==5)
	return 4;
  
    do_crc(sapsector);

    sapsector->crc1sect=(crcpuk_temp>>8)&0xff;
    sapsector->crc2sect=crcpuk_temp&0xff;

    return 0;
}



/* sap_get_sector:
 *  Lit un secteur sur le lecteur spécifié et retourne
 *  un code d'erreur moniteur TO7.
 */
static int sap_get_sector(int drive, sapsector_t *sapsector)
{
    register int i;
    unsigned char buffer[SAP_SECT_SIZE];
    long pos;
    int err=0;
    FILE *file;

    /* lecture du secteur dans le fichier */
    if ((file=fopen(disk[drive].filename, "rb")) == NULL)
        return 4;
             
    pos = SAP_HEADER_SIZE + (sapsector->track*NBSECT+(sapsector->sector-1))*SAP_SECT_SIZE;

    fseek(file, pos, SEEK_SET);
    fread(buffer, sizeof(char), (size_t)SAP_SECT_SIZE, file);
    fclose(file);

    /* pour être portable on n'écrit pas directement dans une structure */
    sapsector->format=buffer[0];
    sapsector->protection=buffer[1];

    /* teste la différence d'info piste/secteur */
    if ((sapsector->track != buffer[2]) || (sapsector->sector != buffer[3]))
        err=4;

    sapsector->track=buffer[2];
    sapsector->sector=buffer[3];

    for (i=0;i<SECTSIZE;i++)
        sapsector->data[i]=buffer[4+i]^SAP_MAGIC_NUM;

    sapsector->crc1sect=buffer[4+i];
    sapsector->crc2sect=buffer[4+i+1];

    return err;
}



/* sap_put_sector:
 *  Ecrit un secteur sur le lecteur spécifié et retourne
 *  un code d'erreur moniteur TO7.
 */
static int sap_put_sector(int drive, sapsector_t *sapsector)
{
   register int i;
   unsigned char buffer[SAP_SECT_SIZE];
   long pos;
   FILE *file;

   buffer[0]=sapsector->format;
   buffer[1]=sapsector->protection;
   buffer[2]=sapsector->track;
   buffer[3]=sapsector->sector;

   for (i=0;i<SECTSIZE;i++)
       buffer[4+i]=sapsector->data[i]^SAP_MAGIC_NUM;

   buffer[4+i]=sapsector->crc1sect;
   buffer[4+i+1]=sapsector->crc2sect;
				
   /* écriture du secteur dans le fichier */
   if ((file=fopen(disk[drive].filename,"rb+"))==NULL)
       return 4;

   pos = SAP_HEADER_SIZE + (sapsector->track*NBSECT+(sapsector->sector-1))*SAP_SECT_SIZE;

   fseek(file, pos, SEEK_SET);
   fwrite(buffer, sizeof(char), (size_t)SAP_SECT_SIZE, file);
   fclose(file);

   return 0;
}



/* sap_format_track:
 *  Formate une piste sur le lecteur spécifié et retourne
 *  un code d'erreur moniteur TO7.
 */
static int sap_format_track(int drive, int track, unsigned char filler_byte)
{
    int i, sect;
    int err=0;
    sapsector_t sapsector;

    sapsector.format = 0;
    sapsector.protection = 0;

    for (i=0; i<SECTSIZE; i++)
        sapsector.data[i]=filler_byte;

    for (sect=1; sect<NBSECT+1; sect++)
    {
        sapsector.track = track;
        sapsector.sector = sect;

        err=verify_sap_ecri(&sapsector);
        /* err est toujours égal à 0 ... */
            
        if (err==0)
            err=sap_put_sector(drive, &sapsector);

        if (err)
            break;
    }

    return err;
}



/**************************************/
/* émulation du Disk Operating System */
/**************************************/


/* ResetDiskCtrl:
 *  Initialise le contrôleur de disquettes.
 */
void ResetDiskCtrl(int *cc)
{
    *cc&=0xFE;
    STORE_BYTE(0x604E,'D');
}



/* ReadSector:
 *  Lit un secteur et modifie le registre d'état.
 */
void ReadSector(int *cc)
{
    register int i;
    int err = 0;
    int drive = LOAD_BYTE(0x6049);
    int dest_data = LOAD_WORD(0x604F);
    sapsector_t sapsector;

    /* Standard C: l'initialisation de structures avec des expressions non constantes n'est pas supportée */
    sapsector.format = 0;
    sapsector.protection = 0;
    sapsector.track = LOAD_WORD(0x604A);
    sapsector.sector = LOAD_BYTE(0x604C);

#ifdef DEBUG
    fprintf(stderr, "ReadSector(): drive  = %d\n"
                    "              track  = %d\n"
                    "              sector = %d\n", drive, sapsector.track, sapsector.sector);
#endif

    if ((drive<0) || (drive>NBDRIVE) || (sapsector.track>=NBTRACK) || (sapsector.sector>NBSECT))
    {
        STORE_BYTE(0x604E, 0x10);
        *cc|=1;
        return;
    }

    to7_SetDiskLed(TRUE);

    switch (disk[drive].state)
    {
        case NO_DISK:
        default:
#ifdef DEBUG
            fprintf(stderr, "ReadSector(): no disk in drive %d\n"
                            "               track  = %d\n"
                            "               sector = %d\n", drive, sapsector.track, sapsector.sector);
#endif
            err=0x10;
            break;

        case DIRECT_ACCESS:
            if (to7_DirectReadSector)
            {
                /* MSDOS: le BIOS a besoin d'un buffer de 512 octets */
                unsigned char direct_buffer[512];

                err=to7_DirectReadSector(drive, sapsector.track, sapsector.sector, 1, direct_buffer);
                if (err==0)
                    for (i=0;i<SECTSIZE;i++)
                        STORE_BYTE(((dest_data+i)&0xFFFF), direct_buffer[i]);
            }
            else
                err=0x10;
            break;

        case NORMAL_ACCESS:
            err=sap_get_sector(drive, &sapsector);

            if (err==0)
     	        err=verify_sap_lect(&sapsector);

            if (err==0)
                for (i=0;i<SECTSIZE;i++)
	            STORE_BYTE(((dest_data+i)%0xFFFF), sapsector.data[i]);
            break;
    }

    if (err==0)
    {
        STORE_BYTE(0x604E, 0);
        *cc&=0xFE;
    }
    else
    {
        STORE_BYTE(0x604E, err);
        *cc|=1;
    }

    to7_SetDiskLed(FALSE);
}



/* WriteSector:
 *  Ecrit un secteur et modifie le registre d'état.
 */
void WriteSector(int *cc)
{
    register int i;
    int err = 0;
    int drive = LOAD_BYTE(0x6049);
    int source_data = LOAD_WORD(0x604F);
    sapsector_t sapsector;

    /* Standard C: l'initialisation de structures avec des expressions non constantes n'est pas supportée */
    sapsector.format = 0;
    sapsector.protection = 0;
    sapsector.track = LOAD_WORD(0x604A);
    sapsector.sector = LOAD_BYTE(0x604C);

#ifdef DEBUG
    fprintf(stderr, "WriteSector(): drive  = %d\n"
                    "               track  = %d\n"
                    "               sector = %d\n", drive, sapsector.track, sapsector.sector);
#endif

    if ((drive<0) || (drive>NBDRIVE) || (sapsector.track>=NBTRACK) || (sapsector.sector>NBSECT))
    {
        STORE_BYTE(0x604E, 0x10);
        *cc|=1;
        return;
    }

    if (disk[drive].mode == TO7_READ_ONLY)
    {
        STORE_BYTE(0x604E, 1);
        *cc|=1;
        return;
    }

    to7_SetDiskLed(TRUE);

    switch (disk[drive].state)
    {
        case NO_DISK:
        default:
#ifdef DEBUG
            fprintf(stderr, "WriteSector(): no disk in drive %d\n"
                            "                track  = %d\n"
                            "                sector = %d\n", drive, sapsector.track, sapsector.sector);
#endif
            err=0x10;
            break;

        case DIRECT_ACCESS:
            if (to7_DirectWriteSector)
            {
                /* MSDOS: le BIOS a besoin d'un buffer de 512 octets */
                unsigned char direct_buffer[512];

                for (i=0; i<SECTSIZE; i++)
                    direct_buffer[i]=LOAD_BYTE((source_data+i)&0xFFFF);

                /* MSDOS: nécessaire pour que le secteur soit lu par un TO7 réel */
                direct_buffer[SECTSIZE]=0;

                err=to7_DirectWriteSector(drive, sapsector.track, sapsector.sector, 1, direct_buffer);
            }
            else if (to7_DirectReadSector)
                err=0x01;  /* disque protégé en écriture */
            else
                err=0x10;
            break;

        case NORMAL_ACCESS:
            for (i=0; i<SECTSIZE; i++)
                sapsector.data[i]=LOAD_BYTE((source_data+i)&0xFFFF);

            err=verify_sap_ecri(&sapsector);
            /* err est toujours égal à 0 ... */
            
            if (err==0)
                err=sap_put_sector(drive, &sapsector);
            break;
    }

    if (err==0)
    {
        STORE_BYTE(0x604E, 0);
        *cc&=0xFE;
    }
    else
    {
        STORE_BYTE(0x604E, err);
        *cc|=1;
    }

    to7_SetDiskLed(FALSE);
}



/* DiskNop:
 *  Ne fait rien en elle-même, se contente de modifier le registre d'état.
 */
void DiskNop(int *cc)
{
    int drive = LOAD_BYTE(0x6049);

    if ((drive<0) || (drive>NBDRIVE))
    {
        STORE_BYTE(0x604E, 0x10);
        *cc|=1;
        return;
    }

    if (disk[drive].state == NO_DISK)
    {
        STORE_BYTE(0x604E, 0x10);
        *cc|=1;
        return;
    }
    else
    {
        STORE_BYTE(0x604E, 0);
        *cc&=0xFE;
        return;
    }
}



/* BuildSectorMap:
 *  Construit la carte des secteurs d'une piste en fonction
 *  du facteur d'entrelacement.
 */
static void BuildSectorMap(int *sector_map, int factor)
{
    int sect, loc=0;

    /* mise à zéro de la table */
    memset(sector_map, 0, sizeof(int)*NBSECT);

    for (sect=1; sect<=NBSECT; sect++)
    {
        while (sector_map[loc] != 0)
            loc=(loc+1)%NBSECT;

        sector_map[loc]=sect;

        loc=(loc+factor)%NBSECT;
    }
}


/* octet de remplissage des pistes formatées du TO7 */
#define FILLER_BYTE 0xE5


/* FormatDrive:
 *  Formate un lecteur et modifie le registre d'état.
 */
void FormatDrive(int *cc)
{
    int err = 0;
    int drive = LOAD_BYTE(0x6049), track, sect, pos;
    int sector_map[NBSECT];
    /* MSDOS: le BIOS a besoin d'un buffer de 512 octets */
    unsigned char headers_table[512];

    if ((drive<0) || (drive>NBDRIVE))
    {
        STORE_BYTE(0x604E, 0x10);
        *cc|=1;
        return;
    }

    STORE_BYTE(0x6048, LOAD_BYTE(0x6048)&0x80);

    if (disk[drive].mode == TO7_READ_ONLY)
    {
        STORE_BYTE(0x604E, 1);
        *cc|=1;
        return;
    }

    /* construction de la carte des secteurs pour chaque piste,
       à partir du facteur d'entrelacement situé en 0x604D */
    BuildSectorMap(sector_map, LOAD_BYTE(0x604D));

    /* formatage des pistes */
    for (track=0; track<NBTRACK; track++)
    { 
	/* construction de la table des headers */
	for (sect=1, pos=0; sect<=NBSECT; sect++, pos+=4)
	{
	    headers_table[pos]   = track;
	    headers_table[pos+1] = 0;
	    headers_table[pos+2] = sector_map[sect-1];
	    headers_table[pos+3] = 1;
	}

        switch (disk[drive].state)
        {
            case NO_DISK:
            default:
                err=0x10;
                break;

            case DIRECT_ACCESS:
                if (to7_DirectFormatTrack)
                    err=to7_DirectFormatTrack(drive, track, headers_table);
                else if (to7_DirectReadSector)
                    err=0x01;  /* disque protégé en écriture */
                else
                    err=0x10;
                break;

            case NORMAL_ACCESS:
                err=sap_format_track(drive, track, FILLER_BYTE);
                break;
        }

        if (err)
            break;
    }

    if (err==0)
    {
        STORE_BYTE(0x604E, 0);
        *cc&=0xFE;
    }
    else
    {
        STORE_BYTE(0x604E, err);
        *cc|=1;
    }

    /* la construction de la piste 20 contenant le répertoire
       et la FAT est assurée par le TO7 lui-même */ 
}



/* InitDisk:
 *  Initialise le module et met en place les trappes.
 */
void InitDisk(void)
{
    int drive;
        
    /* trap reset du contrôleur disk -> ResetDiskCtrl() */
    mem.mon.bank[0][0x012E]=0x02;
    mem.mon.bank[0][0x012F]=0x39;

    /* trap écriture d'un secteur -> WriteSector() */
    mem.mon.bank[0][0x017D]=0x02;
    mem.mon.bank[0][0x017E]=0x39;

    /* trap lecture d'un secteur -> ReadSector() */
    mem.mon.bank[0][0x0202]=0x02;
    mem.mon.bank[0][0x0203]=0x39;

    /* trap formatage lecteur -> FormatDrive() + BRA >E36B */
    mem.mon.bank[0][0x0347]=0x02;
    mem.mon.bank[0][0x0348]=0x20;
    mem.mon.bank[0][0x0349]=0x21;

    /* trap recherche piste 0 -> DiskNop() */
    mem.mon.bank[0][0x030C]=0x02;
    mem.mon.bank[0][0x030D]=0x39;

    /* trap recherche piste effective -> DiskNop() */
    mem.mon.bank[0][0x023F]=0x02;
    mem.mon.bank[0][0x0240]=0x39;

    for (drive=0; drive<NBDRIVE; drive++)
    {
        disk[drive].state=NO_DISK;
# if 0 //LUDO:
        disk[drive].mode=TO7_READ_WRITE;
# else
        disk[drive].mode=TO7_READ_ONLY;
# endif
        disk[drive].filename[0] = '\0';
    }
}



/* CheckFile:
 *  Teste la présence et le mode d'accès du fichier.
 */
static int CheckFile(const char filename[], int mode)
{
    FILE *file;
    
    if (mode==TO7_READ_WRITE)
    {
        if ((file=fopen(filename, "rb+")))
        {
            fclose(file);
            return mode;
        }
        else
            mode=TO7_READ_ONLY;
    }

    if ((file=fopen(filename, "rb")))
    {
        fclose(file);
        return mode;
    }
    else
        return ErrorMessage(TO7_CANNOT_OPEN_FILE, NULL);
}


/**********************************/
/* partie publique                */
/**********************************/


/* DirectSetDrive:
 *  Déclare le lecteur spécifié en accès direct et force
 *  le mode lecture seule pour le premier accès.
 */
int to7_DirectSetDrive(int drive)
{ 
    if (disk[drive].state != DIRECT_ACCESS)
    {
        disk[drive].state = DIRECT_ACCESS;
            
        /* premier accès en lecture seule */
        disk[drive].mode = TO7_READ_ONLY;
    }

    return disk[drive].mode;
}



/* LoadDisk:
 *  Charge l'archive SAP dans le lecteur spécifié et
 *  force si nécessaire le mode lecture seule.
 */
int 
to7_LoadDisk(int drive, const char filename[])
{
    int ret = CheckFile(filename, disk[drive].mode);

    if (ret != TO7_ERROR)
    {
        char header[SAP_HEADER_SIZE];
        FILE *file=fopen(filename, "rb");

        /* on vérifie le header */ 
        fread(header, sizeof(char), SAP_HEADER_SIZE, file);
        fclose(file);

        if (strncmp(header, sap_header, SAP_HEADER_SIZE)) {
          ErrorMessage(TO7_BAD_FILE_FORMAT, NULL);
          return 1;
        }

        strncpy(disk[drive].filename,filename, FILENAME_LENGTH);
        disk[drive].state = NORMAL_ACCESS;
        disk[drive].mode = ret;
    }

    return ret == TO7_ERROR;
}


/* SetDiskMode:
 *  Fixe le mode d'accès à la disquette.
 *  (lecture seule ou lecture écriture)
 */ 
int to7_SetDiskMode(int drive, int mode)
{
    int ret;

    if (disk[drive].mode == mode)
        return disk[drive].mode;

    switch (disk[drive].state)
    {
        case NO_DISK:
            disk[drive].mode = mode;
            return disk[drive].mode;

        case DIRECT_ACCESS:
            if ((mode == TO7_READ_WRITE) && !to7_DirectWriteSector)
                mode = TO7_READ_ONLY;  
  
            disk[drive].mode = mode;
            return disk[drive].mode;

        case NORMAL_ACCESS:
        default:
            ret = CheckFile(disk[drive].filename, mode);

            if (ret != TO7_ERROR)
                disk[drive].mode = ret;

            return ret;
    }
}



/* GetDiskFilename:
 *  Retourne le nom du fichier utilisé comme disquette.
 */
const char* to7_GetDiskFilename(int drive)
{
    return disk[drive].filename;
}

