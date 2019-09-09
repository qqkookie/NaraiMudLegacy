/* ************************************************************************
*  file: magic2.c , Implementation of spells.             Part of DIKUMUD *
*  Usage : The actual effect of magic.                                    *
*  Copyright (C) 1990, 1991 - see 'license.doc' for complete information. *
************************************************************************* */

#include <stdio.h>
#include <string.h>
#include <assert.h>
#include "structs.h"
#include "utils.h"
#include "comm.h"
#include "spells.h"
#include "handler.h"
#include "limit.h"
#include "db.h"
#include "magic_weapon.h"

#include "guild_list.h"

/* Extern structures */
extern struct room_data *world;
extern struct obj_data  *object_list;
extern struct char_data *character_list;
extern struct index_data *mob_index;
extern int noenchantflag;

/* Extern procedures */

void damage(struct char_data *ch, struct char_data *victim,
            int damage, int weapontype);
bool saves_spell(struct char_data *ch, int spell);
void weight_change_object(struct obj_data *obj, int weight);
// char *strdup(char *source);
int dice(int number, int size);
void do_look(struct char_data *ch, char *argument, int cmd);
int number(int from, int to);
void stop_fighting(struct char_data *ch);
void list_obj_to_char(struct obj_data *list,struct char_data *ch, int mode,
  bool show);
void list_char_to_char(struct char_data *list, struct char_data *ch, int mode);
int MIN(int a, int b);
int MAX(int a, int b);
void update_pos(struct char_data *ch);



/* spells2.c - Not directly offensive spells */

void spell_armor(byte level, struct char_data *ch,
  struct char_data *victim, struct obj_data *obj)
{
	struct affected_type af;

	assert(victim);
	if (!affected_by_spell(victim, SPELL_ARMOR)) {
		INCREASE_SKILLED2(ch, victim, SPELL_ARMOR);

		af.type      = SPELL_ARMOR;
		af.duration  = level >> 1;
		af.modifier  = -20;
		af.location  = APPLY_AC;
		if (level > 30)
			af.modifier -= level - 30;
		af.bitvector = 0;
		affect_to_char(victim, &af);
		send_to_char("You feel someone protecting you.\n\r", victim);
	}
}

void spell_teleport(byte level, struct char_data *ch,
  struct char_data *victim, struct obj_data *obj)
{
	int to_room;
	extern int top_of_world;      /* ref to the top element of world */

	assert(ch);

	INCREASE_SKILLED2(ch, victim, SPELL_TELEPORT);
	do {
		to_room = number(0, top_of_world);
	} while (IS_SET(world[to_room].room_flags, PRIVATE));

	act("$n slowly fade out of existence.", FALSE, ch,0,0,TO_ROOM);
	if (ch->specials.fighting) {
		stop_fighting(ch->specials.fighting);
		stop_fighting(ch);
	}
	char_from_room(ch);
	char_to_room(ch, to_room);
	act("$n slowly fade in to existence.", FALSE, ch,0,0,TO_ROOM);
	do_look(ch, "", 0);
}

void spell_far_look(byte level, struct char_data *ch,
  struct char_data *victim, struct obj_data *obj)
{
	int location, i;

	assert(ch);
	if (CAN_SEE(ch, victim)) {
		INCREASE_SKILLED2(ch, victim, SPELL_FAR_LOOK);
		i = GET_LEVEL(ch) - GET_LEVEL(victim);
		if (i < 0) {
			send_to_char("Your spell is not powerful enough for that.\n\r", ch);
			return;
		} 
		location = victim->in_room;
	}
	else {
		send_to_char("No such creature around, what a waste.\n\r", ch);
		return;
	}
	if (IS_SET(world[location].room_flags, PRIVATE) ||
		/*IS_SET(world[location].room_flags, NORELOCATE) || */
		IS_SET(world[location].room_flags, OFF_LIMITS)) {
		send_to_char( "You fail miserably.\n\r", ch);
		return;
	}
/*	if (IS_AFFECTED(victim, AFF_SHADOW_FIGURE)) {
		if (ch->in_room != real_room(ROOM_GUILD_POLICE_LOCKER) &&
			ch->in_room != real_room(ROOM_GUILD_OUTLAW_LOCKER)) {
			send_to_char("You tried, but you can see only shadow.\n\r", ch);
			return;
		}
	} */
	if (!IS_NPC(victim)) {
		send_to_char("You sense an uncertain feeling of being ", victim);
		send_to_char("watched, and you feel somewhat sick.\n\r", victim);
	}
	send_to_char(world[location].name, ch);
	send_to_char("\n\r",ch);
	send_to_char(world[location].description, ch);
	list_obj_to_char(world[location].contents, ch, 0, FALSE);
	list_char_to_char(world[location].people, ch, 0);
}

void spell_relocate(byte level, struct char_data *ch,
  struct char_data *victim, struct obj_data *obj)
{
	int location, i;

	assert(ch);
	if (CAN_SEE(ch, victim)) {
		INCREASE_SKILLED2(ch, victim, SPELL_RELOCATE);
		i = GET_LEVEL(ch) - GET_LEVEL(victim);
		if (i < 0) {
			send_to_char("Your spell is not powerful enough for that.\n\r",ch);
			return;
		}
		else if(i == 0 && (GET_GUILD(ch) != GET_GUILD(victim))){
			send_to_char("Your spell is not powerful enough for that.\n\r",ch);
			return;
		}
		else if (number(0, 2) > i) {
			send_to_char("You make a valiant effort, but barely fail.\n\r", ch);
			return;
		}
		location = victim->in_room;
	}
	else {
		send_to_char("No such creature around, what a waste.\n\r", ch);
		return;
	}
	if (IS_SET(world[location].room_flags, PRIVATE) ||
		IS_SET(world[location].room_flags, NORELOCATE) ||
		IS_SET(world[location].room_flags, OFF_LIMITS)) {
		send_to_char("You fail miserably.\n\r", ch);
		return;
	}
	act("$n disappears in a puff of purple smoke.", FALSE,ch,0,0,TO_ROOM);
	char_from_room(ch);
	char_to_room(ch, location);
	act("$n appears with a modest poof.",FALSE,ch,0,0,TO_ROOM);
	do_look(ch, "",15);
}

void spell_damage_up(byte level, struct char_data *ch,
  struct char_data *victim,struct obj_data *obj)
{
	struct affected_type af;

	assert(ch && victim);
	if (!affected_by_spell(victim, SPELL_DAMAGE_UP)) {
		INCREASE_SKILLED2(ch, victim, SPELL_DAMAGE_UP);

		af.type      = SPELL_DAMAGE_UP;
		af.duration  = 4 + level/10;
		af.modifier  = level / 10 + 1;
		af.location  = APPLY_DAMROLL;
		af.bitvector = 0;
		affect_to_char(victim, &af);
		send_to_char("You feel brave.\n\r", victim);
	}
}

void spell_crush_armor(byte level, struct char_data *ch,
  struct char_data *victim,struct obj_data *obj)
{
	struct affected_type af;

	assert(ch && victim);
    if (!affected_by_spell(victim, SPELL_CRUSH_ARMOR)) {
		INCREASE_SKILLED2(ch, victim, SPELL_CRUSH_ARMOR);
		af.type      = SPELL_CRUSH_ARMOR;
		af.duration  = 3;
		af.modifier  = level;
		af.location  = APPLY_AC;
		af.bitvector = 0;
		affect_to_char(victim, &af);
		send_to_char("Your victim's armor has crumbled!\n\r", ch);
		send_to_char("You feel shrink.\n\r", victim);
	}
}

void spell_bless(byte level, struct char_data *ch,
  struct char_data *victim, struct obj_data *obj)
{
	struct affected_type af;

	assert(ch && (victim || obj));

	INCREASE_SKILLED2(ch, ch, SPELL_BLESS);
	if (obj) {
		if ((5 * GET_LEVEL(ch) > GET_OBJ_WEIGHT(obj)) &&
			!IS_OBJ_STAT(obj, ITEM_EVIL)) {
			SET_BIT(obj->obj_flags.extra_flags, ITEM_BLESS);
			act("$p briefly glows.",FALSE,ch,obj,0,TO_CHAR);
		}
	}
	else {
		if (!affected_by_spell(victim, SPELL_BLESS)) {
			af.type      = SPELL_BLESS;
			af.duration  = 6;
			af.modifier  = level/2;
			af.location  = APPLY_HITROLL;
			af.bitvector = 0;
			affect_to_char(victim, &af);
			af.location = APPLY_SAVING_SPELL;
			af.modifier = -5;                 /* Make better */
			affect_to_char(victim, &af);
			send_to_char("You feel righteous.\n\r", victim);
		}
	}
}


void spell_blindness(byte level, struct char_data *ch,
  struct char_data *victim, struct obj_data *obj)
{
	struct affected_type af;

	assert(ch && victim);

	INCREASE_SKILLED2(ch, victim, SPELL_BLINDNESS);
	if (saves_spell(victim, SAVING_PARA) ||
		affected_by_spell(victim, SPELL_BLINDNESS))
		return;
	if (level + number(1, 5) < GET_LEVEL(victim))
		return;

	act("$n seems to be blinded!", TRUE, victim, 0, 0, TO_ROOM);
	send_to_char("You have been blinded!\n\r", victim);

	af.type      = SPELL_BLINDNESS;
	af.location  = APPLY_HITROLL;
	af.modifier  = -level/2;  /* Make hitroll worse */
	af.duration  = 1;
	af.bitvector = AFF_BLIND;
	affect_to_char(victim, &af);

	af.location = APPLY_AC;
	af.modifier = 40; /* Make AC Worse! */
	affect_to_char(victim, &af);
}

void spell_create_food(byte level, struct char_data *ch,
  struct char_data *victim, struct obj_data *obj)
{
	struct obj_data *tmp_obj;

	assert(ch);

	INCREASE_SKILLED2(ch, ch, SPELL_CREATE_FOOD);

	CREATE(tmp_obj, struct obj_data, 1);
	clear_object(tmp_obj);

	tmp_obj->name = strdup("mushroom");
	tmp_obj->short_description = strdup("A Magic Mushroom");
	tmp_obj->description =
		strdup("A really delicious looking magic mushroom lies here.");

	tmp_obj->obj_flags.type_flag = ITEM_FOOD;
	tmp_obj->obj_flags.wear_flags = ITEM_TAKE | ITEM_HOLD;
	tmp_obj->obj_flags.value[0] = 5+level;
	tmp_obj->obj_flags.weight = 1;
	tmp_obj->obj_flags.cost = 0;

	tmp_obj->next = object_list;
	object_list = tmp_obj;

	obj_to_room(tmp_obj,ch->in_room);

	tmp_obj->item_number = -1;
	// BUG FIX!! rent bug!
	SET_BIT(tmp_obj->obj_flags.extra_flags, ITEM_NORENT);

	act("$p suddenly appears.",TRUE,ch,tmp_obj,0,TO_ROOM);
	act("$p suddenly appears.",TRUE,ch,tmp_obj,0,TO_CHAR);
}



void spell_create_water(byte level, struct char_data *ch,
  struct char_data *victim, struct obj_data *obj)
{
	int water;

	extern struct weather_data weather_info;
	void name_to_drinkcon(struct obj_data *obj,int type);
	void name_from_drinkcon(struct obj_data *obj);

	assert(ch && obj);

	INCREASE_SKILLED2(ch, ch, SPELL_CREATE_WATER);

	if (GET_ITEM_TYPE(obj) == ITEM_DRINKCON) {
		if ((obj->obj_flags.value[2] != LIQ_WATER)
			&& (obj->obj_flags.value[1] != 0)) {
			name_from_drinkcon(obj);
			obj->obj_flags.value[2] = LIQ_SLIME;
			name_to_drinkcon(obj, LIQ_SLIME);
		}
		else {
			water = 2 * level * ((weather_info.sky >= SKY_RAINING) ? 2 : 1);
			/* Calculate water it can contain, or water created */
			water = MIN(obj->obj_flags.value[0]-obj->obj_flags.value[1], water);
			if (water > 0) {
				obj->obj_flags.value[2] = LIQ_WATER;
				obj->obj_flags.value[1] += water;
				weight_change_object(obj, water);
				name_from_drinkcon(obj);
				name_to_drinkcon(obj, LIQ_WATER);
				act("$p is filled.", FALSE, ch,obj,0,TO_CHAR);
			}
		}
	}
}


void spell_create_nectar(byte level, struct char_data *ch,
  struct char_data *victim, struct obj_data *obj)
{
	int nectar;

	void name_to_drinkcon(struct obj_data *obj,int type);
	void name_from_drinkcon(struct obj_data *obj);

	assert(ch && obj);

	INCREASE_SKILLED2(ch, ch, SPELL_CREATE_NECTAR);

	if (GET_ITEM_TYPE(obj) == ITEM_DRINKCON) {
		if ((obj->obj_flags.value[2] != LIQ_NECTAR)
			&& (obj->obj_flags.value[1] != 0)) {
			name_from_drinkcon(obj);
			obj->obj_flags.value[2] = LIQ_SLIME;
			name_to_drinkcon(obj, LIQ_SLIME);
		}
		else {
			nectar = 2*level;
			/* Calculate water it can contain, or water created */
			nectar = MIN(obj->obj_flags.value[0]-obj->obj_flags.value[1],
				nectar);
			if (nectar > 0) {
				obj->obj_flags.value[2] = LIQ_NECTAR;
				obj->obj_flags.value[1] += nectar;
				weight_change_object(obj, nectar);
				name_from_drinkcon(obj);
				name_to_drinkcon(obj, LIQ_NECTAR);
				act("$p is filled.", FALSE, ch,obj,0,TO_CHAR);
			}
		}
	}
}


void spell_create_golden_nectar(byte level, struct char_data *ch,
  struct char_data *victim, struct obj_data *obj)
{
	int nectar;

	void name_to_drinkcon(struct obj_data *obj,int type);
	void name_from_drinkcon(struct obj_data *obj);

	assert(ch && obj);

	INCREASE_SKILLED2(ch, ch, SPELL_CREATE_GOLDEN_NECTAR);

	if (GET_ITEM_TYPE(obj) == ITEM_DRINKCON) {
		if ((obj->obj_flags.value[2] != LIQ_GOLDEN_NECTAR)
			&& (obj->obj_flags.value[1] != 0)) {
			name_from_drinkcon(obj);
			obj->obj_flags.value[2] = LIQ_SLIME;
			name_to_drinkcon(obj, LIQ_SLIME);
		}
		else {
			nectar = 3*level/2;
			/* Calculate water it can contain, or water created */
			nectar = MIN(obj->obj_flags.value[0]-obj->obj_flags.value[1],
				nectar);
			if (nectar > 0) {
				obj->obj_flags.value[2] = LIQ_GOLDEN_NECTAR;
				obj->obj_flags.value[1] += nectar;
				weight_change_object(obj, nectar);
				name_from_drinkcon(obj);
				name_to_drinkcon(obj, LIQ_GOLDEN_NECTAR);
				act("$p is filled.", FALSE, ch,obj,0,TO_CHAR);
			}
		}
	}
}

void spell_cure_blind(byte level, struct char_data *ch,
  struct char_data *victim, struct obj_data *obj)
{
	assert(victim);

	if (affected_by_spell(victim, SPELL_BLINDNESS)) {
		INCREASE_SKILLED2(ch, victim, SPELL_CURE_BLIND);
		affect_from_char(victim, SPELL_BLINDNESS);
		send_to_char("Your vision returns!\n\r", victim);
	}
}


void spell_cure_critic(byte level, struct char_data *ch,
  struct char_data *victim, struct obj_data *obj)
{
	int healpoints;

	assert(victim);
	INCREASE_SKILLED2(ch, victim, SPELL_CURE_CRITIC);
	healpoints = dice(level, 20);
	if ((healpoints + GET_HIT(victim)) > GET_PLAYER_MAX_HIT(victim))
		GET_HIT(victim) = GET_PLAYER_MAX_HIT(victim);
	else
		GET_HIT(victim) += healpoints;
	send_to_char("You feel better!\n\r", victim);
	update_pos(victim);
}

void spell_cause_critic(byte level, struct char_data *ch,
  struct char_data *victim, struct obj_data *obj)
{
	int dam;

	assert(victim);

	INCREASE_SKILLED2(ch, victim, SPELL_CAUSE_CRITIC);
	dam = dice(level, 20);
	if (saves_spell(victim, SAVING_SPELL))
		dam >>= 1;
	send_to_char("You feel pain!\n\r", victim);
	damage(ch, victim, dam, SPELL_CAUSE_CRITIC);
}


void spell_mana_boost(byte level, struct char_data *ch,
  struct char_data *victim, struct obj_data *obj)
{
	int mana = 0;

	assert(victim);
	INCREASE_SKILLED2(ch, victim, SPELL_MANA_BOOST);
	if (GET_MOVE(victim) > 50)
		mana = GET_MOVE(victim) / 2 + 2 * GET_LEVEL(victim);
    GET_MANA(victim) += mana;
    GET_MOVE(victim) = 0; 
	send_to_char("You feel better!\n\r", victim);
	update_pos(victim);
}

void spell_vitalize(byte level, struct char_data *ch,
  struct char_data *victim, struct obj_data *obj)
{
	int hit = 0;

	assert(victim);
	INCREASE_SKILLED2(ch, victim, SPELL_VITALIZE);
	hit = GET_MOVE(victim) / dice(1,3);
	hit += ((hit >> 6) + 1) * level;
    GET_HIT(victim) += hit;
    GET_MOVE(victim) = 0;
	send_to_char("You feel vitalized!\n\r", victim);
	update_pos(victim);
}

void spell_cure_light(byte level, struct char_data *ch,
  struct char_data *victim, struct obj_data *obj)
{
	int hit;

	assert(ch && victim);

	INCREASE_SKILLED2(ch, victim, SPELL_CURE_LIGHT);
	hit = dice(level, 10);
	if ((hit + GET_HIT(victim)) > GET_PLAYER_MAX_HIT(victim))
		GET_HIT(victim) = GET_PLAYER_MAX_HIT(victim);
	else
		GET_HIT(victim) += hit;

	update_pos(victim);
	send_to_char("You feel better!\n\r", victim);
}

void spell_cause_light(byte level, struct char_data *ch,
  struct char_data *victim, struct obj_data *obj)
{
	int dam;

	assert(ch && victim);

	INCREASE_SKILLED2(ch, victim, SPELL_CAUSE_LIGHT);
	dam = dice(level, 10);
	if (saves_spell(victim, SAVING_SPELL))
		dam >>= 1;
	send_to_char("You feel pain!\n\r", victim);
	damage(ch, victim, dam, SPELL_CAUSE_LIGHT);
}



void spell_curse(byte level, struct char_data *ch,
  struct char_data *victim, struct obj_data *obj)
{
	struct affected_type af;

	assert(victim || obj);

	INCREASE_SKILLED2(ch, ch, SPELL_CURSE);
	if (obj) {
		SET_BIT(obj->obj_flags.extra_flags, ITEM_EVIL);
		SET_BIT(obj->obj_flags.extra_flags, ITEM_NODROP);
		if(obj->obj_flags.type_flag == ITEM_WEAPON)
			obj->obj_flags.value[2] -= GET_LEVEL(ch) / 2;
		act("$p glows red.", FALSE, ch, obj, 0, TO_CHAR);
	}
	else {
		if (saves_spell(victim, SAVING_PARA) ||
			affected_by_spell(victim, SPELL_CURSE))
			return;

		af.type      = SPELL_CURSE;
		af.duration  = 7;      
		af.modifier  = -level/2;
		af.location  = APPLY_HITROLL;
		af.bitvector = AFF_CURSE;
		affect_to_char(victim, &af);

		af.location = APPLY_SAVING_SPELL;
		af.modifier = level / 2; /* Make worse */
		affect_to_char(victim, &af);

		act("$n briefly reveal a red aura!", FALSE, victim, 0, 0, TO_ROOM);
		act("You feel very uncomfortable.",FALSE,victim,0,0,TO_CHAR);
	}
}



void spell_detect_evil(byte level, struct char_data *ch,
  struct char_data *victim, struct obj_data *obj)
{
	struct affected_type af;

	assert(victim);

	if (affected_by_spell(victim, SPELL_DETECT_EVIL))
		return;

	INCREASE_SKILLED2(ch, victim, SPELL_DETECT_EVIL);

	af.type      = SPELL_DETECT_EVIL;
	af.duration  = level * 5;
	af.modifier  = 0;
	af.location  = APPLY_NONE;
	af.bitvector = AFF_DETECT_EVIL;

	affect_to_char(victim, &af);

	send_to_char("Your eyes tingle.\n\r", victim);
}



void spell_detect_invisibility(byte level, struct char_data *ch,
  struct char_data *victim, struct obj_data *obj)
{
	struct affected_type af;

	assert(victim);

	if (affected_by_spell(victim, SPELL_DETECT_INVISIBLE))
		return;

	INCREASE_SKILLED2(ch, victim, SPELL_DETECT_INVISIBLE);
	af.type      = SPELL_DETECT_INVISIBLE;
	af.duration  = level * 5;
	af.modifier  = 0;
	af.location  = APPLY_NONE;
	af.bitvector = AFF_DETECT_INVISIBLE;

	affect_to_char(victim, &af);

	send_to_char("Your eyes tingle.\n\r", victim);
}



void spell_recharger(byte level, struct char_data *ch,
  struct char_data *victim, struct obj_data *obj)
{
	assert(ch && obj);

	if ((GET_ITEM_TYPE(obj) == ITEM_WAND) ||
		(GET_ITEM_TYPE(obj) == ITEM_STAFF)) {
		if (!number(0, level / 10 + 1)) {
			act("BANG!!!! $p explodes. It hurts!",FALSE,ch,obj,0,TO_CHAR);
			extract_obj(obj);
			GET_HIT(ch) -= GET_HIT(ch) / 10;
			return;
		}
		INCREASE_SKILLED2(ch, ch, SPELL_RECHARGER);
		obj->obj_flags.value[2] =
			MIN(obj->obj_flags.value[1], obj->obj_flags.value[2] + 5);
		act("$p looks recharged.",FALSE,ch,obj,0,TO_CHAR);
	}
}

/* by wings */
void spell_enchant_person(byte level, struct char_data *ch,
  struct char_data *victim, struct obj_data *obj)
{
	if (noenchantflag && GET_LEVEL(ch) < IMO)
		return;

	INCREASE_SKILLED2(ch, ch, SPELL_ENCHANT_PERSON);

	if (number(0, 9) > 8) {
		act("OoooooooM! $n gain more mana!", TRUE,victim,0,0,TO_ROOM);
		act("OoooooooM! You gain more mana!", TRUE,victim,0,0,TO_CHAR);
		GET_PLAYER_MAX_MANA(victim) += number(1, GET_WIS(victim)) * number(1,3);
	}
	else if (number(0, 9) > 8) {
		act("PoooooooYa! $n gain more hit point!", TRUE,victim,0,0,TO_ROOM); 
		act("PoooooooYa! You gain more hit point!",TRUE,victim,0,0,TO_CHAR);
		GET_PLAYER_MAX_HIT(victim) += number(1, GET_CON(victim)) * number(1,3);
	}
	else if (number(0, 9) > 8) {
		act("Shaaaaaaru! $n gain more move point!", TRUE,victim,0,0,TO_ROOM); 
		act("Shaaaaaaru! You gain more move point!",TRUE,victim,0,0,TO_CHAR);
		GET_PLAYER_MAX_MOVE(victim) += number(1, GET_DEX(victim)) * number(1,3);
	}
	else if (number(0,69)==35) {
		act("Barararang! $n is surrounded by SMILING of XER!",
			TRUE,victim,0,0,TO_ROOM);
		act("Barararang! You are surrounded by SMILING of XER!",
			TRUE,victim,0,0,TO_CHAR);
		GET_PLAYER_MAX_MANA(victim) += number(1, GET_WIS(victim)) * number(2,5);
		GET_PLAYER_MAX_HIT(victim) += number(1, GET_CON(victim)) * number(2,5);
		GET_PLAYER_MAX_MOVE(victim) += number(1, GET_DEX(victim)) * number(2,5);
	}
	else if (number(0, 499) == 267) {
		act("Spar! $n is BLESSED by XER!", TRUE,victim,0,0,TO_ROOM);
		act("Spar! You are BLESSED by XER!", TRUE,victim,0,0,TO_CHAR);
		GET_PLAYER_MAX_MANA(victim) +=
			number(1, GET_WIS(victim)) * number(8, 12);
		GET_PLAYER_MAX_HIT(victim) +=
			number(1, GET_CON(victim)) * number(8, 12);
		GET_PLAYER_MAX_MOVE(victim) +=
			number(1, GET_DEX(victim)) * number(8, 12);
	}
	else if (number(0, 1999) == 1567) {
		act("Spar! $n is RAISED by XER!", TRUE,victim,0,0,TO_ROOM);
		act("Spar! You are RAISED by XER!", TRUE,victim,0,0,TO_CHAR);
		GET_PLAYER_MAX_MANA(victim) +=
			number(1, GET_WIS(victim)) * number(22,30);
		GET_PLAYER_MAX_HIT(victim) +=
			number(1, GET_CON(victim)) * number(22,30);
		GET_PLAYER_MAX_MOVE(victim) +=
			number(1, GET_DEX(victim)) * number(22,30);
	}
	else if (number(0, 3999) == 2678) {
		act("Feeee! XER gives $n new POWER!", TRUE,victim,0,0,TO_ROOM);
		act("Feeee! XER gives you new POWER!", TRUE,victim,0,0,TO_CHAR);
		GET_PLAYER_MAX_MANA(victim) +=
			number(1, GET_WIS(victim)) * number(42,64);
		GET_PLAYER_MAX_HIT(victim) +=
			number(1, GET_CON(victim)) * number(42,64);
		GET_PLAYER_MAX_MOVE(victim) +=
			number(1, GET_DEX(victim)) * number(42,64);
	}
	else if (number(0,8) > 1) {
		act("HICCUP! $n seems lost some power!", TRUE,victim,0,0,TO_ROOM);
		act("HICCUP! You lost some power!", TRUE,victim,0,0,TO_CHAR);
		GET_PLAYER_MAX_MANA(victim) =
			MAX((GET_PLAYER_MAX_MANA(victim) - 
			number(1, GET_WIS(victim)) / 2 * number(0, 2)), 0);
		GET_PLAYER_MAX_HIT(victim) =
			MAX((GET_PLAYER_MAX_HIT(victim) -
			number(1, GET_CON(victim)) / 2 * number(0, 2)), 0);
		GET_PLAYER_MAX_MOVE(victim) =
			MAX((GET_PLAYER_MAX_MOVE(victim) -
			number(1, GET_DEX(victim)) / 2 * number(0, 2)), 0);
	}
	else {
		act("Hrm... $n seems to be unchanged.", TRUE, victim, 0,0,TO_ROOM);
		act("Hrm... You cannot feel any change.", TRUE, victim, 0,0,TO_CHAR);
	}
}

void spell_enchant_weapon(byte level, struct char_data *ch,
  struct char_data *victim, struct obj_data *obj)
{
	int i,type;
	struct obj_data *tmp;

	assert(ch && obj);

	if (noenchantflag && GET_LEVEL(ch) < IMO)
		return;

	if(!IS_NPC(ch) && GET_LEVEL(ch) == IMO){
		send_to_char("You cannot enchant!\n\r",ch);
		return;
	}

	for (i = 0; i < MAX_WEAR; i++) {
		tmp = ch->equipment[i];
		if (obj == tmp) {
			send_to_char("You cannot enchant it while you are wielding.", ch);
			return;
		} /* by process */
	}

	if (GET_ITEM_TYPE(obj) == ITEM_WEAPON) {
		INCREASE_SKILLED2(ch, ch, SPELL_ENCHANT_WEAPON);
		SET_BIT(obj->obj_flags.extra_flags, ITEM_NORENT);
		if (!IS_SET(obj->obj_flags.extra_flags, ITEM_MAGIC)) {
			SET_BIT(obj->obj_flags.extra_flags, ITEM_MAGIC);

			if (obj->affected[0].location != APPLY_NONE)
				obj->affected[0].modifier *= number(1, 2);
			else {
				obj->affected[0].location = APPLY_HITROLL;
				obj->affected[0].modifier =
					1 + (level >= 18) + (level >= 30);
			}
			if (obj->affected[1].location != APPLY_NONE)
				obj->affected[1].modifier *= number(1, 2);
			else {
				obj->affected[1].location = APPLY_DAMROLL;
				obj->affected[1].modifier =
					1 + (level >= 18) + (level >= 30);
			}
			if (IS_GOOD(ch)) {
				SET_BIT(obj->obj_flags.extra_flags, ITEM_ANTI_EVIL);
				act("$p glows blue evilly.",FALSE,ch,obj,0,TO_CHAR);
			}
			else if (IS_EVIL(ch)) {
				SET_BIT(obj->obj_flags.extra_flags, ITEM_ANTI_GOOD);
				act("$p glows red saintly.",FALSE,ch,obj,0,TO_CHAR);
			}
			else {
				act("$p glows yellow.",FALSE,ch,obj,0,TO_CHAR);
			}
		} /* if(ITEM_MAGIC) end */
		else {
		/*	if (number(0,18000) == 444) {		*/
			if (number(0,1800) == 444) {
				obj->obj_flags.value[1] *= (2 + number(0, 4));	
				act("$p laughs evily. 'FU HA HA HA'",FALSE,ch,obj,0,TO_CHAR);
				// BUG FIX?????
				// GET_HIT(ch) =- 3;
				GET_HIT(ch) = -3;

			}
			if (number(0,18000) == 888) {
		/*	if (number(0,1800) == 888) {	*/
				obj->obj_flags.value[2] *= (6 + number(0, 8));  
				act("$p laughs evily. 'HMM.. HA HA'",FALSE,ch,obj,0,TO_CHAR);
				GET_HIT(ch) = -3;
			}
		/*	if (number(0, 4999)==1234 || number(0,4999) == 3456) {	*/
			if (number(0, 49999)==1234 || number(0,49999) == 3456) {
				obj->obj_flags.value[1] *= (6 + number(0, 8));
				obj->obj_flags.value[2] *= (8 + number(0,10));
				act("$p IS BLESSED BY SAINT XARK.",FALSE,ch,obj,0,TO_CHAR);
				GET_HIT(ch) = 10000;
			}
			if (number(0, 100000) < 5 )
		/*	if (number(0, 1000) < 5 )	*/
			{
				SET_BIT(obj->obj_flags.extra_flags,ITEM_MAGIC);
				type = number(0,14);
				if ( type & 0x0001 )
					SET_BIT(obj->obj_flags.extra_flags,ITEM_ANTI_MAGE);
				if ( type & 0x0002 )
					SET_BIT(obj->obj_flags.extra_flags,ITEM_ANTI_CLERIC);
				if ( type & 0x0004 )
					SET_BIT(obj->obj_flags.extra_flags,ITEM_ANTI_THIEF);
				if ( type & 0x0008 )
					SET_BIT(obj->obj_flags.extra_flags,ITEM_ANTI_WARRIOR);
				type = number(1,19);
				switch(type)
				{
				case	WEAPON_SMASH:
				case	WEAPON_FLAME:
				case	WEAPON_ICE:
					obj->obj_flags.value[0] = type;
					break;
				case	WEAPON_DRAGON_SLAYER:
					obj->obj_flags.value[0] = type;
					obj->obj_flags.value[3] = 3;
					obj->name = "dragon slayer";
					obj->short_description = "Mystic Dragon Slayer";
					obj->description =
						"Mystic Dragon Slayer lies here.\n\r";
					break;
				case	WEAPON_ANTI_EVIL_WEAPON:
					SET_BIT(obj->obj_flags.extra_flags,ITEM_ANTI_EVIL);
					obj->obj_flags.value[0] = type;
					obj->obj_flags.value[3] = 3;
					obj->name = "silver spear";
					obj->short_description = "Silver Spear";
					obj->description =
						"Silver Spear lies here.\n\r";
					break;
				case	WEAPON_ANTI_GOOD_WEAPON:
					SET_BIT(obj->obj_flags.extra_flags,ITEM_ANTI_GOOD);
					obj->obj_flags.value[0] = type;
					obj->obj_flags.value[3] = 3;
					obj->name = "demon blade";
					obj->short_description = "Mighty Demon Blade";
					obj->description =
						"Mighty Demon Blade lies here.\n\r";
					break;
				case	WEAPON_DISINTEGRATE:
					obj->obj_flags.value[0] = type;
					obj->obj_flags.value[3] = 3;
					obj->name = "sword disintegrate";
					obj->short_description = "runed longsword";
					obj->description =
						"Longsword runed by a word 'Disintergrate' lies here.\n\r";
					break;
				case	WEAPON_LIGHTNING:
				case	WEAPON_CALL_LIGHTNING:
				case	WEAPON_FIREBALL:
				case	WEAPON_FROST_BREATH:
				case	WEAPON_ENERGY_DRAIN:
				case	WEAPON_FIRE_BREATH:
					obj->obj_flags.value[0] = type;
					break;
				default:
					obj->obj_flags.value[0] = WEAPON_ANY_MAGIC;
					break;
				}
				obj->obj_flags.gpd = number(1,12);
				act("$p glows brightly more and more...",
					FALSE,ch,obj,0,TO_CHAR);
				act("You can't open your eyes.",FALSE,ch,obj,0,TO_CHAR);
				act("New object appeared.",FALSE,ch,obj,0,TO_CHAR);
			}

			if (number(0,8) < 2 || GET_LEVEL(ch) >= IMO) {
				if (number(0, 18000)==900 || number(0,18000) == 176 ) {
					obj->obj_flags.extra_flags ^= ITEM_NORENT;
					act("$p is blessed by ThunderBolt.",FALSE,ch,obj,0,TO_CHAR);
				}
				obj->affected[0].modifier *= number(1, 2);
				act("$p gain new energy.",FALSE,ch,obj,0,TO_CHAR);
				if (number(0,8) < 6) 
					obj->affected[1].modifier *= number(1, 3);
				if (number(0, 8) < 5)
					obj->obj_flags.value[1] += number(1, level / 10 + 1);
				if (number(0, 8) < 4) {
					obj->obj_flags.value[2] += number(1, level / 10 + 1);
					act("$p BRIGHTS WITH LIGHT.",FALSE,ch,obj,0,TO_CHAR);
				}	
				if (number(0, 99) == 13) {  
					obj->obj_flags.value[1] += number(1, level / 10 + 1) * 3;
					act("$p BRIGHTS WITH GLOWING AURA.",FALSE,ch,obj,0,TO_CHAR);
				}
				else if (number(0, 99) == 29) {
					obj->obj_flags.value[2] += number(1, level / 10 + 1) * 3;
					act("$p BRIGHTS WITH GLOWING LIGHT.",
						FALSE,ch,obj,0,TO_CHAR);
				}
				else if (number(0, 99) == 57 || number(0, 99) == 91 ||
					number(0, 99) == 71) {
					obj->affected[0].modifier *= number(2, 4);
					obj->affected[1].modifier *= number(2, 4);
					act("$p BRIGHTS WITH DARKLIGHT.",FALSE,ch,obj,0,TO_CHAR);
				}
				if (number(0, 9) > 7) { 
					obj->obj_flags.value[1] += (1 + number(0, 1));
					obj->obj_flags.value[2] += (1 + number(0, 1));
					act("$p IS SHARPENED BY EVIL XARK.",FALSE,ch,obj,0,TO_CHAR);
				}
			}
			else {
				if (!number(0, 9))  {
					act("Ba..n..$p nearly explodes, but quiet.",
						FALSE, ch, obj, 0, TO_CHAR);
				}
				else {
					extract_obj(obj);
					act("BANG!!!! $p explodes. It hurts!",
						FALSE, ch, obj, 0, TO_CHAR);
					GET_HIT(ch) -= GET_HIT(ch) / number(5, 20);
				}
			}
		}
	}
}

void spell_enchant_armor(byte level, struct char_data *ch,
  struct char_data *victim, struct obj_data *obj)
{
	int i;
	struct obj_data *tmp;

	if (noenchantflag && GET_LEVEL(ch) < IMO)
		return;

    if(!IS_NPC(ch) && GET_LEVEL(ch) == IMO){
		send_to_char("You cannot enchant!\n\r",ch);
        return;
    }

	assert(ch && obj);

	for (i = 0; i < MAX_WEAR; i++) {
		tmp = ch->equipment[i];
		if (obj == tmp) {
			send_to_char("You cannot enchant it while you are wearing ",ch);
			return;
		}
	}
	if (GET_ITEM_TYPE(obj) == ITEM_ARMOR) {
		INCREASE_SKILLED2(ch, ch, SPELL_ENCHANT_ARMOR);
		SET_BIT(obj->obj_flags.extra_flags,ITEM_NORENT);
		if (!IS_SET(obj->obj_flags.extra_flags, ITEM_MAGIC)) {
			SET_BIT(obj->obj_flags.extra_flags, ITEM_MAGIC);

			if (obj->affected[0].location != APPLY_NONE) {
				obj->affected[0].modifier *= number(1, 2);
			}
			else {
				obj->affected[0].location = APPLY_AC;
				obj->affected[0].modifier = -(level / 10 + 1);
			}

			if (IS_GOOD(ch)) {
				SET_BIT(obj->obj_flags.extra_flags, ITEM_ANTI_EVIL);
				act("$p glows blue.",FALSE,ch,obj,0,TO_CHAR);
			}
			else if (IS_EVIL(ch)) {
				SET_BIT(obj->obj_flags.extra_flags, ITEM_ANTI_GOOD);
				act("$p glows red.", FALSE, ch, obj, 0, TO_CHAR);
			}
			else {
				act("$p glows yellow.", FALSE, ch, obj, 0, TO_CHAR);
			}
		} 
		else {
			if (obj->affected[0].location != APPLY_NONE) {
				obj->affected[0].modifier *= number(2, level / 10) / 2;
			}
			else {
				obj->affected[0].location = APPLY_AC;
				obj->affected[0].modifier = number(2, level / 10 + 2) / -2;
			}
			if (obj->affected[1].location != APPLY_NONE) 
				obj->affected[1].modifier *= number(2, level / 10) / 2; 
			else {
				obj->affected[1].location = APPLY_AC;
				obj->affected[1].modifier = number(2, level / 10 + 2) / -2;
			}
			if (number(0, 9) < 2 || GET_LEVEL(ch) >= IMO) {
				obj->affected[0].modifier *= number(1, 2);
				act("$p gain new energy.", FALSE, ch, obj, 0, TO_CHAR);
				if (number(0, 9) < 5) 
					obj->affected[1].modifier *= number(1, 2);
				if (number(0, 9) < 4) {
					obj->obj_flags.value[0] += number(1, level / 10 + 1);
					act("$p GLOWING WITH GREEN LIGHT.",
						FALSE, ch, obj, 0, TO_CHAR);
				}
				if (number(0, 99) == 11) {  
					obj->obj_flags.value[0] += number(1, level / 10 + 1) * 4;
					act("$p BRIGHT WITH GLOWING AURA.",
						FALSE, ch, obj, 0, TO_CHAR);
				}
				else if (number(0, 99) == 88) {
					obj->affected[0].modifier *= number(2, 4);
					obj->affected[1].modifier *= number(2, 4);
					act("$p BRIGHT WITH GLOWING LIGHT.",
						FALSE, ch, obj, 0, TO_CHAR);
				}
			}
			else {
				if (!number(0, 9))  {
					act("Ba..n..$p nearly explodes, but quiet.",
						FALSE, ch, obj, 0, TO_CHAR);
				}
				else {
					extract_obj(obj);
					act("BANG!!!! $p explodes. It hurts!",
						FALSE, ch, obj, 0, TO_CHAR);
					GET_MANA(ch) -= GET_MANA(ch) / number(5, 20);
				}
			}
		}
	}
}



void spell_pray_for_armor(byte level, struct char_data *ch,
  struct char_data *victim, struct obj_data *obj)
{
	int i;
	struct obj_data *tmp;

	if (noenchantflag && GET_LEVEL(ch) < IMO)
		return;

	if(!IS_NPC(ch) && GET_LEVEL(ch) == IMO){
			send_to_char("You cannot cast pray for armor!\n\r",ch);
			return;
	}

	assert(ch && obj);

	for (i = 0; i < MAX_WEAR; i++) {
		tmp = ch->equipment[i];
		if (obj == tmp) {
			send_to_char("You cannot pray it while you are wearing ",ch);
			return;
		}
	}
	if (GET_ITEM_TYPE(obj) == ITEM_ARMOR) {
		INCREASE_SKILLED2(ch, ch, SPELL_PRAY_FOR_ARMOR);
		SET_BIT(obj->obj_flags.extra_flags,ITEM_NORENT);
		if (!IS_SET(obj->obj_flags.extra_flags, ITEM_MAGIC)) {
			SET_BIT(obj->obj_flags.extra_flags, ITEM_MAGIC);

			if (obj->affected[0].location != APPLY_NONE) {
				obj->affected[0].modifier *= number(1, 2);
			}
			else {
				obj->affected[0].location = APPLY_AC;
				obj->affected[0].modifier = -(level / 10 + 1);
			}

			if (IS_GOOD(ch)) {
				SET_BIT(obj->obj_flags.extra_flags, ITEM_ANTI_EVIL);
				act("$p glows blue.", FALSE, ch, obj, 0, TO_CHAR);
			}
			else if (IS_EVIL(ch)) {
				SET_BIT(obj->obj_flags.extra_flags, ITEM_ANTI_GOOD);
				act("$p glows red.", FALSE, ch, obj, 0, TO_CHAR);
			}
			else {
				act("$p glows yellow.", FALSE, ch, obj, 0, TO_CHAR);
			}
		} 
		else {
			if (obj->affected[0].location != APPLY_NONE) {
				obj->affected[0].modifier *= number(2, level / 10) / 2;
			}
			else {
				obj->affected[0].location = APPLY_AC;
				obj->affected[0].modifier = number(2, level / 10 + 2) / -2;
			}
			if (obj->affected[1].location != APPLY_NONE) 
				obj->affected[1].modifier *= number(2, level / 10) / 2; 
			else {
				obj->affected[1].location = APPLY_AC;
				obj->affected[1].modifier = number(2, level / 10 + 2) / -2;
			}
			if (number(0, 9) < 2 || GET_LEVEL(ch) >= IMO) {
				obj->affected[0].modifier *= number(1, 2);
				act("$p gain new energy.", FALSE, ch, obj, 0, TO_CHAR);
				if (number(0, 9) < 5) 
					obj->affected[1].modifier *= number(1, 2);
				if (number(0, 9) < 4) {
					obj->obj_flags.value[0] += number(1, level / 10 + 1);
					act("$p GLOWING WITH GREEN LIGHT.",
						FALSE, ch, obj, 0, TO_CHAR);
				}
				if (number(0, 99) == 11) {  
					obj->obj_flags.value[0] += number(1, level / 10 + 1) * 4;
					act("$p BRIGHT WITH GLOWING AURA.",
						FALSE, ch, obj, 0, TO_CHAR);
				}
				else if (number(0, 99) == 88) {
					obj->affected[0].modifier *= number(2, 4);
					obj->affected[1].modifier *= number(2, 4);
					act("$p BRIGHT WITH GLOWING LIGHT.",
						FALSE, ch, obj, 0, TO_CHAR);
				}
			}
			else {
				if (!number(0, 9))  {
					act("Ba..n..$p nearly explodes, but quiet.",
						FALSE, ch, obj, 0, TO_CHAR);
				}
				else {
					extract_obj(obj);
					act("BANG!!!! $p explodes. It hurts!",
						FALSE, ch, obj, 0, TO_CHAR);
					GET_MANA(ch) -= GET_MANA(ch) / number(5, 20);
				}
			}
		}
	}
}

/* modified by atre */
void spell_self_heal(byte level, struct char_data *ch,
  struct char_data *victim, struct obj_data *obj)
{
	int hit;

	assert(victim);

	INCREASE_SKILLED2(ch, victim, SPELL_SELF_HEAL);
	spell_cure_blind(level, ch, victim, obj);

	/*
	hit = 1000 + (GET_LEVEL(victim) << 4);
	*/
	hit = dice(level, level + GET_SKILLED(ch, SPELL_SELF_HEAL));

	if (hit + GET_HIT(victim) >= GET_PLAYER_MAX_HIT(victim))
		GET_HIT(victim) = GET_PLAYER_MAX_HIT(victim);
	else
		GET_HIT(victim) += hit;

	update_pos(victim);

	send_to_char("You feel better.\n\r", victim);
}

void spell_restore_move(byte level, struct char_data *ch,
  struct char_data *victim, struct obj_data *obj)
{
	assert(victim);

	if (number(0, 7) > 2 + (GET_SKILLED(ch, SPELL_RESTORE_MOVE) >> 5)){
		WAIT_STATE(victim, PULSE_VIOLENCE * 3);
		send_to_char("Oooops! You can't move!!!\n\r", victim);
	}
	else {
		INCREASE_SKILLED2(ch, victim, SPELL_RESTORE_MOVE);
		GET_MOVE(victim) +=
			(GET_PLAYER_MAX_MOVE(victim) - GET_MOVE(victim)) / 5;
		send_to_char("You feel better.\n\r", victim);
	}
}

/* modified by atre */
void spell_heal(byte level, struct char_data *ch,
  struct char_data *victim, struct obj_data *obj)
{
	int hit;
	void gain_exp(struct char_data *ch, int gain);

	assert(victim);

	INCREASE_SKILLED2(ch, victim, SPELL_HEAL);
	spell_cure_blind(level, ch, victim, obj);

	/*
	hit = dice(level, 50);
	*/
	hit = dice((level << 1), level + GET_SKILLED(ch, SPELL_HEAL));

	if (hit + GET_HIT(victim) >= GET_PLAYER_MAX_HIT(victim))
		GET_HIT(victim) = GET_PLAYER_MAX_HIT(victim);
	else
		GET_HIT(victim) += hit;

	update_pos(victim);
	
	if( ch->player.class == 2 && !IS_MOB(victim)) {
		if ( GET_POS(victim) == 7 )
			gain_exp(ch, (int)(hit*GET_LEVEL(victim)*3));
		else 
			gain_exp(ch, (int)(hit*GET_LEVEL(victim)/5));
	}

	send_to_char("A warm feeling fills your body.\n\r", victim);
}

/* modified by atre */
void spell_full_heal(byte level, struct char_data *ch,
  struct char_data *victim, struct obj_data *obj)
{
	int hit;
	void gain_exp(struct char_data *ch, int gain);

	assert(victim);

	INCREASE_SKILLED2(ch, victim, SPELL_FULL_HEAL);
	spell_cure_blind(level, ch, victim, obj);

	/*
	hit = dice(level, 100);
	*/
	hit = dice((level << 2), (level << 1) + GET_SKILLED(ch, SPELL_FULL_HEAL));

	if (hit + GET_HIT(victim) >= GET_PLAYER_MAX_HIT(victim))
		GET_HIT(victim) = GET_PLAYER_MAX_HIT(victim);
	else
		GET_HIT(victim) += hit;

	update_pos(victim);

	if( ch->player.class == 2 && !IS_MOB(victim)) {
		if ( GET_POS(victim) == 7 )
			gain_exp(ch, (int)(hit*GET_LEVEL(victim)*3));
		else 
			gain_exp(ch, (int)(hit*GET_LEVEL(victim)/5));
	}

	send_to_char("A warm feeling fills your body.\n\r", victim);
}

void spell_entire_heal(byte level, struct char_data *ch,
  struct char_data *victim, struct obj_data *obj)
{
	assert(victim);

	INCREASE_SKILLED2(ch, victim, SPELL_ENTIRE_HEAL);
	if (number(0, 6) != 3 || GET_LEVEL(ch) >= (IMO+3)) {
		spell_cure_blind(level, ch, victim, obj);
  		GET_HIT(victim) = victim->points.max_hit;
  		send_to_char("Perhaps kisses your cheek.\n\r", victim);
	}
	else {
		GET_HIT(victim) /= 2;
  		send_to_char("UK! Perhaps slaps your cheek!\n\r", victim);
	}

  	update_pos( victim );
}

void spell_invisibility(byte level, struct char_data *ch,
  struct char_data *victim, struct obj_data *obj)
{
	struct affected_type af;

	assert((ch && obj) || victim);

	INCREASE_SKILLED2(ch, ch, SPELL_INVISIBLE);
	if (obj) {
		if (!IS_SET(obj->obj_flags.extra_flags, ITEM_INVISIBLE)) {
			act("$p turns invisible.",FALSE,ch,obj,0,TO_CHAR);
			act("$p turns invisible.",TRUE,ch,obj,0,TO_ROOM);
			SET_BIT(obj->obj_flags.extra_flags, ITEM_INVISIBLE);
		}
	}
	else {              /* Then it is a PC | NPC */
		if (!affected_by_spell(victim, SPELL_INVISIBLE)) {
			act("$n slowly fade out of existence.", TRUE, victim,0,0,TO_ROOM);
			send_to_char("You vanish.\n\r", victim);
			af.type      = SPELL_INVISIBLE;
			af.duration  = 24;
			af.modifier  = -level;
			af.location  = APPLY_AC;
			af.bitvector = AFF_INVISIBLE;
			affect_to_char(victim, &af);
		}
	}
}

void spell_locate_object(byte level, struct char_data *ch,
  struct char_data *victim, struct obj_data *obj)
{
	struct obj_data *i;
	char name[256];
	char buf[MAX_STRING_LENGTH];
	int j;

	assert(ch);
	INCREASE_SKILLED2(ch, ch, SPELL_LOCATE_OBJECT);
	strcpy(name, fname(obj->name));
	j = level >> 1;
	for (i = object_list; i && j; i = i->next) {
		if (isname(name, i->name)) {
			if (i->carried_by) {
				sprintf(buf,"%s carried by %s.\n\r",
					i->short_description, PERS(i->carried_by, ch));
				send_to_char(buf,ch);
			}
			else if (i->in_obj) {
				sprintf(buf,"%s in %s.\n\r",i->short_description,
					i->in_obj->short_description);
				send_to_char(buf,ch);
			}
			else {
				sprintf(buf,"%s in %s.\n\r",i->short_description,
					((i->in_room==NOWHERE) ? "uncertain." :
					world[i->in_room].name));
				send_to_char(buf,ch);
			}
			if (GET_LEVEL(ch) < IMO)
				j--;
		}
	}

	if (j == 0)
		send_to_char("You are very confused.\n\r",ch);
	if (j == (level >> 1))
		send_to_char("No such object.\n\r",ch);
}


void spell_poison(byte level, struct char_data *ch,
  struct char_data *victim, struct obj_data *obj)
{
	struct affected_type af;

	assert(victim || obj);

	if (victim) {
		if(!saves_spell(victim, SAVING_PARA)) {
			INCREASE_SKILLED2(ch, victim, SPELL_POISON);
			af.type = SPELL_POISON;
			af.duration = level / 5 + 1;
			af.modifier = -1;
			af.location = APPLY_STR;
			af.bitvector = AFF_POISON;
			affect_join(victim, &af, FALSE, FALSE);
			send_to_char("You feel very sick.\n\r", victim);
		}
	}
	else { /* Object poison */
		if ((obj->obj_flags.type_flag == ITEM_DRINKCON) ||
			(obj->obj_flags.type_flag == ITEM_FOOD)) {
			INCREASE_SKILLED2(ch, ch, SPELL_POISON);
			obj->obj_flags.value[3] = 1;
		}
	}
}

void spell_protection_from_evil(byte level, struct char_data *ch,
  struct char_data *victim, struct obj_data *obj)
{
	struct affected_type af;

	assert(victim);

	if (!affected_by_spell(victim, SPELL_PROTECT_FROM_EVIL)) {
		INCREASE_SKILLED2(ch, victim, SPELL_PROTECT_FROM_EVIL);
		af.type      = SPELL_PROTECT_FROM_EVIL;
		af.duration  = 24;
		af.modifier  = 0;
		af.location  = APPLY_NONE;
		af.bitvector = AFF_PROTECT_EVIL;
		affect_to_char(victim, &af);
		send_to_char("You have a righteous feeling!\n\r", victim);
	}
}


void spell_haste(byte level, struct char_data *ch,
  struct char_data *victim, struct obj_data *obj)
{
	struct affected_type af;

	assert(victim);

	if (!affected_by_spell(victim, SPELL_HASTE) &&
		!affected_by_spell(victim, SPELL_IMPROVED_HASTE)) {
		INCREASE_SKILLED2(ch, victim, SPELL_HASTE);
		af.type      = SPELL_HASTE;
		af.duration  = 5;
		af.modifier  = 0;
		af.location  = APPLY_NONE;
		af.bitvector = AFF_HASTE;
		affect_to_char(victim, &af);
		send_to_char("You feel world is going slowly!\n\r", victim);
	}
}

void spell_improved_haste(byte level, struct char_data *ch,
  struct char_data *victim, struct obj_data *obj)
{
	struct affected_type af;

	assert(victim);

	if (!affected_by_spell(victim, SPELL_HASTE) &&
       !affected_by_spell(victim, SPELL_IMPROVED_HASTE) ) {
		INCREASE_SKILLED2(ch, victim, SPELL_IMPROVED_HASTE);
		af.type      = SPELL_IMPROVED_HASTE;
		af.duration  = 3;
		af.modifier  = 0;
		af.location  = APPLY_NONE;
		af.bitvector = AFF_IMPROVED_HASTE;
		affect_to_char(victim, &af);
		send_to_char("You feel world is going slowly!\n\r", victim);
	}
}

void spell_remove_curse(byte level, struct char_data *ch,
  struct char_data *victim, struct obj_data *obj)
{
	assert(ch && (victim || obj));

	if (obj) {
		if (IS_SET(obj->obj_flags.extra_flags, ITEM_EVIL) ||
			IS_SET(obj->obj_flags.extra_flags, ITEM_NODROP)) {
			INCREASE_SKILLED2(ch, ch, SPELL_REMOVE_CURSE);
			act("$p briefly glows blue.", TRUE, ch, obj, 0, TO_CHAR);
			REMOVE_BIT(obj->obj_flags.extra_flags, ITEM_EVIL);
			REMOVE_BIT(obj->obj_flags.extra_flags, ITEM_NODROP);
		}
	}
	else {      /* Then it is a PC | NPC */
		if (affected_by_spell(victim, SPELL_CURSE)) {
			INCREASE_SKILLED2(ch, victim, SPELL_REMOVE_CURSE);
			act("$n briefly glows red, then blue.",FALSE,victim,0,0,TO_ROOM);
			act("You feel better.",FALSE,victim,0,0,TO_CHAR);
			affect_from_char(victim, SPELL_CURSE);
		}
	}
}


void spell_remove_poison(byte level, struct char_data *ch,
  struct char_data *victim, struct obj_data *obj)
{
	assert(ch && (victim || obj));

	if (victim) {
		if(affected_by_spell(victim,SPELL_POISON)) {
			INCREASE_SKILLED2(ch, ch, SPELL_REMOVE_POISON);
			affect_from_char(victim,SPELL_POISON);
			act("A warm feeling runs through your body.",
				FALSE, victim, 0, 0, TO_CHAR);
			act("$N looks better.",FALSE,ch,0,victim,TO_ROOM);
		}
	}
	else {
		if ((obj->obj_flags.type_flag == ITEM_DRINKCON) ||
			(obj->obj_flags.type_flag == ITEM_FOOD)) {
			INCREASE_SKILLED2(ch, victim, SPELL_REMOVE_POISON);
			obj->obj_flags.value[3] = 0;
			act("The $p steams briefly.",FALSE,ch,obj,0,TO_CHAR);
		}
	}
}


void spell_infravision(byte level, struct char_data *ch,
  struct char_data *victim, struct obj_data *obj)
{
	struct affected_type af;

	if (!affected_by_spell(victim, SPELL_INFRAVISION)) {
		INCREASE_SKILLED2(ch, victim, SPELL_INFRAVISION);
		act("$n's eyes glow red.",TRUE,victim,0,0,TO_ROOM);
		act("You feel your eyes become more sensitive.",
			TRUE, victim, 0, 0, TO_CHAR);
		af.type      = SPELL_INFRAVISION;
		af.duration  = 2 * level;
		af.modifier  = 0;
		af.location  = APPLY_NONE;
		af.bitvector = AFF_INFRAVISION;
		affect_to_char(victim, &af);
	}
}

void spell_mirror_image(byte level, struct char_data *ch,
  struct char_data *victim, struct obj_data *obj)
{
	struct affected_type af;

	if (!affected_by_spell(victim, SPELL_MIRROR_IMAGE)) {
		INCREASE_SKILLED2(ch, victim, SPELL_MIRROR_IMAGE);
		af.type      = SPELL_MIRROR_IMAGE;
		af.duration  = (level < IMO) ? 3 : level;
		af.duration += (level > 30) + (level > 35);
		af.modifier  = 0;
		af.location  = APPLY_NONE;
		af.bitvector = AFF_MIRROR_IMAGE;
		affect_to_char(victim, &af);
		act("$n creats own mirror image.",TRUE,victim,0,0,TO_ROOM);
		act("You made your illusion.",TRUE,victim,0,0,TO_CHAR);
	}
}

void spell_sanctuary(byte level, struct char_data *ch,
  struct char_data *victim, struct obj_data *obj)
{
	struct affected_type af;

	if (!affected_by_spell(victim, SPELL_SANCTUARY)) {
		INCREASE_SKILLED2(ch, victim, SPELL_SANCTUARY);
		act("$n is surrounded by a white aura.",TRUE,victim,0,0,TO_ROOM);
		act("You start glowing.",TRUE,victim,0,0,TO_CHAR);
		af.type      = SPELL_SANCTUARY;
		af.duration  = (level<IMO) ? 5 : level;
		af.duration += (level>30) + (level>35);
		af.modifier  = 0;
		af.location  = APPLY_NONE;
		af.bitvector = AFF_SANCTUARY;
		affect_to_char(victim, &af);
	}
}
/*
	Death spell made by process
	when death affect removes,the victim will make his own corpse!
*/
void spell_death(byte level,struct char_data *ch,
	struct char_data *victim,struct obj_data *obj)
{
	struct affected_type af;

	act("You can't do that!!!\n\r", FALSE, ch, 0, 0, TO_CHAR);
	return ;

	if (!affected_by_spell(victim,SPELL_DEATH)) {
		INCREASE_SKILLED2(ch, victim, SPELL_DEATH);
		act("$n is surrounded by a black-death mystery aura.\n\r",
			TRUE,victim,0,0,TO_ROOM);
		act("You feel extremely unease. YOU will be killed in 5 hours!!\n\r",
			TRUE,victim,0,0,TO_CHAR);
		af.type = SPELL_DEATH;
		af.duration = 5;
		af.modifier = 0;
		af.location = APPLY_NONE;
		af.bitvector = AFF_DEATH;
		affect_to_char(victim,&af);
	}
}

void spell_love(byte level, struct char_data *ch,
  struct char_data *victim, struct obj_data *obj)
{
	struct affected_type af;

	if (!affected_by_spell(victim, SPELL_LOVE)) {
		INCREASE_SKILLED2(ch, victim, SPELL_LOVE);
		act("$n is surrounded by a BRIGHT LIGHT.",TRUE,victim,0,0,TO_ROOM);
		act("You feel glorious.",TRUE,victim,0,0,TO_CHAR);
		af.type      = SPELL_LOVE;
		af.duration  = level/2;
		af.modifier  = 0;
		af.location  = APPLY_NONE;
		af.bitvector = AFF_LOVE;
		affect_to_char(victim, &af);
	}
}

/* written by wdshin */
void spell_reraise(byte level,struct char_data *ch,
 struct char_data *victim,struct obj_data *obj)
{
	struct affected_type af;

	act("You can't do that!!!\n\r", FALSE, ch, 0, 0, TO_CHAR);
	return ;

	if (!affected_by_spell(victim, SPELL_RERAISE)) {
		INCREASE_SKILLED2(ch, victim, SPELL_RERAISE);
		act("$n is surrounded by a Angelic Aura.",TRUE,victim,0,0,TO_ROOM);
		act("You feel lovely.",TRUE,victim,0,0,TO_CHAR);
		af.type		= SPELL_RERAISE;
		af.duration = 1;
		af.modifier = 0;
		af.location = APPLY_NONE;
		af.bitvector= AFF_RERAISE;
		affect_to_char(victim,&af);
	}
}

void spell_sleep(byte level, struct char_data *ch,
  struct char_data *victim, struct obj_data *obj)
{
	struct affected_type af;

	assert(victim);

	if (!IS_NPC(victim) &&
		(GET_LEVEL(victim) >= IMO) &&
		(GET_LEVEL(ch) < GET_LEVEL(victim)))
		return;
	if (GET_POS(victim) == POSITION_SLEEPING)
		return;

	if (!saves_spell(victim, SAVING_PARA)) {
		INCREASE_SKILLED2(ch, victim, SPELL_SLEEP);
		af.type      = SPELL_SLEEP;
		af.duration  = 4;
		af.modifier  = 0;
		af.location  = APPLY_NONE;
		af.bitvector = AFF_SLEEP;
		affect_join(victim, &af, FALSE, FALSE);
		if (GET_POS(victim) > POSITION_SLEEPING) {
			act("You feel very sleepy ..... zzzzzz",FALSE,victim,0,0,TO_CHAR);
			act("$n go to sleep.",TRUE,victim,0,0,TO_ROOM);
			GET_POS(victim) = POSITION_SLEEPING;
		}
	}
}

void spell_strength(byte level, struct char_data *ch,
  struct char_data *victim, struct obj_data *obj)
{
	struct affected_type af;

	assert(victim);

	INCREASE_SKILLED2(ch, victim, SPELL_STRENGTH);
	act("You feel stronger.",FALSE,victim,0,0,TO_CHAR);
	af.type      = SPELL_STRENGTH;
	af.duration  = level;
	af.modifier  = 1 + (level > 18);
	af.location  = APPLY_STR;
	af.bitvector = 0;
	affect_join(victim, &af, TRUE, FALSE);
}

void spell_ventriloquate(byte level, struct char_data *ch,
  struct char_data *victim, struct obj_data *obj)
{
  /* Not possible!! No argument! */
  INCREASE_SKILLED2(ch, victim, SPELL_VENTRILOQUATE);
}


void spell_phase(byte level, struct char_data *ch,
  struct char_data *victim, struct obj_data *obj)
{
  /* Not possible!! No argument! */
  INCREASE_SKILLED2(ch, victim, SPELL_PHASE);
}

void spell_word_of_recall(byte level, struct char_data *ch,
  struct char_data *victim, struct obj_data *obj)
{
	extern int top_of_world;
	int loc_nr, location;
	bool found = FALSE;

	assert(victim);

	if (IS_NPC(victim))
		return;

	INCREASE_SKILLED2(ch, victim, SPELL_WORD_OF_RECALL);
	switch (GET_GUILD(ch)) {
		case POLICE:
			loc_nr = ROOM_GUILD_POLICE;
			break;
		case ASSASSIN:
			loc_nr = ROOM_GUILD_ASSASSIN;
			break;
		case OUTLAW:
			loc_nr = ROOM_GUILD_OUTLAW;
			break;
		default:
			loc_nr = 3001;
			break;
	}
	for (location = 0; location <= top_of_world; location++)
		if (world[location].number == loc_nr) {
			found=TRUE;
			break;
		}

	if ((location == top_of_world) || !found) {
		send_to_char("You are completely lost.\n\r", victim);
		return;
	}
	if (ch && IS_AFFECTED(ch, AFF_DEATH)) {
		send_to_char("I'd better die here.\n\r", victim);
		return;
	}
	/* a location has been found. */
	act("$n disappears.", TRUE, victim, 0, 0, TO_ROOM);
	char_from_room(victim);
	char_to_room(victim, location);
	act("$n appears in the middle of the room.", TRUE, victim, 0, 0, TO_ROOM);
	do_look(victim, "", 15);
	victim->points.move = victim->points.move - 50;
}

void spell_summon(byte level, struct char_data *ch,
  struct char_data *victim, struct obj_data *obj)
{
	int target;

	assert(ch && victim);

    if (GET_LEVEL(victim) > GET_LEVEL(ch)) {
      send_to_char("You are too humble a soul.\n\r",ch);
      return;
    }

    if(IS_NPC(victim)){ /* mobile */
    	if(GET_LEVEL(victim) > 20){
      		send_to_char("You are too humble a soul.\n\r",ch);
      		return;
    	}
    }
    else if(GET_GUILD(ch) != GET_GUILD(victim)){
    	if(GET_LEVEL(victim) > 20){
      		send_to_char("You are too humble a soul.\n\r",ch);
      		return;
      	}
    }
    /* no guild member */
    else if(GET_GUILD(ch) == 0){
    	if(GET_LEVEL(victim) > 20){
      		send_to_char("You are too humble a soul.\n\r",ch);
      		return;
    	}
    }

	if (IS_SET(world[victim->in_room].room_flags, NOSUMMON) ||
		IS_SET(world[ch->in_room].room_flags,NOSUMMON)) {
		send_to_char("You nearly succeed, but not quite.\n\r", ch);
		return;
	}
	if (saves_spell(victim, SAVING_SPELL)) {
		send_to_char("You failed.\n\r", ch);
		return;
	}
	INCREASE_SKILLED2(ch, victim, SPELL_SUMMON);
	if (victim->specials.fighting) {
		if (victim->specials.fighting->specials.fighting == victim)
			stop_fighting(victim->specials.fighting);
		stop_fighting(victim);
	}
	act("$n disappears suddenly.",TRUE,victim,0,0,TO_ROOM);
	target = ch->in_room;
	char_from_room(victim);
	char_to_room(victim,target);

	if (victim->specials.was_in_room != NOWHERE)
		victim->specials.was_in_room = target;

	act("$n arrives suddenly.",TRUE,victim,0,0,TO_ROOM);
	act("$n has summoned you!",FALSE,ch,0,victim,TO_VICT);
	do_look(victim,"",15);
}


void spell_charm_person(byte level, struct char_data *ch,
  struct char_data *victim, struct obj_data *obj)
{
	struct affected_type af;
	char buf[200];

	void add_follower(struct char_data *ch, struct char_data *leader);
	bool circle_follow(struct char_data *ch, struct char_data *victim);
	void stop_follower(struct char_data *ch);

	assert(ch && victim);


    if(!IS_NPC(ch) && GET_LEVEL(ch) == IMO){
            send_to_char("You cannot use this spell.\n\r",ch);
            return;
    }

	/* By testing for IS_AFFECTED we avoid ei. Mordenkainens sword to be */
	/* able to be "recharmed" with duration                              */

	if (IS_NPC(ch))
		return;

	/* no charm player */
	if (!IS_NPC(victim))
		return;

	if (victim == ch) {
		send_to_char("You like yourself even better!\n\r", ch);
		return;
	}
	if (level < IMO && (level < GET_LEVEL(victim) || GET_LEVEL(victim) > 21)) {
		send_to_char("You failed.\n\r", ch);
		return;
	}
	if (IS_AFFECTED(ch, AFF_CHARM)) {
		send_to_char("Charmed person can't charm another person.\n\r", ch);
		return;
	}
	if (IS_AFFECTED(victim, AFF_CHARM)) {
		send_to_char("That person is already charmed.\n\r", ch);
		return;
	}
	if (circle_follow(victim, ch)) {
		send_to_char("Sorry, following in circles can ", ch);
		send_to_char("not be allowed.\n\r", ch);
		return;
	}
	if (GET_LEVEL(ch) < (IMO+3))
		if (saves_spell(victim, SAVING_PARA))
			return;

	INCREASE_SKILLED2(ch, victim, SPELL_CHARM_PERSON);
	if (victim->master)
		stop_follower(victim);
	add_follower(victim, ch);

	af.type = SPELL_CHARM_PERSON;
	if (GET_INT(ch) && GET_INT(victim))
		af.duration  = 24 * GET_INT(ch) / GET_INT(victim);
	else if (GET_INT(ch))
		af.duration  = 24 * GET_INT(ch);
	else
		af.duration = 1;
	af.modifier  = 0;
	af.location  = 0;
	af.bitvector = AFF_CHARM;
	affect_to_char(victim, &af);

	sprintf(buf, "%s CHARMED BY %s is here.\n\r",
		victim->player.short_descr, GET_NAME(ch));
	if (victim->player.long_descr) {
		free(victim->player.long_descr);
		victim->player.long_descr = 0;
	}
	CREATE(victim->player.long_descr, char, strlen(buf) + 1);
	strcpy(victim->player.long_descr, buf);

	sprintf(buf, "%s CHARMED BY %s",
		victim->player.short_descr, GET_NAME(ch));
	if (victim->player.short_descr) {
		free(victim->player.short_descr);
		victim->player.short_descr = 0;
	}
	CREATE(victim->player.short_descr, char, strlen(buf) + 1);
	strcpy(victim->player.short_descr, buf);
}

void spell_preach(byte level, struct char_data *ch,
  struct char_data *victim, struct obj_data *obj)
{
	assert(ch && victim);

	if (!IS_NPC(victim))
		return;

	if (GET_LEVEL(victim) > level)
	{
		send_to_char("You are too humble a soul.\n\r",ch);
		return;
	}

	if (IS_SET(victim->specials.act, ACT_AGGRESSIVE) &&
		!saves_spell(victim, SAVING_PARA)) {
		INCREASE_SKILLED2(ch, victim, SPELL_PREACH);
		REMOVE_BIT(victim->specials.act, ACT_AGGRESSIVE);
		act("$n looks less aggressive.",TRUE,victim,0,0,TO_ROOM);
	}
}

void spell_sense_life(byte level, struct char_data *ch,
  struct char_data *victim, struct obj_data *obj)
{
	struct affected_type af;

	assert(victim);

	if (!affected_by_spell(victim, SPELL_SENSE_LIFE)) {
		INCREASE_SKILLED2(ch, victim, SPELL_SENSE_LIFE);
		send_to_char("Your feel your awareness improve.\n\r", ch);
		af.type      = SPELL_SENSE_LIFE;
		af.duration  = 5*level;
		af.modifier  = 0;
		af.location  = APPLY_NONE;
		af.bitvector = AFF_SENSE_LIFE;
		affect_to_char(victim, &af);
	}
}

#define REAL 0
#define VIRTUAL 1

void spell_reanimate(byte level, struct char_data *ch,
  struct char_data *victim, struct obj_data *obj)
{
	void add_follower(struct char_data *ch, struct char_data *leader);
	struct char_data *mob; 
	struct char_data *read_mobile(int nr, int type);

	if ((obj->obj_flags.value[3] != 1) && (obj->obj_flags.value[3] != 2)) {
		send_to_char("There do not appear to be any corpses hereabouts?\n\r",
			ch);
		return;
	}

	INCREASE_SKILLED2(ch, ch, SPELL_REANIMATE);
	extract_obj(obj);
	mob = read_mobile(2, VIRTUAL);
	GET_GOLD(mob) = 0;
	char_to_room(mob, ch->in_room);
	act("$n has created a zombie!",TRUE,ch,0,0,TO_ROOM);
	send_to_char("You have created a zombie.\n\r",ch);
	add_follower(mob, ch);
	mob->points.max_hit += dice(level, 10);
	mob->points.hit = mob->points.max_hit;
}

void spell_clone(byte level, struct char_data *ch,
  struct char_data *victim, struct obj_data *obj)
{
	struct char_data *mob; 
	struct char_data *read_mobile(int nr, int type);
	char buf[256];

	if (obj) {
		send_to_char("Cloning object is not YET possible.\n\r",ch);
		return;
	}
	if (!victim) {
		send_to_char("Clone who?\n\r",ch);
		return;
	}
	if (IS_NPC(victim) && GET_LEVEL(ch) > GET_LEVEL(victim) &&
		GET_LEVEL(victim) < 30 &&
		mob_index[victim->nr].number <
		(GET_LEVEL(ch) - GET_LEVEL(victim)) / 3) {
		INCREASE_SKILLED2(ch, victim, SPELL_CLONE);
		mob = read_mobile(victim->nr, REAL);
		GET_EXP(mob) = 0;
		GET_GOLD(mob) = 0;
		char_to_room(mob,ch->in_room);
		sprintf(buf,"%s has been cloned!\n\r",
		victim->player.short_descr);
		send_to_room(buf,ch->in_room);
	}
	else {
		send_to_char("You may not clone THAT!\n\r",ch);
	}
}

