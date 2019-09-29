/* ************************************************************************
 *  file: magic.c , Implementation of spells.              Part of DIKUMUD *
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
#include "magic_weapon.h"
#include "quest.h"

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


/* Offensive Spells */

void spell_magic_missile(byte level, struct char_data *ch,
                         struct char_data *victim, struct obj_data *obj)
{
    int dam;
    static int dam_each[] = 
        {0,  8,8,9,9,9, 10,10,10,10,10, 10,10,10,10,10, 10,10,10,10,10,
         10,10,10,10,10, 10,10,10,10,10, 10,10,10,10,10, 10,10,10,10,10,
         10,10,10,10,10, 10,10,10,10,10, 10,10,10,10,10};

    assert(victim && ch);
    assert((level >= 1) && (level <= 55)); 

    dam = number(dam_each[level]>>1, dam_each[level]<<1);

    if ( saves_spell(victim, SAVING_SPELL) )
        dam >>= 1;

    damage(ch, victim, dam, SPELL_MAGIC_MISSILE);
}



void spell_chill_touch(byte level, struct char_data *ch,
                       struct char_data *victim, struct obj_data *obj)
{
    struct affected_type af;
    int dam;
    static int dam_each[] = 
        {5,  5,5,15,20,20, 25,25,25,25,25, 25,25,25,25,25, 25,25,25,25,25,
         25,25,25,25,25, 25,25,25,25,25, 25,25,25,25,25, 25,25,25,25,25,
         25,25,25,25,25, 25,25,25,25,25, 25,25,25,25,25};

    assert(victim && ch);
    assert((level >= 3) && (level <= 55)); 

    dam = number(dam_each[level]>>1, dam_each[level]<<1);

    if ( !saves_spell(victim, SAVING_SPELL) )
    {
        af.type      = SPELL_CHILL_TOUCH;
        af.duration  = 6;
        af.modifier  = -1;
        af.location  = APPLY_STR;
        af.bitvector = 0;
        affect_join(victim, &af, TRUE, FALSE);
    } else {
        dam >>= 1;
    }
    damage(ch, victim, dam, SPELL_CHILL_TOUCH);
}



void spell_burning_hands(byte level, struct char_data *ch,
                         struct char_data *victim, struct obj_data *obj)
{
    int dam;
    static int dam_each[] = 
        {0,  0,0,0,0,40, 40,40,40,40,40, 40,40,40,40,40, 40,40,40,40,40,
         40,40,40,40,40, 40,40,40,40,40, 40,40,40,40,40, 40,40,40,40,40,
         40,40,40,40,40, 40,40,40,40,40, 40,40,40,40,40};


    assert(victim && ch);
    assert((level >= 5) && (level <= 55));

    dam = number(dam_each[level]>>1, dam_each[level]<<1);

    if ( saves_spell(victim, SAVING_SPELL) )
        dam >>= 1;

    damage(ch, victim, dam, SPELL_BURNING_HANDS);
}



void spell_shocking_grasp(byte level, struct char_data *ch,
                          struct char_data *victim, struct obj_data *obj)
{
    int dam;
    static int dam_each[] = 
        {0,  0,0,0,0,0, 0,41,40,40,40, 40,40,40,40,40, 40,40,40,40,40,
         40,40,40,40,40, 40,40,40,40,40, 40,40,40,40,40, 40,40,40,40,40,
         40,40,40,40,40, 40,40,40,40,40, 40,40,40,40,40};

    assert(victim && ch);
    assert((level >= 7) && (level <= 55)); 

    dam = number(dam_each[level]>>1, dam_each[level]<<1);

    if ( saves_spell(victim, SAVING_SPELL) ) 
        dam >>= 1;
    
    if(!saves_spell(victim, SAVING_PARA))
        GET_POS(victim) = POSITION_STUNNED;

    damage(ch, victim, dam, SPELL_SHOCKING_GRASP);
}



void spell_lightning_bolt(byte level, struct char_data *ch,
                          struct char_data *victim, struct obj_data *obj)
{
    int dam;
    static int dam_each[] = 
        {0,  0,0,0,0,0, 0,0,0,70,70, 70,70,70,70,70, 70,70,70,70,70,
         70,70,70,70,70, 70,70,70,70,70, 70,70,70,70,70, 70,70,70,70,70,
         70,70,70,70,70, 70,70,70,70,70, 70,70,70,70,70};

    assert(victim && ch);
    assert((level >= 9) && (level <= 55)); 

    dam = number(dam_each[level]>>1, dam_each[level]<<1);

    if ( saves_spell(victim, SAVING_SPELL) )
        dam >>= 1;

    damage(ch, victim, dam, SPELL_LIGHTNING_BOLT);
}



void spell_colour_spray(byte level, struct char_data *ch,
                        struct char_data *victim, struct obj_data *obj)
{
    int dam;
    static int dam_each[] = 
        {0,  0,0,0,0,0, 0,0,0,0,0, 83,83,83,83,83, 83,83,83,83,83,
         83,83,83,83,83, 83,83,83,83,83, 83,83,83,83,83, 83,83,83,83,83,
         83,83,83,83,83, 83,83,83,83,83, 83,83,83,83,83};

    assert(victim && ch);
    assert((level >= 11) && (level <= 55));
    dam = number(dam_each[level]-20, dam_each[level]+20);
    if ( saves_spell(victim, SAVING_SPELL) )
        dam >>= 1;
    damage(ch, victim, dam, SPELL_COLOUR_SPRAY);
}


/* Drain XP, MANA, HP - caster gains HP and MANA */
void spell_energy_drain(byte level, struct char_data *ch,
                        struct char_data *victim, struct obj_data *obj)
{
	extern int	nokillflag ;
    int dam, xp, mana, fourth;

    void set_title(struct char_data *ch);
    void gain_exp(struct char_data *ch, int gain);

    assert(victim && ch);
    assert((level >= 13) && (level <=  55));

	if (nokillflag && !IS_NPC(victim))
		return ;

    if ( !saves_spell(victim, SAVING_SPELL) ) {
		GET_ALIGNMENT(ch) = MAX(-1000, GET_ALIGNMENT(ch)-170);
		if ( GET_ALIGNMENT(ch) > 0 )
			GET_ALIGNMENT(ch) = 0 ;
        if (GET_LEVEL(victim) <= 2) {
            damage(ch, victim, 100, SPELL_ENERGY_DRAIN); /* Kill the sucker */
        } else {
            xp = number(level>>1,level)*1000;
            fourth = xp / 4 ;
            gain_exp(victim, -xp);
            gain_exp(ch, fourth) ;
            dam = dice(1,GET_LEVEL(ch));
            mana = GET_MANA(victim)>>1;
            GET_MOVE(victim) >>= 1;
            GET_MANA(victim) = mana;
            GET_MANA(ch) += mana>>1;
            GET_HIT(ch) += dam;
            send_to_char("Your life energy is drained!\n\r", victim);
            damage(ch, victim, dam, SPELL_ENERGY_DRAIN);
        }
    } else {
        damage(ch, victim, 0, SPELL_ENERGY_DRAIN); /* Miss */
    }
}

void spell_fireball(byte level, struct char_data *ch,
                    struct char_data *victim, struct obj_data *obj)
{
    int dam;
    extern struct weather_data weather_info;
    static int dam_each[] = 
        {0,  0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,143, 120,120,120,120,120,
         120,120,120,120,120, 120,120,120,120,120, 120,120,120,120,120, 120,120,120,120,120,
         120,120,120,120,120, 120,120,120,120,120, 120,120,120,120,120};

    assert(victim && ch);
    assert((level >= 15) && (level <= 55)); 
    dam = number(dam_each[level]-20, dam_each[level]+20);
    if ( saves_spell(victim, SAVING_SPELL) )
        dam >>= 1;
    if (OUTSIDE(ch) && (weather_info.sky==SKY_CLOUDLESS)) {
        send_to_char("Your fireball works excellent!!\n\r", ch);
        act("$n suffers from hot ball.", FALSE, victim, 0,0,TO_ROOM);
        dam += dice(5,4) ;
    }
    damage(ch, victim, dam, SPELL_FIREBALL);
}

void spell_disintegrate(byte level, struct char_data *ch,
                        struct char_data *victim, struct obj_data *obj)
{
    int dam;

    assert(victim && ch);
    assert((level >= 15) && (level <= 55)); 
    dam = GET_LEVEL(ch) * GET_LEVEL(victim);
    if ( saves_spell(victim, SAVING_SPELL) ) {
        dam = 0;
		send_to_char("You Couldn't concentrate enough\n\r", ch);
	}
    if ( IS_AFFECTED(victim, AFF_SANCTUARY) )
        dam <<= 1;
    damage(ch, victim, dam, SPELL_DISINTEGRATE);
}

void spell_corn_of_ice(byte level, struct char_data *ch,
                       struct char_data *victim, struct obj_data *obj)
{
    int dam;
    static int dam_each[] = 
        {0,  0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,143, 120,120,120,120,120,
         300,300,300,300,300, 300,300,300,300,300, 300,300,300,300,300, 300,300,300,300,300,
         300,300,300,300,300, 300,300,300,300,300, 300,300,300,300,300};

    assert(victim && ch);
    assert((level >= 15) && (level <= 55)); 
    dam = number(dam_each[level]-20, dam_each[level]+20);
    if ( saves_spell(victim, SAVING_SPELL) )
        dam >>= 1;
    damage(ch, victim, dam, SPELL_CORN_OF_ICE);
}

void spell_sunburst(byte level, struct char_data *ch,
                    struct char_data *victim, struct obj_data *obj)
{
    int dam;
    void spell_blindness() ;
    static int dam_each[] = 
        {0,  0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0,
         0,180,180,180,180, 180,185,180,180,180, 180,180,180,180,180,
         180,180,180,180,180, 180,180,180,180,180,
         180,180,180,180,180, 180,180,180,180,180
        };

    assert(victim);
	assert(ch);
    assert((level >= 15) && (level <= 55)); 
    dam = number(dam_each[level]-20, dam_each[level]+20);
    if ( saves_spell(victim, SAVING_SPELL) )
        dam >>= 1;
    else if ( number(1,15) == 1 ||  dice(2, 9) > GET_LEVEL(victim)-level+10)
        spell_blindness(level-3, ch, victim, 0) ;
    damage(ch, victim, dam, SPELL_SUNBURST);
}

void spell_energyflow(byte level, struct char_data *ch,
                      struct char_data *victim, struct obj_data *obj)
{
    int dam=0;
    assert(victim && ch);
    assert((level > 0) && (level < 55)); 
    if(GET_EXP(ch)>1000000)
    {
        dam=number(level,3)+200+level*10;
        send_to_char("Energyflow which is made of your Exp Quaaaaaaaaaaaaaaaaaa!\n\r", ch);
        if(ch->points.exp>50000) ch->points.exp-=50000;
        else ch->points.exp=0;
        if ( saves_spell(victim, SAVING_SPELL) )
            dam >>= 1;
        damage(ch, victim, dam, SPELL_ENERGY_FLOW);
    }
    else send_to_char("You have too little experience\n\r",ch);
}

void spell_full_fire(byte level, struct char_data *ch,
                     struct char_data *victim, struct obj_data *obj)
{  
	int dam=0,i;
	int class;

	if (is_solved_quest(ch, QUEST_REMORTAL)) {
		class = GET_CLASS_O(ch);
	}
	else {
		class = GET_CLASS(ch);
	}

 	assert(victim && ch);
 	assert((level > 0) && (level < 55)); 

 	for(i=1;i<GET_MANA(ch);i+=(IMO+20-level)){
     	dam += dice(level,6);
 	}
 	if ( saves_spell(victim, SAVING_SPELL) ) {
     	dam=GET_MANA(ch);
     	i=0;
     	send_to_char("But, You couldn't concentrate enough\n\r",ch);
 	}
	if (class != CLASS_MAGIC_USER) {
		// 원 class가 magic user가 아닌 경우 
		//   	damage를 3/5 - 4/5으로.
		dam = dam * number(3,4) / 5;
	}
 	damage(ch, victim, dam, SPELL_FULL_FIRE);
 	if(i) GET_MANA(ch)=0;
}

void spell_throw(byte level, struct char_data *ch,
                 struct char_data *victim, struct obj_data *obj)
{  
	int dam=0;
	int class;

 	assert(victim && ch);
 	assert((level > 0) && (level < 55)); 

	if (is_solved_quest(ch, QUEST_REMORTAL)) {
		class = GET_CLASS_O(ch);
	}
	else {
		class = GET_CLASS(ch);
	}

 	if(GET_HIT(ch)>10) dam = dice(level,3);
 	GET_HIT(ch)-=GET_HIT(ch)/2;
 	if ( saves_spell(victim, SAVING_SPELL) ) {
     	dam=dice(2,level);
     	send_to_char("You Couldn't concentrate enough\n\r", ch);
 	}
	if (class != CLASS_MAGIC_USER) {
		// 원 class가 magic user가 아닌 경우 
		//   	damage를 3/5 - 4/5으로.
		dam = dam * number(3,4) / 5;
	}
 	damage(ch, victim, dam*(GET_HIT(ch)/(IMO+25-level)), SPELL_THROW);
 	send_to_char("Light Solar  guooooooooroorooorooorooooaaaaaaaaaaa!\n\r", ch);
}

void spell_earthquake(byte level, struct char_data *ch,
                      struct char_data *victim, struct obj_data *obj)
{
    int dam;
    struct char_data *tmp_victim, *temp;

    assert(ch);
    assert((level >= 7) && (level <= 55)); 

    dam =  dice(1,8)+level;
    send_to_char("The earth trembles beneath your feet!\n\r", ch);
    act("$n makes the earth tremble and shiver\n\rYou fall, and hit yourself!",
        FALSE, ch, 0, 0, TO_ROOM);

    for(tmp_victim = character_list; tmp_victim; tmp_victim = temp) {
        temp = tmp_victim->next;
        if ( (ch->in_room == tmp_victim->in_room) && (ch != tmp_victim) ) {
            damage(ch, tmp_victim, dam, SPELL_EARTHQUAKE);
        } else
            if (world[ch->in_room].zone == world[tmp_victim->in_room].zone)
                send_to_char("The earth trembles and shivers.\n\r", tmp_victim);
    }

}

void spell_all_heal(byte level, struct char_data *ch,
                    struct char_data *victim, struct obj_data *obj)
{
    struct char_data *tmp_victim, *temp;
    void spell_heal(byte level, struct char_data *ch,
                    struct char_data *victim, struct obj_data *obj);

    assert(ch);
    assert((level >= 7) && (level <= 55)); 

    send_to_char("You feel some saintness!\n\r", ch);
    act("$n is glowing with saintly aura!", FALSE, ch, 0, 0, TO_ROOM);

	GET_ALIGNMENT(ch) = MIN(+1000, GET_ALIGNMENT(ch)+67);

    for(tmp_victim = character_list; tmp_victim; tmp_victim = temp) {
        temp = tmp_victim->next;
        if ( (ch->in_room == tmp_victim->in_room) && (!IS_NPC(ch)) ) {
            spell_heal(level, ch, tmp_victim, obj);
        } else
            if (world[ch->in_room].zone == world[tmp_victim->in_room].zone)
                send_to_char("You see some aura which make you comfort.\n\r", tmp_victim);
    }
}

void spell_hand_of_god(byte level, struct char_data *ch,
                       struct char_data *victim, struct obj_data *obj)
{
    int dam;
    struct char_data *tmp_victim, *temp;

    assert(ch);
    assert((level >=15) && (level <= 55)); 

    dam =  dice(level,8);
    send_to_char("Thousand hands are filling all your sight.\n\r", ch);
    act("$n summoned unnumerable hands.\n\rYour face is slapped by hands. BLOOD ALL OVER!\n\r"
        ,FALSE, ch, 0, 0, TO_ROOM);
    for(tmp_victim = character_list; tmp_victim; tmp_victim = temp) {
        temp = tmp_victim->next;
        if ( (ch->in_room == tmp_victim->in_room) && (ch != tmp_victim) ) {
            damage(ch, tmp_victim, dam, SPELL_HAND_OF_GOD);
        } else
            if (world[ch->in_room].zone == world[tmp_victim->in_room].zone)
                send_to_char("Dirty hands with long nail is going and coming all over sky.\n\r", tmp_victim);
    }
}

void spell_fire_storm(byte level, struct char_data *ch,
                      struct char_data *victim, struct obj_data *obj)
{
    int dam;
    struct char_data *tmp_victim, *temp;

    assert(ch);
    assert((level >=15) && (level <= 55)); 

    dam =  dice(level,4);
    send_to_char("The fire storm is flowing in the air!\n\r", ch);
    act("$n makes the fire storm flowing in the air.\n\rYou can't see anything!\n\r"
        ,FALSE, ch, 0, 0, TO_ROOM);
    for(tmp_victim = character_list; tmp_victim; tmp_victim = temp) {
        temp = tmp_victim->next;
        if ( (ch->in_room == tmp_victim->in_room) && (ch != tmp_victim) ) {
            damage(ch, tmp_victim, dam, SPELL_FIRE_STORM);
        } else
            if (world[ch->in_room].zone == world[tmp_victim->in_room].zone)
                send_to_char("The fire storm is flowing in the atmosphere.\n\r", tmp_victim);
    }
}
void spell_trick(byte level, struct char_data *ch,
                 struct char_data *victim, struct obj_data *obj)
{
    int dam;
    struct char_data *tmp_victim, *temp;

    assert(ch);
    assert((level >=25) && (level <= 55)); 
    if(IS_NPC(victim)){
        tmp_victim=world[victim->in_room].people;
        if(tmp_victim){
            while(!IS_NPC(tmp_victim) && tmp_victim)
                tmp_victim=tmp_victim->next_in_room;	
            if (tmp_victim == NULL) {
                send_to_char("no other monster !\n\r", ch) ;
                return ;
			}
            if (ch->skills[SPELL_TRICK].learned-95+level-GET_LEVEL(victim) +
                number(1,10)-number(1,25)>0) {
                hit(victim,tmp_victim,TYPE_UNDEFINED);
            } 
        }
    }
    else send_to_char("only monster !\n\r",ch);
}

void spell_dispel_evil(byte level, struct char_data *ch,
                       struct char_data *victim, struct obj_data *obj)
{
    int dam;

    assert(ch && victim);
    assert((level >= 10) && (level<=55));

    if (IS_EVIL(ch))
        victim = ch;
    else
        if (IS_GOOD(victim)) {
            act("God protects $N.", FALSE, ch, 0, victim, TO_CHAR);
            return;
        }

    if ((GET_LEVEL(victim) < level) || (victim == ch))
        dam = 100;
    else {
        dam = dice(level,4);

        if ( saves_spell(victim, SAVING_SPELL) )
            dam >>= 1;
    }

    damage(ch, victim, dam, SPELL_DISPEL_EVIL);
}



void spell_call_lightning(byte level, struct char_data *ch,
                          struct char_data *victim, struct obj_data *obj)
{
    int dam;

    extern struct weather_data weather_info;
    assert(victim && ch);
    assert((level >= 12) && (level <= 55));
    dam = dice(level, 10);
    if (OUTSIDE(ch) && (weather_info.sky>=SKY_RAINING)) {
        damage(ch, victim, dam, SPELL_CALL_LIGHTNING);
    }
}

void spell_harm(byte level, struct char_data *ch,
                struct char_data *victim, struct obj_data *obj)
{
    int dam;

    assert(victim && ch);
    assert((level >= 15) && (level <= 55));
    dam = GET_HIT(victim) - dice(1,4);
    dam = MIN(100, dam);

    if (dam < 0)
        dam = 0; /* Kill the suffering bastard */
    else {
        if ( saves_spell(victim, SAVING_SPELL) )
            dam >>= 1;
    }
    damage(ch, victim, dam, SPELL_HARM);
}

/* ***************************************************************************
 *                     Not cast-able spells                                  *
 * ************************************************************************* */

void spell_identify(byte level, struct char_data *ch,
                    struct char_data *victim, struct obj_data *obj)
{
    int ac_applicable(struct obj_data *obj_object) ;
    char buf[256], buf2[256], bufh[256];
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
    extern char *wear_bits[];
    
    assert(ch && (obj || victim));

    if (obj) {
        send_to_char_han("You feel informed:\n\r",
                         "이런 정보를 알 수 있습니다:\n\r", ch);

        sprintf(buf, "Object '%s', Item type: ", obj->name);
        sprintf(bufh, "물건 '%s', 종류: ", obj->name);
        sprinttype(GET_ITEM_TYPE(obj),item_types,buf2);
        strcat(buf,buf2); 
        strcat(bufh,buf2);
        strcat(buf,"\n\r");
        strcat(bufh,"\n\r");

        strcat(buf,"Can be worn on : ");
        strcat(bufh,"입을 수 있는 곳 : ");
        sprintbit(obj->obj_flags.wear_flags, wear_bits, buf2);
        strcat(buf,buf2); strcat(bufh,buf2);
        strcat(buf,"\n\r");
        strcat(bufh,"\n\r");
        
        send_to_char_han(buf, bufh, ch);

        if (obj->obj_flags.bitvector) {
            send_to_char_han("Item will give you following abilities:  ",
                             "이 물건은 다음과 같은 능력을 줍니다:  ",  ch);
            sprintbit(obj->obj_flags.bitvector,affected_bits,buf);
            strcat(buf,"\n\r");
            send_to_char(buf, ch);
        }

        send_to_char_han("Item is: ", "성질: ", ch);
        sprintbit(GET_OBJ_STAT(obj), extra_bits, buf);
        strcat(buf,"\n\r");
        send_to_char(buf,ch);

        sprintf(buf,"Weight: %d, Value: %d\n\r",
                obj->obj_flags.weight, obj->obj_flags.cost);
        sprintf(bufh,"무게: %d, 값: %d\n\r",
                obj->obj_flags.weight, obj->obj_flags.cost);
        send_to_char_han(buf, bufh, ch);

        switch (GET_ITEM_TYPE(obj)) {
		case ITEM_LIGHT :
			if ( obj->obj_flags.value[2] >= 0 ) {
				sprintf(buf, "Life time : %d \n\r",  obj->obj_flags.value[2]);
				sprintf(bufh, "수명 : %d \n\r",  obj->obj_flags.value[2]);
            }
			else {
				sprintf(buf, "Life time : unlimited\n\r");
				sprintf(bufh, "수명 : 무제한 \n\r");
            }
			send_to_char_han(buf, bufh, ch);
			break ;
        case ITEM_SCROLL : 
        case ITEM_POTION :
            sprintf(buf, "Level %d spells of:\n\r",  obj->obj_flags.value[0]);
            sprintf(bufh, "%d 레벨에 해당하는 마법:\n\r",  obj->obj_flags.value[0]);
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

            sprintf(buf, "Level %d spell of:\n\r",  obj->obj_flags.value[0]);
            sprintf(bufh, "%d 레벨에 해당하는 마법:\n\r",  obj->obj_flags.value[0]);
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
            break;

        case ITEM_ARMOR :
            if (ac_applicable(obj)) {
                sprintf(buf, "AC-apply is %d\n\r", obj->obj_flags.value[0]);
                sprintf(bufh, "무장: %d\n\r", obj->obj_flags.value[0]);
                send_to_char_han(buf, bufh, ch);
            }
        }

        found = FALSE;

        for (i=0;i<MAX_OBJ_AFFECT;i++) {
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
                sprintf(bufh,"    기능 : %d 만큼의 %s\n\r", obj->affected[i].modifier,
                        buf2);
                send_to_char_han(buf, bufh, ch);
            }
        }

    } else {       /* victim */
		sprintf(buf,"Level : %d, Hit: %d/%d, Mana: %d/%d, Move: %d/%d\n\r",
                victim->player.level,
                GET_HIT(victim), hit_limit(victim),
                GET_MANA(victim), mana_limit(victim),
                GET_MOVE(victim), move_limit(victim));
		send_to_char(buf,ch);
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
				GET_HEIGHT(victim), 0.45*GET_WEIGHT(victim));
		send_to_char_han(buf, bufh, ch);

        if (!IS_NPC(victim)) {
            if(GET_LEVEL(victim) > 5) {
                sprintf(buf,"Str %d/%d,  Int %d,  Wis %d,  Dex %d,  Con %d\n\r",
                        GET_STR(victim), GET_ADD(victim),
                        GET_INT(victim),
                        GET_WIS(victim),
                        GET_DEX(victim),
                        GET_CON(victim) );
                send_to_char(buf,ch);
            }
        } else {
			if (number(0,41) <= level) {
				/* mob specific */
				sprintf(buf,"HandDice:%dd%d,Hr:%d,Dr:%d,%s,%s.\n\r",
                        victim->specials.damnodice, victim->specials.damsizedice,
                        GET_HITROLL(victim), GET_DAMROLL(victim),
                        (IS_SET(victim->specials.act,ACT_AGGRESSIVE) ?
                         "AGGR" : "NO AGGR"),
                        IS_AFFECTED((victim),AFF_DETECT_INVISIBLE) ? 
						"DET-INV" : "NO DET-INV");
				send_to_char(buf,ch);
			}
        }
    }
}

int ac_applicable(struct obj_data *obj_object)
{
    return ( CAN_WEAR(obj_object,ITEM_WEAR_KNEE)
             || CAN_WEAR(obj_object,ITEM_WEAR_ABOUTLEGS)
             || CAN_WEAR(obj_object,ITEM_WEAR_SHIELD)
             || CAN_WEAR(obj_object,ITEM_WEAR_WRIST)
             || CAN_WEAR(obj_object,ITEM_WEAR_WAISTE)
             || CAN_WEAR(obj_object,ITEM_WEAR_ARMS)
             || CAN_WEAR(obj_object,ITEM_WEAR_HANDS)
             || CAN_WEAR(obj_object,ITEM_WEAR_FEET)
             || CAN_WEAR(obj_object,ITEM_WEAR_LEGS)
             || CAN_WEAR(obj_object,ITEM_WEAR_ABOUT)
             || CAN_WEAR(obj_object,ITEM_WEAR_HEAD)
             || CAN_WEAR(obj_object,ITEM_WEAR_BODY) ) ;
}

/* ***************************************************************************
 *                     NPC spells..                                          *
 * ************************************************************************* */

void spell_fire_breath(byte level, struct char_data *ch,
                       struct char_data *victim, struct obj_data *obj)
{
    int dam;
    int hpch;
    int has_slayer = 0;
    struct obj_data *burn;
    struct obj_data *weapon;

    assert(victim && ch);
    assert(level >= 1); 
    dam = 8*GET_LEVEL(ch);
    if ( saves_spell(victim, SAVING_BREATH) )
        dam >>= 1;

	weapon = victim->equipment[WIELD];
	if (weapon && weapon->obj_flags.gpd > 0 && 
        weapon->obj_flags.value[0]  ==  WEAPON_DRAGON_SLAYER && 
        IS_OBJ_STAT( weapon, ITEM_MAGIC ) )
	{
        has_slayer = 1;
        
		act("HOLY LIGHT from $p protects $n from dragon's breath",
            TRUE,victim,weapon,ch,TO_ROOM);
		act("HOLY LIGHT from $p protects you from dragon's breath",
            TRUE,victim,weapon,ch,TO_CHAR);
		dam >>= 1;
	}
    damage(ch, victim, dam, SPELL_FIRE_BREATH);

    /* And now for the damage on inventory */

    if(number(0,IMO)<GET_LEVEL(ch)) {
        if (!saves_spell(victim, SAVING_BREATH)) {
            for(burn=victim->carrying ; burn;
                burn=burn->next_content) {
                if ((burn->obj_flags.type_flag==ITEM_SCROLL) && number(0,1)>=1)
					continue;
                if ((burn->obj_flags.type_flag==ITEM_WAND) && number(0,4)>=3)
					continue;
                if ((burn->obj_flags.type_flag==ITEM_STAFF) && number(0,5)>=3)
					continue;
                if ((burn->obj_flags.type_flag==ITEM_NOTE) && number(0,1)>=1) 
					continue;

				if (!(burn->obj_flags.type_flag == ITEM_SCROLL ||
                      burn->obj_flags.type_flag == ITEM_WAND ||
                      burn->obj_flags.type_flag == ITEM_STAFF ||
                      burn->obj_flags.type_flag == ITEM_NOTE)) continue;

                // dragon slayer를 가진 경우에는 1/4 확률로 깨지는 거 방지
                if (has_slayer && number(1,4) == 1) continue;
                
				act("$o burns",0,victim,burn,0,TO_CHAR);
				extract_obj(burn);
            }
        }
    }
}


void spell_frost_breath(byte level, struct char_data *ch,
                        struct char_data *victim, struct obj_data *obj)
{
    int dam;
    int hpch;
    int has_slayer = 0;
    struct obj_data *frozen, *weapon;

    assert(victim && ch);
    assert(level >= 1); 

    dam = 7*GET_LEVEL(ch);

    if ( saves_spell(victim, SAVING_BREATH) )
        dam >>= 1;

	weapon = victim->equipment[WIELD];
	if (weapon && weapon->obj_flags.gpd > 0 && 
        weapon->obj_flags.value[0]  ==  WEAPON_DRAGON_SLAYER && 
        IS_OBJ_STAT( weapon, ITEM_MAGIC ) )
	{
        has_slayer = 1;
		act("HOLY LIGHT from $p protects $n from dragon's breath",
            TRUE,victim,weapon,ch,TO_ROOM);
		act("HOLY LIGHT from $p protects you from dragon's breath",
            TRUE,victim,weapon,ch,TO_CHAR);
		dam >>= 1;
	}
    damage(ch, victim, dam, SPELL_FROST_BREATH);

    /* And now for the damage on inventory */

    if(number(0,30)<GET_LEVEL(ch)) {
        if (!saves_spell(victim, SAVING_BREATH) ) {
            for(frozen=victim->carrying ; frozen ;
                frozen=frozen->next_content) {
				if (number(0,6) == 5) continue;
                if((frozen->obj_flags.type_flag==ITEM_DRINKCON) || 
                   (frozen->obj_flags.type_flag==ITEM_FOOD) ||
                   (frozen->obj_flags.type_flag==ITEM_POTION) ) {

                    // dragon slayer를 가진 경우에 1/6 확률로 깨지는 거 방지
                    if (has_slayer && number(1,6) == 1) continue;

                    act("$o breaks.",0,victim,frozen,0,TO_CHAR);
                    extract_obj(frozen);
                }
            }
        }
    }
}

void spell_gas_breath(byte level, struct char_data *ch,
                      struct char_data *victim, struct obj_data *obj)
{
    int dam;
    int hpch;
    int has_slayer = 0;
    struct obj_data *melt, *temp, *next_obj, *weapon;
    void cast_poison( byte level, struct char_data *ch, char *arg, int type,
                      struct char_data *tar_ch, struct obj_data *tar_obj );


    assert(victim && ch);
    assert(level >= 1); 

    dam = 6*GET_LEVEL(ch);

    if ( saves_spell(victim, SAVING_BREATH) )
        dam >>= 1;
	else {
		if (ch->specials.fighting) {
        	cast_poison( GET_LEVEL(ch), ch, "", SPELL_TYPE_SPELL,
           	          ch->specials.fighting, 0);
		}
	}

	weapon = victim->equipment[WIELD];
	if (weapon && weapon->obj_flags.gpd > 0 && 
        weapon->obj_flags.value[0]  ==  WEAPON_DRAGON_SLAYER && 
        IS_OBJ_STAT( weapon, ITEM_MAGIC ) )
	{
        has_slayer = 1;
        
		act("HOLY LIGHT from $p protects $n from dragon's breath",
            TRUE,victim,weapon,ch,TO_ROOM);
		act("HOLY LIGHT from $p protects you from dragon's breath",
            TRUE,victim,weapon,ch,TO_CHAR);

        dam >>= 1;
	}

    damage(ch, victim, dam, SPELL_GAS_BREATH);
    /* And now for the damage on inventory */

    if(number(15,IMO+20)<GET_LEVEL(ch)) {
        if (!saves_spell(victim, SAVING_BREATH) ) {
            melt = victim->carrying ;
            if(melt && (number(0,3) < 3)) {
                act("$o is melting away.",0,victim,melt,0,TO_CHAR);
                if (melt->obj_flags.type_flag==ITEM_CONTAINER) {

                    act("Some trash dumped out as $o is broken!",0,victim,melt,0,TO_ROOM);
                    for(temp = melt->contains; temp; temp=next_obj) {
                        next_obj = temp->next_content;
                        obj_from_obj(temp);
                        obj_to_room(temp,victim->in_room);
                    }
                }
                extract_obj(melt);
            }
        }
    }
}

void spell_lightning_breath(byte level, struct char_data *ch,
                            struct char_data *victim, struct obj_data *obj)
{
    int dam;
    int hpch;
    struct obj_data *explode, *weapon;

    assert(victim && ch);
    assert(level >= 1); 

    dam = 5*GET_LEVEL(ch);

    if ( saves_spell(victim, SAVING_BREATH) )
        dam >>= 1;

	weapon = victim->equipment[WIELD];
	if (weapon && weapon->obj_flags.gpd > 0 && 
        weapon->obj_flags.value[0]  ==  WEAPON_DRAGON_SLAYER && 
        IS_OBJ_STAT( weapon, ITEM_MAGIC ) )
	{
		act("HOLY LIGHT from $p protects $n from dragon's breath",
            TRUE,victim,weapon,0,TO_ROOM);
		act("HOLY LIGHT from $p protects you from dragon's breath",
            TRUE,victim,weapon,0,TO_CHAR);
		dam >>= 1;
	}

    damage(ch, victim, dam, SPELL_LIGHTNING_BREATH);

    /*  cyb : add damage on inventory  */
    if(number(0,IMO)<GET_LEVEL(ch)) {
        if (!saves_spell(victim, SAVING_BREATH) ) {
            for(explode=victim->carrying ; explode ;
                explode=explode->next_content) {
				if (number(0,6) == 5) continue;
                if((explode->obj_flags.type_flag==ITEM_LIGHT) || 
                   (explode->obj_flags.type_flag==ITEM_WEAPON) ||
                   (explode->obj_flags.type_flag==ITEM_FIREWEAPON) ||
                   (explode->obj_flags.type_flag==ITEM_ARMOR) ||
                   (explode->obj_flags.type_flag==ITEM_WORN)) {
                    act("$o explodes.",0,victim,explode,0,TO_CHAR);
                    extract_obj(explode);
                }
            }
        }
    }
}
