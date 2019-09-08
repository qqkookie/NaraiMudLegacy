/*
GUILD_SKILLS.C
made by wdshin@eve.kaist.ac.kr
*/
#include <stdio.h>
#include <assert.h>
#include "structs.h"
#include "utils.h"
#include "comm.h"
#include "spells.h"
#include "handler.h"
#include "limit.h"
#include "db.h"

#include "guild_list.h"

/* Extern structures */
extern struct room_data *world;
extern struct obj_data  *object_list;
extern struct char_data *character_list;
extern struct index_data *mob_index;
extern struct descriptor_data *descriptor_list;

extern char *guild_names[];

void damage(struct char_data *ch, struct char_data *victim,
	    int damage, int weapontype);
char *one_argument(char *str, char *first);
void do_say(struct char_data *ch, char *str, int cmd);
int number(int from, int to);
void die(struct char_data *ch, int level, struct char_data *who);
void do_look(struct char_data *ch, char *arg, int cmd);
int MAX(int a, int b);
void stop_fighting(struct char_data *ch);
void group_gain(struct char_data *ch, struct char_data *victim);
void gain_exp(struct char_data *ch, int gain);
void change_alignment(struct char_data *ch, struct char_data *victim);


/* skills of police */
void do_power_bash(struct char_data *ch,char *argument,int cmd)
{
  char name[256];
  struct char_data *victim;
  int percent;
  
  /* remove guild skills by atre */
  return ;
  
  if(GET_GUILD(ch)!=POLICE&&GET_LEVEL(ch)<IMO) {
    return;
  }
  
  one_argument(argument,name);
  if(!(victim=get_char_room_vis(ch,name))) {
    if(ch->specials.fighting) {
      victim=ch->specials.fighting;
    }
    else {
      send_to_char("power-bash who?\n\r",ch);
      return;
    }
  }
  
  if(victim==ch || IS_NPC(victim)) {
    do_say(ch,"HAHAHA...I am babo!!!", 0);
    return;
  }
  percent=number(1,130);
  if( percent < GET_GUILD_SKILL(ch, POLICE_SKILL_POWER_BASH)) {
    if(GET_MANA(ch)<100) {
      send_to_char("You do not have enough mana!",ch);
      return;
    }
    damage(ch,victim,GET_GUILD_SKILL(ch,POLICE_SKILL_POWER_BASH),SKILL_BASH);
    if( percent < GET_GUILD_SKILL(ch,POLICE_SKILL_POWER_BASH) )
      GET_POS(victim)=POSITION_STUNNED;
    if(victim) 
      WAIT_STATE(victim,
		 (GET_GUILD_SKILL(ch,POLICE_SKILL_POWER_BASH)/20)
		 * PULSE_VIOLENCE);
  }
  else {
    GET_MOVE(ch)-=1000;
    if(ch) WAIT_STATE(ch,PULSE_VIOLENCE*2);
    return;	
  }
  if(ch)
    WAIT_STATE(ch,
	       (GET_GUILD_SKILL(ch,POLICE_SKILL_POWER_BASH)/20)
	       * PULSE_VIOLENCE);
}
/* victim의 장소를 broadcast하고 flag을 붙인다. */
/* flag이 붙었을때 죽으면 flag이 없어진다 */
void do_whistle(struct char_data *ch,char *argument,int cmd)
{
  char name[256];
  char buf[256];
  struct char_data *victim;
  struct descriptor_data *d;
  
  /* remove guild skills by atre */
  return ;
  
  if(GET_GUILD(ch)!=POLICE&&GET_LEVEL(ch)<IMO) {
    return;
  }
  one_argument(argument,name);
  
  if (!(victim = get_char_room_vis(ch, name))) {
    if (ch->specials.fighting) {
      victim = ch->specials.fighting;
    } 
    else {
      send_to_char("whistle who?\n\r", ch);
      return;
    }
  }
  
  if(IS_NPC(victim)||victim==ch)
    return;
  
  sprintf(buf,
	  "%s>>>>Emergency!!! %s(%s) is in %s(%d)!!!!!\n\r",
	  GET_NAME(ch),
	  GET_NAME(victim),
	  GET_GUILD_NAME((int) GET_GUILD(victim)),
	  world[victim->in_room].name,world[victim->in_room].number);

  for(d=descriptor_list;d;d=d->next) {
    if( d->character && (d->connected==CON_PLYNG)&&
	(d->character->in_room != NOWHERE) && 
	d->character->player.guild == POLICE ) {
      send_to_char(buf,d->character);
      GET_MANA(ch)-=10;
    }
  }
  if(!IS_SET(victim->specials.act,PLR_CRIMINAL)&&GET_MANA(ch)>1000) {
    SET_BIT(victim->specials.act,PLR_CRIMINAL);
    send_to_char("flag set!!\n\r",ch);
    GET_MANA(ch)-=1000;
  }
}
/* 방안에 있는 경찰들이 한대씩 전부 때림 */
void do_simultaneous(struct char_data *ch,char *argument,int cmd)
{
  char name[256];
  struct char_data *victim,*tmp_victim,*vict;
  int dam=0;
  
  /* remove guild skills by atre */
  return ;
  
  one_argument(argument,name);
  if(GET_GUILD(ch)!=POLICE&&GET_LEVEL(ch)<IMO) {
    return;
  }
  
  if (!(victim = get_char_room_vis(ch, name))) {
    if (ch->specials.fighting) {
      victim = ch->specials.fighting;
    } 
    else {
      send_to_char("simultaneous who?\n\r", ch);
      return;
    }
  }
  if(victim==ch)
    return;
  for(vict=world[ch->in_room].people;vict;vict=tmp_victim) {
    tmp_victim=vict->next_in_room;

    if( (vict && 
	 !IS_NPC(vict) && 
	 GET_GUILD(vict)==POLICE&&GET_GUILD_SKILL(vict,POLICE_SKILL_SIMULTANEOUS)>number(1,99)
	 && GET_MANA(vict)>180) 
	|| (!IS_NPC(vict)&&GET_LEVEL(vict)>IMO-1)) {

      dam=GET_INT(vict)*GET_GUILD_SKILL(vict,POLICE_SKILL_SIMULTANEOUS);

      if(victim)
	damage(vict,victim,dam,TYPE_HIT);

      do_say(vict,"Here I go!!!", 0);

      if(victim)
	act("$n found $N's a weak point!!",TRUE,vict,0,victim,TO_ROOM);
      GET_MANA(vict)-=20*GET_INT(victim);
      if(vict)
	WAIT_STATE(vict,PULSE_VIOLENCE/3);
      if(victim) 
	WAIT_STATE(victim,PULSE_VIOLENCE/3);
    }
  }
}
void do_arrest(struct char_data *ch,char *argument,int cmd)
{
  char name[256];
  struct char_data *victim;
  struct affected_type af;
  char buf[256];
  int	exp;
  
  /* remove guild skills by atre */
  return ;
  
  one_argument(argument,name);
  if(GET_GUILD(ch)!=POLICE&&GET_LEVEL(ch)<IMO)
    return;
  
  if (!(victim = get_char_room_vis(ch, name))) {
    if (ch->specials.fighting) {
      victim = ch->specials.fighting;
    } 
    else {
      send_to_char("arrest who?\n\r", ch);
      return;
    }
  }
#define JALE_ROOM	1800
  if(victim==ch)
    return;
  if( ((GET_GUILD_SKILL(ch,POLICE_SKILL_ARREST)>number(1,99)&&
	GET_HIT(victim)<GET_PLAYER_MAX_HIT(victim)/8)&&GET_MANA(ch)>500 )|| 
      GET_LEVEL(ch) > (IMO-1)) {
    do_say(ch,"YOU MUST SUFFER IN THE JALE!!!!", 0);
    act("$n disappears suddenly to JALE!!!!!!",TRUE,victim,0,0,TO_ROOM);
    char_from_room(victim);
    char_to_room(victim,real_room(JALE_ROOM));
    if ( !IS_NPC(victim) ) {
      sprintf(buf,"%s was sended to the JALE room!!!!\n\r",
	      GET_NAME(victim));
      send_to_all(buf);
      af.type = SKILL_ARREST;
      af.duration = 44;
      af.modifier = 0;
      af.location = APPLY_NONE;
      af.bitvector = AFF_ARREST;
      affect_to_char(victim,&af);
    }
    else {
      sprintf(buf,"%s died in the JALE room!!!!\n\r",GET_NAME(victim));
      send_to_all(buf);
      if (IS_AFFECTED(ch, AFF_GROUP))
	group_gain(ch, victim);
      else { 
	/* Calculate level-difference bonus? */
	exp = GET_EXP(victim);
	exp = MAX(exp, 1);
	gain_exp(ch, exp);
	GET_GOLD(ch) += GET_GOLD(victim);
	change_alignment(ch, victim);
      }
      die(victim, GET_LEVEL(ch),ch);
    }
    do_look(ch,"",15);
    GET_HIT(ch) *= 9/10;
    return;
  }
  GET_MANA(ch)-=100;
  send_to_char("You failed to arrest him!!!\n\r",ch);
}

/* skills of outlaws */
void do_charge(struct char_data *ch,char *argument,int cmd)
{
  char name[256];
  struct char_data *victim;
  int dam;
  int use;
  int level_dif;
  
  /* remove guild skills by atre */
  return ;
  
  if ((GET_GUILD(ch) != OUTLAW && GET_LEVEL(ch) < IMO) || IS_NPC(ch))
    return;
  if (!(victim = get_char_room_vis(ch, name))) {
    if (ch->specials.fighting) {
      victim = ch->specials.fighting;
    }
    else {
      send_to_char("Charge who?\n\r", ch);
      return;
    }
  }
  if (victim == ch) {
    send_to_char("HUK! Really?", ch);
    return;
  }
  
  if (!IS_NPC(victim)&&!IS_NPC(ch)&&GET_LEVEL(ch)<IMO) {
    send_to_char("You can't use charge to player\n\r",ch);
    return;
  }
  level_dif = GET_LEVEL(victim) - GET_LEVEL(ch);
  if (level_dif <= 0)
    level_dif = 1;
  level_dif <<= 4;
  
  if (number(1, 200) + level_dif < GET_GUILD_SKILL(ch, OUTLAW_SKILL_CHARGE)) {
    if (GET_MANA(ch) < 5000 || GET_MOVE(ch) < 5000)
      return;
    dam = GET_HIT(ch) + GET_MANA(ch) + GET_MOVE(ch);
    dam = number(dam >> 1, dam << 1);
    do_say(ch, "CHARGE!!!!!", 0);
    do_say(victim, "HUK!!!!", 0);
    act("$n charged $N!!!", TRUE, ch, 0, victim, TO_ROOM);
    act("$n crawls in the ground.", TRUE, victim, 0, 0, TO_ROOM);
    damage(ch, victim, dam, TYPE_UNDEFINED);
    use = 5000;
    use = number(use >> 1, use);
    GET_MANA(ch) -= use;
    GET_MOVE(ch) -= use;
    WAIT_STATE(ch, 2 * PULSE_VIOLENCE);
  }
  else {
    dam = GET_MOVE(victim);
    GET_POS(ch) = POSITION_STUNNED;
    do_say(ch, "JJUP!!!!", 0);
    do_say(victim, "HAHAHA!!!", 0);
    act("$n failed to CHARGE $N!!!", TRUE, ch, 0, victim, TO_ROOM);
    GET_MANA(ch) -= 100;
    damage(victim, ch, dam, TYPE_UNDEFINED);
    WAIT_STATE(ch, PULSE_VIOLENCE);
  }
}

/* just damage */
void do_angry_yell(struct char_data *ch,char *argument,int cmd)
{
  char name[256];
  struct char_data *victim;
  int dam;
  
  /* remove guild skills by atre */
  return ;
  
  one_argument(argument,name);
  
  if ((GET_GUILD(ch) != OUTLAW && GET_LEVEL(ch) < IMO) || IS_NPC(ch))
    return;
  if (!(victim = get_char_room_vis(ch, name))) {
    if (ch->specials.fighting) {
      victim = ch->specials.fighting;
    }
    else {
      send_to_char("yell at who?\n\r",ch);
      return;
    }
  }
  if (victim == ch)
    return;
  dam = GET_LEVEL(ch) - GET_LEVEL(victim);
  dam = MAX(1, dam);
  dam *= (GET_CON(ch) + GET_STR(ch) + GET_DEX(ch)) -
    (GET_INT(ch) + GET_WIS(ch)) +
    GET_GUILD_SKILL(ch, OUTLAW_SKILL_ANGRYYELL);
  if (GET_GUILD_SKILL(ch, OUTLAW_SKILL_ANGRYYELL) > number(10, 150)) {
    if (GET_MANA(ch) < 1000) {
      send_to_char("You do not have enough mana!",ch);
      return;
    }
    dam *= GET_GUILD_SKILL(ch, OUTLAW_SKILL_ANGRYYELL) >> 4;
    act("$n yells YOK at $N angrilly!!!",TRUE,ch,0,victim,TO_ROOM);
    do_say(ch,"YOKYOKYOKYOKYOKYOK!!!!!!!!!!!!!!!!", 0);
    damage(ch,victim,dam,TYPE_UNDEFINED);
    GET_POS(victim) = POSITION_STUNNED;
    GET_MANA(ch) -= dam >> 1;
    GET_HIT(ch) += dam >> 1;
    WAIT_STATE(ch, PULSE_VIOLENCE);
  }
  else {
    send_to_char("Get the fuck out!!!\n\r",ch);
    GET_HIT(ch) -= dam >> 1;
    GET_MANA(ch) -= dam;
  }
}

void do_smoke(struct char_data *ch,char *argument,int cmd)
{
}

void do_inject(struct char_data *ch,char *argument,int cmd)
{
}

/* skills of assasins */
void do_shadow(struct char_data *ch,char *argument,int cmd)
{
  struct affected_type af;
  
  /* remove guild skills by atre */
  return ;
  
  if(GET_GUILD(ch)!=ASSASSIN && GET_LEVEL(ch) < IMO) {
    send_to_char("You are not assasin!!!\n\r",ch);
    return;
  }
  if(GET_MANA(ch)<1000 && GET_LEVEL(ch) < IMO) {
    send_to_char("Your mana isn't enough to make a shadow!!!\n\r",ch);
    return;
  }
  if(!affected_by_spell(ch,SPELL_SHADOW_FIGURE)) {
    act("$n disappears suddenly!!!",TRUE,ch,0,0,TO_ROOM);
    act("Now,They see your shadow only.",TRUE,ch,0,0,TO_CHAR);
    af.type=SPELL_SHADOW_FIGURE;
    af.duration=5;
    af.modifier=0;
    af.location=APPLY_NONE;
    af.bitvector=AFF_SHADOW_FIGURE;
    affect_to_char(ch,&af);
  }
  GET_MANA(ch)-=(20*(100-GET_GUILD_SKILL(ch,ASSASSIN_SKILL_SHADOW_FIGURE)));
}
/* 싸움 도중 hp가 1/6 일때 자신의 길드로 돌아감 */
void do_solace(struct char_data *ch,char *argument,int cmd)
{
  int home=ROOM_GUILD_ASSASSIN,location;
  bool found=FALSE;	
  extern int top_of_world;
  
  /* remove guild skills by atre */
  return ;
  
  if((GET_GUILD(ch)!=ASSASSIN&&GET_LEVEL(ch)<IMO)||IS_NPC(ch))
    return;
  for(location=0;location<=top_of_world;location++) {
    if(world[location].number == home ) {
      found=TRUE;
      break;
    }
  }
  if((location==top_of_world)||!found) {
    send_to_char("You cannot get the chance!!!\n\r",ch);
    return;
  }
  if( ( GET_GUILD_SKILL(ch,ASSASSIN_SKILL_SOLACE) > number(0,100) &&
	GET_HIT(ch) < (GET_PLAYER_MAX_HIT(ch)/7) )
      || GET_LEVEL(ch)>=IMO) {
    do_say(ch,"HAHAHAHAHAH!!! BYEBYE!!!!!", 0);
    act("$n disappears.",TRUE,ch,0,0,TO_ROOM);
    if(ch->specials.fighting) {
      stop_fighting(ch->specials.fighting);
      stop_fighting(ch);
    }
    char_from_room(ch);
    char_to_room(ch,location);
    act("$n appears in the middle of the room.",TRUE,ch,0,0,TO_ROOM);
    do_look(ch,"",15);
    GET_MOVE(ch)=0;
  }
  else {
    send_to_char("But,You are healthy enought to fight!!!\n\r",ch);
    GET_MOVE(ch)-=100;
  }
}

void do_evil_strike(struct char_data *ch,char *argument,int cmd)
{
  int dam,percent;
  char name[256];
  struct char_data *victim;
  
  /* remove guild skills by atre */
  return ;
  
  one_argument(argument,name);
  
  if((GET_GUILD(ch)!=ASSASSIN&&GET_LEVEL(ch)<IMO)||IS_NPC(ch))
    return;
  if(!(victim=get_char_room_vis(ch,name))) {
    if(ch->specials.fighting) {
      victim=ch->specials.fighting;
    }
    else {
      send_to_char("evil strike who?\n\r",ch);
      return;
    }
  }
  if(ch==victim) {
    do_say(ch,"Wahhh...I cannot use on mobs!!", 0);
    return;
  }
  if(!IS_EVIL(ch)) {
    send_to_char("Huh! You are not evil!!",ch);
    return;
  }
  /* you must think carefully what this code means */
  percent = GET_ALIGNMENT(ch) - GET_ALIGNMENT(victim);
  percent >>= 3;
  percent += number(1, 500);
  
  dam = GET_ALIGNMENT(ch);
  dam = MAX(dam, -dam);
  dam *= number(1, GET_LEVEL(ch) - 20);
  
  if(percent < GET_GUILD_SKILL(ch, ASSASSIN_SKILL_EVIL_STRIKE)) {
    if (GET_MANA(ch) < 1000) {
      send_to_char("You do not have enough mana.",ch);
      return;
    }
    act("You grin evilly and strike with $s DEVIL POWER!!!",TRUE,
	ch,0,0,TO_CHAR);
    act("$n grins evilly and strike with $s DEVIL POWER!!!",TRUE,
	ch,0,ch,TO_ROOM);
    
    WAIT_STATE(victim, PULSE_VIOLENCE);
    damage(ch, victim, dam, TYPE_UNDEFINED);
    GET_MANA(ch) -= 1000 -
      (GET_GUILD_SKILL(ch, ASSASSIN_SKILL_EVIL_STRIKE) << 2);
  }
  else {
    act("You could not summon $s POWER!!",TRUE,ch,0,0,TO_CHAR);
    act("$n could not summon $s POWER!!!",TRUE,ch,0,ch,TO_ROOM);
    GET_HIT(victim) += dam >> 2;
  }
}
