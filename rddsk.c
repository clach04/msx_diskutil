/*************************************************************/
/**                                                         **/
/**                rddsk.c                                  **/
/**                                                         **/
/** Program to read files from disk-images that can be      **/
/**  used with fMSX                                         **/
/**                                                         **/
/**                                                         **/
/** Copyright (c) Arnold Metselaar 1996                     **/
/**     You are not allowed to distribute this software     **/
/**     commercially. Please, notify me, if you make any    **/
/**     changes to this file.                               **/
/*************************************************************/

/* Version history
 * 1.1 (26 feb 1997)
 *  - added -t option to view directory of image
 * 1.2 ( 6 may 1997)
 *  - stricter checking of diskimage
 *  - more informative error messages
 *  - added -l option for longer view of directory
 * 1.4 (26 may 1997)
 *  - read the FAT! (got lost somehow)
 *  - layout changed
 * 1.5 (27 aug 1997)
 *  - using less flags in open-calls
 *  - some features can be suppressed by compiling with -DPOOR
 *  - can be compiled on MSX using HITECH-C 
 * 1.6 (11 sep 1998)
 *  - clusters no longer need to be 1024 bytes
 *  - use longs to compute the offset in diskimages
 *  - progname set to "rddsk" if argv[0] is not set 
 */

#ifdef HI_TECH_C
#ifdef z80
#define POOR
#endif 
#endif

#include <stdio.h>
#include <unistd.h>
#include <stddef.h>
#include <string.h>
#include <ctype.h>
#include <time.h>

#ifndef POOR
#include <fcntl.h>
#include <utime.h>
#include <sys/errno.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/time.h>
#endif /* POOR */

#include "DiskUtil.h"

/* general */
int /*fd*/  diskim; /* file descriptor for diskimage */
char *progname; /* name of program (for PError) */ 

/* drive parameters */
usint maxcl; /* highest clusternumber */
long  dir_ofs,  /* offset of first byte of directory */
      data_ofs, /* offset of first byte of data  */
      clus_len;  /* length of one cluster (in bytes) */

/* buffers */
byte *FAT;
char secbuf[seclen];
byte BootBlock[seclen];

int Toupper(int c)
{ return (islower(c)?toupper(c):c); }

void mask(char *arg, char *p)
{
    int k;
    char c;

    k=0;
    while (k<11) {
	c=Toupper(*arg);
	switch (c) {
	case '.' : 
	    if (k<8) p[k++]=' ';
	    else arg++;
	    break;
	case '*' : 
	    p[k++]='?'; 
	    if (k==8) arg++;
	    break;
	case '\0': 
	    p[k++]=' '; 
	    break;
	default: 
	    p[k++]=c;
	    arg++;
	};
    }
}

char * masks(int argc, char **argv, int n)
{
    char *res;
    int j;

    if (n==argc) return NULL;
    
    if (argc>n) {
	res=xalloc((argc-n)*11+1);
	for (j=n; j<argc; j++) 
	    mask(argv[j],res+11*(j-n));
	res[(argc-n)*11]='\0';
    }
    else res=NULL;
    return res;
}

int matches(char *fn, char *list)
{
  int k;
  char *p;

  if ((fn[0]!='\345') && (fn[0]!='\0')) {
      if (list==NULL) return 1;

      for (p=list; *p ; p+=11) {
	  for (k=0 ; (k<11) && ( fn[k]==p[k] || p[k]=='?' ); k++)
	      ;
	  if (k==11) return 1;
      }
  }
  return 0;
}

/* open a diskimage file for reading */
int OpenDisk(char *DiskName)
{
    int /*fd*/ res,i;
    long len;
    size_t FAT_len;
    byte cl;

    if ((res=open(DiskName,O_RDONLY|O_BINARY))==-1) {
	PError("%s: Can't open %s for reading",DiskName);
	exit(2);
    }
    else {
	if((lseek(res,0,SEEK_SET)==-1)
	   || (read(res, BootBlock, seclen)!=seclen)) {
	    PError("%s: Can't read bootsector from %s", DiskName);
	    exit(2);
	}

	if ((len=lseek(res, 0, SEEK_END))==-1) {
	    PError("%s: Can't get length from %s", DiskName);
	    exit(2);
 	}

	if (len!= (long)seclen *rdsh((BootBlock+0x13))) {
	    fprintf(stderr,"%s: %s does not have the right length\n",
		    progname, DiskName); 
	    exit(2);
	}

	for ( cl=BootBlock[0x0D],i=0 ; cl>1 ; cl>>=1 )
	    i++;
	clus_len=(long)seclen << i;

	if((rdsh((BootBlock+0x0B))!=seclen) || (cl!=1)) {
	    fprintf(stderr,"%s: %s does not seem to be a valid (MSX-DOS) diskimage\n",
		    progname, DiskName); 
	    exit(2);
	}
	dir_ofs=(long)seclen*(1+((int)BootBlock[0x10])*((int)BootBlock[0x16]));
	data_ofs=dir_ofs+((long)rdsh(BootBlock+0x11))*sizeof(DirEntry);
	maxcl=(rdsh(BootBlock+0x13)-data_ofs/seclen)/((int)BootBlock[0x0D])+1;

	FAT_len=(maxcl*3+1)/2;
	FAT=xalloc(FAT_len);

	if((lseek(res, seclen, SEEK_SET)==-1) 
	   ||(read(res,FAT,FAT_len)!=FAT_len))
	{
	    PError("%s: Can't read FAT from %s", DiskName);
	    exit(2);
	}
    }
    printf("Diskimage: %s\n",DiskName);

    return res;
} 

/* close diskimage file */
void CloseDisk(int /*fd*/  fp,char *DiskName)
{
    if (close(fp))
	PError("%s: while closing %s",DiskName);
}

struct tm * rdtime(DirEntry *msxf)
{
    static struct tm mtim;
    usint t;
    time_t tim;
    
    tim=time(NULL);
    mtim=*localtime(&tim);/* get timezone info */
    mtim.tm_isdst=-1;     /* check for daylight saving time */
    
    t=rdsh(msxf->d_time);
    mtim.tm_sec =(t&0x001F)<<1;
    mtim.tm_min =(t&0x07E0)>>5;
    mtim.tm_hour=(t&0xF100)>>11;
    t=rdsh(msxf->d_date);
    mtim.tm_mday= (t&0x001F);
    mtim.tm_mon =((t&0x01E0)>>5)-1;
    mtim.tm_year=((t&0xFE00)>>9)+1980-1900;

    mktime(&mtim); /* fill in tm_wday and tm_yday */
    return &mtim;
}

int Tolower(int c)
{ return (isupper(c)?tolower(c):c); }

/* copy  a file from the diskimage */
void ReadFile(DirEntry *msxf)
{
    unsigned long size;
    int i,j;
    int fd;
    usint curcl;
    char r, name[13];

    for(i=j=0; i<8 && msxf->d_fname[i]!=' ';) 
	name[j++]=Tolower(msxf->d_fname[i++]);
    name[j++]='.';
    for(i=0; i<3 && msxf->d_ext[i]!=' ';) 
	name[j++]=Tolower(msxf->d_ext[i++]);
    if (name[j-1]=='.') j--;
    name[j]='\0';

    if ( (fd=open(name,O_RDONLY|O_BINARY)) != -1) {
	close(fd);
	printf("%s: replace `%s'? ", progname, name);
	do  
	    r=getchar();
	while ( !strchr("YyNn",r) );
	if (Toupper(r)=='N')
	    return;
    }

    printf("%.8s.%.3s ==> %s\n",msxf->d_fname,msxf->d_ext,name);
    if ((fd=creat(name,0666))==-1) { 
	PError("%s: Can't create %s", name); 
	exit(2);
    }
    close(fd);
    if ( (fd=open(name,O_WRONLY|O_BINARY))==-1) {
	PError("%s: Can't open %s for writing",name);
	return;
    }

    size=rdlg(msxf->d_size);
    curcl=rdsh(msxf->d_first);

    while(size && (curcl>=2) && (curcl<=maxcl) ) {
      	lseek(diskim, data_ofs+clus_len*(curcl-2), SEEK_SET);
	for ( i=0 ; (i < BootBlock[0x0D]) && size ; i++ ) {
	    read(diskim, secbuf, seclen);
	    write(fd, secbuf, (size>seclen)?seclen:size );
	    size -= ((size>seclen)?seclen:size);
	}
	curcl=ReadFAT(curcl,FAT);
    }
    if (size)
	fprintf(stderr,"%s: diskimage corrupt, %s truncated.\n", progname, name);

    if (close(fd)) 
	PError("%s: while closing %s",name);

#ifndef POOR
    /* use msx-dos time-stamp to set modification time */
    {
	struct utimbuf tbuf;
	
	tbuf.actime =time(NULL);
	tbuf.modtime=mktime(rdtime(msxf));
	utime(name, &tbuf);
    }
#endif POOR
}

void ls(DirEntry *de)
{	      
    printf("%.8s.%.3s\t",de->d_fname,de->d_ext);
}

void lsl(DirEntry *de)
{	      
    struct tm *tp;
    static char datebuf[60];

    tp=rdtime(de);
#ifndef POOR
    strftime(datebuf, 60, "%c", tp);
#else
    strncpy(datebuf, asctime(tp), 60);
#endif /* POOR */
    printf(
#ifndef POOR
	"%.8s.%.3s %6ld %s\n",
#else
	"%.8s.%.3s %6ld %s",
#endif
	de->d_fname,de->d_ext, 
	(long)rdlg(de->d_size), datebuf);
}

/* Give some help */
void help(void)
{
    puts("Extract files from a diskimage used with fmsx.");
#ifndef POOR
    printf("Usage: %s <diskimage> [-d <directory>|-l|-t] [<fspec1>] [<fspec2>] ...\n",
	   progname);
    puts("Use -d <directory> to specify destination directory.");
#else
    printf("Usage: %s <diskimage> [-l|-t] [<fspec1>] [<fspec2>] ...\n",
	   progname);
#endif
    puts("Use -l to list filenames with size and date.");
    puts("Use -t to list filenames only.");
    puts("If <fspec>'s  are given only the files matching at least one ");
    puts("  of the <fspec>'s are extracted or listed, following MSX-DOS-rules.");
    puts("Otherwise all files are extracted or listed.");
    puts("To prevent wildcards in the <fspec>'s from being expanded by the ");
    puts("  shell, enclose the <fspec>'s in quotes.");
}

/* process the command line */
int  main (int argc, char* argv[])
{ 
    DirEntry de;
    int n;
    long i;
    char *fl;
    void (*actie)(DirEntry*);
    
    if (sizeof(DirEntry)!=32) { 
	puts("please fix struct de and compile again"); 
	exit(2); 
    }
    puts("rddsk version 1.6 by Arnold Metselaar, (c) 1996--1998");
    
    progname=(*argv[0])?argv[0]:"rddsk";
    if (argc<2) { 
	help(); 
	exit(1); 
    }
    
    diskim=OpenDisk(argv[1]);
    actie=ReadFile;
    n=2;
#ifndef POOR
    if ( (argc>2) && !strcmp(argv[2],"-d")) {
	n=4;
	if (argc>3) {
	    if (chdir(argv[3])) { 
		PError("%s: chdir %s",argv[3]); 
		exit(2);
	    }
	    else
		printf("Directory: %s\n",argv[3]);
	}
	else 
	    printf("%s: No directory supplied", progname);
    }
#endif /* POOR */
    if ( (argc>2) && !strcmp(argv[2],"-t")) { 
	actie=ls; 
	n++; 
    }
    if ( (argc>2) && !strcmp(argv[2],"-l")) {
	actie=lsl;
	n++; 
    }
  
    fl=masks(argc, argv, n);

    for(i=dir_ofs; i<data_ofs; i+=sizeof(DirEntry)) {
	lseek(diskim,i,SEEK_SET);
	read( diskim,&de,sizeof(DirEntry));
	if (matches(de.d_fname, fl))
	    actie (&de);
    }

    if (actie==ls) printf("\n");
    CloseDisk(diskim, argv[1]);

    return 0;
}
