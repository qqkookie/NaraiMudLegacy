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

#define PLAYERF	"players"
#define CHUCK (sizeof( struct char_file_u ))

#pragma GCC diagnostic ignored "-Wunused-result"

int main()
{
    struct char_file_u st;
    char name[100], passwd[100];
    FILE *FL;
    int newlevel ;

    if( !(FL=fopen( PLAYERF,"r+") ) ) {
	fprintf( stderr, "Error open file");
	exit(-1);
    }

    printf("\nChange Password or Level of non-active player...\n\n");
    printf("Player Name? ");
    fgets(name, sizeof(name)-1, stdin);
    printf("Enter new PASSWORD or LEVEL >> ");
    fgets(passwd, sizeof(passwd)-1, stdin);
    if ( !*name || !*passwd )
	exit(1);
    newlevel =  atoi(passwd);

    while( fread( &st , CHUCK  , 1 , FL )> 0) {
	if(strcasecmp(name,st.name)==0) {
	    printf("\nFound player NAME : \"%s\" ( LEVEL %d )\n",
	    		st.name, st.level ); 

	    if ( newlevel  > 0 &&  newlevel <= 44 ) {
		st.level = newlevel ; 
		printf("Change LEVEL to [%d]... ", st.level );
	    }
	    else {
		strncpy(st.pwd, crypt(passwd, st.pwd), 10);
		st.pwd[10] = '\0' ;
		printf("Change PASSWORD to [%s]... ", passwd );
	    }

	    if( (fseek( FL , -CHUCK, SEEK_CUR ) == 0 )
		    && ( fwrite( &st , CHUCK , 1 , FL ) > 0) )
		printf( "Ok!.\n"); 
	    else
		printf( "FAILED!.\n"); 
	    fclose(FL);
	    exit(0);
	}
    }
    fclose(FL);
    printf("\nNAME NOT found...\"%s\".\n", name );
    return 1;
}

