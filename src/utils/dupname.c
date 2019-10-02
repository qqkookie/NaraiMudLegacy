/* ***********************************************************
    Purge players with duplicated entries in "players" file.
    Result is saved as file named "players.new" 
    dupname will NOT delete stash file.
    This program should not have needed or used.
   WARNING!!! DON'T run this program while MUD is RUNNING.!!!
   *********************************************************** */

#include <stdio.h>
#include <string.h>

#include "player.c"

#define MAX_PC 1000

int main()
{
    char buf[100], **pl;
    int all, killed, duped ;
    struct char_file_u  st;
    char *namelist[MAX_PC] ; 

    printf( "\nWARNING!!! DON'T run this program while MUD is RUNNING.!!!\n");
    printf( "\nPURGE all duplicated names in player file.\n");

    open_files(1);

    all = killed = 0 ;
    *namelist = NULL ; 
    while( fread( &st , CHUCK , 1 , FP ) > 0 ) { 
	all++;
	duped = 0;
	for( pl = namelist; *pl && (pl-namelist) < MAX_PC ; pl++ )
	     if( strcasecmp(*pl, st.name) == 0) duped++ ;

	if( !duped ) { 
	    *pl = strdup(st.name);
	    *++pl = NULL ;
	}
	else {
	    show_PC( &st) ;
	    printf( "\nReally delete [%s] ? (Y/N) >> ", st.name );
	    fgets(buf, 100, stdin);

	    if ( LOWER(buf[0]) == 'y' ) {
		killed++;
		continue;
	    }
	} 
	if ( fwrite( &st , CHUCK , 1 , FN ) <= 0 ) {
	    fprintf(stderr, "Error writing output file.\n");
	    exit(3); 
	}
    }
    fclose(FP);
    fclose(FN);
    printf( "\nDeleted (%d) chars out of total %d chars.\n", killed, all );
    printf( "New player file: \"%s\" is OK.\n", NEWF);
    return 0;
} 
