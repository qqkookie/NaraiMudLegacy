/* ************************************************************************
*  file: magic.c , Implementation of spells.              Part of DIKUMUD *
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

#include "guild_list.h"
#include "magic_weapon.h"

/* Extern structures */
extern struct room_data *world;
extern struct obj_data  *object_list;
extern struct char_data *character_list;
extern struct index_data *mob_index;

/* Extern procedures */

void damage(struct char_data *ch, struct char_data *victim,
            int damage, int weapontype);
bool saves_spell(struct char_data *ch, sh_int spell);
void weight_change_object(struct obj_data *obj, int weight);
int dice(int number, int size);
void do_look(struct char_data *ch, char *argument, int cmd);
int number(int from, int to);
int MIN(int a, int b);
int MAX(int a, int b);
void hit(struct char_data *ch, struct char_data *victim, int type);
void sprinttype(int type, char *name[], char *res);
void sprintbit(long vector, char *name[], char *res);
void spell_sanctuary(byte level, struct char_data *ch,
		     struct char_data *victim, struct obj_data *obj);


/* Offensive Spells */

/* modified by atre */
void spell_magic_missile(byte level, struct char_data *ch,
  struct char_data *victim, struct obj_data *obj)
{
	int dam;
	/*
	static int dam_each[] = 
		{ 0,
		  10,10,15,15,15, 20,20,20,20,20, 20,20,20,20,20, 20,20,20,20,20,
		  20,20,20,20,20, 20,20,20,20,20, 20,20,20,20,20, 20,20,20,20,20,
		  20,20,20,20,20, 20,20,20,20,20 };
	*/
	INCREASE_SKILLED2(ch, victim, SPELL_MAGIC_MISSILE);
	dam = dice(level*2, level + GET_SKILLED(ch, SPELL_MAGIC_MISSILE));
	/*
	dam = number(dam_each[level] >> 1, dam_each[level] << 1);
	*/

	if (saves_spell(victim, SAVING_SPELL))
		dam >>= 1;

	damage(ch, victim, dam, SPELL_MAGIC_MISSILE);
}



void spell_chill_touch(byte level, struct char_data *ch,
  struct char_data *victim, struct obj_data *obj)
{
	struct affected_type af;
	int dam;
	static int dam_each[] = 
		{ 0,
		  20,20,25,25,25, 30,30,30,30,30, 30,30,30,30,30, 30,30,30,30,30,
		  35,35,35,35,35, 35,35,35,35,35, 35,35,35,35,35, 35,35,35,35,35,
		  40,40,40,40,40, 40,40,40,40,40 };

	INCREASE_SKILLED2(ch, victim, SPELL_CHILL_TOUCH);
	dam = number(dam_each[(int)level] >> 1, dam_each[(int)level] << 1);
	if (!saves_spell(victim, SAVING_SPELL)) {
		af.type      = SPELL_CHILL_TOUCH;
		af.duration  = 6 + (GET_SKILLED(ch, SPELL_CHILL_TOUCH) >> 4);
		af.modifier  = -1;
		af.location  = APPLY_STR;
		af.bitvector = 0;
		affect_join(victim, &af, TRUE, FALSE);
	}
	else {
		dam >>= 1;
	}
	damage(ch, victim, dam, SPELL_CHILL_TOUCH);
}



void spell_burning_hands(byte level, struct char_data *ch,
  struct char_data *victim, struct obj_data *obj)
{
	int dam;
	static int dam_each[] = 
		{ 0,
		  30,30,35,35,35, 40,40,40,40,40, 40,40,40,40,40, 40,40,40,40,40,
		  45,45,45,45,45, 45,45,45,45,45, 45,45,45,45,45, 45,45,45,45,45,
		  50,50,50,50,50, 50,50,50,50,50 };

	INCREASE_SKILLED2(ch, victim, SPELL_BURNING_HANDS);
	dam = number(dam_each[(int)level] >> 1, dam_each[(int)level] << 1);

	if (saves_spell(victim, SAVING_SPELL))
		dam >>= 1;

	damage(ch, victim, dam, SPELL_BURNING_HANDS);
}



void spell_shocking_grasp(byte level, struct char_data *ch,
  struct char_data *victim, struct obj_data *obj)
{
	int dam;
	static int dam_each[] = 
		{ 0,
		  40,40,45,45,45, 50,50,50,50,50, 50,50,50,50,50, 50,50,50,50,50,
		  55,55,55,55,55, 55,55,55,55,55, 55,55,55,55,55, 55,55,55,55,55,
		  60,60,60,60,60, 60,60,60,60,60 };

	INCREASE_SKILLED2(ch, victim, SPELL_SHOCKING_GRASP);
	dam = number(dam_each[(int)level] >> 1, dam_each[(int)level] << 1);
	if (saves_spell(victim, SAVING_SPELL)) 
		dam >>= 1;
    
	if (!saves_spell(victim, SAVING_PARA))
		GET_POS(victim) = POSITION_STUNNED;

	damage(ch, victim, dam, SPELL_SHOCKING_GRASP);
}



void spell_lightning_bolt(byte level, struct char_data *ch,
  struct char_data *victim, struct obj_data *obj)
{
	int dam;
	/*
	static int dam_each[] = 
		{ 0,
		  100,100,100,100,100, 100,100,100,100,100, 120,120,120,120,120,
		  130,130,130,130,130, 140,140,140,140,140, 140,140,140,140,140,
		  150,150,150,150,150, 150,150,150,150,150, 150,150,150,150,150,
		  150,150,150,150,150 };
		  */
	if (!ch || !victim) return;

	INCREASE_SKILLED2(ch, victim, SPELL_LIGHTNING_BOLT);

	dam = dice(level, level + GET_SKILLED(ch, SPELL_LIGHTNING_BOLT));

	if (saves_spell(victim, SAVING_SPELL)) 
		dam >>= 1;
    
	if (!saves_spell(victim, SAVING_PARA))
		GET_POS(victim) = POSITION_STUNNED;

	damage(ch, victim, dam, SPELL_LIGHTNING_BOLT);
}



void spell_color_spray(byte level, struct char_data *ch,
  struct char_data *victim, struct obj_data *obj)
{
	int dam;
	static int dam_each[] = 
		{ 0,
		  100,100,100,100,100, 100,100,100,100,100, 120,120,120,120,120,
		  130,130,130,130,130, 140,140,140,140,140, 140,140,140,140,140,
		  150,150,150,150,150, 150,150,150,150,150, 150,150,150,150,150,
		  150,150,150,150,150 };

	INCREASE_SKILLED2(ch, victim, SPELL_COLOUR_SPRAY);
	dam = number(dam_each[(int)level], dam_each[(int)level] << 2); /* 1 */
	if (saves_spell(victim, SAVING_SPELL)) 
		dam >>= 1;

	damage(ch, victim, dam, SPELL_COLOUR_SPRAY);
}


/* Drain XP, MANA, HP - caster gains HP and MANA */
void spell_energy_drain(byte level, struct char_data *ch,
  struct char_data *victim, struct obj_data *obj)
{
	int dam, xp, mana;
	int alignment;

	void gain_exp(struct char_data *ch, int gain);

	if (!ch) return;
	if (!victim) return;

	INCREASE_SKILLED2(ch, victim, SPELL_ENERGY_DRAIN);

	if (!saves_spell(victim, SAVING_PARA)) {
		alignment = (GET_ALIGNMENT(ch) + 1000) / 10;
		GET_ALIGNMENT(ch) = MAX(-1000, GET_ALIGNMENT(ch) - alignment);
		if (GET_LEVEL(victim) <= 2) {
			damage(ch, victim, 100, SPELL_ENERGY_DRAIN); /* Kill the sucker */
		}
		else {
			xp = number(level, level << 1);
			xp *= -GET_ALIGNMENT(ch);

			gain_exp(victim, -xp);
			gain_exp(ch, xp >> 1);
			dam = dice(1, level);
			mana = GET_MANA(victim) >> 1;
			GET_MOVE(victim) >>= 1;
			GET_MANA(victim) = mana;
			GET_MANA(ch) += mana >> 1;
			GET_HIT(ch) += dam;
			GET_MANA(ch) = MIN(GET_MANA(ch),mana_limit(ch));
			GET_HIT(ch) = MIN(GET_HIT(ch),hit_limit(ch));
			send_to_char("Your life energy is drained!\n\r", victim);
			damage(ch, victim, dam, SPELL_ENERGY_DRAIN);
		}
	}
	else {
		damage(ch, victim, 0, SPELL_ENERGY_DRAIN); /* Miss */
	}
}

/* modified by atre */
void spell_fireball(byte level, struct char_data *ch,
  struct char_data *victim, struct obj_data *obj)
{
	int dam;
	extern struct weather_data weather_info;
	/*
	static int dam_each[] = 
		{ 0,
		  100,100,100,100,100, 100,100,100,100,100, 140,140,140,140,140,
		  150,150,150,150,150, 160,160,160,160,160, 160,160,160,160,160,
		  170,170,170,170,170, 170,170,170,170,170, 170,170,170,170,170,
		  180,180,180,180,180 };
		  */
	if (!ch || !victim) return;

	INCREASE_SKILLED2(ch,  victim, SPELL_FIREBALL);
	/*
	dam = number(dam_each[(int) level], dam_each[(int)level] << 1);
	*/
    dam = dice(level, (level+GET_SKILLED(ch, SPELL_FIREBALL)) >> 1);


	if (saves_spell(victim, SAVING_SPELL))
		dam >>= 1;

	if (OUTSIDE(ch) && (weather_info.sky == SKY_CLOUDLESS)) {
		send_to_char("Your fireball works excellent!!\n\r", ch);
		act("$n suffers from hot ball.", FALSE, victim, 0,0,TO_ROOM);
		dam *= GET_INT(ch);
	}
	damage(ch, victim, dam, SPELL_FIREBALL);
}

void spell_disintegrate(byte level, struct char_data *ch,
  struct char_data *victim, struct obj_data *obj)
{
	int dam;

	if (!ch || !victim) return;

	INCREASE_SKILLED2(ch, victim, SPELL_DISINTEGRATE);

	dam = level * GET_LEVEL(victim);
	dam = dice(dam, 2 + (GET_SKILLED(ch, SPELL_DISINTEGRATE) >> 5));
	if (saves_spell(victim, SAVING_SPELL))
		dam >>= 2;
	if (IS_AFFECTED(victim, AFF_SANCTUARY))
		dam <<= 1;
	damage(ch, victim, dam, SPELL_DISINTEGRATE);
}

/* modified by atre */
void spell_corn_of_ice(byte level, struct char_data *ch,
  struct char_data *victim, struct obj_data *obj)
{
	int dam;
	/*
	static int dam_each[] = 
	{ 0,
	  500,500,500,500,500, 500,500,500,500,500, 540,540,540,540,540,
	  550,550,550,550,550, 560,560,560,560,560, 560,560,560,560,560,
	  570,570,570,570,570, 570,570,570,570,570, 570,570,570,570,570,
	  580,580,580,580,580 }; */

	if (!ch || !victim) return;
	
	INCREASE_SKILLED2(ch, victim, SPELL_CORN_OF_ICE);
	/*
	dam = number(dam_each[level], dam_each[level] << 1);
	*/
	dam=dice(level*GET_INT(ch),(level+(GET_SKILLED(ch,SPELL_CORN_OF_ICE)>>3)));
	if (saves_spell(victim, SAVING_SPELL))
		dam >>= 1;
	damage(ch, victim, dam, SPELL_CORN_OF_ICE);
}

void spell_sunburst(byte level, struct char_data *ch,
  struct char_data *victim, struct obj_data *obj)
{
	int dam;
	void spell_blindness(byte level, struct char_data *ch,
		struct char_data *victim, struct obj_data *obj);
	char buf[MAX_STRING_LENGTH];

	static int dam_each[] = 
		{ 0,
		  400,400,400,400,400, 400,400,400,400,400, 440,440,440,440,440,
		  440,440,440,440,440, 460,460,460,460,460, 460,460,460,460,460,
		  470,470,470,470,470, 470,470,470,470,470, 470,470,470,470,470,
		  480,480,480,480,480 };

	if (!ch || !victim) return;

	INCREASE_SKILLED2(ch, victim, SPELL_SUNBURST);
	dam = number(dam_each[(int)level], dam_each[(int)level] << 1);
	if (saves_spell(victim, SAVING_SPELL))
		dam >>= 1;
	if (number(1,15) == 1)
		spell_blindness(level, ch, victim, 0);
	sprintf(buf, "DEBUG: sunburst: %d\n", dam);
	log(buf);

	damage(ch, victim, dam, SPELL_SUNBURST);
}

/* modified by atre */
void spell_energyflow(byte level, struct char_data *ch,
  struct char_data *victim, struct obj_data *obj)
{
	int dam;
	int exp;

	if (GET_EXP(ch) > 2000000) {
		INCREASE_SKILLED2(ch, victim, SPELL_ENERGY_FLOW);

		/*
		dam = dice(level, 30) + 200;
		*/
		dam = dice(level, (level>>1)+GET_SKILLED(ch, SPELL_ENERGY_FLOW));
		dam <<= (1+(GET_SKILLED(ch, SPELL_ENERGY_FLOW)>>5));

		send_to_char("Energyflow which is made of your Exp Quaaaaaaa!\n\r", ch);

		exp=level*level*level*(20-(GET_SKILLED(ch,SPELL_ENERGY_FLOW)>>3));
		GET_EXP(ch) -= exp;

		if (saves_spell(victim, SAVING_SPELL))
			dam >>= 1; 

		damage(ch, victim, dam, SPELL_ENERGY_FLOW);
	}
	else
		send_to_char("You have too little experience...\n\r",ch);
}

void spell_full_fire(byte level, struct char_data *ch,
  struct char_data *victim, struct obj_data *obj)
{
	int dam = 0;
	int i;

	assert(victim && ch);

    if(!IS_NPC(ch) && GET_LEVEL(ch) == IMO){
             send_to_char("You cannot use this spell.\n\r",ch);
             return;
    }

	INCREASE_SKILLED2(ch, victim, SPELL_FULL_FIRE);

	dam = dice(level, GET_INT(ch)+(GET_SKILLED(ch, SPELL_FULL_FIRE)>>4));

	if (saves_spell(victim, SAVING_SPELL)) {
		send_to_char("But, You couldn't concentrate enough\n\r",ch);
		GET_MANA(ch) -= dam;
	}
	else {
		i = GET_MANA(ch) / (500 - level - GET_SKILLED(ch, SPELL_FULL_FIRE));
		for(;i; i --){
			dam += dice(level, level);
		}
		/*
		limit = level * number(200, 300);
		i = GET_MANA(ch) > limit ? GET_MANA(ch) : 0;
		step = (300 - (level << 2)) << 1;
		for (dam = GET_MANA(ch); i > 0; i -= step)
			dam += dice(level, level);
		*/
		GET_MANA(ch) = 0;
		WAIT_STATE(ch, 3 * PULSE_VIOLENCE);
	}

	damage(ch, victim, dam, SPELL_FULL_FIRE);
}

void spell_throw(byte level, struct char_data *ch,
  struct char_data *victim, struct obj_data *obj)
{
	int dam = 0;

	assert(victim && ch);

	INCREASE_SKILLED2(ch, victim, SPELL_THROW);

	if (IS_NPC(ch) && !IS_NPC(victim)) {
		dam = MIN(level*level, level*level*3);
		GET_HIT(ch) -= dam;
	}
	else {
		if (saves_spell(victim, SAVING_SPELL)) {
			dam = number(level, (GET_HIT(ch) / 5));
			send_to_char("You Couldn't concentrate enough\n\r", ch);
			GET_HIT(ch) /= 2;
    	}
		else {
			dam = GET_HIT(ch) * ((GET_SKILLED(ch, SPELL_THROW) >> 4) + 3) / 5;
			GET_HIT(ch) /= 4;
		}
	}

	damage(ch, victim, dam, SPELL_THROW);
	send_to_char("Light Solar  guooooooooroorooorooorooooaaaaaaaaaaa!\n\r", ch);
}

/* modified by atre */
void spell_earthquake(byte level, struct char_data *ch,
  struct char_data *victim, struct obj_data *obj)
{
	int dam;
	struct char_data *tmp_victim, *temp;
	/*
	static int dam_each[] = 
		{ 0,
		  40,40,45,45,45, 100,100,100,100,100, 100,100,100,100,100,
		  100,100,100,100,100, 120,120,120,120,120, 120,120,120,120,120,
		  130,130,130,130,130, 130,130,130,130,130, 140,140,140,140,140,
		  150,150,150,150,150 };
		  */
	INCREASE_SKILLED2(ch, ch, SPELL_EARTHQUAKE);

	dam = dice(level, (level >> 2) + GET_SKILLED(ch, SPELL_EARTHQUAKE));
	/*
	dam = dam_each[level] + number(level, level << 1);
	*/
	send_to_char("The earth trembles beneath your feet!\n\r", ch);
	act("$n makes the earth tremble and shiver\n\rYou fall, and hit yourself!",
		FALSE, ch, 0, 0, TO_ROOM);
	for (tmp_victim = character_list; tmp_victim; tmp_victim = temp) {
		temp = tmp_victim->next;
		if ((ch->in_room == tmp_victim->in_room) && (ch != tmp_victim)){
			if(IS_NPC(ch) && IS_NPC(tmp_victim)) continue;

			damage(ch, tmp_victim, dam, SPELL_EARTHQUAKE);

			if(!IS_NPC(tmp_victim)){
				WAIT_STATE(tmp_victim,
					PULSE_VIOLENCE*(1+(GET_SKILLED(ch,SPELL_EARTHQUAKE)>>5)));
			}
		}
		else
			if (world[ch->in_room].zone == world[tmp_victim->in_room].zone)
				send_to_char("The earth trembles and shivers.\n\r", tmp_victim);
	}

	WAIT_STATE(ch, PULSE_VIOLENCE);
}

void spell_all_heal(byte level, struct char_data *ch,
  struct char_data *victim, struct obj_data *obj)
{
	struct char_data *tmp_victim, *temp;
	void spell_heal(byte level, struct char_data *ch,
		struct char_data *victim, struct obj_data *obj);

	INCREASE_SKILLED2(ch, ch, SPELL_ALL_HEAL);

	/* going to saint */
	GET_ALIGNMENT(ch) += (GET_LEVEL(ch)>>2);

	send_to_char("You feel some saintness!\n\r", ch);
	act("$n is glowing with saintly aura!", FALSE, ch, 0, 0, TO_ROOM);

	for (tmp_victim = character_list; tmp_victim; tmp_victim = temp) {
		temp = tmp_victim->next;
		if ((ch->in_room == tmp_victim->in_room) && (!IS_NPC(ch))) {
			spell_heal(level, ch, tmp_victim, obj);
		}
	}
}

void spell_sanctuary_cloud(byte level, struct char_data *ch,
  struct char_data *victim, struct obj_data *obj)
{
	struct char_data *tmp_victim, *temp;
	void spell_heal(byte level, struct char_data *ch,
		struct char_data *victim, struct obj_data *obj);

	INCREASE_SKILLED2(ch, ch, SPELL_SANCTUARY_CLOUD);

	send_to_char("You feel some saintness!\n\r", ch);
	act("$n makes a bloody cloud!", FALSE, ch, 0, 0, TO_ROOM);

	for (tmp_victim = character_list; tmp_victim; tmp_victim = temp) {
		temp = tmp_victim->next;
		if ((ch->in_room == tmp_victim->in_room) && (!IS_NPC(ch))) {
			spell_sanctuary(level, ch, tmp_victim, obj);
		}
	}
}

void spell_hand_of_god(byte level, struct char_data *ch,
  struct char_data *victim, struct obj_data *obj)
{
	int dam;
	struct char_data *tmp_victim, *temp;

	INCREASE_SKILLED2(ch, ch, SPELL_HAND_OF_GOD);

	dam = dice(level, level + GET_SKILLED(ch, SPELL_HAND_OF_GOD));
	send_to_char("Thousand hands are filling all your sight.\n\r", ch);
	act("$n summoned unnumerable hands.\n\r", FALSE, ch, 0, 0, TO_ROOM);
	act("Your face is slapped by hands. BLOOD ALL OVER!\n\r",
		FALSE, ch, 0, 0, TO_ROOM);
	for(tmp_victim = character_list; tmp_victim; tmp_victim = temp) {
		temp = tmp_victim->next;
		if ((ch->in_room == tmp_victim->in_room) && (ch != tmp_victim)) {
			damage(ch, tmp_victim, dam, SPELL_HAND_OF_GOD);
		}
		else
			if (world[ch->in_room].zone == world[tmp_victim->in_room].zone) {
				send_to_char("Dirty hands with long nail is ", tmp_victim);
				send_to_char("going and coming all over sky.\n\r", tmp_victim);
			}
	}
}

void spell_fire_storm(byte level, struct char_data *ch,
  struct char_data *victim, struct obj_data *obj)
{
	int dam;
	struct char_data *tmp_victim, *temp;

	INCREASE_SKILLED2(ch, ch, SPELL_FIRE_STORM);

	dam = dice(level, (level >> 1) + GET_SKILLED(ch, SPELL_FIRE_STORM));
	send_to_char("The fire storm is flowing in the air!\n\r", ch);
	act("$n makes the fire storm flowing in the air.\n\r",
		FALSE, ch, 0, 0, TO_ROOM);
	act("You can't see anything!\n\r", FALSE, ch, 0, 0, TO_ROOM);
	for (tmp_victim = character_list; tmp_victim; tmp_victim = temp) {
		temp = tmp_victim->next;
		if ((ch->in_room == tmp_victim->in_room) && (ch != tmp_victim) &&
			(IS_NPC(ch)^IS_NPC(tmp_victim))) {
			damage(ch, tmp_victim, dam, SPELL_FIRE_STORM);
		}
		else
			if (world[ch->in_room].zone == world[tmp_victim->in_room].zone)
				send_to_char("The fire storm is flowing in the atmosphere.\n\r",
					tmp_victim);
	}
}

void spell_trick(byte level, struct char_data *ch,
  struct char_data *victim, struct obj_data *obj)
{
	struct char_data *tmp_victim;
	int percent;
	int level_gap;

	INCREASE_SKILLED2(ch, victim, SPELL_TRICK);

	if (IS_NPC(victim)) {
		tmp_victim = world[victim->in_room].people;
		if (tmp_victim) {
			level_gap = level - GET_LEVEL(victim);
			if(level_gap > 6) level_gap = 6;

			percent = ch->skills[SPELL_TRICK].learned - 195
					+ level_gap * 10
					+ number(1, 155);

			while (!IS_NPC(tmp_victim) && tmp_victim)
				tmp_victim = tmp_victim->next_in_room;	

			if (percent > 0)
				hit(victim,tmp_victim,TYPE_UNDEFINED);
		}
    }
    else
		send_to_char("Only monster!\n\r", ch);
}

void spell_dispel_evil(byte level, struct char_data *ch,
  struct char_data *victim, struct obj_data *obj)
{
	int dam;

	INCREASE_SKILLED2(ch, victim, SPELL_DISPEL_EVIL);

	if (IS_EVIL(ch)) {
		act("God watches you.", FALSE, ch, 0, victim, TO_CHAR);
		return;
	}
	else {
		if (IS_GOOD(victim)) {
			act("God protects $N.", FALSE, ch, 0, victim, TO_CHAR);
			return;
		}
	}

	if (GET_LEVEL(victim) <= level) {
		dam = GET_ALIGNMENT(ch);
		dam = MAX(dam, -dam);
		damage(ch, victim, dam, SPELL_DISPEL_EVIL);
	}
	else {
		act("Hmm....nothing...", FALSE, ch, 0, victim, TO_CHAR);
	}
}


/* modified by atre */
void spell_call_lightning(byte level, struct char_data *ch,
  struct char_data *victim, struct obj_data *obj)
{
	int dam;

	extern struct weather_data weather_info;

	if (!ch || !victim) return;

	INCREASE_SKILLED2(ch, victim, SPELL_CALL_LIGHTNING);

	if (OUTSIDE(ch) && (weather_info.sky >= SKY_RAINING)) {
		dam = dice(level, level+GET_SKILLED(ch, SPELL_CALL_LIGHTNING));
		/*
		dam = dice(level, level);
		*/
		dam <<= (weather_info.sky - 1);
		damage(ch, victim, dam, SPELL_CALL_LIGHTNING);
	}
	else
		act("Hmm...is it rainy?", FALSE, ch, 0, 0, TO_CHAR);
}

/* modified by atre */
void spell_harm(byte level, struct char_data *ch,
  struct char_data *victim, struct obj_data *obj)
{
	int dam;

	INCREASE_SKILLED2(ch, victim, SPELL_HARM);

	dam = dice(level+GET_WIS(ch)*2,level+(GET_SKILLED(ch, SPELL_HARM)<<1));

	if(saves_spell(victim, SAVING_SPELL)){
			dam >>= 1;
	}

	if(GET_HIT(victim) < dam){
		dam = 0; /* Kill the suffering bastard */
	}

	damage(ch, victim, dam, SPELL_HARM);

/*
	dam = GET_HIT(victim);
	if (dam < 0)
		dam = 0;
	else {
		if (saves_spell(victim, SAVING_SPELL)) {
			dam >>= 1;
			damage(ch, victim, 0, SPELL_HARM);
		}
	}
	limit = level * GET_INT(ch);
	dam = MIN(limit, dam);
	GET_HIT(victim) -= dam;
*/
}

/* ***************************************************************************
 *                     Not cast-able spells                                  *
 * ************************************************************************* */

void spell_identify(byte level, struct char_data *ch,
  struct char_data *victim, struct obj_data *obj)
{
	int ac_applicable(struct obj_data *obj_object) ;
	char buf[MAX_OUTPUT_LENGTH], buf2[256], bufh[MAX_OUTPUT_LENGTH];
	int i;
	bool found;

	struct time_info_data age(struct char_data *ch);

	/* Spell Names */
	extern char *spells[];

	/* For Objects */
	extern char *item_types[];
	extern char *extra_bits[];
	extern char *apply_types[];
	extern char *affected_bits[];

	INCREASE_SKILLED2(ch, ch, SPELL_IDENTIFY);

	if (obj) {
		send_to_char_han("You feel informed:\n\r",
			"이런 정보를 알 수 있습니다:\n\r", ch);

		sprintf(buf, "Object '%s', Item type: ", obj->name);
		sprintf(bufh, "물건 '%s', 종류: ", obj->name);
		sprinttype(GET_ITEM_TYPE(obj),item_types,buf2);
		strcat(buf,buf2); strcat(buf,"\n\r");
		strcat(bufh,buf2); strcat(bufh,"\n\r");
		send_to_char_han(buf, bufh, ch);

		if (obj->obj_flags.bitvector) {
			send_to_char_han("Item will give you following abilities:  ",
				"이 물건은 다음과 같은 능력을 줍니다:  ",  ch);
			sprintbit(obj->obj_flags.bitvector,affected_bits,buf);
			strcat(buf,"\n\r");
			send_to_char(buf, ch);
		}

		send_to_char_han("Item is: ", "성질: ", ch);
		sprintbit(obj->obj_flags.extra_flags,extra_bits,buf);
		strcat(buf,"\n\r");
		send_to_char(buf,ch);

		sprintf(buf,"Weight: %d, Value: %d\n\r",
		obj->obj_flags.weight, obj->obj_flags.cost);
		sprintf(bufh,"무게: %d, 값: %d\n\r",
		obj->obj_flags.weight, obj->obj_flags.cost);
		send_to_char_han(buf, bufh, ch);

		switch (GET_ITEM_TYPE(obj)) {
			case ITEM_SCROLL : 
			case ITEM_POTION :
				sprintf(buf, "Level %d spells of:\n\r",
					obj->obj_flags.value[0]);
				sprintf(bufh, "%d 레벨에 해당하는 마법:\n\r",
					obj->obj_flags.value[0]);
				send_to_char_han(buf, bufh, ch);
				if (obj->obj_flags.value[1] >= 1) {
					sprinttype(obj->obj_flags.value[1]-1,spells,buf);
					strcat(buf,"\n\r");
					send_to_char(buf, ch);
				}
				if (obj->obj_flags.value[2] >= 1) {
					sprinttype(obj->obj_flags.value[2]-1,spells,buf);
					strcat(buf,"\n\r");
					send_to_char(buf, ch);
				}
				if (obj->obj_flags.value[3] >= 1) {
					sprinttype(obj->obj_flags.value[3]-1,spells,buf);
					strcat(buf,"\n\r");
					send_to_char(buf, ch);
				}
				break;
			case ITEM_WAND : 
			case ITEM_STAFF : 
				sprintf(buf, "Has %d charges, with %d charges left.\n\r",
					obj->obj_flags.value[1], obj->obj_flags.value[2]);
				send_to_char(buf, ch);

				sprintf(buf, "Level %d spell of:\n\r",
					obj->obj_flags.value[0]);
				sprintf(bufh, "%d 레벨에 해당하는 마법:\n\r",
					obj->obj_flags.value[0]);
				send_to_char_han(buf, bufh, ch);

				if (obj->obj_flags.value[3] >= 1) {
					sprinttype(obj->obj_flags.value[3]-1,spells,buf);
					strcat(buf,"\n\r");
					send_to_char(buf, ch);
				}
				break;
			case ITEM_WEAPON :
			case ITEM_FIREWEAPON :
				sprintf(buf, "Damage Dice is '%dD%d'\n\r",
					obj->obj_flags.value[1], obj->obj_flags.value[2]);
				sprintf(bufh, "Damage Dice 는 '%dD%d'\n\r",
					obj->obj_flags.value[1], obj->obj_flags.value[2]);
				send_to_char_han(buf, bufh, ch);
				if ( obj->obj_flags.gpd )
				{
					sprintf(buf,"This is magic weapon.\n\r");
					switch(obj->obj_flags.value[0])
					{
					case	WEAPON_SMASH:
						sprintf(buf,"smash\n\r"); break;
					case	WEAPON_FLAME:
						sprintf(buf,"flame\n\r"); break;
					case	WEAPON_ICE:
						sprintf(buf,"ice beam\n\r"); break;
					case	WEAPON_BOMBARD:
						sprintf(buf,"bombard\n\r"); break;
					case	WEAPON_SHOT:
						sprintf(buf,"shot\n\r"); break;
					case	WEAPON_DRAGON_SLAYER:
						sprintf(buf,"dragon slayer\n\r"); break;
					case	WEAPON_ANTI_EVIL_WEAPON:
						sprintf(buf,"anti evil\n\r"); break;
					case	WEAPON_ANTI_GOOD_WEAPON:
						sprintf(buf,"anti good\n\r"); break;
					case	WEAPON_GIANT_SLAYER:
						sprintf(buf,"giant slayer\n\r"); break;
					case	WEAPON_BEHEAD:
						sprintf(buf,"behead\n\r"); break;
					case	WEAPON_LIGHTNING:
						sprintf(buf,"lightning\n\r"); break;
					case	WEAPON_CALL_LIGHTNING:
						sprintf(buf,"call lightning\n\r"); break;
					case	WEAPON_FIREBALL:
						sprintf(buf,"fireball\n\r"); break;
					case	WEAPON_FIRE_BREATH:
						sprintf(buf,"fire breath\n\r"); break;
					case	WEAPON_FROST_BREATH:
						sprintf(buf,"frost breath\n\r"); break;
					case	WEAPON_ENERGY_DRAIN:
						sprintf(buf,"energy drain\n\r"); break;
					case	WEAPON_DISINTEGRATE:
						sprintf(buf,"disintegrate\n\r"); break;
					/*
					case	WEAPON_GOD:
						sprintf(buf,"god blessed\n\r"); break;
					*/
					case	WEAPON_ANY_MAGIC:
						sprintf(buf,"random magic\n\r"); break;
					default:
						sprintf(buf,"unknown magic\n\r");
					}
					send_to_char(buf,ch);
				}
				break;
			case ITEM_ARMOR :
				if (ac_applicable(obj)) {
					sprintf(buf, "AC-apply is %d\n\r", obj->obj_flags.value[0]);
					sprintf(bufh, "무장: %d\n\r", obj->obj_flags.value[0]);
					send_to_char_han(buf, bufh, ch);
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

				sprinttype(obj->affected[i].location,apply_types,buf2);
				sprintf(buf,"    Affects : %s By %d\n\r", buf2,
					obj->affected[i].modifier);
				sprintf(bufh,"    기능 : %d 만큼의 %s\n\r",
					obj->affected[i].modifier, buf2);
				send_to_char_han(buf, bufh, ch);
			}
		}
	}
	else {       /* victim */
		if (!IS_NPC(victim)) {
			sprintf(buf,"%d Years,  %d Months,  %d Days,  %d Hours old.\n\r",
				age(victim).year, age(victim).month,
				age(victim).day, age(victim).hours);
			sprintf(bufh,"나이 %d 년 %d 달 %d 일 %d 시간 입니다.\n\r",
				age(victim).year, age(victim).month,
				age(victim).day, age(victim).hours);
			send_to_char_han(buf, bufh, ch);

			sprintf(buf,"Height %dcm  Weight %dpounds \n\r",
				GET_HEIGHT(victim), GET_WEIGHT(victim));
			sprintf(bufh,"키 %dcm  몸무게 %d Kg\n\r",
				GET_HEIGHT(victim), GET_WEIGHT(victim));
			send_to_char_han(buf, bufh, ch);
			if (GET_LEVEL(victim) > 5) {
				sprintf(buf,"Str %d/%d,  Int %d,  Wis %d,  Dex %d,  Con %d\n\r",
					GET_STR(victim), GET_ADD(victim),
					GET_INT(victim),
					GET_WIS(victim),
					GET_DEX(victim),
					GET_CON(victim));
				send_to_char(buf,ch);
			}
		}
		else {
			send_to_char_han("You learn nothing new.\n\r",
				"별다른게 없네요.\n\r",ch);
		}
	}
}

int ac_applicable(struct obj_data *obj_object)
{
	return (CAN_WEAR(obj_object,ITEM_WEAR_KNEE) ||
		CAN_WEAR(obj_object,ITEM_WEAR_ABOUTLEGS) ||
		CAN_WEAR(obj_object,ITEM_WEAR_SHIELD) ||
		CAN_WEAR(obj_object,ITEM_WEAR_WRIST) ||
		CAN_WEAR(obj_object,ITEM_WEAR_WAISTE) ||
		CAN_WEAR(obj_object,ITEM_WEAR_ARMS) ||
		CAN_WEAR(obj_object,ITEM_WEAR_HANDS) ||
		CAN_WEAR(obj_object,ITEM_WEAR_FEET) ||
		CAN_WEAR(obj_object,ITEM_WEAR_LEGS) ||
		CAN_WEAR(obj_object,ITEM_WEAR_ABOUT) ||
		CAN_WEAR(obj_object,ITEM_WEAR_HEAD) ||
		CAN_WEAR(obj_object,ITEM_WEAR_BODY));
}

/* ***************************************************************************
 *                     NPC spells..                                          *
 * ************************************************************************* */

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
	if (number(0, IMO) < GET_LEVEL(ch)) {
		if (!saves_spell(victim, SAVING_BREATH)) {
			for (burn = victim->carrying; 
				burn && (burn->obj_flags.type_flag!=ITEM_SCROLL) && 
				(burn->obj_flags.type_flag!=ITEM_WAND) &&
				(burn->obj_flags.type_flag!=ITEM_STAFF) &&
				(burn->obj_flags.type_flag!=ITEM_NOTE) &&
				(number(0,4) <= 3);
				burn = burn->next_content) ;
			if (burn) {
				act("$o burns.",0,victim,burn,0,TO_CHAR);
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
	if (number(0, IMO) < GET_LEVEL(ch)) {
		if (!saves_spell(victim, SAVING_BREATH)) {
			for (frozen = victim->carrying; 
				frozen && (frozen->obj_flags.type_flag!=ITEM_SCROLL) && 
				(frozen->obj_flags.type_flag!=ITEM_WAND) &&
				(frozen->obj_flags.type_flag!=ITEM_STAFF) &&
				(frozen->obj_flags.type_flag!=ITEM_NOTE) &&
				(number(0,4) <= 3);
				frozen = frozen->next_content) ;
			if (frozen) {
				act("$o breaks.",0,victim,frozen,0,TO_CHAR);
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
	if (number(0, IMO) < GET_LEVEL(ch)) {
		if (!saves_spell(victim, SAVING_BREATH)) {
			for (melt = victim->carrying; 
				melt && (melt->obj_flags.type_flag!=ITEM_SCROLL) && 
				(melt->obj_flags.type_flag!=ITEM_WAND) &&
				(melt->obj_flags.type_flag!=ITEM_STAFF) &&
				(melt->obj_flags.type_flag!=ITEM_NOTE) &&
				(number(0,4) <= 3);
				melt = melt->next_content) ;
			if (melt) {
				act("$o is melting away.",0,victim,melt,0,TO_CHAR);
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
	if (number(0, IMO) < GET_LEVEL(ch)) {
		if (!saves_spell(victim, SAVING_BREATH)) {
			for (explode = victim->carrying; 
				explode && (explode->obj_flags.type_flag!=ITEM_SCROLL) && 
				(explode->obj_flags.type_flag!=ITEM_WAND) &&
				(explode->obj_flags.type_flag!=ITEM_STAFF) &&
				(explode->obj_flags.type_flag!=ITEM_NOTE) &&
				(number(0,4) <= 3);
				explode = explode->next_content) ;
			if (explode) {
				act("$o explodes.",0,victim,explode,0,TO_CHAR);
				extract_obj(explode);
			}
		}
	}
}
