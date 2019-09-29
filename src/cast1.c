/* ************************************************************************
   *  file: cast1.c , handling of magic.                   Part of DIKUMUD *
   *  Usage : Procedures handling all offensive magic.                       *
   *  Copyright (C) 1990, 1991 - see 'license.doc' for complete information. *
   ************************************************************************* */

#include <stdio.h>

#include "char.h"
#include "object.h"
#include "global.h"
#include "comm.h"
#include "play.h"
#include "spells.h"

void cast_identify(byte level, struct char_data *ch, char *arg, int type,
		   struct char_data *tar_ch, struct obj_data *tar_obj)
{
    switch (type) {
    case SPELL_TYPE_SPELL:
    case SPELL_TYPE_SCROLL:
	spell_identify(level, ch, tar_ch, tar_obj);
	break;
    default:
	log("Serious screw-up in identify!");
	break;
    }
}

void cast_fire_breath(byte level, struct char_data *ch, char *arg, int type,
		      struct char_data *tar_ch, struct obj_data *tar_obj)
{
    switch (type) {
    case SPELL_TYPE_SPELL:
	spell_fire_breath(level, ch, tar_ch, 0);
	break;		/* It's a spell.. But people can'c cast it! */
    default:
	log("Serious screw-up in firebreath!");
	break;
    }
}

void cast_frost_breath(byte level, struct char_data *ch, char *arg, int type,
		       struct char_data *tar_ch, struct obj_data *tar_obj)
{
    switch (type) {
    case SPELL_TYPE_SPELL:
	spell_frost_breath(level, ch, tar_ch, 0);
	break;		/* It's a spell.. But people can'c cast it! */
    default:
	log("Serious screw-up in frostbreath!");
	break;
    }
}

void cast_gas_breath(byte level, struct char_data *ch, char *arg, int type,
		     struct char_data *tar_ch, struct obj_data *tar_obj)
{
    switch (type) {
    case SPELL_TYPE_SPELL:
	for (tar_ch = world[ch->in_room].people;
	     tar_ch; tar_ch = tar_ch->next_in_room)
	    if ((tar_ch != ch) && (GET_LEVEL(tar_ch) < IMO))
		spell_gas_breath(level, ch, tar_ch, 0);
	break;
	/* THIS ONE HURTS!! */
    default:
	log("Serious screw-up in gasbreath!");
	break;
    }
}

void cast_lightning_breath(byte level, struct char_data *ch, char *arg, int type,
			 struct char_data *tar_ch, struct obj_data *tar_obj)
{
    switch (type) {
    case SPELL_TYPE_SPELL:
	spell_lightning_breath(level, ch, tar_ch, 0);
	break;		/* It's a spell.. But people can'c cast it! */
    default:
	log("Serious screw-up in lightningbreath!");
	break;
    }
}



void cast_burning_hands(byte level, struct char_data *ch, char *arg, int type,
			struct char_data *victim, struct obj_data *tar_obj)
{
    switch (type) {
    case SPELL_TYPE_SPELL:
	spell_burning_hands(level, ch, victim, 0);
	break;
    default:
	log("Serious screw-up in burning hands!");
	break;
    }
}


void cast_call_lightning(byte level, struct char_data *ch, char *arg, int type,
			 struct char_data *victim, struct obj_data *tar_obj)
{
    extern struct weather_data weather_info;

    switch (type) {
    case SPELL_TYPE_SPELL:
	if (OUTSIDE(ch) && (weather_info.sky >= SKY_RAINING)) {
	    spell_call_lightning(level, ch, victim, 0);
	}
	else {
	    send_to_char("You fail to call upon the lightning from the sky!\n\r", ch);
	}
	break;
    case SPELL_TYPE_POTION:
	if (OUTSIDE(ch) && (weather_info.sky >= SKY_RAINING)) {
	    spell_call_lightning(level, ch, ch, 0);
	}
	break;
    case SPELL_TYPE_SCROLL:
	if (OUTSIDE(ch) && (weather_info.sky >= SKY_RAINING)) {
	    if (victim)
		spell_call_lightning(level, ch, victim, 0);
	    else if (!tar_obj)
		spell_call_lightning(level, ch, ch, 0);
	}
	break;
    case SPELL_TYPE_STAFF:
	if (OUTSIDE(ch) && (weather_info.sky >= SKY_RAINING)) {
	    for (victim = world[ch->in_room].people;
		 victim; victim = victim->next_in_room)
		if (victim != ch)
		    spell_call_lightning(level, ch, victim, 0);
	}
	break;
    default:
	log("Serious screw-up in call lightning!");
	break;
    }
}


void cast_chill_touch(byte level, struct char_data *ch, char *arg, int type,
		      struct char_data *victim, struct obj_data *tar_obj)
{
    switch (type) {
    case SPELL_TYPE_SPELL:
	spell_chill_touch(level, ch, victim, 0);
	break;
    default:
	log("Serious screw-up in chill touch!");
	break;
    }
}


void cast_shocking_grasp(byte level, struct char_data *ch, char *arg, int type,
			 struct char_data *victim, struct obj_data *tar_obj)
{
    switch (type) {
    case SPELL_TYPE_SPELL:
	spell_shocking_grasp(level, ch, victim, 0);
	break;
    default:
	log("Serious screw-up in shocking grasp!");
	break;
    }
}


void cast_color_spray(byte level, struct char_data *ch, char *arg, int type,
		      struct char_data *victim, struct obj_data *tar_obj)
{
    switch (type) {
    case SPELL_TYPE_SPELL:
	spell_color_spray(level, ch, victim, 0);
	break;
    case SPELL_TYPE_SCROLL:
	if (victim)
	    spell_color_spray(level, ch, victim, 0);
	else if (!tar_obj)
	    spell_color_spray(level, ch, ch, 0);
	break;
    case SPELL_TYPE_WAND:
	if (victim)
	    spell_color_spray(level, ch, victim, 0);
	break;
    default:
	log("Serious screw-up in color spray!");
	break;
    }
}


void cast_earthquake(byte level, struct char_data *ch, char *arg, int type,
		     struct char_data *victim, struct obj_data *tar_obj)
{
    switch (type) {
    case SPELL_TYPE_SPELL:
    case SPELL_TYPE_SCROLL:
    case SPELL_TYPE_STAFF:
	spell_earthquake(level, ch, 0, 0);
	break;
    default:
	log("Serious screw-up in earthquake!");
	break;
    }
}

void cast_all_heal(byte level, struct char_data *ch, char *arg, int type,
		   struct char_data *victim, struct obj_data *tar_obj)
{
    switch (type) {
    case SPELL_TYPE_SPELL:
    case SPELL_TYPE_SCROLL:
    case SPELL_TYPE_STAFF:
	spell_all_heal(level, ch, 0, 0);
	break;
    default:
	log("Serious screw-up in all_heal!");
	break;
    }
}

void cast_sanctuary_cloud(byte level, struct char_data *ch, char *arg, int type,
			  struct char_data *victim, struct obj_data *tar_obj)
{
    switch (type) {
    case SPELL_TYPE_SPELL:
    case SPELL_TYPE_SCROLL:
    case SPELL_TYPE_STAFF:
	spell_sanctuary_cloud(level, ch, 0, 0);
	break;
    default:
	log("Serious screw-up in all_heal!");
	break;
    }
}


void cast_hand_of_god(byte level, struct char_data *ch, char *arg, int type,
		      struct char_data *victim, struct obj_data *tar_obj)
{
    switch (type) {
    case SPELL_TYPE_SPELL:
    case SPELL_TYPE_SCROLL:
    case SPELL_TYPE_STAFF:
	spell_hand_of_god(level, ch, 0, 0);
	break;
    default:
	log("Serious screw-up in hand of god!");
	break;
    }
}

void cast_firestorm(byte level, struct char_data *ch, char *arg, int type,
		     struct char_data *victim, struct obj_data *tar_obj)
{
    switch (type) {
    case SPELL_TYPE_SPELL:
    case SPELL_TYPE_SCROLL:
    case SPELL_TYPE_STAFF:
	spell_firestorm(level, ch, 0, 0);
	break;
    default:
	log("Serious screw-up in firestorm!");
	break;
    }
}

void cast_energy_drain(byte level, struct char_data *ch, char *arg, int type,
		       struct char_data *victim, struct obj_data *tar_obj)
{
    switch (type) {
    case SPELL_TYPE_SPELL:
	spell_energy_drain(level, ch, victim, 0);
	break;
    case SPELL_TYPE_POTION:
	spell_energy_drain(level, ch, ch, 0);
	break;
    case SPELL_TYPE_SCROLL:
	if (victim)
	    spell_energy_drain(level, ch, victim, 0);
	else if (!tar_obj)
	    spell_energy_drain(level, ch, ch, 0);
	break;
    case SPELL_TYPE_WAND:
	if (victim)
	    spell_energy_drain(level, ch, victim, 0);
	break;
    case SPELL_TYPE_STAFF:
	for (victim = world[ch->in_room].people;
	     victim; victim = victim->next_in_room)
	    if (victim != ch)
		spell_energy_drain(level, ch, victim, 0);
	break;
    default:
	log("Serious screw-up in energy drain!");
	break;
    }
}

void cast_disintegrate(byte level, struct char_data *ch, char *arg, int type,
		       struct char_data *victim, struct obj_data *tar_obj)
{
    switch (type) {
    case SPELL_TYPE_SPELL:
	spell_disintegrate(level, ch, victim, 0);
	break;
    default:
	log("Serious screw-up in disintegrate!");
	break;
    }
}

void cast_sunburst(byte level, struct char_data *ch, char *arg, int type,
		   struct char_data *victim, struct obj_data *tar_obj)
{
    switch (type) {
    case SPELL_TYPE_SPELL:
	spell_sunburst(level, ch, victim, 0);
	break;
    default:
	log("Serious screw-up in sunburst!");
	break;
    }
}

void cast_energyflow(byte level, struct char_data *ch, char *arg, int type,
		     struct char_data *victim, struct obj_data *tar_obj)
{
    switch (type) {
    case SPELL_TYPE_SPELL:
	spell_energyflow(level, ch, victim, 0);
	break;
    default:
	log("Serious screw-up in energyflow!");
	break;
    }
}

void cast_full_fire(byte level, struct char_data *ch, char *arg, int type,
		    struct char_data *victim, struct obj_data *tar_obj)
{
    /* 
       if (!IS_NPC(victim)&&GET_LEVEL(ch)<IMO&&!IS_NPC(ch)) {
       send_to_char("You can't use full fire to player!\n\r",ch); return; } */
    switch (type) {
    case SPELL_TYPE_SPELL:
	send_to_char("You are trying to concentrate your all energy\n\r", ch);
	act("$n tries to concentrate your energy.\n\r", FALSE, ch, 0, 0, TO_ROOM);
	send_to_char("You concentrated your energy to small bright sphere\n\r", ch);
	act("$n concentrated energy to small bright sphere.\n\r",
	    FALSE, ch, 0, 0, TO_ROOM);
	spell_full_fire(level, ch, victim, 0);
	send_to_char("You throw your energy sphere. You see sudden brightness.\n\r", ch);
	act("$n throws the energy sphere. You cannot see anything by sudden brghtness.\n\r"
	    ,FALSE, ch, 0, 0, TO_ROOM);
	send_to_char("You are exhausted.\n\r", ch);
	break;
    default:
	log("Serious screw-up in full fire!");
	break;
    }
}

void cast_throw(byte level, struct char_data *ch, char *arg, int type,
		struct char_data *victim, struct obj_data *tar_obj)
{
    /* 
       if (!IS_NPC(victim)&&GET_LEVEL(ch)<IMO&&!IS_NPC(ch)) {
       send_to_char("You can't use throw to player!\n\r",ch); return; } */
    switch (type) {
    case SPELL_TYPE_SPELL:
	spell_throw(level, ch, victim, 0);
	break;
    default:
	log("Serious screw-up in throw!");
	break;
    }
}

void cast_fireball(byte level, struct char_data *ch, char *arg, int type,
		   struct char_data *victim, struct obj_data *tar_obj)
{
    switch (type) {
    case SPELL_TYPE_SPELL:
	spell_fireball(level, ch, victim, 0);
	break;
    case SPELL_TYPE_SCROLL:
	if (victim)
	    spell_fireball(level, ch, victim, 0);
	else if (!tar_obj)
	    spell_fireball(level, ch, ch, 0);
	break;
    case SPELL_TYPE_WAND:
	if (victim)
	    spell_fireball(level, ch, victim, 0);
	break;
    default:
	log("Serious screw-up in fireball!");
	break;

    }
}

void cast_cone_of_ice(byte level, struct char_data *ch, char *arg, int type,
		      struct char_data *victim, struct obj_data *tar_obj)
{
    switch (type) {
    case SPELL_TYPE_SPELL:
	spell_cone_of_ice(level, ch, victim, 0);
	break;
    case SPELL_TYPE_SCROLL:
	if (victim)
	    spell_cone_of_ice(level, ch, victim, 0);
	else if (!tar_obj)
	    spell_cone_of_ice(level, ch, ch, 0);
	break;
    case SPELL_TYPE_WAND:
	if (victim)
	    spell_cone_of_ice(level, ch, victim, 0);
	break;
    default:
	log("Serious screw-up in cone_of_ice!");
	break;

    }
}


void cast_harm(byte level, struct char_data *ch, char *arg, int type,
	       struct char_data *victim, struct obj_data *tar_obj)
{
    switch (type) {
    case SPELL_TYPE_SPELL:
	spell_harm(level, ch, victim, 0);
	break;
    case SPELL_TYPE_POTION:
	spell_harm(level, ch, ch, 0);
	break;
    case SPELL_TYPE_STAFF:
	for (victim = world[ch->in_room].people;
	     victim; victim = victim->next_in_room)
	    if (victim != ch)
		spell_harm(level, ch, victim, 0);
	break;
    default:
	log("Serious screw-up in harm!");
	break;

    }
} 

void cast_lightning_bolt(byte level, struct char_data *ch, char *arg, int type,
			 struct char_data *victim, struct obj_data *tar_obj)
{
    switch (type) {
    case SPELL_TYPE_SPELL:
	spell_lightning_bolt(level, ch, victim, 0);
	break;
    case SPELL_TYPE_SCROLL:
	if (victim)
	    spell_lightning_bolt(level, ch, victim, 0);
	else if (!tar_obj)
	    spell_lightning_bolt(level, ch, ch, 0);
	break;
    case SPELL_TYPE_WAND:
	if (victim)
	    spell_lightning_bolt(level, ch, victim, 0);
	break;
    default:
	log("Serious screw-up in lightning bolt!");
	break;

    }
} 

void cast_magic_missile(byte level, struct char_data *ch, char *arg, int type,
			struct char_data *victim, struct obj_data *tar_obj)
{
    switch (type) {
    case SPELL_TYPE_SPELL:
	spell_magic_missile(level, ch, victim, 0);
	break;
    case SPELL_TYPE_SCROLL:
	if (victim)
	    spell_magic_missile(level, ch, victim, 0);
	else if (!tar_obj)
	    spell_magic_missile(level, ch, ch, 0);
	break;
    case SPELL_TYPE_WAND:
	if (victim)
	    spell_magic_missile(level, ch, victim, 0);
	break;
    default:
	log("Serious screw-up in magic missile!");
	break;

    }
}

void cast_clone(byte level, struct char_data *ch, char *arg, int type,
		struct char_data *tar_ch, struct obj_data *tar_obj)
{
    spell_clone(level, ch, tar_ch, tar_obj);
}


/* ************************************************************************
   *  file: spells3.c , Implementation of magic.             Part of NARAIMUD*
   *  Usage : All the non-offensive magic handling routines.                 *
   ************************************************************************* */ 

/*
   cast 'spell block' victim
   by process
 */
void cast_spell_block(byte level, struct char_data *ch, char *arg,
	       int type, struct char_data *tar_ch, struct obj_data *tar_obj)
{
    switch (type) {
    case SPELL_TYPE_SPELL:
	if (number(1, 10) > 5)
	    spell_spell_block(level, ch, tar_ch, 0);
	break;
    default:
	break;
    }
}

void cast_dumb(byte level, struct char_data *ch, char *arg,
	       int type, struct char_data *tar_ch, struct obj_data *tar_obj)
{
    switch (type) {
    case SPELL_TYPE_SPELL:
	spell_dumb(level, ch, tar_ch, 0);
	break;
    default:
	break;
    }
}

void cast_reflect_damage(byte level, struct char_data *ch, char *arg,
	       int type, struct char_data *tar_ch, struct obj_data *tar_obj)
{
    switch (type) {
    case SPELL_TYPE_SPELL:
	spell_reflect_damage(level, ch, tar_ch, 0);
	break;
    default:
	break;
    }
}

void cast_holy_shield(byte level, struct char_data *ch, char *arg,
	       int type, struct char_data *tar_ch, struct obj_data *tar_obj)
{
    switch (type) {
    case SPELL_TYPE_SPELL:
	spell_holy_shield(level, ch, tar_ch, 0);
	break;
    default:
	break;
    }
} 
/*
   cast 'mana transfer' victim
   by process
 */
void cast_mana_transfer(byte level, struct char_data *ch, char *arg,
	       int type, struct char_data *tar_ch, struct obj_data *tar_obj)
{
    switch (type) {
    case SPELL_TYPE_SPELL:
	spell_mana_transfer(level, ch, tar_ch, 0);
	break;
    default:
	break;
    }
}
/*
   cast 'kiss of process' no_victim
   by process
 */
void cast_kiss_of_process(byte level, struct char_data *ch, char *arg,
	       int type, struct char_data *tar_ch, struct obj_data *tar_obj)
{ 
    switch (type) {
    case SPELL_TYPE_SPELL:
	if (GET_MANA(ch) < 30000) {
	    send_to_char("You do not have enough mana\n\r", ch);
	    return;
	}
	GET_HIT(ch) = 1;
	GET_MANA(ch) = 1;
	GET_MOVE(ch) = 1;
    case SPELL_TYPE_WAND:
    case SPELL_TYPE_STAFF:
    case SPELL_TYPE_SCROLL:
	spell_kiss_of_process(level, ch, 0, 0);
	break;
    default:
	break;
    }
}

void cast_thunderbolt(byte level, struct char_data *ch, char *arg,
	       int type, struct char_data *tar_ch, struct obj_data *tar_obj)
{
    switch (type) {
    case SPELL_TYPE_SPELL:
	spell_thunderbolt(level, ch, tar_ch, 0);
	break;
    default:
	break;
    }
}
