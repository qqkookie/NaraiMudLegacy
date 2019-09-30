/* **********************************************************************
 *	Subroutine library for handling "players" file.			*
 *	Used by purge.c, dupname.c					*
 ************************************************************************ */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <ctype.h>
#include <time.h>

#include "char.h"
#include "gamedb.h"

#define SECPERDAY 	86400

#define PLAYERF		"players"
#define NEWF		"players.new"

#define CHUCK ( sizeof( struct char_file_u ))

#define LOWER(c) (((c)>='A'  && (c) <= 'Z') ? ((c)+('a'-'A')) : (c))

#pragma GCC diagnostic ignored "-Wunused-result"

void open_files(int new); 
void show_PC( struct char_file_u *pst );
void delete_stash(char name[]);

FILE *FP, *FN;

void open_files(int new ) 
{
    if( !(FP=fopen( PLAYERF, "r") ) ) {
	fprintf(stderr, "Error opening players file.\n");
	exit(1);
    }
    if ( new )
	if( !(FN=fopen( NEWF, "w") ) ) {
	    fprintf(stderr, "Error opening output file.\n");
	    exit(2);
	}
}

void show_PC( struct char_file_u *pst ) 
{ 
    char *tp ;

    printf( "\nPLAYER: \"%s\"   \t(LEVEL %d, HIT %d, MANA %d )\n",
	pst->name, pst->level, pst->points.max_hit, pst->points.max_mana );
    tp = ctime( &(pst->last_logon));
    tp[16] = '\0' ;
    printf( "Last logon : %s :: (%ld) days ago.\n",
	tp, (time(0) - pst->last_logon)/SECPERDAY) ;
}

void delete_stash(char name[])
{
    char buf[100], *cp ;

    for( cp = name ; *cp  && isalpha(*cp); cp++ ) 
	*cp = LOWER(*cp); 
    sprintf( buf, "%s/%c/%s.%s", STASH, name[0], name, STASH_EXT );
    printf( "\nDeleting stash file: \"%s\"... ", buf );
    printf((( unlink(buf) == 0 ) ? "Ok.\n" : "Fail!\n")) ;
}

