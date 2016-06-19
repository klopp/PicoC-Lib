/*
 *  Created on: 19 июня 2016 г.
 *      Author: Vsevolod Lutovinov <klopp@yandex.ru>
 */

#ifndef PICOC_LIB_H_
#define PICOC_LIB_H_

#include "../picoc/picoc.h"

#define     PICOC_STACK_SIZE    1024*1024
#define     PICOC_OUTBUF_SIZE   1024*4

#ifdef UNIX_HOST
# define PICOC_DEV_NULL "/dev/null"
#else
# define PICOC_DEV_NULL "NUL"
#endif

typedef struct _PicoCLib {
    Picoc pc;
    char PicocOutBuf[PICOC_OUTBUF_SIZE];
} PicoCLib;

PicoCLib *PicoCLibInit( PicoCLib *pc );
PicoCLib *PicoCLibReset( PicoCLib *pc );
void PicoCLibDown( PicoCLib *pc );
int PicoCLibMain( PicoCLib *pc, const char *file );

void PicoCLibUnbind( PicoCLib *pc, const char *name );

int PicoCLibBindShort( PicoCLib *pc, const char *name, short *val );
int PicoCLibBindUShort( PicoCLib *pc, const char *name, unsigned short *val );
int PicoCLibBindInt( PicoCLib *pc, const char *name, int *val );
int PicoCLibBindUInt( PicoCLib *pc, const char *name, unsigned int *val );
int PicoCLibBindLong( PicoCLib *pc, const char *name, long *val );
int PicoCLibBindULong( PicoCLib *pc, const char *name, unsigned long *val );
int PicoCLibBindPtr( PicoCLib *pc, const char *name, void *val );
int PicoCLibBindCharArray( PicoCLib *pc, const char *name, char *val );

#endif

/*
 *  That's All, Folks!
 */

