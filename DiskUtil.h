/*************************************************************/
/**                                                         **/
/**                DiskUtil.h                               **/
/**                                                         **/
/** Common definitions and some functions for programs      **/
/**   working with disk-images that can be used with        **/
/**   fMSX                                                  **/
/**                                                         **/
/** Copyright (c) Arnold Metselaar 1996,1997                **/
/**     You are not allowed to distribute this software     **/
/**     commercially. Please, notify me, if you make any    **/
/**     changes to this file.                               **/
/*************************************************************/

/* some .h-files may lack these definitions: */
#ifndef O_BINARY
#define O_BINARY 0
#endif
#ifndef SEEK_SET
#define SEEK_SET 0
#endif
#ifndef SEEK_CUR
#define SEEK_CUR 1
#endif
#ifndef SEEK_END
#define SEEK_END 2
#endif

#define usint unsigned short int
#define byte unsigned char

#define seclen 512 /* length of sector */
#define EOF_FAT 0xFFF /* signals EOF in FAT */

typedef struct de {
  char d_fname[8];
  char d_ext[3];
  char d_attrib;
  char d_reserv[10]; /* unused */
  byte d_time[2];     /* byte-ordering of ints is machine-dependent */
  byte d_date[2];
  byte d_first[2];
  byte d_size[4];
} DirEntry;

/* give error message mes about file, mes Must have two %s's in it. */
#define PError(mes,file) {			\
    char linbuf[160];				\
    sprintf(linbuf,mes,progname,file); 		\
    perror(linbuf); 				\
}

/* prototypes for functions in DiskUtil.c */

/* functions to change DirEntries */
void setsh(byte *x, int y);
void setlg(byte *x, long y); 

/* functions to read DirEntries */
int rdsh(byte *x);
long rdlg(byte *x);

/* allocate memory or die */
void *xalloc(size_t len);

/* handle 12-bit FATs */
usint ReadFAT(usint clnr, byte * FAT);
void WriteFAT(usint clnr, usint val, byte * FAT);
