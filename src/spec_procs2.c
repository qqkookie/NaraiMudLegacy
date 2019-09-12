
/* ************************************************************************
*  file: spec_procs.c , Special module.                   Part of DIKUMUD *
*  Usage: Procedures handling special procedures for object/room/mobile   *
*  Copyright (C) 1990, 1991 - see 'license.doc' for complete information. *
************************************************************************* */

#define FUDGE (100+dice(6,20))

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
#include "limit.h"
#include "mob_magic.h"		/* cyb */

/*   external vars  */

extern struct room_data *world;
extern struct char_data *character_list;
extern struct descriptor_data *descriptor_list;
extern struct index_data *obj_index;
extern struct time_info_data time_info;
extern struct title_type titles[4][IMO+4];
extern struct index_data *mob_index;

/* extern procedures */

void log(char *str);
void hit(struct char_data *ch, struct char_data *victim, int type);
void gain_exp(struct char_data *ch, int gain);
void stop_fighting(struct char_data *ch);
void set_title(struct char_data *ch);
int number(int from, int to);
void do_say(struct char_data *ch, char *str, int cmd);
void die(struct char_data *ch, int level, struct char_data *who);
int dice(int num, int size);
void damage(struct char_data *ch, struct char_data *victim, int dam, int type);
void wear(struct char_data *ch, struct obj_data *o, int keyword);
void shoot(struct char_data *ch, struct char_data *victim, int type);
void add_follower(struct char_data *ch, struct char_data *leader);
int str_cmp(char *arg1, char *arg2);
int find_name(char *name);
int MIN(int a, int b);
void list_obj_to_char(struct obj_data *list,struct char_data *ch, int mode,
  bool show);
void list_char_to_char(struct char_data *list, struct char_data *ch, int mode);
void do_kick(struct char_data *ch, char *arg, int cmd);
void do_bash(struct char_data *ch, char *arg, int cmd);

void do_start(struct char_data *ch);

void cast_cure_light(byte level,struct char_data *ch,char *arg,int type,
    struct char_data *victim, struct obj_data *tar_obj );
void cast_cure_critic(byte level,struct char_data *ch,char *arg,int type,
    struct char_data *victim, struct obj_data *tar_obj );
void cast_heal(byte level, struct char_data *ch, char *arg, int type,
    struct char_data *tar_ch, struct obj_data *tar_obj );
void cast_full_heal(byte level, struct char_data *ch, char *arg, int type,
    struct char_data *tar_ch, struct obj_data *tar_obj );
void cast_sunburst(byte level, struct char_data *ch, char *arg, int type,
    struct char_data *victim, struct obj_data *tar_obj );
void cast_fireball(byte level, struct char_data *ch, char *arg, int type,
    struct char_data *victim, struct obj_data *tar_obj );
void cast_color_spray(byte level, struct char_data *ch, char *arg, int type,
    struct char_data *victim, struct obj_data *tar_obj );
void cast_all_heal( byte level, struct char_data *ch, char *arg, int si,
    struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_corn_of_ice( byte level, struct char_data *ch, char *arg, int si,
    struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_sanctuary( byte level, struct char_data *ch, char *arg, int si,
    struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_haste( byte level, struct char_data *ch, char *arg, int si,
    struct char_data *tar_ch, struct obj_data *tar_obj);


/* ********************************************************************
*  Special procedures for mobiles                                      *
******************************************************************** */

int puff(struct char_data *ch, int cmd, char *arg)
{
  if (cmd)
    return(0);

  switch (number(0, 60)) {
    case 0:
      do_say(ch, "My god! It's full of stars!", 0);
       return(1);
    case 1:
      do_say(ch, "How'd all those fish get up here?", 0);
      return(1);
    case 2:
      do_say(ch, "I'm a very female dragon.", 0);
      return(1);
    case 3:
      do_say(ch, "I've got a peaceful, easy feeling.", 0);
      return(1);
    default:
      return(0);
  }
}

int super_deathcure(struct char_data *ch, int cmd, char *arg)
{
  struct char_data *vict,*mob;
  struct char_data *choose_victim(struct char_data *mob, int fightmode,
    int mode) ;

  int h,real_number,ran_num;
  if(cmd)
    return(0);
  /* cyb vict=world[ch->in_room].people; */
  vict = choose_victim(ch, VIC_ALL, MODE_MOVE_MAX) ;

    h=GET_HIT(ch);
    if(h < 5000)
    cast_full_heal(GET_LEVEL(ch), ch, "", SPELL_TYPE_SPELL, ch, 0);
  if(vict && !IS_NPC(vict)){
    if(number(1,10)>7){
      act("$n control your move -100.",1,ch,0,0,TO_ROOM);
	vict->points.move=0;vict->points.move-=200;
      act("$n Wuhhhhhh...... thank you very much ? .",1,ch,0,0,TO_ROOM);	
	return TRUE;	
    }
    else if(number(1,10)>9){
	act("$n hmmmmm.... miracle !!!",1,ch,0,0,TO_ROOM);	
	vict->points.mana+=40*(1+GET_LEVEL(vict)/9);
	return TRUE;
    }	
  }
	vict=ch->specials.fighting;
	if(vict){
           h=number(0,12);
           if(h>7) return(0);
  	  switch(h){
              case 0:ran_num=5107;break;  /* mother */
              case 1:ran_num=5010;break;  /* dracolich */
              case 2:ran_num=5004;break;  /* worm */
              case 3:ran_num=2109;break;  /* medusa third gorgon */
              case 4:ran_num=2118;break;  /* hecate */
              case 5:
              case 6:ran_num=15107;break; /* semi robot : cyb */
              case 7:
              case 8:ran_num=16014;break; /* head soldier wee : cyb */
              default:return(1);
	}
	if((real_number=real_mobile(ran_num))) {
	    act("$n utters 'Em pleh! Em pleh!! Em pleh!!!",1,ch,0,0,TO_ROOM);	
	    mob=read_mobile(real_number,REAL);
	    char_to_room(mob,ch->in_room);
	}
	return(1);
	}
	return(0);
}

int deathcure(struct char_data *ch, int cmd, char *arg)
{
  struct char_data *vict;
  int h;

  if(cmd)
    return(0);
  vict=world[ch->in_room].people;
    h=GET_HIT(ch);
    if(h < 4000)
    cast_full_heal(GET_LEVEL(ch), ch, "", SPELL_TYPE_SPELL, ch, 0);
  if (GET_POS(ch) == POSITION_FIGHTING)
    return(TRUE);
  if(vict->equipment[WEAR_ABOUTLEGS]){
    if(obj_index[vict->equipment[WEAR_ABOUTLEGS]->item_number].virtual==1317){
    /* ANTI deathcure */
      act("$n tries to do something to you, but failed miserably by ANTI deathcure."
      ,1,ch,0,0,TO_ROOM);
      return(TRUE);
    }
  }
  if(vict && !IS_NPC(vict)){
    if(number(1,10)>7){
      act("$n control your body ZERO.",1,ch,0,0,TO_ROOM);
      vict->points.hit=1;
	vict->points.move=0;
      act("$n Wuhhhhhh...... thank you very much ? .",1,ch,0,0,TO_ROOM);	
	return TRUE;	
    }
else if(number(1,10)>8){
      vict->points.hit+=60*(1+GET_LEVEL(vict)/7);
	act("$n Wuhhhhhh...... that's too bad ! ? .",1,ch,0,0,TO_ROOM);	
      return TRUE;
    }
else if(number(1,10)>5){
	act("$n control your mana ZERO.",1,ch,0,0,TO_ROOM);
	vict->points.mana=0;
        act("$n hmmmmm.... great miracle !!! ",1,ch,0,0,TO_ROOM);
	return TRUE;
    }
else if(number(1,10)>9){
	act("$n hmmmmm.... miracle !!!",1,ch,0,0,TO_ROOM);	
	vict->points.mana+=40*(1+GET_LEVEL(vict)/9);
	return TRUE;
    }	
  }
  switch (number(0,40)){
    case 0:
    do_say(ch, "Chouchouchouchouzzzzzaaaa. .....", 0);
    return(1);
    case 1:
    do_say(ch, "HaHaHa ......", 0);
    return(1);
    case 2:
    do_say(ch, "You are insect me !! ......", 0);
    return(1);
    case 3:
    do_say(ch, "if you give me 50000 coins , i forgive me your crime ...", 0);
    return(1);
    case 4:
    do_say(ch, "I can make your body Zero ! ", 0);
    return(1);
    case 5:
    do_say(ch, "I can heal your body until 100 ! ", 0);
    return(1);
    case 6:
    do_say(ch, "Please, allow me to make your body ZERO.", 0);
    return(1);
    case 7:
    do_say(ch, "I admire Dangun the greatgod !",0);
    return(1);
    case 8:
    act("$n massacres you to small fragment with his slash .",1,ch,0,0,TO_ROOM);
    act("$n massacres you to small fragment with his slash .",1,ch,0,0,TO_ROOM);
    act("$n massacres you to small fragment with his slash .",1,ch,0,0,TO_ROOM);
    return(1);
    case 9:
    act("$n massacres you to small fragment with his slash .",1,ch,0,0,TO_ROOM);
    act("$n massacres you to small fragment with his slash .",1,ch,0,0,TO_ROOM);
    return(1);
    case 10:
    act("$n try to steal your inventory .",1,ch,0,0,TO_ROOM);
    return(1);
    default:
    return(0);
  }
}

int perhaps(struct char_data *ch,int cmd,char *arg)
{
	struct char_data *vict,*next;
#ifdef 	MID_HELPER
	static struct char_data *perhaps = NULL;
	char buf[256];
	char *blessings[3] = {
		"%s님에게 신의 가호가 있기를...",
		"%s님에게 축복을 드립니다...",
		"%s님에게 신의 은총이 내리기를..." ,
	};
	while(*arg==' ') arg++;
	if (cmd == 98 && strcasecmp(arg, MID_HELPER)==0 	// bow narai 
		&& strcasecmp(GET_NAME(ch), MID_HELPER) != 0) { 
            cast_sanctuary(GET_LEVEL(perhaps), perhaps, NULL, 
		SPELL_TYPE_SPELL, ch, NULL);
            cast_haste(GET_LEVEL(perhaps), perhaps, NULL, 
		SPELL_TYPE_SPELL, ch, NULL);
	   
	    char *msg =  blessings[number(0,2)];
	    sprintf(buf, msg , GET_NAME(ch));
            do_say(perhaps, buf, 0);
            return 0;
        }
	if (perhaps == NULL && cmd == 0 && ch != NULL
            && mob_index[ch->nr].virtual == 1000 ) 
            perhaps  = ch;
#endif
	if(cmd) return(0); /* If void return */
	if(!ch) return(0);
	for(vict=world[ch->in_room].people;vict
		;vict=next) {
		next = vict->next_in_room;
		if( vict&&((vict->points.max_hit/9)>vict->points.hit)) {
			act("$n kisses $N on $s cheek.",1,ch,0,vict,TO_ROOM);
			vict->points.hit=vict->points.max_hit;
		}
		if( vict&&IS_NPC(vict)&&ch!=vict)
		{
			die(vict, GET_LEVEL(ch), ch);
			continue;
		}
	/*
		if( vict&&((vict->points.armor)>1) )
		{
			act("$n hugs $N.",1,ch,0,vict,TO_ROOM);
			vict->points.armor=1;
		}
		if( vict&&((vict->points.hitroll) < GET_LEVEL(vict))&&GET_LEVEL(vict)<IMO )
		{
			act("$n pats $N.",1,ch,0,vict,TO_ROOM);
			vict->points.hitroll=GET_LEVEL(vict);
		}
		if( vict&&((vict->points.damroll) < GET_LEVEL(vict))&&GET_LEVEL(vict)<IMO )
		{
			act("$n raises $s POWER!!!",1,ch,0,vict,TO_ROOM);
			vict->points.damroll=GET_LEVEL(vict);
		}
	*/
	}
	switch(number(0,500))
	{
	case 1:
		do_say(ch,"나래 머드에 오신걸 환영합니다.", 0);
		return 1;
	case 2:
		do_say(ch,"나래 머드에 관한 의견이 있으면 board에 써주세요.", 0);
		return 1;
	case 3:
		do_say(ch,"버그가 발견되면 보드에 적어주세요. 사례를 드립니다.", 0);
		return 1;
	case 4:
		do_say(ch,"길드에 가입하면 level-down이 안된답니다.", 0);
		return 1;
	case 5:
		do_say(ch,"경찰길드와 깡패길드는 midgaard근처에 있습니다.", 0);
		return 1;
	case 6:
		do_say(ch,"암살자 길드는 DeathKingdom에 있습니다.", 0);
		return 1;
	case 7:
		do_say(ch,"reimburse는 안해준데요...", 0);
		return 1;
	case 8:
		do_say(ch, "NEWS를 자주 보세요...type 'NEWS' ", 0);
		return 1;
	case 9:
		do_say(ch, "초보자 이시면, 'help'를 쳐보세요.", 0);
		return 1;
	default:
		return 1;
	}
}

int Quest_bombard(struct char_data *ch, int cmd, char *arg)
{   /* question function and level up monster */
  void cast_sunburst(byte level, struct char_data *ch, char *arg, int type,
    struct char_data *victim, struct obj_data *tar_obj );
  void cast_fireball(byte level, struct char_data *ch, char *arg, int type,
    struct char_data *victim, struct obj_data *tar_obj );
  struct char_data *vict;
  struct obj_data *list;
  struct affected_type af;
  int bombard_stat=0,maxnum=0;
  int newnum = 0 ;

/* bombard like sword 2702 */
/* bombard room is 3035 and other room is 3094 */
/* if ch->in_room == real_room(3035)  .... */

  if (ch->in_room == real_room(3035)) newnum = 3027 ;
  else if (ch->in_room == real_room(3094)) newnum = 2702 ;
  else newnum = 3027 ;
  
  if(cmd)
    return(0);	
  for(list=ch->carrying;list;list=list->next_content){
     if(obj_index[list->item_number].virtual==3025) maxnum++;
     bombard_stat=1;
  }
  if(maxnum>=7) {
     while((list=ch->carrying)) {
       extract_obj(list);
      }
     if((newnum=real_object(newnum))>=0) { 
                    list=read_object(newnum,REAL);
                    obj_to_char(list,ch);
     			 	wear(ch,list,12);
				}
    return(1); 
  }
  if(maxnum<7 && maxnum>0)
      do_say(ch,"I need more particle.\n\r",0);
  if(ch->master && ch->in_room==ch->master->in_room) {  hit(ch,ch->master,TYPE_UNDEFINED);bombard_stat=1; } 
  vict=ch->specials.fighting;
  if(vict){
   	if(ch->points.hit%2==1 && ch->player.level<35)  
    {     
	    act("$n	get experience .",1,ch,0,0,TO_ROOM);
	    act("$n	LEVEL UP !.",1,ch,0,0,TO_ROOM);
        ch->player.level+=1;
    }
  if(ch->specials.fighting && !affected_by_spell(ch,SPELL_SANCTUARY)){
    act("$n is surrounded by a white aura.",TRUE,ch,0,0,TO_ROOM);
    af.type      = SPELL_SANCTUARY;
    af.duration  = 4;
    af.modifier  = 0;
    af.location  = APPLY_NONE;
    af.bitvector = AFF_SANCTUARY;
    affect_to_char(ch, &af);
    return(1);
    }
    if(GET_LEVEL(ch)==35){ 
           do_say(ch, "Shou Ryu Ken..", 0);
                vict->points.hit-=vict->points.hit/3-number(1,vict->points.hit/8+GET_LEVEL(vict)/2);
           send_to_char("당신은 꽈당 넘어집니다.\n\r",vict);
           send_to_char("크으으아아아아 . .  .  . \n\r",vict);
    }
    if(GET_LEVEL(ch)>= 30){
            act("$n utters the words 'sunburst'.", 1, ch, 0, 0, TO_ROOM);
            cast_sunburst(GET_LEVEL(ch),ch,"",SPELL_TYPE_SPELL,vict,0);
    }
    if(GET_LEVEL(ch)>= 25){
            act("$n utters the words 'fireball'.", 1, ch, 0, 0, TO_ROOM);
            cast_fireball(GET_LEVEL(ch),ch,"",SPELL_TYPE_SPELL,vict,0);
    }
    if(GET_LEVEL(ch)>= 20){
            hit(ch,vict,TYPE_UNDEFINED);
    }
    if(GET_LEVEL(ch)>= 15)
            hit(ch,vict,TYPE_UNDEFINED);
    if(GET_LEVEL(ch)>= 10)
            damage(ch,vict,GET_LEVEL(ch),SKILL_BASH);
    if(GET_LEVEL(ch)>= 5)
            damage(ch,vict,GET_LEVEL(ch),SKILL_KICK);
            	return(1); 
  }
  if(bombard_stat==1) return(1);
 return (FALSE);
}

int mud_message(struct char_data *ch, int cmd, char *arg)
{
  struct char_data *vict;
  if(cmd)
    return(0);	
  vict=ch->specials.fighting;
  if(vict){
	act("$n\tannihilate you with his full power.\n\r",1,ch,0,0,TO_ROOM);
        if ( IS_NPC(vict)) return TRUE ;
	if(vict->points.hit>=51 && vict->points.hit<101)
          damage(ch,vict,dice(14,GET_LEVEL(ch)),TYPE_UNDEFINED);
	if(vict->points.hit>=101 && vict->points.hit<201)
          damage(ch,vict,dice(16,GET_LEVEL(ch)),TYPE_UNDEFINED);
	if(vict->points.hit>=201 && vict->points.hit<401)
          damage(ch,vict,dice(20,GET_LEVEL(ch)),TYPE_UNDEFINED);
	if(vict->points.hit>=401 && vict->points.hit<801)
          damage(ch,vict,dice(25,GET_LEVEL(ch)),TYPE_UNDEFINED);
	if(vict->points.hit>=801 && vict->points.hit<1601)
          damage(ch,vict,dice(28,GET_LEVEL(ch)),TYPE_UNDEFINED);
	if(vict->points.hit>=1601 && vict->points.hit<2001)
          damage(ch,vict,dice(33,GET_LEVEL(ch)),TYPE_UNDEFINED);
	if(vict->points.hit>=2001)
          damage(ch,vict,dice(40,GET_LEVEL(ch)),TYPE_UNDEFINED);
     return(TRUE);
    }
  return(FALSE);
}
int musashi(struct char_data *ch, int cmd, char *arg)
{
  void cast_sunburst(byte level, struct char_data *ch, char *arg, int type,
    struct char_data *victim, struct obj_data *tar_obj );
  struct char_data *choose_victim(struct char_data *mob, int fightmode,
    int mode) ;
  struct char_data *vict, *next_vict;
  struct affected_type af;
  int h,i,musash_mod=0;

  if(cmd)
     return(0);

  /* cyb  vict=world[ch->in_room].people; */
  vict = choose_victim(ch, VIC_ALL, MODE_HIGH_LEVEL) ;

  h=GET_HIT(ch);
  if(h < 2000){
    cast_full_heal(GET_LEVEL(ch), ch, "", SPELL_TYPE_SPELL, ch, 0);
    musash_mod=1;
    }

  if(ch->specials.fighting){
    for (; vict; vict = next_vict) {
          next_vict = vict->next_in_room ;
  	  if(vict->specials.fighting==ch) {
                   hit(ch,vict,TYPE_UNDEFINED);
                   hit(ch,vict,TYPE_UNDEFINED);musash_mod=1;
                   }
          }
    vict=ch->specials.fighting;
	if (!vict) return(1);
    if(number(1,2)==1){
      damage(ch,vict,2*GET_LEVEL(ch),SKILL_KICK);musash_mod=1;
    } else {
      damage(ch,vict,2*GET_LEVEL(ch),SKILL_BASH);musash_mod=1;
    }
  if(ch->specials.fighting && !affected_by_spell(ch,SPELL_SANCTUARY)){
    act("$n 주위로 휘황한 광채가  감쌉니다.",TRUE,ch,0,0,TO_ROOM);
    af.type      = SPELL_SANCTUARY;
    af.duration  = 8;
    af.modifier  = 0;
    af.location  = APPLY_NONE;
    af.bitvector = AFF_SANCTUARY;
    affect_to_char(ch, &af);
    return(1);
  }
    vict=ch->specials.fighting;
	if (!vict) return(1);
   switch (number(0,18)){
    case 0:
      act("$n utters the words 'fire'.", 1, ch, 0, 0, TO_ROOM);
      cast_sunburst(GET_LEVEL(ch),ch,"",SPELL_TYPE_SPELL,vict,0);
      return(1);
    case 1:
      act("$n utters the words 'frost'.", 1, ch, 0, 0, TO_ROOM);
      cast_sunburst(GET_LEVEL(ch),ch,"",SPELL_TYPE_SPELL,vict,0);
      return(1);
    case 2:
      act("$n double attack by double sword method .", 1, ch, 0, 0, TO_ROOM);
      if(ch->specials.fighting) hit(ch,ch->specials.fighting,TYPE_UNDEFINED);
      return(1);
    case 3:
      act("$n double kick by double circle kick .", 1, ch, 0, 0, TO_ROOM);
      damage(ch,vict,3*GET_LEVEL(ch),SKILL_KICK);
      vict=ch->specials.fighting;
      damage(ch,vict,3*GET_LEVEL(ch),SKILL_KICK);
      return(1);
    case 4:
    case 5:
      do_say(ch, "A cha cha cha cha ..", 0);
      act("$n tornado fire with miracle speed .", 1, ch, 0, 0, TO_ROOM);
      for(i=0;i<number(5,8);i++)
	  {
    	  vict=ch->specials.fighting;
    	  cast_sunburst(GET_LEVEL(ch),ch,"",SPELL_TYPE_SPELL,vict,0);
	  }
      return(1);
    case 6:
      do_say(ch, "Yak Yak Yak Yak Ya..", 0);
      act("$n use thousands  kick .", 1, ch, 0, 0, TO_ROOM);
      for(i=0;i<number(10,20);i++)
	  {
			vict=ch->specials.fighting;
			damage(ch,vict,2*GET_LEVEL(ch),SKILL_KICK);
	  }
      return(1);
    case 7:
      do_say(ch, "Heau Heau Heau Heau Heau..", 0);
      act("$n use hundreds bash .", 1, ch, 0, 0, TO_ROOM);
      for(i=0;i<number(10,20);i++)
	  {
			vict=ch->specials.fighting;
			damage(ch,vict,2*GET_LEVEL(ch),SKILL_KICK);
	  }
      return(1);
    case 8:
      do_say(ch, "Ya uuuuu aaaaa    ..", 0);
      act("$n throw powerfull punch ! .", 1, ch, 0, 0, TO_ROOM);
	  damage(ch,vict,80,SKILL_KICK);
      return(1);
    case 9:
    case 10:
    case 11:
    case 12:
      do_say(ch, "Crush Armor    ..", 0);
	  if (!vict) break;
      send_to_char("You feel shrink.\n\r", vict);
      if(!affected_by_spell(vict,SPELL_CRUSH_ARMOR)){
      af.type      = SPELL_CRUSH_ARMOR;
      af.duration  = 8;
      af.modifier  = GET_LEVEL(ch)/10*(-1) ;
      af.location  = APPLY_DAMROLL;
      af.bitvector = 0;
      affect_to_char(vict, &af);
      af.location  = APPLY_AC;
      af.modifier  = GET_LEVEL(ch)+8;
      affect_to_char(vict, &af);
	  }
	  break;
    case 13:
    case 14:
    case 15:
    case 16:
    case 17:
    case 18:
      do_say(ch, "Shou Ryu Ken..", 0);
	  damage(ch,vict,vict->points.hit/3-number(1,vict->points.hit/8+GET_LEVEL(vict)/2),SKILL_BASH);
      send_to_char("You are falling down.\n\r",vict);
      send_to_char("Quuu aaaa rrrrrrrr . .  .  . \n\r",vict);
      return(1);
   }
      return(1);
  }
 if(musash_mod==1) return(1);

    /*  high level victim is already selected */
    if(vict && number(1,IMO-GET_LEVEL(vict))<5 ) {
        send_to_char("I CHALLENGE YOU.\n\r",vict);
        hit(ch,vict,TYPE_UNDEFINED);
        return(1);
        }
 return(FALSE);
}
int super_musashi(struct char_data *ch, int cmd, char *arg)
{
  void cast_sunburst(byte level, struct char_data *ch, char *arg, int type,
		     struct char_data *victim, struct obj_data *tar_obj );
  struct char_data *choose_victim(struct char_data *mob, int fightmode,
				  int mode) ;
  struct char_data *vict, *next_vict;
  struct affected_type af;
  int h,i,musash_mod=0;
  
  if(cmd)
    return(0);
  
  /* cyb  vict=world[ch->in_room].people; */
  vict = choose_victim(ch, VIC_ALL, MODE_HIGH_LEVEL) ;
  
  h=GET_HIT(ch);
  if(h < 1000000){
    cast_full_heal(GET_LEVEL(ch), ch, "", SPELL_TYPE_SPELL, ch, 0);
    musash_mod=1;
  }
  if(h < 2000000){
    cast_full_heal(GET_LEVEL(ch), ch, "", SPELL_TYPE_SPELL, ch, 0);
    cast_full_heal(GET_LEVEL(ch), ch, "", SPELL_TYPE_SPELL, ch, 0);
    musash_mod=1;
  }
  
  if(ch->specials.fighting){
    for (; vict; vict = next_vict) {
      next_vict = vict->next_in_room ;
      if(vict->specials.fighting==ch) {
	hit(ch,vict,TYPE_UNDEFINED);
	hit(ch,vict,TYPE_UNDEFINED);
	musash_mod=1;
      }
    }
    vict=ch->specials.fighting;
    if (!vict) return(1);
    
    if(number(1,2)==1){
      damage(ch,vict,10*GET_LEVEL(ch),SKILL_KICK);musash_mod=1;
    } 
    else {
      damage(ch,vict,10*GET_LEVEL(ch),SKILL_BASH);musash_mod=1;
    }
    
    if(ch->specials.fighting && !affected_by_spell(ch,SPELL_SANCTUARY)){
      act("$n 주위로 휘황한 광채가  감쌉니다.",TRUE,ch,0,0,TO_ROOM);
      af.type      = SPELL_SANCTUARY;
      af.duration  = 30;
      af.modifier  = 0;
      af.location  = APPLY_NONE;
      af.bitvector = AFF_SANCTUARY;
      affect_to_char(ch, &af);
      return(1);
    }
    vict=ch->specials.fighting;
    
    if (!vict) return(1);
    
    switch (number(0,18)){
      
    case 0:
      act("$n utters the words 'fire'.", 1, ch, 0, 0, TO_ROOM);
      for (vict = ch->specials.fighting; vict; vict = next_vict) {
	if ( vict->in_room != ch->in_room )
	  break;
	next_vict = vict->next_in_room ;
	if(vict->specials.fighting==ch) {
	  cast_sunburst(GET_LEVEL(ch),ch,"",SPELL_TYPE_SPELL,vict,0);
	}
      }
      return(1);
      
    case 1:
      act("$n utters the words 'frost'.", 1, ch, 0, 0, TO_ROOM);
      for (vict = ch->specials.fighting; vict; vict = next_vict) {
	if ( vict->in_room != ch->in_room )
	  break;
	next_vict = vict->next_in_room ;
	if(vict->specials.fighting==ch) {
	  cast_corn_of_ice(GET_LEVEL(ch),ch,"",SPELL_TYPE_SPELL,vict,0);
	}
      }
      return(1);
      
    case 2:
      
    case 3:
      act("$n double kick by double circle kick .", 1, ch, 0, 0, TO_ROOM);
      damage(ch,vict,20*GET_LEVEL(ch),SKILL_KICK);
      damage(ch,vict,20*GET_LEVEL(ch),SKILL_KICK);
      return(1);
      
    case 4:
      
    case 5:
      do_say(ch, "A cha cha cha cha ..", 0);
      act("$n tornado fire with miracle speed .", 1, ch, 0, 0, TO_ROOM);
      for(i=0;i<number(10,20);i++)
	cast_sunburst(GET_LEVEL(ch),ch,"",SPELL_TYPE_SPELL,vict,0);
      return(1);
      
    case 6:
      do_say(ch, "Yak Yak Yak Yak Ya..", 0);
      act("$n use thousands  kick .", 1, ch, 0, 0, TO_ROOM);
      for(i=0;i<number(10,20);i++)
	damage(ch,vict,10*GET_LEVEL(ch),SKILL_KICK);
      return(1);
      
    case 7:
      do_say(ch, "Heau Heau Heau Heau Heau..", 0);
      act("$n use hundreds bash .", 1, ch, 0, 0, TO_ROOM);
      for(i=0;i<number(10,20);i++)
	damage(ch,vict,10*GET_LEVEL(ch),SKILL_BASH);
      return(1);
      
    case 8:
      do_say(ch, "Ya uuuuu aaaaa    ..", 0);
      act("$n throw powerfull punch ! .", 1, ch, 0, 0, TO_ROOM);
      damage(ch,vict,GET_HIT(vict)/3,SKILL_BASH);
      return(1);
    case 9:
    case 10:
    case 11:
    case 12:
      do_say(ch, "Crush Armor    ..", 0);
      if (!vict) break;
      send_to_char("You feel shrink.\n\r", vict);
      if(!affected_by_spell(vict,SPELL_CRUSH_ARMOR)){
	af.type      = SPELL_CRUSH_ARMOR;
	af.duration  = 20;
	af.modifier  = GET_LEVEL(ch)/4*(-1) ;
	af.location  = APPLY_DAMROLL;
	af.bitvector = 0;
	affect_to_char(vict, &af);
	af.location  = APPLY_AC;
	af.modifier  = GET_LEVEL(ch)+20;
	affect_to_char(vict, &af);
      }
    case 13:
    case 14:
    case 15:
    case 16:
    case 17:
    case 18:
      do_say(ch, "Shou Ryu Ken..", 0);
      damage(ch,vict,GET_HIT(vict)/3-number(1,GET_HIT(vict)/15
					    +GET_LEVEL(vict)/2),SKILL_BASH);
      send_to_char("You are falling down.\n\r",vict);
      send_to_char("Quuu aaaa rrrrrrrr . .  .  . \n\r",vict);
      return(1);
    }
    return(1);
  }
  if(musash_mod==1) return(1);
  
  /*  high level victim is already selected */
  if(vict && number(1,IMO-GET_LEVEL(vict))<5 ) {
    send_to_char("I CHALLENGE YOU.\n\r",vict);
    hit(ch,vict,TYPE_UNDEFINED);
    return(1);
  }
  return(FALSE);
}


int mom(struct char_data *ch, int cmd, char *arg)
{
  struct char_data *vict;
  int h;
  

  if(cmd)
    return(0);
  vict=ch->specials.fighting;

  if(vict){
    if ( (GET_MOVE(vict) > 50)) {   /* cyb : reduce move point  */
      send_to_char("으라차아  !\n\r", vict);
      GET_MOVE(vict) -= dice(10,10);
    }
    h=GET_HIT(ch);

    if((h > 100) && (h < 999)){
      act("$n 이 크고 푸른 알약을 삼킵니다. (윙크)",1,ch,0,0,TO_ROOM);
      cast_heal(GET_LEVEL(ch), ch, "", SPELL_TYPE_SPELL, ch, 0);
      return TRUE;
    } 
    else {
      if(ch->in_room == vict->in_room) {
        shoot(ch,vict,TYPE_SHOOT);
      }
    }
  }

  switch (number(0,50)){

  case 0:
    do_say(ch, "오늘 할 일을 내일로 미루지 마라.", 0);
    return(1);

  case 1:
    do_say(ch, "집에서 너무 멀리 가지 말아라.", 0);
    return(1);

  case 2:
    do_say(ch, "일찍 자고 일찍 일어나라.", 0);
    return(1);

  case 3:
    do_say(ch, "얘들아 잘시간이다.", 0);
    return(1);

  case 4:
    do_say(ch, "누가 여기 쓰레기 치우지 않았지 ?", 0);
    return(1);

  case 5:
    do_say(ch, "얘 ! 방좀 가서 치워라.", 0);
    return(1);

  case 6:
    do_say(ch, "밥먹기 전에는 손을 씻어야지.", 0);
    return(1);

  case 7:
    do_say(ch, "밤늦게 까지 오락 하지 말랬지 !",0);
    return(1);

  case 8:
    do_say(ch, "넌 집에서 가정교육을 좀 더 받아야해", 0);
    return(1);

  case 9:
    do_say(ch, "보고서 다 썼니 ?", 0);
    return(1);

  case 10:
    do_say(ch, "숙제 다 했어 ?", 0);
    return(1);

  default:
    return(0);
  }
}

static char *song[]={
     "$n sings, '...lights flicker from the opposite loft'",
     "$n sings, 'in this room the heat pipes just coughed'",
     "$n sings, 'the country music station plays soft'",
     "$n sings, 'but there's nothing, really nothing, to turn off...'"
};
int singer(struct char_data *ch, int cmd, char *arg)
{
  static int n=0;
  if(cmd) return FALSE;
  if(GET_POS(ch)!=POSITION_STANDING) return FALSE;
  if(n < 4)
    act(song[n],TRUE,ch,0,0,TO_ROOM);
  if((++n)==16) n=0;
  return TRUE;
}
int fido(struct char_data *ch, int cmd, char *arg)
{

  struct obj_data *i, *temp, *next_obj;

  if (cmd || !AWAKE(ch))
    return(FALSE);

  for (i = world[ch->in_room].contents; i; i = i->next_content) {
    if (GET_ITEM_TYPE(i)==ITEM_CONTAINER && i->obj_flags.value[3]) {
      act("$n savagely devour a corpse.", FALSE, ch, 0, 0, TO_ROOM);
      for(temp = i->contains; temp; temp=next_obj) {
        next_obj = temp->next_content;
        obj_from_obj(temp);
        obj_to_room(temp,ch->in_room);
      }
      extract_obj(i);
      return(TRUE);
    }
  }
  return(FALSE);
}


int janitor(struct char_data *ch, int cmd, char *arg)
{
  struct obj_data *i;

  if (cmd || !AWAKE(ch))
    return(FALSE);

  for (i = world[ch->in_room].contents; i; i = i->next_content) {
    if (IS_SET(i->obj_flags.wear_flags, ITEM_TAKE) && 
      ((i->obj_flags.type_flag == ITEM_DRINKCON) ||
      (i->obj_flags.cost <= 10))) {
      act("$n picks up some trash.", FALSE, ch, 0, 0, TO_ROOM);
      obj_from_room(i);
      obj_to_char(i, ch);
      return(TRUE);
    }
  }
  return(FALSE);
}
int cityguard(struct char_data *ch, int cmd, char *arg)
{
  struct char_data *tch, *evil;
  int max_evil;

  if (cmd || !AWAKE(ch) || (GET_POS(ch) == POSITION_FIGHTING))
    return (FALSE);
  max_evil = 1001;
  evil = 0;
  for (tch=world[ch->in_room].people; tch; tch = tch->next_in_room) {
    if (tch->specials.fighting) {
      if((GET_ALIGNMENT(tch)<max_evil)&&(IS_NPC(tch->specials.fighting))) {
        max_evil = GET_ALIGNMENT(tch);
        evil = tch;
      }
    }
  }
  if (evil && (GET_ALIGNMENT(evil->specials.fighting) >= 0)) {
   act("$n screams 'PROTECT THE INNOCENT!!! CHARGE!!!'",
     FALSE, ch, 0, 0, TO_ROOM);
    hit(ch, evil, TYPE_UNDEFINED);
    return(TRUE);
  }
  return(FALSE);
}

#define RESCUER_VICTIM	5
int rescuer(struct char_data *ch, int cmd, char *arg)
{
  void mob_light_move(struct char_data *ch, char *argument, int cmd) ;
  struct char_data *tmp_ch, *vict;
  struct char_data *choose_rescue_mob(struct char_data *mob) ;
  struct char_data *choose_victim(struct char_data *mob, int fmode, int mode) ;

  if(cmd) return FALSE;

  if (ch->specials.fighting && (IS_NPC(ch->specials.fighting))) {
    act("$n screams 'PROTECT THE INNOCENT!!!'",FALSE,ch,0,0,TO_ROOM);
    mob_light_move(ch, "", 0);
    return TRUE ;
    }

  /* find the partner */
  vict = choose_rescue_mob(ch) ;

  if ( vict == ch ) {	/* only I am fighting  */
     mob_light_move(ch, "", 0) ;
     switch(number(0,3)) {
      case 0 : vict = choose_victim(ch, VIC_ALL, MODE_HIT_MIN) ; break ;
      case 1 : vict = choose_victim(ch, VIC_ALL, MODE_AC_MAX) ; break ;
      case 2 : vict = choose_victim(ch, VIC_ALL, MODE_DR_MAX) ; break ;
      case 3 : vict = choose_victim(ch, VIC_ALL, MODE_HR_MAX) ; break ;
      }
     if (vict) {
       act("$n screams 'PROTECT THE INNOCENT! CHARGE!'",FALSE,ch,0,0,TO_ROOM);
       hit(ch, vict, TYPE_UNDEFINED) ;	/* set fighting */
       }
     return TRUE ;
     }

  if (vict) {	/* other mob is fighting  */
     /* find who fight against vict */
     for (tmp_ch=world[ch->in_room].people; tmp_ch &&
       (tmp_ch->specials.fighting != vict); tmp_ch=tmp_ch->next_in_room)  ;

     if (!tmp_ch) return FALSE;

     send_to_char("Yaho! To the rescue...\n\r", ch);
     act("$n screams 'PROTECT THE INNOCENT!'",FALSE,ch,0,0,TO_ROOM);
     act("You are rescued by $N, you are confused!",FALSE,vict,0,ch,TO_CHAR);
     act("$n heroically rescues $N.", FALSE, ch, 0, vict, TO_NOTVICT);
     if (vict->specials.fighting == tmp_ch)
       stop_fighting(vict);
     if (tmp_ch->specials.fighting)
       stop_fighting(tmp_ch);
     if (ch->specials.fighting)
       stop_fighting(ch);
	 if( !ch->specials.fighting )
	 {
		hit(ch,tmp_ch,TYPE_UNDEFINED);
	 }
     return TRUE ;
     }

  return FALSE;
}

#undef RESCUER_VICTIM

int superguard(struct char_data *ch, int cmd, char *arg)
{
  struct char_data *tch, *evil;

  if (cmd || !AWAKE(ch) || (GET_POS(ch) == POSITION_FIGHTING))
    return (FALSE);
  evil = 0;
  for (tch=world[ch->in_room].people; tch; tch = tch->next_in_room) {
    if(IS_NPC(tch)) continue;
    if(IS_SET(tch->specials.act,PLR_CRIMINAL)){
      evil=tch;
      break;
    }
  }
  if(evil){
    act("$n 외칩니다. '누가 법을 깨뜨리는 녀석이냐 !!!!'",
     FALSE, ch, 0, 0, TO_ROOM);
    hit(ch, evil, TYPE_UNDEFINED);
    return(TRUE);
  }
  return(FALSE);
}

int pet_shops(struct char_data *ch, int cmd, char *arg)
{
  char buf[MAX_STRING_LENGTH], pet_name[256];
  int pet_room,k;
  struct char_data *pet;
  struct follow_type *j;

  if(IS_NPC(ch))
    return(FALSE);
  pet_room = ch->in_room+1;
  if (cmd==59) { /* List */
    send_to_char("애완동물 이 이런 것이 있습니다:\n\r", ch);
    for(pet = world[pet_room].people; pet; pet = pet->next_in_room) {
      /* can't buy PC */
      if(!IS_NPC(pet)) continue;
      sprintf(buf, "%8lld - %s\n\r",10*GET_EXP(pet), pet->player.short_descr);
      send_to_char(buf, ch);
    }
    return(TRUE);
  } else if (cmd==56) { /* Buy */
    arg = one_argument(arg, buf);
    arg = one_argument(arg, pet_name);
    /* Pet_Name is for later use when I feel like it */
    if (!(pet = get_char_room(buf, pet_room))) {
      send_to_char("아 그런 동물은 없는데요 ?\n\r", ch);
      return(TRUE);
    }
    if(!IS_NPC(pet)){
      send_to_char("사람은 안파는데요 ?\n\r", ch);
      return(TRUE);
    }
    for(k=0,j=ch->followers;(j)&&(k<5);++k){
      j=j->next;
    }
    if(k>=3){
      send_to_char("당신은 지금 데리고 있는 동물로 충분할 것 같은데요.\n\r",ch);
      return(TRUE);
    }
    if (GET_GOLD(ch) < (GET_EXP(pet)*10)) {
      send_to_char("돈 더 가져오세요!\n\r", ch);
      return(TRUE);
    }
    GET_GOLD(ch) -= GET_EXP(pet)*10;
    pet = read_mobile(pet->nr, REAL);
    GET_EXP(pet) = 0;
    SET_BIT(pet->specials.affected_by, AFF_CHARM);
    if (*pet_name) {
      sprintf(buf,"%s %s", pet->player.name, pet_name);
      free(pet->player.name);
      pet->player.name = strdup(buf);    

      sprintf(buf,"%sA small sign on a chain around the neck says 'My Name is %s'\n\r",
        pet->player.description, pet_name);
      free(pet->player.description);
      pet->player.description = strdup(buf);
    }

    char_to_room(pet, ch->in_room);
    add_follower(pet, ch);

    /* Be certain that pet's can't get/carry/use/wield/wear items */
/*
    And why not?

    IS_CARRYING_W(pet) = 1000;
    IS_CARRYING_N(pet) = 100;
*/
    send_to_char("애완동물이 마음에 드실껍니다.\n\r", ch);
    act("$n님이 $N 을 데리고 갑니다.",FALSE,ch,0,pet,TO_ROOM);

    return(TRUE);
  }

  /* All commands except list and buy */
  return(FALSE);
}
int hospital(struct char_data *ch, int cmd, char *arg)
{
  char buf[MAX_STRING_LENGTH];
  extern struct descriptor_data *descriptor_list;
  extern struct player_index_element *player_table;
  struct descriptor_data *k;
  // BUG FIX!!!
  int opt,/* lev, */ cost[7],c=0;
  int i;
  char *temp;
  char stash_file1[100];
  char stash_file2[100];
  char stash_name[30];
  
  // lev = GET_LEVEL(ch);
  cost[0] = 100-(40-GET_LEVEL(ch));
  cost[1] = cost[0] * (GET_MAX_HIT(ch) - GET_HIT(ch)) ;
  cost[2] = cost[0] * (GET_MAX_MANA(ch) - GET_MANA(ch)) ;
  cost[3] = cost[0] * (GET_MAX_MOVE(ch) - GET_MOVE(ch)) ;
  cost[4] = 50000 + cost[0] * 20 ;
  cost[5] = 500000000;
  cost[6] = 4000000 + GET_SEX(ch) * 1000000;
  
  if (cmd==59) { /* List */
    sprintf(buf, "1 - Hit points restoration (%d coins)\n\r", cost[1]);
    send_to_char(buf, ch);
    sprintf(buf, "2 - Mana restoration (%d coins)\n\r", cost[2]);
    send_to_char(buf, ch);
    sprintf(buf, "3 - Move restoration (%d coins)\n\r", cost[3]);
    send_to_char(buf, ch);
    sprintf(buf, "4 - Poison cured (%d coins)\n\r", cost[4]);
    send_to_char(buf, ch);
    sprintf(buf, "5 - Change Your name (%d coins)\n\r", cost[5]);
    send_to_char(buf, ch);
    sprintf(buf, "6 - Change Your Sex(toggle) (%d coins)\n\r", cost[6]);
    send_to_char(buf,ch);
    sprintf(buf, "To change name, TYPE \"buy 5 <NAME>\"\n\r");
    send_to_char(buf, ch);
    return(TRUE);
  } else if (cmd==56) { /* Buy */
    if (!arg || !*arg) {
      send_to_char("Buy what?\n\r",ch);
      return TRUE;
    }
    arg = one_argument(arg, buf);
    if ((opt = atoi(buf)) > 6) {
      send_to_char("Huh?\n\r",ch);
      return(TRUE);
    }
    /* opt의 value가 1보다 작을수도 있으므로 아래의 routine을 넣었습니다. */
    if (opt < 1) {
      send_to_char("Huh?\n\r",ch);
      return(TRUE);
    }
    if(cost[opt] > GET_GOLD(ch)){
      if (GET_LEVEL(ch) >= IMO) {
	send_to_char("쩝...당신한테만 외상으로 해드리는 거에요...\n\r",ch);
	cost[opt] = 0;
      }
      else {
	send_to_char("외상은 안됩니다.\n\r",ch);
	return(TRUE);
      }
    }
    switch(opt){
    case 1:
      GET_HIT(ch) = GET_MAX_HIT(ch);
      GET_GOLD(ch) -= cost[1];
      send_to_char("You feel magnificent!\n\r",ch);
      return TRUE;
    case 2:
      GET_MANA(ch) = GET_MAX_MANA(ch);
      GET_GOLD(ch) -= cost[2];
      send_to_char("You feel marvelous!\n\r",ch);
      return TRUE;
    case 3:
      GET_MOVE(ch) = GET_MAX_MOVE(ch);
      GET_GOLD(ch) -= cost[3];
      send_to_char("You feel The GOD!\n\r",ch);
      return TRUE;
    case 4:
      if(affected_by_spell(ch,SPELL_POISON)){
	affect_from_char(ch,SPELL_POISON);
	GET_GOLD(ch) -= cost[4];
	send_to_char("You feel stupendous!\n\r",ch);
      } else {
	send_to_char("Nothing wrong with you.\n\r",ch);
      }
      return TRUE;
    case 6:
      send_to_char("프로세스 라는 의사가 당신에게 물어봅니다.\n\r",ch);
      send_to_char("\"왜 성전환을 하려고 합니까?\"\n\r",ch);
      switch(GET_SEX(ch))
	{
	case SEX_NEUTRAL:
	  do_say(ch,"그냥요,왜 떫으요?\n\r", 0);
	  break;
	case SEX_MALE:
	  do_say(ch,"전요, 여자랑 그짓만 할 생각하는 남자가 싫어요!!!\n\r",0); 
	  send_to_char("프로세스는 멍텅해서 거시기를 잘라버린다!\n\r",ch);
	  GET_SEX(ch)=SEX_FEMALE;
	  break;
	case SEX_FEMALE:
	  do_say(ch,"전 당신같은 멋진 남자가 되고 싶었어요....\n\r",0);
	  send_to_char("프로세스는 그녀를 보고 안타까워 하면서 오이를 심어줍니다.\r\n",ch);
	  GET_SEX(ch)=SEX_MALE;
	  break;
	}
      GET_GOLD(ch) -= cost[6];
      return TRUE;
    case 5:
      /* by ares */
      /* skip white space */
      for (c = 0; isspace(arg[c]); c++) ;
      temp = &arg[c];
      /* check illegal name */
      for (i = 0; temp[i]; i++) {
	if ((temp[i] < 0) || !isalpha(temp[i]) || i > 15) {
	  i = -1;
	  break;
	}
      }
      if (i == -1) {
	send_to_char("That's Illegal name!\n\r", ch);
	return TRUE;
      }
      if (i < 2) {
	send_to_char("That's too short name!\n\r", ch);
	return TRUE;
      }
      /* check if already playing */
      /* stolen from interpreter.c */
      for(k = descriptor_list; k; k = k->next) {
	if ((k->character != ch) && k->character) {
	  if (k->original) {
	    if (GET_NAME(k->original) &&
		(str_cmp(GET_NAME(k->original), temp) == 0)) {
	      send_to_char("That name already exist!\n\r", ch);
	      return TRUE; /* jhpark 설명 필요없죠? */
	    }
	  }
	  else { /* No switch has been made */
	    if (GET_NAME(k->character) &&
		(str_cmp(GET_NAME(k->character), temp) == 0)) {
	      send_to_char("That name already exist!\n\r", ch);
	      return TRUE; /* jhpark */
	    }
	  }
	}
      }
      /* check player index table */
      /* need not check player file */
      /* for detail, see load_char() and find_name() */
      if ((i = find_name(temp)) != -1) {
	send_to_char("That name already used by another player!\n\r", ch);
	return TRUE;
      }
      i = find_name(ch->player.name);
      if (i == -1) {
	log("이럴루가!!!");
	send_to_char("ING? Then, how can i be here?\n\r", ch);
	return TRUE;
      }
      /* now, name can be changed! */
      /* but u must update not only ch's name but also player_table */
      /* and rename item file, and force save */
      strcpy(stash_name, ch->player.name);
      CAP(temp);
      free(player_table[i].name);
      CREATE(player_table[i].name, char, strlen(temp) + 1);
      strcpy(player_table[i].name, temp);
      free(ch->player.name);
      CREATE(ch->player.name, char, strlen(temp) + 1);
      strcpy(ch->player.name, temp);
      /* now remove item file */
      /* first make stash_name and temp to lowercase */
      for (i = 0; stash_name[i]; i++)
	stash_name[i] = tolower(stash_name[i]);
      for (i = 0; temp[i]; i++)
	temp[i] = tolower(temp[i]);
      
      /* now rename .x file */
      sprintf(stash_file1, "stash/%c/%s.x", stash_name[0], stash_name);
      sprintf(stash_file2, "stash/%c/%s.x", temp[0], temp);
      rename(stash_file1, stash_file2);
      /* now rename .x.y file */
      sprintf(stash_file1, "stash/%c/%s.x.y", stash_name[0], stash_name);
      sprintf(stash_file2, "stash/%c/%s.x.y", temp[0], temp);
      rename(stash_file1, stash_file2);
      /* now rename .x.tmp file */
      sprintf(stash_file1, "stash/%c/%s.x.tmp", stash_name[0], stash_name);
      sprintf(stash_file2, "stash/%c/%s.x.tmp", temp[0], temp);
      rename(stash_file1, stash_file2);
      /* now rename .lock file */
      sprintf(stash_file1, "stash/%c/%s.lock", stash_name[0], stash_name);
      sprintf(stash_file2, "stash/%c/%s.lock", temp[0], temp);
      rename(stash_file1, stash_file2);
      
      GET_GOLD(ch) -= cost[5];
      send_to_char("\n\r", ch);
      send_to_char("이름을 바꾸기 위해 당신은 ARES님에게 갑니다.\n\r", ch);
      send_to_char("흠...그래, 이름을 바꾸시겠다고?\n\r", ch);
      send_to_char("잘 생각했네! 자네가 지금 쓰는 이름은 딱 ", ch);
      send_to_char("비명횡사하기 좋지...\n\r", ch);
      send_to_char("자, 그럼 뭘로 할텐가?\n\r\n\r", ch);
      do_say(ch, temp, 0);
      send_to_char("\n\r", ch);
      send_to_char(temp, ch);
      send_to_char("? 그것도 제 명에 살기는 틀린 이름인데?\n\r", ch);
      send_to_char("허허..거참..허나 자네가 좋다니 별수 없지 뭐...\n\r", ch);
      send_to_char("당신이 가고 난 후 ARES님이 이렇게 중얼거립니다.\n\r", ch);
      send_to_char("하! 이름 바꾼다고 잘 살수 있을거 같지?\n\r", ch);
      send_to_char("어림 반 푼어치도 없다!!!!!!!!!!!!!!\n\r", ch);
#ifdef  RETURN_TO_QUIT  
      save_char(ch,world[ch->in_room].number);
#else
      save_char(ch,ch->in_room);
#endif
      return TRUE;
    default:
      send_to_char("뭐요?\n\r",ch);
      return(TRUE);
    }
  }
  return(FALSE);
}

/* modified by atre */
/*
remove
	5. exp --> gold
	6. freedom of hunger, thirst
add
	use ticket
*/
/* modified by epochal
   use ticket is changed. */

int metahospital(struct char_data *ch, int cmd, char *arg)
{
  char buf[MAX_STRING_LENGTH];
  char buf2[MAX_STRING_LENGTH];
  int k,opt;
  // int mult;
  long int cost=0;
  struct obj_data *tmp_obj;
  
  if (IS_NPC(ch))
    return(FALSE);
  
  if (cmd==59) { /* List */
    send_to_char("1 - Hit points inflator(NO LIMIT)\n\r",ch);
    send_to_char("2 - Mana increase(NO LIMIT)\n\r",ch);
    send_to_char("3 - Movement increase(NO LIMIT)\n\r",ch);
    send_to_char("4 - Increase your practice number (+2)(small exp depend on level)\n\r",ch);
    /*
      send_to_char("5 - Change exp to gold(3:2)\n\r",ch);
      send_to_char("6 - Freedom from hunger and thirsty(100M exp)\n\r",ch);
      */
    send_to_char("1,2,3,4 of these for some experience points\n\r",ch);
    send_to_char("use tickets for meta\n\r",ch);
    return(TRUE);
  }
  else if (cmd==56) { /* Buy */
    half_chop(arg,buf,buf2);
    opt = atoi(buf); 
    // mult = 1 ;
    
    /*
      if (1 <= opt && opt <= 6) {
      */
    /* remove 5, 6 */
    if (1 <= opt && opt <= 4) {
      switch (opt) {
      case 1:
	k = GET_PLAYER_MAX_HIT(ch);
	cost = k * 200;
	cost = number(cost, cost << 1);
	cost = MIN(50000000, cost);
	if (cost <= 0 || cost > GET_EXP(ch)) {
	  send_to_char("Come back when you are ", ch);
	  send_to_char("more experienced.\n\r",ch);
	  return(TRUE);
	}
	if (k > 3000000) {
	  send_to_char("you are full mana point!!!",ch);
	  cost=0;
	  break;
	}
	else
	  {
	    ch->points.max_hit += dice(3,5) + ch->abilities.con/6;
	    send_to_char("@>->->--\n\r",ch);
	    break;
	  }
      case 2:
	k=ch->points.max_mana;
	cost = k * 200;
	cost = number(cost, cost << 1);
	cost = MIN(50000000, cost);
	if (cost <= 0 || cost > GET_EXP(ch)) {
	  send_to_char("Come back when you are ", ch);
	  send_to_char("more experienced.\n\r",ch);
	  return(TRUE);
	}
	if (k > 3000000) {
	  send_to_char("you are full mana point!!!",ch);
	  cost=0;
	  break;
	}
	else {
	  ch->points.max_mana += dice(2,4)
	    + (GET_INT(ch) + GET_WIS(ch)) / 6;
	  send_to_char("<,,,,,,,>\n\r",ch);
	  break;
	}
      case 3:
	k=ch->points.max_move;
	cost = k * 200;
	cost = number(cost, cost << 1);
	cost = MIN(50000000, cost);
	if (cost <= 0 || cost > GET_EXP(ch)) {
	  send_to_char("Come back when you are ", ch);
	  send_to_char("more experienced.\n\r",ch);
	  return(TRUE);
	}
	if (k > 3000000) {
	  send_to_char("you are full move point!!!",ch);
	  cost=0;
	  break;
	}
	else {
	  ch->points.max_move += dice(3,5) + ch->abilities.dex/6; 
	  send_to_char("<xxxxx>\n\r",ch);
	  break;
	}
      case 4:
	cost = 3000;
	cost *= GET_LEVEL(ch);
	cost *= GET_LEVEL(ch);
	if (cost > GET_EXP(ch)) {
	  send_to_char("Come back when you are more experienced.\n\r",ch);
	  return(TRUE);
	}
	ch->specials.spells_to_learn+=(int)(ch->abilities.wis/3);
	send_to_char("Aaaaaaarrrrrrrrggggg\n\r",ch);
	break;
      case 5:
	cost = 30000;
	cost *= GET_LEVEL(ch);
	cost = number(cost, cost << 1);
	if (cost > GET_EXP(ch)) {
	  send_to_char("Come back when you are more experienced.\n\r",ch);
	  return(TRUE);
	}
	ch->points.gold+=cost*2/3;
	break;
      case 6:
	if (GET_EXP(ch) < 100000000)
	  {
	    send_to_char("Come back when you are more experienced.\n\r",ch);
	    return(TRUE);
	  }
	ch->specials.conditions[0] = -1;
	ch->specials.conditions[1] = -1;
	ch->specials.conditions[2] = -1;
	cost = 100000000;
	send_to_char_han(
			 "You are free from hunger and thirsty from now on!!!\n\r     Worship the God!\n\r",
			 "당신은 이제 배고픔과 목마름으로부터 해방입니다.\n\r신께 경배드리십시요.\n\r",ch);
	break;
      default:
	break;
      }
      ch->points.exp-=cost;
    }
    else {
      send_to_char("예?\n\r",ch);
    }
    return(TRUE);
  }
  
  else if (cmd==172) { /* use */
    half_chop(arg,buf,buf2);
    /* buf == tickets */
    if(*buf){
      if(strncmp("tickets", buf, strlen(buf))){
	/* not ticket */
	return FALSE;
      }
      tmp_obj = get_obj_in_list_vis(ch, buf, ch->carrying);
      if(tmp_obj){
	if(GET_EXP(ch) < 250000000){
	  send_to_char("Come back when you are more experienced.\n\r", ch);
	  return TRUE;
	}
	if(ch->quest.solved <= 0) {
	  send_to_char("You need to solve more quests!!!.\n\r", ch);
	  return TRUE;
	}
	if(GET_LEVEL(ch) < 40 && ch->quest.solved < 70){
	  send_to_char("Come back when you solve more quest.\n\r", ch);
	  return TRUE;
	}	
	
	/* use meta ticket */
	switch(obj_index[tmp_obj->item_number].virtual){
	case 7991 : /* ticket for AC */
	  k = GET_AC(ch);
	  GET_AC(ch) -= number(2, 3);
	  GET_EXP(ch) -= 250000000;
	  if(GET_AC(ch) < -120) ch->quest.solved -= 15;
	  else ch->quest.solved -= 7;
	  sprintf(buf2, "Your AC changed from %d to %d!!!\r\n", k, GET_AC(ch));
	  send_to_char(buf2, ch);
	  break;
	case 7992 : /* ticket for HR */
	  k = GET_HITROLL(ch);
	  GET_HITROLL(ch) += number(1, 2);
	  GET_EXP(ch) -= 150000000;
	  if(GET_HITROLL(ch) > 100) ch->quest.solved -= 10;
	  else ch->quest.solved -= 6;
	  sprintf(buf2, "Your HITROLL changed from %d to %d!!!\r\n", k, GET_HITROLL(ch));
	  send_to_char(buf2, ch);
	  break;
	case 7993 : /* ticket for DR */
	  /*
	    GET_DAMROLL(ch) += number(1, 2);
	    */
	  k = GET_DAMROLL(ch);
	  GET_DAMROLL(ch) ++;
	  GET_EXP(ch) -= 250000000;
	  if(GET_DAMROLL(ch) > 100) ch->quest.solved -= 15;
	  else ch->quest.solved -= 8;
	  sprintf(buf2, "You DAMROLL changed from %d to %d!!!\r\n", k, GET_DAMROLL(ch));
	  send_to_char(buf2, ch);
	  break;
	default :
	  send_to_char("I cannot understand your ticket.\n\r", ch);
	  return TRUE;
	}
	
	extract_obj(tmp_obj);
      }
      else {
	send_to_char("You do not have that item.\n\r", ch);
      }
      return TRUE;
    }
  }
  return(FALSE);
}

int remortal( struct char_data *ch , int cmd ,char *arg)
{
  char buf1[100],buf2[100];
  char class;
  char buf[255];
  char classes[4][20]=
  {
    "지존 마법사",
    "지존 힐러",
    "지존 도둑으",
    "지존 워리어" 
  };
  int all_done;
  
  /* not say */
  if(cmd != 17) return 0;
  
  /* furfuri modify ... under if .. && >>> || */
  /*
    if(ch->points.exp<500000000 || ch->player.level!=40) {
    */
  if(GET_LEVEL(ch) != 40) {
    send_to_char("You are not enough to remortal.!!!\n\r",ch);
    return 0;
  }
  
  /*
    defined in structs.h
    #define REMORTAL_MAGIC_USER     1
    #define REMORTAL_CLERIC         2
    #define REMORTAL_THIEF          4
    #define REMORTAL_WARRIOR        8
    */
  all_done = (ch->player.remortal == (1 + 2 + 4 + 8));
  
  if(all_done){
    if(ch->points.exp<500000000){
      send_to_char("You are not enough to remortal.!!!\n\r",ch);
      return 0;
    }
  }
  
  half_chop(arg,buf1,buf2);
  class=buf1[0];
  switch(class) {
  case '?':
    /* show remortal */
    if(ch->player.remortal & REMORTAL_MAGIC_USER)
      send_to_char("You did a magic user.!!!\n\r", ch);
    if(ch->player.remortal & REMORTAL_CLERIC)
      send_to_char("You did a cleric.!!!\n\r", ch);
    if(ch->player.remortal & REMORTAL_THIEF)
      send_to_char("You did a thief.!!!\n\r", ch);
    if(ch->player.remortal & REMORTAL_WARRIOR)
      send_to_char("You did a warrior.!!!\n\r", ch);
    return TRUE;

  case 'W':
  case 'w': class=4;
    if(ch->abilities.str==18) {
      if(ch->abilities.str_add<100) {
	ch->abilities.str_add+=10;
	ch->abilities.str_add = MIN(ch->abilities.str_add,100);
      } else { break; } 
    } else { ch->abilities.str++; }
    ch->player.remortal |= REMORTAL_WARRIOR;
    break;

  case 'C':
  case 'c': class=2;
    if(ch->abilities.wis<18)ch->abilities.wis++;
    ch->player.remortal |= REMORTAL_CLERIC;
    break;

  case 'M':
  case 'm': class=1;
    if(ch->abilities.intel<18)ch->abilities.intel++;
    ch->player.remortal |= REMORTAL_MAGIC_USER;
    break;

  case 'T':
  case 't': class=3;
    if(ch->abilities.dex<18)ch->abilities.dex++;
    ch->player.remortal |= REMORTAL_THIEF;
    break;

  default: 
    return TRUE; 
  }
  sprintf(buf,"%s님이 %s로 직업을 바꾸셨습니다.",GET_NAME(ch),
	  classes[class-1]);
  send_to_all(buf);
  
  GET_CLASS(ch) = class;
  
  /* initialize other data */
  if(!all_done){
    GET_EXP(ch) = 0;
    GET_LEVEL(ch) = 0;
    
    GET_HIT(ch) = GET_PLAYER_MAX_HIT(ch) = 0;
    GET_MANA(ch) = GET_PLAYER_MAX_MANA(ch) = 0;
    GET_MOVE(ch) = GET_PLAYER_MAX_MOVE(ch) = 0;
    
    GET_AC(ch) += 40;
    GET_HITROLL(ch) -= 40;
    GET_DAMROLL(ch) -= 40;
    
    /* init-quest */
    ch->quest.type = 0;
    ch->quest.data = 0;
    ch->quest.solved = 0;
    
    do_start(ch);
  }
  else {
    GET_EXP(ch) -= 500000000;
  }
  
  return TRUE;
}

int jale_room(struct char_data *ch, int cmd, char *arg)
{
  /* if(IS_AFFECTED(ch, AFF_ARREST)){ */
  if(GET_LEVEL(ch) < IMO){
    switch (cmd) {
    case 15 : /* look */
    case 16 : /* score */
    case 17 : /* say */
    case 18 : /* shout */
    case 42 : /* stand */
    case 43 : /* sit */
    case 44 : /* rest */
    case 45 : /* sleep */
    case 46 : /* wake */
      return FALSE;
    default :
      send_to_char("Only, you can SLEEP!!!, you are a PRISONER.\n\r",ch);
      return TRUE;
    }
  }
  return FALSE;
}

int safe_house(struct char_data *ch, int cmd, char *arg)
{
  char buf[100];
  
  if(GET_LEVEL(ch) >= (IMO+2))
    return FALSE;
  switch (cmd) {
//  case 25: 	/* kill */
//  case 70:	/* hit */
//  case 84:	/* cast Perhaps modified */
//  case 87:	/* order */
//  case 154:	/* backstab */
//  case 156:	/* steal */
//  case 157:	/* bash */
//  case 159:	/* kick */
//  case 172:	/* use */
//  case 192:	/* shoot */
//  case 207:	/* recite */
//  case 234:	/* tornado */
//  case 239:	/* flash */
//  case 268:   /* disarm by chase*/
//  case 269:   /* shouryuken by chase */
//  case 270:   /* throw object */
//  case 271:	/* punch */
//  case 272:  /* assault by process*/
//  case 280:
//  case 281:
//  case 282:
//  case 283:
//  case 284:
//  case 287:
//  case 288:
//  case 289:
//  case 290:
//  case 292:
//  case 293:
//  case 298:	/* spin bird kick */
  case 9: 	/* kill */
  case 70:	/* hit */
  case 84:	/* cast Perhaps modified */
  case 87:	/* order */
  case 22:	/* backstab */
  case 156:	/* steal */
  case 157:	/* bash */
  case 159:	/* kick */
  case 172:	/* use */
  case 192:	/* shoot */
  case 207:	/* recite */
  case 234:	/* tornado */
  case 209:	/* flash */
  case 268:   /* disarm by chase*/
  case 269:   /* shouryuken by chase */
  case 270:   /* throw object */
  case 271:	/* punch */
  case 272:  /* assault by process*/
  case 280:
  case 281:
  case 282:
  case 283:
  case 284:
  case 287:
  case 288:
  case 289:
  case 290:
  case 292:
  case 293:
  case 298:	/* spin bird kick */
/* by Moon */
    send_to_char("You cannot do that here.\n\r",ch);
    sprintf(buf,"%s attempts to misbehave here.\n\r",ch->player.name);
    send_to_room_except(buf,ch->in_room,ch);
    return TRUE;
  default:
    return FALSE;
  }
}

int shooter(struct char_data *ch, int cmd, char *arg)
{
  struct char_data *tch;
  
  if(cmd) return(FALSE);
  if(GET_POS(ch) < POSITION_RESTING) return(FALSE);
  if(IS_SET(ch->specials.act,ACT_AGGRESSIVE)||(GET_POS(ch)==POSITION_FIGHTING))
    { for (tch=world[ch->in_room].people; tch; tch = tch->next_in_room){
      if((!IS_NPC(tch))&&(GET_LEVEL(tch) < IMO)){
	if(GET_POS(tch) <= POSITION_DEAD) continue;
	act("$n yells '$N must die!'",FALSE,ch,0,tch,TO_ROOM);
	shoot(ch, tch, TYPE_SHOOT);
      }
    }
    return(TRUE);
  }
  return(FALSE);
}

int finisher(struct char_data *ch, int cmd, char *arg)
{
  struct char_data *vict;
  int max_dam=0;

  if(cmd) return(FALSE); 
  if((vict = ch->specials.fighting)) {
     if(GET_HIT(ch) < GET_PLAYER_MAX_HIT(ch)/5){
      act("$n 최후의 기를 모은다.  ' 하압 ~~~~~~~~~ '",FALSE,ch,0,vict,TO_ROOM);
      max_dam = (GET_HITROLL(ch)+1)*(GET_DAMROLL(ch)+1)*number(5,1)/2;
      if(max_dam<1000) 
        act(" 화르르르르르르르르르르르르 `````````` ",FALSE,ch,0,vict,TO_ROOM);
      else if(max_dam<3000)
      {	act(" 팟팟파파파파파팟파파파아아 ````````앗 ",FALSE,ch,0,vict,TO_ROOM);
      	act(" 팟팟파파파파파팟파파파아아 ````````앗 ",FALSE,ch,0,vict,TO_ROOM);
      }
      else if(max_dam<5000)
      { act(" 화르르르르르르르르르르르르 `````````` ",FALSE,ch,0,vict,TO_ROOM);
      	act(" 팟팟파파파파파팟파파파아아 ````````앗 ",FALSE,ch,0,vict,TO_ROOM);
      	act(" 꽈콰콰콰콰콰아아아아아아앙 ~~~~~~~~~~ ",FALSE,ch,0,vict,TO_ROOM);
      }
      else if(max_dam<7000)
      { act(" 지              ~                  잉 ",FALSE,ch,0,vict,TO_ROOM);
        act(" 지              ~                     ",FALSE,ch,0,vict,TO_ROOM);
        act(" 징              ~                     ",FALSE,ch,0,vict,TO_ROOM);
      }
      else {
      	act(" 				            ",FALSE,ch,0,vict,TO_ROOM);
      	act(" 				            ",FALSE,ch,0,vict,TO_ROOM);
      	act(" 				            ",FALSE,ch,0,vict,TO_ROOM);
      	act(" 				            ",FALSE,ch,0,vict,TO_ROOM);
      	act(" 				            ",FALSE,ch,0,vict,TO_ROOM);
      }
      damage(ch,vict,max_dam,TYPE_UNDEFINED);
      	act("$n 기합을 준다. '끼요요요~~~~ 아아아앗~~~~~~~~  아쵸우~ 아쵸!!!' ",FALSE,ch,0,vict,TO_ROOM);
      return(TRUE);
      }
  }
  return(FALSE);
}

int bank(struct char_data *ch, int cmd, char *arg)
{
  char buf[MAX_STRING_LENGTH];
  LONGLONG amt;

  if (IS_NPC(ch))
    return(FALSE);
  if (cmd==59) { /* List */
    send_to_char("At the bank you may:\n\r\n\r", ch);
    send_to_char("list - get this list\n\r",ch);
    send_to_char("balance - check your balance\n\r",ch);
    send_to_char("deposit <amount>, or\n\r",ch);
    send_to_char("withdraw <amount>\n\r\n\r",ch);
    return(TRUE);
  } else if (cmd==227) { /* Balance */
    sprintf(buf,"You have %lld coins in the bank.\n\r",(LONGLONG)ch->bank);
    send_to_char(buf,ch);
    return(TRUE);
  } else if ((cmd==228)||(cmd==229)) {  /* deposit or withdraw */
    if(! *arg){
      send_to_char("The banker says 'Specify an amount.'\n\r",ch);
      return(TRUE);
    }
    arg=one_argument(arg, buf);
    amt=atoll(buf);
    if(amt <= 0){
      send_to_char("The banker says 'Amount must be positive.'\n\r",ch);
      return(TRUE);
    }
    if(cmd==228){
      if(amt > GET_GOLD(ch)){
        send_to_char("The banker says 'You don't have that much.'\n\r",ch);
        return(TRUE);
      }
	  if ( ch->bank + amt < ch->bank )
	  {
		send_to_char("Too much money! I can't afford so much money.\n\r",ch);
		return(TRUE);
	  }
      GET_GOLD(ch)-=amt;
      ch->bank+=amt;
    } else {
      if(amt > ch->bank){
        send_to_char("The banker says 'We don't make loans.'\n\r",ch);
        return(TRUE);
      }
      if ( GET_GOLD(ch) + amt < GET_GOLD(ch) ){
				send_to_char("Too much money! You can't afford so much money.\n\r",ch);
				return(TRUE);
			}
      GET_GOLD(ch)+=amt;
      ch->bank-=amt;
    }
    send_to_char("The banker says 'Have a nice day.'\n\r",ch);
    return(TRUE);
  }
  return(FALSE);
}
int totem(struct char_data *ch, int cmd, char *arg)
{
  int rm;
  struct obj_data *obj;

  if(cmd!=15) return(FALSE);
  obj=ch->equipment[HOLD];
  if(!obj) return(FALSE);
  if(obj_index[obj->item_number].virtual != 1311) return(FALSE);
  if(!ch->followers) return(FALSE);
  rm=ch->followers->follower->in_room;
  send_to_char(world[rm].name, ch);
  send_to_char("\n\r", ch);
  if(!IS_SET(ch->specials.act,PLR_BRIEF))
    send_to_char(world[rm].description, ch);
  list_obj_to_char(world[rm].contents, ch, 0,FALSE);
  list_char_to_char(world[rm].people, ch, 0);
  return(TRUE);
}
int kickbasher(struct char_data *ch, int cmd, char *arg)
{
  struct char_data *vict;

  char buf[80];

  if((vict = ch->specials.fighting))
  	sprintf(buf,"%s",GET_NAME(vict));
  if(cmd)
    return(FALSE);
  if((vict = ch->specials.fighting)) {
    if(number(1,2)==1) do_kick(ch,buf,0);
    else do_bash(ch,buf,0);
    if(GET_LEVEL(ch)>20){
      if(number(1,2)==1) do_kick(ch,buf,0);
      else do_bash(ch,buf,0);
    }
    if(GET_LEVEL(ch)>30){
      if(number(1,2)==1) do_kick(ch,buf,0);
      else do_bash(ch,buf,0);
    }
    if(GET_LEVEL(ch)>35){
      if(number(1,2)==1) do_kick(ch,buf,0);
      else do_bash(ch,buf,0);
    }
    if(GET_LEVEL(ch)>40){
      if(number(1,2)==1) do_kick(ch,buf,0);
      else do_bash(ch,buf,0);
    }
    return(TRUE);
  } 
  return(FALSE);
}

int spell_blocker(struct char_data *ch, int cmd, char *arg)
{
  if(cmd==84){
    do_say(ch,"하하하, 여기서는 마술을 못쓰지!",0);
    return TRUE;
  }
  else return FALSE;
}

int archmage(struct char_data *ch, int cmd, char *arg)
{
  struct char_data *vict;
  void do_shout(struct char_data *ch, char *argument, int cmd);
  if(cmd) return FALSE;
  if(!ch->specials.fighting) return FALSE;
  do_shout(ch,"SUNFIRE",0);
  act("The outside sun flares brightly, flooding the room with searing rays.",
    TRUE, ch, 0, 0, TO_ROOM);
  for(vict=world[ch->in_room].people; vict; vict = vict->next_in_room ){
    if(vict->specials.fighting==ch){
      if (IS_NPC(vict)) damage(ch,vict,dice(100,2),0);
    }
  }
  return TRUE;
}

int spitter(struct char_data *ch, int cmd, char *arg)
{
  struct char_data *vict;
  void cast_blindness(byte level,struct char_data *ch,char *arg,int type,
    struct char_data *victim, struct obj_data *tar_obj );

  if(cmd==156){
    if(IS_NPC(ch))
      return(TRUE);
    for(vict=world[ch->in_room].people;vict;vict=vict->next_in_room)
      if(IS_NPC(vict))
        hit(vict,ch,TYPE_UNDEFINED);
    return(TRUE);
  }
  if(!ch->specials.fighting)
    return(FALSE);
  for(vict=world[ch->in_room].people;vict;vict=vict->next_in_room){
    if((!IS_NPC(vict))&&(GET_POS(vict)==POSITION_FIGHTING || 
       (GET_MANA(vict) > 100 && GET_CLASS(vict)==CLASS_CLERIC ))) {
    switch(number(1,7)){
      case 1:
      case 2:
      case 3:
       damage(ch,vict,50,SKILL_KICK);
       return(FALSE);
      case 4:
      case 5:
      case 6:
       act("$n makes a disgusting noise - then spits at $N.",
        1,ch,0,vict,TO_NOTVICT);
       act("$n spit in your eye...",1,ch,0,vict,TO_VICT);
       cast_blindness(GET_LEVEL(ch), ch,"",SPELL_TYPE_SPELL,vict,0);
       return(FALSE);
      default:
       return(FALSE);
      }
    }
    if (GET_LEVEL(ch) < 35 || dice(1,8) < 6) break ;
  }
  return(FALSE);
}
int portal(struct char_data *ch, int cmd, char *arg)
{
  int location,ok;
  extern int top_of_world;
  void do_look(struct char_data *ch, char *argument, int cmd);

  if(cmd != 3)   /* specific to Room 2176 */
    return(FALSE);
  location = number(1,top_of_world);
  ok=TRUE;
  if (IS_SET(world[location].room_flags,OFF_LIMITS))
    ok=FALSE;
  else if (IS_SET(world[location].room_flags,PRIVATE))
    ok=FALSE;
  if(!ok){
    send_to_char("You bump into something, and go nowhere.\n\r",ch);
    act("$n seems to bump into nothing??",FALSE,ch,0,0,TO_NOTVICT);
  } else {
    act("$n님이 없어진것 같네요 ??",FALSE,ch,0,0,TO_NOTVICT);
    send_to_char("You are momentarily disoriented.\n\r",ch);
    char_from_room(ch);
    char_to_room(ch,location);
    do_look(ch,"",15);
  }
  return(TRUE);
}
int magicseed(struct char_data *ch, int cmd, char *arg)
{
  int skno;
  char name[256];
  struct obj_data *obj;

  if(cmd!=12) return(FALSE);
  one_argument(arg,name);
  obj=get_obj_in_list_vis(ch,name,ch->carrying);
  if(!obj) return(FALSE);
  if(GET_COND(ch,FULL)>20 && GET_COND(ch,FULL) != -1)  
    return(FALSE);
  if(obj_index[obj->item_number].virtual != 2157)
    return(FALSE);

	do {
		skno = number(0, MAX_SKILLS - 1);
		if (ch->skills[skno].learned > 50 && ch->skills[skno].learned < 99) {
			ch->skills[skno].learned = 99;
			send_to_char("자욱한 연기가 눈앞을 가립니다.\n\r",ch);
			if ( GET_COND(ch,FULL) >= 0 )
				GET_COND(ch,FULL)++;	
			break;
		}
	} while (number(0, 10));

	extract_obj(obj);
	send_to_char("OK.\n\r", ch);
	return TRUE;
}
