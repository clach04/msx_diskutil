/*************************************************************/
/**                                                         **/
/**                DiskUtil.c                               **/
/**                                                         **/
/** Some functions for programs working with disk-images    **/
/**     that can be used with fMSX                          **/
/**                                                         **/
/** Copyright (c) Arnold Metselaar 1997                     **/
/**     You are not allowed to distribute this software     **/
/**     commercially. Please, notify me, if you make any    **/
/**     changes to this file.                               **/
/*************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include "DiskUtil.h"

/* functions to change DirEntries */
void setsh(byte *x, int y) 
{ x[0]=y;x[1]=y>>8; }
void setlg(byte *x, long y) 
{ x[0]=y;x[1]=y>>8;x[2]=y>>16;x[3]=y>>24; }

/* functions to read DirEntries */
int rdsh(byte *x) 
{
    return ((int)x[0])+(((int)x[1])<<8);
}
long rdlg(byte *x) 
{
    return (long)(((int)x[0])+(((int)x[1])<<8)) 
	+(((long)(((int)x[2])+(((int)x[3])<<8)))<<16);
}

void *xalloc(size_t len)
{ 
  void *p;

  if (!(p=malloc(len))) 
    {
      puts("Out of memory\n"); exit(2);
    }
  return p;
}

extern byte *FAT;

/* read FAT-entry from FAT in memory */
usint ReadFAT(usint clnr, byte * FAT)
{ 
  byte *P;

  P=FAT+(clnr*3)/2;
  return ((clnr&1)
	  ? ((rdsh(P)>>4)&0x0FFF)
	  : (rdsh(P)&0x0FFF));
}

/* write an entry to FAT in memory */
void WriteFAT(usint clnr, usint val, byte * FAT)
{ register byte *P;

  P=FAT+(clnr*3)/2;
  if (clnr&1)
    { 
      P[0]=(P[0]&0x0F)+(val<<4);
      P[1]=val>>4;
    }
  else
    {
      P[0]=val;
      P[1]=(P[1]&0xF0)+((val>>8)&0x0F);
    }
}
