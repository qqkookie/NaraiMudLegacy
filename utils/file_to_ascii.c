/*

*/

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
#include "structs.h"

#define WRITE_FILE	"players.asc"

void build_player_index(char *player_file, int *n, struct char_file_u **np) ;
void dump_ability(FILE *fp, struct char_ability_data *np) ;
void dump_point(FILE *fp, struct char_point_data *np) ;
void dump_skill(FILE *fp, struct char_skill_data *np) ;
void dump_affected(FILE *fp, struct affected_type *np) ;
void write_result(int n, struct char_file_u *p) ;
void debug(char *str) ;

/*  main procedure */
main(int argc, char *argv[])
{
	struct char_file_u *u_table ;
	int number_of_player ;
	char	name[BUFSIZ] ;

	debug("Start") ;
	if ( argc < 2 ) { 
		fprintf(stderr, "Usage: %s player_file\n", argv[0]) ;
		fprintf(stderr, "       adjust the player file and save into %s\n",
			WRITE_FILE) ;
		exit(1) ;
		}
	else {
		strcpy(name, argv[1]) ;
		fprintf(stderr, "Open file %s\n", name) ;
		build_player_index(name, &number_of_player, &u_table);
		}

  write_result(number_of_player, u_table);
}

/* generate index table for the player file */
void build_player_index(char *file_name, int *nplayer,
	struct char_file_u **u_table)
{
	int	nr, i;
	struct char_file_u	dummy;
	char	tmp[BUFSIZ] ;
	FILE	*fl;

	fprintf(stderr, "Player file is %s\n", file_name) ;
	strcpy(tmp, file_name) ;
	debug("Open player file") ;
  if (!(fl = fopen(tmp, "rb+"))) {
    perror("build player index");
    exit(0);
    }

	*nplayer = 0 ;
	nr = -1 ;
  /* count player number */
	debug("Open player file twice") ;
  for (; !feof(fl);) {
    fread(&dummy, sizeof(struct char_file_u), 1, fl);
    if (!feof(fl)) {  /* std record */
      if (nr == -1)	nr = 0;
      else	nr++ ;
    }
  }
  *nplayer = nr + 1 ;
  fclose(fl);

	fprintf(stderr, "Number of player read = %d(%d)\n", *nplayer, nr) ;

	/*strcpy(tmp, file_name) ;*/
  if (!(fl = fopen(tmp, "rb+")))
  {
    perror("re-read player file");
    exit(0);
  }
  *u_table = (struct char_file_u *) malloc ( *nplayer * sizeof(struct char_file_u)) ;
  if (*u_table == NULL) {
    perror("utable") ;
    exit(1) ;
    }
  for (i=0 ; i < *nplayer ;i ++ ) {
    fread(&((*u_table)[i]), sizeof(struct char_file_u), 1, fl);
    }
  fclose(fl) ;
}

void dump_one_player(FILE *fp, struct char_file_u *p)
{
	int	i, j ;

	/* dump one player - strings first */
	fprintf(fp, "%s~\n", p->title) ;
	fprintf(fp, "%s~\n", p->description) ;
	fprintf(fp, "%s~\n", p->name) ;
	fprintf(fp, "%s~\n", p->pwd) ;

	/* dump stats */
	fprintf(fp, "%d %d %d %d %d %d %d %d %d\n",
		p->sex, p->class, p->level, (unsigned) p->birth, p->played,
		p->life, p->weight, p->height, p->load_room) ;


	fprintf(fp, "%d %d %d %d %d %d %d %d %d %d\n", p->spells_to_learn,
		p->alignment, (unsigned) p->last_logon, (unsigned) p->act,
		p->bank, p->conditions[0], p->conditions[1], p->conditions[2],
		p->quest[0], p->quest[1] ) ;

	for ( j = 0 ; j < 18 ; j ++) {
		/*
		fprintf(fp, "%d ", p->reserved[j]) ;
		*/
		fprintf(fp, "%d ", 0) ;
		}
	fprintf(fp, "\n") ;

	dump_ability(fp, &(p->abilities)) ;
	dump_point(fp, &(p->points)) ;
	dump_skill(fp, p->skills) ;
	dump_affected(fp, p->affected) ;
	fprintf(fp, "$\n") ;
}

void dump_ability(FILE *fp, struct char_ability_data *p)
{
	fprintf(fp, "%d %d %d %d %d %d\n",
		p->str, p->str_add, p->intel, p->wis, p->dex, p->con) ;
}

void dump_point(FILE *fp, struct char_point_data *p)
{
	fprintf(fp, "%d %d %d %d %d %d %d %d %d %d %d\n",
		p->mana, p->max_mana, p->hit, p->max_hit, p->move, p->max_move,
		p->armor, p->gold, p->exp, p->hitroll, p->damroll ) ;
}

void dump_skill(FILE *fp, struct char_skill_data *p)
{
	int	i, count ;

	count = 1 ;
	for ( i = 0 ; i < MAX_SKILLS ; i++, count++) {
		fprintf(fp, "%d %d ", p[i].learned, p[i].recognise) ;
		if ( count % 12 == 0 )
			fprintf(fp, "\n") ;
		}
	fprintf(fp, "\nS\n") ;
}

void dump_affected(FILE *fp, struct affected_type *p)
{
	int	i, count ;

	count = 1 ;
	for ( i = 0 ; i < MAX_AFFECT ; i++, count++) {
		fprintf(fp, "%d %d %d %d %d %d ", p[i].type, p[i].duration,
			p[i].modifier, p[i].location, p[i].bitvector, p[i].next) ;
		if ( count % 4 == 0 )
			fprintf(fp, "\n") ;
		}
	fprintf(fp, "\nA\n") ;
}

void write_result(int nplayer, struct char_file_u *u_table)
{
	FILE	*fp ;
	int	i ;
	struct char_file_u *p;

	if ((fp = fopen(WRITE_FILE, "w")) == NULL) {
		perror("write player file");
		exit(0);
		}

	fprintf(fp, "+%d\n", nplayer) ;
	for ( i = 0 ; i < nplayer ; i ++ ) {
		dump_one_player(fp, &(u_table[i])) ;
		}
	fclose(fp);
}

void debug(char *str)
{
	fprintf(stderr, "BUG: %s\n", str) ;
}
