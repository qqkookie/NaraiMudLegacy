/* ************************************************************************
*  file: limits.c , Limit and gain control module.        Part of DIKUMUD *
*  Usage: Procedures controling gain and limit.                           *
*  Copyright (C) 1990, 1991 - see 'license.doc' for complete information. *
************************************************************************* */

#include <stdio.h>
#include <assert.h>
#include "structs.h"
#include "limits.h"
#include "utils.h"
#include "spells.h"
#include "comm.h"
#include "memory.h"
#include "quest.h"

#define SPELL_LEARN_MAX	120

#define READ_TITLE(ch) (GET_SEX(ch) == SEX_MALE ?   \
  titles[GET_CLASS(ch)-1][GET_LEVEL(ch)].title_m :  \
  titles[GET_CLASS(ch)-1][GET_LEVEL(ch)].title_f)


extern struct char_data *character_list;
extern struct obj_data *object_list;
extern struct title_type titles[4][IMO+4];
extern struct room_data *world;

/* External procedures */

void move_stashfile( char *victim);
void update_pos( struct char_data *victim );                 /* in fight.c */
void damage(struct char_data *ch, struct char_data *victim,  /*    do      */
            int damage, int weapontype);
struct time_info_data age(struct char_data *ch);



/* When age < 15 return the value p0 */
/* When age in 15..29 calculate the line between p1 & p2 */
/* When age in 30..44 calculate the line between p2 & p3 */
/* When age in 45..59 calculate the line between p3 & p4 */
/* When age in 60..79 calculate the line between p4 & p5 */
/* When age >= 80 return the value p6 */
int graf(int age, int p0, int p1, int p2, int p3, int p4, int p5, int p6)
{

    if (age < 15)
        return(p0);                               /* < 15   */
    else if (age <= 29) 
        return (int) (p1+(((age-15)*(p2-p1))/15));  /* 15..29 */
    else if (age <= 44)
        return (int) (p2+(((age-30)*(p3-p2))/15));  /* 30..44 */
    else if (age <= 59)
        return (int) (p3+(((age-45)*(p4-p3))/15));  /* 45..59 */
    else if (age <= 79)
        return (int) (p4+(((age-60)*(p5-p4))/20));  /* 60..79 */
    else
        return(p6);                               /* >= 80 */
}


/* The three MAX functions define a characters Effective maximum */
/* Which is NOT the same as the ch->points.max_xxxx !!!          */
int mana_limit(struct char_data *ch)
{
    int max;

    max = 100+ch->points.max_mana;
    return(max);
}

int hit_limit(struct char_data *ch)
{
    int max;

    if (!IS_NPC(ch))
        max = (ch->points.max_hit) +
            (graf((int)age(ch).year, 2,4,17,14,8,4,3));
    else 
        max = (ch->points.max_hit);


/* Class/Level calculations */

/* Skill/Spell calculations */
  
    return (max);
}


int move_limit(struct char_data *ch)
{
    int max;

    if (!IS_NPC(ch))
        max = graf((int)age(ch).year, 50,70,160,120,100,40,20)+
            (int)ch->points.max_move;
    else 
        max = (ch->points.max_move);

    return (max);
}

int mana_gain(struct char_data *ch)
{
#ifdef DEATHFIGHT
	extern int deathfight ;
#endif 
    int gain,i;
    int regen_multiplier = 0;
	int regen_gain;
	int class;

	if (is_solved_quest(ch, QUEST_REMORTAL)) {
		class = GET_CLASS_O(ch);
	}
	else {
		class = GET_CLASS(ch);
	}

    i = GET_INT(ch)/4;
    regen_multiplier += (GET_INT(ch) >= 18)*2 + (GET_INT(ch) >= 16) 
		+ (GET_INT(ch) >= 12) + (GET_INT(ch) >= 6);
    
    if(IS_NPC(ch)) {
        /* Neat and fast */
        gain = GET_LEVEL(ch) + (1 + GET_REGENERATION(ch)) * regen_multiplier;
    } else {
		regen_gain =
            GET_REGENERATION(ch)*regen_multiplier*4/5*GET_LEVEL(ch)/40;

        /* Class calculations */

        if (class==CLASS_MAGIC_USER||class==CLASS_CLERIC)
            regen_gain = regen_gain*6/5;

		if (class== CLASS_THIEF)
			regen_gain = regen_gain*2/3;

        /* Skill/Spell calculations */
        
        gain = graf(age(ch).year, i,i*2,i*3,4*i,i*3,3*i,4*i) + regen_gain;

#ifdef DEATHFIGHT
        if (deathfight == 0 && !IS_NPC(ch))
            gain += 5 ;
#endif 
        /* Position calculations    */
        switch (GET_POS(ch)) {
        case POSITION_SLEEPING:
            gain += (gain>>1); /* Divide by 2 */
            break;
        case POSITION_RESTING:
            gain+= (gain>>2);  /* Divide by 4 */
            break;
        case POSITION_SITTING:
            gain += (gain>>3); /* Divide by 8 */
            break;
		case POSITION_STANDING:
            gain >>= 1;
            break;
        case POSITION_FIGHTING:
            gain >>= 2;
            break;
        }
    }

    if (IS_AFFECTED(ch,AFF_POISON))
        gain >>= 1;
    
    if (IS_AFFECTED(ch,AFF_CURSE))
        gain >>= 1;

    if(GET_COND(ch,FULL)==0)
        gain >>= 1;
 
	if (GET_COND(ch,THIRST)==0)
        gain >>= 1;

    return (gain);
}


int hit_gain(struct char_data *ch)
/* Hitpoint gain pr. game hour */
{
#ifdef DEATHFIGHT
	extern int deathfight ;
#endif 
    int gain, c;
    int regen_multiplier = 0;
	int regen_gain;
	int class;

	if (is_solved_quest(ch, QUEST_REMORTAL)) {
		class = GET_CLASS_O(ch);
	}
	else {
		class = GET_CLASS(ch);
	}

    // i = GET_INT(ch)/4;

    c = GET_CON(ch)/4;

    regen_multiplier += (GET_CON(ch) >= 18)*2 + (GET_CON(ch) >= 16) 
		+ (GET_CON(ch) >= 12) + (GET_CON(ch) >= 6);
    
    if(IS_NPC(ch)) {
        gain = GET_LEVEL(ch)
            + (5+GET_REGENERATION(ch))*regen_multiplier*GET_LEVEL(ch)*2/3;
        /* Neat and fast */
    } else {
		regen_gain = GET_REGENERATION(ch)*regen_multiplier*3/4*GET_LEVEL(ch)/40;
        
        /* Class/Level calculations */


        if (class == CLASS_MAGIC_USER) {
            regen_gain >>= 1;
        }
        else if (class == CLASS_CLERIC) {
            regen_gain = regen_gain*3/4;
        }

        /* Skill/Spell calculations */

        gain = graf(age(ch).year, c,3*c,5*c,9*c,3*c,2*c,c) + regen_gain;
        
#ifdef DEATHFIGHT
        if (deathfight == 0 && !IS_NPC(ch))
            gain += 5 ;
#endif 

        /* Position calculations    */

        switch (GET_POS(ch)) {
        case POSITION_SLEEPING:
            gain += (gain>>1); /* Divide by 2 */
            break;
        case POSITION_RESTING:
            gain+= (gain>>2);  /* Divide by 4 */
            break;
        case POSITION_SITTING:
            gain += (gain>>3); /* Divide by 8 */
            break;
		case POSITION_STANDING:
            gain >>= 1;
            break;
        case POSITION_FIGHTING:
            gain >>= 2;
            break;
        }
    }

    if (IS_AFFECTED(ch,AFF_POISON))
    {
        gain >>= 1;
        damage(ch,ch,2,SPELL_POISON);
    }

    if (IS_AFFECTED(ch,AFF_CURSE))
        gain >>= 1;

    if(GET_COND(ch,FULL)==0)
        gain >>= 1;
 
	if (GET_COND(ch,THIRST)==0)
        gain >>= 1;

	if (GET_LEVEL(ch) <= 25) return MAX(3,gain);
	return (gain);
}

int move_gain(struct char_data *ch)
/* move gain pr. game hour */
{
#ifdef DEATHFIGHT
	extern int deathfight ;
#endif 
    int gain,d;
    int regen_multiplier = 0;
	int regen_gain;

	int class;

	if (is_solved_quest(ch, QUEST_REMORTAL)) {
		class = GET_CLASS_O(ch);
	}
	else {
		class = GET_CLASS(ch);
	}

    d = GET_DEX(ch)/3;

    regen_multiplier += (GET_DEX(ch) >= 18)*2 + (GET_DEX(ch) >= 16) 
		+ (GET_DEX(ch) >= 12) + (GET_DEX(ch) >= 6);
    
    if(IS_NPC(ch)) {
        return(GET_LEVEL(ch)) + (1 + GET_REGENERATION(ch)) * regen_multiplier;  
        /* Neat and fast */
    } else {
		regen_gain = GET_REGENERATION(ch)*regen_multiplier*GET_LEVEL(ch)/40;

        /* Class/Level calculations */

		if(class==CLASS_THIEF)
			regen_gain = regen_gain*4/3;

		if(class==CLASS_CLERIC)
			regen_gain = regen_gain*3/4;

		if(class==CLASS_MAGIC_USER)
			regen_gain = regen_gain*2/3;

        /* Skill/Spell calculations */

        gain = graf((int)age(ch).year, 4*d,6*d,7*d,7*d,5*d,4*d,2*d)
			+ regen_gain;

#ifdef DEATHFIGHT
        if (deathfight == 0 && !IS_NPC(ch))
            gain += 5 ;
#endif 

        /* Position calculations    */
        switch (GET_POS(ch)) {
        case POSITION_SLEEPING:
            gain += (gain>>1); /* Divide by 2 */
            break;
        case POSITION_RESTING:
            gain+= (gain>>2);  /* Divide by 4 */
            break;
        case POSITION_SITTING:
            gain += (gain>>3); /* Divide by 8 */
            break;
		case POSITION_STANDING:
            gain >>= 1;
            break;
        case POSITION_FIGHTING:
            gain >>= 2;
            break;
        }
    }

    if (IS_AFFECTED(ch,AFF_POISON))
        gain >>= 1;

    if (IS_AFFECTED(ch,AFF_CURSE))
        gain >>= 1;

    if(GET_COND(ch,FULL)==0)
        gain >>= 1;
 
	if (GET_COND(ch,THIRST)==0)
        gain >>= 1;

	if (GET_LEVEL(ch) <= 25) return MAX(20, gain);
	else return (gain);
}


/* Gain maximum in various points */
void advance_level(struct char_data *ch)
{
    int add_hp, add_mana, add_move, i;
    int is_solved_quest(struct char_data *ch, int quest);

    extern struct wis_app_type wis_app[];
    extern struct con_app_type con_app[];
    extern struct dex_app_type dex_app[];

    // remortal 한 character를 advance 한 경우에는 수치 증가 없도록
    //
    if (is_solved_quest(ch, QUEST_REMORTAL))
        return;
 
    add_hp = con_app[GET_CON(ch)].hitp;
    add_mana = wis_app[GET_WIS(ch)].bonus;
    add_move = dex_app[GET_DEX(ch)].reaction;

    switch(GET_CLASS(ch)) {

    case CLASS_MAGIC_USER : {
        add_hp += number(4, 8);
        add_mana += number(3, 8);
        add_move += number(4, 7);
    } break;

    case CLASS_CLERIC : {
        add_hp += number(5, 10);
        add_mana += number(4, 6);
        add_move += number(4, 7);
    } break;

    case CLASS_THIEF : {
        add_hp += number(7,13);
        add_mana += number(1, 3);
        add_move += number(6, 9);
    } break;

    case CLASS_WARRIOR : {
        add_hp += number(10,15);
        add_mana += number(1, 3);
        add_move += number(5, 8);
    } break;
    }
    GET_AC(ch)--;
    ch->points.max_hit += MAX(1, add_hp);
    ch->points.max_mana += MAX(1, add_mana);
    ch->points.max_move += MAX(1, add_move);
    if (ch->specials.spells_to_learn < SPELL_LEARN_MAX) {
        if (GET_CLASS(ch) == CLASS_MAGIC_USER || GET_CLASS(ch) == CLASS_CLERIC)
            ch->specials.spells_to_learn += MAX(2, wis_app[GET_WIS(ch)].bonus);
        else
            ch->specials.spells_to_learn += MIN(2,MAX(1, wis_app[GET_WIS(ch)].bonus));
    }
    if (GET_LEVEL(ch) > IMO)
        for (i = 0; i < 3; i++)
            ch->specials.conditions[i] = -1;
}  

void set_title(struct char_data *ch)
{
	if (IS_NPC(ch))	/* title is only for player */
		return ;

    if (GET_TITLE(ch)) {
		/*
          recreate(GET_TITLE(ch),char,strlen(READ_TITLE(ch))+1);
		*/
		free_string_type(GET_TITLE(ch)) ;
		GET_TITLE(ch) = malloc_string_type(strlen(READ_TITLE(ch))+1) ;
	}
    else {
        /*
          create(GET_TITLE(ch),char,strlen(READ_TITLE(ch)));
        */
		GET_TITLE(ch) = malloc_string_type(strlen(READ_TITLE(ch)) + 1) ;
    }

    strcpy(GET_TITLE(ch), READ_TITLE(ch));
}
void gain_exp(struct char_data *ch, int gain)
{
    if (IS_NPC(ch) || ((GET_LEVEL(ch)<IMO) && (GET_LEVEL(ch) > 0))) {
        if (gain > 0) {
            gain = MIN(GET_LEVEL(ch) * 30000, gain);
            GET_EXP(ch) += gain;
        }
        if (gain < 0) {
            if (GET_EXP(ch) < (-gain))
                GET_EXP(ch) = 0 ;
            else {
				if (GET_LEVEL(ch) <30) 
					gain = (-1) * MIN(100000*GET_LEVEL(ch), -gain);
				else {
					gain = (-1) * MIN(200000*GET_LEVEL(ch), -gain);
				}
                if (GET_EXP(ch) < (-gain))
                    GET_EXP(ch) = 0;
                else
                    GET_EXP(ch) += gain;
            }
        }
    }
}

void gain_gold(struct char_data *ch, int gain)
{
    if (IS_NPC(ch) || ((GET_LEVEL(ch)<IMO) && (GET_LEVEL(ch) > 0))) {
        if (gain > 0) {
            gain = MIN(GET_LEVEL(ch) * 30000, gain);
            GET_GOLD(ch) += gain;
        }
        if (gain < 0) {
            if (GET_GOLD(ch) < (-gain))
                GET_GOLD(ch) = 0 ;
            else {
                gain = (-1) * MIN(200000*GET_LEVEL(ch), -gain);
                if (GET_GOLD(ch) < (-gain))
                    GET_GOLD(ch) = 0;
                else
                    GET_GOLD(ch) += gain;
            }
        }
    }
}

void gain_exp_regardless(struct char_data *ch, int gain)
{
    int i;
    bool is_altered = FALSE;

    if (!IS_NPC(ch)) {
        if (gain > 0) {
            GET_EXP(ch) += gain;
            for(i=0;(i<=(IMO+3))&&
                    (titles[GET_CLASS(ch)-1][i].exp<=GET_EXP(ch));i++){
                if (i > GET_LEVEL(ch)) {
                    send_to_char("You raise a level.\n\r",ch);
                    GET_LEVEL(ch) = i;
                    advance_level(ch);
                    is_altered = TRUE;
                }
            }
        }
        if (gain < 0) 
            GET_EXP(ch) += gain;
        if (GET_EXP(ch) < 0)
            GET_EXP(ch) = 0;
    }
    if (is_altered)
        set_title(ch);
}
void gain_condition(struct char_data *ch,int condition,int value)
{
    bool intoxicated;

    if (GET_COND(ch, condition)==-1) {
		// GET_EXP(ch) += 1000000;
		//    return;
	}

    intoxicated=(GET_COND(ch, DRUNK) > 0);
    GET_COND(ch, condition)  += value;
    GET_COND(ch,condition) = MAX(0,GET_COND(ch,condition));
    GET_COND(ch,condition) = MIN(36,GET_COND(ch,condition));
    if(GET_COND(ch,condition))
        return;

	if (GET_LEVEL(ch) > IMO) return;

    switch(condition){
    case FULL :
    {
        send_to_char("You are hungry.\n\r",ch);
        return;
    }
    case THIRST :
    {
        send_to_char("You are thirsty.\n\r",ch);
        return;
    }
    case DRUNK :
    {
        if(intoxicated)
            send_to_char("You are now sober.\n\r",ch);
        return;
    }
    default : break;
    }

}

#define	IDLE_TIMER_VOID	5		/* 5 tick */
#define	IDLE_TIMER_RENT	10		/* 10 tick */
#define	IDLE_TIMER_CONNECT	100	/* 100 second */
#define	IDLE_TIMER_MENU	3		/* 10 tick */

void check_idle_player(void)
{
	extern struct descriptor_data *descriptor_list;
	void check_idling(struct char_data *ch) ;
	struct descriptor_data	*d, *d_next ;
	int	thistime ;

	thistime = time(0) ;
	for ( d = descriptor_list ; d ; d = d_next ) {
		d_next = d->next ;
		/* need not check switched body, playing character .. */
		if ( d->original )	continue ;
		else if ( d->character ) {
			if ( d->connected != CON_PLYNG ) {
				if ( ++(d->character->specials.timer) > IDLE_TIMER_MENU ) {
					close_socket(d) ;
                }
            }
        }
		else {	/* UNDEF - Get name, have no char_data  */
			if ( thistime - d->contime > IDLE_TIMER_CONNECT ) {
				/* wait for 100 seconds */
				close_socket(d) ;
            }
        }
    }
}

void check_idling(struct char_data *ch)
{
	char	buf[BUFSIZ] ;

    if ( ++(ch->specials.timer) > IDLE_TIMER_VOID) {
        if (ch->specials.was_in_room == NOWHERE && ch->in_room != NOWHERE) {
            ch->specials.was_in_room = ch->in_room;
            if (ch->specials.fighting) {
                stop_fighting(ch->specials.fighting);
                stop_fighting(ch);
            }
            act("$n disappears into the void.", TRUE, ch, 0, 0, TO_ROOM);
            send_to_char("You have been idle, and are pulled into a void.\n\r", ch);
            char_from_room(ch);
            char_to_room(ch, 1);  /* Into room number 0 */
        } else if (ch->specials.timer > IDLE_TIMER_RENT) {
            sprintf(buf, "check_idling : Force %s rent", GET_NAME(ch)) ;
            log(buf) ;
            do_rent(ch,0,"");
            if (ch->desc)
                close_socket(ch->desc);
        }
    }
}

/* Update both PC's & NPC's and objects*/
void point_update( void )
{  
    void update_char_objects( struct char_data *ch ); /* handler.c */
    void extract_obj(struct obj_data *obj); /* handler.c */
    struct char_data *i, *next_dude;
    struct obj_data *j, *next_thing, *jj, *next_thing2;

    /* characters */
    for (i = character_list; i; i = next_dude) {
        next_dude = i->next;
        if (GET_POS(i) > POSITION_STUNNED) {
            GET_HIT(i)  = MIN(GET_HIT(i)  + hit_gain(i),  hit_limit(i));
            GET_MANA(i) = MIN(GET_MANA(i) + mana_gain(i), mana_limit(i));
            GET_MOVE(i) = MIN(GET_MOVE(i) + move_gain(i), move_limit(i));
        } else if (GET_POS(i) == POSITION_STUNNED) {
            GET_HIT(i)  = MIN(GET_HIT(i)  + hit_gain(i),  hit_limit(i));
            GET_MANA(i) = MIN(GET_MANA(i) + mana_gain(i), mana_limit(i));
            GET_MOVE(i) = MIN(GET_MOVE(i) + move_gain(i), move_limit(i));
            update_pos( i );
        } else if  (GET_POS(i) == POSITION_INCAP)
            damage(i, i, 1, TYPE_SUFFERING);
        else if (!IS_NPC(i) && (GET_POS(i) == POSITION_MORTALLYW))
            damage(i, i, 2, TYPE_SUFFERING);
        if (!IS_NPC(i))
        {
            update_char_objects(i);
            if (GET_LEVEL(i) < (IMO+1))
                check_idling(i);
        }
        gain_condition(i,FULL,-1);
        gain_condition(i,DRUNK,-1);
        gain_condition(i,THIRST,-1);
    } /* for */

    /* objects */
    for(j = object_list; j ; j = next_thing){
        next_thing = j->next; /* Next in object list */

        /* If this is a corpse */
        if ( (GET_ITEM_TYPE(j) == ITEM_CONTAINER) && (j->obj_flags.value[3]) ) {
            /* timer count down */
            if (j->obj_flags.timer > 0) j->obj_flags.timer--;

            if (!j->obj_flags.timer) {

                /* do not touch rented_by items here.. it is not affected by
                   mud timer.. Rented item is safe.
                */
                if (j->carried_by)
                    act("$p decay in your hands.", FALSE, j->carried_by, j, 0, TO_CHAR);
                else if ((j->in_room != NOWHERE) && (world[j->in_room].people)){
                    act("A quivering hoard of maggots consume $p.",
                        TRUE, world[j->in_room].people, j, 0, TO_ROOM);
                    act("A quivering hoard of maggots consume $p.",
                        TRUE, world[j->in_room].people, j, 0, TO_CHAR);
                }

                for(jj = j->contains; jj; jj = next_thing2) {
                    next_thing2 = jj->next_content; /* Next in inventory */
                    obj_from_obj(jj);

                    if (j->in_obj)
                        obj_to_obj(jj,j->in_obj);
                    else if (j->carried_by)
                        obj_to_room(jj,j->carried_by->in_room);
                    /* probabily, there must not be rented_by item .. here */
                    else if (j->in_room != NOWHERE)
                        obj_to_room(jj,j->in_room);
                    else {
						char tmp[255];
						sprintf(tmp, "jj: %s\n", jj->name);
						log(tmp);
						sprintf(tmp, "j: %s\n", j->name);
						log(tmp);
                        assert(FALSE);
					}
                }
                extract_obj(j);
            }
        }
    }
}

/*
	by cold
	bomb.  value[0] - current timer
          value[1] - damage dice 1
          value[2] - damage dice 2
          value[3] - bomb special - not implemented yet
*/
void bomb_time_update(void)
{  
	void extract_obj(struct obj_data *obj); /* handler.c */
	struct obj_data	*j, *next_obj;
	struct char_data	*vict, *next_victim ;
	int	dam ;

    /* bomb objects */
	for(j = object_list; j ; j = next_obj) {
		next_obj = j->next; /* Next in object list */
		/* If this is a bomb */
		if ( (GET_ITEM_TYPE(j) == ITEM_BOMB) ) {
			if ( !(j->in_obj || j->carried_by || j->rented_by )
                 && j->in_room != NOWHERE ) {
				/* only if obj is on the ground decrease counter */
				if (j->obj_flags.value[0] > 0)
					j->obj_flags.value[0]--;
				if (j->obj_flags.value[0] <= 0) {	/* BANG !!! */
					for ( vict = character_list ; vict ; vict = next_victim ) {
						next_victim = vict->next ;
						if ( j->in_room == vict->in_room ) {
							send_to_char(" ***  콰콰광 !!! ***\n\r", vict) ;
							dam = dice(j->obj_flags.value[1], j->obj_flags.value[2]) ;
							/* No problem ? if GET_HIT(ch) < dam ?? */
							GET_HIT(vict) -= dam ;
							update_pos(vict) ;
							if ( GET_POS(vict) == POSITION_DEAD)
								die(vict, NULL) ;
                        }
						else if (world[j->in_room].zone == world[vict->in_room].zone) {
							send_to_char("어디선가 <쾅> 하는 폭음이 들려옵니다.\n\r", vict) ;
                        }
                    }
					extract_obj(j) ;
                }
            }
        }	/* end of bomb */
    }	/* end of for */
}

// 
// 필요 exp를 변경 
// 
void init_exp_table()
{
	extern struct title_type titles[4][IMO+4];
	int	i, j ;

	for ( i = 0 ; i < 4 ; i ++ ) {	/* class */
		for ( j = 0 ; j < IMO+4 ; j ++ ) {	/* level */
			titles[i][j].exp = titles[i][j].exp * 3 / 4;
        }
    }
}

#ifdef DEATHFIGHT
void init_deathfight_exp()
{
	extern struct title_type titles[4][IMO+4];
	int	i, j ;

	for ( i = 0 ; i < 4 ; i ++ ) {	/* class */
		for ( j = 0 ; j < IMO+4 ; j ++ ) {	/* level */
			titles[i][j].exp = titles[i][j].exp / 25 ;
        }
    }
}
#endif 
