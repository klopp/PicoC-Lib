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
#ifndef NO_DEBUGGER
    pc->InitDebug = 1;
#endif
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
static int _PicoCLibCallMain( PicoCLib *pc ) {
    struct Value *FuncValue = NULL;
    if( PicocPlatformSetExitPoint( &pc->pc ) ) {
        return -1;
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
                FALSE,
                pc->InitDebug );
    return 0;
}

/* -----------------------------------------------------------------------------
 *
 -----------------------------------------------------------------------------*/
int PicoCLibMainFromFile( PicoCLib *pc, const char *file ) {
    return PicoCLibLoadFiles( pc, file, NULL ) ? -1 : _PicoCLibCallMain( pc );
}

int PicoCLibMainFromSource( PicoCLib *pc, const char *source ) {
    return PicoCLibLoadSources( pc, source, NULL ) ? -1 : _PicoCLibCallMain( pc );
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
    fflush( pc->pc.CStdOut );
    if( PicocPlatformSetExitPoint( &pc->pc ) ) {
        va_end( ap );
        return -1;
    }
    while( current ) {
        sprintf( file, "[source/%04u]", idx );
        PicocParse( &pc->pc, file, current, strlen( current ), TRUE, FALSE, TRUE,
                    pc->InitDebug );
        current = va_arg( ap, char * );
    }
    va_end( ap );
    return 0;
}

int PicoCLibLoadFiles( PicoCLib *pc, const char *file, ... ) {
    va_list ap;
    const char *current = file;
    va_start( ap, file );
    fflush( pc->pc.CStdOut );
    if( PicocPlatformSetExitPoint( &pc->pc ) ) {
        va_end( ap );
        return -1;
    }
    while( current ) {
        char *SourceStr = PlatformReadFile( &pc->pc, current );
        /* ignore "#!/path/to/picoc" .. by replacing the "#!" with "//" */
        if( SourceStr && SourceStr[0] == '#' && SourceStr[1] == '!' ) {
            SourceStr[0] = '/';
            SourceStr[1] = '/';
        }
        PicocParse( &pc->pc, current, SourceStr, strlen( SourceStr ), TRUE, FALSE,
                    TRUE,
                    pc->InitDebug );
        current = va_arg( ap, char * );
    }
    va_end( ap );
    return 0;
}

/* -----------------------------------------------------------------------------
 *
 -----------------------------------------------------------------------------*/
static int _PicoCLibBind( PicoCLib *pc, const char *name, void *val,
                          struct ValueType *type ) {
    fflush( pc->pc.CStdOut );
    if( PicocPlatformSetExitPoint( &pc->pc ) ) {
        return -1;
    }
    VariableDefinePlatformVar( &pc->pc, NULL, ( char * ) name, type,
                               ( union AnyValue * ) val, 1 );
    return 0;
}

/* -----------------------------------------------------------------------------
 *
 -----------------------------------------------------------------------------*/
int PicoCLibBindShort( PicoCLib *pc, const char *name, short *val ) {
    return _PicoCLibBind( pc, name, val, &pc->pc.ShortType );
}
int PicoCLibBindUShort( PicoCLib *pc, const char *name, unsigned short *val ) {
    return _PicoCLibBind( pc, name, val, &pc->pc.UnsignedShortType );
}
int PicoCLibBindInt( PicoCLib *pc, const char *name, int *val ) {
    return _PicoCLibBind( pc, name, val, &pc->pc.IntType );
}
int PicoCLibBindUInt( PicoCLib *pc, const char *name, unsigned int *val ) {
    return _PicoCLibBind( pc, name, val, &pc->pc.UnsignedIntType );
}
int PicoCLibBindLong( PicoCLib *pc, const char *name, long *val ) {
    return _PicoCLibBind( pc, name, val, &pc->pc.LongType );
}
int PicoCLibBindULong( PicoCLib *pc, const char *name, unsigned long *val ) {
    return _PicoCLibBind( pc, name, val, &pc->pc.UnsignedLongType );
}

/* -----------------------------------------------------------------------------
 *
 -----------------------------------------------------------------------------*/
int PicoCLibBindArray( PicoCLib *pc, const char *name, void *val ) {
    if( pc->nArrayPointers >= PICOC_ARRAY_POINTERS_MAX ) {
        fprintf( pc->pc.CStdOut,
                 "PicoCLibBindArray(): %u array pointers already exists",
                 PICOC_ARRAY_POINTERS_MAX );
        return -1;
    }
    pc->ArrayPointers[pc->nArrayPointers] = val;
    pc->nArrayPointers++;
    return _PicoCLibBind( pc, name, &pc->ArrayPointers[pc->nArrayPointers - 1],
                          pc->pc.VoidPtrType );
}
int PicoCLibBindCharArray( PicoCLib *pc, const char *name, char *val ) {
    return _PicoCLibBind( pc, name, val, pc->pc.CharArrayType );
}

/* -----------------------------------------------------------------------------
 *
 -----------------------------------------------------------------------------*/
static void *_PicoCLibGetFunction( PicoCLib *pc, const char *name ) {
    struct Value *LVal;
    if( PicocPlatformSetExitPoint( &pc->pc ) ) {
        return NULL;
    }
    fflush( pc->pc.CStdOut );
    if( !VariableDefined( &pc->pc, TableStrRegister( &pc->pc, name ) ) ) {
        fprintf( pc->pc.CStdOut, "\"%s\" is not defined!", name );
        return NULL;
    }
    VariableGet( &pc->pc, NULL, TableStrRegister( &pc->pc, name ), &LVal );
    if( LVal->Typ->Base != TypeFunction ) {
        fprintf( pc->pc.CStdOut, "\"%s\" is not a function!", name );
        return NULL;
    }
    return LVal->Val->Pointer;
}

/* -----------------------------------------------------------------------------
 *
 -----------------------------------------------------------------------------*/
static const char *_PicoCLibGetTypeStr( Picoc *pc, enum BaseType type,
                                        struct ValueType **retptr ) {
    switch( type ) {
        case TypeChar:
                *retptr = &pc->CharType;
            return "char";
        case TypeShort:
            *retptr = &pc->ShortType;
            return "short";
        case TypeInt:
            *retptr = &pc->IntType;
            return "int";
        case TypeLong:
            *retptr = &pc->LongType;
            return "long";
        case TypeUnsignedChar:
            *retptr = &pc->UnsignedCharType;
            return "unsigned char";
        case TypeUnsignedShort:
            *retptr = &pc->UnsignedShortType;
            return "unsigned short";
        case TypeUnsignedInt:
            *retptr = &pc->UnsignedIntType;
            return "unsigned int";
        case TypeUnsignedLong:
            *retptr = &pc->UnsignedLongType;
            return "unsigned long";
        case TypePointer:
            *retptr = pc->VoidPtrType;
            return "void *";
        default:
            break;
    }
    return NULL;
}

/* -----------------------------------------------------------------------------
 *
 -----------------------------------------------------------------------------*/
static void _PicoCLibCleanFunctionVars( PicoCLib *pc ) {
    int i;
    for( i = 1; i < PICOC_MAX_ARGS; i++ ) {
        char arg[0x10];
        sprintf( arg, "__arg__%u", i );
        if( VariableDefined( &pc->pc, TableStrRegister( &pc->pc, arg ) ) ) {
            TableDelete( &pc->pc, &pc->pc.GlobalTable,
                         TableStrRegister( &pc->pc, arg ) );
        }
    }
    TableDelete( &pc->pc, &pc->pc.GlobalTable,
                 TableStrRegister( &pc->pc, "__ret__" ) );
}

/* -----------------------------------------------------------------------------
 *
 -----------------------------------------------------------------------------*/
union AnyValue PicoCLibCallFunction( PicoCLib *pc, enum BaseType ret,
                                             const char *name, const char *fmt, ... ) {
    void *ptr;
    va_list ap;
    char call[PICOC_CALLSTR_SIZE];
    union AnyValue rc = { 0 };
    struct ValueType *retptr;
    const char *s = _PicoCLibGetTypeStr( &pc->pc, ret, &retptr );
    int idx = 1;
    char arg[0x10];
    union AnyValue args[PICOC_MAX_ARGS];
    fflush( pc->pc.CStdOut );
    if( !s ) {
        fprintf( pc->pc.CStdOut, "invalid return type: \"%d\"!", ret );
        return rc;
    }
    ptr = _PicoCLibGetFunction( pc, name );
    if( !ptr ) {
        return rc;
    }
    memset( args, 0, sizeof( args ) );
    sprintf( call, "__ret__ = %s(", name );
    fflush( pc->pc.CStdOut );
    va_start( ap, fmt );
    while( *fmt ) {
        sprintf( arg, "__arg__%u", idx );
        strcat( call, arg );
        strcat( call, "," );
        switch( *fmt ) {
            case 'c':
                args[idx].Character = va_arg( ap, char );
                VariableDefinePlatformVar( &pc->pc, NULL, arg, &pc->pc.CharType,
                                           &args[idx],
                                           TRUE );
                break;
            case 'C':
                args[idx].UnsignedCharacter = va_arg( ap, unsigned char );
                VariableDefinePlatformVar( &pc->pc, NULL, arg,
                                           &pc->pc.UnsignedCharType, &args[idx],
                                           TRUE );
                break;
            case 's':
                args[idx].ShortInteger = va_arg( ap, short );
                VariableDefinePlatformVar( &pc->pc, NULL, arg, &pc->pc.ShortType,
                                           &args[idx],
                                           TRUE );
                break;
            case 'S':
                args[idx].UnsignedShortInteger = va_arg( ap, unsigned short );
                VariableDefinePlatformVar( &pc->pc, NULL, arg,
                                           &pc->pc.UnsignedShortType, &args[idx],
                                           TRUE );
                break;
            case 'i':
                args[idx].Integer = va_arg( ap, int );
                VariableDefinePlatformVar( &pc->pc, NULL, arg, &pc->pc.IntType,
                                           &args[idx],
                                           TRUE );
                break;
            case 'I':
                args[idx].UnsignedInteger = va_arg( ap, unsigned int );
                VariableDefinePlatformVar( &pc->pc, NULL, arg,
                                           &pc->pc.UnsignedIntType, &args[idx],
                                           TRUE );
                break;
            case 'l':
                args[idx].LongInteger = va_arg( ap, long );
                VariableDefinePlatformVar( &pc->pc, NULL, arg, &pc->pc.LongType,
                                           &args[idx],
                                           TRUE );
                break;
            case 'L':
                args[idx].UnsignedLongInteger = va_arg( ap, unsigned long );
                VariableDefinePlatformVar( &pc->pc, NULL, arg,
                                           &pc->pc.UnsignedLongType, &args[idx],
                                           TRUE );
                break;
            case 'p':
                args[idx].Pointer = va_arg( ap, void * );
                VariableDefinePlatformVar( &pc->pc, NULL, arg,
                                           pc->pc.VoidPtrType, &args[idx],
                                           TRUE );
                break;
            case 'z':
                args[idx].Pointer = va_arg( ap, char * );
                VariableDefinePlatformVar( &pc->pc, NULL, arg,
                                           pc->pc.CharArrayType, args[idx].Pointer,
                                           TRUE );
                break;
            default:
                va_end( ap );
                fprintf( pc->pc.CStdOut, "invalid argument type: \"%c\"!", *fmt );
                _PicoCLibCleanFunctionVars( pc );
                return rc;
                break;
        }
        fmt++;
        idx++;
        if( idx >= PICOC_MAX_ARGS ) {
            va_end( ap );
            fprintf( pc->pc.CStdOut, "too many arguments, %u max!",
                     PICOC_MAX_ARGS );
            _PicoCLibCleanFunctionVars( pc );
            return rc;
        }
    }
    va_end( ap );
    VariableDefinePlatformVar( &pc->pc, NULL, "__ret__", retptr, &rc,
                               TRUE );
    call[strlen( call ) - 1] = 0;
    strcat( call, ");" );
    printf( "%s\n", call );
    PicocParse( &pc->pc, name, call, strlen( call ), TRUE,
                TRUE,
                FALSE,
                pc->InitDebug );
    _PicoCLibCleanFunctionVars( pc );
    return rc;
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

/* -----------------------------------------------------------------------------
 *
 -----------------------------------------------------------------------------*/
