/* ************************************************************************
*  file: act.movement.c , Implementation of commands      Part of DIKUMUD *
*  Usage : Movement commands, close/open & lock/unlock,  sleep/wake       *
*  Copyright (C) 1990, 1991 - see 'license.doc' for complete information. *
************************************************************************* */

#include <stdio.h>
#include <string.h>

#include "char.h"
#include "object.h"
#include "global.h"
#include "comm.h"
#include "play.h"
#include "etc.h"

/*   external vars  */
extern int rev_dir[];

int do_simple_move(struct char_data *ch, int cmd, int following)
/* Assumes,
   1. That there is no master and no followers.
   2. That the direction exists.

   Returns :
   1 : If succes.
   0 : If fail
   -1 : If dead.
 */
{
    char tmp[80];
    int was_in, to_room;
    int need_movement;
    struct obj_data *obj;
    bool has_boat, has_wing;
    // int obj_number;

    extern int movement_loss[];
    extern int special(struct char_data *ch, int cmd, char *arg);
    extern int House_can_enter(struct char_data * ch, sh_int house);

    if (special(ch, cmd + 1, ""))/* Check for special routines (North is 1) */
	return (FALSE);

    need_movement = (movement_loss[world[ch->in_room].sector_type] +
		     movement_loss[world[world[ch->in_room].dir_option[cmd]
			 ->to_room].sector_type]) / 2;

    if ((world[ch->in_room].sector_type == SECT_WATER_NOSWIM)
	|| (world[world[ch->in_room].dir_option[cmd]->to_room].sector_type
	    == SECT_WATER_NOSWIM)) {
	has_boat = FALSE;
	/* See if char is carrying a boat */
	for (obj = ch->carrying; obj; obj = obj->next_content)
	    // check  char has boat
	    if (GET_ITEM_TYPE(obj) == ITEM_BOAT)
		has_boat = TRUE;
	if (!has_boat && IS_MORTAL(ch)) {
	    send_to_char("You need a boat to go there.\r\n", ch);
	    return (FALSE);
	}
    }

    if ((world[ch->in_room].sector_type == SECT_SKY)
	|| (world[world[ch->in_room].dir_option[cmd]->to_room].sector_type
	    == SECT_SKY)) {
	has_wing = FALSE;
	/* See if char is carrying a wing */
	for (obj = ch->carrying; obj; obj = obj->next_content) {
	    // obj_number = GET_OBJ_VIRTUAL(obj);
	    // if (obj_number == OBJ_PEGASUS_WINGS || obj_number == OBJ_BIRD_FEATHER)
	    if ( GET_ITEM_TYPE(obj) == ITEM_WINGS
		    || GET_OBJ_VIRTUAL(obj) == OBJ_PEGASUS_WINGS)
		/* Wings of Pegasus  and feather */
		has_wing = TRUE;
	}
	if (!has_wing && NOT_GOD(ch)) {
	    send_to_char("You need wings to fly there.\r\n", ch);
	    return (FALSE);
	}
    }

    to_room = TOROOM(ch->in_room, cmd);
    ASSERT(to_room != NOWHERE);		/* Check validity */

    /* NOTE:  Only wizard level 43 or up can enter OFF_LIMIT room */
    if ( ROOM_FLAGGED( to_room, OFF_LIMITS ) && (GET_LEVEL(ch) < LEV_DEMI )) {
	send_to_char("That room is off-limits!\r\n", ch);
	return (FALSE);
    }
    /* NOTE: For private house. Check ROOM_HOUSE */
    if ( ROOM_FLAGGED( to_room, ROOM_HOUSE )
	    /* (ROOM_FLAGGED(ch->in_room, ROOM_ATRIUM) */
	 && !House_can_enter(ch, to_room)) {
	send_to_char("That's private property -- no trespassing!\r\n", ch);
	return (FALSE);
    }

    if (GET_MOVE(ch) < need_movement && PC_MORTAL(ch)) {
	if (!following)
	    send_to_char("You are too exhausted.\r\n", ch);
	else
	    send_to_char("You are too exhausted to follow.\r\n", ch);

	return (FALSE);
    }

    if (PC_MORTAL(ch))
	GET_MOVE(ch) -= need_movement;

    if (!IS_AFFECTED(ch, AFF_SNEAK)) {
	sprintf(tmp, "$n leaves %s.", dirs[cmd]);
	act(tmp, TRUE, ch, 0, 0, TO_ROOM);
    }

    was_in = ch->in_room;

    char_from_room(ch);
    char_to_room(ch, world[was_in].dir_option[cmd]->to_room);
    if (!IS_AFFECTED(ch, AFF_SNEAK))
	act("$n has arrived.", TRUE, ch, 0, 0, TO_ROOM);
    /* NOTE: NPC don't need to look room. */
    if (!IS_MOB(ch))
	do_look(ch, "\0", 15);
    return (1);
}

void do_move(struct char_data *ch, char *argument, int cmd)
{
    char tmp[80];
    int was_in;
    struct follow_type *k, *next_dude;

    /* NOTE: CMD_NORTH ==1 , CMD_EAST == 2, ... etc. */
    --cmd;

    if (!world[ch->in_room].dir_option[cmd])
	send_to_char("Alas, you cannot go that way...\r\n", ch);
    /* Direction is possible */
    else if (IS_SET(EXIT(ch, cmd)->exit_info, EX_CLOSED)) {
	if (EXIT(ch, cmd)->keyword) {
	    sprintf(tmp, "The %s seems to be closed.\r\n",
		    fname(EXIT(ch, cmd)->keyword));
	    send_to_char(tmp, ch);
	}
	else
	    send_to_char("It seems to be closed.\r\n", ch);
    }
    else if (EXIT(ch, cmd)->to_room == NOWHERE)
	send_to_char("Alas, you can't go that way.\r\n", ch);
    else if (!ch->followers && !ch->master)
	do_simple_move(ch, cmd, FALSE);
    else {

	if (IS_AFFECTED(ch, AFF_CHARM) && (ch->master) &&
	    (ch->in_room == ch->master->in_room)) {
	    send_to_char("The thought of leaving your master makes you weep.\r\n", ch);
	    act("$n bursts into tears.", FALSE, ch, 0, 0, TO_ROOM);
	    return;
	}

	was_in = ch->in_room;
	if ( do_simple_move(ch, cmd, TRUE) && (ch->followers))
	    /* If succes move followers */
	    for (k = ch->followers; k; k = next_dude) {
		next_dude = k->next;
		if ((was_in == k->follower->in_room) &&
		      (GET_POS(k->follower) >= POS_STANDING)) {
		    act("You follow $N.", FALSE, k->follower, 0, ch, TO_CHAR);
		    send_to_char("\r\n", k->follower);
		    do_move(k->follower, argument, cmd+1);
		}
	    }
    }
}

int find_door(struct char_data *ch, char *type, char *dir)
{
    char buf[MAX_STRING_LENGTH];
    int door;
    static char *dirs[] = {
	"north",
	"east",
	"south",
	"west",
	"up",
	"down",
	"\n"
    };

    if (*dir) {		/* a direction was specified */
	/* Partial Match */
	if ((door = search_block(dir, dirs, FALSE)) == -1) {
	    send_to_char("That's not a direction.\r\n", ch);
	    return (-1);
	}

	if (EXIT(ch, door))
	    if (EXIT(ch, door)->keyword)
		if (isname(type, EXIT(ch, door)->keyword))
		    return (door);
		else {
		    sprintf(buf, "I see no %s there.\r\n", type);
		    send_to_char(buf, ch);
		    return (-1);
		}
	    else
		return (door);
	else {
	    send_to_char(
	    "I really don't see how you can close anything there.\r\n", ch);
	    return (-1);
	}
    }
    else {		/* try to locate the keyword */
	for (door = 0; door <= 5; door++)
	    if (EXIT(ch, door))
		if (EXIT(ch, door)->keyword)
		    if (isname(type, EXIT(ch, door)->keyword))
			return (door);

	sprintf(buf, "I see no %s here.\r\n", type);
	send_to_char(buf, ch);
	return (-1);
    }
}


/* NOTE: Similar to find_door(), search_door() checks if there is ANY
   door of that name. (direction ignored)               */
/* NOTE: This is used in all door related functions:
   do_open(), do_close(), do_lock(), do_unlock(). do_pick() */
int search_door(struct char_data *ch, char *type, char *dir)
{
    int door;

    if (!*dir || !strchr("newsud.", *dir))	/* no or doubious direction  */
	if (!isname(type, "door gate grate trapdoor hatch window . room hall"))
	    return (-1);

    /* try to locate the keyword *//* given direction ignored */
    for (door = 0; door <= 5; door++)
	if (EXIT(ch, door) && EXIT(ch, door)->keyword)
	    if (isname(type, EXIT(ch, door)->keyword))
		return (door);

    return (-1);
}

/* NOTE: This applies to all door related  routines:
   do_open(), do_close(), do_lock(), do_unlock(), do_pick().
   These routine searchs door in inventory first , room next, exits last.
   This is wrong. If you have item like 'door key', 'west gate key' in your
   inventory or room, These are confused.  Search door name first!  */
/* NOTE: And they will use half_chop() insted of argument_interpreter()
    to parse input arguments.	*/

void do_open(struct char_data *ch, char *argument, int cmd)
{
    int door, other_room;
    char type[MAX_LINE_LEN], dir[MAX_LINE_LEN], buf[MAX_LINE_LEN];
    struct room_direction_data *back;
    struct obj_data *obj;
    struct char_data *victim;

    half_chop(argument, type, dir);

    if (!*type)
	send_to_char("Open what?\r\n", ch);
    /* NOTE: Search for door first, inv & room next       */
    else if (search_door(ch, type, dir) >= 0)
	goto door;
    else if (generic_find(argument, FIND_OBJ_INV | FIND_OBJ_ROOM,
			  ch, &victim, &obj)) {
	/* this is an object */

	if (obj->obj_flags.type_flag != ITEM_CONTAINER)
	    send_to_char("That's not a container.\r\n", ch);
	else if (!IS_SET(obj->obj_flags.value[1], CONT_CLOSED))
	    send_to_char("But it's already open!\r\n", ch);
	else if (!IS_SET(obj->obj_flags.value[1], CONT_CLOSEABLE))
	    send_to_char("You can't do that.\r\n", ch);
	else if (IS_SET(obj->obj_flags.value[1], CONT_LOCKED))
	    send_to_char("It seems to be locked.\r\n", ch);
	else {
	    REMOVE_BIT(obj->obj_flags.value[1], CONT_CLOSED);
	    send_to_char("Ok.\r\n", ch);
	    act("$n opens $p.", FALSE, ch, obj, 0, TO_ROOM);
	}
	return;
    }

  door: 	/* perhaps it is a door */
    if ((door = find_door(ch, type, dir)) < 0)
	return;
    if (!IS_SET(EXIT(ch, door)->exit_info, EX_ISDOOR))
	send_to_char("That's impossible, I'm afraid.\r\n", ch);
    else if (!IS_SET(EXIT(ch, door)->exit_info, EX_CLOSED))
	send_to_char("It's already open!\r\n", ch);
    else if (IS_SET(EXIT(ch, door)->exit_info, EX_LOCKED))
	send_to_char("It seems to be locked.\r\n", ch);
    else {
	REMOVE_BIT(EXIT(ch, door)->exit_info, EX_CLOSED);
	if (EXIT(ch, door)->keyword)
	    act("$n opens the $F.", FALSE, ch, 0, EXIT(ch, door)->keyword,
		TO_ROOM);
	else
	    act("$n opens the door.", FALSE, ch, 0, 0, TO_ROOM);
	/* NOTE: for automatic door open, show message. */
	if( cmd == 0 )
	    send_to_char("You open it.\r\n", ch);
	else
	    send_to_char("Ok.\r\n", ch);
	/* now for opening the OTHER side of the door! */
	if ((other_room = EXIT(ch, door)->to_room) != NOWHERE
		&& ((back = world[other_room].dir_option[rev_dir[door]]))
		&& (back->to_room == ch->in_room)) {
	    REMOVE_BIT(back->exit_info, EX_CLOSED);
	    if (back->keyword) {
		sprintf(buf, "The %s is opened from the other side.\r\n",
			fname(back->keyword));
		send_to_room(buf, EXIT(ch, door)->to_room);
	    }
	    else
		send_to_room( "The door is opened from the other side.\r\n",
				EXIT(ch, door)->to_room);
	}
    }
}

void do_close(struct char_data *ch, char *argument, int cmd)
{
    int door, other_room;
    char type[MAX_LINE_LEN], dir[MAX_LINE_LEN], buf[MAX_LINE_LEN];
    struct room_direction_data *back;
    struct obj_data *obj;
    struct char_data *victim;

    half_chop(argument, type, dir);
    if (!*type)
	send_to_char("Close what?\r\n", ch);
    /* NOTE: Search for door first, inv & room next       */
    else if (search_door(ch, type, dir) >= 0)
	goto door;
    else if (generic_find(argument, FIND_OBJ_INV | FIND_OBJ_ROOM,
			  ch, &victim, &obj)) {
	/* this is an object */
	if (obj->obj_flags.type_flag != ITEM_CONTAINER)
	    send_to_char("That's not a container.\r\n", ch);
	else if (IS_SET(obj->obj_flags.value[1], CONT_CLOSED))
	    send_to_char("But it's already closed!\r\n", ch);
	else if (!IS_SET(obj->obj_flags.value[1], CONT_CLOSEABLE))
	    send_to_char("That's impossible.\r\n", ch);
	else {
	    SET_BIT(obj->obj_flags.value[1], CONT_CLOSED);
	    send_to_char("Ok.\r\n", ch);
	    act("$n closes $p.", FALSE, ch, obj, 0, TO_ROOM);
	}
	return;
    }

  door: /* Or a door */
    if ((door = find_door(ch, type, dir)) < 0)
	return;
    if (!IS_SET(EXIT(ch, door)->exit_info, EX_ISDOOR))
	send_to_char("That's absurd.\r\n", ch);
    else if (IS_SET(EXIT(ch, door)->exit_info, EX_CLOSED))
	send_to_char("It's already closed!\r\n", ch);
    else {
	SET_BIT(EXIT(ch, door)->exit_info, EX_CLOSED);
	if (EXIT(ch, door)->keyword)
	    act("$n closes the $F.", 0, ch, 0, EXIT(ch, door)->keyword,
		TO_ROOM);
	else
	    act("$n closes the door.", FALSE, ch, 0, 0, TO_ROOM);
	send_to_char("Ok.\r\n", ch);
	/* now for closing the other side, too */
	if ((other_room = EXIT(ch, door)->to_room) != NOWHERE
		&& (back = world[other_room].dir_option[rev_dir[door]])
		&& (back->to_room == ch->in_room)) {
	    SET_BIT(back->exit_info, EX_CLOSED);
	    if (back->keyword) {
		sprintf(buf, "The %s closes quietly.\r\n", back->keyword);
		send_to_room(buf, EXIT(ch, door)->to_room);
	    }
	    else
		send_to_room( "The door closes quietly.\r\n",
				EXIT(ch, door)->to_room);
	}
    }
}

int has_key(struct char_data *ch, int key)
{
    struct obj_data *o;

    for (o = ch->carrying; o; o = o->next_content)
	if (GET_OBJ_VIRTUAL(o) == key)
	    return (1);

    if (ch->equipment[HOLD])
	if (GET_OBJ_VIRTUAL(ch->equipment[HOLD]) == key)
	    return (1);

    /* NOTE: Lesser-God(42) has universal key */
    return (IS_DIVINE(ch));
}

void do_lock(struct char_data *ch, char *argument, int cmd)
{
    int door, other_room;
    char type[MAX_LINE_LEN], dir[MAX_LINE_LEN];
    struct room_direction_data *back;
    struct obj_data *obj;
    struct char_data *victim;

    half_chop(argument, type, dir);

    if (!*type)
	send_to_char("Lock what?\r\n", ch);
    /* NOTE: Search for door first, inv & room next       */
    else if (search_door(ch, type, dir) >= 0)
	goto door;
    else if (generic_find(argument, FIND_OBJ_INV | FIND_OBJ_ROOM,
			  ch, &victim, &obj)) {
	/* this is an object */

	if (obj->obj_flags.type_flag != ITEM_CONTAINER)
	    send_to_char("That's not a container.\r\n", ch);
	else if (!IS_SET(obj->obj_flags.value[1], CONT_CLOSED))
	    send_to_char("Maybe you should close it first...\r\n", ch);
	else if (obj->obj_flags.value[2] < 0)
	    send_to_char("That thing can't be locked.\r\n", ch);
	else if (!has_key(ch, obj->obj_flags.value[2]))
	    send_to_char("You don't seem to have the proper key.\r\n", ch);
	else if (IS_SET(obj->obj_flags.value[1], CONT_LOCKED))
	    send_to_char("It is locked already.\r\n", ch);
	else {
	    SET_BIT(obj->obj_flags.value[1], CONT_LOCKED);
	    send_to_char("*Cluck*\r\n", ch);
	    act("$n locks $p - 'cluck', it says.", FALSE, ch, obj, 0, TO_ROOM);
	}
	return;
    }

  door: /* a door, perhaps */
    if ((door = find_door(ch, type, dir)) < 0)
	return;
    if (!IS_SET(EXIT(ch, door)->exit_info, EX_ISDOOR))
	send_to_char("That's absurd.\r\n", ch);
    else if (!IS_SET(EXIT(ch, door)->exit_info, EX_CLOSED))
	send_to_char("You have to close it first, I'm afraid.\r\n", ch);
    else if (EXIT(ch, door)->key < 0)
	send_to_char("There does not seem to be any keyholes.\r\n", ch);
    else if (!has_key(ch, EXIT(ch, door)->key))
	send_to_char("You don't have the proper key.\r\n", ch);
    else if (IS_SET(EXIT(ch, door)->exit_info, EX_LOCKED))
	send_to_char("It's already locked!\r\n", ch);
    else {
	SET_BIT(EXIT(ch, door)->exit_info, EX_LOCKED);
	if (EXIT(ch, door)->keyword)
	    act("$n locks the $F.", 0, ch, 0, EXIT(ch, door)->keyword, TO_ROOM);
	else
	    act("$n locks the door.", FALSE, ch, 0, 0, TO_ROOM);
	send_to_char("*Click*\r\n", ch);
	/* now for locking the other side, too */
	if ((other_room = EXIT(ch, door)->to_room) != NOWHERE
		 && (back = world[other_room].dir_option[rev_dir[door]])
		 && (back->to_room == ch->in_room))
	    SET_BIT(back->exit_info, EX_LOCKED);
    }
}


void do_unlock(struct char_data *ch, char *argument, int cmd)
{
    int door, other_room;
    char type[MAX_LINE_LEN], dir[MAX_LINE_LEN];
    struct room_direction_data *back;
    struct obj_data *obj;
    struct char_data *victim;

    half_chop(argument, type, dir);

    if (!*type)
	send_to_char("Unlock what?\r\n", ch);
    /* NOTE: Search for door first, inv & room next       */
    else if (search_door(ch, type, dir) >= 0)
	goto door;
    else if (generic_find(argument, FIND_OBJ_INV | FIND_OBJ_ROOM,
			  ch, &victim, &obj)) {
	/* this is an object */

	if (obj->obj_flags.type_flag != ITEM_CONTAINER)
	    send_to_char("That's not a container.\r\n", ch);
	else if (!IS_SET(obj->obj_flags.value[1], CONT_CLOSED))
	    send_to_char("Silly - it ain't even closed!\r\n", ch);
	else if (obj->obj_flags.value[2] < 0)
	    send_to_char("Odd - you can't seem to find a keyhole.\r\n", ch);
	else if (!has_key(ch, obj->obj_flags.value[2]))
	    send_to_char("You don't seem to have the proper key.\r\n", ch);
	else if (!IS_SET(obj->obj_flags.value[1], CONT_LOCKED))
	    send_to_char("Oh.. it wasn't locked, after all.\r\n", ch);
	else {
	    REMOVE_BIT(obj->obj_flags.value[1], CONT_LOCKED);
	    send_to_char("*Click*\r\n", ch);
	    act("$n unlocks $p.", FALSE, ch, obj, 0, TO_ROOM);
	}
	return;
    }

  door: /* it is a door */
    if ((door = find_door(ch, type, dir)) < 0)
	return;
    if (!IS_SET(EXIT(ch, door)->exit_info, EX_ISDOOR))
	send_to_char("That's absurd.\r\n", ch);
    else if (!IS_SET(EXIT(ch, door)->exit_info, EX_CLOSED))
	send_to_char("Heck.. it ain't even closed!\r\n", ch);
    else if (EXIT(ch, door)->key < 0)
	send_to_char("You can't seem to spot any keyholes.\r\n", ch);
    else if (!has_key(ch, EXIT(ch, door)->key))
	send_to_char("You do not have the proper key for that.\r\n", ch);
    else if (!IS_SET(EXIT(ch, door)->exit_info, EX_LOCKED))
	send_to_char("It's already unlocked, it seems.\r\n", ch);
    else {
	REMOVE_BIT(EXIT(ch, door)->exit_info, EX_LOCKED);
	if (EXIT(ch, door)->keyword)
	    act("$n unlocks the $F.", 0, ch, 0, EXIT(ch, door)->keyword,
		TO_ROOM);
	else
	    act("$n unlocks the door.", FALSE, ch, 0, 0, TO_ROOM);
	send_to_char("*click*\r\n", ch);
	/* now for unlocking the other side, too */
	if ((other_room = EXIT(ch, door)->to_room) != NOWHERE
		&& (back = world[other_room].dir_option[rev_dir[door]])
		&& (back->to_room == ch->in_room))
		    REMOVE_BIT(back->exit_info, EX_LOCKED);
	/* NOTE:  Automatic door open upon unlocking it. */
	if (IS_ACTPLR(ch, PLR_AUTOOPEN))
	    do_open( ch, argument, 0 );
    }
}

void do_pick(struct char_data *ch, char *argument, int cmd)
{
    byte percent;
    int door, other_room;
    char type[MAX_LINE_LEN], dir[MAX_LINE_LEN];
    struct room_direction_data *back;
    struct obj_data *obj;
    struct char_data *victim;

    half_chop(argument, type, dir);

    percent = number(1, 101);	/* 101% is a complete failure */

    if (percent > (ch->skills[SKILL_PICK_LOCK].learned)) {
	send_to_char("You failed to pick the lock.\r\n", ch);
	return;
    }

    if (!*type)
	send_to_char("Pick what?\r\n", ch);
    /* NOTE: Search for door first, inv & room next       */
    else if (search_door(ch, type, dir) >= 0)
	goto door;
    else if (generic_find(argument, FIND_OBJ_INV | FIND_OBJ_ROOM,
			  ch, &victim, &obj)) {
	/* this is an object */
	/* furfuri modify... God pick.. It didn't work */
	/* NOTE: Wizard pick.. I made it work.	*/

	if (obj->obj_flags.type_flag != ITEM_CONTAINER)
	    send_to_char("That's not a container.\r\n", ch);
	else if (!IS_SET(obj->obj_flags.value[1], CONT_CLOSED))
	    send_to_char("Silly - it ain't even closed!\r\n", ch);
	else if (obj->obj_flags.value[2] < 0)
	    send_to_char("Odd - you can't seem to find a keyhole.\r\n", ch);
	else if (!IS_SET(obj->obj_flags.value[1], CONT_LOCKED))
	    send_to_char("Oho! This thing is NOT locked!\r\n", ch);
	else if (IS_SET(obj->obj_flags.value[1], CONT_PICKPROOF)
		&& IS_MORTAL(ch))
		send_to_char("It resists your attempt to pick it.\r\n", ch);
	else {
	    if ( IS_SET(obj->obj_flags.value[1], CONT_PICKPROOF))
		send_to_char("Pickproof. But Fufuri picks it for you.\r\n", ch);
	    REMOVE_BIT(obj->obj_flags.value[1], CONT_LOCKED);
	    send_to_char("*Click*\r\n", ch);
	    act("$n fiddles with $p.", FALSE, ch, obj, 0, TO_ROOM);
	}
	return;
    }

  door:
    if ((door = find_door(ch, type, dir)) < 0)
	return;
    if (!IS_SET(EXIT(ch, door)->exit_info, EX_ISDOOR))
	send_to_char("That's absurd.\r\n", ch);
    else if (!IS_SET(EXIT(ch, door)->exit_info, EX_CLOSED))
	send_to_char("You realize that the door is already open.\r\n", ch);
    else if (EXIT(ch, door)->key < 0)
	send_to_char("You can't seem to spot any lock to pick.\r\n", ch);
    else if (!IS_SET(EXIT(ch, door)->exit_info, EX_LOCKED))
	send_to_char("Oh.. it wasn't locked at all.\r\n", ch);
    else if (IS_SET(EXIT(ch, door)->exit_info, EX_PICKPROOF)
	    && IS_MORTAL(ch))
	    send_to_char("You seem to be unable to pick this lock.\r\n",ch);
    else {
	if (IS_SET(EXIT(ch, door)->exit_info, EX_PICKPROOF))
	    send_to_char("Pickproof. But Cookie picks it for you.\r\n", ch);
	REMOVE_BIT(EXIT(ch, door)->exit_info, EX_LOCKED);
	if (EXIT(ch, door)->keyword)
	    act("$n skillfully picks the lock of the $F.", 0, ch, 0,
		EXIT(ch, door)->keyword, TO_ROOM);
	else
	    act("$n picks the lock of obscure keyhole",
		TRUE, ch, 0, 0, TO_ROOM);
	send_to_char("The lock quickly yields to your skills.\r\n", ch);
	/* now for unlocking the other side, too */
	if ((other_room = EXIT(ch, door)->to_room) != NOWHERE
		&& (back = world[other_room].dir_option[rev_dir[door]])
		&& (back->to_room == ch->in_room))
	    REMOVE_BIT(back->exit_info, EX_LOCKED);
    }
}

void do_enter(struct char_data *ch, char *argument, int cmd)
{
    int door;
    char buf[MAX_LINE_LEN], tmp[MAX_OUT_LEN];

    one_argument(argument, buf);

    if (*buf) {		/* an argument was supplied, search for door keyword */
	for (door = 0; door <= 5; door++)
	    if (EXIT(ch, door) && (EXIT(ch, door)->keyword))
		if (!str_cmp(EXIT(ch, door)->keyword, buf)) {
		    do_move(ch, "", ++door);
		    return;
		}
	sprintf(tmp, "There is no %s here.\r\n", buf);
	send_to_char(tmp, ch);
    }
    else if (IS_SET(world[ch->in_room].room_flags, INDOORS))
	send_to_char("You are already indoors.\r\n", ch);
    else {
	/* try to locate an entrance */
	for (door = 0; door <= 5; door++)
	    if (EXIT(ch, door) && (EXIT(ch, door)->to_room != NOWHERE)
		    && !IS_SET(EXIT(ch, door)->exit_info, EX_CLOSED)
		&& IS_SET(world[EXIT(ch, door)->to_room].room_flags, INDOORS)) {
		do_move(ch, "", ++door);
		return;
	    }
	send_to_char("You can't seem to find anything to enter.\r\n", ch);
    }
}

void do_leave(struct char_data *ch, char *argument, int cmd)
{
    int door;

    if (!IS_SET(world[ch->in_room].room_flags, INDOORS))
	send_to_char("You are outside.. where do you want to go?\r\n", ch);
    else {
	for (door = 0; door <= 5; door++)
	    if (EXIT(ch, door) && (EXIT(ch, door)->to_room != NOWHERE)
		    && !IS_SET(EXIT(ch, door)->exit_info, EX_CLOSED)
		&& !IS_SET(world[EXIT(ch, door)->to_room].room_flags, INDOORS)) {
		do_move(ch, "", ++door);
		return;
	    }
	send_to_char("I see no obvious exits to the outside.\r\n", ch);
    }
}

void do_where(struct char_data *ch, char *argument, int cmd)
{
    char name[MAX_LINE_LEN], buffer[MAX_STR_LEN], buf2[MAX_LINE_LEN];
    struct char_data *i, *vic;
    int j, n;
    struct obj_data *k;
    struct descriptor_data *d;
    char  *buf = buffer;

    /* NOTE: Some code clean up */
    /* NOTE: use page_string(), especially for IMO */
    one_argument(argument, name);
    n = 0; *buf = '\0';
    if ( !*name ) {
	for (d = descriptor_list; d; d = d->next) {
	    vic = d->character;
	    if( !vic || d->connected != CON_PLYNG || vic->in_room == NOWHERE )
		continue;
	    /* NOTE: check page buffer overflow */
	    if (buf >  buffer + sizeof(buffer) - 100)
		break;
	    if (IS_MORTAL(ch)) {
		if (( world[vic->in_room].zone != world[ch->in_room].zone )
		    || ((!CAN_SEE(ch, vic) || IS_AFFECTED(vic, AFF_HIDE)
			|| IS_AFFECTED(vic, AFF_SHADOW_FIGURE)) && ch != vic ))
		    continue;

	    /* NOTE: Did you omitted room number deliberately or by mistake? */
	    /* NOTE: I guess it is not avilable to mortal player */
		sprintf(buf, "%-20s - %20s\r\n", d->character->player.name,
			world[d->character->in_room].name);
		    /* , world[d->character->in_room].number ); */
	    }
	    else if (d->original)	/* If switched */
		sprintf(buf, "%-20s - %s [%d] In body of %s\r\n",
			d->original->player.name,
			world[d->character->in_room].name,
			world[d->character->in_room].number,
			fname(d->character->player.name));
	    else
		sprintf(buf, "%-20s - %s [%d]\r\n",
			d->character->player.name,
			world[d->character->in_room].name,
			world[d->character->in_room].number);
	    buf += strlen(buf);
	}
	page_string( ch->desc, buffer, 1 );
	return;
    }

    for (i = character_list; i; i = i->next) {
	if ((i->in_room == NOWHERE) || !isname(name, i->player.name)
	    || (IS_MORTAL(ch) && ( i != ch )
		&& ( !CAN_SEE(ch, i) || IS_AFFECTED(i, AFF_SHADOW_FIGURE)
		    || (world[i->in_room].zone != world[ch->in_room].zone))))
	    continue;

	if (IS_MORTAL(ch)) {
	    sprintf(buf, "    %-30s- %s\r\n",
		( IS_NPC(i) ? i->player.short_descr : i->player.name ),
		    world[i->in_room].name );
	    send_to_char(buf, ch);
	    return;
	}
	else {
	    /* NOTE: check page buffer overflow */
	    if (buf >  buffer + sizeof(buffer) - 100)
		break;
	    sprintf(buf, "%2d: %-30s- %s [%d]\r\n", ++n,
		( IS_NPC(i) ? i->player.short_descr : i->player.name ),
		    world[i->in_room].name, world[i->in_room].number);
	    buf += strlen(buf);
	}
    }

    if (IS_MORTAL(ch)) {
	send_to_char("Couldn't find any such thing.\r\n", ch);
	return;
    }
    for (i = character_list; i; i = i->next)
	for (j = 0; j < MAX_WEAR; ++j)
	    if (i->equipment[j] && isname(name, i->equipment[j]->name)) {
		if (buf >  buffer + sizeof(buffer) - 100)
		    break;
		sprintf(buf, "%2d: %s worn by %s.\r\n", ++n,
		    i->equipment[j]->short_description, GET_NAME(i));
		buf += strlen(buf);
	    }

    for (k = object_list; k; k = k->next)
	if (isname(name, k->name)) {
	    if (buf >  buffer + sizeof(buffer) - 100)
		break;
	    if (k->carried_by)
		sprintf(buf, "%2d: %s carried by %s.\r\n",
		    ++n, k->short_description, PERS(k->carried_by, ch));
	    else if (k->in_obj && k->in_obj->carried_by)
		sprintf(buf2, "%2d: %s in %s carried by %s.\r\n", ++n,
		    k->short_description, k->in_obj->short_description,
		    PERS(k->in_obj->carried_by, ch));
	    else if (k->in_obj && !(k->in_obj->carried_by))
		sprintf(buf2, "%2d: %s in %s.\r\n", ++n,
		    k->short_description, k->in_obj->short_description );
	    else if (k->in_room != NOWHERE)
		sprintf(buf, "%2d: %s in %s [%d].\r\n", ++n,
		    k->short_description,
		    world[k->in_room].name, world[k->in_room].number);
	    buf += strlen(buf);
	}
    if (*buffer)
	page_string(ch->desc, buffer, 1 );
    else
	send_to_char("Couldn't find any such thing.\r\n", ch);
}

void do_exits(struct char_data *ch, char *argument, int cmd)
{
    int door, rn, round, lev;
    char buf[MAX_STRING_LENGTH];
    static char *exits[] = {
	"North",
	"East ",
	"South",
	"West ",
	"Up   ",
	"Down "
    };

    strcpy(buf, STRHAN("Obvious exits: ", "명백한 출구는: ", ch));
    /* NOTE: Show current virtual room number guess with some probalility */
    rn = world[ch->in_room].number;
    /* NOTE: add some perturbation (make Gaussian distribution) */
    if (!IS_DIVINE(ch)) {
	lev = GET_LEVEL(ch);
	for (round = 0; round < lev / 4 + 2; round++)
	    rn += number(-12 + lev / 4, 12 - lev / 4);
    }
    sprintf(buf + strlen(buf), "  ROOM ???  [%5d]\r\n", rn);

    for (door = 0; door <= 5; door++)
	if (EXIT(ch, door))
	    if (EXIT(ch, door)->to_room != NOWHERE &&
		!IS_SET(EXIT(ch, door)->exit_info, EX_CLOSED)) {
		if (IS_WIZARD(ch))
		    sprintf(buf + strlen(buf), "%-5s - [%5d] %s\r\n",
			 exits[door], world[EXIT(ch, door)->to_room].number,
			    world[EXIT(ch, door)->to_room].name);
		else if (IS_DARK(EXIT(ch, door)->to_room) && !IS_NPC(ch))
		    sprintf(buf + strlen(buf), "%s - Too dark to tell\r\n",
		    	    exits[door]);
		else
		    sprintf(buf + strlen(buf), "%s - %s\r\n", exits[door],
			    world[EXIT(ch, door)->to_room].name);
	    }

    send_to_char(( *buf ? buf : "None.\r\n"), ch);
}

void do_stand(struct char_data *ch, char *argument, int cmd)
{
    switch (GET_POS(ch)) {
    case POS_STANDING:
	act("You are already standing.", FALSE, ch, 0, 0, TO_CHAR);
	break;
    case POS_SITTING:
	act("You stand up.", FALSE, ch, 0, 0, TO_CHAR);
	act("$n clambers on $s feet.", TRUE, ch, 0, 0, TO_ROOM);
	GET_POS(ch) = POS_STANDING;
	break;
    case POS_RESTING:
	act("You stop resting, and stand up.", FALSE, ch, 0, 0, TO_CHAR);
	act("$n stops resting, and clambers on $s feet.",
	    TRUE, ch, 0, 0, TO_ROOM);
	GET_POS(ch) = POS_STANDING;
	break;
    case POS_SLEEPING:
	act("You have to wake up first!", FALSE, ch, 0, 0, TO_CHAR);
	break;
    case POS_FIGHTING:
	act("Do you not consider fighting as standing?",
	    FALSE, ch, 0, 0, TO_CHAR);
	break;
    default:
	act("You stop floating around, and put your feet on the ground.",
	    FALSE, ch, 0, 0, TO_CHAR);
	act("$n stops floating around, and puts $s feet on the ground.",
	    TRUE, ch, 0, 0, TO_ROOM);
	break;
    }
}

void do_sit(struct char_data *ch, char *argument, int cmd)
{
    switch (GET_POS(ch)) {
    case POS_STANDING:
	act("You sit down.", FALSE, ch, 0, 0, TO_CHAR);
	act("$n sits down.", FALSE, ch, 0, 0, TO_ROOM);
	GET_POS(ch) = POS_SITTING;
	break;
    case POS_SITTING:
	send_to_char("You'r sitting already.\r\n", ch);
	break;
    case POS_RESTING:
	act("You stop resting, and sit up.", FALSE, ch, 0, 0, TO_CHAR);
	act("$n stops resting.", TRUE, ch, 0, 0, TO_ROOM);
	GET_POS(ch) = POS_SITTING;
	break;
    case POS_SLEEPING:
	act("You have to wake up first.", FALSE, ch, 0, 0, TO_CHAR);
	break;
    case POS_FIGHTING:
	act("Sit down while fighting? are you MAD?", FALSE, ch, 0, 0, TO_CHAR);
	break;
    default:
	act("You stop floating around, and sit down.",
	    FALSE, ch, 0, 0, TO_CHAR);
	act("$n stops floating around, and sits down.",
	    TRUE, ch, 0, 0, TO_ROOM);
	GET_POS(ch) = POS_SITTING;
	break;
    }
}

void do_rest(struct char_data *ch, char *argument, int cmd)
{
    switch (GET_POS(ch)) {
    case POS_STANDING:
	act("You sit down and rest your tired bones.",
	    FALSE, ch, 0, 0, TO_CHAR);
	act("$n sits down and rests.", TRUE, ch, 0, 0, TO_ROOM);
	GET_POS(ch) = POS_RESTING;
	break;
    case POS_SITTING:
	act("You rest your tired bones.", FALSE, ch, 0, 0, TO_CHAR);
	act("$n rests.", TRUE, ch, 0, 0, TO_ROOM);
	GET_POS(ch) = POS_RESTING;
	break;
    case POS_RESTING:
	act("You are already resting.", FALSE, ch, 0, 0, TO_CHAR);
	break;
    case POS_SLEEPING:
	act("You have to wake up first.", FALSE, ch, 0, 0, TO_CHAR);
	break;
    case POS_FIGHTING:
	act("Rest while fighting? are you MAD?", FALSE, ch, 0, 0, TO_CHAR);
	break;
    default:
	act("You stop floating around, and stop to rest your tired bones.",
	    FALSE, ch, 0, 0, TO_CHAR);
	act("$n stops floating around, and rests.", FALSE, ch, 0, 0, TO_ROOM);
	GET_POS(ch) = POS_SITTING;
	break;
    }
}

void do_sleep(struct char_data *ch, char *argument, int cmd)
{
    switch (GET_POS(ch)) {
    case POS_STANDING:
    case POS_SITTING:
    case POS_RESTING:
	send_to_char("자러 갑니다. 아이 졸려\r\n", ch);
	act("$n님이 눕더니 잠이 듭니다.", TRUE, ch, 0, 0, TO_ROOM);
	GET_POS(ch) = POS_SLEEPING;
	break;
    case POS_SLEEPING:
	send_to_char("쿨쿨...\r\n", ch);
	break;
    case POS_FIGHTING:
	send_to_char("싸우면서 잔다구요 ? 지금 정신이 있습니까 ?\r\n", ch);
	break;
    default:
	act("You stop floating around, and lie down to sleep.",
	    FALSE, ch, 0, 0, TO_CHAR);
	act("$n stops floating around, and lie down to sleep.",
	    TRUE, ch, 0, 0, TO_ROOM);
	GET_POS(ch) = POS_SLEEPING;
	break;
    }
}

void do_wake(struct char_data *ch, char *argument, int cmd)
{
    struct char_data *tmp_char;
    char arg[MAX_LINE_LEN];

    one_argument(argument, arg);
    /* NOTE: Some code simplification. */
    if (*arg) {
	if (GET_POS(ch) == POS_SLEEPING)
	    act("You can't wake people up if you are asleep yourself!",
		FALSE, ch, 0, 0, TO_CHAR);
	else if (!( tmp_char = get_char_room_vis(ch, arg)))
	    send_to_char("You do not see that person here.\r\n", ch);
	else if (tmp_char == ch)
	    act("If you want to wake yourself up, just type 'wake'",
		FALSE, ch, 0, 0, TO_CHAR);
	else if (GET_POS(tmp_char) > POS_SLEEPING)
	    act("$N is already awake.", FALSE, ch, 0, tmp_char, TO_CHAR);
	else if (IS_AFFECTED(tmp_char, AFF_SLEEP))
	    act("You can not wake $M up!", FALSE, ch, 0, tmp_char, TO_CHAR);
	else {
	    act("You wake $M up.", FALSE, ch, 0, tmp_char, TO_CHAR);
	    GET_POS(tmp_char) = POS_SITTING;
	    act("You are awakened by $n.", FALSE, ch, 0, tmp_char, TO_VICT);
	}
	return;
    }
    else if (IS_AFFECTED(ch, AFF_SLEEP))
	    send_to_char("You can't wake up!\r\n", ch);
    else if (GET_POS(ch) > POS_SLEEPING)
	send_to_char("You are already awake...\r\n", ch);
    else {
	/* NOTE: When you wake up, you are resting, not sitting. */
	/*
	send_to_char("You wake, and sit up.\r\n", ch);
	GET_POS(ch) = POS_SITTING;
	 */
	send_to_char("You wake, and rest.\r\n", ch);
	GET_POS(ch) = POS_RESTING;
	act("$n awakens.", TRUE, ch, 0, 0, TO_ROOM);
    }
}

