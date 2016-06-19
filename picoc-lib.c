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

#define CALL_MAIN_NO_ARGS_RETURN_INT "__exit_value = main();"

static void PicoCLibCallMain( PicoCLib *pc ) {
    struct Value *FuncValue = NULL;
    if( !VariableDefined( &pc->pc, TableStrRegister( &pc->pc, "main" ) ) ) {
        ProgramFailNoParser( &pc->pc, "\"main()\" is not defined" );
    }
    VariableGet( &pc->pc, NULL, TableStrRegister( &pc->pc, "main" ), &FuncValue );
    if( FuncValue->Typ->Base != TypeFunction ) {
        ProgramFailNoParser( &pc->pc, "\"main\" is not a function - can't call it" );
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
    fflush( pc->pc.CStdOut );
    memset( pc->PicocOutBuf, 0, PICOC_OUTBUF_SIZE );
    if( PicocPlatformSetExitPoint( &pc->pc ) ) {
        return pc->pc.PicocExitValue;
    }
    PicocPlatformScanFile( &pc->pc, file );
    PicoCLibCallMain( pc );
    return pc->pc.PicocExitValue;
}

static int PicoCLibBind( PicoCLib *pc, const char *name, void *val,
                         struct ValueType *type ) {
    fflush( pc->pc.CStdOut );
    if( PicocPlatformSetExitPoint( &pc->pc ) ) {
        return 1;
    }
    VariableDefinePlatformVar( &pc->pc, NULL, ( char * )name, type,
                               ( union AnyValue * ) val,
                               1 );
    return 0;
}

int PicoCLibBindInt( PicoCLib *pc, const char *name, int *val ) {
    return PicoCLibBind( pc, name, val, &pc->pc.IntType );
}

int PicoCLibBindUInt( PicoCLib *pc, const char *name, unsigned int *val ) {
    return PicoCLibBind( pc, name, val, &pc->pc.UnsignedIntType );
}

/*
 *  That's All, Folks!
 */

