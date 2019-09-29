
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
#include "limits.h"
#include "mob_magic.h"		/* cyb */
#include "command.h"
#include "quest.h"

/*   external vars  */

extern struct room_data *world;
extern struct char_data *character_list;
extern struct descriptor_data *descriptor_list;
extern struct index_data *obj_index;
extern struct time_info_data time_info;
extern struct title_type titles[4][IMO+4];
extern struct index_data *mob_index;

/* extern procedures */

void hit(struct char_data *ch, struct char_data *victim, int type);
void gain_exp(struct char_data *ch, int gain);
void advance_level(struct char_data *ch);
void stop_fighting(struct char_data *ch);
void set_title(struct char_data *ch);
char *string_dup(char *source);

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
void cast_colour_spray(byte level, struct char_data *ch, char *arg, int type,
                       struct char_data *victim, struct obj_data *tar_obj );

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
    char buf[256];
    if(cmd)
        return(0);
    /* cyb vict=world[ch->in_room].people; */
    vict = choose_victim(ch, VIC_ALL, MODE_MOVE_MAX) ;

    h=GET_HIT(ch);
    if(h < 5000)
        cast_full_heal(GET_LEVEL(ch), ch, "", SPELL_TYPE_SPELL, ch, 0);
    if(vict && !IS_NPC(vict)){
        if(number(1,10)>7){
            act("$n control your move -200.",1,ch,0,0,TO_ROOM);
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
        if(real_number=real_mobile(ran_num)){
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
    if(vict && !IS_NPC(vict) && (GET_LEVEL(vict) < IMO)){
        if(number(1,10)>7){
            act("$n control your body ZERO.",1,ch,0,0,TO_ROOM);
            vict->points.hit=1;
            vict->points.move=0;
            act("$n Wuhhhhhh...... thank you very much ? .",1,ch,0,0,TO_ROOM);	
            return TRUE;	
        }
        else if(number(1,10)>7){
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
        else if(number(1,10)>7){
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
        while(list=ch->carrying){
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
        do_say(ch,"I need more particle.",0);
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
	int	dam ;

	if(cmd)
		return(0);	
	vict=ch->specials.fighting;
	if(vict && ch->in_room == vict->in_room){
		act("$n\tannihilate you with his full power.\n\r",1,ch,0,0,TO_ROOM);
		/*if ( IS_NPC(vict)) return TRUE ;*/
		if(vict->points.hit>=51 && vict->points.hit<101)
			dam = dice(8,GET_LEVEL(ch)) ;
		else if(vict->points.hit>=101 && vict->points.hit<201)
			dam = dice(10,GET_LEVEL(ch)) ;
		else if(vict->points.hit>=201 && vict->points.hit<401)
			dam = dice(12,GET_LEVEL(ch)) ;
		else if(vict->points.hit>=401 && vict->points.hit<801)
			dam = dice(18,GET_LEVEL(ch)) ;
		else if(vict->points.hit>=801 && vict->points.hit<1601)
			dam = dice(24,GET_LEVEL(ch)) ;
		else if(vict->points.hit>=1601 && vict->points.hit<2001)
			dam = dice(33,GET_LEVEL(ch)) ;
		else if(vict->points.hit>=2001)
			dam = dice(41,GET_LEVEL(ch)) ;
		else
			dam = 0 ;
		damage(ch, vict, dam, TYPE_UNDEFINED);
		return(TRUE);
        /* old message.
           if(vict->points.hit>=51 && vict->points.hit<101)
           damage(ch,vict,dice(8,GET_LEVEL(ch)),TYPE_UNDEFINED);
           if(vict->points.hit>=101 && vict->points.hit<201)
           damage(ch,vict,dice(10,GET_LEVEL(ch)),TYPE_UNDEFINED);
           if(vict->points.hit>=201 && vict->points.hit<401)
           damage(ch,vict,dice(12,GET_LEVEL(ch)),TYPE_UNDEFINED);
           if(vict->points.hit>=401 && vict->points.hit<801)
           damage(ch,vict,dice(18,GET_LEVEL(ch)),TYPE_UNDEFINED);
           if(vict->points.hit>=801 && vict->points.hit<1601)
           damage(ch,vict,dice(24,GET_LEVEL(ch)),TYPE_UNDEFINED);
           if(vict->points.hit>=1601 && vict->points.hit<2001)
           damage(ch,vict,dice(33,GET_LEVEL(ch)),TYPE_UNDEFINED);
           if(vict->points.hit>=2001)
           damage(ch,vict,dice(41,GET_LEVEL(ch)),TYPE_UNDEFINED);
        */
    }
	return(FALSE);
}

int lag_monster(struct char_data *ch, int cmd, char *arg)
{
    struct char_data *choose_victim(struct char_data *mob, int fightmode,
                                    int mode) ;
    struct char_data *vict;
    struct affected_type af;
	int turn = number(3,4);

    if(cmd) return(0);

	/*  Choose high level player  */
	vict = choose_victim(ch, VIC_ALL, MODE_HIGH_LEVEL) ;


	if ( ch->specials.fighting ) {
		if ( ch->in_room != ch->specials.fighting->in_room) {
			stop_fighting(ch) ;	/* he flee already */
			return TRUE;
		}

		if(ch->specials.fighting && !affected_by_spell(ch,SPELL_SANCTUARY)){
			act("$n 주위로 휘황한 광채가  감쌉니다.",TRUE,ch,0,0,TO_ROOM);
			af.type      = SPELL_SANCTUARY;
			af.duration  = 1;
			af.modifier  = 0;
			af.location  = APPLY_NONE;
			af.bitvector = AFF_SANCTUARY;
			affect_to_char(ch, &af);
			return(TRUE);
        }
		vict = ch->specials.fighting;
		switch( number(1,5) ) {
		case 1:
			act("$n utters the words '%!@#&@*@#!...'.", 1, ch, 0, 0, TO_ROOM);
			cast_sunburst(GET_LEVEL(ch),ch,"",SPELL_TYPE_SPELL,vict,0);
			break;
		default:
			send_to_char("받아라... LAG이다!!! \n\r",vict);
			WAIT_STATE(vict, PULSE_VIOLENCE*turn+2);
		}

		return 1;
	}

	/*  High level victim is already selected  */
	if (vict && number(1,3) == 1 && (GET_LEVEL(vict)< IMO) ) {
		send_to_char("HaHaHa... LAG!!! LAG!!! \n\r",vict);

		WAIT_STATE(vict, PULSE_VIOLENCE*turn);
    }
	else if (vict && (GET_LEVEL(vict) < IMO)) {
		switch (number(0,20)){
		case 0:
			do_say(ch, "음...", 0);
			return(1);
		case 1:
			do_say(ch, "I hope network becomes fast...", 0);
			return(1);
		case 2:
			do_say(ch, "I **HATE** LAG...", 0);
			return(1);
		case 3:
			do_say(ch, "Is ISDN good?", 0);
			return(1);
		case 4:
			do_say(ch, "Is ADSL more good?", 0);
			return(1);
		case 5:
			do_say(ch, "Modem player is great...", 0);
			return(1);
		default:
		}
	}

	return(FALSE);
}

int musashi(struct char_data *ch, int cmd, char *arg)
{
/*    struct char_data *victim, struct obj_data *tar_obj );*/
    struct char_data *choose_victim(struct char_data *mob, int fightmode,
                                    int mode) ;
    struct char_data *vict, *next_vict, *king;
    struct affected_type af;
    int h,i,musash_mod=0;

    if(cmd)
        return(0);

	if(GET_HIT(ch) < 2000) {
		cast_full_heal(GET_LEVEL(ch), ch, "", SPELL_TYPE_SPELL, ch, 0);
		musash_mod=1;
	}

	/* Is Musashi fighting now ? */
	if ( ch->specials.fighting ) {
		if ( ch->in_room != ch->specials.fighting->in_room) {
			stop_fighting(ch) ;	/* he flee already */
			return (TRUE) ;
		}

		vict = world[ch->in_room].people ;
		for (vict; vict; vict = next_vict) {
			next_vict = vict->next_in_room ;
			if (vict->specials.fighting == ch) {
				hit(ch,vict,TYPE_UNDEFINED);
				hit(ch,vict,TYPE_UNDEFINED);
				musash_mod=1;
            }
        }

		vict=ch->specials.fighting ;
	
		if (number(1,2)==1){
			damage(ch,vict,2*GET_LEVEL(ch),SKILL_KICK);
		} else {
			damage(ch,vict,2*GET_LEVEL(ch),SKILL_BASH);
		}
		musash_mod=1;

		if(ch->specials.fighting && !affected_by_spell(ch,SPELL_SANCTUARY)){
			act("$n 주위로 휘황한 광채가  감쌉니다.",TRUE,ch,0,0,TO_ROOM);
			af.type      = SPELL_SANCTUARY;
			af.duration  = 8;
			af.modifier  = 0;
			af.location  = APPLY_NONE;
			af.bitvector = AFF_SANCTUARY;
			affect_to_char(ch, &af);
			return(TRUE);
        }
		if ( vict == NULL ) {	/* dead or fleed already */
			return TRUE ;
        }
		else if ( ch->in_room != vict->in_room ) {	/* victim fleed */
			if (ch->specials.fighting)
				stop_fighting(ch) ;
			return (TRUE) ;
        }
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
            damage(ch,vict,3*GET_LEVEL(ch),SKILL_KICK);
            return(1);
        case 4:
        case 5:
            do_say(ch, "A cha cha cha cha ..", 0);
            act("$n tornado fire with miracle speed .", 1, ch, 0, 0, TO_ROOM);
            for(i=0;i<number(5,8);i++)
                cast_sunburst(GET_LEVEL(ch),ch,"",SPELL_TYPE_SPELL,vict,0);
            return(1);
        case 6:
            do_say(ch, "Yak Yak Yak Yak Ya..", 0);
            act("$n use thousands  kick .", 1, ch, 0, 0, TO_ROOM);
            for(i=0;i<number(5,8);i++)
                damage(ch,vict,3*GET_LEVEL(ch),SKILL_KICK);
            return(1);
        case 7:
            do_say(ch, "Heau Heau Heau Heau Heau..", 0);
            act("$n use hundreds bash .", 1, ch, 0, 0, TO_ROOM);
            for(i=0;i<number(5,8);i++)
                damage(ch,vict,3*GET_LEVEL(ch),SKILL_BASH);
            return(1);
        case 8:
            do_say(ch, "Ya uuuuu aaaaa    ..", 0);
            act("$n throw powerfull punch ! .", 1, ch, 0, 0, TO_ROOM);
            vict->points.hit-= 120;
            return(1);
        case 9:
        case 10:
        case 11:
        case 12:
            do_say(ch, "Crush Armor    ..", 0);
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
            return(TRUE);
        case 13:
        case 14:
        case 15:
        case 16:
        case 17:
        case 18:
            do_say(ch, "Shou Ryu Ken..", 0);
            vict->points.hit-=vict->points.hit/3-number(1,vict->points.hit/8+GET_LEVEL(vict)/2);
            send_to_char("You are falling down.\n\r",vict);
            send_to_char("Quuu aaaa rrrrrrrr . .  .  . \n\r",vict);
            return(TRUE);
        }
        return(TRUE);
    }	/* end of if (ch->specials.fighting) */


    /* Musash is fighting */
    if(musash_mod==1) return(TRUE);


	/* cyb  vict=world[ch->in_room].people; */
    king = choose_victim(ch, VIC_ALL, MODE_HAS_OKSE) ;
    if ( king ) {
        /* holding okse */
        act("$n bows before you.", FALSE,ch,0,king,TO_VICT) ;
        act("$n bows before $N.", FALSE,ch,0,king,TO_NOTVICT);
        if (number(1,5) > 4) {
            act("$n님이 '$N 전하 밤새 안녕하셨습니까 ?' 하고 인사를 합니다",
                FALSE, ch, 0, king, TO_ROOM);
        }
        return (TRUE);
    }

/*  Choose high level player  */
    vict = choose_victim(ch, VIC_ALL, MODE_HIGH_LEVEL) ;

/*  High level victim is already selected  */
    if (vict && number(1,IMO-GET_LEVEL(vict))<5 ) {
        send_to_char("I CHALLENGE YOU.\n\r",vict);
        hit(ch,vict,TYPE_UNDEFINED);
        return(TRUE);
    }

    return(FALSE);
}

int seraphim(struct char_data *ch, int cmd, char *arg)
{
    struct char_data *vict;
    int h;

    if(cmd == COM_CAST) {
		send_to_char("정신을 집중하기 힘들군요.\n\r", ch);
		send_to_room_except("\n\r검은 기운이 마나를 흡수합니다\n\r", 
                            ch->in_room, ch);
		GET_MANA(ch) -= dice(5,10);
		if (number(0,5)) return(FALSE);
		else return(TRUE);
	}
	else if (cmd) return 0;

    h=GET_HIT(ch);
    if(h < 5000)
        cast_full_heal(GET_LEVEL(ch), ch, "", SPELL_TYPE_SPELL, ch, 0);

    vict=ch->specials.fighting;
    if(vict){
        if ( (GET_MOVE(vict) > 50)) {   /* cyb : reduce move point  */
            act("$n utters, 'mihpares' !!!",1,ch,0,0,TO_ROOM);	
            GET_MOVE(vict) -= dice(5,10);
        }
        h=GET_HIT(ch);
        if((h < 10000)){
            act("모래로부터 검은 기운이 $n으로 흘러들어갑니다.",1,ch,0,0,TO_ROOM);
			GET_HIT(ch) += dice(30,20);
            return TRUE;
		}
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
        } else {
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
    struct obj_data *i, *temp, *next_obj;

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
        act("$n screams 'PROTECT THE INNOCENT! CHARGE!!!'",
            FALSE, ch, 0, 0, TO_ROOM);
        hit(ch, evil, TYPE_UNDEFINED);
        return(TRUE);
    }
    return(FALSE);
}

int rescuer(struct char_data *ch, int cmd, char *arg)
{
    void mob_light_move(struct char_data *ch, char *argument, int cmd) ;
    struct char_data *tmp_ch, *vict;
    int count ;
    struct char_data *choose_rescue_mob(struct char_data *mob) ;
    struct char_data *choose_victim(struct char_data *mob, int fmode, int mode) ;

    if(cmd) return FALSE;

    if (ch->specials.fighting && (IS_NPC(ch->specials.fighting))) {
        act("$n screams 'PROTECT THE INNOCENT! '",FALSE,ch,0,0,TO_ROOM);
        mob_light_move(ch, "", 0);
        return TRUE ;
    }

    /* find the partner */
    vict = choose_rescue_mob(ch) ;

	if ( vict == ch ) {	/* only I am fighting  */
		mob_light_move(ch, "", 0) ;
		if ( IS_SET(ch->specials.act, ACT_AGGRESSIVE)) {
			switch(number(0,3)) {
			case 0 : vict = choose_victim(ch, VIC_ALL, MODE_HIT_MIN) ; break ;
			case 1 : vict = choose_victim(ch, VIC_ALL, MODE_AC_MAX) ; break ;
			case 2 : vict = choose_victim(ch, VIC_ALL, MODE_DR_MAX) ; break ;
			case 3 : vict = choose_victim(ch, VIC_ALL, MODE_HR_MAX) ; break ;
            }
        }
		else {
			switch(number(0,3)) {
			case 0 : vict = choose_victim(ch, VIC_FIGHTING, MODE_HIT_MIN) ; break ;
			case 1 : vict = choose_victim(ch, VIC_FIGHTING, MODE_AC_MAX) ; break ;
			case 2 : vict = choose_victim(ch, VIC_FIGHTING, MODE_DR_MAX) ; break ;
			case 3 : vict = choose_victim(ch, VIC_FIGHTING, MODE_HR_MAX) ; break ;
            }
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

        if (!tmp_ch) return;

        send_to_char("!!! To the rescue...\n\r", ch);
        act("$n screams 'PROTECT THE INNOCENT! '",FALSE,ch,0,0,TO_ROOM);
        act("You are rescued by $N, you are confused!",FALSE,vict,0,ch,TO_CHAR);
        act("$n heroically rescues $N.", FALSE, ch, 0, vict, TO_NOTVICT);
        if (vict->specials.fighting == tmp_ch)
            stop_fighting(vict);
        if (tmp_ch->specials.fighting)
            stop_fighting(tmp_ch);
        if (ch->specials.fighting)
            stop_fighting(ch);
        set_fighting(ch, tmp_ch);
        set_fighting(tmp_ch, ch);
        return TRUE ;
    }

    return FALSE;
}


int superguard(struct char_data *ch, int cmd, char *arg)
{
    void do_shout(struct char_data *ch, char *argument, int cmd);
    struct char_data *tch, *criminal, *king;
    char buf[80];

    if (cmd || !AWAKE(ch) || (GET_POS(ch) == POSITION_FIGHTING))
        return (FALSE);
    criminal = 0;
    for (tch=world[ch->in_room].people; tch; tch = tch->next_in_room) {
        if(IS_NPC(tch)) continue;
        if(IS_SET(tch->specials.act,PLR_CRIMINAL)){
            criminal=tch;
            break;
        }
    }
    if(criminal){
        act("$n 외칩니다. '누가 법을 깨뜨리는 녀석이냐 !!!!'",
            FALSE, ch, 0, 0, TO_ROOM);
        hit(ch, criminal, TYPE_UNDEFINED);
        return(TRUE);
    }

    /* bow to king */
    king = 0 ;
    for (tch=world[ch->in_room].people; tch; tch = tch->next_in_room) {
        if(IS_NPC(tch)) continue;
        if(tch->equipment[HOLD]) {
            if(obj_index[tch->equipment[HOLD]->item_number].virtual==2706){
                /* it is okse */
                king=tch;
                break;
            }
        }
    }
    if (king) {
        act("$n bows before you.", FALSE,ch,0,king,TO_VICT) ;
        act("$n bows before $N.", FALSE,ch,0,king,TO_NOTVICT);
        if (number(1,5) > 4) {
            sprintf(buf, "OUR KING '%s' IS HERE !!", GET_NAME(king)) ;
            do_say(ch, buf, 0) ;
        }
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
    if (cmd== COM_LIST) { /* List */
        send_to_char("애완동물 이 이런 것이 있습니다:\n\r", ch);
        for(pet = world[pet_room].people; pet; pet = pet->next_in_room) {
			if ( !IS_NPC(pet) )
				continue ;

            sprintf(buf, "%8d - %s\n\r",10*GET_EXP(pet), pet->player.short_descr);
            send_to_char(buf, ch);
        }
        return(TRUE);
    } else if (cmd== COM_BUY ) { /* Buy */
        arg = one_argument(arg, buf);
        arg = one_argument(arg, pet_name);
        /* Pet_Name is for later use when I feel like it */
        if (!(pet = get_char_room(buf, pet_room))) {
            send_to_char("아 그런 동물은 없는데요 ?\n\r", ch);
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
            free_string_type(pet->player.name);
            pet->player.name = string_dup(buf);    

            sprintf(buf,"%sA small sign on a chain around the neck says 'My Name is %s'\n\r",
                    pet->player.description, pet_name);
            free_string_type(pet->player.description);
            pet->player.description = string_dup(buf);
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

/*
 *
 * 로봇 고물상을 위한 공간
 *  
 */
int pet_shops2(struct char_data *ch, int cmd, char *arg)
{
    char buf[MAX_STRING_LENGTH], pet_name[256];
    int pet_room,k;
    struct char_data *pet;
    struct follow_type *j;

    if(IS_NPC(ch))
        return(FALSE);
    pet_room = ch->in_room+1;
    if (cmd== COM_LIST) { /* List */
        send_to_char("로봇 종류로는 이런 것이 있습니다:\n\r", ch);
        for(pet = world[pet_room].people; pet; pet = pet->next_in_room) {
			if ( !IS_NPC(pet) )
				continue ;

            sprintf(buf, "%8d - %s\n\r",
                    (501-GET_LEVEL(ch)*10)*GET_EXP(pet), 
                    pet->player.short_descr);
            send_to_char(buf, ch);
        }
        return(TRUE);
    } else if (cmd== COM_BUY ) { /* Buy */
    	if (GET_LEVEL(ch) < 35) {
            send_to_char("아직은 로봇을 제어할 수 없습니다.\n\r", ch);

			return(TRUE);
		}
		if (GET_ALIGNMENT(ch) > -200) {
            send_to_char("로봇을 쓰기엔 당신은 너무 착하네요. :-) \n\r", ch);

			return(TRUE);
		}

        arg = one_argument(arg, buf);
        arg = one_argument(arg, pet_name);
        /* Pet_Name is for later use when I feel like it */
        if (!(pet = get_char_room(buf, pet_room))) {
            send_to_char("그런 로봇은 없는데요 ?\n\r", ch);
            return(TRUE);
        }
        for(k=0,j=ch->followers;(j)&&(k<5);++k){
            j=j->next;
        }
        if(k>=3){
            send_to_char("당신은 지금 데리고 있는 동료로도 충분할 것 같은데요.\n\r",ch);
            return(TRUE);
        }
        if (GET_GOLD(ch) < (GET_EXP(pet)*(501-GET_LEVEL(ch)*10))) {
            send_to_char("돈 더 가져오세요!\n\r", ch);
            return(TRUE);
        }
        GET_GOLD(ch) -= GET_EXP(pet)*(501-GET_LEVEL(ch)*10);
        pet = read_mobile(pet->nr, REAL);
        GET_EXP(pet) = 0;
        SET_BIT(pet->specials.affected_by, AFF_CHARM);
        if (*pet_name) {
            sprintf(buf,"%s %s", pet->player.name, pet_name);
            free_string_type(pet->player.name);
            pet->player.name = string_dup(buf);    

            sprintf(buf,"%sA small sign on a chain around the neck says 'My Name is %s'\n\r",
                    pet->player.description, pet_name);
            free_string_type(pet->player.description);
            pet->player.description = string_dup(buf);
        }

        char_to_room(pet, ch->in_room);
        add_follower(pet, ch);

        /* Be certain that pet's can't get/carry/use/wield/wear items */
/*
  And why not?

  IS_CARRYING_W(pet) = 1000;
  IS_CARRYING_N(pet) = 100;
*/
        send_to_char("이 로봇이 마음에 드실껍니다.\n\r", ch);
        act("$n님이 $N 을 데리고 갑니다.",FALSE,ch,0,pet,TO_ROOM);

        return(TRUE);
    }

    /* All commands except list and buy */
    return(FALSE);
}
int hospital(struct char_data *ch, int cmd, char *arg)
{
    char buf[MAX_STRING_LENGTH];
    int opt,lev,cost[6];
	extern long jackpot;

    lev = GET_LEVEL(ch);
    cost[0] = lev * lev * 0.7 ;
    cost[1] = 7345 + cost[0] * (GET_MAX_HIT(ch) - GET_HIT(ch)) ;
    cost[2] = 5353 + cost[0] * (GET_MAX_MANA(ch) - GET_MANA(ch)) ;
    cost[3] = 4122 + cost[0] * (GET_MAX_MOVE(ch) - GET_MOVE(ch)) ;
    cost[4] = 657 + cost[0] * 20 ;
    cost[5] = 757 + cost[0] * 30 ;

    if (cmd== COM_LIST) { /* List */
        sprintf(buf, "1 - Hit points restoration (%d coins)\n\r", cost[1], ch);
        send_to_char(buf, ch);
        sprintf(buf, "2 - Mana restoration (%d coins)\n\r", cost[2], ch);
        send_to_char(buf, ch);
        sprintf(buf, "3 - Move restoration (%d coins)\n\r", cost[3], ch);
        send_to_char(buf, ch);
        sprintf(buf, "4 - Poison cured (%d coins)\n\r", cost[4], ch);
        send_to_char(buf, ch);
        sprintf(buf, "5 - Remove curse (%d coins)\n\r", cost[5], ch);
        send_to_char(buf, ch);
        return(TRUE);
    } else if (cmd== COM_BUY ) { /* Buy */
        arg = one_argument(arg, buf);
        if ((opt = atoi(buf)) > 5) {
            send_to_char("Huh?\n\r",ch);
            return(TRUE);
        }
        if(cost[opt] > GET_GOLD(ch)){
            send_to_char("외상은 안됩니다.\n\r",ch);
            return(TRUE);
        }
        switch(opt){
        case 1:
            GET_HIT(ch) = GET_MAX_HIT(ch);
            GET_GOLD(ch) -= cost[1];
			jackpot += 0.3 * cost[1];
            send_to_char("You feel magnificent!\n\r",ch);
            WAIT_STATE(ch, PULSE_VIOLENCE/3);
            return TRUE;
        case 2:
            GET_MANA(ch) = GET_MAX_MANA(ch);
            GET_GOLD(ch) -= cost[2];
			jackpot += 0.3 * cost[2];
            send_to_char("You feel marvelous!\n\r",ch);
            WAIT_STATE(ch, PULSE_VIOLENCE/3);
            return TRUE;
        case 3:
            GET_MOVE(ch) = GET_MAX_MOVE(ch);
            GET_GOLD(ch) -= cost[3];
			jackpot += 0.3 * cost[3];
            send_to_char("You feel The GOD!\n\r",ch);
            WAIT_STATE(ch, PULSE_VIOLENCE/2);
            return TRUE;
        case 4:
            if(affected_by_spell(ch,SPELL_POISON)){
                affect_from_char(ch,SPELL_POISON);
                GET_GOLD(ch) -= cost[4];
				jackpot += 0.3 * cost[4];
                send_to_char("You feel stupendous!\n\r",ch);
            } else {
                send_to_char("Nothing wrong with you.\n\r",ch);
            }
            return TRUE;
        case 5:
            if(affected_by_spell(ch,SPELL_CURSE)){
                affect_from_char(ch,SPELL_CURSE);
                GET_GOLD(ch) -= cost[5];
				jackpot += 0.3 * cost[5];
                send_to_char("You feel blessed!\n\r",ch);
            } else {
                send_to_char("Nothing wrong with you.\n\r",ch);
            }
            return TRUE;
        default:
            send_to_char("뭐요?\n\r",ch);
            return(TRUE);
        }
    }
    return(FALSE);
}

#define COST 1000000
int metahospital(struct char_data *ch, int cmd, char *arg)
{
    char buf[MAX_STRING_LENGTH];
    char buf2[MAX_STRING_LENGTH];
    extern char *spells[];
    extern struct spell_info_type spell_info[MAX_SPL_LIST];
    int k,opt,opt2;
    int lev,cla;

    if (IS_NPC(ch))
        return(FALSE);
    if (cmd== COM_LIST ) { /* List */
		strcpy(buf, "1 - Hit points inflator(up to 2.3k)\n\r");
		strcat(buf, "2 - Mana increase(up to 1.2k)\n\r");
		strcat(buf, "3 - Movement increase(up to 1.3k)\n\r");
/* 
   strcat(buf, "4 - Freedom from hunger\n\r");
   strcat(buf, "5 - Freedom from thirst\n\r"); 
*/
		strcat(buf, "4 - Increase your practice number (+2)\n\r");
		strcat(buf, "5 - Gold, gold, gold (500-900k random)\n\r");
		strcat(buf, "6 - Increase Hitroll temporarily  (up to 4)\n\r");
		strcat(buf, "Any of these for 1,000,000 experience points?\n\r\n\r");
		strcat(buf, "9 - Change remortal class\n\r");
		strcat(buf, "Any of these for 25,000,000 experience points?\n\r\n\r");
		send_to_char(buf, ch) ;
		return(TRUE);
    } else if (cmd== COM_BUY ) { /* Buy */
        half_chop(arg,buf,buf2);
        /* arg = one_argument(arg, buf); */
        opt = atoi(buf); 
        if( (COST > GET_EXP(ch) && opt <= 6)
			|| (20000000 > GET_EXP(ch) && opt == 9)) {
            send_to_char("Come back when you are more experienced.\n\r",ch);
            return(TRUE);
        }

        if((opt >= 1) && (opt <= 6 )){
            ch->points.exp-=COST;
            switch(opt){
            case 1: k=ch->points.max_hit;
                ch->points.max_hit+= (3*(k<180)+2*(k<300)+(k<500)+(k<700)
                                      +(k<1000)+(k<1400)+(k<2000)) ;
				if ( k>= 2000 && k < 2100) {
					if (number(1,2) == 1) ch->points.max_hit++;
				}
				else if (k >= 2100 && k < 2200) {
					if (number(1,3) == 1) ch->points.max_hit++;
				}
				else if (k >= 2200 && k < 2300) {
					if (number(1,4) == 1) ch->points.max_hit++;
				}
                send_to_char("@>->->--\n\r",ch); break;

            case 2: k=ch->points.max_mana;
                ch->points.max_mana+= ((k<51)+(k<101)+(k<151)+(k<200)
                                       +(k<420)+(k<600)+(k<1200)) ;
                send_to_char("<,,,,,,,>\n\r",ch); break;
            case 3: k=ch->points.max_move;
                ch->points.max_move+= ((k<50)+(k<100)+(k<150)+(k<240)+(k<500)
                                       +(k<850)+(k<1300)) ;
                send_to_char("<xxxxx>\n\r",ch); break;
/*
  case 4: ch->specials.conditions[1]=(-1);
  send_to_char("You will never again be hungry.\n\r",ch); break;
  case 5: ch->specials.conditions[2]=(-1);
  send_to_char("You will never again be thirsty.\n\r",ch); break;
*/
            case 4: ch->specials.spells_to_learn++;
                ch->specials.spells_to_learn++;
                send_to_char("Aaaaaaarrrrrrrrggggg\n\r",ch); break;
			case 5: GET_GOLD(ch) += 500000 + 1000 * number(0,400) ;
				send_to_char("$$$$$\n\r", ch) ;
				break ;
			case 6:
				if (ch->points.hitroll >=0 && ch->points.hitroll < 4) {
					ch->points.hitroll++;
                    send_to_char_han("Your hitroll point is increased !\n\r",
                                     "당신의 정확도가 좋아짐을 느낍니다.\n\r",
                                     ch) ;
				}
				break;
            default:
                break;
            }
            save_char(ch, NOWHERE) ;
		} else if (opt == 9) {
			char o_class = GET_CLASS_O(ch);

        	if ( !is_solved_quest(ch, QUEST_REMORTAL) ) {
				send_to_char_han("Only remortal character can change class.\n\r", 
					"Remortal한 char만이 class를 변경할 수 있습니다.\n\r",ch);
				return TRUE;
			}
            ch->points.exp-=25000000;

			ch->player.class = GET_CLASS(ch);
			ch->player.class <<= 4;
			ch->player.class += o_class;

			send_to_char_han("Class is changed.\n\r", 
				"클래스 전직을 마쳤습니다.\n\r", ch);

            save_char(ch, NOWHERE) ;
        } else {
            send_to_char_han("What?\n\r", "예?\n\r",ch);
        }
        return(TRUE);
    }
    return(FALSE);
}

int metahospital2(struct char_data *ch, int cmd, char *arg)
{
    char buf[MAX_STRING_LENGTH];
    char buf2[MAX_STRING_LENGTH];
    int k,opt,opt2;
    int lev,cla;
    // long cost = titles[GET_CLASS(ch)-1][40].exp * 0.4;
	long cost = 1000000 * 140;		// 140 M

    if (IS_NPC(ch))
        return(FALSE);
    if (cmd== COM_LIST ) { /* List */
		strcpy(buf, "1 - Strength(up to 18/100)\n\r");
		strcat(buf, "2 - Wisdom(up to 18)\n\r");
		strcat(buf, "3 - Intelligence(up to 18)\n\r");
		strcat(buf, "4 - Dexterity(up to 18)\n\r");
        strcat(buf, "5 - Consititution(up to 18)\n\r");
//        strcat(buf, "6 - Hit roll(up to 8)\n\r");
        sprintf(buf2, "Any of these for %d experiences points?\n\r", cost);
		strcat(buf, buf2);
		send_to_char(buf, ch) ;
		return(TRUE);
    } else if (cmd== COM_BUY ) { /* Buy */
        half_chop(arg,buf,buf2);
        /* arg = one_argument(arg, buf); */
        opt = atoi(buf); 
        if( cost > GET_EXP(ch) ) {
            send_to_char("Come back when you are more experienced.\n\r",ch);
            return(TRUE);
        }
        if((opt >= 1) && (opt <= 5 )){
            ch->points.exp-= cost;
            switch(opt){
            case 1:
                /* Strength bonus */
                if ( ch->abilities.str < 18 ) {
                    ch->abilities.str ++ ;
                }
                else if ( ch->abilities.str_add < 90 ) {
                    ch->abilities.str_add += 10 ;
                }
                else {
                    ch->abilities.str_add = 100 ;
                }
                affect_total(ch) ;
                send_to_char_han("Your strength increased !\n\r",
                                 "당신의 힘이 증가 하였습니다.\n\r", ch) ;
                save_char(ch, NOWHERE) ;
                break;
            case 2:
                if (ch->abilities.wis < 18 ) {
                    ch->abilities.wis ++ ;
                    send_to_char_han("Your wisdom point increased !\n\r",
                                     "조금 영리해진 기분이 듭니다.\n\r",
                                     ch) ;
                    save_char(ch, NOWHERE) ;
                }
                break;

            case 3:
                if (ch->abilities.intel < 18 ) {
                    ch->abilities.intel ++ ;
                    send_to_char_han("Your intelligence point increased !\n\r",
                                     "조금 똑똑해진 기분이 듭니다.\n\r",
                                     ch) ;
                    save_char(ch, NOWHERE) ;
                }
                break;
            case 4:
                if (ch->abilities.dex < 18 ) {
                    ch->abilities.dex ++ ;
                    send_to_char_han("Your dexterity point increased !\n\r",
                                     "조금 민첩해진 느낌이 듭니다.\n\r",
                                     ch) ;
                    save_char(ch, NOWHERE) ;
                }
                break;

                
            case 5:
                if (ch->abilities.con < 18 ) {
                    ch->abilities.con ++ ;
                    send_to_char_han("Your constitution point increased !\n\r",
                                     "당신의 몸이 좋아짐을 느낍니다.\n\r",
                                     ch) ;
                    save_char(ch, NOWHERE) ;
                }
                break;
			case 6:
				if (ch->points.hitroll < 8) {
					ch->points.hitroll++;
                    send_to_char_han("Your hitroll point is increased !\n\r",
                                     "당신의 정확도가 좋아짐을 느낍니다.\n\r",
                                     ch) ;
                    save_char(ch, NOWHERE) ;
				}
				break;
            default:
                break;
            }
        } else {
            send_to_char_han("What?\n\r", "예?\n\r",ch);
        }
        return(TRUE);
    }
    return(FALSE);
}

// remortal house
// 
int remortal_house(struct char_data *ch, int cmd, char *arg)
{
    char buf[MAX_STRING_LENGTH];
    char buf2[MAX_STRING_LENGTH];
    int k,opt,opt2;
    int lev,cla;
    static int done = 0;
    long cost = titles[GET_CLASS(ch)-1][40].exp * 0.5;

    if (IS_NPC(ch))
        return(FALSE);
    if (cmd== COM_LIST ) { /* List */
		strcpy(buf, "1 - Change class to MAGIC USER\n\r");
		strcat(buf, "2 - Change class to CLERIC\n\r");
		strcat(buf, "3 - Change class to WARRIOR\n\r");
		strcat(buf, "4 - Change class to THIEF\n\r");
        sprintf(buf2, "Any of these for %d experiences points?\n\r", cost);
		strcat(buf, buf2);
		send_to_char(buf, ch) ;
		return(TRUE);
    } else if (cmd== COM_BUY ) { /* Buy */
        half_chop(arg,buf,buf2);
        /* arg = one_argument(arg, buf); */
        opt = atoi(buf); 
        if( cost > GET_EXP(ch) ) {
            send_to_char("Come back when you are more experienced.\n\r",ch);
            return(TRUE);
        }
        if (opt < 1 || opt > 4) {
            send_to_char_han("What?\n\r", "예?\n\r",ch);
                        
            return(TRUE);
        }

        if (GET_MAX_HIT(ch) < 1500) {
            send_to_char("Remortal 하려면 hit 가 1500을 넘어야 합니다.\n\r", ch);
            return(TRUE);
        }

        if (done) {
            send_to_char("한번에 한명만 Remortal이 가능합니다.\n\r", ch);
            return(TRUE);
        }
        
        if (GET_LEVEL(ch) < IMO -1) {
            send_to_char_han("Only dangun can change class\n\r",
                             "단군만이 전직할 수 있습니다\n\r",ch);
                        
            return(TRUE);
        }
        if ( is_solved_quest(ch, QUEST_REMORTAL) ) {
            send_to_char("You can change class only one times.\n\r", ch);
            return(TRUE);
        }
		set_solved_quest(ch, QUEST_REMORTAL);

        ch->points.exp = 0;
        ch->player.class <<= 4;
        ch->player.time.birth = time(0);
        GET_LEVEL(ch) = 1;

        switch(opt){
        case 1: 
            // magic user
            ch->player.class += CLASS_MAGIC_USER;
            send_to_char("Magic user로 전직하였습니다.\n\r", ch);
            save_char(ch, NOWHERE) ;
            break;
        case 2:
            // cleric
            ch->player.class += CLASS_CLERIC;
            send_to_char("Cleric로 전직하였습니다.\n\r", ch);

            save_char(ch, NOWHERE) ;
            break;
        case 3:
            // warrior
            ch->player.class += CLASS_WARRIOR;
            send_to_char("Warrior로 전직하였습니다.\n\r", ch);

            save_char(ch, NOWHERE) ;
            break;
        case 4:
            // thief
            ch->player.class += CLASS_THIEF;
            send_to_char("Thief로 전직하였습니다.\n\r", ch);
            
            save_char(ch, NOWHERE) ;
            break;
        default:
            break;
        }

        done = 1;
        
        return(TRUE);
    }
    return(FALSE);
}

int safe_house(struct char_data *ch, int cmd, char *arg)
{
    char buf[100];

    if(GET_LEVEL(ch) >= (IMO+2))
        return FALSE;
    switch (cmd) {
        // case COM_RECITE:	/* recite */
    case COM_KILL: 	/* kill */
    case COM_HIT:	/* hit */
    case COM_ORDER:	/* order */
    case COM_BACKSTAB:	/* backstab */
    case COM_STEAL:	/* steal */
    case COM_BASH:	/* bash */
    case COM_RESCUE:	/* kick */
    case COM_USE:	/* use */
    case COM_SHOOT:	/* shoot */
    case COM_TORNADO:	/* tornado */
    case COM_FLASH:	/* flash */
	case COM_THROW:	/* throw */
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
        for (tch=world[ch->in_room].people; tch; tch = tch->next_in_room){
            if((!IS_NPC(tch))&&(GET_LEVEL(tch) < IMO)){
                if(GET_POS(tch) <= POSITION_DEAD) continue;
                act("$n yells '$N must die!'",FALSE,ch,0,tch,TO_ROOM);
                shoot(ch, tch, TYPE_SHOOT);
            }
        }
    return(FALSE);
}

int bank(struct char_data *ch, int cmd, char *arg)
{
    char buf[MAX_STRING_LENGTH];
    int k,amt;

    if (IS_NPC(ch))
        return(FALSE);
    if (cmd== COM_LIST) { /* List */
        send_to_char("At the bank you may:\n\r\n\r", ch);
        send_to_char("list - get this list\n\r",ch);
        send_to_char("balance - check your balance\n\r",ch);
        send_to_char("deposit all|<amount>, or\n\r",ch);
        send_to_char("withdraw all|<amount>\n\r\n\r",ch);
        return(TRUE);
    } else if (cmd== COM_BALANCE) { /* Balance */
        sprintf(buf,"You have %d coins in the bank.\n\r",ch->bank);
        send_to_char(buf,ch);
        return(TRUE);
    } else if ((cmd== COM_DEPOSIT)||(cmd== COM_WITHDRAW)) {
        /* deposit or withdraw */
        if(! *arg){
            send_to_char("The banker says 'Specify an amount.'\n\r",ch);
            return(TRUE);
        }
        arg=one_argument(arg, buf);
		if (!strcmp(buf, "all")) {
			if (cmd == COM_DEPOSIT) amt = GET_GOLD(ch);
			else amt = ch->bank;
		}
        else amt=atoi(buf);

        if(amt <= 0){
            send_to_char("The banker says 'Amount must be positive.'\n\r",ch);
            return(TRUE);
        }
        if(cmd== COM_DEPOSIT){
            if(amt > GET_GOLD(ch)){
                send_to_char("The banker says 'You don't have that much.'\n\r",ch);
                return(TRUE);
            }
            GET_GOLD(ch)-=amt;
            ch->bank+=amt;
        } else {	/* withdraw */
            if(amt > ch->bank){
                send_to_char("The banker says 'We don't make loans.'\n\r",ch);
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

    if(cmd!= COM_LOOK ) return(FALSE);
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

	if(cmd)
		return(FALSE);

	if(vict=ch->specials.fighting){
		if ( ch->in_room != vict->in_room ) {	/* not in same room */
			stop_fighting(ch) ;
			return FALSE ;
        }
		if (number(1,2)==1) do_kick(ch,"",0);
		else do_bash(ch,"",0);
		if (GET_LEVEL(ch)>20) {
			if (number(1,2)==1) do_kick(ch,"",0);
			else do_bash(ch,"",0);
        }
		if (GET_LEVEL(ch)>30){
			if(number(1,2)==1) do_kick(ch,"",0);
			else do_bash(ch,"",0);
        }
		if (GET_LEVEL(ch)>35){
			if (number(1,2)==1) do_kick(ch,"",0);
			else do_bash(ch,"",0);
        }
		if (GET_LEVEL(ch)>40){
			if (number(1,2)==1) do_kick(ch,"",0);
			else do_bash(ch,"",0);
        }
		return(TRUE);
    }
    return(FALSE);
}

int spell_blocker(struct char_data *ch, int cmd, char *arg)
{
    if(cmd== COM_CAST ){
        do_say(ch,"하하하, 여기서는 마술을 못쓰지!",0);
        return TRUE;
    }
    else return FALSE;
}

// add by jmjeong (2/8/00)
//
int yuiju(struct char_data *ch, int cmd, char *arg)
{
	int h;
	char	buf[BUFSIZ] ;

    if(cmd == COM_CAST ){
		send_to_char("상서로운 기운이 정신집중을 막습니다.\n\r",ch);

        return TRUE;
    }
    else if (cmd) return FALSE;

    h=GET_HIT(ch);
    if(h < 5000) {
		if (ch->equipment[HOLD]) {
	        act("여의주로부터 맑고 고운 빛이 $n을 감쌉니다.",1,ch,0,0,TO_ROOM);
   	     	cast_full_heal(GET_LEVEL(ch), ch, "", SPELL_TYPE_SPELL, ch, 0);
		}
	}
	return(FALSE);
}

int archmage(struct char_data *ch, int cmd, char *arg)
{
    struct char_data *vict;
    void do_shout(struct char_data *ch, char *argument, int cmd);
	struct descriptor_data *i ;
	char	buf[BUFSIZ] ;

    if(cmd) return FALSE;
    if(!ch->specials.fighting) return FALSE;

	sprintf(buf, "$n shouts 'SUNFIRE'") ;

	for ( i = descriptor_list ; i ; i = i->next) {
		if (i->character != ch && !i->connected &&
			!IS_SET(i->character->specials.act, PLR_EARMUFFS))
			act(buf, 0, ch, 0, i->character, TO_VICT);
    }

    act("The outside sun flares brightly, flooding the room with searing rays.",
        TRUE, ch, 0, 0, TO_ROOM);
    for(vict=world[ch->in_room].people; vict; vict = vict->next_in_room ){
        if(vict->specials.fighting==ch){
            if (!IS_NPC(vict)) damage(ch, vict, dice(100,2), TYPE_UNDEFINED);
        }
    }
    return TRUE;
}

int spitter(struct char_data *ch, int cmd, char *arg)
{
    struct char_data *vict;
    void cast_blindness(byte level,struct char_data *ch,char *arg,int type,
                        struct char_data *victim, struct obj_data *tar_obj );

    if(cmd== COM_STEAL ){
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

    if(cmd != COM_SOUTH)   /* specific to Room 2176 */
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
    int skno, maxtry;
    char name[256];
    struct obj_data *obj;
    extern struct spell_info_type spell_info[MAX_SPL_LIST];

    if(cmd!= COM_EAT ) return(FALSE);
    one_argument(arg,name);
    obj=get_obj_in_list_vis(ch,name,ch->carrying);
    if(!obj) return(FALSE);
    if(GET_COND(ch,FULL)>20)  
        return(FALSE);
    if(obj_index[obj->item_number].virtual != 2157)	/* seed */
        return(FALSE);
    /* cyb
       skno=number(1,54);
       if(skno > 46) skno+=8;
    */
	skno = number(1,MAX_SPL_LIST) ;
	maxtry = 0 ;
	while ( ++maxtry < 20) {
		if ( spell_info[skno].min_level[GET_CLASS(ch)-1] < GET_LEVEL(ch) &&
             (ch->skills[skno].learned >= 40) )
			break ;
		else
			skno = number(1, MAX_SPL_LIST) ;	/* find skill */
    }

/*
  if ( maxtry < 20 && number(0,4)!=0 ) {	// added by cyb 
  ch->skills[skno].learned=99;
  }
*/

	if ( maxtry < 20 && number(0,4)>1 ) {	/* added by jmjeong */
		if (ch->skills[skno].learned < 99) {
			ch->skills[skno].learned = 99;
		}
    }
	send_to_char("자욱한 연기가 눈앞을 가립니다.\n\r",ch);

	return(FALSE);
}
