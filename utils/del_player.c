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

#define WRITE_FILE	"players.del"

/*  declarations of most of the 'global' variables */
struct char_file_u *u_table ;
int number_of_player ;
struct spell_info_type spell_info[MAX_SPL_LIST];

/*  main procedure */
void main(int argc, char *argv[])
{
  void build_player_index() ;

  if ( argc < 3 ) { 
    fprintf(stderr, "Usage: %s player_file player_name\n", argv[0]) ;
    fprintf(stderr, "       delete the player and save new file into \"%s\"\n",
		WRITE_FILE) ;
	}
  else {
      build_player_index(argv[1]);
      delete_player(argv[1], argv[2]);
      }
}

/* generate index table for the player file */
void build_player_index(char *player_file)
{
  int nr = -1, i;
  struct char_file_u dummy;
  extern struct char_file_u *u_table ;
  extern int number_of_player ;
  FILE *fl;

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
  for (i=0 ; i < number_of_player ;i ++ ) {
    fread(&(u_table[i]), sizeof(struct char_file_u), 1, fl);
    }
  fclose(fl) ;
}

delete_player(char *file_name, char *player_name)
{
	FILE	*fp ;
	int	i, deleted ;
	extern struct char_file_u *u_table ;
	extern int number_of_player ;
	struct char_file_u *p;

	if ((fp = fopen(WRITE_FILE, "wb+")) == NULL) {
		perror("write player file");
		exit(0);
		}

	deleted = 0 ;
	for ( i = 0 ; i < number_of_player ; i ++ ) {
		p = &(u_table[i]) ;
		if ( strcmp(p->name, player_name) != NULL )
			fwrite(p, sizeof(struct char_file_u), 1, fp);
		else 
			deleted = 1 ;
		}
	if ( deleted == 0 ) {
		fprintf(stderr, "Can't find player \"%s\" in file \"%s\"\n",
			player_name, file_name) ;
		}
	else {
		fprintf(stderr, "Player \"%s\" deleted\n", player_name) ;
		fprintf(stderr, "New player file is \"%s\"\n", WRITE_FILE);
		}
	fclose(fp);
}

