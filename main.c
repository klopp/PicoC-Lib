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
    PicoCLibInit( &pc );
    PicoCLibBindInt( &pc, "aaa", &aaa );
    PicoCLibBindCharArray( &pc, "bbb", bbb );
    PicoCLibBindArray( &pc, "ccc", &a );
    PicoCLibBindArray( &pc, "ddd", &b );
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
    av = PicoCLibCallFunction( &pc, TypeInt, "xx", "izp", 1, "zzz", &a );

    if( pc.pc.PicocExitValue ) {
        printf( "a) exit value: %d, error:\n%s\n", pc.pc.PicocExitValue,
                pc.PicocOutBuf );
        PicoCLibDown( &pc );
        return pc.pc.PicocExitValue;
    }

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

