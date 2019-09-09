/* ************************************************************************
*  file: magic3.c , Implementation of spells.             Part of NARAIMUD*
*  Usage : The actual effect of magic.                                    *
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
void hit(struct char_data *ch, struct char_data *victim, int type);
void do_say(struct char_data *ch, char *str, int cmd);
void do_shout(struct char_data *ch, char *str, int cmd);
int MAX(int a, int b);

/*
cast 'spell block' victim
while affected hours,the victim is blocked by any spell.
by process
*/
void spell_spell_block(byte level,struct char_data *ch,
	struct char_data *victim,struct obj_data *obj)
{
	struct affected_type af;

	if (number(1, 10) < 4) {
		send_to_char("You failed to cast spell block!!!",ch);
		if(victim&&victim->specials.fighting!=ch
			&&victim->in_room==ch->in_room)
		{
			if(!victim->specials.fighting)
			{
				if(victim!=ch)
				{
					hit(victim,ch,TYPE_UNDEFINED);
				}
			}
			return;
		}
	}
    if (!affected_by_spell(victim,SPELL_SPELL_BLOCK) )
    {
    INCREASE_SKILLED2(ch, victim, SPELL_SPELL_BLOCK);
    act("$n is surrounded by a Magical cloud.",TRUE,victim,0,0,TO_ROOM);
    act("You feel safe. ",TRUE,victim,0,0,TO_CHAR);
    af.type = SPELL_SPELL_BLOCK;
    af.duration = 4;
    af.modifier = 0;
    af.location = APPLY_NONE;
    af.bitvector = AFF_SPELL_BLOCK;
    affect_to_char(victim,&af);
    }
	send_to_char("ok\n\r",ch);
}
void spell_reflect_damage(byte level,struct char_data *ch,
	struct char_data *victim,struct obj_data *obj)
{
	struct affected_type af;
	if(!affected_by_spell(victim,SPELL_REFLECT_DAMAGE))
	{
		INCREASE_SKILLED2(ch, victim, SPELL_REFLECT_DAMAGE);
		act("$n gets ready to reflect damages!",TRUE,ch,0,0,TO_ROOM);
		af.type = SPELL_REFLECT_DAMAGE;
		af.duration = 5;
		af.modifier = IMO - (level >> 1);
		af.location = APPLY_AC;
		af.bitvector = AFF_REFLECT_DAMAGE;
		affect_to_char(victim,&af);
	}
	send_to_char("You get ready to reflect damages!",ch);
}
void spell_dumb(byte level,struct char_data *ch,
	struct char_data *victim,struct obj_data *obj)
{
	struct affected_type af;
	if(!affected_by_spell(victim,SPELL_DUMB))
	{
		INCREASE_SKILLED2(ch, victim, SPELL_DUMB);
		act("$n gets DUMB!!",TRUE,victim,0,0,TO_ROOM);
		af.type = SPELL_DUMB;
		af.duration = 2;
		af.modifier = 0;
		af.location = 0;
		af.bitvector = AFF_DUMB;
		affect_to_char(victim,&af);
	}
}

void spell_holy_shield(byte level,struct char_data *ch,
	struct char_data *victim,struct obj_data *obj)
{
	struct affected_type af;

	if (IS_NPC(victim))
		return;
	if (!IS_GOOD(ch)) {
		do_say(ch,"I forgot I am a devil!!!", 0);
		return;
	}
	if ( !affected_by_spell(victim,SPELL_HOLY_SHIELD) 
		&& !affected_by_spell(victim,SPELL_PROTECT_FROM_EVIL) )
	{
	INCREASE_SKILLED2(ch, victim, SPELL_HOLY_SHIELD);
	act("$n is surrounded by a HOLY SHIELD!!!",TRUE,victim,0,0,TO_ROOM);
	act("You feel holy.",TRUE,victim,0,0,TO_CHAR);
	af.type = SPELL_HOLY_SHIELD;
	af.duration = level/10;
	af.modifier = 0;
	af.location = APPLY_NONE;
	af.bitvector = AFF_HOLY_SHIELD;
	affect_to_char(victim,&af);
	}
	send_to_char("OK!\n\r",ch);
}
/*
cast 'mana transfer' victim
*/
void spell_mana_transfer(byte level,struct char_data *ch,
	struct char_data *victim,struct obj_data *obj)
{
	assert(ch);
	INCREASE_SKILLED2(ch, victim, SPELL_MANA_TRANSFER);
	act("$n's mana is transferring to $N!!!",TRUE,ch,0,victim,TO_ROOM);
	act("You feel better!!!",TRUE,victim,0,0,TO_CHAR);
	GET_MANA(victim)+=GET_MANA(ch);
	GET_MANA(ch)=0;
}
/*
cast 'kiss of process'
*/
void spell_kiss_of_process(byte level,struct char_data *ch,
	struct char_data *victim,struct obj_data *obj)
{
	struct char_data *tmp_victim,*temp;
	assert(ch);

	do_shout(ch,"HEAL THE WORLD!!!",0);
	for(tmp_victim=character_list;tmp_victim;tmp_victim=temp)
	{
		temp=tmp_victim->next;

		if(!IS_NPC(tmp_victim)){
			GET_HIT(tmp_victim)=tmp_victim->points.max_hit;
			send_to_char("Michael Jackson kisses you in your lips.\n\r",tmp_victim);
			do_say(tmp_victim,"Heal the world!!", 0);
		}
	}
}


void spell_thunder_bolt(byte level, struct char_data *ch,
	struct char_data *victim, struct obj_data *obj)
{
	extern struct weather_data weather_info;
	struct obj_data *tmp_obj = NULL;
	int percent;
	int tmp;
	int level_plus;
    int flag=0;

	assert(ch);

	if (!(OUTSIDE(ch) && weather_info.sky >= SKY_RAINING)) {
		act("Hmm...I don't wanna be a chicken...",
			TRUE, ch, 0, 0, TO_CHAR);
		return;
	}

	INCREASE_SKILLED2(ch, ch, SPELL_THUNDERBOLT);
	if(weather_info.sky==SKY_LIGHTNING)
	{
		flag=1;
	}
	level_plus = (GET_LEVEL(ch) - 30) / 2;
	percent = number(1, 100) + level_plus + 2*weather_info.sky;
	if (percent > 80) {
		CREATE(tmp_obj, struct obj_data, 1);
		clear_object(tmp_obj);

		SET_BIT(tmp_obj->obj_flags.extra_flags, ITEM_NORENT);

		tmp_obj->name = strdup("thunderbolt");
		tmp_obj->short_description = strdup("A dull broad thunderbolt");
		tmp_obj->description = strdup("A dull broad thunderbolt lies here.");

		tmp_obj->obj_flags.type_flag = ITEM_WEAPON;
		tmp_obj->obj_flags.wear_flags = ITEM_TAKE | ITEM_WIELD;
		tmp_obj->obj_flags.value[0] = 0;
		tmp_obj->obj_flags.value[1] = number(1+flag, 5) + number(1, level_plus);
		tmp_obj->obj_flags.value[2] = number(1+flag, 5) + number(1, level_plus);
		tmp_obj->obj_flags.value[3] = 7;
		tmp_obj->obj_flags.weight = 8;
		tmp_obj->obj_flags.cost = 0;
		tmp_obj->next = object_list;
		object_list = tmp_obj;
		obj_to_room(tmp_obj,ch->in_room);

		tmp_obj->item_number = -1;

		act("$p suddenly appears. You feel hot!",
			TRUE, ch, tmp_obj, 0, TO_ROOM);
		act("$p suddenly appears. You feel hot!",
			TRUE, ch, tmp_obj, 0, TO_CHAR);
	}
	else {
		tmp = GET_HIT(ch);
		tmp -= number(2000, 5000) - GET_LEVEL(ch);
		GET_HIT(ch) = MAX(10, tmp);
		GET_MANA(ch) -= number(300, 500);
		act("Ooops! You barely save your life from damn hot flame...",
			TRUE, ch, tmp_obj, 0, TO_CHAR);
		act("How fool! $n barely save $s life from hot flame...",
			TRUE, ch, tmp_obj, 0, TO_ROOM);
	}
}
