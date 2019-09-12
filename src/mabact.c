/* ************************************************************************
*  file: mobact.c , Mobile action module.                 Part of DIKUMUD *
*  Usage: Procedures generating 'intelligent' behavior in the mobiles.    *
*  Copyright (C) 1990, 1991 - see 'license.doc' for complete information. *
************************************************************************* */

#include <stdio.h>

#include "utils.h"
#include "structs.h"
#include "db.h"
#include "comm.h"
#include "spells.h"

extern struct char_data *character_list;
extern struct index_data *mob_index;
extern struct room_data *world;
extern struct str_app_type str_app[];

void hit(struct char_data *ch, struct char_data *victim, int type);
void log(char *str);
int number(int from, int to);
void obj_from_room(struct obj_data *o);
void obj_to_char(struct obj_data *o, struct char_data *ch);
bool saves_spell(struct char_data *ch, int type);
int thief(struct char_data *ch, int cmd, char *arg);
int dragon(struct char_data *ch, int cmd, char *arg);
int magic_user(struct char_data *ch, int cmd, char *arg);
int shooter(struct char_data *ch, int cmd, char *arg);
int kickbasher(struct char_data *ch, int cmd, char *arg);
int spitter(struct char_data *ch, int cmd, char *arg);
int cleric(struct char_data *ch, int cmd, char *arg);
int paladin(struct char_data *ch, int cmd, char *arg);
int cityguard(struct char_data *ch, int cmd, char *arg);
int superguard(struct char_data *ch, int cmd, char *arg);
int rescuer(struct char_data *ch, int cmd, char *arg);
int helper(struct char_data *ch, int cmd, char *arg);
int finisher(struct char_data *ch, int cmd, char *arg);

int warrior(struct char_data *ch, int cmd, char *arg);
void first_attack(struct char_data *ch, struct char_data *victim);
void do_cast(struct char_data *ch, char *arg, int cmd);

int check_stat(struct char_data *ch)
{
  int ratio, success;
  char buf[80];
  
  /* check hit */
	if(GET_LEVEL(ch) > 25){
	  ratio = (100 * GET_HIT(ch)) / hit_limit(ch);
	  if(ratio < 50){
	    success = GET_WIS(ch) + GET_INT(ch) + GET_LEVEL(ch);
	    if(number(1, 100) < success){ /* heal */
	      switch(GET_CLASS(ch)){
	      case CLASS_CLERIC     :
		sprintf(buf, " 'full heal' %s", GET_NAME(ch));
							break;
	      case CLASS_MAGIC_USER :
	      case CLASS_WARRIOR    :
	      case CLASS_THIEF    :
		sprintf(buf, " 'self heal' %s", GET_NAME(ch));
							break;
	      }
	      
	      do_cast(ch, buf, 0);
	      return 1;
	    }
	  }
	}
	
	return 0;
}

void mobile_activity2(void)
{
  register struct char_data *ch;
  for (ch = character_list; ch; ch = ch->next) {
    if(IS_NPC(ch) && GET_CLASS(ch) > 30){
      if(check_stat(ch)) continue;
      
      /* fighting */
      if(GET_POS(ch)!=POSITION_FIGHTING) continue;
      switch(GET_CLASS(ch)){
      case CLASS_MAGIC_USER : magic_user(ch, 0, ""); break;
      case CLASS_CLERIC     : cleric(ch, 0, ""); break;
      case CLASS_THIEF      : thief(ch, 0, ""); break;
          case CLASS_WARRIOR    : warrior(ch, 0, ""); break;
      }
    }
  }
}

void mobile_activity(void)
{
  register struct char_data *ch;
  struct char_data *tmp_ch = NULL,*cho_ch = NULL;
  struct obj_data *obj, *best_obj;
  int door, found, max;
  char buf[100];
  
  extern int no_specials;
  
  void do_move(struct char_data *ch, char *argument, int cmd);
  void do_get(struct char_data *ch, char *argument, int cmd);
  
  /* son_ogong mirror */
  ch = character_list;
  while(ch){
#define SON_OGONG_MIRROR	11141
    tmp_ch = ch->next;
    if(mob_index[ch->nr].virtual == SON_OGONG_MIRROR){
      (ch->quest.solved) ++;
      if(ch->quest.solved > 50){
	extract_char(ch);
      }
    }
    ch = tmp_ch;
  }
  
  for (ch = character_list; ch; ch = ch->next) {
    if (IS_MOB(ch)) {
      /* Examine call for special procedure */
      if (IS_SET(ch->specials.act, ACT_SPEC) && !no_specials) {
	if (!mob_index[ch->nr].func) {
	  sprintf(buf,"Attempting to call a non-existing MOB func.\n (mobact.c) %s",
		  ch->player.short_descr );
	  log(buf);
	  REMOVE_BIT(ch->specials.act, ACT_SPEC);
	} 
	else {
	  if ((*mob_index[ch->nr].func) (ch, 0, ""))
	    /*continue;*/;
	}
      }
      if (AWAKE(ch) && !(ch->specials.fighting)) {
	if (IS_SET(ch->specials.act, ACT_SCAVENGER)) {
	  if (world[ch->in_room].contents && !number(0,10)) {
	    for (max = 1, best_obj = 0, obj = world[ch->in_room].contents;
		 obj; obj = obj->next_content) {
	      if (CAN_GET_OBJ(ch, obj)) {
		if (obj->obj_flags.cost > max) {
		  best_obj = obj;
		  max = obj->obj_flags.cost;
		}
	      }
	    } /* for */
	    
	    if (best_obj) {
	      obj_from_room(best_obj);
	      obj_to_char(best_obj, ch);
	      act("$n gets $p.",FALSE,ch,best_obj,0,TO_ROOM);
	    }
	  }
	} /* Scavenger */
	if (!IS_SET(ch->specials.act, ACT_SENTINEL) && 
	    (GET_POS(ch) == POSITION_STANDING) &&
	    ((door = number(0, 45)) <= 5) && CAN_GO(ch,door) &&
	    !IS_SET(world[EXIT(ch, door)->to_room].room_flags, NO_MOB)) {
	  if (ch->specials.last_direction == door) {
	    ch->specials.last_direction = -1;
	  } 
	  else {
	    if (!IS_SET(ch->specials.act, ACT_STAY_ZONE)) {
	      ch->specials.last_direction = door;
	      do_move(ch, "", ++door);
	    } 
	    else {
	      if (world[EXIT(ch, door)->to_room].zone == world[ch->in_room].zone) {
		ch->specials.last_direction = door;
		do_move(ch, "", ++door);
	      }
	    }
	  }
	} /* if can go */
	
	if (IS_SET(ch->specials.act,ACT_AGGRESSIVE)) {
	  found = FALSE;
	  for (tmp_ch = world[ch->in_room].people; tmp_ch;
	       tmp_ch = tmp_ch->next_in_room) {
	    if (!IS_NPC(tmp_ch) && CAN_SEE(ch, tmp_ch) 
		&& (GET_LEVEL(tmp_ch)<IMO)) {
	      if (!IS_SET(ch->specials.act, ACT_WIMPY) || !AWAKE(tmp_ch)) {
		if(!found){
		  cho_ch = tmp_ch;
		  found = TRUE;
		  if(IS_EVIL(ch) && IS_GOOD(cho_ch) && IS_AFFECTED(cho_ch,AFF_PROTECT_EVIL)){
		    if(!saves_spell(ch,SAVING_PARA) && GET_LEVEL(ch) < GET_LEVEL(cho_ch)){
		      act("$n tries to attack, but failed miserably.",TRUE,ch,0,0,TO_ROOM);
		      found = FALSE;
		    }
		  }
		} else {
		  if(number(1,6)<=3)
		    cho_ch=tmp_ch;
		} /* else */
	      } /* if IS_SET */
	    } /* if IS_NPC */
	  } /* for */

	  if(ch&&cho_ch) {
	    if(!IS_AFFECTED(cho_ch,AFF_HOLY_SHIELD)) {
	      if(found) {
		first_attack(ch, cho_ch);
		/*
		  hit(ch, cho_ch, 0);
		  */
	      }
	    }
	    else {
	    }
	  }
	} /* is aggressive */
      } /* If AWAKE(ch)   */
      
   if(check_stat(ch)) continue;

   switch(GET_CLASS(ch)){
       case CLASS_MAGIC_USER : magic_user(ch, 0, ""); break;
       case CLASS_CLERIC     : cleric(ch, 0, ""); break;
       case CLASS_THIEF      : thief(ch, 0, ""); break;
       case CLASS_WARRIOR    : warrior(ch, 0, ""); break;
   }
/*
   if(IS_SET(ch->specials.act, ACT_THIEF)){
      thief(ch,0,"");
    }
   if(IS_SET(ch->specials.act, ACT_MAGE)){
     magic_user(ch,0,"");
   }
   if(IS_SET(ch->specials.act, ACT_CLERIC)){
     cleric(ch,0,"");
   }
*/
   if(IS_SET(ch->specials.act, ACT_DRAGON)){
     dragon(ch,0,"");
   }
   if(IS_SET(ch->specials.act, ACT_SHOOTER)){
     shooter(ch,0,"");
   }
   if(IS_SET(ch->specials.act, ACT_FIGHTER)){
     kickbasher(ch,0,"");
   }
   if(IS_SET(ch->specials.act, ACT_SPITTER)){
     spitter(ch,0,"");
   }
   if(IS_SET(ch->specials.act, ACT_PALADIN)){
     paladin(ch,0,"");
   }
   if(IS_SET(ch->specials.act, ACT_GUARD)){
     cityguard(ch,0,"");
   }
   if(IS_SET(ch->specials.act, ACT_SUPERGUARD)){
     superguard(ch,0,"");
   }
   if(IS_SET(ch->specials.act, ACT_RESCUER)){
     rescuer(ch,0,"");
   }
   if(IS_SET(ch->specials.act, ACT_HELPER)){
     helper(ch,0,"");
   }
   if(IS_SET(ch->specials.act, ACT_FINISH_BLOW)){
     finisher(ch,0,"");
   }

   
  }   /* If IS_MOB(ch)  */
 } /* for..*/
}

