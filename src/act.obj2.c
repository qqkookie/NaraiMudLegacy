/* ************************************************************************
*  file: act.obj2.c , Implementation of commands.         Part of DIKUMUD *
*  Usage : Commands eating/drinking or using or shopping object. 	  *
*  Copyright (C) 1990, 1991 - see 'license.doc' for complete information. *
*									  *
*  NOTE: Merged OLD shop.c and OLD newshop.h, newshop.c (unused)	  *
************************************************************************* */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>


#include "char.h"
#include "object.h"
#include "global.h"
#include "comm.h"
#include "play.h"
#include "actions.h"


/* extern variables/procs. */
extern char *drinks[];
extern int drink_aff[][3];
extern void gain_condition(struct char_data *ch, int condition, int value);
extern void weight_change_object(struct obj_data *obj, int weight);

void name_from_drinkcon(struct obj_data *obj)
{
    int i;
    char *new_name;

    for (i = 0; (*((obj->name) + i) != ' ') && (*((obj->name) + i) != '\0'); i++) ;

    if (*((obj->name) + i) == ' ') {
	new_name = strdup((obj->name) + i + 1);
	free(obj->name);
	obj->name = new_name;
    }
} 

void name_to_drinkcon(struct obj_data *obj, int type)
{
    char *new_name;
    extern char *drinknames[];

    CREATE(new_name, char, strlen(obj->name) + strlen(drinknames[type]) + 2);

    sprintf(new_name, "%s %s", drinknames[type], obj->name);
    free(obj->name);
    obj->name = new_name;
} 

/* NOTE: do_drink(), do_sip() have merged. */
void do_drink(struct char_data *ch, char *argument, int cmd)
{
    char buf[MAX_BUFSIZ];
    struct obj_data *temp;
    struct affected_type af;
    int amount, liq;
    extern void cast_heal(byte level, struct char_data *ch, char *arg,
    	int si, struct char_data *tar_ch, struct obj_data *tar_obj);
    extern void cast_cure_critic(byte level, struct char_data *ch, char *arg,
    	int si, struct char_data *tar_ch, struct obj_data *tar_obj);

    one_argument(argument, buf);
    if (!*buf) {
	send_to_char("Drink what?\r\n", ch);
	return; 
    }
    else if (!strcmp("sundew", buf))
	temp = get_obj_in_list_vis(ch, buf, world[ch->in_room].contents);
    else 
	temp = get_obj_in_list_vis(ch, buf, ch->carrying);

    if (!temp) 
	act("You can't find it!", FALSE, ch, 0, 0, TO_CHAR);
    else if (temp->obj_flags.type_flag != ITEM_DRINKCON) 
	act("You can't drink from that!", FALSE, ch, 0, 0, TO_CHAR); 

    /* NOTE: raised drunk level to fail to drink */
    /* 	OLD: sip 10, drink 20. NEW: sip 30, drink 25 */
    else if ( GET_COND(ch, DRUNK) > ((cmd == CMD_SIP) ? 30 : 25 ) ) {
	/* The pig is drunk */
	act("You simply fail to reach your mouth!", FALSE, ch, 0, 0, TO_CHAR);
	act("$n tried to drink but missed $s mouth!", TRUE, ch, 0, 0, TO_ROOM);
    } 
    else if ( cmd != CMD_SIP && 
	    (GET_COND(ch, FULL) > 32) && (GET_COND(ch, THIRST) > 0))
	/* Stomach full */
	act("Your stomach can't contain anymore!", FALSE, ch, 0, 0, TO_CHAR);
    else if (temp->obj_flags.value[1] <= 0) 	/* empty */
	act("It's empty already.", FALSE, ch, 0, 0, TO_CHAR);
    else 
	goto drink_ok ;
    return;

drink_ok:
    liq = temp->obj_flags.value[2]; 
    sprintf(buf, "$n drinks %s from $p.", drinks[liq]);
    act(buf, TRUE, ch, temp, 0, TO_ROOM); 

    if ( cmd == CMD_SIP ) {
	sprintf(buf, "It tastes like %s.\n\r", drinks[liq]);
	send_to_char(buf, ch);
	amount = 1;
    }
    else {
	sprintf(buf, "You drink the %s.\r\n", drinks[liq]);
	send_to_char(buf, ch);
	if ( liq == LIQ_NECTAR)
	    cast_cure_critic(GET_LEVEL(ch), ch, "", SPELL_TYPE_POTION, ch, 0);
	else if ( liq == LIQ_GOLDEN_NECTAR)
	    cast_heal(GET_LEVEL(ch), ch, "", SPELL_TYPE_POTION, ch, 0); 

	if ( liq == LIQ_SUNDEW || liq == LIQ_WATER )
	    amount = 12; 
	else  if (drink_aff[liq][DRUNK] > 0 )
	    amount = ( 50 - GET_COND(ch,THIRST)) / (drink_aff[liq][DRUNK]+3);
	else
	    amount = number(3,10);
	amount = MIN(amount, temp->obj_flags.value[1]);
    }

    weight_change_object(temp, -amount);	/* Subtract amount */

    gain_condition(ch, DRUNK, ( drink_aff[liq][DRUNK] * amount) / 4); 
    gain_condition(ch, FULL, ( drink_aff[liq][FULL] * amount) / 4); 
    gain_condition(ch, THIRST, ( drink_aff[liq][THIRST] * amount) / 4);

    if (GET_COND(ch, DRUNK) > 20)
	act("You feel drunk.", FALSE, ch, 0, 0, TO_CHAR); 
    if (GET_COND(ch, THIRST) > 32)
	act("You do not feel thirsty.", FALSE, ch, 0, 0, TO_CHAR); 
    if (GET_COND(ch, FULL) > 32)
	act("You are full.", FALSE, ch, 0, 0, TO_CHAR);

    switch (temp->obj_flags.value[3]) {
    case 1:	/* The shit was poisoned ! */
	act("Ooups, it tasted rather strange ?!!?", FALSE, ch, 0, 0, TO_CHAR);
	act("$n chokes and utters some strange sounds.",
	    TRUE, ch, 0, 0, TO_ROOM);
	af.type = SPELL_POISON;
	af.duration = amount * 3;
	af.modifier = 0;
	af.location = APPLY_NONE;
	af.bitvector = AFF_POISON;
	affect_join(ch, &af, FALSE, FALSE);
	break;

	/* NOTE: consolidate affection power about 30-50 times or so. */
    case 2:	/* hit point restore */
	act("You feel momentarily warm.", FALSE, ch, 0, 0, TO_CHAR);
	acthan("$n blushes.", "$n님의 얼굴이 붉어집니다.",
	       TRUE, ch, 0, 0, TO_ROOM);
	GET_HIT(ch) = MIN(GET_MAX_HIT(ch), GET_HIT(ch) +
	       dice(drink_aff[liq][DRUNK], 50*amount));
	       /* dice(drink_aff[liq][DRUNK], 8));*/
	break;
    case 3:	/* mana restore */
	act("You feel momentarily stimulated.", FALSE, ch, 0, 0, TO_CHAR);
	act("$n blushes.", TRUE, ch, 0, 0, TO_ROOM);
	GET_MANA(ch) = MIN(GET_MAX_MANA(ch), GET_MANA(ch) +
	       dice(drink_aff[liq][DRUNK], 30*amount));
	       /* dice(drink_aff[liq][DRUNK], 7)); */
	break;
    case 4:	/* move restore */
	break;
    case 0:	/* nothing */
    default:
	break;
    } 
    temp->obj_flags.value[1] -= amount;

    /* empty the container, and no longer poison. */
    if (!temp->obj_flags.value[1]) {	/* The last bit */
	temp->obj_flags.value[2] = 0;
	temp->obj_flags.value[3] = 0;
	name_from_drinkcon(temp);
    }
    return;
}

#ifdef  UNUSED_CODE
void do_sip(struct char_data *ch, char *argument, int cmd)
{
    struct affected_type af;
    char arg[MAX_LINE_LEN];
    char buf[MAX_BUFSIZ];
    struct obj_data *temp;

    one_argument(argument, arg);

    if (!(temp = get_obj_in_list_vis(ch, arg, ch->carrying))) {
	act("You can't find it!", FALSE, ch, 0, 0, TO_CHAR);
	return;
    }

    if (temp->obj_flags.type_flag != ITEM_DRINKCON) {
	act("You can't sip from that!", FALSE, ch, 0, 0, TO_CHAR);
	return;
    }

    if (GET_COND(ch, DRUNK) > 10) {	/* The pig is drunk ! */
	act("You simply fail to reach your mouth!", FALSE, ch, 0, 0, TO_CHAR);
	act("$n tries to sip, but fails!", TRUE, ch, 0, 0, TO_ROOM);
	return;
    }

    if (!temp->obj_flags.value[1]) {	/* Empty */
	act("But there is nothing in it?", FALSE, ch, 0, 0, TO_CHAR);
	return;
    }

    act("$n sips from the $o", TRUE, ch, temp, 0, TO_ROOM);
    sprintf(buf, "It tastes like %s.\n\r", drinks[temp->obj_flags.value[2]]);
    send_to_char(buf, ch);

    gain_condition(ch, DRUNK,
		    (int) (drink_aff[temp->obj_flags.value[2]][DRUNK] / 4));

    gain_condition(ch, FULL,
		    (int) (drink_aff[temp->obj_flags.value[2]][FULL] / 4));

    gain_condition(ch, THIRST,
		    (int) (drink_aff[temp->obj_flags.value[2]][THIRST] / 4));

    weight_change_object(temp, -1);	/* Subtract one unit */

    if (GET_COND(ch, DRUNK) > 10)
	act("You feel drunk.", FALSE, ch, 0, 0, TO_CHAR);

    if (GET_COND(ch, THIRST) > 20)
	act("You do not feel thirsty.", FALSE, ch, 0, 0, TO_CHAR);

    if (GET_COND(ch, FULL) > 20)
	act("You are full.", FALSE, ch, 0, 0, TO_CHAR);

    if (temp->obj_flags.value[3] && !IS_AFFECTED(ch, AFF_POISON)) {
    /* The shit was poisoned ! */
	act("But it also had a strange taste!", FALSE, ch, 0, 0, TO_CHAR);

	af.type = SPELL_POISON;
	af.duration = 3;
	af.modifier = 0;
	af.location = APPLY_NONE;
	af.bitvector = AFF_POISON;
	affect_to_char(ch, &af);
    }

    temp->obj_flags.value[1]--;

    if (!temp->obj_flags.value[1]) {	/* The last bit */
	temp->obj_flags.value[2] = 0;
	temp->obj_flags.value[3] = 0;
	name_from_drinkcon(temp);
    }

    return; 
}
#endif		/* UNUSED_CODE */

void do_pour(struct char_data *ch, char *argument, int cmd)
{
    char arg1[MAX_LINE_LEN];
    char arg2[MAX_LINE_LEN];
    char buf[MAX_OUT_LEN];
    struct obj_data *from_obj;
    struct obj_data *to_obj;
    int amount;

    argument_interpreter(argument, arg1, arg2);

    if (!*arg1) {	/* No arguments */
	act("What do you want to pour from?", FALSE, ch, 0, 0, TO_CHAR);
	return;
    }

    if (!(from_obj = get_obj_in_list_vis(ch, arg1, ch->carrying))) {
	act("You can't find it!", FALSE, ch, 0, 0, TO_CHAR);
	return;
    }

    if (from_obj->obj_flags.type_flag != ITEM_DRINKCON) {
	act("You can't pour from that!", FALSE, ch, 0, 0, TO_CHAR);
	return;
    }

    if (from_obj->obj_flags.value[1] == 0) {
	act("The $p is empty.", FALSE, ch, from_obj, 0, TO_CHAR);
	return;
    }

    if (!*arg2) {
	act("Where do you want it? Out or in what?", FALSE, ch, 0, 0, TO_CHAR);
	return;
    }

    if (!str_cmp(arg2, "out")) {
	act("$n empties $p", TRUE, ch, from_obj, 0, TO_ROOM);
	act("You empty the $p.", FALSE, ch, from_obj, 0, TO_CHAR);

	weight_change_object(from_obj, -from_obj->obj_flags.value[1]);
		/* Empty */

	from_obj->obj_flags.value[1] = 0;
	from_obj->obj_flags.value[2] = 0;
	from_obj->obj_flags.value[3] = 0;
	name_from_drinkcon(from_obj);

	return; 
    }

    if (!(to_obj = get_obj_in_list_vis(ch, arg2, ch->carrying))) {
	act("You can't find it!", FALSE, ch, 0, 0, TO_CHAR);
	return;
    }

    if (to_obj->obj_flags.type_flag != ITEM_DRINKCON) {
	act("You can't pour anything into that.", FALSE, ch, 0, 0, TO_CHAR);
	return;
    }
    if ((to_obj->obj_flags.value[1] != 0) &&
	(to_obj->obj_flags.value[2] != from_obj->obj_flags.value[2])) {
	act("There is already another liquid in it!", FALSE, ch, 0, 0, TO_CHAR);
	return;
    }
    if (!(to_obj->obj_flags.value[1] < to_obj->obj_flags.value[0])) {
	act("There is no room for more.", FALSE, ch, 0, 0, TO_CHAR);
	return;
    }
    if (from_obj == to_obj) {
	act("That would be silly.", FALSE, ch, 0, 0, TO_CHAR);
	return;
    }
    sprintf(buf, "You pour the %s into the %s.",
	    drinks[from_obj->obj_flags.value[2]], arg2);
    send_to_char(buf, ch);

    /* New alias */
    if (to_obj->obj_flags.value[1] == 0)
	name_to_drinkcon(to_obj, from_obj->obj_flags.value[2]);

    /* First same type liq. */
    to_obj->obj_flags.value[2] = from_obj->obj_flags.value[2];

    /* Then how much to pour */
    from_obj->obj_flags.value[1] -= (amount =
		 (to_obj->obj_flags.value[0] - to_obj->obj_flags.value[1]));

    to_obj->obj_flags.value[1] = to_obj->obj_flags.value[0];

    if (from_obj->obj_flags.value[1] < 0) {	/* There was to little */
	to_obj->obj_flags.value[1] += from_obj->obj_flags.value[1];
	amount += from_obj->obj_flags.value[1];
	from_obj->obj_flags.value[1] = 0;
	from_obj->obj_flags.value[2] = 0;
	from_obj->obj_flags.value[3] = 0;
	name_from_drinkcon(from_obj);
    }

    /* Then the poison boogie */
    to_obj->obj_flags.value[3] =
	(to_obj->obj_flags.value[3] || from_obj->obj_flags.value[3]);

    /* And the weight boogie */

    weight_change_object(from_obj, -amount);
    weight_change_object(to_obj, amount);	/* Add weight */

    return;
}

void do_eat(struct char_data *ch, char *argument, int cmd)
{
    char buf[100];
    struct obj_data *temp;
    struct affected_type af;

    one_argument(argument, buf);

    if (!(temp = get_obj_in_list_vis(ch, buf, ch->carrying))) {
	act("You can't find it!", FALSE, ch, 0, 0, TO_CHAR);
	return;
    }
    if ((temp->obj_flags.type_flag == ITEM_DRINKCON) && (cmd == CMD_TASTE)) {
	do_drink( ch, argument, CMD_SIP ); /* NOTE : Was do_sip() */
	return;
    }
    if ((temp->obj_flags.type_flag != ITEM_FOOD) 
	    && !IS_DIVINE(ch)) {
	act("Your stomach refuses to eat that!?!", FALSE, ch, 0, 0, TO_CHAR);
	return;
    }
    if (GET_COND(ch, FULL) > 32 && cmd != CMD_TASTE ) {	/* Stomach full */
	act("You are too full to eat more!", FALSE, ch, 0, 0, TO_CHAR);
	return;
    }
    act("$n eats $p", TRUE, ch, temp, 0, TO_ROOM);
    act("You eat the $o.", FALSE, ch, temp, 0, TO_CHAR);
    gain_condition(ch, FULL, temp->obj_flags.value[0]);

    if (GET_COND(ch, FULL) > 32)
	act("You are full.", FALSE, ch, 0, 0, TO_CHAR);

    if ((temp->obj_flags.value[3] == 1) && IS_MORTAL(ch)) {
	act("Ooups, it tasted rather strange ?!!?", FALSE, ch, 0, 0, TO_CHAR);
	act("$n coughs and utters some strange sounds.",
	    FALSE, ch, 0, 0, TO_ROOM);
	af.type = SPELL_POISON;
	af.duration = ((cmd == CMD_TASTE) ? 1 : temp->obj_flags.value[0]) * 2;
	af.modifier = 0;
	af.location = APPLY_NONE;
	af.bitvector = AFF_POISON;
	affect_join(ch, &af, FALSE, FALSE);
    }
    else if (temp->obj_flags.value[3] >= 2) {
	act("Wow, you feel great !!", FALSE, ch, 0, 0, TO_CHAR);
	act("$n looks better now.", FALSE, ch, 0, 0, TO_ROOM);
	GET_HIT(ch) = MIN(GET_MAX_HIT(ch), GET_HIT(ch)
			  + dice(2, 4) * temp->obj_flags.value[3]);
	GET_MANA(ch) = MIN(GET_MAX_MANA(ch), GET_MANA(ch)
			   + dice(2, 2) * temp->obj_flags.value[3]);
	GET_MOVE(ch) = MIN(GET_MAX_MOVE(ch), GET_MOVE(ch)
			   + dice(2, 3) * temp->obj_flags.value[3]);
	update_pos(ch);
    }

    if( cmd == CMD_TASTE ){
	temp->obj_flags.value[0]--;
	if ( temp->obj_flags.value[0] <= 0 ) { /* Nothing left */
	    act("There is nothing left now.", FALSE, ch, 0, 0, TO_CHAR);
	    extract_obj(temp);
	}
    }
    else
	extract_obj(temp);
}

#ifdef  UNUSED_CODE
void do_taste(struct char_data *ch, char *argument, int cmd)
{
    struct affected_type af;
    char arg[MAX_LINE_LEN];
    struct obj_data *temp;

    one_argument(argument, arg);

    if (!(temp = get_obj_in_list_vis(ch, arg, ch->carrying))) {
	act("You can't find it!", FALSE, ch, 0, 0, TO_CHAR);
	return;
    }

    if (temp->obj_flags.type_flag == ITEM_DRINKCON) {
	do_sip(ch, argument, 0);
	return;
    }

    if (!(temp->obj_flags.type_flag == ITEM_FOOD)) {
	act("Taste that?!? Your stomach refuses!", FALSE, ch, 0, 0, TO_CHAR);
	return;
    }

    act("$n tastes the $o", FALSE, ch, temp, 0, TO_ROOM);
    act("You taste the $o", FALSE, ch, temp, 0, TO_CHAR);

    gain_condition(ch, FULL, 1);

    if (GET_COND(ch, FULL) > 20)
	act("You are full.", FALSE, ch, 0, 0, TO_CHAR);

    if (temp->obj_flags.value[3] && !IS_AFFECTED(ch, AFF_POISON)) {
	/* The shit was poisoned ! */
	act("Ooups, it did not taste good at all!", FALSE, ch, 0, 0, TO_CHAR);

	af.type = SPELL_POISON;
	af.duration = 2;
	af.modifier = 0;
	af.location = APPLY_NONE;
	af.bitvector = AFF_POISON;
	affect_to_char(ch, &af);
    }

    temp->obj_flags.value[0]--;

    if (!temp->obj_flags.value[0]) {	/* Nothing left */
	act("There is nothing left now.", FALSE, ch, 0, 0, TO_CHAR);
	extract_obj(temp);
    } 
    return; 
}
#endif		/*  UNUSED_CODE  */ 

void do_junk(struct char_data *ch, char *argument, int cmd)
{
    char buf[100];
    struct obj_data *temp;

    one_argument(argument, buf);

    if (!(temp = get_obj_in_list_vis(ch, buf, ch->carrying))) {
	act("You can't find it!", FALSE, ch, 0, 0, TO_CHAR);
	return;
    }
    if (IS_SET(temp->obj_flags.extra_flags, ITEM_NODROP)) {
	act("You can't junk it! Cursed?", FALSE, ch, 0, 0, TO_CHAR);
	return;
    }
    act("$n junks $p", TRUE, ch, temp, 0, TO_ROOM);
    act("You junk the $o.", FALSE, ch, temp, 0, TO_CHAR);
    extract_obj(temp);
}

/* NOTE: Simplified reloading firearm syntex. (NEW: reload [gun [ammo]]) */
/*  Now, you can reload gun while holding it. No need to specify ammo.
    If gun name (1st arg) is ommited, reload gun you are holding. 
    If ammo name (2nd arg) is ommited, inventory is searched for ammo.   */
void do_reload(struct char_data *ch, char *argument, int cmd)
{
    char buffer[MAX_OUT_LEN];
    char arg1[MAX_LINE_LEN];
    char arg2[MAX_LINE_LEN];
    struct obj_data *gun;
    struct obj_data *ammo, *tmp_obj;
    int ammo_nr ;

    if (GET_POS(ch) < POS_STANDING) {
	send_to_char("You need to be standing still for that.\n\r", ch);
	return;
    }
    argument_interpreter(argument, arg1, arg2);

    if (*arg1) {
	/* NOTE: Look holding item before inventory.  */
	tmp_obj = ch->equipment[HOLD];
	if ( tmp_obj && isname(arg1, tmp_obj->name )
	    && (GET_ITEM_TYPE(tmp_obj) == ITEM_FIREWEAPON)) 
	    gun = tmp_obj ;
	else { 
	    gun = get_obj_in_list_vis(ch, arg1, ch->carrying);
	    if (!gun) {
		sprintf(buffer, "You dont have the %s.\n\r", arg1);
		send_to_char(buffer, ch);
		return;
	    }
	    if (GET_ITEM_TYPE(gun) != ITEM_FIREWEAPON) {
		send_to_char("You can't reload that!\n\r", ch);
		return;
	    }
	}
    }
    /* NOTE: If no arg is given, reload holding firearm */
    else if (!( gun = ch->equipment[HOLD] ) 
		|| (GET_ITEM_TYPE(gun) != ITEM_FIREWEAPON)) {
	    send_to_char("Reload what with what?\n\r", ch);
	    return;
    }

    if (*arg2) {
	ammo = get_obj_in_list_vis(ch, arg2, ch->carrying);
	if (!ammo) {
	    sprintf(buffer, "You dont have the %s.\n\r", arg2);
	    send_to_char(buffer, ch);
	    return;
	}
	if ((GET_OBJ_VIRTUAL(gun) + 1) != GET_OBJ_VIRTUAL(ammo)) {
	    sprintf(buffer, "%s cannot be used as ammo for %s\n\r", arg2, arg1);
	    send_to_char(buffer, ch);
	    return;
	}
    }
    else {
	/* NOTE: No ammo name specified. Serach inventory for ammo. */
	ammo_nr = obj_index[GET_OBJ_VIRTUAL(gun) + 1 ].virtual;
	ammo = NULL;
	for ( tmp_obj = ch->carrying ; tmp_obj ; 
		tmp_obj = tmp_obj->next_content )
	    if (CAN_SEE_OBJ(ch, tmp_obj)
		&& ( GET_OBJ_VIRTUAL(tmp_obj) == ammo_nr )) {
		ammo = tmp_obj ;
		break ;
	    }
	    
	if (!ammo) {
	    sprintf(buffer, "You dont have ammo for %s.\n\r", gun->name );
	    send_to_char(buffer, ch);
	    return; 
	}
    }

    gun->obj_flags.value[0] = gun->obj_flags.value[1];
    if (gun->obj_flags.value[2] > gun->obj_flags.value[1])
	--gun->obj_flags.value[2];
    extract_obj(ammo);
    act("$n reloads $p", TRUE, ch, gun, 0, TO_ROOM);
    send_to_char("You reload.\n\r", ch);

} 

void do_quaff(struct char_data *ch, char *argument, int cmd)
{
    char buf[100];
    struct obj_data *temp;
    int i;
    bool equipped;

    equipped = FALSE;

    one_argument(argument, buf);

    if (!(temp = get_obj_in_list_vis(ch, buf, ch->carrying))) {
	temp = ch->equipment[HOLD];
	equipped = TRUE;
	if ((temp == 0) || !isname(buf, temp->name)) {
	    act("You do not have that item.", FALSE, ch, 0, 0, TO_CHAR);
	    return;
	}
    }

    if (temp->obj_flags.type_flag != ITEM_POTION) {
	act("You can only quaff potions.", FALSE, ch, 0, 0, TO_CHAR);
	return;
    }

    act("$n quaffs $p.", TRUE, ch, temp, 0, TO_ROOM);
    act("You quaff $p which dissolves.", FALSE, ch, temp, 0, TO_CHAR);

    for (i = 1; i < 4; i++)
	if (temp->obj_flags.value[i] >= 1)
	    ((*spell_info[temp->obj_flags.value[i]].spell_pointer)
	     ((byte) temp->obj_flags.value[0], ch, "", SPELL_TYPE_POTION, ch, 0));

    if (equipped)
	unequip_char(ch, HOLD);

    extract_obj(temp);
} 

void do_recite(struct char_data *ch, char *argument, int cmd)
{
    char buf[100];
    struct obj_data *scroll, *obj;
    struct char_data *victim;
    int i, bits;
    bool equipped;

    equipped = FALSE;
    obj = 0;
    victim = 0;

    if (!ch)
	return;
    if (IS_SET(world[ch->in_room].room_flags, NO_MAGIC)) {
	send_to_char("Some misterious power prevent you reading scroll\n\r", ch);
	return;
    }
    argument = one_argument(argument, buf);

    if (!(scroll = get_obj_in_list_vis(ch, buf, ch->carrying))) {
	scroll = ch->equipment[HOLD];
	equipped = TRUE;
	if ((scroll == 0) || !isname(buf, scroll->name)) {
	    act("You do not have that item.", FALSE, ch, 0, 0, TO_CHAR);
	    return;
	}
    }

    if (scroll->obj_flags.type_flag != ITEM_SCROLL) {
	act("Recite is normally used for scroll's.", FALSE, ch, 0, 0, TO_CHAR);
	return;
    }

    if (*argument) {
	bits = generic_find(argument, FIND_OBJ_INV | FIND_OBJ_ROOM |
			    FIND_OBJ_EQUIP, ch, &victim, &obj);
	if (bits == 0) {
	    send_to_char("No such thing around to recite the scroll on.\n\r", ch);
	    return;
	}
    }
    else {
	victim = ch;
    }

    act("$n recites $p.", TRUE, ch, scroll, 0, TO_ROOM);
    act("You recite $p which dissolves.", FALSE, ch, scroll, 0, TO_CHAR);

    for (i = 1; i < 4; i++)
	if (scroll->obj_flags.value[i] >= 1)
	    ((*spell_info[scroll->obj_flags.value[i]].spell_pointer)
	     ((byte) scroll->obj_flags.value[0], ch, "", 
		    SPELL_TYPE_SCROLL, victim, obj));

    if (equipped)
	unequip_char(ch, HOLD);

    extract_obj(scroll);
}

void do_use(struct char_data *ch, char *argument, int cmd)
{
    char buf[MAX_BUFSIZ];
    struct char_data *tmp_char;
    struct obj_data *tmp_object, *stick;

    int bits;

    argument = one_argument(argument, buf);

    if (ch->equipment[HOLD] == 0 ||
	!isname(buf, ch->equipment[HOLD]->name)) {
	act("You do not hold that item in your hand.", FALSE, ch, 0, 0, TO_CHAR);
	return;
    }

    /* by ares */
    sprintf(buf, "Use log : %s uses %s", ch->player.name, argument);
    log(buf);

    stick = ch->equipment[HOLD];

    if (stick->obj_flags.type_flag == ITEM_STAFF) {
	act("$n taps $p three times on the ground.",
	    TRUE, ch, stick, 0, TO_ROOM);
	act("You tap $p three times on the ground.",
	    FALSE, ch, stick, 0, TO_CHAR);

	if (stick->obj_flags.value[2] > 0) {  /* Is there any charges left? */
	    stick->obj_flags.value[2]--;
	    ((*spell_info[stick->obj_flags.value[3]].spell_pointer)
	     ((byte) stick->obj_flags.value[0], ch, "", SPELL_TYPE_STAFF, 0, 0));

	}
	else {
	    send_to_char("The staff seems powerless.\n\r", ch);
	}
    }
    else if (stick->obj_flags.type_flag == ITEM_WAND) {

	bits = generic_find(argument, FIND_CHAR_ROOM | FIND_OBJ_INV | FIND_OBJ_ROOM |
			    FIND_OBJ_EQUIP, ch, &tmp_char, &tmp_object);
	if (bits) {
	    if (bits == FIND_CHAR_ROOM) {
		act("$n point $p at $N.", TRUE, ch, stick, tmp_char, TO_ROOM);
		act("You point $p at $N.", FALSE, ch, stick, tmp_char, TO_CHAR);
	    }
	    else {
		act("$n point $p at $P.", TRUE, ch, stick, tmp_object, TO_ROOM);
		act("You point $p at $P.", FALSE, ch, stick, tmp_object, TO_CHAR);
	    }

	    if (stick->obj_flags.value[2] > 0) {	/* Is there any
							   charges left? */
		stick->obj_flags.value[2]--;
		((*spell_info[stick->obj_flags.value[3]].spell_pointer)
		 ((byte) stick->obj_flags.value[0], ch, "", SPELL_TYPE_WAND, tmp_char, tmp_object));
	    }
	    else {
		send_to_char("The wand seems powerless.\n\r", ch);
	    }
	}
	else {
	    send_to_char("What should the wand be pointed at?\n\r", ch);
	}
    }
    else {
	send_to_char("Use is normally only for wand's and staff's.\n\r", ch);
    }
}

/* ************************************************************************
   *  OLD  file: shop.c , Shop module. 			Part of DIKUMUD *
   *  Usage: Procedures handling shops and shopkeepers.			*
   ************************************************************************* */


#define MAX_TRADE 5
#define MAX_PROD 5
// #define MAX_PROD 10

struct shop_data {
    int producing[MAX_PROD];	/* Which item to produce (virtual)      */
    float profit_buy;	/* Factor to multiply cost with.        */
    float profit_sell;	/* Factor to multiply cost with.        */
    byte type[MAX_TRADE];	/* Which item to trade.                 */
    char *no_such_item1;	/* Message if keeper hasn't got an item */
    char *no_such_item2;	/* Message if player hasn't got an item */
    char *missing_cash1;	/* Message if keeper hasn't got cash    */
    char *missing_cash2;	/* Message if player hasn't got cash    */
    char *do_not_buy;	/* If keeper dosn't buy such things.   */
    char *message_buy;	/* Message when player buys item        */
    char *message_sell;	/* Message when player sells item       */
    int temper1;	/* How does keeper react if no money    */
    int temper2;	/* How does keeper react when attacked  */
    int keeper;		/* The mobil who owns the shop (virtual) */
    int with_who;	/* Who does the shop trade with?  */
    int in_room;	/* Where is the shop?      */
    int open1, open2;	/* When does the shop open?    */
    int close1, close2;	/* When does the shop close?    */
};

/* internal var's */
struct shop_data *shop_index;
int number_of_shops;

/* extern procs */
extern void do_tell(struct char_data *ch, char *arg, int cmd);

int is_ok(struct char_data *keeper, struct char_data *ch, int shop_nr)
{
    if (shop_index[shop_nr].open1 > time_info.hours) {
	do_say(keeper, "Come back later!", 0);
	return (FALSE);
    }
    else if (shop_index[shop_nr].close1 < time_info.hours) {
	if (shop_index[shop_nr].open2 > time_info.hours) {
	    do_say(keeper, "Sorry, we have closed, but come back later.", 0);
	    return (FALSE);
	}
	else if (shop_index[shop_nr].close2 < time_info.hours) {
	    do_say(keeper, "Sorry, come back tomorrow.", 0);
	    return (FALSE);
	};
    }

    if (!(CAN_SEE(keeper, ch))) {
	do_say(keeper, "I don't trade with someone I can't see!", 0);
	return (FALSE);
    };

    switch (shop_index[shop_nr].with_who) {
    case 0:
	return (TRUE);
    case 1:
	return (TRUE);
    default:
	return (TRUE);
    };
}

int trade_with(struct obj_data *item, int shop_nr)
{
    int counter;

    if (item->obj_flags.cost < 1)
	return (FALSE);

    for (counter = 0; counter < MAX_TRADE; counter++)
	if (shop_index[shop_nr].type[counter] == item->obj_flags.type_flag)
	    return (TRUE);
    return (FALSE);
}

int shop_producing(struct obj_data *item, int shop_nr)
{
    int counter;

    if (item->item_number < 0)
	return (FALSE);

    for (counter = 0; counter < MAX_PROD; counter++)
	if (shop_index[shop_nr].producing[counter] == item->item_number)
	    return (TRUE);
    return (FALSE);
}

/* NOTE:  return -1 on failed transaction, 0 on success */
/* void shopping_buy( char *arg, struct char_data *ch,... ) */
int shopping_buy(char *arg, struct char_data *ch,
		 struct char_data *keeper, int shop_nr)
{
    char argm[100], buf[MAX_BUFSIZ];
    struct obj_data *temp1;
    extern void do_action(struct char_data *ch, char *arg, int cmd);
    extern void do_emote(struct char_data *ch, char *arg, int cmd);
    extern int find_action(char *action_name, int *pmin_lev, int *pmin_pos );

    if (!(is_ok(keeper, ch, shop_nr)))
	return -1;

    one_argument(arg, argm);

    if (!(*argm)) {
	sprintf(buf, "%s What do you want to buy??", GET_NAME(ch));
	do_tell(keeper, buf, 0);
	return -1;
    };

    if (!(temp1 = get_obj_in_list_vis(ch, argm, keeper->carrying))) {
	sprintf(buf, shop_index[shop_nr].no_such_item1 ,GET_NAME(ch));
	do_tell(keeper, buf, 0);
	return -1;
    }

    if (temp1->obj_flags.cost <= 0) {
	sprintf(buf, shop_index[shop_nr].no_such_item1 ,GET_NAME(ch));
	do_tell(keeper, buf, 0);
	extract_obj(temp1);
	return -1;
    }

    if (GET_GOLD(ch) < (int) (temp1->obj_flags.cost *
	     shop_index[shop_nr].profit_buy) && !IS_DIVINE(ch)) {
	sprintf(buf, shop_index[shop_nr].missing_cash2, GET_NAME(ch));
	do_tell(keeper, buf, 0);

	switch (shop_index[shop_nr].temper1) {
	case 0:
	    /* NOTE: Use find_action() to get cmd arg to do_action */
	    do_action(keeper, GET_NAME(ch), find_action("puke", 0, 0 ));
	    return -1;
	case 1:
	    do_emote(keeper, "smokes on his joint", CMD_EMOTE);
	    return -1;
	default:
	    return -1;
	}
    }

    if ((GET_CARRYING_N(ch) + 1 > CAN_CARRY_N(ch))) {
	sprintf(buf, "%s : You can't carry that many items.\n\r",
		fname(temp1->name));
	send_to_char(buf, ch);
	return -1;
    }

    if ((GET_CARRYING_W(ch) + temp1->obj_flags.weight) > CAN_CARRY_W(ch)) {
	sprintf(buf, "%s : You can't carry that much weight.\n\r",
		fname(temp1->name));
	send_to_char(buf, ch);
	return -1;
    }

    act("$n buys $p.", FALSE, ch, temp1, 0, TO_ROOM);

    sprintf(buf, shop_index[shop_nr].message_buy, GET_NAME(ch),
	    (int) (temp1->obj_flags.cost * shop_index[shop_nr].profit_buy));
    do_tell(keeper, buf, 0);
    sprintf(buf, "You now have %s.\n\r", temp1->short_description);
    send_to_char(buf, ch);
    if (!IS_DIVINE(ch))
	GET_GOLD(ch) -= (int) (temp1->obj_flags.cost *
			       shop_index[shop_nr].profit_buy);

    /* to prevent shop keeper's death */
/*
    GET_GOLD(keeper) += (int)(temp1->obj_flags.cost*
	shop_index[shop_nr].profit_buy);
 */

    /* Test if producing shop ! */
    if (shop_producing(temp1, shop_nr))
	temp1 = read_object(temp1->item_number, REAL);
    else
	obj_from_char(temp1);

    obj_to_char(temp1, ch);

    return 0;
}

void shopping_sell(char *arg, struct char_data *ch,
		   struct char_data *keeper, int shop_nr)
{
    char argm[100], buf[MAX_BUFSIZ];
    struct obj_data *temp1;
    extern struct obj_data *get_obj_in_list(char *name, struct obj_data *list);


    if (!(is_ok(keeper, ch, shop_nr)))
	return;

    one_argument(arg, argm);

    if (!(*argm)) {
	sprintf(buf, "%s What do you want to sell??" ,GET_NAME(ch));
	do_tell(keeper, buf, 0);
	return;
    }

    if (!(temp1 = get_obj_in_list_vis(ch, argm, ch->carrying))) {
	sprintf(buf,
		shop_index[shop_nr].no_such_item2
		,GET_NAME(ch));
	do_tell(keeper, buf, 0);
	return;
    }

    if (!(trade_with(temp1, shop_nr)) || (temp1->obj_flags.cost < 1)) {
	sprintf(buf,
		shop_index[shop_nr].do_not_buy,
		GET_NAME(ch));
	do_tell(keeper, buf, 0);
	return;
    }

    if (GET_GOLD(keeper) < (int) (temp1->obj_flags.cost *
				  shop_index[shop_nr].profit_sell)) {
	sprintf(buf, shop_index[shop_nr].missing_cash1 ,GET_NAME(ch));
	do_tell(keeper, buf, 0);
	return;
    }

    act("$n sells $p.", FALSE, ch, temp1, 0, TO_ROOM);

    sprintf(buf, shop_index[shop_nr].message_sell, GET_NAME(ch), 
	    (int) (temp1->obj_flags.cost * shop_index[shop_nr].profit_sell));
    do_tell(keeper, buf, 0);
    sprintf(buf, "The shopkeeper now has %s.\n\r", temp1->short_description);
    send_to_char(buf, ch);
    GET_GOLD(ch) += (int) (temp1->obj_flags.cost *
			   shop_index[shop_nr].profit_sell);
    /* to prevent shop keeper's death */
    /* 
    GET_GOLD(keeper) -= (int) (temp1->obj_flags.cost*
	shop_index[shop_nr].profit_sell);
    */

    if ((get_obj_in_list(argm, keeper->carrying)) ||
    /* NOTE: Item type to junk when sold to shopkeeper: was ITEM_TRASH */
	    (GET_ITEM_TYPE(temp1) == ITEM_TRASH) ||
	    (GET_ITEM_TYPE(temp1) == ITEM_OTHER))
	extract_obj(temp1);
    else {
	obj_from_char(temp1);
	obj_to_char(temp1, keeper);
    }

    return;
}

void shopping_value(char *arg, struct char_data *ch,
		    struct char_data *keeper, int shop_nr)
{
    char argm[100], buf[MAX_BUFSIZ];
    struct obj_data *temp1;

    if (!(is_ok(keeper, ch, shop_nr)))
	return;

    one_argument(arg, argm);

    if (!(*argm)) {
	sprintf(buf, "%s What do you want me to valuate??", GET_NAME(ch));
	do_tell(keeper, buf, 0);
	return;
    }

    if (!(temp1 = get_obj_in_list_vis(ch, argm, ch->carrying))) {
	sprintf(buf, shop_index[shop_nr].no_such_item2, GET_NAME(ch));
	do_tell(keeper, buf, 0);
	return;
    }

    if (!(trade_with(temp1, shop_nr))) {
	sprintf(buf,
		shop_index[shop_nr].do_not_buy,
		GET_NAME(ch));
	do_tell(keeper, buf, 0);
	return;
    }

    sprintf(buf, "%s I'll give you %d gold coins for that!", GET_NAME(ch),
	    (int) (temp1->obj_flags.cost * shop_index[shop_nr].profit_sell));
    do_tell(keeper, buf, 0);

    return;
}

void shopping_list(char *arg, struct char_data *ch,
		   struct char_data *keeper, int shop_nr)
{
    char buf[MAX_STR_LEN*2], buf2[MAX_LINE_LEN], buf3[MAX_NAME_LEN];
    struct obj_data *temp1;
    int found_obj;
    extern char *drinks[];

    if (!(is_ok(keeper, ch, shop_nr)))
	return;

    strcpy(buf, "You can buy:\n\r");
    found_obj = FALSE;
    if (keeper->carrying)
	for (temp1 = keeper->carrying;
	     temp1;
	     temp1 = temp1->next_content)
	    if ((CAN_SEE_OBJ(ch, temp1)) && (temp1->obj_flags.cost > 0)) {
		found_obj = TRUE;
		if (temp1->obj_flags.type_flag != ITEM_DRINKCON)
		    sprintf(buf2, "%s for %d gold coins.\n\r"
			    ,(temp1->short_description)
			    ,(int) (temp1->obj_flags.cost *
				    shop_index[shop_nr].profit_buy));
		else {
		    if (temp1->obj_flags.value[1])
			sprintf(buf3, "%s of %s", (temp1->short_description)
				,drinks[temp1->obj_flags.value[2]]);
		    else
			sprintf(buf3, "%s", (temp1->short_description));
		    sprintf(buf2, "%s for %d gold coins.\n\r", buf3,
			    (int) (temp1->obj_flags.cost * shop_index[shop_nr].profit_buy));
		}
		CAP(buf2);
		strcat(buf, buf2);
	    };

    if (!found_obj)
	strcat(buf, "Nothing!\n\r");

    send_to_char(buf, ch);
    return;
}

int shop_keeper(struct char_data *ch, int cmd, char *arg)
{
    struct char_data *temp_char;
    struct char_data *keeper;
    int shop_nr;

    keeper = 0;
    for (temp_char = world[ch->in_room].people; (!keeper) && (temp_char);
	 temp_char = temp_char->next_in_room)
	/* NOTE: not IS_NPC(temp_char) */
	if (IS_MOB(temp_char) && temp_char->nr >= 0 ) 
	    if (mob_index[temp_char->nr].func == shop_keeper)
		keeper = temp_char;
    for (shop_nr = 0; shop_index[shop_nr].keeper != keeper->nr; shop_nr++) ;

    if ( ch->in_room != real_room(shop_index[shop_nr].in_room))
	return(FALSE);
    else if (cmd == CMD_BUY ) { 	/* Buy 56 */
	char arg1[100], arg2[100];
	int amount;

	argument_interpreter(arg, arg1, arg2);
	amount = atoi(arg1);

	/* NOTE: for buying N items in single trade. ( "buy 10 solar" ) */
	/* NOTE: BUG FIX: Reported by Ljw: check amount *before* shopping. */
	if ( amount > 0 && *arg2 )
	    while ( amount > 0 && !shopping_buy(arg2, ch, keeper, shop_nr))
		amount--;
	else
	    shopping_buy(arg, ch, keeper, shop_nr);

    }
    else if (cmd == CMD_SELL ) { 	/* Sell 57  */
	shopping_sell(arg, ch, keeper, shop_nr);
    }
    else if (cmd == CMD_VALUE) { 	/* value  58  */
	shopping_value(arg, ch, keeper, shop_nr);
    }
    else if (cmd == CMD_LIST ) {	/* List 59  */
	shopping_list(arg, ch, keeper, shop_nr);
    }
    else if (cmd == CMD_STEAL ) {	/* Steal 156 */
	send_to_char("Oops.\n\r", ch);
	hit(keeper, ch, TYPE_UNDEFINED);
    }
    else if (((cmd == CMD_CAST) || (cmd == CMD_RECITE) || (cmd == CMD_USE))
	&& IS_MORTAL(ch)) {
	/* Cast 84 , recite 207 , use  172 */
	act("$N tells you 'No magic here - kid!'.", 
		FALSE, ch, 0, keeper, TO_CHAR);
    }
    else
	return (FALSE);
    return (TRUE);
}

void boot_the_shops(void)
{
    char *buf;
    int temp;
    int count;
    FILE *shop_f;

    if (!(shop_f = fopen(lookup_db("shop"), "r"))) {
	perror("Error in boot shop\n");
	exit(0);
    }

    number_of_shops = 0;

    for (;;) {
	buf = fread_string(shop_f);
	if (*buf == '#') {	/* a new shop */
	    if (!number_of_shops)	/* first shop */
		CREATE(shop_index, struct shop_data, 1);

	    else if (!(shop_index = (struct shop_data *) realloc( shop_index, 
		 (number_of_shops + 1) * sizeof(struct shop_data)))) {
		perror("Error in boot shop\n");
		exit(0);
	    }

	    for (count = 0; count < MAX_PROD; count++) {
		fscanf(shop_f, "%d \n", &temp);
		if (temp >= 0)
		    shop_index[number_of_shops].producing[count] =
			real_object(temp);
		else
		    shop_index[number_of_shops].producing[count] = temp;
	    }
	    fscanf(shop_f, "%f \n",
		   &shop_index[number_of_shops].profit_buy);
	    fscanf(shop_f, "%f \n",
		   &shop_index[number_of_shops].profit_sell);
	    for (count = 0; count < MAX_TRADE; count++) {
		fscanf(shop_f, "%d \n", &temp);
		shop_index[number_of_shops].type[count] =
		    (byte) temp;
	    }
	    shop_index[number_of_shops].no_such_item1 =
		fread_string(shop_f);
	    shop_index[number_of_shops].no_such_item2 =
		fread_string(shop_f);
	    shop_index[number_of_shops].do_not_buy =
		fread_string(shop_f);
	    shop_index[number_of_shops].missing_cash1 =
		fread_string(shop_f);
	    shop_index[number_of_shops].missing_cash2 =
		fread_string(shop_f);
	    shop_index[number_of_shops].message_buy =
		fread_string(shop_f);
	    shop_index[number_of_shops].message_sell =
		fread_string(shop_f);
	    fscanf(shop_f, "%d \n",
		   &shop_index[number_of_shops].temper1);
	    fscanf(shop_f, "%d \n",
		   &shop_index[number_of_shops].temper2);
	    fscanf(shop_f, "%d \n",
		   &shop_index[number_of_shops].keeper);

	    shop_index[number_of_shops].keeper =
		real_mobile(shop_index[number_of_shops].keeper);

	    fscanf(shop_f, "%d \n",
		   &shop_index[number_of_shops].with_who);
	    fscanf(shop_f, "%d \n",
		   &shop_index[number_of_shops].in_room);
	    fscanf(shop_f, "%d \n",
		   &shop_index[number_of_shops].open1);
	    fscanf(shop_f, "%d \n",
		   &shop_index[number_of_shops].close1);
	    fscanf(shop_f, "%d \n",
		   &shop_index[number_of_shops].open2);
	    fscanf(shop_f, "%d \n",
		   &shop_index[number_of_shops].close2);

	    number_of_shops++;
	}
	else if (*buf == '$')	/* EOF */
	    break;
    }

    fclose(shop_f);
}

void assign_the_shopkeepers(void)
{
    int temp1;

    for (temp1 = 0; temp1 < number_of_shops; temp1++)
	mob_index[shop_index[temp1].keeper].func = shop_keeper;
}

#ifdef UNUSED_CODE

/* **********************************************************************
   *  file: newshop.h, newshop.c , New Shop module.			*
   *  Usage: This code is never used in NARAI. 			*
   ********************************************************************** */
/*
   used goods shops
   made by Process(wdshin@eve.kaist.ac.kr)
 */ 

#define GET_SHOP_STORAGE(x) (used_shop_storage_rooms[(x)])
#define GET_SHOP_ROOM(x) (used_shop_roomsl[(x)])

/* virtual number of room */
int used_shop_rooms[] = { 0,	};			/* NOT USED */ 
int used_shop_storage_rooms[] = { 0,	};		/* NOT USED */ 

int find_shop_index(int room_nr)
{
    int i, result;
    int flag = FALSE;

    for (i = 0; used_shop_rooms[i]; i++) {
	if (room_nr == used_shop_rooms[i]) {
	    flag = TRUE;
	    result = i;
	}
    }
    if (flag)
	return result;
    else
	return 0;
}

int used_shop(struct char_data *ch, int cmd, char *arg)
{
    int shop_num;
    struct room_data *storage_room;
    struct obj_data *storage, *i;
    char buf[MAX_BUFSIZ], one_line[MAX_STRING_LENGTH];
    int no_of_items;
    struct obj_data *item_sorted_list, *tmp_obj, *min_obj;

    shop_num = find_shop_index(world[ch->in_room].number);
    if (shop_num == 0) {
	log("ERROR in find_shop_index!");
	send_to_char("Ooops. bug???\n\r", ch);
	return TRUE;
    }

    storage_room = world[real_room(used_shop_storage_rooms[shop_num])];
    storage = storage_room.contents;

    for (i = storage; i; i->next_content) {
	for (j = i->next_content; j; j->next_content) {
	}
    }

    strcpy(buf,, "");
    if (cmd == CMD_LIST ) {	/* list 59 */
	/* lists goods' name and value in the storage room */
	for (i = storage; i; i = i->next_content) {
	}
    }
    else if (cmd == CMD_BUY ) {	/* buy 56 */
	/* get the goods from the storage room and give it to character */
    }
    else if (cmd == CMD_SELL ) {	/* sell 57 */
	/* get the goods from the character and transfer it to storage room */
    }
    else if (cmd == CMD_VIEW ) {	/* view 299 */
	/* identify the goods' status */
    }
    return FALSE;
}

#endif		/* UNUSED_CODE */
