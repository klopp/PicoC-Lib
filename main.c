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
    PicoCLibInit( &pc );
    PicoCLibBindInt( &pc, "aaa", &aaa );
    PicoCLibBindCharArray( &pc, "bbb", bbb );
    PicoCLibBindArray( &pc, "ccc", &a );
    PicoCLibBindArray( &pc, "ddd", &b );
    PicoCLibLoadFiles( &pc, "./t/hello-bye.picoc", NULL );
    /*
     * First main() call
     */
    rc = PicoCLibMainFromFile( &pc, "./t/main.picoc" );
    printf( "rc = %d, exit value: %d, error:\n%s\n", rc, pc.pc.PicocExitValue,
            pc.PicocOutBuf );
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
    printf( "rc = %d, exit value: %d, error:\n%s\n", rc, pc.pc.PicocExitValue,
            pc.PicocOutBuf );
    PicoCLibDown( &pc );
    return 0;
}

/*
 *  That's All, Folks!
 */

