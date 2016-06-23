/*
 *  Created on: 19 июня 2016 г.
 *      Author: Vsevolod Lutovinov <klopp@yandex.ru>
 */

#ifndef PICOC_LIB_H_
#define PICOC_LIB_H_

#include "../picoc/picoc.h"

#include <string.h>
#include <stdio.h>

#ifdef __unix
# include <linux/limits.h>
# include <stdarg.h>
# define PICOC_DEV_NULL     "/dev/null"
#else
# include <windows.h>
# define PICOC_DEV_NULL     "NUL"
# define PATH_MAX           MAX_PATH
#endif

#define     TCC_ERROR_BUF_SIZE      PATH_MAX+PATH_MAX

#define     PICOC_STACK_SIZE            1024*1024
#define     PICOC_OUTBUF_SIZE           PATH_MAX+PATH_MAX
#define     PICOC_ARRAY_POINTERS_MAX    64
#define     PICOC_MAX_ARGS              16
#define     PICOC_FUNCNAME_MAX          256
#define     PICOC_FUNCTION_RET          "__f_ret_%u="
#define     PICOC_FUNCTION_ARG          "__f_arg_%u"
#define     PICOC_CALLSTR_SIZE          sizeof(PICOC_FUNCTION_RET) + \
                                        PICOC_FUNCNAME_MAX + \
                                        ((sizeof(PICOC_FUNCTION_ARG)+8) * \
                                        PICOC_MAX_ARGS) + \
                                        32
#define     PICOC_ARG_SEPARATORS        "\t\n\r ,-:;"

typedef struct _PicoCLib {
    Picoc pc;
    unsigned curid;
    int InitDebug;
    struct {
        void *val;
        char *name;
    } ArrayPointers[PICOC_ARRAY_POINTERS_MAX];
    char PicocOutBuf[PICOC_OUTBUF_SIZE];
} PicoCLib;

typedef struct _PicoCLibFunc {
    PicoCLib *pc;
    unsigned argmin;
    unsigned argmax;
    char name[PICOC_FUNCNAME_MAX + 8];
    char callstr[PICOC_CALLSTR_SIZE];
    char fmt[PICOC_MAX_ARGS + 1];
    struct {
        union AnyValue arg;
        char name[sizeof( PICOC_FUNCTION_ARG ) + 8];
    } args[PICOC_MAX_ARGS];
    struct ValueType *retptr;
    union AnyValue rc;
} PicoCLibFunc;

PicoCLibFunc *PicoCLibFunction( PicoCLib *pc, enum BaseType ret,
                                const char *name, const char *fmt );
int PicoCLibCall( PicoCLibFunc *function, ... );

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
int PicoCLibUnbindArray( PicoCLib *pc, void *val );

/*
 * format (uppercase for unsigned):
 *  c - char
 *  s - short
 *  i - int
 *  l - long
 *  p - pointer
 *  z - char array (string)
 *  space, ",", "-", ":", ";", \t, \r, \n - separator, skipped
 *  Set pc->pc.PicocExitValue to != 0 on errors;
 */
union AnyValue PicoCLibCallFunction( PicoCLib *pc, enum BaseType ret,
                                             const char *name, const char *fmt, ... );
/*
 * From ../picoc/picoc.c etc
 */
char *PlatformReadFile( Picoc *pc, const char *FileName );

#endif

/*
 *  That's All, Folks!
 */

