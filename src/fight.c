/* ************************************************************************
   *  File: fight.c , Combat module.                         Part of DIKUMUD *
   *  Usage: Combat system and messages.                                     *
   *  Copyright (C) 1990, 1991 - see 'license.doc' for complete information. *
   ************************************************************************* */

#include <stdio.h>
#include <string.h>

#include "char.h"
#include "object.h"
#include "global.h"
#include "comm.h"
#include "play.h"
#include "spells.h"
#include "etc.h"

/* Structures */
struct char_data *combat_list = 0;	/* head of l-list of fighting chars */
struct char_data *combat_next_dude = 0;		/* Next dude global trick */ 

/* =================================================================== */ 
/* NOTE:    Moved following type, defintion from "play.h" */ 

#define MAX_MESSAGES		61

struct msg_type {
    char *attacker_msg;			/* message to attacker */
    char *victim_msg;			/* message to victim   */
    char *room_msg;			/* message to room     */
};

struct message_type {
    struct msg_type die_msg;		/* messages when death		  */
    struct msg_type miss_msg;		/* messages when missi		  */
    struct msg_type hit_msg;		/* messages when hiti		  */
    struct msg_type sanctuary_msg;	/* messages when hit on sanctuary */
    struct msg_type god_msg;		/* messages when hit on god	  */
    struct message_type *next;		/* to next messages of this kind. */
}; 

struct message_list {
    int a_type;				/* Attack type		*/
    int number_of_attacks;	/* How many attack messages to chose from. */
    struct message_type *msg;		/* List of messages.	*/
};

/* Attacktypes with grammar */

struct attack_hit_type {
    char *singular;
    char *plural;
};

struct message_list fight_messages[MAX_MESSAGES];

/*--------------------------------------------------*/

/* The Fight related routines */
/* start one char fighting another (yes, it is horrible, I know... )  */
void set_fighting(struct char_data *ch, struct char_data *vict)
{
    assert(!ch->specials.fighting);

    ch->next_fighting = combat_list;
    combat_list = ch;

    if (IS_AFFECTED(ch, AFF_SLEEP))
	affect_from_char(ch, SPELL_SLEEP);

    ch->specials.fighting = vict;
    GET_POS(ch) = POS_FIGHTING;
} 

/* remove a char from the list of fighting chars */
void stop_fighting(struct char_data *ch)
{
    struct char_data *tmp;

    if (!ch)
	return;

    /* assert(ch->specials.fighting); */
    /* NOTE: DEBUG: This assertion fails sometimes.. why? */
    if (!ch->specials.fighting) {
	log("stop_fighting(): No ch->fighting!!");
	return;
    }

    if (ch == combat_next_dude)
	combat_next_dude = ch->next_fighting;

    if (combat_list == ch)
	combat_list = ch->next_fighting;
    else {
	for (tmp = combat_list; tmp && (tmp->next_fighting != ch);
	     tmp = tmp->next_fighting) ;
	if (!tmp) {
	    log("Char fighting not found Error (fight.c, stop_fighting)");
	    /* abort(); */
	    goto next;
	}
	tmp->next_fighting = ch->next_fighting;
    }

  next:
    ch->next_fighting = 0;
    ch->specials.fighting = 0;
    GET_POS(ch) = POS_STANDING;
    update_pos(ch);
}

#define MAX_NPC_CORPSE_TIME 15
#define MAX_PC_CORPSE_TIME 40
  
/* NOTE: Change of 2nd arg: OLD killer level -> char. who killed me */
void make_corpse(struct char_data *ch, struct char_data *who)
{
    struct obj_data *corpse;
    struct obj_data *otmp;
    char buf[MAX_STRING_LENGTH];
    int i;
    extern void object_list_new_owner(struct obj_data *list,
    		struct char_data *ch); 
    extern struct obj_data *create_money(int amount);
    extern  void special_death(struct char_data *ch, struct char_data *who,
	struct obj_data *corpse);
    extern void check_light_off(struct obj_data *obj_object, sh_int room);

    CREATE(corpse, struct obj_data, 1);

    clear_object(corpse);

    corpse->item_number = NOWHERE;
    corpse->in_room = NOWHERE;
    sprintf(buf, "corpse %s",
	    (IS_NPC(ch) ? ch->player.short_descr : GET_NAME(ch)));
    corpse->name = strdup(buf);

    sprintf(buf, "Corpse of %s is lying here.",
	    (IS_NPC(ch) ? ch->player.short_descr : GET_NAME(ch)));
    corpse->description = strdup(buf);

    sprintf(buf, "Corpse of %s",
	    (IS_NPC(ch) ? ch->player.short_descr : GET_NAME(ch)));
    corpse->short_description = strdup(buf);

    corpse->contains = ch->carrying;
    corpse->obj_flags.type_flag = ITEM_CONTAINER;
    corpse->obj_flags.wear_flags = ITEM_TAKE;
    corpse->obj_flags.value[0] = 0;	/* You can't store stuff in a corpse */
    /* corpse->obj_flags.value[3] = 1; *//* corpse identifyer */
    corpse->obj_flags.weight = ch->player.weight + GET_CARRYING_W(ch);

    /* norent */
    corpse->obj_flags.extra_flags = ITEM_NORENT;

    /* changed by jhpark */
    if (IS_NPC(ch)) {
	corpse->obj_flags.timer = MAX_NPC_CORPSE_TIME;
	corpse->obj_flags.value[3] = 1;		/* corpse identifyer : NPC */
    }
    else {
	corpse->obj_flags.timer = MAX_PC_CORPSE_TIME;
	corpse->obj_flags.value[3] = 2;		/* corpse identifyer : PC */
    }

    corpse->next = object_list;
    object_list = corpse;

    /* NOTE: special death/corpse processing in "spec_misc.c" 	*/
    /* NOTE: After creating empty corpse and before unequip ch  */ 
    special_death(ch, who, corpse);

    /* NOTE: Do light adjustment here. Not in extract_char(). */
    if ( ch->equipment[WEAR_LIGHT])
	check_light_off(ch->equipment[WEAR_LIGHT], ch->in_room);

    for (i = 0; i < MAX_WEAR; i++) {
	if (ch->equipment[i]) {
#ifdef  NO_DEF
	    /* NOTE: Move this code to special_death() */
	    /* 손오공 */ 
	    if (mob_index[ch->nr].virtual == 11101 && i == WEAR_HEAD) {
		otmp = unequip_char(ch, i);
		extract_obj(otmp);
	    }
#endif		/* NO_DEF */

	    otmp = unequip_char(ch, i);
	    obj_to_obj(otmp, corpse); 
	}
    }

    ch->carrying = 0;
    GET_CARRYING_N(ch) = 0;
    GET_CARRYING_W(ch) = 0;


    for (otmp = corpse->contains; otmp; otmp = otmp->next_content)
	otmp->in_obj = corpse;

    object_list_new_owner(corpse, 0);

#ifdef NO_DEF
    /* NOTE: Move this code to special_death() proc. */

    /* 삼장법사(11111) (put 금테(11127), 성수병(11134) Into corpse) */
    if (mob_index[ch->nr].virtual == 11111) {
	o = read_object(11127, VIRTUAL);
	obj_to_obj(o, corpse);
	o = read_object(11134, VIRTUAL);
	obj_to_obj(o, corpse);
    }

    /* GoodBadIsland */
    /* IRON GOLEM(23323) */
    if (mob_index[ch->nr].virtual == 23323) {
	act("거대한 철문이 열리고, 위층으로 올라가는 계단이 보입니다.",
	    FALSE, ch, 0, 0, TO_ROOM);
	REMOVE_BIT(EXIT(ch, 4)->exit_info, EX_LOCKED);
	REMOVE_BIT(EXIT(ch, 4)->exit_info, EX_CLOSED);
    }

    /* KAALM(23301) */
    if (mob_index[ch->nr].virtual == 23301) {
	o = read_object(23309, VIRTUAL);
	obj_to_obj(o, corpse);
    } 
#endif		/* NO_DEF */

    obj_to_room(corpse, ch->in_room);

    /* ch is freed in extact_char */
    /* if (IS_NPC(ch) && ch != NULL) {
	free(ch);
	ch = NULL;
    } */
}

/* When ch kills victim */
void change_alignment(struct char_data *ch, struct char_data *victim)
{
    int al, al_v;

    /* 
       al=(7*GET_ALIGNMENT(ch)-GET_ALIGNMENT(victim))/8;
       al=(al-GET_ALIGNMENT(ch))/10; */
    al_v = -GET_ALIGNMENT(victim);
    al = al_v / 200;
    if (al_v > 0) {
	al += number(1, 3);
    }
    else {
	al -= number(1, 3);
    }

    GET_ALIGNMENT(ch) += al;
    if (GET_ALIGNMENT(ch) < -1000)
	GET_ALIGNMENT(ch) = -1000;
    if (GET_ALIGNMENT(ch) > 1000)
	GET_ALIGNMENT(ch) = 1000;
}

void death_cry(struct char_data *ch)
{
    int door, was_in;

    acthan("Your blood freezes as you hear $n's death cry.",
	   "$n님의 처절한 비명소리가 들립니다. 으 소름끼쳐 ~~", 
	   FALSE, ch, 0, 0, TO_ROOM);
    was_in = ch->in_room;

    for (door = 0; door <= 5; door++) {
	if (CAN_GO(ch, door)) {
	    ch->in_room = world[was_in].dir_option[door]->to_room;
	    acthan("Your blood freezes as you hear someones death cry.",
		   "어디선가 소름끼치는 비명소리가 들려옵니다.",
		   FALSE, ch, 0, 0, TO_ROOM);
	    ch->in_room = was_in;
	}
    }
}

/* NOTE: 2nd arg: OLD killer level -> char. who killed me : Not used yet */
void raw_kill(struct char_data *ch, struct char_data *who)
{
    if (ch->specials.fighting)
	stop_fighting(ch);

    death_cry(ch);
    make_corpse(ch, who);
    extract_char(ch);
    /* NOTE: Move funeral message to die(). */
}

void die(struct char_data *ch, int level, struct char_data *who)
{
    /* int home; */
    int exp;
    extern void wipe_stash(char *name);

    if (!ch)
	return; 

    if (IS_NPC(ch)) {
	raw_kill(ch, who);
	return;
    }

#ifdef NO_DEF
    GET_PLAYER_MAX_HIT(ch) -= GET_LEVEL(ch);
    GET_PLAYER_MAX_MANA(ch) -= GET_LEVEL(ch);
    GET_PLAYER_MAX_MOVE(ch) -= GET_LEVEL(ch);

    if (IS_SET(ch->specials.act, PLR_CRIMINAL))
	REMOVE_BIT(ch->specials.act, PLR_CRIMINAL);
    home = 0;
    switch (GET_GUILD(ch)) {
    case POLICE:
	home = real_room(ROOM_GUILD_POLICE_LOCKER);
	break;
    case OUTLAW:
	home = real_room(ROOM_GUILD_OUTLAW_LOCKER);
	break;
    case ASSASSIN:
	home = real_room(ROOM_GUILD_ASSASSIN_LOCKER);
	break;
    }
    if (home != 0 && who) {
	if (!IS_NPC(who)) {
	    if (ch->in_room != NOWHERE)
		char_from_room(ch);
	    char_to_room(ch,home);
	    act("$n appears in the room BLEEDING!!!",
	    TRUE,ch,0,0,TO_ROOM);
	}
    }
#endif		/* NO_DEF */

    GET_GOLD(ch) = 0;

    /* NOTE: when player died, player lose at least fixed amount of exp
       according to level, minimum 1/10 of current exp  but not exceeding 
       half of current exp              */
    /* OLD: exp = GET_LEVEL(ch)*GET_LEVEL(ch)*level*200); */
    /* NEW: MAX(MIN(fixed exp according to levels,exp/2) exp /10 )   */

    exp = GET_LEVEL(ch) * GET_LEVEL(ch) * level * 300;
    exp = MAX(MIN(exp, GET_EXP(ch) / 2), GET_EXP(ch) / 10);

    gain_exp(ch, -exp);

    /* killed number(no only pk) */
    /* if(who && !IS_NPC(who)) */
    ch->player.pked_num++; 

    raw_kill(ch, who);

    /* NOTE: Trans dead soul (not corpse) to MID */
    /* NOTE: You will return to MID even after disconnecting and re-enter */
    ch->in_room =  real_room(MID_TEMPLE);
    /* NOTE: New save_char() can replace save_char_nocon() */
    /* For coin copy bug , fixed by dsshin */
    wipe_stash(GET_NAME(ch));
    save_char(ch);

    /* NOTE: Moved funeral message from raw_kill(). */
    if ( ch->desc ) {
	/* NOTE: show funeral message when player passwed away  */
	SEND_TO_Q("\r\nYou feel sudden heavenly weightlessness "
		  "looking down *BLOODY* body of your own.\r\n", ch->desc);

	/* NOTE: leaving game code moved from extract_char() (handler.c) */
	ch->desc->connected = CON_RMOTD;
	SEND_TO_Q("\r\n*** PRESS RETURN : ", ch->desc);
    }
}

void update_pos(struct char_data *victim)
{ 
    int unit_hit;
    
    /* NOTE: Extend range of dead, mortally wounded,.. etc */
    unit_hit = MIN(10000, GET_MAX_HIT(victim))/500 + 2 ;
    if ((GET_HIT(victim) > 0) && (GET_POS(victim) > POS_STUNNED))
	return;
    else if (GET_HIT(victim) > 0)
	GET_POS(victim) = POS_STANDING;
    else if (GET_HIT(victim) <= -11 * unit_hit)
	GET_POS(victim) = POS_DEAD;
    else if (GET_HIT(victim) <= -6 * unit_hit)
	GET_POS(victim) = POS_MORTALLYW;
    else if (GET_HIT(victim) <= -3 * unit_hit)
	GET_POS(victim) = POS_INCAP;
    else
	GET_POS(victim) = POS_STUNNED; 
} 

void load_messages(void)
{
    FILE *f1;
    int i, type;
    struct message_type *messages;
    char chk[100];

    if (!(f1 = fopen(lookup_db("hitmsg"), "r"))) {
	perror("read messages");
	exit(0);
    }

    for (i = 0; i < MAX_MESSAGES; i++) {
	fight_messages[i].a_type = 0;
	fight_messages[i].number_of_attacks = 0;
	fight_messages[i].msg = 0;
    }

    fscanf(f1, " %s \n", chk);

    while (*chk == 'M') {
	fscanf(f1, " %d\n", &type);
	for (i = 0; (i < MAX_MESSAGES) && (fight_messages[i].a_type != type) &&
	     (fight_messages[i].a_type); i++) ;
	if (i >= MAX_MESSAGES) {
	    log("Too many combat messages.");
	    exit(0);
	}

	CREATE(messages, struct message_type, 1);

	fight_messages[i].number_of_attacks++;
	fight_messages[i].a_type = type;
	messages->next = fight_messages[i].msg;
	fight_messages[i].msg = messages;
	messages->die_msg.attacker_msg = fread_string(f1);
	messages->die_msg.victim_msg = fread_string(f1);
	messages->die_msg.room_msg = fread_string(f1);
	messages->miss_msg.attacker_msg = fread_string(f1);
	messages->miss_msg.victim_msg = fread_string(f1);
	messages->miss_msg.room_msg = fread_string(f1);
	messages->hit_msg.attacker_msg = fread_string(f1);
	messages->hit_msg.victim_msg = fread_string(f1);
	messages->hit_msg.room_msg = fread_string(f1);
	messages->god_msg.attacker_msg = fread_string(f1);
	messages->god_msg.victim_msg = fread_string(f1);
	messages->god_msg.room_msg = fread_string(f1);
	fscanf(f1, " %s \n", chk);
    }
    fclose(f1);
}

void appear(struct char_data *ch)
{
    act("$n slowly fade into existence.", FALSE, ch, 0, 0, TO_ROOM);
    if (affected_by_spell(ch, SPELL_INVISIBLE))
	affect_from_char(ch, SPELL_INVISIBLE);
    REMOVE_BIT(ch->specials.affected_by, AFF_INVISIBLE);
}

void dam_message(int dam, struct char_data *ch, struct char_data *victim,
		 int w_type)
{
    struct obj_data *wield;
    char *buf, *buf2;
    int msg_index;

    /* NOTE: Moved damage messages definition to "constants.c"	*/

    extern struct dam_weapon_type brief_dam_weapons[];
    extern struct dam_weapon_type you_dam_weapons[];
    extern struct dam_weapon_type brief_han_weapons[];
    /* extern struct dam_weapon_type han_weapons[]; */  /* NOTE: not used */
    /* extern struct dam_weapon_type dam_weapons[]; */  /* NOTE: not used */

    w_type -= TYPE_HIT;	/* Change to base of table with text */
    wield = ch->equipment[WIELD];

/* updated by atre */
    if (dam == 0) msg_index = 0;
    else if (dam <= 20) msg_index = 1;
    else if (dam <= 50) msg_index = 2;
    else if (dam <= 100) msg_index = 3;
    else if (dam <= 200) msg_index = 4;
    else if (dam <= 400) msg_index = 5;
    else if (dam <= 600) msg_index = 6;
    else if (dam <= 800) msg_index = 7;
    else if (dam <= 1000) msg_index = 8;
    else if (dam <= 1200) msg_index = 9;
    else if (dam <= 1300) msg_index = 10;
    else if (dam <= 1500) msg_index = 11;
    else if (dam <= 2000) msg_index = 12;
    else if (dam <= 3000) msg_index = 13;
    else if (dam <= 5000) msg_index = 14;
    else msg_index = 15;

/*
   all messages are brief...
 */
/* NOTE: And Brief message has no string to replace (no #W macro ).
   Omit replace_string() with attack_hit_text[] for efficency.  */
/* NOTE: OLD example of replace_string() usage.
   buf = replace_string ( brief_dam_weapons[msg_index].to_room,
   attack_hit_text[w_type].plural);
 */
    buf = brief_dam_weapons[msg_index].to_room;
    buf2 = brief_han_weapons[msg_index].to_room;

    /* no miss */
    if (msg_index > 0)
	acthan(buf, buf2, FALSE, ch, wield, victim, TO_NOTVICT);

    /* NOTE: When Player is in group fighting and English mode, prepend '*'
       char to 'You', 'you', 'YOU' word to help they be easily recognized
       among fast and long damage messages scroll */

    if (!IS_NPC(ch) && IS_AFFECTED(ch, AFF_GROUP))
	buf = you_dam_weapons[msg_index].to_char;
    else
	buf = brief_dam_weapons[msg_index].to_char;
    buf2 = brief_han_weapons[msg_index].to_char;

    if (ch->desc)
	acthan(buf, buf2, FALSE, ch, wield, victim, TO_CHAR);

    if (!IS_NPC(victim) && IS_AFFECTED(victim, AFF_GROUP))
	buf = you_dam_weapons[msg_index].to_victim;
    else
	buf = brief_dam_weapons[msg_index].to_victim;
    buf2 = brief_han_weapons[msg_index].to_victim;

    if (victim->desc)
	acthan(buf, buf2, FALSE, ch, wield, victim, TO_VICT);

}

#ifdef OLD_DAM_MESSAGE

/* NOTE: Weapon attack texts moved to "constants.c"
    struct attack_hit_type attack_hit_text[] =
    struct attack_hit_type attack_hit_han[] =
 */ 

char *replace_string(char *str, char *weapon)
{
    static char buf[3][256];
    static int count = 0;
    char *rtn;
    char *cp;

    cp = rtn = buf[count % 3];
    count++;

    for (; *str; str++) {
	if (*str == '#') {
	    if (*(++str) == 'W')
		for (; *weapon; *(cp++) = *(weapon++)) ;
	    else
		*(cp++) = '#';
	}
	else
	    *(cp++) = *str;
	*cp = 0;
    }			/* For */
    return (rtn);
}

void dam_message(int dam, struct char_data *ch, struct char_data *victim,
		 int w_type)
{
    struct obj_data *wield;
    char *buf, *buf2;
    int msg_index;

    w_type -= TYPE_HIT;	/* Change to base of table with text */

    wield = ch->equipment[WIELD];

/* updated by atre */
/*
   if (dam == 0)                 msg_index = 0 ;
   else if ( dam <= 10 )        msg_index = 1 ;
   else if ( dam <= 20 )        msg_index = 2 ;
   else if ( dam <= 30 )        msg_index = 3 ;
   else if ( dam <= 40 )        msg_index = 4 ;
   else if ( dam <= 50 )        msg_index = 5 ;
   else if ( dam <= 70 )        msg_index = 6 ;
   else if ( dam <= 110)        msg_index = 7 ;
   else if ( dam <= 160)        msg_index = 8 ;
   else if ( dam <= 200) msg_index = 9 ;
   else if ( dam <= 300) msg_index = 10 ;
   else if ( dam <= 500) msg_index = 11 ;
   else if ( dam <= 700) msg_index = 12 ;
   else if ( dam <= 1000) msg_index = 13;
   else if ( dam <= 2000) msg_index = 14;
   else msg_index=15;
 */
    if (dam == 0) msg_index = 0;
    else if (dam <= 20) msg_index = 1;
    else if (dam <= 50) msg_index = 2;
    else if (dam <= 100) msg_index = 3;
    else if (dam <= 200) msg_index = 4;
    else if (dam <= 400) msg_index = 5;
    else if (dam <= 600) msg_index = 6;
    else if (dam <= 800) msg_index = 7;
    else if (dam <= 1000) msg_index = 8;
    else if (dam <= 1200) msg_index = 9;
    else if (dam <= 1300) msg_index = 10;
    else if (dam <= 1500) msg_index = 11;
    else if (dam <= 2000) msg_index = 12;
    else if (dam <= 3000) msg_index = 13;
    else if (dam <= 5000) msg_index = 14;
    else msg_index = 15;

    if (msg_index == 3 || msg_index == 4 || msg_index == 5 || msg_index == 7) {
	buf = replace_string(dam_weapons[msg_index].to_room,
			     attack_hit_text[w_type].plural);
    }
    else {
	buf = replace_string(dam_weapons[msg_index].to_room,
			     attack_hit_text[w_type].singular);
    }
    buf2 = replace_string(han_weapons[msg_index].to_room,
			  attack_hit_han[w_type].singular);
    acthan(buf, buf2, FALSE, ch, wield, victim, TO_NOTVICT);

    buf = replace_string(dam_weapons[msg_index].to_char,
			 attack_hit_text[w_type].singular);
    buf2 = replace_string(han_weapons[msg_index].to_char,
			  attack_hit_han[w_type].singular);
    acthan(buf, buf2, FALSE, ch, wield, victim, TO_CHAR);

    if (msg_index == 1 || msg_index == 2 || msg_index == 3 || msg_index == 5
	|| msg_index == 7) {
	buf = replace_string(dam_weapons[msg_index].to_victim,
			     attack_hit_text[w_type].plural);
    }
    else {
	buf = replace_string(dam_weapons[msg_index].to_victim,
			     attack_hit_text[w_type].singular);
    }
    buf2 = replace_string(han_weapons[msg_index].to_victim,
			  attack_hit_han[w_type].singular);
    acthan(buf, buf2, FALSE, ch, wield, victim, TO_VICT);
}

#endif		/* OLD_DAM_MESSAGE */

/* NOTE: NEW! Non-hit/non-weapon type message: separated from damage()  */
void fight_message(int dam, struct char_data *ch, struct char_data *victim,
		 int attacktype)
{
    struct message_type *messages;
    int i, j, nr ;

    /* NOTE: find matching message type */
    for (i = 0; i < MAX_MESSAGES; i++)
	if (fight_messages[i].a_type == attacktype) 
	    break;
    if( i >= MAX_MESSAGES ) {
	char buf[MAX_BUFSIZ];
	sprintf( buf, "fight_message(): type %d, attacker= %s, vic = %s ",
		attacktype, GET_NAME(ch), GET_NAME(victim) );
	log(buf);
	return;
    }

    nr = dice(1, fight_messages[i].number_of_attacks);
    for (j = 1, messages = fight_messages[i].msg;
	 j < nr && messages; j++)
	messages = messages->next;

    if (!IS_NPC(victim) && (GET_LEVEL(victim) >= IMO)) {
	act(messages->god_msg.attacker_msg, FALSE, ch,
	    ch->equipment[WIELD], victim, TO_CHAR);
	act(messages->god_msg.victim_msg, FALSE, ch,
	    ch->equipment[WIELD], victim, TO_VICT);
	act(messages->god_msg.room_msg, FALSE, ch,
	    ch->equipment[WIELD], victim, TO_NOTVICT);
    }
    else if (dam != 0) {
	if (GET_POS(victim) == POS_DEAD) {
	    act(messages->die_msg.attacker_msg, FALSE, ch,
		ch->equipment[WIELD], victim, TO_CHAR);
	    act(messages->die_msg.victim_msg, FALSE, ch,
		ch->equipment[WIELD], victim, TO_VICT);
	    act(messages->die_msg.room_msg, FALSE, ch,
		ch->equipment[WIELD], victim, TO_NOTVICT);
	}
	else {
	    act(messages->hit_msg.attacker_msg, FALSE, ch,
		ch->equipment[WIELD], victim, TO_CHAR);
	    act(messages->hit_msg.victim_msg, FALSE, ch,
		ch->equipment[WIELD], victim, TO_VICT);
	    act(messages->hit_msg.room_msg, FALSE, ch,
		ch->equipment[WIELD], victim, TO_NOTVICT);
	}
    }
    else {	/* Dam == 0 */
	/* NOTE: skip suffring/ poison damage miss message */
	if ( ch != victim )
	    act(messages->miss_msg.attacker_msg, FALSE, ch,
		ch->equipment[WIELD], victim, TO_CHAR);
	act(messages->miss_msg.victim_msg, FALSE, ch,
	    ch->equipment[WIELD], victim, TO_VICT);
	act(messages->miss_msg.room_msg, FALSE, ch,
	    ch->equipment[WIELD], victim, TO_NOTVICT);
    }
}

void damage(struct char_data *ch, struct char_data *victim,
	    int dam, int attacktype)
{
    int max_hit, attack;
    extern int nokillflag;

    int die_special(struct char_data *ch, struct char_data *victim);
    extern void do_flee(struct char_data *ch, char *argument, int cmd);
    extern void victory(struct char_data *ch, struct char_data *victim); 

    if (!ch || !victim)
	return;

    attack = ( attacktype != TYPE_SUFFERING && attacktype != TYPE_NOATTACK
		&& attacktype != SPELL_POISON );
    /* NOTE: attack == 0 means damge is not by attacking. */

#ifdef GHOST
    /* connectionless PC can't damage or be damaged */
    if ((!IS_NPC(ch) && !ch->desc) || (!IS_NPC(victim) && !victim->desc))
	return;
#endif				/* GHOST */

    if (nokillflag && !IS_NPC(ch) && !IS_NPC(victim) && ch != victim)
	return;

    /* can't hit same guild member */
    if (ch->player.guild == victim->player.guild &&
	(ch->player.guild || victim->player.guild) && ch != victim ) {
	send_to_char("Wanna hit your guild member?  CAN'T!\n\r", ch);
	return;
    }
    /* NOTE: Check victim's room */
    if( ch->in_room != victim->in_room && attack )
	return;
	
    /* NOTE: If victim is already dead position, give eternal peace to it. */
    if (GET_POS(victim) <= POS_DEAD) {
	GET_HIT(victim) -= dam;
	goto eternal_peace; 
    }

    if (GET_LEVEL(victim) >= IMO && !IS_NPC(victim))
	dam = 0;

    if (victim != ch && attack ) {
	if (GET_POS(victim) > POS_STUNNED) {
	    if (!victim->specials.fighting)
		set_fighting(victim, ch);
	    GET_POS(victim) = POS_FIGHTING;
	}

	if (GET_POS(ch) > POS_STUNNED) {
	    if (!ch->specials.fighting && ch->in_room == victim->in_room)
		set_fighting(ch, victim);

	    /* forbid charmed person damage charmed person */
	    /* 
	    if (IS_AFFECTED(ch, AFF_CHARM) && IS_AFFECTED(victim,AFF_CHARM)) {
		if(ch->specials.fighting)
		   stop_fighting(ch); if(victim->specials.fighting)
		stop_fighting(victim);
		return;
	    }
	   */

	    /* charmed mob can't ALWAYS kill another mob for player */
	    if (IS_NPC(ch) && IS_NPC(victim) && victim->master &&
		!number(0, 4) && IS_AFFECTED(victim, AFF_CHARM) &&
		(victim->master->in_room == ch->in_room)) {
		if (ch->specials.fighting)
		    stop_fighting(ch);
		if (ch->in_room == victim->master->in_room)
		    hit(ch, victim->master, TYPE_UNDEFINED);
		return;
	    }
	}
	if (IS_AFFECTED(ch, AFF_INVISIBLE))
	    appear(ch);
    }

    if (victim->master == ch)
	stop_follower(victim);
    if (IS_AFFECTED(victim, AFF_SANCTUARY))
    /* NOTE: Less damage reduction from sanctuary (50 % damage -> 67 %) */
	/* dam /= 2; */
	dam = ( dam * 2 ) / 3 ;
    if (IS_AFFECTED(victim, AFF_LOVE))
	dam *= 2;
    /* if (IS_NPC(ch)) dam *= 2; */

    dam = MAX(dam, -dam);
    /* MAX DAMAGE */
    dam = MIN(dam, 100000); /* this is the maximum damage */
    GET_HIT(victim) -= dam+2/3;

    /* exp gained by damage */
    if (ch != victim)
	/* NOTE: Less exp from damage : dam*level/2 -> dam*level/10 */
	gain_exp(ch, GET_LEVEL(victim) * dam / 10);

    update_pos(victim); 
eternal_peace: 
    
    if (attacktype >= TYPE_HIT && attacktype < TYPE_SHOOT ) {
	if (!ch->equipment[WIELD])
	    dam_message(dam, ch, victim, TYPE_HIT);
	else
	    dam_message(dam, ch, victim, attacktype);
    }
    else if (( attacktype > 0  && attacktype < TYPE_MISC )
	|| attacktype == TYPE_SUFFERING ) {
	/* NOTE: Separate non-hit/non-weapon damage msg */
	fight_message(dam, ch, victim, attacktype);
    }

    switch (GET_POS(victim)) {
    case POS_MORTALLYW:
	act("$n is mortally wounded, and will die soon, if not aided.",
	    TRUE, victim, 0, 0, TO_ROOM);
	act("You are mortally wounded, and will die soon, if not aided.",
	    FALSE, victim, 0, 0, TO_CHAR);
	break;
    case POS_INCAP:
	act("$n is incapacitated and will slowly die, if not aided.",
	    TRUE, victim, 0, 0, TO_ROOM);
	act("You are incapacitated an will slowly die, if not aided.",
	    FALSE, victim, 0, 0, TO_CHAR);
	break;
    case POS_STUNNED:
	act("$n is stunned, but could regain consciousness again.",
	    TRUE, victim, 0, 0, TO_ROOM);
	act("You're stunned, but could regain consciousness again.",
	    FALSE, victim, 0, 0, TO_CHAR);
	break;
    case POS_DEAD:
	act("$n is dead! R.I.P.", TRUE, victim, 0, 0, TO_ROOM);
	act("You are dead!  Sorry...", FALSE, victim, 0, 0, TO_CHAR);
	break;
    default:		/* >= POSITION SLEEPING */
	max_hit = hit_limit(victim);
	if (dam > max_hit / 10)
	    act("That Really did HURT!", FALSE, victim, 0, 0, TO_CHAR);
	if (IS_NPC(victim) && IS_SET(victim->specials.act, ACT_WIMPY) &&
	    GET_HIT(victim) < max_hit / 30) {
	    /* NOTE: NPC don't need wishing flee message */
	    /* act("You wish that your wounds would stop BLEEDING that
	       much!", FALSE, victim, 0, 0, TO_CHAR); */
	    do_flee(victim, "", 0);
	}
	else if (!IS_NPC(victim) && IS_SET(victim->specials.act, PLR_WIMPY))
	    /* NOTE: wimpiness == 0 means PC flee at max_hit/10  */
	    if (GET_HIT(victim) < (victim->specials.wimpyness
		 ? victim->specials.wimpyness : GET_MAX_HIT(victim) / 10)) {
		act("You wish that your wounds would stop BLEEDING that much!",
		    FALSE, victim, 0, 0, TO_CHAR);
		do_flee(victim, "", 0);
	    }
	break;
    }

    if (GET_POS(victim) < POS_MORTALLYW)
	if (ch->specials.fighting == victim)
	    stop_fighting(ch);

    if (!AWAKE(victim))
	if (victim->specials.fighting)
	    stop_fighting(victim);

    if (GET_POS(victim) == POS_DEAD) { 
	if( die_special(ch, victim))
	    return;
	/* NOTE: Separate code for winner gain to victory() */
	victory(ch, victim); 
	die(victim, GET_LEVEL(ch), ch);
    }
}

/* NOTE: NEW! special death handing part separated from damage() */
/* 	Return 0 for real and immediate death. Else 1 if rearaised. */ 
int die_special(struct char_data *ch, struct char_data *victim)
{
    char buf[MAX_BUFSIZ];
    extern void affect_remove_all(struct char_data *ch);

    /* NOTE: No second call by DEATH_SPELL, so no need to remove the affect.
            See remove_affect_special() in "handler.c"	*/

#ifdef NO_DEF
    /* to forbid 2 calls, by ares */
    if (IS_AFFECTED(ch, AFF_DEATH)) {
    /* NOTE: Use affect_from_char() */
        for (af = victim->affected; af; af = af->next)
            if (af->type == SPELL_DEATH)
                break;
    }
#endif		/* NO_DEF*/ 

    /* chase modified this for reraise */
    if (IS_AFFECTED(victim, AFF_RERAISE )) {

	/* NOTE: for NPC with AFF_RERISE/AFF_DEATH flag set */ 
	REMOVE_BIT(victim->specials.affected_by, AFF_RERAISE|AFF_DEATH);
	/* NOTE: 1/10 probability to fail to reraise. */
	/* NOTE: SPELL_DEATH / SPELL_CURSE will exert bad influence on it. */ 
	if( number(1, (IS_AFFECTED(victim, AFF_CURSE) ? 5 : 10)) == 1 
	    || IS_AFFECTED(victim, AFF_DEATH)) {
	    send_to_char("Alas! You FAILED to RERAISE!\r\n", victim);
	    return 0;
	}
	act("$n is RERAISED by Almighty Power!", TRUE, victim, 0, 0, TO_ROOM);
	send_to_char("God blesses you with Almighty Power!\r\n", victim) ;
	/* NOTE: victim stopped fight in damage() */
	/*
	if (victim->specials.fighting)
	    stop_fighting(victim);
	*/
	GET_HIT(victim) = GET_PLAYER_MAX_HIT(victim);
	/* NOTE: More mana, move for reraised victim. */
	GET_MANA(victim) = MAX(500, GET_MANA(victim));
	GET_MOVE(victim) = MAX(500, GET_MOVE(victim));

	/* NOTE: Dead man has no affect: to forbid second reraise.  */
	affect_remove_all(victim);
	GET_POS(victim) = POS_STANDING;
	if (number(1, 5) != 4) {
	    GET_ALIGNMENT(victim) = MIN(-1000, GET_ALIGNMENT(victim) - 200);
	    send_to_char("You feel sacred!", victim);
	}
	else {
	    GET_ALIGNMENT(victim) = MAX(GET_ALIGNMENT(victim) + 200, 1000);
	    send_to_char("You feel peaceful!", victim);
	}
	if (!IS_NPC(victim)) {	/* just for log */
	    sprintf(buf, "RERAISED: %s was reraised at killing of %s at %s",
		GET_NAME(victim),  GET_NAME(ch), world[victim->in_room].name);
	    log(buf);
	}
	return 1;
    }
    else if ( !IS_NPC(victim)) {	/* just for log */
	sprintf(buf, "KILLED: %s killed by %s at %s", GET_NAME(victim),
	    (IS_NPC(ch) ? ch->player.short_descr : GET_NAME(ch)),
	    world[victim->in_room].name);
	log(buf);
    }
    return 0;
}

/* NOTE: NEW! Sepaarated code for parry, mirror image and 
	miss case from hit(). */ 
int hit_miss(struct char_data *ch, struct char_data *victim, int type)
{
    int parry_num, prf, miss;
    extern int thaco[4][IMO + 4];

    prf = 0;
    /* parry for NPC */
    if (GET_LEARNED(victim, SKILL_PARRY)) {
	/* new mechanism by atre */
	parry_num = GET_LEARNED(victim, SKILL_PARRY);
	parry_num += (GET_SKILLED(victim, SKILL_PARRY) << 1);
	parry_num += (GET_DEX(victim) << 1);

	if (GET_CLASS(victim) == CLASS_THIEF) 
	    parry_num += GET_LEVEL(victim);

	if (number(0, parry_num) > 
	    (GET_HITROLL(ch) << 2) + GET_LEVEL(ch) - GET_LEVEL(victim) + 43) {
	    prf = 1;
	}

#ifdef OLD_PARRY
	if (IS_NPC(victim))
	    parry_num = GET_LEVEL(victim) * 50;
	/* parry_num = GET_LEVEL(victim)*35; */
	/* parry for PC */
	else {
	    /* parry_num = GET_LEVEL(victim)*6; */
	    parry_num = GET_LEVEL(victim) * ((GET_SKILLED(victim, SKILL_PARRY) >> 3) + 6);
	}

	if (GET_CLASS(victim) == CLASS_THIEF)
	    parry_num -= 2 * GET_LEVEL(victim);

	if (victim->skills[SKILL_PARRY].learned + 2 * GET_DEX(victim) >
	  number(1, parry_num) - ((GET_LEVEL(victim) - GET_LEVEL(ch)) << 2))
	    prf = 1;
#endif				/* OLD_PARRY */
    }
/* mirror image */
#ifdef OLD_MIRROR_IMAGE
    if (IS_AFFECTED(victim, AFF_MIRROR_IMAGE) &&
	3 * number(1, GET_LEVEL(ch)) < number(1, GET_LEVEL(victim)))
	prf = 2;
#endif				/* OLD_MIRROR_IMAGE */
/* new mechanism by atre */
    if (!prf && IS_AFFECTED(victim, AFF_MIRROR_IMAGE)) {
	parry_num = GET_LEVEL(victim) + GET_LEARNED(victim, SPELL_MIRROR_IMAGE);
	parry_num += (GET_SKILLED(victim, SPELL_MIRROR_IMAGE) << 1);
	if (number(0, parry_num) > 3 * GET_LEVEL(ch)) {
	    prf = 2;
	}
    }

    if ( prf ) {
	if (prf == 1) {
	    send_to_char("You parry successfully.\n\r", victim);
	    act("$N parries successfully.", FALSE, ch, 0, victim, TO_CHAR);
	    INCREASE_SKILLED1(victim, ch, SKILL_PARRY);
	}
	else if (prf == 2) {
	    send_to_char("You hit illusion. You are confused.\n\r", ch);
	    act("$n hits illusion, looks confused.",
		FALSE, ch, 0, victim, TO_VICT);
	    INCREASE_SKILLED1(victim, ch, SPELL_MIRROR_IMAGE);
	}
	return (2); /* NOTE: Return from hit() */
    }

    miss = -GET_HITROLL(ch);
    miss -= number(0, dex_app[GET_DEX(ch)].attack);
    miss -= number(0, thaco[GET_CLASS(ch) - 1][GET_LEVEL(ch)]);
    miss += dex_app[GET_DEX(victim)].defensive;
    if (type == SKILL_BACKSTAB) {
	miss -= (GET_LEVEL(ch) >> 2);
	miss -= ((GET_LEVEL(ch) >> 2) + (GET_SKILLED(ch, SKILL_BACKSTAB) >> 2));
    } 
    return ( miss > 0 ? 1 : 0 ) ;
}

/* NOTE: NEW! return damage type of weapon object (separated from hit()) */ 
int weapon_type(struct obj_data *weapon)
{
	switch (weapon->obj_flags.value[3]) {
	case 0: case 1: case 2:
	    return(TYPE_WHIP);
	case 3:
	    return(TYPE_SLASH);
	case 4: case 5: case 6:
	    return(TYPE_CRUSH);
	case 7:
	    return(TYPE_BLUDGEON);
	case 8: case 9: case 10: case 11:
	    return(TYPE_PIERCE);
	default:
	    return(TYPE_HIT);
	}
}

/* NOTE: Additional damage for warrior class: barehand dice and holding dice.
        Separated code from hit(). */ 
int warrior_damage(struct char_data *ch )
{ 
    int add_dam;
    int limit_nodice, limit_sizedice;

    if (GET_CLASS(ch) != CLASS_WARRIOR )
	return 0;

    /* increase dice of bare hand */
    if ( !ch->equipment[WIELD] ) {
	add_dam = dice(ch->specials.damnodice, ch->specials.damsizedice);

	if (number(0, 49 + ((ch->specials.damnodice * ch->specials.damsizedice) << 4)) == 37) {
	    /* Check for all remortaled by dsshin   */
	    if ((ch->player.level >= (IMO - 1)) && (ch->player.remortal >= 15)) {
	    /* Limit is changed by epochal          */
		limit_nodice = 50; limit_sizedice = 50;
		/* limit_nodice = 21; limit_sizedice = 31; */
	    }
	    else {
		limit_nodice = 15; limit_sizedice = 15;
	    }
/*  to here */
	    /* NOTE: For more balanced hand dice : number and size */
	    if (number(0, 1)) {
		if (ch->specials.damnodice < limit_nodice) {
		    send_to_char("Your bare hand dice is added!!!\n\r", ch);
		    ch->specials.damnodice++;
		}
	    }
	    else {
		if (ch->specials.damsizedice < limit_sizedice) {
		    send_to_char("Your bare hand dice is enlarged!!!\n\r", ch);
		    ch->specials.damsizedice++;
		}
	    }
	}
    }
    else 
	add_dam = 0;

    /* NOTE: Wielding dice and holding dice is independent. */
    if (ch->equipment[HOLD] && number(1, 10) > 4 &&
	CAN_WEAR(ch->equipment[HOLD], ITEM_WIELD)) 
	add_dam += dice(ch->equipment[HOLD]->obj_flags.value[1],
		    ch->equipment[HOLD]->obj_flags.value[2]);
    return(add_dam);
}

void hit(struct char_data *ch, struct char_data *victim, int type)
{
    struct obj_data *wielded = 0;
    /* struct obj_data *held = 0; */
    int w_type;
    int dam;
    int miss, rn;

    extern byte backstab_mult[];
    extern int special_damage(struct char_data *ch, 
    		struct char_data *victim, int damage );

    if (ch == NULL || victim == NULL)
	return;

    if (ch->in_room != victim->in_room) {
	/* NOTE: Victim fleed , maybe. Let's confirm it. */
	/* NOTE: This is for confirm, not DEBUGGING. Remove when confirmed. */
	char buf1[MAX_OUT_LEN], buf2[MAX_LINE_LEN];

	if (ch->in_room && victim->in_room)
	    sprintf(buf2, " at ch = %d, vic = %d",
		world[ch->in_room].number, world[victim->in_room].number );
	else
	    buf2[0] = 0 ; 
	sprintf(buf1, "fight.c: hit(). (type = %d) ch = %s victim = %s %s\n",
	    type, GET_NAME(ch), GET_NAME(victim), buf2);
	log("DEBUG: NOT SAME ROOM WHEN FIGHTING!");
	log(buf1);
	return; /* NOTE: To here */
    }
    miss = hit_miss(ch, victim, type);
    if ( miss > 1  )
	return; 
/*
    if (ch->equipment[HOLD])
	held = ch->equipment[HOLD];
*/ 
    if (ch->equipment[WIELD] &&
	(ch->equipment[WIELD]->obj_flags.type_flag == ITEM_WEAPON)) {
	wielded = ch->equipment[WIELD];
	w_type = weapon_type(wielded);
    }
    else {
	if (IS_NPC(ch) && (ch->specials.attack_type >= TYPE_HIT))
	    w_type = ch->specials.attack_type;
	else
	    w_type = TYPE_HIT;
    }

    /* attack misses */
    if (AWAKE(victim) && miss ) {
	if (type == SKILL_BACKSTAB)
	    damage(ch, victim, 0, SKILL_BACKSTAB);
	else
	    damage(ch, victim, 0, w_type);
	return;
    }

    dam = str_app[STRENGTH_APPLY_INDEX(ch)].todam;
    dam += GET_DAMROLL(ch);
    if (wielded)
	dam += dice(wielded->obj_flags.value[1], wielded->obj_flags.value[2]);

    if (GET_CLASS(ch) == CLASS_WARRIOR)
	dam += warrior_damage(ch); 

    /* lss belt sword */ /* 여의봉 */
    /* NOTE: code moved to special_damage()	*/
    /* NOTE: Add special damage from equipment etc. */
    dam += special_damage( ch, victim, dam );

    if (GET_AC(victim) > 0)
	dam -= (GET_AC(victim) >> 2);
    else
	dam += (GET_AC(victim) >> 1);

    if (GET_POS(victim) < POS_FIGHTING)
	dam *= 1 + (POS_FIGHTING - GET_POS(victim)) / 3;
    /* Position  sitting  x 1.33 */
    /* Position  resting  x 1.66 */
    /* Position  sleeping x 2.00 */
    /* Position  stunned  x 2.33 */
    /* Position  incap    x 2.66 */
    /* Position  mortally x 3.00 */

    if (ch->skills[SKILL_EXTRA_DAMAGING].learned) {
	if (ch->skills[SKILL_EXTRA_DAMAGING].learned >
	    number(1, 500) - 4 * GET_LEVEL(ch)) {
	    /* warrior's extra damaging is more powerful */
	    INCREASE_SKILLED1(ch, victim, SKILL_EXTRA_DAMAGING);
	    if (ch->player.class == CLASS_WARRIOR)
		dam += (dam * ((GET_SKILLED(ch, SKILL_EXTRA_DAMAGING) >> 5) + 1));
	    else
		dam += number(dam >> 1, dam * ((GET_SKILLED(ch, SKILL_EXTRA_DAMAGING) >> 5) + 1));
	}
    }

    if (IS_AFFECTED(ch, AFF_LOVE))
	dam *= 2;
    if (IS_AFFECTED(ch, AFF_DEATH))
	dam *= 4;

    /* NOTE: Rewrite serial 'if' clause to more efficent 'switch' clause. */
    /* each class has dedicated weapon */
    rn = number(1, 10);
    switch (GET_CLASS(ch)) {
    case CLASS_THIEF:
	if (w_type == TYPE_PIERCE && rn > 5)
	    dam *= 2;
	break;
    case CLASS_MAGIC_USER:
	if (w_type == TYPE_BLUDGEON && rn > 5)
	    dam *= 2;
	break;
    case CLASS_WARRIOR:
	if (w_type == TYPE_SLASH && rn > 5)
	    dam *= 2;
	break;
    case CLASS_CLERIC:
	/* NOTE: Cleric do more damages with crush weapon, too */
	if ((w_type == TYPE_CRUSH || w_type == TYPE_BLUDGEON) && rn > 5)
	    dam *= 2;
	break;
    }

    if (IS_AFFECTED(ch, AFF_SHADOW_FIGURE) && number(1, 10) > 6)
	/* NOTE: dam *= 3/2;  Dubious integer division. */
	dam = (dam * 3) / 2;

    dam = MAX(1, dam);

#ifdef  NO_DEF 
    if (type == SKILL_BACKSTAB) {
	if (IS_AFFECTED(ch, AFF_HIDE))
	    dam <<= 1;
	if (IS_NPC(ch)) {
	    dam *= backstab_mult[GET_LEVEL(ch) / 2];
	}
	else {		/* PC */
	    dam *= (backstab_mult[GET_LEVEL(ch)] + (GET_SKILLED(ch, SKILL_BACKSTAB) >> 2));
	}
	if (!IS_AFFECTED(victim, AFF_REFLECT_DAMAGE))
	    damage(ch, victim, dam << 1, SKILL_BACKSTAB);
	else {
	    if (ch->skills[SPELL_REFLECT_DAMAGE].learned > number(1, 800)) {
		INCREASE_SKILLED1(victim, ch, SPELL_REFLECT_DAMAGE);
		act("You reflect damage on $N succesfully.",
		    TRUE, ch, 0, victim, TO_CHAR);
		act("$n reflects damage on $N succesfully.",
		    TRUE, victim, 0, ch, TO_ROOM);
		dam >>= 2;
		dam = MAX(1, dam);
		/* if (GET_GUILD(victim) == ASSASSIN && number(1,300) < 50 )
		       dam += dice(((GET_LEVEL(ch) - 20) >> 2) + 1,
			   victim->player.guild_skills[
				ASSASSIN_SKILL_IMPROVED_REFLECT_DAMAGE]); */
		damage(victim, ch, dam, w_type);
	    }
	    else
		damage(ch, victim, dam << 1, SKILL_BACKSTAB);
	}
    }
    else {
	if (!IS_AFFECTED(victim, AFF_REFLECT_DAMAGE))
	    damage(ch, victim, dam, w_type);
	else {
	    if (ch->skills[SPELL_REFLECT_DAMAGE].learned > number(1, 400)) {
		INCREASE_SKILLED1(victim, ch, SPELL_REFLECT_DAMAGE);
		act("You reflect damage on $N successfully.",
		    TRUE, ch, 0, victim, TO_CHAR);
		act("$n reflects damage on $N successfully.",
		    TRUE, victim, 0, ch, TO_ROOM);
		dam >>= 2;
		/* if (GET_GUILD(victim) == ASSASSIN && number(1,100) < 50 )
		       dam += victim->player.guild_skills[
				   ASSASSIN_SKILL_IMPROVED_REFLECT_DAMAGE]; */
		damage(victim, ch, dam, w_type);
	    }
	    else
		damage(ch, victim, dam, w_type);
	}
    }
#endif		/* NO_DEF */

    /* NOTE: Duplicative and confusing code, Clean up the code */ 
    if ( type == SKILL_BACKSTAB ) {
	/* NOTE:  Same damage formular of backstab multipiler for PC/NPC.
		More NPC backstab damage. Less PC backstab damage  */
	/* NOTE: for PC: OLD : mult[] + skill/4 -> NEW: mult[] + skill/8. */
	dam *= (backstab_mult[GET_LEVEL(ch)] + (GET_SKILLED(ch, SKILL_BACKSTAB)/8 ));
	if (IS_AFFECTED(ch, AFF_HIDE)) 
	    dam  *= 2;
    }

    /* NOTE: Unified code for backstab/non-backstab reflect damage. */
    if (IS_AFFECTED(victim, AFF_REFLECT_DAMAGE) 
	 && ((GET_LEARNED(victim, SPELL_REFLECT_DAMAGE) +
	   GET_SKILLED(victim, SPELL_REFLECT_DAMAGE))/2 > number(1, 400))) {
	INCREASE_SKILLED1(victim, ch, SPELL_REFLECT_DAMAGE);
	act("You reflect damage on $N succesfully.",
	    TRUE, ch, 0, victim, TO_CHAR);
	act("$n reflects damage on $N succesfully.",
	    TRUE, victim, 0, ch, TO_ROOM);
	dam /= 4 ;
	/* if (GET_GUILD(victim) == ASSASSIN && number(1,300) < 50 )
	    dam += dice(((GET_LEVEL(ch) - 20) >> 2) + 1,
		victim->player.guild_skills[
		ASSASSIN_SKILL_IMPROVED_REFLECT_DAMAGE]);
	*/
	damage(victim, ch, dam, w_type);
    } 
    else if ( type == SKILL_BACKSTAB )
	/* NOTE: No * 2 damage multiplication for backstab */ 
	damage(ch, victim, dam, SKILL_BACKSTAB);
    else
	damage(ch, victim, dam , w_type);
}

/* control the fights going on */
void perform_violence(void)
{
    struct char_data *ch;
    int i, dat = 100;
    extern void magic_weapon_hit();
    struct obj_data *weapon, *held;
    int percent;

    for (ch = combat_list; ch; ch = combat_next_dude) {
	combat_next_dude = ch->next_fighting;
	/* assert(ch->specials.fighting); */
	/* NOTE: FOR DEBUGGING. This assertion fails sometimes. Why? */
	if (!ch->specials.fighting) {
	    log("perform_violence(): No ch->fighting!!");
	    return;
	}

	if (AWAKE(ch) && (ch->in_room == ch->specials.fighting->in_room)) {
	    weapon = ch->equipment[WIELD];
	    if (weapon && weapon->obj_flags.gpd > 0 && weapon->obj_flags.value[0] > 0 &&
		IS_SET(weapon->obj_flags.extra_flags, ITEM_MAGIC))
		magic_weapon_hit(ch, ch->specials.fighting, weapon);
	    held = ch->equipment[HOLD];
	    if ((GET_CLASS(ch) == CLASS_WARRIOR)&& held && number(1, 10) > 5 
		&& held->obj_flags.gpd > 0 && held->obj_flags.value[0] > 0 
		&& IS_SET(held->obj_flags.extra_flags, ITEM_MAGIC) 
		&& CAN_WEAR(held, ITEM_WIELD)) 
		magic_weapon_hit(ch, ch->specials.fighting, held); 

	    hit(ch, ch->specials.fighting, TYPE_UNDEFINED);
	    if (!IS_NPC(ch)) {
		/* double attack */
		if (ch->skills[SKILL_DOUBLE_ATTACK].learned > 0) {
		    percent = GET_LEARNED(ch, SKILL_DOUBLE_ATTACK)
			+ GET_SKILLED(ch, SKILL_DOUBLE_ATTACK)
			+ 3 * GET_LEVEL(ch);
		    if (percent > number(1, 200)) {
			/* if(ch->skills[SKILL_DOUBLE_ATTACK].learned
			    > number(1,200)-4*GET_LEVEL(ch)) */
			hit(ch, ch->specials.fighting, TYPE_UNDEFINED);
			INCREASE_SKILLED1(ch, ch, SKILL_DOUBLE_ATTACK);
		    }
		}

		if (ch->skills[SKILL_QUADRUPLE_ATTACK].learned > 0) {
		/* NOTE: Equal hit chance for all class.   */
		/*
		    if (GET_CLASS(ch) == CLASS_MAGIC_USER 
			|| GET_CLASS(ch) == CLASS_CLERIC)
			if (number(1, 40) > 
			    20 + (GET_SKILLED(ch, SKILL_QUADRUPLE) >> 3))
			    goto octa;
		*/ 
		/* NOTE: OLD percent for octa and quadruple attack :
		    percent = LEARNED + SKILLED << 1 + 6 * LEVEL; */ 

		    percent = GET_LEARNED(ch, SKILL_QUADRUPLE_ATTACK)
			+ (GET_SKILLED(ch, SKILL_QUADRUPLE_ATTACK))
			+ 3 * GET_LEVEL(ch) + GET_DEX(ch)* 5 ;
		    INCREASE_SKILLED1(ch, ch, SKILL_QUADRUPLE_ATTACK);
		    for (i = 0; i < 2; i++) {
			if (percent > number(1, 450))
			    hit(ch, ch->specials.fighting, TYPE_UNDEFINED);
		    }
		}
/* octa: */
		if (ch->skills[SKILL_OCTA_ATTACK].learned > 0) {
		/* NOTE: Equal hit chance for all class.  */
		/*
		    if (GET_CLASS(ch) == CLASS_MAGIC_USER 
			    || GET_CLASS(ch) == CLASS_CLERIC)
			if (number(1, 40) > 10 + (GET_SKILLED(ch, SKILL_OCTA) >> 3))
			    goto octa;  // NOTE: Should it be next ???
		*/ 
		/* NOTE: OLD percent for octa and quadruple attack :
		    percent = LEARNED + SKILLED << 1 + 6 * LEVEL; */ 

		    percent = GET_LEARNED(ch, SKILL_OCTA_ATTACK)
			+ (GET_SKILLED(ch, SKILL_OCTA_ATTACK))
			+ 3 * GET_LEVEL(ch) + GET_DEX(ch)*5;
		    INCREASE_SKILLED1(ch, ch, SKILL_OCTA_ATTACK);
		    for (i = 0; i < 4; i++) {
			if (percent > number(1, 450))
			    hit(ch, ch->specials.fighting, TYPE_UNDEFINED);
		    }
		}
	    }
/* next: */
	    if (IS_NPC(ch)) { 
		/* NOTE: Original formular for NPC octa attack. */
		/*     dat = ( !ACT_CLERIC && !ACT_MAGE ) ? 7 : 15;
		     for(i=dat;i<GET_LEVEL(ch);i+=dat) hit();  */ 

		if (!IS_SET(ch->specials.act, ACT_CLERIC) &&
		    !IS_SET(ch->specials.act, ACT_MAGE))
		    dat = ( GET_LEVEL(ch)/2+ GET_DEX(ch)-4)/8;
		else
		    dat = ( GET_LEVEL(ch)/2+ GET_DEX(ch)-4)/5;
		for (i = 0; i < dat  ; i++ )
		    hit(ch, ch->specials.fighting, TYPE_UNDEFINED);
	    }

	    if (IS_AFFECTED(ch, AFF_HASTE)) {
		hit(ch, ch->specials.fighting, TYPE_UNDEFINED);
		hit(ch, ch->specials.fighting, TYPE_UNDEFINED);
	    }
	    if (IS_AFFECTED(ch, AFF_IMPROVED_HASTE)) {
		hit(ch, ch->specials.fighting, TYPE_UNDEFINED);
		hit(ch, ch->specials.fighting, TYPE_UNDEFINED);
		hit(ch, ch->specials.fighting, TYPE_UNDEFINED);
	    }

	    if (ch->equipment[WEAR_FEET] 
		    && ch->equipment[WEAR_FEET]->item_number >= 0 
		    && GET_OBJ_VIRTUAL(ch->equipment[WEAR_FEET]) == 2012) {
		    /* SPEED boots */
		hit(ch, ch->specials.fighting, TYPE_UNDEFINED);
		hit(ch, ch->specials.fighting, TYPE_UNDEFINED);
		hit(ch, ch->specials.fighting, TYPE_UNDEFINED);
	    }
	}
	else {		/* Not in same room */
	    stop_fighting(ch);
	}
    }
}

