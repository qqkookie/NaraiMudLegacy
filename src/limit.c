/* ************************************************************************
*  file: limits.c , Limit and gain control module.        Part of DIKUMUD *
*  Usage: Procedures controling gain and limit.                           *
*  Copyright (C) 1990, 1991 - see 'license.doc' for complete information. *
************************************************************************* */

#include <stdio.h>
#include <string.h>
#include <assert.h>
#include "structs.h"
#include "limit.h"
#include "utils.h"
#include "spells.h"
#include "comm.h"

#include "guild_list.h"

#define SPELL_LEARN_MAX	120

#define READ_TITLE(ch) (GET_SEX(ch) == SEX_MALE ?   \
  titles[(int)GET_CLASS(ch) - 1][(int) GET_LEVEL(ch)].title_m :  \
  titles[(int)GET_CLASS(ch) - 1][(int) GET_LEVEL(ch)].title_f)


extern struct char_data *character_list;
extern struct obj_data *object_list;
extern struct title_type titles[4][IMO+4];
extern struct room_data *world;

/* External procedures */

void move_stashfile( char *victim);
void update_pos( struct char_data *victim );                 /* in fight.c */
void damage(struct char_data *ch, struct char_data *victim,  /*    do      */
            int damage, int weapontype);
struct time_info_data age(struct char_data *ch);
int number(int from, int to);
int dice(int num, int size);
int MIN(int a, int b);
int MAX(int a, int b);
void stop_fighting(struct char_data *ch);
void char_from_room(struct char_data *ch);
void char_to_room(struct char_data *ch, int to);
void do_rent(struct char_data *ch, int cmd, char *arg);
void close_socket(struct descriptor_data *d);
void obj_from_obj(struct obj_data *o);
void obj_to_obj(struct obj_data *o, struct obj_data *to);
void obj_to_room(struct obj_data *o, int room);



/* When age < 15 return the value p0 */
/* When age in 15..29 calculate the line between p1 & p2 */
/* When age in 30..44 calculate the line between p2 & p3 */
/* When age in 45..59 calculate the line between p3 & p4 */
/* When age in 60..79 calculate the line between p4 & p5 */
/* When age >= 80 return the value p6 */
int graf(int age, int p0, int p1, int p2, int p3, int p4, int p5, int p6)
{
  if (age < 15)
    return(p0);                               /* < 15   */
  else if (age <= 29) 
    return (int) (p1+(((age-15)*(p2-p1))/15));  /* 15..29 */
  else if (age <= 44)
    return (int) (p2+(((age-30)*(p3-p2))/15));  /* 30..44 */
  else if (age <= 59)
    return (int) (p3+(((age-45)*(p4-p3))/15));  /* 45..59 */
  else if (age <= 79)
    return (int) (p4+(((age-60)*(p5-p4))/20));  /* 60..79 */
  else
    return(p6);                               /* >= 80 */
}


/* The three MAX functions define a characters Effective maximum */
/* Which is NOT the same as the ch->points.max_xxxx !!!          */

int hit_limit(struct char_data *ch)
{
  int extra[7] = { 1, 2, 4, 8, 4, 2, 1 };
  int max;
  
  max = ch->points.max_hit;
  
  if (!IS_NPC(ch)) {
    /*
      share = max / 10;
      for (i = 0; i < 7; i++)
      extra[i] *= share;
      */
    max += graf(age(ch).year, extra[0], extra[1], extra[2], extra[3],
		extra[4], extra[5], extra[6]);
  }
  
  return max;
}

int mana_limit(struct char_data *ch)
{
  int extra[7] = { 1, 2, 3, 4, 3, 2, 1 };
  int max;
  
  max = ch->points.max_mana;
  
	if (!IS_NPC(ch)) {
	  /*
	    share = max / 10;
	    for (i = 0; i < 7; i++)
	    extra[i] *= share;
	    */
	  max += graf(age(ch).year, extra[0], extra[1], extra[2], extra[3],
		      extra[4], extra[5], extra[6]);
	}
	return max;
}

int move_limit(struct char_data *ch)
{
  int extra[7] = { 1, 2, 4, 8, 4, 2, 1 };
  int max;
  
  max = ch->points.max_move;
  
  if (!IS_NPC(ch)) {
    /*
      share = max / 10;
      for (i = 0; i < 7; i++)
      extra[i] *= share;
      */
    max += graf(age(ch).year, extra[0], extra[1], extra[2], extra[3],
		extra[4], extra[5], extra[6]);
  }
  return max;
}

/* modified by atre */
int hit_gain(struct char_data *ch)
{
  int gain, c;
  int extra[7] = { 1, 3, 6, 8, 5, 3, 1 };
  int i;
  
  c = GET_CON(ch)/2;
  
  if (IS_NPC(ch)) {
    gain = GET_LEVEL(ch) * c;
  }
  else {
    for (i = 0; i < 7; i++)
      extra[i] *= c;
    gain = graf(age(ch).year, extra[0], extra[1], extra[2], extra[3],
		extra[4], extra[5], extra[6])
      + ((GET_REGENERATION(ch) + GET_LEVEL(ch)) << 2);
    
    /* class */
    switch (GET_CLASS(ch)) {
    case CLASS_MAGIC_USER:
      gain += gain >> 3;
      break;
    case CLASS_CLERIC:
      gain += gain >> 3;
      break;
    case CLASS_THIEF:
      gain += (gain/2);
      break;
    case CLASS_WARRIOR:
      gain += gain;
      break;
    }
    
    /* position */
    switch (GET_POS(ch)) {
    case POSITION_SLEEPING:
      gain += gain;
      break;
    case POSITION_RESTING:
      gain += gain >> 1;
      break;
    case POSITION_SITTING:
      gain += gain >> 2;
      break;
    }
    /* rest room */
    if (IS_SET(world[ch->in_room].room_flags, RESTROOM))
      gain += gain;
  }
  
  if (IS_AFFECTED(ch, AFF_POISON)) {
    gain >>= 2;
    gain = number(1, gain);
    GET_HIT(ch) -= dice(30 - GET_CON(ch), 30 - GET_CON(ch));
    /*
      GET_HIT(ch) -= dice(GET_LEVEL(ch), GET_LEVEL(ch));
      if (IS_NPC(ch))
      GET_HIT(ch) -= dice(GET_LEVEL(ch), GET_LEVEL(ch));
      */
  }
  
  if ((GET_COND(ch, FULL) == 0) || (GET_COND(ch, THIRST) == 0))
    gain >>= 2;
  
  return gain;
}

int mana_gain(struct char_data *ch)
{
  int gain, c;
  int extra[7] = { 1, 2, 4, 8, 4, 2, 1 };
  int i;
  
  c = GET_INT(ch)/2;
  
  if (IS_NPC(ch)) {
    gain = GET_LEVEL(ch);
  }
  else {
    for (i = 0; i < 7; i++)
      extra[i] *= c;
    gain = graf(age(ch).year, extra[0], extra[1], extra[2], extra[3],
		extra[4], extra[5], extra[6])
      + ((GET_REGENERATION(ch) + GET_LEVEL(ch)) << 2);
    
    /* level */
    gain += GET_LEVEL(ch)*2;
    /* class */
    switch (GET_CLASS(ch)) {
    case CLASS_MAGIC_USER:
      gain += gain;
      break;
    case CLASS_CLERIC:
      gain += gain;
      break;
    case CLASS_THIEF:
      gain += gain >> 3;
      break;
    case CLASS_WARRIOR:
      gain += gain >> 3;
      break;
    }
    
    /* position */
    switch (GET_POS(ch)) {
    case POSITION_SLEEPING:
      gain += gain;
      break;
    case POSITION_RESTING:
      gain += gain >> 1;
      break;
    case POSITION_SITTING:
      gain += gain >> 2;
      break;
    }
    /* rest room */
    if (IS_SET(world[ch->in_room].room_flags, RESTROOM))
      gain += gain;
  }
  
  if (IS_AFFECTED(ch, AFF_POISON)) {
    gain >>= 2;
    gain = number(1, gain);
  }
  
  if ((GET_COND(ch, FULL) == 0) || (GET_COND(ch, THIRST) == 0))
    gain >>= 2;
  
  return gain;
}

int move_gain(struct char_data *ch)
{
  int gain, c;
  int extra[7] = { 1, 2, 4, 8, 4, 2, 1 };
  int i;
  
  c = GET_DEX(ch)/2;
  
  if (IS_NPC(ch)) {
    gain = GET_LEVEL(ch);
  }
  else {
    for (i = 0; i < 7; i++)
      extra[i] *= c;
    gain = graf(age(ch).year, extra[0], extra[1], extra[2], extra[3],
		extra[4], extra[5], extra[6])
      + ((GET_REGENERATION(ch) + GET_LEVEL(ch)) << 2);
    
    /* level */
    gain += GET_LEVEL(ch)*2;
    /* class */
    switch (GET_CLASS(ch)) {
    case CLASS_MAGIC_USER:
      gain += gain >> 3;
      break;
    case CLASS_CLERIC:
      gain += gain >> 3;
      break;
    case CLASS_THIEF:
      gain += gain;
      break;
    case CLASS_WARRIOR:
      gain = gain*3/2;
      break;
    }
    
    /* position */
    switch (GET_POS(ch)) {
    case POSITION_SLEEPING:
      gain += gain;
      break;
    case POSITION_RESTING:
      gain += gain >> 1;
      break;
    case POSITION_SITTING:
      gain += gain >> 2;
      break;
    }
    /* rest room */
    if (IS_SET(world[ch->in_room].room_flags, RESTROOM))
      gain += gain;
  }
  
  if (IS_AFFECTED(ch, AFF_POISON)) {
    gain >>= 2;
    gain = number(1, gain);
  }
  
  if ((GET_COND(ch, FULL) == 0) || (GET_COND(ch, THIRST) == 0))
    gain >>= 2;
  
  return gain;
}

/* Gain maximum in various points */
void advance_level(struct char_data *ch, int level_up)
{
  int add_hp, add_mana, add_move, i;
  extern struct wis_app_type wis_app[26];
  int level;
  
  level = GET_LEVEL(ch);
  level = MAX(7, level);
  
  add_hp = level * GET_CON(ch);
  add_mana = level * GET_WIS(ch);
  add_move = level * GET_DEX(ch);
  
  add_hp = number(add_hp >> 1, add_hp);
  add_mana = number(add_mana >> 1, add_mana);
  add_move = number(add_move >> 1, add_move);
  
  switch (GET_CLASS(ch)) {
  case CLASS_MAGIC_USER:
  case CLASS_CLERIC:
    add_mana += number(1, level);
    break;
  case CLASS_THIEF:
    add_move += number(1, level);
    break;
  case CLASS_WARRIOR:
    add_hp += number(1, level);
    break;
  }
  
  if (level_up) {
    GET_AC(ch)--;
    GET_HITROLL(ch)++;
    GET_DAMROLL(ch)++;
    ch->points.max_hit += add_hp;
    ch->points.max_mana += add_mana;
    ch->points.max_move += add_move;
    
    if (ch->specials.spells_to_learn < SPELL_LEARN_MAX) {
      if (GET_CLASS(ch) == CLASS_MAGIC_USER ||
	  GET_CLASS(ch) == CLASS_CLERIC)
	ch->specials.spells_to_learn += wis_app[GET_WIS(ch)].bonus;
      else
	ch->specials.spells_to_learn +=
	  number(1, wis_app[GET_WIS(ch)].bonus);
    }
  }
  else {
    GET_AC(ch) += number(2, 5);
    GET_HITROLL(ch)--;
    GET_DAMROLL(ch)--;
    ch->points.max_hit -= add_hp << 1;
    ch->points.max_mana -= add_mana << 1;
    ch->points.max_move -= add_move << 1;
    ch->specials.spells_to_learn -= number(3, 7);
    ch->specials.spells_to_learn = MAX(0, ch->specials.spells_to_learn);
  }
  
  if (GET_LEVEL(ch) > IMO)
    for (i = 0; i < 3; i++)
      ch->specials.conditions[i] = -1;
}  

void set_title(struct char_data *ch)
{
  if (GET_TITLE(ch))
    RECREATE(GET_TITLE(ch),char,strlen(READ_TITLE(ch))+1);
  else
    CREATE(GET_TITLE(ch),char,strlen(READ_TITLE(ch))+1);
  
  strcpy(GET_TITLE(ch), READ_TITLE(ch));
}
void gain_exp(struct char_data *ch, int gain)
{
  if (IS_NPC(ch) || ((GET_LEVEL(ch)<IMO) && (GET_LEVEL(ch) > 0))) {
    if (gain > 0) {
      gain = MIN(GET_LEVEL(ch) * GET_LEVEL(ch) * 20000, gain);
      GET_EXP(ch) += gain;
    }
    if (gain < 0) {
      if (GET_EXP(ch) < (-gain))
	GET_EXP(ch) = 0 ;
      else {
        if (GET_EXP(ch) < (-gain))
          GET_EXP(ch) = 0;
        else
          GET_EXP(ch) += gain;
      }
    }
  }
}
void gain_exp_regardless(struct char_data *ch, int gain)
{
  int i;
  bool is_altered = FALSE;
  
  if (!IS_NPC(ch)) {
    if (gain > 0) {
      GET_EXP(ch) += gain;
      for(i=0;(i<=(IMO+3))&&
	    (titles[GET_CLASS(ch)-1][i].exp<=GET_EXP(ch));i++){
        if (i > GET_LEVEL(ch)) {
          send_to_char("You raise a level.\n\r",ch);
          GET_LEVEL(ch) = i;
          advance_level(ch, 1);
          is_altered = TRUE;
        }
      }
    }
    if (gain < 0) 
      GET_EXP(ch) += gain;
    if (GET_EXP(ch) < 0)
      GET_EXP(ch) = 0;
  }
  if (is_altered)
    set_title(ch);
}
void gain_condition(struct char_data *ch,int condition,int value)
{
  bool intoxicated;
  
  if(GET_COND(ch, condition)==-1) /* No change */
    return;
  intoxicated=(GET_COND(ch, DRUNK) > 0);
  GET_COND(ch, condition)  += value;
  GET_COND(ch,condition) = MAX(0,GET_COND(ch,condition));
  GET_COND(ch,condition) = MIN(36,GET_COND(ch,condition));
  if(GET_COND(ch,condition)) {
    if(GET_COND(ch,condition)<5) {
      switch(condition)	{
      case FULL :
	send_to_char_han("Your stomach will be empty soon.\n\r",
	   "약간 배가 고파오기 시작합니다.\n\r",
	   ch );
	return;
      case THIRST :
	send_to_char_han("You need some water now.\n\r",
			 "약간 목이 말라오기 시작합니다.\n\r",
			 ch );
	return;
      case DRUNK :
	send_to_char_han("You feel you begin to be sober.\n\r",
			 "조금 술이 깨는것 같습니다.\n\r",
			 ch);
	return;
      default:
	return;
      }
    }
    return;
  }
  
  switch(condition){
  case FULL :
    {
      send_to_char("You are hungry.\n\r",ch);
      ch->points.hit *= 9;
      ch->points.hit /= 10;
      return;
    }
  case THIRST :
    {
      send_to_char("You are thirsty.\n\r",ch);
      ch->points.hit *= 9;
      ch->points.hit /= 10;
      return;
    }
  case DRUNK :
    {
      if(intoxicated)
        send_to_char("You are now sober.\n\r",ch);
      return;
    }
  default : break;
  }
  
}


void check_idling(struct char_data *ch)
{
  if (++(ch->specials.timer) > 7) {
    if (ch->specials.was_in_room == NOWHERE && ch->in_room != NOWHERE) {
      ch->specials.was_in_room = ch->in_room;
      if (ch->specials.fighting) {
        stop_fighting(ch->specials.fighting);
        stop_fighting(ch);
      }
      act("$n disappears into the void.", TRUE, ch, 0, 0, TO_ROOM);
      send_to_char("You have been idle, and are pulled into a void.\n\r", ch);
      char_from_room(ch);
      char_to_room(ch, 1);  /* Into room number 0 */
    } 
    else if (ch->specials.timer >= 35) {
      do_rent(ch,0,"");
      if (ch->desc)
        close_socket(ch->desc);
    }
  }
}

/* Update both PC's & NPC's and objects*/
void point_update( void )
{  
  void update_char_objects( struct char_data *ch ); /* handler.c */
  void extract_obj(struct obj_data *obj); /* handler.c */
  struct char_data *i, *next_dude;
  struct obj_data *j, *next_thing, *jj, *next_thing2;
  char buf[100];
  extern int level_quest[];
  
  /* characters */
  for (i = character_list; i; i = next_dude) {
    next_dude = i->next;
    if (GET_POS(i) > POSITION_STUNNED) {
      GET_HIT(i)  = MIN(GET_HIT(i)  + hit_gain(i),  hit_limit(i));
      GET_MANA(i) = MIN(GET_MANA(i) + mana_gain(i), mana_limit(i));
      GET_MOVE(i) = MIN(GET_MOVE(i) + move_gain(i), move_limit(i));
    } else if (GET_POS(i) == POSITION_STUNNED) {
      GET_HIT(i)  = MIN(GET_HIT(i)  + hit_gain(i),  hit_limit(i));
      GET_MANA(i) = MIN(GET_MANA(i) + mana_gain(i), mana_limit(i));
      GET_MOVE(i) = MIN(GET_MOVE(i) + move_gain(i), move_limit(i));
      update_pos( i );
    } else if  (GET_POS(i) == POSITION_INCAP)
      damage(i, i, 1, TYPE_SUFFERING);
    else if (!IS_NPC(i) && (GET_POS(i) == POSITION_MORTALLYW))
      damage(i, i, 2, TYPE_SUFFERING);
    if (!IS_NPC(i)) {
      update_char_objects(i);
      if (GET_LEVEL(i) < IMO)
	check_idling(i);
    }
    gain_condition(i,FULL,-1);
    gain_condition(i,DRUNK,-1);
    gain_condition(i,THIRST,-1);
    
    /* auto level up by Perhaps */
    if((titles[GET_CLASS(i)-1][(int) GET_LEVEL(i)+1].exp+1000)<GET_EXP(i)&&!IS_NPC(i)
       &&(GET_LEVEL(i)<40) && (GET_QUEST_SOLVED(i)>=level_quest[(int) GET_LEVEL(i)])){
      GET_LEVEL(i)++;
      advance_level(i, 1);
      sprintf(buf,"\n\r %s LEVEL UP !! ---==Congratulations==--- \n",i->player.name);
      send_to_all(buf);
    }
    
    /* auto level down by Perhaps */
    /* remove level-down */
    /*
      if( (
      titles[GET_CLASS(i)-1][GET_LEVEL(i)].exp > GET_EXP(i) 
      &&!IS_NPC(i)
      &&(GET_LEVEL(i)>5)&&(GET_LEVEL(i)<=40)
      ) && (GET_GUILD(i)==0) ) {
      advance_level(i, 0);
      GET_LEVEL(i)--;
      sprintf(buf,"\n\r %s LEVEL DOWNED!! <--==Congratulations?!?!==--> \n",i->player.name);
      send_to_all(buf);
      }
      */
    
  } /* for */
  
  /* objects */
  for(j = object_list; j ; j = next_thing){
    next_thing = j->next; /* Next in object list */
    
    /* If this is a corpse */
    if ( (GET_ITEM_TYPE(j) == ITEM_CONTAINER) && (j->obj_flags.value[3]) ) {
      /* timer count down */
      if (j->obj_flags.timer > 0) j->obj_flags.timer--;
      
      if (!j->obj_flags.timer) {
	
        if (j->carried_by)
          act("$p decay in your hands.", FALSE, j->carried_by, j, 0, TO_CHAR);
        else if ((j->in_room != NOWHERE) && (world[j->in_room].people)){
          act("A quivering hoard of maggots consume $p.",
	      TRUE, world[j->in_room].people, j, 0, TO_ROOM);
          act("A quivering hoard of maggots consume $p.",
	      TRUE, world[j->in_room].people, j, 0, TO_CHAR);
        }
	
        for(jj = j->contains; jj; jj = next_thing2) {
          next_thing2 = jj->next_content; /* Next in inventory */
          obj_from_obj(jj);
	  
	  /* keep items in PC's corpse, changed by jhpark */
	  if((GET_ITEM_TYPE(j)==ITEM_CONTAINER) && (j->obj_flags.value[3]==2)) {
	    if (j->in_obj)  obj_to_obj(jj,j->in_obj);
	    else if (j->carried_by) obj_to_room(jj,j->carried_by->in_room);
	    else if (j->in_room != NOWHERE) obj_to_room(jj,j->in_room);
	  }
	  else {
	    if((GET_ITEM_TYPE(jj)==ITEM_CONTAINER) && (jj->obj_flags.value[3])) {
	      if (j->in_obj) obj_to_obj(jj,j->in_obj);
	      else if (j->carried_by) obj_to_room(jj,j->carried_by->in_room);
	      else if (j->in_room != NOWHERE) obj_to_room(jj,j->in_room);
	    }
	  }
	}
        extract_obj(j);
      }
    }
  }
}
