/* ************************************************************************
*  file: act.other.c , Implementation of commands.        Part of DIKUMUD *
*  Usage : Other commands.                                                *
*  Copyright (C) 1990, 1991 - see 'license.doc' for complete information. *
************************************************************************* */

#include <stdio.h>
#include <string.h>
#include <ctype.h>

#include "structs.h"
#include "utils.h"
#include "comm.h"
#include "interpreter.h"
#include "handler.h"
#include "db.h"
#include "spells.h"

#include "guild_list.h"

/* extern variables */

extern struct str_app_type str_app[];
extern struct room_data *world;
extern struct descriptor_data *descriptor_list;
extern struct room_data *world;
extern struct dex_skill_type dex_app_skill[];
extern struct spell_info_type spell_info[];


/* extern procedures */

void stash_char(struct char_data *ch, char *filename,int flag);
void wipe_stash(char *filename);
void hit(struct char_data *ch, struct char_data *victim, int type);
void do_shout(struct char_data *ch, char *argument, int cmd);
void log(char *str);
void close_socket(struct descriptor_data *d);
int number(int from, int to);
int str_cmp(char *arg1, char *arg2);
void page_string(struct descriptor_data *d, char *str, int keep);
void do_say(struct char_data *ch, char *str, int cmd);
bool saves_spell(struct char_data *ch, int type);
void damage(struct char_data *ch, struct char_data *victim, int dam, int type);
int MIN(int a, int b);


void do_quit(struct char_data *ch, char *argument, int cmd)
{
  void do_save(struct char_data *ch, char *argument, int cmd);
  void die(struct char_data *ch, int level,struct char_data *v);
  char cyb[70];

  if (IS_NPC(ch) || !ch->desc)
    return;
  if (GET_POS(ch) == POSITION_FIGHTING) {
    send_to_char("No way! You are fighting.\n\r", ch);
    return;
  }
  if (GET_POS(ch) < POSITION_STUNNED) {
    send_to_char("You die before your time!\n\r", ch);
    die(ch, GET_LEVEL(ch), NULL);   /* cyb :do not know who killed him */
    return;
  }
#ifdef	RETURN_TO_QUIT
  save_char(ch,world[ch->in_room].number);
#else
  save_char(ch,ch->in_room);
#endif

  act("Goodbye, friend.. Come back soon!", FALSE, ch, 0, 0, TO_CHAR);
  sprintf(cyb, "%s closed connect(quit)", GET_NAME(ch));
  log(cyb);
  if (ch->desc) close_socket(ch->desc);
}

void do_wimpy(struct char_data *ch, char *argument, int cmd)
{
  char time[256];
  int wimpyness;
  char buf[256];

  if (IS_NPC(ch))
    return;

  argument = one_argument(argument, time);

  if(time[0]){
    wimpyness = atoi(time);
    if(wimpyness < 0){
      wimpyness = 0;
    }

	if((ch->player.level >= (IMO -1)) && (ch->player.remortal >= 15)){
	    if(wimpyness > GET_MAX_HIT(ch) * 25 / 100){
     	send_to_char("All that you can do is fleeing?\n\r", ch);
   	 	}
    	else {
      	sprintf(buf, "Set your wimpy time to %d, now.\n\r", wimpyness);
      	send_to_char(buf, ch);
      	ch->specials.wimpyness = wimpyness;
    	}
    }
    else{
	    if(wimpyness > GET_MAX_HIT(ch) * 6 / 10){
     	send_to_char("All that you can do is fleeing?\n\r", ch);
   	 	}
    	else {
      	sprintf(buf, "Set your wimpy time to %d, now.\n\r", wimpyness);
      	send_to_char(buf, ch);
      	ch->specials.wimpyness = wimpyness;
    	}
    }
  }
  else {
    if (IS_SET(ch->specials.act, PLR_WIMPY)) {
      send_to_char("You are not wimpy now.\n\r", ch);
      REMOVE_BIT(ch->specials.act, PLR_WIMPY);
    } else {
      send_to_char("You are wimpy now.\n\r", ch);
      SET_BIT(ch->specials.act, PLR_WIMPY);
    }
  }
}

void do_save(struct char_data *ch, char *argument, int cmd)
{
  char buf[100];

  if (IS_NPC(ch) || !ch->desc)
    return;
  sprintf(buf, "Saving %s.\n\r", GET_NAME(ch));
  send_to_char(buf, ch);
#ifdef  RETURN_TO_QUIT  
  save_char(ch,world[ch->in_room].number);
#else
  save_char(ch,ch->in_room);
#endif
  stash_char(ch,0,0);
}

void do_not_here(struct char_data *ch, char *argument, int cmd)
{
  send_to_char("Sorry, but you cannot do that here!\n\r",ch);
}

void do_sneak(struct char_data *ch, char *argument, int cmd)
{
  struct affected_type af;
  byte percent;

  send_to_char("Ok, you'll try to move silently for a while.\n\r", ch);
  if (ch&&IS_AFFECTED(ch, AFF_SNEAK))
    affect_from_char(ch, SKILL_SNEAK);
  percent=number(1,101); /* 101% is a complete failure */
  if (percent > ch->skills[SKILL_SNEAK].learned +
      dex_app_skill[GET_DEX(ch)].sneak)
    return;
  INCREASE_SKILLED(ch, ch, SKILL_SNEAK);
  af.type = SKILL_SNEAK;
  af.duration = GET_LEVEL(ch);
  af.modifier = 0;
  af.location = APPLY_NONE;
  af.bitvector = AFF_SNEAK;
  affect_to_char(ch, &af);
}

void do_hide(struct char_data *ch, char *argument, int cmd)
{
  byte percent;

  send_to_char("You attempt to hide yourself.\n\r", ch);
  if (ch&&IS_AFFECTED(ch, AFF_HIDE))
    REMOVE_BIT(ch->specials.affected_by, AFF_HIDE);
  percent=number(1,101); /* 101% is a complete failure */
  if (percent > ch->skills[SKILL_HIDE].learned +
      dex_app_skill[GET_DEX(ch)].hide)
    return;
  INCREASE_SKILLED(ch, ch, SKILL_HIDE);
  SET_BIT(ch->specials.affected_by, AFF_HIDE);
}

void do_steal(struct char_data *ch, char *argument, int cmd)
{
	struct char_data *victim;
	struct obj_data *obj;
	char victim_name[240];
	char obj_name[240];
	char buf[240];
	int percent;
	int gold, eq_pos;
	bool ohoh = FALSE;
	extern int nostealflag;

	argument = one_argument(argument, obj_name);
	one_argument(argument, victim_name);

	if (!(victim = get_char_room_vis(ch, victim_name))) {
		send_to_char("Steal what from who?\n\r", ch);
		return;
	}
	else if (victim == ch) {
		send_to_char("Come on now, that's rather stupid!\n\r", ch);
		return;
	}
	if ((!IS_NPC(victim)) && (nostealflag)) {
		act("Oops..", FALSE, ch,0,0,TO_CHAR);
		act("$n tried to steal something from you!",FALSE,ch,0,victim,TO_VICT);
		act("$n tries to rob $N.", TRUE, ch, 0, victim, TO_NOTVICT);
		return;
	}
	if(!IS_NPC(victim) && !(victim->desc)) {
		send_to_char("Steal something from the ghost? No way!\n\r", ch);
		return;
	}
	/* 101% is a complete failure */
	percent = number(1, 101) + 2 * GET_LEVEL(victim) - GET_LEVEL(ch)
		- dex_app_skill[GET_DEX(ch)].p_pocket
		- (GET_SKILLED(ch, SKILL_STEAL) >> 3);

	if (!IS_NPC(victim) &&
		(GET_LEVEL(victim) >= IMO) &&
		(GET_LEVEL(ch) < GET_LEVEL(victim)))
		percent = 101;
	if (str_cmp(obj_name, "coins") && str_cmp(obj_name,"gold")) {
		if (!(obj = get_obj_in_list_vis(victim, obj_name, victim->carrying))) {
			for (eq_pos = 0; (eq_pos < MAX_WEAR); eq_pos++)
				if (victim->equipment[eq_pos] &&
					(isname(obj_name, victim->equipment[eq_pos]->name)) &&
					CAN_SEE_OBJ(ch,victim->equipment[eq_pos])) {
					obj = victim->equipment[eq_pos];
					break;
				}
			if (!obj) {
				act("$E has not got that item.",FALSE,ch,0,victim,TO_CHAR);
				return;
			}
			else { /* It is equipment */
				send_to_char("Steal the equipment? Impossible!\n\r", ch);
				return;
			}
		}
		else {  /* obj found in inventory */
			percent += GET_OBJ_WEIGHT(obj); /* Make heavy harder */
			if (AWAKE(victim) && (percent > ch->skills[SKILL_STEAL].learned)) {
				ohoh = TRUE;
				act("Oops..", FALSE, ch,0,0,TO_CHAR);
				act("$n tries to rob $N.", TRUE, ch, 0, victim, TO_NOTVICT);
				if(!IS_NPC(victim))
					act("$n tried to rob you.",TRUE,ch,0,victim,TO_VICT);
			}
			else { /* Steal the item */
				if ((IS_CARRYING_N(ch) + 1 < CAN_CARRY_N(ch))) {
					if ((IS_CARRYING_W(ch) + GET_OBJ_WEIGHT(obj))
						< CAN_CARRY_W(ch)) {
						obj_from_char(obj);
						obj_to_char(obj, ch);
						send_to_char("Got it!\n\r", ch);
						INCREASE_SKILLED(ch, victim, SKILL_STEAL);
						if(!IS_NPC(victim))
							act("$n robbed you!",TRUE,ch,0,victim,TO_VICT);
					}
					else
						send_to_char("You cannot carry that much weight.\n\r",
							ch);
				}
				else
					send_to_char("You cannot carry that much.\n\r", ch);
			}
		}
	}
	else { /* Steal some coins */
		if (AWAKE(victim) && (percent > ch->skills[SKILL_STEAL].learned)) {
			ohoh = TRUE;
			act("Oops..", FALSE, ch,0,0,TO_CHAR);
			act("$n tries to steal gold from $N.",TRUE,ch,0,victim,TO_NOTVICT);
			if(!IS_NPC(victim))
				act("$n tried to rob you.",TRUE,ch,0,victim,TO_VICT);
		}
		else {
			/* Steal some gold coins */
			gold = (int) ((GET_GOLD(victim)*number(1,25))/100);
			if (gold > 0) {
				INCREASE_SKILLED(ch, victim, SKILL_STEAL);
				GET_GOLD(ch) += gold;
				GET_GOLD(victim) -= gold;
				sprintf(buf, "Bingo! You got %d gold coins.\n\r", gold);
				send_to_char(buf, ch);
				if(!IS_NPC(victim))
					act("$n robbed you.",TRUE,ch,0,victim,TO_VICT);
			}
			else {
				send_to_char("You couldn't get any gold...\n\r", ch);
			}
		}
	}
	if (ohoh && AWAKE(victim))
		if (!IS_SET(victim->specials.act, ACT_NICE_THIEF))
			hit(victim, ch, TYPE_UNDEFINED);
}

bool do_practice(struct char_data *ch, char *arg, int cmd)
{
	int i;
	extern char *spells[], *how_good();
	extern struct spell_info_type spell_info[MAX_SPL_LIST];
	char buf[100 * MAX_SKILLS];
	char tmp[MAX_STRING_LENGTH];
	char victim_name[256];
	struct char_data *victim;

	if (GET_LEVEL(ch) >= IMO) {
		arg = one_argument(arg, victim_name);
		if (*victim_name) {
			victim = get_char_vis(ch, victim_name);
			if (victim) {
				sprintf(tmp, "Practice of %s\n\r", victim->player.name);
				send_to_char(tmp, ch);
			}
			else {
				send_to_char("There's no such a person or mob.\n", ch);
				return FALSE;
			}
		}
		else
			victim = ch;
	}
	else
		victim = ch;

	strcpy(buf, "");
	for (i = 0; *spells[i] != '\n'; i++) {
		if (!*spells[i])
			continue;
		if (!victim->skills[i + 1].learned)
			continue;
		if (!(spell_info[i + 1].min_level[GET_CLASS(victim) - 1] >
			GET_LEVEL(victim))) { 
			if (victim->skills[i + 1].learned <= 0)
				continue;
			sprintf(tmp, "%-20s %-4s\t", spells[i],
				how_good(victim->skills[i+1].learned, victim->skills[i+1].skilled));
			strcat(buf, tmp);
		}
		else
			continue;
		if (*spells[i + 1] != '\n') { 
			i++;
			if (!(spell_info[i + 1].min_level[GET_CLASS(victim) - 1] >
				GET_LEVEL(victim)) &&
				victim->skills[i + 1].learned != 0) { 
				sprintf(tmp, "%-20s %-4s", spells[i],
					how_good(victim->skills[i+1].learned, victim->skills[i+1].skilled));
				strcat(buf, tmp);
			} 
			strcat(buf, "\n\r");
		}
	}
	page_string(ch->desc, buf, 1);
	return TRUE;
}

void do_brief(struct char_data *ch, char *argument, int cmd)
{
  if (IS_NPC(ch))
    return;

  if (IS_SET(ch->specials.act, PLR_BRIEF))
  {
    send_to_char("Brief mode off.\n\r", ch);
    REMOVE_BIT(ch->specials.act, PLR_BRIEF);
  } else {
    send_to_char("Brief mode on.\n\r", ch);
    SET_BIT(ch->specials.act, PLR_BRIEF);
  }
}

void do_compact(struct char_data *ch, char *argument, int cmd)
{
  if (IS_NPC(ch))
    return;
  if (IS_SET(ch->specials.act, PLR_COMPACT)) {
    send_to_char("You are now in the uncompacted mode.\n\r", ch);
    REMOVE_BIT(ch->specials.act, PLR_COMPACT);
  } else {
    send_to_char("You are now in compact mode.\n\r", ch);
    SET_BIT(ch->specials.act, PLR_COMPACT);
  }
}


void do_group(struct char_data *ch, char *argument, int cmd)
{
  char buf[256], name[256];
  struct char_data *victim, *k;
  struct follow_type *f;
  bool found;

  one_argument(argument, name);

  if (!*name) {
    if (ch&&!IS_AFFECTED(ch, AFF_GROUP)) {
      send_to_char("But you are a member of no group?!\n\r", ch);
    } else {
      send_to_char("Your group consists of:\n\r", ch);
      if (ch->master)
        k = ch->master;
      else
        k = ch;

      if (k&&IS_AFFECTED(k, AFF_GROUP)) {
        sprintf(buf,"  [ %5d/%5d %5d/%5d %5d/%5d ]   $N (Head of group)",
         GET_HIT(k),GET_PLAYER_MAX_HIT(k),GET_MANA(k),
         GET_PLAYER_MAX_MANA(k),GET_MOVE(k),GET_PLAYER_MAX_MOVE(k) );
        act(buf,FALSE,ch, 0, k, TO_CHAR);
      }

      for(f=k->followers; f; f=f->next)
        if ( f->follower&&IS_AFFECTED(f->follower, AFF_GROUP)){
          sprintf(buf,"  [ %5d/%5d %5d/%5d %5d/%5d ]   $N",
            GET_HIT(f->follower),GET_PLAYER_MAX_HIT(f->follower),
            GET_MANA(f->follower),GET_PLAYER_MAX_MANA(f->follower),
            GET_MOVE(f->follower),GET_PLAYER_MAX_MOVE(f->follower) );
          act(buf,FALSE,ch, 0, f->follower, TO_CHAR);
        }
    }

    return;
  }

  if (!(victim = get_char_room_vis(ch, name))) {
    send_to_char("No one here by that name.\n\r", ch);
  } else {

    if (ch->master) {
      act("You can not enroll group members without being head of a group.",
         FALSE, ch, 0, 0, TO_CHAR);
      return;
    }

	if(IS_NPC(victim)) {
	  act("You can not enroll mob into your group.",FALSE,ch,0,0,TO_CHAR);
	  send_to_char("But you can enroll mobs by amiable Process.",ch);
	}

    found = FALSE;

    if (victim == ch)
      found = TRUE;
    else {
      for(f=ch->followers; f; f=f->next) {
        if (f->follower == victim) {
          found = TRUE;
          break;
        }
      }
    }
    
    if (found) {
      if (victim&&IS_AFFECTED(victim, AFF_GROUP)) {
        act("$n has been kicked out of the group!", FALSE, victim, 0, ch, TO_ROOM);
        act("You are no longer a member of the group!", FALSE, victim, 0, 0, TO_CHAR);
        REMOVE_BIT(victim->specials.affected_by, AFF_GROUP);
      } else {
        act("$n is now a group member.", FALSE, victim, 0, 0, TO_ROOM);
        act("You are now a group member.", FALSE, victim, 0, 0, TO_CHAR);
        SET_BIT(victim->specials.affected_by, AFF_GROUP);
      }
    } else {
      act("$N must follow you, to enter the group", FALSE, ch, 0, victim, TO_CHAR);
    }
  }
}
void do_quaff(struct char_data *ch, char *argument, int cmd)
{
  char buf[100];
  struct obj_data *temp;
  int i;
  bool equipped;

  equipped = FALSE;

  one_argument(argument,buf);

  if (!(temp = get_obj_in_list_vis(ch,buf,ch->carrying))) {
    temp = ch->equipment[HOLD];
    equipped = TRUE;
    if ((temp==0) || !isname(buf, temp->name)) {
      act("You do not have that item.",FALSE,ch,0,0,TO_CHAR);
      return;
    }
  }

  if (temp->obj_flags.type_flag!=ITEM_POTION)
  {
    act("You can only quaff potions.",FALSE,ch,0,0,TO_CHAR);
    return;
  }

  act("$n quaffs $p.", TRUE, ch, temp, 0, TO_ROOM);
  act("You quaff $p which dissolves.",FALSE,ch,temp,0,TO_CHAR);

  for (i=1; i<4; i++)
    if (temp->obj_flags.value[i] >= 1)
      ((*spell_info[temp->obj_flags.value[i]].spell_pointer)
        ((byte) temp->obj_flags.value[0], ch, "", SPELL_TYPE_POTION, ch, 0));

  if (equipped)
    unequip_char(ch, HOLD);

  extract_obj(temp);
}


void do_recite(struct char_data *ch, char *argument, int cmd)
{
  char buf[100];
  struct obj_data *scroll, *obj;
  struct char_data *victim;
  int i, bits;
  bool equipped;

  equipped = FALSE;
  obj = 0;
  victim = 0;

  if (!ch)
	return;
  if (IS_SET(world[ch->in_room].room_flags,NO_MAGIC))
  {
	send_to_char("Some misterious power prevent you reading scroll\n\r",ch);
	return;
  }
  argument = one_argument(argument,buf);

  if (!(scroll = get_obj_in_list_vis(ch,buf,ch->carrying))) {
    scroll = ch->equipment[HOLD];
    equipped = TRUE;
    if ((scroll==0) || !isname(buf, scroll->name)) {
      act("You do not have that item.",FALSE,ch,0,0,TO_CHAR);
      return;
    }
  }

  if (scroll->obj_flags.type_flag!=ITEM_SCROLL)
  {
    act("Recite is normally used for scroll's.",FALSE,ch,0,0,TO_CHAR);
    return;
  }

  if (*argument) {
    bits = generic_find(argument, FIND_OBJ_INV | FIND_OBJ_ROOM |
        FIND_OBJ_EQUIP , ch, &victim, &obj);
    if (bits == 0) {
      send_to_char("No such thing around to recite the scroll on.\n\r", ch);
      return;
    }
  } else {
    victim = ch;
  }

  act("$n recites $p.", TRUE, ch, scroll, 0, TO_ROOM);
  act("You recite $p which dissolves.",FALSE,ch,scroll,0,TO_CHAR);

  for (i=1; i<4; i++)
    if (scroll->obj_flags.value[i] >= 1)
      ((*spell_info[scroll->obj_flags.value[i]].spell_pointer)
      ((byte) scroll->obj_flags.value[0], ch, "", SPELL_TYPE_SCROLL, victim, obj));

  if (equipped)
    unequip_char(ch, HOLD);

  extract_obj(scroll);
}



void do_use(struct char_data *ch, char *argument, int cmd)
{
  char buf[MAX_STRING_LENGTH];
  struct char_data *tmp_char;
  struct obj_data *tmp_object, *stick;

  int bits;

  argument = one_argument(argument,buf);

  if (ch->equipment[HOLD] == 0 ||
      !isname(buf, ch->equipment[HOLD]->name)) {
    act("You do not hold that item in your hand.",FALSE,ch,0,0,TO_CHAR);
    return;
  }

	/* by ares */
	sprintf(buf, "Use log : %s uses %s", ch->player.name, argument);
	log(buf);

  stick = ch->equipment[HOLD];

  if (stick->obj_flags.type_flag == ITEM_STAFF)
  {
    act("$n taps $p three times on the ground.",TRUE, ch, stick, 0,TO_ROOM);
    act("You tap $p three times on the ground.",FALSE,ch, stick, 0,TO_CHAR);

    if (stick->obj_flags.value[2] > 0) {  /* Is there any charges left? */
      stick->obj_flags.value[2]--;
      ((*spell_info[stick->obj_flags.value[3]].spell_pointer)
      ((byte) stick->obj_flags.value[0], ch, "", SPELL_TYPE_STAFF, 0, 0));

    } else {
      send_to_char("The staff seems powerless.\n\r", ch);
    }
  } else if (stick->obj_flags.type_flag == ITEM_WAND) {

    bits = generic_find(argument, FIND_CHAR_ROOM | FIND_OBJ_INV | FIND_OBJ_ROOM |
                        FIND_OBJ_EQUIP, ch, &tmp_char, &tmp_object);
    if (bits) {
      if (bits == FIND_CHAR_ROOM) {
        act("$n point $p at $N.", TRUE, ch, stick, tmp_char, TO_ROOM);
        act("You point $p at $N.",FALSE,ch, stick, tmp_char, TO_CHAR);
      } else {
        act("$n point $p at $P.", TRUE, ch, stick, tmp_object, TO_ROOM);
        act("You point $p at $P.",FALSE,ch, stick, tmp_object, TO_CHAR);
      }

      if (stick->obj_flags.value[2] > 0) { /* Is there any charges left? */
        stick->obj_flags.value[2]--;
        ((*spell_info[stick->obj_flags.value[3]].spell_pointer)
        ((byte) stick->obj_flags.value[0], ch, "", SPELL_TYPE_WAND, tmp_char, tmp_object));
      } else {
        send_to_char("The wand seems powerless.\n\r", ch);
      }
    } else {
      send_to_char("What should the wand be pointed at?\n\r", ch);
    }
  } else {
    send_to_char("Use is normally only for wand's and staff's.\n\r", ch);
  }
}

void do_post(struct char_data *ch, char *argument, int cmd)
{
    send_to_char( "You can only post on board.\n\r", ch );
}
/*
Assault skill: 남자나 여자를 강간한다. :)
		입고있는 아이템을 벗긴다. 헤헤헤...야한 메시지와 함께.
by Process
*/
void do_assault(struct char_data *ch,char *argument,int cmd)
{
  struct char_data *victim;
  struct obj_data *obj;

  char victim_name[240];
  int percent;
  int location;

  if(GET_GUILD(ch)!=OUTLAW&&GET_LEVEL(ch)<IMO)
  {
	send_to_char("How about join outlaws???\n\r",ch);
	return;
  }
  one_argument(argument,victim_name);

  if( !(victim = get_char_room_vis(ch, victim_name) ) )
  {
	send_to_char("who do you wanna assault?\n\r",ch);
	return;
  }

  if ( !IS_NPC(victim) && !(victim->desc) )
  {
	send_to_char("You cannot assault him or her.\n\r",ch);
	return;
  }

  if ( GET_SEX(victim) == GET_SEX(ch) )
  {
	switch(GET_SEX(ch))
	{
	case SEX_MALE:
		send_to_char("너 호모냐?\n\r",ch);
		break;
	case SEX_FEMALE:
		send_to_char("너 레즈비언이냐?\n\r",ch);
		break;
	}
	return;
  }	
  percent = number(1,101) + ((GET_LEVEL(victim) + GET_DEX(victim)) << 1) -
	(GET_LEVEL(ch) + GET_DEX(ch));

  if ( percent < GET_GUILD_SKILL(ch,OUTLAW_SKILL_ASSAULT) )
  {
	location=number(2,MAX_WEAR-1);
	obj=victim->equipment[location];
	if(obj&&CAN_SEE_OBJ(ch,obj))
	{
		if (victim) obj_to_char(unequip_char(victim,location),victim);
		switch(GET_SEX(victim))
		{
		case SEX_MALE:
		case SEX_NEUTRAL:
			do_say(ch,"아하아~~~~너무 좋아\n\r", 0);
			do_say(victim,"아~~~나도 좋아요~~~\n\r", 0);
			do_say(victim,"아...기막히다....한 번 더하고 싶다~~\n\r", 0);
			break;
		case SEX_FEMALE:
			do_say(ch,"아하아~~~~너무 좋아\n\r", 0);
			do_say(victim,"아~~~나도 좋아요~~~\n\r", 0);
			do_say(victim,"이제 진정한 여자가 된 기분이에요~~~\n\r", 0);
			break;
		}
		
	}
	else
	{
		send_to_char("바보....거긴 안 입었다..:)\n\r",ch);
	}
  }
  else
  {
	send_to_char("먼 챙피? 하하하...\n\r",ch);
  }
}
/* 
	Chase written 
Description:
	Disarms other player's wielding weapon
*/
void do_disarm(struct char_data *ch,char *argument,int cmd)
{
  struct char_data *victim;
  struct obj_data *obj;
  char victim_name[240];
  int percent;
  extern int nodisarmflag;

  one_argument(argument,victim_name);

  if (!(victim = get_char_room_vis(ch,victim_name))) {
	send_to_char("Disarm who?\n\r",ch);
	return;
  } else if (victim==ch) {
	send_to_char("You stupid! How about using remove instead of disarm?\n\r",ch);
	return;
  }
  if (!IS_NPC(victim) &&
	  (GET_LEVEL(victim) >= IMO) &&
	  (GET_LEVEL(ch) < GET_LEVEL(victim)))
	  return;
  if ((!IS_NPC(victim))&&(nodisarmflag)){
	act("Oops..",FALSE,ch,0,0,TO_CHAR);
	act("$n tried to disarm your weapon!",FALSE,ch,0,victim,TO_VICT);
	act("$n tries to disarm $N.",TRUE,ch,0,victim,TO_NOTVICT);
	return;
  }
  if ( !IS_NPC(victim) && !(victim->desc) )
  {
	send_to_char("Disarm the ghost? No way!\n\r",ch);
	return;
  }
  percent=number(1,100)-GET_LEVEL(ch)+2*GET_LEVEL(victim)
		-dex_app_skill[GET_DEX(ch)].p_pocket
		- (GET_SKILLED(ch, SKILL_DISARM) >> 3);
  if( percent < ch->skills[SKILL_DISARM].learned )
  {
  	obj=victim->equipment[WIELD];
	if(!obj&&GET_CLASS(victim)==CLASS_WARRIOR)
	{
		obj=victim->equipment[HOLD];
	}
  	if( obj && CAN_SEE_OBJ(ch,obj) ) 
  	{
  		INCREASE_SKILLED(ch, victim, SKILL_DISARM);
		if(victim->equipment[WIELD])
			obj_to_char(unequip_char(victim,WIELD),victim);
 		if(victim->equipment[HOLD])
			obj_to_char(unequip_char(victim,HOLD),victim);
		act("$n disarmed you!",TRUE,ch,0,victim,TO_VICT);
		act("$n disarmed $N!",TRUE,ch,0,victim,TO_NOTVICT);
		act("Your disarming successful!",FALSE,ch,0,0,TO_CHAR);
  	}
  	else
  	{
		act("$E is not wielding any weapon now.",FALSE,ch,0,victim,TO_CHAR);
		return;
  	}
  }
  else
  {
	act("$n failed to disarm your weapon!",FALSE,ch,0,victim,TO_VICT);
	act("$n fails to disarm $N.",TRUE,ch,0,victim,TO_NOTVICT);
	if ( AWAKE(victim) && GET_LEVEL(ch) < GET_LEVEL(victim) )
		hit(victim,ch,TYPE_UNDEFINED);
  }
}

void do_spin_bird_kick(struct char_data *ch,char *argument,int cmd)
{
	unsigned int dam;
	struct char_data *victim,*tmp_vic;
	char victim_name[240];
	int percent;
	int tmp,i;
	int level_plus;

    if ((GET_LEVEL(ch) == IMO) && (!IS_NPC(ch))) {
    return;
    }
	if (GET_SEX(ch) != SEX_FEMALE) {
		send_to_char("여자 전용 스킬입니다.성전환을 하는게 어떨지~~\n\r",ch);
		return;
	}

	one_argument(argument,victim_name);

	if (!*victim_name)
	{
		if (ch->specials.fighting)
			victim = ch->specials.fighting;
		else {
			send_to_char_han("Kick who?","누굴 때려요?\n\r",ch);
			return;
		}
	}
	if (!(victim=get_char_room_vis(ch,victim_name))) {
		if (ch->specials.fighting)
			victim = ch->specials.fighting;
		else {
			send_to_char_han("Kick who?","누굴 때려요?\n\r",ch);
			return;
		}
	}

	if (!IS_NPC(victim) && !(victim->desc)) {
		send_to_char("유령을 치겠다고요? 호호!\n\r",ch);
		return;
	}

	if (!AWAKE(victim)) {
		send_to_char("자는 사람을 치다니 비겁한 년!!\n\r",ch);
		return;
	}

	if (GET_MANA(ch) < 100) {
		send_to_char("You do not have enough mana.\n\r",ch);
		return;
	}

	dam=GET_LEVEL(ch)*(number(25,30)+(GET_SKILLED(ch,SKILL_SPIN_BIRD_KICK)>>3));

	if (saves_spell(victim, SAVING_HIT_SKILL))
		dam >>= 1;

	percent = number(1, 150) - (GET_LEVEL(ch) + GET_DEX(ch)) +
		2 * (GET_LEVEL(victim) + GET_DEX(victim))
		- (GET_SKILLED(ch, SKILL_SPIN_BIRD_KICK) >> 3);

	do_say(ch,"스핀~~~버드~~~킥!!!", 0);

	if(ch == victim){
		send_to_char("잉~~~괜히 속옷만 보여줬다...\n\r",ch);
		return;
	}

	if (GET_MOVE(ch) < 0) {
		send_to_char("당신은 너무 지쳐서 시도할 수 가 없군요!\n\r",ch);
		do_say(ch,"학학학...난 너무 지쳤어!!!\n\r", 0);
		return;
	}

	if (percent < ch->skills[SKILL_SPIN_BIRD_KICK].learned) {
		INCREASE_SKILLED(ch, victim, SKILL_SPIN_BIRD_KICK);
		level_plus=10+((GET_LEVEL(ch)+GET_SKILLED(ch,SKILL_SPIN_BIRD_KICK))>>3);
		tmp = number(10, level_plus);
		GET_MOVE(ch) -= 600 - GET_SKILLED(ch, SKILL_SPIN_BIRD_KICK);
		GET_MANA(ch) -= 300 - GET_SKILLED(ch, SKILL_SPIN_BIRD_KICK);
		for (i = 0; i < tmp; i++) {
			switch(number(1,4)) {
				case 1:
					act("차차차차찻~~~~",TRUE,ch,0,0,TO_ROOM);
					break;
				case 2:
					act("요~~~오~~~옷!!",TRUE,ch,0,0,TO_ROOM);
					break;
				case 3:
					act("파바바바박~~!!",TRUE,ch,0,0,TO_ROOM);
					break;
				case 4:
					act("얍얍얍얍얍!!!!",TRUE,ch,0,0,TO_ROOM);
					break;
				default:
					break;
			}
			if(!(tmp_vic=get_char_room_vis(ch,victim_name))) {
				if (ch->specials.fighting)
					tmp_vic = ch->specials.fighting;
			}
			if (victim == tmp_vic)
				damage(ch, victim, dam, TYPE_HIT);
			else	break;
		}
		send_to_char("하하...내 치맛바람 맛이 어떠냐!!!\n\r",ch);
	}
	else {
		WAIT_STATE(ch, PULSE_VIOLENCE / 3);
		damage(ch, victim, dam, TYPE_HIT);
		send_to_char("잉~~~괜히 속옷만 보여줬다...\n\r",ch);
	}
}

/* chase added this for Shou Ryu Ken */
void do_shouryuken(struct char_data *ch,char *argument,int cmd)
{
	unsigned int dam;
	struct char_data *victim;
	char victim_name[240];
	int percent;
	// int tmp;
	// int level_plus;

    if ((GET_LEVEL(ch) == IMO) && (!IS_NPC(ch))) {
       return;
    }
  	if (GET_SEX(ch) != SEX_MALE) {
		send_to_char("남자 전용 스킬입니다.성전환을 하는게 어떨지~~\n\r",ch);
		return;
	}

	one_argument(argument,victim_name);

	if (!*victim_name)
	{
		if (ch->specials.fighting)
			victim = ch->specials.fighting;
		else {
			send_to_char("누굴 때려?\n\r",ch);
			return;
		}
		/*
		send_to_char_han("Kick who?","누굴 때려요?\n\r",ch);
		return;
		*/
	}
	if(!(victim=get_char_room_vis(ch,victim_name))) {
		if (ch->specials.fighting)
			victim = ch->specials.fighting;
		else {
			send_to_char("누굴 때려?\n\r",ch);
			return;
		}
	}

	if (!IS_NPC(victim) && !(victim->desc)) {
		send_to_char("유령을 치겠다고? 미친소리!\n\r",ch);
		return;
	}

	if (!AWAKE(victim)) {
		send_to_char("자는 사람을 치다니 비겁한 놈!!\n\r",ch);
		return;
	}

	if (GET_MANA(ch) < 100) {
		/*
		send_to_char("You do not have enough mana.\n\r",ch);
		*/
		send_to_char("당신은 너무 지쳐서 시도할 수 가 없군요!\n\r",ch);
		do_say(ch,"학학학...난 너무 지쳤어!!!\n\r", 0);
		return;
	}

	dam=GET_LEVEL(ch)*(number(13,18)+(GET_SKILLED(ch,SKILL_SHOURYUKEN)>>3));

	if (saves_spell(victim, SAVING_HIT_SKILL))
		dam >>= 1;

	percent = number(1, 150) - (GET_LEVEL(ch) + GET_DEX(ch)) +
		2 * (GET_LEVEL(victim) + GET_DEX(victim))
		- (GET_SKILLED(ch, SKILL_SHOURYUKEN) >> 3);

	do_say(ch,"쇼~~~류~~켄~", 0);

	if(ch == victim){
		send_to_char("당신은 승룡권을 실패해서 몹시 지칩니다.\n\r",ch);
		return;
	}

	if (GET_MOVE(ch) < 0) {
		send_to_char("당신은 너무 지쳐서 시도할 수 가 없군요!\n\r",ch);
		do_say(ch,"헥헥헥...난 너무 지쳤어!!!\n\r", 0);
		return;
	}

	if (percent < ch->skills[SKILL_SHOURYUKEN].learned) {
		INCREASE_SKILLED(ch, victim, SKILL_SHOURYUKEN);
		// level_plus = 10 + (GET_LEVEL(ch) + GET_SKILLED(ch, SKILL_SHOURYUKEN)) / 8;
		// tmp = number(10, level_plus);
		GET_MOVE(ch) -= (600 - GET_SKILLED(ch, SKILL_SHOURYUKEN));
		GET_MANA(ch) -= (300 - GET_SKILLED(ch, SKILL_SHOURYUKEN));
		/*
		for(i = 0; i < tmp; i++) {
			switch(number(1,4)) {
				case 1:
					act("이얍얍얍얍~~~",TRUE,ch,0,0,TO_ROOM);
					break;
				case 2:
					act("끼야오홋!!!!!",TRUE,ch,0,0,TO_ROOM);
					break;
				case 3:
					act("아자자자자~~!!",TRUE,ch,0,0,TO_ROOM);
					break;
				case 4:
					act("으샤샤샤샤!!!!",TRUE,ch,0,0,TO_ROOM);
					break;
				default:
					break;
			}
			if(!(tmp_vic=get_char_room_vis(ch,victim_name))) {
				if (ch->specials.fighting)
					tmp_vic = ch->specials.fighting;
			}
			if (victim == tmp_vic)
				damage(ch, victim, dam, TYPE_HIT);
			else	break;
		}
		*/
		act("쇼오오오~~", TRUE, ch, 0, 0, TO_ROOM);
		damage(ch, victim, dam, TYPE_HIT);

		dam *= number(2, 3);
		act("류우우우~~", TRUE, ch, 0, 0, TO_ROOM);
		damage(ch, victim, dam, TYPE_HIT);

		dam *= number(5, 7 + (GET_SKILLED(ch, SKILL_SHOURYUKEN) >> 4));
		act("켄!!!!!!!!", TRUE, ch, 0, 0, TO_ROOM);
		damage(ch, victim, dam, TYPE_HIT);

		send_to_char("빠샤!!!!!!\n\r", ch);
	}
	else {
		WAIT_STATE(ch, PULSE_VIOLENCE / 3);
		damage(ch, victim, dam, TYPE_HIT);
		send_to_char("당신은 승룡권을 실패해서 몹시 지칩니다.\n\r",ch);
	}
}

/* throw skill by chase */
void do_throw_object(struct char_data *ch,char *argument,int cmd)
{
	struct char_data *victim;
	struct obj_data *obj;
	char victim_name[240];
	char obj_name[240];
	int percent;
	char buf[100];
	int dam;

	argument=one_argument(argument,obj_name);
	one_argument(argument,victim_name);

    if ((GET_LEVEL(ch) == IMO) && (!IS_NPC(ch))) {
       return;
    }
    if ( GET_CLASS(ch) != CLASS_THIEF && GET_LEVEL(ch) < IMO )
	{
		send_to_char("This skill is for thieves only.\n\r",ch);
		return;
	}

	if( !(victim = get_char_room_vis(ch, victim_name)) ) {
		send_to_char("throw what to who?\n\r", ch);
		return;
	} else if (victim == ch) {
		send_to_char("Come on now, that's rather stupid!\n\r", ch);
		return;
	}

	if ( !IS_NPC(victim) && !(victim->desc) )
	{
		send_to_char("throw something to ghost? No way!\n\r",ch);
		return;
	}

	if ( ch && GET_MANA(ch) < 100 )
	{
		send_to_char("You do not have enough mana!\n\r",ch);
		return;
	}
	GET_MANA(ch) -= 100;

	if(ch->carrying)
		obj=get_obj_in_list_vis(ch,obj_name,ch->carrying);
	else
		return;
	if(obj)
	{
		if( !IS_SET(obj->obj_flags.extra_flags,ITEM_NODROP) )
		{
			INCREASE_SKILLED(ch, victim, SKILL_THROW_OBJECT);
			dam = obj->obj_flags.cost / 1000;
			dam += (GET_LEVEL(ch) + GET_STR(ch) + GET_ADD(ch) + GET_CON(ch));
			dam *= ((GET_LEVEL(ch) + GET_SKILLED(ch, SKILL_THROW_OBJECT)) >> 3);
			percent=number(1,100)-GET_LEVEL(ch)+2*GET_LEVEL(victim)
					-(GET_SKILLED(ch, SKILL_THROW_OBJECT) >> 2);
			if (percent > ch->skills[SKILL_THROW_OBJECT].learned)
			{
				dam=0;
				do_say(ch,"윽...이렇게 비싼건....망했당..\n\r", 0);
			}
			sprintf(buf, "$n님이 $N님에게 %s을 던집니다!!!", fname(obj->name) );
			act("하이야~~~~압~~~",TRUE,ch,0,0,TO_ROOM);
			act("이야야~~~~합~~~",TRUE,ch,0,0,TO_ROOM);
			do_say(ch,"받아랏~~~~!!!@!#@!", 0);
			act(buf,TRUE,ch,0,victim,TO_NOTVICT);
			damage(ch,victim,dam,TYPE_UNDEFINED);
			if(obj) obj_from_char(obj);
			if(obj) extract_obj(obj);
		}
		else
		{
			send_to_char("You can't throw it, it must be CURSED!\n\r",ch);
			return;
		}
	}
	else
	{
		send_to_char("You do not have that item.\n\r",ch);
		return;
	}
	if(!IS_NPC(ch))
		WAIT_STATE(ch,PULSE_VIOLENCE);
}
