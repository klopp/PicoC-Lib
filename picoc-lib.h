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
void PicoCLibDown( PicoCLib *pc );
int PicoCLibMain( PicoCLib *pc, const char *file );



#endif

/*
 *  That's All, Folks!
 */

