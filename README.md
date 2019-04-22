# MSX disk image utils rddsk and wrdsk

This is a copy of (the no longer available) DiskUtil16.tar.gz from http://www.math.utwente.nl/~metselaa/msx/diskutil.html

The readme as it appeared is reproduced below (NOTE notes about fMSX including rddsk and wrdsk no longer appears to be correct as of April 2019).


# Programs to handle disk images

I made programs to copy files to and from disk images as used by [fMSX](https://web.archive.org/web/20020204045404/http://www.komkon.org/fms/fMSX/), an [MSX](https://web.archive.org/web/20020204045404/http://www.komkon.org/fms/MSX/)-emulator written by Marat Fayzullin, for which I have also written [a set of X11-drivers](X-drivers.html). They make strictly binary copies so you may need additional conversion programs. Use rddsk to read files from disk images and wrdsk to write files to disk images. Get help by running the programs without arguments. Both programs support modification times using the local time on the disk image.

I have done some testing on these programs but I can not guarantee correctness so use them with some caution. A known problem is that wrdsk may create disk images with duplicate filenames, this may be helped by renaming the files with duplicate names using the emulator. This will cause some rename errors but you will end up with unique filenames only. Another known problem is that rddsk does not check whether writes have been successful I know how to fix it but it will take some time before I have done so and checked the new version. Before that you may want to compare the length of the copy with the file-length in the disk image (use `rddsk -l` for the latter)

If you are working under HP-UX you could also use `doscp` and other utilities starting with `dos` to work with disk images. See the manuals on your system for details. Users of windows 95 or windows NT 4.0 who like their mouse, may prefer using the program [Disk-Manager](https://web.archive.org/web/20020204045404/http://members.eunet.at/lexlechz/DiskMgr.html), written by [Rudolf Lechleitner](https://web.archive.org/web/20020204045404/http://members.eunet.at/lexlechz)

.

## Sources

This site always has the latest release, which is now version 1.6. The sources included in the [main fMSX-distribution](https://web.archive.org/web/20020204045404/http://www.komkon.org/fms/fMSX/) can be out of date. Starting from version 1.5 there is an extra source file (DiskUtil.c) which should also be compiled. If you use the makefile from the main distribution add " `DiskUtil.c`" to the following lines

    contrib:rddsk.c wrdsk.c DiskUtil.h Boot.h
            ${CC} -o rddsk rddsk.c
            ${CC} -o wrdsk wrdsk.c



[wrdsk.c](wrdsk.c) (9 418 bytes, Mon Sep 14 09:38:55 CEST 1998 )

Use this program to copy files to disk images. It can also be used to create new disk images.

[rddsk.c](rddsk.c) (9 570 bytes, Fri Sep 11 13:14:42 CEST 1998 )

Use this program to copy files from disk images. It can also be used to list the contents of a disk image.

[DiskUtil.c](DiskUtil.c) (1 834 bytes, Fri Sep 11 12:53:14 CEST 1998 )

This file contains some common functions for wrdsk and rddsk.

[DiskUtil.h](DiskUtil.h) (2 039 bytes, Fri Sep 11 12:18:14 CEST 1998 )

This file contains some common definitions and declarations for wrdsk and rddsk.

[Boot.h](Boot.h) (3 448 bytes, Wed Nov 13 13:03:06 CET 1996 )

This file contains the boot-sector (`byte BootBlock[]`), that is used by wrdsk to make a new disk image. The same boot-sector is used by fMSX to format a disk.



You can get the sources above at once as a [gzipped tar-archive](DiskUtil16.tar.gz) (7 323 bytes, Mon Sep 14 11:35:23 CEST 1998 ).

For the HI-TECH C-compiler on MSX you will also need the following files, which may also be useful on other systems:



[mktime.c](mktime.c) (2 601 bytes, Wed Aug 27 14:36:57 CEST 1997 )

This is an implementation of the `mktime()`-function, that is used by rddsk. Like all time related functions on MSX it doesn't know about time-zones.

[unistd.h](unistd.h) (192 bytes, Mon Aug 25 15:20:28 CEST 1997 )

This file just includes `<unixio.h>` and provides some `#define`'s that are missing that file.



## Compiled versions

Compiled versions are available for the following systems:



MSX-DOS

The programs have been compiled using Hi_Tech C under MSX-DOS 1\. Version 1.5 for MSX was very bad but version 1.6 has undergone more extensive tests and seems ok. These programs were linked with the c-libraries provided with Hi_Tech C, which have some limitations, see my [wish list](#wish_list).

*   [wrdsk.com](wrdsk.com) (20 608 bytes, Mon Sep 14 11:35:17 CEST 1998 )
*   [rddsk.com](rddsk.com) (17 664 bytes, Mon Sep 14 11:35:17 CEST 1998 )

You can also download both programs as a single [zip-file](dsk_msx.zip) (18 559 bytes, Mon Sep 14 11:35:27 CEST 1998 ).

MS-DOS

Compiled versions for MS-DOS are available from the [fMSX MS-DOS Distribution Site](https://web.archive.org/web/20020204045404/http://www.komkon.org/~dekogel/fmsx.html).



## Wish list

There are some things that could be improved in these programs, but I think they are already useful as they are now.

*   Checking whether writes are successful in rddsk (in progress).
*   Checking filenames to preventing creating duplicate filenames in disk images.
*   Creating an option to delete files in disk images.
*   The libraries from Hi_Tech C could also be improved:
    *   Rddsk.com should be able to create file with sizes that are not a multiple of 128 bytes but for most purposes the padding is not a real problem.
    *   It would be nice if rddsk.com could read disk images on a cd-rom directly.

If you would like to contribute to these programs, it is wise to contact me before you start. Otherwise we might end up doing things twice.

## Changes

### April 4 1997

Some users reported problems problems with wrdsk under MS-DOS. These problems were caused by opening the files to copy to the disk image, in text-mode (i.e. without `|O_BINARY`), this makes no difference under unix, but it does under systems that have other conventions for text-files. Anyway, it is fixed now.

### May 6 1997

I have added some extra checks to reduce the chance of wrdsk spoiling something that is not a disk image-file. rddsk now has a `-l` option to show the directory of the disk image in long format. Both programs gives more informative error messages now.

### May 26 1997

I fixed some bugs related to not reading the Fat from the disk image. wrdsk no longer thinks existing disk images are too long.

### August 27 1997

The programs have been adapted for compilation for MSX-DOS using the [HI-TECH](https://web.archive.org/web/20020204045404/http://www.htsoft.com/) [C-compiler](https://web.archive.org/web/20020204045404/ftp://ftp.funet.fi/pub/msx/c/). I had to make work-arounds for some flaws in the library and to simplify some expressions. The programs now call `creat()` to truncate or create files, the only flags used in `open()`-calls are `O_RDONLY`, `O_WRONLY`, `O_RDWR` and `O_BINARY`. This should make it easier to port the programs to other platforms. The time-stamp-support can be suppressed by compiling with `-DPOOR`.

### September 14 1998

I fixed some bugs involving using int's instead of long int's for computing the offset in diskimages and a similar problem in computing numbers in `DirEntry`'s. These caused problems on systems where int's default to short. A bug in the Hi-Tech C compiler also caused some problems. The programs now allow disk images with cluster that are not 1024 bytes in size.

* * *

This page can be found at `[http://www.math.utwente.nl](https://web.archive.org/web/20020204045404/http://www.math.utwente.nl/)/[~metselaa](https://web.archive.org/web/20020204045404/http://www.math.utwente.nl/~metselaa/)/[msx](https://web.archive.org/web/20020204045404/http://www.math.utwente.nl/~metselaa/msx/)/[diskutil.html](https://web.archive.org/web/20020204045404/http://www.math.utwente.nl/~metselaa/msx/diskutil.html)`  
and is made by [Arnold Metselaar](/web/20020204045404/http://www.math.utwente.nl/~metselaa) ([A.Metselaar@math.utwente.nl](https://web.archive.org/web/20020204045404/mailto:A.Metselaar@math.utwente.nl)).  
It was generated Tue Jan 22 09:21:53 CET 2002 from a file that was last modified on Tue Nov 20 14:00:04 CET 2001 .