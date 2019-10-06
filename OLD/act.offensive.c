/* ************************************************************************
   *  file: act.offensive.c , Implementation of commands.    Part of DIKUMUD *
   *  Usage : Offensive attack commands. 				     *
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
#include "actions.h"

/* NOTE: NEW!  Get victim of given name and check it is valid victim.
        If not valid, print error message and return NULL.
       	Called by do_backstab(), do_bash(), do_kick(), do_punch(), do_flash(),
       	do_shoot(), do_shouryuken(), do_spin_bird_kick(), do_throw_object(). */
struct char_data *get_victim(struct char_data *ch, char *argument, int cmd)
{
    char name[MAX_INPUT_LENGTH];
    struct char_data *victim;

    one_argument(argument, name);

    /* if ((GET_LEVEL(ch) == IMO) && (!IS_NPC(ch))) { */
    if ((GET_LEVEL(ch) >= IMO) && (!IS_NPC(ch))) 
	return NULL;

    if ((!name[0] || !(victim = get_char_room_vis(ch, name)))
	    && !(victim = ch->specials.fighting )) {
	if (cmd == CMD_KICK )
	    send_to_char("Kick who?\n\r", ch);
	else if (cmd == CMD_BACKSTAB )
	    send_to_char("backstab who?\n\r", ch);
	else if (cmd == CMD_BASH )
	    send_to_char("Bash who?\n\r", ch);
	else if (cmd == CMD_PUNCH )
	    send_to_char("punch who?\n\r", ch);
	else if (cmd == CMD_FLASH )
	    send_to_char("flash who?\n\r", ch);
	else if (cmd == CMD_SHOOT )
	    send_to_char("Shoot whom?\n\r", ch);
	else if (cmd == CMD_SPIN_BIRD_KICK )
	    send_to_char_han("Kick who?", "누굴 때려요?\n\r", ch);
	else if (cmd == CMD_SHOURYUKEN )
	    send_to_char("누굴 때려?\n\r", ch);
	else if (cmd == CMD_THROW )
	    send_to_char("throw what to whom?\n\r", ch);
	else
	    send_to_char("Attack who?\n\r", ch);
	return NULL;
    } 
    else if (victim == ch) {
	if (cmd == CMD_BACKSTAB )
	    send_to_char("How can you sneak up on yourself?\n\r", ch);
	else if (!IS_NPC(ch) && cmd == CMD_SHOOT)
	    send_to_char("Shoot yourself? Nah...\n\r", ch);
	else if ( cmd == CMD_SPIN_BIRD_KICK || cmd == CMD_SHOURYUKEN )
	    send_to_char("앗! 나의 실수 ~~~ ...\n\r", ch);
	else if (cmd == CMD_THROW )
	    send_to_char("Come on now, that's rather stupid!\n\r", ch);
	else
	    send_to_char("Aren't we funny today...\n\r", ch);
	return NULL;
    } 
    else if (!IS_NPC(victim) && !(victim->desc)) {
	if (cmd == CMD_THROW)
	    send_to_char("throw something to ghost? No way!\n\r",ch);
	else
	    send_to_char_han("Attack ghost? No way!\n\r", 
		"유령을 치겠다고? 미친소리!\n\r", ch); 
	return NULL;
    }
    else if ((cmd == CMD_SHOOT) 
	    && (GET_LEVEL(victim) >= IMO) && (!IS_NPC(victim))) {
	if (!IS_NPC(ch))
	    send_to_char("Shoot an immortal?  Never.\n\r", ch);
	return NULL;
    }

    return victim;
} 

void do_hit(struct char_data *ch, char *argument, int cmd)
{
    char arg[MAX_INPUT_LENGTH];
    struct char_data *victim;
    extern int nokillflag;

    one_argument(argument, arg);

    if ((GET_LEVEL(ch) == IMO) && (!IS_NPC(ch))) 
	return;
    if (!*arg) {
	send_to_char("Hit whom?\n\r", ch);
	return;
    } 
    victim = get_char_room_vis(ch, arg);
    if (!victim) {
	send_to_char("They aren't here.\n\r", ch);
	return;
    }
    if (victim == ch) {
	send_to_char("You hit yourself..OUCH!.\n\r", ch);
	act("$n hits $mself, and says OUCH!",
	    FALSE, ch, 0, victim, TO_ROOM);
	return;
    }

    if (nokillflag && !IS_NPC(ch) && !IS_NPC(victim))
	return;

    /* NOTE: Forbid hitting member of same guild. */
    /* NOTE: But permit hitting mob of same guild */
    if (!IS_NPC(ch) && !IS_NPC(victim) && victim->player.guild
	&& (ch->player.guild == victim->player.guild)) {
	send_to_char("Your peer seems uninterested.\r\n", ch);
	return;
    }

    /* NOTE: Prevent hitting player in same group   */
    /* NOTE: Hitting grouped mob permited           */
    if (!IS_NPC(ch) && !IS_NPC(victim)
	&& IS_AFFECTED(ch, AFF_GROUP) 
	&& IS_AFFECTED(victim, AFF_GROUP)) {
	struct char_data *ch_group, *vic_group;

	ch_group = ch->master ? ch->master : ch;
	vic_group = victim->master ? victim->master : victim;

	if (ch_group == vic_group) {
	    send_to_char( "Your friend seems frightened, glancing at you.\r\n", ch);
	    act("$n tried to hit you by mistake and feels sorry"
		" about that.", FALSE, ch, 0, victim, TO_VICT);
	    act("$n tried to attack $N by mistake. $N seems frightened about that.",
		TRUE, ch, 0, victim, TO_NOTVICT);
	    return;
	}
    }

    /* forbid charmed mob hit player */
    if (ch && IS_AFFECTED(ch, AFF_CHARM) && (!IS_NPC(victim)))
	return;
    /* forbid charmed mob hit charmed mob */
    if (ch && victim && IS_AFFECTED(ch, AFF_CHARM)
	    && IS_AFFECTED(victim, AFF_CHARM))
	return;

    if ((GET_POS(ch) == POS_STANDING)
	    && (victim != ch->specials.fighting)) {
	hit(ch, victim, TYPE_UNDEFINED);
	if (!IS_NPC(ch))
	    WAIT_STATE(ch, PULSE_VIOLENCE + 2);
	    /* HVORFOR DET?? */
    }
    else
	send_to_char("You do the best you can!\n\r", ch);
}

void do_kill(struct char_data *ch, char *argument, int cmd)
{
    static char arg[MAX_BUFSIZ];
    char buf[70];
    struct char_data *victim;

    if (GET_LEVEL(ch) >= IMO && GET_LEVEL(ch) < IMO + 3) {
	sprintf(buf, "Fight IMO:%s v.s. %s", GET_NAME(ch), argument);
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
    }
    else {
	if (!(victim = get_char_room_vis(ch, arg)))
	    send_to_char("They aren't here.\n\r", ch);
	else if (ch == victim)
	    send_to_char("Your mother would be so sad.. :(\n\r", ch);
	else {
	    if (GET_LEVEL(ch) < IMO + 3 && GET_LEVEL(victim) >= (IMO + 3))
		return;
	    act("You chop $M to pieces! Ah! The blood!",
		FALSE, ch, 0, victim, TO_CHAR);
	    act("$N chops you to pieces!", FALSE, victim, 0, ch, TO_CHAR);
	    act("$n brutally slays $N", FALSE, ch, 0, victim, TO_NOTVICT);
	    die(victim, GET_LEVEL(ch), ch);
	}
    }
}

void do_backstab(struct char_data *ch, char *argument, int cmd)
{
    struct char_data *victim;
    byte percent;

    victim = get_victim(ch, argument, cmd);
    if ( !victim )
	return;

    if (!IS_NPC(ch)) {
	if (!ch->equipment[WIELD]) 
	    send_to_char(
		"You need to wield a weapon, to make it a succes.\n\r", ch); 
	else if (ch->equipment[WIELD]->obj_flags.value[3] != 11) 
	    send_to_char(
		"Only piercing weapons can be used for backstabbing.\n\r", ch);
	else if (ch->specials.fighting) 
	    send_to_char(
		"You can't backstab while fighting, too alert!\n\r", ch);
	/* NOTE: check mana and move */
	else if (GET_MOVE(ch) <= 0 || GET_MANA(ch) <= 0)
		send_to_char("You are too exhausted to backstab.\n\r", ch);
	/* NOTE: Doubious code. Removed */
	/* else if (GET_CLASS(ch) <= 20) return; */ 
	else
	    goto ok;
	return;
    }
ok:
    percent = number(1, 101) + (GET_LEVEL(victim) << 1) - GET_LEVEL(ch)
	- GET_DEX(ch);	/* 101% is a complete failure */

    if (AWAKE(victim) && (percent > ch->skills[SKILL_BACKSTAB].learned)) {
	/* NOTE: Damage on failure */
	GET_HIT(ch) -= MIN(GET_MAX_HIT(ch)/number(30, 40), 1000) ;
	/* NOTE: Add half deley on failure */
	WAIT_STATE(ch, PULSE_VIOLENCE/2);

	damage(ch, victim, 0, SKILL_BACKSTAB);
    }
    else {
	/* NOTE: similar MANA, MOVE consumption as shouryuken */ 
	GET_MOVE(ch) -= 400 - GET_SKILLED(ch, SKILL_BACKSTAB);
	GET_MANA(ch) -= 250 - GET_SKILLED(ch, SKILL_BACKSTAB);
	/* NOTE: Add full deley on backstab success. */
	WAIT_STATE(ch, PULSE_VIOLENCE);

	INCREASE_SKILLED(ch, victim, SKILL_BACKSTAB);
	hit(ch, victim, SKILL_BACKSTAB);
    }
}

void do_bash(struct char_data *ch, char *argument, int cmd)
{
    struct char_data *victim;
    int percent;
    int dam;

    victim = get_victim(ch, argument, cmd);
    if ( !victim )
	return;

/* NOTE: Dyaus and Tido thinks 
     mob (e.g. jangkak ) shoud not bash player without wielding weapon.   */
//      if (/* !IS_NPC(ch) && */ !ch->equipment[WIELD]) {
/* NOTE: Cookie thinks PC/NPC should be able to bash without wielding weapon. */
/*
    if (IS_NPC(ch) && !ch->equipment[WIELD])
	return;
    if (!IS_NPC(ch) && !ch->equipment[WIELD]) {
	send_to_char("You need to wield a weapon to bash.\n\r", ch);
	return;
    }
*/
    percent = number(1, 101) + GET_LEVEL(victim) - GET_LEVEL(ch);
    if (percent > ch->skills[SKILL_BASH].learned) {
	damage(ch, victim, 0, SKILL_BASH);
	GET_POS(ch) = POS_SITTING;
	WAIT_STATE(ch, PULSE_VIOLENCE * 2);
    }
    else {
	INCREASE_SKILLED(ch, victim, SKILL_BASH);
	WAIT_STATE(victim,
		    PULSE_VIOLENCE * (2 + (GET_SKILLED(ch, SKILL_BASH) >> 5)));
	dam = GET_LEVEL(ch) * (10 + GET_SKILLED(ch, SKILL_BASH));
	damage(ch, victim, dam, SKILL_BASH);

	percent = number(1, IMO << 1);
	if (percent < GET_LEVEL(ch))
	    GET_POS(victim) = POS_STUNNED;
	else
	    GET_POS(victim) = POS_SITTING;
    }
}

void do_multi_kick(struct char_data *ch, char *argument, int cmd)
{
    struct char_data *victim;
    int percent;
    int i;
    int dam;

    victim = ch->specials.fighting;
    if (!victim) {
	send_to_char("This skill can be used only in fighting!\n\r", ch);
	return;
    }
    else if (GET_MOVE(ch) < 100) {
	send_to_char("You are too exhausted!\n\r", ch);
	return;
    }
    else if (victim == ch) {
	send_to_char("Aren't we funny today...\n\r", ch);
	return;
    }

    GET_MOVE(ch) -= ((IMO << 1) - GET_LEVEL(ch));
    dam = GET_LEVEL(ch) * (1 + (GET_SKILLED(ch, SKILL_KICK) >> 2));
    dam = number(dam, dam << 1);

    if (saves_spell(victim, SAVING_HIT_SKILL))
	dam >>= 1;

    INCREASE_SKILLED(ch, victim, SKILL_MULTI_KICK);

    i = 2 + (GET_LEVEL(ch) >> 3) + (GET_SKILLED(ch, SKILL_MULTI_KICK) >> 4);
    for (; i; i--) {
	percent = ((200 - GET_AC(victim) - GET_HITROLL(ch)) >> 5)
		    + number(1, 101);

	/* NOTE: Check victim location */
	if ( ch->in_room != victim->in_room )
	    break; 
	if (percent > GET_LEARNED(ch, SKILL_MULTI_KICK))
	    damage(ch, victim, 0, SKILL_MULTI_KICK);
	else 
	    damage(ch, victim, dam, SKILL_MULTI_KICK);
    }
    WAIT_STATE(ch, PULSE_VIOLENCE); 
}

void do_kick(struct char_data *ch, char *argument, int cmd)
{
    struct char_data *victim;
    int percent;
    int dam;

    victim = get_victim(ch, argument, cmd );
    if(!victim)
	return;

    percent = ((200 - GET_AC(victim) - GET_HITROLL(ch)) >> 5) + number(1, 101);
    WAIT_STATE(ch, PULSE_VIOLENCE);
    if (percent > ch->skills[SKILL_KICK].learned) {
	damage(ch, victim, 0, SKILL_KICK);
    }
    else {
	INCREASE_SKILLED(ch, victim, SKILL_KICK);
	WAIT_STATE(victim, PULSE_VIOLENCE);
	dam = GET_LEVEL(ch) * ( 5 + (GET_SKILLED(ch, SKILL_KICK) >> 2));
	dam = number(dam, dam << 1);
	if (saves_spell(victim, SAVING_HIT_SKILL))
	    dam >>= 1;
	damage(ch, victim, dam, SKILL_KICK);
    }
}

/* Perhaps made */
void do_punch(struct char_data *ch, char *argument, int cmd)
{
    struct char_data *victim;
    int dam;
    int percent;

    victim = get_victim(ch, argument, cmd );
    if(!victim)
	return;

    if (GET_MANA(ch) < 200) {
	send_to_char("You do not have enough mana!!!\n\r", ch);
	return;
    }
    /* NOTE: Now, you can 'punch' while wielding weapon. */
    /*
    if (!IS_NPC(ch) && ch->equipment[WIELD]) {
	send_to_char("You must unwield to make it success!!!\n\r", ch);
	return;
    }
    */
/* NOTE:  Lower punch damage to 1/3 of old ( 83/250 = 33%)
        OLD:  dam = (LEARNED/2 + SKILLED*2) * number(LEVEL, LEVEL*2);
        NEW:  dam = (LEARNED/3 + SKILLED/2) * number(LEVEL, LEVEL*2); */

    dam = (GET_LEARNED(ch, SKILL_PUNCH) /3 + GET_SKILLED(ch, SKILL_PUNCH) /2)
		    * number(GET_LEVEL(ch), GET_LEVEL(ch) *2);

    percent = ((300 - GET_AC(victim) - GET_HITROLL(ch)
		    - GET_SKILLED(ch, SKILL_PUNCH)) >> 4) + number(1, 101);
    WAIT_STATE(ch, PULSE_VIOLENCE);
    if (percent > ch->skills[SKILL_PUNCH].learned) {
        /* NOTE: To print punch damage message, use fight_message() in damage() */ 
	/*
	send_to_char("You failed to punch him WHAT a DAMN!!!!!\n\r", ch);
	act("$n failed to punch down $N!!!", TRUE, ch, 0, victim, TO_ROOM);
	*/
	damage(ch, victim, 0, SKILL_PUNCH);
	return;
    }
    else {
	INCREASE_SKILLED(ch, victim, SKILL_PUNCH);
	/*
	send_to_char("You send a big punch ORURURURURURURURURU!!!!!!\n\r", ch);
	act("$n sends a POWERFUL PUNCH!!! ORURURURURURURURU!!!!!",
	    TRUE, ch, 0, 0, TO_ROOM);
	*/
	damage(ch, victim, dam, SKILL_PUNCH);
    }
    GET_MANA(ch) -= 200;
}

void do_tornado(struct char_data *ch, char *argument, int cmd)
{
    struct char_data *tch, *tch2;

    if ((GET_LEVEL(ch) == IMO) && (!IS_NPC(ch))) {
	return;
    }
    for (tch = world[ch->in_room].people; tch; tch = tch2) {
	tch2 = tch->next_in_room;
	if ((IS_NPC(ch) ^ IS_NPC(tch)) && ch->points.move > 0) {
	    /* if(ch->skills[SKILL_TORNADO].learned>number(1,99)) { */
	    if (GET_LEARNED(ch, SKILL_TORNADO)
		    + (GET_SKILLED(ch, SKILL_TORNADO) >> 3) > number(1, 111)) {
		hit(ch, tch, TYPE_UNDEFINED);
	    }
	}
    }
    if (ch->points.move > 0)
	ch->points.move -= (IMO - GET_LEVEL(ch) + 2);
    INCREASE_SKILLED(ch, ch, SKILL_TORNADO);
    if (!IS_NPC(ch))
	WAIT_STATE(ch, PULSE_VIOLENCE * 1);
}

void do_flash(struct char_data *ch, char *argument, int cmd)
{
    struct char_data *victim;
    int percent;
    int d;

    /* NOTE:  if (level < 6) return: Removed. */
    victim = get_victim(ch, argument, cmd );
    if(!victim)
	return;

    /*
    if (!IS_NPC(victim) && !IS_NPC(ch) && GET_LEVEL(ch) < IMO) {
        send_to_char("You can't use flash to player\n\r",ch);
	return;
    }
    */

    percent = number(1, 101) + GET_LEVEL(victim) * 2 - GET_LEVEL(ch)
	- (GET_SKILLED(ch, SKILL_FLASH) >> 2);

    if (!IS_NPC(ch))
	/* WAIT_STATE(ch, PULSE_VIOLENCE/2); */
	WAIT_STATE(ch, PULSE_VIOLENCE);

    if (percent > ch->skills[SKILL_FLASH].learned) {
    /* NOTE: To print flash damage message, use fight_message() in damage() */ 
	/*
	send_to_char("You can't get chance ...\n\r", ch);
	act("$n try flash attack $N, but failed."
	    ,FALSE, ch, 0, victim, TO_NOTVICT);
	*/
	damage(ch, victim, 0, SKILL_FLASH);
    }
    else { 
	/* NOTE: Easier INCREASE_SKILLED() */;
	INCREASE_SKILLED2(ch, victim, SKILL_FLASH);

	/* flash is enhenced by about 3.1 times of old */
	/* NOTE: OLD: d = GET_MOVE(ch) * 
			(2 + (GET_SKILLED(ch, SKILL_FLASH) >> 5)) / 5; */
	d = GET_MOVE(ch) 
	    * (GET_SKILLED(ch, SKILL_FLASH) / 2 + GET_DEX(ch) * 2 + 16 ) / 32; 
	ch->points.move = 0;

	/* messages */
	/* send_to_char("You attack like flash ...\n\r", ch); */
	/*
	send_to_char("You can see star above your head ?!? , you are confused!\r\n",
		     victim); 
	act("$n moves like flash and attack with flash power!!! ",
	    FALSE, ch, 0, victim, TO_NOTVICT);
	*/
	act("$n strike $N's head with circular spanning triple kick .",
	    FALSE, ch, 0, victim, TO_NOTVICT);
	act("$n throw $N into sun with double tumbling.",
	    FALSE, ch, 0, victim, TO_NOTVICT);
	act("$n jump kick down  $N's heart with making beautifle circle.",
	    FALSE, ch, 0, victim, TO_NOTVICT);
	act("$n punch $N's head line drive.",
	    FALSE, ch, 0, victim, TO_NOTVICT);
	damage(ch, victim, d, SKILL_FLASH);
    }
}

void shoot(struct char_data *ch, struct char_data *victim, int type)
{
    byte percent;
    int dam;

    if (!ch->equipment[HOLD] ||		/* Perhaps modified */
	GET_ITEM_TYPE(ch->equipment[HOLD]) != ITEM_FIREWEAPON) {
	if (!IS_NPC(ch))
	    send_to_char("To shoot, you need to HOLD a firing weapon.\n\r", ch);
	return;
    }
    if (ch->equipment[HOLD]->obj_flags.value[0] <= 0) {
	if (!IS_NPC(ch))
	    send_to_char("Oops.  Nothing to shoot.\n\r", ch);
	act("Hmmm.  $n fires an empty $p.",
	    FALSE, ch, ch->equipment[HOLD], 0, TO_ROOM);
	return;
    }
    if ((GET_LEVEL(ch) < (IMO + 3)) &&
	((ch && !IS_NPC(ch))
	    || (ch && IS_NPC(ch) && IS_AFFECTED(ch, AFF_CHARM))))
	ch->equipment[HOLD]->obj_flags.value[0]--;
    if ((!IS_NPC(ch)) && (GET_LEVEL(ch) >= (IMO + 3)))
	percent = 0;
    else
	percent = number(1, 101) + GET_LEVEL(victim) - GET_LEVEL(ch);
    if (percent < 60) {
	act("Thwapp! You shoot $M with $p.",
	    FALSE, ch, ch->equipment[HOLD], victim, TO_CHAR);
	act("Thwapp! You are shot by $n with $p.",
	    FALSE, ch, ch->equipment[HOLD], victim, TO_VICT);
	act("Thwapp! $n shoots $N with $p.",
	    FALSE, ch, ch->equipment[HOLD], victim, TO_NOTVICT);
	dam = ch->equipment[HOLD]->obj_flags.value[2];
	if ((!IS_NPC(ch)) 
	    && (GET_GUILD(ch) == OUTLAW || GET_GUILD(ch) == POLICE))
	    dam = dam * 2;
	if (!IS_NPC(victim))
	    WAIT_STATE(victim, PULSE_VIOLENCE * 2);
	damage(ch, victim, dam, TYPE_SHOOT);
	if (GET_LEVEL(ch) < (IMO + 3))
	    if (!IS_NPC(ch))
		WAIT_STATE(ch, PULSE_VIOLENCE * 2);
    }
    else {
	act("You try to shoot $M with $p, but miss.",
	    FALSE, ch, ch->equipment[HOLD], victim, TO_CHAR);
	act("You are shot at by $n with $p, but missed.",
	    FALSE, ch, ch->equipment[HOLD], victim, TO_VICT);
	act("$n tries to shoot $N with $p, but misses.",
	    FALSE, ch, ch->equipment[HOLD], victim, TO_NOTVICT);
    }
}

void do_shoot(struct char_data *ch, char *argument, int cmd)
{
    struct char_data *victim;

    victim = get_victim(ch, argument, cmd);
    if ( !victim )
	return;

    shoot(ch, victim, TYPE_SHOOT);
}


/* NOTE: NEW! Calculate single hit damage dice of warrior. 
This is used to calculate shouryuken/spin bird kick damage. */ 
int warrior_total_damage(struct char_data *ch)
{
    int dam;
    struct obj_data *wielded, *held;
    wielded = ch->equipment[WIELD];
    held = ch->equipment[HOLD];

    dam = str_app[STRENGTH_APPLY_INDEX(ch)].todam;
    dam += GET_DAMROLL(ch);
    if (wielded) 
	dam += dice(wielded->obj_flags.value[1], wielded->obj_flags.value[2]);
    else
	dam += dice(ch->specials.damnodice, ch->specials.damsizedice); 

    if ( held && CAN_WEAR( held, ITEM_WIELD) && number(1, 10) > 4 )
	dam += dice(held->obj_flags.value[1], held->obj_flags.value[2]);
    return(dam);
}

void do_spin_bird_kick(struct char_data *ch, char *argument, int cmd)
{
    unsigned int dam;
    struct char_data *victim;
    int percent;
    int tmp, i;
    int level_plus;

    if (GET_SEX(ch) != SEX_FEMALE) {
	send_to_char("여자 전용 스킬입니다. 성전환을 하는게 어떨지~~\n\r", ch); 
	return;
    }
    
    victim = get_victim(ch, argument, cmd);
    if ( !victim )
	return; 
    else if (GET_MANA(ch) < 100) {
	send_to_char("You do not have enough mana.\n\r", ch); 
	return;
    }
    else if (GET_MOVE(ch) < 0) {
	send_to_char("당신은 너무 지쳐서 시도할 수 가 없군요!\n\r", ch);
	do_say(ch, "학학학...난 너무 지쳤어!!!\n\r", 0);
	return;
    }

    do_say(ch, "스핀~~~버드~~~킥!!!", 0);
    if (!AWAKE(victim)) {
	send_to_char("자는 사람을 치다니 비겁한 년!!\n\r", ch);
	/* NOTE: You can attack shouryuken/spin bird kick sleeping victim  */
	/* return; */
    }

    /* NOTE: Equal base dam value to shouryuken */
    /* NOTE: OLD base damage about 1100,   NEW base : 992 */
    /*  dam = (GET_LEVEL(ch) * (number(13,18) 
		+ (GET_SKILLED(ch, SKILL_SPIN_BIRD_KICK) >> 3))) */
    dam = (GET_LEVEL(ch) * (number(13,18) + (GET_SKILLED(ch, SKILL_SPIN_BIRD_KICK) >> 3)) 
	+ warrior_total_damage(ch))/2;

    if (saves_spell(victim, SAVING_HIT_SKILL))
	dam >>= 1;

    /* NOTE: Enhence chance of attack OLD: 30 % -> NEW: 40 %*/
    /* NOTE: OLD percent.
	percent = number(1, 150) - (GET_LEVEL(ch) + GET_DEX(ch)) +
	    2 * (GET_LEVEL(victim) + GET_DEX(victim))
	    - (GET_SKILLED(ch, SKILL_SPIN_BIRD_KICK) >> 3); */

    percent = number(1,101) - ( GET_AC(victim) + GET_HITROLL(ch))/16 
		- (GET_LEVEL(ch) + 2*GET_DEX(ch)) - 10 
		+ 2 * (GET_LEVEL(victim) + 2*GET_DEX(victim))
		- GET_SKILLED(ch, SKILL_SPIN_BIRD_KICK) / 6; 

    if (percent < GET_LEARNED(ch, SKILL_SPIN_BIRD_KICK)) {
	INCREASE_SKILLED(ch, victim, SKILL_SPIN_BIRD_KICK);
	level_plus =  ((GET_LEVEL(ch) - 25)  + GET_SKILLED(ch, SKILL_SPIN_BIRD_KICK))/8;
	tmp = number( 24, 36 + level_plus/2)/4;
	/* NOTE: Reduce point consumption: MANA: 300->250, MOVE 600 -> 400  */
	GET_MOVE(ch) -= 400 - GET_SKILLED(ch, SKILL_SPIN_BIRD_KICK);
	GET_MANA(ch) -= 250 - GET_SKILLED(ch, SKILL_SPIN_BIRD_KICK);
	/* NOTE: Avrge damage = 36 dam(24-48): similar to shouryuken */  
	for (i = 0; i < tmp; i++) {
	    switch (number(1, 9)) {
	    case 1: act("차차차차찻~~~~", TRUE, ch, 0, 0, TO_ROOM); break;
	    case 2: act("요~~~오~~~옷!!", TRUE, ch, 0, 0, TO_ROOM); break;
	    case 3: act("파바바바박~~!!", TRUE, ch, 0, 0, TO_ROOM); break;
	    case 4: act("얍얍얍얍얍!!!!", TRUE, ch, 0, 0, TO_ROOM); break;
	    default: break;
	    }
	    /*
	    if (!(tmp_vic = get_char_room_vis(ch, victim_name))) {
		if (ch->specials.fighting)
		    tmp_vic = ch->specials.fighting;
	    }
	    if (victim == tmp_vic)
		damage(ch, victim, dam, TYPE_HIT);
	    else
		break;
	    */
	    damage(ch, victim, dam*4, TYPE_HIT);
	    /* NOTE: Check victim's location */
	    if(ch->in_room != victim->in_room)
		break;
	}
	send_to_char("하하...내 치맛바람 맛이 어떠냐!!!\n\r", ch);
	WAIT_STATE(ch, PULSE_VIOLENCE);  /* NOTE: Full deley on success */
    }
    else {
	damage(ch, victim, dam, TYPE_HIT);
	send_to_char("잉~~~괜히 속옷만 보여줬다...\n\r", ch);
	WAIT_STATE(ch, PULSE_VIOLENCE/2); /* NOTE: Half-deley on failure */
    }
    /* WAIT_STATE(ch, PULSE_VIOLENCE/3); */
}

/* chase added this for Shou Ryu Ken */
void do_shouryuken(struct char_data *ch, char *argument, int cmd)
{
    unsigned int dam;
    struct char_data *victim;	/* ,*tmp_vic; */
    int percent;
    int tmp, i;
    int level_plus;

    if (GET_SEX(ch) != SEX_MALE) {
	send_to_char("남자 전용 스킬입니다.성전환을 하는게 어떨지~~\n\r", ch);
	return; 
    }
    victim = get_victim(ch, argument, cmd);
    if ( !victim )
	return; 
    else if (GET_MANA(ch) < 100) {
	send_to_char("You do not have enough mana.\n\r",ch);
	return;
    }
    else if (GET_MOVE(ch) < 0) {
	send_to_char("당신은 너무 지쳐서 시도할 수 가 없군요!\n\r", ch);
	do_say(ch, "헥헥헥...난 너무 지쳤어!!!\n\r", 0);
	return;
    }

    do_say(ch, "쇼~~~류~~켄~", 0); 
    if (!AWAKE(victim)) {
	send_to_char("자는 사람을 치다니 비겁한 놈!!\n\r", ch);
	/* NOTE: You can attack shouryuken/spin bird kick sleeping victim  */
	/* return; */
    } 
    /* NOTE: OLD base damage about 1100,   NEW base : 992 */
    /*  dam = (GET_LEVEL(ch) * (number(13, 18) + 
		(GET_SKILLED(ch, SKILL_SHOURYUKEN) >> 3))   */
    dam = (GET_LEVEL(ch) * (number(13, 18) + (GET_SKILLED(ch, SKILL_SHOURYUKEN) >> 3))
	+ warrior_total_damage(ch))/2;  

    if (saves_spell(victim, SAVING_HIT_SKILL))
	dam >>= 1;

    /* NOTE: Enhence chance of attack OLD: 30% -> NEW: 40% */
    /* NOTE: OLD percent.
	percent = number(1, 150) - (GET_LEVEL(ch) + GET_DEX(ch)) +
	    2 * (GET_LEVEL(victim) + GET_DEX(victim))
	    - (GET_SKILLED(ch, SKILL_SHOURYUKEN) >> 3); */

    percent = number(1,101) - ( GET_AC(victim) + GET_HITROLL(ch))/16
		- (GET_LEVEL(ch) + 2*GET_DEX(ch)) -10
		+ 2 * (GET_LEVEL(victim) + 2*GET_DEX(victim))
		- GET_SKILLED(ch, SKILL_SHOURYUKEN) / 6;

    if (percent < ch->skills[SKILL_SHOURYUKEN].learned) {
	INCREASE_SKILLED(ch, victim, SKILL_SHOURYUKEN);
	/*
	level_plus = 10 + (GET_LEVEL(ch) + GET_SKILLED(ch, SKILL_SHOURYUKEN)) / 8;
	tmp = number(10, level_plus);
	*/
	/* NOTE: Av damage = 26 dam -> 26 dam + 9 dam  */ 
	level_plus = (GET_LEVEL(ch)- 25 + GET_SKILLED(ch, SKILL_SHOURYUKEN))/8;
	tmp = number(0, level_plus/3);
	/* NOTE: Reduce point consumption: MANA: 300->250, MOVE 600 -> 400  */
	GET_MOVE(ch) -= (400 - GET_SKILLED(ch, SKILL_SHOURYUKEN));
	GET_MANA(ch) -= (250 - GET_SKILLED(ch, SKILL_SHOURYUKEN));

	/* NOTE: Additional hit damages and messages */
	for(i = 0; i < tmp; i++) {
	    switch(number(1, 9)) {
	    case 1: act("이얍얍얍얍~~~",TRUE,ch,0,0,TO_ROOM); break;
	    case 2: act("끼야오홋!!!!!",TRUE,ch,0,0,TO_ROOM); break;
	    case 3: act("아자자자자~~!!",TRUE,ch,0,0,TO_ROOM); break;
	    case 4: act("으샤샤샤샤!!!!",TRUE,ch,0,0,TO_ROOM); break;
	    default: break;
	    }
	    /*
	    if(!(tmp_vic=get_char_room_vis(ch,victim_name))) {
		if (ch->specials.fighting) 
		    tmp_vic = ch->specials.fighting;
		} 
	    */
	    damage(ch, victim, dam, TYPE_HIT);
	    /* NOTE: Check victim's location */
	    if (ch->in_room != victim->in_room) goto end; 
	} 

	act("쇼오오오~~", TRUE, ch, 0, 0, TO_ROOM);
	damage(ch, victim, dam, TYPE_HIT);

	/* NOTE: Check victim's location */
	if (ch->in_room != victim->in_room) goto end; 
	dam *= number(2, 3);
	act("류우우우~~", TRUE, ch, 0, 0, TO_ROOM);
	damage(ch, victim, dam, TYPE_HIT);

	if (ch->in_room != victim->in_room) goto end; 
	dam *= number(5, 7 + (GET_SKILLED(ch, SKILL_SHOURYUKEN) >> 4));
	act("켄!!!!!!!!", TRUE, ch, 0, 0, TO_ROOM);
	damage(ch, victim, dam, TYPE_HIT);
end:
	send_to_char("빠샤!!!!!!\n\r", ch);
	WAIT_STATE(ch, PULSE_VIOLENCE);  /* NOTE: Full-deley on failure */
    }
    else {
	damage(ch, victim, dam, TYPE_HIT);
	send_to_char("당신은 승룡권을 실패해서 몹시 지칩니다.\n\r", ch);
	WAIT_STATE(ch, PULSE_VIOLENCE/2);  /* NOTE: Half-deley on failure */
    }
    /* WAIT_STATE(ch, PULSE_VIOLENCE/3); */
}

/* throw skill by chase */
void do_throw_object(struct char_data *ch, char *argument, int cmd)
{
    struct char_data *victim;
    struct obj_data *obj;
    char obj_name[MAX_INPUT_LENGTH];
    int percent;
    int dam;

    argument = one_argument(argument, obj_name);
    victim = get_victim(ch, argument, cmd);
    if ( !victim )
	return; 

    else if (GET_CLASS(ch) != CLASS_THIEF && GET_LEVEL(ch) < IMO) {
	send_to_char("This skill is for thieves only.\n\r", ch);
	return;
    } 
    else if (ch && GET_MANA(ch) < 100) {
	send_to_char("You do not have enough mana!\n\r", ch);
	return;
    }
    else if (!ch->carrying
	    || !(obj = get_obj_in_list_vis(ch, obj_name, ch->carrying))) {
	send_to_char("You do not have that item.\n\r", ch);
	return;
    }
    else if (IS_SET(obj->obj_flags.extra_flags, ITEM_NODROP)) {
	send_to_char("You can't throw it, it must be CURSED!\n\r", ch);
	return;
    }

    GET_MANA(ch) -= 100;

    INCREASE_SKILLED(ch, victim, SKILL_THROW_OBJECT);
    /* NOTE: STR_ADD apply is not rational */
    /* NOTE: wieght of obj is added in damage calc. */
    /* OLD damage:
    dam = obj->obj_flags.cost / 1000;
    dam += (GET_LEVEL(ch) + GET_STR(ch) + GET_ADD(ch) + GET_CON(ch));
    dam *= ((GET_LEVEL(ch) + GET_SKILLED(ch, SKILL_THROW_OBJECT)) >> 3);
    */
    dam = obj->obj_flags.cost /1000 + obj->obj_flags.weight*2 + 20;
    dam *= (GET_LEVEL(ch) + GET_STR(ch)*2+GET_ADD(ch)/10 + GET_CON(ch) +10 );
    dam *= ((GET_LEVEL(ch) + GET_SKILLED(ch, SKILL_THROW_OBJECT)) >> 3);
    dam /= 100;
    percent = number(1, 100) - GET_LEVEL(ch) + 2 * GET_LEVEL(victim)
	- (GET_SKILLED(ch, SKILL_THROW_OBJECT) >> 2);

    /* NOTE: Some skill message change. */
    send_to_room( "하이야~~~~압~~~\r\n이야야~~~~합~~~\r\n", ch->in_room); 
    do_say(ch, "받아랏~~~~!!!@!#@!", 0);
    act( "$n님이 $N님에게 $p을 던집니다!!!", TRUE, ch, obj, victim, TO_NOTVICT);

    if (percent > ch->skills[SKILL_THROW_OBJECT].learned) {
	dam = 0;
	do_say(ch, "윽...이렇게 비싼걸....망했당..", 0);
    }
    else {
	act("$p이 $n님의 이마빡에 정통으로 맞았습니다!",
	    FALSE, victim, obj, 0, TO_ROOM);
	act("$n님이 던진 $p에 맞아 눈에 불이 번쩍납니다!",
	    FALSE, ch, obj, victim, TO_VICT);
    }
    damage(ch, victim, dam, TYPE_MISC);
    if (obj)
	obj_from_char(obj);
    if (obj)
	extract_obj(obj);
    if (!IS_NPC(ch))
	WAIT_STATE(ch, PULSE_VIOLENCE);
} 

void do_assist(struct char_data *ch, char *argument, int cmd)
{
    char arg[MAX_STRING_LENGTH];
    struct char_data *victim;
    extern int nokillflag;
    char cyb[80];

    one_argument(argument, arg);

    if (!*arg) {
	send_to_char("Assist who?\n\r", ch);
	return;
    } 
    victim = get_char_room_vis(ch, arg);
    if (!victim && strcmp(arg, "master") == 0) {
	if (ch->master)
	    victim = ch->master;
    }
    if (!victim) {
	send_to_char("They aren't here.\n\r", ch);
	return;
    }
    else if (victim == ch) {
	send_to_char("Can you assist your self?\n\r", ch);
	act("$n tries to help $mself, and says YAHA!", 
	    FALSE, ch, 0, victim, TO_ROOM);
	return;
    } 
    else if (nokillflag && !IS_NPC(ch) && IS_NPC(victim)) {
	send_to_char("You can't help MOB.\n\r", ch);
	return; 
    } 
    /* block indirect player killing */
    else if (IS_AFFECTED(ch, AFF_CHARM) && (IS_NPC(victim)))
	return;

    else if ((GET_POS(ch) != POS_STANDING) 
	|| (victim == ch->specials.fighting)) {
	send_to_char("You do the best you can!\n\r", ch);
	return;
    }
    else if (!(victim->specials.fighting)) {
	send_to_char("Who do you think fight ?\n\r", ch);
	return;
    }

    sprintf(cyb, "%s assist you !!\n\r", GET_NAME(ch));
    send_to_char(cyb, victim);
    sprintf(cyb, "You assist %s !!\n\r", GET_NAME(victim));
    send_to_char(cyb, ch);
    act("$n ASSIST $N !", FALSE, ch, 0, victim, TO_NOTVICT);
    hit(ch, victim->specials.fighting, TYPE_UNDEFINED);

    if (!IS_NPC(ch))
	WAIT_STATE(ch, PULSE_VIOLENCE + 2);	/* HVORFOR DET?? */
}
