/*
 *  Created on: 19 июня 2016 г.
 *      Author: Vsevolod Lutovinov <klopp@yandex.ru>
 */

#ifndef PICOC_LIB_H_
#define PICOC_LIB_H_

#include "../picoc/picoc.h"

#define     PICOC_STACK_SIZE    1024*1024
#define     PICOC_OUTBUF_SIZE   1024*4
#define     PICOC_POINTERS_MAX  64

#ifdef UNIX_HOST
# define PICOC_DEV_NULL "/dev/null"
#else
# define PICOC_DEV_NULL "NUL"
#endif

typedef struct _PicoCLib {
    Picoc pc;
    int InitDebug;
    void *Pointers[PICOC_POINTERS_MAX];
    size_t nPointers;
    char PicocOutBuf[PICOC_OUTBUF_SIZE];
} PicoCLib;

/*
 * From picoc.c
 */
char *PlatformReadFile( Picoc *pc, const char *FileName );

PicoCLib *PicoCLibInit( PicoCLib *pc );
PicoCLib *PicoCLibReset( PicoCLib *pc );
void PicoCLibDown( PicoCLib *pc );
int PicoCLibMainFromFiles( PicoCLib *pc, const char *file, ... );
int PicoCLibMainFromSources( PicoCLib *pc, const char *source, ... );

int PicoCLibBindShort( PicoCLib *pc, const char *name, short *val );
int PicoCLibBindUShort( PicoCLib *pc, const char *name, unsigned short *val );
int PicoCLibBindInt( PicoCLib *pc, const char *name, int *val );
int PicoCLibBindUInt( PicoCLib *pc, const char *name, unsigned int *val );
int PicoCLibBindLong( PicoCLib *pc, const char *name, long *val );
int PicoCLibBindULong( PicoCLib *pc, const char *name, unsigned long *val );
int PicoCLibBindArray( PicoCLib *pc, const char *name, void *val );
int PicoCLibBindCharArray( PicoCLib *pc, const char *name, char *val );

#endif

/*
 *  That's All, Folks!
 */

