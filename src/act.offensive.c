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
#include "quest.h"

/* extern variables */

extern struct room_data *world;
extern struct descriptor_data *descriptor_list;
extern struct char_data *character_list ;

void raw_kill(struct char_data *ch, int level, int isgroup);


void do_hit(struct char_data *ch, char *argument, int cmd)
{
    char arg[MAX_STRING_LENGTH];
    char buffer[MAX_STRING_LENGTH];
    struct char_data *victim;
    extern int nokillflag;

    one_argument(argument, arg);

    if (*arg) {
        victim = get_char_room_vis(ch, arg);
        if (victim) {
            if (victim == ch) {
                send_to_char("You hit yourself..OUCH!.\n\r", ch);
                act("$n hits $mself, and says OUCH!", FALSE, ch, 0, victim, TO_ROOM);
            } else {

                /* this is new.. made death place */
                if(nokillflag) {
                    if((!IS_NPC(ch))&&(!IS_NPC(victim))) {
                        if ( !IS_SET(world[ch->in_room].room_flags, DEATH_PLACE) )
                            return ;
					}
				}

                /* it is old..
                   if(nokillflag)
                   if((!IS_NPC(ch)) && (!IS_NPC(victim))) return;
                */

                if (IS_AFFECTED(ch, AFF_CHARM) && (!IS_NPC(victim))) return;
                if ((GET_POS(ch)==POSITION_STANDING) &&
                    (victim != ch->specials.fighting)) {
                    hit(ch, victim, TYPE_UNDEFINED);
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
                raw_kill(victim, GET_LEVEL(ch), 0);
            }
    }
}

void do_backstab(struct char_data *ch, char *argument, int cmd)
{
    struct char_data *victim;
    char name[256];
    byte percent;

	if ( *argument == 0 ) {
		send_to_char("Backstab who?\n\r", ch) ;
		return ;
    }

    one_argument(argument, name);

    if (!(victim = get_char_room_vis(ch, name))) {
        send_to_char("Backstab who?\n\r", ch);
        return;
    }
    if (victim == ch) {
        send_to_char("How can you sneak up on yourself?\n\r", ch);
        return;
    }
    if (!ch->equipment[WIELD]) {
        send_to_char("You need to wield a weapon, to make it a succes.\n\r",ch);
        return;
    }
    if (ch->equipment[WIELD]->obj_flags.value[3] != 11) {
        send_to_char("Only piercing weapons can be used for backstabbing.\n\r",ch);
        return;
    }
    if (ch->specials.fighting) {
        send_to_char("You can't backstab while fighting, too alert!\n\r", ch);
        return;
    }
    percent=number(1,101)+GET_LEVEL(victim); /* 101% is a complete failure */
    if (AWAKE(victim) && (percent > ch->skills[SKILL_BACKSTAB].learned))
        damage(ch, victim, 0, SKILL_BACKSTAB);
    else
        hit(ch,victim,SKILL_BACKSTAB);
    
    WAIT_STATE(ch, PULSE_VIOLENCE/8); /* new tick system */
}

void do_order(struct char_data *ch, char *argument, int cmd)
{
    char name[100], message[256];
    char buf[256];
    bool found = FALSE;
    int org_room;
    struct char_data *victim;
    struct follow_type *k;

	if (IS_SET(ch->specials.affected_by, AFF_BERSERK)) {
		send_to_char("You can think of nothing but the battle!\n\r",ch);
		return;
	}

    half_chop(argument, name, message);

    victim = NULL ;
    if (!*name || !*message)
        send_to_char("Order who to do what?\n\r", ch);
    else if (!(victim = get_char_room_vis(ch, name)) &&
             str_cmp("follower", name) && str_cmp("followers", name))
        send_to_char("That person isn't here.\n\r", ch);
    else if (ch == victim)
        send_to_char("You decline to do it.\n\r", ch);
    else {
        if (IS_AFFECTED(ch, AFF_CHARM)) {
            send_to_char("Your superior would not approve.\n\r",ch);
            return;
        }
		WAIT_STATE(ch, PULSE_VIOLENCE*1/2);
        if (victim) {
            sprintf(buf, "$N orders you to '%s'", message);
            act(buf, FALSE, victim, 0, ch, TO_CHAR);
            act("$n gives $N an order.", FALSE, ch, 0, victim, TO_ROOM);
            if ( (victim->master!=ch) || !IS_AFFECTED(victim, AFF_CHARM) )
                act("$n has an indifferent look.", FALSE, victim, 0, 0, TO_ROOM);
            else {
                send_to_char("Ok.\n\r", ch);
                command_interpreter(victim, message);
            }
        } else {  /* This is order "followers" */
            sprintf(buf, "$n issues the order '%s'.", message);
            act(buf, FALSE, ch, 0, victim, TO_ROOM);

            org_room = ch->in_room;

            for (k = ch->followers; k; k = k->next) {
                if (org_room == k->follower->in_room)
                    if (IS_AFFECTED(k->follower, AFF_CHARM)) {
                        found = TRUE;
                        command_interpreter(k->follower, message);
                    }
            }
            if (found)
                send_to_char("Ok.\n\r", ch);
            else
                send_to_char("Nobody here are loyal subjects of yours!\n\r", ch);
        }
    }
    if (victim) WAIT_STATE(victim, PULSE_VIOLENCE);
}

void do_flee(struct char_data *ch, char *argument, int cmd)
{
    int i, attempt, loose, die;
    struct char_data *tmp_victim, *temp ;

    void gain_exp(struct char_data *ch, int gain);
    int special(struct char_data *ch, int cmd, char *arg);

	if (IS_SET(ch->specials.affected_by, AFF_BERSERK)) {
		send_to_char("You can think of nothing but the battle!\n\r",ch);
		return;
	}
	if (IS_SET(ch->specials.affected_by, AFF_PARALYSIS)) {
		send_to_char("You wanted to flee, but you couldn't!\n\r",ch);
		return;
	}

	if (!(ch->specials.fighting)) {
		act("$n panics, and attempts to flee. But not really", TRUE,
			ch, 0, 0, TO_ROOM);
		send_to_char("PANIC! You couldn't escape! ... (strange?)\n\r", ch);
		return;
    }

    for(i=0; i<6; i++) {
        attempt = number(0, 5);  /* Select a random direction */
        if (CAN_GO(ch, attempt)) {
            act("$n panics, and attempts to flee.", TRUE, ch, 0, 0, TO_ROOM);

            if ((die = do_simple_move(ch, attempt, FALSE))== 1) { 
                /* The escape has succeded */
                send_to_char("You flee head over heels.\n\r", ch);
                /* Insert later when using huntig system        */
                /* ch->specials.fighting->specials.hunting = ch */

                loose = GET_MAX_HIT(ch->specials.fighting)-
                    GET_HIT(ch->specials.fighting);
                loose *= GET_LEVEL(ch->specials.fighting);
                if (!IS_NPC(ch) && 
					( !IS_SET(world[ch->in_room].room_flags, DEATH_PLACE) )) {
                    /* added by jmjeong */
                    if(GET_LEVEL(ch)>=20 && GET_EXP(ch)<1000000
                        && number(1,100) <= 2) {
                        send_to_char("You lost your hit, mana, move points\r\n", 
							ch);
                        ch->points.max_hit -=
                            MIN(ch->points.max_hit/50, 3);
                        ch->points.max_move -=
                            MIN(ch->points.max_move/50, 2);
                        ch->points.max_mana -=
                            MIN(ch->points.max_mana/50, 2);
                    }
                    gain_exp(ch, -loose);
                }
                
                /* stop fighting who was fighting against ch */
                for(tmp_victim = character_list; tmp_victim; tmp_victim = temp) {
                    temp = tmp_victim->next;
                    if (tmp_victim->specials.fighting == ch)
                        stop_fighting(tmp_victim);
                }

                /* cyb  if (ch->specials.fighting->specials.fighting == ch)
                   stop_fighting(ch->specials.fighting);
                */
                stop_fighting(ch);
                return;
            } else {
                if (!die) act("$n tries to flee, but is too exhausted!", TRUE, ch, 0, 0, TO_ROOM);
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
    char name[256], buf[256];
    byte percent;

    one_argument(argument, name);

    if (!(victim = get_char_room_vis(ch, name))) {
        if (ch->specials.fighting) {
            victim = ch->specials.fighting;
        } else {
            send_to_char("Bash who?\n\r", ch);
            return;
        }
    }

    if (victim == ch) {
        send_to_char("Aren't we funny today...\n\r", ch);
        return;
    }
    if (!ch->equipment[WIELD]) {
        send_to_char("You need to wield a weapon, to make it a success.\n\r",ch);
        return;
    }
    percent=number(1,101); /* 101% is a complete failure */
    if (percent > ch->skills[SKILL_BASH].learned) {
        damage(ch, victim, 0, SKILL_BASH);
        GET_POS(ch) = POSITION_SITTING;
    } else {
        damage(ch, victim, 2+GET_LEVEL(ch)/2, SKILL_BASH);
        percent=number(1,300);
        if(percent < GET_LEVEL(ch)) GET_POS(victim) = POSITION_STUNNED;
        else GET_POS(victim) = POSITION_SITTING;
        WAIT_STATE(victim, PULSE_VIOLENCE*2);
    }
    WAIT_STATE(ch, PULSE_VIOLENCE*2);
}

void do_rescue(struct char_data *ch, char *argument, int cmd)
{
    struct char_data *victim, *tmp_ch;
    int percent;
    char victim_name[240];
    char buf[240];

	if (IS_SET(ch->specials.affected_by, AFF_BERSERK)) {
		send_to_char("You can think of nothing but the battle!\n\r",ch);
		return;
	}

    one_argument(argument, victim_name);

    if (!(victim = get_char_room_vis(ch, victim_name))) {
        send_to_char("Who do you want to rescue?\n\r", ch);
        return;
    }
    if (victim == ch) {
        send_to_char("What about fleeing instead?\n\r", ch);
        return;
    }
  
    if (IS_NPC(victim) && !IS_NPC(ch)){ /* enable mob's rescue  ... cyb */
        send_to_char("But why?\n\r",ch);
        return;
    }
    if (ch->specials.fighting == victim) {
        send_to_char("How can you rescue someone you are trying to kill?\n\r",ch);
        return;
    }
    for (tmp_ch=world[ch->in_room].people; tmp_ch &&
             (tmp_ch->specials.fighting != victim); tmp_ch=tmp_ch->next_in_room)  ;
    if (!tmp_ch) {
        act("But nobody is fighting $M?", FALSE, ch, 0, victim, TO_CHAR);
        return;
    }
    percent=number(1,101); /* 101% is a complete failure */
    if (percent > ch->skills[SKILL_RESCUE].learned) {
        act("You fail to rescue $N.", FALSE, ch, 0, victim, TO_CHAR);
        // send_to_char("You fail the rescue.\n\r", ch);
        return;
    }
    send_to_char("Banzai! To the rescue...\n\r", ch);
    act("You are rescued by $N, you are confused!",FALSE,victim,0,ch,TO_CHAR);
    act("$n heroically rescues $N.", FALSE, ch, 0, victim, TO_NOTVICT);
    if (victim->specials.fighting == tmp_ch)
        stop_fighting(victim);
    if (tmp_ch->specials.fighting)
        stop_fighting(tmp_ch);
    if (ch->specials.fighting)
        stop_fighting(ch);
    set_fighting(ch, tmp_ch);
    set_fighting(tmp_ch, ch);
    WAIT_STATE(victim, 2*PULSE_VIOLENCE);
}

void do_berserk(struct char_data *ch, char *argument, int cmd)
{
    struct char_data *victim;
    byte percent;
    char name[256], buf[256];
    int i;
    struct char_data *tmp_victim, *temp;
	extern struct char_data *character_list;

    victim = ch->specials.fighting;
    if(victim==0){
        send_to_char("This skill can be used only in fighting!\n\r",ch);
        return;
    }

    one_argument(argument, name);
	if ( *name == 0 ) {
		if ( ch->specials.fighting)
			victim = ch->specials.fighting ;
		else {
			send_to_char("You need to begin fighting before you can go berserk.\n\r", ch);

			return ;
		}
    }
    else if (!(victim = get_char_room_vis(ch, name))) {
        if (ch->specials.fighting) {
            victim = ch->specials.fighting;
        } else {
			send_to_char("You need to begin fighting before you can go berserk.\n\r", ch);
            return;
        }
    }
    if (victim == ch) {
        send_to_char("Aren't we funny today...\n\r", ch);
        return;
    }
	if (IS_SET(ch->specials.affected_by, AFF_BERSERK)) {
   		send_to_char("You are already berserked!\n\r",ch);
      	return;
	}
    if ( ch->in_room != victim->in_room ) {
		send_to_char("Fleed already ?\n\r", ch) ;
		return ;
    }
    if(GET_MANA(ch) < 0){
        send_to_char("You are too exhausted!\n\r",ch);
        return;
    }

    percent=number(1,101);

    if (percent > ch->skills[SKILL_BERSERK].learned) {
		act("$n growls at $mself, and looks very angry!", FALSE, ch, 0, 
				victim , TO_ROOM);
		act("You can't seem to get mad enough right now.",FALSE,ch,0,0,
				TO_CHAR);

		GET_MANA(ch) -= 25 - GET_LEVEL(ch)/8;

    } else {
		if (GET_POS(victim) > POSITION_DEAD) {
			GET_MANA(ch) -= 65 - GET_LEVEL(ch)/8;

			SET_BIT(ch->specials.affected_by,AFF_BERSERK);
			act("$n growls at $mself, and whirls into a killing frenzy!", FALSE,
							  ch, 0, victim, TO_ROOM);
			act("The madness overtakes you quickly!",FALSE,ch,0,0,TO_CHAR);

			for(tmp_victim = character_list; tmp_victim; tmp_victim = temp) {
				temp = tmp_victim->next;
				if ( ch->in_room != tmp_victim->in_room  &&
					world[ch->in_room].zone == world[tmp_victim->in_room].zone){
					send_to_char("You hear the frightning sound.\n\r", 
							tmp_victim);
				}
			}
		}
		WAIT_STATE(victim, PULSE_VIOLENCE);
    }
	WAIT_STATE(ch, PULSE_VIOLENCE*1.5);
}

void do_multi_kick(struct char_data *ch, char *argument, int cmd)
{
    struct char_data *victim;
    byte percent;
    int i;
	int class;

	if (is_solved_quest(ch, QUEST_REMORTAL)) class= GET_CLASS_O(ch);
	else class = GET_CLASS(ch);

    victim = ch->specials.fighting;
    if(victim==0){
        send_to_char("This skill can be used only in fighting!\n\r",ch);
        return;
    }
    if(GET_MOVE(ch)<0 || GET_MANA(ch) < 0){
        send_to_char("You are too exhausted!\n\r",ch);
        return;
    }
    GET_MOVE(ch) -= 18 - GET_LEVEL(ch)/8;
    GET_MANA(ch) -= 9 - GET_LEVEL(ch)/10;

    for(i=0;i<GET_LEVEL(ch)/5;i++){
		if ( victim && ch->in_room == victim->in_room) {
			percent=((10-(GET_AC(victim)/10))<<1)+number(1,101) - 
				((class==CLASS_WARRIOR) ? 0 : number(5,10));
			if (percent > ch->skills[SKILL_MULTI_KICK].learned) {
				damage(ch, victim, 0, SKILL_KICK);
            }
			else {
				damage(ch,victim,GET_LEVEL(ch),SKILL_KICK);
            }
			if(number(1,GET_LEVEL(ch)/4)==1) break;
        }
    }
    WAIT_STATE(ch, PULSE_VIOLENCE*2.5);
}

void do_kick(struct char_data *ch, char *argument, int cmd)
{
    struct char_data *victim;
    char name[256], buf[256];
    byte percent;

    one_argument(argument, name);

	if ( *name == 0 ) {
		if ( ch->specials.fighting)
			victim = ch->specials.fighting ;
		else
			return ;
    }
    else if (!(victim = get_char_room_vis(ch, name))) {
        if (ch->specials.fighting) {
            victim = ch->specials.fighting;
        } else {
            send_to_char("Kick who?\n\r", ch);
            return;
        }
    }
    if (victim == ch) {
        send_to_char("Aren't we funny today...\n\r", ch);
        return;
    }
    if ( ch->in_room != victim->in_room ) {
		send_to_char("Fleed already ?\n\r", ch) ;
		return ;
    }
    percent=((10-(GET_AC(victim)/10))<<1)+number(1,101);
    if (percent > ch->skills[SKILL_KICK].learned) {
        damage(ch, victim, 0, SKILL_KICK);
    } else {
        damage(ch,victim,GET_LEVEL(ch),SKILL_KICK);
    }

	/* for chamie mob */
    if (IS_NPC(ch)) {
        if (ch->master) WAIT_STATE(ch->master, PULSE_VIOLENCE*2);
    }
    else {
        WAIT_STATE(ch, PULSE_VIOLENCE*2);
    }
    
}

void do_tornado(struct char_data *ch, char *argument, int cmd)
{
	struct char_data *tch, *tch2;

	if ( GET_MOVE(ch) > 0 ) {
		GET_MOVE(ch) -= 18 - GET_LEVEL(ch)/5;
		for (tch=world[ch->in_room].people; tch; tch = tch2) {
			tch2 = tch->next_in_room;
#ifndef DEATHFIGHT
			if (!IS_NPC(tch)) continue ;
#endif 
			if (ch->skills[SKILL_TORNADO].learned>number(1,99)) {
				hit(ch, tch, TYPE_UNDEFINED); 
            }
        }
    }
	WAIT_STATE(ch, PULSE_VIOLENCE*1);
}

void do_light_move(struct char_data *ch, char *argument, int cmd)
{
    struct char_data *victim;
	int class;

	if (is_solved_quest(ch, QUEST_REMORTAL)) class= GET_CLASS_O(ch);
	else class = GET_CLASS(ch);

    if((victim=ch->specials.fighting)){
        if (ch->skills[SKILL_LIGHT_MOVE].learned>10 && ch->points.mana>0) {
            hit(ch,victim,TYPE_UNDEFINED);
			ch->points.mana-= 2;
            if((GET_LEVEL(ch)+1)*number(2,3)+ch->skills[SKILL_LIGHT_MOVE].learned/2 
				- ((class == CLASS_THIEF) ? 0 : number(5,10))
               > GET_LEVEL(victim)*number(2,3)+number(30,40))
            {	   
				ch->points.mana-=3+GET_LEVEL(ch)/4;	
				if(ch->specials.fighting) stop_fighting(ch); 
            	if(victim->specials.fighting) stop_fighting(victim);
				send_to_char("You succeed.\n\r",ch);
            }
			else {
				send_to_char("You failed.\n\r",ch);
			}
        }
        WAIT_STATE(ch, PULSE_VIOLENCE*2/3);
    }
    else {	
		send_to_char("this skill can used only in fighting.\n\r",ch);
    }
}
void do_flash(struct char_data *ch, char *argument, int cmd)
{
    struct char_data *victim;
    char name[BUFSIZ], buf[BUFSIZ];
    byte percent;
	int class;

    if(GET_LEVEL(ch) < 5) return ;

    name[0] = 0 ;
    one_argument(argument, name);

    if ( name[0] == 0 ) {
        if (ch->specials.fighting) {
            victim = ch->specials.fighting;
        } else {
            send_to_char("flash who?\n\r", ch);
            return;
        }
    }
    else if ((victim = get_char_room_vis(ch, name)) == NULL ) {
        send_to_char("flash who?\n\r", ch);
        return;
    }

    if (victim == ch) {
        send_to_char("you are foolish..\n\r", ch);
        return;
    }

	if (is_solved_quest(ch, QUEST_REMORTAL)) class= GET_CLASS_O(ch);
	else class = GET_CLASS(ch);

    percent=(!IS_NPC(ch) && class ==CLASS_THIEF)?
        number(1,101)+GET_LEVEL(victim)*2-GET_LEVEL(ch) : 
			101 - number(1,GET_LEVEL(ch)/5);
    WAIT_STATE(ch, PULSE_VIOLENCE*0.8);
    if ( GET_MOVE(ch) <= 0 && number(1,10) < 5 ) {
        send_to_char("You can't get chance ...\n\r", ch);
        act("$n try flash attack $N, but failed.", FALSE,ch,0,victim, TO_NOTVICT);
        return ;
    }
    else if (percent > ch->skills[SKILL_FLASH].learned) {
        send_to_char("You can't get chance ...\n\r", ch);
        act("$n try flash attack $N, but failed.", FALSE,ch,0,victim, TO_NOTVICT);
        damage(ch, victim, 0, SKILL_FLASH);
        ch->points.hit-=1;
    } else {
        send_to_char("You attack like flash ...\n\r", ch);
        act("You can star above your head ?!? by $N, you are confused!",FALSE,
            victim,0,ch,TO_CHAR);
        act("$n strike $N's head with circular spanning triple kick .", FALSE,
            ch, 0, victim, TO_NOTVICT);
        act("$n throw $N into sun with double tumbling.", FALSE, ch, 0, victim,
            TO_NOTVICT);
        act("$n jump kick down  $N's heart with making beautifle circle.", FALSE,
            ch, 0, victim, TO_NOTVICT);
        act("$n punch $N's head line drive.", FALSE, ch, 0, victim, TO_NOTVICT);
        damage(ch, victim, GET_MOVE(ch)+GET_LEVEL(ch), SKILL_FLASH);
        ch->points.move=0;
    }
}

void shoot(struct char_data *ch, struct char_data *victim, int type)
{
    byte percent;
    int dam;

    if ( ch == NULL ) {
        log("shoot: null player's shoot") ;
        return ;
    }
    if ( victim == NULL ) {
        log("shoot: no victim") ;
        return ;
    }
    if(victim == ch){
        if(!IS_NPC(ch))
            send_to_char("Shoot yourself? Nah...\n\r", ch);
        return;
    }

	if (ch->equipment[HOLD] == NULL) {
		send_to_char("Shoot with empty hand ?!?\n\r", ch) ;
		return ;
    }

    if(GET_ITEM_TYPE(ch->equipment[HOLD])!=ITEM_FIREWEAPON) {
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
    if(!IS_NPC(ch) || (IS_NPC(ch)&&IS_AFFECTED(ch,AFF_CHARM)))
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
        damage(ch, victim, dam, TYPE_SHOOT);
        if(GET_LEVEL(ch) < (IMO+3))
            WAIT_STATE(ch, PULSE_VIOLENCE*2);
        WAIT_STATE(victim, PULSE_VIOLENCE*2);
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
    char name[256], buf[256];

    one_argument(argument, name);
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

