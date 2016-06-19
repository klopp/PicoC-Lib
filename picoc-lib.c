/*
 *  Created on: 19 июня 2016 г.
 *      Author: Vsevolod Lutovinov <klopp@yandex.ru>
 */

#include "picoc-lib.h"
#include <stdio.h>

PicoCLib *PicoCLibInit( PicoCLib *pc ) {
    PicocInitialise( &pc->pc, PICOC_STACK_SIZE );
    memset( pc->PicocOutBuf, 0, PICOC_OUTBUF_SIZE );
    pc->pc.CStdOut = fopen( PICOC_DEV_NULL, "a" );
    setvbuf( pc->pc.CStdOut, pc->PicocOutBuf, _IOFBF, PICOC_OUTBUF_SIZE );
    return pc;
}

void PicoCLibDown( PicoCLib *pc ) {
    if( pc->pc.CStdOut ) {
        fclose( pc->pc.CStdOut );
        PicocCleanup( &pc->pc );
        pc->pc.CStdOut = NULL;
    }
}

#define CALL_MAIN_NO_ARGS_RETURN_INT "__exit_value = main();"

static void PicoCLibCallMain( PicoCLib *pc ) {
    struct Value *FuncValue = NULL;
    if( !VariableDefined( &pc->pc, TableStrRegister( &pc->pc, "main" ) ) ) {
        ProgramFailNoParser( &pc->pc, "\"main()\" is not defined" );
    }
    VariableGet( &pc->pc, NULL, TableStrRegister( &pc->pc, "main" ), &FuncValue );
    if( FuncValue->Typ->Base != TypeFunction ) {
        ProgramFailNoParser( &pc->pc,
                             "\"main\" is not a function - can't call it" );
    }
    if( FuncValue->Val->FuncDef.NumParams != 0 ) {
        ProgramFailNoParser( &pc->pc, "\"main()\" can not have arguments" );
    }
    if( FuncValue->Val->FuncDef.ReturnType != &pc->pc.IntType ) {
        ProgramFailNoParser( &pc->pc, "\"main()\" must return int" );
    }
    VariableDefinePlatformVar( &pc->pc, NULL, "__exit_value", &pc->pc.IntType,
                               ( union AnyValue * ) &pc->pc.PicocExitValue, TRUE );
    PicocParse( &pc->pc, "startup", CALL_MAIN_NO_ARGS_RETURN_INT,
                strlen( CALL_MAIN_NO_ARGS_RETURN_INT ), TRUE, TRUE, FALSE, FALSE );
}

int PicoCLibMain( PicoCLib *pc, const char *file ) {
    PicoCLibDown( pc );
    PicoCLibInit( pc );
    if( PicocPlatformSetExitPoint( &pc->pc ) ) {
        return 1;
    }
    PicocPlatformScanFile( &pc->pc, file );
    PicoCLibCallMain( pc );
    return 0;
}

static int PicoCLibBind( PicoCLib *pc, const char *name, void *val,
                         struct ValueType *type ) {
    fflush( pc->pc.CStdOut );
    if( PicocPlatformSetExitPoint( &pc->pc ) ) {
        return 1;
    }
    VariableDefinePlatformVar( &pc->pc, NULL, ( char * ) name, type,
                               ( union AnyValue * ) val, 1 );
    return 0;
}

void PicoCLibUnbind( PicoCLib *pc, const char *name ) {
    TableDelete( &pc->pc, &pc->pc.GlobalTable, TableStrRegister( &pc->pc, name ) );
}

int PicoCLibBindShort( PicoCLib *pc, const char *name, short *val ) {
    return PicoCLibBind( pc, name, val, &pc->pc.ShortType );
}
int PicoCLibBindUShort( PicoCLib *pc, const char *name, unsigned short *val ) {
    return PicoCLibBind( pc, name, val, &pc->pc.UnsignedShortType );
}
int PicoCLibBindInt( PicoCLib *pc, const char *name, int *val ) {
    return PicoCLibBind( pc, name, val, &pc->pc.IntType );
}
int PicoCLibBindUInt( PicoCLib *pc, const char *name, unsigned int *val ) {
    return PicoCLibBind( pc, name, val, &pc->pc.UnsignedIntType );
}
int PicoCLibBindLong( PicoCLib *pc, const char *name, long *val ) {
    return PicoCLibBind( pc, name, val, &pc->pc.LongType );
}
int PicoCLibBindULong( PicoCLib *pc, const char *name, unsigned long *val ) {
    return PicoCLibBind( pc, name, val, &pc->pc.UnsignedLongType );
}

int PicoCLibBindPtr( PicoCLib *pc, const char *name, void *val ) {
    return PicoCLibBind( pc, name, val, &pc->pc.VoidPtrType );
}
int PicoCLibBindCharPtr( PicoCLib *pc, const char *name, char *val ) {
    return PicoCLibBind( pc, name, val, &pc->pc.CharPtrType );
}


/*
 *  That's All, Folks!
 */

