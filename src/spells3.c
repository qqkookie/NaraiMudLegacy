/* ************************************************************************
*  file: spells3.c , Implementation of magic.             Part of NARAIMUD*
*  Usage : All the non-offensive magic handling routines.                 *
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


/* Global data */

extern struct room_data *world;
extern struct char_data *character_list;
extern struct spell_info_type spell_info[MAX_SPL_LIST];
extern struct obj_data  *object_list;


/* Extern procedures */

void update_pos( struct char_data *victim );
void damage(struct char_data *ch, struct char_data *victim,
            int damage, int weapontype);
int number(int from, int to);


void say_spell( struct char_data *ch, int si );
bool saves_spell(struct char_data *ch, sh_int spell);
void add_follower(struct char_data *ch, struct char_data *victim);
// char *strdup(char *str);
void spell_spell_block(byte level,struct char_data *ch,struct char_data *tar_ch,
struct obj_data *tar_obj);
void spell_mana_transfer(byte level,struct char_data *ch,struct char_data *tar_ch,struct obj_data *tar_obj);
void spell_holy_shield(byte level,struct char_data *ch,struct char_data *tar_ch,struct obj_data *tar_obj);
void spell_kiss_of_process(byte level,struct char_data *ch,struct char_data *tar_ch,struct obj_data *tar_obj);
void spell_reflect_damage(byte level,struct char_data *ch,struct char_data *tar_ch,struct obj_data *tar_obj);
void spell_dumb(byte level,struct char_data *ch,struct char_data *tar_ch,
struct obj_data *tar_obj);
void spell_thunder_bolt(byte level,struct char_data *ch,
	struct char_data *tar_ch, struct obj_data *tar_obj);

/*
cast 'spell block' victim
by process
*/
void cast_spell_block( byte level,struct char_data *ch,char *arg,
int type,struct char_data *tar_ch,struct obj_data *tar_obj)
{
	switch(type)
	{
	case SPELL_TYPE_SPELL:
		if(number(1,10)>5)
			spell_spell_block(level,ch,tar_ch,0);
		break;
	default:
		break;
	}
}
void cast_dumb(byte level,struct char_data *ch,char *arg,
int type,struct char_data *tar_ch,struct obj_data *tar_obj)
{
	switch(type)
	{
	case SPELL_TYPE_SPELL:
		spell_dumb(level,ch,tar_ch,0);
		break;
	default:
		break;
	}
}
void cast_reflect_damage(byte level,struct char_data *ch,char *arg,
int type,struct char_data *tar_ch,struct obj_data *tar_obj)
{
	switch(type)
	{
	case SPELL_TYPE_SPELL:
		spell_reflect_damage(level,ch,tar_ch,0);
		break;
	default:
		break;
	}
}
void cast_holy_shield(byte level,struct char_data *ch,char *arg,
int type,struct char_data *tar_ch,struct obj_data *tar_obj)
{
	switch(type)
	{
	case SPELL_TYPE_SPELL:
		spell_holy_shield(level,ch,tar_ch,0);
		break;
	default:
		break;
	}
}
/*
cast 'mana transfer' victim
by process
*/
void cast_mana_transfer(byte level,struct char_data *ch,char *arg,
int type,struct char_data *tar_ch,struct obj_data *tar_obj)
{
	switch(type)
	{
	case SPELL_TYPE_SPELL:
		spell_mana_transfer(level,ch,tar_ch,0);
		break;
	default:
		break;
	}
}
/*
cast 'kiss of process' no_victim
by process
*/
void cast_kiss_of_process(byte level,struct char_data *ch,char *arg,
int type,struct char_data *tar_ch,struct obj_data *tar_obj)
{
  switch(type) {
  case SPELL_TYPE_SPELL:
    if(GET_MANA(ch)<30000) {
      send_to_char("You do not have enough mana\n\r",ch);
      return;
    }
    GET_HIT(ch)=1;
    GET_MANA(ch)=1;
    GET_MOVE(ch)=1;
  case SPELL_TYPE_WAND:
  case SPELL_TYPE_STAFF:
  case SPELL_TYPE_SCROLL:
    spell_kiss_of_process(level,ch,0,0);
    break;
  default:
    break;
  }
}

void cast_thunder_bolt(byte level,struct char_data *ch,char *arg,
		       int type,struct char_data *tar_ch,struct obj_data *tar_obj)
{
  switch(type) {
  case SPELL_TYPE_SPELL:
    spell_thunder_bolt(level,ch,tar_ch,0);
    break;
  default:
    break;
  }
}
