/**************************************************************************
*  file: db.c , Database module.                          Part of DIKUMUD *
*  Usage: Loading/Saving chars, booting world, resetting etc.             *
*  Copyright (C) 1990, 1991 - see 'license.doc' for complete information. *
***************************************************************************/

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <time.h>

#include "structs.h"
#include "utils.h"
#include "spells.h"
#include "db.h"

#define WRITE_FILE	"players.name"
#define	OK		0
#define	NOT_FOUND	1
#define	NAME_EXIST	2

/*  declarations of most of the 'global' variables */
struct char_file_u *u_table ;
int number_of_player ;
struct spell_info_type spell_info[MAX_SPL_LIST];

/*  main procedure */
void main(int argc, char *argv[])
{
  int build_player_index(), status ;

  if ( argc < 3 ) { 
    fprintf(stderr, "Usage: %s player_file player_name transformed_name\n",
       argv[0]) ;
    fprintf(stderr, "       change player_name to transformed_name\n") ;
    fprintf(stderr, "       save new file into \"%s\"\n", WRITE_FILE) ;
    fprintf(stderr, "  NOTE ! the first letter must be capital letter !!!\n");
	}
  else {
      if ((status = build_player_index(argv[1], argv[2], argv[3])) != OK ) {
	if ( status == NOT_FOUND ) {
	  fprintf(stderr, "Can't find name '%s'\n", argv[2]) ;
	  exit(1) ;
	  }
	if ( status == NAME_EXIST ) {
	  fprintf(stderr, "Sorry, the name '%s' is exist already\n", argv[3]) ;
	  exit(1) ;
	  }
	}
      else {
        change_name(argv[2], argv[3]);
        }
      }
}

/* generate index table for the player file */
int build_player_index(char *player_file, char *before, char *after)
{
  int nr = -1, i;
  struct char_file_u dummy;
  extern struct char_file_u *u_table ;
  extern int number_of_player ;
  FILE *fl;
  int	found, exist ;

  if (!(fl = fopen(player_file, "rb+"))) {
    perror("build player index");
    exit(0);
    }

  /* count player number */
  for (; !feof(fl);) {
    fread(&dummy, sizeof(struct char_file_u), 1, fl);
    if (!feof(fl)) {  /* new record */
      if (nr == -1)	nr = 0;
      else	nr++ ;
    }
  }
  number_of_player = nr + 1 ;
  fclose(fl);

  if (!(fl = fopen(player_file, "rb+")))
  {
    perror("re-read player file");
    exit(0);
  }
  u_table = (struct char_file_u *) malloc ( number_of_player * sizeof(struct char_file_u)) ;
  if (u_table == NULL) {
    perror("utable") ;
    exit(1) ;
    }

  found = 0 ;
  exist = 0 ;
  for (i=0 ; i < number_of_player ;i ++ ) {
    fread(&(u_table[i]), sizeof(struct char_file_u), 1, fl);
    if ( strcmp(u_table[i].name, before) == NULL ) found = 1 ;
    if ( strcmp(u_table[i].name, after) == NULL ) exist = 1 ;
    }
  fclose(fl) ;

  if (found == 0) return NOT_FOUND ;
  else if (exist == 1) return NAME_EXIST ;
  else return OK ;
}

change_name(char *before, char *after)
{
	FILE	*fp ;
	int	i, found, exist ;
	extern struct char_file_u *u_table ;
	extern int number_of_player ;
	struct char_file_u *p;

	if ((fp = fopen(WRITE_FILE, "wb+")) == NULL) {
		perror("write player file");
		exit(0);
		}

	found = 0 ;
	for ( i = 0 ; i < number_of_player ; i ++ ) {
		p = &(u_table[i]) ;
		if ( strcmp(p->name, before) == NULL ) {
			found = 1 ;
			strcpy(p->name, after) ;
			}
		fwrite(p, sizeof(struct char_file_u), 1, fp);
		}
	if ( found == 0 ) {
		fprintf(stderr, "Can't find player \"%s\" \n", before) ;
		}
	else {
		fprintf(stderr, "Player \"%s\" transformed\n", before) ;
		fprintf(stderr, "New player file is \"%s\"\n", WRITE_FILE);
		}
	fclose(fp);
}

