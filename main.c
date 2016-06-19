/*
 *  Created on: 19 июня 2016 г.
 *      Author: Vsevolod Lutovinov <klopp@yandex.ru>
 */

#include "picoc-lib.h"

int main() {
    PicoCLib pc;
    int rc;
    int aaa = 9;
    char bbb[16] = { 'b', 'B', 0 };
    struct {
        int a;
        int b;
    } ab = { 33, 44 };
    struct {
        int a;
        int b;
    } ad = { 55, 66 };
    PicoCLibInit( &pc );
    PicoCLibBindInt( &pc, "aaa", &aaa );
    PicoCLibBindCharArray( &pc, "bbb", bbb );
    PicoCLibBindArray( &pc, "ccc", &ab );
    PicoCLibBindArray( &pc, "ddd", &ad );
    rc = PicoCLibMain( &pc, "./t/main.picoc" );
    printf( "rc = %d, exit value: %d, error:\n%s\n", rc, pc.pc.PicocExitValue,
            pc.PicocOutBuf );
    /*
        PicoCLibReset( &pc );
        rc = PicoCLibMain( &pc, "./t/main.picoc" );
        printf( "rc = %d, exit value: %d, error:\n%s\n", rc, pc.pc.PicocExitValue,
                pc.PicocOutBuf );
    */
    PicoCLibDown( &pc );
    return 0;
}

/*
 *  That's All, Folks!
 */

