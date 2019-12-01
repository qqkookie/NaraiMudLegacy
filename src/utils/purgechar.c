/* ***********************************************************
    1.  Delete specific player by name.
    2.  Purge player not logged in for long time.
    3.  Extract a entry in "players" file into new file.
    4.  Remove players with empty or duplicate entries in "players" file.
    	This function should not have needed or used.

      Result is saved as file named "players.new"
      purge will delete STASH file, too.
      WARNING!!! DON'T run this program while MUD is RUNNING.!!!
   ************************************************************ */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>
#include <time.h>

#include "char.h"
#include "gamedb.h"

#define FNAME_IN	"players"
#define FNAME_OUT	"players.new"

#define MAX_PC 		10000

#define MIN_DAYS 	31
#define ONE_DAY 	86400

// #define LOWER(c) (((c)>='A'  && (c) <= 'Z') ? ((c)+('a'-'A')) : (c))

#pragma GCC diagnostic ignored "-Wunused-result"
#pragma GCC diagnostic ignored "-Wunused-parameter"

struct char_file_u  Player;

unsigned CHUCK = sizeof(Player);

FILE *F_in, *F_out;

char Buffer[1000];

char ArgName[100] = {0};
int ArgNum = 0;

int g_all = 0;
int g_purged = 0;

// void show_PC( struct char_file_u *pst );

#define PRINTF		printf
#define PRINT(s)	fputs(s, stdout)

#define GETINPUT fgets(Buffer, sizeof(Buffer), stdin)

bool by_name(char *argname, int argnum)
{
    return( strcasecmp(Player.name, argname )== 0 );
}

bool by_date(char *argname, int argnum)
{
     return( Player.last_logon < argnum );
}

bool dup_name(char *argname, int argnum)
{
    int ii;
    static char *Namelist[MAX_PC+1] = { NULL } ;

    for( ii = 0; ii < MAX_PC && Namelist[ii] != NULL; ii++ )
	 if( strcasecmp(Namelist[ii], Player.name) == 0)
	    return(true);

    Namelist[ii] = strdup(Player.name);
    Namelist[ii+1] = NULL ;
    return(false);
}

bool empty_name(char *argname, int argnum)
{
    return (strlen(Player.name) == 0);
}

bool delete_stash(char name[])
{
    char path[200], namebuf[100];

    for( int i = 0; name[i]  && isalnum(name[i]); i++ )
	namebuf[i] = tolower(name[i]);

    sprintf( path, "%s/%c/%s.%s", STASH, namebuf[0], namebuf, STASH_EXT );
    PRINTF( "\nDeleting stash file: \"%s\"... ", path );

    return (unlink(path) == 0 );
}

bool delete_if_true( bool (*test)(char *argname, int argnum), bool del_stash )
{
    char name[100];
    while( fread( &Player , CHUCK , 1 , F_in ) == CHUCK ) {

	g_all++;
	if(!(test)(ArgName, ArgNum))
	    continue;

	strcpy(name, Player.name);

	PRINTF( "\nPLAYER: \"%s\"   \t(LEVEL %d, HIT %d, MANA %d )\n",
		name, Player.level, Player.points.max_hit, Player.points.max_mana );

	char *ts = ctime( &(Player.last_logon));
	ts[16] = '\0' ;
	PRINTF( "Last logon : %s :: (%ld) days ago.\n",
	    ts, (time(0) - Player.last_logon)/ONE_DAY) ;

	PRINTF("\nReally delete [%s] ? (y/n) >> ", name );

	GETINPUT;

	if ( Buffer[0] == 'y' || Buffer[0] == 'Y')
	{
	    if ( del_stash && !delete_stash(name))
		PRINT("Stash remove fail!\n");
	    else
		PRINT("Ok.\n");

	    g_purged++;
	    continue;
	}

	if ( fwrite( &Player , CHUCK , 1 , F_out ) != CHUCK ) {
	    PRINT("Error writing output file.\n");
	    return false;
	}
    }
    return true;
}

void copy_player(char *argname)
{
    while( fread( &Player, CHUCK , 1 , F_in ) == CHUCK ) {
	g_all++;
	if ( strcasecmp(Player.name, argname )==0 ) {
	    fwrite( &Player, CHUCK , 1 , F_out );
	    PRINTF( "\nPlayer [%s] extracted to \"%s\" file. \n",
		Player.name, FNAME_OUT);
	    return ;
	}
    }
    PRINT("\nNo such player\n");
}

int main()
{
    int deadline;

    if( !(F_in = fopen( FNAME_IN, "r") ) ) {
	PRINT( "Error opening '" FNAME_IN "' file.\n");
	return(1);
    }

    if( !(F_out=fopen( FNAME_OUT, "w") ) ) {
	PRINT("Error opening output '" FNAME_OUT "' file.\n");
	return(1);
    }

    PRINT("\n\n    WARNING!!! DON'T run this program while MUD is RUNNING!!!\n\n"
	"  [1] Delete a player by NAME\n"
	"  [2] Delete players not logged for many DAYS\n"
	"  [3] COPY a player entry to file.\n"
	"  [4] Delete players with DUPLICATE name\n"
	"  [5] Delete players with EMPTY name\n"
	"  [9] QUIT\n\n"
	"Select job [1-5,9] > " );

    GETINPUT;

    switch(atoi(Buffer)) {

    case 1 :
	PRINT("\nPlayer NAME to DELETE >>>> ");
	GETINPUT;
	strcpy(ArgName, Buffer);
	if ( strlen(Buffer) > 2 )
	    delete_if_true( by_name, true);
	break;

    case 2 :
	PRINT("\nNumber of DAYS not logged in >>>> ");
	GETINPUT;
	deadline = atoi(Buffer);
    	if ( deadline >= MIN_DAYS ) {
	    PRINTF( "\nPlayers not played within (%d) DAYS...\n", deadline);
	    ArgNum = time(0) - deadline * ONE_DAY ;
	    delete_if_true( by_date, true );
	}
	break;

    case 3 :
	PRINT("\nPlayer NAME to COPY >>>> ");
	GETINPUT;
	if ( strlen(Buffer) > 2 )
	    copy_player(Buffer);
	break;

    case 4 :
	PRINT( "\nDuplicate name entries...\n");
	/* dupcated name will NOT delete stash file. */
	delete_if_true( dup_name, false );
	break;

    case 5 :
	PRINT( "\nEmpty named entries...\n");
	/* dupcated name will NOT delete stash file. */
	delete_if_true( empty_name, false );
	break;

    case 9 :
    default :
	break;
    }

    fclose(F_in);
    fclose(F_out);
    PRINTF( "\nDeleted (%d) chars out of total %d chars.\n", g_purged, g_all );
    PRINTF( "New player file: \"%s\" is OK.\n", FNAME_OUT);
    return 0;
}
