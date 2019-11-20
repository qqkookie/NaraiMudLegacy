
#include <stdio.h>

#include "char.h"
#include "object.h"
#include "global.h"
#include "comm.h"
#include "play.h"
#include "etc.h"

/* ************************************************************************
   *  file: limits.c , Limit and gain control module.        Part of DIKUMUD *
   *  Usage: Procedures controling gain and limit.                           *
   *  Copyright (C) 1990, 1991 - see 'license.doc' for complete information. *
   ************************************************************************* */


#define SPELL_LEARN_MAX	120

/* When age < 15 return the value p0 */
/* When age in 15..29 calculate the line between p1 & p2 */
/* When age in 30..44 calculate the line between p2 & p3 */
/* When age in 45..59 calculate the line between p3 & p4 */
/* When age in 60..79 calculate the line between p4 & p5 */
/* When age >= 80 return the value p6 */
int graf(int age, int p0, int p1, int p2, int p3, int p4, int p5, int p6)
{
    if (age < 15)
	return (p0);	/* < 15   */
    else if (age <= 29)
	return (int) (p1 + (((age - 15) * (p2 - p1)) / 15));	/* 15..29 */
    else if (age <= 44)
	return (int) (p2 + (((age - 30) * (p3 - p2)) / 15));	/* 30..44 */
    else if (age <= 59)
	return (int) (p3 + (((age - 45) * (p4 - p3)) / 15));	/* 45..59 */
    else if (age <= 79)
	return (int) (p4 + (((age - 60) * (p5 - p4)) / 20));	/* 60..79 */
    else
	return (p6);	/* >= 80 */
}


/* The three MAX functions define a characters Effective maximum */
/* Which is NOT the same as the ch->points.max_xxxx !!!          */

int hit_limit(struct char_data *ch)
{
    int extra[7] =
    {1, 2, 4, 8, 4, 2, 1};
    int max;

    /* int share, i ; */

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
    int extra[7] =
    {1, 2, 3, 4, 3, 2, 1};
    int max;

    /* int share, i; */

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
    int extra[7] =
    {1, 2, 4, 8, 4, 2, 1};
    int max;

    /* int share, i; */

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
    int extra[7] =
    {1, 3, 6, 8, 5, 3, 1};
    int i;

    c = GET_CON(ch) / 2;

    if (IS_NPC(ch)) {
	gain = GET_LEVEL(ch) * c;
    }
    else {
	for (i = 0; i < 7; i++)
	    extra[i] *= c;
	gain = graf(age(ch).year, extra[0], extra[1], extra[2], extra[3],
		    extra[4], extra[5], extra[6])
	    + ((ch->regeneration + GET_LEVEL(ch)) * 4 );

	/* class */
	switch (GET_CLASS(ch)) {
	case CLASS_MAGIC_USER:
	    gain += gain >> 3;
	    break;
	case CLASS_CLERIC:
	    gain += gain >> 3;
	    break;
	case CLASS_THIEF:
	    gain += (gain / 2);
	    break;
	case CLASS_WARRIOR:
	    gain += gain;
	    break;
	}

	/* position */
	switch (GET_POS(ch)) {
	case POS_SLEEPING:
	    gain += gain;
	    break;
	case POS_RESTING:
	    gain += gain >> 1;
	    break;
	case POS_SITTING:
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
	/* NOTE: Move This code to point_update(). */
	/* GET_HIT(ch) -= dice(30 - GET_CON(ch), 30 - GET_CON(ch)); */
    }

    if ((GET_COND(ch, FULL) == 0) || (GET_COND(ch, THIRST) == 0))
	gain >>= 2;

    return gain;
}

int mana_gain(struct char_data *ch)
{
    int gain, c;
    int extra[7] =
    {1, 2, 4, 8, 4, 2, 1};
    int i;

    c = GET_INT(ch) / 2;

    if (IS_NPC(ch)) {
	gain = GET_LEVEL(ch);
    }
    else {
	for (i = 0; i < 7; i++)
	    extra[i] *= c;
	gain = graf(age(ch).year, extra[0], extra[1], extra[2], extra[3],
		    extra[4], extra[5], extra[6])
	    + (( ch->regeneration + GET_LEVEL(ch)) << 2);

	/* level */
	gain += GET_LEVEL(ch) * 2;
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
	case POS_SLEEPING:
	    gain += gain;
	    break;
	case POS_RESTING:
	    gain += gain >> 1;
	    break;
	case POS_SITTING:
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
    int extra[7] =
    {1, 2, 4, 8, 4, 2, 1};
    int i;

    c = GET_DEX(ch) / 2;

    if (IS_NPC(ch)) {
	gain = GET_LEVEL(ch);
    }
    else {
	for (i = 0; i < 7; i++)
	    extra[i] *= c;
	gain = graf(age(ch).year, extra[0], extra[1], extra[2], extra[3],
		    extra[4], extra[5], extra[6])
	    + (( ch->regeneration + GET_LEVEL(ch)) << 2);

	/* level */
	gain += GET_LEVEL(ch) * 2;
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
	    gain = gain * 3 / 2;
	    break;
	}

	/* position */
	switch (GET_POS(ch)) {
	case POS_SLEEPING:
	    gain += gain;
	    break;
	case POS_RESTING:
	    gain += gain >> 1;
	    break;
	case POS_SITTING:
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

    if (IS_DIVINE(ch))
	for (i = 0; i < 3; i++)
	    ch->specials.conditions[i] = -1;
}

void gain_gold(struct char_data *ch, LONGLONG money)
{
    if (money > 0) {
	/* NOTE: Check overflow, purse max is about 2100 M coin */
	if (GET_GOLD(ch) + money > 0)
	    GET_GOLD(ch) += money;
	else
	    send_to_char("Your perse is almost bursting.\n\r", ch);
    }
    else {
	if (GET_GOLD(ch) > (-money))
	    GET_GOLD(ch) += money;
	else
	    GET_GOLD(ch) = 0;
    }
}

void gain_exp(struct char_data *ch, LONGLONG gain)
{
    if (IS_NPC(ch) || (IS_MORTAL(ch) && (GET_LEVEL(ch) > 0))) {
	if (gain > 0) {
	    gain = MIN(GET_LEVEL(ch) * GET_LEVEL(ch) * 20000, gain);
	    /* NOTE: prevent exp. overflow */
	    if (GET_EXP(ch) + gain > 0)
		GET_EXP(ch) += gain;
	    else
		send_to_char("You seems too much experinced.\n\r", ch);
	}
	else {		/* NOTE : prevent negative exp. */
	    if (GET_EXP(ch) > (-gain))
		GET_EXP(ch) += gain;
	    else
		GET_EXP(ch) = 0;
	}
    }
}

void gain_exp_regardless(struct char_data *ch, LONGLONG gain)
{
    int i;
    bool is_altered = FALSE;
    extern void set_title(struct char_data *ch);

    if (!IS_NPC(ch)) {
	if (gain > 0) {
	    GET_EXP(ch) += gain;
	    for (i = 0; (i <= LEV_GOD) &&
		 (titles[GET_CLASS(ch) - 1][i].exp <= GET_EXP(ch)); i++) {
		if (i > GET_LEVEL(ch)) {
		    send_to_char("You raise a level.\n\r", ch);
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

void gain_condition(struct char_data *ch, int condition, int value)
{ 
    if (GET_COND(ch, condition) == -1)	/* No change */
	return;

    GET_COND(ch, condition) += value;
    GET_COND(ch, condition) = MAX(0, GET_COND(ch, condition));
    GET_COND(ch, condition) = MIN(36, GET_COND(ch, condition));
    /* NOTE: hungry, thirst message moved to report_condition(). */
}

/* NOTE: NEW! reportting condition called by point_update() */
void report_condition(struct char_data *ch )
{
    if (GET_COND(ch, FULL) < 5 && GET_COND(ch, FULL) > 0 ) 
	send_to_char_han ( "Your stomach will be empty soon.\n\r",
	    "약간 배가 고파오기 시작합니다.\n\r", ch);
    else if ( GET_COND(ch, FULL) == 0 ) {
	send_to_char("You are hungry.\n\r", ch);
	ch->points.hit = ( ch->points.hit * 9 ) /10;
    }
    if (GET_COND(ch, THIRST) < 5 && GET_COND(ch, THIRST) > 0 )
	send_to_char_han ( "You need some water now.\n\r",
	    "약간 목이 말라오기 시작합니다.\n\r", ch);
    else if (GET_COND(ch, THIRST) == 0 ) {
	send_to_char("You are thirsty.\n\r", ch);
	ch->points.hit = ( ch->points.hit * 9 ) /10 ;
    }
    /* NOTE: Reduce move point when you are drunk */
    if ( GET_COND(ch, DRUNK) >= 20 ) {
	send_to_char("You are drunk.\n\r", ch);
	ch->points.move = ( ch->points.move * 9 ) /10 ;
    }
    else if ( GET_COND(ch, DRUNK) > 15 ) 
	send_to_char_han ( "You feel you begin to be sober.\n\r",
	    "조금 술이 깨는것 같습니다.\n\r", ch);
    else if ( GET_COND(ch, DRUNK) == 2 || GET_COND(ch, DRUNK)== 1) 
	send_to_char("You are now sober.\n\r", ch);
}

/* Update both PC's & NPC's and objects */
void point_update(void)
{
    void update_char_objects(struct char_data *ch);	/* handler.c */
    void extract_obj(struct obj_data *obj);	/* handler.c */
    struct char_data *i, *next_dude;
    struct obj_data *j, *next_thing, *jj, *next_thing2;
    char buf[100];
    extern int level_quest[];
    extern void check_idling(struct char_data *ch);

    /* characters */
    for (i = character_list; i; i = next_dude) {
	next_dude = i->next;

	if (GET_POS(i) > POS_STUNNED) {
	    GET_HIT(i) = MIN(GET_HIT(i) + hit_gain(i), hit_limit(i));
	    GET_MANA(i) = MIN(GET_MANA(i) + mana_gain(i), mana_limit(i));
	    GET_MOVE(i) = MIN(GET_MOVE(i) + move_gain(i), move_limit(i));

	    /* NOTE: Moved from hit_gain() */
	    if (IS_AFFECTED(i, AFF_POISON)) {
		/* GET_HIT(ch) -= dice(30 - GET_CON(i), 30 - GET_CON(i)); */
		/* NOTE: More poison damage : Avrg: 72 -> 510  */
		damage( i, i, dice(30 - GET_CON(i), GET_LEVEL(i)*2 + 5),
			SPELL_POISON );  
		/* 
		GET_HIT(ch) -= dice(GET_LEVEL(i), GET_LEVEL(i));
		if (IS_NPC(ch))
		   GET_HIT(ch) -= dice(GET_LEVEL(i), GET_LEVEL(i));
		*/
	    }
	}
	else if (GET_POS(i) == POS_STUNNED) {
	    GET_HIT(i) = MIN(GET_HIT(i) + hit_gain(i), hit_limit(i));
	    GET_MANA(i) = MIN(GET_MANA(i) + mana_gain(i), mana_limit(i));
	    GET_MOVE(i) = MIN(GET_MOVE(i) + move_gain(i), move_limit(i));
	    update_pos(i);
	} 
	/* NOTE: OLD suffering.
	else if (GET_POS(i) == POS_INCAP) 
	    damage(i, i, 1, TYPE_SUFFERING);
	else if (!IS_NPC(i) && (GET_POS(i) == POS_MORTALLYW))
	    damage(i, i, 2, TYPE_SUFFERING);
	*/ 
	/* NOTE: More damage to suffering char */
	else if (!IS_NPC(i) && (GET_POS(i) <= POS_MORTALLYW))
	    damage(i, i, 20, TYPE_SUFFERING);
	else 
	    damage(i, i, 10, TYPE_SUFFERING);

	gain_condition(i, FULL, -1);
	/* NOTE: Speed up drunk recovery. It was -1 before. */
	gain_condition(i, DRUNK, -2);
	gain_condition(i, THIRST, -1);

	if (IS_NPC(i)) 
	    continue;

	/* NOTE: gain_coindition() and report_condition() separated */
	report_condition(i); 

	update_char_objects(i);

	/* auto level up by Perhaps */
	if ((titles[GET_CLASS(i) - 1][GET_LEVEL(i) + 1].exp + 1000) < GET_EXP(i)
		&& !IS_NPC(i) && (GET_LEVEL(i) < 40) 
		&& ((i->quest.solved) >= level_quest[GET_LEVEL(i)])) {
	    GET_LEVEL(i)++;
	    advance_level(i, 1);
	    sprintf(buf, "\n\r %s LEVEL UP !! ---==Congratulations==--- \n",
		i->player.name);
	    send_to_all(buf);
	}

	/* auto level down by Perhaps */
	/* remove level-down */
	/* 
	if( ( titles[GET_CLASS(i)-1][GET_LEVEL(i)].exp > GET_EXP(i) 
		&&!IS_NPC(i) &&(GET_LEVEL(i)>5)&&(GET_LEVEL(i)<=40) ) 
		&& (GET_GUILD(i)==0) ) {
	    advance_level(i, 0);
	    GET_LEVEL(i)--;
	    sprintf(buf,"\n\r %s LEVEL DOWN!! <--==Congratulations?!?!==-->\n",
	    i->player.name);
	    send_to_all(buf);
	}
	*/

	/* NOTE: char i may be free()'d in check_idling()  */
	if (IS_MORTAL(i))
	    check_idling(i); 
    }			/* for */

    /* objects */
    for (j = object_list; j; j = next_thing) {
	next_thing = j->next;	/* Next in object list */

	/* If this is a corpse */
	if ((GET_ITEM_TYPE(j) == ITEM_CONTAINER) && (j->obj_flags.value[3])) {
	    /* timer count down */
	    if (j->obj_flags.timer > 0)
		j->obj_flags.timer--;

	    if (!j->obj_flags.timer) {

		if (j->carried_by)
		    act("$p decay in your hands.",
			FALSE, j->carried_by, j, 0, TO_CHAR);
		else if ((j->in_room != NOWHERE) 
			&& (world[j->in_room].people)) {
		    act("A quivering hoard of maggots consume $p.",
			TRUE, world[j->in_room].people, j, 0, TO_ROOM);
		    act("A quivering hoard of maggots consume $p.",
			TRUE, world[j->in_room].people, j, 0, TO_CHAR);
		}

		for (jj = j->contains; jj; jj = next_thing2) {
		    next_thing2 = jj->next_content;	/* Next in inventory */
		    obj_from_obj(jj);

		    /* keep items in PC's corpse, changed by jhpark */
		    if ((GET_ITEM_TYPE(j) == ITEM_CONTAINER)
			    && (j->obj_flags.value[3] == 2)) {
			if (j->in_obj)
			    obj_to_obj(jj, j->in_obj);
			else if (j->carried_by)
			    obj_to_room(jj, j->carried_by->in_room);
			else if (j->in_room != NOWHERE)
			    obj_to_room(jj, j->in_room);
		    }
		    else {
			if ((GET_ITEM_TYPE(jj) == ITEM_CONTAINER)
				&& (jj->obj_flags.value[3])) {
			    if (j->in_obj)
				obj_to_obj(jj, j->in_obj);
			    else if (j->carried_by)
				obj_to_room(jj, j->carried_by->in_room);
			    else if (j->in_room != NOWHERE)
				obj_to_room(jj, j->in_room);
			}
		    }
		}
		extract_obj(j);
	    }
	}
    }
} 

void check_idling(struct char_data *ch)
{
    extern void do_rent(struct char_data *ch, char *arg, int cmd);
    extern void purge_player(struct char_data *ch);
    extern void stash_char(struct char_data *ch);

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
	    char_to_room(ch, ROOM_VOID);	/* Into room number 0 */
	}
	else if (ch->specials.timer >= 35) {
	    /* NOTE: do_rent(), extract_char() and free_char(). */
	    save_char(ch);
	    stash_char(ch);
	    purge_player(ch);
	}
    }
}

/* ================================================================ */

#define OLD_NARAI_EXP
/* NOTE: Define This to return to OLD Narai exp and gold gain system.   */

#ifdef OLD_NARAI_EXP

int eval_exp(struct char_data *victim)
{
    int exp;

    if (IS_NPC(victim))
	exp = GET_EXP(victim);
    else
	exp = GET_EXP(victim) * 7 / 100;
    /* exp = ((GET_EXP(victim) / 10) * 4 / 9); */

    exp = MAX(exp, 1);
    return (exp);
}

/* #define EVAL_GOLD(victim) ( GET_GOLD(victim) + \
		(GET_LEVEL(victim) * GET_LEVEL(victim) )*60) */
int eval_gold(struct char_data *victim)
{
   return( GET_GOLD(victim) + (GET_LEVEL(victim) * GET_LEVEL(victim) )*60);
}

#else				/* !OLD_NARAI_EXP */

#define GOLDSCALE 15

/* NOTE: evaluate how hard to kill mob in experince gain.       */
/* NOTE: It's very rough estimation, needs much REFINEMENT.     */
/* approximately exp = sqrt( dam * hit * AC * 100/40 );         */
/* where dam = (hand_dice + damroll) * (level/6) for warrior.   */
/* Very careful not to overflow or underflow. (loss of precision) */
/* NOTE: Rewrite! This proved to be wrong !!                    */
/* NOTE: Back to OLD Narai exp system :(    Fri Sep 26 14:04:18 KST 1997 */

int eval_exp(struct char_data *victim)
{
#define DIV 40		/* scale factor! Need to do new balancing  */
    u_int lev, dam, hitAC, exp;
    int hit_limit(struct char_data *);
    int isqrt(int n);

    /* NOTE: Should this moved to adjust_gain() .... *??? */
    if (!victim || (IS_NPC(victim) && GET_EXP(victim) == 0))
	return (0);	/* Reanimated or cloned mob */

    lev = GET_LEVEL(victim) + 5;	/* +5 for lowest levels */
    dam = ((victim->specials.damnodice + 2) * (victim->specials.damsizedice + 1)) / 2;
    if (GET_CLASS(victim) == CLASS_WARRIOR)
	dam = (dam + GET_DAMROLL(victim) + 6) * (lev / 6);
    else
	dam = (dam / 2 + GET_DAMROLL(victim) + 7) * (lev / 7) + lev * 7;
    /* dam = Approximate average damge of each turn */

    /* hitAC = (hit_limit(victim) + 1000) * (-GET_AC(victim) + 30 ) ; */
    hitAC = (hit_limit(victim) + 1000) * (-GET_AC(victim) + 30);

    /* In short, exp =  sqrt (max_hit * level * dam * AC * 100/DIV)  */
    /* NOTE: Careful not to overflow or underflow */
    if (hitAC > 1000000)	/* below  about level 15 */
	exp = isqrt(hitAC / (DIV * 10)) * isqrt(dam * lev / 10) * 100;
    else
	exp = isqrt((hitAC * lev / DIV) * dam);
    if (IS_NPC(victim)) {
	/* prevent extreme over or under estimation         */
	exp = MIN(exp, GET_EXP(victim) * 2);
	exp = (GET_EXP(victim) + exp * 2) / 3;
    }
    else		/* Advantage for PK! Evil *grin*... */
	exp = exp * 10 + GET_EXP(victim) / 10;

    return (exp);
}

/* NOTE: calculate square root of interger n by Newton-Rapsohn method */
int isqrt(int n)
{
    int guess = 500, itr = 8;

    while (itr--)
	guess = (guess + n / guess) / 2;
    return (guess);
}

#endif				/* OLD_NARAI_EXP */


/* NOTE: Do exp, gold and alignment manipulation and show scoring */
void adjust_gain(struct char_data *ch, struct char_data *victim,
		 u_int exp, u_int money)
{
    char buf[256];
    extern void change_alignment(struct char_data *ch,
    		struct char_data *victim);
    extern void check_quest_mob_die(struct char_data *ch,
		struct char_data *victim);

    /* NOTE: Char affected by SPELL_RERAISE can't get point. */
    if (!ch || !victim || IS_AFFECTED(ch, AFF_RERAISE))
	return;
    /* NOTE: Checking for cloned/reanimated mob victim moved to victory() */

    if (!IS_NPC(victim)) {	/* if player killed . */
	ch->player.pk_num++;
	GET_ALIGNMENT(ch) -= 100;
    }
    else {
	change_alignment(ch, victim);
	/* Quest : check quest */
	/* NOTE: check master/and peer only when ch is grouped. */
	/* NOTE: This is bug found by BADA  */ 
	check_quest_mob_die( ch, victim ); 
	/* NOTE:  Check for victim is 'target' which char is hunting for. */
	if( ch->specials.hunting == victim )
		ch->specials.hunting = NULL;
    }

    /* NOTE: Check overflow. */
    gain_exp(ch, exp);
    gain_gold(ch, money);

    if (!IS_NPC(ch)) {
	/* NOTE: format gain for display */
	sprintf(buf, STRHAN(
		  "You receive about %s experiences and %s gold coins.\r\n",
		"당신은 약 %s 점의 경험치와 %s의 금을 얻었습니다.\r\n", ch),
		monetary4(exp), monetary4(money));
	send_to_char(buf, ch);
    }
}

/* NOTE: NEW! Separate code for add winner kill gain from damage() */
void victory(struct char_data *ch, struct char_data *victim)
{ 
    int exp, money;
    unsigned no_members, share;
    struct char_data *k;
    struct follow_type *f;
    /* unsigned high_level; */
    unsigned total_level;
    unsigned level_exp;

    if (!ch || !victim || ch == victim 
	/* NOTE: NO GAIN  from Reanimated or cloned mob */
	|| (IS_NPC(victim) && GET_EXP(victim) == 0))
	return;

    /* NOTE: Code to quest_mob_die/ hunting mob moved to adjust_gain() */

    if (!IS_AFFECTED(ch, AFF_GROUP|AFF_CHARM)) {
	/* NOTE: Solo  gain */
	/* Calculate level-difference bonus? */
	/* NOTE: OLD NARAI exp, gold for solo player.   */
	/*
	exp = GET_EXP(victim);
	exp = MAX(exp, 1);
	GET_GOLD(ch) += GET_GOLD(victim);
	GET_GOLD(ch) += (GET_LEVEL(victim)*GET_LEVEL(victim)*500);
	*/ 
	/* NOTE: New solo player gain code is here. */
	/* NOTE: more elaborate exp gain calculation. */
	/* NOTE: potential gold inflation pitfall. */
	exp = eval_exp(victim);
	if (IS_NPC(victim))
	    money = eval_gold(victim);
	else
	    money = GET_GOLD(victim);

	/* manipulate exp, gold, align, pk_num */
	adjust_gain(ch, victim, exp, money);
	return;
    }

    /* NOTE: Group gain */ 
    if (!(k = ch->master))
	k = ch;

    /* NOTE: To apply group gain, make sure leader is grouped.  */
    if (!k || !IS_AFFECTED(k, AFF_GROUP)) 
	return;

    /* NOTE: To get point, leader/follower should be in same room 
		with victim, not character.    */
    if (k->in_room == victim->in_room) { 
	no_members = 1; 
	total_level = GET_LEVEL(k);
    }
    else {
	no_members = 0; 
	total_level = 0;
    }

    /* find highest level in same room */
/* NOTE: calculated high_level no longer used. Comment out */
/*
    high_level = GET_LEVEL(k);
    for (f = k->followers; f; f = f->next)
	if (IS_AFFECTED(f->follower, AFF_GROUP) &&
		(f->follower->in_room == victim->in_room))
	    high_level = MAX(high_level, GET_LEVEL(f->follower));
 */

    /* calculate total member, total level */
    for (f = k->followers; f; f = f->next)
	if (IS_AFFECTED(f->follower, AFF_GROUP) && !IS_NPC(f->follower) &&
	    (f->follower->in_room == victim->in_room)) {
	    no_members++;
	    total_level += GET_LEVEL(f->follower);
	}

    /* NOTE: This assertion fails sometimes.. Why? Winner fled? Find out! */
    /* assert(no_members); *//* To prevent divide by zero */
    if ( no_members <= 0 || total_level <= 0)
	log("DEBUG: victory(): no_members/total_level ???");
    no_members = MAX(1, no_members);
    total_level = MAX(1, total_level);

#ifdef NO_DEF
/*
    money = GET_GOLD(victim) * ((no_members >> 1) + 1);
    money += (GET_LEVEL(victim) * GET_LEVEL(victim) * GET_LEVEL(victim));
    money /= no_members;
    level_exp = GET_EXP(victim) * ((no_members >> 1) + 1) / total_level; 
  */

    /* group advantage */
/* NOTE: OLD NARAI exp, gold for group players. */
/*
    money = GET_GOLD(victim) * ((no_members >> 1) + 1);
    money += (GET_LEVEL(victim) * GET_LEVEL(victim) * GET_LEVEL(victim));
    money /= ( no_members * no_members );
    level_exp = GET_EXP(victim) * ((no_members >> 1) + 1) / total_level;
 */
#endif				/* NO_DEF */

    /* NOTE: New rule. Old rule was too unfair to large group. */
    /* NOTE: potential gold inflation (coin copy bug) pitfall. */
    /* Solution: prohibit giving gold to mob (Not yet done) */
    if (IS_NPC(victim)) {
/* NOTE: This *WAS* NEW NARAI exp system.. but not yet... */
/*
    share = ( GET_GOLD(victim) + 
    (GET_LEVEL(victim) * GET_LEVEL(victim) )*GOLDSCALE)/no_members ;
 */
	share = eval_gold(victim) / no_members;
	level_exp = eval_exp(victim) * ((no_members - 1) / 2 + 1) / total_level;
    }
    else {		/* Group PK! Hmm.. Goood...  */
	share = GET_GOLD(victim) / no_members;
	level_exp = eval_exp(victim) / total_level;
    }

    if (k->in_room == victim->in_room)
	/* manipulate exp, gold, align, pk_num */
	adjust_gain(k, victim, level_exp * GET_LEVEL(k), share);

    for (f = k->followers; f; f = f->next) {
	if (IS_AFFECTED(f->follower, AFF_GROUP) &&
	    f->follower->in_room == victim->in_room)
	    adjust_gain(f->follower, victim,
			level_exp * GET_LEVEL(f->follower), share);
    }

/* NOTE: adjust_gain() will do same thing. Comment out. */
#ifdef NO_DEF
    if (IS_AFFECTED(k, AFF_GROUP) && (k->in_room == ch->in_room)) {
	share = level_exp * GET_LEVEL(k);
	sprintf(buf, "You receive %d experience and %d gold coins.",
		share, money);
	sprintf(buf2, "당신은 %d 점의 경험치와 %d의 금을 얻었습니다.",
		share, money);
	acthan(buf, buf2, FALSE, k, 0, 0, TO_CHAR);
	if (!IS_NPC(k)) {
	    gain_exp(k, share);		/* Perhaps modified for mob's exp */
	    GET_GOLD(k) += money;
	}
	change_alignment(k, victim);
    }

    /* followers gain exp and gold */
    for (f = k->followers; f; f = f->next) {
	if (IS_AFFECTED(f->follower, AFF_GROUP) &&
	    f->follower->in_room == ch->in_room) {
	    share = level_exp * GET_LEVEL(f->follower);
	    sprintf(buf, "You receive %d experience and %d gold coins.",
		    share, money);
	    sprintf(buf2, "당신은 %d 점의 경험치와 %d의 금을 얻었습니다.",
		    share, money);
	    acthan(buf, buf2, FALSE, f->follower, 0, 0, TO_CHAR);
	    if (!IS_NPC(f->follower)) {
		gain_exp(f->follower, share);	/* Perhaps modified */
		GET_GOLD(f->follower) += money;
	    }
	    change_alignment(f->follower, victim);
	}
    }
#endif				/* NO_DEF */
}
