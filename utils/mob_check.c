/**************************************************************************
*  file: db.c , Database module.                          Part of DIKUMUD *
*  Usage: Loading/Saving chars, booting world, resetting etc.             *
*  Copyright (C) 1990, 1991 - see 'license.doc' for complete information. *
***************************************************************************/

#include <stdio.h>

#include "structs.h"
#include "utils.h"

/* local procedures */
char *fread_string(FILE *fl) ;
void clear_char(struct char_data *mob) ;

/* local variables */
int last_number = 0 ;

/* body of the booting system */
main(int argc, char *argv[])
{
	FILE	*mob_f ;
	extern int wearing, item_types ;

	if ( argc < 2 ) {
		fprintf(stderr, "Usage: %s mob_file\n", argv[0]) ;
		exit(1) ;
		}

	if (!(mob_f = fopen(argv[1], "r"))) {
		perror("main:");
		exit(1);
		}

	check_mobile(mob_f);
      
}

check_mob_print(struct char_data *mob)
{
	static int	number = 0 ;
	int	ave_damroll, d1, d2, d3 ;
	int	sanc, attack_freq, dat, i ;

	d1 = mob->specials.damnodice ;
	d2 = mob->specials.damsizedice ;
	d3 = mob->points.damroll ;
	ave_damroll = d1 * (d2 + 1) / 2 + d3 ;

	if (!IS_SET(mob->specials.act, ACT_CLERIC) &&
		!IS_SET(mob->specials.act, ACT_MAGE))
		dat = 7 ;
	else
		dat = 15 ;
	attack_freq = 1 ;
	for ( i = dat ; i < GET_LEVEL(mob) ; i += dat )
		attack_freq ++ ;
	if (IS_AFFECTED(mob, AFF_HASTE)) {
		attack_freq ++ ;
		}
	else if (IS_AFFECTED(mob, AFF_IMPROVED_HASTE)) {
		attack_freq += 2 ;
		}

	if (IS_AFFECTED(mob, AFF_SANCTUARY))
		sanc = 2 ;
	else
		sanc = 1 ;
	printf("%2d %5d %4d %3d %3d %8d %8d %d %2d %s\n",
		GET_LEVEL(mob), GET_HIT(mob),
		GET_AC(mob), GET_HITROLL(mob), ave_damroll,
		GET_EXP(mob), GET_GOLD(mob), sanc, attack_freq,
		GET_NAME(mob)) ;
}

check_mobile(FILE *mob_f)
{
	char	line[BUFSIZ], buf[BUFSIZ], chk[BUFSIZ], *status ;
	struct char_data *mob;
	int	i, tmp, tmp2, tmp3 ;
	int	flag ;
	char	letter ;
	extern int	last_number ;

	flag = 0 ;
	mob = (struct char_data *) malloc(sizeof(struct char_data));
	clear_char(mob);
	status = fgets(buf, 81, mob_f) ;
	for (;;) {
		if (flag) {
			check_mob_print(mob) ;
			}
		if (status) {
			if (*buf == '#') {
				flag = 1 ;
				clear_char(mob);
				sscanf(buf, "#%d", &last_number) ;

				/***** String data *** */
				mob->player.name = fread_string(mob_f);
				mob->player.short_descr = fread_string(mob_f);
				mob->player.long_descr = fread_string(mob_f);
				mob->player.description = fread_string(mob_f);
				mob->player.title = 0;

				/* *** Numeric data *** */
				fscanf(mob_f, "%d ", &tmp);
				mob->specials.act = tmp;
				/* SET_BIT(mob->specials.act, ACT_ISNPC); */

				fscanf(mob_f, " %d ", &tmp);
				mob->specials.affected_by = tmp;

				fscanf(mob_f, " %d ", &tmp);
				mob->specials.alignment = tmp;

				fscanf(mob_f, " %c \n", &letter); /* it is S */

				/* The new easy monsters */
				mob->abilities.str   = 18;
				mob->abilities.intel = 11; 
				mob->abilities.wis   = 11;
				mob->abilities.dex   = 18;
				mob->abilities.con   = 11;

				fscanf(mob_f, " %D ", &tmp);
				GET_LEVEL(mob) = tmp;

				fscanf(mob_f, " %D ", &tmp);
				mob->points.hitroll = 20-tmp;

				fscanf(mob_f, " %D ", &tmp);
				mob->points.armor = 10*tmp;

				fscanf(mob_f, " %Dd%D+%D ", &tmp, &tmp2, &tmp3);
				mob->points.max_hit = tmp*(tmp2+1)/2 + tmp3;
				mob->points.hit = mob->points.max_hit;

				fscanf(mob_f, " %Dd%D+%D \n", &tmp, &tmp2, &tmp3);
				mob->points.damroll = tmp3;
				mob->specials.damnodice = tmp;
				mob->specials.damsizedice = tmp2;
				mob->points.mana = 10 + GET_LEVEL(mob)*3;
				mob->points.max_mana = 10 + GET_LEVEL(mob)*3;
				mob->points.move = 100 + GET_LEVEL(mob)*4 ;
				mob->points.max_move = 100 + GET_LEVEL(mob)*4 ;
				fscanf(mob_f, " %D ", &tmp);
				mob->points.gold = tmp;
				fscanf(mob_f, " %D \n", &tmp);
				GET_EXP(mob) = tmp;
				fscanf(mob_f, " %D ", &tmp);
				mob->specials.position = tmp;
				fscanf(mob_f, " %D ", &tmp);
				mob->specials.default_pos = tmp;
				fscanf(mob_f, " %D \n", &tmp);
				mob->player.sex = tmp;

			/* need not
				mob->player.class = 0;
				mob->player.time.birth = 0;
				mob->player.time.played  = 0;
				mob->player.time.logon  = 0;
				mob->player.weight = 200;
				mob->player.height = 198;
			*/

				mob->tmpabilities = mob->abilities;

				status = fgets(line, 81, mob_f) ;
				if (*line == '#') {
					strcpy(buf, line);
					continue ;
					}
				else { /* ERROR? */
					fprintf(stderr, "Last mob: %d\n", last_number) ;
					exit(0);
					}
				}
			else if (*buf == '$')  /* EOF */
  				break;
			}
		else {
			fprintf(stderr, "Last item: %d\n", last_number) ;
			exit(0);
    		}
  		}
}

/* clear ALL the working variables of a char and do NOT free any space alloc'ed*/
void clear_char(struct char_data *ch)
{
  bzero(ch, sizeof(struct char_data));

  ch->in_room = NOWHERE;
  ch->specials.was_in_room = NOWHERE;
  ch->specials.position = POSITION_STANDING;
  ch->specials.default_pos = POSITION_STANDING;
  GET_AC(ch) = 100; /* Basic Armor */
}

/* read and allocate space for a '~'-terminated string from a given file */
char *fread_string(FILE *fl)
{
  char buf[MAX_STRING_LENGTH], tmp[500];
  char *rslt;
  register char *point;
  int flag;
	extern int	last_number ;

  bzero(buf, MAX_STRING_LENGTH);
  do
  {
    if (!fgets(tmp, MAX_STRING_LENGTH, fl))
    {
		fprintf(stderr, "Last item: %d\n", last_number) ;
      perror("fread_str");
      exit(0);
    }
    if (strlen(tmp) + strlen(buf) > MAX_STRING_LENGTH)
    {
      fprintf(stderr, "fread_string: string too large.\n");
		fprintf(stderr, "Last item: %d\n", last_number) ;
      exit(1);
    }
    else
      strcat(buf, tmp);

    for (point = buf + strlen(buf) - 2; point >= buf && isspace(*point);
      point--);    
    if (flag = (*point == '~'))
      if (*(buf + strlen(buf) - 3) == '\n')
      {
        *(buf + strlen(buf) - 2) = '\r';
        *(buf + strlen(buf) - 1) = '\0';
      }
      else
        *(buf + strlen(buf) -2) = '\0';
    else
    {
      *(buf + strlen(buf) + 1) = '\0';
      *(buf + strlen(buf)) = '\r';
    }
  }
  while (!flag);

  /* do the allocate boogie  */

  if (strlen(buf) > 0)
  {
	 rslt = (char *) malloc ( strlen(buf) + 1 ) ;
    strcpy(rslt, buf);
  }
  else
    rslt = 0;
  return(rslt);
}

