/*
 *  Created on: 19 июня 2016 г.
 *      Author: Vsevolod Lutovinov <klopp@yandex.ru>
 */

#ifndef PICOC_LIB_H_
#define PICOC_LIB_H_

#include "../picoc/picoc.h"
#include <limits.h>

#ifndef UNIX_HOST
# define PATH_MAX MAX_PATH
#endif

#define     PICOC_STACK_SIZE            1024*1024
#define     PICOC_OUTBUF_SIZE           PATH_MAX+PATH_MAX
#define     PICOC_ARRAY_POINTERS_MAX    64

#ifdef UNIX_HOST
# define PICOC_DEV_NULL "/dev/null"
#else
# define PICOC_DEV_NULL "NUL"
#endif

typedef struct _PicoCLib {
    Picoc pc;
#ifndef NO_DEBUGGER
    int InitDebug;
#endif
    void *ArrayPointers[PICOC_ARRAY_POINTERS_MAX];
    size_t nArrayPointers;
    char PicocOutBuf[PICOC_OUTBUF_SIZE];
} PicoCLib;

/*
 * From picoc.c
 */
char *PlatformReadFile( Picoc *pc, const char *FileName );

PicoCLib *PicoCLibInit( PicoCLib *pc );
void PicoCLibDown( PicoCLib *pc );
PicoCLib *PicoCLibReset( PicoCLib *pc );

void PicoCLibClearFileVars( PicoCLib *pc, const char *file );
void PicoCLibClearMainVars( PicoCLib *pc );

int PicoCLibMainFromFile( PicoCLib *pc, const char *file );
int PicoCLibMainFromSource( PicoCLib *pc, const char *source );

int PicoCLibLoadFiles( PicoCLib *pc, const char *file, ... );
int PicoCLibLoadSources( PicoCLib *pc, const char *source, ... );

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

