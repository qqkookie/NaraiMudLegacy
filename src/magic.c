/* ************************************************************************
   *  file: magic.c , Implementation of spells.              Part of DIKUMUD *
   *  Usage : The actual effect of magic.                                    *
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

/* NOTE:  Weapon special magic constant #define WEPON_* moved to "spells.h"
   Value of weapon special magic is same as spell/skill/damage type number. */

#define	WEAPON_LIGHTNING 		SPELL_LIGHTNING_BOLT
#define	WEAPON_CALL_LIGHTNING		SPELL_CALL_LIGHTNING
#define	WEAPON_FIREBALL 		SPELL_FIREBALL
#define	WEAPON_FIRE_BREATH 		SPELL_FIRE_BREATH
#define	WEAPON_FROST_BREATH 		SPELL_FROST_BREATH
#define WEAPON_ENERGY_DRAIN 		SPELL_ENERGY_DRAIN
#define	WEAPON_DISINTEGRATE 		SPELL_DISINTEGRATE
#define WEAPON_MAGIC_MISSILE 		SPELL_MAGIC_MISSILE

/*
#define WEAPON_GIANT_SLAYER 9 
#define WEAPON_POLICE 10
#define WEAPON_OUTLAW 11
#define WEAPON_ASSASSIN 12
#define	WEAPON_BEHEAD 13

#define WEAPON_GOD 21
*/
/****************************************************************/
/* NOTE: This is OLD magic_weapon.c 				*/
/****************************************************************/

void sweapon_smash(ch, victim, weapon)
    struct char_data *ch, *victim;
    struct obj_data *weapon;
{
    int dam = 80;

    dam = dam - GET_INT(ch) - GET_WIS(ch);
    dam <<= 1;
    if (GET_CLASS(ch) == CLASS_MAGIC_USER || GET_CLASS(ch) == CLASS_CLERIC)
	dam <<= 2;
    act("$p smashes $n with bone crushing sound.", TRUE, victim, weapon, 0, TO_ROOM);
    act("You are smashed by $p. You are hard to stand.", TRUE, victim, weapon, 0, TO_CHAR);

    /* NOTE: damage type : TYPE_UNDEFINED -> WEAPON_* */ 
    damage(ch, victim, dam, WEAPON_SMASH);
    if (GET_POS(victim) > POS_SITTING)
	GET_POS(victim) = POS_SITTING;
}

void sweapon_flame(ch, victim, weapon)
    struct char_data *ch, *victim;
    struct obj_data *weapon;
{
    int dam;

    dam = number(50, 100);
    dam <<= 1;
    if (OUTSIDE(ch) && (weather_info.sky == SKY_CLOUDLESS))
	dam <<= 2;
    act("Large flame from $o strikes $n. $n is burned."
	,TRUE, victim, weapon, 0, TO_ROOM);
    act("Large flame from $o strikes you. You are burned."
	,TRUE, victim, weapon, 0, TO_CHAR);
    if (!IS_NPC(victim))
	WAIT_STATE(victim, PULSE_VIOLENCE);
    damage(ch, victim, dam, WEAPON_FLAME);
}

void sweapon_ice(ch, victim, weapon)
    struct char_data *ch, *victim;
    struct obj_data *weapon;
{
    int dam;

    dam = number(50, 150);
    dam <<= 3;
    act("White beam from $o strikes $n. $n is frozen."
	,TRUE, victim, weapon, 0, TO_ROOM);
    act("White beam from $o strikes you. You are frozen."
	,TRUE, victim, weapon, 0, TO_CHAR);
    if (!IS_NPC(victim))
	WAIT_STATE(victim, PULSE_VIOLENCE);
    damage(ch, victim, dam, WEAPON_ICE);
}

void sweapon_bombard(ch, victim, weapon)
    struct char_data *ch, *victim;
    struct obj_data *weapon;
{
    int dam;

    dam = number(GET_LEVEL(ch), GET_LEVEL(ch) + GET_LEVEL(victim));
    dam <<= 3;
    act("   *** Bombard ***", TRUE, ch, weapon, 0, TO_ROOM);
    act("   *** Bombard ***", TRUE, ch, weapon, 0, TO_CHAR);
    WAIT_STATE(victim, PULSE_VIOLENCE);
    damage(ch, victim, dam, WEAPON_BOMBARD);
    if (GET_POS(victim) > POS_STUNNED)
	GET_POS(victim) = POS_STUNNED;
}

void sweapon_shot(ch, victim, weapon)
    struct char_data *ch, *victim;
    struct obj_data *weapon;
{
    int dam = 150;

    dam = dam + (GET_STR(ch) << 3) + (GET_CON(ch) << 3);
    dam <<= 2;
    act("$p glows with striking aura.", TRUE, victim, weapon, 0, TO_ROOM);
    act("You see $p glows. It HURTS you.", TRUE, victim, weapon, 0, TO_CHAR);
    damage(ch, victim, dam, WEAPON_SHOT);
}

void sweapon_dragon_slayer(ch, victim, weapon)
    struct char_data *ch, *victim;
    struct obj_data *weapon;
{
    int dam;

    if (IS_NPC(victim) && !IS_SET(victim->specials.act, ACT_DRAGON))
	return;
    dam = number(GET_LEVEL(ch), 2 * GET_LEVEL(ch));
    dam <<= 2;
    act("$p PENETRATES the $n's tough skin.", TRUE, victim, weapon, 0, TO_ROOM);
    damage(ch, victim, dam, WEAPON_DRAGON_SLAYER);
}

void sweapon_anti_good(ch, victim, weapon)
    struct char_data *ch, *victim;
    struct obj_data *weapon;
{
    int dam;

    dam = number(GET_LEVEL(ch), GET_LEVEL(ch) + GET_LEVEL(victim));
    dam -= GET_ALIGNMENT(ch);
    dam <<= 1;
    if (!IS_EVIL(ch)) {
	act("$n screams in pain. $n looks like being shocked by something.",
	    TRUE, ch, weapon, 0, TO_ROOM);
	act("You feel sudden pain from your hands. You are shocked.",
	    TRUE, ch, weapon, 0, TO_CHAR);
	dam = MIN(GET_HIT(ch) - 1, 2 * dam);
	damage(victim, ch, dam, WEAPON_ANTI_ALIGN);
	return;
    }

    dam <<= 1;
    if (IS_GOOD(victim)) {
	act("$p glows with dark aura.", TRUE, victim, weapon, 0, TO_ROOM);
	act("You feel sudden darkness. It really did hurt."
	    ,TRUE, victim, weapon, 0, TO_CHAR);
	damage(ch, victim, dam, WEAPON_ANTI_GOOD_WEAPON);
	return;
    }
}

void sweapon_anti_evil(ch, victim, weapon)
    struct char_data *ch, *victim;
    struct obj_data *weapon;
{
    int dam;

    dam = number(GET_LEVEL(ch), GET_LEVEL(ch) + GET_LEVEL(victim));
    dam += GET_ALIGNMENT(ch);
    dam <<= 1;
    if (!IS_GOOD(ch)) {
	act("$n screams in pain. $n looks like being shocked by something.",
	    TRUE, ch, weapon, 0, TO_ROOM);
	act("You feel sudden pain from your hands. You are shocked.",
	    TRUE, ch, weapon, 0, TO_CHAR);
	dam = MIN(GET_HIT(ch) - 1, 2 * dam);
	damage(victim, ch, dam, WEAPON_ANTI_ALIGN);
	return;
    }

    dam <<= 1;
    if (IS_EVIL(victim)) {
	act("$p glows with bright aura.", TRUE, victim, weapon, 0, TO_ROOM);
	act("You feel sudden brightness. It really did hurt."
	    ,TRUE, victim, weapon, 0, TO_CHAR);
	damage(ch, victim, dam, WEAPON_ANTI_EVIL_WEAPON);
	return;
    }
}

void sweapon_god(ch, victim, weapon)
    struct char_data *ch, *victim;
    struct obj_data *weapon;
{
    struct affected_type af;
    extern bool affected_by_spell(struct char_data *ch, byte skill);
    extern void affect_to_char(struct char_data *ch,
			       struct affected_type *af);

/*
   spell_blindness( 100, ch, victim, 0 );
   spell_crush_armor( 50, ch, victim, 0 );
   spell_poison ( 50, ch, victim, 0 );
   spell_curse ( 50, ch, victim, 0 );
 */
    if (!affected_by_spell(ch, SPELL_SANCTUARY)) {
	af.type = SPELL_SANCTUARY;
	af.duration = 20;
	af.modifier = 0;
	af.location = APPLY_NONE;
	af.bitvector = AFF_SANCTUARY;
	affect_to_char(ch, &af);
    }
    if (!affected_by_spell(ch, SPELL_BLESS)) {
	af.type = SPELL_BLESS;
	af.duration = 20;
	af.modifier = 20;
	af.location = APPLY_HITROLL;
	af.bitvector = 0;
	affect_to_char(ch, &af);
	af.modifier = -20;
	af.location = APPLY_SAVING_SPELL;
	affect_to_char(ch, &af);
    }
    if (!affected_by_spell(ch, SPELL_LOVE)) {
	af.type = SPELL_LOVE;
	af.duration = 20;
	af.modifier = 0;
	af.location = APPLY_NONE;
	af.bitvector = AFF_LOVE;
	affect_to_char(ch, &af);
    }
    if (!affected_by_spell(ch, SPELL_DAMAGE_UP)) {
	af.type = SPELL_DAMAGE_UP;
	af.duration = 20;
	af.modifier = 7 + number(1, 10);
	af.location = APPLY_DAMROLL;
	af.bitvector = 0;
	affect_to_char(ch, &af);
    }
    if (!affected_by_spell(ch, SPELL_REFLECT_DAMAGE)) {
	af.type = SPELL_REFLECT_DAMAGE;
	af.duration = 20;
	af.modifier = -50;
	af.location = APPLY_AC;
	af.bitvector = AFF_REFLECT_DAMAGE;
	affect_to_char(ch, &af);
    }
    if (!affected_by_spell(ch, SKILL_SNEAK)) {
	af.type = SKILL_SNEAK;
	af.duration = GET_LEVEL(ch);
	af.modifier = 0;
	af.location = APPLY_NONE;
	af.bitvector = AFF_SNEAK;
	affect_to_char(ch, &af);
    }

    GET_POS(victim) = POS_STUNNED;
    if (GET_HIT(victim) > 5000)
	spell_disintegrate(50, ch, victim, 0);
    if (GET_HIT(victim) > 5000)
	spell_disintegrate(50, ch, victim, 0);
    WAIT_STATE(victim, PULSE_VIOLENCE * 3);
    GET_POS(victim) = POS_STUNNED;
} 

void magic_weapon_hit(struct char_data *ch, struct char_data *victim,
		      struct obj_data *weapon)
{
    int d;
    char buf[MAX_BUFSIZ];

    if (!ch || !victim || !weapon)
	return;
    d = (weapon->obj_flags.gpd == 1) ? 1 : number(1, weapon->obj_flags.gpd);
    if (d == 1 && (ch ) && (victim )) {		/* By Knife */
	switch (weapon->obj_flags.value[0]) {
	case WEAPON_SMASH:
	    sweapon_smash(ch, victim, weapon);
	    break;
	case WEAPON_FLAME:
	    sweapon_flame(ch, victim, weapon);
	    break;
	case WEAPON_ICE:
	    sweapon_ice(ch, victim, weapon);
	    break;
	case WEAPON_BOMBARD:
	    sweapon_bombard(ch, victim, weapon);
	    break;
	case WEAPON_SHOT:
	    sweapon_shot(ch, victim, weapon);
	    break;
	case WEAPON_DRAGON_SLAYER:
	    sweapon_dragon_slayer(ch, victim, weapon);
	    break;
	case WEAPON_ANTI_GOOD_WEAPON:
	    sweapon_anti_good(ch, victim, weapon);
	    break;
	case WEAPON_ANTI_EVIL_WEAPON:
	    sweapon_anti_evil(ch, victim, weapon);
	    break;
	case WEAPON_LIGHTNING:
	    spell_lightning_bolt(GET_LEVEL(ch), ch, victim, 0);
	    break;
	case WEAPON_CALL_LIGHTNING:
	    spell_call_lightning(GET_LEVEL(ch), ch, victim, 0);
	    break;
	case WEAPON_FIREBALL:
	    spell_fireball(GET_LEVEL(ch), ch, victim, 0);
	    break;
	case WEAPON_FIRE_BREATH:
	    spell_fire_breath(GET_LEVEL(ch), ch, victim, 0);
	    break;
	case WEAPON_FROST_BREATH:
	    spell_frost_breath(GET_LEVEL(ch), ch, victim, 0);
	    break;
	case WEAPON_ENERGY_DRAIN:
	    spell_energy_drain(GET_LEVEL(ch), ch, victim, 0);
	    break;
	case WEAPON_DISINTEGRATE:
	    spell_disintegrate(GET_LEVEL(ch), ch, victim, 0);
	    break;
	case WEAPON_MAGIC_MISSILE:
	    spell_magic_missile(GET_LEVEL(ch), ch, victim, 0);
	    break;
	/* 
	case WEAPON_GOD:
	    sweapon_god ( ch, victim, weapon );
	    break;
	*/
	case WEAPON_ANY_MAGIC:
	    switch (number(1, 9)) {
	    case 1:
		sweapon_smash(ch, victim, weapon);
		break;
	    case 2:
		sweapon_flame(ch, victim, weapon);
		break;
	    case 3:
		sweapon_ice(ch, victim, weapon);
		break;
	    case 4:
		sweapon_bombard(ch, victim, weapon);
		break;
	    case 5:
		spell_lightning_bolt(GET_LEVEL(ch), ch, victim, 0);
		break;
	    case 6:
		spell_call_lightning(GET_LEVEL(ch), ch, victim, 0);
		break;
	    case 7:
		spell_energy_drain(GET_LEVEL(ch), ch, victim, 0);
		break;
	    case 8:
		spell_disintegrate(GET_LEVEL(ch), ch, victim, 0);
		break;
	    case 9:
		spell_full_heal(GET_LEVEL(ch), ch, ch, 0);
		break;
	    }
	    break;
	default: 
	    /* NOTE: Log illegal type of weapon magic. */ 
	    sprintf(buf,"magic_weapon_hit(): Unknown magic %d of %s (#%d)",
		weapon->obj_flags.value[0], 
		weapon->name, GET_OBJ_VIRTUAL(weapon)); 

	    /* NOTE: REMOVE THIS CODE AFTER JAN '98 */
	    if ( weapon->obj_flags.value[0] > 0 && weapon->obj_flags.value[0] <= 9 ) 
		weapon->obj_flags.value[0] += 170;
	    if ( weapon->obj_flags.value[0] == 14 ) 
		weapon->obj_flags.value[0] = 30;
		
	    log(buf);
	}
    }
    return;
}

/* NOTE: NEW! This was part of spell_enchant_weapon() */
/* NOTE: Add weapon special magic to enchanted weapon */
void enchant_weapon_special( struct char_data *ch, struct obj_data *obj)
{
    int type;

    if (!obj || !ch ) 
	    return;
    SET_BIT(obj->obj_flags.extra_flags, ITEM_MAGIC);

    /* NOTE: At least enchanter should be able to use it */
    type = number(0, 15);
    if (type & 0x0001 && GET_CLASS(ch) != CLASS_MAGIC_USER)
	SET_BIT(obj->obj_flags.extra_flags, ITEM_ANTI_MAGE);
    if (type & 0x0002 && GET_CLASS(ch) != CLASS_CLERIC)
	SET_BIT(obj->obj_flags.extra_flags, ITEM_ANTI_CLERIC);
    if (type & 0x0004 && GET_CLASS(ch) != CLASS_THIEF)
	SET_BIT(obj->obj_flags.extra_flags, ITEM_ANTI_THIEF);
    if (type & 0x0008 && GET_CLASS(ch) != CLASS_WARRIOR)
	SET_BIT(obj->obj_flags.extra_flags, ITEM_ANTI_WARRIOR);

    /* NOTE:  More varied weapon magic type.
              Change according to WEPON_* renumbering. */ 
    do {
	type = number(1, 200); /* MAX DAMGE TYPE */
	obj->obj_flags.value[0] = type;

	switch (type) {
	case WEAPON_DRAGON_SLAYER:
	    obj->obj_flags.value[3] = 3;
	    obj->name = "dragon slayer";
	    obj->short_description = "Mystic Dragon Slayer";
	    obj->description =
		"Mystic Dragon Slayer lies here.\n\r";
	    break;
	case WEAPON_ANTI_EVIL_WEAPON:
	    SET_BIT(obj->obj_flags.extra_flags, ITEM_ANTI_EVIL);
	    obj->obj_flags.value[3] = 3;
	    obj->name = "silver spear";
	    obj->short_description = "Silver Spear";
	    obj->description =
		"Silver Spear lies here.\n\r";
	    break;
	case WEAPON_ANTI_GOOD_WEAPON:
	    SET_BIT(obj->obj_flags.extra_flags, ITEM_ANTI_GOOD);
	    obj->obj_flags.value[3] = 3;
	    obj->name = "demon blade";
	    obj->short_description = "Mighty Demon Blade";
	    obj->description =
		"Mighty Demon Blade lies here.\n\r";
	    break;
	case WEAPON_DISINTEGRATE:
	    obj->obj_flags.value[3] = 3;
	    obj->name = "sword disintegrate";
	    obj->short_description = "runed longsword";
	    obj->description =
		"Longsword runed by a word 'Disintergrate' lies here.\n\r";
	    break;

	case WEAPON_ANY_MAGIC:
	case WEAPON_SMASH:
	case WEAPON_FLAME:
	case WEAPON_ICE:
	case WEAPON_BOMBARD:	/* NOTE: Added */
	case WEAPON_SHOT:	/* NOTE: Added */
	    break;

	case WEAPON_LIGHTNING:
	case WEAPON_CALL_LIGHTNING:
	case WEAPON_FIREBALL:
	case WEAPON_FIRE_BREATH:
	case WEAPON_FROST_BREATH:
	case WEAPON_ENERGY_DRAIN:
	case WEAPON_MAGIC_MISSILE:	/* NOTE: Added */
	    break; 

	case TYPE_HIT:
	case TYPE_BLUDGEON:
	case TYPE_SLASH:
	case TYPE_PIERCE:
	case TYPE_WHIP:
	    /* NOTE: Probability of WEPON_ANY_MAGIC : 6/22 */
	    obj->obj_flags.value[0] = WEAPON_ANY_MAGIC;
	    break;

	default:
	    obj->obj_flags.value[0] = -1 ;
		break;
	}
    } while ( obj->obj_flags.value[0] < 0 );

    obj->obj_flags.gpd = number(1, 12);
}

/********************************************************************/ 
/* NOTE: defintion of saving_throw[] moved to constants.c */

/* NOTE: print_increased_skilled() is merged to increase_skilled() */
/* void print_increased_skilled(struct char_data *ch, int sk_no) */

/* NOTE: function version of INCREASE_SKILLED() macro   */
/* NOTE: INCREASE_SKILLED(ch,victim, sk_no)'s are too big for macro. */
/*       probabityity = 1/( skilled * mul + add )        */
/* max skilled = 100 */
void increase_skilled(struct char_data *ch, struct char_data *victim,
		      int sk_no, int mul, int add)
{
    char buf[MAX_BUFSIZ];
    int mob_nr;

    if(IS_NPC(ch) || number(0, GET_SKILLED(ch, sk_no) * mul + add))
	return;
/* NOTE: IMHO, Checking char or victim guild is pointless. 
   Even if case applying skill to same guild member is excluded, 
   there are plenty of ways to drill skills without harming player.
   And what about good skill like cast 'heal'?  */

    /* jhpark, skilled increase시 같은 길드멤버가 아닌지 확인 */
/*
   if(IS_NPC(victim) ||( ch == victim )
   || (ch->player.guild != victim->player.guild)) 
 */
    /* NOTE: test validity of increase after rolling dice       */
    /* victim이 pet이 아닌지 확인....       - jhpark        */
    /* NOTE: Check null or cloned/reanimated victim mobile */
    if( victim && IS_NPC(victim)) {
	mob_nr = GET_MOB_VIRTUAL(victim);

	if (IS_PETSHOP_MOB(mob_nr))
	    return;
	if (GET_EXP(victim) == 0)
	    return;
    }
    if (GET_SKILLED(ch, sk_no) < 100) {
	GET_SKILLED(ch, sk_no) = GET_SKILLED(ch, sk_no) + 1;
	/* NOTE: This is OLD print_increase_skilled() */
	/* NOTE: spells index starts from 1, not zero.   */
	sprintf(buf, "Your %s POWER is more skilled!\n\r", spells[sk_no]);
	send_to_char(buf, ch);
    } 
}

/* NOTE: This is FYI on INCREASE_SKILLED() */
#if NO_DEF
#define INCREASE_SKILLED(ch, sk_no) in "utils.h"
Original:
    if (!number(0, 99 + (GET_SKILLED(ch, sk_no) << 2))) { .....  }

By  JHPARK:
      /* skilled increase시 같은 길드멤버가 아닌지 확인 */
    if  ((IS_NPC(victim) || ch == victim) \
	 ||  (ch->player.guild != victim->player.guild)) { \
	    if (!number(0, 99 + (GET_SKILLED(ch, sk_no) << 2))) { ....  } }

By JHPARK NEW:
  /* victim이 pet이 아닌지 확인....    - jhpark */
#define INCREASE_SKILLED(ch, victim, sk_no)  \
	if((IS_NPC(victim) || ch == victim) \
	    || (ch->player.guild != victim->player.guild)) \
		{ \
		    if (mob_index[victim->nr].virtual != KITTEN &&	\
			mob_index[victim->nr].virtual != PUPPY &&	\
			mob_index[victim->nr].virtual != BEAGLE && 	\
			mob_index[victim->nr].virtual != ROTTWEILER &&	\
			mob_index[victim->nr].virtual != WOLF &&	\
			mob_index[victim->nr].virtual != JUGGERNAUT)	\
			{	\
			    if(!number(0,99+(GET_SKILLED(ch,sk_no)<<1))) {}}}
COOKIE:
#define INCREASE_SKILLED(ch, victim, sk_no) \
       increase_skilled(ch, victim, sk_no, 2, 99 )
void increase_skilled(struct char_data *ch, struct char_data *victim,
		      int sk_no, int mul, int add);

/* end of FYI */
#endif				/* NO_DEF */

bool saves_spell(struct char_data *ch, int save_type)
{
    int save;
    /* saving_throws[class][type][level] */
    extern byte saving_throws[4][5][LEVEL_SIZE] ;

    /* Negative apply_saving_throw makes saving throw better! */
    save = ch->specials.apply_saving_throw[save_type];

    if (!IS_NPC(ch))
	save += saving_throws[GET_CLASS(ch) - 1][save_type][GET_LEVEL(ch) - 1];

    return (MAX(1, save) < number(1, 100));
}

/* **********************************************************************
 * 		spell 'identify'					*
 * ******************************************************************** */ 

void identify_char(struct char_data *ch, struct char_data *victim )
{
    char buf[MAX_BUFSIZ]; 
    sprintf(buf, STRHAN( "%d Years,  %d Months,  %d Days,  %d Hours old.\n\r",
			"나이 %d 년 %d 달 %d 일 %d 시간 입니다.\n\r", ch ),
	    age(victim).year, age(victim).month,
	    age(victim).day, age(victim).hours);
    send_to_char(buf, ch);

    sprintf(buf, STRHAN( "Height %d cm  Weight %d pounds \n\r",
    		"키 %d cm  몸무게 %d 파운드 \n\r", ch ),
	    victim->player.height, victim->player.weight);
    send_to_char(buf, ch);

    /* NOTE: Show carrying items number and total weight. */
    sprintf(buf, STRHAN( "You are carring %d items of %d pounds total.\n\r",
	"당신은 %d 개의 물건 (총중량 %d 파운드)을 가지고 있습니다.\n\r", ch),
	    GET_CARRYING_N(victim), GET_CARRYING_W(victim));
    send_to_char(buf, ch);

    if (GET_LEVEL(victim) > 5) {
	sprintf(buf, "Str %d/%d,  Int %d,  Wis %d,  Dex %d,  Con %d\n\r",
	GET_STR(victim), GET_ADD(victim), GET_INT(victim),
	GET_WIS(victim), GET_DEX(victim), GET_CON(victim));
	send_to_char(buf, ch);
    }
}

void identify_object(struct char_data *ch, struct obj_data *obj )
{
    char buf[MAX_LINE_LEN], buf2[MAX_NAME_LEN], bufh[MAX_LINE_LEN], *msg; 
    int i, found;
    extern int ac_applicable(struct obj_data *obj_object); 

    /* For Objects */
    extern char *item_types[];
    extern char *extra_bits[];
    extern char *apply_types[];
    extern char *affected_bits[];

    send_to_char_han("You feel informed:\n\r",
		     "이런 정보를 알 수 있습니다:\n\r", ch);

    sprinttype(GET_ITEM_TYPE(obj), item_types, buf2);
    sprintf(buf, STRHAN( "Object '%s', Item type: %s\r\n", 
		    "물건 '%s', 종류: %s\r\n", ch), obj->name, buf2 );
    send_to_char(buf, ch);

    if (obj->obj_flags.bitvector) {
	sprintbit(obj->obj_flags.bitvector, affected_bits, buf2);
	sprintf(buf, STRHAN("Item will give you following abilities:  %s\r\n",
	     "이 물건은 다음과 같은 능력을 줍니다:  %s\r\n", ch), buf2);
	send_to_char(buf, ch);
    }

    sprintbit(obj->obj_flags.extra_flags, extra_bits, buf2);
    sprintf(buf, STRHAN("Item is: %s\r\n", "성질: %s\r\n", ch), buf2);
    send_to_char(buf, ch);

    sprintf(buf, 
	    STRHAN( "Weight: %d, Value: %d\n\r", "무게: %d, 값: %d\n\r", ch), 
	obj->obj_flags.weight, obj->obj_flags.cost);
    send_to_char(buf, ch);

    switch (GET_ITEM_TYPE(obj)) {
    case ITEM_SCROLL:
    case ITEM_POTION:
	sprintf(buf, STRHAN("Level %d spells of:\n\r", 
		"%d 레벨에 해당하는 마법:\n\r", ch), obj->obj_flags.value[0]);
	send_to_char(buf, ch);
	/* NOTE: spells index starts from 1, not zero.   */
	if (obj->obj_flags.value[1] > 0) {
	    sprinttype(obj->obj_flags.value[1] , spells, buf);
	    strcat(buf, "\n\r");
	    send_to_char(buf, ch);
	}
	if (obj->obj_flags.value[2] > 0) {
	    sprinttype(obj->obj_flags.value[2] , spells, buf);
	    strcat(buf, "\n\r");
	    send_to_char(buf, ch);
	}
	if (obj->obj_flags.value[3] > 0) {
	    sprinttype(obj->obj_flags.value[3] , spells, buf);
	    strcat(buf, "\n\r");
	    send_to_char(buf, ch);
	}
	break;
    case ITEM_WAND:
    case ITEM_STAFF:
	sprintf(buf, "Has %d charges, with %d charges left.\n\r",
		obj->obj_flags.value[1], obj->obj_flags.value[2]);
	send_to_char(buf, ch);

	sprintf(buf, STRHAN("Level %d spell of:\n\r", 
		"%d 레벨에 해당하는 마법:\n\r", ch), obj->obj_flags.value[0]);
	send_to_char(buf, ch);

	/* NOTE: spells index starts from 1, not zero.   */
	if (obj->obj_flags.value[3] > 0) {
	    sprinttype(obj->obj_flags.value[3], spells, buf);
	    strcat(buf, "\n\r");
	    send_to_char(buf, ch);
	}
	break;
    case ITEM_WEAPON:
    case ITEM_FIREWEAPON:
	sprintf(buf, "Damage Dice %s '%dD%d'\n\r", STRHAN("is", "는", ch ),
		obj->obj_flags.value[1], obj->obj_flags.value[2]);
	send_to_char(buf, ch);
	if (obj->obj_flags.gpd) {
	    sprintf(buf, "This is magic weapon.\n\r");
	    switch (obj->obj_flags.value[0]) {
	    case WEAPON_SMASH:
		msg = "smash\n\r"; break;
	    case WEAPON_FLAME:
		msg = "flame\n\r"; break;
	    case WEAPON_ICE:
		msg = "ice beam\n\r"; break;
	    case WEAPON_BOMBARD:
		msg = "bombard\n\r"; break;
	    case WEAPON_SHOT:
		msg = "shot\n\r"; break;
	    case WEAPON_DRAGON_SLAYER:
		msg = "dragon slayer\n\r"; break;
	    case WEAPON_ANTI_EVIL_WEAPON:
		msg = "anti evil\n\r"; break;
	    case WEAPON_ANTI_GOOD_WEAPON:
		msg = "anti good\n\r"; break;
	    /*
	    case WEAPON_GIANT_SLAYER:
		msg = "giant slayer\n\r"; break;
	    case WEAPON_BEHEAD:
		msg = "behead\n\r"; break;
	    */
	    case WEAPON_LIGHTNING:
		msg = "lightning\n\r"; break;
	    case WEAPON_CALL_LIGHTNING:
		msg = "call lightning\n\r"; break;
	    case WEAPON_FIREBALL:
		msg = "fireball\n\r"; break;
	    case WEAPON_FIRE_BREATH:
		msg = "fire breath\n\r"; break;
	    case WEAPON_FROST_BREATH:
		msg = "frost breath\n\r"; break;
	    case WEAPON_ENERGY_DRAIN:
		msg = "energy drain\n\r"; break;
	    case WEAPON_DISINTEGRATE:
		msg = "disintegrate\n\r"; break;
	    /*
	    case WEAPON_GOD:
		msg ="god blessed\n\r";
	    break;
	    */
	    case WEAPON_ANY_MAGIC:
		msg = "random magic\n\r"; break;
	    default:
		msg = "unknown magic\n\r";
	    }
	    send_to_char(msg, ch);
	}
	break;
    case ITEM_ARMOR:
	if (ac_applicable(obj)) {
	    sprintf(buf, STRHAN("AC-apply is %d\n\r", "무장: %d\n\r", ch),
		obj->obj_flags.value[0]);
	    send_to_char(buf, ch);
	}
	break;
    }
    found = FALSE;

    for (i = 0; i < MAX_OBJ_AFFECT; i++) {
	if ((obj->affected[i].location != APPLY_NONE) &&
	    (obj->affected[i].modifier != 0)) {
	    if (!found) {
		send_to_char_han("Can affect you as :\n\r",
				 "다음과 같은 작용을 합니다 :\n\r", ch);
		found = TRUE;
	    }

	    sprinttype(obj->affected[i].location, apply_types, buf2);
	    sprintf(buf, "    Affects : %s By %d\n\r", buf2,
		    obj->affected[i].modifier);
	    sprintf(bufh, "    기능 : %d 만큼의 %s\n\r",
		    obj->affected[i].modifier, buf2);
	    send_to_char_han(buf, bufh, ch);
	}
    }
} 

/* NOTE: Divide spell_identify() to 3 pieces:
    spell_identify(), identify_char() and identify_object() */
void spell_identify(byte level, struct char_data *ch,
		    struct char_data *victim, struct obj_data *obj)
{
    INCREASE_SKILLED2(ch, ch, SPELL_IDENTIFY);

    if (obj) 
	identify_object(ch, obj );

    else if (victim && !IS_NPC(victim))  /* victim */
	identify_char(ch, victim); 
    else 
	send_to_char_han("You learn nothing new.\n\r",
			 "별다른게 없네요.\n\r", ch);
   
}

/* ***************************************************************************
 *                     NPC spells..                                          *
 * ************************************************************************* */

extern void extract_obj(struct obj_data *obj);

void spell_fire_breath(byte level, struct char_data *ch,
		       struct char_data *victim, struct obj_data *obj)
{
    int dam;
    struct obj_data *burn;

    dam = level * GET_INT(ch) * 4;
    if (saves_spell(victim, SAVING_BREATH))
	dam >>= 1;
    if (!IS_NPC(victim))
	damage(ch, victim, dam, SPELL_FIRE_BREATH);

    /* And now for the damage on inventory */
    if (number(0, LEVEL_LIMIT+2) <= GET_LEVEL(ch)) {
	if (!saves_spell(victim, SAVING_BREATH)) {
	    for (burn = victim->carrying;
		 burn && (burn->obj_flags.type_flag != ITEM_SCROLL) &&
		 (burn->obj_flags.type_flag != ITEM_WAND) &&
		 (burn->obj_flags.type_flag != ITEM_STAFF) &&
		 (burn->obj_flags.type_flag != ITEM_NOTE) &&
		 (number(0, 4) <= 3);
		 burn = burn->next_content) ;
	    if (burn) {
		act("$o burns.", 0, victim, burn, 0, TO_CHAR);
		extract_obj(burn);
	    }
	}
    }
}


void spell_frost_breath(byte level, struct char_data *ch,
			struct char_data *victim, struct obj_data *obj)
{
    int dam;
    struct obj_data *frozen;

    dam = level * GET_INT(ch) * 3;
    if (saves_spell(victim, SAVING_BREATH))
	dam >>= 1;
    if (!IS_NPC(victim))
	damage(ch, victim, dam, SPELL_FROST_BREATH);

    /* And now for the damage on inventory */
    if (number(0, LEVEL_LIMIT+2) <= GET_LEVEL(ch)) {
	if (!saves_spell(victim, SAVING_BREATH)) {
	    for (frozen = victim->carrying;
		 frozen && (frozen->obj_flags.type_flag != ITEM_SCROLL) &&
		 (frozen->obj_flags.type_flag != ITEM_WAND) &&
		 (frozen->obj_flags.type_flag != ITEM_STAFF) &&
		 (frozen->obj_flags.type_flag != ITEM_NOTE) &&
		 (number(0, 4) <= 3);
		 frozen = frozen->next_content) ;
	    if (frozen) {
		act("$o breaks.", 0, victim, frozen, 0, TO_CHAR);
		extract_obj(frozen);
	    }
	}
    }
}

void spell_gas_breath(byte level, struct char_data *ch,
		      struct char_data *victim, struct obj_data *obj)
{
    int dam;
    struct obj_data *melt;

    dam = level * GET_INT(ch) * 2;
    if (saves_spell(victim, SAVING_BREATH))
	dam >>= 1;
    if (!IS_NPC(victim))
	damage(ch, victim, dam, SPELL_GAS_BREATH);

    /* And now for the damage on inventory */
    if (number(0, LEVEL_LIMIT+2) <= GET_LEVEL(ch)) {
	if (!saves_spell(victim, SAVING_BREATH)) {
	    for (melt = victim->carrying;
		 melt && (melt->obj_flags.type_flag != ITEM_SCROLL) &&
		 (melt->obj_flags.type_flag != ITEM_WAND) &&
		 (melt->obj_flags.type_flag != ITEM_STAFF) &&
		 (melt->obj_flags.type_flag != ITEM_NOTE) &&
		 (number(0, 4) <= 3);
		 melt = melt->next_content) ;
	    if (melt) {
		act("$o is melting away.", 0, victim, melt, 0, TO_CHAR);
		extract_obj(melt);
	    }
	}
    }
}

void spell_lightning_breath(byte level, struct char_data *ch,
			    struct char_data *victim, struct obj_data *obj)
{
    int dam;
    struct obj_data *explode;

    dam = level * GET_INT(ch) * 4;
    if (saves_spell(victim, SAVING_BREATH))
	dam >>= 1;
    if (!IS_NPC(victim))
	damage(ch, victim, dam, SPELL_LIGHTNING_BREATH);

    /* And now for the damage on inventory */
    if (number(0, LEVEL_LIMIT+2) <= GET_LEVEL(ch)) {
	if (!saves_spell(victim, SAVING_BREATH)) {
	    for (explode = victim->carrying;
		 explode && (explode->obj_flags.type_flag != ITEM_SCROLL) &&
		 (explode->obj_flags.type_flag != ITEM_WAND) &&
		 (explode->obj_flags.type_flag != ITEM_STAFF) &&
		 (explode->obj_flags.type_flag != ITEM_NOTE) &&
		 (number(0, 4) <= 3);
		 explode = explode->next_content) ;
	    if (explode) {
		act("$o explodes.", 0, victim, explode, 0, TO_CHAR);
		extract_obj(explode);
	    }
	}
    }
}

