/* ************************************************************************
*  file: act.obj1.c , Implementation of commands.         Part of DIKUMUD *
*  Usage : Commands mainly moving around objects and wearing, hold/wield. *
*  Copyright (C) 1990, 1991 - see 'license.doc' for complete information. *
************************************************************************* */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "char.h"
#include "object.h"
#include "global.h"
#include "comm.h"


/* extern functions */
extern int is_number(char *arg);

/* procedures related to get */
void get(struct char_data *ch, struct obj_data *obj_object,
	 struct obj_data *sub_object)
{
    char buffer[MAX_BUFSIZ];

    if (sub_object) {
	obj_from_obj(obj_object);
	obj_to_char(obj_object, ch);
	if (sub_object->carried_by == ch) {
	    act("You get $p from $P.", 0, ch, obj_object, sub_object,
		TO_CHAR);
	    act("$n gets $p from $s $P.",
		1, ch, obj_object, sub_object, TO_ROOM);
	}
	else {
	    act("You get $p from $P.", 0, ch, obj_object, sub_object, TO_CHAR); 
	    act("$n gets $p from $P.", 1, ch, obj_object, sub_object, TO_ROOM);
	}
    }
    else {
	obj_from_room(obj_object);
	obj_to_char(obj_object, ch);
	act("You get $p.", 0, ch, obj_object, 0, TO_CHAR);
	act("$n gets $p.", 1, ch, obj_object, 0, TO_ROOM);
    }
    if ((obj_object->obj_flags.type_flag == ITEM_MONEY) &&
	(obj_object->obj_flags.value[0] >= 1)) {
	obj_from_char(obj_object);
	sprintf(buffer, "There was %d coins.\n\r",
		obj_object->obj_flags.value[0]);
	send_to_char(buffer, ch);
	GET_GOLD(ch) += obj_object->obj_flags.value[0];
	extract_obj(obj_object);
    }
} 

/* NOTE: NEW! Check ch can 'get' object or not. (Except visibility check) 
    Return 0 if ok to get the object.
    Return 2 if carry number exceeded. Return 1 if failed by other cause. */
int can_not_get_item(struct char_data *ch, struct obj_data *obj )
{ 
    char buffer[MAX_BUFSIZ];

    ASSERT(obj);

    if ( !CAN_WEAR(obj, ITEM_TAKE)) {
	send_to_char("You can't take that.\n\r", ch);
	return(1);
    }
    else if ((GET_CARRYING_N(ch) + 1) > CAN_CARRY_N(ch)) {
	sprintf(buffer, "%s : You can't carry that many items.\n\r",
		fname(obj->name));
	send_to_char(buffer, ch);
	return(2);
    }
    else if ((GET_CARRYING_W(ch) + GET_OBJ_WEIGHT(obj)) > CAN_CARRY_W(ch)) {
	sprintf(buffer, "%s : You can't carry that much weight.\n\r",
		fname(obj->name));
	send_to_char(buffer, ch);
	return(1);
    }
    else 
	return(0);
}

void do_get(struct char_data *ch, char *argument, int cmd)
{
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    char buffer[MAX_BUFSIZ];
    struct obj_data *sub_object;
    struct obj_data *obj_object;
    struct obj_data *next_obj;
    bool found = FALSE;
    int fail = 0;

    argument_interpreter(argument, arg1, arg2);

    /* Old code was too comlex. Cleaned up code */

    if (!*arg1) { 	/* get */
	send_to_char("Get what?\n\r", ch); 
	return;
    } 
    else if ( *arg2 && str_cmp( arg2, "all") == 0) {
	if (str_cmp(arg1, "all") == 0) /* get all all */
	    send_to_char("You must be joking?!\n\r", ch);
	else /* get ??? all */
	    send_to_char("You can't take a thing from more than one container.\n\r", ch);
	return;
    }

    if ( !*arg2 && str_cmp(arg1, "all") == 0 ) {	/* get all */
	for (obj_object = world[ch->in_room].contents; obj_object;
	     obj_object = next_obj) {
	    next_obj = obj_object->next_content;
	    if (!CAN_SEE_OBJ(ch, obj_object))
		continue;
	    else if (!( fail = can_not_get_item(ch, obj_object))) {
		get(ch, obj_object, NULL);
		found = TRUE;
	    }
	    else if ( fail > 1 )
		break; 
	}
	if (found)
	    send_to_char("OK.\n\r", ch);
	else if ( !fail )
	    send_to_char("You see nothing here.\n\r", ch);
	return;
    } 
    else if ( !*arg2 ) {    /* get ??? */

	obj_object = get_obj_in_list_vis(ch, arg1, world[ch->in_room].contents);
	if (!obj_object) {
	    sprintf(buffer, "You do not see a %s here.\n\r", arg1);
	    send_to_char(buffer, ch);
	}
	else if (!can_not_get_item(ch, obj_object)) 
	    get(ch, obj_object, NULL);
	return;
    }

    /* get all ??? */ /* get ??? ??? */
    sub_object = get_obj_in_list_vis(ch, arg2, world[ch->in_room].contents);
    if (!sub_object)
	sub_object = get_obj_in_list_vis(ch, arg2, ch->carrying);

    if (!sub_object) {
	sprintf(buffer, "You do not see or have the %s.\n\r", arg2);
	send_to_char(buffer, ch);
    }
    else if (GET_ITEM_TYPE(sub_object) != ITEM_CONTAINER) {
	sprintf(buffer, "The %s is not a container.\n\r",
		fname(sub_object->name));
	send_to_char(buffer, ch);
    }
    else if (IS_SET(sub_object->obj_flags.value[1], CONT_CLOSED))
	send_to_char("It's closed.\n\r", ch);
    else if (IS_SET(sub_object->obj_flags.value[1], CONT_LOCKED))
	send_to_char("It's locked.\n\r", ch);
    else if ( str_cmp(arg1, "all") == 0) {	/* get all ??? */ 
	for (obj_object = sub_object->contains; obj_object;
	     obj_object = next_obj) {
	    next_obj = obj_object->next_content;
	    if (!CAN_SEE_OBJ(ch, obj_object)) 
		continue;
	    else if (!(fail = can_not_get_item(ch, obj_object))) {
		get(ch, obj_object, sub_object);
		found = TRUE;
	    }
	    else if ( fail > 1 )
		break;
	}
	if (!found && !fail) {
	    sprintf(buffer, "You do not see anything in the %s.\n\r",
		    fname(sub_object->name));
	    send_to_char(buffer, ch);
	}
    }
    else { 		/* get ??? ??? */
	obj_object = get_obj_in_list_vis(ch, arg1, sub_object->contains);
	if (!obj_object) {
	    sprintf(buffer, "The %s does not contain the %s.\n\r",
		    fname(sub_object->name), arg1);
	    send_to_char(buffer, ch);
	}
	else if ( !can_not_get_item(ch, obj_object ))
	    get(ch, obj_object, sub_object);
    }
}


void do_drop(struct char_data *ch, char *argument, int cmd)
{
    char arg[MAX_INPUT_LENGTH];
    int amount;
    char buffer[MAX_BUFSIZ];
    struct obj_data *tmp_object;
    struct obj_data *next_obj;
    bool test = FALSE;
    extern struct obj_data *create_money(int amount);

    argument = one_argument(argument, arg);

    /* NOTE: Cleaned up confusing code  */
    if (!*arg) {
	send_to_char("Drop what?\n\r", ch);
	return;
    }
    if (is_number(arg)) {
	amount = atoi(arg);
	argument = one_argument(argument, arg);
	if (str_cmp("coins", arg) && str_cmp("coin", arg)) {
	    send_to_char("Sorry, you can't do that (yet)...\n\r", ch);
	    return;
	}
	if (amount < 0) {
	    send_to_char("Sorry, you can't do that!\n\r", ch);
	    return;
	}
	if (GET_GOLD(ch) < amount) {
	    send_to_char("You haven't got that many coins!\n\r", ch);
	    return;
	}
	send_to_char("OK.\n\r", ch);
	if (amount == 0)
	    return;
	/* NOTE: minimum gold pile is 1000 coin */
	if (amount >= 1000) {
	    act("$n drops some gold.", FALSE, ch, 0, 0, TO_ROOM);
	    tmp_object = create_money(amount);
	    obj_to_room(tmp_object, ch->in_room);
	}
	else {
	    act("$n scatters coins to winds...", FALSE, ch, 0, 0, TO_ROOM);
	}
	GET_GOLD(ch) -= amount;
    }
    else if (!str_cmp(arg, "all")) {
	for (tmp_object = ch->carrying; tmp_object; tmp_object = next_obj) {
	    next_obj = tmp_object->next_content;
	    if (!IS_SET(tmp_object->obj_flags.extra_flags, ITEM_NODROP)) {
		if (CAN_SEE_OBJ(ch, tmp_object)) {
		    sprintf(buffer, "You drop the %s.\n\r",
			    fname(tmp_object->name));
		    send_to_char(buffer, ch);
		}
		else {
		    send_to_char("You drop something.\n\r", ch);
		}
		act("$n drops $p.", 1, ch, tmp_object, 0, TO_ROOM);
		obj_from_char(tmp_object);
		obj_to_room(tmp_object, ch->in_room);
		test = TRUE;
	    }
	    else if (CAN_SEE_OBJ(ch, tmp_object)) {
		sprintf(buffer, "You can't drop the %s, it must be CURSED!\n\r",
			fname(tmp_object->name));
		send_to_char(buffer, ch);
		test = TRUE;
	    }
	}
	if (!test)
	    send_to_char("You do not seem to have anything.\n\r", ch);
    }
    else { 
	tmp_object = get_obj_in_list_vis(ch, arg, ch->carrying);
	if (!tmp_object) 
	    send_to_char("You do not have that item.\n\r", ch);
	else {
	    if (!IS_SET(tmp_object->obj_flags.extra_flags, ITEM_NODROP)) {
		sprintf(buffer, "You drop the %s.\n\r",
			fname(tmp_object->name));
		send_to_char(buffer, ch);
		act("$n drops $p.", 1, ch, tmp_object, 0, TO_ROOM);
		obj_from_char(tmp_object);
		obj_to_room(tmp_object, ch->in_room);
	    }
	    else 
		send_to_char("You can't drop it, it must be CURSED!\n\r", ch);
	}
    }
}

/* NOTE: NEW! Separated from do_put() */
int perform_put(struct char_data *ch, struct obj_data *obj_object,
	    struct obj_data *sub_object, int bits)
{
    if ((GET_OBJ_WEIGHT(sub_object) + GET_OBJ_WEIGHT(obj_object)) > 
	(sub_object->obj_flags.value[0])) {
	send_to_char("It won't fit.\n\r", ch);
	return FALSE ;
    }
    if (bits == FIND_OBJ_INV) {
	obj_from_char(obj_object);
	/* make up for above line */
	GET_CARRYING_W(ch) += GET_OBJ_WEIGHT(obj_object);
    /*  obj_from_char(sub_object); do not rearrange order... */
	obj_to_obj(obj_object, sub_object);
    /*  obj_to_char(sub_object,ch);do not rearrange order... */
    }
    else {
	obj_from_char(obj_object);
	/* Do we need obj_from_room???(sub_object,....); */
	obj_to_obj(obj_object, sub_object);
	/* Dow we need obj_to_room???(sub_object,ch);    */
    } 
    send_to_char("Ok.\n\r", ch);
    act("$n puts $p in $P", TRUE, ch, obj_object, sub_object, TO_ROOM);
    return TRUE;
}

void do_put(struct char_data *ch, char *argument, int cmd)
{
    char buffer[MAX_BUFSIZ];
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    struct obj_data *obj_object;
    struct obj_data *sub_object;
    struct obj_data *next_object;
    struct char_data *tmp_char;
    int bits;
    char *obj_arg;

    argument_interpreter(argument, arg1, arg2);

    /* NOTE: Too complex code. Clean up it */

    obj_arg = arg1;
    if (!*arg1) {
	send_to_char("Put what in what?\n\r", ch);
	return;
    }
    else if (!*arg2) {
	sprintf(buffer, "Put %s in what?\n\r", arg1);
	send_to_char(buffer, ch);
	return;
    }
    /* NOTE: Little code clean up for 'all.xxx'  handling. */
    else if ( strcmp(arg1, "all") == 0)
	obj_arg = NULL; 
    else if ( strncmp(arg1, "all.all", 7) == 0 || strcmp(arg1, "all.") == 0 ) {
	    send_to_char("What?\n\r", ch);
	    return;
    }
    else if (strncmp(arg1, "all.", 3) == 0) 
	obj_arg = arg1 + 4;

    bits = generic_find(arg2, FIND_OBJ_INV | FIND_OBJ_ROOM,
			ch, &tmp_char, &sub_object);
    if (!sub_object) {
	sprintf(buffer, "You dont have the %s.\n\r", arg2);
	send_to_char(buffer, ch);
	return;
    }
    else if (GET_ITEM_TYPE(sub_object) != ITEM_CONTAINER) {
	sprintf(buffer, "The %s is not a container.\n\r", 
	    fname(sub_object->name));
	send_to_char(buffer, ch);
	return;
    }
    else if (IS_SET(sub_object->obj_flags.value[1], CONT_CLOSED)) {
	send_to_char("It seems to be closed.\n\r", ch);
	return;
    }
    
    obj_object = get_obj_in_list_vis(ch, obj_arg, ch->carrying);
    if (!obj_object) {
	sprintf(buffer, "You dont have the %s.\n\r", obj_arg);
	send_to_char(buffer, ch);
	return;
    }

    if ( strncmp(arg1, "all", 3) == 0) {
	next_object = ch->carrying;
	while ((obj_object = get_obj_in_list_vis(ch, obj_arg, next_object))) {
		next_object = obj_object->next_content;
	    if (obj_object == sub_object)
		continue;
	    if (! perform_put(ch, obj_object, sub_object, bits))
		break;
	} 
	return;
    } 

    if (obj_object == sub_object) 
	send_to_char("You attempt to fold it into itself, but fail.\n\r", ch);
    else 
	perform_put(ch, obj_object, sub_object, bits);
}

/* NOTE: NEW! Separated from do_give() */
void give_gold(struct char_data *ch, struct char_data *vict, int amount )
{ 
    char buf[MAX_BUFSIZ];
    if (amount < 0) {
	send_to_char("Sorry, you can't do that!\n\r", ch);
	return;
    }
    if ((GET_GOLD(ch) < amount) && (IS_NPC(ch) 
	    || (GET_LEVEL(ch) < (IMO + 1)))) {
	send_to_char("You haven't got that many coins!\n\r", ch);
	return;
    }
    /* 
     * NOTE: Don't give money to mob... They don't know value of it.
     * Prohibit it to prevent gold inflation. See group_gain() in fight.c.
     * NOTE: Exception: Darimsa 4th jangro accepts money. See daerimsa.c.
     * Do not activated this code before solving the exception.
     */
    /* 
    if (IS_NPC(vict)) {
	send_to_char("Don't feed mob. It will bite your hand.\n\r",ch);
	return;
    }
    */

    send_to_char("Ok.\n\r", ch);
    sprintf(buf, "%s gives you %d gold coins.\n\r", PERS(ch, vict), amount);
    send_to_char(buf, vict);
    act("$n gives some gold to $N.", 1, ch, 0, vict, TO_NOTVICT);
    if (IS_NPC(ch) || (GET_LEVEL(ch) < (IMO + 1)))
	GET_GOLD(ch) -= amount;
    GET_GOLD(vict) += amount;
}

void do_give(struct char_data *ch, char *argument, int cmd)
{
    char obj_name[MAX_INPUT_LENGTH], vict_name[MAX_INPUT_LENGTH];
    char arg[MAX_INPUT_LENGTH], buf[MAX_BUFSIZ];
    int amount;
    struct char_data *vict;
    struct obj_data *obj;

    argument = one_argument(argument, obj_name);
    if (is_number(obj_name)) {
	argument = one_argument(argument, arg);
	if (strn_cmp("coin", arg, 4) && strn_cmp("gold", arg, 4)) {
	    send_to_char("Sorry, you can't do that (yet)...\n\r", ch);
	    return;
	}
    }
    one_argument(argument, vict_name);
    if (!*obj_name || !*vict_name) {
	send_to_char("Give what to who?\n\r", ch);
	return;
    }
    else if (!(vict = get_char_room_vis(ch, vict_name))) {
	send_to_char(MSGSTR("No-one by that name here..\n\r", ch), ch);
	return;
    }

    if (is_number(obj_name)) {
	amount = atoi(obj_name); 
	give_gold(ch, vict, amount);

	if (GET_LEVEL(ch) >= IMO && !IS_NPC(ch)) {
	    sprintf(buf, "%s gives %d coins to %s.",
		    GET_NAME(ch), amount, GET_NAME(vict));
	    log(buf);
	}
	return;
    }

    if (!(obj = get_obj_in_list_vis(ch, obj_name, ch->carrying))) {
	send_to_char("You do not seem to have anything like that.\n\r",
		     ch);
	return;
    }
    else if (IS_SET(obj->obj_flags.extra_flags, ITEM_NODROP)) {
	send_to_char("You can't let go of it! Yeech!!\n\r", ch);
	return;
    }
    else if ((1 + GET_CARRYING_N(vict)) > CAN_CARRY_N(vict)) {
	act("$N seems to have $S hands full.", 0, ch, 0, vict, TO_CHAR);
	return;
    }
    else if ((GET_OBJ_WEIGHT(obj) + GET_CARRYING_W(vict)) > CAN_CARRY_W(vict)) {
	act("$E can't carry that much weight.", 0, ch, 0, vict, TO_CHAR);
	return;
    }

    obj_from_char(obj);
    obj_to_char(obj, vict);
    act("$n gives $p to $N.", 1, ch, obj, vict, TO_NOTVICT);
    act("$n gives you $p.", 0, ch, obj, vict, TO_VICT);
    send_to_char("Ok.\n\r", ch);

    if (GET_LEVEL(ch) >= IMO && !IS_NPC(ch)) {
	/* NOTE: Added item name to log */
	sprintf(buf, "%s gives %s (%d) to %s." ,
		GET_NAME(ch), obj->name, GET_OBJ_VIRTUAL(obj), GET_NAME(vict));
	log(buf);
    }
}

void do_flick(struct char_data *ch, char *argument, int cmd)
{
    struct char_data *victim;
    struct obj_data *obj;
    char victim_name[240];
    char obj_name[240];
    int eq_pos;

    argument = one_argument(argument, obj_name);
    one_argument(argument, victim_name);
    if (!(victim = get_char_vis(ch, victim_name))) {
	send_to_char("Who?\n\r", ch);
	return;
    }
    else if (victim == ch) {
	send_to_char("Odd?\n\r", ch);
	return;
    }
    else if (GET_LEVEL(ch) <= GET_LEVEL(victim)) {
	send_to_char("Bad idea.\n\r", ch);
	return;
    }
    if (!(obj = get_obj_in_list_vis(victim, obj_name, victim->carrying))) {
	for (eq_pos = 0; (eq_pos < MAX_WEAR); eq_pos++)
	    if (victim->equipment[eq_pos] &&
		(isname(obj_name, victim->equipment[eq_pos]->name))) {
		obj = victim->equipment[eq_pos];
		break;
	    }
	if (!obj) {
	    send_to_char("Can't find that item.\n\r", ch);
	    return;
	}
	else {		/* It is equipment */
	    obj_to_char(unequip_char(victim, eq_pos), ch);
	    send_to_char("Done.\n\r", ch);
	}
    }
    else {		/* obj found in inventory */
	obj_from_char(obj);
	obj_to_char(obj, ch);
	send_to_char("Done.\n\r", ch);
    }
} 

/* functions related to wear */

void perform_wear(struct char_data *ch, struct obj_data *obj_object,
		  int keyword)
{
    switch (keyword) {
    case 0:
	act("$n light $p and holds it.", FALSE, ch, obj_object, 0, TO_ROOM);
	break;
    case 1:
	act("$n wears $p on $s finger.", TRUE, ch, obj_object, 0, TO_ROOM);
	break;
    case 2:
	act("$n wears $p around $s neck.", TRUE, ch, obj_object, 0, TO_ROOM);
	break;
    case 3:
	act("$n wears $p on $s body.", TRUE, ch, obj_object, 0, TO_ROOM);
	break;
    case 4:
	act("$n wears $p on $s head.", TRUE, ch, obj_object, 0, TO_ROOM);
	break;
    case 5:
	act("$n wears $p on $s legs.", TRUE, ch, obj_object, 0, TO_ROOM);
	break;
    case 6:
	act("$n wears $p on $s feet.", TRUE, ch, obj_object, 0, TO_ROOM);
	break;
    case 7:
	act("$n wears $p on $s hands.", TRUE, ch, obj_object, 0, TO_ROOM);
	break;
    case 8:
	act("$n wears $p on $s arms.", TRUE, ch, obj_object, 0, TO_ROOM);
	break;
    case 9:
	act("$n wears $p about $s body.", TRUE, ch, obj_object, 0, TO_ROOM);
	break;
    case 10:
	act("$n wears $p about $s waist.", TRUE, ch, obj_object, 0, TO_ROOM);
	break;
    case 11:
	act("$n wears $p around $s wrist.", TRUE, ch, obj_object, 0, TO_ROOM);
	break;
    case 12:
	act("$n wields $p.", TRUE, ch, obj_object, 0, TO_ROOM);
	break;
    case 13:
	act("$n grabs $p.", TRUE, ch, obj_object, 0, TO_ROOM);
	break;
    case 14:
	act("$n starts using $p as a shield.",
	    TRUE, ch, obj_object, 0, TO_ROOM);
	break;
    case 15:
	act("$n wears $p around $s knee.", TRUE, ch, obj_object, 0, TO_ROOM);
	break;
    case 16:
	act("$n wears $p about $s legs.", TRUE, ch, obj_object, 0, TO_ROOM);
	break;
    case 17:
	act("$n wears $p about $s back.", TRUE, ch, obj_object, 0, TO_ROOM);
	break;
    }
}


void wear(struct char_data *ch, struct obj_data *obj_object, int keyword)
{
    char buffer[MAX_BUFSIZ];
    int gpd;

    gpd = obj_object->obj_flags.gpd;
    if (IS_SET(obj_object->obj_flags.extra_flags, ITEM_NOLOAD)) {
	if (gpd == 0 && (GET_LEVEL(ch) < 10)) {
	    sprintf(buffer, "You are too lowly to use %s.\n\r",
		    obj_object->short_description);
	    send_to_char(buffer, ch);
	    return;
	}
	if (gpd < IMO && (GET_LEVEL(ch) < gpd)) {
	    sprintf(buffer, "You can use %s from %d level.\n\r",
		    obj_object->short_description, gpd);
	    send_to_char(buffer, ch);
	    return;
	}
    }
    switch (keyword) {
    case 0:{		/* LIGHT SOURCE */
	    if (ch->equipment[WEAR_LIGHT])
		send_to_char("You are already holding a light source.\n\r", ch);
	    else {
		send_to_char("Ok.\n\r", ch);
		perform_wear(ch, obj_object, keyword);
		obj_from_char(obj_object);
		equip_char(ch, obj_object, WEAR_LIGHT);
		if (obj_object->obj_flags.value[2])
		    world[ch->in_room].light++;
	    }
	}
	break;

    case 1:{
	    if (CAN_WEAR(obj_object, ITEM_WEAR_FINGER)) {
		if ((ch->equipment[WEAR_FINGER_L]) 
			&& (ch->equipment[WEAR_FINGER_R])) {
		    send_to_char( 
		"You are already wearing something on your fingers.\n\r", ch);
		}
		else {
		    perform_wear(ch, obj_object, keyword);
		    if (ch->equipment[WEAR_FINGER_L]) {
			sprintf(buffer,
				"You put the %s on your right finger.\n\r",
				fname(obj_object->name));
			send_to_char(buffer, ch);
			obj_from_char(obj_object);
			equip_char(ch, obj_object, WEAR_FINGER_R);
		    }
		    else {
			sprintf(buffer,
				"You put the %s on your left finger.\n\r",
				fname(obj_object->name));
			send_to_char(buffer, ch);
			obj_from_char(obj_object);
			equip_char(ch, obj_object, WEAR_FINGER_L);
		    }
		}
	    }
	    else {
		send_to_char("You can't wear that on your finger.\n\r", ch);
	    }
	}
	break;
    case 2:{
	    if (CAN_WEAR(obj_object, ITEM_WEAR_NECK)) {
		if ((ch->equipment[WEAR_NECK_1])
			&& (ch->equipment[WEAR_NECK_2])) {
		    send_to_char(
			"You can't wear any more around your neck.\n\r", ch);
		}
		else {
		    send_to_char("OK.\n\r", ch);
		    perform_wear(ch, obj_object, keyword);
		    if (ch->equipment[WEAR_NECK_1]) {
			obj_from_char(obj_object);
			equip_char(ch, obj_object, WEAR_NECK_2);
		    }
		    else {
			obj_from_char(obj_object);
			equip_char(ch, obj_object, WEAR_NECK_1);
		    }
		}
	    }
	    else {
		send_to_char("You can't wear that around your neck.\n\r", ch);
	    }
	}
	break;
    case 3:{
	    if (CAN_WEAR(obj_object, ITEM_WEAR_BODY)) {
		if (ch->equipment[WEAR_BODY]) {
		    send_to_char(
			"You already wear something on your body.\n\r", ch);
		}
		else {
		    send_to_char("OK.\n\r", ch);
		    perform_wear(ch, obj_object, keyword);
		    obj_from_char(obj_object);
		    equip_char(ch, obj_object, WEAR_BODY);
		}
	    }
	    else {
		send_to_char("You can't wear that on your body.\n\r", ch);
	    }
	}
	break;
    case 4:{
	    if (CAN_WEAR(obj_object, ITEM_WEAR_HEAD)) {
		if (ch->equipment[WEAR_HEAD]) {
		    send_to_char(
			"You already wear something on your head.\n\r", ch);
		}
		else {
		    send_to_char("OK.\n\r", ch);
		    perform_wear(ch, obj_object, keyword);
		    obj_from_char(obj_object);
		    equip_char(ch, obj_object, WEAR_HEAD);
		}
	    }
	    else {
		send_to_char("You can't wear that on your head.\n\r", ch);
	    }
	}
	break;
    case 5:{
	    if (CAN_WEAR(obj_object, ITEM_WEAR_LEGS)) {
		if (ch->equipment[WEAR_LEGS]) {
		    send_to_char(
			"You already wear something on your legs.\n\r", ch);
		}
		else {
		    send_to_char("OK.\n\r", ch);
		    perform_wear(ch, obj_object, keyword);
		    obj_from_char(obj_object);
		    equip_char(ch, obj_object, WEAR_LEGS);
		}
	    }
	    else {
		send_to_char("You can't wear that on your legs.\n\r", ch);
	    }
	}
	break;
    case 6:{
	    if (CAN_WEAR(obj_object, ITEM_WEAR_FEET)) {
		if (ch->equipment[WEAR_FEET]) {
		    send_to_char(
			"You already wear something on your feet.\n\r", ch);
		}
		else {
		    send_to_char("OK.\n\r", ch);
		    perform_wear(ch, obj_object, keyword);
		    obj_from_char(obj_object);
		    equip_char(ch, obj_object, WEAR_FEET);
		}
	    }
	    else {
		send_to_char("You can't wear that on your feet.\n\r", ch);
	    }
	}
	break;
    case 7:{
	    if (CAN_WEAR(obj_object, ITEM_WEAR_HANDS)) {
		if (ch->equipment[WEAR_HANDS]) {
		    send_to_char(
			"You already wear something on your hands.\n\r", ch);
		}
		else {
		    send_to_char("OK.\n\r", ch);
		    perform_wear(ch, obj_object, keyword);
		    obj_from_char(obj_object);
		    equip_char(ch, obj_object, WEAR_HANDS);
		}
	    }
	    else {
		send_to_char("You can't wear that on your hands.\n\r", ch);
	    }
	}
	break;
    case 8:{
	    if (CAN_WEAR(obj_object, ITEM_WEAR_ARMS)) {
		if (ch->equipment[WEAR_ARMS]) {
		    send_to_char(
			"You already wear something on your arms.\n\r", ch);
		}
		else {
		    send_to_char("OK.\n\r", ch);
		    perform_wear(ch, obj_object, keyword);
		    obj_from_char(obj_object);
		    equip_char(ch, obj_object, WEAR_ARMS);
		}
	    }
	    else {
		send_to_char("You can't wear that on your arms.\n\r", ch);
	    }
	}
	break;
    case 9:{
	    if (CAN_WEAR(obj_object, ITEM_WEAR_ABOUT)) {
		if (ch->equipment[WEAR_ABOUT]) {
		    send_to_char(
			"You already wear something about your body.\n\r", ch);
		}
		else {
		    send_to_char("OK.\n\r", ch);
		    perform_wear(ch, obj_object, keyword);
		    obj_from_char(obj_object);
		    equip_char(ch, obj_object, WEAR_ABOUT);
		}
	    }
	    else {
		send_to_char("You can't wear that about your body.\n\r", ch);
	    }
	}
	break;
    case 10:{
	    if (CAN_WEAR(obj_object, ITEM_WEAR_WAISTE)) {
		if (ch->equipment[WEAR_WAISTE]) {
		    send_to_char(
		    "You already wear something about your waiste.\n\r", ch);
		}
		else {
		    send_to_char("OK.\n\r", ch);
		    perform_wear(ch, obj_object, keyword);
		    obj_from_char(obj_object);
		    equip_char(ch, obj_object, WEAR_WAISTE);
		}
	    }
	    else {
		send_to_char("You can't wear that about your waist.\n\r", ch);
	    }
	}
	break;
    case 11:{
	    if (CAN_WEAR(obj_object, ITEM_WEAR_WRIST)) {
		if ((ch->equipment[WEAR_WRIST_L]) && (ch->equipment[WEAR_WRIST_R])) {
		    send_to_char(
		"You already wear something around both your wrists.\n\r", ch);
		}
		else {
		    perform_wear(ch, obj_object, keyword);
		    obj_from_char(obj_object);
		    if (ch->equipment[WEAR_WRIST_L]) {
			sprintf(buffer,
			    "You wear the %s around your right wrist.\n\r",
				fname(obj_object->name));
			send_to_char(buffer, ch);
			equip_char(ch, obj_object, WEAR_WRIST_R);
		    }
		    else {
			sprintf(buffer,
			    "You wear the %s around your left wrist.\n\r",
				fname(obj_object->name));
			send_to_char(buffer, ch);
			equip_char(ch, obj_object, WEAR_WRIST_L);
		    }
		}
	    }
	    else {
		send_to_char("You can't wear that around your wrist.\n\r", ch);
	    }
	}
	break;

    case 15:{
	    if (CAN_WEAR(obj_object, ITEM_WEAR_KNEE)) {
		if ((ch->equipment[WEAR_KNEE_R]) 
			&& (ch->equipment[WEAR_KNEE_L])) {
		    send_to_char( "You already wear something around your both knees.\n\r", ch);
		}
		else {
		    perform_wear(ch, obj_object, keyword);
		    obj_from_char(obj_object);
		    if (ch->equipment[WEAR_KNEE_L]) {
			sprintf(buffer,
				"You put the %s on your right knee.\n\r",
				fname(obj_object->name));
			send_to_char(buffer, ch);
			equip_char(ch, obj_object, WEAR_KNEE_R);
		    }
		    else {
			sprintf(buffer, "You put the %s on your left knee.\n\r",
				fname(obj_object->name));
			send_to_char(buffer, ch);
			equip_char(ch, obj_object, WEAR_KNEE_L);
		    }
		}
	    }
	    else {
		send_to_char(
		    "You cant'wear that on around your both knees.\n\r", ch);
	    }
	}
	break;

    case 16:{
	    if (CAN_WEAR(obj_object, ITEM_WEAR_ABOUTLEGS)) {
		if (ch->equipment[WEAR_ABOUTLEGS]) {
		    send_to_char(
			"You already wear something about your legs.\n\r", ch);
		}
		else {
		    send_to_char("OK.\n\r", ch);
		    perform_wear(ch, obj_object, keyword);
		    obj_from_char(obj_object);
		    equip_char(ch, obj_object, WEAR_ABOUTLEGS);
		}
	    }
	    else {
		send_to_char("You can't wear that about your legs.\n\r", ch);
	    }
	}
	break;
    case 12:
	if (CAN_WEAR(obj_object, ITEM_WIELD)) {
	    if (ch->equipment[WIELD]) {
		send_to_char("You are already wielding something.\n\r", ch);
	    }
	    else {
	    /* Cleric execption has been removed, and is temporarily placed */
	    /* at the end of this file */

		if (GET_OBJ_WEIGHT(obj_object) >
		    str_app[STRENGTH_APPLY_INDEX(ch)].wield_w) {
		    send_to_char("It is too heavy for you to use.\n\r", ch);
		}
		else {
		    send_to_char("OK.\n\r", ch);
		    perform_wear(ch, obj_object, keyword);
		    obj_from_char(obj_object);
		    equip_char(ch, obj_object, WIELD);
		}
	    }
	}
	else {
	    send_to_char("You can't wield that.\n\r", ch);
	}
	break;

    case 13:
	if (CAN_WEAR(obj_object, ITEM_HOLD)) {
	    if (ch->equipment[HOLD]) {
		send_to_char("You are already holding something.\n\r", ch);
	    }
	    else {
	    /* Cleric execption has been removed, and is temporarily placed */
	    /* at the end of this file */

		send_to_char("OK.\n\r", ch);
		perform_wear(ch, obj_object, keyword);
		obj_from_char(obj_object);
		equip_char(ch, obj_object, HOLD);
	    }
	}
	else if (CAN_WEAR(obj_object, ITEM_WIELD) &&
		 (ch->player.class == CLASS_WARRIOR)) {
	    /* Perhaps modified to double wield */
	    if (ch->equipment[HOLD]) {
		send_to_char("You are already holding something.\n\r", ch);
	    }
	    else {
		send_to_char("OK.\n\r", ch);
		perform_wear(ch, obj_object, keyword);
		obj_from_char(obj_object);
		equip_char(ch, obj_object, HOLD);
	    }
	}
	else {
	    send_to_char("You can't hold this.\n\r", ch);
	}
	break;
    case 14:{
	    if (CAN_WEAR(obj_object, ITEM_WEAR_SHIELD)) {
		if ((ch->equipment[WEAR_SHIELD])) {
		    send_to_char(
				  "You are already using a shield\n\r", ch);
		}
		else {
		    perform_wear(ch, obj_object, keyword);
		    sprintf(buffer, "You start using the %s.\n\r",
			    fname(obj_object->name));
		    send_to_char(buffer, ch);
		    obj_from_char(obj_object);
		    equip_char(ch, obj_object, WEAR_SHIELD);
		}
	    }
	    else {
		send_to_char("You can't use that as a shield.\n\r", ch);
	    }
	}
	break;
    case 17:
	if (CAN_WEAR(obj_object, ITEM_WEAR_BACK)) {
	    if ((ch->equipment[WEAR_BACK])) {
		send_to_char("You are already wearing a backpack\n\r", ch);
	    }
	    else {
		send_to_char("Ok!\n\r", ch);
		perform_wear(ch, obj_object, keyword);
		obj_from_char(obj_object);
		equip_char(ch, obj_object, WEAR_BACK);
	    }
	}
	else {
	    send_to_char("You can't use that as a shield.\n\r", ch);
	}
	break;
    case -1:{
	    sprintf(buffer, "Wear %s where?.\n\r", fname(obj_object->name));
	    send_to_char(buffer, ch);
	}
	break;
    case -2:{
	    sprintf(buffer,
		"You can't wear the %s.\n\r", fname(obj_object->name));
	    send_to_char(buffer, ch);
	}
	break;
    default:{
	    log("Unknown type called in wear.");
	}
	break;
    }
}

int where_wear(struct obj_data *obj_object)
{
    if (CAN_WEAR(obj_object, ITEM_WEAR_KNEE))
	return (15);
    if (CAN_WEAR(obj_object, ITEM_WEAR_ABOUTLEGS))
	return (16);
    if (CAN_WEAR(obj_object, ITEM_WEAR_SHIELD))
	return (14);
    if (CAN_WEAR(obj_object, ITEM_WEAR_WRIST))
	return (11);
    if (CAN_WEAR(obj_object, ITEM_WEAR_WAISTE))
	return (10);
    if (CAN_WEAR(obj_object, ITEM_WEAR_ARMS))
	return (8);
    if (CAN_WEAR(obj_object, ITEM_WEAR_HANDS))
	return (7);
    if (CAN_WEAR(obj_object, ITEM_WEAR_FEET))
	return (6);
    if (CAN_WEAR(obj_object, ITEM_WEAR_LEGS))
	return (5);
    if (CAN_WEAR(obj_object, ITEM_WEAR_ABOUT))
	return (9);
    if (CAN_WEAR(obj_object, ITEM_WEAR_HEAD))
	return (4);
    if (CAN_WEAR(obj_object, ITEM_WEAR_BODY))
	return (3);
    if (CAN_WEAR(obj_object, ITEM_WEAR_FINGER))
	return (1);
    if (CAN_WEAR(obj_object, ITEM_WEAR_NECK))
	return (2);
    if (CAN_WEAR(obj_object, ITEM_WEAR_BACK))
	return (17);
    return (-2);
}

void do_wear(struct char_data *ch, char *argument, int cmd)
{
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    char buf[MAX_BUFSIZ];
    struct obj_data *obj_object, *next_object;
    int keyword;
    static char *keywords[] =
    {
	"finger",
	"neck",
	"body",
	"head",
	"legs",
	"aboutlegs",
	"knee",
	"feet",
	"hands",
	"arms",
	"about",
	"waist",
	"wrist",
	"shield",
	"back",
	"\n"
    };

    argument_interpreter(argument, arg1, arg2);

    if (*arg1) {
	if (strcmp(arg1, "all") == 0) {
	    for (obj_object = ch->carrying;
		    obj_object; obj_object = next_object) {
		next_object = obj_object->next_content;
		if (CAN_SEE_OBJ(ch, obj_object)) {
		    keyword = where_wear(obj_object);
		    if (keyword >= 0) 
			wear(ch, obj_object, keyword);
		    /* NOTE: "wear all" will do hold / wield, too */  
		    else if (obj_object->obj_flags.type_flag == ITEM_LIGHT)
			wear(ch, obj_object, WEAR_LIGHT);
		    else if(CAN_WEAR(obj_object, ITEM_HOLD))
			wear(ch, obj_object, 13);
		    else if(CAN_WEAR(obj_object, ITEM_WIELD))
			wear(ch, obj_object, 12);
		    /* NOTE: Double wield not yet supported for "wear all" */  
		    /* Perhaps modified to double wield */
		    /* || ( CAN_WEAR(obj_object, ITEM_WIELD) 
			    && (ch->player.class == CLASS_WARRIOR)) 
			wear(ch, obj_object, 13); */
		}
	    }
	    return;
	}
	obj_object = get_obj_in_list_vis(ch, arg1, ch->carrying);
	if (obj_object) {
	    if (*arg2) {
		keyword = search_block(arg2, keywords, FALSE);
		    /* Partial Match */
		if (keyword == -1) {
		    sprintf(buf, "%s is an unknown body location.\n\r", arg2);
		    send_to_char(buf, ch);
		}
		else {
		    wear(ch, obj_object, keyword + 1);
		}
	    }
	    else {
		keyword = where_wear(obj_object);
		wear(ch, obj_object, keyword);
	    }
	}
	else {
	    sprintf(buf, "You do not seem to have the '%s'.\n\r", arg1);
	    send_to_char(buf, ch);
	}
    }
    else
	send_to_char("Wear what?\n\r", ch);
}

/* NOTE: do_unwield() code merged into do_wield(). Command 'unwield' removed.
	'wield' with no argument will unwield item you are wielding.	*/
void do_wield(struct char_data *ch, char *argument, int cmd)
{
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    char buffer[MAX_BUFSIZ];
    struct obj_data *obj_object;
    int keyword = 12;

    argument_interpreter(argument, arg1, arg2);

    if (*arg1) {
	obj_object = get_obj_in_list_vis(ch, arg1, ch->carrying);
	if (obj_object) {
	    wear(ch, obj_object, keyword);
	}
	else {
	    sprintf(buffer, "You do not seem to have the '%s'.\n\r", arg1);
	    send_to_char(buffer, ch);
	}
    }
    /* NOTE: do_unwield() code here */
    else if (( obj_object = ch->equipment[WIELD] )) {
	if (CAN_CARRY_N(ch) != GET_CARRYING_N(ch)) {
	    act("You stop using $p.", FALSE, ch, obj_object, 0, TO_CHAR);
	    act("$n stops using $p.", TRUE, ch, obj_object, 0, TO_ROOM);
	    obj_to_char(unequip_char(ch, WIELD), ch);
	}
	else 
	    send_to_char("You can't carry that many items.\n\r", ch);
    }
    else
	send_to_char("Wield what?\n\r", ch);
}

/* NOTE: do_unhold() code merged into do_grab(). Command 'unhold' removed.
	'hold' with no argument will unhold item you are holding.	*/
void do_grab(struct char_data *ch, char *argument, int cmd)
{
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    char buffer[MAX_BUFSIZ];
    struct obj_data *obj_object;
    extern struct obj_data *get_obj_in_list(char *name, struct obj_data *list); 

    argument_interpreter(argument, arg1, arg2);
    if (*arg1) {
	obj_object = get_obj_in_list(arg1, ch->carrying);
	if (obj_object) {
	    if (obj_object->obj_flags.type_flag == ITEM_LIGHT)
		wear(ch, obj_object, WEAR_LIGHT);
	    else
		wear(ch, obj_object, 13);
	}
	else {
	    sprintf(buffer, "You do not seem to have the '%s'.\n\r", arg1);
	    send_to_char(buffer, ch);
	}
    }
    /* NOTE: do_unhold() code here */
    else if (( obj_object = ch->equipment[HOLD])) {
	if (CAN_CARRY_N(ch) != GET_CARRYING_N(ch)) {
	    act("You stop using $p.", FALSE, ch, obj_object, 0, TO_CHAR);
	    act("$n stops using $p.", TRUE, ch, obj_object, 0, TO_ROOM);
	    obj_to_char(unequip_char(ch, HOLD), ch);
	}
	else 
	    send_to_char("You can't carry that many items.\n\r", ch);
    }
    else 
	send_to_char("Hold what?\n\r", ch);
}

/* NOTE: NEW! If object is light, Decrement light in room. 
	(Make sure light was in WEAR_LIGHT position)  */
void check_light_off(struct obj_data *obj_object, sh_int room)
{
    if ( obj_object && room >= 0 && GET_ITEM_TYPE(obj_object) == ITEM_LIGHT 
	&& obj_object->obj_flags.value[2] && world[room].light > 0) 
	    world[room].light--;
}

void do_remove(struct char_data *ch, char *argument, int cmd)
{
    char arg1[MAX_INPUT_LENGTH];
    struct obj_data *obj_object;
    int i, j;
    extern struct obj_data *get_object_in_equip_vis(struct char_data *ch,
			    char *arg, struct obj_data **equipment, int *j);

    one_argument(argument, arg1);

    if (*arg1) {
	if (strcmp(arg1, "all")) {
	    obj_object = get_object_in_equip_vis(ch, arg1, ch->equipment, &j);
	    if (obj_object) {
		if (CAN_CARRY_N(ch) != GET_CARRYING_N(ch)) {

		    obj_to_char(unequip_char(ch, j), ch); 
		    /* NOTE: Moved light adjustment to check_light_off(). */
		    check_light_off(obj_object, ch->in_room);
		    /*
		    if (obj_object->obj_flags.type_flag == ITEM_LIGHT)
			if (obj_object->obj_flags.value[2])
			    world[ch->in_room].light--;
		    */ 
		    act("You stop using $p.",
			FALSE, ch, obj_object, 0, TO_CHAR);
		    act("$n stops using $p.", TRUE, ch, obj_object, 0, TO_ROOM);
		}
		else 
		    send_to_char("You can't carry that many items.\n\r", ch);
	    }
	    else
		send_to_char("You are not using it.\n\r", ch);
	}
	else {
	    for (i = 0; i < MAX_WEAR; i++) {
		obj_object = ch->equipment[i];
		if (obj_object && CAN_CARRY_N(ch) != GET_CARRYING_N(ch)) {
		    obj_to_char(unequip_char(ch, i), ch);
		    /* NOTE: Moved light adjustment to check_light_off(). */
		    check_light_off(obj_object, ch->in_room);

		    act("$n unequips $p.", TRUE, ch, obj_object, 0, TO_ROOM);
		    send_to_char("Ok\n\r", ch);
		}
	    }
	}
    }
    else
	send_to_char("Remove what?\n\r", ch);
}

