#include "structs.h"
#include "comm.h"
#include "magic_weapon.h"
#include "utils.h"
#include "spells.h"

int number(int from, int to);
void damage(struct char_data *ch, struct char_data *victim, int dam, int type);
int MIN(int a, int b);

void spell_lightning_bolt(byte level, struct char_data *ch,
  struct char_data *victim, struct obj_data *obj);
void spell_call_lightning(byte level, struct char_data *ch,
  struct char_data *victim, struct obj_data *obj);
void spell_fireball(byte level, struct char_data *ch,
  struct char_data *victim, struct obj_data *obj);
void spell_fire_breath(byte level, struct char_data *ch,
  struct char_data *victim, struct obj_data *obj);
void spell_frost_breath(byte level, struct char_data *ch,
  struct char_data *victim, struct obj_data *obj);
void spell_energy_drain(byte level, struct char_data *ch,
  struct char_data *victim, struct obj_data *obj);
void spell_disintegrate(byte level, struct char_data *ch,
  struct char_data *victim, struct obj_data *obj);
void spell_magic_missile(byte level, struct char_data *ch,
  struct char_data *victim, struct obj_data *obj);
void spell_full_heal(byte level, struct char_data *ch,
  struct char_data *victim, struct obj_data *obj);

bool affected_by_spell( struct char_data *ch, byte skill );
void affect_to_char( struct char_data *ch, struct affected_type *af );


extern struct weather_data weather_info;
extern struct room_data *world;

void magic_weapon_hit( struct char_data *ch, struct char_data *victim,
  struct obj_data *weapon )
{
  int d;

  if (!ch || !victim || !weapon) return;
  d = (weapon->obj_flags.gpd==1)? 1: number(1,weapon->obj_flags.gpd);
  if(d==1 && (ch != 0) && (victim != 0) ){ /* By Knife */
    switch(weapon->obj_flags.value[0]){
      case WEAPON_SMASH: sweapon_smash(ch,victim,weapon); break;
      case WEAPON_FLAME: sweapon_flame(ch,victim,weapon); break;
      case WEAPON_ICE:   sweapon_ice  (ch,victim,weapon); break;
      case WEAPON_BOMBARD:  sweapon_bombard(ch,victim,weapon); break;
      case WEAPON_SHOT:  sweapon_shot (ch,victim,weapon);break;
      case WEAPON_DRAGON_SLAYER: sweapon_dragon_slayer(ch,victim,weapon); break;
      case WEAPON_ANTI_GOOD_WEAPON: sweapon_anti_good(ch,victim,weapon); break;
      case WEAPON_ANTI_EVIL_WEAPON: sweapon_anti_evil(ch,victim,weapon); break;
	  case WEAPON_LIGHTNING:
		spell_lightning_bolt(GET_LEVEL(ch),ch,victim,0); break;
	  case WEAPON_CALL_LIGHTNING:
		spell_call_lightning(GET_LEVEL(ch),ch,victim,0); break;
	  case WEAPON_FIREBALL:
		spell_fireball(GET_LEVEL(ch),ch,victim,0); break;
	  case WEAPON_FIRE_BREATH:
		spell_fire_breath(GET_LEVEL(ch),ch,victim,0); break;
	  case WEAPON_FROST_BREATH:
		spell_frost_breath(GET_LEVEL(ch),ch,victim,0); break;
	  case WEAPON_ENERGY_DRAIN:
		spell_energy_drain(GET_LEVEL(ch),ch,victim,0); break;
	  case WEAPON_DISINTEGRATE:
		spell_disintegrate(GET_LEVEL(ch),ch,victim,0); break;
	  case WEAPON_MAGIC_MISSILE:
		spell_magic_missile(GET_LEVEL(ch),ch,victim,0); break;
	  /*
	  case WEAPON_GOD:
		sweapon_god ( ch, victim, weapon ); break;
	  */
	  case WEAPON_ANY_MAGIC:
		switch(number(1,9))
		{
		case	1: sweapon_smash(ch,victim,weapon); break;
		case	2: sweapon_flame(ch,victim,weapon); break;
		case	3: sweapon_ice(ch,victim,weapon); break;
		case	4: sweapon_bombard(ch,victim,weapon); break;
		case	5:
			spell_lightning_bolt(GET_LEVEL(ch),ch,victim,0); break;
		case	6:
			spell_call_lightning(GET_LEVEL(ch),ch,victim,0); break;
		case	7:
			spell_energy_drain(GET_LEVEL(ch),ch,victim,0); break;
		case	8:
			spell_disintegrate(GET_LEVEL(ch),ch,victim,0); break;
		case	9:
			spell_full_heal(GET_LEVEL(ch),ch,ch,0);	break;
		}
      default: return;  
    }
  }
  else return;
}

void sweapon_smash(ch,victim,weapon)
struct char_data *ch, *victim;
struct obj_data *weapon;
{
  int dam=80;

  dam = dam - GET_INT(ch) - GET_WIS(ch);
  dam <<= 1; 
  if (GET_CLASS(ch) == CLASS_MAGIC_USER || GET_CLASS(ch) == CLASS_CLERIC)
	dam <<= 2;
  act("$p smashes $n with bone crushing sound.",TRUE,victim,weapon,0,TO_ROOM);
  act("You are smashed by $p. You are hard to stand.",TRUE,victim,weapon,0,TO_CHAR);
  damage(ch,victim,dam,TYPE_UNDEFINED);
  if(GET_POS(victim)>POSITION_SITTING) GET_POS(victim)=POSITION_SITTING;
}
 
void sweapon_flame(ch,victim,weapon)
struct char_data *ch, *victim;
struct obj_data *weapon;
{
  int dam;
  dam=number(50,100);
  dam<<=1;
  if ( OUTSIDE(ch) && (weather_info.sky == SKY_CLOUDLESS))
	dam <<= 2;
  act("Large flame from $o strikes $n. $n is burned."
    ,TRUE,victim,weapon,0,TO_ROOM);
  act("Large flame from $o strikes you. You are burned."
    ,TRUE,victim,weapon,0,TO_CHAR);
  if(!IS_NPC(victim))
	WAIT_STATE(victim,PULSE_VIOLENCE);
  damage(ch,victim,dam,TYPE_UNDEFINED);
}
 
void sweapon_ice(ch,victim,weapon)
struct char_data *ch, *victim;
struct obj_data *weapon;
{
  int dam;
  dam=number(50,150);
  dam<<=3;
  act("White beam from $o strikes $n. $n is frozen."
    ,TRUE,victim,weapon,0,TO_ROOM);
  act("White beam from $o strikes you. You are frozen."
    ,TRUE,victim,weapon,0,TO_CHAR);
  if(!IS_NPC(victim))
	WAIT_STATE(victim,PULSE_VIOLENCE);
  damage(ch,victim,dam,TYPE_UNDEFINED);
}

void sweapon_bombard(ch,victim,weapon)
struct char_data *ch, *victim;
struct obj_data *weapon;
{
  int dam;
  dam=number(GET_LEVEL(ch),GET_LEVEL(ch)+GET_LEVEL(victim));
  dam<<=3;
  act("   *** Bombard ***",TRUE,ch,weapon,0,TO_ROOM);
  act("   *** Bombard ***",TRUE,ch,weapon,0,TO_CHAR);
  WAIT_STATE(victim,PULSE_VIOLENCE);
  damage(ch,victim,dam,TYPE_UNDEFINED);
  if(GET_POS(victim)>POSITION_STUNNED) GET_POS(victim)=POSITION_STUNNED;
}

void sweapon_shot(ch,victim,weapon)
struct char_data *ch, *victim;
struct obj_data *weapon;
{
  int dam=150;
  dam = dam + (GET_STR(ch) << 3) + (GET_CON(ch) << 3);
  dam<<=2;
  act("$p glows with striking aura.",TRUE,victim,weapon,0,TO_ROOM);
  act("You see $p glows. It HURTS you.",TRUE,victim,weapon,0,TO_CHAR);
  damage(ch,victim,dam,TYPE_UNDEFINED);
}

void sweapon_dragon_slayer(ch,victim,weapon)
struct char_data *ch, *victim;
struct obj_data *weapon;
{
  int dam;
  if(IS_NPC(victim) && !IS_SET(victim->specials.act,ACT_DRAGON)) return;
  dam=number(GET_LEVEL(ch),2*GET_LEVEL(ch));
  dam<<=2;
  act("$p PENETRATES the $n's tough skin.",TRUE,victim,weapon,0,TO_ROOM);
  damage(ch,victim,dam,TYPE_UNDEFINED);
}

void sweapon_anti_good(ch,victim,weapon)
struct char_data *ch, *victim;
struct obj_data *weapon;
{
  int dam;
  dam=number(GET_LEVEL(ch),GET_LEVEL(ch)+GET_LEVEL(victim));
  dam-=GET_ALIGNMENT(ch);
  dam<<=1;
  if(!IS_EVIL(ch)){
    act("$n screams in pain. $n looks like being shocked by something.",
      TRUE,ch,weapon,0,TO_ROOM);
    act("You feel sudden pain from your hands. You are shocked.",
      TRUE,ch,weapon,0,TO_CHAR);
    dam=MIN(GET_HIT(ch)-1,2*dam);
    damage(victim,ch,dam,TYPE_UNDEFINED);
    return;
  }

  dam <<= 1;
  if(IS_GOOD(victim)){
    act("$p glows with dark aura.",TRUE,victim,weapon,0,TO_ROOM);
    act("You feel sudden darkness. It really did hurt."
      ,TRUE,victim,weapon,0,TO_CHAR);
    damage(ch,victim,dam,TYPE_UNDEFINED);
    return;
  }
}

void sweapon_anti_evil(ch,victim,weapon)
struct char_data *ch, *victim;
struct obj_data *weapon;
{
  int dam;
  dam=number(GET_LEVEL(ch),GET_LEVEL(ch)+GET_LEVEL(victim));
  dam+=GET_ALIGNMENT(ch);
  dam<<=1;
  if(!IS_GOOD(ch)){
    act("$n screams in pain. $n looks like being shocked by something.",
      TRUE,ch,weapon,0,TO_ROOM);
    act("You feel sudden pain from your hands. You are shocked.",
      TRUE,ch,weapon,0,TO_CHAR);
    dam=MIN(GET_HIT(ch)-1,2*dam);
    damage(victim,ch,dam,TYPE_UNDEFINED);
    return;
  }

  dam <<= 1;
  if(IS_EVIL(victim)){
    act("$p glows with bright aura.",TRUE,victim,weapon,0,TO_ROOM);
    act("You feel sudden brightness. It really did hurt."
      ,TRUE,victim,weapon,0,TO_CHAR);
    damage(ch,victim,dam,TYPE_UNDEFINED);
    return;
  }
}

void sweapon_god(ch,victim,weapon)
struct char_data *ch, *victim;
struct obj_data *weapon;
{
	struct affected_type af;
/*
	spell_blindness( 100, ch, victim, 0 );
	spell_crush_armor( 50, ch, victim, 0 );
	spell_poison ( 50, ch, victim, 0 );
	spell_curse ( 50, ch, victim, 0 );
*/
    if (!affected_by_spell(ch, SPELL_SANCTUARY)) {
        af.type      = SPELL_SANCTUARY;
        af.duration  = 20;
        af.modifier  = 0;
        af.location  = APPLY_NONE;
        af.bitvector = AFF_SANCTUARY;
        affect_to_char(ch, &af);
    }
    if (!affected_by_spell(ch, SPELL_BLESS)) {
        af.type      = SPELL_BLESS;
        af.duration  = 20;
        af.modifier  = 20;
        af.location  = APPLY_HITROLL;
        af.bitvector = 0;
        affect_to_char(ch, &af);
        af.modifier  = -20;
        af.location  = APPLY_SAVING_SPELL;
        affect_to_char(ch, &af);
	}
    if (!affected_by_spell(ch, SPELL_LOVE)) {
        af.type      = SPELL_LOVE;
        af.duration  = 20;
        af.modifier  = 0;
        af.location  = APPLY_NONE;
        af.bitvector = AFF_LOVE;
        affect_to_char(ch, &af);
    }
    if (!affected_by_spell(ch, SPELL_DAMAGE_UP)) {
        af.type      = SPELL_DAMAGE_UP;
        af.duration  = 20;
        af.modifier  = 7+number(1,10);
        af.location  = APPLY_DAMROLL;
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

	GET_POS(victim) = POSITION_STUNNED;
	if ( GET_HIT(victim) > 5000 )
		spell_disintegrate( 50, ch, victim, 0 );
	if ( GET_HIT(victim) > 5000 )
		spell_disintegrate( 50, ch, victim, 0 );
	WAIT_STATE(victim, PULSE_VIOLENCE*3);
	GET_POS(victim) = POSITION_STUNNED;
}
