/*
 *  Created on: 19 июня 2016 г.
 *      Author: Vsevolod Lutovinov <klopp@yandex.ru>
 */

#include "picoc-lib.h"
#include "./t/main.picoc.h"

int main() {
    PicoCLib pc;
    int rc;
    int aaa = 9;
    char bbb[16] = { 'b', 'B', 0 };
    ab a = { 33, 44 };
    ab b = { 55, 66 };
    //int ( *xx )( int ) = NULL;
    union AnyValue av;
    PicoCLibFunc *vf;
    PicoCLibInit( &pc );
    PicoCLibBindInt( &pc, "aaa", &aaa );
    PicoCLibBindCharArray( &pc, "bbb", bbb );
    PicoCLibBindPointer( &pc, "ccc", &a );
    PicoCLibBindPointer( &pc, "ddd", &b );
    rc = PicoCLibLoadFiles( &pc, "./t/hello-bye.picoc", NULL );

    if( rc ) {
        printf( "1) rc = %d, exit value: %d, error:\n%s\n", rc,
                pc.pc.PicocExitValue, pc.PicocOutBuf );
        PicoCLibDown( &pc );
        return pc.pc.PicocExitValue;
    }

    /*
     * External function call
     */
    /*
     * typedef struct _PicoCLibFunc
     * {
     *      PicoCLib *pc;
     *      char callstr[PICOC_CALLSTR_SIZE];
     *      const char fmt[PICOC_MAX_ARGS+1];
     *      union AnyValue rc;
     * } PicoCLibFunc;
     *
     * xx = PicoCLibFunction( &pc, "xx", "i,z,p" );
     * rc = PicoCLibCall( xx, 1, "xyz", &a );
     *
     */
    vf = PicoCLibFunction( &pc, TypeInt, "xx", "i:z:p" );
    rc = PicoCLibCall( vf, 1, "xyz", &a );

    //    av = PicoCLibCallFunction( &pc, TypeInt, "xx", "i,z p", 1, "xyz", &a );

    if( rc ) {
        printf( "a) exit value: %d, error:\n%s\n", rc,
                pc.PicocOutBuf );
        PicoCLibDown( &pc );
        return pc.pc.PicocExitValue;
    }
    else {
        printf( "OK, 'int xx(...)' return %d\n", av.Integer );
    }

    /*
        PicoCLibCallFunction( &pc, TypeVoid, "voidfunc", NULL );
        if( pc.pc.PicocExitValue ) {
            printf( "b) exit value: %d, error:\n%s\n", pc.pc.PicocExitValue,
                    pc.PicocOutBuf );
            PicoCLibDown( &pc );
            return pc.pc.PicocExitValue;
        }
    */
    /*
     * First main() call
     */
    rc = PicoCLibMainFromFile( &pc, "./t/main.picoc" );

    if( rc ) {
        printf( "2) rc = %d, exit value: %d, error:\n%s\n", rc,
                pc.pc.PicocExitValue, pc.PicocOutBuf );
        PicoCLibDown( &pc );
        return pc.pc.PicocExitValue;
    }

    printf( "aaa: %d, bbb: %s, [%d:%d], [%d:%d]\n", aaa, bbb, a.a, a.b, b.a,
            b.b );
    /*
     * Cleanup:
     */
    PicoCLibClearFileVars( &pc, "./t/main.picoc.h" );
    PicoCLibClearMainVars( &pc );
    aaa = 6;
    /*
     * Second main() call
     */
    //PicoCLibUnbindArray( &pc, &a );
    rc = PicoCLibMainFromFile( &pc, "./t/main.picoc" );

    if( rc ) {
        printf( "3) rc = %d, exit value: %d, error:\n%s\n", rc,
                pc.pc.PicocExitValue, pc.PicocOutBuf );
    }

    PicoCLibDown( &pc );
    return 0;
}

/*
 *  That's All, Folks!
 */

