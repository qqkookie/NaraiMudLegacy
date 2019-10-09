/* **************************************************************************
   *  file: guild.c , Special module. 				    *
   *  Usage: Procedures handling guild skill, command, room *
   *  Made by Shin Won-dong  in KAIST					    *
   *									    *
   *  NOTE: Merged guild_commad.c, guild_constant.c and guild_informative.c *
   *		and guild_skill.c  guild_list.h				    *
   ************************************************************************ */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "char.h"
#include "object.h"
#include "global.h"
#include "comm.h"
#include "play.h"
#include "actions.h"
#include "etc.h"

/* ********************************************************************* */
/*  NOTE:    From file: OLD guild_constant.c			  	 */
/* ********************************************************************* */

char *guild_names[] =
{
    "NONE",
    "POLICE",
    "OUTLAW",
    "ASSASSIN"
};

/*
   useless guild skills
 */
int guild_skill_nums[] =
{
    0,
    0,
    0
};

/*
   int guild_skill_nums[] =
   {
   4,
   5,
   4
   };
 */

char *police_skills[] =
{
    "broadcast",
    "simultaneous",
    "arrest",
    "power bash"
};

int police_skill_costs[] =
{
    5000,
    10000,
    10000,
    20000,
};

char *outlaw_skills[] =
{
    "angry yell",
    "assault",
    "smoke",
    "inject",
    "charge"
};

int outlaw_skill_costs[] =
{
    10000,
    12000,
    1000,
    2000,
    50000
};

char *assasin_skills[] =
{
    "solace",
    "shadow figure",
    "evil strike",
    "improved reflect damage"
};

int assasin_skill_costs[] =
{
    12000,
    15000,
    20000,
    100000
};

/* ********************************************************************* */
/*  file: OLD guild_command.c , Special guild roomodule.	         */
/* ********************************************************************* */

/* NOTE: *******   taxi() and related code moved to spec_mid.c  ********/

/* always set exit NORTH to guild from entry */
int guild_entry(struct char_data *ch, int cmd, char *arg)
{
    char buf[MAX_STRING_LENGTH], buf2[MAX_STRING_LENGTH];
    int i, room_number;
    int guild_number;

    if (IS_NPC(ch))
	return (FALSE);

    /* guild_number = */
    room_number = world[ch->in_room].number;

    switch (room_number) {
    case ROOM_GUILD_POLICE:
	guild_number = 1;
	break;
    case ROOM_GUILD_OUTLAW:
	guild_number = 2;
	break;
    case ROOM_GUILD_ASSASSIN:
	guild_number = 3;
	break;
    default:
	return FALSE;
    }
    if (cmd == CMD_JOIN) {	/* JOIN */
	/* find out room-number and figure out guild number which the
	   character want to join */
	switch (guild_number) {
	case POLICE:
	    if ((GET_LEVEL(ch) < 15) && IS_MORTAL(ch)) {
		send_to_char("당신은 경찰이 될만한 자격이 없군요.\n\r", ch);
		return TRUE;
	    }
	    break;
	case OUTLAW:
	    if ((GET_LEVEL(ch) < 20) && IS_MORTAL(ch)) {
		send_to_char("당신은 깡패가 될만한 자격이 없군요.\n\r", ch);
		return TRUE;
	    }
	    break;
	case ASSASSIN:
	    if ((GET_LEVEL(ch) < 25) && IS_MORTAL(ch)) {
		send_to_char("당신은 암살자가 될만한 자격이 없군요.\n\r", ch);
		return TRUE;
	    }
	    break;
	default:
	    return TRUE;
	}

	if (guild_number == ch->player.guild) {
	    send_to_char("You are joining that guild already!!!", ch);
	    return TRUE;
	}
	if (ch->player.guild != 0) {
	    send_to_char("You must leave your guild first!!!", ch);
	    return TRUE;
	}
	ch->player.guild = guild_number;
	sprintf(buf, "%s JOINED %s guild", GET_NAME(ch), guild_names[guild_number]);
	/* NOTE: Don't send CR-LF to log()   */
	log(buf);
	/* NOTE: But tty need CR-LF   */
	strcat(buf, "\r\n");
	send_to_all(buf);
	for (i = 0; i < MAX_GUILD_SKILLS; i++) {
	    ch->player.guild_skills[i] = 0;
	}
	return TRUE;
    }
    else if (cmd == CMD_LEAVE) {	/* LEAVE */
	if (guild_number != ch->player.guild) {
	    send_to_char("You can't leave this guild as you are not a member!!", ch);
	    return TRUE;
	}
	/* NOTE: Don't send CR-LF to log()   */
	sprintf(buf, "%s LEFT %s guild", GET_NAME(ch), guild_names[ch->player.guild]);
	log(buf);
	/* NOTE: But tty need CR-LF   */
	strcat(buf, "\r\n");
	send_to_all(buf);
	ch->player.guild = 0;
	for (i = 0; i < MAX_GUILD_SKILLS; i++) {
	    ch->player.guild_skills[i] = 0;
	}
	return TRUE;
    }
    else if (cmd == CMD_NORTH) {	/* north */
	if ((guild_number != ch->player.guild) && IS_MORTAL(ch)) {
	    sprintf(buf, "The guild guard humiliates you,and block your way.\n\r");
	    sprintf(buf2, "The guard humilates $n,and blocks $s way.");
	    send_to_char(buf, ch);
	    act(buf2, FALSE, ch, 0, 0, TO_ROOM);
	    return TRUE;
	}
    }
    else if (cmd == CMD_CAST) {	/* to prevent cast 'phase' */
	if ((guild_number != ch->player.guild) && IS_MORTAL(ch)) {
	    sprintf(buf, "The guild guard screams, SHUT UP~~!!\n\r");
	    send_to_char(buf, ch);
	    return TRUE;
	}
    }

    return (FALSE);
}

int locker_room(struct char_data *ch, int cmd, char *arg)
{
    char buf[MAX_STRING_LENGTH];
    int i, room_number;
    char guild_number;
    static int count = 0;
    extern void unstash_char(struct char_data *ch, char *stashname);
    extern void stash_char(struct char_data *ch);

#define LIMIT_LOAD 10
    static char load_names[LIMIT_LOAD + 1][30];
    int equiped_something = 0;

    void extract_carrying_obj(struct obj_data *p);

    if (IS_NPC(ch)) {
	return (FALSE);
    }
    room_number = world[ch->in_room].number;
    switch (room_number) {
    case 3000:
	guild_number = 0;
	break;
    case ROOM_GUILD_POLICE_LOCKER:
	guild_number = 1;
	break;
    case ROOM_GUILD_OUTLAW_LOCKER:
	guild_number = 2;
	break;
    case ROOM_GUILD_ASSASSIN_LOCKER:
	guild_number = 3;
	break;
    default:
	return FALSE;
    }

    if (guild_number < 0 || guild_number > MAX_GUILD_LIST) {
	return FALSE;
    }
    if (guild_number != ch->player.guild && IS_MORTAL(ch)) {
	send_to_char("But you are not the member of this guild!", ch);
	return FALSE;
    }
    if (cmd == CMD_SAVE) {	/* SAVE */
	return FALSE;
#define COST	10000
	if (GET_GOLD(ch) < GET_LEVEL(ch) * GET_LEVEL(ch) * COST) {
	    sprintf(buf, "You need %d coins to SAVE your items.",
		    GET_LEVEL(ch) * GET_LEVEL(ch) * COST);
	    send_to_char(buf, ch);
	    return TRUE;
	}

	/* only allow to save equiped item */
	if (ch->carrying) {
	    send_to_char("You can SAVE only equiped item.\n\r", ch);
	    send_to_char("Drop all carrying item!\n\r", ch);
	    return TRUE;
	}
	/* only allow to save container which DOSEN'T contain something */
	for (i = 0; i < MAX_WEAR; i++) {
	    if (ch->equipment[i]) {
		equiped_something = 1;
		if (ch->equipment[i]->contains) {
		    send_to_char("You can't SAVE container which ", ch);
		    send_to_char("contains something.\n\r", ch);
		    return TRUE;
		}
	    }
	}
	/* But, need not to save! what a waste of coins! */
	if (!equiped_something) {
	    send_to_char("But, you don't have anything to save!\n\r", ch);
	    return TRUE;
	}

	GET_GOLD(ch) = GET_GOLD(ch) - (GET_LEVEL(ch) * GET_LEVEL(ch) * COST);
	stash_char(ch);
	send_to_char("Ok. When you want to LOAD items, type LOAD.\n\r", ch);
	return TRUE;
    }
    else if (cmd == CMD_LOAD ) {	/* LOAD */
	/* only allow 1 load per reboot */
	for (i = 0; i < count; i++) {
	    if (strcmp(load_names[i], GET_NAME(ch)) == 0) {
		send_to_char("You already used ONE chance of LOAD!!\n\r", ch);
		return TRUE;
	    }
	}
	/* limit number of LOAD in guild */
	if (count >= LIMIT_LOAD) {
	    send_to_char("All Lockers already used by other persons!\n\r", ch);
	    send_to_char("The following persons used locker!!\n\r", ch);
	    for (i = 0; i < LIMIT_LOAD; i++) {
		sprintf(buf, "%s\n\r", load_names[i]);
		send_to_char(buf, ch);
	    }
	    return TRUE;
	}

	strcpy(load_names[i], GET_NAME(ch));

	if (ch->carrying) {
	    send_to_char("You must carry nothing to LOAD!!\n\r", ch);
	    return TRUE;
	}
	for (i = 0; i < MAX_WEAR; i++) {
	    if (ch->equipment[i]) {
		send_to_char("You must equip nothing to LOAD!!\n\r", ch);
		return TRUE;
	    }
	}
	count++;
	unstash_char(ch, NULL);
	send_to_char("Ok. Your items restored...\n\r", ch);
	return TRUE;
    }
    return FALSE;
}

int guild_practice_yard(struct char_data *ch, int cmd, char *arg)
{
    char buf[MAX_STRING_LENGTH];
    int i, room_number;
    char guild_number;
    int number;
    int cost = 0;

    if (IS_NPC(ch))
	return FALSE;
    room_number = world[ch->in_room].number;
    switch (room_number) {
    case ROOM_GUILD_POLICE_PRACTICE:
	guild_number = 1;
	break;
    case ROOM_GUILD_OUTLAW_PRACTICE:
	guild_number = 2;
	break;
    case ROOM_GUILD_ASSASSIN_PRACTICE:
	guild_number = 3;
	break;
    default:
	return FALSE;
    }
    if (guild_number != GET_GUILD(ch) && IS_MORTAL(ch)) {
	send_to_char("But you are not a member of this guild.\n\r", ch);
	return FALSE;
    }
    if (cmd == CMD_TRAIN ) {	/* train */
	number = atoi(arg) - 1;
	if (number >= guild_skill_nums[guild_number - 1] ||
	    number < 0) {
	    send_to_char("There's no such skill\n\r", ch);
	    return TRUE;
	}
	if ((GET_GUILD_SKILL(ch, number)) >= 90) {
	    send_to_char("You trained enough!!!\n\r", ch);
	    return TRUE;
	}
	switch (guild_number) {
	case POLICE:
	    cost = police_skill_costs[number] * ((GET_GUILD_SKILL(ch, number) / 10) + 1) * GET_LEVEL(ch);
	    sprintf(buf, "You train %s.\n\r", police_skills[number]);
	    send_to_char(buf, ch);
	    break;
	case OUTLAW:
	    cost = outlaw_skill_costs[number] * ((GET_GUILD_SKILL(ch, number) / 10) + 1) * GET_LEVEL(ch);
	    sprintf(buf, "You train %s.\n\r", outlaw_skills[number]);
	    send_to_char(buf, ch);
	    break;
	case ASSASSIN:
	    cost = assasin_skill_costs[number] * ((GET_GUILD_SKILL(ch, number) / 10) + 1) * GET_LEVEL(ch);
	    sprintf(buf, "You train %s.\n\r", assasin_skills[number]);
	    send_to_char(buf, ch);
	    break;
	default:
	    log("shit in train");
	    break;
	}
	if (GET_EXP(ch) > cost) {
	    GET_EXP(ch) = GET_EXP(ch) - cost;
	    GET_GUILD_SKILL(ch, number) = MIN(GET_GUILD_SKILL(ch, number) + 10, 90);
	}
	else {
	    sprintf(buf, "You need %d exp.\n\r", cost);
	    send_to_char(buf, ch);
	}
	return TRUE;
    }
    else if (cmd == CMD_LIST) {	/* list */
	if (guild_skill_nums[guild_number - 1] < 1) {
	    send_to_char("There's no skill now\n\r", ch);
	}
	send_to_char("You can train skills by typing 'train #'\n\r", ch);
	switch (guild_number) {
	case POLICE:
	    for (i = 0; i < guild_skill_nums[guild_number - 1]; i++) {
	/* NOTE: '%' char in sprintf format string should be noted as %%. */
		sprintf(buf, "%d: %s %d(now %d%%)\n\r", i + 1, police_skills[i],
		 police_skill_costs[i] * ((GET_GUILD_SKILL(ch, i) / 10) + 1)
			* GET_LEVEL(ch),
			GET_GUILD_SKILL(ch, i));
		send_to_char(buf, ch);
	    }
	    break;
	case OUTLAW:
	    for (i = 0; i < guild_skill_nums[guild_number - 1]; i++) {
		sprintf(buf, "%d: %s %d(now %d%%)\n\r", i + 1, outlaw_skills[i],
		 outlaw_skill_costs[i] * ((GET_GUILD_SKILL(ch, i) / 10) + 1)
			* GET_LEVEL(ch),
			GET_GUILD_SKILL(ch, i));
		send_to_char(buf, ch);
	    }
	    break;
	case ASSASSIN:
	    for (i = 0; i < guild_skill_nums[guild_number - 1]; i++) {
		sprintf(buf, "%d: %s %d(now %d%%)\n\r", i + 1, assasin_skills[i],
		assasin_skill_costs[i] * ((GET_GUILD_SKILL(ch, i) / 10) + 1)
			* GET_LEVEL(ch),
			GET_GUILD_SKILL(ch, i));
		send_to_char(buf, ch);
	    }
	    break;
	}
	return TRUE;
    }
    return FALSE;
}


/* ***********************************************************************
   *  NOTE:  old file: guild_informative.c , Implementation of commands. *
   *  Usage : guild Informative commands.    made by Process		 *
   *********************************************************************** */

void do_cant(struct char_data *ch, char *argument, int cmd)
{

    struct descriptor_data *i;
    struct char_data *victim;
    char buf[100];

    int j;

    if (IS_NPC(ch))
	return;
    if (IS_SET(ch->specials.act, PLR_DUMB_BY_WIZ) && NOT_GOD(ch)) {
	return;
    }
    if (ch->player.guild < 1 || ch->player.guild > MAX_GUILD_LIST) {
	send_to_char("You must join any guild first.\n\r", ch);
	return;
    }
    /* NOTE: "cant" command without argument is same as "cant /who"   */
    if (strcmp(argument, " /who") == 0 || argument[0] == 0) {
	j = 0;
	for (i = descriptor_list; i; i = i->next)
	    if (!i->connected) {
		if (i->original)
		    continue;
		victim = i->character;
		if (ch->player.guild == victim->player.guild 
			&& IS_MORTAL(victim)) {
		    j++;
		    sprintf(buf, "<%2d> %s <%5d,%5d,%5d> %s \n\r", GET_LEVEL(victim),
			    GET_NAME(victim), GET_PLAYER_MAX_HIT(victim),
		    GET_PLAYER_MAX_MANA(victim), GET_PLAYER_MAX_MOVE(victim),
			    world[victim->in_room].name
			);
		    send_to_char(buf, ch);
		}
	    }
	sprintf(buf, "You can see %d member(s) of %s guild.\n\r",
		j, guild_names[ch->player.guild]);
	send_to_char(buf, ch);
    }
    else {
	sprintf(buf, "(%s) %s >>> %s\n\r", guild_names[ch->player.guild],
		ch->player.name, argument);
	for (i = descriptor_list; i; i = i->next)
	    if (!i->connected) {
		if (i->original)
		    continue;
		victim = i->character;
		if (ch->player.guild == victim->player.guild || IS_DIVINE(victim)) {
		    send_to_char(buf, victim);
		}
	    }
    }
}

void do_query(struct char_data *ch, char *argument, int cmd)
{
    struct char_data *victim;
    char victim_name[240];
    char buf[MAX_STRING_LENGTH];

    one_argument(argument, victim_name);

    if (!(victim = get_char_room_vis(ch, victim_name))) {
	send_to_char("QUERY who?\n\r", ch);
	return;
    }
    else if (victim == ch) {
	send_to_char("The better idea is to type \"sc\".\n\r", ch);
	return;
    }
    if ( victim->player.guild <= MAX_GUILD_LIST) {
	sprintf(buf, "%s(%d) is a member of %s guild\n\r", GET_NAME(victim), GET_LEVEL(victim),
		guild_names[victim->player.guild]);
	send_to_char(buf, ch);
    }

    /* modified by ares */
    if (GET_LEVEL(ch) >= GET_LEVEL(victim))
	sprintf(buf, "%s's hit is %d.\n\r", GET_NAME(victim),
		GET_HIT(victim));
    else if (GET_HIT(victim) > GET_HIT(ch))
	sprintf(buf, "%s's hit is higher than you.\n\r", GET_NAME(victim));
    else
	sprintf(buf, "%s's hit is lower than you.\n\r", GET_NAME(victim));
    send_to_char(buf, ch);

    if (GET_LEVEL(ch) >= GET_LEVEL(victim))
	sprintf(buf, "%s's mana is %d.\n\r", GET_NAME(victim),
		GET_MANA(victim));
    else if (GET_MANA(victim) > GET_MANA(ch))
	sprintf(buf, "%s's mana is higher than you.\n\r", GET_NAME(victim));
    else
	sprintf(buf, "%s's mana is lower than you.\n\r", GET_NAME(victim));
    send_to_char(buf, ch);

    if (GET_LEVEL(ch) >= GET_LEVEL(victim))
	sprintf(buf, "%s's move is %d.\n\r", GET_NAME(victim),
		GET_MOVE(victim));
    else if (GET_MOVE(victim) > GET_MOVE(ch))
	sprintf(buf, "%s's move is higher than you.\n\r", GET_NAME(victim));
    else
	sprintf(buf, "%s's move is lower than you.\n\r", GET_NAME(victim));
    send_to_char(buf, ch);

    if (GET_LEVEL(ch) >= GET_LEVEL(victim))
	sprintf(buf, "%s's hitroll is %d.\n\r", GET_NAME(victim),
		GET_HITROLL(victim));
    else if (GET_HITROLL(victim) > GET_HITROLL(ch))
	sprintf(buf, "%s's hitroll is higher than you.\n\r", GET_NAME(victim));
    else
	sprintf(buf, "%s's hitroll is lower than you.\n\r", GET_NAME(victim));
    send_to_char(buf, ch);

    if (GET_LEVEL(ch) >= GET_LEVEL(victim))
	sprintf(buf, "%s's damroll is %d.\n\r", GET_NAME(victim),
		GET_DAMROLL(victim));
    else if (GET_DAMROLL(victim) > GET_DAMROLL(ch))
	sprintf(buf, "%s's damroll is higher than you.\n\r", GET_NAME(victim));
    else
	sprintf(buf, "%s's damroll is lower than you.\n\r", GET_NAME(victim));
    send_to_char(buf, ch);

    act("$n QUERYS $N.", TRUE, ch, 0, victim, TO_ROOM);
    GET_MANA(ch) -= 10;
}

/*--------------------------------------------------------
   GUILD_SKILLS.C made by wdshin@eve.kaist.ac.kr
 *-------------------------------------------------------- */

/* start of police */
#define POLICE_SKILL_WHISTLE		0
#define POLICE_SKILL_SIMULTANEOUS	1
#define POLICE_SKILL_ARREST		2
#define POLICE_SKILL_POWER_BASH		3
/* start of outlaw */
#define OUTLAW_SKILL_ANGRYYELL		0
#define OUTLAW_SKILL_ASSAULT    	1
#define OUTLAW_SKILL_SMOKE		2
#define OUTLAW_SKILL_INJECT		3
#define OUTLAW_SKILL_CHARGE		4
/* start of assasin */
#define ASSASSIN_SKILL_SOLACE		0
#define ASSASSIN_SKILL_SHADOW_FIGURE	1
#define ASSASSIN_SKILL_EVIL_STRIKE 	2
#define ASSASSIN_SKILL_IMPROVED_REFLECT_DAMAGE 3

/* skills of police */
void do_power_bash(struct char_data *ch, char *argument, int cmd)
{
    char name[256];
    struct char_data *victim;
    int percent;

    /* remove guild skills by atre */
    return;

    if (GET_GUILD(ch) != POLICE && IS_MORTAL(ch)) {
	return;
    }

    one_argument(argument, name);
    if (!(victim = get_char_room_vis(ch, name))) {
	if (ch->specials.fighting) {
	    victim = ch->specials.fighting;
	}
	else {
	    send_to_char("power-bash who?\n\r", ch);
	    return;
	}
    }
    if (victim == ch || IS_NPC(victim)) {
	do_say(ch, "HAHAHA...I am babo!!!", 0);
	return;
    }
    percent = number(1, 130);
    if (percent < GET_GUILD_SKILL(ch, POLICE_SKILL_POWER_BASH)) {
	if (GET_MANA(ch) < 100) {
	    send_to_char("You do not have enough mana!", ch);
	    return;
	}
	damage(ch, victim, GET_GUILD_SKILL(ch, POLICE_SKILL_POWER_BASH), SKILL_BASH);
	if (percent < GET_GUILD_SKILL(ch, POLICE_SKILL_POWER_BASH))
	    GET_POS(victim) = POS_STUNNED;
	if (victim)
	    WAIT_STATE(victim,
		       (GET_GUILD_SKILL(ch, POLICE_SKILL_POWER_BASH) / 20) * PULSE_VIOLENCE);
    }
    else {
	GET_MOVE(ch) -= 1000;
	if (ch)
	    WAIT_STATE(ch, PULSE_VIOLENCE * 2);
	return;
    }
    if (ch)
	WAIT_STATE(ch, (GET_GUILD_SKILL(ch, POLICE_SKILL_POWER_BASH) / 20) * PULSE_VIOLENCE);
}

/* victim의 장소를 broadcast하고 flag을 붙인다. */
/* flag이 붙었을때 죽으면 flag이 없어진다 */
void do_whistle(struct char_data *ch, char *argument, int cmd)
{
    char name[256];
    char buf[256];
    struct char_data *victim;
    struct descriptor_data *d;

    /* remove guild skills by atre */
    return;

    if (GET_GUILD(ch) != POLICE && IS_MORTAL(ch)) {
	return;
    }
    one_argument(argument, name);

    if (!(victim = get_char_room_vis(ch, name))) {
	if (ch->specials.fighting) {
	    victim = ch->specials.fighting;
	}
	else {
	    send_to_char("whistle who?\n\r", ch);
	    return;
	}
    }

    if (IS_NPC(victim) || victim == ch)
	return;

    sprintf(buf, "%s>>>>Emergency!!! %s(%s) is in %s(%d)!!!!!\n\r", 
	    GET_NAME(ch), GET_NAME(victim), guild_names[ GET_GUILD(victim)],
	    world[victim->in_room].name, world[victim->in_room].number);
    for (d = descriptor_list; d; d = d->next) {
	if (d->character && (d->connected == CON_PLYNG) &&
	    (d->character->in_room != NOWHERE) &&
	    d->character->player.guild == POLICE) {
	    send_to_char(buf, d->character);
	    GET_MANA(ch) -= 10;
	}
    }
    if (!IS_SET(victim->specials.act, PLR_CRIMINAL) && GET_MANA(ch) > 1000) {
	SET_BIT(victim->specials.act, PLR_CRIMINAL);
	send_to_char("flag set!!\n\r", ch);
	GET_MANA(ch) -= 1000;
    }
}

/* 방안에 있는 경찰들이 한대씩 전부 때림 */
void do_simultaneous(struct char_data *ch, char *argument, int cmd)
{
    char name[256];
    struct char_data *victim, *tmp_victim, *vict;
    int dam = 0;

    /* remove guild skills by atre */
    return;

    one_argument(argument, name);
    if (GET_GUILD(ch) != POLICE && IS_MORTAL(ch)) {
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
    if (victim == ch)
	return;
    for (vict = world[ch->in_room].people; vict; vict = tmp_victim) {
	tmp_victim = vict->next_in_room;
	if ((vict && !IS_NPC(vict) && GET_GUILD(vict) == POLICE 
	    && GET_GUILD_SKILL(vict, POLICE_SKILL_SIMULTANEOUS) > number(1, 99)
	    && GET_MANA(vict) > 180) 
	    || (IS_WIZARD(vict))) {
	    dam = GET_INT(vict) * GET_GUILD_SKILL(vict, POLICE_SKILL_SIMULTANEOUS);
	    if (victim)
		damage(vict, victim, dam, TYPE_HIT);
	    do_say(vict, "Here I go!!!", 0);
	    if (victim)
		act("$n found $N's a weak point!!",
		    TRUE, vict, 0, victim, TO_ROOM);
	    GET_MANA(vict) -= 20 * GET_INT(victim);
	    if (vict)
		WAIT_STATE(vict, PULSE_VIOLENCE / 3);
	    if (victim)
		WAIT_STATE(victim, PULSE_VIOLENCE / 3);
	}
    }
}

void do_arrest(struct char_data *ch, char *argument, int cmd)
{
    char name[256];
    struct char_data *victim;
    struct affected_type af;
    char buf[256];
    extern void change_alignment(struct char_data *ch, struct char_data *vic);
    extern void group_gain(struct char_data *ch, struct char_data *vic);

    /* remove guild skills by atre */
    return;

    one_argument(argument, name);
    if (GET_GUILD(ch) != POLICE && IS_MORTAL(ch))
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
    /* NOTE: JALE_ROOM: spell correction and renamed POLICE_JAIL_ROOM   */
    /* NOTE: defintion moved to  "../include/guild_list.h"              */
/* #define JALE_ROOM    1800    */
    if (victim == ch)
	return;
    if ((GET_GUILD_SKILL(ch, POLICE_SKILL_ARREST) < number(1, 99) || 
	  (GET_HIT(victim) >= GET_PLAYER_MAX_HIT(victim) / 8) 
	  || GET_MANA(ch) < 500) && IS_MORTAL(ch)) {
	GET_MANA(ch) -= 100;
	send_to_char("You failed to arrest him!!!\n\r", ch);
	return;
    }
    do_say(ch, "YOU MUST SUFFER IN THE JAIL!!!!", 0);
    act("$n disappears suddenly to JAIL!!!!!!",
	TRUE, victim, 0, 0, TO_ROOM);
    char_from_room(victim);
    char_to_room(victim, real_room(ROOM_POLICE_JAIL));
    if (!IS_NPC(victim)) {
	sprintf(buf, "%s was sent to the JAIL room!!!!\n\r", GET_NAME(victim));
	send_to_all(buf);
	af.type = SKILL_ARREST;
	af.duration = 44;
	af.modifier = 0;
	af.location = APPLY_NONE;
	af.bitvector = AFF_ARREST;
	affect_to_char(victim, &af);
	/* NOTE: victim looks jail room.  */
	do_look(victim, "", 15);
    }
    else {
#ifdef NO_DEF
	sprintf(buf, "%s died in the JAIL room!!!!\n\r", GET_NAME(victim));
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
	die(victim, GET_LEVEL(ch), ch);
#endif		/* NO_DEF*/
	sprintf(buf, "%s died in the JAIL room!!!!\n\r", GET_NAME(victim));
	send_to_room(buf, ch->in_room);;
	/* NOTE: indirect way to kill victim and get exp from it. */
	GET_HIT(victim) = 0; 
	GET_POS(victim) = POS_MORTALLYW;
	/* NOTE: TYPE_SUFFERING: char and victim is not in same room */
	damage(ch, victim, 1000, TYPE_SUFFERING);
    }
    /* NOTE: Why look here? */ 
    /*  do_look(ch, "", 15); */
    GET_HIT(ch) -=  GET_HIT(ch)/10;
}

/* skills of outlaws */
void do_charge(struct char_data *ch, char *argument, int cmd)
{
    char name[256];
    struct char_data *victim;
    int dam;
    int use;
    int level_dif;

    /* remove guild skills by atre */
    return;

    if ((GET_GUILD(ch) != OUTLAW && IS_MORTAL(ch)) || IS_NPC(ch))
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

    if (!IS_NPC(victim) && PC_MORTAL(ch)) {
	send_to_char("You can't use charge to player\n\r", ch);
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
	damage(ch, victim, dam, TYPE_MISC);
	use = 5000;
	use = number(use >> 1, use);
	GET_MANA(ch) -= use;
	GET_MOVE(ch) -= use;
	WAIT_STATE(ch, 2 * PULSE_VIOLENCE);
    }
    else {
	dam = GET_MOVE(victim);
	GET_POS(ch) = POS_STUNNED;
	do_say(ch, "JJUP!!!!", 0);
	do_say(victim, "HAHAHA!!!", 0);
	act("$n failed to CHARGE $N!!!", TRUE, ch, 0, victim, TO_ROOM);
	GET_MANA(ch) -= 100;
	damage(victim, ch, dam, TYPE_MISC);
	WAIT_STATE(ch, PULSE_VIOLENCE);
    }
}

/* just damage */
void do_angry_yell(struct char_data *ch, char *argument, int cmd)
{
    char name[256];
    struct char_data *victim;
    int dam;

    /* remove guild skills by atre */
    return;

    one_argument(argument, name);

    if ((GET_GUILD(ch) != OUTLAW && IS_MORTAL(ch)) || IS_NPC(ch))
	return;
    if (!(victim = get_char_room_vis(ch, name))) {
	if (ch->specials.fighting) {
	    victim = ch->specials.fighting;
	}
	else {
	    send_to_char("yell at who?\n\r", ch);
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
	    send_to_char("You do not have enough mana!", ch);
	    return;
	}
	dam *= GET_GUILD_SKILL(ch, OUTLAW_SKILL_ANGRYYELL) >> 4;
	act("$n yells YOK at $N angrilly!!!", TRUE, ch, 0, victim, TO_ROOM);
	do_say(ch, "YOKYOKYOKYOKYOKYOK!!!!!!!!!!!!!!!!", 0);
	damage(ch, victim, dam, TYPE_MISC);
	GET_POS(victim) = POS_STUNNED;
	GET_MANA(ch) -= dam >> 1;
	GET_HIT(ch) += dam >> 1;
	WAIT_STATE(ch, PULSE_VIOLENCE);
    }
    else {
	send_to_char("Get the fuck out!!!\n\r", ch);
	GET_HIT(ch) -= dam >> 1;
	GET_MANA(ch) -= dam;
    }
}

void do_smoke(struct char_data *ch, char *argument, int cmd)
{
}

void do_inject(struct char_data *ch, char *argument, int cmd)
{
}

/* skills of assasins */
void do_shadow(struct char_data *ch, char *argument, int cmd)
{
    struct affected_type af;

    /* remove guild skills by atre */
    return;

    if (GET_GUILD(ch) != ASSASSIN && IS_MORTAL(ch)) {
	send_to_char("You are not assasin!!!\n\r", ch);
	return;
    }
    if (GET_MANA(ch) < 1000 && IS_MORTAL(ch)) {
	send_to_char("Your mana isn't enough to make a shadow!!!\n\r", ch);
	return;
    }
    if (!affected_by_spell(ch, SPELL_SHADOW_FIGURE)) {
	act("$n disappears suddenly!!!", TRUE, ch, 0, 0, TO_ROOM);
	act("Now,They see your shadow only.", TRUE, ch, 0, 0, TO_CHAR);
	af.type = SPELL_SHADOW_FIGURE;
	af.duration = 5;
	af.modifier = 0;
	af.location = APPLY_NONE;
	af.bitvector = AFF_SHADOW_FIGURE;
	affect_to_char(ch, &af);
    }
    GET_MANA(ch) -= (20 * (100 - GET_GUILD_SKILL(ch, ASSASSIN_SKILL_SHADOW_FIGURE)));
}

/* 싸움 도중 hp가 1/6 일때 자신의 길드로 돌아감 */
void do_solace(struct char_data *ch, char *argument, int cmd)
{
    int home = ROOM_GUILD_ASSASSIN, location;
    bool found = FALSE;
    extern int top_of_world;

    /* remove guild skills by atre */
    return;

    if ((GET_GUILD(ch) != ASSASSIN && IS_MORTAL(ch)) || IS_NPC(ch))
	return;
    for (location = 0; location <= top_of_world; location++) {
	if (world[location].number == home) {
	    found = TRUE;
	    break;
	}
    }
    if ((location == top_of_world) || !found) {
	send_to_char("You cannot get the chance!!!\n\r", ch);
	return;
    }
    if ((GET_GUILD_SKILL(ch, ASSASSIN_SKILL_SOLACE) > number(0, 100) &&
	 GET_HIT(ch) < (GET_PLAYER_MAX_HIT(ch) / 7))
	|| IS_WIZARD(ch)) {
	do_say(ch, "HAHAHAHAHAH!!! BYEBYE!!!!!", 0);
	act("$n disappears.", TRUE, ch, 0, 0, TO_ROOM);
	if (ch->specials.fighting) {
	    stop_fighting(ch->specials.fighting);
	    stop_fighting(ch);
	}
	char_from_room(ch);
	char_to_room(ch, location);
	act("$n appears in the middle of the room.", TRUE, ch, 0, 0, TO_ROOM);
	do_look(ch, "", 15);
	GET_MOVE(ch) = 0;
    }
    else {
	send_to_char("But,You are healthy enought to fight!!!\n\r", ch);
	GET_MOVE(ch) -= 100;
    }
}

void do_evil_strike(struct char_data *ch, char *argument, int cmd)
{
    int dam, percent;
    char name[256];
    struct char_data *victim;

    /* remove guild skills by atre */
    return;

    one_argument(argument, name);

    if ((GET_GUILD(ch) != ASSASSIN && IS_MORTAL(ch)) || IS_NPC(ch))
	return;
    if (!(victim = get_char_room_vis(ch, name))) {
	if (ch->specials.fighting) {
	    victim = ch->specials.fighting;
	}
	else {
	    send_to_char("evil strike who?\n\r", ch);
	    return;
	}
    }
    if (ch == victim) {
	do_say(ch, "Wahhh...I cannot use on mobs!!", 0);
	return;
    }
    if (!IS_EVIL(ch)) {
	send_to_char("Huh! You are not evil!!", ch);
	return;
    }
    /* you must think carefully what this code means */
    percent = GET_ALIGNMENT(ch) - GET_ALIGNMENT(victim);
    percent >>= 3;
    percent += number(1, 500);

    dam = GET_ALIGNMENT(ch);
    dam = MAX(dam, -dam);
    dam *= number(1, GET_LEVEL(ch) - 20);

    if (percent < GET_GUILD_SKILL(ch, ASSASSIN_SKILL_EVIL_STRIKE)) {
	if (GET_MANA(ch) < 1000) {
	    send_to_char("You do not have enough mana.", ch);
	    return;
	}
	act("You grin evilly and strike with $s DEVIL POWER!!!", TRUE,
	    ch, 0, 0, TO_CHAR);
	act("$n grins evilly and strike with $s DEVIL POWER!!!", TRUE,
	    ch, 0, ch, TO_ROOM);

	WAIT_STATE(victim, PULSE_VIOLENCE);
	damage(ch, victim, dam, TYPE_MISC);
	GET_MANA(ch) -= 1000 -
	    (GET_GUILD_SKILL(ch, ASSASSIN_SKILL_EVIL_STRIKE) << 2);
    }
    else {
	act("You could not summon $s POWER!!", TRUE, ch, 0, 0, TO_CHAR);
	act("$n could not summon $s POWER!!!", TRUE, ch, 0, ch, TO_ROOM);
	GET_HIT(victim) += dam >> 2;
    }
}

/*
   Assault skill: 남자나 여자를 강간한다. :)
   입고있는 아이템을 벗긴다. 헤헤헤...야한 메시지와 함께.
   by Process
 */
void do_assault(struct char_data *ch, char *argument, int cmd)
{
    struct char_data *victim;
    struct obj_data *obj;

    char victim_name[240];
    int percent;
    int location;

    if (GET_GUILD(ch) != OUTLAW && IS_MORTAL(ch)) {
	send_to_char("How about join outlaws???\n\r", ch);
	return;
    }
    one_argument(argument, victim_name);

    if (!(victim = get_char_room_vis(ch, victim_name))) {
	send_to_char("who do you wanna assault?\n\r", ch);
	return;
    }

    if (!IS_NPC(victim) && !(victim->desc)) {
	send_to_char("You cannot assault him or her.\n\r", ch);
	return;
    }

    if (GET_SEX(victim) == GET_SEX(ch)) {
	switch (GET_SEX(ch)) {
	case SEX_MALE:
	    send_to_char("너 호모냐?\n\r", ch);
	    break;
	case SEX_FEMALE:
	    send_to_char("너 레즈비언이냐?\n\r", ch);
	    break;
	}
	return;
    }
    percent = number(1, 101) + ((GET_LEVEL(victim) + GET_DEX(victim)) << 1) -
	(GET_LEVEL(ch) + GET_DEX(ch));

    if (percent < GET_GUILD_SKILL(ch, OUTLAW_SKILL_ASSAULT)) {
	location = number(2, MAX_WEAR - 1);
	obj = victim->equipment[location];
	if (obj && CAN_SEE_OBJ(ch, obj)) {
	    if (victim)
		obj_to_char(unequip_char(victim, location), victim);
	    switch (GET_SEX(victim)) {
	    case SEX_MALE:
	    case SEX_NEUTRAL:
		do_say(ch, "아하아~~~~너무 좋아\n\r", 0);
		do_say(victim, "아~~~나도 좋아요~~~\n\r", 0);
		do_say(victim, "아...기막히다....한 번 더하고 싶다~~\n\r", 0);
		break;
	    case SEX_FEMALE:
		do_say(ch, "아하아~~~~너무 좋아\n\r", 0);
		do_say(victim, "아~~~나도 좋아요~~~\n\r", 0);
		do_say(victim, "이제 진정한 여자가 된 기분이에요~~~\n\r", 0);
		break;
	    }

	}
	else {
	    send_to_char("바보....거긴 안 입었다..:)\n\r", ch);
	}
    }
    else {
	send_to_char("먼 챙피? 하하하...\n\r", ch);
    }
}
