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

#define READ_FILE	"players"
#define WRITE_FILE	"players.tr"

/*  declarations of most of the 'global' variables */
struct char_file_u *u_table ;
int number_of_player, printmode ;
struct spell_info_type spell_info[MAX_SPL_LIST];

/*  main procedure */
void main(int argc, char *argv[])
{
  void build_player_index() ;

	if ( argc < 3 ) {
		fprintf(stderr, "Usage: %s player_file mode\n", argv[0]) ;
		fprintf(stderr, "     mode 0 :  status of players\n") ;
		fprintf(stderr, "     mode 1 :  skills of players\n") ;
		fprintf(stderr, "     mode 2 :  money of players\n") ;
		fprintf(stderr, "     Name   :  show players status\n") ;
		exit(1) ;
		}
	else {
		build_player_index(argv[1]);
		if (isname(argv[2]))
			show_player_stat(argv[2]) ;
		else {
			printmode = atoi(argv[2]) ;
			show_all_players() ;
			}
		}
}

isname(char *str)
{
	int	i, leng ;

	leng = strlen(str) ;
	if ( leng == 1 && (*str >= '0' || *str <= '2'))
		return 0 ;
	else
		return 1 ;
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
		fprintf(stderr, "Can't open file \"%s\"\n", player_file) ;
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
		fprintf(stderr, "Malloc failed\n") ;
    exit(1) ;
    }
  for (i=0 ; i < number_of_player ;i ++ ) {
    fread(&(u_table[i]), sizeof(struct char_file_u), 1, fl);
    }
  fclose(fl) ;
}

show_all_players()
{
	extern struct char_file_u *u_table ;
	extern int number_of_player ;
	int i ;

	if (printmode==0){
		show_header(1,1,0,1,1,0,0,0,0,1,1,1,1,1,1,0,0);
		for ( i = 0 ; i < number_of_player ; i ++) {
			show_players(&(u_table[i]),1,1,0,1,1,0,0,0,0,1,1,1,1,1,1,0,0);
			}
		/*  num, name, sex, class, level, nspell, align, act, bank
			1,   1,    0,   1,     1,     0,      0,     0,   0,
			1,   1,        1,   1,   1,     1,     0,    0
			cond, ability, hit, mana, move, armor, gold, exp */
		}
	else if ( printmode == 2) {
		show_header(1,1,0,1,1,0,0,0,1,0,0,0,0,0,0,1,0);
		for ( i = 0 ; i < number_of_player ; i ++) {
			show_players(&(u_table[i]),1,1,0,1,1,0,0,0,1,0,0,0,0,0,0,1,0);
			}
		/* num, name, sex, class, level, nspell, align, act, bank
			1,   1,    0,   1,     1,     0,      0,     0,   1,
			0,   0,        0,   0,   0,     0,     1,    0
			cond, ability, hit, mana, move, armor, gold, exp */
		}
	else {	/* print mode == 1 */
		for ( i = 0 ; i < number_of_player ; i ++) {
			show_player_skills(&(u_table[i]));
			}
		}
}

show_header(num, name, sex, class, level, nspell, align, act, bank,
  cond, ability, hit, mana, move, armor, gold, exp)
int	num, name, sex, class, level, nspell, align, act, bank ;
int	cond, ability, hit, mana, move, armor, gold, exp;
{
if(num) printf("%-5s", "num") ;
if(name) printf("%-12s", "name") ;
if(sex) printf("%-4s", "sex") ;
if(class) printf("%2s", " ") ;
if(level) printf("%-4s", "lev") ;
if(nspell) printf("%-6s", "spell") ;
if(align) printf("%-6s", "align") ;
if(act) printf("%-6s", "act") ;
if(bank) printf("%-11s", "bank") ;
if(cond) printf("%-10s", "condition") ;
if(ability) printf("%-20s", "str int wis dex con") ;
if(hit) printf("%-6s", "hitp") ;
if(mana) printf("%-6s", "mana") ;
if(move) printf("%-6s", "move") ;
if(armor) printf("%-4s", "AC") ;
if(gold) printf("%-11s", "gold") ;
if(exp) printf("%-11s", "exp") ;
putchar('\n') ;
}

show_players(p, num, name, sex, class, level, nspell,
	 align, act, bank, cond, ability, hit, mana, move, armor, gold, exp)
struct char_file_u  *p ;
int	num, name, sex, class, level, nspell, align, act, bank ;
int	cond, ability, hit, mana, move, armor, gold, exp;
{
static int i = 0 ;
int j ;
i ++ ;
if(num) printf("%-5d", i) ;
if(name) printf("%-12s", p->name) ;
if(sex) {
    switch (p->sex) {
      case SEX_NEUTRAL : printf("Neu ") ; break ;
      case SEX_MALE : printf("Man ") ; break ;
      case SEX_FEMALE : printf("Fem ") ; break ;
      }
    }
if(class) {
    switch (p->class) {
      case CLASS_MAGIC_USER : printf("M ") ; break ;
      case CLASS_CLERIC : printf("C ") ; break ;
      case CLASS_THIEF : printf("T ") ; break ;
      case CLASS_WARRIOR : printf("W ") ; break ;
    }
  }
if(level) printf("%-4d", p->level) ;
if(nspell) printf("%-6d", p->spells_to_learn) ;
if(align) printf("%-6d", p->alignment) ;
if(act) printf("%-6d", p->act) ;
if(bank) printf("%-11d", p->bank) ;
if(cond)
 printf("%-3d%-3d%-3d ", p->conditions[0], p->conditions[1], p->conditions[2]);
if(ability) 
 printf("%3d/%-3d %-3d%-3d%-3d%-3d ", p->abilities.str, p->abilities.str_add,
  p->abilities.intel, p->abilities.wis, p->abilities.dex, p->abilities.con );
if(hit) printf("%-5d,", p->points.max_hit) ;
if(mana) printf("%-5d,", p->points.max_mana) ;
if(move) printf("%-5d ", p->points.max_move) ;
if(armor) printf("%-4d", p->points.armor) ;
if(gold) printf("%-11d", p->points.gold) ;
if(exp) printf("%-11d", p->points.exp) ;
putchar('\n') ;
}

show_player_stat(char *name)
{
  struct char_file_u  *p ;
  int i, j, found ;
  extern int number_of_player ;
  extern struct char_file_u *u_table ;

  found = 0 ;
  for ( i = 0 ; i < number_of_player ; i ++ ) {
	p = &(u_table[i]) ;
	if ( strcmp(p->name, name) == 0) {
		found = 1 ;
		break ;
		}
	}

  if (found == 0 ) {
	fprintf(stderr, "Can't find player \"%s\"\n", name) ;
	return ;
	}

  printf("Name  : %s  ", p->name) ;
    switch (p->sex) {
      case SEX_NEUTRAL : printf("  neutral ") ; break ;
      case SEX_MALE    : printf("  male ") ; break ;
      case SEX_FEMALE  : printf("  female ") ; break ;
      }
    switch (p->class) {
      case CLASS_MAGIC_USER : printf("Magic User") ; break ;
      case CLASS_CLERIC : printf("Cleric") ; break ;
      case CLASS_THIEF : printf("Thief") ; break ;
      case CLASS_WARRIOR : printf("Warrior") ; break ;
    }
  printf("(%d level)\n", p->level) ;
  printf("Passwd: %s\n", p->pwd);
  printf("Birth : %s", (char *)ctime((time_t *) &(p->birth))) ;
  printf("Last login: %s", (char *)ctime((time_t *) &(p->last_logon))) ;
  printf("Played: %d    Life: %d\n", p->played, p->life) ;

  printf("Spell : %d practice left    ", p->spells_to_learn) ;
  printf("Align : %d\n", p->alignment) ;
  printf("Title : %s\n", p->title) ;
  printf("Description: %s\n", p->description) ;
  printf("Act   :   %d     ", p->act) ;
  printf("Condition is %d %d %d   ", p->conditions[0], p->conditions[1],
	p->conditions[2]);
  printf("Ability: %3d/%-3d %-3d%-3d%-3d%-3d\n", p->abilities.str,
	p->abilities.str_add, p->abilities.intel, p->abilities.wis,
	p->abilities.dex, p->abilities.con );
  printf("Body  : [%d,%d,%d]    ", p->points.max_hit, p->points.max_mana,
	p->points.max_move) ;
  printf("Armor : %d\n", p->points.armor) ;
  printf("Money : %d coins in bank, ", p->bank) ;
  printf("%d coins in pocket\n", p->points.gold) ;
  printf("Exp   : %d", p->points.exp) ;
  putchar('\n') ;
  printf("Skills:\n") ;
  for ( i = 0 ; i < MAX_SKILLS ; i ++ ) {
	printf("%3d%c", p->skills[i].learned, (i%20==19) ? '\n' : ' ') ;
	}
putchar('\n') ;
}

show_player_skills(struct char_file_u *p)
{
static int i = 0 ;
int j ;
i ++ ;
printf("Player number %d : %s (", i, p->name) ;
switch (p->class) {
  case CLASS_MAGIC_USER : printf("M") ; break ;
  case CLASS_CLERIC : printf("C") ; break ;
  case CLASS_THIEF : printf("T") ; break ;
  case CLASS_WARRIOR : printf("W") ; break ;
}
printf("%d)\n", p->level) ;

/* skill data */
for (j=0; j < MAX_SKILLS ; j ++ ) {
printf("%3d%c", (p->skills)[j].learned, (j%20 == 19) ? '\n' : ' ') ;
}
printf("\n\n") ;
}
