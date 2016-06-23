# PicoC-Lib

[PicoC library](https://github.com/zsaleeba/picoc) wrapper.

## Usage

```c

    #include "picoc-lib.h"
    #include "./t/main.picoc.h"

    int a = 0;
    char b[] = "DEADBEEF";
    const char lib1[] = "./t/lib1.picoc";
    const char lib2[] = "./t/lib2.picoc";
    const char mainh[] = "./t/main.picoc.h";
    const char main1[] = "./t/main1.picoc";
    const char main2[] = "./t/main2.picoc";    
    struct ab {
        int a; int b;
    } c = { 33, 44 };
    int rc;
    PicoCLibFunc *f;
    
    PicoCLib pc;
    PicoCLibInit( &pc );
   
    PicoCLibBindInt( &pc, "a", &a );
    PicoCLibBindCharArray( &pc, "b", b );
    PicoCLibBindPointer( &pc, "c", &c );

    rc = PicoCLibLoadFiles( &pc, lib1, lib2, NULL );
    if( rc ) {
        printf( "rc = %d, exit value: %d, error:\n%s\n", rc,
                pc.pc.PicocExitValue, pc.PicocOutBuf );
    }

    printf( "Before: a = %d, b = %s, c = {%d, %d}\n", a, b, c.a, c.b );  
    rc = PicoCLibMainFromFile( &pc, main1 );
    if( rc ) {
        printf( "rc = %d, exit value: %d, error:\n%s\n", rc,
                pc.pc.PicocExitValue, pc.PicocOutBuf );
    }
    printf( "After: a = %d, b = %s, c = {%d, %d}\n", a, b, c.a, c.b );

    /*
     * Clean include names:
     */
    PicoCLibClearFileVars( &pc, mainh );
    /*
     * Clean possible global names from main1.picoc file:
     */
    PicoCLibClearFileVars( &pc, main1 );
    /*
     * Clean global names from main():
     */
    PicoCLibClearMainVars( &pc );
    
    printf( "Before: a = %d, b = %s, c = {%d, %d}\n", a, b, c.a, c.b );  
    rc = PicoCLibMainFromFile( &pc, main2 );
    if( rc ) {
        printf( "rc = %d, exit value: %d, error:\n%s\n", rc,
                pc.pc.PicocExitValue, pc.PicocOutBuf );
    }
    printf( "After: a = %d, b = %s, c = {%d, %d}\n", a, b, c.a, c.b );

    /*
     * External xx( int, char *, void * ):
     */
    f = PicoCLibFunction( &pc, TypeInt, "xx", "i:z:p" );
    if( !f ) {
        printf( "exit value: %d, error:\n%s\n", 
                pc.pc.PicocExitValue, pc.PicocOutBuf );
    }
    rc = PicoCLibCall( f, 1, "xyz", &a );
    if( !rc ) {
        printf( "rc = %d, exit value: %d, error:\n%s\n", rc,  
                pc.pc.PicocExitValue, pc.PicocOutBuf );
    }
    else {
        printf( "f() return %d\n", f.rc.Integer );
    }

    HeapFreeMem( f );
    PicoCLibDown( &pc );
    exit( pc.pc.PicocExitValue );
   
```
