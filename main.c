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
    PicoCLibInit( &pc );
    PicoCLibBindInt( &pc, "aaa", &aaa );
    PicoCLibBindCharArray( &pc, "bbb", bbb );
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

