
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
#include "limit.h"

#include "guild_list.h"

/* extern variables */

extern struct room_data *world;
extern struct descriptor_data *descriptor_list;
extern struct char_data *character_list ;

void raw_kill(struct char_data *ch);
void hit(struct char_data *ch, struct char_data *victim, int type);
void damage(struct char_data *ch, struct char_data *victim, int dam, int type);
void set_fighting(struct char_data *ch, struct char_data *victim);
void stop_fighting(struct char_data *ch);
void do_say(struct char_data *ch, char *arg, int cmd);
void log(char *str);
bool saves_spell(struct char_data *ch, int type);
int str_cmp(char *arg1, char *arg2);
int do_simple_move(struct char_data *ch, int cmd, int following);
int number(int from, int to);
int MIN(int a, int b);
int MAX(int a, int b);
void die(struct char_data *ch, int level,struct char_data *v);
void first_attack(struct char_data *ch, struct char_data *victim);

void do_hit(struct char_data *ch, char *argument, int cmd)
{
  char arg[MAX_STRING_LENGTH];
  struct char_data *victim;
  extern int nokillflag;

  one_argument(argument, arg);

  if ((GET_LEVEL(ch) == IMO) && (!IS_NPC(ch))) { 
	  return;
	}
  if (*arg) {
    victim = get_char_room_vis(ch, arg);
    if (victim) {
      if (victim == ch) {
        send_to_char("You hit yourself..OUCH!.\n\r", ch);
        act("$n hits $mself, and says OUCH!", FALSE, ch, 0, victim, TO_ROOM);
      } else {

        if(nokillflag)
        if((!IS_NPC(ch)) && (!IS_NPC(victim))) return;
		if(ch->player.guild == victim->player.guild && (
			ch->player.guild != 0 || victim->player.guild != 0 ) ) /* forbid guild member hit */
		{
			send_to_char("You cannot hit him or her",ch);
			return;
		}
		/* forbid charmed mob hit player */
        if (ch&&IS_AFFECTED(ch, AFF_CHARM) && (!IS_NPC(victim))) return;
		/* forbid charmed mob hit charmed mob */
		if (ch&&victim&&IS_AFFECTED(ch, AFF_CHARM) && IS_AFFECTED(victim,AFF_CHARM))
		{
			return;
		}
        if ((GET_POS(ch)==POSITION_STANDING) &&
            (victim != ch->specials.fighting)) {
          hit(ch, victim, TYPE_UNDEFINED);
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
    send_to_char("Hit who?\n\r", ch);
  }
}

void do_kill(struct char_data *ch, char *argument, int cmd)
{
  static char arg[MAX_STRING_LENGTH];
  char buf[70];
  struct char_data *victim;

  if( GET_LEVEL(ch) >= IMO && GET_LEVEL(ch)<IMO+3 ){
    sprintf(buf,"Fight IMO:%s v.s. %s",GET_NAME(ch),argument);
    log(buf);
    return;
  }
  if ((GET_LEVEL(ch) < IMO) || IS_NPC(ch)) {
    do_hit(ch, argument, 0);
    return;
  }
  one_argument(argument, arg);
  if (!*arg) {
    send_to_char("Kill who?\n\r", ch);
  } else {
    if (!(victim = get_char_room_vis(ch, arg)))
       send_to_char("They aren't here.\n\r", ch);
    else
       if (ch == victim)
        send_to_char("Your mother would be so sad.. :(\n\r", ch);
       else {
        if(GET_LEVEL(ch)<IMO+3 && GET_LEVEL(victim)>=(IMO+3)) return;
        act("You chop $M to pieces! Ah! The blood!",
          FALSE, ch, 0, victim, TO_CHAR);
        act("$N chops you to pieces!", FALSE, victim, 0, ch, TO_CHAR);
        act("$n brutally slays $N", FALSE, ch, 0, victim, TO_NOTVICT);
        die(victim,GET_LEVEL(ch),ch);
      }
  }
}

void do_backstab(struct char_data *ch, char *argument, int cmd)
{
	struct char_data *victim;
	char name[256];
	byte percent;

	one_argument(argument, name);

    if ((GET_LEVEL(ch) == IMO) && (!IS_NPC(ch))) {
       return;
    }
	if (!(victim = get_char_room_vis(ch, name))) {
		send_to_char("Backstab who?\n\r", ch);
		return;
	}
	if (victim == ch) {
		send_to_char("How can you sneak up on yourself?\n\r", ch);
		return;
	}
	if(!IS_NPC(ch)){
	  if (!ch->equipment[WIELD]) {
		send_to_char("You need to wield a weapon, to make it a succes.\n\r",ch);
		return;
	  }
	  if (ch->equipment[WIELD]->obj_flags.value[3] != 11) {
		send_to_char("Only piercing weapons can be used for backstabbing.\n\r",
			ch);
		return;
	  }
	  if (ch->specials.fighting) {
		send_to_char("You can't backstab while fighting, too alert!\n\r", ch);
		return;
	  }
	}
	else {
	  if(GET_CLASS(ch) <= 20){
	    return;
	  }
	}
	percent = number(1,101) + (GET_LEVEL(victim) << 1) - GET_LEVEL(ch)
		- GET_DEX(ch); /* 101% is a complete failure */
	if (AWAKE(victim) && (percent > ch->skills[SKILL_BACKSTAB].learned))
		damage(ch, victim, 0, SKILL_BACKSTAB);
	else {
		INCREASE_SKILLED(ch, victim, SKILL_BACKSTAB); 
		hit(ch,victim,SKILL_BACKSTAB);
	}
}

void do_order(struct char_data *ch, char *argument, int cmd)
{
  char name[100], message[MAX_INPUT_LENGTH];
  char buf[MAX_OUTPUT_LENGTH];
  bool found = FALSE;
  int org_room;
  struct char_data *victim;
  struct follow_type *k;

  half_chop(argument, name, message);

  if (!*name || !*message) {
    send_to_char("Order who to do what?\n\r", ch);
    return ;
    }
  else if (!(victim = get_char_room_vis(ch, name)) &&
           str_cmp("follower", name) && str_cmp("followers", name))
      send_to_char("That person isn't here.\n\r", ch);
  else if (ch == victim)
    send_to_char("You decline to do it.\n\r", ch);
  else {
    if (ch&&IS_AFFECTED(ch, AFF_CHARM)) {
      send_to_char("Your superior would not approve.\n\r",ch);
      return;
    }
    if (victim) {
      sprintf(buf, "$N orders you to '%s'", message);
      act(buf, FALSE, victim, 0, ch, TO_CHAR);
      act("$n gives $N an order.", FALSE, ch, 0, victim, TO_ROOM);
      if ( (victim->master!=ch) || (victim&&!IS_AFFECTED(victim, AFF_CHARM)) )
        act("$n has an indifferent look.", FALSE, victim, 0, 0, TO_ROOM);
      else {
        send_to_char("Ok.\n\r", ch);
		if(!IS_NPC(victim))
			WAIT_STATE(victim,PULSE_VIOLENCE);
        command_interpreter(victim, message);
      }
    } else {  /* This is order "followers" */
      sprintf(buf, "$n issues the order '%s'.", message);
      act(buf, FALSE, ch, 0, victim, TO_ROOM);

      org_room = ch->in_room;

      for (k = ch->followers; k; k = k->next) {
        if (org_room == k->follower->in_room)
          if (k->follower&&IS_AFFECTED(k->follower, AFF_CHARM)) {
            found = TRUE;
			if(!IS_NPC(k->follower))
				WAIT_STATE(k->follower,PULSE_VIOLENCE);
            command_interpreter(k->follower, message);
          }
      }
      if (found)
        send_to_char("Ok.\n\r", ch);
      else
        send_to_char("Nobody here are loyal subjects of yours!\n\r", ch);
    }
  }
}

void do_flee(struct char_data *ch, char *argument, int cmd)
{
	int i, attempt, loose, die;
	struct char_data *tmp_victim, *temp;
	int level_dif;

	void gain_exp(struct char_data *ch, int gain);
	int special(struct char_data *ch, int cmd, char *arg);

	/* when not fighting */
	if (!(ch->specials.fighting)) {
		for(i=0; i<6; i++) {
			attempt = number(0, 5);  /* Select a random direction */
			if (CAN_GO(ch, attempt)) {
				act("$n panics, and attempts to flee.",
					TRUE, ch, 0, 0, TO_ROOM);
				if ((die = do_simple_move(ch, attempt, FALSE)) == 1) {
					/* The escape has succeded */
					send_to_char("You flee head over heels.\n\r", ch);
				}
				else {
					if (!die)
						act("$n tries to flee, but is too exhausted!",
							TRUE, ch, 0, 0, TO_ROOM);
					return;
				}
			}
		} /* for */
		/* No exits was found */
		send_to_char("PANIC! You couldn't escape!\n\r", ch);
		return;
	}

	if(GET_CLASS(ch) == CLASS_THIEF){
		i = 7;
	}
	else {
		i = 5;
	}

	for (;i > 0; i --) {
		attempt = number(0, 5);  /* Select a random direction */
		if (CAN_GO(ch, attempt)) {
			act("$n panics, and attempts to flee.", TRUE, ch, 0, 0, TO_ROOM);
			if ((die = do_simple_move(ch, attempt, FALSE))== 1) { 
				/* The escape has succeded */
				loose = (GET_PLAYER_MAX_HIT(ch->specials.fighting)
					- GET_HIT(ch->specials.fighting)) * 100 /
					GET_PLAYER_MAX_HIT(ch->specials.fighting);
				loose *= GET_LEVEL(ch->specials.fighting);
				level_dif = GET_LEVEL(ch->specials.fighting) - GET_LEVEL(ch);
				if (level_dif <= 0)
					level_dif = 1;
				loose *= level_dif;
				loose *= level_dif;
				if (loose < 0)
					loose = -loose;
				loose = MIN(1000000, loose);
				if (!IS_NPC(ch))
					gain_exp(ch, -loose);
				send_to_char("You flee head over heels.\n\r", ch);
				/* Insert later when using huntig system        */
				ch->specials.fighting->specials.hunting = ch;

				/* stop fighting who was fighting against ch */
				for (tmp_victim = character_list; tmp_victim;
					tmp_victim = temp) {
					temp = tmp_victim->next;
					if (tmp_victim->specials.fighting == ch)
						stop_fighting(tmp_victim);
				}
				if (ch->specials.fighting->specials.fighting == ch)
					stop_fighting(ch->specials.fighting);
				stop_fighting(ch);
				return;
			}
			else {
				if (!die)
					act("$n tries to flee, but is too exhausted!",
						TRUE, ch, 0, 0, TO_ROOM);
				return;
			}
		}
	} /* for */

	/* No exits was found */
	send_to_char("PANIC! You couldn't escape!\n\r", ch);
}

void do_bash(struct char_data *ch, char *argument, int cmd)
{
	struct char_data *victim;
	char name[256];
	int percent;
	int dam;

	one_argument(argument, name);

    if ((GET_LEVEL(ch) == IMO) && (!IS_NPC(ch))) {
       return;
    }
	if (!(victim = get_char_room_vis(ch, name))) {
		if (ch->specials.fighting) {
			victim = ch->specials.fighting;
		}
		else {
			send_to_char("Bash who?\n\r", ch);
			return;
		}
	}

	if (victim == ch) {
		send_to_char("Aren't we funny today...\n\r", ch);
		return;
	}
	if (!IS_NPC(ch) && !ch->equipment[WIELD]) {
		send_to_char("You need to wield a weapon to bash.\n\r",ch);
		return;
	}
	percent = number(1,101) + GET_LEVEL(victim) - GET_LEVEL(ch);
	if (percent > ch->skills[SKILL_BASH].learned) {
		damage(ch, victim, 0, SKILL_BASH);
		GET_POS(ch) = POSITION_SITTING;
	}
	else {
		INCREASE_SKILLED(ch, victim, SKILL_BASH);
		WAIT_STATE(victim, PULSE_VIOLENCE * (2 + (GET_SKILLED(ch, SKILL_BASH) >> 5)));
		dam = GET_LEVEL(ch) * (10 + GET_SKILLED(ch, SKILL_BASH));
		damage(ch, victim, dam, SKILL_BASH);

		percent = number(1, IMO << 1);
		if (percent < GET_LEVEL(ch))
			GET_POS(victim) = POSITION_STUNNED;
		else
			GET_POS(victim) = POSITION_SITTING;
	}
	WAIT_STATE(ch, PULSE_VIOLENCE * 2);
}

void do_rescue(struct char_data *ch, char *argument, int cmd)
{
	struct char_data *victim, *tmp_ch;
	int percent;
	char victim_name[240];

	one_argument(argument, victim_name);

	if (!(victim = get_char_room_vis(ch, victim_name))) {
		send_to_char("Who do you want to rescue?\n\r", ch);
		return;
	}
	if (victim == ch) {
		send_to_char("What about fleeing instead?\n\r", ch);
		return;
	}
  
	if (ch->specials.fighting == victim) {
		send_to_char("How can you rescue your foe?\n\r",ch);
		return;
	}
	for (tmp_ch=world[ch->in_room].people; tmp_ch &&
		(tmp_ch->specials.fighting != victim); tmp_ch=tmp_ch->next_in_room) ;
	if (!tmp_ch) {
		act("But nobody is fighting $M?", FALSE, ch, 0, victim, TO_CHAR);
		return;
	}

	/* check same guild member, jhpark 	*/
	if (victim->specials.fighting)
	if ((!IS_NPC(victim->specials.fighting) && !IS_NPC(ch)) && (victim->specials.fighting->player.guild == ch->player.guild)){
		act("You cannot rescue $M!", FALSE, ch, 0, victim, TO_CHAR);
		return;
	}

	percent = number(1,111) - (GET_SKILLED(ch, SKILL_RESCUE) >> 3);
	if (percent > ch->skills[SKILL_RESCUE].learned) {
		send_to_char("You fail the rescue.\n\r", ch);
		return;
	}

	INCREASE_SKILLED(ch, victim, SKILL_RESCUE);
	send_to_char("Yaho! To the rescue...\n\r", ch);
	act("You are rescued by $N, you are confused!",
		FALSE, victim, 0, ch, TO_CHAR);
	act("$n heroically rescues $N.", FALSE, ch, 0, victim, TO_NOTVICT);
	WAIT_STATE(victim, 2 * PULSE_VIOLENCE);
	if (victim->specials.fighting == tmp_ch)
		stop_fighting(victim);
	if (tmp_ch->specials.fighting)
		stop_fighting(tmp_ch);
	if (ch->specials.fighting)
		stop_fighting(ch);
	if (!ch->specials.fighting && !tmp_ch->specials.fighting) {
		set_fighting(ch, tmp_ch);
		set_fighting(tmp_ch, ch);
	}
}

void do_multi_kick(struct char_data *ch, char *argument, int cmd)
{
	struct char_data *victim;
	int percent;
	int i;
	int dam;
	int succeed = 0;

	victim = ch->specials.fighting;
	if (!victim) {
		send_to_char("This skill can be used only in fighting!\n\r",ch);
		return;
	}
	if (GET_MOVE(ch) < 0) {
		send_to_char("You are too exhausted!\n\r",ch);
		return;
	}
	if (victim == ch) {
		send_to_char("Aren't we funny today...\n\r", ch);
		return;
	}
	GET_MOVE(ch) -= ((IMO << 1) - GET_LEVEL(ch));
	WAIT_STATE(ch, PULSE_VIOLENCE);

	dam = GET_LEVEL(ch) * (1 + (GET_SKILLED(ch, SKILL_KICK) >> 2));
	dam = number(dam, dam << 1);
	if (saves_spell(victim, SAVING_HIT_SKILL))
		dam >>= 1;

	INCREASE_SKILLED(ch, victim, SKILL_MULTI_KICK);

	i = 2 + (GET_LEVEL(ch)>>3) + (GET_SKILLED(ch,SKILL_MULTI_KICK)>>4);
	for(; i; i --){
		percent = ((200-GET_AC(victim)-GET_HITROLL(ch)) >> 5) + number(1, 101);
		if (percent > GET_LEARNED(ch, SKILL_MULTI_KICK)){
			damage(ch, victim, 0, SKILL_KICK);
		}
		else {
			succeed = 1;
			damage(ch, victim, dam, SKILL_KICK);
		}
	}
	if (succeed)
		WAIT_STATE(victim, PULSE_VIOLENCE);
}

void do_kick(struct char_data *ch, char *argument, int cmd)
{
	struct char_data *victim;
	char name[256];
	int percent;
	int dam;

	one_argument(argument, name);

    if ((GET_LEVEL(ch) == IMO) && (!IS_NPC(ch))) {
		return;
    }

	if (!(victim = get_char_room_vis(ch, name))) {
		if (ch->specials.fighting) {
			victim = ch->specials.fighting;
		}
		else {
			send_to_char("Kick who?\n\r", ch);
			return;
		}
	}

	if (victim == ch) {
		send_to_char("Aren't we funny today...\n\r", ch);
		return;
	}

	percent = ((200-GET_AC(victim)-GET_HITROLL(ch)) >> 5) + number(1, 101);
	WAIT_STATE(ch, PULSE_VIOLENCE);
	if (percent > ch->skills[SKILL_KICK].learned) {
		damage(ch, victim, 0, SKILL_KICK);
	}
	else {
		INCREASE_SKILLED(ch, victim, SKILL_KICK);
		WAIT_STATE(victim, PULSE_VIOLENCE);
		dam = GET_LEVEL(ch) * (1 + (GET_SKILLED(ch, SKILL_KICK) >> 2));
		dam = number(dam, dam << 1);
		if (saves_spell(victim, SAVING_HIT_SKILL))
			dam >>= 1;
		damage(ch, victim, dam, SKILL_KICK);
	}
}

/* Perhaps made */
void do_punch(struct char_data *ch, char *argument, int cmd)
{
	struct char_data *victim;
	char name[256];
	int dam;
	int percent;

	one_argument(argument, name);

    if ((GET_LEVEL(ch) == IMO) && (!IS_NPC(ch))) {
       return;
    }
	if (!(victim = get_char_room_vis(ch, name))) {
		if (ch->specials.fighting) {
			victim = ch->specials.fighting;
		}
		else {
			send_to_char("punch who?\n\r", ch);
			return;
		}
	}
	if (victim == ch) {
		send_to_char("Aren't we funny today...\n\r", ch);
		return;
	}
	if (GET_MANA(ch)<200) {
		send_to_char("You do not have enough mana!!!\n\r",ch);
		return;
	}
	if(!IS_NPC(ch) && ch->equipment[WIELD]) {
		send_to_char("You must unwield to make it success!!!\n\r",ch);
		return;
	}
	dam = ((GET_LEARNED(ch, SKILL_PUNCH)>>1)+(GET_SKILLED(ch, SKILL_PUNCH)<<1))
		 * number(GET_LEVEL(ch), GET_LEVEL(ch) << 1);

	percent = ((300-GET_AC(victim)-GET_HITROLL(ch)-GET_SKILLED(ch, SKILL_PUNCH))>>4) + number(1, 101);
	WAIT_STATE(ch, PULSE_VIOLENCE);
	if (percent > ch->skills[SKILL_PUNCH].learned ) {
		// NOTE: No skill gain on failure
		// INCREASE_SKILLED(ch, victim, SKILL_PUNCH);
		send_to_char("You failed to punch him WHAT a DAMN!!!!!\n\r",ch);
		act("$n failed to punch down $N!!!",TRUE,ch,0,victim,TO_ROOM);
		return;
	}
	else {
		INCREASE_SKILLED(ch, victim, SKILL_PUNCH);
		send_to_char("You send a big punch ORURURURURURURURURU!!!!!!\n\r",ch);
		act("$n sends a POWERFUL PUNCH!!! ORURURURURURURURU!!!!!",
				TRUE,ch,0,0,TO_ROOM);
		damage(ch,victim,dam,SKILL_PUNCH);
	}
	GET_MANA(ch)-=200;
}

void do_tornado(struct char_data *ch, char *argument, int cmd)
{
  struct char_data *tch, *tch2;
  if ((GET_LEVEL(ch) == IMO) && (!IS_NPC(ch))) {
     return;
  }
  for (tch=world[ch->in_room].people; tch; tch = tch2) {
    tch2 = tch->next_in_room;
    if ((IS_NPC(ch) ^ IS_NPC(tch))&& ch->points.move>0) {
   	  /*
   	  if(ch->skills[SKILL_TORNADO].learned>number(1,99)) {
   	  */
   	  if(GET_LEARNED(ch, SKILL_TORNADO) + (GET_SKILLED(ch, SKILL_TORNADO) >> 3)
   	  	   > number(1, 111)){
    	hit(ch,tch,TYPE_UNDEFINED); 
  	  }
    }
  }
	if(ch->points.move>0) ch->points.move-=( IMO-GET_LEVEL(ch)+2 );
  INCREASE_SKILLED(ch, ch, SKILL_TORNADO);
  if(!IS_NPC(ch))	
	WAIT_STATE(ch, PULSE_VIOLENCE*1);
}

void do_light_move(struct char_data *ch, char *argument, int cmd)
{
	struct char_data *victim;

	WAIT_STATE(ch,PULSE_VIOLENCE/3);
	if((victim=ch->specials.fighting)){
		if (ch->skills[SKILL_LIGHT_MOVE].learned>10 && ch->points.mana>0) {
			hit(ch,victim,TYPE_UNDEFINED);
			ch->points.mana -= (IMO << 1) - GET_LEVEL(ch);	
			if ((GET_LEVEL(ch) + 1) * number(2, 3) +
				ch->skills[SKILL_LIGHT_MOVE].learned / 2 >
				GET_LEVEL(victim) * number(2,3) + number(30, 40)) {
				INCREASE_SKILLED(ch, victim, SKILL_LIGHT_MOVE);
				if (ch->specials.fighting) 
					stop_fighting(ch); 
				if (victim->specials.fighting)
					stop_fighting(victim);
			}
		}
 	}
	else {
		send_to_char("this skill can used only in fighting.\n\r",ch);
	}
}

void do_flash(struct char_data *ch, char *argument, int cmd)
{
  struct char_data *victim;
  char name[256];
  int percent;
  int d;

  if(GET_LEVEL(ch) < 6) return;
  one_argument(argument, name);

  if ((GET_LEVEL(ch) == IMO) && (!IS_NPC(ch))) {
     return;
  }
  if (!(victim = get_char_room_vis(ch, name))) {
    if (ch->specials.fighting) {
      victim = ch->specials.fighting;
    } else {
      send_to_char("flash who?\n\r", ch);
      return;
    }
  }
  if (victim == ch) {
    send_to_char("you are foolish..\n\r", ch);
    return;
  }

  /*
  if (!IS_NPC(victim)&&!IS_NPC(ch)&&GET_LEVEL(ch)<IMO)
  {
	send_to_char("You can't use flash to player\n\r",ch);
	return;
  }
  */

  percent=number(1,101)+GET_LEVEL(victim)*2-GET_LEVEL(ch)
  			- (GET_SKILLED(ch, SKILL_FLASH) >> 2);

  if(!IS_NPC(ch))
	WAIT_STATE(ch, PULSE_VIOLENCE/2);

  if (percent > ch->skills[SKILL_FLASH].learned) {
  	send_to_char("You can't get chance ...\n\r", ch);
 	act("$n try flash attack $N, but failed."
			, FALSE, ch, 0, victim, TO_NOTVICT);
    damage(ch, victim, 0, SKILL_FLASH);
  } else {
  	INCREASE_SKILLED(ch, victim, SKILL_FLASH);

	d = GET_MOVE(ch) * (2 + (GET_SKILLED(ch, SKILL_FLASH) >> 5)) / 5;
	ch->points.move=0;

  /* messages */
  send_to_char("You attack like flash ...\n\r", ch);
  send_to_char("You can see star above your head ?!? , you are confused!",
		 victim );
  act("$n moves like flash and attack with flash power!!! ",
		FALSE, ch , 0 , victim , TO_NOTVICT);
  act("$n strike $N's head with circular spanning triple kick .", 
		FALSE, ch, 0, victim, TO_NOTVICT);
  act("$n throw $N into sun with double tumbling.", 
		FALSE, ch, 0, victim, TO_NOTVICT);
  act("$n jump kick down  $N's heart with making beautifle circle.", 
		FALSE, ch, 0, victim, TO_NOTVICT);
  act("$n punch $N's head line drive.", 
		FALSE, ch, 0, victim, TO_NOTVICT);
  damage(ch,victim,d,SKILL_FLASH);
  }
}
void shoot(struct char_data *ch, struct char_data *victim, int type)
{
  byte percent;
  int dam;

  if(victim == ch){
	if(!IS_NPC(ch))
      send_to_char("Shoot yourself? Nah...\n\r", ch);
    return;
  }
  if(!ch->equipment[HOLD]|| /* Perhaps modified */
	GET_ITEM_TYPE(ch->equipment[HOLD])!=ITEM_FIREWEAPON) {
    if(!IS_NPC(ch))
      send_to_char("To shoot, you need to HOLD a firing weapon.\n\r",ch);
    return;
  }
  if(ch->equipment[HOLD]->obj_flags.value[0] <= 0){
    if(!IS_NPC(ch))
      send_to_char("Oops.  Nothing to shoot.\n\r",ch);
    act("Hmmm.  $n fires an empty $p.",
      FALSE, ch, ch->equipment[HOLD],0,TO_ROOM);
    return;
  }
  if((GET_LEVEL(ch) < (IMO+3)) &&
    ( (ch&&!IS_NPC(ch)) || (ch&&IS_NPC(ch)&&IS_AFFECTED(ch,AFF_CHARM))) )
    ch->equipment[HOLD]->obj_flags.value[0]--;
  if((!IS_NPC(ch)) && (GET_LEVEL(ch)>=(IMO+3)))
    percent=0;
  else
    percent=number(1,101)+GET_LEVEL(victim)-GET_LEVEL(ch);
  if(percent < 60){
    act("Thwapp! You shoot $M with $p.",
      FALSE, ch, ch->equipment[HOLD],victim,TO_CHAR);
    act("Thwapp! You are shot by $n with $p.",
      FALSE, ch, ch->equipment[HOLD],victim,TO_VICT);
    act("Thwapp! $n shoots $N with $p.",
      FALSE, ch, ch->equipment[HOLD],victim,TO_NOTVICT);
    dam=ch->equipment[HOLD]->obj_flags.value[2];
	if((!IS_NPC(ch))&&(GET_GUILD(ch)==OUTLAW||GET_GUILD(ch)==POLICE))
		dam=dam*2;
	if(!IS_NPC(victim))
		WAIT_STATE(victim,PULSE_VIOLENCE*2);
    damage(ch, victim, dam, TYPE_SHOOT);
    if(GET_LEVEL(ch) < (IMO+3))
		if(!IS_NPC(ch))
			WAIT_STATE(ch, PULSE_VIOLENCE*2);
  } else {
    act("You try to shoot $M with $p, but miss.",
      FALSE, ch, ch->equipment[HOLD],victim,TO_CHAR);
    act("You are shot at by $n with $p, but missed.",
      FALSE, ch, ch->equipment[HOLD],victim,TO_VICT);
    act("$n tries to shoot $N with $p, but misses.",
      FALSE, ch, ch->equipment[HOLD],victim,TO_NOTVICT);
  }
}

void do_shoot(struct char_data *ch, char *argument, int cmd)
{
  struct char_data *victim;
  char name[256];

  one_argument(argument, name);
 /* if( strcmp(name,"")==0 ) send_to_char("Shoot whom?\n\r",ch); */
  if (!(victim = get_char_room_vis(ch, name))) {
    if (ch->specials.fighting) {
      victim = ch->specials.fighting;
    } else {
      send_to_char("Shoot whom?\n\r", ch);
      return;
    }
  }
  if((GET_LEVEL(victim) >= IMO)&&(!IS_NPC(victim))){
    if(!IS_NPC(ch))
      send_to_char("Shoot an immortal?  Never.\n\r",ch);
    return;
  }
  shoot(ch,victim,TYPE_SHOOT);
}
/* by process */
/*
ONLY for female
*/
void do_temptation(struct char_data *ch,char *argument,int cmd)
{
	struct char_data *victim;
	char name[100],message[256];
	int percent;

	half_chop(argument,name,message);
	if(IS_NPC(ch)||(!(GET_SEX(ch)==SEX_FEMALE)&&GET_LEVEL(ch)<IMO))
	{
		send_to_char("You cannot seduce anyone as you are not female.\n\r",ch);
		return;
	} else if(!*name||!*message)
	{ send_to_char("temptation who to do what????\n\r",ch);
		return;
	}
	else if(!(victim=get_char_room_vis(ch,name)))
	{ send_to_char("That person isn't here.\n\r",ch);
	}
	else if(ch==victim) {
		send_to_char("Are you babo????\n\r",ch);
	}
	else if(victim)
	{
		percent = number(1, 151)
			+ GET_CON(ch) - GET_INT(ch) - GET_LEVEL(ch)
			+ GET_INT(victim) - GET_CON(victim) + GET_LEVEL(victim)
			- (GET_SKILLED(ch, SKILL_TEMPTATION) >> 2);

		if((ch->skills[SKILL_TEMPTATION].learned > percent
			&& GET_SEX(victim)==SEX_MALE && GET_LEVEL(victim) < 34)){ 
			/*
			GET_LEVEL(victim)<GET_LEVEL(ch))) {
			*/
			INCREASE_SKILLED(ch, victim, SKILL_TEMPTATION);
			do_say(ch,"HOHOHOHOHOHO!!!!!!", 0);
			if(!IS_NPC(ch)) WAIT_STATE(ch,PULSE_VIOLENCE);
			if(!IS_NPC(victim)) WAIT_STATE(victim,PULSE_VIOLENCE);
			act("$n seduces $N successfully!!!",
			FALSE,ch,0,victim,TO_ROOM); command_interpreter(victim,message);
		}
		else
		{
			do_say(victim,"HAHAHAHAHAHA!!!!!", 0);
			if(!IS_NPC(ch)) WAIT_STATE(ch,2*PULSE_VIOLENCE);
			act("$n failed to seduce $N!!!!",
			FALSE,ch,0,victim,TO_ROOM); command_interpreter(ch,message);
			hit(victim,ch,TYPE_UNDEFINED);
			first_attack(victim, ch);
		}

		if(GET_MANA(ch)>200)
			GET_MANA(ch)-=200;
	}
}
