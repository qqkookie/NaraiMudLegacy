/* ************************************************************************
*  file: act.offensive.c , Implementation of commands.    Part of DIKUMUD *
*  Usage : Offensive commands.                                            *
*  Copyright (C) 1990, 1991 - see 'license.doc' for complete information. *
************************************************************************* */

#include <stdio.h>
#include <string.h>

#include "structs.h"
#include "utils.h"
#include "comm.h"
#include "interpreter.h"
#include "handler.h"
#include "db.h"
#include "spells.h"
#include "limits.h"

/* extern variables */

extern struct room_data *world;
extern struct descriptor_data *descriptor_list;
extern struct char_data *character_list ;
extern int top_of_zone_table;
extern struct zone_data *zone_table;

extern void load_zones(int i);
void raw_kill(struct char_data *ch);
void hit(struct char_data *ch, struct char_data *victim, int type);
void log(char *str);


void do_assist(struct char_data *ch, char *argument, int cmd)
{
  char arg[MAX_STRING_LENGTH];
  struct char_data *victim;
  extern int nokillflag;
  char cyb[80];

  one_argument(argument, arg);

  if (*arg) {
    victim = get_char_room_vis(ch, arg);
	if(!victim&&strcmp(arg,"master")==0)
	{
		if(ch->master)
			victim=ch->master;
	}
    if (victim) {
      if (victim == ch) {
        send_to_char("Can you assist your self?\n\r", ch);
        act("$n tries to help $mself, and says YAHA!", FALSE, ch, 0, victim, TO_ROOM);
      } else {

        if(nokillflag) {
          if((!IS_NPC(ch)) && (IS_NPC(victim))) {
            send_to_char("You can't help MOB.\n\r", ch);
	    return;
	    }
          }

        /* block indirect player killing */
        if (IS_AFFECTED(ch, AFF_CHARM) && (IS_NPC(victim))) return;
        if ((GET_POS(ch)==POSITION_STANDING) &&
            (victim != ch->specials.fighting)) {
	  if (!(victim->specials.fighting)) {
	    send_to_char("Who do you think fight ?\n\r", ch);
	    return ;
	    }
	  sprintf(cyb, "%s assist you !!\n\r", GET_NAME(ch));
          send_to_char(cyb, victim);
	  sprintf(cyb, "You assist %s !!\n\r", GET_NAME(victim));
          send_to_char(cyb, ch);
          act("$n ASSIST $N !", FALSE, ch, 0, victim, TO_NOTVICT);
          hit(ch, victim->specials.fighting, TYPE_UNDEFINED);
          if(!IS_NPC(ch))
			WAIT_STATE(ch, PULSE_VIOLENCE+2); /* HVORFOR DET?? */
        } else {
          send_to_char("You do the best you can!\n\r",ch);
        }
      }
    } else {
      send_to_char("They aren't here.\n\r", ch);
    }
  } else {
    send_to_char("Assist who?\n\r", ch);
  }
}

void stop_group(struct char_data *ch)
{
 struct follow_type *j, *k;

 if(!ch->master) {
   log("No master but called stop_group");
   return ;
   }

 act("You stop following $N.", FALSE, ch, 0, ch->master, TO_CHAR);
 act("$n stops following $N.", FALSE, ch, 0, ch->master, TO_NOTVICT);
 act("$n stops following you.", FALSE, ch, 0, ch->master, TO_VICT);

 if (ch->master->followers->follower == ch) { /* Head of follower-list? */
  k = ch->master->followers;
  ch->master->followers = k->next;
  free(k);
 } else { /* locate follower who is not head of list */
  for(k = ch->master->followers; k->next->follower!=ch; k=k->next)  ;

  j = k->next;
  k->next = j->next;
  free(j);
 }

 ch->master = 0;
 REMOVE_BIT(ch->specials.affected_by, AFF_GROUP);
}



/* Called when a character that follows/is followed dies */
void do_ungroup(struct char_data *ch, char *argument, int cmd)
{
 struct follow_type *j, *k;

 if (ch->master)
  stop_group(ch);

 for (k=ch->followers; k; k=j) {
  j = k->next;
  stop_group(k->follower);
 }
 REMOVE_BIT(ch->specials.affected_by, AFF_GROUP);
 send_to_char("Your group is dismissed.\n\r", ch);
}

void do_version(struct char_data *ch, char *argument, int cmd)
{
 extern char fileid[] ;
 send_to_char(fileid, ch);
}

/* by atre */
#ifdef RESET_ZONE_BY_WIZARD

void do_resetzone(struct char_data *ch, char *argument, int cmd)
{
	char	buf[255],buf2[255];
	int		i;
	if (!ch) return;
	if ( GET_LEVEL(ch) < (IMO+3) )
		return;
	if (!argument || !*argument)
	{
		send_to_char("Which zone do you want to reset?\n\r",ch);
		return;
	}
	while( isspace(*argument) ) argument++;
	if ( strcmp(argument,"ALL") == 0 )
	{
		for ( i=0; i <= top_of_zone_table; i++ )
		{
			sprintf(buf2,"Zone %s is now updating..\n\r",
				zone_table[i].name);
			send_to_char(buf2,ch);
			reset_zone(i);
		}
		return;
	}

	if ( strcmp(argument,"THIS") == 0 )
	{
		i = world[ch->in_room].zone;
		sprintf(buf2,"Zone %s is now updating..\n\r",
			zone_table[i].name);
		send_to_char(buf2,ch);
		reset_zone(i);
		return;
	}

	for ( i=0; i <= top_of_zone_table; i++ )
		if ( strncmp(zone_table[i].name,argument,strlen(argument)) == 0 )
		{
			sprintf(buf2,"Zone %s is now updating..\n\r",
				zone_table[i].name);
			send_to_char(buf2,ch);
			reset_zone(i);
			break;
		}
}

void do_reloadzone(struct char_data *ch, char *argument, int cmd)
{
	char	buf[255],buf2[255];
	int		i;
	if (!ch) return;
	if ( GET_LEVEL(ch) < (IMO+3) )
		return;
	if (!argument || !*argument)
	{
		send_to_char("Which zone do you want to reset?\n\r",ch);
		return;
	}
	while( isspace(*argument) ) argument++;
	if ( strcmp(argument,"ALL") == 0 )
	{
		for ( i=0; i <= top_of_zone_table; i++ )
		{
			sprintf(buf2,"Zone %s is now reloading..\n\r",
				zone_table[i].name);
			send_to_char(buf2,ch);
			log(buf2);
			load_zones(i);
		}
		return;
	}

	if ( strcmp(argument,"THIS") == 0 )
	{
		i = world[ch->in_room].zone;
		sprintf(buf2,"Zone %s is now reloading..\n\r",
			zone_table[i].name);
		send_to_char(buf2,ch);
		log(buf2);
		load_zones(i);
		return;
	}

	for ( i=0; i <= top_of_zone_table; i++ )
		if ( strncmp(zone_table[i].name,argument,strlen(argument)) == 0 )
		{
			sprintf(buf2,"Zone %s is now reloading..\n\r",
				zone_table[i].name);
			send_to_char(buf2,ch);
			log(buf2);
			load_zones(i);
			break;
		}
}

#endif /* RESET_ZONE_BY_WIZARD */
