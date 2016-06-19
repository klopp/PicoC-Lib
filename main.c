/*
 *  Created on: 19 июня 2016 г.
 *      Author: Vsevolod Lutovinov <klopp@yandex.ru>
 */

#include "picoc-lib.h"
#include "./t/main.picoc.h"

void TableDeleteFromFile( Picoc *pc, struct Table *Tbl, const char *Key ) {
    struct TableEntry **EntryPtr;
    short i;
    for( i = 0; i < Tbl->Size; i++ ) {
        EntryPtr = &Tbl->HashTable[i];
        while( EntryPtr && *EntryPtr ) {
            if( ( *EntryPtr )->DeclFileName
                    && !strcmp( ( *EntryPtr )->DeclFileName, Key ) ) {
                struct TableEntry *DeleteEntry = *EntryPtr;
                *EntryPtr = DeleteEntry->Next;
                HeapFreeMem( pc, DeleteEntry );
            }
            else {
                EntryPtr = &( *EntryPtr )->Next;
            }
        }
    }
}

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
    rc = PicoCLibMainFromFiles( &pc, "./t/main.picoc", NULL );
    printf( "rc = %d, exit value: %d, error:\n%s\n", rc, pc.pc.PicocExitValue,
            pc.PicocOutBuf );
    printf( "aaa: %d, bbb: %s, [%d:%d], [%d:%d]\n", aaa, bbb, a.a, a.b, b.a,
            b.b );
    TableDeleteFromFile( &pc.pc, &pc.pc.GlobalTable, "./t/main.picoc.h" );
    TableDelete( &pc.pc, &pc.pc.GlobalTable, TableStrRegister( &pc.pc, "main" ) );
    TableDelete( &pc.pc, &pc.pc.GlobalTable, TableStrRegister( &pc.pc,
                 "__exit_value" ) );
    aaa = 6;
    rc = PicoCLibMainFromFiles( &pc, "./t/main.picoc", NULL );
    printf( "rc = %d, exit value: %d, error:\n%s\n", rc, pc.pc.PicocExitValue,
            pc.PicocOutBuf );
    PicoCLibDown( &pc );
    return 0;
}

/*
 *  That's All, Folks!
 */

