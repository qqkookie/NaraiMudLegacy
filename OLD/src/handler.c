/* ************************************************************************
   *  file: handler.c , Handler module.                      Part of DIKUMUD *
   *  Usage: Various routines for moving about objects/players               *
   *  Copyright (C) 1990, 1991 - see 'license.doc' for complete information. *
   ************************************************************************* */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "char.h"
#include "object.h"
#include "global.h"
#include "comm.h"
#include "play.h"
#include "etc.h"

/* NOTE: char / object search funtions ( get_char_*(), get_obj_*(), ...)
        are moved to "library.c" */ 

extern char *spell_wear_off_msg[];

void free_char(struct char_data *ch);
void free_obj(struct obj_data *o);

void affect_modify(struct char_data *ch, byte loc, short mod, long bitv, bool add)
{
    int maxabil;

    if (add) {
	SET_BIT(ch->specials.affected_by, bitv);
    }
    else {
	REMOVE_BIT(ch->specials.affected_by, bitv);
	mod = -mod;
    }

    maxabil = (IS_NPC(ch) ? 25 : (GET_LEVEL(ch) >= (IMO + 2) ? 25 : 18));

    switch (loc) {
    case APPLY_NONE: break;
    case APPLY_STR: GET_STR(ch) += mod; break;
    case APPLY_DEX: GET_DEX(ch) += mod; break;
    case APPLY_INT: GET_INT(ch) += mod; break;
    case APPLY_WIS: GET_WIS(ch) += mod; break;
    case APPLY_CON: GET_CON(ch) += mod; break;
    case APPLY_SEX: /* ??? GET_SEX(ch) += mod; */ break;
    case APPLY_CLASS: /* ??? GET_CLASS(ch) += mod; */ break;
    case APPLY_LEVEL: /* ??? GET_LEVEL(ch) += mod; */ break;
    case APPLY_AGE: /* age(ch).year += mod; */ break;
    case APPLY_CHAR_WEIGHT: ch->player.weight += mod; break;
    case APPLY_CHAR_HEIGHT: ch->player.height += mod; break;
    case APPLY_MANA: ch->points.max_mana += mod; break;
    case APPLY_HIT: ch->points.max_hit += mod; break;
    case APPLY_MOVE: ch->points.max_move += mod; break;
    case APPLY_GOLD: break;
    case APPLY_EXP: break;
    case APPLY_AC: GET_AC(ch) += mod; break;
    case APPLY_HITROLL: GET_HITROLL(ch) += mod; break;
    case APPLY_DAMROLL: GET_DAMROLL(ch) += mod; break;
    case APPLY_SAVING_PARA: ch->specials.apply_saving_throw[0] += mod; break;
    case APPLY_SAVING_HIT_SKILL:
	ch->specials.apply_saving_throw[1] += mod;
	break;
    case APPLY_SAVING_PETRI: ch->specials.apply_saving_throw[2] += mod; break;
    case APPLY_SAVING_BREATH: ch->specials.apply_saving_throw[3] += mod; break;
    case APPLY_SAVING_SPELL: ch->specials.apply_saving_throw[4] += mod; break;
    case APPLY_REGENERATION: ch->regeneration += mod; break;
    case APPLY_INVISIBLE:
	if (mod >= 0 && !IS_AFFECTED(ch, AFF_INVISIBLE)) {
	    SET_BIT(ch->specials.affected_by, AFF_INVISIBLE);
	}
	if (mod < 0 && IS_AFFECTED(ch, AFF_INVISIBLE)) {
	    REMOVE_BIT(ch->specials.affected_by, AFF_INVISIBLE);
	}
	break;
    default:

	/* here is test for fixing bug */
	/* sprintf(buf,"loc is %d.",loc); log(buf); */

	/* log("Unknown apply adjust attempt (handler.c, affect_modify)."); */
	break;

    }			/* switch */
}

/* This updates a character by subtracting everything he is affected by */
/* restoring original abilities, and then affecting all again           */
void affect_total(struct char_data *ch)
{
    struct affected_type *af;
    int i, j;

    /* remove all affected by equiped item */
    for (i = 0; i < MAX_WEAR; i++) {
	if (ch->equipment[i])
	    for (j = 0; j < MAX_OBJ_AFFECT; j++)
		affect_modify(ch, ch->equipment[i]->affected[j].location,
			      ch->equipment[i]->affected[j].modifier,
			      ch->equipment[i]->obj_flags.bitvector, FALSE);
    }
    /* remove all affected by spell */
    for (af = ch->affected; af; af = af->next)
	affect_modify(ch, af->location, af->modifier, af->bitvector, FALSE);

    ch->tmpabilities = ch->abilities;

    /* re-affect by item */
    for (i = 0; i < MAX_WEAR; i++) {
	if (ch->equipment[i])
	    for (j = 0; j < MAX_OBJ_AFFECT; j++)
		affect_modify(ch, ch->equipment[i]->affected[j].location,
			      ch->equipment[i]->affected[j].modifier,
			      ch->equipment[i]->obj_flags.bitvector, TRUE);
    }
    /* re-affect by spell */
    for (af = ch->affected; af; af = af->next)
	affect_modify(ch, af->location, af->modifier, af->bitvector, TRUE);

    /* Make certain values are between 0..25, not < 0 and not > 25! */
    i = IS_NPC(ch) ? 25 : 18;
    GET_DEX(ch) = MAX(0, MIN(GET_DEX(ch), i));
    GET_INT(ch) = MAX(0, MIN(GET_INT(ch), i));
    GET_WIS(ch) = MAX(0, MIN(GET_WIS(ch), i));
    GET_CON(ch) = MAX(0, MIN(GET_CON(ch), i));
    GET_STR(ch) = MAX(0, GET_STR(ch));

    if (IS_NPC(ch))
	GET_STR(ch) = MIN(GET_STR(ch), i);
    else {
	if (GET_STR(ch) > 18) {
	    i = GET_ADD(ch) + ((GET_STR(ch) - 18) * 10);
	    GET_ADD(ch) = MIN(i, 100);
	    GET_STR(ch) = 18;
	}
    }
} 

/* Insert an affect_type in a char_data structure
   Automatically sets apropriate bits and apply's */
void affect_to_char(struct char_data *ch, struct affected_type *af)
{
    struct affected_type *affected_alloc;

    if (af->bitvector > 0 && IS_SET(ch->specials.affected_by, af->bitvector))
	return;

    CREATE(affected_alloc, struct affected_type, 1);

    *affected_alloc = *af;
    affected_alloc->next = ch->affected;
    ch->affected = affected_alloc;

    affect_modify(ch, af->location, af->modifier,
		  af->bitvector, TRUE);
    affect_total(ch);
}

/* NOTE: NEW! Special handling when affect is removed. */
/*	Separeted from affect_remove() */
void affect_remove_special(struct char_data *ch, struct affected_type *af )
{
    char buf[200];
    char *tmp;

    if ( !ch || !af )
	return; 
    if (af->type == SPELL_CHARM_PERSON) {
	tmp = strstr(ch->player.short_descr, " CHARMED BY");
	tmp[0] = '\0';
	strcpy(buf, ch->player.short_descr);
	if (ch->player.short_descr) {
	    free(ch->player.short_descr);
	    ch->player.short_descr = 0;
	}
	CREATE(ch->player.short_descr, char, strlen(buf) + 1);

	strcpy(ch->player.short_descr, buf);

	sprintf(buf, "Confused %s is standing here.\n\r",
		ch->player.short_descr);
	if (ch->player.long_descr) {
	    free(ch->player.long_descr);
	    ch->player.long_descr = 0;
	}
	CREATE(ch->player.long_descr, char, strlen(buf) + 1);

	strcpy(ch->player.long_descr, buf);
	/* NOTE: stop follow charmer */
	if( ch->master && af->duration < 0 )
	    stop_follower(ch);
    }
    else if (af->type == SKILL_ARREST ) {
	char_from_room(ch);
	char_to_room(ch, real_room(MID_TEMPLE));
    }
    /* NOTE: Don't die from SPELL_DEATH removal if it is not expired.  */
    else if (af->type == SPELL_DEATH && af->duration < 0 ) {
	/* NOTE: Don't use die(). Religate real daeth to point_update() */
	/* NOTE: If char is dead already, No more death */
	if ( GET_HIT(ch) > 0 && GET_LEVEL(ch) < IMO) {
	    GET_HIT(ch) = -1000;
	    GET_POS(ch) = POS_MORTALLYW;
	}
    }

    return;
}

/* Remove an affected_type structure from a char (called when duration
   reaches zero). Pointer *af must never be NIL! Frees mem and calls 
   affect_location_apply         */
/* NOTE: Warnning! Don't use af->next for ch->affected list traversal.
	*af is free()'d in affect_remove(). 
	So af->next is invalid pointer after calling affect_remove(ch, af).
	Save af->next before calling affect_remove() and use it for traversal.  
*/
void affect_remove(struct char_data *ch, struct affected_type *af)
{
    struct affected_type *hjp;

    if (!ch->affected)
	return;

    affect_modify(ch, af->location, af->modifier, af->bitvector, FALSE);

    /* remove structure *af from linked list */
    if (ch->affected == af) {
	/* remove head of list */
	ch->affected = af->next;
    }
    else {
	for (hjp = ch->affected; (hjp->next) && (hjp->next != af);
	     hjp = hjp->next) ;

	if (hjp->next != af) {
	    log("FATAL : Could not locate affected_type in ch->affected.");
	    abort();
	}
	hjp->next = af->next;	/* skip the af element */
    }

    if (af != NULL) {
	 /* NOTE: Separate code for special affect removal handling to 
		 affect_remove_special(). */ 
	affect_remove_special(ch, af); 

	free(af);
	af = NULL;
    }

    affect_total(ch);
} 

/* Call affect_remove with every spell of spelltype "skill" */
void affect_from_char(struct char_data *ch, byte skill)
{
    struct affected_type *hjp, *hjp_next;

    /* NOTE: hjp->next is invalid pointer. See affect_remove() */
    for (hjp = ch->affected; hjp; hjp = hjp_next) {
	hjp_next = hjp->next;
	if (hjp->type == skill)
	    affect_remove(ch, hjp);
    } 
}


/* Return if a char is affected by a spell (SPELL_XXX), NULL indicates 
   not affected                                                        */
bool affected_by_spell(struct char_data *ch, byte skill)
{
    struct affected_type *hjp;

    for (hjp = ch->affected; hjp; hjp = hjp->next)
	if (hjp->type == skill)
	    return (TRUE);

    return (FALSE);
}


void affect_join(struct char_data *ch, struct affected_type *af,
		 bool avg_dur, bool avg_mod)
{
    struct affected_type *hjp, *hjp_next;
    bool found = FALSE;

    /* NOTE: hjp->next is invalid pointer. See affect_remove() */
    for (hjp = ch->affected; !found && hjp; hjp = hjp_next) {
	hjp_next = hjp->next;
	if (hjp->type == af->type) {
	    af->duration += hjp->duration;
	    if (avg_dur)
		af->duration /= 2;
	    af->modifier += hjp->modifier;
	    if (avg_mod)
		af->modifier /= 2;
	    affect_remove(ch, hjp);
	    affect_to_char(ch, af);
	    found = TRUE;
	}
    }
    if (!found)
	affect_to_char(ch, af);
}

void affect_update(void)
{
    /* NOTE: af, next_af_dude, i were static. No reason for static modifier. */
    struct affected_type *af, *next_af_dude;
    struct char_data *i;

    for (i = character_list; i; i = i->next)
	for (af = i->affected; af; af = next_af_dude) {
	    next_af_dude = af->next;
	    af->duration--;
	    if (af->duration < 0) {
		if (!af->next || (af->next->type != af->type))
		/* NOTE: Is this needed ?? I am not sure. */
		/*    || (af->next->duration < 0)) */
		/*  (af->next->duration > 0)) */
		{
		    send_to_char(spell_wear_off_msg[af->type], i);
		    send_to_char("\n\r", i);
		}
		/* NOTE: call affect_remove() when af->duration < 0 */ 
		affect_remove(i, af);
	    }
	}
}

/* NOTE: NEW! remove all affect from char */
void affect_remove_all(struct char_data *ch )
{
    struct affected_type *hjp, *hjp_next;

    for (hjp = ch->affected; hjp; hjp = hjp_next) {
	hjp_next = hjp->next;
	affect_remove(ch, hjp);
    } 
}

/********************************************************************/

/* move a player out of a room */
void char_from_room(struct char_data *ch)
{
    struct char_data *i; 
    
    if (ch->in_room == NOWHERE) {
	log("NOWHERE char from room (handler.c, char_from_room)");
	return;
	/* NOTE: Special logging for ARRESTED flag removed. */
    }
    /* NOTE: it may be sympthom of programming error */
    if (ch->in_room > top_of_world || ch->in_room < NOWHERE) {
	log("Can't find Player's location");
	return;
    }

    if ((ch->equipment[WEAR_LIGHT])
	&& (GET_ITEM_TYPE(ch->equipment[WEAR_LIGHT]) == ITEM_LIGHT)
	&& (ch->equipment[WEAR_LIGHT]->obj_flags.value[2])) /* Light is ON */
	    world[ch->in_room].light--;

    if (ch == world[ch->in_room].people)	/* head of list */
	world[ch->in_room].people = ch->next_in_room;

    else {		/* locate the previous element */
	/* NOTE: make sure i is not NULL */
	for (i = world[ch->in_room].people; i; i = i->next_in_room) { 
	    if ( i->next_in_room == ch ) {
		i->next_in_room = ch->next_in_room;
		break;
	    }
	ASSERT(i);
	}
    }

    ch->in_room = NOWHERE;
    ch->next_in_room = 0;
}

/* place a character in a room */
void char_to_room(struct char_data *ch, int room)
{
    ch->next_in_room = world[room].people;
    world[room].people = ch;
    ch->in_room = room;
    if ((ch->equipment[WEAR_LIGHT])
	&& (GET_ITEM_TYPE(ch->equipment[WEAR_LIGHT]) == ITEM_LIGHT)
	&& (ch->equipment[WEAR_LIGHT]->obj_flags.value[2])) /* Light is ON */
	    world[room].light++;
}

/* clear ALL the working variables of a char and do NOT free any space alloc'ed */
void clear_char(struct char_data *ch)
{
    bzero(ch, sizeof(struct char_data));

    ch->in_room = NOWHERE;
    ch->specials.was_in_room = NOWHERE;
    ch->specials.position = POS_STANDING;
    ch->specials.default_pos = POS_STANDING;
    GET_AC(ch) = 1;	/* Basic Armor */
}

/* Extract a ch completely from the world, and leave his stuff behind */
void extract_char(struct char_data *ch)
{
    struct char_data *k, *next_char;
    int l;
    /* struct affected_type *af, *af_next; */
    char *tmp, buf[256];
    void wipe_obj(struct obj_data *obj);

    extern void return_original(struct char_data *ch );
    extern int loglevel; 
    extern struct char_data *combat_list; 
    extern void die_follower(struct char_data *ch);

    if (!ch)
	return;

    /* NOTE: Next part is for debugging. comment out when done */
    /* I suspect some mob has invalid name. So SEGV error when it died  */
    tmp = IS_NPC(ch) ? ch->player.short_descr : GET_NAME(ch);
    /* NOTE: Log "extract char ..." selectively.    */
    if (tmp && loglevel == 4 ) {
	sprintf(buf, "extract_char(%s)", tmp);
	log(buf);
    }

    /* NOTE: Next part is for debugging. comment out when done */
    if (ch->in_room == NOWHERE) {
	log("NOWHERE extracting char. extract_char()");
	abort();
    }

    if (ch->followers || ch->master)
	die_follower(ch); 

    if (ch->specials.fighting)
	stop_fighting(ch);
    for (k = combat_list; k; k = next_char) {
	next_char = k->next_fighting;
	if (k->specials.fighting == ch)
	    stop_fighting(k);
    }

    /* Must remove from room before removing the equipment! */
    char_from_room(ch);

    /* NOTE: Wipe out all objects and equips from char and junk them. */
    for (l = 0; l < MAX_WEAR; l++)
	if (ch->equipment[l]) {
	    extract_obj(unequip_char(ch, l));
	    ch->equipment[l] = 0;
	}
    if (ch->carrying) {
	wipe_obj(ch->carrying);
	ch->carrying = 0;
    } 

#ifdef NO_DEF
/* NOTE: make_corpse() already transfered objects to corpse obj */
    if (ch->carrying) {
	/* transfer ch's objects to room */
	if (world[ch->in_room].contents) {	/* room nonempty */
	    /* locate tail of room-contents */
	    for (i = world[ch->in_room].contents; i->next_content;
		 i = i->next_content) ;
	    /* append ch's stuff to room-contents */
	    i->next_content = ch->carrying;
	}
	else
	    world[ch->in_room].contents = ch->carrying;

	/* connect the stuff to the room */
	for (i = ch->carrying; i; i = i->next_content) {
	    i->carried_by = 0;
	    i->in_room = ch->in_room;
	}
    }
    ch->carrying = 0;	/* NOTE: No inventory */

    /* clear equipment_list */
    for (l = 0; l < MAX_WEAR; l++)
	if (ch->equipment[l])
	    obj_to_room(unequip_char(ch, l), was_in);
#endif		/* NO_DEF */

    /* pull the char from the list */
    if (!character_list)
	return;
    else if (ch == character_list)
	character_list = ch->next;
    else {
	for (k = character_list; (k) && (k->next != ch); k = k->next) ;
	if (k)
	    k->next = ch->next;
	else {
	    log("Can't Find character in the list. (handler.c extract_char)");
	}
    }
    /* NOTE: Undo switch and forget snooping separated as proc.  */
    return_original(ch);
    if (IS_NPC(ch)) {
	if (ch->nr >= 0)	/* if mobile */
	    mob_index[ch->nr].number--;
	free_char(ch);
    }
    else { 
	/* remove all affected by spell */
	affect_remove_all(ch);
#ifdef  NO_DEF 
	/* NOTE: This list traversal is incorrect. SEGV error on do_rent()   */
	/*    *af is free()'d in affect_remove(). So, af->next is invalid
	      pointer. save af->next before free() and use it for traversal  */
	for (af = ch->affected; af; af = af_next) {
	    af_next = af->next;
	    affect_remove(ch, af);
	}
	/* NOTE: don't save here, Char must be explicitedly saved by caller */
	ch->in_room = was_in;
	save_char(ch);
	/* NOTE: moved this code to die() and do_rent() */
	ch->desc->connected = CON_RMOTD;
	SEND_TO_Q(MENU, ch->desc);
#endif 		/* NO_DEF */
    }
}

/* release memory allocated for a char struct */
void free_char(struct char_data *ch)
{
    /* struct affected_type *af, *af_next; */

    free(GET_NAME(ch));

    if (GET_TITLE(ch)) {
	free(GET_TITLE(ch));
    }
    if (ch->player.short_descr) {
	free(ch->player.short_descr);
    }
    if (ch->player.long_descr) {
	free(ch->player.long_descr);
    }
    if (ch->player.description) {
	free(ch->player.description);
    }

    /* NOTE: af->next is invalid pointer. See affect_remove() in handler.c */
    /*
    for (af = ch->affected; af; af = af_next) {
	af_next = af->next;
	affect_remove(ch, af);
    }
    */
    affect_remove_all(ch);
    /* NOTE: zero it out to make sure it is not used any more */
    bzero((void *)ch, sizeof(struct char_data));

    free(ch);
} 

/* give an object to a char   */
void obj_to_char(struct obj_data *object, struct char_data *ch)
{
    if (!object)
	return;
    object->next_content = ch->carrying;
    ch->carrying = object;
    object->carried_by = ch;
    object->in_room = NOWHERE;
    GET_CARRYING_W(ch) += GET_OBJ_WEIGHT(object);
    GET_CARRYING_N(ch)++;
}


/* take an object from a char */
void obj_from_char(struct obj_data *object)
{
    struct obj_data *tmp;
    char buf[255];

    if (!object) {
	log("obj_from_char: NULL object");
	return;
    }
    if (!object->carried_by) {
	sprintf(buf, "obj from char: %s has no owner", object->short_description);
	log(buf);
	return;
    }
    if (!object->carried_by->carrying) {
	sprintf(buf, "obj from char: %s has no owner", object->short_description);
	log(buf);
	return;
    }
    if (object->carried_by->carrying == object)		/* head of list */
	object->carried_by->carrying = object->next_content;

    else {
	for (tmp = object->carried_by->carrying;
	     tmp && (tmp->next_content != object);
	     tmp = tmp->next_content) ;		/* locate previous */

	tmp->next_content = object->next_content;
    }

/*
   sprintf(buf,"obj from char: %s %s",GET_NAME(object->carried_by),object->short_description);
   log(buf);
 */
    GET_CARRYING_W(object->carried_by) -= GET_OBJ_WEIGHT(object);
    GET_CARRYING_N(object->carried_by)--;
    object->carried_by = 0;
    object->next_content = 0;
}

/* Return the effect of a piece of armor in position eq_pos */
int apply_ac(struct char_data *ch, int eq_pos)
{
    char buf[128];

    if (!(ch->equipment[eq_pos])) {
	sprintf(buf, "DEBUG: apply_ac(): %s %d", ch->player.name, eq_pos);
	log(buf);
	return (0);
    }
/*assert(ch->equipment[eq_pos]); */
    if (!(GET_ITEM_TYPE(ch->equipment[eq_pos]) == ITEM_ARMOR))
	return 0;
    switch (eq_pos) {
    case WEAR_BODY:
    case WEAR_HEAD:
    case WEAR_LEGS:
    case WEAR_FEET:
    case WEAR_HANDS:
    case WEAR_ARMS:
    case WEAR_SHIELD:
    case WEAR_ABOUT:
    case WEAR_WAISTE:
    case WEAR_WRIST_R:
    case WEAR_WRIST_L:
    case WEAR_KNEE_R:
    case WEAR_KNEE_L:
    case WEAR_ABOUTLEGS:
	return (ch->equipment[eq_pos]->obj_flags.value[0]);
    default:
	break;
    }
    return 0;
}

void equip_char(struct char_data *ch, struct obj_data *obj, int pos)
{
    int j;

    assert(pos >= 0);
    assert(pos < MAX_WEAR);
/*  assert(!(ch->equipment[pos]));  */
    if (ch->equipment[pos]) {
	return;
    }
    if (obj->carried_by) {
	log("EQUIP: Obj is carried_by when equip.");
	return;
    }

    if (obj->in_room != NOWHERE) {
	log("EQUIP: Obj is in_room when equip.");
	return;
    }

    if ((IS_OBJ_STAT(obj, ITEM_ANTI_EVIL) && IS_EVIL(ch)) ||
	(IS_OBJ_STAT(obj, ITEM_ANTI_GOOD) && IS_GOOD(ch)) ||
	(IS_OBJ_STAT(obj, ITEM_ANTI_NEUTRAL) && IS_NEUTRAL(ch)) ||
	(!IS_NPC(ch) && IS_OBJ_STAT(obj, ITEM_ANTI_MAGE) 
	&& (GET_CLASS(ch) == CLASS_MAGIC_USER)) ||
	(!IS_NPC(ch) && IS_OBJ_STAT(obj, ITEM_ANTI_CLERIC) 
	&& (GET_CLASS(ch) == CLASS_CLERIC)) ||
	(!IS_NPC(ch) && IS_OBJ_STAT(obj, ITEM_ANTI_WARRIOR) 
	&& (GET_CLASS(ch) == CLASS_WARRIOR)) ||
	(!IS_NPC(ch) && IS_OBJ_STAT(obj, ITEM_ANTI_THIEF) 
	&& (GET_CLASS(ch) == CLASS_THIEF)) ||
	(!IS_NPC(ch) && IS_OBJ_STAT(obj, ITEM_ANTI_POLICE) 
	&& (GET_GUILD(ch) == POLICE || GET_GUILD(ch) == 0)) ||
	(!IS_NPC(ch) && IS_OBJ_STAT(obj, ITEM_ANTI_OUTLAW) 
	&& (GET_GUILD(ch) == OUTLAW || GET_GUILD(ch) == 0)) ||
	(!IS_NPC(ch) && IS_OBJ_STAT(obj, ITEM_ANTI_ASSASSIN) 
	&& (GET_GUILD(ch) == ASSASSIN || GET_GUILD(ch) == 0))) {
	if (ch->in_room != NOWHERE) {

	    act("You are zapped by $p and instantly drop it.", FALSE, ch, obj, 0, TO_CHAR);
	    act("$n is zapped by $p and instantly drop it.", FALSE, ch, obj, 0, TO_ROOM);
	    obj_to_char(obj, ch);
	    return;
	}
	else {
	    log("ch->in_room = NOWHERE when equipping char.");
	}
    }

    ch->equipment[pos] = obj;

    if (GET_ITEM_TYPE(obj) == ITEM_ARMOR)
	GET_AC(ch) = GET_AC(ch) - (sh_int) apply_ac(ch, pos);

    for (j = 0; j < MAX_OBJ_AFFECT; j++)
	affect_modify(ch, obj->affected[j].location,
		      obj->affected[j].modifier,
		      obj->obj_flags.bitvector, TRUE);

    affect_total(ch);
}

struct obj_data *unequip_char(struct char_data *ch, int pos)
{
    int j;
    struct obj_data *obj;

    assert(pos >= 0 && pos < MAX_WEAR);
    assert(ch->equipment[pos]);

    obj = ch->equipment[pos];
    if (GET_ITEM_TYPE(obj) == ITEM_ARMOR)
	GET_AC(ch) = GET_AC(ch) + (sh_int) apply_ac(ch, pos); 

/* NOTE:  Here is not correct place to adjusting light source no in room. */

    ch->equipment[pos] = 0;

    for (j = 0; j < MAX_OBJ_AFFECT; j++)
	affect_modify(ch, obj->affected[j].location,
		      obj->affected[j].modifier,
		      obj->obj_flags.bitvector, FALSE);

    affect_total(ch);

    return (obj);
}


/* put an object in a room */
void obj_to_room(struct obj_data *object, int room)
{
    object->next_content = world[room].contents;
    world[room].contents = object;
    object->in_room = room;
    object->carried_by = 0;
}


/* Take an object from a room */
void obj_from_room(struct obj_data *object)
{
    struct obj_data *i;

    /* remove object from room */

    if (object == world[object->in_room].contents)	/* head of list */
	world[object->in_room].contents = object->next_content;

    else {		/* locate previous element in list */
	for (i = world[object->in_room].contents; i &&
	     (i->next_content != object); i = i->next_content) ;

	i->next_content = object->next_content;
    }

    object->in_room = NOWHERE;
    object->next_content = 0;
}


/* put an object in an object (quaint)  */
void obj_to_obj(struct obj_data *obj, struct obj_data *obj_to)
{
    struct obj_data *tmp_obj;

    obj->next_content = obj_to->contains;
    obj_to->contains = obj;
    obj->in_obj = obj_to;

    for (tmp_obj = obj->in_obj; tmp_obj;
	 GET_OBJ_WEIGHT(tmp_obj) += GET_OBJ_WEIGHT(obj), tmp_obj = tmp_obj->in_obj) ;
}


/* remove an object from an object */
void obj_from_obj(struct obj_data *obj)
{
    struct obj_data *tmp, *obj_from;

    if (obj->in_obj) {
	obj_from = obj->in_obj;
	if (obj == obj_from->contains)	/* head of list */
	    obj_from->contains = obj->next_content;
	else {
	    for (tmp = obj_from->contains;
		 tmp && (tmp->next_content != obj);
		 tmp = tmp->next_content) ;	/* locate previous */

	    if (!tmp) {
		perror("Fatal error in object structures.");
		abort();
	    }
	    tmp->next_content = obj->next_content;
	}
	/* Subtract weight from containers container */
	for (tmp = obj->in_obj; tmp->in_obj; tmp = tmp->in_obj)
	    GET_OBJ_WEIGHT(tmp) -= GET_OBJ_WEIGHT(obj);
	GET_OBJ_WEIGHT(tmp) -= GET_OBJ_WEIGHT(obj);
	/* Subtract weight from char that carries the object */
	if (tmp->carried_by)
	    GET_CARRYING_W(tmp->carried_by) -= GET_OBJ_WEIGHT(obj);
	obj->in_obj = 0;
	obj->next_content = 0;
    }
    else {
	perror("Trying to object from object when in no object.");
	abort();
    }
}

/* Set all carried_by to point to new owner */
void object_list_new_owner(struct obj_data *list, struct char_data *ch)
{
    if (list) {
	object_list_new_owner(list->contains, ch);
	object_list_new_owner(list->next_content, ch);
	list->carried_by = ch;
    }
}


/* Extract an object from the world */
void extract_obj(struct obj_data *obj)
{
    struct obj_data *temp1, *temp2;

    if (obj->in_room != NOWHERE)
	obj_from_room(obj);
    else if (obj->carried_by)
	obj_from_char(obj);
    else if (obj->in_obj) {
	temp1 = obj->in_obj;
	if (temp1->contains == obj)	/* head of list */
	    temp1->contains = obj->next_content;
	else {
	    for (temp2 = temp1->contains;
		 temp2 && (temp2->next_content != obj);
		 temp2 = temp2->next_content) ;
	    if (temp2) {
		temp2->next_content =
		    obj->next_content;
	    }
	}
    }
    for (; obj->contains; extract_obj(obj->contains)) ;
    /* leaves nothing ! */
    if (object_list == obj)	/* head of list */
	object_list = obj->next;
    else {
	for (temp1 = object_list;
	     temp1 && (temp1->next != obj);
	     temp1 = temp1->next) ;
	if (temp1)
	    temp1->next = obj->next;
    }
    if (obj->item_number >= 0)
	(obj_index[obj->item_number].number)--;
    if (obj != NULL) {
	free_obj(obj);
	obj = NULL;
    }
}


/* release memory allocated for an obj struct */
void free_obj(struct obj_data *obj)
{
    struct extra_descr_data *this, *next_one;

    free(obj->name);
    if (obj->description)
	free(obj->description);
    if (obj->short_description)
	free(obj->short_description);
    if (obj->action_description)
	free(obj->action_description);

    for (this = obj->ex_description; (this != 0); this = next_one) {
	next_one = this->next;
	if (this->keyword)
	    free(this->keyword);
	if (this->description)
	    free(this->description);
	free(this);
	this = NULL;
    }

    free(obj);
}

void wipe_obj(struct obj_data *obj)
{
    if (obj) {
	wipe_obj(obj->contains);
	wipe_obj(obj->next_content);
	if (obj->in_obj)
	    obj_from_obj(obj);
	extract_obj(obj);
    }
}

void update_object(struct obj_data *obj, int use)
{
    if (!obj)
	return;
    if (obj->obj_flags.timer > 0)
	obj->obj_flags.timer -= use;
    if (obj->contains && obj->contains != obj)
	update_object(obj->contains, use);
    if (obj->next_content && obj->next_content != obj)
	update_object(obj->next_content, use);
}

void update_char_objects(struct char_data *ch)
{ 
    int i;

    if (!ch)
	return;
    if (ch->equipment[WEAR_LIGHT])
	if (ch->equipment[WEAR_LIGHT]->obj_flags.type_flag == ITEM_LIGHT)
	    if (ch->equipment[WEAR_LIGHT]->obj_flags.value[2] > 0)
		(ch->equipment[WEAR_LIGHT]->obj_flags.value[2])--;

    for (i = 0; i < MAX_WEAR; i++)
	if (ch->equipment[i])
	    update_object(ch->equipment[i], 2);

    if (ch->carrying)
	update_object(ch->carrying, 1);
} 

void weight_change_object(struct obj_data *obj, int weight)
{
    struct obj_data *tmp_obj;
    struct char_data *tmp_ch;

    if (obj->in_room != NOWHERE) {
	GET_OBJ_WEIGHT(obj) += weight;
    }
    else if ((tmp_ch = obj->carried_by)) {
	obj_from_char(obj);
	GET_OBJ_WEIGHT(obj) += weight;
	obj_to_char(obj, tmp_ch);
    }
    else if ((tmp_obj = obj->in_obj)) {
	obj_from_obj(obj);
	GET_OBJ_WEIGHT(obj) += weight;
	obj_to_obj(obj, tmp_obj);
    }
    else {
	log("Unknown attempt to subtract weight from an object.");
    }
} 

struct obj_data *create_money(int amount)
{
    struct obj_data *obj;
    struct extra_descr_data *new_descr;
    char buf[80];

    if (amount == 0) {
	log("ERROR: Try to create null money.");
	abort();
    }

    if (amount < 0) {	/* cyb */
	amount = MIN(-amount, amount >> 1);
    }

    if (amount < 0) {
	log("ERROR: Try to create negative money.");
	abort();
    }

    CREATE(obj, struct obj_data, 1);
    CREATE(new_descr, struct extra_descr_data, 1);

    clear_object(obj);

    if (amount == 1) {
	obj->name = strdup("coin gold");
	obj->short_description = strdup("a gold coin");
	obj->description = strdup("One miserable gold coin.");

	new_descr->keyword = strdup("coin gold");
	new_descr->description = strdup("One miserable gold coin.");
    }
    else {
	obj->name = strdup("coins gold");
	obj->short_description = strdup("gold coins");
	obj->description = strdup("A pile of gold coins.");

	/* NOTE: Due to hyper inflation, less then 1K coins are worth not
	   much :) */
	new_descr->keyword = strdup("coins gold");
	if (amount < 1000) {
	    sprintf(buf, "There is %d coins.", amount);
	    new_descr->description = strdup(buf);
	}
	else if (amount < 10000) {
	    sprintf(buf, "There is about %d coins", 1000 * (amount / 1000));
	    new_descr->description = strdup(buf);
	}
	else if (amount < 100000) {
	    sprintf(buf, "It looks like something round %d coins", 1000 * (amount / 1000));
	    new_descr->description = strdup(buf);
	}
	/* NOTE: less then 1K coins are not worth counting */
	else if (amount < 1000000) {
	    sprintf(buf, "You guess there is %d coins", (10000 * ((amount / 10000)
						  + number(0, 10) * 1000)));
	    new_descr->description = strdup(buf);
	}
	else
	    new_descr->description = strdup("There is A LOT of coins");
    }

    new_descr->next = 0;
    obj->ex_description = new_descr;

    obj->obj_flags.type_flag = ITEM_MONEY;
    obj->obj_flags.wear_flags = ITEM_TAKE;
    obj->obj_flags.value[0] = amount;
    obj->obj_flags.cost = amount;
    obj->item_number = -1;

    obj->next = object_list;
    object_list = obj;

    return (obj);
}


void clear_object(struct obj_data *obj)
{
    bzero(obj, sizeof(struct obj_data));

    obj->item_number = -1;
    obj->in_room = NOWHERE;
} 

#ifdef NO_DEF
/* NOTE: NEW! remove all objects and equips from char and junk them. */
void wipe_obj_from_char(struct char_data *ch)
{
    int i;
    for (i = 0; i < MAX_WEAR; i++)
	if (ch->equipment[i]) {
	    extract_obj(unequip_char(ch, i));
	    ch->equipment[i] = 0;
	}
    if (ch->carrying) {
	wipe_obj(ch->carrying);
	ch->carrying = 0;
    }
}
#endif		/* NO_DEF */

/* NOTE: NEW! free char and obj and kick player out of game. */
void purge_player(struct char_data *ch)
{ 
    if (IS_NPC(ch))
	return;
    act("$n perishs in towering flame.", FALSE, ch, 0, 0, TO_ROOM);
    extract_char(ch); 
    if (ch->desc){
	ch->desc->connected = CON_CLOSE;
	ch->desc->character = 0; 
    }
    free_char(ch);
}

