/* ************************************************************************
   *  file: act.other.c , Implementation of commands.        Part of DIKUMUD *
   *  Usage : Other commands.                                                *
   *  Copyright (C) 1990, 1991 - see 'license.doc' for complete information. *
   ************************************************************************* */

#include <stdio.h>
#include <string.h>
/* #include <ctype.h> */
#include <stdlib.h>


#include "char.h"
#include "object.h"
#include "global.h"
#include "comm.h"
#include "play.h"
#include "spells.h"


bool circle_follow(struct char_data *ch, struct char_data *victim);
int check_leadership(struct char_data *leader);
void die_follower(struct char_data *ch);

void do_group(struct char_data *ch, char *argument, int cmd)
{
    char buf[256], name[256];
    struct char_data *victim, *k;
    struct follow_type *f;
    bool found;

    one_argument(argument, name);

    if (!*name) {
	if (ch && !IS_AFFECTED(ch, AFF_GROUP)) {
	    send_to_char("But you are a member of no group?!\n\r", ch);
	}
	else {
	    send_to_char("Your group consists of:\n\r", ch);
	    if (ch->master)
		k = ch->master;
	    else
		k = ch;

	    if (k && IS_AFFECTED(k, AFF_GROUP)) {
		sprintf(buf, "  [ %5d/%5d %5d/%5d %5d/%5d ]   $N (Head of group)",
			GET_HIT(k), GET_PLAYER_MAX_HIT(k), GET_MANA(k),
		GET_PLAYER_MAX_MANA(k), GET_MOVE(k), GET_PLAYER_MAX_MOVE(k));
		act(buf, FALSE, ch, 0, k, TO_CHAR);
	    }

	    for (f = k->followers; f; f = f->next)
		if (f->follower && IS_AFFECTED(f->follower, AFF_GROUP)) {
		    sprintf(buf, "  [ %5d/%5d %5d/%5d %5d/%5d ]   $N",
		      GET_HIT(f->follower), GET_PLAYER_MAX_HIT(f->follower),
		    GET_MANA(f->follower), GET_PLAYER_MAX_MANA(f->follower),
		    GET_MOVE(f->follower), GET_PLAYER_MAX_MOVE(f->follower));
		    act(buf, FALSE, ch, 0, f->follower, TO_CHAR);
		}
	}

	return;
    }

    if (!(victim = get_char_room_vis(ch, name))) {
	send_to_char("No one here by that name.\n\r", ch);
    }
    else {

	if (ch->master) {
	    act("You can not enroll group members without being head of a group.",
		FALSE, ch, 0, 0, TO_CHAR);
	    return;
	}

	if (IS_NPC(victim)) {
	    act("You can not enroll mob into your group.",
		FALSE, ch, 0, 0, TO_CHAR);
	    send_to_char("But you can enroll mobs by amiable Process.\r\n", ch);
	}

	found = FALSE;
/*
   if (victim == ch) 
   found = TRUE;
 */
	/* NOTE: Now, you can't group/ungroup yourself. And no need to do so */
	if (victim == ch) {
	    if (ch->followers)	/* NOTE: leader is trying to get out of group */
		send_to_char("Wanna abandon your followers? Try ungroup.\r\n",
				ch);
	    else	/* NOTE: group yourself alone? Bad idea!   */
		send_to_char("One man makes solo; Two men, a group; "
			     "Three is a crowd.\r\n", ch);
	    return;
	}
	else {
	    for (f = ch->followers; f; f = f->next) {
		if (f->follower == victim) {
		    found = TRUE;
		    break;
		}
	    }
	}

	if (found) {
	    if (victim && IS_AFFECTED(victim, AFF_GROUP)) {
		act("$n has been kicked out of the group!",
		    FALSE, victim, 0, ch, TO_ROOM);
		act("You are no longer a member of the group!",
		    FALSE, victim, 0, 0, TO_CHAR);
		REMOVE_BIT(victim->specials.affected_by, AFF_GROUP);

		/* NOTE: If no one except leader is grouped, disband group   */
		check_leadership(ch);	/* Am I still a group leader? */

	    }
	    else {
		act("$n is now a group member.", FALSE, victim, 0, 0, TO_ROOM);
		act("You are now a group member. $N is head of this group.",
		    FALSE, victim, 0, ch, TO_CHAR);
		SET_BIT(victim->specials.affected_by, AFF_GROUP);

		/* NOTE: group leader will be grouped implicitely    */
		if (!IS_AFFECTED(ch, AFF_GROUP)) {
		    SET_BIT(ch->specials.affected_by, AFF_GROUP);
		    act("You are now group leader.", FALSE, ch, 0, 0, TO_CHAR);
		}
	    }
	}
	else {
	    act("$N must follow you, to enter the group",
		    FALSE, ch, 0, victim, TO_CHAR);
	}
    }
} 

/* NOTE: Confusing code, I clean it up! */
void do_follow(struct char_data *ch, char *argument, int cmd)
{
    char name[160];
    struct char_data *leader;

    void stop_follower(struct char_data *ch);
    void add_follower(struct char_data *ch, struct char_data *leader);

    one_argument(argument, name);

    /* NOTE: cleaned up confusing code for clarity. */
    if (!*name) {
	send_to_char("Who do you wish to follow?\n\r", ch);
    }
    else if (!(leader = get_char_room_vis(ch, name))) {
	send_to_char("I see no person by that name here!\n\r", ch);
    }
    else if (IS_SET(leader->specials.act, PLR_SOLO)) {
	send_to_char("The leader is a solo player.\n\r", ch);
    }
    else if (IS_AFFECTED(ch, AFF_CHARM) && (ch->master)) {
	act("But you only feel like following $N!",
	    FALSE, ch, 0, ch->master, TO_CHAR);
    }
    /* Not Charmed follow person */
    else if (leader == ch) {
	if (!ch->master)
	    send_to_char("You are already following yourself.\n\r", ch);
	else
	    stop_follower(ch);
    }
    /* NOTE: You can't follow someone who is alredy following other leader */
    else if (leader->master) {
	act("Sorry, $S is following someone else.",
	    FALSE, ch, 0, leader, TO_CHAR);
    }
    else if (circle_follow(ch, leader)) {
	act("Sorry, but following in 'loops' is not allowed",
	    FALSE, ch, 0, 0, TO_CHAR);
    }
    else {
	if (ch->master)
	    stop_follower(ch);

	/* NOTE: when player starting follow, he/she is not member of group.
	   Follower must be explicitely grouped by leader */
	if (IS_AFFECTED(ch, AFF_GROUP))
	    REMOVE_BIT(ch->specials.affected_by, AFF_GROUP);

	add_follower(ch, leader);
    }
    return;
} 

/* NOTE: die_follower() does equal job to stop_group() */
void do_ungroup(struct char_data *ch, char *argument, int cmd)
{
    die_follower(ch);
    REMOVE_BIT(ch->specials.affected_by, AFF_GROUP);
    send_to_char("Your group is dismissed.\n\r", ch);
} 

/* NOTE: Check if char is still a leader.                       */
/*      If no one except leader is grouped, disband this group  */
int check_leadership(struct char_data *leader)
{
    struct follow_type *f;

    /* NOTE: BUG FIX : If ch is not leader, don't check leadership. 
	A followed B. But B doesn't group A and followed C.
	If A stop following B, asset(!ch->master) will fail. */

    if (leader->master)	/* should be leader */
	return(0);

    for (f = leader->followers; f; f = f->next)
	if (IS_AFFECTED(f->follower, AFF_GROUP))
	    return (1);

    REMOVE_BIT(leader->specials.affected_by, AFF_GROUP);   /* resign leader */
    return (0);
}

/* Check if making CH follow VICTIM will create an illegal */
/* Follow "Loop/circle"                                    */
bool circle_follow(struct char_data *ch, struct char_data *victim)
{
    struct char_data *k;

    for (k = victim; k; k = k->master) {
	if (k == ch)
	    return (TRUE);
    } 
    return (FALSE);
}

#ifdef  NO_DEF
/* NOTE: stop_group() is merged to stop_follower().
	It does essentially same thing to stop_follower(). */
void stop_group(struct char_data *ch)
{
    struct follow_type *j, *k;

    if (!ch->master) {
	log("No master but called stop_group");
	return;
    }

    act("You stop following $N.", FALSE, ch, 0, ch->master, TO_CHAR);
    act("$n stops following $N.", FALSE, ch, 0, ch->master, TO_NOTVICT);
    act("$n stops following you.", FALSE, ch, 0, ch->master, TO_VICT);

    /* NOTE: DEBUG: my master has no follower?   */
    ASSERT(ch->master->followers );

    if ( ch->master->followers->follower == ch) {   
	/* Head of follower-list? */
	j = ch->master->followers;
	ch->master->followers = j->next;
	free(j);
    }
    else {		/* locate follower who is not head of list */
	/* NOTE: check validity of k->next before comparing */
	for (k = ch->master->followers; k->next ; k = k->next )
	    if ( k->next->follower == ch) {
		j = k->next; 
		k->next = j->next;
		free(j);
		break;
	    }
    }
    /* NOTE: DEBUG: OOPS! ch is not in follower list of leader. */
    ASSERT(j) ;

    ch->master = 0;
    REMOVE_BIT(ch->specials.affected_by, AFF_GROUP);
} 
#endif		/* NO_DEF */

/* NOTE: Merged functionality of OLD stop_group() */
/* Called when stop following persons, or stopping charm */
/* This will NOT do if a character quits/dies!!          */
void stop_follower(struct char_data *ch)
{
    struct follow_type *j, *k;
    int check_leadership(struct char_data *leader);

    /* NOTE: Check master: This is for DEBUGGING */
    if (!ch->master) {
	log("No master but called stop_follower()");
	return;
    }
    /* assert(ch->master); */

    if (IS_AFFECTED(ch, AFF_CHARM)) {
	act("You realize that $N is a jerk!",
	    FALSE, ch, 0, ch->master, TO_CHAR);
	act("$n realizes that $N is a jerk!",
	    FALSE, ch, 0, ch->master, TO_NOTVICT);
	act("$n hates your guts!", FALSE, ch, 0, ch->master, TO_VICT);
	if (affected_by_spell(ch, SPELL_CHARM_PERSON))
	    affect_from_char(ch, SPELL_CHARM_PERSON);
    }
    else {
	act("You stop following $N.", FALSE, ch, 0, ch->master, TO_CHAR);
	act("$n stops following $N.", FALSE, ch, 0, ch->master, TO_NOTVICT);
	act("$n stops following you.", FALSE, ch, 0, ch->master, TO_VICT);
    } 

    j = NULL;

    /* NOTE: This is for DEBUGGING */
    if (!ch->master->followers || !ch->master->followers->follower ) {
	char buf[MAX_BUFSIZ];
	sprintf(buf, "DEBUG: stop_follower(): ch = %s ma = %s followers = %p",
	    GET_NAME(ch), GET_NAME(ch->master), ch->master->followers);
	return;
    }
    /* NOTE: DEBUG: my master has no follower?   */
    ASSERT(ch->master->followers );

    if (ch->master->followers->follower == ch) {  /* Head of follower-list? */
	j = ch->master->followers;
	ch->master->followers = j->next;
	free(j);
    }
    else {		/* locate follower who is not head of list */
	/* NOTE: check validity of k->next before comparing */
	for (k = ch->master->followers; k && k->next ; k = k->next)
	    if( k->next->follower == ch ) {
		j = k->next;
		k->next = j->next;
		free(j);
		break;
	    } 
    }
    /* NOTE: DEBUG: OOPS! ch is not in follower list of leader. */
    ASSERT(j) ;
	
    /* NOTE: BIG FIX:  assert failed on check_leadership()	*/
    /*   check leadership only when you are member of any group.*/ 
    /* NOTE: Call check_leadership() after removing follower */
    if (IS_AFFECTED( ch, AFF_CHARM | AFF_GROUP))
	/* NOTE: update leadership of your leader      */
	check_leadership(ch->master);
    REMOVE_BIT(ch->specials.affected_by, AFF_CHARM | AFF_GROUP);
    /* REMOVE_BIT(ch->specials.affected_by, AFF_GROUP); */

    ch->master = 0; 
}

/* Called when a character that follows/is followed dies */
void die_follower(struct char_data *ch)
{
    struct follow_type *j, *k;

    if (ch->master)
	stop_follower(ch);

    for (k = ch->followers; k; k = j) {
	j = k->next;
	stop_follower(k->follower);
    }
} 

/* Do NOT call this before having checked if a circle of followers */
/* will arise. CH will follow leader                               */
void add_follower(struct char_data *ch, struct char_data *leader)
{
    struct follow_type *k;

    assert(!ch->master);

    ch->master = leader;

    CREATE(k, struct follow_type, 1);

    k->follower = ch;
    k->next = leader->followers;
    leader->followers = k;

    act("You now follow $N.", FALSE, ch, 0, leader, TO_CHAR);
    act("$n starts following you.", TRUE, ch, 0, leader, TO_VICT);
    act("$n now follows $N.", TRUE, ch, 0, leader, TO_NOTVICT);
}


void do_order(struct char_data *ch, char *argument, int cmd)
{
    char name[100], message[MAX_LINE_LEN];
    char buf[MAX_OUT_LEN];
    bool found = FALSE;
    int org_room;
    struct char_data *victim;
    struct follow_type *k;

    half_chop(argument, name, message);

    if (!*name || !*message) {
	send_to_char("Order who to do what?\n\r", ch);
	return;
    }
    else if (!(victim = get_char_room_vis(ch, name)) &&
	     str_cmp("follower", name) && str_cmp("followers", name))
	send_to_char("That person isn't here.\n\r", ch);
    else if (ch == victim)
	send_to_char("You decline to do it.\n\r", ch);
    else {
	if (ch && IS_AFFECTED(ch, AFF_CHARM)) {
	    send_to_char("Your superior would not approve.\n\r", ch);
	    return;
	}
	if (victim) {
	    sprintf(buf, "$N orders you to '%s'", message);
	    act(buf, FALSE, victim, 0, ch, TO_CHAR);
	    act("$n gives $N an order.", FALSE, ch, 0, victim, TO_ROOM);
	    if ((victim->master != ch) 
		    || (victim && !IS_AFFECTED(victim, AFF_CHARM)))
		act("$n has an indifferent look.",
		    FALSE, victim, 0, 0, TO_ROOM);
	    else {
		send_to_char("Ok.\n\r", ch);
		if (!IS_NPC(victim))
		    WAIT_STATE(victim, PULSE_VIOLENCE);
		command_interpreter(victim, message);
	    }
	}
	else {		/* This is order "followers" */
	    sprintf(buf, "$n issues the order '%s'.", message);
	    act(buf, FALSE, ch, 0, victim, TO_ROOM);

	    org_room = ch->in_room;

	    for (k = ch->followers; k; k = k->next) {
		if (org_room == k->follower->in_room)
		    if (k->follower && IS_AFFECTED(k->follower, AFF_CHARM)) {
			found = TRUE;
			if (!IS_NPC(k->follower))
			    WAIT_STATE(k->follower, PULSE_VIOLENCE);
			command_interpreter(k->follower, message);
		    }
	    }
	    if (found)
		send_to_char("Ok.\n\r", ch);
	    else
		send_to_char(
		    "Nobody here are loyal subjects of yours!\n\r", ch);
	}
    }
} 

/* NOTE: NEW! Specify target for forthcoming violent action.  */
/*	'point' will set 'target' name to who is pointed to.
	If group leader points target, target of group members 
	in same room will be set, too.
	If pointed char cannot be found, fall back to old social action.   */  
void do_point(struct char_data *ch, char *argument, int cmd)
{
    char victim_name[100];
    struct char_data *victim = NULL;
    struct follow_type *f;
    extern void do_action(struct char_data *ch, char *argument, int cmd);

    one_argument(argument, victim_name);
    if ( *victim_name && 
	( victim = get_char_room_vis(ch, victim_name)) && ( ch != victim )) {

	act("You point at $N as target.", 0, ch, 0, victim, TO_CHAR );
	act("$n points YOU with menacing finger. Shudder...",
		FALSE, ch, 0, victim, TO_VICT);
	act("$n points $N as target for violence.",
		TRUE, ch, 0, victim, TO_ROOM);
	
	ch->specials.hunting = victim; 
	/*  NOTE: Skipped CAN_SEE(f->follower, victim) check   */
	if( ch->followers && IS_AFFECTED(ch, AFF_GROUP))
	    for( f = ch->followers ; f ; f = f->next ) 
		if( IS_AFFECTED(f->follower, AFF_GROUP) 
		    && (f->follower->in_room == ch->in_room))
			f->follower->specials.hunting = victim; 
    }
    else
	/* NOTE: Old behavior. harmless social action. 	*/
	do_action( ch, argument, cmd);
} 

void do_sneak(struct char_data *ch, char *argument, int cmd)
{
    struct affected_type af;
    byte percent;

    send_to_char("Ok, you'll try to move silently for a while.\n\r", ch);
    if (ch && IS_AFFECTED(ch, AFF_SNEAK))
	affect_from_char(ch, SKILL_SNEAK);
    percent = number(1, 101);	/* 101% is a complete failure */
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
    if (ch && IS_AFFECTED(ch, AFF_HIDE))
	REMOVE_BIT(ch->specials.affected_by, AFF_HIDE);
    percent = number(1, 101);	/* 101% is a complete failure */
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
	act("Oops..", FALSE, ch, 0, 0, TO_CHAR);
	act("$n tried to steal something from you!", FALSE, ch, 0, victim, TO_VICT);
	act("$n tries to rob $N.", TRUE, ch, 0, victim, TO_NOTVICT);
	return;
    }
    if (!IS_NPC(victim) && !(victim->desc)) {
	send_to_char("Steal something from the ghost? No way!\n\r", ch);
	return;
    }
    /* 101% is a complete failure */
    percent = number(1, 101) + 2 * GET_LEVEL(victim) - GET_LEVEL(ch)
	- dex_app_skill[GET_DEX(ch)].p_pocket
	- (GET_SKILLED(ch, SKILL_STEAL) >> 3);

    if (IS_WIZARD(victim) && HIGHER_LEV(victim, ch))
	percent = 101;
    if (str_cmp(obj_name, "coins") && str_cmp(obj_name, "gold")) {
	if (!(obj = get_obj_in_list_vis(victim, obj_name, victim->carrying))) {
	    for (eq_pos = 0; (eq_pos < MAX_WEAR); eq_pos++)
		if (victim->equipment[eq_pos] &&
		    (isname(obj_name, victim->equipment[eq_pos]->name)) &&
		    CAN_SEE_OBJ(ch, victim->equipment[eq_pos])) {
		    obj = victim->equipment[eq_pos];
		    break;
		}
	    if (!obj) {
		act("$E has not got that item.", FALSE, ch, 0, victim, TO_CHAR);
		return;
	    }
	    else {	/* It is equipment */
		send_to_char("Steal the equipment? Impossible!\n\r", ch);
		return;
	    }
	}
	else {		/* obj found in inventory */
	    percent += GET_OBJ_WEIGHT(obj);	/* Make heavy harder */
	    if (AWAKE(victim) && (percent > ch->skills[SKILL_STEAL].learned)) {
		ohoh = TRUE;
		act("Oops..", FALSE, ch, 0, 0, TO_CHAR);
		act("$n tries to rob $N.", TRUE, ch, 0, victim, TO_NOTVICT);
		if (!IS_NPC(victim))
		    act("$n tried to rob you.", TRUE, ch, 0, victim, TO_VICT);
	    }
	    else {	/* Steal the item */
		if ((GET_CARRYING_N(ch) + 1 < CAN_CARRY_N(ch))) {
		    if ((GET_CARRYING_W(ch) + GET_OBJ_WEIGHT(obj))
			< CAN_CARRY_W(ch)) {
			obj_from_char(obj);
			obj_to_char(obj, ch);
			send_to_char("Got it!\n\r", ch);
			INCREASE_SKILLED(ch, victim, SKILL_STEAL);
			if (!IS_NPC(victim))
			    act("$n robbed you!", TRUE, ch, 0, victim, TO_VICT);
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
    else {		/* Steal some coins */
	if (AWAKE(victim) && (percent > ch->skills[SKILL_STEAL].learned)) {
	    ohoh = TRUE;
	    act("Oops..", FALSE, ch, 0, 0, TO_CHAR);
	    act("$n tries to steal gold from $N.", TRUE, ch, 0, victim, TO_NOTVICT);
	    if (!IS_NPC(victim))
		act("$n tried to rob you.", TRUE, ch, 0, victim, TO_VICT);
	}
	else {
	    /* Steal some gold coins */
	    gold = (int) ((GET_GOLD(victim) * number(1, 25)) / 100);
	    if (gold > 0) {
		INCREASE_SKILLED(ch, victim, SKILL_STEAL);
		GET_GOLD(ch) += gold;
		GET_GOLD(victim) -= gold;
		sprintf(buf, "Bingo! You got %d gold coins.\n\r", gold);
		send_to_char(buf, ch);
		if (!IS_NPC(victim))
		    act("$n robbed you.", TRUE, ch, 0, victim, TO_VICT);
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

void do_flee(struct char_data *ch, char *argument, int cmd)
{
    int i, attempt, loose, die;
    struct char_data *tmp_victim, *temp;
    int level_dif; 

    /* when not fighting */
    if (!(ch->specials.fighting)) {
	for (i = 0; i < 6; i++) {
	    attempt = number(0, 5);	/* Select a random direction */
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
	}		/* for */
	/* No exits was found */
	send_to_char("PANIC! You couldn't escape!\n\r", ch);
	return;
    }

    if (GET_CLASS(ch) == CLASS_THIEF)
	i = 7;
    else
	i = 5;

    for (; i > 0; i--) {
	attempt = number(0, 5);		/* Select a random direction */
	if (CAN_GO(ch, attempt)) {
	    act("$n panics, and attempts to flee.", TRUE, ch, 0, 0, TO_ROOM);
	    if ((die = do_simple_move(ch, attempt, FALSE)) == 1) {
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
    }			/* for */

    /* No exits was found */
    send_to_char("PANIC! You couldn't escape!\n\r", ch);
} 

void do_rescue(struct char_data *ch, char *argument, int cmd)
{
    struct char_data *victim, *tmp_ch;
    int percent;
    char victim_name[240];
    extern void set_fighting(struct char_data *ch, struct char_data *victim); 

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
	send_to_char("How can you rescue your foe?\n\r", ch);
	return;
    }
    for (tmp_ch = world[ch->in_room].people; tmp_ch &&
     (tmp_ch->specials.fighting != victim); tmp_ch = tmp_ch->next_in_room) ;
    if (!tmp_ch) {
	act("But nobody is fighting $M?", FALSE, ch, 0, victim, TO_CHAR);
	return;
    }

    /* check same guild member, jhpark      */
    if (victim->specials.fighting)
	if ((!IS_NPC(victim->specials.fighting) && !IS_NPC(ch)) 
	    && (victim->specials.fighting->player.guild == ch->player.guild)) {
	    act("You cannot rescue $M!", FALSE, ch, 0, victim, TO_CHAR);
	    return;
	}

    percent = number(1, 111) - (GET_SKILLED(ch, SKILL_RESCUE) >> 3);
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

void do_light_move(struct char_data *ch, char *argument, int cmd)
{
    struct char_data *victim;

    WAIT_STATE(ch, PULSE_VIOLENCE / 3);
    if ((victim = ch->specials.fighting)) {
	if (ch->skills[SKILL_LIGHT_MOVE].learned > 10 && ch->points.mana > 0) {
	    hit(ch, victim, TYPE_UNDEFINED);
	    ch->points.mana -= LEVEL_LIMIT *2 - GET_LEVEL(ch) + 2;
	    if ((GET_LEVEL(ch) + 1) * number(2, 3) +
		ch->skills[SKILL_LIGHT_MOVE].learned / 2 >
		GET_LEVEL(victim) * number(2, 3) + number(30, 40)) {
		INCREASE_SKILLED(ch, victim, SKILL_LIGHT_MOVE);

		/* NOTE: Add lightning move skill message */
		act("$n quickly steps back and avoids $N's hit.",
		    TRUE, ch, 0, victim, TO_NOTVICT );
		act("$n quickly steps back and avoids your hit.",
		    TRUE, ch, 0, victim, TO_VICT );
		act("You step back and get out of $N's circle.",
		    TRUE, ch, 0, victim, TO_CHAR );

		if (ch->specials.fighting)
		    stop_fighting(ch);
		if (victim->specials.fighting)
		    stop_fighting(victim);
	    }
	    else {
		/* NOTE: Add fail message, too */
		send_to_char("Wheeing.... Uk!\r\n", ch);
	    }
	}
    }
    else {
	send_to_char("this skill can used only in fighting.\n\r", ch);
    }
}

/* by process */
/*
   ONLY for female
 */
void do_temptation(struct char_data *ch, char *argument, int cmd)
{
    struct char_data *victim;
    char name[100], message[256];
    int percent;

    half_chop(argument, name, message);
    if (IS_NPC(ch) || (!(GET_SEX(ch) == SEX_FEMALE) && IS_MORTAL(ch))) {
	send_to_char("You cannot seduce anyone as you are not female.\n\r", ch);
	return;
    }
    else if (!*name || !*message) {
	send_to_char("temptation who to do what????\n\r", ch);
	return;
    }
    else if (!(victim = get_char_room_vis(ch, name))) {
	send_to_char("That person isn't here.\n\r", ch);
    }
    else if (ch == victim) {
	send_to_char("Are you babo????\n\r", ch);
    }
    else if (victim) {
	percent = number(1, 151)
	    + GET_CON(ch) - GET_INT(ch) - GET_LEVEL(ch)
	    + GET_INT(victim) - GET_CON(victim) + GET_LEVEL(victim)
	    - (GET_SKILLED(ch, SKILL_TEMPTATION) >> 2);

	if ((ch->skills[SKILL_TEMPTATION].learned > percent
	     && GET_SEX(victim) == SEX_MALE && GET_LEVEL(victim) < 34)) {
	    /* 
	       GET_LEVEL(victim)<GET_LEVEL(ch))) { */
	    INCREASE_SKILLED(ch, victim, SKILL_TEMPTATION);
	    do_say(ch, "HOHOHOHOHOHO!!!!!!", 0);
	    if (!IS_NPC(ch))
		WAIT_STATE(ch, PULSE_VIOLENCE);
	    if (!IS_NPC(victim))
		WAIT_STATE(victim, PULSE_VIOLENCE);
	    act("$n seduces $N successfully!!!",
		FALSE, ch, 0, victim, TO_ROOM);
	    command_interpreter(victim, message);
	}
	else {
	    do_say(victim, "HAHAHAHAHAHA!!!!!", 0);
	    if (!IS_NPC(ch))
		WAIT_STATE(ch, 2 * PULSE_VIOLENCE);
	    act("$n failed to seduce $N!!!!",
		FALSE, ch, 0, victim, TO_ROOM);
	    command_interpreter(ch, message);
	    hit(victim, ch, TYPE_UNDEFINED);
	    first_attack(victim, ch);
	}

	if (GET_MANA(ch) > 200)
	    GET_MANA(ch) -= 200;
    }
} 

/* 
   Chase written 
   Description: Disarms other player's wielding weapon
 */
void do_disarm(struct char_data *ch, char *argument, int cmd)
{
    struct char_data *victim;
    struct obj_data *obj;
    char victim_name[240];
    int percent;
    extern int nodisarmflag;

    one_argument(argument, victim_name);

    if (!(victim = get_char_room_vis(ch, victim_name))) {
	send_to_char("Disarm who?\n\r", ch);
	return;
    }
    else if (victim == ch) {
	send_to_char("You stupid! How about using remove instead of disarm?\n\r", ch);
	return;
    }
    if (IS_WIZARD(victim) && HIGHER_LEV(victim, ch))
	return;
    if ((!IS_NPC(victim)) && (nodisarmflag)) {
	act("Oops..", FALSE, ch, 0, 0, TO_CHAR);
	act("$n tried to disarm your weapon!", FALSE, ch, 0, victim, TO_VICT);
	act("$n tries to disarm $N.", TRUE, ch, 0, victim, TO_NOTVICT);
	return;
    }
    if (!IS_NPC(victim) && !(victim->desc)) {
	send_to_char("Disarm the ghost? No way!\n\r", ch);
	return;
    }
    percent = number(1, 100) - GET_LEVEL(ch) + 2 * GET_LEVEL(victim)
	- dex_app_skill[GET_DEX(ch)].p_pocket
	- (GET_SKILLED(ch, SKILL_DISARM) >> 3);
    if (percent < ch->skills[SKILL_DISARM].learned) {
	obj = victim->equipment[WIELD];
	if (!obj && GET_CLASS(victim) == CLASS_WARRIOR) {
	    obj = victim->equipment[HOLD];
	}
	if (obj && CAN_SEE_OBJ(ch, obj)) {
	    INCREASE_SKILLED(ch, victim, SKILL_DISARM);
	    if (victim->equipment[WIELD])
		obj_to_char(unequip_char(victim, WIELD), victim);
	    if (victim->equipment[HOLD])
		obj_to_char(unequip_char(victim, HOLD), victim);
	    act("$n disarmed you!", TRUE, ch, 0, victim, TO_VICT);
	    act("$n disarmed $N!", TRUE, ch, 0, victim, TO_NOTVICT);
	    act("Your disarming successful!", FALSE, ch, 0, 0, TO_CHAR);
	}
	else {
	    act("$E is not wielding any weapon now.", FALSE, ch, 0, victim, TO_CHAR);
	    return;
	}
    }
    else {
	act("$n failed to disarm your weapon!", FALSE, ch, 0, victim, TO_VICT);
	act("$n fails to disarm $N.", TRUE, ch, 0, victim, TO_NOTVICT);
	if (AWAKE(victim) && GET_LEVEL(ch) < GET_LEVEL(victim))
	    hit(victim, ch, TYPE_UNDEFINED);
    }
}

void do_consider(struct char_data *ch, char *argument, int cmd)
{
    struct char_data *victim;
    char name[MAX_INPUT_LENGTH];
    int diff;

    one_argument(argument, name);

    if (!(victim = get_char_room_vis(ch, name))) {
	send_to_char("Consider killing who?\n\r", ch);
	return;
    } 
    if (victim == ch) {
	send_to_char("Easy! Very easy indeed!\n\r", ch);
	return;
    }
    /* NOTE: Now, you can 'consider' PC player *evil grin*  */
    /*
    if (!IS_NPC(victim)) {
	send_to_char("Would you like to borrow a cross and a shovel?\n\r", ch);
	return;
    }
    */

    diff = (GET_LEVEL(victim) - GET_LEVEL(ch));

    if (diff <= -10)
	send_to_char("Now where did that chicken go?\n\r", ch);
    else if (diff <= -5)
	send_to_char("You could do it with a needle!\n\r", ch);
    else if (diff <= -2)
	send_to_char("Easy.\n\r", ch);
    else if (diff <= -1)
	send_to_char("Fairly easy.\n\r", ch);
    else if (diff == 0)
	send_to_char("The perfect match!\n\r", ch);
    else if (diff <= 1)
	send_to_char("You would need some luck!\n\r", ch);
    else if (diff <= 2)
	send_to_char("You would need a lot of luck!\n\r", ch);
    else if (diff <= 3)
	send_to_char("You would need a lot of luck and great equipment!\n\r", ch);
    else if (diff <= 5)
	send_to_char("Do you feel lucky, punk?\n\r", ch);
    else if (diff <= 10)
	send_to_char("Are you mad!?\n\r", ch);
    else if ( IS_NPC(victim) || diff < 15 )
	send_to_char("You ARE mad!\n\r", ch);
    else { 
	/* NOTE: Victim is PC and diff >= 15, dig your grave...  */
	send_to_char("Would you like to borrow a cross and a shovel?\n\r", ch);
	act("$n asks you to borrow a cross and a shovel.",
	    TRUE, ch, 0, victim, TO_VICT);
	act("$n asks $N to borrow a cross and a shovel.",
	    TRUE, ch, 0, victim, TO_NOTVICT);
    }
} 

