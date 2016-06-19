/*
 *  Created on: 19 июня 2016 г.
 *      Author: Vsevolod Lutovinov <klopp@yandex.ru>
 */

#include "picoc-lib.h"
#include <stdio.h>

PicoCLib *PicoCLibInit( PicoCLib *pc ) {
    if( !pc ) {
        pc = malloc( sizeof( PicoCLib ) );
    }
    if( pc ) {
        memset( pc, 0, sizeof( PicoCLib ) );
        PicocInitialise( &pc->pc, PICOC_STACK_SIZE );
        pc->pc.CStdOut = fopen( PICOC_DEV_NULL, "a" );
        setvbuf( pc->pc.CStdOut, pc->PicocOutBuf, _IOFBF, PICOC_OUTBUF_SIZE );
    }
    return pc;
}

void PicoCLibDown( PicoCLib *pc ) {
    if( pc ) {
        fclose( pc->pc.CStdOut );
        PicocCleanup( &pc->pc );
    }
}

/*
 *  That's All, Folks!
 */

