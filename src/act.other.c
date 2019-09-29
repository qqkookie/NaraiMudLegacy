/* ************************************************************************
*  file: act.other.c , Implementation of commands.        Part of DIKUMUD *
*  Usage : Other commands.                                                *
*  Copyright (C) 1990, 1991 - see 'license.doc' for complete information. *
************************************************************************* */

#include <stdio.h>
#include <string.h>
#include <ctype.h>

#include "structs.h"
#include "utils.h"
#include "comm.h"
#include "interpreter.h"
#include "handler.h"
#include "db.h"
#include "spells.h"
#include "quest.h"

/* extern variables */

extern struct str_app_type str_app[];
extern struct room_data *world;
extern struct descriptor_data *descriptor_list;
extern struct room_data *world;
extern struct dex_skill_type dex_app_skill[];
extern struct spell_info_type spell_info[];


/* extern procedures */

int stash_char(struct char_data *ch, char *filename);
void wipe_stash(char *filename);
void hit(struct char_data *ch, struct char_data *victim, int type);
void do_shout(struct char_data *ch, char *argument, int cmd);

void do_qui(struct char_data *ch, char *argument, int cmd)
{
	send_to_char("If you want to quit, type 'quit'.\n\r", ch) ;
}

void do_quit(struct char_data *ch, char *argument, int cmd)
{
    void do_save(struct char_data *ch, char *argument, int cmd);
    void die(struct char_data *ch, struct char_data *vic) ;
    char cyb[BUFSIZ];

    if (IS_NPC(ch) || !ch->desc)
        return;
    if (GET_POS(ch) == POSITION_FIGHTING) {
        send_to_char("No way! You are fighting.\n\r", ch);
        return;
    }
    if (GET_POS(ch) < POSITION_STUNNED) {
        send_to_char("You die before your time!\n\r", ch);
        die(ch, NULL);   /* cyb :do not know who killed him */
        return;
    }
    /* wipe_stash(GET_NAME(ch)); */
    send_to_char("Goodbye, friend.. Come back soon!\n\r", ch);
    /* act("$n has aborted the game.", TRUE, ch,0,0,TO_ROOM);*/
    /* extract_char(ch); Char is saved in extract char - quit is temporary */
    sprintf(cyb, "%s closed connection(quit)", GET_NAME(ch));
    log(cyb);
    do_save(ch,"",0);
    if ( stash_char(ch,0) < 0 ) {
		sprintf(cyb, "saving rent %s failed (quit)", GET_NAME(ch));
		log(cyb) ;
    }
    if (ch->desc) close_socket(ch->desc);
}

do_wimpy(struct char_data *ch, char *argument, int cmd)
{
    if (IS_NPC(ch))
        return;

    if (IS_SET(ch->specials.act, PLR_WIMPY)) {
        send_to_char("You are not wimpy now.\n\r", ch);
        REMOVE_BIT(ch->specials.act, PLR_WIMPY);
    } else {
        send_to_char("You are wimpy now.\n\r", ch);
        SET_BIT(ch->specials.act, PLR_WIMPY);
    }
}

void do_save(struct char_data *ch, char *argument, int cmd)
{
    char buf[100];

    if (IS_NPC(ch) || !ch->desc)
        return;
    sprintf(buf, "Saving %s.\n\r", GET_NAME(ch));
    send_to_char(buf, ch);
    save_char(ch,NOWHERE);
    if ( stash_char(ch,0) < 0 ) {
        send_to_char("Fail to save rent file - report this to wizard\n\r", ch) ;
	}
}

void do_not_here(struct char_data *ch, char *argument, int cmd)
{
    send_to_char("Sorry, but you cannot do that here!\n\r",ch);
}

void do_sneak(struct char_data *ch, char *argument, int cmd)
{
    struct affected_type af;
    byte percent;

    send_to_char("Ok, you'll try to move silently for a while.\n\r", ch);
    if (IS_AFFECTED(ch, AFF_SNEAK))
        affect_from_char(ch, SKILL_SNEAK);
    percent=number(1,101); /* 101% is a complete failure */
    if (percent > ch->skills[SKILL_SNEAK].learned +
        dex_app_skill[GET_DEX(ch)].sneak)
        return;
    af.type = SKILL_SNEAK;
    af.duration = GET_LEVEL(ch);
    af.modifier = 0;
    af.location = APPLY_NONE;
    af.bitvector = AFF_SNEAK;
    affect_to_char(ch, &af);
}

void do_hide(struct char_data *ch, char *argument, int cmd)
{
    byte percent;

    send_to_char("You attempt to hide yourself.\n\r", ch);
    if (IS_AFFECTED(ch, AFF_HIDE))
        REMOVE_BIT(ch->specials.affected_by, AFF_HIDE);
    percent=number(1,101); /* 101% is a complete failure */
    if (percent > ch->skills[SKILL_HIDE].learned +
        dex_app_skill[GET_DEX(ch)].hide)
        return;
    SET_BIT(ch->specials.affected_by, AFF_HIDE);
}

void do_steal(struct char_data *ch, char *argument, int cmd)
{
    struct char_data *victim;
    struct obj_data *obj;
    char victim_name[240];
    char obj_name[240];
    char buf[240];
    int percent, bits;
    bool equipment = FALSE;
    int gold, eq_pos;
    bool ohoh = FALSE;
    extern int nostealflag;

    argument = one_argument(argument, obj_name);
    one_argument(argument, victim_name);

    if (!(victim = get_char_room_vis(ch, victim_name))) {
        send_to_char("Steal what from who?\n\r", ch);
        return;
    } else if (victim == ch) {
        send_to_char("Come on now, that's rather stupid!\n\r", ch);
        return;
    }
    if((!IS_NPC(victim))&&(nostealflag)){
        act("Oops..", FALSE, ch,0,0,TO_CHAR);
        act("$n tried to steal something from you!",FALSE,ch,0,victim,TO_VICT);
        act("$n tries to rob $N.", TRUE, ch, 0, victim, TO_NOTVICT);
        return;
    }
    if( !IS_NPC(victim) && !(victim->desc ) )
    {
        send_to_char("Steal something from the ghost? No way!\n\r", ch);
        return;
    }
    /* 101% is a complete failure */
    percent=number(1,101)+2*GET_LEVEL(victim)-GET_LEVEL(ch)
        -dex_app_skill[GET_DEX(ch)].p_pocket;
    if (str_cmp(obj_name, "coins") && str_cmp(obj_name,"gold")) {
        if (!(obj = get_obj_in_list_vis(victim, obj_name, victim->carrying))) {
            for (eq_pos = 0; (eq_pos < MAX_WEAR); eq_pos++)
                if (victim->equipment[eq_pos] &&
                    (isname(obj_name, victim->equipment[eq_pos]->name)) &&
                    CAN_SEE_OBJ(ch,victim->equipment[eq_pos])) {
                    obj = victim->equipment[eq_pos];
                    break;
                }
            if (!obj) {
                act("$E has not got that item.",FALSE,ch,0,victim,TO_CHAR);
                return;
            } else { /* It is equipment */
                send_to_char("Steal the equipment? Impossible!\n\r", ch);
                return;
            }
        } else {  /* obj found in inventory */
            percent += GET_OBJ_WEIGHT(obj); /* Make heavy harder */
            if (AWAKE(victim) && (percent > ch->skills[SKILL_STEAL].learned)) {
                ohoh = TRUE;
                act("Oops..", FALSE, ch,0,0,TO_CHAR);
                act("$n tries to rob $N.", TRUE, ch, 0, victim, TO_NOTVICT);
                if(!IS_NPC(victim))
                    act("$n tried to rob you.",TRUE,ch,0,victim,TO_VICT);
            } else { /* Steal the item */
                if ((IS_CARRYING_N(ch) + 1 < CAN_CARRY_N(ch))) {
                    if ((IS_CARRYING_W(ch) + GET_OBJ_WEIGHT(obj)) < CAN_CARRY_W(ch)) {
                        obj_from_char(obj);
                        obj_to_char(obj, ch);
                        send_to_char("Got it!\n\r", ch);
                        if(!IS_NPC(victim)) {
                            act("$n robbed you!",TRUE,ch,0,victim,TO_VICT);
                            stash_char(victim,0);
                        }
                        stash_char(ch,0);
                    }
                    else
                        send_to_char("You cannot carry that much weight.\n\r", ch);
                } else
                    send_to_char("You cannot carry that much.\n\r", ch);
            }
        }
    } else { /* Steal some coins */
        if (AWAKE(victim) && (percent > ch->skills[SKILL_STEAL].learned)) {
            ohoh = TRUE;
            act("Oops..", FALSE, ch,0,0,TO_CHAR);
            act("$n tries to steal gold from $N.",TRUE,ch,0,victim,TO_NOTVICT);
            if(!IS_NPC(victim))
                act("$n tried to rob you.",TRUE,ch,0,victim,TO_VICT);
        } else {
            /* Steal some gold coins */
            gold = (int) ((GET_GOLD(victim)*number(1,25))/100);
            if (gold > 0) {
                GET_GOLD(ch) += gold;
                GET_GOLD(victim) -= gold;
                sprintf(buf, "Bingo! You got %d gold coins.\n\r", gold);
                send_to_char(buf, ch);
                if(!IS_NPC(victim))
                    act("$n robbed you.",TRUE,ch,0,victim,TO_VICT);
            } else {
                send_to_char("You couldn't get any gold...\n\r", ch);
            }
        }
    }
	// steal하는 경우 criminal tag를 두기
	//
	if (number(1,5) == 1) SET_BIT(ch->specials.act, PLR_CRIMINAL);

    if (ohoh && AWAKE(victim))
        if (!IS_SET(victim->specials.act, ACT_NICE_THIEF))
            hit(victim, ch, TYPE_UNDEFINED);
}

bool do_practice(struct char_data *ch, char *arg, int cmd)
{
    int i, count;
    extern char *spells[], *how_good();
    extern struct spell_info_type spell_info[MAX_SPL_LIST];
    char prbuf[80*MAX_SKILLS], buf[80], buf2[80];
    int is_solved_quest(struct char_data *ch, int quest);

	strcpy(prbuf, "You practiced :\n\r") ;
    for(i=0, count=0; *spells[i] != '\n';i++) {
        if(! *spells[i]) continue;
        if(! ch->skills[i+1].learned) continue;
        if((!is_solved_quest(ch, QUEST_REMORTAL)  ||
			spell_info[i+1].min_level[GET_CLASS_O(ch)-1] > GET_LEVEL(ch))
					&& 
            spell_info[i+1].min_level[GET_CLASS(ch)-1] > GET_LEVEL(ch)) continue;
/*
  send_to_char(spells[i], ch);
  send_to_char(how_good(ch->skills[i+1].learned), ch);
  send_to_char("\n\r", ch);
*/
        sprintf(buf, "%s %s", spells[i], how_good(ch->skills[i+1].learned));
        if (count %2 == 0)
            sprintf(buf2, "%-35s", buf) ;
        else
            sprintf(buf2, "%s\n\r", buf) ;
        strcat(prbuf, buf2) ;
        count++;
    }
    strcat(prbuf, "\n\r");
    page_string(ch->desc, prbuf, 1);
    return(TRUE);
}

void do_typo(struct char_data *ch, char *argument, int cmd)
{
    FILE *fl;
    char str[MAX_STRING_LENGTH];

    if (IS_NPC(ch)) {
        send_to_char("Monsters can't spell - leave me alone.\n\r", ch);
        return;
    }

    /* skip whites */
    for (; isspace(*argument) && !is_hangul(*argument) ; argument++);
    if (!*argument) {
        send_to_char("I beg your pardon?\n\r", ch);
        return;
    }

    if (!(fl = fopen(TYPO_FILE, "a"))) {
        perror ("do_typo");
        send_to_char("Could not open the typo-file.\n\r", ch);
        return;
    }

    sprintf(str, "TYPO**%s[%d]: %s\n",
            GET_NAME(ch), world[ch->in_room].number, argument);
    fputs(str, fl);
    fclose(fl);
	log(str) ;
    send_to_char("Ok. thanks.\n\r", ch);
}

void do_cookie(struct char_data *ch, char *argument, int cmd)
{
    FILE *fl;
    char str[MAX_STRING_LENGTH];

    if (IS_NPC(ch)) {
        send_to_char("Monsters can't spell - leave me alone.\n\r", ch);
        return;
    }
	if (GET_LEVEL(ch) < IMO-1) {
		send_to_char("To write cookie, you should be Dangun.\n\r", ch);
		return;
	}

    /* skip whites */
    for (; isspace(*argument) && !is_hangul(*argument) ; argument++);
    if (!*argument) {
        send_to_char("I beg your pardon?\n\r", ch);
        return;
    }

    if (!(fl = fopen(FORTUNE_COOKIE_FILE, "a"))) {
        perror ("do_cookie");
        send_to_char("Could not open the cookie-file.\n\r", ch);
        return;
    }

    sprintf(str, "%s (%s)\n", argument, GET_NAME(ch));
    fputs(str, fl);
    fclose(fl);
	log(str) ;
    send_to_char("Ok. thanks.\n\r", ch);
}

void do_brief(struct char_data *ch, char *argument, int cmd)
{
    if (IS_NPC(ch))
        return;

    if (IS_SET(ch->specials.act, PLR_BRIEF))
    {
        send_to_char("Brief mode off.\n\r", ch);
        REMOVE_BIT(ch->specials.act, PLR_BRIEF);
    } else {
        send_to_char("Brief mode on.\n\r", ch);
        SET_BIT(ch->specials.act, PLR_BRIEF);
    }
}

void do_compact(struct char_data *ch, char *argument, int cmd)
{
    if (IS_NPC(ch))
        return;
    if (IS_SET(ch->specials.act, PLR_COMPACT)) {
        send_to_char("You are now in the uncompacted mode.\n\r", ch);
        REMOVE_BIT(ch->specials.act, PLR_COMPACT);
    } else {
        send_to_char("You are now in compact mode.\n\r", ch);
        SET_BIT(ch->specials.act, PLR_COMPACT);
    }
}


void do_group(struct char_data *ch, char *argument, int cmd)
{
    char buf[256], name[256];
    struct char_data *victim, *k;
    struct follow_type *f;
    bool found;

    one_argument(argument, name);

    if (!*name) {
        if (!IS_AFFECTED(ch, AFF_GROUP)) {
            send_to_char("But you are a member of no group?!\n\r", ch);
        } else {
            send_to_char("Your group consists of:\n\r", ch);
            if (ch->master)
                k = ch->master;
            else
                k = ch;

            if (IS_AFFECTED(k, AFF_GROUP))
                sprintf(buf,
				"  [ %4d/%4d %4d/%4d %4d/%4d ]   %s (Head of group)\n\r",
                        GET_HIT(k),GET_MAX_HIT(k),GET_MANA(k),
                        GET_MAX_MANA(k),GET_MOVE(k),GET_MAX_MOVE(k),
						GET_NAME(k) );
            else {
                send_to_char("Your boss is not in group.\n\r", ch) ;
                return ;
			}
			send_to_char(buf, ch);

            for(f=k->followers; f; f=f->next)
                if (IS_AFFECTED(f->follower, AFF_GROUP)){
                    sprintf(buf,"  [ %4d/%4d %4d/%4d %4d/%4d ]   %s\n\r",
                            GET_HIT(f->follower),GET_MAX_HIT(f->follower),
                            GET_MANA(f->follower),GET_MAX_MANA(f->follower),
                            GET_MOVE(f->follower),GET_MAX_MOVE(f->follower),
							GET_NAME(f->follower));
					send_to_char(buf, ch);
                }
        }

        return;
    }

    if (!(victim = get_char_room_vis(ch, name))) {
        send_to_char("No one here by that name.\n\r", ch);
    } else {

        if (ch->master) {
            act("You can not enroll group members without being head of a group.",
                FALSE, ch, 0, 0, TO_CHAR);
            return;
        }

        found = FALSE;

        if (victim == ch)
            found = TRUE;
        else {
            for(f=ch->followers; f; f=f->next) {
                if (f->follower == victim) {
                    found = TRUE;
                    break;
                }
            }
        }
    
        if (found) {
            if (IS_AFFECTED(victim, AFF_GROUP)) {
                act("$n has been kicked out of the group!", FALSE, victim, 0, ch, TO_ROOM);
                act("You are no longer a member of the group!", FALSE, victim, 0, 0, TO_CHAR);
                REMOVE_BIT(victim->specials.affected_by, AFF_GROUP);
            } else {
                act("$n is now a group member.", FALSE, victim, 0, 0, TO_ROOM);
                act("You are now a group member.", FALSE, victim, 0, 0, TO_CHAR);
                SET_BIT(victim->specials.affected_by, AFF_GROUP);
            }
        } else {
            act("$N must follow you, to enter the group", FALSE, ch, 0, victim, TO_CHAR);
        }
    }
}

void do_quaff(struct char_data *ch, char *argument, int cmd)
{
    char buf[100];
    struct obj_data *temp;
    int i, spell_no ;
    bool equipped;

    equipped = FALSE;

    one_argument(argument,buf);

    if (!(temp = get_obj_in_list_vis(ch,buf,ch->carrying))) {
        temp = ch->equipment[HOLD];
        equipped = TRUE;
        if ((temp==0) || !isname(buf, temp->name)) {
            act("You do not have that item.",FALSE,ch,0,0,TO_CHAR);
            return;
        }
    }

    if (temp->obj_flags.type_flag!=ITEM_POTION)
    {
        act("You can only quaff potions.",FALSE,ch,0,0,TO_CHAR);
        return;
    }

    act("$n quaffs $p.", TRUE, ch, temp, 0, TO_ROOM);
    act("You quaff $p which dissolves.",FALSE,ch,temp,0,TO_CHAR);

    for (i=1; i<4; i++) {
        spell_no = temp->obj_flags.value[i] ;
        if ( spell_no >= 1 && spell_no < MAX_SPL_LIST )
            ((*spell_info[ spell_no ].spell_pointer)
             ((byte) temp->obj_flags.value[0], ch, "", SPELL_TYPE_POTION, ch, 0));
	}

    if (equipped)
        unequip_char(ch, HOLD);

    extract_obj(temp);
}


void do_recite(struct char_data *ch, char *argument, int cmd)
{
    char buf[100];
    struct obj_data *scroll, *obj;
    struct char_data *victim;
    int i, bits, spell_no ;
    bool equipped;

    equipped = FALSE;
    obj = 0;
    victim = 0;

    argument = one_argument(argument,buf);

    if (!(scroll = get_obj_in_list_vis(ch,buf,ch->carrying))) {
        scroll = ch->equipment[HOLD];
        equipped = TRUE;
        if ((scroll==0) || !isname(buf, scroll->name)) {
            act("You do not have that item.",FALSE,ch,0,0,TO_CHAR);
            return;
        }
    }

    if (scroll->obj_flags.type_flag!=ITEM_SCROLL)
    {
        act("Recite is normally used for scroll's.",FALSE,ch,0,0,TO_CHAR);
        return;
    }

	if (*argument) {
		bits = generic_find(argument, FIND_OBJ_INV, ch, &victim, &obj);
		if ( bits == 0 ) {
			bits = generic_find(argument, FIND_OBJ_ROOM |
                                FIND_OBJ_EQUIP | FIND_CHAR_ROOM,
                                ch, &victim, &obj);
			if (bits == 0) {
				send_to_char("No such thing around to recite the scroll on.\n\r", ch);
				return;
            }
        }
    }
	else {
  		victim = ch;
    }

    act("$n recites $p.", TRUE, ch, scroll, 0, TO_ROOM);
    act("You recite $p which dissolves.",FALSE,ch,scroll,0,TO_CHAR);

    for (i=1; i<4; i++) {
        spell_no = scroll->obj_flags.value[i] ;
        if ( spell_no >= 1 && spell_no < MAX_SPL_LIST &&
             *spell_info[spell_no].spell_pointer != NULL )
            ((*spell_info[ spell_no ].spell_pointer)
             ((byte) scroll->obj_flags.value[0], ch, "", SPELL_TYPE_SCROLL, victim, obj));
	}

    if (equipped)
        unequip_char(ch, HOLD);

    extract_obj(scroll);
}



void do_use(struct char_data *ch, char *argument, int cmd)
{
    char buf[100];
    struct char_data *tmp_char;
    struct obj_data *tmp_object, *stick;
    int bits, spell_no ;

    argument = one_argument(argument,buf);

    if (ch->equipment[HOLD] == 0 ||
        !isname(buf, ch->equipment[HOLD]->name)) {
        act("You do not hold that item in your hand.",FALSE,ch,0,0,TO_CHAR);
        return;
    }

    stick = ch->equipment[HOLD];

    if (stick->obj_flags.type_flag == ITEM_STAFF)
    {
        act("$n taps $p three times on the ground.",TRUE, ch, stick, 0,TO_ROOM);
        act("You tap $p three times on the ground.",FALSE,ch, stick, 0,TO_CHAR);

        if (stick->obj_flags.value[2] > 0) {  /* Is there any charges left? */
            stick->obj_flags.value[2]--;
            spell_no = stick->obj_flags.value[3] ;
            if ( spell_no >= 1 && spell_no < MAX_SPL_LIST )
                ((*spell_info[ spell_no ].spell_pointer)
                 ((byte) stick->obj_flags.value[0], ch, "", SPELL_TYPE_STAFF, 0, 0));

        } else {
            send_to_char("The staff seems powerless.\n\r", ch);
        }
    } else if (stick->obj_flags.type_flag == ITEM_WAND) {

        bits = generic_find(argument, FIND_CHAR_ROOM | FIND_OBJ_INV | FIND_OBJ_ROOM |
                            FIND_OBJ_EQUIP, ch, &tmp_char, &tmp_object);
        if (bits) {
            if (bits == FIND_CHAR_ROOM) {
                act("$n point $p at $N.", TRUE, ch, stick, tmp_char, TO_ROOM);
                act("You point $p at $N.",FALSE,ch, stick, tmp_char, TO_CHAR);
            } else {
                act("$n point $p at $P.", TRUE, ch, stick, tmp_object, TO_ROOM);
                act("You point $p at $P.",FALSE,ch, stick, tmp_object, TO_CHAR);
            }

            if (stick->obj_flags.value[2] > 0) { /* Is there any charges left? */
                stick->obj_flags.value[2]--;
                spell_no = stick->obj_flags.value[3] ;
                if ( spell_no >= 1 && spell_no < MAX_SPL_LIST )
                    ((*spell_info[ spell_no ].spell_pointer)
                     ((byte) stick->obj_flags.value[0], ch, "", SPELL_TYPE_WAND, tmp_char, tmp_object));
            } else {
                send_to_char("The wand seems powerless.\n\r", ch);
            }
        } else {
            send_to_char("What should the wand be pointed at?\n\r", ch);
        }
    } else {
        send_to_char("Use is normally only for wand's and staff's.\n\r", ch);
    }
}

void do_post(struct char_data *ch, char *argument, int cmd)
{
    send_to_char( "You can only post on board.\n\r", ch );
}

