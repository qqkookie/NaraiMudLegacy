
#include <stdio.h>
#include <string.h>

#include "char.h"
#include "object.h"
#include "global.h"
#include "comm.h"
#include "play.h"
#include "spells.h"

extern char *spell_wear_off_msg[];
/* Offensive Spells */

/* modified by atre */
void spell_magic_missile(byte level, struct char_data *ch,
			 struct char_data *victim, struct obj_data *obj)
{
    int dam;

    /* static int dam_each[] =  { 0, 10,10,15,15,15, 20,20,20,20,20,
       20,20,20,20,20, 20,20,20,20,20, 20,20,20,20,20, 20,20,20,20,20,
       20,20,20,20,20, 20,20,20,20,20, 20,20,20,20,20, 20,20,20,20,20 }; */

    INCREASE_SKILLED2(ch, victim, SPELL_MAGIC_MISSILE);
    dam = dice(level, level + GET_SKILLED(ch, SPELL_MAGIC_MISSILE));
    /* dam = number(dam_each[level] >> 1, dam_each[level] << 1); */

    if (saves_spell(victim, SAVING_SPELL))
	dam >>= 1;

    damage(ch, victim, dam, SPELL_MAGIC_MISSILE);
} 

void spell_chill_touch(byte level, struct char_data *ch,
		       struct char_data *victim, struct obj_data *obj)
{
    struct affected_type af;
    int dam;
    int chilled = FALSE;
    static int dam_each[] =
    {0,
     20, 20, 25, 25, 25, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30,
     35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35,
     40, 40, 40, 40, 40, 40, 40, 40, 40, 40};

    INCREASE_SKILLED2(ch, victim, SPELL_CHILL_TOUCH);
    dam = number(dam_each[level] >> 1, dam_each[level] << 1);
    if (!saves_spell(victim, SAVING_SPELL)) {

	af.type = SPELL_CHILL_TOUCH;
	af.duration = 6 + (GET_SKILLED(ch, SPELL_CHILL_TOUCH) >> 4);
	af.modifier = -1;
	af.location = APPLY_STR;
	af.bitvector = 0;
	affect_join(victim, &af, TRUE, FALSE);

	chilled = TRUE;
	if ( number(1, 101) < 
		GET_LEVEL(ch) + GET_SKILLED(ch, SPELL_CHILL_TOUCH)/2 ) {
	    /* NOTE: Chill touch will remove 'improved haste' spell */
	    if ( IS_AFFECTED( victim, AFF_IMPROVED_HASTE )) {
		if (affected_by_spell(victim, SPELL_IMPROVED_HASTE)) {
		    affect_from_char(victim, SPELL_IMPROVED_HASTE);
		    act( spell_wear_off_msg[SPELL_IMPROVED_HASTE], 
			FALSE, victim, 0, 0, TO_CHAR);
		} 
		if ( IS_AFFECTED( victim, AFF_IMPROVED_HASTE ))
		    REMOVE_BIT( victim->specials.affected_by,
			    AFF_IMPROVED_HASTE );
	    }
	    /* NOTE: Chill touch will remove 'haste' spell */
	    else if ( IS_AFFECTED( victim, AFF_HASTE )) {
		if (affected_by_spell(victim, SPELL_HASTE)) {
		    affect_from_char(victim, SPELL_HASTE);
		    act( spell_wear_off_msg[SPELL_HASTE], 
			FALSE, victim, 0, 0, TO_CHAR);
		} 
		if ( IS_AFFECTED( victim, AFF_HASTE ))
		    REMOVE_BIT( victim->specials.affected_by, AFF_HASTE ); 
	    }
	    /* NOTE: Chill touch will lower victim's dextirity */
	    else if ( GET_DEX(victim) > 5 ) {
		af.type = SPELL_CHILL_TOUCH;
		af.duration = 2 + ((GET_SKILLED(ch, SPELL_CHILL_TOUCH )+19)/40);
		af.modifier = -1;
		af.location = APPLY_DEX;
		af.bitvector = 0;
		affect_join(victim, &af, TRUE, FALSE);
	    }
	    else
		chilled = FALSE;
	}
    }
    else {
	dam >>= 1;
    }
    damage(ch, victim, dam, SPELL_CHILL_TOUCH);

    /* NOTE: Show messge if victim is successfully chilled */
    if ( chilled ) {
	send_to_char("Chilling! Your body gets stiffened.\n\r", victim);
	act("$n seems less vigorous from your chilling.",
	    TRUE, victim, 0, 0, TO_ROOM);
    }
}

void spell_burning_hands(byte level, struct char_data *ch,
			 struct char_data *victim, struct obj_data *obj)
{
    int dam;
    static int dam_each[] =
    {0,
     30, 30, 35, 35, 35, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40,
     45, 45, 45, 45, 45, 45, 45, 45, 45, 45, 45, 45, 45, 45, 45, 45, 45, 45, 45, 45,
     50, 50, 50, 50, 50, 50, 50, 50, 50, 50};

    INCREASE_SKILLED2(ch, victim, SPELL_BURNING_HANDS);
    dam = number(dam_each[level] >> 1, dam_each[level] << 1);

    if (saves_spell(victim, SAVING_SPELL))
	dam >>= 1;

    damage(ch, victim, dam, SPELL_BURNING_HANDS);
} 

void spell_shocking_grasp(byte level, struct char_data *ch,
			  struct char_data *victim, struct obj_data *obj)
{
    int dam;
    static int dam_each[] =
    {0,
     40, 40, 45, 45, 45, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50,
     55, 55, 55, 55, 55, 55, 55, 55, 55, 55, 55, 55, 55, 55, 55, 55, 55, 55, 55, 55,
     60, 60, 60, 60, 60, 60, 60, 60, 60, 60};

    INCREASE_SKILLED2(ch, victim, SPELL_SHOCKING_GRASP);
    dam = number(dam_each[level] >> 1, dam_each[level] << 1);
    if (saves_spell(victim, SAVING_SPELL))
	dam >>= 1;

    if (!saves_spell(victim, SAVING_PARA))
	GET_POS(victim) = POS_STUNNED;

    damage(ch, victim, dam, SPELL_SHOCKING_GRASP);
} 

void spell_lightning_bolt(byte level, struct char_data *ch,
			  struct char_data *victim, struct obj_data *obj)
{
    int dam;

    /* static int dam_each[] =  { 0, 100,100,100,100,100,
       100,100,100,100,100, 120,120,120,120,120, 130,130,130,130,130,
       140,140,140,140,140, 140,140,140,140,140, 150,150,150,150,150,
       150,150,150,150,150, 150,150,150,150,150, 150,150,150,150,150 }; */

    if (!ch || !victim)
	return;

    INCREASE_SKILLED2(ch, victim, SPELL_LIGHTNING_BOLT);

    dam = dice(level, level + GET_SKILLED(ch, SPELL_LIGHTNING_BOLT));

    if (saves_spell(victim, SAVING_SPELL))
	dam >>= 1;

    if (!saves_spell(victim, SAVING_PARA))
	GET_POS(victim) = POS_STUNNED;

    damage(ch, victim, dam, SPELL_LIGHTNING_BOLT);
} 

void spell_color_spray(byte level, struct char_data *ch,
		       struct char_data *victim, struct obj_data *obj)
{
    int dam;
    static int dam_each[] =
    {0,
     100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 120, 120, 120, 120, 120,
     130, 130, 130, 130, 130, 140, 140, 140, 140, 140, 140, 140, 140, 140, 140,
     150, 150, 150, 150, 150, 150, 150, 150, 150, 150, 150, 150, 150, 150, 150,
     150, 150, 150, 150, 150};

    INCREASE_SKILLED2(ch, victim, SPELL_COLOR_SPRAY);
    dam = number(dam_each[level], dam_each[level] << 1);
    if (saves_spell(victim, SAVING_SPELL))
	dam >>= 1;

    damage(ch, victim, dam, SPELL_COLOR_SPRAY);
} 

/* Drain XP, MANA, HP - caster gains HP and MANA */
void spell_energy_drain(byte level, struct char_data *ch,
			struct char_data *victim, struct obj_data *obj)
{
    int dam, xp, mana;
    int alignment;

    if (!ch)
	return;
    if (!victim)
	return;

    INCREASE_SKILLED2(ch, victim, SPELL_ENERGY_DRAIN);

    if (!saves_spell(victim, SAVING_PARA)) {
	alignment = (GET_ALIGNMENT(ch) + 1000) / 10;
	GET_ALIGNMENT(ch) = MAX(-1000, GET_ALIGNMENT(ch) - alignment);
	if (GET_LEVEL(victim) <= 2) {
	    damage(ch, victim, 100, SPELL_ENERGY_DRAIN);	/* Kill the
								   sucker */
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
	    GET_MANA(ch) = MIN(GET_MANA(ch), mana_limit(ch));
	    GET_HIT(ch) = MIN(GET_HIT(ch), hit_limit(ch));
	    send_to_char("Your life energy is drained!\n\r", victim);
	    damage(ch, victim, dam, SPELL_ENERGY_DRAIN);
	}
    }
    else {
	damage(ch, victim, 0, SPELL_ENERGY_DRAIN);	/* Miss */
    }
}

/* modified by atre */
void spell_fireball(byte level, struct char_data *ch,
		    struct char_data *victim, struct obj_data *obj)
{
    int dam;
    extern struct weather_data weather_info;

    /* static int dam_each[] =  { 0, 100,100,100,100,100,
       100,100,100,100,100, 140,140,140,140,140, 150,150,150,150,150,
       160,160,160,160,160, 160,160,160,160,160, 170,170,170,170,170,
       170,170,170,170,170, 170,170,170,170,170, 180,180,180,180,180 }; */

    if (!ch || !victim)
	return;

    INCREASE_SKILLED2(ch, victim, SPELL_FIREBALL);
    /* dam = number(dam_each[level], dam_each[level] << 1); */
    dam = dice(level, (level + GET_SKILLED(ch, SPELL_FIREBALL)) >> 1);


    if (saves_spell(victim, SAVING_SPELL))
	dam >>= 1;

    if (OUTSIDE(ch) && (weather_info.sky == SKY_CLOUDLESS)) {
	send_to_char("Your fireball works excellent!!\n\r", ch);
	act("$n suffers from hot ball.", FALSE, victim, 0, 0, TO_ROOM);
	dam *= GET_INT(ch);
    }
    damage(ch, victim, dam, SPELL_FIREBALL);
}

void spell_disintegrate(byte level, struct char_data *ch,
			struct char_data *victim, struct obj_data *obj)
{
    int dam;

    if (!ch || !victim)
	return;

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
void spell_cone_of_ice(byte level, struct char_data *ch,
		       struct char_data *victim, struct obj_data *obj)
{
    int dam;

    /* static int dam_each[] =  { 0, 500,500,500,500,500,
       500,500,500,500,500, 540,540,540,540,540, 550,550,550,550,550,
       560,560,560,560,560, 560,560,560,560,560, 570,570,570,570,570,
       570,570,570,570,570, 570,570,570,570,570, 580,580,580,580,580 }; */

    if (!ch || !victim)
	return;

    INCREASE_SKILLED2(ch, victim, SPELL_CONE_OF_ICE);
    /* dam = number(dam_each[level], dam_each[level] << 1); */
    dam = dice(level * GET_INT(ch), (level + (GET_SKILLED(ch, SPELL_CONE_OF_ICE) >> 3)));
    if (saves_spell(victim, SAVING_SPELL))
	dam >>= 1;
    damage(ch, victim, dam, SPELL_CONE_OF_ICE);
}

void spell_sunburst(byte level, struct char_data *ch,
		    struct char_data *victim, struct obj_data *obj)
{
    int dam;
    void spell_blindness(byte level, struct char_data *ch,
			 struct char_data *victim, struct obj_data *obj);
    static int dam_each[] =
    {0,
     400, 400, 400, 400, 400, 400, 400, 400, 400, 400, 440, 440, 440, 440, 440,
     440, 440, 440, 440, 440, 460, 460, 460, 460, 460, 460, 460, 460, 460, 460,
     470, 470, 470, 470, 470, 470, 470, 470, 470, 470, 470, 470, 470, 470, 470,
     480, 480, 480, 480, 480};

    if (!ch || !victim)
	return;

    INCREASE_SKILLED2(ch, victim, SPELL_SUNBURST);
    dam = number(dam_each[level], dam_each[level] << 1);
    if (saves_spell(victim, SAVING_SPELL))
	dam >>= 1;
    if (number(1, 15) == 1)
	spell_blindness(level, ch, victim, 0);
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

	/* dam = dice(level, 30) + 200; */
	dam = dice(level, (level >> 1) + GET_SKILLED(ch, SPELL_ENERGY_FLOW));
	dam <<= (1 + (GET_SKILLED(ch, SPELL_ENERGY_FLOW) >> 5));

	send_to_char("Energyflow which is made of your Exp Quaaaaaaa!\n\r", ch);

	exp = level * level * level * (20 - (GET_SKILLED(ch, SPELL_ENERGY_FLOW) >> 3));
	GET_EXP(ch) -= exp;

	if (saves_spell(victim, SAVING_SPELL))
	    dam >>= 1;

	damage(ch, victim, dam, SPELL_ENERGY_FLOW);
    }
    else
	send_to_char("You have too little experience...\n\r", ch);
}

void spell_full_fire(byte level, struct char_data *ch,
		     struct char_data *victim, struct obj_data *obj)
{
    int dam = 0;
    int i;

    /* int limit; int step; */

    assert(victim && ch);

    // OLD: if (!IS_NPC(ch) && GET_LEVEL(ch) == LEV_IMMO)
    if (prohibit_violence(ch)) {
	send_to_char("You cannot use this spell.\n\r", ch);
	return;
    }

    INCREASE_SKILLED2(ch, victim, SPELL_FULL_FIRE);

    dam = dice(level, GET_INT(ch) + (GET_SKILLED(ch, SPELL_FULL_FIRE) >> 4));

    if (saves_spell(victim, SAVING_SPELL)) {
	send_to_char("But, You couldn't concentrate enough\n\r", ch);
	GET_MANA(ch) -= dam;
    }
    else {
	i = GET_MANA(ch) / (500 - level - GET_SKILLED(ch, SPELL_FULL_FIRE));
	for (; i; i--) {
	    dam += dice(level, level);
	}
	/* 
	limit = level * number(200, 300); 
	i = GET_MANA(ch) > limit ?
	GET_MANA(ch) : 0;
	step = (300 - (level << 2)) << 1;
	for (dam = GET_MANA(ch); i > 0; i -= step)
	    dam += dice(level, level);
	*/
	GET_MANA(ch) = 0;
	/* WAIT_STATE(ch, 3 * PULSE_VIOLENCE); */
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
	dam = MIN(level * level, level * level * 3);
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

/* NOTE: NEW! Is victim is safe from casters room-wide spell like 
	'earthquake', 'firestorm', 'hand of god'.
	Char. of same PC/NPC kind, caster's master, mob charmed by caster 
	is considered safe. It doesn't check victim's room. */
int safe_from_room_spell( struct char_data *ch, struct char_data *victim,
			    int spell)
{ 
    return( ch == victim || IS_NPC(ch) == IS_NPC(victim)
	|| (victim->master && (victim->master == ch))
	|| (ch->master && (ch->master == victim)));
}

/* modified by atre */
void spell_earthquake(byte level, struct char_data *ch,
		      struct char_data *victim, struct obj_data *obj)
{
    int dam;
    struct char_data *tmp_victim, *temp;

    /* static int dam_each[] =  { 0, 40,40,45,45,45, 100,100,100,100,100,
       100,100,100,100,100, 100,100,100,100,100, 120,120,120,120,120,
       120,120,120,120,120, 130,130,130,130,130, 130,130,130,130,130,
       140,140,140,140,140, 150,150,150,150,150 }; */

    INCREASE_SKILLED2(ch, ch, SPELL_EARTHQUAKE);

    dam = dice(level, (level >> 2) + GET_SKILLED(ch, SPELL_EARTHQUAKE));
    /* dam = dam_each[level] + number(level, level << 1); */
    send_to_char("The earth trembles beneath your feet!\n\r", ch);
    act("$n makes the earth tremble and shiver\n\rYou fall, and hit yourself!",
	FALSE, ch, 0, 0, TO_ROOM);
    for (tmp_victim = character_list; tmp_victim; tmp_victim = temp) {
	temp = tmp_victim->next;
	if ((ch->in_room == tmp_victim->in_room) && (ch != tmp_victim)) {
	    /* NOTE: NPC can use this spell. Charmed mob OK.  */
	    if ( safe_from_room_spell( ch, tmp_victim, SPELL_EARTHQUAKE))
		continue;

	    /* NOTE: damage type: SPELL_EARTHQUAKE -> TYPE_MISC */
	    damage(ch, tmp_victim, dam, TYPE_MISC);

	    if (!IS_NPC(tmp_victim)) {
		WAIT_STATE(tmp_victim,
	       PULSE_VIOLENCE * (1 + (GET_SKILLED(ch, SPELL_EARTHQUAKE) >> 5)));
	    }
	}
	else if (world[ch->in_room].zone == world[tmp_victim->in_room].zone)
	    send_to_char("The earth trembles and shivers.\n\r", tmp_victim);
    } 
    /*  WAIT_STATE(ch, PULSE_VIOLENCE); */
}

void spell_all_heal(byte level, struct char_data *ch,
		    struct char_data *victim, struct obj_data *obj)
{
    struct char_data *tmp_victim, *temp;
    void spell_heal(byte level, struct char_data *ch,
		    struct char_data *victim, struct obj_data *obj);

    INCREASE_SKILLED2(ch, ch, SPELL_ALL_HEAL);

    /* going to saint */
    /* NOTE: Slow down align change : level/4 -> level/10 */
    GET_ALIGNMENT(ch) += GET_LEVEL(ch) /10;

    send_to_char("You feel some saintness!\n\r", ch);
    act("$n is glowing with saintly aura!", FALSE, ch, 0, 0, TO_ROOM);

    for (tmp_victim = character_list; tmp_victim; tmp_victim = temp) {
	temp = tmp_victim->next;
	if ((ch->in_room == tmp_victim->in_room) && (!IS_NPC(ch))) {
	    spell_heal(level, ch, tmp_victim, obj);
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
    for (tmp_victim = character_list; tmp_victim; tmp_victim = temp) {
	temp = tmp_victim->next;
	if ((ch->in_room == tmp_victim->in_room) && (ch != tmp_victim)) {
	    /* NOTE: NPC can use this spell. Charmed mob OK.  */
	    if ( safe_from_room_spell( ch, tmp_victim, SPELL_HAND_OF_GOD))
		continue;
	    damage(ch, tmp_victim, dam, TYPE_MISC);
	}
	else if (world[ch->in_room].zone == world[tmp_victim->in_room].zone) {
	    send_to_char("Dirty hands with long nail is ", tmp_victim);
	    send_to_char("going and coming all over sky.\n\r", tmp_victim);
	}
    }
}

void spell_firestorm(byte level, struct char_data *ch,
		      struct char_data *victim, struct obj_data *obj)
{
    int dam;
    struct char_data *tmp_victim, *temp;

    INCREASE_SKILLED2(ch, ch, SPELL_FIRESTORM);

    dam = dice(level, (level >> 1) + GET_SKILLED(ch, SPELL_FIRESTORM));
    send_to_char("The fire storm is flowing in the air!\n\r", ch);
    act("$n makes the fire storm flowing in the air.\n\rYou can't see anything!",
	    FALSE, ch, 0, 0, TO_ROOM);
    for (tmp_victim = character_list; tmp_victim; tmp_victim = temp) {
	temp = tmp_victim->next;
	if ((ch->in_room == tmp_victim->in_room) && (ch != tmp_victim)) {
	    /* NOTE: NPC can use this spell. Charmed mob OK.  */
	    if ( safe_from_room_spell( ch, tmp_victim, SPELL_FIRESTORM))
		continue;
	    damage(ch, tmp_victim, dam, TYPE_MISC);
	}
	else if (world[ch->in_room].zone == world[tmp_victim->in_room].zone)
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
	    if (level_gap > 6)
		level_gap = 6;

	    percent = ch->skills[SPELL_TRICK].learned - 195
		+ level_gap * 10
		+ number(1, 155);

	    while (!IS_NPC(tmp_victim) && tmp_victim)
		tmp_victim = tmp_victim->next_in_room;

	    if (percent > 0)
		hit(victim, tmp_victim, TYPE_UNDEFINED);
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

    if (!ch || !victim)
	return;

    INCREASE_SKILLED2(ch, victim, SPELL_CALL_LIGHTNING);

    if (OUTSIDE(ch) && (weather_info.sky >= SKY_RAINING)) {
	dam = dice(level, level + GET_SKILLED(ch, SPELL_CALL_LIGHTNING));
	/* 
	   dam = dice(level, level); */
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

    /* int limit;   */

    INCREASE_SKILLED2(ch, victim, SPELL_HARM);

    dam = dice(level + GET_WIS(ch), level + (GET_SKILLED(ch, SPELL_HARM) << 1));

    if (saves_spell(victim, SAVING_SPELL)) {
	dam >>= 1;
    }

    if (GET_HIT(victim) < dam) {
	dam = 0;	/* Kill the suffering bastard */
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


/* ************************************************************************
   *  file: magic3.c , Implementation of spells.             Part of NARAIMUD*
   *  Usage : The actual effect of magic.                                    *
   ************************************************************************* */ 

/*
   cast 'spell block' victim
   while affected hours,the victim is blocked by any spell.
   by process
 */
void spell_spell_block(byte level, struct char_data *ch,
		       struct char_data *victim, struct obj_data *obj)
{
    struct affected_type af;

    if (number(1, 10) < 4) {
	send_to_char("You failed to cast spell block!!!", ch);
	if (victim && victim->specials.fighting != ch
	    && victim->in_room == ch->in_room) {
	    if (!victim->specials.fighting) {
		if (victim != ch) {
		    hit(victim, ch, TYPE_UNDEFINED);
		}
	    }
	    return;
	}
    }
    if (!affected_by_spell(victim, SPELL_SPELL_BLOCK)) {
	INCREASE_SKILLED2(ch, victim, SPELL_SPELL_BLOCK);
	act("$n is surrounded by a Magical cloud.", TRUE, victim, 0, 0, TO_ROOM);
	act("You feel safe. ", TRUE, victim, 0, 0, TO_CHAR);
	af.type = SPELL_SPELL_BLOCK;
	af.duration = 4;
	af.modifier = 0;
	af.location = APPLY_NONE;
	af.bitvector = AFF_SPELL_BLOCK;
	affect_to_char(victim, &af);
    }
    send_to_char("ok\n\r", ch);
}
void spell_reflect_damage(byte level, struct char_data *ch,
			  struct char_data *victim, struct obj_data *obj)
{
    struct affected_type af;

    if (!affected_by_spell(victim, SPELL_REFLECT_DAMAGE)) {
	INCREASE_SKILLED2(ch, victim, SPELL_REFLECT_DAMAGE);
	act("$n gets ready to reflect damages!", TRUE, ch, 0, 0, TO_ROOM);
	af.type = SPELL_REFLECT_DAMAGE;
	af.duration = 5;
	af.modifier = LEVEL_LIMIT+1 - (level/2);
	af.location = APPLY_AC;
	af.bitvector = AFF_REFLECT_DAMAGE;
	affect_to_char(victim, &af);
    }
    send_to_char("You get ready to reflect damages!", ch);
}
void spell_dumb(byte level, struct char_data *ch,
		struct char_data *victim, struct obj_data *obj)
{
    struct affected_type af;

    if (!affected_by_spell(victim, SPELL_DUMB)) {
	INCREASE_SKILLED2(ch, victim, SPELL_DUMB);
	act("$n gets DUMB!!", TRUE, victim, 0, 0, TO_ROOM);
	af.type = SPELL_DUMB;
	af.duration = 2;
	af.modifier = 0;
	af.location = 0;
	af.bitvector = AFF_DUMB;
	affect_to_char(victim, &af);
    }
}

void spell_holy_shield(byte level, struct char_data *ch,
		       struct char_data *victim, struct obj_data *obj)
{
    struct affected_type af;

    if (IS_NPC(victim))
	return;
    if (!IS_GOOD(ch)) {
	do_say(ch, "I forgot I am a devil!!!", 0);
	return;
    }
    if (!affected_by_spell(victim, SPELL_HOLY_SHIELD)
	&& !affected_by_spell(victim, SPELL_PROTECT_FROM_ALIGN)) {
	INCREASE_SKILLED2(ch, victim, SPELL_HOLY_SHIELD);
	act("$n is surrounded by a HOLY SHIELD!!!", TRUE, victim, 0, 0, TO_ROOM);
	act("You feel holy.", TRUE, victim, 0, 0, TO_CHAR);
	af.type = SPELL_HOLY_SHIELD;
	af.duration = level / 10;
	af.modifier = 0;
	af.location = APPLY_NONE;
	af.bitvector = AFF_HOLY_SHIELD;
	affect_to_char(victim, &af);
    }
    send_to_char("OK!\n\r", ch);
}
/*
   cast 'mana transfer' victim
 */
void spell_mana_transfer(byte level, struct char_data *ch,
			 struct char_data *victim, struct obj_data *obj)
{
    assert(ch);
    INCREASE_SKILLED2(ch, victim, SPELL_MANA_TRANSFER);
    act("$n's mana is transferring to $N!!!", TRUE, ch, 0, victim, TO_ROOM);
    act("You feel better!!!", TRUE, victim, 0, 0, TO_CHAR);
    GET_MANA(victim) += GET_MANA(ch);
    GET_MANA(ch) = 0;
}

/*
   cast 'kiss of process'
 */
void spell_kiss_of_process(byte level, struct char_data *ch,
			   struct char_data *victim, struct obj_data *obj)
{
    struct char_data *tmp_victim, *temp;
    extern void do_shout(struct char_data *ch, char *str, int cmd);

    assert(ch); 
    do_shout(ch, "HEAL THE WORLD!!!", 0);
    for (tmp_victim = character_list; tmp_victim; tmp_victim = temp) {
	temp = tmp_victim->next;

	if (!IS_NPC(tmp_victim)) {
	    GET_HIT(tmp_victim) = tmp_victim->points.max_hit;
	    send_to_char("Michael Jackson kisses you in your lips.\n\r", tmp_victim);
	    do_say(tmp_victim, "Heal the world!!", 0);
	}
    }
}


void spell_thunderbolt(byte level, struct char_data *ch,
			struct char_data *victim, struct obj_data *obj)
{
    struct obj_data *tmp_obj = NULL;
    int percent;
    int tmp;
    int level_plus;
    int flag = 0;

    assert(ch);

    if (!(OUTSIDE(ch) && weather_info.sky >= SKY_RAINING)) {
	act("Hmm...I don't wanna be a chicken...",
	    TRUE, ch, 0, 0, TO_CHAR);
	return;
    }

    INCREASE_SKILLED2(ch, ch, SPELL_THUNDERBOLT);
    if (weather_info.sky == SKY_LIGHTNING) {
	flag = 1;
    }
    level_plus = (GET_LEVEL(ch) - 30) / 2;
    percent = number(1, 100) + level_plus + 2 * weather_info.sky;
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
	tmp_obj->obj_flags.value[1] = number(1 + flag, 5) + number(1, level_plus);
	tmp_obj->obj_flags.value[2] = number(1 + flag, 5) + number(1, level_plus);
	tmp_obj->obj_flags.value[3] = 7;
	tmp_obj->obj_flags.weight = 8;
	tmp_obj->obj_flags.cost = 0;
	tmp_obj->next = object_list;
	object_list = tmp_obj;
	obj_to_room(tmp_obj, ch->in_room);

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

void spell_sanctuary_cloud(byte level, struct char_data *ch,
			   struct char_data *victim, struct obj_data *obj)
{
    struct char_data *tmp_victim, *temp;

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

