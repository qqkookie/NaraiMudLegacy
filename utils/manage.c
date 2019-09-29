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

#define WRITE_FILE	"players.tr"
#define DELETED_FILE	"deleted.name"

/*  declarations of most of the 'global' variables */
struct char_file_u *u_table ;
int number_of_player ;
struct spell_info_type spell_info[MAX_SPL_LIST];
int proc_level = 2 ;	/* process level : how much strict to them ? */

char *proc_options(int argc, char *argv[]) ;

/*  main procedure */
void main(int argc, char *argv[])
{
  void build_player_index() ;
  void assign_spell_pointers(void);
	char	*name ;

  assign_spell_pointers() ;

  if ((name = proc_options(argc, argv)) == NULL ) { 
	fprintf(stderr, "Usage: %s [-1 | ... | -9] player_file\n", argv[0]) ;
	fprintf(stderr, "       adjust the player file and save into %s\n",
		WRITE_FILE) ;
	fprintf(stderr, "       and deleted player name is saved into %s\n",
		DELETED_FILE) ;
	}
  else
      build_player_index(name);

  modify_player();
  write_result();
}

char *proc_options(int argc, char *argv[])
{
	char	*tmp ;
	extern int proc_level ;
	int	i ;

	tmp = NULL ;
	for ( i = 1 ; i < argc ; i ++ ) {
		if (argv[i][0] == '-') {
			switch(argv[i][1]) {
				case '1' :	proc_level = 1 ; break ;
				case '2' :	proc_level = 2 ; break ;
				case '3' :	proc_level = 3 ; break ;
				case '4' :	proc_level = 4 ; break ;
				case '5' :	proc_level = 5 ; break ;
				case '6' :	proc_level = 6 ; break ;
				case '7' :	proc_level = 7 ; break ;
				case '8' :	proc_level = 8 ; break ;
				case '9' :	proc_level = 9 ; break ;
				default :
					proc_level = 2 ;
					break ;
				}
			}
		else {	/* file name */
			if ( tmp == NULL )	/* first name */
				tmp = argv[i] ;
			else
				return NULL ;	/* file name duplicated ? */
		}
	}

	return (tmp) ;
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
modify_player()
{
	int	i, j ;
	extern struct char_file_u *u_table ;
	extern int number_of_player ;
	extern struct spell_info_type spell_info[MAX_SPL_LIST];
	struct char_file_u *p;

	for(i=0; i < number_of_player ; i++){ 
		p = &(u_table[i]) ;
		if ( i > 5 && p->level == 44) {
			p->level = 0 ;
			}
		if ( p->level <  IMO) {
			if( p->abilities.str > 18) {
				p->abilities.str = 18 ;
				p->abilities.str_add = 100 ;
				}
			if( p->abilities.str_add > 100) 
				p->abilities.str_add = 100 ;
			if( p->abilities.intel > 18) 
				p->abilities.intel = 18 ;
			if( p->abilities.wis > 18) 
				p->abilities.wis = 18 ;
			if( p->abilities.dex > 18) 
				p->abilities.dex = 18 ;
			if( p->abilities.con > 18) 
				p->abilities.con = 18 ;
			for (j=0; j < MAX_SKILLS ; j ++ ) {
			if(spell_info[j].min_level[p->class -1] > p->level) {
				(p->skills)[j].learned = 0 ;
				}
			    }
			}
		}
}

write_result()
{
	FILE	*fp, *fdel ;
	int	i, delete ;
	extern struct char_file_u *u_table ;
	extern int number_of_player ;
	extern int proc_level ;
	struct char_file_u *p;
	time_t	current, time_unlog, one_day ;

	current = time(0) ;
	one_day = 3600 * 24 ;	/* in second */

	if ((fp = fopen(WRITE_FILE, "wb+")) == NULL) {
		perror("write player file");
		exit(0);
		}

	if ((fdel = fopen(DELETED_FILE, "w")) == NULL) {
		perror("write player file");
		exit(0);
		}

	for ( i = 0 ; i < number_of_player ; i ++ ) {
		p = &(u_table[i]) ;
		delete = 0 ;
		if ( strlen(p->name) <= 0) delete = 1 ;
		time_unlog = current - p->last_logon ;
		if ( p->level < 1 )
			delete = 1 ;
		switch(proc_level) {
			case 9 :
				if ( p->level < IMO )
					delete = 1 ;
					break ;
			case 8 :
			case 7 :
				if ( p->level < IMO && time_unlog > 80*(9-proc_level)*one_day )
					delete = 1 ;
			case 6 :	/* most strict */
			case 5 :
				if ( p->level < 30 && time_unlog > 50*(7-proc_level)*one_day )
					delete = 1 ;
			case 4 :
			case 3 :
			case 2 :	/* take easy */
			default :
				if ( p->level < 13 && time_unlog > 6*(7-proc_level)*one_day )
					delete = 1 ;
				break ;
			}
		if ( delete == 0) 
			fwrite(p, sizeof(struct char_file_u), 1, fp);
		else { /* deleted */
			fprintf(fdel, "%s level %d mv %c/%c%s.x.* ../deleted\n",
			p->name, p->level, tolower(*(p->name)),
			tolower(*(p->name)),
			&((p->name)[1])) ;
			}
		}
	fclose(fp);
	fclose(fdel) ;
}

#include <stdio.h>
#include <assert.h>

#define SPELLO(nr,beat,pos,l0,l1,l2,l3,s0,s1,s2,s3,mana,tar,func) { \
 spell_info[nr].spell_pointer = NULL;    \
 spell_info[nr].beats = (beat);            \
 spell_info[nr].minimum_position = (pos);  \
 spell_info[nr].min_usesmana = (mana);     \
 spell_info[nr].min_level[0] = (l0); \
 spell_info[nr].min_level[1] = (l1); \
 spell_info[nr].min_level[2] = (l2); \
 spell_info[nr].min_level[3] = (l3); \
 spell_info[nr].max_skill[0] = (s0); \
 spell_info[nr].max_skill[1] = (s1); \
 spell_info[nr].max_skill[2] = (s2); \
 spell_info[nr].max_skill[3] = (s3); \
 spell_info[nr].targets = (tar);     \
}

#define SPELL_LEVEL(ch,sn) (spell_info[sn].min_level[GET_CLASS(ch)-1])

void assign_spell_pointers(void)
{
 int i;
 int cast=0;

 for(i=0; i<MAX_SPL_LIST; i++)
  spell_info[i].spell_pointer = 0;

 /* From spells1.c */

 SPELLO(32,12,POSITION_FIGHTING, 1, 2, 3, 3,95,60,30,30,15, 
  TAR_CHAR_ROOM | TAR_FIGHT_VICT, cast );
 SPELLO( 8,12,POSITION_FIGHTING, 3, 5, 6, 6,95,60,30,30,15,
  TAR_CHAR_ROOM | TAR_FIGHT_VICT, cast);
 SPELLO( 5,12,POSITION_FIGHTING, 5, 8,10,10,95,60,30,30,15,
  TAR_CHAR_ROOM | TAR_FIGHT_VICT, cast);
 SPELLO(37,12,POSITION_FIGHTING, 7,10,12,12,95,60,30,30,15,
  TAR_CHAR_ROOM | TAR_FIGHT_VICT, cast);
 SPELLO(30,12,POSITION_FIGHTING, 9,13,16,16,95,60,30,30,15,
  TAR_CHAR_ROOM | TAR_FIGHT_VICT, cast);
 SPELLO(10,12,POSITION_FIGHTING,11,15,18,18,95,60,30,30,15,
  TAR_CHAR_ROOM | TAR_FIGHT_VICT, cast);
 SPELLO(25,12,POSITION_FIGHTING,13,20,15,20,95,40,65,40,35,
  TAR_CHAR_ROOM | TAR_FIGHT_VICT, cast);
 SPELLO(26,12,POSITION_FIGHTING,15,40,40,40,95,30,30,30,15,
  TAR_CHAR_ROOM | TAR_FIGHT_VICT, cast);
 SPELLO(23,12,POSITION_FIGHTING,15, 7,40,40,80,95,25,50,15,
  TAR_IGNORE, cast);
 SPELLO(22,12,POSITION_FIGHTING,25,10,40,40,30,95,10,30,15,
  TAR_CHAR_ROOM | TAR_FIGHT_VICT, cast);
 SPELLO( 6,12,POSITION_FIGHTING,20,15,40,40,80,95,10,10,15,
  TAR_CHAR_ROOM | TAR_FIGHT_VICT, cast);
 SPELLO(27,12,POSITION_FIGHTING,40,15,40,40,80,95,10,10,15,
  TAR_CHAR_ROOM | TAR_FIGHT_VICT, cast);
 SPELLO(45,12,POSITION_FIGHTING,22,25,40,40,85,95,25,25,15,
  TAR_CHAR_ROOM | TAR_FIGHT_VICT, cast);
 SPELLO(46,12,POSITION_STANDING,30,40,40,40,95,75,75,10,30,
  TAR_CHAR_ROOM | TAR_FIGHT_VICT, cast);
 SPELLO(72,12,POSITION_FIGHTING,4,40,40,40,95,30,30,30,3,
  TAR_CHAR_ROOM | TAR_FIGHT_VICT, cast);
 SPELLO(75,36,POSITION_FIGHTING,17,40,40,40,95,30,30,30,70,
  TAR_CHAR_ROOM | TAR_FIGHT_VICT, cast);
 SPELLO(76,12,POSITION_FIGHTING,22,40,40,40,95,37,37,37,15,
  TAR_CHAR_ROOM | TAR_FIGHT_VICT, cast);
 SPELLO(77,12,POSITION_FIGHTING,23,18,40,40,90,95,50,50,30,
  TAR_IGNORE, cast);

 /* Spells2.c */

 SPELLO( 1,12,POSITION_STANDING, 5, 1,40,40,95,95,60,10, 5,
  TAR_CHAR_ROOM, cast);
 SPELLO( 2,12,POSITION_FIGHTING, 8,18,40,40,95,95,50,10,35,
  TAR_SELF_ONLY, cast);
 SPELLO( 3,12,POSITION_STANDING,25, 5,40,40,25,95,10,25, 5,
  TAR_OBJ_INV | TAR_OBJ_EQUIP | TAR_CHAR_ROOM, cast);
 SPELLO( 4,12,POSITION_STANDING, 8, 6,40,40,95,95,50,10,15,
  TAR_CHAR_ROOM, cast);
 SPELLO( 7,12,POSITION_STANDING,14,26,40,40,95,10,25,20,20,
  TAR_CHAR_ROOM | TAR_SELF_NONO, cast);
 SPELLO( 9,12,POSITION_STANDING, 6,12,40,40,95,95,20,10,10,
  TAR_OBJ_ROOM,cast);
 SPELLO(11,12,POSITION_STANDING,20,24,25,25,90,80,60,40,50,
  TAR_CHAR_ZONE,cast);
 SPELLO(12,12,POSITION_STANDING,10, 3,20,20,40,95,25,10, 5,
  TAR_IGNORE, cast);
 SPELLO(13,12,POSITION_STANDING,10, 2,20,20,40,95,25,10, 5,
  TAR_OBJ_INV | TAR_OBJ_EQUIP, cast);
 SPELLO(14,12,POSITION_STANDING,20, 4,40,40,50,95,10,10, 5,
  TAR_CHAR_ROOM, cast);
 SPELLO(15,12,POSITION_FIGHTING,16, 9,20,18,60,95,45,50,15,
  TAR_CHAR_ROOM, cast);
 SPELLO(16,12,POSITION_FIGHTING, 4, 1,10,10,50,95,45,45,15,
  TAR_CHAR_ROOM, cast);
 SPELLO(17,12,POSITION_STANDING,12,20,40,40,95,30,50,10,20,
  TAR_CHAR_ROOM | TAR_OBJ_ROOM | TAR_OBJ_INV | TAR_OBJ_EQUIP, cast);
 SPELLO(18,12,POSITION_STANDING,20, 4,20,25,50,95,75,25, 5,
  TAR_CHAR_ROOM | TAR_SELF_ONLY, cast);
 SPELLO(19,12,POSITION_STANDING, 2, 5,20,30,95,95,60,10, 5,
  TAR_CHAR_ROOM | TAR_SELF_ONLY, cast);
 SPELLO(20,12,POSITION_STANDING,9,11,40,40,95,95,50,20,50,
  TAR_OBJ_INV | TAR_OBJ_EQUIP, cast);
 SPELLO(21,12,POSITION_STANDING,40,30,40,40,60,95,40,30,50,
  TAR_CHAR_ROOM, cast);
 SPELLO(24,12,POSITION_STANDING,12,40,40,40,90,50,30,20,100,
  TAR_OBJ_INV | TAR_OBJ_EQUIP, cast);
 SPELLO(28,12,POSITION_FIGHTING,24,14,25,25,40,95,30,30,50,
  TAR_CHAR_ROOM, cast);
 SPELLO(29,12,POSITION_STANDING, 4,20,40,40,95,50,60,10, 5,
  TAR_CHAR_ROOM|TAR_OBJ_INV|TAR_OBJ_ROOM|TAR_OBJ_EQUIP,cast);
 SPELLO(31,12,POSITION_STANDING, 6,10,40,40,95,95,60,10,20,
  TAR_OBJ_WORLD, cast);
 SPELLO(33,12,POSITION_STANDING,20, 8,40,40,60,95,60,10,20,
  TAR_CHAR_ROOM | TAR_SELF_NONO | TAR_OBJ_INV | TAR_OBJ_EQUIP, cast);
 SPELLO(34,12,POSITION_STANDING,20, 6,40,40,60,95,10,10, 5,
  TAR_CHAR_ROOM | TAR_SELF_ONLY, cast);
 SPELLO(35,12,POSITION_STANDING,40,12,40,40,60,95,10,10, 5,
  TAR_CHAR_ROOM|TAR_OBJ_INV|TAR_OBJ_EQUIP|TAR_OBJ_ROOM, cast);
 SPELLO(36,12,POSITION_STANDING,25,13,40,40,40,95,10,10,33,
  TAR_CHAR_ROOM, cast);
 SPELLO(38,12,POSITION_STANDING,14,20,40,40,95,80,50,10,20,
  TAR_CHAR_ROOM, cast);
 SPELLO(39,12,POSITION_STANDING, 7,20,40,40,95,50,20,40,20,
  TAR_CHAR_ROOM | TAR_SELF_ONLY, cast);
 SPELLO(40,12,POSITION_STANDING,20, 8,40,40,60,95,10,10,50,
  TAR_CHAR_WORLD, cast);
 SPELLO(41,12,POSITION_STANDING, 1,20,20,20,95,95,95,95, 5,
  TAR_CHAR_ROOM | TAR_OBJ_ROOM | TAR_SELF_NONO, cast);
 SPELLO(42,12,POSITION_STANDING,20,11,40,40,50,95,50,10, 5,
  TAR_CHAR_ROOM | TAR_SELF_ONLY, cast);
 SPELLO(43,12,POSITION_STANDING,40, 9,40,40,50,95,40,30, 5,
  TAR_CHAR_ROOM | TAR_OBJ_INV | TAR_OBJ_ROOM, cast);
 SPELLO(44,12,POSITION_STANDING,15, 7,17,19,50,95,40,30, 5,
   TAR_CHAR_ROOM | TAR_SELF_ONLY,cast);
 SPELLO(71,12,POSITION_STANDING,9,23,40,40,95,25,20,15,25,
   TAR_SELF_ONLY, cast);
 SPELLO(73,12,POSITION_STANDING,15,22,40,40,95,20,10,20,10,
  TAR_CHAR_ROOM | TAR_SELF_ONLY, cast);
 SPELLO(74,12,POSITION_STANDING,17,40,40,40,95,40,35,30,25,
  TAR_CHAR_ROOM , cast);
 SPELLO(78,12,POSITION_STANDING,19,28,40,40,95,35,30,25,45,
   TAR_CHAR_ROOM , cast);
 SPELLO(79,12,POSITION_FIGHTING,40,21,40,40,40,95,30,30,70,
  TAR_CHAR_ROOM, cast);
 SPELLO(80,12,POSITION_FIGHTING,25,40,40,40,95,20,20,20,50,
  TAR_CHAR_ROOM, cast);
 SPELLO(81,12,POSITION_STANDING,17,23,40,40,95,95,35,20,30,
  TAR_CHAR_ROOM, cast);
 SPELLO(82,12,POSITION_STANDING,25,40,40,40,95,95,35,20,40,
  TAR_SELF_ONLY, cast);
 SPELLO(83,12,POSITION_STANDING,7,5,40,40,95,95,35,20,40,
  TAR_CHAR_WORLD, cast);
 SPELLO(84,12,POSITION_FIGHTING,40,30,40,40,80,95,25,50,85,
  TAR_IGNORE, cast);
 SPELLO(47,1,POSITION_STANDING,IMO+4,IMO+4,IMO+4,IMO+4,0,0,0,0,10,TAR_IGNORE,0);
 SPELLO(48,1,POSITION_STANDING,IMO+4,IMO+4,IMO+4,IMO+4,0,0,0,0,10,TAR_IGNORE,0);
 SPELLO(49,1,POSITION_STANDING,IMO+4,IMO+4,IMO+4,IMO+4,0,0,0,0,10,TAR_IGNORE,0);
 SPELLO(50,1,POSITION_STANDING,IMO+4,IMO+4,IMO+4,IMO+4,0,0,0,0,10,TAR_IGNORE,0);
 SPELLO(51,1,POSITION_STANDING,IMO+4,IMO+4,IMO+4,IMO+4,0,0,0,0,10,TAR_IGNORE,0);
 SPELLO(52,1,POSITION_STANDING,IMO+4,IMO+4,IMO+4,IMO+4,0,0,0,0,10,TAR_IGNORE,0);
 SPELLO(53,1,POSITION_STANDING,17,40,40,40,95,5,5,5,25,TAR_OBJ_INV,
   cast);
 SPELLO(54,1,POSITION_STANDING,IMO+4,IMO+4,IMO+4,IMO+4,0,0,0,0,10,TAR_IGNORE,0);
 SPELLO(55,1,POSITION_STANDING,1,1,1,1,30, 1,95,20,10,TAR_IGNORE,0);
 SPELLO(56,1,POSITION_STANDING,1,1,1,1,30, 1,95,20,100,TAR_IGNORE,0);
 SPELLO(57,1,POSITION_STANDING,1,1,1,1,30, 1,95,20,100,TAR_IGNORE,0);
 SPELLO(58,1,POSITION_STANDING,1,1,1,1,30, 1,95,30,100,TAR_IGNORE,0);
 SPELLO(59,1,POSITION_STANDING,1,1,1,1,30, 1,95,20,100,TAR_IGNORE,0);
 SPELLO(60,1,POSITION_STANDING,1,1,1,1,30,10,60,90,100,TAR_IGNORE,0);
 SPELLO(61,1,POSITION_STANDING,1,1,1,1,30,10,30,90,100,TAR_IGNORE,0);
 SPELLO(62,1,POSITION_STANDING,1,1,1,1,30,40,30,90,100,TAR_IGNORE,0);
 SPELLO(63,1,POSITION_STANDING,IMO,IMO,10,8,0,0,100,100,0,TAR_IGNORE,0);
 SPELLO(64,1,POSITION_STANDING,IMO,IMO,27,15,0,0,100,100,0,TAR_IGNORE,0);
 SPELLO(65,1,POSITION_STANDING,IMO,29,IMO,25,0,70,0,100,0,TAR_IGNORE,0);
 SPELLO(85,1,POSITION_STANDING,IMO,IMO,1,33,0,0,95,80,0,TAR_IGNORE,0);
 SPELLO(86,1,POSITION_STANDING,IMO,IMO,1,IMO,0,0,95,0,0,TAR_IGNORE,0);
 SPELLO(87,1,POSITION_STANDING,IMO,IMO,1,29,0,0,95,50,0,TAR_IGNORE,0);
 SPELLO(88,1,POSITION_STANDING,IMO,IMO,5,IMO,0,0,95,0,0,TAR_IGNORE,0);
 SPELLO(89,1,POSITION_STANDING,IMO,IMO,IMO,17,0,0,0,90,0,TAR_IGNORE,0);
 SPELLO(90,12,POSITION_STANDING,30,40,40,40,90,50,30,20,100,
  TAR_OBJ_INV | TAR_OBJ_EQUIP, cast);
 SPELLO(91,12,POSITION_STANDING,40,34,40,40,90,90,30,20,100,
  TAR_OBJ_INV | TAR_OBJ_EQUIP, cast);
 SPELLO(92,12,POSITION_STANDING,5,7,40,40,95,95,10,10,5,
  TAR_CHAR_ROOM, cast);
 SPELLO(93,12,POSITION_STANDING,40,25,40,40,40,95,25,10,50,
  TAR_OBJ_INV | TAR_OBJ_EQUIP, cast);
 SPELLO(94,12,POSITION_STANDING,40,33,40,40,40,95,25,10,70,
  TAR_OBJ_INV | TAR_OBJ_EQUIP, cast);
 SPELLO(95,12,POSITION_FIGHTING,30,40,40,40,95,30,30,30,20,
  TAR_CHAR_ROOM | TAR_FIGHT_VICT, cast);
 SPELLO(96,12,POSITION_FIGHTING,33,40,40,40,95,30,30,30,50,
  TAR_CHAR_ROOM | TAR_FIGHT_VICT, cast);
 SPELLO(97,12,POSITION_STANDING,20,24,40,40,95,95,30,30,10,
  TAR_IGNORE, cast);
 SPELLO(98,12,POSITION_STANDING,35,40,40,40,95,95,35,20,70,
  TAR_SELF_ONLY, cast);
 SPELLO(99,12,POSITION_FIGHTING,40,35,40,40,90,95,50,50,33,
  TAR_IGNORE, cast);
 SPELLO(100,12,POSITION_FIGHTING, 4, 1,40,40,50,95,45,45,15,
  TAR_CHAR_ROOM, cast);
 SPELLO(101,12,POSITION_FIGHTING,16, 9,40,40,60,95,45,50,15,
  TAR_CHAR_ROOM, cast);
}

