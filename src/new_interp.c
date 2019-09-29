/* 
	new_interp.c  - new command interpreter
		improving search algorithm.
*/

#include <string.h>
#include <ctype.h>
#include <stdio.h>

#include "structs.h"
#include "comm.h"
#include "interpreter.h"
#include "db.h"
#include "utils.h"
#include "limits.h"
#include "command.h"
#include "memory.h"

#define	CMD_CLASS_ENG_UPPER	26
#define	CMD_CLASS_HANGUL	27
#define	CMD_CLASS_OTHER	28
#define	CMD_CLASS_SIZE	30		/* 26 + eng_upper + hangul + other */

/* extern variable */
extern char	*command[] ;	/* defined in interpreter.c */

/* local variable */
int	*command_hash[CMD_CLASS_SIZE] ;

void init_command_block()
{
	int	i, n, count, array[CMD_CLASS_SIZE] ;
	char	ch, **list ;
	
	for ( i = 0 ; i < CMD_CLASS_SIZE ; i++ )
		array[i] = 0 ;

	/* first step - analyze commands */
	list = command ;
	count = 0 ;
	while ( (ch = *(list[count])) != '\n' ) {	/* end of list is '\n' */
		if ( ch >= 'a' && ch <= 'z' )
			array[ ch - 'a' ] ++ ;
		else if ( ch >= 'A' && ch <= 'Z' )
			array[CMD_CLASS_ENG_UPPER] ++ ;
		else if ( is_hangul(ch) )
			array[CMD_CLASS_HANGUL] ++ ;
		else
			array[CMD_CLASS_OTHER] ++ ;	/* ex) ' , !  */
		count ++ ;
    }

	/* now, allocate for commands block */
	/* fprintf(stderr, "Analysis of %d commands\n", count) ; */
	for ( i = 0 ; i < CMD_CLASS_SIZE ; i++) {
		/*
          if ( i < 26 )
          fprintf(stderr, "%c : %d\n", i + 'a', array[i]) ;
          else if ( i == CMD_CLASS_ENG_UPPER )
          fprintf(stderr, "UPPER : %d\n", array[i]) ;
          else if ( i == CMD_CLASS_HANGUL )
          fprintf(stderr, "HANGUL : %d\n", array[i]) ;
          else if ( i == CMD_CLASS_OTHER )
          fprintf(stderr, "OTHER : %d\n", array[i]);
          else
          fprintf(stderr, "END OF LIST\n") ;
		*/

		if ( array[i] > 0 ) {
			command_hash[i] = (int *) malloc_string_type ( sizeof(int)
                                                           * (array[i] + 1)) ;
        }
    }

	/* re-use of array */
	for ( i = 0 ; i < CMD_CLASS_SIZE ; i++ )
		array[i] = 0 ;

	/* make table */
	for ( i = 0 ; i < count ; i ++) {
		ch = *(list[i]) ;
		if ( ch >= 'a' && ch <= 'z' ) {
			n = ch - 'a' ;
        }
		else if ( ch >= 'A' && ch <= 'Z' )
			n = CMD_CLASS_ENG_UPPER ;
		else if ( is_hangul(ch) )
			n = CMD_CLASS_HANGUL ;
		else
			n = CMD_CLASS_OTHER ;

		command_hash[n][ array[n] ] = i + 1 ;	/* index +1 */
		array[n] ++ ;
    }

	/* end mark : (-1) is end of array sign */
	for ( i = 0 ; i < CMD_CLASS_SIZE ; i++ )
		if ( command_hash[i] )
			command_hash[i][ array[i] ] = -1 ;

}

int search_hash_block(char *name, int length, int mode)
{
	int	i, n, count, found, search ;
	char	ch ;

	if ( length < 1 )	/* already found */
		return 0 ;
	if ( name == NULL )
		return 0 ;
	if ( *name == 0 )
		return 0 ;

	/* n : index */
	ch = *name ;
	if ( ch >= 'a' && ch <= 'z' )
		n = ch - 'a' ;
	else if ( ch >= 'A' && ch <= 'Z' )
		n = CMD_CLASS_ENG_UPPER ;
	else if ( is_hangul(ch) )
		n = CMD_CLASS_HANGUL ;
	else
		n = CMD_CLASS_OTHER ;

	i = 0 ;
	found = 0 ;
	if ( mode ) {	/* complete compare mode */
		if ( command_hash[n] ) {
			for ( i = 0 ; command_hash[n][i] > 0 ; i ++ ) {
				if ( strcmp(name, command[ command_hash[n][i] -1 ] ) == 0 ) {
					found = 1 ;
					count = command_hash[n][i] ;
					break ;
                }
            }
        }
		else	/* not found */
			found = 0 ;
    }
	else {	/* partial match mode */
		if ( command_hash[n] ) {
			for ( i = 0 ; command_hash[n][i] > 0 ; i++ ) {
				if ( strncmp(name, command[ command_hash[n][i] -1], length)
                     == 0 ) {
					found = 1 ;
					count = command_hash[n][i] ;
					break ;
                }
            }
        }
		else	/* not found ??? or found ??? */
			found = 0 ;
    }

	return ( found ? count : (-1) ) ;
}

#undef	CMD_CLASS_ENG_UPPER
#undef	CMD_CLASS_HANGUL
#undef	CMD_CLASS_OTHER
#undef	CMD_CLASS_SIZE

