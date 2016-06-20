/*
 *  Created on: 19 июня 2016 г.
 *      Author: Vsevolod Lutovinov <klopp@yandex.ru>
 */
#include "picoc-lib.h"

/* -----------------------------------------------------------------------------
 *
 -----------------------------------------------------------------------------*/
#define MAIN_EXIT_CODE      "__exit_value"
#define INT_MAIN_VOID       MAIN_EXIT_CODE " = main();"

/* -----------------------------------------------------------------------------
 *
 -----------------------------------------------------------------------------*/
PicoCLib *PicoCLibReset( PicoCLib *pc ) {
    PicoCLibDown( pc );
    return PicoCLibInit( pc );
}

void PicoCLibClearFileVars( PicoCLib *pc, const char *file ) {
    struct TableEntry **EntryPtr;
    short i;
    for( i = 0; i < pc->pc.GlobalTable.Size; i++ ) {
        EntryPtr = &pc->pc.GlobalTable.HashTable[i];
        while( EntryPtr && *EntryPtr ) {
            if( ( *EntryPtr )->DeclFileName
                    && !strcmp( ( *EntryPtr )->DeclFileName, file ) ) {
                struct TableEntry *DeleteEntry = *EntryPtr;
                *EntryPtr = DeleteEntry->Next;
                HeapFreeMem( &pc->pc, DeleteEntry );
            }
            else {
                EntryPtr = &( *EntryPtr )->Next;
            }
        }
    }
}

void PicoCLibClearMainVars( PicoCLib *pc ) {
    TableDelete( &pc->pc, &pc->pc.GlobalTable,
                 TableStrRegister( &pc->pc, "main" ) );
    TableDelete( &pc->pc, &pc->pc.GlobalTable,
                 TableStrRegister( &pc->pc, MAIN_EXIT_CODE ) );
}

PicoCLib *PicoCLibInit( PicoCLib *pc ) {
    PicocInitialise( &pc->pc, PICOC_STACK_SIZE );
    memset( pc->PicocOutBuf, 0, PICOC_OUTBUF_SIZE );
    memset( pc->ArrayPointers, 0, PICOC_ARRAY_POINTERS_MAX * sizeof( void * ) );
    pc->nArrayPointers = 0;
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

/* -----------------------------------------------------------------------------
 *
 -----------------------------------------------------------------------------*/
static int PicoCLibCallMain( PicoCLib *pc ) {
    struct Value *FuncValue = NULL;
    if( PicocPlatformSetExitPoint( &pc->pc ) ) {
        return 1;
    }
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
    VariableDefinePlatformVar( &pc->pc, NULL, MAIN_EXIT_CODE, &pc->pc.IntType,
                               ( union AnyValue * ) &pc->pc.PicocExitValue, TRUE );
    PicocParse( &pc->pc, "[main]", INT_MAIN_VOID, strlen( INT_MAIN_VOID ), TRUE,
                TRUE,
                FALSE, pc->InitDebug );
    return 0;
}

/* -----------------------------------------------------------------------------
 *
 -----------------------------------------------------------------------------*/
int PicoCLibMainFromFile( PicoCLib *pc, const char *file ) {
    return PicoCLibLoadFiles( pc, file, NULL ) ? 1 : PicoCLibCallMain( pc );
}

int PicoCLibMainFromSource( PicoCLib *pc, const char *source ) {
    return PicoCLibLoadSources( pc, source, NULL ) ? 1 : PicoCLibCallMain( pc );
}

/* -----------------------------------------------------------------------------
 *
 -----------------------------------------------------------------------------*/
int PicoCLibLoadSources( PicoCLib *pc, const char *source, ... ) {
    va_list ap;
    char file[0x20];
    unsigned int idx = 1;
    const char *current = source;
    va_start( ap, source );
    if( PicocPlatformSetExitPoint( &pc->pc ) ) {
        va_end( ap );
        return 1;
    }
    while( current ) {
        sprintf( file, "[source/%04u]", idx );
        PicocParse( &pc->pc, file, current, strlen( current ), TRUE, FALSE, TRUE,
                    pc->InitDebug );
        current = va_arg( ap, char * );
    }
    //PicoCLibCallMain( pc );
    va_end( ap );
    return 0;
}

/* -----------------------------------------------------------------------------
 *
 -----------------------------------------------------------------------------*/
int PicoCLibLoadFiles( PicoCLib *pc, const char *file, ... ) {
    va_list ap;
    const char *current = file;
    va_start( ap, file );
    if( PicocPlatformSetExitPoint( &pc->pc ) ) {
        va_end( ap );
        return 1;
    }
    while( current ) {
        char *SourceStr = PlatformReadFile( &pc->pc, current );
        /* ignore "#!/path/to/picoc" .. by replacing the "#!" with "//" */
        if( SourceStr && SourceStr[0] == '#' && SourceStr[1] == '!' ) {
            SourceStr[0] = '/';
            SourceStr[1] = '/';
        }
        PicocParse( &pc->pc, current, SourceStr, strlen( SourceStr ), TRUE, FALSE,
                    TRUE, pc->InitDebug );
        current = va_arg( ap, char * );
    }
    //PicoCLibCallMain( pc );
    va_end( ap );
    return 0;
}

/* -----------------------------------------------------------------------------
 *
 -----------------------------------------------------------------------------*/
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

/* -----------------------------------------------------------------------------
 *
 -----------------------------------------------------------------------------*/
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

/* -----------------------------------------------------------------------------
 *
 -----------------------------------------------------------------------------*/
int PicoCLibBindArray( PicoCLib *pc, const char *name, void *val ) {
    if( pc->nArrayPointers >= PICOC_ARRAY_POINTERS_MAX ) {
        fprintf( pc->pc.CStdOut,
                 "PicoCLibBindArray(): %u array pointers already exists",
                 PICOC_ARRAY_POINTERS_MAX );
        return 1;
    }
    pc->ArrayPointers[pc->nArrayPointers] = val;
    pc->nArrayPointers++;
    return PicoCLibBind( pc, name, &pc->ArrayPointers[pc->nArrayPointers - 1],
                         pc->pc.VoidPtrType );
}
int PicoCLibBindCharArray( PicoCLib *pc, const char *name, char *val ) {
    return PicoCLibBind( pc, name, val, pc->pc.CharArrayType );
}

/* -----------------------------------------------------------------------------
 *
 -----------------------------------------------------------------------------*/
#if defined(NO_DEBUGGER)
void DebugCheckStatement( struct ParseState *Parser ) {
    ( void ) Parser;
}
void DebugInit( Picoc *pc ) {
    ( void ) pc;
}
void DebugCleanup( Picoc *pc ) {
    ( void ) pc;
}
#endif
