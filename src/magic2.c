/* ************************************************************************
*  file: magic2.c , Implementation of spells.             Part of DIKUMUD *
*  Usage : The actual effect of magic.                                    *
*  Copyright (C) 1990, 1991 - see 'license.doc' for complete information. *
************************************************************************* */

#include <stdio.h>
#include <assert.h>
#include "structs.h"
#include "utils.h"
#include "comm.h"
#include "spells.h"
#include "handler.h"
#include "limits.h"
#include "db.h"
#include "memory.h"
#include "command.h"

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
char *string_dup(char *source);
int dice(int number, int size);
void do_look(struct char_data *ch, char *argument, int cmd);


/* spells2.c - Not directly offensive spells */

void spell_armor(byte level, struct char_data *ch,
                 struct char_data *victim, struct obj_data *obj)
{
    struct affected_type af;

    assert(victim);
    assert((level >= 0) && (level <= 55));
    if (!affected_by_spell(victim, SPELL_ARMOR)) {
        af.type      = SPELL_ARMOR;
        af.duration  = 24;
        af.modifier  = -20;
        af.location  = APPLY_AC;
        if(level>30) af.modifier=af.modifier-level+30;
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
	int choose_teleport_room(struct char_data *ch) ;	/* spec_procs3.c */

    assert(ch);
#ifdef DEATHFIGHT
	send_to_char_han("You can't use this magic while DEATH FIGHT\n\r",
                     "Death Fight 하는 동안에는 이 마법을 쓸 수 없습니다.\n\r", ch) ;
	return ;
#endif 

	/* choose room */
	to_room = choose_teleport_room(ch) ;

    if (ch->specials.fighting)
        stop_fighting(ch);
    act("$n slowly fade out of existence.", FALSE, ch,0,0,TO_ROOM);
    char_from_room(ch);
    char_to_room(ch, to_room);
    act("$n slowly fade in to existence.", FALSE, ch,0,0,TO_ROOM);
    do_look(ch, "", 0);
}

void spell_far_look(byte level, struct char_data *ch,
                    struct char_data *victim, struct obj_data *obj)
{
    int location,i;

    assert(ch);
    if (CAN_SEE(ch,victim)){
        i=GET_LEVEL(ch)-GET_LEVEL(victim);
        if(i < 0){
            send_to_char("Your spell is not powerful enough for that.\n\r",ch);
            return;
        } 
        location = victim->in_room;
    } else {
        send_to_char("No such creature around, what a waste.\n\r", ch);
        return;
    }
    if (IS_SET(world[location].room_flags, PRIVATE) ||
        IS_SET(world[location].room_flags, NORELOCATE) ||
        IS_SET(world[location].room_flags, OFF_LIMITS)) {
        send_to_char( "You fail miserably.\n\r", ch);
        return;
    }
    send_to_char(world[location].name,ch);
    send_to_char("\n\r",ch);
    send_to_char(world[location].description,ch);
    list_obj_to_char(world[location].contents,ch,0,FALSE);
    list_char_to_char(world[location].people,ch,0);
}

void spell_relocate(byte level, struct char_data *ch,
                    struct char_data *victim, struct obj_data *obj)
{
    char buf[MAX_INPUT_LENGTH];
    int location,i;
    struct char_data *pers;

    assert(ch);
    if (CAN_SEE(ch,victim)){
        i=GET_LEVEL(ch)-GET_LEVEL(victim);
        if(i < 0){
            send_to_char("Your spell is not powerful enough for that.\n\r",ch);
            return;
        } else if(number(0,2) > i){
            send_to_char("You make a valiant effort, but barely fail.\n\r",ch);
            send_to_char("Temporary note: This means you had a real chance.\n\r",ch);
            return;
        }
        location = victim->in_room;
    } else {
        send_to_char("No such creature around, what a waste.\n\r", ch);
        return;
    }
    if (IS_SET(world[location].room_flags, PRIVATE) ||
        IS_SET(world[location].room_flags, NORELOCATE) ||
        IS_SET(world[location].room_flags, OFF_LIMITS)) {
        send_to_char( "You fail miserably.\n\r", ch);
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

    assert(ch && (victim));
    assert((level >=0) && (level <= 55));
    if ((GET_POS(victim) != POSITION_FIGHTING) &&
        (!affected_by_spell(victim, SPELL_DAMAGE_UP))) {

        send_to_char("You feel brave.\n\r", victim);
        af.type      = SPELL_DAMAGE_UP;
        af.duration  = 4+level/10;
        af.modifier  = level/14+1;
        af.location  = APPLY_DAMROLL;
        af.bitvector = 0;
        affect_to_char(victim, &af);
    }
}
void spell_crush_armor(byte level, struct char_data *ch,
                       struct char_data *victim,struct obj_data *obj)
{
    struct affected_type af;

    assert(ch && (victim));
    assert((level >=15) && (level <= 55));
    if ((GET_POS(victim) != POSITION_FIGHTING) &&
        (!affected_by_spell(victim, SPELL_CRUSH_ARMOR))) {

        send_to_char("Your victim's armor has crumbled!\n\r", ch);
        send_to_char("You feel shrink.\n\r", victim);
        af.type      = SPELL_CRUSH_ARMOR;
        af.duration  = 3;
        af.modifier  = level/10*(-1) ;
        af.location  = APPLY_DAMROLL;
        af.bitvector = 0;
        affect_to_char(victim, &af);
        af.location  = APPLY_AC;
        af.modifier  = 2*level+8;
        affect_to_char(victim, &af);
    }
}
void spell_bless(byte level, struct char_data *ch,
                 struct char_data *victim, struct obj_data *obj)
{
    struct affected_type af;

    assert(ch && (victim || obj));
    assert((level >= 0) && (level <= 55));

    if (obj) {
        if ( (5*GET_LEVEL(ch) > GET_OBJ_WEIGHT(obj)) &&
             (GET_POS(ch) != POSITION_FIGHTING) &&
             !IS_OBJ_STAT(obj, ITEM_EVIL)) {
            SET_BIT(GET_OBJ_STAT(obj), ITEM_BLESS);
            act("$p briefly glows.",FALSE,ch,obj,0,TO_CHAR);
            GET_OBJ_RENTCOST(obj) = 0 ;	/* reset rent cost */
        }
    } else {

        if ((GET_POS(victim) != POSITION_FIGHTING) &&
            (!affected_by_spell(victim, SPELL_BLESS))) {

            send_to_char("You feel righteous.\n\r", victim);
            af.type      = SPELL_BLESS;
            af.duration  = 6;
            af.modifier  = 1;
            af.location  = APPLY_HITROLL;
            af.bitvector = 0;
            affect_to_char(victim, &af);

            af.location = APPLY_SAVING_SPELL;
            af.modifier = -1;                 /* Make better */
            affect_to_char(victim, &af);
        }
    }
}


void spell_blindness(byte level, struct char_data *ch,
                     struct char_data *victim, struct obj_data *obj)
{
    struct affected_type af;

    /* assert(ch && victim); */
	if ( ch == NULL ) {
		log("spell_blindness : no ch is present") ;
		return ;
    }
	if ( victim == NULL ) {
		log("spell_blindness : no victim is present") ;
		return ;
    }
    assert((level >= 0) && (level <= 55));

    if(GET_LEVEL(ch) < (IMO+3))
        if (saves_spell(victim, SAVING_SPELL) ||
            affected_by_spell(victim, SPELL_BLINDNESS))
            return;
    if(level+number(1,15) < GET_LEVEL(victim)) return;

    act("$n seems to be blinded!", TRUE, victim, 0, 0, TO_ROOM);
    send_to_char("You have been blinded!\n\r", victim);

    af.type      = SPELL_BLINDNESS;
    af.location  = APPLY_HITROLL;
    af.modifier  = -4;  /* Make hitroll worse */
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
    assert((level >= 0) && (level <= 55));

	/*
      create(tmp_obj, struct obj_data, 1);
	*/
	tmp_obj = (struct obj_data *) malloc_general_type(MEMORY_OBJ_DATA) ;
    clear_object(tmp_obj);

    tmp_obj->name = string_dup("mushroom");
    tmp_obj->short_description = string_dup("A Magic Mushroom");
    tmp_obj->description = string_dup("A really delicious looking magic mushroom lies here.");

    tmp_obj->obj_flags.type_flag = ITEM_FOOD;
    tmp_obj->obj_flags.wear_flags = ITEM_TAKE | ITEM_HOLD;
    tmp_obj->obj_flags.value[0] = 5+level;
    tmp_obj->obj_flags.weight = 1;
    tmp_obj->obj_flags.cost = 10;

    tmp_obj->next = object_list;
    object_list = tmp_obj;

    obj_to_room(tmp_obj,ch->in_room);

    tmp_obj->item_number = -1;

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

    if (GET_ITEM_TYPE(obj) == ITEM_DRINKCON) {
        if ((obj->obj_flags.value[2] != LIQ_WATER)
            && (obj->obj_flags.value[1] != 0)) {

            name_from_drinkcon(obj);
            obj->obj_flags.value[2] = LIQ_SLIME;
            name_to_drinkcon(obj, LIQ_SLIME);

        } else {

            water = 2*level * ((weather_info.sky >= SKY_RAINING) ? 2 : 1);

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

    if (GET_ITEM_TYPE(obj) == ITEM_DRINKCON) {
        if ((obj->obj_flags.value[2] != LIQ_NECTAR)
            && (obj->obj_flags.value[1] != 0)) {

            name_from_drinkcon(obj);
            obj->obj_flags.value[2] = LIQ_SLIME;
            name_to_drinkcon(obj, LIQ_SLIME);

        } else {

            nectar = 2*level;

            /* Calculate water it can contain, or water created */
            nectar = MIN(obj->obj_flags.value[0]-obj->obj_flags.value[1], nectar);

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

    if (GET_ITEM_TYPE(obj) == ITEM_DRINKCON) {
        if ((obj->obj_flags.value[2] != LIQ_GOLDEN_NECTAR)
            && (obj->obj_flags.value[1] != 0)) {

            name_from_drinkcon(obj);
            obj->obj_flags.value[2] = LIQ_SLIME;
            name_to_drinkcon(obj, LIQ_SLIME);

        } else {

            nectar = 3*level/2;

            /* Calculate water it can contain, or water created */
            nectar = MIN(obj->obj_flags.value[0]-obj->obj_flags.value[1], nectar);

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
    assert((level >= 0) && (level <= 55));

    if (affected_by_spell(victim, SPELL_BLINDNESS)) {
        affect_from_char(victim, SPELL_BLINDNESS);

        send_to_char("Your vision returns!\n\r", victim);
    }
}


void spell_cure_critic(byte level, struct char_data *ch,
                       struct char_data *victim, struct obj_data *obj)
{
    int healpoints;

/*   assert(victim);*/
	if ( victim == NULL )
		return ;
    assert((level >= 0) && (level <= 55));
    healpoints = dice(3,8)+10;
    if ( (healpoints + GET_HIT(victim)) > hit_limit(victim) )
        GET_HIT(victim) = hit_limit(victim);
    else
        GET_HIT(victim) += healpoints;
    send_to_char("You feel better!\n\r", victim);
    update_pos(victim);
}

void spell_cause_critic(byte level, struct char_data *ch,
                        struct char_data *victim, struct obj_data *obj)
{
    int dampoints;

    assert(victim);
    assert((level >= 0) && (level <= 55));
    dampoints = dice(level/5,8)+3;
    send_to_char("You feel pain!\n\r", victim);
    damage(ch,victim,dampoints,SPELL_CAUSE_CRITIC);
}


void spell_mana_boost(byte level, struct char_data *ch,
                      struct char_data *victim, struct obj_data *obj)
{
    int healpoints=0;

    assert(victim);
    assert((level > 0) && (level < 55));
    if(GET_MOVE(victim)>50) healpoints = GET_MOVE(victim)/2+2*GET_LEVEL(victim);
    GET_MANA(victim) += healpoints;
    GET_MOVE(victim) =0; 
    send_to_char("Your mana is boosted!\n\r", victim);
    update_pos(victim);
}

void spell_vitalize(byte level, struct char_data *ch,
                    struct char_data *victim, struct obj_data *obj)
{
    int healpoints=0;

    assert(victim);
    assert((level > 0) && (level < 55));
    healpoints = GET_MOVE(victim)/dice(1,3)+GET_MOVE(victim)*level*dice(3,5)/300;
    GET_HIT(victim) += healpoints;
    GET_MOVE(victim) =0;
    GET_MANA(victim)-=10;
    send_to_char("You feel vitalized!\n\r", victim);
    update_pos(victim);
}
void spell_cure_light(byte level, struct char_data *ch,
                      struct char_data *victim, struct obj_data *obj)
{
    int healpoints;

    assert(ch && victim);
    assert((level >= 0) && (level <= 55));

    healpoints = dice(1,10)+3;

    if ( (healpoints+GET_HIT(victim)) > hit_limit(victim) )
        GET_HIT(victim) = hit_limit(victim);
    else
        GET_HIT(victim) += healpoints;

    update_pos( victim );

    send_to_char("You feel better!\n\r", victim);
}

void spell_cause_light(byte level, struct char_data *ch,
                       struct char_data *victim, struct obj_data *obj)
{
    int dampoints;

    assert(ch && victim);
    assert((level >= 0) && (level <= 55));

    dampoints = dice(1,10)+3;

    send_to_char("You feel pain!\n\r", victim);
    damage(ch,victim,dampoints,SPELL_CAUSE_LIGHT);
}



void spell_curse(byte level, struct char_data *ch,
                 struct char_data *victim, struct obj_data *obj)
{
    struct affected_type af;

    assert(victim || obj);
    assert((level >= 0) && (level <= 55));

    if (obj) {
        SET_BIT(GET_OBJ_STAT(obj), ITEM_EVIL);
        SET_BIT(GET_OBJ_STAT(obj), ITEM_NODROP);

        /* LOWER ATTACK DICE BY -1 */
        if(obj->obj_flags.type_flag == ITEM_WEAPON)
            obj->obj_flags.value[2]--;
        act("$p glows red.", FALSE, ch, obj, 0, TO_CHAR);
		GET_OBJ_RENTCOST(obj) = 0 ;	/* reset rent cost */
    } else {
        if ( victim == NULL )
            return ;
        if ( saves_spell(victim, SAVING_SPELL) ||
             affected_by_spell(victim, SPELL_CURSE))
            return;

        af.type      = SPELL_CURSE;
        af.duration  = 24*7;       /* 7 Days */
        af.modifier  = -1;
        af.location  = APPLY_HITROLL;
        af.bitvector = AFF_CURSE;
        affect_to_char(victim, &af);

        af.location = APPLY_SAVING_PARA;
        af.modifier = 1; /* Make worse */
        affect_to_char(victim, &af);

        act("$n briefly reveal a red aura!", FALSE, victim, 0, 0, TO_ROOM);
        act("You feel very uncomfortable.",FALSE,victim,0,0,TO_CHAR);
    }
}

void spell_paralysis(byte level, struct char_data *ch,
                 struct char_data *victim, struct obj_data *obj)
{
    struct affected_type af;

    assert(victim || obj);
    assert((level >= 0) && (level <= 55));

	if ( victim == NULL )
		return ;
	if ( saves_spell(victim, SAVING_PARA) ||
		affected_by_spell(victim, SPELL_PARALYSIS))
		return;

	af.type      = SPELL_PARALYSIS;
	af.duration  = level/10;       /* 4 tick */
	af.modifier  = -1 * level/10;
	af.location  = APPLY_DAMROLL;
	af.bitvector = AFF_PARALYSIS;
	affect_to_char(victim, &af);

	act("$n seems freezed!", FALSE, victim, 0, 0, TO_ROOM);
	act("You feel freezed.",FALSE,victim,0,0,TO_CHAR);
}

void spell_detect_align(byte level, struct char_data *ch,
                       struct char_data *victim, struct obj_data *obj)
{
    struct affected_type af;

    assert(victim);
    assert((level >= 0) && (level <= 55));

    if ( affected_by_spell(victim, SPELL_DETECT_ALIGN) )
        return;

    af.type      = SPELL_DETECT_ALIGN;
    af.duration  = level*5;
    af.modifier  = 0;
    af.location  = APPLY_NONE;
    af.bitvector = AFF_DETECT_ALIGN;

    affect_to_char(victim, &af);

    send_to_char("Your eyes tingle.\n\r", victim);
}



void spell_detect_invisibility(byte level, struct char_data *ch,
                               struct char_data *victim, struct obj_data *obj)
{
    struct affected_type af;

    assert(victim);
    assert((level >= 0) && (level <= 55));

    if ( affected_by_spell(victim, SPELL_DETECT_INVISIBLE) )
        return;

    af.type      = SPELL_DETECT_INVISIBLE;
/*    af.duration  = level*5; */
    af.duration  = level*5/5; 
    af.modifier  = 0;
    af.location  = APPLY_NONE;
    af.bitvector = AFF_DETECT_INVISIBLE;

    affect_to_char(victim, &af);

    send_to_char("Your eyes tingle.\n\r", victim);
}



void spell_recharger(byte level, struct char_data *ch,
                     struct char_data *victim, struct obj_data *obj)
{
    int i;

    assert(ch && obj);
    assert(MAX_OBJ_AFFECT >= 2);

    if ((GET_ITEM_TYPE(obj) == ITEM_WAND) ||
        (GET_ITEM_TYPE(obj) == ITEM_STAFF)) {
        if( number(1,1+level/5) == 1 ){
            act("BANG!!!! $p explodes. It hurts!",FALSE,ch,obj,0,TO_CHAR);
            extract_obj(obj);
            GET_HIT(ch)-=GET_HIT(ch)/10;
            return;
        }
        obj->obj_flags.value[2]=
            MIN(obj->obj_flags.value[1],obj->obj_flags.value[2]+5);
        act("$p looks recharged.",FALSE,ch,obj,0,TO_CHAR);
    }
}

void spell_enchant_weapon(byte level, struct char_data *ch,
                          struct char_data *victim, struct obj_data *obj)
{
    int i;

    assert(ch && obj);
    assert(MAX_OBJ_AFFECT >= 2);

    if ((GET_ITEM_TYPE(obj) == ITEM_WEAPON) &&
        !IS_OBJ_STAT(obj, ITEM_MAGIC)) {

        for (i=0; i < MAX_OBJ_AFFECT; i++)
            if (obj->affected[i].location != APPLY_NONE)
                return;

        SET_BIT(GET_OBJ_STAT(obj), ITEM_MAGIC);
		GET_OBJ_RENTCOST(obj) = 0 ;	/* reset rent cost */

        obj->affected[0].location = APPLY_HITROLL;
        obj->affected[0].modifier = 1 + (level >= 18) + (level >= 30);

        obj->affected[1].location = APPLY_DAMROLL;
        obj->affected[1].modifier = 1 + (level >= 20) + (level >= 35);

        if (IS_GOOD(ch)) {
            SET_BIT(GET_OBJ_STAT(obj), ITEM_ANTI_EVIL);
            act("$p glows blue.",FALSE,ch,obj,0,TO_CHAR);
        } else if (IS_EVIL(ch)) {
            SET_BIT(GET_OBJ_STAT(obj), ITEM_ANTI_GOOD);
            act("$p glows red.",FALSE,ch,obj,0,TO_CHAR);
        } else {
            act("$p glows yellow.",FALSE,ch,obj,0,TO_CHAR);
        }
    }
}

void spell_enchant_armor(byte level, struct char_data *ch,
                         struct char_data *victim, struct obj_data *obj)
{
    int i;

    assert(ch && obj);
    assert(MAX_OBJ_AFFECT >= 2);

    if ((GET_ITEM_TYPE(obj) == ITEM_ARMOR) &&
        !IS_OBJ_STAT(obj, ITEM_MAGIC)) {

        for (i=0; i < MAX_OBJ_AFFECT; i++)
            if (obj->affected[i].location != APPLY_NONE)
                return;

        SET_BIT(GET_OBJ_STAT(obj), ITEM_MAGIC);
		GET_OBJ_RENTCOST(obj) = 0 ;	/* reset rent cost */

        obj->affected[0].location = APPLY_AC;
        obj->affected[0].modifier = -1 - (level >= 35);

        if (IS_GOOD(ch)) {
            SET_BIT(GET_OBJ_STAT(obj), ITEM_ANTI_EVIL);
            act("$p glows blue.",FALSE,ch,obj,0,TO_CHAR);
        } else if (IS_EVIL(ch)) {
            SET_BIT(GET_OBJ_STAT(obj), ITEM_ANTI_GOOD);
            act("$p glows red.",FALSE,ch,obj,0,TO_CHAR);
        } else {
            act("$p glows yellow.",FALSE,ch,obj,0,TO_CHAR);
        }
    }
}



void spell_pray_for_armor(byte level, struct char_data *ch,
                          struct char_data *victim, struct obj_data *obj)
{
    int i;

    assert(ch && obj);
    assert(MAX_OBJ_AFFECT >= 2);

    if ((GET_ITEM_TYPE(obj) == ITEM_ARMOR) &&
        !IS_OBJ_STAT(obj, ITEM_MAGIC)) {

        for (i=0; i < MAX_OBJ_AFFECT; i++)
            if (obj->affected[i].location != APPLY_NONE)
                return;

        SET_BIT(GET_OBJ_STAT(obj), ITEM_MAGIC);
		GET_OBJ_RENTCOST(obj) = 0 ;	/* reset rent cost */

        obj->affected[0].location = APPLY_HITROLL;
        obj->affected[0].modifier = 1;
        if(level>=38){
            obj->affected[1].location=APPLY_HITROLL;
            obj->affected[1].modifier = 1;
        }

        if (IS_GOOD(ch)) {
            SET_BIT(GET_OBJ_STAT(obj), ITEM_ANTI_EVIL);
            act("$p glows blue.",FALSE,ch,obj,0,TO_CHAR);
        } else if (IS_EVIL(ch)) {
            SET_BIT(GET_OBJ_STAT(obj), ITEM_ANTI_GOOD);
            act("$p glows red.",FALSE,ch,obj,0,TO_CHAR);
        } else {
            act("$p glows yellow.",FALSE,ch,obj,0,TO_CHAR);
        }
    }
}


void spell_heal(byte level, struct char_data *ch,
                struct char_data *victim, struct obj_data *obj)
{
    assert(victim);

    spell_cure_blind(level, ch, victim, obj);

    GET_HIT(victim) += 100;

    if (GET_HIT(victim) >= hit_limit(victim))
        GET_HIT(victim) = hit_limit(victim)-dice(1,4);

    update_pos( victim );

    send_to_char("A warm feeling fills your body.\n\r", victim);
}
void spell_full_heal(byte level, struct char_data *ch,
                     struct char_data *victim, struct obj_data *obj)
{
    assert(victim);

    spell_cure_blind(level, ch, victim, obj);

    GET_HIT(victim) += 200;

    if (GET_HIT(victim) >= hit_limit(victim))
        GET_HIT(victim) = hit_limit(victim)-dice(1,4);

    update_pos( victim );

    send_to_char("A warm feeling fills your body.\n\r", victim);
}

void spell_invisibility(byte level, struct char_data *ch,
                        struct char_data *victim, struct obj_data *obj)
{
    struct affected_type af;

    assert((ch && obj) || victim);

    if (obj) {
        if ( !IS_OBJ_STAT(obj, ITEM_INVISIBLE) ) {
            act("$p turns invisible.",FALSE,ch,obj,0,TO_CHAR);
            act("$p turns invisible.",TRUE,ch,obj,0,TO_ROOM);
            SET_BIT(GET_OBJ_STAT(obj), ITEM_INVISIBLE);
        }
    } else {              /* Then it is a PC | NPC */
        if (!affected_by_spell(victim, SPELL_INVISIBLE)) {

            act("$n slowly fade out of existence.", TRUE, victim,0,0,TO_ROOM);
            send_to_char("You vanish.\n\r", victim);

            af.type      = SPELL_INVISIBLE;
            af.duration  = 24;
            af.modifier  = -40;
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
    int j, found ;

    assert(ch);
#ifdef DEATHFIGHT
	send_to_char_han("You can't use this magic while DEATH FIGHT\n\r",
                     "Death Fight 하는 동안에는 이 마법을 쓸 수 없습니다.\n\r", ch) ;
	return ;
#else 
	found = 0 ;
    strcpy(name, fname(obj->name));
    j=level>>1;
    for (i = object_list; i && (j>0); i = i->next)
        if (isname(name, i->name)) {
            found ++ ;
            if (i->rented_by) {	/* this is the rented object. don't concern */
                if ( GET_LEVEL(ch) > IMO ) {
                    sprintf(buf,"%s rented by %s.\n\r",
                            i->short_description,PERS(i->rented_by,ch));
                    send_to_char(buf,ch);
				}
                else {	/* just skip for mortals */
                    found -- ;
				}
			}
            else if(i->carried_by) {
                sprintf(buf,"%s carried by %s.\n\r",
                        i->short_description,PERS(i->carried_by,ch));
                send_to_char(buf,ch);
            } else if (i->in_obj) {
                sprintf(buf,"%s in %s.\n\r",i->short_description,
                        i->in_obj->short_description);
                send_to_char(buf,ch);
            } else {
                sprintf(buf,"%s in %s.\n\r",i->short_description,
                        ((i->in_room==NOWHERE) ? "uncertain." : world[i->in_room].name));
                send_to_char(buf,ch);
                if(GET_LEVEL(ch) < IMO) j--;
            }
        }
    if(j==0)
        send_to_char("You are very confused.\n\r",ch);
    if( found == 0 )
        send_to_char("No such object.\n\r",ch);
#endif 
}


void spell_poison(byte level, struct char_data *ch,
                  struct char_data *victim, struct obj_data *obj)
{
    struct affected_type af;

    assert(victim || obj);

    if (victim) {
        if(!saves_spell(victim, SAVING_PARA))
        {
            af.type = SPELL_POISON;
            af.duration = level*2;
            af.modifier = -2;
            af.location = APPLY_STR;
            af.bitvector = AFF_POISON;
            affect_join(victim, &af, FALSE, FALSE);
            act("$n seems to feel very sick.",FALSE,victim,0,0,TO_ROOM);
            send_to_char("You feel very sick.\n\r", victim);
        }
    } else { /* Object poison */
        if ((obj->obj_flags.type_flag == ITEM_DRINKCON) ||
            (obj->obj_flags.type_flag == ITEM_FOOD)) {
            obj->obj_flags.value[3] = 1;
        }
    }
}

void spell_protection_from_evil(byte level, struct char_data *ch,
                                struct char_data *victim, struct obj_data *obj)
{
    struct affected_type af;

    assert(victim);

    if (!affected_by_spell(victim, SPELL_PROTECT_FROM_EVIL) ) {
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
        !affected_by_spell(victim, SPELL_IMPROVED_HASTE) ) {
        af.type      = SPELL_HASTE;
        af.duration  = 2;
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
        af.type      = SPELL_IMPROVED_HASTE;
        af.duration  = 2;
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
    struct affected_type af;

    assert(ch && (victim || obj));

    if (obj) {

        if ( IS_OBJ_STAT(obj, ITEM_EVIL) || IS_OBJ_STAT(obj, ITEM_NODROP)) {
            act("$p briefly glows blue.", TRUE, ch, obj, 0, TO_CHAR);

            REMOVE_BIT(GET_OBJ_STAT(obj), ITEM_EVIL);
            REMOVE_BIT(GET_OBJ_STAT(obj), ITEM_NODROP);
            GET_OBJ_RENTCOST(obj) = 0 ;	/* reset rent cost */
        }
    } else {      /* Then it is a PC | NPC */
        if (affected_by_spell(victim, SPELL_CURSE) ) {
            act("$n briefly glows red, then blue.",FALSE,victim,0,0,TO_ROOM);
            act("You feel better.",FALSE,victim,0,0,TO_CHAR);
            affect_from_char(victim, SPELL_CURSE);
        }
    }
}

void spell_cure_paralysis(byte level, struct char_data *ch,
                         struct char_data *victim, struct obj_data *obj)
{
    assert(ch && victim);

    if (victim) {
        if(affected_by_spell(victim,SPELL_PARALYSIS)) {
            affect_from_char(victim,SPELL_PARALYSIS);
            act("A warm feeling runs through your body.",FALSE,victim,0,0,TO_CHAR);
            act("$N looks better.",FALSE,victim,0,victim,TO_ROOM);
        }
    }
}

void spell_remove_poison(byte level, struct char_data *ch,
                         struct char_data *victim, struct obj_data *obj)
{
    assert(ch && (victim || obj));

    if (victim) {
        if(affected_by_spell(victim,SPELL_POISON)) {
            affect_from_char(victim,SPELL_POISON);
            act("A warm feeling runs through your body.",FALSE,victim,0,0,TO_CHAR);
            act("$N looks better.",FALSE,ch,0,victim,TO_ROOM);
        }
    } else {
        if ((obj->obj_flags.type_flag == ITEM_DRINKCON) ||
            (obj->obj_flags.type_flag == ITEM_FOOD)) {
            obj->obj_flags.value[3] = 0;
            act("The $p steams briefly.",FALSE,ch,obj,0,TO_CHAR);
        }
    }
}


void spell_infravision(byte level, struct char_data *ch,
                       struct char_data *victim, struct obj_data *obj)
{
    struct affected_type af;

    if(IS_NPC(victim)) return;
    if (!affected_by_spell(victim, SPELL_INFRAVISION) ) {
        act("$n's eyes glow red.",TRUE,victim,0,0,TO_ROOM);
        act("You feel your eyes become more sensitive.",TRUE,victim,0,0,TO_CHAR);
        af.type      = SPELL_INFRAVISION;
        af.duration  = 2*level;
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

    if(IS_NPC(victim)) return;
    if (!affected_by_spell(victim, SPELL_MIRROR_IMAGE) ) {
        act("$n creats own mirror image.",TRUE,victim,0,0,TO_ROOM);
        act("You made your illusion.",TRUE,victim,0,0,TO_CHAR);
        af.type      = SPELL_MIRROR_IMAGE;
        af.duration  = (level<IMO) ? 3 : level;
        af.duration += (level>30) + (level>35);
        af.modifier  = 0;
        af.location  = APPLY_NONE;
        af.bitvector = AFF_MIRROR_IMAGE;
        affect_to_char(victim, &af);
    }
}


void spell_sanctuary(byte level, struct char_data *ch,
                     struct char_data *victim, struct obj_data *obj)
{
    struct affected_type af;

    if(IS_NPC(victim)) return;
    if (!affected_by_spell(victim, SPELL_SANCTUARY) ) {
        act("$n is surrounded by a white aura.",TRUE,victim,0,0,TO_ROOM);
        act("You start glowing.",TRUE,victim,0,0,TO_CHAR);
        af.type      = SPELL_SANCTUARY;
        af.duration  = (level<IMO) ? 3 : level;
        af.duration += (level>30) + (level>35);
        af.modifier  = 0;
        af.location  = APPLY_NONE;
        af.bitvector = AFF_SANCTUARY;
        affect_to_char(victim, &af);
    }
}

void spell_group_sanctuary(byte level, struct char_data *ch,
                     struct char_data *victim, struct obj_data *obj)
{
    struct affected_type af;
	int org_room;
	struct follow_type *f;
	struct char_data *k;


	assert(ch);
	assert(level >= 35 && level <= 55);

	org_room = ch->in_room;

	if (ch->master) 
		k = ch->master;
	else 
		k = ch;

	if (IS_AFFECTED(k, AFF_GROUP) && !IS_NPC(k) 
		&& org_room == k->in_room) {

		affect_from_char(k, SPELL_SANCTUARY);

        act("$n is surrounded by a white aura.",TRUE,k,0,0,TO_ROOM);
        act("You start glowing.",TRUE,k,0,0,TO_CHAR);
        af.type      = SPELL_SANCTUARY;
        af.duration  = (level<IMO) ? 1 : 3;
        af.duration += (level>35) + (level>39);
        af.modifier  = 0;
        af.location  = APPLY_NONE;
        af.bitvector = AFF_SANCTUARY;
        affect_to_char(k, &af);
	}


	for (f= k->followers; f; f = f->next) {
		if (org_room == f->follower->in_room && 
			!IS_NPC(f->follower) && IS_AFFECTED(f->follower, AFF_GROUP)) {

			affect_from_char(f->follower, SPELL_SANCTUARY);

			act("$n is surrounded by a white aura.",
				TRUE,f->follower,0,0,TO_ROOM);
			act("You start glowing.",TRUE,f->follower,0,0,TO_CHAR);
			af.type      = SPELL_SANCTUARY;
			af.duration  = (level<IMO) ? 1 : 3;
			af.duration += (level>35) + (level>39);
			af.modifier  = 0;
			af.location  = APPLY_NONE;
			af.bitvector = AFF_SANCTUARY;
			affect_to_char(f->follower, &af);
		}
	}
}

void spell_sleep(byte level, struct char_data *ch,
                 struct char_data *victim, struct obj_data *obj)
{
    struct affected_type af;

    assert(victim);

    if(IS_NPC(victim)) {
        if(number(1,25) < GET_LEVEL(victim))
            return;
    }
    else if (GET_LEVEL(victim) >= (IMO+3) ||GET_POS(victim) == POSITION_SLEEPING)
        return ;

    if(!saves_spell(victim, SAVING_SPELL)) {
        af.type      = SPELL_SLEEP;
        af.duration  = 4+level/10;
        af.modifier  = 0;
        af.location  = APPLY_NONE;
        af.bitvector = AFF_SLEEP;
        affect_join(victim, &af, FALSE, FALSE);
        if (GET_POS(victim)>POSITION_SLEEPING) {
            act("You feel very sleepy ..... zzzzzz",FALSE,victim,0,0,TO_CHAR);
            act("$n go to sleep.",TRUE,victim,0,0,TO_ROOM);
            GET_POS(victim)=POSITION_SLEEPING;
        }
        return;
    }
}

void spell_strength(byte level, struct char_data *ch,
                    struct char_data *victim, struct obj_data *obj)
{
    struct affected_type af;
    assert(victim);

    act("You feel stronger.",FALSE,victim,0,0,TO_CHAR);
    af.type      = SPELL_STRENGTH;
    af.duration  = level;
    af.modifier  = 1+(level>18);
    af.location  = APPLY_STR;
    af.bitvector = 0;
    affect_join(victim, &af, TRUE, FALSE);
}

void spell_ventriloquate(byte level, struct char_data *ch,
                         struct char_data *victim, struct obj_data *obj)
{
    /* Not possible!! No argument! */
}


void spell_phase(byte level, struct char_data *ch,
                 struct char_data *victim, struct obj_data *obj)
{
    /* Not possible!! No argument! */
}

void spell_word_of_recall(byte level, struct char_data *ch,
                          struct char_data *victim, struct obj_data *obj)
{
	extern struct index_data *mob_index;

    extern int top_of_world;
	extern int nokillflag ;
    int loc_nr,location;
    bool found = FALSE;
    char buf[100];

    assert(victim);
    if (IS_NPC(victim) && 
			(mob_index[victim->nr].virtual != 3097 
			 && mob_index[victim->nr].virtual != 3098))
        return;

	if (IS_SET(world[ch->in_room].room_flags, NO_RECALL)) {
        send_to_char("You cannot do that here.\n\r",ch);
        sprintf(buf,"%s attempts to misbehave here.\n\r",ch->player.name);
        send_to_room_except(buf,ch->in_room,ch);
		return;
	}

    /*  loc_nr = GET_HOME(ch); */
	if ( nokillflag == 0 ) {	/* in Death Fight */
		loc_nr = 10052 ;
    }
	else {
		if (GET_LEVEL(ch) > 25) loc_nr=3072;
		else loc_nr = 3078;
    }

    for(location=0;location<=top_of_world;location++)
        if(world[location].number == loc_nr) {
            found=TRUE;
            break;
        }
    if ((location == top_of_world) || !found) {
        send_to_char("You are completely lost.\n\r", victim);
        return;
    }
    /* a location has been found. */
    act("$n disappears.", TRUE, victim, 0, 0, TO_ROOM);
    char_from_room(victim);
    char_to_room(victim, location);
    if (victim->specials.fighting)
        stop_fighting(victim);
    act("$n appears in the middle of the room.", TRUE, victim, 0, 0, TO_ROOM);
    do_look(victim, "",15);
    victim->points.move=victim->points.move - 50;
}
void spell_summon(byte level, struct char_data *ch,
                  struct char_data *victim, struct obj_data *obj)
{
#ifdef DEATHFIGHT
	extern int	deathfight ;
#endif 
    sh_int target;

    assert(ch && victim);

#ifdef DEATHFIGHT
	if ( deathfight != 0 ) {	/* death fight mode (2 round) */
		if ( !IS_NPC(victim) ) {
			send_to_char("You can't summon player. It's DEATH FIGHT !\n\r", ch) ;
			return ;
        }
    }
#endif 

    if(IS_NPC(victim)){
        if(GET_LEVEL(victim) > MIN(20,level+3)) {
            send_to_char("You failed.\n\r",ch);
            return;
        }
        if ( IS_AFFECTED(victim, AFF_NO_SUMMON) ) {
            send_to_char("You are interrupted by magical field.\n\r", ch);
            return ;
		}
    } else {
        if(GET_LEVEL(victim) > GET_LEVEL(ch) + 3){
            send_to_char("You are too humble a soul.\n\r",ch);
            return;
        }
    }
    if(IS_SET(world[victim->in_room].room_flags,NOSUMMON)
       || IS_SET(world[ch->in_room].room_flags,NOSUMMON) ){
        send_to_char("You nearly succeed, but not quite.\n\r",ch);
        return;
    }
    if (IS_NPC(victim) && saves_spell(victim, SAVING_SPELL) ) {
        send_to_char("You failed.\n\r", ch);
        return;
    }
    act("$n disappears suddenly.",TRUE,victim,0,0,TO_ROOM);
    target = ch->in_room;
    char_from_room(victim);
    char_to_room(victim,target);

    if(victim->specials.was_in_room != NOWHERE )
        victim->specials.was_in_room = target ;

    act("$n arrives suddenly.",TRUE,victim,0,0,TO_ROOM);
    act("$n has summoned you!",FALSE,ch,0,victim,TO_VICT);
    do_look(victim,"",COM_LOOK);
}


void spell_charm_person(byte level, struct char_data *ch,
                        struct char_data *victim, struct obj_data *obj)
{
    struct affected_type af;
	struct follow_type *f;
	int num_of_charmie = 0;

    void add_follower(struct char_data *ch, struct char_data *leader);
    bool circle_follow(struct char_data *ch, struct char_data *victim);
    void stop_follower(struct char_data *ch);

    assert(ch && victim);

	for (f = ch->followers; f; f = f->next) {
		if (f->follower != ch && IS_AFFECTED(f->follower, AFF_CHARM)
					&& IS_NPC(f->follower)) {
			num_of_charmie++;
		}
	}

    /* By testing for IS_AFFECTED we avoid ei. Mordenkainens sword to be */
    /* able to be "recharmed" with duration                              */

    if (victim == ch) {
        send_to_char("You like yourself even better!\n\r", ch);
        return;
    }
    if (!IS_AFFECTED(victim, AFF_CHARM) && !IS_AFFECTED(ch, AFF_CHARM) &&
        (level >= GET_LEVEL(victim))) {
        if (circle_follow(victim, ch)) {
            send_to_char("Sorry, following in circles can not be allowed.\n\r", ch);
            return;
        }
        if((level < IMO) && (GET_LEVEL(victim) > 20)){
            send_to_char("You fail.\n\r",ch);
            return;
        }
		if (num_of_charmie >= GET_WIS(ch)/9 + GET_LEVEL(ch)/20) {
            send_to_char("You can't control too many charmie.\n\r",ch);
            return;
		}
		if ( IS_AFFECTED(victim, AFF_NO_CHARM)) {
            send_to_char("Victim resist.\n\r",ch);
			return ;
        }
        if(GET_LEVEL(ch) < (IMO+3))
            if (saves_spell(victim, SAVING_PARA))
                return;
        if (victim->master)
            stop_follower(victim);
        add_follower(victim, ch);
        af.type = SPELL_CHARM_PERSON;
        if (GET_INT(victim))
            af.duration  = 24*18/GET_INT(victim);
        else
            af.duration  = 24*18;
        af.modifier  = 0;
        af.location  = 0;
        af.bitvector = AFF_CHARM;
        affect_to_char(victim, &af);
        act("Isn't $n just such a nice fellow?",FALSE,ch,0,victim,TO_VICT);
    }
}

void spell_preach(byte level, struct char_data *ch,
                  struct char_data *victim, struct obj_data *obj)
{
    assert(ch && victim);

    if( !IS_NPC(victim) ) return;
    if( GET_LEVEL(victim)>=IMO ) {
		send_to_char("Your spell is not powerful enough for that.\n\r",ch);	
		return;
	}
    if(level+number(1,15) < GET_LEVEL(victim)) {
		send_to_char("You fail, but not by much.\n\r",ch);
		return;
	}
    if (IS_SET(victim->specials.act,ACT_AGGRESSIVE) &&
        saves_spell(victim, SAVING_PARA)){
        REMOVE_BIT(victim->specials.act,ACT_AGGRESSIVE);
        act("$n looks less aggressive.",TRUE,victim,0,0,TO_ROOM);
    }
    return;
}

void spell_sense_life(byte level, struct char_data *ch,
                      struct char_data *victim, struct obj_data *obj)
{
    struct affected_type af;

    assert(victim);

    if (!affected_by_spell(victim, SPELL_SENSE_LIFE)) {
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
	int	monster ;
	char	*string_dup(char *s) ;
	char	buf[BUFSIZ] ;

    if(obj->obj_flags.value[3] != 1){
        send_to_char("There do not appear to be any corpses hereabouts?\n\r",ch);
        return;
    }
    if(dice(9,9) > ch->skills[SPELL_REANIMATE].learned){
        send_to_char("The spell fails miserably.\n\r",ch);
        return;
    }

	if (GET_ITEM_TYPE(obj) == ITEM_CONTAINER)
		monster = 0 ;
	else
		monster = 1 ;
	extract_obj(obj);
	mob=read_mobile(2, VIRTUAL);
	if ( monster ) {
		free_string_type(mob->player.name) ;
		free_string_type(mob->player.short_descr) ;
		free_string_type(mob->player.long_descr) ;
		sprintf(buf, "%s monster", obj->name) ;
		mob->player.name = string_dup(buf) ;
		sprintf(buf, "a tiny %s monster", fname(obj->name)) ;
		mob->player.short_descr = string_dup(buf) ;
		sprintf(buf, "A tiny %s monster is standing here.\n\r", fname(obj->name));
		mob->player.long_descr = string_dup(buf) ;
		act("$n has created a strange monster!",TRUE,ch,0,0,TO_ROOM);
		send_to_char("You have created a monster.\n\r",ch);
		char_to_room(mob,ch->in_room);
		add_follower(mob,ch);
		mob->points.max_hit += dice(9,GET_LEVEL(ch)) ;
		mob->points.hit = mob->points.max_hit ;
		mob->points.max_move = 4 ;
    }
	else {
		char_to_room(mob,ch->in_room);
		act("$n has created a zombie!",TRUE,ch,0,0,TO_ROOM);
		send_to_char("You have created a zombie.\n\r",ch);
		add_follower(mob,ch);
		mob->points.max_hit+=dice(6,GET_LEVEL(ch));
		mob->points.hit=mob->points.max_hit;
		mob->points.max_move=4;
    }
}
void spell_clone(byte level, struct char_data *ch,
                 struct char_data *victim, struct obj_data *obj)
{
    struct char_data *mob; 
    struct char_data *read_mobile(int nr, int type);
    char buf[256];

    if(obj){
        send_to_char("Cloning object is not YET possible.\n\r",ch);
        return;
    }
    if(!victim){
        send_to_char("Clone who?\n\r",ch);
        return;
    }
    if(dice(5,25) > ch->skills[SPELL_CLONE].learned){
        send_to_char("You fail, but not by much.\n\r",ch);
        return;
    }
    if(IS_NPC(victim) &&
       mob_index[victim->nr].number < (GET_LEVEL(ch)-GET_LEVEL(victim))/3 ){
        mob=read_mobile(victim->nr, REAL);
        GET_EXP(mob)=0;
        GET_GOLD(mob)=0;
        char_to_room(mob,ch->in_room);
        sprintf(buf,"%s has been cloned!\n\r",
                victim->player.short_descr);
        send_to_room(buf,ch->in_room);
    } else {
        send_to_char("You may not clone THAT!\n\r",ch);
    }
}

