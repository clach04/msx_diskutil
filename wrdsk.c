/*************************************************************/
/**                                                         **/
/**                wrdsk.c                                  **/
/**                                                         **/
/** Program to write files to disk-images that can be       **/
/**  used with fMSX                                         **/
/**                                                         **/
/**                                                         **/
/** Copyright (c) Arnold Metselaar 1996                     **/
/**     You are not allowed to distribute this software     **/
/**     commercially. Please, notify me, if you make any    **/
/**     changes to this file.                               **/
/*************************************************************/

/* version history:
 * 1.1 (12 jul 1996) 
 *  - dots in path no longer force an extension
 *  - directories are skipped now
 * 1.2 ( 4 apr 1997)
 *  - open files to put in diskimage with O_BINARY.
 *    MS-DOS needs this
 * 1.3 ( 6 may 1997)
 *  - more informative error messages
 *  - stricter checking of diskimage
 * 1.4 (26 may 1997)
 *  - bug fixed in getting length of diskimage
 *  - layout changed
 *  - clear the fat for a diskimage made from a bootsector-image
 * 1.5 (27 aug 1997)
 *  - using less flags in open-calls
 *  - some features can be suppressed by compiling with -DPOOR
 *  - can be compiled on MSX using HITECH-C 
 * 1.6 (14 sep 1998)
 *  - clusters no longer need to be 1024 bytes
 *  - use longs to compute the offset in diskimages
 *  - progname set to "wrdsk" if argv[0] is not set 
 */

#ifdef HI_TECH_C
#ifdef z80
#define POOR
#endif 
#endif

#include <stdio.h>
#include <unistd.h>
#include <time.h>
#include <string.h>
#include <ctype.h>
#ifndef POOR
#include <fcntl.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/types.h> 
#endif

#include "DiskUtil.h"
#include "Boot.h" /* Contains byte BootBlock[] */

/* drive parameters */
usint ndir; /* # of dir entries */
usint maxcl; /* highest clusternumber */
long  dir_ofs,  /* offset of first byte of directory */
      data_ofs, /* offset of first byte of data  */ 
      clus_len;  /* length of one cluster (in bytes) */

/* buffers */
byte *FAT;
DirEntry *DIR;
char secbuf[seclen];

/* general */
int /*fd*/  diskim;    /* file descriptor of diskimage */
char *progname;/* name of program (for PError) */ 

int Toupper(int c) { return islower(c)?toupper(c):c; }

/* copy a file to the diskimage and 
   return a directory entry for this file */ 
void WriteFile(char *name, DirEntry *res)
{
    int /*fd*/  file;
    char *p;
    int i;
    long size;
    long bsiz;
    usint curcl,prevcl;

    if ((file=open(name,O_RDONLY|O_BINARY))==-1) {
	memset(res,0,sizeof(DirEntry));
	PError("%s: Can't open %s for reading",name);
    } else {  
	if ( !(p=strrchr(name,(int) '/')) )
	    p=name;
	else 
	    p++;
	if (p[1]==':') p+=2;
	for( ; *p=='.'; p++);
	for(i=0; (i<8) && p[i] && p[i]!='.' ; i++) 
	    res->d_fname[i]=Toupper(p[i]);
	while (i<8) res->d_fname[i++]=' ';
	i=0;
	if ( (p=strrchr(p,'.')) )
	for(p++, i=0; (i<3) && p[i] ; i++) 
	    res->d_ext[i]=Toupper(p[i]);
	while (i<3) res->d_ext[i++]=' ';
	res->d_attrib=0;

	/* compute time/date stamps */
#ifndef POOR
	{
	    struct stat fst;
	    struct tm * mtim;
	    usint t;

	    stat(name, &fst);

	    mtim = localtime(&(fst.st_mtime));
	    if (S_ISDIR(fst.st_mode)) { 
		res->d_fname[0]='\0';
		fprintf(stderr,"%s is directory (skipped).\n", name);
	    }

	    t=(mtim->tm_sec>>1)+(mtim->tm_min<<5)+(mtim->tm_hour<<11);
	    setsh(res->d_time,t);
	    t=mtim->tm_mday+((mtim->tm_mon+1)<<5)
		+((mtim->tm_year+1900-1980)<<9);
	    setsh(res->d_date,t);
	}
#else
	setsh(res->d_time,0);
	setsh(res->d_date,0);
#endif
    }
    if (res->d_fname[0]) {
	for(curcl=2; (curcl <= maxcl) && ReadFAT(curcl,FAT); curcl++);
	setsh(res->d_first,curcl);
	printf("%s ==> %.8s.%.3s\n", name, res->d_fname, res->d_ext);
	
	size=0; prevcl=0; 
	while( (bsiz=read(file,secbuf,seclen)) && (curcl<= maxcl) ) {
	    lseek(diskim, data_ofs+clus_len*(curcl-2), SEEK_SET);
	    for( i=0 ; (bsiz!=0) && (i<BootBlock[0x0D]) ; ) {
		size+=bsiz;
		write(diskim, secbuf, bsiz);
		if (++i < BootBlock[0x0D])
		    bsiz=read(file, secbuf, seclen);
	    }
	    if (prevcl) WriteFAT(prevcl,curcl,FAT);
	    for(prevcl=curcl++; (curcl<=maxcl) && ReadFAT(curcl,FAT); curcl++)
		;
	}

	if (size) WriteFAT(prevcl,EOF_FAT,FAT);
	if (bsiz)
	    fprintf(stderr,"%s: Diskimage full, %s truncated.\n",progname,name);
	setlg(res->d_size,size);
	if (close(file))
	    PError("%s: while closing %s", name);
    }
}

void New_FAT() 
{
    FAT=xalloc(seclen*BootBlock[0x16]);
    memset(FAT,0,seclen*BootBlock[0x16]);
    WriteFAT(0,BootBlock[0x15]+0xF00,FAT); /* media descriptor byte */
    WriteFAT(1,0xFFF,FAT);
}

void New_DIR() 
{
    ndir=rdsh(BootBlock+0x11);
    DIR=xalloc(ndir*sizeof(DirEntry));
    memset(DIR,0,ndir*sizeof(DirEntry));
}

/* open or create a diskimage */
int OpenDisk(char * DiskName)
{ 
    int /*fd*/  res;
    int i;
    long len;
    char *mode;
    byte cl;
    
    if ((res=open(DiskName,O_RDWR|O_BINARY))!=-1) {
	if((lseek(res,0,SEEK_SET)==-1)
	   || (read(res,BootBlock,seclen)!= seclen)) {
	    PError("%s: Can't read bootsector from %s", DiskName);
	    exit(2);
	}

	for ( cl=BootBlock[0x0D],i=0 ; cl>1 ; cl>>=1 )
	    i++;
	clus_len=(long)seclen << i;

	if ((rdsh((BootBlock+0x0B))!=seclen) || (cl!=1)) {
	    fprintf(stderr,"%s: %s does not seem to be a valid (MSX-DOS) diskimage\n",
		    progname, DiskName); 
	    exit(2);
	}
    
	if ( (len=lseek(res, 0, SEEK_END))==-1) {
	    PError("%s: Can't get length from %s", DiskName);
	    exit(2);
	}

	if (len==(long)seclen*rdsh((BootBlock+0x13)))
	    mode="add";
	else if (len==seclen) {
	    mode="expanding from bootsector";
	}
	else {
	    fprintf(stderr,"%s: %s does not have a valid length\n",
		    progname, DiskName); 
	    exit(2);
	}
    } 
    else {
	mode="new";
	if ((res=creat(DiskName,0666))==-1) { 
	    PError("%s: Can't create %s", DiskName); 
	    exit(2);
	}
	close(res);
	if ((res=open(DiskName,O_WRONLY|O_BINARY))==-1) { 
	    PError("%s: Can't open %s for writing", DiskName); 
	    exit(2);
	}
	lseek(res,0,SEEK_SET);
	write(res,BootBlock,seclen);

	for ( cl=BootBlock[0x0D],i=0 ; cl>1 ; cl>>=1 )
	    i++;
	clus_len=(long)seclen << i;
	len=seclen;
    }

    ndir=rdsh(BootBlock+0x11);
    dir_ofs=(long)seclen*(1+(int)BootBlock[0x10]*(int)BootBlock[0x16]);
    data_ofs=dir_ofs+(long)ndir*sizeof(DirEntry);
    maxcl=(rdsh(BootBlock+0x13)-data_ofs/seclen)/((int)BootBlock[0x0D])+1;

    printf("Diskimage: %s (%s)\n",DiskName, mode);

    if (len==seclen) { /* image needs to be 'formatted' */
	New_FAT();
	New_DIR();
	memset(secbuf,0,seclen);
	if( (lseek(res,(long)seclen*(rdsh((BootBlock+0x13))-1), SEEK_SET)==-1)
	    ||(write(res,secbuf,seclen)!= seclen)) {
	    PError("%s: can't set file size for %s", DiskName);
	    exit(2);
	}
    }
    else {
	FAT=xalloc(seclen*BootBlock[0x16]);
	if((lseek(res, seclen, SEEK_SET)==-1) 
	   ||(read( res,FAT,seclen*BootBlock[0x16])!=seclen*BootBlock[0x16])) {
	    PError("%s: Can't read FAT from %s", DiskName);
	    exit(2);
	}
	DIR=xalloc(ndir*rdsh(BootBlock+0x11));
	if((lseek(res, dir_ofs, SEEK_SET)==-1) 
	   ||(read( res,DIR,sizeof(DirEntry)*ndir)!=sizeof(DirEntry)*ndir)) {
	    PError("%s: Can't read DIR from %s", DiskName);
	    exit(2);
	}
    }
    return res;
}

/* close a diskimage */
void CloseDisk(int /*fd*/  img, char * DiskName)
{
    int i;

    lseek(img,seclen,SEEK_SET);
    for( i=0; i<BootBlock[0x10]; i++)
	if (write(img,FAT,seclen*BootBlock[0x16])!=seclen*BootBlock[0x16])
	    PError("%s:can't write FAT to %s", DiskName);
    if (write( img,DIR,sizeof(DirEntry)*ndir)!=  sizeof(DirEntry)*ndir)
	PError("%s can't write DIR to %s", DiskName);
    if (close(img))
	PError("%s: while closing %s", DiskName);
}

/* check whether dp points to a directory entry that is in use */
int Used(int dp)
{ 
    char c;

    c=DIR[dp].d_fname[0];
    return ( (c!=(char) 0xE5) && (c!=(char) '\0') );
}

/* Give some help */
void help(void)
{
    puts("Store files in a diskimage for use with fmsx.");
    printf("Usage: %s <diskimage> [<file1>] [<file2>] ...\n",progname);
    puts("If <diskimage> does not exist wrdsk will automaticly create it.");
    puts("Otherwise <diskimage> should be either a diskimage or an image of a bootsector.");
    puts("Wildcards can be used on systems that expand them."); 
}

/* process the command line */
int  main (int argc, char *argv[])
{
    int i,cnt;
    int  dirpointer;
    DirEntry dirbuf;
    
    if (sizeof(DirEntry)!=32) { 
	puts("please fix struct de and compile again"); 
	exit(2); 
    }
    puts("wrdsk version 1.6 by Arnold Metselaar, (c) 1996--1998");

    progname=(*argv[0])?argv[0]:"wrdsk";
  
    if (argc<2) { 
	help(); 
	exit(1); 
    }
  
    diskim=OpenDisk(argv[1]);
    
    dirpointer=0; cnt=0;
    for (i=2; i<argc; i++)
    {
	while( Used(dirpointer) && (dirpointer < ndir) ) 
	    dirpointer++;
      if (dirpointer<data_ofs) {
	  WriteFile(argv[i], &dirbuf);
	  if (dirbuf.d_fname[0]) {
	      DIR[dirpointer]=dirbuf;
	      cnt++;
	  }
      }
      else
	  printf("%s: Directory full, %s not written.\n", progname, argv[i]);
    }
    CloseDisk(diskim,argv[1]);
    printf("%d file%s added\n",cnt,(cnt==1)?"":"s");

    return 0;
}
