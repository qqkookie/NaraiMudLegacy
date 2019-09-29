/**************************************************************************
***************************************************************************/

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <time.h>

#include "strnew.h"

#define WRITE_FILE	"players.trans"

cp_ability(struct char_ability_data *np, struct char_ability_data *p);
cp_point(struct char_point_data *np, struct char_point_data *p);
/*  declarations of most of the 'global' variables */
struct char_file_u *u_table ;
struct new_char_file_u *new_u_table ;
int number_of_player ;

/*  main procedure */
void main(int argc, char *argv[])
{
  void build_player_index() ;

  if ( argc < 2 ) { 
	fprintf(stderr, "Usage: %s player_file\n", argv[0]) ;
	fprintf(stderr, "       adjust the player file and save into %s\n",
		WRITE_FILE) ;
	}
  else
      build_player_index(argv[1]);

  modify_player();
  write_result();
}

/* generate index table for the player file */
void build_player_index(char *player_file)
{
  int nr = -1, i;
  struct char_file_u dummy;
  extern struct char_file_u *u_table ;
  extern struct new_char_file_u *new_u_table ;
  extern int number_of_player ;
  FILE *fl;

	fprintf(stderr, "table size = %d, new size = %d\n",
		sizeof(struct char_file_u), sizeof(struct new_char_file_u)) ;

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
  new_u_table = (struct new_char_file_u *) malloc ( number_of_player * sizeof(struct new_char_file_u)) ;
  if (u_table == NULL) {
    perror("utable") ;
    exit(1) ;
    }
  for (i=0 ; i < number_of_player ;i ++ ) {
    fread(&(u_table[i]), sizeof(struct char_file_u), 1, fl);
    }
  fclose(fl) ;
}
modify_player()
{
	int	i, j ;
	extern struct char_file_u *u_table ;
	extern struct new_char_file_u *new_u_table ;
	extern int number_of_player ;
	struct char_file_u *p;
	struct new_char_file_u *np;

	for(i=0; i < number_of_player ; i++){ 
		p = &(u_table[i]) ;
		np = &(new_u_table[i]) ;
		/* copy 1 by 1 */
		np->sex = p->sex ;
		np->class = p->class ;
		np->level = p->level ;
		np->birth = p->birth ;
		np->played = p->played ;
		np->life = p->life ;
		np->weight = p->weight ;
		np->height = p->height ;
		strcpy(np->title, p->title) ;
		strcpy(np->description, p->description) ;
		np->load_room = p->load_room ;
		  cp_ability(&(np->abilities), &(p->abilities)) ;
		  cp_point(&(np->points), &(p->points)) ;
		  cp_skill(np->skills, p->skills) ;
		  cp_affected(np->affected, p->affected) ;
		np->spells_to_learn = p->spells_to_learn ;
		np->alignment = p->alignment ;
		np->last_logon = p->last_logon ;
		np->act = p->act ;
		np->bank = p->bank ;
		strcpy(np->name, p->name) ;
		strcpy(np->pwd, p->pwd) ;
		np->conditions[0] = p->conditions[0] ;
		np->conditions[1] = p->conditions[1] ;
		np->conditions[2] = p->conditions[2] ;
		np->quest[0] = p->quest[0] ;
		np->quest[1] = p->quest[1] ;
		for ( j = 0 ; j < 18 ; j ++)
			np->reserved[j] = 0 ;
		}
}

cp_ability(struct char_ability_data *np, struct char_ability_data *p)
{
	np->str = p->str ;
	np->str_add = p->str_add ;
	np->intel = p->intel ;
	np->wis = p->wis ;
	np->dex = p->dex ;
	np->con = p->con ;
}

cp_point(struct char_point_data *np, struct char_point_data *p)
{
np->mana = p->mana ;
	fprintf(stderr, "%d %d\n", (int)np->mana, (int)p->mana) ;
np->max_mana = p->max_mana ;
np->hit = p->hit ;
np->max_hit = p->max_hit ;
np->move = p->move ;
np->max_move = p->max_move ;
np->armor = p->armor ;
np->gold = p->gold ;
np->exp = p->exp ;
np->hitroll = p->hitroll ;
np->damroll = p->damroll ;
}

cp_skill(struct char_skill_data *np, struct char_skill_data *p)
{
	int	i ;
	for ( i = 0 ; i < MAX_SKILLS ; i++) {
		np[i].learned = p[i].learned ;
		np[i].recognise = p[i].recognise ;
		}
}

cp_affected(struct new_affected_type *np, struct affected_type *p)
{
	int	i ;
	for ( i = 0 ; i < MAX_AFFECT ; i++) {
		np[i].type = p[i].type ;
		np[i].duration = p[i].duration ;
		np[i].modifier = p[i].modifier ;
		np[i].location = p[i].location ;
		np[i].bitvector = p[i].bitvector ;
		np[i].next = (struct affected_type *) p[i].next ;
		}
}

write_result()
{
	FILE	*fp ;
	int	i ;
	extern struct new_char_file_u *new_u_table ;
	extern int number_of_player ;
	struct new_char_file_u *p;

	if ((fp = fopen(WRITE_FILE, "wb+")) == NULL) {
		perror("write player file");
		exit(0);
		}

	for ( i = 0 ; i < number_of_player ; i ++ ) {
		p = &(new_u_table[i]) ;
		fwrite(p, sizeof(struct new_char_file_u), 1, fp);
		}
	fclose(fp);
}

