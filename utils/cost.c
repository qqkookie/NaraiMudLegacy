/*
	cost.c - calculate item's cost
*/

#include <stdio.h>
#include "structs.h"
#include "utils.h"

int intabs(int val) ;
int item_cost(struct obj_data *obj) ;
int calc_apply_type(struct obj_data *obj, int type) ;
int weapon_cost(struct obj_data *obj) ;
int apply_cost(int type, int x) ;
int count_can_wear(struct obj_data *obj) ;
int has_anti_class(struct obj_data *obj) ;
int has_anti_align(struct obj_data *obj) ;
float wearing_rarity(struct obj_data *obj) ;
int total_scaling(int cost) ;

char fileid[] = "@(#)cost.c	1.3 94/11/18 by Cold" ;

#define	LEAST_ITEM_COST	10

/* return absolute value */
int intabs(int val)
{
	if ( val >= 0 )
		return (val) ;
	else
		return (-val) ;
}

/* return cost of object */
int item_cost(struct obj_data *obj)
{
	int	cost, damrol, armor, armor_cost, wield_cost ;
	int	count, j, location, modify_value ;

	/* There is three sort of items.
	  1) used only weapon  ->  cost is calculated by weapon_cost()
	  2) used except weapon -> cost is summantion of apply_cost()
	  3) can use weapon and others -> max of 1) and 2)
	  and must be consider container also.
	*/

	wield_cost = armor = cost = 0 ;
	if (CAN_WEAR( obj, ITEM_WIELD ))
		wield_cost = weapon_cost(obj) ;

	if ( GET_ITEM_TYPE(obj) == ITEM_ARMOR ) {
		armor = -1 * obj->obj_flags.value[0] ;	/* negative value is good */
		}

	/* If container, cost is propotional to it's capacity */
	if ( GET_ITEM_TYPE(obj) == ITEM_CONTAINER ) {
		cost += 2 * obj->obj_flags.value[0] ;
		}

	for ( j = 0 ; j < MAX_OBJ_AFFECT ; j++ ) {
		location = obj->affected[j].location ;
		modify_value = obj->affected[j].modifier ;

		/* Do not calculate armor cost here  */
		if ( location == APPLY_ARMOR ) {
			armor += modify_value ;
			}
		else {
			cost += apply_cost(location, modify_value ) ;
			}
		}

	/* add armor cost -- AC and apply */
	cost = cost + apply_cost(APPLY_ARMOR, armor) ;

	/* If one item can wield and wear, choose maxmum value b/w them */
	count = count_can_wear(obj) ;
	switch (count) {
		case 0 :	/* used by only take. example paper, bag */
			break ;
		case 1 : 
			if ( wield_cost > 0 )	/* only weapon */
				cost = wield_cost ;
			/* else, only except weapon. cost is correct value now */
			break ;
		default : /* choose maximum */
			cost =  (wield_cost > cost) ? wield_cost : cost ;
		}

	/* temporary chech */
	if ( cost < 0 ) {
		cost = LEAST_ITEM_COST ;
		}

	/* If one item can wear several location, muliply factor */
	if ( count > 1 )
		for ( j = 0 ; j < count ; j ++ )
			cost = 1.4 * cost ;

	/* add weight to cost */
	if ( GET_OBJ_WEIGHT(obj) < 40 ) {
		cost += (80 - GET_OBJ_WEIGHT(obj)*2) ;
		}

	/* anti-align factor */
	switch ( has_anti_align(obj) ) {
		case 0 :	/* very good item : has no align */
			cost *= 1.9 ; break ;
		case 2 : /* only for one align : has some discount */
			cost *= 0.7 ; break ;
		case 1 :	/* do noting */
		default : break ;
		}
	
	/* anti-class factor */
	switch ( has_anti_class(obj) ) {
		case 0 : /* very good item : no limit on class */
			cost *= 1.8 ; break ;
		case 1 : /* some good item */
			cost *= 1.2 ; break ;
		case 3 : /* only for one class : has some discount */
			cost *= 0.75 ; break ;
		case 2 : /* for two class : do noting */
		deafult : break ;
		}

	/* now consider the rarity of items : ex) aboutlegs item .*/
	cost = cost * wearing_rarity(obj) ;

	/* Glowing item's advantage (harder to break by death) */
	if (IS_OBJ_STAT(obj, ITEM_GLOW))
		cost *= 1.2 ;

	/* Total scaling for all items */
	cost = total_scaling(cost) ;

	if ( cost < LEAST_ITEM_COST )
		return LEAST_ITEM_COST ;
	else
		return (cost) ;
}

int calc_apply_type(struct obj_data *obj, int type)
{
	int 	j, value ;

	if ( obj == NULL ) return 0 ;

	for ( j = 0, value = 0 ; j < MAX_OBJ_AFFECT ; j++ ) {
		if ( obj->affected[j].location == type )
			value += obj->affected[j].modifier ;
		}

	return (value) ;
}
	

int weapon_cost(struct obj_data *obj)
{
	int	average, additive, cost, cost_app ;
	int	j, tmp1, tmp2, location, modify_value ;
	int	dice1, dice2, damrol ;

	/* make positive value */
	damrol = calc_apply_type(obj, APPLY_DAMROLL) ;
	dice1 = intabs(obj->obj_flags.value[1]) ;
	dice2 = intabs(obj->obj_flags.value[2]) ;

	additive = 0 ;
	average = ( dice1 * (dice2+1) + 1 ) / 2 + damrol ;
	if ( average > 15 ) {
		tmp1 = (average - 15)*(average - 15) ;
		tmp2 = 10*(average - 15) ;
		additive = ( tmp1 < tmp2 ) ? tmp1 : tmp2 ;
		}
	if ( average > 10 )
		cost = 120 * average + 100 * additive ;
	else
		cost = 60 * average ;

	/*  consider apply  : regen, armor etc. */
	for ( j = 0 ; j < MAX_OBJ_AFFECT ; j++ ) {
		location = obj->affected[j].location ;
		modify_value = obj->affected[j].modifier ;
		/* Do not calculate damrol cost */
		if ( location != APPLY_DAMROLL ) {
			cost += apply_cost(location, modify_value ) ;
			}
		}

	if ( obj->obj_flags.value[3] == 11 )	/* can backstab */
		cost = 1.4 * cost ;

	return cost ;
}

int apply_cost(int type, int x)
{
	int	cost ;

	if ( x == 0 )
		return 0 ;

	if ( x > 0 ) {	/* for positive values */
		switch ( type ) {
			case APPLY_DAMROLL :	/* Positive is good */
				cost = 300 + 120 * (x + 1)*(x + 1) ;
				break ;
			case APPLY_HITROLL :	/* Positive is good */
				cost = 150 + 100 * (( (x+1)*(x+1) < (6*x) ) ? (x+1)*(x+1) : (6*x)) ;
				break ;
			case APPLY_ARMOR :	/* positive is worse */
				cost = -30 * x ;
				break ;
			case APPLY_REGENERATION :	/* positive is good */
				if ( x == 1 )
					cost = 100 ;
				else
					cost = 400 * x ;
				break ;
			case APPLY_STR :	/* positive is good */
				cost = 500 * x ;
				break ;
			case APPLY_SAVING_BREATH :	/* positive is worse */
			case APPLY_SAVING_SPELL :
			case APPLY_SAVING_PARA :
			case APPLY_SAVING_ROD :
			case APPLY_SAVING_PETRI :
				cost = -200 * x ;
				break ;
			case APPLY_HIT :	/* positive is some good */
			case APPLY_MANA :
			case APPLY_MOVE :
				if ( x <= 10 )
					cost = 20 * x ;
				else if ( x <= 70)
					cost = 30 * x ;
				else
					cost = 50 * x ;
				break ;
			case APPLY_DEX :	/* positive is good */
			case APPLY_INT :
			case APPLY_WIS :
			case APPLY_CON :
				if ( x == 1 )
					cost = 180 ;
				else
					cost = 400 * x ;
				break ;
			case APPLY_INVISIBLE : cost = 1000 ;
				break ;
			case APPLY_CHAR_WEIGHT :	/* positive is bad */
			case APPLY_CHAR_HEIGHT :
			case APPLY_AGE : cost = -50 ;
				break ;
			default : cost = 0 ;
				break ;
			}
		}
	else {	/* for negative values */
		switch ( type ) {
			case APPLY_DAMROLL :	/* negative is too bad */
				cost = -1 * (150 + 60 * x * x ) ;
				break ;
			case APPLY_HITROLL :	/* negative is some bad */
				cost = 80 * x ;	/* cost < 0 */
				break ;
			case APPLY_ARMOR :	/* negative is good */
				if ( x >= -3 )
					cost = -30 * x ;
				else
					cost = -100 * x ;
				if ( x <= -10 )
					cost = cost - 50 * (x + 10) ;	/* it makes cost high */
				if ( x <= -12 )
					cost = cost + 100 * (x + 12)*(x + 12) ;
				break ;
			case APPLY_REGENERATION :	/* negative is bad */
				if ( x == 1 )
					cost = -50 ;
				else
					cost = 200 * x ;	/* cost < 0, bacause x < 0 */
				break ;
			case APPLY_STR :	/* negative is bad */
				cost = 250 * x ;	/* cost < 0, because x < 0 */
				break ;
			case APPLY_SAVING_BREATH :	/* negative is better */
			case APPLY_SAVING_SPELL :	/* use (-x) to make cost > 0 */
			case APPLY_SAVING_PARA :
			case APPLY_SAVING_ROD :
			case APPLY_SAVING_PETRI : cost = 300 * (-x) ;
				break ;
			case APPLY_HIT :	/* negative is also useful .. meta item */
			case APPLY_MANA :	/* because x is negative, use (-x) */
			case APPLY_MOVE :	/* to make cost > 0  */
				if ( x >= -10 )
					cost = 15 * (-x) ;
				else if ( x >= -70)
					cost = 25 * (-x) ;
				else
					cost = 45 * (-x) ;
				break ;
			case APPLY_DEX :	/* negative is some bad.. but not too bad */
			case APPLY_INT :
			case APPLY_WIS :
			case APPLY_CON :
				if ( x == 1 )
					cost = -20 ;
				else
					cost = 40 * x ;	/* cost < 0, because x is negative */
				break ;
			case APPLY_INVISIBLE : cost = 1000 ;	/* It has no meaning */
				break ;
			case APPLY_CHAR_WEIGHT :	/* negative is good */
			case APPLY_CHAR_HEIGHT :
			case APPLY_AGE : cost = 500 ;
				break ;
			default : cost = 0 ;
				break ;
			}
		}

	return (cost) ;
}

int count_can_wear(struct obj_data *obj)
{
	int	count = 0 ;

	if (CAN_WEAR( obj, ITEM_WEAR_FINGER))	count ++ ;
	if (CAN_WEAR( obj, ITEM_WEAR_NECK))	count ++ ;
	if (CAN_WEAR( obj, ITEM_WEAR_BODY))	count ++ ;
	if (CAN_WEAR( obj, ITEM_WEAR_HEAD))	count ++ ;
	if (CAN_WEAR( obj, ITEM_WEAR_LEGS))	count ++ ;
	if (CAN_WEAR( obj, ITEM_WEAR_FEET))	count ++ ;
	if (CAN_WEAR( obj, ITEM_WEAR_HANDS))	count ++ ;
	if (CAN_WEAR( obj, ITEM_WEAR_ARMS))	count ++ ;
	if (CAN_WEAR( obj, ITEM_WEAR_SHIELD))	count ++ ;
	if (CAN_WEAR( obj, ITEM_WEAR_ABOUT))	count ++ ;
	if (CAN_WEAR( obj, ITEM_WEAR_WAISTE))	count ++ ;
	if (CAN_WEAR( obj, ITEM_WEAR_WRIST))	count ++ ;
	if (CAN_WEAR( obj, ITEM_WIELD))	count ++ ;
	if (CAN_WEAR( obj, ITEM_HOLD))	count ++ ;
	if (CAN_WEAR( obj, ITEM_WEAR_KNEE))	count ++ ;
	if (CAN_WEAR( obj, ITEM_WEAR_ABOUTLEGS))	count ++ ;

	return (count) ;
}

int has_anti_class(struct obj_data *obj)
{
	/* return number of anti-class */
	int	nanti ;

	nanti = 0 ;
	if (IS_OBJ_STAT(obj, ITEM_ANTI_MAGE)) 	nanti ++ ;
	if (IS_OBJ_STAT(obj, ITEM_ANTI_CLERIC)) 	nanti ++ ;
	if (IS_OBJ_STAT(obj, ITEM_ANTI_WARRIOR)) 	nanti ++ ;
	if (IS_OBJ_STAT(obj, ITEM_ANTI_THIEF)) 	nanti ++ ;

	return (nanti) ;
}

int has_anti_align(struct obj_data *obj)
{
	/* return number of anti-class */
	int	nanti ;

	nanti = 0 ;
	if (IS_OBJ_STAT(obj, ITEM_ANTI_GOOD)) 	nanti ++ ;
	if (IS_OBJ_STAT(obj, ITEM_ANTI_NEUTRAL)) 	nanti ++ ;
	if (IS_OBJ_STAT(obj, ITEM_ANTI_EVIL)) 	nanti ++ ;

	return (nanti) ;
}

/*
	Some sort of item is very rare (etc knee, plates)
	So it has some multiplier : only consern rare items ( < 30 )
	Here is number of items in present world
    199   item_hold
    108   item_wield
     23   wear_about
      4   wear_aboutlegs
     13   wear_arms
     41   wear_body
     29   wear_feet
     32   wear_finger
     24   wear_hands
     44   wear_head
      8   wear_knee
     27   wear_legs
     22   wear_neck
     20   wear_shield
     17   wear_waist
     12   wear_wrist
    623   total
*/
#define	NUMBER_AVERAGE_EXPECT	35.0
#define	N_CONSIDER_RARITY	11
struct Rare_item {
	int	type ;
	float	nitem ;
} rare_items[N_CONSIDER_RARITY] = {
	{ITEM_WEAR_ABOUTLEGS, 7.3},	/* actually 4.0 but it is too expensive */
	{ITEM_WEAR_KNEE, 8.5},	/* actually 8.0 but even leather is expensive */
	{ITEM_WEAR_WRIST, 12.0},
	{ITEM_WEAR_ARMS, 13.0},
	{ITEM_WEAR_WAISTE, 17.0},
	{ITEM_WEAR_SHIELD, 20.0},
	{ITEM_WEAR_NECK, 22.0},
	{ITEM_WEAR_ABOUT, 23.0},
	{ITEM_WEAR_HANDS, 24.0},
	{ITEM_WEAR_LEGS, 27.0},
	{ITEM_WEAR_FEET, 29.0}
	} ;

float wearing_rarity(struct obj_data *obj)
{
	/* return : sorting by rate */
	int	i ;
	extern struct Rare_item	rare_items[] ;

	for ( i = 0 ; i < N_CONSIDER_RARITY ; i++ ) {
		if (CAN_WEAR( obj, rare_items[i].type))
			return (NUMBER_AVERAGE_EXPECT / rare_items[i].nitem) ;
		}
	return (1.0) ;
}

/*  Total scaling function  : the rich pays more the poor pays lesser */
int total_scaling(int cost)
{
	float	tmp ;

	cost = 0.4 * cost ;
	if ( cost <= 0 )
		return 0 ;
	if ( cost < 2000 )
		return ( cost * ( 0.2 + 0.8 * 0.0005 * cost)) ;
	else {
		return cost ;
		}
		/*
		tmp = 0.8 + 0.2 * 0.00025* cost ;
		if ( tmp > 1.0 )
			return ( cost * tmp ) ;
		else
			return cost ;
		*/
		/*
		return ( cost * ( 0.8 + 0.2 * 0.0005 * cost)) ;
		*/
}
