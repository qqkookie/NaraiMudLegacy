/* *******************************************************
    Change Password or Level of non-active player.
    It will update current "players" file.
    Don't change password or level of player who is playing NOW!
    Merged function of OLD change_pass.c and change_level.c
    ****************************************************** */

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>

#ifndef __FreeBSD__
#include <crypt.h>
#endif

#include "char.h"
#include "gamedb.h"

#define PLAYERFILE	"players"
#define CHUCK (sizeof( struct char_file_u ))

#pragma GCC diagnostic ignored "-Wunused-result"

int main()
{
    struct char_file_u player;
    char name[100], buf[100];
    FILE *p_file;
    int newlevel = 0;

    printf("\n\nCheck/rchange Password or Level of player...\n\n"
	    "  [1] Change password\n"
	    "  [2] Check password\n"
	    "  [3] Change character level\n"
	    "  [9] QUIT\n\n"
	    "Select job [1-3,9] > " );

    fgets(buf, sizeof(buf)-1, stdin);
    int job = atoi(buf);

    printf( "\nPlayer must not logged in now\n"
	    "Player Name? >>>> ");
    fgets(name, sizeof(name)-1, stdin);

    if ( job == 1 )
	printf("Enter NEW password >> ");
    else if ( job == 2 )
	printf("Enter password to CHECK >> ");
    else if ( job == 3 )
	printf("Enter new LEVEL >> ");
    else
	return 1;

    fgets(buf, sizeof(buf)-1, stdin);

    if ( !name[0] || !buf[0] )
	return 1 ;

    if ( job == 3 ) {
	newlevel = atoi(buf);
	if ( newlevel < 1 || newlevel > 44 )
	    return 1;
    }

    p_file = fopen( PLAYERFILE, "r+");
    if(!p_file) {
	printf("Error open '" PLAYERFILE "' file");
	return 1;
    }

    while( fread( &player , CHUCK  , 1 , p_file ) == CHUCK ) {
	// printf("Reading #%d %s\n", num, player.name );
	if(strcasecmp(name, player.name) == 0)
	    goto found;
    }

    printf("\nNAME NOT found...\"%s\".\n", name );
    fclose(p_file);
    return 1;

found:
    printf("\nFound player NAME : \"%s\" ( LEVEL %d )\n",
	player.name, player.level );

    if ( job == 2 ) {
	if ( strncmp(player.pwd, crypt(buf, player.pwd), 10) == 0)
	    printf("Checking passowrd OK!\n");
	else
	    printf("Checking passowrd does NOT match!\n");
	fclose (p_file);
	return 0;
    }

    if ( job == 1 ) {
	printf("Changing passowrd ");
	strncpy(player.pwd, crypt(buf, player.pwd), 10);
	player.pwd[10] = '\0' ;
    }
    else if ( job == 3 ) {
	printf("Setting level ");
	player.level = newlevel ;
    }
    else
	return 1;

    if( (fseek( p_file , -CHUCK, SEEK_CUR ) == 0 )
	    && ( fwrite( &player , CHUCK , 1 , p_file ) > 0) )
	printf( "Ok!.\n");
    else
	printf( "FAILED!.\n");
    fclose(p_file);
    return(0);
}

