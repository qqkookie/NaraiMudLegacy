/* ************************************************************************
*  file: spells2.c , Implementation of magic.             Part of DIKUMUD *
*  Usage : All the non-offensive magic handling routines.                 *
*  Copyright (C) 1990, 1991 - see 'license.doc' for complete information. *
************************************************************************* */

#include <stdio.h>
#include <string.h>

#include "structs.h"
#include "utils.h"
#include "comm.h"
#include "db.h"
#include "interpreter.h"
#include "spells.h"
#include "handler.h"

#include "guild_list.h"

/* Global data */

extern struct room_data *world;
extern struct char_data *character_list;
extern struct spell_info_type spell_info[MAX_SPL_LIST];
extern struct obj_data  *object_list;


/* Extern procedures */

int special(struct char_data *ch, int cmd, char *arg);
void update_pos( struct char_data *victim );
void damage(struct char_data *ch, struct char_data *victim,
            int damage, int weapontype);
void say_spell( struct char_data *ch, int si );
bool saves_spell(struct char_data *ch, sh_int spell);
void add_follower(struct char_data *ch, struct char_data *victim);
// char *strdup(char *str);
void log(char *str);
void do_look(struct char_data *ch, char *arg, int cmd);



void spell_armor(byte level, struct char_data *ch,
	struct char_data *victim, struct obj_data *obj);
void spell_teleport(byte level, struct char_data *ch,
	struct char_data *victim, struct obj_data *obj);
void spell_damage_up(byte level, struct char_data *ch,
	struct char_data *victim, struct obj_data *obj);
void spell_crush_armor(byte level, struct char_data *ch,
	struct char_data *victim, struct obj_data *obj);
void spell_bless(byte level, struct char_data *ch,
	struct char_data *victim, struct obj_data *obj);
void spell_blindness(byte level, struct char_data *ch,
	struct char_data *victim, struct obj_data *obj);
void spell_create_food(byte level, struct char_data *ch,
	struct char_data *victim, struct obj_data *obj);
void spell_create_water(byte level, struct char_data *ch,
	struct char_data *victim, struct obj_data *obj);
void spell_create_nectar(byte level, struct char_data *ch,
	struct char_data *victim, struct obj_data *obj);
void spell_create_golden_nectar(byte level, struct char_data *ch,
	struct char_data *victim, struct obj_data *obj);
void spell_cure_blind(byte level, struct char_data *ch,
	struct char_data *victim, struct obj_data *obj);
void spell_mana_boost(byte level, struct char_data *ch,
	struct char_data *victim, struct obj_data *obj);
void spell_vitalize(byte level, struct char_data *ch,
	struct char_data *victim, struct obj_data *obj);
void spell_trick(byte level, struct char_data *ch,
	struct char_data *victim, struct obj_data *obj);
void spell_cause_critic(byte level, struct char_data *ch,
	struct char_data *victim, struct obj_data *obj);
void spell_cure_critic(byte level, struct char_data *ch,
	struct char_data *victim, struct obj_data *obj);
void spell_cause_light(byte level, struct char_data *ch,
	struct char_data *victim, struct obj_data *obj);
void spell_cure_light(byte level, struct char_data *ch,
	struct char_data *victim, struct obj_data *obj);
void spell_curse(byte level, struct char_data *ch,
	struct char_data *victim, struct obj_data *obj);
void spell_detect_evil(byte level, struct char_data *ch,
	struct char_data *victim, struct obj_data *obj);
void spell_detect_invisibility(byte level, struct char_data *ch,
	struct char_data *victim, struct obj_data *obj);
void spell_detect_evil(byte level, struct char_data *ch,
	struct char_data *victim, struct obj_data *obj);
void spell_preach(byte level, struct char_data *ch,
	struct char_data *victim, struct obj_data *obj);
void spell_dispel_evil(byte level, struct char_data *ch,
	struct char_data *victim, struct obj_data *obj);
void spell_recharger(byte level, struct char_data *ch,
	struct char_data *victim, struct obj_data *obj);
void spell_enchant_weapon(byte level, struct char_data *ch,
	struct char_data *victim, struct obj_data *obj);
void spell_enchant_person(byte level, struct char_data *ch,
	struct char_data *victim, struct obj_data *obj);
void spell_enchant_armor(byte level, struct char_data *ch,
	struct char_data *victim, struct obj_data *obj);
void spell_pray_for_armor(byte level, struct char_data *ch,
	struct char_data *victim, struct obj_data *obj);
void spell_self_heal(byte level, struct char_data *ch,
	struct char_data *victim, struct obj_data *obj);
void spell_restore_move(byte level, struct char_data *ch,
	struct char_data *victim, struct obj_data *obj);
void spell_heal(byte level, struct char_data *ch,
	struct char_data *victim, struct obj_data *obj);
void spell_full_heal(byte level, struct char_data *ch,
	struct char_data *victim, struct obj_data *obj);
void spell_entire_heal(byte level, struct char_data *ch,
	struct char_data *victim, struct obj_data *obj);
void spell_invisibility(byte level, struct char_data *ch,
	struct char_data *victim, struct obj_data *obj);
void spell_locate_object(byte level, struct char_data *ch,
	struct char_data *victim, struct obj_data *obj);
void spell_poison(byte level, struct char_data *ch,
	struct char_data *victim, struct obj_data *obj);
void spell_protection_from_evil(byte level, struct char_data *ch,
	struct char_data *victim, struct obj_data *obj);
void spell_haste(byte level, struct char_data *ch,
	struct char_data *victim, struct obj_data *obj);
void spell_improved_haste(byte level, struct char_data *ch,
	struct char_data *victim, struct obj_data *obj);
void spell_remove_curse(byte level, struct char_data *ch,
	struct char_data *victim, struct obj_data *obj);
void spell_remove_poison(byte level, struct char_data *ch,
	struct char_data *victim, struct obj_data *obj);
void spell_infravision(byte level, struct char_data *ch,
	struct char_data *victim, struct obj_data *obj);
void spell_mirror_image(byte level, struct char_data *ch,
	struct char_data *victim, struct obj_data *obj);
void spell_death(byte level, struct char_data *ch,
	struct char_data *victim, struct obj_data *obj);
void spell_sanctuary(byte level, struct char_data *ch,
	struct char_data *victim, struct obj_data *obj);
void spell_love(byte level, struct char_data *ch,
	struct char_data *victim, struct obj_data *obj);
void spell_reraise(byte level, struct char_data *ch,
	struct char_data *victim, struct obj_data *obj);
void spell_sleep(byte level, struct char_data *ch,
	struct char_data *victim, struct obj_data *obj);
void spell_strength(byte level, struct char_data *ch,
	struct char_data *victim, struct obj_data *obj);
void spell_word_of_recall(byte level, struct char_data *ch,
	struct char_data *victim, struct obj_data *obj);
void spell_summon(byte level, struct char_data *ch,
	struct char_data *victim, struct obj_data *obj);
void spell_far_look(byte level, struct char_data *ch,
	struct char_data *victim, struct obj_data *obj);
void spell_relocate(byte level, struct char_data *ch,
	struct char_data *victim, struct obj_data *obj);
void spell_charm_person(byte level, struct char_data *ch,
	struct char_data *victim, struct obj_data *obj);
void spell_sense_life(byte level, struct char_data *ch,
	struct char_data *victim, struct obj_data *obj);
void spell_identify(byte level, struct char_data *ch,
	struct char_data *victim, struct obj_data *obj);
void spell_fire_breath(byte level, struct char_data *ch,
	struct char_data *victim, struct obj_data *obj);
void spell_frost_breath(byte level, struct char_data *ch,
	struct char_data *victim, struct obj_data *obj);
void spell_gas_breath(byte level, struct char_data *ch,
	struct char_data *victim, struct obj_data *obj);
void spell_lightning_breath(byte level, struct char_data *ch,
	struct char_data *victim, struct obj_data *obj);
void spell_reanimate(byte level, struct char_data *ch,
	struct char_data *victim, struct obj_data *obj);

void spell_ventriloquate(byte level, struct char_data *ch,
	struct char_data *victim, struct obj_data *obj);
void spell_phase(byte level, struct char_data *ch,
	struct char_data *victim, struct obj_data *obj);


void cast_armor( byte level, struct char_data *ch, char *arg, int type,
  struct char_data *tar_ch, struct obj_data *tar_obj )
{
  switch (type) {
    case SPELL_TYPE_SPELL:
      if ( affected_by_spell(tar_ch, SPELL_ARMOR) ){
        send_to_char("Nothing seems to happen.\n\r", ch);
        return;
      }
      if (ch != tar_ch)
        act("$N is protected by your deity.", FALSE, ch, 0, tar_ch, TO_CHAR);

      spell_armor(level,ch,tar_ch,0);
      break;
    case SPELL_TYPE_POTION:
      if ( affected_by_spell(ch, SPELL_ARMOR) )
        return;
      spell_armor(level,ch,ch,0);
      break;
    case SPELL_TYPE_SCROLL:
      if (tar_obj) return;
         if (!tar_ch) tar_ch = ch;
      if ( affected_by_spell(tar_ch, SPELL_ARMOR) )
        return;
      spell_armor(level,ch,ch,0);
      break;
    case SPELL_TYPE_WAND:
      if (tar_obj) return;
      if ( affected_by_spell(tar_ch, SPELL_ARMOR) )
        return;
      spell_armor(level,ch,ch,0);
      break;
      default : 
         log("Serious screw-up in armor!");
         break;
  }
}

void cast_teleport( byte level, struct char_data *ch, char *arg, int type,
  struct char_data *tar_ch, struct obj_data *tar_obj )
{
  switch (type) {
    case SPELL_TYPE_SCROLL:
    case SPELL_TYPE_POTION:
    case SPELL_TYPE_SPELL:
      if (!tar_ch)
        tar_ch = ch;
      spell_teleport(level, ch, tar_ch, 0);
      break;
    case SPELL_TYPE_WAND:
      if(!tar_ch) return;
      spell_teleport(level, ch, tar_ch, 0);
      break;
    case SPELL_TYPE_STAFF:
      for (tar_ch = world[ch->in_room].people ; 
           tar_ch ; tar_ch = tar_ch->next_in_room)
         if (tar_ch != ch) 
            spell_teleport(level, ch, tar_ch, 0);
      break;
      
    default : 
      log("Serious screw-up in teleport!");
      break;
  }
}


void cast_damage_up( byte level, struct char_data *ch, char *arg, int type,
  struct char_data *tar_ch,struct obj_data *tar_obj)
{
  switch (type) {
    case SPELL_TYPE_SPELL:
        if ( affected_by_spell(tar_ch, SPELL_DAMAGE_UP) ||
           (GET_POS(tar_ch) == POSITION_FIGHTING)) 
          {	 send_to_char("Nothing seems to happen.\n\r", ch);
          	return;      }
        spell_damage_up(level,ch,tar_ch,0);
         break;
    case SPELL_TYPE_POTION:
       if ( affected_by_spell(ch, SPELL_DAMAGE_UP) ||
        (GET_POS(ch) == POSITION_FIGHTING))
        return;
      spell_damage_up(level,ch,ch,0);
         break;
    default : 
         log("Serious screw-up in more!");
         break;
  }
}
void cast_crush_armor( byte level, struct char_data *ch, char *arg, int type,
  struct char_data *tar_ch,struct obj_data *tar_obj)
{
  switch (type) {
    case SPELL_TYPE_SPELL:
        if ( affected_by_spell(tar_ch, SPELL_DAMAGE_UP) ||
           (GET_POS(tar_ch) == POSITION_FIGHTING)) 
         {  send_to_char("Nothing seems to happen.\n\r", ch);
           return;		}
        spell_crush_armor(level,ch,tar_ch,0);
         break;
    default : 
         log("Serious screw-up in crush armor!");
         break;
  }
}

void cast_bless( byte level, struct char_data *ch, char *arg, int type,
  struct char_data *tar_ch, struct obj_data *tar_obj)
{
  switch (type) {
    case SPELL_TYPE_SPELL:
      if (tar_obj) {        /* It's an object */
        if ( IS_SET(tar_obj->obj_flags.extra_flags, ITEM_BLESS) ) {
          send_to_char("Nothing seems to happen.\n\r", ch);
          return;
        }
        spell_bless(level,ch,0,tar_obj);

      } else {              /* Then it is a PC | NPC */

        if ( affected_by_spell(tar_ch, SPELL_BLESS) ||
          (GET_POS(tar_ch) == POSITION_FIGHTING)) {
          send_to_char("Nothing seems to happen.\n\r", ch);
          return;
        } 
        spell_bless(level,ch,tar_ch,0);
      }
      break;
   case SPELL_TYPE_POTION:
       if ( affected_by_spell(ch, SPELL_BLESS) ||
        (GET_POS(ch) == POSITION_FIGHTING))
        return;
      spell_bless(level,ch,ch,0);
         break;
    case SPELL_TYPE_SCROLL:
      if (tar_obj) {        /* It's an object */
        if ( IS_SET(tar_obj->obj_flags.extra_flags, ITEM_BLESS) )
          return;
        spell_bless(level,ch,0,tar_obj);

      } else {              /* Then it is a PC | NPC */

        if (!tar_ch) tar_ch = ch;
        
        if ( affected_by_spell(tar_ch, SPELL_BLESS) ||
          (GET_POS(tar_ch) == POSITION_FIGHTING))
          return;
        spell_bless(level,ch,tar_ch,0);
      }
      break;
    case SPELL_TYPE_WAND:
      if (tar_obj) {        /* It's an object */
        if ( IS_SET(tar_obj->obj_flags.extra_flags, ITEM_BLESS) )
          return;
        spell_bless(level,ch,0,tar_obj);

      } else {              /* Then it is a PC | NPC */

        if ( affected_by_spell(tar_ch, SPELL_BLESS) ||
          (GET_POS(tar_ch) == POSITION_FIGHTING))
          return;
        spell_bless(level,ch,tar_ch,0);
      }
      break;
    default : 
         log("Serious screw-up in bless!");
         break;
  }
}



void cast_blindness( byte level, struct char_data *ch, char *arg, int type,
  struct char_data *tar_ch, struct obj_data *tar_obj )
{
  switch (type) {
    case SPELL_TYPE_SPELL:
      if ( IS_AFFECTED(tar_ch, AFF_BLIND) ){
        send_to_char("Nothing seems to happen.\n\r", ch);
        return;
      }
      spell_blindness(level,ch,tar_ch,0);
      break;
    case SPELL_TYPE_POTION:
      if ( IS_AFFECTED(ch, AFF_BLIND) )
        return;
      spell_blindness(level,ch,ch,0);
      break;
    case SPELL_TYPE_SCROLL:
         if (tar_obj) return;
         if (!tar_ch) tar_ch = ch;
      if ( IS_AFFECTED(ch, AFF_BLIND) )
        return;
      spell_blindness(level,ch,ch,0);
      break;
    case SPELL_TYPE_WAND:
         if (tar_obj) return;
      if ( IS_AFFECTED(ch, AFF_BLIND) )
        return;
      spell_blindness(level,ch,ch,0);
      break;
    case SPELL_TYPE_STAFF:
         for (tar_ch = world[ch->in_room].people ; 
              tar_ch ; tar_ch = tar_ch->next_in_room)
            if (tar_ch != ch) 
               if (!(IS_AFFECTED(tar_ch, AFF_BLIND)))
                  spell_blindness(level,ch,tar_ch,0);
         break;
    default : 
         log("Serious screw-up in blindness!");
         break;
  }
}

void cast_create_food( byte level, struct char_data *ch, char *arg, int type,
  struct char_data *tar_ch, struct obj_data *tar_obj )
{

  switch (type) {
    case SPELL_TYPE_SPELL:
      act("$n magically creates a mushroom.",FALSE, ch, 0, 0, TO_ROOM);
         spell_create_food(level,ch,0,0);
      break;
    case SPELL_TYPE_SCROLL:
         if(tar_obj) return;
         if(tar_ch) return;
         spell_create_food(level,ch,0,0);
      break;
    default : 
         log("Serious screw-up in create food!");
         break;
  }
}



void cast_create_water( byte level, struct char_data *ch, char *arg, int type,
  struct char_data *tar_ch, struct obj_data *tar_obj )
{
  switch (type) {
    case SPELL_TYPE_SPELL:
      if (tar_obj->obj_flags.type_flag != ITEM_DRINKCON) {
        send_to_char("It is unable to hold water.\n\r", ch);
        return;
      }
      spell_create_water(level,ch,0,tar_obj);
      break;
      default : 
         log("Serious screw-up in create water!");
         break;
  }
}


void cast_create_nectar( byte level, struct char_data *ch, char *arg, int type,
  struct char_data *tar_ch, struct obj_data *tar_obj )
{
  switch (type) {
    case SPELL_TYPE_SPELL:
      if (tar_obj->obj_flags.type_flag != ITEM_DRINKCON) {
        send_to_char("It is unable to hold nectar.\n\r", ch);
        return;
      }
      spell_create_nectar(level,ch,0,tar_obj);
      break;
      default : 
         log("Serious screw-up in create nectar!");
         break;
  }
}

void cast_create_golden_nectar( byte level, struct char_data *ch, char *arg, int type,
  struct char_data *tar_ch, struct obj_data *tar_obj )
{
  switch (type) {
    case SPELL_TYPE_SPELL:
      if (tar_obj->obj_flags.type_flag != ITEM_DRINKCON) {
        send_to_char("It is unable to hold golden_nectar.\n\r", ch);
        return;
      }
      spell_create_golden_nectar(level,ch,0,tar_obj);
      break;
      default : 
         log("Serious screw-up in create golden_nectar!");
         break;
  }
}


void cast_cure_blind( byte level, struct char_data *ch, char *arg, int type,
  struct char_data *tar_ch, struct obj_data *tar_obj )
{
  switch (type) {
    case SPELL_TYPE_SPELL:
      spell_cure_blind(level,ch,tar_ch,0);
      break;
    case SPELL_TYPE_POTION:
      spell_cure_blind(level,ch,ch,0);
      break;
    case SPELL_TYPE_STAFF:
         for (tar_ch = world[ch->in_room].people ; 
              tar_ch ; tar_ch = tar_ch->next_in_room)
            if (tar_ch != ch) 
               spell_cure_blind(level,ch,tar_ch,0);
         break;
    default : 
         log("Serious screw-up in cure blind!");
         break;
  }
}



void cast_mana_boost( byte level, struct char_data *ch, char *arg, int type,
  struct char_data *tar_ch, struct obj_data *tar_obj )
{
  switch (type) {
    case SPELL_TYPE_SPELL:
      spell_mana_boost(level,ch,tar_ch,0);
      break;
    case SPELL_TYPE_STAFF:
         for (tar_ch = world[ch->in_room].people ; 
              tar_ch ; tar_ch = tar_ch->next_in_room)
            if (tar_ch != ch) 
               spell_mana_boost(level,ch,tar_ch,0);
         break;
    default : 
         log("Serious screw-up in increase!");
         break;
  }
}
void cast_vitalize( byte level, struct char_data *ch, char *arg, int type,
  struct char_data *tar_ch, struct obj_data *tar_obj )
{
  switch (type) {
    case SPELL_TYPE_SPELL:
      spell_vitalize(level,ch,tar_ch,0);
      break;
    case SPELL_TYPE_WAND:
         if(tar_obj) return;
         if (!tar_ch) tar_ch = ch;
         spell_vitalize(level, ch, tar_ch, 0);
         break;
    default : 
         log("Serious screw-up in vital_move!");
         break;
  }
}

void cast_trick( byte level, struct char_data *ch, char *arg, int type,
  struct char_data *tar_ch, struct obj_data *tar_obj )
{
  switch (type) {
    case SPELL_TYPE_SPELL:
      spell_trick(level,ch,tar_ch,0);
      break;
      default : 
         log("Serious screw-up in trick!");
         break;
  }
}

void cast_cause_critic( byte level, struct char_data *ch, char *arg, int type,
  struct char_data *tar_ch, struct obj_data *tar_obj )
{
  switch (type) {
    case SPELL_TYPE_SPELL:
      spell_cause_critic(level,ch,tar_ch,0);
      break;
    case SPELL_TYPE_POTION:
      spell_cause_critic(level,ch,ch,0);
      break;
    case SPELL_TYPE_WAND:
      spell_cause_critic(level,ch,tar_ch,0);
      break;
    case SPELL_TYPE_STAFF:
         for (tar_ch = world[ch->in_room].people ; 
              tar_ch ; tar_ch = tar_ch->next_in_room)
            if (tar_ch != ch) 
               spell_cause_critic(level,ch,tar_ch,0);
         break;
      default : 
         log("Serious screw-up in cause critic!");
         break;

  }
}

void cast_cure_critic( byte level, struct char_data *ch, char *arg, int type,
  struct char_data *tar_ch, struct obj_data *tar_obj )
{
  switch (type) {
    case SPELL_TYPE_SPELL:
      spell_cure_critic(level,ch,tar_ch,0);
      break;
    case SPELL_TYPE_POTION:
      spell_cure_critic(level,ch,ch,0);
      break;
    case SPELL_TYPE_WAND:
      spell_cure_critic(level,ch,tar_ch,0);
      break;
    case SPELL_TYPE_STAFF:
         for (tar_ch = world[ch->in_room].people ; 
              tar_ch ; tar_ch = tar_ch->next_in_room)
            if (tar_ch != ch) 
               spell_cure_critic(level,ch,tar_ch,0);
         break;
      default : 
         log("Serious screw-up in cure critic!");
         break;

  }
}



void cast_cause_light( byte level, struct char_data *ch, char *arg, int type,
  struct char_data *tar_ch, struct obj_data *tar_obj )
{
  switch (type) {
    case SPELL_TYPE_SPELL:
      spell_cause_light(level,ch,tar_ch,0);
      break;
    case SPELL_TYPE_POTION:
      spell_cause_light(level,ch,ch,0);
      break;
    case SPELL_TYPE_STAFF:
         for (tar_ch = world[ch->in_room].people ; 
              tar_ch ; tar_ch = tar_ch->next_in_room)
            if (tar_ch != ch) 
               spell_cause_light(level,ch,tar_ch,0);
         break;
    default : 
         log("Serious screw-up in cause light!");
         break;
  }
}


void cast_cure_light( byte level, struct char_data *ch, char *arg, int type,
  struct char_data *tar_ch, struct obj_data *tar_obj )
{
  switch (type) {
    case SPELL_TYPE_SPELL:
      spell_cure_light(level,ch,tar_ch,0);
      break;
    case SPELL_TYPE_POTION:
      spell_cure_light(level,ch,ch,0);
      break;
    case SPELL_TYPE_STAFF:
         for (tar_ch = world[ch->in_room].people ; 
              tar_ch ; tar_ch = tar_ch->next_in_room)
            if (tar_ch != ch) 
               spell_cure_light(level,ch,tar_ch,0);
         break;
    default : 
         log("Serious screw-up in cure light!");
         break;
  }
}


void cast_curse( byte level, struct char_data *ch, char *arg, int type,
  struct char_data *tar_ch, struct obj_data *tar_obj )
{
  switch (type) {
    case SPELL_TYPE_SPELL:
      if (tar_obj)   /* It is an object */ 
        spell_curse(level,ch,0,tar_obj);
      else {              /* Then it is a PC | NPC */
        spell_curse(level,ch,tar_ch,0);
      }
      break;
    case SPELL_TYPE_POTION:
      spell_curse(level,ch,ch,0);
      break;
    case SPELL_TYPE_SCROLL:
      if (tar_obj)   /* It is an object */ 
        spell_curse(level,ch,0,tar_obj);
      else {              /* Then it is a PC | NPC */
        if (!tar_ch) tar_ch = ch;
        spell_curse(level,ch,tar_ch,0);
      }
      break;
    case SPELL_TYPE_STAFF:
         for (tar_ch = world[ch->in_room].people ; 
              tar_ch ; tar_ch = tar_ch->next_in_room)
            if (tar_ch != ch) 
               spell_curse(level,ch,tar_ch,0);
         break;
    default : 
         log("Serious screw-up in curse!");
         break;
  }
}


void cast_detect_evil( byte level, struct char_data *ch, char *arg, int type,
  struct char_data *tar_ch, struct obj_data *tar_obj )
{
  switch (type) {
    case SPELL_TYPE_SPELL:
      if ( affected_by_spell(tar_ch, SPELL_DETECT_EVIL) ){
        send_to_char("Nothing seems to happen.\n\r", tar_ch);
        return;
      }
      spell_detect_evil(level,ch,tar_ch,0);
      break;
    case SPELL_TYPE_POTION:
      if ( affected_by_spell(ch, SPELL_DETECT_EVIL) )
        return;
      spell_detect_evil(level,ch,ch,0);
      break;
    case SPELL_TYPE_STAFF:
         for (tar_ch = world[ch->in_room].people ; 
              tar_ch ; tar_ch = tar_ch->next_in_room)
            if (tar_ch != ch) 
               if(!(IS_AFFECTED(tar_ch, SPELL_DETECT_EVIL)))
                  spell_detect_evil(level,ch,tar_ch,0);
         break;
    default : 
         log("Serious screw-up in detect evil!");
         break;
  }
}



void cast_detect_invisibility( byte level, struct char_data *ch, char *arg, int type,
  struct char_data *tar_ch, struct obj_data *tar_obj )
{
  switch (type) {
    case SPELL_TYPE_SPELL:
      if ( affected_by_spell(tar_ch, SPELL_DETECT_INVISIBLE) ){
        send_to_char("Nothing seems to happen.\n\r", tar_ch);
        return;
      }
      spell_detect_invisibility(level,ch,tar_ch,0);
      break;
    case SPELL_TYPE_POTION:
      if ( affected_by_spell(ch, SPELL_DETECT_INVISIBLE) )
        return;
      spell_detect_invisibility(level,ch,ch,0);
      break;
    case SPELL_TYPE_STAFF:
         for (tar_ch = world[ch->in_room].people ; 
              tar_ch ; tar_ch = tar_ch->next_in_room)
            if (tar_ch != ch) 
               if(!(IS_AFFECTED(tar_ch, SPELL_DETECT_INVISIBLE)))
                  spell_detect_invisibility(level,ch,tar_ch,0);
         break;
    default : 
         log("Serious screw-up in detect invisibility!");
         break;
  }
}


void cast_preach( byte level, struct char_data *ch, char *arg, int type,
  struct char_data *tar_ch, struct obj_data *tar_obj )
{
  switch (type) {
    case SPELL_TYPE_SPELL:
      spell_preach(level, ch, tar_ch,0);
      break;
    case SPELL_TYPE_SCROLL:
      spell_preach(level, ch, tar_ch, 0);
      break;
    default : 
         log("Serious screw-up in preach!");
         break;
  }
}



void cast_dispel_evil( byte level, struct char_data *ch, char *arg, int type,
  struct char_data *tar_ch, struct obj_data *tar_obj )
{
  switch (type) {
    case SPELL_TYPE_SPELL:
      spell_dispel_evil(level, ch, tar_ch,0);
      break;
    case SPELL_TYPE_POTION:
      spell_dispel_evil(level,ch,ch,0);
      break;
    case SPELL_TYPE_SCROLL:
      if (tar_obj) return;
      if (!tar_ch) tar_ch = ch;
      spell_dispel_evil(level, ch, tar_ch,0);
      break;
    case SPELL_TYPE_WAND:
      if (tar_obj) return;
      spell_dispel_evil(level, ch, tar_ch,0);
      break;
    case SPELL_TYPE_STAFF:
         for (tar_ch = world[ch->in_room].people ; 
              tar_ch ; tar_ch = tar_ch->next_in_room)
            if (tar_ch != ch) 
              spell_dispel_evil(level,ch,tar_ch,0);
         break;
    default : 
         log("Serious screw-up in dispel evil!");
         break;
  }
}


void cast_recharger( byte level, struct char_data *ch, char *arg, int type,
  struct char_data *tar_ch, struct obj_data *tar_obj )
{
  switch (type) {
    case SPELL_TYPE_SPELL:
      spell_recharger(level, ch, 0,tar_obj);
      break;

    case SPELL_TYPE_SCROLL:
      if(!tar_obj) return;
      spell_recharger(level, ch, 0,tar_obj);
      break;
    default : 
      log("Serious screw-up in recharger!");
      break;
  }
}

void cast_enchant_weapon( byte level, struct char_data *ch, char *arg, int type,
  struct char_data *tar_ch, struct obj_data *tar_obj )
{
  switch (type) {
    case SPELL_TYPE_SPELL:
      spell_enchant_weapon(level, ch, 0,tar_obj);
      break;

    case SPELL_TYPE_SCROLL:
      if(!tar_obj) return;
      spell_enchant_weapon(level, ch, 0,tar_obj);
      break;
    default : 
      log("Serious screw-up in enchant weapon!");
      break;
  }
}

void cast_enchant_person( byte level, struct char_data *ch, char *arg, int type,
 struct char_data *tar_ch, struct obj_data *tar_obj)
{
  switch(type) {
	case SPELL_TYPE_SPELL:
	  spell_enchant_person(level, ch, tar_ch, 0);
	  break;
    case SPELL_TYPE_SCROLL:
	  if(!tar_ch) return;
      spell_enchant_person(level, ch, tar_ch, 0);
    default:
	  log("Serious screw-up in enchant person!");
	  break;
  }
}

void cast_enchant_armor( byte level, struct char_data *ch, char *arg, int type,
  struct char_data *tar_ch, struct obj_data *tar_obj )
{
  switch (type) {
    case SPELL_TYPE_SPELL:
      spell_enchant_armor(level, ch, 0,tar_obj);
      break;

    case SPELL_TYPE_SCROLL:
      if(!tar_obj) return;
      spell_enchant_armor(level, ch, 0,tar_obj);
      break;
    default : 
      log("Serious screw-up in enchant armor!");
      break;
  }
}

void cast_pray_for_armor( byte level, struct char_data *ch, char *arg, int type,
  struct char_data *tar_ch, struct obj_data *tar_obj )
{
  switch (type) {
    case SPELL_TYPE_SPELL:
      spell_pray_for_armor(level, ch, 0,tar_obj);
      break;

    case SPELL_TYPE_SCROLL:
      if(!tar_obj) return;
      spell_pray_for_armor(level, ch, 0,tar_obj);
      break;
    default : 
      log("Serious screw-up in pray_for armor!");
      break;
  }
}

void cast_self_heal( byte level, struct char_data *ch, char *arg, int type,
  struct char_data *tar_ch, struct obj_data *tar_obj )
{
  switch (type) {
    case SPELL_TYPE_SPELL:
      act("$n self-heals $N.", FALSE, ch, 0, tar_ch, TO_NOTVICT);
      act("You self-heal $N.", FALSE, ch, 0, tar_ch, TO_CHAR);
      spell_self_heal(level, ch, tar_ch, 0);
      break;
    case SPELL_TYPE_POTION:
         spell_self_heal(level, ch, ch, 0);
         break;
    case SPELL_TYPE_STAFF:
         for (tar_ch = world[ch->in_room].people ;
              tar_ch ; tar_ch = tar_ch->next_in_room)
            if (tar_ch != ch)
              spell_self_heal(level,ch,tar_ch,0);
         break;
    default :
         log("Serious screw-up in heal!");
         break;
  }
}

void cast_restore_move( byte level, struct char_data *ch, char *arg, int type,
  struct char_data *tar_ch, struct obj_data *tar_obj )
{
  switch (type) {
    case SPELL_TYPE_SPELL:
      act("$n restores $N's move.", FALSE, ch, 0, tar_ch, TO_NOTVICT);
      act("You restores your move.", FALSE, ch, 0, tar_ch, TO_CHAR);
      spell_restore_move(level, ch, tar_ch, 0);
      break;
    case SPELL_TYPE_POTION:
         spell_restore_move(level, ch, ch, 0);
         break;
    case SPELL_TYPE_STAFF:
         for (tar_ch = world[ch->in_room].people ;
              tar_ch ; tar_ch = tar_ch->next_in_room)
            if (tar_ch != ch)
              spell_restore_move(level,ch,tar_ch,0);
         break;
    default :
         log("Serious screw-up in restore move!");
         break;
  }
}

void cast_heal( byte level, struct char_data *ch, char *arg, int type,
  struct char_data *tar_ch, struct obj_data *tar_obj )
{
  switch (type) {
    case SPELL_TYPE_SPELL:
      act("$n heals $N.", FALSE, ch, 0, tar_ch, TO_NOTVICT);
      act("You heal $N.", FALSE, ch, 0, tar_ch, TO_CHAR);
      spell_heal(level, ch, tar_ch, 0);
      break;
    case SPELL_TYPE_POTION:
         spell_heal(level, ch, ch, 0);
         break;
    case SPELL_TYPE_STAFF:
         for (tar_ch = world[ch->in_room].people ; 
              tar_ch ; tar_ch = tar_ch->next_in_room)
            if (tar_ch != ch) 
              spell_heal(level,ch,tar_ch,0);
         break;
    default : 
         log("Serious screw-up in heal!");
         break;
  }
}
void cast_full_heal( byte level, struct char_data *ch, char *arg, int type,
  struct char_data *tar_ch, struct obj_data *tar_obj )
{
  switch (type) {
    case SPELL_TYPE_SPELL:
      act("$n full heals $N.", FALSE, ch, 0, tar_ch, TO_NOTVICT);
      act("You full heal $N.", FALSE, ch, 0, tar_ch, TO_CHAR);
      spell_full_heal(level, ch, tar_ch, 0);
      break;
    case SPELL_TYPE_POTION:
         spell_full_heal(level, ch, ch, 0);
         break;
    case SPELL_TYPE_STAFF:
         for (tar_ch = world[ch->in_room].people ; 
              tar_ch ; tar_ch = tar_ch->next_in_room)
            if (tar_ch != ch) 
              spell_full_heal(level,ch,tar_ch,0);
         break;
    default : 
         log("Serious screw-up in heal!");
         break;
  }
}

void cast_entire_heal( byte level, struct char_data *ch, char *arg, int type,
  struct char_data *tar_ch, struct obj_data *tar_obj )
{
  switch (type) {
    case SPELL_TYPE_SPELL:
      act("$n entire heals $N.", FALSE, ch, 0, tar_ch, TO_NOTVICT);
      act("You entire heal $N.", FALSE, ch, 0, tar_ch, TO_CHAR);
      spell_entire_heal(level, ch, tar_ch, 0);
      break;
    case SPELL_TYPE_POTION:
         spell_entire_heal(level, ch, ch, 0);
         break;
    case SPELL_TYPE_STAFF:
         for (tar_ch = world[ch->in_room].people ; 
              tar_ch ; tar_ch = tar_ch->next_in_room)
            if (tar_ch != ch) 
              spell_entire_heal(level,ch,tar_ch,0);
         break;
    default : 
         log("Serious screw-up in heal!");
         break;
  }
}

void cast_invisibility( byte level, struct char_data *ch, char *arg, int type,
  struct char_data *tar_ch, struct obj_data *tar_obj )
{
  switch (type) {
    case SPELL_TYPE_SPELL:
      if (tar_obj) {
        if ( IS_SET(tar_obj->obj_flags.extra_flags, ITEM_INVISIBLE) )
          send_to_char("Nothing new seems to happen.\n\r", ch);
        else
          spell_invisibility(level, ch, 0, tar_obj);
      } else { /* tar_ch */
        if ( IS_AFFECTED(tar_ch, AFF_INVISIBLE) )
          send_to_char("Nothing new seems to happen.\n\r", ch);
        else
          spell_invisibility(level, ch, tar_ch, 0);
      }
      break;
    case SPELL_TYPE_POTION:
         if (!IS_AFFECTED(ch, AFF_INVISIBLE) )
            spell_invisibility(level, ch, ch, 0);
         break;
    case SPELL_TYPE_SCROLL:
      if (tar_obj) {
        if (!(IS_SET(tar_obj->obj_flags.extra_flags, ITEM_INVISIBLE)) )
          spell_invisibility(level, ch, 0, tar_obj);
      } else { /* tar_ch */
            if (!tar_ch) tar_ch = ch;

        if (!( IS_AFFECTED(tar_ch, AFF_INVISIBLE)) )
          spell_invisibility(level, ch, tar_ch, 0);
      }
      break;
    case SPELL_TYPE_WAND:
      if (tar_obj) {
        if (!(IS_SET(tar_obj->obj_flags.extra_flags, ITEM_INVISIBLE)) )
          spell_invisibility(level, ch, 0, tar_obj);
      } else { /* tar_ch */
        if (!( IS_AFFECTED(tar_ch, AFF_INVISIBLE)) )
          spell_invisibility(level, ch, tar_ch, 0);
      }
      break;
    case SPELL_TYPE_STAFF:
         for (tar_ch = world[ch->in_room].people ; 
              tar_ch ; tar_ch = tar_ch->next_in_room)
            if (tar_ch != ch)
               if (!( IS_AFFECTED(tar_ch, AFF_INVISIBLE)) )
                  spell_invisibility(level,ch,tar_ch,0);
         break;
    default : 
         log("Serious screw-up in invisibility!");
         break;
  }
}




void cast_locate_object( byte level, struct char_data *ch, char *arg, int type,
  struct char_data *tar_ch, struct obj_data *tar_obj )
{
  switch (type) {
    case SPELL_TYPE_SPELL:
      spell_locate_object(level, ch, 0, tar_obj);
      break;
      default : 
         log("Serious screw-up in locate object!");
         break;
  }
}


void cast_poison( byte level, struct char_data *ch, char *arg, int type,
  struct char_data *tar_ch, struct obj_data *tar_obj )
{
  switch (type) {
    case SPELL_TYPE_SPELL:
      spell_poison(level, ch, tar_ch, tar_obj);
      break;
    case SPELL_TYPE_POTION:
      spell_poison(level, ch, ch, 0);
      break;
    case SPELL_TYPE_STAFF:
         for (tar_ch = world[ch->in_room].people ; 
              tar_ch ; tar_ch = tar_ch->next_in_room)
            if (tar_ch != ch) 
                  spell_poison(level,ch,tar_ch,0);
         break;
    default : 
         log("Serious screw-up in poison!");
         break;
  }
}


void cast_protection_from_evil( byte level, struct char_data *ch, char *arg, int type,
  struct char_data *tar_ch, struct obj_data *tar_obj )
{
  switch (type) {
    case SPELL_TYPE_SPELL:
      spell_protection_from_evil(level, ch, tar_ch, 0);
      break;
    case SPELL_TYPE_POTION:
         spell_protection_from_evil(level, ch, ch, 0);
         break;
    case SPELL_TYPE_SCROLL:
         if(tar_obj) return;
         if(!tar_ch) tar_ch = ch;
      spell_protection_from_evil(level, ch, tar_ch, 0);
      break;
    case SPELL_TYPE_STAFF:
         for (tar_ch = world[ch->in_room].people ; 
              tar_ch ; tar_ch = tar_ch->next_in_room)
            if (tar_ch != ch) 
                  spell_protection_from_evil(level,ch,tar_ch,0);
         break;
    default : 
         log("Serious screw-up in protection from evil!");
         break;
  }
}

void cast_haste( byte level, struct char_data *ch, char *arg, int type,
  struct char_data *tar_ch, struct obj_data *tar_obj )
{
  switch (type) {
    case SPELL_TYPE_SPELL:
      spell_haste(level, ch, tar_ch, 0);
      break;
    case SPELL_TYPE_POTION:
         spell_haste(level, ch, ch, 0);
         break;
    case SPELL_TYPE_SCROLL:
         if(tar_obj) return;
         if(!tar_ch) tar_ch = ch;
      spell_haste(level, ch, tar_ch, 0);
      break;
    case SPELL_TYPE_STAFF:
         for (tar_ch = world[ch->in_room].people ; 
              tar_ch ; tar_ch = tar_ch->next_in_room)
            if (tar_ch != ch) 
                  spell_haste(level,ch,tar_ch,0);
         break;
    default : 
         log("Serious screw-up in haste!");
         break;
  }
}

void cast_improved_haste( byte level, struct char_data *ch, char *arg, int type,
  struct char_data *tar_ch, struct obj_data *tar_obj )
{
  switch (type) {
    case SPELL_TYPE_SPELL:
      spell_improved_haste(level, ch, tar_ch, 0);
      break;
    case SPELL_TYPE_POTION:
         spell_improved_haste(level, ch, ch, 0);
         break;
    case SPELL_TYPE_SCROLL:
         if(tar_obj) return;
         if(!tar_ch) tar_ch = ch;
      spell_improved_haste(level, ch, tar_ch, 0);
      break;
    case SPELL_TYPE_STAFF:
         for (tar_ch = world[ch->in_room].people ; 
              tar_ch ; tar_ch = tar_ch->next_in_room)
            if (tar_ch != ch) 
                  spell_improved_haste(level,ch,tar_ch,0);
         break;
    default : 
         log("Serious screw-up in improved haste!");
         break;
  }
}


void cast_remove_curse( byte level, struct char_data *ch, char *arg, int type,
  struct char_data *tar_ch, struct obj_data *tar_obj )
{
  switch (type) {
    case SPELL_TYPE_SPELL:
      spell_remove_curse(level, ch, tar_ch, tar_obj);
      break;
    case SPELL_TYPE_POTION:
         spell_remove_curse(level, ch, ch, 0);
         break;
    case SPELL_TYPE_SCROLL:
         if(tar_obj) {
        spell_remove_curse(level, ch, 0, tar_obj);
         return;
      }
         if(!tar_ch) tar_ch = ch;
      spell_remove_curse(level, ch, tar_ch, 0);
      break;
    case SPELL_TYPE_STAFF:
         for (tar_ch = world[ch->in_room].people ; 
              tar_ch ; tar_ch = tar_ch->next_in_room)
            if (tar_ch != ch) 
                  spell_remove_curse(level,ch,tar_ch,0);
         break;
    default : 
         log("Serious screw-up in remove curse!");
         break;
  }
}



void cast_remove_poison( byte level, struct char_data *ch, char *arg, int type,
  struct char_data *tar_ch, struct obj_data *tar_obj )
{
  switch (type) {
    case SPELL_TYPE_SPELL:
      spell_remove_poison(level, ch, tar_ch, tar_obj);
      break;
    case SPELL_TYPE_POTION:
         spell_remove_poison(level, ch, ch, 0);
         break;
    case SPELL_TYPE_STAFF:
         for (tar_ch = world[ch->in_room].people ; 
              tar_ch ; tar_ch = tar_ch->next_in_room)
            if (tar_ch != ch) 
                  spell_remove_poison(level,ch,tar_ch,0);
         break;
    default : 
         log("Serious screw-up in remove poison!");
         break;
  }
}


void cast_infravision( byte level, struct char_data *ch, char *arg, int type,
  struct char_data *tar_ch, struct obj_data *tar_obj )
{
  switch (type) {
    case SPELL_TYPE_SPELL:
      spell_infravision(level, ch, tar_ch, 0);
      break;
    case SPELL_TYPE_POTION:
         spell_infravision(level, ch, ch, 0);
         break;
    case SPELL_TYPE_SCROLL:
         if(tar_obj)
         return;
         if(!tar_ch) tar_ch = ch;
      spell_infravision(level, ch, tar_ch, 0);
      break;
    case SPELL_TYPE_STAFF:
         for (tar_ch = world[ch->in_room].people ; 
              tar_ch ; tar_ch = tar_ch->next_in_room)
            if (tar_ch != ch) 
                  spell_infravision(level,ch,tar_ch,0);
         break;
    default : 
         log("Serious screw-up in infravision!");
         break;
  }
}

void cast_mirror_image( byte level, struct char_data *ch, char *arg, int type,
  struct char_data *tar_ch, struct obj_data *tar_obj )
{
  switch (type) {
    case SPELL_TYPE_SPELL:
      spell_mirror_image(level, ch, tar_ch, 0);
      break;
    case SPELL_TYPE_POTION:
         spell_mirror_image(level, ch, ch, 0);
         break;
    case SPELL_TYPE_SCROLL:
         if(tar_obj)
         return;
         if(!tar_ch) tar_ch = ch;
      spell_mirror_image(level, ch, tar_ch, 0);
      break;
    case SPELL_TYPE_STAFF:
         for (tar_ch = world[ch->in_room].people ; 
              tar_ch ; tar_ch = tar_ch->next_in_room)
            if (tar_ch != ch) 
                  spell_mirror_image(level,ch,tar_ch,0);
         break;
    default : 
         log("Serious screw-up in mirror_image!");
         break;
  }
}
/* death */
/* by process */
void cast_death( byte level,struct char_data *ch,char *arg,int type,
	struct char_data *tar_ch,struct obj_data *tar_obj)
{
	switch(type)
	{
	case SPELL_TYPE_SPELL:
		spell_death(level,ch,tar_ch,0);
	default:
		break;
	}
}
void cast_sanctuary( byte level, struct char_data *ch, char *arg, int type,
  struct char_data *tar_ch, struct obj_data *tar_obj )
{
  switch (type) {
    case SPELL_TYPE_SPELL:
      spell_sanctuary(level, ch, tar_ch, 0);
      break;
    case SPELL_TYPE_POTION:
         spell_sanctuary(level, ch, ch, 0);
         break;
    case SPELL_TYPE_SCROLL:
         if(tar_obj)
         return;
         if(!tar_ch) tar_ch = ch;
      spell_sanctuary(level, ch, tar_ch, 0);
      break;
    case SPELL_TYPE_STAFF:
         for (tar_ch = world[ch->in_room].people ; 
              tar_ch ; tar_ch = tar_ch->next_in_room)
            if (tar_ch != ch) 
                  spell_sanctuary(level,ch,tar_ch,0);
         break;
    default : 
         log("Serious screw-up in sanctuary!");
         break;
  }
}

void cast_love( byte level, struct char_data *ch, char *arg, int type,
  struct char_data *tar_ch, struct obj_data *tar_obj )
{
  switch (type) {
    case SPELL_TYPE_SPELL:
      spell_love(level, ch, tar_ch, 0);
      break;
    case SPELL_TYPE_POTION:
         spell_love(level, ch, ch, 0);
         break;
    case SPELL_TYPE_SCROLL:
         if(tar_obj)
         return;
         if(!tar_ch) tar_ch = ch;
      spell_love(level, ch, tar_ch, 0);
      break;
    case SPELL_TYPE_STAFF:
         for (tar_ch = world[ch->in_room].people ; 
              tar_ch ; tar_ch = tar_ch->next_in_room)
            if (tar_ch != ch) 
                  spell_love(level,ch,tar_ch,0);
         break;
    default : 
         log("Serious screw-up in sanctuary!");
         break;
  }
}

void cast_reraise( byte level, struct char_data *ch, char *arg, int type,
  struct char_data *tar_ch, struct obj_data *tar_obj )
{
  char tmpbuf[80];
  sprintf(tmpbuf,"%s is tried cast reraise.\n\r",GET_NAME(ch));
  log(tmpbuf);
  switch (type) 
  {
  case SPELL_TYPE_SPELL:
	if (IS_AFFECTED(ch, AFF_DEATH)) {
		send_to_char("No way! You must die!\n\r", ch);
		return;
	}
	if(GET_MANA(ch)<GET_PLAYER_MAX_HIT(ch)/5)
	{
		send_to_char("You does not have enough mana.\n\r",ch);
		return;
	}
	GET_MANA(ch)-=GET_PLAYER_MAX_HIT(ch)/5;
	spell_reraise(level,ch,tar_ch,0);
	break;
  case SPELL_TYPE_POTION:
	spell_reraise(level,ch,ch,0);
	break;
  case SPELL_TYPE_SCROLL:
	if(tar_obj)
		return;
    if(!tar_ch) tar_ch = ch;
	spell_reraise(level,ch,tar_ch,0);
	break;
  case SPELL_TYPE_STAFF:
	for( tar_ch = world[ch->in_room].people; tar_ch ; 
			tar_ch = tar_ch->next_in_room)
   		if( tar_ch != ch )
			spell_reraise(level,ch,tar_ch,0);
    break;
  default:
	log("Serious screw-up in reraise!");
	break;
  }
}

void cast_sleep( byte level, struct char_data *ch, char *arg, int type,
  struct char_data *tar_ch, struct obj_data *tar_obj )
{
  int flag;

  switch (type) {
    case SPELL_TYPE_SPELL:
      spell_sleep(level, ch, tar_ch, 0);
      break;
    case SPELL_TYPE_POTION:
      spell_sleep(level, ch, ch, 0);
      break;
    case SPELL_TYPE_SCROLL:
         if(tar_obj) return;
         if (!tar_ch) tar_ch = ch;
         spell_sleep(level, ch, tar_ch, 0);
         break;
    case SPELL_TYPE_WAND:
         if(tar_obj) return;
         spell_sleep(level, ch, tar_ch, 0);
         break;
    case SPELL_TYPE_STAFF:
         flag=0;
         for (tar_ch = world[ch->in_room].people ; 
              tar_ch ; tar_ch = tar_ch->next_in_room)
            if(tar_ch != ch){
               spell_sleep(level,ch,tar_ch,0);
               if((!IS_NPC(tar_ch))&&(GET_LEVEL(tar_ch) < IMO)){
                  flag=1;
                  break;
               }
            }
         if(flag==1)
           spell_sleep(IMO+3,ch,ch,0);
         break;
    default : 
         log("Serious screw-up in sleep!");
         break;
  }
}


void cast_strength( byte level, struct char_data *ch, char *arg, int type,
  struct char_data *tar_ch, struct obj_data *tar_obj )
{
  switch (type) {
    case SPELL_TYPE_SPELL:
      spell_strength(level, ch, tar_ch, 0);
      break;
    case SPELL_TYPE_POTION:
      spell_strength(level, ch, ch, 0);
      break;
    case SPELL_TYPE_SCROLL:
         if(tar_obj) return;
         if (!tar_ch) tar_ch = ch;
         spell_strength(level, ch, tar_ch, 0);
         break;
    case SPELL_TYPE_STAFF:
         for (tar_ch = world[ch->in_room].people ; 
              tar_ch ; tar_ch = tar_ch->next_in_room)
            if (tar_ch != ch) 
                  spell_strength(level,ch,tar_ch,0);
         break;
    default : 
         log("Serious screw-up in strength!");
         break;
  }
}


void cast_phase( byte level, struct char_data *ch, char *arg, int type,
  struct char_data *tar_ch, struct obj_data *tar_obj )
{
  int direction, was_in;

  if (type != SPELL_TYPE_SPELL) {
    log("Attempt to phase by non-cast-spell.");
    return;
  }
  for(; *arg && (*arg == ' '); arg++);
  switch(*arg){
    case 'e':
    case 'E': direction=1; break;
    case 'w':
    case 'W': direction=3; break;
    case 'n':
    case 'N': direction=0; break;
    case 's':
    case 'S': direction=2; break;
	case 'd':
	case 'D': direction=5; break;
	case 'u':
	case 'U': direction=4; break;
    /*
    case NULL: send_to_char("Phase to where?\n\r",ch); return;
    */
    case NUL:
    case ' ': send_to_char("Phase to where?\n\r",ch); return;
    /* Compile error. by Moon */
    default : send_to_char("ashgjklasdghiaudsgthl\n\r",ch);return;
  }
  if (!world[ch->in_room].dir_option[direction]) {
    send_to_char("You cannot phase there...\n\r", ch);
    return;
  }
  if(IS_SET(world[ch->in_room].dir_option[direction]->exit_info,EX_NOPHASE)) {
    send_to_char("You feel it's too solid.\n\r", ch);
    return;
  }

  if (special(ch, direction + 1, ""))
	return;

  was_in=ch->in_room;
  act("$n phases out silently.",FALSE,ch,0,0,TO_ROOM);
  char_from_room(ch);
  char_to_room(ch, world[was_in].dir_option[direction]->to_room);
  act("$n phases in silently.",FALSE,ch,0,0,TO_ROOM);
  do_look(ch,"",0);

  spell_phase(level, ch, ch, 0);
}

void cast_ventriloquate( byte level, struct char_data *ch, char *arg, int type,
  struct char_data *tar_ch, struct obj_data *tar_obj )
{
  struct char_data *tmp_ch;
  char buf1[MAX_STRING_LENGTH];
  char buf2[MAX_STRING_LENGTH];
  char buf3[MAX_STRING_LENGTH];

  if (type != SPELL_TYPE_SPELL) {
    log("Attempt to ventriloquate by non-cast-spell.");
    return;
  }
  for(; *arg && (*arg == ' '); arg++);
  if (tar_obj) {
    sprintf(buf1, "The %s says '%s'\n\r", fname(tar_obj->name), arg);
    sprintf(buf2, "Someone makes it sound like the %s says '%s'.\n\r",
      fname(tar_obj->name), arg);
  }  else {
    sprintf(buf1, "%s says '%s'\n\r", GET_NAME(tar_ch), arg);
    sprintf(buf2, "Someone makes it sound like %s says '%s'\n\r",
      GET_NAME(tar_ch), arg);
  }

  sprintf(buf3, "Someone says, '%s'\n\r", arg);

  for (tmp_ch = world[ch->in_room].people; tmp_ch;
    tmp_ch = tmp_ch->next_in_room) {

    if ((tmp_ch != ch) && (tmp_ch != tar_ch)) {
      if ( saves_spell(tmp_ch, SAVING_SPELL) )
        send_to_char(buf2, tmp_ch);
      else
        send_to_char(buf1, tmp_ch);
    } else {
      if (tmp_ch == tar_ch)
        send_to_char(buf3, tar_ch);
    }
  }

  spell_ventriloquate(level, ch, ch, 0);
}

void cast_word_of_recall( byte level, struct char_data *ch, char *arg, int type,
  struct char_data *tar_ch, struct obj_data *tar_obj )
{
  switch (type) {
    case SPELL_TYPE_SPELL:
      spell_word_of_recall(level, ch, ch, 0);
      break;
    case SPELL_TYPE_POTION:
      spell_word_of_recall(level, ch, ch, 0);
      break;
    case SPELL_TYPE_SCROLL:
    case SPELL_TYPE_WAND:
         if(tar_obj) return;
         if (!tar_ch) tar_ch = ch;
         spell_word_of_recall(level, ch, tar_ch, 0);
         break;
    default : 
         log("Serious screw-up in word of recall!");
         break;
  }
}

void cast_summon( byte level, struct char_data *ch, char *arg, int type,
  struct char_data *tar_ch, struct obj_data *tar_obj )
{
  switch (type) {
    case SPELL_TYPE_SPELL:
      spell_summon(level, ch, tar_ch, 0);
      break;
      default : 
         log("Serious screw-up in summon!");
         break;
  }
}

void cast_far_look( byte level, struct char_data *ch, char *arg, int type,
  struct char_data *tar_ch, struct obj_data *tar_obj )
{
  switch (type) {
    case SPELL_TYPE_SPELL:
      spell_far_look(level, ch, tar_ch, 0);
      break;
      default : 
         log("Serious screw-up in far look!");
         break;
  }
}

void cast_relocate( byte level, struct char_data *ch, char *arg, int type,
  struct char_data *tar_ch, struct obj_data *tar_obj )
{
  switch (type) {
    case SPELL_TYPE_SPELL:
      spell_relocate(level, ch, tar_ch, 0);
      break;
      default : 
         log("Serious screw-up in relocate!");
         break;
  }
}

void cast_charm_person( byte level, struct char_data *ch, char *arg, int type,
  struct char_data *tar_ch, struct obj_data *tar_obj )
{

  if(!IS_MOB(tar_ch)&&GET_LEVEL(ch)<(IMO+3)){
    send_to_char("You cannot charm player!\n\r", ch);
    return;
  }
  switch (type) {
    case SPELL_TYPE_SPELL:
      spell_charm_person(level, ch, tar_ch, 0);
      break;
      case SPELL_TYPE_SCROLL:
		 /*	
         if(!tar_ch) return;
         spell_charm_person(level, ch, tar_ch, 0); */
         break;
      case SPELL_TYPE_STAFF:
/*
         for (tar_ch = world[ch->in_room].people ; 
              tar_ch ; tar_ch = tar_ch->next_in_room)
            if (tar_ch != ch) 
                  spell_charm_person(level,ch,tar_ch,0); */
         break;
      default : 
         log("Serious screw-up in charm person!");
         break;
  }
}



void cast_sense_life( byte level, struct char_data *ch, char *arg, int type,
  struct char_data *tar_ch, struct obj_data *tar_obj )
{
  switch (type) {
    case SPELL_TYPE_SPELL:
      spell_sense_life(level, ch, ch, 0);
      break;
      case SPELL_TYPE_POTION:
         spell_sense_life(level, ch, ch, 0);
         break;
      case SPELL_TYPE_STAFF:
         for (tar_ch = world[ch->in_room].people ; 
              tar_ch ; tar_ch = tar_ch->next_in_room)
            if (tar_ch != ch) 
                  spell_sense_life(level,ch,tar_ch,0);
         break;
      default : 
         log("Serious screw-up in sense life!");
         break;
  }
}

void cast_identify( byte level, struct char_data *ch, char *arg, int type,
  struct char_data *tar_ch, struct obj_data *tar_obj )
{
  switch (type) {
    case SPELL_TYPE_SPELL :
    case SPELL_TYPE_SCROLL:
      spell_identify(level, ch, tar_ch, tar_obj);
      break;
    default : 
      log("Serious screw-up in identify!");
      break;
  }
}

void cast_fire_breath( byte level, struct char_data *ch, char *arg, int type,
  struct char_data *tar_ch, struct obj_data *tar_obj )
{
  switch (type) {
    case SPELL_TYPE_SPELL:
      spell_fire_breath(level, ch, tar_ch, 0);
      break;   /* It's a spell.. But people can'c cast it! */
      default : 
         log("Serious screw-up in firebreath!");
         break;
  }
}

void cast_frost_breath( byte level, struct char_data *ch, char *arg, int type,
  struct char_data *tar_ch, struct obj_data *tar_obj )
{
  switch (type) {
    case SPELL_TYPE_SPELL:
      spell_frost_breath(level, ch, tar_ch, 0);
      break;   /* It's a spell.. But people can'c cast it! */
      default : 
         log("Serious screw-up in frostbreath!");
         break;
  }
}

void cast_gas_breath( byte level, struct char_data *ch, char *arg, int type,
  struct char_data *tar_ch, struct obj_data *tar_obj )
{
  switch (type) {
    case SPELL_TYPE_SPELL:
    for (tar_ch = world[ch->in_room].people ; 
        tar_ch ; tar_ch = tar_ch->next_in_room)
      if ((tar_ch != ch)&&(GET_LEVEL(tar_ch) < IMO))
        spell_gas_breath(level,ch,tar_ch,0);
         break;
      /* THIS ONE HURTS!! */
      default : 
         log("Serious screw-up in gasbreath!");
         break;
  }
}

void cast_lightning_breath( byte level, struct char_data *ch, char *arg, int type,
  struct char_data *tar_ch, struct obj_data *tar_obj )
{
  switch (type) {
    case SPELL_TYPE_SPELL:
      spell_lightning_breath(level, ch, tar_ch, 0);
      break;   /* It's a spell.. But people can'c cast it! */
      default : 
         log("Serious screw-up in lightningbreath!");
         break;
  }
}

void cast_reanimate( byte level, struct char_data *ch, char *arg, int type,
  struct char_data *tar_ch, struct obj_data *tar_obj )
{
  spell_reanimate(level, ch, tar_ch, tar_obj);
}

