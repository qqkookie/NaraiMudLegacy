/* ***********************************************************
    1.  Delete specific player by name.
    2.  Purge player not logged on for long time.
    3.  Remove players with duplicated entries in "players" file.
    	This function should not have needed or used.
    3.  Extract a entry in "players" file into new file.

      Result is saved as file named "players.new" 
      purge will delete STASH file, too.
      WARNING!!! DON'T run this program while MUD is RUNNING.!!!
   ************************************************************ */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "player.c"

#define MAX_PC 1000

#define DEADLINE 	30 

int all = 0;
int killed = 0;

char dname[100];
int  deadline;

int by_dname( struct char_file_u *st) 
{
    return( strcasecmp(st->name, dname )==0 );
}

int by_date( struct char_file_u *st) 
{
     return( st->last_logon  < deadline );
}

int duple( struct char_file_u *st) 
{
    char **pl;
    static char *namelist[MAX_PC] = { NULL } ; 
    
    for( pl = namelist; *pl && (pl-namelist) < MAX_PC ; pl++ )
	 if( strcasecmp(*pl, st->name) == 0)
	    return(1);

    *pl = strdup(st->name);
    *++pl = NULL ;
    return(0);
}
	 
void delete_if_true(char *name, 
		    int (*test)(struct char_file_u *st), int del_stash )
{
    char 	buf[100];
    struct char_file_u  st;

    while( fread( &st , CHUCK , 1 , FP ) > 0 ) { 
	all++;
	if((test)(&st)) {
	    show_PC( &st) ;
	    printf("\nReally delete [%s] ? (Y/N) >> ", st.name );
	    gets(buf);

	    if ( LOWER(buf[0]) == 'y' ) {
		if ( del_stash )
		    delete_stash(st.name) ;
		killed++;
		continue;
	    }
	} 
	if ( fwrite( &st , CHUCK , 1 , FN ) <= 0 ) {
	    fprintf(stderr, "Error writing output file.\n");
	    exit(3); 
	}
    }
}


void extract_it(char *name)
{
    char 	buf[100];
    struct char_file_u  st;

    while( fread( &st , CHUCK , 1 , FP ) > 0 ) { 
	all++;
	if ( strcasecmp(st.name, name )==0 ) {
	    show_PC( &st) ;
	    printf("\nOK to EXTRACT? (Y/N) >> ");
	    gets(buf);

	    if ( LOWER(buf[0]) == 'y' ) {
		fwrite( &st , CHUCK , 1 , FN );
		printf( "\nPlayer %s extracted  to \"%s\" file. \n",
		    st.name, NEWF);
		break;
	    }
	} 
    }
}

int main(int argc, char **argv)
{
    char 	buf[100];

    printf("\n    WARNING!!! DON'T run this program while MUD is RUNNING.!!!\n\n");

    printf("  [D] DELETE a player by NAME.\n" );
    printf("  [P] PURGE player not played long time.\n");
    printf("  [R] REMOVE DUPLICATED player entry.\n");
    printf("  [E] EXTRACT a player entry to file.\n");
    printf("  [Q] QUIT.\n");
    printf("\nSelect Function [D/P/R/E/Q] : " );

    gets(buf); 
    if( strlen(buf) == 0 )
	exit(0);

    open_files(1);

    switch( toupper(buf[0])) {
    case 'D' :
	printf("\nPlayer NAME to DELETE >>>> ");
	gets(buf); 
	strcpy(dname, buf);
	if ( strlen(buf) >= 2 )
	    delete_if_true(buf, by_dname, 1); 
	break;

    case 'P' : 
	printf("\nNumber of DAYS not logged in >>>> ");
	gets(buf); 
    	if ((deadline = atoi(buf)) < 7 )
	    deadline = DEADLINE ; 
	printf( "\nPlayers not played within (%d) DAYS...\n", deadline);
	deadline *= SECPERDAY ;
	deadline = time(0) - deadline ;
	delete_if_true(buf, by_date, 1 ); 
	break;

    case 'R' : 
	printf( "\nDuplicated Entries...\n");
	/* dupcated name will NOT delete stash file. */
	delete_if_true(buf, duple, 0 ); 
	break;

    case 'E' : 
	printf("\nPlayer NAME to EXTRACT >>>> ");
	gets(buf); 
	if ( strlen(buf) >= 2 ) {
	    printf( "\nExtrcting [%s] player entry into file...\n", buf);
	    extract_it(buf); 
	}
	break;

    case 'Q' :
    default :
	break;
    }

    fclose(FP);
    fclose(FN);
    printf( "\nDeleted (%d) chars out of total %d chars.\n", killed, all );
    printf( "New player file: \"%s\" is OK.\n", NEWF);
    return 0;
} 
