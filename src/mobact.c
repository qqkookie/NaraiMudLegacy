/* ************************************************************************
*  file: mobact.c , Mobile action module.                 Part of DIKUMUD *
*  Usage: Procedures generating 'intelligent' behavior in the mobiles.    *
*  Copyright (C) 1990, 1991 - see 'license.doc' for complete information. *
************************************************************************* */

#include <stdio.h>

#include "structs.h"
#include "utils.h"
#include "db.h"
#include "comm.h"
#include "spells.h"

extern struct char_data *character_list;
extern struct index_data *mob_index;
extern struct room_data *world;
extern struct str_app_type str_app[];

void hit(struct char_data *ch, struct char_data *victim, int type);


void mobile_activity(void)
{
#ifdef DEATHFIGHT
	extern int	deathfight ;
#endif 
    register struct char_data *ch;
    struct char_data *tmp_ch,*cho_ch;
    struct obj_data *obj, *best_obj, *worst_obj;
    int door, found, max, min;
    char buf[100];

    extern int no_specials;

    void do_move(struct char_data *ch, char *argument, int cmd);
    void do_get(struct char_data *ch, char *argument, int cmd);

    for (ch = character_list; ch; ch = ch->next) {
        if (IS_MOB(ch))
        {
            /* Examine call for special procedure */
            if (IS_SET(ch->specials.act, ACT_SPEC) && !no_specials) {
                if (!mob_index[ch->nr].func) {
                    sprintf(buf,"Attempting to call a non-existing MOB func.\n (mobact.c) %s",
                            ch->player.short_descr );
                    log(buf);
                    REMOVE_BIT(ch->specials.act, ACT_SPEC);
                } else {
                    if ((*mob_index[ch->nr].func) (ch, 0, ""))
                        continue;
                }
            }
            if (AWAKE(ch) && !(ch->specials.fighting)) {
                if (IS_SET(ch->specials.act, ACT_SCAVENGER)) {
                    if (world[ch->in_room].contents && !number(0,10)) {
                        for (max = 1, best_obj = 0, obj = world[ch->in_room].contents;
                             obj; obj = obj->next_content) {
                            if (CAN_GET_OBJ(ch, obj)) {
                                if (obj->obj_flags.cost > max) {
                                    best_obj = obj;
                                    max = obj->obj_flags.cost;
                                }
                            }
                        } /* for */

                        if (best_obj) {
                            obj_from_room(best_obj);
                            obj_to_char(best_obj, ch);
                            act("$n gets $p.",FALSE,ch,best_obj,0,TO_ROOM);
                        }
                    }
                } /* Scavenger */
                if (!IS_SET(ch->specials.act, ACT_SENTINEL) && 
                    (GET_POS(ch) == POSITION_STANDING) &&
                    ((door = number(0, 45)) <= 5) && CAN_GO(ch,door) &&
                    !IS_SET(world[EXIT(ch, door)->to_room].room_flags, NO_MOB)) {
                    if (ch->specials.last_direction == door) {
                        ch->specials.last_direction = -1;
                    } else {
                        if (!IS_SET(ch->specials.act, ACT_STAY_ZONE)) {
                            ch->specials.last_direction = door;
                            do_move(ch, "", ++door);
                        } else {
                            if (world[EXIT(ch, door)->to_room].zone == world[ch->in_room].zone) {
                                ch->specials.last_direction = door;
                                do_move(ch, "", ++door);
                            }
                        }
                    }
                } /* if can go */

#ifdef DEATHFIGHT
                if ( deathfight == 0 ) {	/* deathfight second round */
#endif 

                    if (IS_SET(ch->specials.act,ACT_AGGRESSIVE)) {
                        found = FALSE;
                        for (tmp_ch = world[ch->in_room].people; tmp_ch;
                             tmp_ch = tmp_ch->next_in_room) {
                            if (!IS_NPC(tmp_ch) && CAN_SEE(ch, tmp_ch) && (GET_LEVEL(tmp_ch)<IMO)) {
                                if (!IS_SET(ch->specials.act, ACT_WIMPY) || !AWAKE(tmp_ch)) {
                                    if(!found){
                                        cho_ch=tmp_ch;
                                        found = TRUE;
                                        if(IS_EVIL(ch) && IS_GOOD(cho_ch) && IS_AFFECTED(cho_ch,AFF_PROTECT_EVIL)){
                                            if(!saves_spell(ch,SAVING_PARA) && GET_LEVEL(ch) < GET_LEVEL(cho_ch)){
                                                act("$n tries to attack, but failed miserably.",TRUE,ch,0,0,TO_ROOM);
                                                found = FALSE;
                                            }
                                        }
                                    } else {
                                        if(number(1,6)<=3)
                                            cho_ch=tmp_ch;
                                    } /* else */
                                } /* if IS_SET */
                            } /* if IS_NPC */
                        } /* for */
                        if(found)
                            hit(ch, cho_ch, 0);
                    } /* is aggressive */

#ifdef DEATHFIGHT
                }	/* checking deathfight round */
#endif 

            } /* If AWAKE(ch)   */

            if(IS_SET(ch->specials.act, ACT_THIEF)){
                thief(ch,0,"");
            }
            if(IS_SET(ch->specials.act, ACT_DRAGON)){
                dragon(ch,0,"");
            }
            if(IS_SET(ch->specials.act, ACT_MAGE)){
                magic_user(ch,0,"");
            }
            if(IS_SET(ch->specials.act, ACT_SHOOTER)){
                shooter(ch,0,"");
            }
            if(IS_SET(ch->specials.act, ACT_FIGHTER)){
                kickbasher(ch,0,"");
            }
            if(IS_SET(ch->specials.act, ACT_SPITTER)){
                spitter(ch,0,"");
            }
            if(IS_SET(ch->specials.act, ACT_CLERIC)){
                cleric(ch,0,"");
            }
            if(IS_SET(ch->specials.act, ACT_PALADIN)){
                paladin(ch,0,"");
            }
            if(IS_SET(ch->specials.act, ACT_GUARD)){
                cityguard(ch,0,"");
            }
            if(IS_SET(ch->specials.act, ACT_SUPERGUARD)){
                superguard(ch,0,"");
            }
            if(IS_SET(ch->specials.act, ACT_RESCUER)){
                rescuer(ch,0,"");
            }
            if(IS_SET(ch->specials.act, ACT_HELPER)){
                helper(ch,0,"");
            }
            if(IS_SET(ch->specials.act, ACT_SPELL_BLOCKER)){
                spell_blocker(ch,0,"");
            }
   
        }   /* If IS_MOB(ch)  */
    } /* for..*/
}

