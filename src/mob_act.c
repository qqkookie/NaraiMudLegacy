/*
 * ************************************************************************
 * file: mob_act.c , Mobile action module.                 Part of DIKUMUD *
 * Usage: Procedures generating 'intelligent' behavior in the mobiles.    *
 * Copyright (C) 1990, 1991 - see 'license.doc' for complete information. *
 ************************************************************************ */

#include <stdio.h>
#include <string.h>

#include "char.h"
#include "object.h" 
#include "global.h"
#include "comm.h"
#include "play.h"
#include "spells.h"
#include "actions.h" 
#include "etc.h" 

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

int scarvenger(struct char_data *ch, int cmd, char *arg);
int sentinel(struct char_data *ch, int cmd, char *arg);
int aggressive(struct char_data *ch, int cmd, char *arg);

void regened_mobile(struct char_data *ch );

#define MAGIC_CLASS(ch) \
	(GET_CLASS(ch) == CLASS_MAGIC_USER || GET_CLASS(ch) == CLASS_CLERIC )

int check_stat(struct char_data *ch)
{
    char buf[80];

    /* NOTE: Return zero at any failure to heal */
    if (GET_LEVEL(ch) < 20 
	|| ((10 * GET_HIT(ch)) > (5 * hit_limit(ch)))  /* check hit < 50% */
	|| ( number(1, 100) > (GET_WIS(ch)*2 + GET_INT(ch) + GET_LEVEL(ch))))
/* OLD: || ( number(1, 100) > (GET_WIS(ch) + GET_INT(ch) + GET_LEVEL(ch)))) */
	return(0) ;

    switch (GET_CLASS(ch)) {
    case CLASS_CLERIC:
	sprintf(buf, " 'full heal' %s", GET_NAME(ch));
	break;
    case CLASS_MAGIC_USER:
	sprintf(buf, " 'heal' %s", GET_NAME(ch));
	break;
    case CLASS_WARRIOR:
    case CLASS_THIEF:
	sprintf(buf, " 'self heal' %s", GET_NAME(ch));
	break;
    } 
    do_cast(ch, buf, 0);
    return 1;
}

void mobile_activity2(void)
{
    struct char_data *ch;

    for (ch = character_list; ch ; ch = ch->next) {
	if (IS_NPC(ch) && GET_CLASS(ch) > 30) {
	    if (check_stat(ch))
		continue; 
	    if (GET_POS(ch) != POS_FIGHTING )
		continue;
	    /* fighting */
	    switch (GET_CLASS(ch)) {
	    case CLASS_MAGIC_USER:
		magic_user(ch, 0, ""); break;
	    case CLASS_CLERIC:
		cleric(ch, 0, ""); break;
	    case CLASS_THIEF:
		thief(ch, 0, ""); break;
	    case CLASS_WARRIOR:
		warrior(ch, 0, ""); break;
	    }
	}
    }
}

void mobile_activity(void)
{
    register struct char_data *ch;
    char buf[100]; 
    extern int no_specials; 

    for (ch = character_list; ch; ch = ch->next) {
	if (!IS_MOB(ch) || ch->nr < 0 ) 	/* NOTE: not !IS_NPC(ch) */
	    continue;

	/* NOTE: make mobile more combat-ready. */
	if (ch->regened == 1 )
	    regened_mobile(ch);
	/* Examine call for special procedure */
	if (IS_SET(ch->specials.act, ACT_SPEC) && !no_specials) {
	    if (!mob_index[ch->nr].func) {
		sprintf(buf, "Attempting to call a non-existing MOB func.\n (mobact.c) %s",
			ch->player.short_descr);
		log(buf);
		REMOVE_BIT(ch->specials.act, ACT_SPEC);
	    }
	    else {
		// if ((*mob_index[ch->nr].func) (ch, 0, ""))
		//    /* continue; */ ;
		(*mob_index[ch->nr].func) (ch, 0, "");
	    }
	}
	if (AWAKE(ch) && !(ch->specials.fighting)) { 
	    /* NOTE: NEW! Separate scarvenger(), sentinel(), aggressive() */
	    if (IS_SET(ch->specials.act, ACT_SCAVENGER)) {
		scarvenger(ch, 0, "");
	    }
	    if (!IS_SET(ch->specials.act, ACT_SENTINEL)) {
		sentinel(ch, 0, "");
	    }	
	    if (IS_SET(ch->specials.act, ACT_AGGRESSIVE)) {
		aggressive(ch, 0, "");
	    }
	}		/* If AWAKE(ch)   */
	if (check_stat(ch))
	    continue;

	switch (GET_CLASS(ch)) {
	case CLASS_MAGIC_USER:
	    magic_user(ch, 0, ""); break;
	case CLASS_CLERIC:
	    cleric(ch, 0, ""); break;
	case CLASS_THIEF:
	    thief(ch, 0, ""); break;
	case CLASS_WARRIOR:
	    warrior(ch, 0, ""); break;
	}
	/* 
	if(IS_SET(ch->specials.act, ACT_THIEF))
	    thief(ch,0,""); 
	if(IS_SET(ch->specials.act, ACT_MAGE)) 
	    magic_user(ch,0,"");
	if(IS_SET(ch->specials.act, ACT_CLERIC))
	cleric(ch,0,"");
	 */
	if (IS_SET(ch->specials.act, ACT_DRAGON))
	    dragon(ch, 0, "");
	if (IS_SET(ch->specials.act, ACT_SHOOTER))
	    shooter(ch, 0, "");
	if (IS_SET(ch->specials.act, ACT_FIGHTER))
	    kickbasher(ch, 0, "");
	if (IS_SET(ch->specials.act, ACT_SPITTER))
	    spitter(ch, 0, "");
	if (IS_SET(ch->specials.act, ACT_PALADIN))
	    paladin(ch, 0, "");
	if (IS_SET(ch->specials.act, ACT_GUARD))
	    cityguard(ch, 0, "");
	if (IS_SET(ch->specials.act, ACT_SUPERGUARD))
	    superguard(ch, 0, "");
	if (IS_SET(ch->specials.act, ACT_RESCUER))
	    rescuer(ch, 0, "");
	if (IS_SET(ch->specials.act, ACT_HELPER))
	    helper(ch, 0, "");
	if (IS_SET(ch->specials.act, ACT_FINISH_BLOW))
	    finisher(ch, 0, "");
    }			/* for.. */
} 

/* NOTE: NEW! scarvenger() code separated from mobile_activity() */
int scarvenger(struct char_data *ch, int cmd, char *arg)
{
    struct obj_data *obj, *best_obj;
    int max;
    extern int CAN_GET_OBJ(struct char_data *ch, struct obj_data *obj);

    if (world[ch->in_room].contents && !number(0, 10)) {
	for (max = 1, best_obj = 0, obj = world[ch->in_room].contents;
	     obj; obj = obj->next_content) {
	    if (CAN_GET_OBJ(ch, obj)) {
		if (obj->obj_flags.cost > max) {
		    best_obj = obj;
		    max = obj->obj_flags.cost;
		}
	    }
	}	/* for */

	if (best_obj) {
	    obj_from_room(best_obj);
	    obj_to_char(best_obj, ch);
	    act("$n gets $p.", FALSE, ch, best_obj, 0, TO_ROOM);
	}
    }
    return TRUE;
}

/* NOTE: NEW! sentinel() code separated from mobile_activity() */
int sentinel(struct char_data *ch, int cmd, char *arg)
{
    int door;

    if ( (GET_POS(ch) >= POS_STANDING ) 
	    && ((door = number(0, 45)) <= 5) && CAN_GO(ch, door) &&
	    !IS_SET(world[EXIT(ch, door)->to_room].room_flags, NO_MOB)) {
	if (ch->specials.last_direction == door)
	    ch->specials.last_direction = -1;
	else {
	    if (!IS_SET(ch->specials.act, ACT_STAY_ZONE)) {
		ch->specials.last_direction = door;
		do_move(ch, "", ++door);
	    }
	    else if (world[EXIT(ch, door)->to_room].zone 
		    == world[ch->in_room].zone) {
		ch->specials.last_direction = door;
		do_move(ch, "", ++door); 
	    }
	}
    }
    return TRUE;
}

/* NOTE: NEW! aggressive() code separated from mobile_activity() */
int aggressive(struct char_data *ch, int cmd, char *arg)
{
    struct char_data *tmp_ch;
    int count = 0;

    for (tmp_ch = world[ch->in_room].people; tmp_ch;
	 tmp_ch = tmp_ch->next_in_room) {
	if (IS_NPC(tmp_ch) || !CAN_SEE(ch, tmp_ch) || IS_WIZARD(tmp_ch))
	    continue;
	if (IS_SET(ch->specials.act, ACT_WIMPY) && AWAKE(tmp_ch)) 
	    continue; 
	if (IS_AFFECTED(tmp_ch, AFF_HOLY_SHIELD))
	    continue; 
	/* NOTE: Spread probablilty to be a victim among all PC's in the room */
	if (number(1, 6) < ( 2 + (++count) ))
	    continue;
	/* NOTE: SPELL_PROTECT_EVIL will work for evil player against 
	    good mobile, as well as good player against evil mobile. */
	if ( IS_AFFECTED(tmp_ch, AFF_PROTECT_EVIL)
		&& GET_LEVEL(ch) < GET_LEVEL(tmp_ch)
		&& ((IS_EVIL(ch) && IS_GOOD(tmp_ch)) 
		    || (IS_GOOD(ch) && IS_EVIL(tmp_ch)))
		&& !saves_spell(ch, SAVING_PARA)) {
	    act("$n tries to attack, but failed miserably.",
		TRUE, ch, 0, 0, TO_ROOM);
	    continue;
	}
	first_attack(ch, tmp_ch);
	break; 
    }	/* for */
    return TRUE;
}

/* ************************************************************************
   *  file: mob_magic.c , Implementation of spells.          Part of DIKUMUD *
   *  Usage : The actual effect of magic.                                    *
   *  Copyright (C) 1990, 1991 - see 'license.doc' for complete information. *
   ************************************************************************* */

#ifdef UNUSED_CODE
/* NOTE: If you want it, use spell_fire_storm(), instead. */
void mob_spell_fire_storm(byte level, struct char_data *ch,
			  struct char_data *victim, struct obj_data *obj)
{
    int dam;
    struct char_data *tmp_victim, *temp;

    assert(ch);
    assert((level >= 15) && (level <= 55));

    dam = dice(level, 4);
    send_to_char("The fire storm is flowing in the air!\n\r", ch);
    act("$n makes the fire storm flowing in the air.\n\rYou can't see anything!\n\r"
	,FALSE, ch, 0, 0, TO_ROOM);
    for (tmp_victim = character_list; tmp_victim; tmp_victim = temp) {
	temp = tmp_victim->next;
	if ((ch->in_room == tmp_victim->in_room) && (ch != tmp_victim)
	    && !IS_NPC(tmp_victim)) {
	    damage(ch, tmp_victim, dam, SPELL_FIRE_STORM);
	}
	else if (world[ch->in_room].zone == world[tmp_victim->in_room].zone)
	    send_to_char("The fire storm is flowing in the atmosphere.\n\r",
			 tmp_victim);
    }
}

/* NOTE: NEW! But No need. Not used */
void mob_bash(struct char_data *ch, char *argument, int cmd)
{
    struct char_data *victim;
    if (get_obj_in_list_vis(ch, "sword", ch->carrying))
	do_wield(ch, "sword", 0);
    do_bash(ch, argument, cmd);
    do_wield(ch, "", 0);	/* NOTE: Unwield */
}
#endif		/* UNUSED_CODE */


void mob_light_move(struct char_data *ch, char *argument, int cmd)
{
    struct char_data *victim;

    if ((victim = ch->specials.fighting)) {
	/* NOTE: Add lightning move skill message */
	act("$n quickly steps back and avoids $N's hit.",
	    TRUE, ch, 0, victim, TO_NOTVICT );
	act("$n quickly steps back and avoids your hit.",
	    TRUE, ch, 0, victim, TO_VICT );

	hit(ch, victim, TYPE_UNDEFINED);
	if (ch->specials.fighting)
	    stop_fighting(ch);
	if (victim->specials.fighting)
	    stop_fighting(victim);
	/* NOTE: WAIT_STATE() is not applied to NPC */
	/* if (!IS_NPC(ch))
	    WAIT_STATE(ch, PULSE_VIOLENCE * 2 / 3); */
    }
}

void mob_punch_drop(struct char_data *ch, struct char_data *victim)
{
    int i;
    char buffer[MAX_STRING_LENGTH];
    struct obj_data *tmp_object;
    struct obj_data *next_obj;
    // bool test = FALSE;

    /* punch out equipments  */
    for (i = 0; i < MAX_WEAR; i++) {
	if (((victim->equipment)[i]) && (number(10, 100) < GET_LEVEL(ch))) {
	    /* obj is removed form player */
	    tmp_object = unequip_char(victim, i);
	    if (CAN_SEE_OBJ(victim, tmp_object)) {
		sprintf(buffer, "%s is punched out.\n\r", fname(tmp_object->name));
		send_to_char(buffer, victim);
	    }
	    else {
		send_to_char("Something is punched out.\n\r", victim);
	    }
	    act("$n's $p flies in the sky for a while and falls.", 1, victim,
		tmp_object, 0, TO_ROOM);
	    obj_to_room(tmp_object, ch->in_room);
	}
    }

    /* punch out carrying items..  */
    for (tmp_object = victim->carrying; tmp_object; tmp_object = next_obj) {
	next_obj = tmp_object->next_content;
	if (GET_LEVEL(ch) > number(10, 160)) {
	    /* item nodrop.. but.. can be punched also.. */
	    if (CAN_SEE_OBJ(victim, tmp_object)) {
		sprintf(buffer, "%s is punched out.\n\r", fname(tmp_object->name));
		send_to_char(buffer, victim);
	    }
	    else {
		send_to_char("Something is punched out.\n\r", victim);
	    }
	    act("$n's $p flies in the sky for a while and falls.", 1, victim,
		tmp_object, 0, TO_ROOM);
	    obj_from_char(tmp_object);
	    obj_to_room(tmp_object, ch->in_room);
	    // test = TRUE;
	}
    }
}

/* mode and fightmode is defined in mob_magic.h  */
#define NUMBER_OF_MAX_VICTIM	8
#define VICT_IS_SAME_ROOM(mob)	(mob->in_room==mob->specials.fighting->in_room)
struct char_data *choose_victim(struct char_data *mob, int fightmode, int mode)
{
    int i, min, max, tmp, count, lev;
    struct char_data *vict, *next_vict;
    struct char_data *victims[NUMBER_OF_MAX_VICTIM];

    /* find the partner */
    count = 0;
    victims[0] = NULL;
    for (vict = world[mob->in_room].people; vict &&
	 count < NUMBER_OF_MAX_VICTIM; vict = next_vict) {
	next_vict = vict->next_in_room;
	if (!IS_NPC(vict)) {	/* is player */
	    if (vict->specials.fighting) {
		if (fightmode == VIC_FIGHTING || fightmode == VIC_ALL) {
		    victims[count] = vict;
		    count++;
		}
	    }
	    else {	/* not fighting */
		if (fightmode == VIC_WATCHING || fightmode == VIC_ALL) {
		    victims[count] = vict;
		    count++;
		}
	    }
	}
    }

    if (count == 0) 
	return NULL;

    /* now choose min or max.. */

    if (mode == MODE_RANDOM) {
	tmp = number(0, count - 1);
	assert(tmp >= 0 && tmp < count);
	/* NOTE: Impossible!! incorrect and of no need.
	if (tmp < 0 || tmp >= count) {
	    char buf[BUFSIZ];
	    log("number error in choose victim");
	    sprintf(buf, "choose mob: count = %d, rnd # %d, (%s)", count, tmp,
		    victims[tmp]->player.name);
	    log(buf);
	}
	*/
	return (victims[tmp]);
    }

    if (mode == MODE_HIT_MIN || mode == MODE_MANA_MIN || mode == MODE_MOVE_MIN
     || mode == MODE_AC_MIN || mode == MODE_HR_MIN || mode == MODE_DR_MIN) {
	/* find minimum */
	min = 32000;
	vict = victims[0];
	for (i = 1; i < count; i++) {
	    switch (mode) {
	    case MODE_HIT_MIN: tmp = GET_HIT(victims[i]); break;
	    case MODE_MANA_MIN: tmp = GET_MANA(victims[i]); break;
	    case MODE_MOVE_MIN: tmp = GET_MOVE(victims[i]); break;
	    case MODE_AC_MIN: tmp = GET_AC(victims[i]); break;
	    case MODE_HR_MIN: tmp = GET_DAMROLL(victims[i]); break;
	    case MODE_DR_MIN: tmp = GET_HITROLL(victims[i]); break;
	    case MODE_LEVEL_MIN: tmp = GET_LEVEL(victims[i]); break;
	    default: tmp = GET_HIT(victims[i]);
	    }
	    if (min > tmp) {
		min = tmp;
		vict = victims[i];
	    }
	}
    }
    else if (mode == MODE_HIT_MAX || mode == MODE_MANA_MAX
	     || mode == MODE_MOVE_MAX || mode == MODE_AC_MAX
	     || mode == MODE_HR_MAX || mode == MODE_DR_MAX) {
	/* find minimum */
	max = -3000;
	vict = victims[0];
	for (i = 1; i < count; i++) {
	    switch (mode) {
	    case MODE_HIT_MAX: tmp = GET_HIT(victims[i]); break;
	    case MODE_MANA_MAX: tmp = GET_MANA(victims[i]); break;
	    case MODE_MOVE_MAX: tmp = GET_MOVE(victims[i]); break;
	    case MODE_AC_MAX: tmp = GET_AC(victims[i]); break;
	    case MODE_HR_MAX: tmp = GET_DAMROLL(victims[i]); break;
	    case MODE_DR_MAX: tmp = GET_HITROLL(victims[i]); break;
	    case MODE_LEVEL_MAX: tmp = GET_LEVEL(victims[i]); break;
	    default: tmp = GET_HIT(victims[i]);
	    }
	    if (max < tmp) {
		max = tmp;
		vict = victims[i];
	    }
	}
    }
    else if (mode == MODE_HIGH_LEVEL || mode == MODE_MID_LEVEL
	     || mode == MODE_LOW_LEVEL) {
	for (i = 0; i < count; i++) {
	    lev = GET_LEVEL(victims[i]);
	    if (mode == MODE_HIGH_LEVEL && lev >= 31 && lev <= LEVEL_LIMIT)
		return (victims[i]);
	    else if (mode == MODE_MID_LEVEL && lev >= 13 && lev <= 30)
		return (victims[i]);
	    else if (mode == MODE_LOW_LEVEL && lev >= 1 && lev <= 12)
		return (victims[i]);
	}
	/* return NULL because thereis no one such that */
	return NULL;
    }

    return (vict);
}
#undef NUMBER_OF_MAX_VICTIM

/*  choose mob routine for rescuer  */
#define NUMBER_OF_MAX_MOB	8
struct char_data *choose_rescue_mob(struct char_data *mob)
{
    int tmp, count;
    struct char_data *vict, *next_vict;
    struct char_data *victims[NUMBER_OF_MAX_MOB];

    /* find the partner */
    count = 0;
    victims[count] = NULL;
    for (vict = world[mob->in_room].people; vict &&
	 count < NUMBER_OF_MAX_MOB; vict = next_vict) {
	next_vict = vict->next_in_room;
	if (IS_NPC(vict)) {	/* is mob */
	    if (vict->specials.fighting && (mob != vict)) {
		victims[count] = vict;
		count++;
	    }
	}
    }

    if (count == 0) {	/* there is no mob fighting except me.. */
	if (mob->specials.fighting)
	    return mob;
	else
	    return NULL;
    }

    /* random select  */
    tmp = number(0, count - 1);
    if (tmp >= 0 && tmp < count)
	return (victims[tmp]);
    else
	return NULL;
}
#undef NUMBER_OF_MAX_MOB 

void npc_tornado(struct char_data *ch)
{
    struct char_data *tch;

    acthan("$n says 'Ya haa haa hap'.", "$n님이 '야하햐합' 하고 말합니다",
	   1, ch, 0, 0, TO_ROOM);

    for (tch = world[ch->in_room].people; tch; tch = tch->next_in_room) {
	if (!IS_NPC(tch) && ch->points.move > 0) {
	    if (ch->skills[SKILL_TORNADO].learned > number(1, 99)) {
		hit(ch, tch, TYPE_UNDEFINED);
	    }
	}
    }

    ch->points.move -= (LEVEL_LIMIT - GET_LEVEL(ch) + 3);
}

void npc_steal(struct char_data *ch, struct char_data *victim)
{
    int dir, gold;

    if (IS_NPC(victim) || IS_WIZARD(victim))
	return;

/* NOTE: - Suggestions by Tido.  */
/*    Sleeping or dying mob or mob who can't see victim (blind, invisible,
   dark room and no light... ) should not steal victim's gold.  
   Charmed mob should not steal its master's gold.                      */

    /* NOTE: Wrong! assert(!IS_AFFECTED(ch, AFF_CHARM )||(ch->master)); */
    /* NOTE: Check ch->master before checking victim  - by jhpark */
    if (!AWAKE(ch) || !CAN_SEE(ch, victim) ||
     (IS_AFFECTED(ch, AFF_CHARM) && (ch->master) && (victim == ch->master)))
	return;

    /* NOTE: Enhence odd of discover pickpocketing. */
    if (AWAKE(victim) && 
    	(number(0, GET_LEVEL(ch)+8) <= (GET_LEVEL(victim)/5+1))) {
	acthan("You discover that $n has $s hands in your wallet.",
	       "앗! $n님이 당신의 지갑에 손을 넣습니다.",
	       FALSE, ch, 0, victim, TO_VICT);
	acthan("$n tries to steal gold from $N.",
	       "$n님이 $N님으로부터 돈을 훔치려고 합니다.", TRUE, ch, 0, victim, TO_NOTVICT);
    }
    else {
	/* Steal some gold coins */
	acthan("$n suppresses a laugh.", "$n님이 키득키득 웃습니다.(왜그렇까?)",
	       TRUE, ch, 0, 0, TO_NOTVICT);
	gold = (int) ((GET_GOLD(victim) * number(1, 10)) / 25);
	if (gold > 0) {
	    GET_GOLD(ch) += gold / 2;
	    GET_GOLD(victim) -= gold;
	    dir = number(0, 5);

	    /* NOTE: Charmed mob can't leave beside its master        */
	    /* NOTE: Check ch->master before checking victim  - by jhpark */
	    if (CAN_GO(ch, dir)
		&& (!IS_AFFECTED(ch, AFF_CHARM) || !(ch->master)
		    || (ch->in_room != ch->master->in_room)))
		do_simple_move(ch, dir, FALSE);
	}
    }
}

void first_attack(struct char_data *ch, struct char_data *victim)
{
    char buf[MAX_NAME_LEN], sbuf[MAX_LINE_LEN];
    int num;

    assert(GET_NAME(victim));
    strcpy(buf, GET_NAME(victim));

    if (GET_LEVEL(ch) < 10) {
	hit(ch, victim, TYPE_UNDEFINED);
	return;
    }
    /* NOTE: If char has Solar flare, quaff it before attack */
    if (get_obj_in_list_vis(ch, "solar flare", ch->carrying))
	do_quaff(ch, "solar flare", 0 ); 
	
    if ( number(0, 99) < 33 ) {
	hit(ch, victim, TYPE_UNDEFINED);
	return;
    }
    else if ( MAGIC_CLASS(ch)) {
	/* NOTE: Prefered spell for first attack is
		'mana boost', 'chill touch', 'energy drain'.   */ 

        /* NOTE: Min. level to use 'mana boost' : 15 -> 20. */  
	if ( GET_LEVEL(ch) > 20 && GET_MOVE(victim) > 100 && 
	    number(0,99) < (MAGIC_CLASS(victim) ? 15 : 30 ))
	    sprintf(sbuf, " 'mana boost' %s", buf);
	else if ( number(0,99) < (MAGIC_CLASS(victim) ? 15 : 20 ))
	    sprintf(sbuf, " 'chill touch' %s", buf);
	/* NOTE: energy drain is perfered for mage char, magic class victim */
	else if ( GET_LEVEL(ch) > 15 && 
	    (((GET_CLASS(ch) == CLASS_MAGIC_USER) ?
		(MAGIC_CLASS(victim) ? 40 : 20 ) : 10 ) > number(0, 99))) 
	    sprintf(sbuf, " 'energy drain' %s", buf);
	else
	    goto ok;
	do_cast(ch, sbuf, 0);
	return;
    }
ok:
    num = number(GET_LEVEL(ch) - 15 , GET_LEVEL(ch));
    switch (GET_CLASS(ch)) {
    case CLASS_MAGIC_USER:
	/* NOTE: 'cone of ice' spell is better than 'disintegrate' spell */
	/* if (num > 33)
	    sprintf(sbuf, " 'disintegrate' %s", buf); */
	if (num > 30)
	    sprintf(sbuf, " 'cone of ice' %s", buf);
	else if (num > 26)
	    sprintf(sbuf, " 'full fire' %s", buf);
	else if (num > 20)
	    sprintf(sbuf, " 'throw' %s", buf);
	else if (num > 15)
	    sprintf(sbuf, " 'energy drain' %s", buf);
	else 
	    sprintf(sbuf, " 'lightning bolt' %s", buf);
	do_cast(ch, sbuf, 0);
	break;

    case CLASS_CLERIC:
	if (num > 35) 
	    sprintf(sbuf, " 'throw' %s", buf);
	else if (num > 25)
	    sprintf(sbuf, " 'sunburst' %s", buf);
	else if (num > 15)
	    sprintf(sbuf, " 'harm' %s", buf);
	else 
	    sprintf(sbuf, " 'call lightning' %s", buf);
	do_cast(ch, sbuf, 0);
	break;

    case CLASS_THIEF: 
	/* NOTE: Chill touch will reduces victim's hit chance */
	if ( num %5 == 0  && num > 15 ) {
	    sprintf(sbuf, " 'chill touch' %s", buf);
	    do_cast(ch, sbuf, 0);
	} 
	else if ( num %2 == 0  && num > 15 )
	    do_backstab(ch, buf, 0);
	else if (num > 30)
	    do_punch(ch, buf, 0);
	else 
	    do_flash(ch, buf, 0);
	break;

    case CLASS_WARRIOR: 
	if (num > 30) {
	    if (GET_SEX(ch) == SEX_MALE)    /* male */
		do_shouryuken(ch, buf, 0);
	    else 			    /* female */
		do_spin_bird_kick(ch, buf, 0);
	}
	else if ( num  > 25 )
	    do_punch(ch, buf, 0);
	else if ( ch->equipment[WIELD] )
	    do_bash(ch, buf, 0);
	else 
	    hit(ch, victim, TYPE_UNDEFINED);
    }
}

struct char_data *select_victim(struct char_data *ch)
{
    return ch->specials.fighting;
}

/* new mob act by atre */
/*
   warrior
 */
int warrior(struct char_data *ch, int cmd, char *arg)
{
    struct char_data *victim;
    char *vicname;
    int do_what;

    /* NOTE: Symbolic skill value is minimum level of that command. */
#define KICK		1
#define BASH		2
#define DISARM		10
#define MULTI		17	/* From level 17 */
#define PUNCH		25	/* From level 25 */
#define SHOUR		30 	/* From level 30 */

    /* NOTE: Use 'disarm', too */ 
    static u_char do_what_cmd[] = {
	BASH, KICK, BASH, KICK, KICK, BASH, KICK, BASH, KICK, BASH,
	DISARM, KICK, BASH, KICK, BASH, BASH, KICK, MULTI, MULTI, MULTI,
	BASH, MULTI, DISARM, MULTI, PUNCH, BASH, MULTI, PUNCH, PUNCH, DISARM,
	SHOUR, MULTI, PUNCH, SHOUR, PUNCH, MULTI, PUNCH, DISARM, SHOUR, SHOUR,
	SHOUR, SHOUR, SHOUR, SHOUR, SHOUR, };

    if (cmd)
	return FALSE; 
    if (GET_POS(ch) != POS_FIGHTING)
	return FALSE;

    /* select victim */
    if(!(victim = select_victim(ch)))
	return FALSE; 
    vicname = GET_NAME(victim);

    do {
	do_what = do_what_cmd[number(GET_LEVEL(ch) / 2, GET_LEVEL(ch))]; 

    /* NOTE: Retry until mobile can perform that command. */ 
    } while ( (do_what > GET_LEVEL(ch))
		/* NOTE: || ( do_what == BASH && !ch->equipment[WIELD]) */
		|| ( do_what == DISARM && !victim->equipment[WIELD]));

    switch(do_what) {
    case BASH :  	do_bash( ch, vicname, 0 ); break;
    case KICK :		do_kick( ch, vicname, 0 ); break;
    case MULTI : 	do_multi_kick( ch, vicname, 0 ); break;
    case PUNCH :	do_punch( ch, vicname, 0 ); break;
    case DISARM :	do_disarm( ch, vicname, 0 ); break;
    case SHOUR :	
	/* NOTE: Select skill according to sexuality */
	if (GET_SEX(ch) == SEX_MALE )
	    do_shouryuken( ch, vicname, 0 );
	else if (GET_SEX(ch) == SEX_FEMALE )
	    do_spin_bird_kick( ch, vicname, 0 );
	break;
	/* ((*cmd_info[do_what_cmd[do_what]].command_pointer) 
		(ch, GET_NAME(victim), 0)); */

    default: return FALSE; break;
    }
    return TRUE;
}

/* backstab lig flash steal tornado */
int thief(struct char_data *ch, int cmd, char *arg)
{
    struct char_data *victim, *cons;
    struct obj_data *steal;
    int do_what;
    char buf[MAX_INPUT_LENGTH];

    if (cmd)
	return FALSE;
    if (GET_POS(ch) == POS_FIGHTING) {
	victim = select_victim(ch);
	if (!victim)
	    return FALSE;

	/* NOTE: More intelligent behavior */
	do_what = number(1, GET_LEVEL(ch)/2);
	switch (do_what) { 
	case 17: case 19: case 20:
	    do_punch(ch, GET_NAME(victim), 0);	/* 30 */
	    break;

	case 13: case 16:
	    sprintf( buf, "'energy drain' %s",  GET_NAME(victim)); /* 25 */
	    do_cast( ch, buf, 0);
	    break; 

	case 10: case 14:
	    npc_tornado(ch);			/* 20 */
	    break;

	case 7: case 12: 
	    sprintf( buf, "'chill touch' %s",  GET_NAME(victim)); /* 13 */
	    do_cast( ch, buf, 0);
	    break;

	case 1: case 2: case 6: case 9: case 18: case 21: case 22: 
	    if ( ch->equipment[WIELD]) {
		do_backstab(ch, GET_NAME(victim), 0); /* 1 */
		break;
	    }
	    /* FALL THRU */ 
	case 3: case 8:
	    if ( GET_MOVE(ch) * 3 > GET_MAX_MOVE(ch)) {
		do_flash(ch, GET_NAME(victim), 0);	/* 5 */
		break;
	    } 
	    /* FALL THRU */ 
	case 4: case 15:  
	    if ( victim->equipment[WIELD] ) {
		do_disarm(ch, GET_NAME(victim), 0); /* 1 */
		break;
	    }
	    /* FALL THRU */ 
	case 5: case 11: 
	    do_light_move(ch, "", 0);
	    break;
	default:
	    hit(ch,victim, TYPE_UNDEFINED);

	}
	return TRUE;
    }
    else {
	if (GET_LEVEL(ch) < 5)
	    return FALSE;

	for (cons = world[ch->in_room].people; cons; cons = cons->next_in_room) {
	    if (PC_MORTAL(cons) &&
		!HIGHER_LEV(ch, cons)) {
		do_what = number(1, 6);
		if ( do_what <= 2 ) { 
		    npc_steal(ch, cons);
		    return TRUE;
		}
		/* NOTE: steal PC's weapon (hopefully just disarmed) */
		else if( do_what >= 5 && IS_ACTPLR(ch, ACT_AGGRESSIVE)
		    /* NOTE Check cons carry anything */
			&& cons->carrying
			&& GET_ITEM_TYPE(cons->carrying) == ITEM_WEAPON ) {

		    steal = cons->carrying; 
		    sprintf(buf, " %s %s", steal->name, GET_NAME(cons)); 
		    do_steal(cons, buf, 0 ); 

		    if ( steal != ch->carrying ) {
			if( do_what == 5 || ch->equipment[WIELD] )
			    do_junk(ch, steal->name, 0); 
			else
			    wear(ch, steal, 12 );  /* wield it */
		    }
		    return TRUE;
		}
	    }
	}
    }
    return FALSE;
}

int cleric(struct char_data *ch, int cmd, char *arg)
{
    struct char_data *victim;
    char buf[80];

    if (cmd)
	return FALSE;
    if (GET_POS(ch) != POS_FIGHTING) {
	return FALSE;
    }

    victim = select_victim(ch);
    if (!victim)
	return FALSE;

    switch (number(GET_LEVEL(ch)/2, GET_LEVEL(ch))) {
    case 0: case 1: case 2: case 3: case 4: case 5:
	sprintf(buf, " 'magic missile' %s", GET_NAME(victim));
	break;
    case 6: case 7: case 8:
	sprintf(buf, " 'chill touch' %s", GET_NAME(victim));
	break;
    case 9: case 10:
	sprintf(buf, " 'burning hands' %s", GET_NAME(victim));
	break;
    case 11: case 12:
	sprintf(buf, " 'earthquake' %s", GET_NAME(victim));
	break;
    case 13: case 14:
	sprintf(buf, " 'shocking grasp' %s", GET_NAME(victim));
	break;
    case 15: case 16: case 17:
	sprintf(buf, " 'harm' %s", GET_NAME(victim));
	break;
    case 18: case 19:
	sprintf(buf, " 'color spray' %s", GET_NAME(victim));
	break;
    case 20:
	sprintf(buf, " 'blindness' %s", GET_NAME(victim));
	break;
    case 21: case 22: 
	sprintf(buf, " 'call lightning' %s", GET_NAME(victim));
	break;
    case 23: case 24:
	sprintf(buf, " 'curse' %s", GET_NAME(victim));
	break;
    case 25: case 26: 
	sprintf(buf, " 'poison' %s", GET_NAME(victim));
	break;
    case 27: case 28: case 29: 
	sprintf(buf, " 'fireball' %s", GET_NAME(victim));
	break;
	sprintf( buf, "'energy drain' %s",  GET_NAME(victim));
	break;
    case 30: 
	sprintf(buf, " 'firestorm' %s", GET_NAME(victim));
	break;
    case 31: case 32:
	sprintf(buf, " 'dumb' %s", GET_NAME(victim));
	break;
    case 33: case 34: case 35: case 36:
	sprintf(buf, " 'harm' %s", GET_NAME(victim));
	break;
    case 37: 
	sprintf(buf, " 'mana boost' %s", GET_NAME(victim));
	break;
    case 38: case 39: 
	sprintf(buf, " 'sunburst' %s", GET_NAME(victim));
	break;
    case 40: case 41: case 42: case 43:
	sprintf(buf, " 'throw' %s", GET_NAME(victim));
	break;
    }
    do_cast(ch, buf, 84);

    return TRUE;
}

int magic_user(struct char_data *ch, int cmd, char *arg)
{
    struct char_data *victim;
    char buf[80];

    if (cmd)
	return FALSE;
    if (GET_POS(ch) != POS_FIGHTING) {
	return FALSE;
    }

    victim = select_victim(ch);
    if (!victim)
	return FALSE;

    switch (number(GET_LEVEL(ch)/2, GET_LEVEL(ch))) {
    case 1: case 2:
	sprintf(buf, " 'magic missile' %s", GET_NAME(victim));
	break;
    case 3: case 4:
	sprintf(buf, " 'chill touch' %s", GET_NAME(victim));
	break;
    case 5: case 6:
	sprintf(buf, " 'energy flow' %s", GET_NAME(victim));
	break;
    case 7: case 8:
	sprintf(buf, " 'burning hands' %s", GET_NAME(victim));
	break;
    case 9: case 10:
    case 11:
	sprintf(buf, " 'shocking grasp' %s", GET_NAME(victim));
	break;
    case 12: case 13:
	sprintf(buf, " 'lightning bolt' %s", GET_NAME(victim));
	break;
    case 14: case 15: case 16:
	sprintf(buf, " 'colour spray' %s", GET_NAME(victim));
	break;
    case 17: case 18: case 19: case 20:
	sprintf(buf, " 'fireball' %s", GET_NAME(victim));
	break;
    case 21: case 22: case 23:
	sprintf(buf, " 'energy drain' %s", GET_NAME(victim));
	break;
    case 24: case 25: case 26:
	sprintf(buf, " 'firestorm' %s", GET_NAME(victim));
	break;
    case 27: case 28: case 29: case 30:
	sprintf(buf, " 'cone of ice' %s", GET_NAME(victim));
	break;
    case 31: case 32: case 33: case 34:
	sprintf(buf, " 'throw' %s", GET_NAME(victim));
	break;
    case 35: case 36: 
	sprintf(buf, " 'cone of ice' %s", GET_NAME(victim));
	break;
    case 37: case 38:
	sprintf(buf, " 'disintegrate' %s", GET_NAME(victim));
	break;
    case 39: case 40: case 41: case 42: case 43:
	sprintf(buf, " 'full fire' %s", GET_NAME(victim));
	break;
    }
    do_cast(ch, buf, 84);

    return TRUE;
}

/* modified by atre */
#ifdef OldMobAct
int thief(struct char_data *ch, int cmd, char *arg)
{
    struct char_data *cons;

    if (cmd)
	return FALSE;
    if (GET_POS(ch) == POS_FIGHTING) {
	if (number(1, 4) > 1) {
	    acthan("$n says 'Ya haa haa hap'.", "$n님이 '야하햐합' 하고 말합니다",
		   1, ch, 0, 0, TO_ROOM);
	    for (cons = world[ch->in_room].people; cons; cons = cons->next_in_room) {
		if (number(10, IMO + 3) < GET_LEVEL(ch) && IS_MORTAL(cons))
		    if (!IS_NPC(cons) && ch != cons)
			hit(ch, cons, TYPE_UNDEFINED);	/* It is tornado */
	    }
	}
	else if (GET_LEVEL(ch) > 10 && (GET_HIT(ch) * 100 / GET_PLAYER_MAX_HIT(ch)) < 47) {
	    acthan("$n says 'Quu ha cha cha'.", "$n님이 '크하차차' 하고 외칩니다",
		   1, ch, 0, 0, TO_ROOM);
	    do_flash(ch, "", 0);
	}
    }
    else if (GET_POS(ch) != POS_STANDING)
	return FALSE;
    else {
	for (cons = world[ch->in_room].people; cons; cons = cons->next_in_room) {
	    if (PC_MORTAL(cons) &&
		!HIGHER_LEV(ch, cons) && (number(1, 3) == 1))
		npc_steal(ch, cons);
	}
    }
    return TRUE;
}

int cleric(struct char_data *ch, int cmd, char *arg)
{
    struct char_data *vict;
    char buf[80];

    if (cmd)
	return FALSE;
    if (GET_POS(ch) != POS_FIGHTING) {
	return FALSE;
    }
    if (!ch->specials.fighting)
	return FALSE;
    for (vict = world[ch->in_room].people; vict; vict = vict->next_in_room)
	if (vict->specials.fighting == ch)
	    break;
    if (!vict)
	return FALSE;
    switch (number(1, GET_LEVEL(ch))) {
    case 1:
	sprintf(buf, " 'armor' %s", GET_NAME(ch));
	break;
    case 2:
	sprintf(buf, " 'cure light' %s", GET_NAME(ch));
    case 3:
    case 4:
    case 5:
	sprintf(buf, " 'bless' %s", GET_NAME(ch));
	break;
    case 6:
	sprintf(buf, " 'blindness' %s", GET_NAME(vict));
	break;
    case 7:
    case 8:
	sprintf(buf, " 'poison' %s", GET_NAME(ch));
	break;
    case 9:
	sprintf(buf, " 'cure critic' %s", GET_NAME(ch));
	break;
    case 10:
	sprintf(buf, " 'dispel evil' %s", GET_NAME(ch));
	break;
    case 11:
	sprintf(buf, " 'remove poison' %s", GET_NAME(ch));
	break;
    case 12:
	sprintf(buf, " 'remove curse' %s", GET_NAME(ch));
	break;
    case 13:
	sprintf(buf, " 'sancuary' %s", GET_NAME(ch));
    case 14:
	sprintf(buf, " 'heal' %s", GET_NAME(ch));
	break;
    case 15:
	sprintf(buf, " 'call lightning' %s", GET_NAME(vict));
	break;
    case 16:
	sprintf(buf, " 'harm' %s", GET_NAME(vict));
	break;
    case 17:
    case 18:
	sprintf(buf, " 'love' %s", GET_NAME(ch));
	break;
    case 19:
	sprintf(buf, " 'love' %s", GET_NAME(vict));
	break;
    case 20:
	sprintf(buf, " 'curse' %s", GET_NAME(vict));
	break;
    case 21:
	sprintf(buf, " 'full heal' %s", GET_NAME(ch));
	break;
    case 22:
    case 23:
	sprintf(buf, " 'haste' %s", GET_NAME(ch));
	break;
    case 24:
    case 25:
	sprintf(buf, " 'sunburst' %s", GET_NAME(vict));
	break;
    case 26:
    case 27:
    case 28:
    case 29:
    case 30:
    case 31:
    case 32:
	sprintf(buf, " 'full heal' %s", GET_NAME(vict));
	break;
    case 33:
    case 34:
	sprintf(buf, " 'full heal' %s", GET_NAME(ch));
	break;
    case 35:
	sprintf(buf, " 'throw' %s", GET_NAME(vict));
	break;
    case 36:
	sprintf(buf, " 'full fire' %s", GET_NAME(vict));
	break;
    case 37:
    case 38:
    case 39:
    case 40:
    default:
	sprintf(buf, " 'improved haste'");
	do_cast(ch, buf, 84);
	sprintf(buf, " 'mirror'");
	break;
    }
    do_cast(ch, buf, 84);
    return TRUE;
}

int magic_user(struct char_data *ch, int cmd, char *arg)
{
    struct char_data *vict;
    char buf[80];

    if (cmd)
	return FALSE;
    if (GET_POS(ch) != POS_FIGHTING) {
	if (GET_LEVEL(ch) != 27)	/* Hypnos */
	    return (FALSE);
	for (vict = world[ch->in_room].people; vict; vict = vict->next_in_room)
	    if ((!IS_NPC(vict)) && (GET_LEVEL(vict) < IMO)) {
		act("$n utters the words 'Hypnos Thanatos'.", 1, ch, 0, 0, TO_ROOM);
		cast_sleep(GET_LEVEL(ch), ch, "", SPELL_TYPE_SPELL, vict, 0);
		return (TRUE);
	    }
	return FALSE;
    }
    if (!ch->specials.fighting)
	return FALSE;
    for (vict = world[ch->in_room].people; vict; vict = vict->next_in_room)
	if (vict->specials.fighting == ch)
	    break;
    if (!vict)
	return FALSE;
    if (!ch)
	return FALSE;
    switch (number(1, GET_LEVEL(ch))) {
    case 1:
	sprintf(buf, " 'magic missile' %s", GET_NAME(vict));
	break;
    case 2:
    case 3:
	sprintf(buf, " 'chill touch' %s", GET_NAME(vict));
	break;
    case 4:
	sprintf(buf, " 'invisibility' %s", GET_NAME(ch));
	do_cast(ch, buf, 84);
	sprintf(buf, " 'energy flow' %s", GET_NAME(vict));
	break;
    case 5:
	sprintf(buf, " 'armor' %s", GET_NAME(ch));
	do_cast(ch, buf, 84);
	sprintf(buf, " 'burning hands' %s", GET_NAME(vict));
	break;
    case 6:
    case 7:
	sprintf(buf, " 'shocking grasp' %s", GET_NAME(vict));
	break;
    case 8:
	sprintf(buf, " 'blindness' %s", GET_NAME(ch));
	break;
    case 9:
	sprintf(buf, " 'lightning bolt' %s", GET_NAME(vict));
	do_cast(ch, buf, 84);
	sprintf(buf, " 'damage up'");
	break;
    case 10:
	sprintf(buf, " 'self heal'");
	break;
    case 11:
	sprintf(buf, " 'colour spray' %s", GET_NAME(vict));
	break;
    case 12:
	sprintf(buf, " 'curse' %s", GET_NAME(vict));
	break;
    case 13:
	sprintf(buf, " 'energy drain' %s", GET_NAME(vict));
	break;
    case 14:
	sprintf(buf, " 'sleep' %s", GET_NAME(vict));
	break;
    case 15:
	sprintf(buf, " 'fireball' %s", GET_NAME(vict));
	break;
    case 16:
    case 17:
	sprintf(buf, " 'vitalize' %s", GET_NAME(ch));
	do_cast(ch, buf, 84);
	sprintf(buf, " 'throw' %s", GET_NAME(vict));
	break;
    case 18:
    case 19:
	sprintf(buf, " 'crush aromr' %s", GET_NAME(vict));
	break;
    case 20:
	sprintf(buf, " 'mana boost' %s", GET_NAME(vict));
	break;
    case 21:
    case 22:
	sprintf(buf, " 'haste' %s", GET_NAME(ch));
	break;
    case 23:
    case 24:
    case 25:
	sprintf(buf, " 'improved haste'");
	break;
    case 26:
	sprintf(buf, " 'full fire' %s", GET_NAME(vict));
	break;
    case 27:
    case 28:
    case 29:
    case 30:
	sprintf(buf, " 'cone of ice' %s", GET_NAME(vict));
	break;
    case 31:
	sprintf(buf, " 'vitalize' %s", GET_NAME(vict));
	break;
    case 32:
    case 33:
	sprintf(buf, " 'disintegrate' %s", GET_NAME(vict));
	break;
    case 34:
    case 35:
	sprintf(buf, " 'mirror image'");
	break;
    case 36:
    case 37:
    case 38:
    case 39:
    case 40:
	sprintf(buf, " 'full heal' %s", GET_NAME(ch));
	break;
    default:
	sprintf(buf, " 'disintegrate' %s", GET_NAME(vict));
	break;
    }
    do_cast(ch, buf, 84);
    return TRUE;
}
#endif				/* OldMobAct */

int paladin(struct char_data *ch, int cmd, char *arg)
{
    struct char_data *vict;

    if (cmd)
	return FALSE;
    if (GET_POS(ch) != POS_FIGHTING) {
	return FALSE;
    }
    if (!ch->specials.fighting)
	return FALSE;
    for (vict = world[ch->in_room].people; vict; vict = vict->next_in_room)
	if (vict->specials.fighting == ch)
	    break;
    if (!vict)
	return FALSE;
    switch (number(1, GET_LEVEL(ch))) {
    case 1:
    case 2:
    case 3:
	do_kick(ch, GET_NAME(vict), 0);
	break;
    case 4:
    case 5:
    case 6:
    case 7:
	do_bash(ch, GET_NAME(vict), 0);
	break;
    case 8:
    case 9:
    case 10:
    case 11:
    case 12:
    case 13:
	act("$n utters the words 'asdghjkasdgi'.", 1, ch, 0, 0, TO_ROOM);
	if (!IS_AFFECTED(ch, AFF_SPELL_BLOCK))
	    cast_cure_light(GET_LEVEL(ch), ch, "", SPELL_TYPE_SPELL, ch, 0);
	else
	    act("$n failed to cure light!!!", TRUE, ch, 0, 0, TO_ROOM);
	break;
    case 14:
    case 15:
    case 16:
	do_multi_kick(ch, GET_NAME(vict), 0);
	break;
    case 17:
    case 18:
    case 19:
    case 20:
	act("$n utters the words 'saghkasdlghui'.", 1, ch, 0, 0, TO_ROOM);
	if (!IS_AFFECTED(ch, AFF_SPELL_BLOCK))
	    cast_cure_critic(GET_LEVEL(ch), ch, "", SPELL_TYPE_SPELL, ch, 0);
	else
	    act("$n failed to cure critic!!!", TRUE, ch, 0, 0, TO_ROOM);
	break;
    case 21:
    case 22:
    case 23:
	do_light_move(ch, "", 0);
	/*FALLTHRU*/
    case 24:
    case 25:
	act("$n utters the words 'heal'.", 1, ch, 0, 0, TO_ROOM);
	if (!IS_AFFECTED(ch, AFF_SPELL_BLOCK))
	    cast_heal(GET_LEVEL(ch), ch, "", SPELL_TYPE_SPELL, ch, 0);
	else
	    act("$n failed to heal!!!", TRUE, ch, 0, 0, TO_ROOM);
	break;
    case 26:
    case 27:
	do_punch(ch, GET_NAME(vict), 0);
    case 28:
    case 29:
	break;
    case 30:
	do_flash(ch, GET_NAME(vict), 0);
	break;
    case 31:
    case 32:
	break;
    case 33:
    case 34:
    case 35:
	do_disarm(ch, GET_NAME(vict), 0);
	break;
    case 36:
    case 37:
	break;
    case 38:
	if (GET_SEX(ch) == SEX_MALE)
	    do_shouryuken(ch, GET_NAME(vict), 0);
	else
	    do_spin_bird_kick(ch, GET_NAME(vict), 0);
	break;
    case 39:
    case 40:
	act("$n utters the words 'sdagh'.", 1, ch, 0, 0, TO_ROOM);
	if (!IS_AFFECTED(ch, AFF_SPELL_BLOCK))
	    cast_full_heal(GET_LEVEL(ch), ch, "", SPELL_TYPE_SPELL, ch, 0);
	else
	    act("$n failed to full heal!!!", TRUE, ch, 0, 0, TO_ROOM);
	break;
    case 41:
    default:
	break;
    }
    return TRUE;
}

int dragon(struct char_data *ch, int cmd, char *arg)
{
    int mh;
    struct char_data *vict;
    void cast_fire_breath(byte level, struct char_data *ch, char *arg, int type,
			struct char_data *tar_ch, struct obj_data *tar_obj);
    void cast_frost_breath(byte level, struct char_data *ch, char *arg, int type,
			struct char_data *tar_ch, struct obj_data *tar_obj);
    void cast_gas_breath(byte level, struct char_data *ch, char *arg, int type,
			 struct char_data *tar_ch, struct obj_data *tar_obj);
    void cast_lightning_breath(byte level, struct char_data *ch, char *arg,
	      int type, struct char_data *tar_ch, struct obj_data *tar_obj);

    if (cmd)
	return FALSE;
    vict = ch->specials.fighting;
    if (!vict)
	return FALSE;
    mh = GET_HIT(vict);
    switch (number(1, 4)) {
    case 1:
	act("$n utters the words 'qassir plaffa'.", 1, ch, 0, 0, TO_ROOM);
	cast_fire_breath(GET_LEVEL(ch), ch, "", SPELL_TYPE_SPELL, vict, 0);
	if (mh < 400)
	    return TRUE;
	/*FALLTHRU*/
    case 2:
	act("$n utters the words 'qassir porolo'.", 1, ch, 0, 0, TO_ROOM);
	cast_gas_breath(GET_LEVEL(ch), ch, "", SPELL_TYPE_SPELL, vict, 0);
	if (mh < 400)
	    return TRUE;
	/*FALLTHRU*/
    case 3:
	act("$n utters the words 'qassir relata'.", 1, ch, 0, 0, TO_ROOM);
	cast_lightning_breath(GET_LEVEL(ch), ch, "", SPELL_TYPE_SPELL, vict, 0);
	if (mh < 400)
	    return TRUE;
	/*FALLTHRU*/
    case 4:
	act("$n utters the words 'qassir moolim'.", 1, ch, 0, 0, TO_ROOM);
	cast_frost_breath(GET_LEVEL(ch), ch, "", SPELL_TYPE_SPELL, vict, 0);
	return TRUE;
    }
    return TRUE;
}

#define RESCUER_VICTIM	5
int rescuer(struct char_data *ch, int cmd, char *arg)
{
    void mob_light_move(struct char_data *ch, char *argument, int cmd);
    struct char_data *tmp_ch, *vict;
    struct char_data *choose_rescue_mob(struct char_data *mob);
    struct char_data *choose_victim(struct char_data *mob, int fmode, int mode);

    if (cmd)
	return FALSE;

    if (ch->specials.fighting && (IS_NPC(ch->specials.fighting))) {
	act("$n screams 'PROTECT THE INNOCENT!!!'", FALSE, ch, 0, 0, TO_ROOM);
	mob_light_move(ch, "", 0);
	return TRUE;
    }

    /* find the partner */
    vict = choose_rescue_mob(ch);

    if (vict == ch) {	/* only I am fighting  */
	mob_light_move(ch, "", 0);
	switch (number(0, 3)) {
	case 0:
	    vict = choose_victim(ch, VIC_ALL, MODE_HIT_MIN);
	    break;
	case 1:
	    vict = choose_victim(ch, VIC_ALL, MODE_AC_MAX);
	    break;
	case 2:
	    vict = choose_victim(ch, VIC_ALL, MODE_DR_MAX);
	    break;
	case 3:
	    vict = choose_victim(ch, VIC_ALL, MODE_HR_MAX);
	    break;
	}
	if (vict) {
	    act("$n screams 'PROTECT THE INNOCENT! CHARGE!'", FALSE, ch, 0, 0, TO_ROOM);
	    hit(ch, vict, TYPE_UNDEFINED);	/* set fighting */
	}
	return TRUE;
    }

    if (vict) {		/* other mob is fighting  */
	/* find who fight against vict */
	for (tmp_ch = world[ch->in_room].people; tmp_ch &&
	(tmp_ch->specials.fighting != vict); tmp_ch = tmp_ch->next_in_room) ;

	if (!tmp_ch)
	    return FALSE;

	send_to_char("Yaho! To the rescue...\n\r", ch);
	act("$n screams 'PROTECT THE INNOCENT!'", FALSE, ch, 0, 0, TO_ROOM);
	act("You are rescued by $N, you are confused!", FALSE, vict, 0, ch, TO_CHAR);
	act("$n heroically rescues $N.", FALSE, ch, 0, vict, TO_NOTVICT);
	if (vict->specials.fighting == tmp_ch)
	    stop_fighting(vict);
	if (tmp_ch->specials.fighting)
	    stop_fighting(tmp_ch);
	if (ch->specials.fighting)
	    stop_fighting(ch);
	if (!ch->specials.fighting) {
	    hit(ch, tmp_ch, TYPE_UNDEFINED);
	}
	return TRUE;
    }

    return FALSE;
}

#undef RESCUER_VICTIM

int superguard(struct char_data *ch, int cmd, char *arg)
{
    struct char_data *tch, *evil;

    if (cmd || !AWAKE(ch) || (GET_POS(ch) == POS_FIGHTING))
	return (FALSE);
    evil = 0;
    for (tch = world[ch->in_room].people; tch; tch = tch->next_in_room) {
	if (IS_NPC(tch))
	    continue;
	if (IS_SET(tch->specials.act, PLR_CRIMINAL)) {
	    evil = tch;
	    break;
	}
    }
    if (evil) {
	act("$n 외칩니다. '누가 법을 깨뜨리는 녀석이냐 !!!!'",
	    FALSE, ch, 0, 0, TO_ROOM);
	hit(ch, evil, TYPE_UNDEFINED);
	return (TRUE);
    }
    return (FALSE);
}

int shooter(struct char_data *ch, int cmd, char *arg)
{
    struct char_data *tch;
    extern void shoot(struct char_data *ch, struct char_data *victim, int type);

    if (cmd)
	return (FALSE);
    if (GET_POS(ch) < POS_RESTING)
	return (FALSE);
    if (IS_SET(ch->specials.act, ACT_AGGRESSIVE) || (GET_POS(ch) == POS_FIGHTING)) {
	for (tch = world[ch->in_room].people; tch; tch = tch->next_in_room) {
	    if (PC_MORTAL(tch)) {
		if (GET_POS(tch) <= POS_DEAD)
		    continue;
		act("$n yells '$N must die!'", FALSE, ch, 0, tch, TO_ROOM);
		shoot(ch, tch, TYPE_SHOOT);
	    }
	}
	return (TRUE);
    }
    return (FALSE);
}

int finisher(struct char_data *ch, int cmd, char *arg)
{
    struct char_data *vict;
    int max_dam = 0;

    if (cmd)
	return (FALSE);
    if ((vict = ch->specials.fighting)) {
	if (GET_HIT(ch) < GET_PLAYER_MAX_HIT(ch) / 5) {
	    act("$n 최후의 기를 모은다.  ' 하압 ~~~~~~~~~ '", FALSE, ch, 0, vict, TO_ROOM);
	    max_dam = (GET_HITROLL(ch) + 1) * (GET_DAMROLL(ch) + 1) * number(5, 1) / 2;
	    if (max_dam < 1000)
		act(" 화르르르르르르르르르르르르 `````````` ", FALSE, ch, 0, vict, TO_ROOM);
	    else if (max_dam < 3000) {
		act(" 팟팟파파파파파팟파파파아아 ````````앗 ", FALSE, ch, 0, vict, TO_ROOM);
		act(" 팟팟파파파파파팟파파파아아 ````````앗 ", FALSE, ch, 0, vict, TO_ROOM);
	    }
	    else if (max_dam < 5000) {
		act(" 화르르르르르르르르르르르르 `````````` ", FALSE, ch, 0, vict, TO_ROOM);
		act(" 팟팟파파파파파팟파파파아아 ````````앗 ", FALSE, ch, 0, vict, TO_ROOM);
		act(" 꽈콰콰콰콰콰아아아아아아앙 ~~~~~~~~~~ ", FALSE, ch, 0, vict, TO_ROOM);
	    }
	    else if (max_dam < 7000) {
		act(" 지              ~                  잉 ", FALSE, ch, 0, vict, TO_ROOM);
		act(" 지              ~                     ", FALSE, ch, 0, vict, TO_ROOM);
		act(" 징              ~                     ", FALSE, ch, 0, vict, TO_ROOM);
	    }
	    else {
		act(" 				            ", FALSE, ch, 0, vict, TO_ROOM);
		act(" 				            ", FALSE, ch, 0, vict, TO_ROOM);
		act(" 				            ", FALSE, ch, 0, vict, TO_ROOM);
		act(" 				            ", FALSE, ch, 0, vict, TO_ROOM);
		act(" 				            ", FALSE, ch, 0, vict, TO_ROOM);
	    }
	    damage(ch, vict, max_dam, TYPE_MISC);
	    act("$n 기합을 준다. '끼요요요~~~~ 아아아앗~~~~~~~~  아쵸우~ 아쵸!!!' ", FALSE, ch, 0, vict, TO_ROOM);
	    return (TRUE);
	}
    }
    return (FALSE);
}

int kickbasher(struct char_data *ch, int cmd, char *arg)
{
    struct char_data *vict;

    char buf[80];
    char levels[] = { 0, 20, 30, 35, 40, 99 };
    int j ;

    if ((vict = ch->specials.fighting))
	sprintf(buf, "%s", GET_NAME(vict));
    if (cmd)
	return (FALSE);
    if ((vict = ch->specials.fighting)) { 
	/* NOTE: Simplified code to decide kick/bash number */
	for( j = 0; GET_LEVEL(ch) > levels[j] ; j++)
	    if (number(1, 2) == 1)
		do_kick(ch, buf, 0);
	    else
		do_bash(ch, buf, 0);

	return (TRUE); 
    }
    return (FALSE);
}

int spell_blocker(struct char_data *ch, int cmd, char *arg)
{
    if (cmd == CMD_CAST) {
	do_say(ch, "하하하, 여기서는 마술을 못쓰지!", 0);
	return TRUE;
    }
    else
	return FALSE;
}

int spitter(struct char_data *ch, int cmd, char *arg)
{
    struct char_data *vict;
    void cast_blindness(byte level, struct char_data *ch, char *arg, int type,
			struct char_data *victim, struct obj_data *tar_obj);

    if (cmd == CMD_STEAL) {
	if (IS_NPC(ch))
	    return (TRUE);
	for (vict = world[ch->in_room].people; vict; vict = vict->next_in_room)
	    if (IS_NPC(vict))
		hit(vict, ch, TYPE_UNDEFINED);
	return (TRUE);
    }
    if (!ch->specials.fighting)
	return (FALSE);
    for (vict = world[ch->in_room].people; vict; vict = vict->next_in_room) {
	if ((!IS_NPC(vict)) && (GET_POS(vict) == POS_FIGHTING ||
	       (GET_MANA(vict) > 100 && GET_CLASS(vict) == CLASS_CLERIC))) {
	    switch (number(1, 7)) {
	    case 1:
	    case 2:
	    case 3:
		damage(ch, vict, 50, SKILL_KICK);
		return (FALSE);
	    case 4:
	    case 5:
	    case 6:
		act("$n makes a disgusting noise - then spits at $N.",
		    1, ch, 0, vict, TO_NOTVICT);
		act("$n spit in your eye...", 1, ch, 0, vict, TO_VICT);
		cast_blindness(GET_LEVEL(ch), ch, "", SPELL_TYPE_SPELL, vict, 0);
		return (FALSE);
	    default:
		return (FALSE);
	    }
	}
	if (GET_LEVEL(ch) < 35 || dice(1, 8) < 6)
	    break;
    }
    return (FALSE);
}

int helper(struct char_data *ch, int cmd, char *arg)
{
    struct char_data *vict, *next_victim;
    int lev;

    if (cmd)
	return FALSE;
    for (vict = world[ch->in_room].people; vict; vict = next_victim) {
	next_victim = vict->next_in_room;
	if (IS_NPC(vict)) {
	    if ((100 * GET_HIT(vict) / GET_PLAYER_MAX_HIT(vict)) > 85)
		continue;
	    else
		break;
	}
    }
    if (!vict || number(0, 4) > 3)
	return FALSE;
    lev = GET_LEVEL(ch);
    if (lev <= 11) {
	act("$n utters the words 'asd iasp'.", 1, ch, 0, 0, TO_ROOM);
	cast_cure_light(GET_LEVEL(ch), ch, "", SPELL_TYPE_SPELL, vict, 0);
    }
    else if (lev <= 19) {
	act("$n utters the words 'sagok sghui'.", 1, ch, 0, 0, TO_ROOM);
	cast_cure_critic(GET_LEVEL(ch), ch, "", SPELL_TYPE_SPELL, vict, 0);
    }
    else if (lev <= 27) {
	act("$n utters the words 'laeh'.", 1, ch, 0, 0, TO_ROOM);
	cast_heal(GET_LEVEL(ch), ch, "", SPELL_TYPE_SPELL, vict, 0);
    }
    else {
	act("$n utters the words 'sjagh'.", 1, ch, 0, 0, TO_ROOM);
	cast_full_heal(GET_LEVEL(ch), ch, "", SPELL_TYPE_SPELL, vict, 0);
    }
    return TRUE;
} 

int cityguard(struct char_data *ch, int cmd, char *arg)
{
    struct char_data *tch, *evil;
    int max_evil;

    if (cmd || !AWAKE(ch) || (GET_POS(ch) == POS_FIGHTING))
	return (FALSE);
    max_evil = 1001;
    evil = 0;
    for (tch = world[ch->in_room].people; tch; tch = tch->next_in_room) {
	if (tch->specials.fighting) {
	    if ((GET_ALIGNMENT(tch) < max_evil) && (IS_NPC(tch->specials.fighting))) {
		max_evil = GET_ALIGNMENT(tch);
		evil = tch;
	    }
	}
    }
    if (evil && (GET_ALIGNMENT(evil->specials.fighting) >= 0)) {
	act("$n screams 'PROTECT THE INNOCENT!!! CHARGE!!!'",
	    FALSE, ch, 0, 0, TO_ROOM);
	hit(ch, evil, TYPE_UNDEFINED);
	return (TRUE);
    }
    return (FALSE);
} 

/* NOTE: NEW! Make newly regened better prepared and combat-ready. */
void regened_mobile(struct char_data *mob)
{
    struct obj_data *obj;

    ASSERT( mob->regened == 1 ); 
    /* NOTE: Give 'Solar Flare' potion to high level aggressive mobile */
    if ( GET_LEVEL(mob) >= 35 && (number(33, 43 ) < GET_LEVEL(mob) )
	&& IS_ACTPLR(mob, ACT_AGGRESSIVE) && !IS_AFFECTED(mob, AFF_SANCTUARY)) {
	obj = read_object(POTION_SOLAR, VIRTUAL );
	obj_to_char( obj, mob );
    }

    /* NOTE: Give 'dagger' to thief mobile for backstab.    */
    /* 	     Give 'sword' to warrior mobile for bash.	    */
    if ( !MAGIC_CLASS(mob) && !mob->equipment[WIELD] 
	    && (number(10, 30) < GET_LEVEL(mob) ) ) {
	if ( GET_CLASS(mob) == CLASS_THIEF ) {
	    obj = read_object(WEAPON_BASIC_THIEF, VIRTUAL); /* 2D4 dagger */
#ifdef  UNUSED_CODE
        /* NOTE: Warrior mobile can 'bash' without wielding sword. */ 
	else 
	    obj = read_object(WEAPON_BASIC_WARRIOR, VIRTUAL);  /* 1D8 sword */
#endif		/* UNUSED_CODE */
	
	    if (obj) {
		obj_to_char( obj, mob );
		wear(mob, obj, 12);	/* NOTE: 12 == wield it */
	    } 
	}
    }

    mob->regened++ ; /* NOTE: Done! */
}
