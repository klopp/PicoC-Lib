/*
 *  Created on: 19 июня 2016 г.
 *      Author: Vsevolod Lutovinov <klopp@yandex.ru>
 */
#include "picoc-lib.h"

/* -----------------------------------------------------------------------------
 *
 -----------------------------------------------------------------------------*/
#define MAIN_EXIT_CODE      "__main_ret"
#define INT_MAIN_VOID       MAIN_EXIT_CODE "=main();"

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

static void _PicoCLibClearVar( PicoCLib *pc, const char *var ) {
    if( VariableDefined( &pc->pc, TableStrRegister( &pc->pc, var ) ) ) {
        TableDelete( &pc->pc, &pc->pc.GlobalTable,
                     TableStrRegister( &pc->pc, var ) );
    }
}

void PicoCLibClearMainVars( PicoCLib *pc ) {
    _PicoCLibClearVar( pc, "main" );
    _PicoCLibClearVar( pc, MAIN_EXIT_CODE );
}

PicoCLib *PicoCLibInit( PicoCLib *pc ) {
    PicocInitialise( &pc->pc, PICOC_STACK_SIZE );
    memset( pc->PicocOutBuf, 0, PICOC_OUTBUF_SIZE );
    memset( pc->ArrayPointers, 0, PICOC_ARRAY_POINTERS_MAX * sizeof( void * ) );
    pc->pc.CStdOut = fopen( PICOC_DEV_NULL, "w" );
    setvbuf( pc->pc.CStdOut, pc->PicocOutBuf, _IOFBF, PICOC_OUTBUF_SIZE );
#ifndef NO_DEBUGGER
    pc->InitDebug = 1;
#endif
    return pc;
}

void PicoCLibDown( PicoCLib *pc ) {
    if( pc->pc.CStdOut ) {
        size_t i;

        for( i = 0; i < PICOC_ARRAY_POINTERS_MAX; i++ ) {
            if( pc->ArrayPointers[i].name ) {
                HeapFreeMem( &pc->pc, pc->ArrayPointers[i].name );
            }
        }

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
                FALSE, pc->InitDebug );
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
                    TRUE, pc->InitDebug );
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
int PicoCLibUnbindArray( PicoCLib *pc, void *val ) {
    size_t i;

    for( i = 0; i < PICOC_ARRAY_POINTERS_MAX; i++ ) {
        if( pc->ArrayPointers[i].val == val ) {
            _PicoCLibClearVar( pc, pc->ArrayPointers[i].name );
            HeapFreeMem( &pc->pc, pc->ArrayPointers[i].name );
            pc->ArrayPointers[i].name = NULL;
            pc->ArrayPointers[i].val = NULL;
            return 0;
        }
    }

    fflush( pc->pc.CStdOut );
    fprintf( pc->pc.CStdOut, "PicoCLibUnbindArray(): array is not binded" );
    return -1;
}

int PicoCLibBindArray( PicoCLib *pc, const char *name, void *val ) {
    size_t i;
    fflush( pc->pc.CStdOut );

    for( i = 0; i < PICOC_ARRAY_POINTERS_MAX; i++ ) {
        if( !pc->ArrayPointers[i].val ) {
            pc->ArrayPointers[i].name = HeapAllocMem( &pc->pc, strlen( name ) + 1 );

            if( !pc->ArrayPointers[i].name ) {
                fprintf( pc->pc.CStdOut,
                         "PicoCLibBindArray(): %u array pointers already exists",
                         PICOC_ARRAY_POINTERS_MAX );
                return -2;
            }

            pc->ArrayPointers[i].val = val;
            strcpy( pc->ArrayPointers[i].name, name );
            return _PicoCLibBind( pc, name, &pc->ArrayPointers[i].val,
                                  pc->pc.VoidPtrType );
        }
    }

    fprintf( pc->pc.CStdOut,
             "PicoCLibBindArray(): %u array pointers already exists",
             PICOC_ARRAY_POINTERS_MAX );
    return -1;
}

int PicoCLibBindCharArray( PicoCLib *pc, const char *name, char *val ) {
    return _PicoCLibBind( pc, name, val, pc->pc.CharArrayType );
}

/* -----------------------------------------------------------------------------
 *
 -----------------------------------------------------------------------------*/
struct Value *_PicoCLibGetFunction( PicoCLib *pc, const char *name ) {
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

    return LVal;
}

/* -----------------------------------------------------------------------------
 *
 -----------------------------------------------------------------------------*/
static const char *_PicoCLibGetTypeData( Picoc *pc, enum BaseType type,
        struct ValueType **retptr ) {
    *retptr = NULL;

    switch( type ) {
        case TypeVoid:
            return "void";

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
static void _PicoCLibClearFunctionVars( PicoCLibFunc *function ) {
    unsigned i;
    char arg[sizeof( PICOC_FUNCTION_ARG ) + 8];

    for( i = function->argmax; i > function->argmin; --i ) {
        sprintf( arg, PICOC_FUNCTION_ARG, i );
        _PicoCLibClearVar( function->pc, arg );
    }

    sprintf( arg, PICOC_FUNCTION_RET, function->argmin - 1 );
    _PicoCLibClearVar( function->pc, PICOC_FUNCTION_RET );
}

/* -----------------------------------------------------------------------------
 *
 -----------------------------------------------------------------------------*/
PicoCLibFunc *PicoCLibFunction( PicoCLib *pc, enum BaseType ret,
                                const char *name,
                                const char *fmt ) {
    const char *s;
    PicoCLibFunc *function;
    int idx = 0;
    fflush( pc->pc.CStdOut );

    if( strlen( name ) > PICOC_FUNCNAME_MAX ) {
        fprintf( pc->pc.CStdOut, "function name is too long!" );
        return NULL;
    }

    function = HeapAllocMem( &pc->pc, sizeof( PicoCLibFunc ) );

    if( function == NULL ) {
        fprintf( pc->pc.CStdOut, "not enough memory!" );
        return NULL;
    }

    memset( function, 0, sizeof( PicoCLibFunc ) );
    s = _PicoCLibGetTypeData( &pc->pc, ret, &function->retptr );

    if( !s ) {
        fprintf( pc->pc.CStdOut, "invalid return type: %d!", ret );
        HeapFreeMem( &pc->pc, function );
        return NULL;
    }

    function->pc = pc;

    if( function->retptr ) {
        sprintf( function->callstr, PICOC_FUNCTION_RET, pc->curid++ );
    }

    strcat( function->callstr, name );
    strcat( function->callstr, "(" );
    sprintf( function->name, "%s()", name );
    function->argmax = function->argmin = pc->curid;

    while( fmt && *fmt ) {
        if( strchr( PICOC_ARG_SEPARATORS, *fmt ) ) {
            fmt++;
            continue;
        }

        sprintf( function->args[idx].name, PICOC_FUNCTION_ARG, pc->curid );
        function->fmt[idx] = *fmt;
        strcat( function->callstr, function->args[idx].name );
        strcat( function->callstr, "," );
        ++fmt;
        ++idx;

        if( idx > PICOC_MAX_ARGS ) {
            fprintf( pc->pc.CStdOut, "too many arguments (%u max)!",
                     PICOC_MAX_ARGS );
            HeapFreeMem( &pc->pc, function );
            return NULL;
        }

        ++pc->curid;
        ++function->argmax;
    }

    if( idx ) {
        function->callstr[strlen( function->callstr ) - 1] = 0;
    }

    strcat( function->callstr, ");" );
    return function;
}
/* -----------------------------------------------------------------------------
 *
 -----------------------------------------------------------------------------*/
int PicoCLibCall( PicoCLibFunc *function, ... ) {
    va_list ap;
    int idx = 0;
    char *fmt = function->fmt;
    Picoc *pc = &function->pc->pc;
    fflush( pc->CStdOut );
    va_start( ap, function );

    while( fmt && *fmt ) {
        switch( *fmt ) {
            case 'c':
                function->args[idx].arg.Character = ( char ) va_arg( ap, int );
                VariableDefinePlatformVar( pc, NULL, function->args[idx].name, &pc->CharType,
                                           &function->args[idx].arg,
                                           TRUE );
                break;

                /*
                            case 'C':
                                args[idx].UnsignedCharacter = (unsigned char) va_arg(ap,
                                        unsigned int);
                                VariableDefinePlatformVar(&pc->pc, NULL, arg,
                                        &pc->pc.UnsignedCharType, &args[idx],
                                        TRUE);
                                break;

                            case 's':
                                args[idx].ShortInteger = (short) va_arg(ap, int);
                                VariableDefinePlatformVar(&pc->pc, NULL, arg, &pc->pc.ShortType,
                                        &args[idx],
                                        TRUE);
                                break;

                            case 'S':
                                args[idx].UnsignedShortInteger = (unsigned short) va_arg(ap,
                                        int);
                                VariableDefinePlatformVar(&pc->pc, NULL, arg,
                                        &pc->pc.UnsignedShortType, &args[idx],
                                        TRUE);
                                break;
                */
            case 'i':
                function->args[idx].arg.Integer = va_arg( ap, int );
                VariableDefinePlatformVar( pc, NULL, function->args[idx].name, &pc->IntType,
                                           &function->args[idx].arg,
                                           TRUE );
                break;

                /*
                            case 'I':
                                args[idx].UnsignedInteger = va_arg(ap, unsigned int);
                                VariableDefinePlatformVar(&pc->pc, NULL, arg,
                                        &pc->pc.UnsignedIntType, &args[idx],
                                        TRUE);
                                break;

                            case 'l':
                                args[idx].LongInteger = va_arg(ap, long);
                                VariableDefinePlatformVar(&pc->pc, NULL, arg, &pc->pc.LongType,
                                        &args[idx],
                                        TRUE);
                                break;

                            case 'L':
                                args[idx].UnsignedLongInteger = va_arg(ap, unsigned long);
                                VariableDefinePlatformVar(&pc->pc, NULL, arg,
                                        &pc->pc.UnsignedLongType, &args[idx],
                                        TRUE);
                                break;
                */
            case 'p':
                function->args[idx].arg.Pointer = va_arg( ap, void * );
                VariableDefinePlatformVar( pc, NULL, function->args[idx].name,
                                           pc->VoidPtrType, &function->args[idx].arg,
                                           TRUE );
                break;

            case 'z':
                function->args[idx].arg.Pointer = va_arg( ap, char * );
                VariableDefinePlatformVar( pc, NULL, function->args[idx].name,
                                           pc->CharArrayType, function->args[idx].arg.Pointer,
                                           TRUE );
                break;

            default:
                va_end( ap );
                fprintf( pc->CStdOut, "invalid argument type: \"%c\"!", *fmt );
                _PicoCLibClearFunctionVars( function );
                return -1;
        }

        fmt++;
        idx++;
    }

    if( function->retptr ) {
        char ret[sizeof( PICOC_FUNCTION_RET ) + 8];
        sprintf( ret, PICOC_FUNCTION_RET, function->argmin - 1 );
        ret[strlen( ret ) - 1] = 0;
        VariableDefinePlatformVar( pc, NULL, ret, function->retptr,
                                   &function->rc,
                                   TRUE );
    }

    PicocParse( pc, function->name, function->callstr, strlen( function->callstr ),
                TRUE,
                TRUE,
                FALSE, function->pc->InitDebug );
    _PicoCLibClearFunctionVars( function );
    va_end( ap );
    return 0;
}
/* -----------------------------------------------------------------------------
 *
 -----------------------------------------------------------------------------*/
/*
union AnyValue PicoCLibCallFunction(PicoCLib *pc, enum BaseType ret,
        const char *name, const char *fmt, ...) {
    char call[PICOC_CALLSTR_SIZE];
    union AnyValue rc = { 0 };
    struct ValueType *retptr;
    int idx = 0;
    char arg[sizeof( PICOC_FUNCTION_ARG) + 8];
    union AnyValue args[PICOC_MAX_ARGS];
    va_list ap;
    const char *s;

    fflush(pc->pc.CStdOut);
    pc->pc.PicocExitValue = 0;

    if (strlen(name) > PICOC_FUNCNAME_MAX) {
        fprintf(pc->pc.CStdOut, "function name is too long!");
        pc->pc.PicocExitValue = -4;
        return rc;
    }

    s = _PicoCLibGetTypeData(&pc->pc, ret, &retptr);

    if (!s) {
        fprintf(pc->pc.CStdOut, "invalid return type: %d!", ret);
        pc->pc.PicocExitValue = -1;
        return rc;
    }

    memset(args, 0, sizeof(args));
    *call = 0;

    if (retptr) {
        strcpy(call, PICOC_FUNCTION_RET "=");
    }

    strcat(call, name);
    strcat(call, "(");
    fflush(pc->pc.CStdOut);
    va_start(ap, fmt);

    while (fmt && *fmt) {
        sprintf(arg, PICOC_FUNCTION_ARG, idx);

        if (strchr( PICOC_ARG_SEPARATORS, *fmt)) {
            fmt++;
            continue;
        }

        switch (*fmt) {
            case 'c':
                args[idx].Character = (char) va_arg(ap, int);
                VariableDefinePlatformVar(&pc->pc, NULL, arg, &pc->pc.CharType,
                        &args[idx],
                        TRUE);
                break;

            case 'C':
                args[idx].UnsignedCharacter = (unsigned char) va_arg(ap,
                        unsigned int);
                VariableDefinePlatformVar(&pc->pc, NULL, arg,
                        &pc->pc.UnsignedCharType, &args[idx],
                        TRUE);
                break;

            case 's':
                args[idx].ShortInteger = (short) va_arg(ap, int);
                VariableDefinePlatformVar(&pc->pc, NULL, arg, &pc->pc.ShortType,
                        &args[idx],
                        TRUE);
                break;

            case 'S':
                args[idx].UnsignedShortInteger = (unsigned short) va_arg(ap,
                        int);
                VariableDefinePlatformVar(&pc->pc, NULL, arg,
                        &pc->pc.UnsignedShortType, &args[idx],
                        TRUE);
                break;

            case 'i':
                args[idx].Integer = va_arg(ap, int);
                VariableDefinePlatformVar(&pc->pc, NULL, arg, &pc->pc.IntType,
                        &args[idx],
                        TRUE);
                break;

            case 'I':
                args[idx].UnsignedInteger = va_arg(ap, unsigned int);
                VariableDefinePlatformVar(&pc->pc, NULL, arg,
                        &pc->pc.UnsignedIntType, &args[idx],
                        TRUE);
                break;

            case 'l':
                args[idx].LongInteger = va_arg(ap, long);
                VariableDefinePlatformVar(&pc->pc, NULL, arg, &pc->pc.LongType,
                        &args[idx],
                        TRUE);
                break;

            case 'L':
                args[idx].UnsignedLongInteger = va_arg(ap, unsigned long);
                VariableDefinePlatformVar(&pc->pc, NULL, arg,
                        &pc->pc.UnsignedLongType, &args[idx],
                        TRUE);
                break;

            case 'p':
            case 'P':
                args[idx].Pointer = va_arg(ap, void *);
                VariableDefinePlatformVar(&pc->pc, NULL, arg,
                        pc->pc.VoidPtrType, &args[idx],
                        TRUE);
                break;

            case 'z':
            case 'Z':
                args[idx].Pointer = va_arg(ap, char *);
                VariableDefinePlatformVar(&pc->pc, NULL, arg,
                        pc->pc.CharArrayType, args[idx].Pointer,
                        TRUE);
                break;

            default:
                va_end(ap);
                fprintf(pc->pc.CStdOut, "invalid argument type: \"%c\"!", *fmt);
                _PicoCLibClearFunctionVars(pc);
                return rc;
        }

        strcat(call, arg);
        strcat(call, ",");
        fmt++;
        idx++;

        if (idx > PICOC_MAX_ARGS) {
            va_end(ap);
            fprintf(pc->pc.CStdOut, "too many arguments (%u max)!",
            PICOC_MAX_ARGS);
            _PicoCLibClearFunctionVars(pc);
            pc->pc.PicocExitValue = -3;
            return rc;
        }
    }

    va_end(ap);

    if (retptr) {
        VariableDefinePlatformVar(&pc->pc, NULL, PICOC_FUNCTION_RET, retptr,
                &rc,
                TRUE);
    }

    if (idx) {
        call[strlen(call) - 1] = 0;
    }

    strcat(call, ");");
    PicocParse(&pc->pc, name, call, strlen(call), TRUE,
    TRUE,
    FALSE, pc->InitDebug);
    _PicoCLibClearFunctionVars(pc);
    return rc;
}
*/

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
