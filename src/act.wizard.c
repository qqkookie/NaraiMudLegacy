/* ************************************************************************
*  file: actwiz.c , Implementation of commands.           Part of DIKUMUD *
*  Usage : Wizard Commands.                                               *
*  Copyright (C) 1990, 1991 - see 'license.doc' for complete information. *
************************************************************************* */

#include <stdio.h>
#include <string.h>
#include <ctype.h>

#include <sys/time.h>
#include <sys/resource.h>

#include "structs.h"
#include "utils.h"
#include "comm.h"
#include "interpreter.h"
#include "handler.h"
#include "db.h"
#include "spells.h"
#include "limits.h"
#include "quest.h"	/* cyb */
#include "command.h"
#include "memory.h"

/*   external vars  */

extern struct room_data *world;
extern struct char_data *character_list;
extern struct obj_data *object_list;
extern struct descriptor_data *descriptor_list;
extern struct title_type titles[4][IMO+4];
extern struct index_data *mob_index;
extern struct index_data *obj_index;
extern struct int_app_type int_app[26];
extern struct wis_app_type wis_app[26];
extern struct player_index_element *player_table;

/* external functs */

void set_title(struct char_data *ch);
int str_cmp(char *arg1, char *arg2);
struct time_info_data age(struct char_data *ch);
void sprinttype(int type, char *names[], char *result);
void sprintbit(long vektor, char *names[], char *result);
int mana_limit(struct char_data *ch);
int hit_limit(struct char_data *ch);
int move_limit(struct char_data *ch);
int mana_gain(struct char_data *ch);
int hit_gain(struct char_data *ch);
int move_gain(struct char_data *ch);
extern void do_look(struct char_data *ch, char *argument, int cmd);

do_emote(struct char_data *ch, char *argument, int cmd)
{
    int i;
    static char buf[MAX_STRING_LENGTH];

    if (IS_NPC(ch))
        return;
    for (i = 0; *(argument + i) == ' '; i++);
    if (!*(argument + i))
        send_to_char("Yes.. But what?\n\r", ch);
    else {
        sprintf(buf,"$n %s", argument + i);
        act(buf,FALSE,ch,0,0,TO_ROOM);
        send_to_char("Ok.\n\r", ch);
    }
}

void do_transform(struct char_data *ch, char *argument, int cmd)
{
    struct char_data *tmp_ch;
    struct char_file_u tmp;
    int i_player;
    char buf[200];

    return;
    if(IS_NPC(ch)) return;
    one_argument(argument,buf);
    if(*buf) return;
    for(tmp_ch=character_list;tmp_ch;tmp_ch=tmp_ch->next){
        if(!strcmp(buf,GET_NAME(tmp_ch)) && !IS_NPC(tmp_ch)){
            send_to_char("Body already in game.\n\r",ch);
            return;
        }
    }
    if((i_player=load_char(buf,&tmp))<0){
        send_to_char("No such player\n\r",ch);
        return;
    }
    store_to_char_for_transform(&tmp, ch->desc->original);
    store_to_char_for_transform(&tmp, ch);
    ch->desc->pos=player_table[i_player].nr;
    unstash_char(ch,0);
    if ( stash_char(ch,0) < 0 ) {
        send_to_char("saving char failed.\n\r", ch) ;
	}
}

void do_echo(struct char_data *ch, char *argument, int cmd)
{
    int i;
    static char buf[MAX_STRING_LENGTH];
  
    if (IS_NPC(ch))
        return;

    for (i = 0; *(argument + i) == ' '; i++);

    if (!*(argument + i))
        send_to_char("That must be a mistake...\n\r", ch);
    else
    {
        sprintf(buf,"%s\n\r", argument + i);
        send_to_room_except(buf, ch->in_room, ch);
        send_to_char("Ok.\n\r", ch);
    }
}
void do_trans(struct char_data *ch, char *argument, int cmd)
{
    struct descriptor_data *i;
    struct char_data *victim;
    char buf[100];
    sh_int target;

    if (IS_NPC(ch))
        return;

    one_argument(argument,buf);
    if (!*buf)
        send_to_char("Who do you wich to transfer?\n\r",ch);
    else if (str_cmp("all", buf)) {
        if (!(victim = get_char_vis(ch,buf)))
            send_to_char("No-one by that name around.\n\r",ch);
        else {
            if(GET_LEVEL(ch)<IMO+3 && GET_LEVEL(victim) > GET_LEVEL(ch)){
                send_to_char("That might not be appreciated.\n\r",ch);
                return;
            }
            act("$n disappears in a mushroom cloud.",FALSE,victim,0,0,TO_ROOM);
            target = ch->in_room;
            char_from_room(victim);
            char_to_room(victim,target);
            act("$n arrives from a puff of smoke.",FALSE,victim,0,0,TO_ROOM);
            act("$n has transferred you!",FALSE,ch,0,victim,TO_VICT);
            do_look(victim,"",15);
            send_to_char("Ok.\n\r",ch);
        }
    } else { /* Trans All */
        if(ch->player.level >= (IMO+3))
            for (i = descriptor_list; i; i = i->next)
                if (i->character != ch && !i->connected) {
                    target = ch->in_room;
                    victim = i->character;
                    char_from_room(victim);
                    char_to_room(victim,target);
                    act("$n arrives from a puff of smoke.", FALSE, victim, 0, 0, TO_ROOM);
                    act("$n has transferred you!",FALSE,ch,0,victim,TO_VICT);
                    do_look(victim,"",15);
                }
        send_to_char("Ok.\n\r",ch);
    }
}

void do_at(struct char_data *ch, char *argument, int cmd)
{
    char command[MAX_INPUT_LENGTH], loc_str[MAX_INPUT_LENGTH];
    int loc_nr, location, original_loc;
    struct char_data *target_mob;
    struct obj_data *target_obj;
    extern int top_of_world;
  
    if (IS_NPC(ch))
        return;

    half_chop(argument, loc_str, command);
    if (!*loc_str)
    {
        send_to_char("You must supply a room number or a name.\n\r", ch);
        return;
    }
    if (ISDIGIT(*loc_str))
    {
        loc_nr = atoi(loc_str);
        for (location = 0; location <= top_of_world; location++)
            if (world[location].number == loc_nr)
                break;
            else if (location == top_of_world)
            {
                send_to_char("No room exists with that number.\n\r", ch);
                return;
            }
    }
    else if (target_mob = get_char_vis(ch, loc_str))
        location = target_mob->in_room;
    else if (target_obj = get_obj_vis(ch, loc_str))
        if (target_obj->in_room != NOWHERE)
            location = target_obj->in_room;
        else
        {
            send_to_char("The object is not available.\n\r", ch);
            return;
        } else {
            send_to_char("No such creature or object around.\n\r", ch);
            return;
        }

    /* a location has been found. */

    if((GET_LEVEL(ch)<(IMO+3))&&(IS_SET(world[location].room_flags,OFF_LIMITS))){
        send_to_char("That room is off-limits.\n",ch);
        return;
    }
    original_loc = ch->in_room;
    char_from_room(ch);
    char_to_room(ch, location);
    command_interpreter(ch, command);

    /* check if the guy's still there */
    for (target_mob = world[location].people; target_mob; target_mob =
             target_mob->next_in_room)
        if (ch == target_mob)
        {
            char_from_room(ch);
            char_to_room(ch, original_loc);
        }
}

void do_goto(struct char_data *ch, char *argument, int cmd)
{
    char buf[MAX_INPUT_LENGTH];
    int loc_nr, location,i,flag;
    struct char_data *target_mob, *pers;
    struct obj_data *target_obj;
    extern int top_of_world;

    if (IS_NPC(ch) || GET_LEVEL(ch) > (IMO+3))
        return;
    one_argument(argument, buf);
    if (!*buf) {
        send_to_char("You must supply a room number or a name.\n\r", ch);
        return;
    }
			
    if (ISDIGIT(*buf)) {
        loc_nr = atoi(buf);
        for (location = 0; location <= top_of_world; location++)
            if (world[location].number == loc_nr)
                break;
            else if (location == top_of_world) {
                send_to_char("No room exists with that number.\n\r", ch);
                return;
            }
    }
    else if (target_mob = get_char_vis(ch, buf))
        location = target_mob->in_room;
    else if (target_obj = get_obj_vis(ch, buf))
        if (target_obj->in_room != NOWHERE)
            location = target_obj->in_room;
        else {
            send_to_char("The object is not available.\n\r", ch);
            return;
        } else {
            send_to_char("No such creature or object around.\n\r", ch);
            return;
        }

    /* a location has been found. */

    if(GET_LEVEL(ch) < (IMO+2)){
        if (IS_SET(world[location].room_flags, OFF_LIMITS)){
            send_to_char("Sorry, off limits.\n",ch);
            return;
        }
        if (IS_SET(world[location].room_flags, PRIVATE)) {
            for (i = 0, pers = world[location].people; pers; pers =
                     pers->next_in_room, i++);
            if (i > 1) {
                send_to_char(
                    "There's a private conversation going on in that room.\n\r", ch);
                return;
            }
        }
    }
    flag=((GET_LEVEL(ch)>=(IMO+2))&&
          IS_SET(ch->specials.act,PLR_WIZINVIS));
    if(!flag)
        act("$n disappears in a puff of smoke.", FALSE, ch, 0, 0, TO_ROOM);
    char_from_room(ch);
    char_to_room(ch, location);
    if(!flag)
        act("$n appears with an ear-splitting bang.", FALSE, ch, 0,0,TO_ROOM);
    do_look(ch, "",15);
}

void do_demote(struct char_data *ch, char *argument, int cmd)
{
    struct char_data *victim;
    char name[100], buf[BUFSIZ];
    int level;
    void do_start(struct char_data *ch);

    sprintf(buf, "%s demotes %s", GET_NAME(ch), argument);
    log(buf);

    if (IS_NPC(ch) || GET_LEVEL(ch) > (IMO+3))
        return;
    one_argument(argument, name);
    if (*name) {
        if (!(victim = get_char_room_vis(ch, name))) {
            send_to_char("That player is not here.\n\r", ch);
            return;
        }
    } else {
        send_to_char("Demote who?\n\r", ch);
        return;
    }
    if (IS_NPC(victim)) {
        send_to_char("NO! Not on NPC's.\n\r", ch);
        return;
    }
    if(GET_LEVEL(victim) > IMO && !implementor(GET_NAME(ch)))
        return ;

    if(implementor(GET_NAME(victim)) && GET_LEVEL(victim) >= IMO+3)
        return ;

    do_start(victim);
}

void do_stat(struct char_data *ch, char *argument, int cmd)
{
    extern char *spells[];
    struct affected_type *aff;
    char arg1[MAX_STRING_LENGTH];
    char buf[MAX_STRING_LENGTH];
    char buf2[MAX_STRING_LENGTH];
    struct char_data *victim=0;
    struct room_data *rm=0;
    struct char_data *k=0;
    struct obj_data  *j=0;
    struct obj_data  *j2=0;
    struct extra_descr_data *desc;
    struct follow_type *fol;
    int i,i2,t,virtual;
    bool found;

    /* for objects */
    extern char *item_types[];
    extern char *wear_bits[];
    extern char *extra_bits[];
    extern char *drinks[];

    /* for rooms */
    extern char *dirs[];
    extern char *room_bits[];
    extern char *exit_bits[];
    extern char *sector_types[];

    /* for chars */
    extern char *equipment_types[];
    extern char *affected_bits[];
    extern char *apply_types[];
    extern char *pc_class_types[];
    extern char *npc_class_types[];
    extern char *action_bits[];
    extern char *player_bits[];
    extern char *position_types[];
    extern char *connected_types[];
    struct char_data *get_specific_vis(struct char_data *ch,char *name,int type);

    if (IS_NPC(ch))
        return;
    argument = one_argument(argument, arg1);
    if (!*arg1) {
        send_to_char("Stats on who or what?\n\r",ch);
        return;
    } else {
        /* stats on room */
        if((cmd != COM_PSTAT) && (cmd != COM_MSTAT) && (!str_cmp("room", arg1))) {
            rm = &world[ch->in_room];
            sprintf(buf,"Room: %s, Zone: %d. V-Num: %d, R-num: %d, Light: %d.\n\r",
                    rm->name, rm->zone, rm->number, ch->in_room, rm->light);
            send_to_char(buf, ch);
            sprinttype(rm->sector_type,sector_types,buf2);
            sprintf(buf, "Sector type : %s ", buf2);
            send_to_char(buf, ch);
            strcpy(buf,"Special procedure : ");
            strcat(buf,(rm->funct) ? "Exists\n\r" : "No\n\r");
            send_to_char(buf, ch);
            send_to_char("Room flags: ", ch);
            sprintbit((long) rm->room_flags,room_bits,buf);
            strcat(buf,"\n\r");
            send_to_char(buf,ch);
            send_to_char("Description:\n\r", ch);
            send_to_char(rm->description, ch);
            strcpy(buf, "Extra description keywords(s): ");
            if(rm->ex_description) {
                strcat(buf, "\n\r");
                for (desc = rm->ex_description; desc; desc = desc->next) {
                    strcat(buf, desc->keyword);
                    strcat(buf, "\n\r");
                }
                strcat(buf, "\n\r");
                send_to_char(buf, ch);
            } else {
                strcat(buf, "None\n\r");
                send_to_char(buf, ch);
            }
            strcpy(buf, "Chars present:\n\r");
            for (k = rm->people; k; k = k->next_in_room) {
                if(!CAN_SEE(ch,k)) continue;
                strcat(buf, "  ") ;
                strcat(buf, GET_NAME(k));
                strcat(buf,
                       (!IS_NPC(k) ? "(PC)\n\r" : (!IS_MOB(k) ? "(NPC)\n\r" : "(MOB)\n\r")));
            }
            strcat(buf, "\n\r");
            send_to_char(buf, ch);
            strcpy(buf, "Contents:\n\r");
            for (j = rm->contents; j; j = j->next_content) {
                strcat(buf, "  ") ;
                strcat(buf, j->name);
                strcat(buf, "\n\r");
            }
            strcat(buf, "\n\r");
            send_to_char(buf, ch);
            send_to_char("Exits:\n\r", ch);
            for (i = 0; i <= 5; i++) {
                if (rm->dir_option[i]) {
                    sprintf(buf,"Direction %s . Keyword : %s\n\r",
                            dirs[i], rm->dir_option[i]->keyword);
                    send_to_char(buf, ch);
                    strcpy(buf, "Description:\n\r  ");
                    if(rm->dir_option[i]->general_description)
                        strcat(buf, rm->dir_option[i]->general_description);
                    else
                        strcat(buf,"UNDEFINED\n\r");
                    send_to_char(buf, ch);
                    sprintbit(rm->dir_option[i]->exit_info,exit_bits,buf2);
                    sprintf(buf,"Exit flag: %s\n\rKey #: %d\n\rTo room(V-Num): %d\n\r",
                            buf2, rm->dir_option[i]->key,
                            world[rm->dir_option[i]->to_room].number) ;
                    send_to_char(buf, ch);
                }
            }
            return;
        }
        if((cmd != COM_MSTAT) && (cmd != COM_PSTAT) && (j = get_obj_vis(ch, arg1))) {
            virtual = (j->item_number >= 0) ? obj_index[j->item_number].virtual : 0;
            sprintf(buf,"Object name: [%s], R-num: [%d], V-number: [%d] Item type: ",
                    j->name, j->item_number, virtual);
            sprinttype(GET_ITEM_TYPE(j),item_types,buf2);
            strcat(buf,buf2); strcat(buf,"\n\r");
            send_to_char(buf, ch);
            sprintf(buf, "Short desc: %s\n\rLong desc:\n\r%s\n\r",
                    ((j->short_description) ? j->short_description : "None"),
                    ((j->description) ? j->description : "None") );
            send_to_char(buf, ch);
            if(j->ex_description){
                strcpy(buf, "Extra desc keyword(s):\n\r");
                for (desc = j->ex_description; desc; desc = desc->next) {
                    strcat(buf, desc->keyword);
                    strcat(buf, "\n\r");
                }
                send_to_char(buf, ch);
            } else {
                strcpy(buf,"Extra desc keyword(s): None\n\r");
                send_to_char(buf, ch);
            }
            send_to_char("Can be worn on :", ch);
            sprintbit(j->obj_flags.wear_flags,wear_bits,buf);
            strcat(buf,"\n\r");
            send_to_char(buf, ch);
            send_to_char("Extra flags: ", ch);
            sprintbit(GET_OBJ_STAT(j), extra_bits, buf);
            strcat(buf,"\n\r");
            send_to_char(buf,ch);
            sprintf(buf,"Weight: %d, Value: %d, Timer: %d\n\r",
                    j->obj_flags.weight,j->obj_flags.cost, j->obj_flags.timer);
            send_to_char(buf, ch);
            sprintf(buf,"Values 0-3 : [%d] [%d] [%d] [%d]\n\r",
                    j->obj_flags.value[0],
                    j->obj_flags.value[1],
                    j->obj_flags.value[2],
                    j->obj_flags.value[3]);
            send_to_char(buf, ch);
            send_to_char("Can affect char :\n\r", ch);
            for (i=0;i<MAX_OBJ_AFFECT;++i) {
                sprinttype(j->affected[i].location,apply_types,buf2);
                sprintf(buf,"  Affects: %s by %d\n\r",buf2,j->affected[i].modifier);
                send_to_char(buf, ch);      
            }
            return;
        }
        if(cmd==COM_PSTAT) k=get_specific_vis(ch,arg1,0);
        else if(cmd==COM_MSTAT) k=get_specific_vis(ch,arg1,1);
        else k=get_char_vis(ch,arg1);
        if(k){
            sprintf(buf,"Sex: %d,",k->player.sex);
            sprintf(buf2, " %s, Name: %s\n\r",
                    (!IS_NPC(k) ? "PC" : (!IS_MOB(k) ? "NPC" : "MOB")),
                    GET_NAME(k));
            strcat(buf, buf2);
            send_to_char(buf, ch);
            if(IS_NPC(k)) {
                sprintf(buf, "V-Number [%d]\n\r", mob_index[k->nr].virtual);
                send_to_char(buf, ch);
                strcpy(buf,"Short desc: ");
                strcat(buf,(k->player.short_descr ? k->player.short_descr : "None"));
                strcat(buf,"\n\r");
                send_to_char(buf,ch);
                send_to_char("Long desc: ", ch);
                if (k->player.long_descr)
                    send_to_char(k->player.long_descr, ch);
                else
                    send_to_char("None.\n\r", ch);
            } else {
                strcpy(buf,"Title: ");
                strcat(buf, (k->player.title ? k->player.title : "None"));
                strcat(buf,"\n\r");
                send_to_char(buf,ch);
            }
            if (IS_NPC(k)) {
                strcpy(buf,"Monster Class: ");
                sprinttype(k->player.class,npc_class_types,buf2);
            } else {
                strcpy(buf,"Class: ");
                sprinttype(k->player.class,pc_class_types,buf2);
            }
            strcat(buf, buf2);
            sprintf(buf2,"  Level [%d] Alignment[%d]\n\r",k->player.level,
                    k->specials.alignment);
            strcat(buf, buf2);
            send_to_char(buf, ch);
            strcpy(buf,"Birth: ");
            t=k->player.time.birth; 
            strncat(buf,(char *)ctime((time_t *)&t),24);
            strcat(buf,", Logon: ");
            t=k->player.time.logon; 
            strncat(buf,(char *)ctime((time_t *)&t),24);
            t=k->player.time.played; i=t%86400;
            sprintf(buf2,"\n\rPlayed: %d days, %d:%02d\n\r",
                    t/86400,i/3600,(i+30)%60);
            strcat(buf,buf2);
            send_to_char(buf, ch);
            sprintf(buf,"Age: %d Y, %d M, %d D, %d H.",
                    age(k).year, age(k).month, age(k).day, age(k).hours);
            sprintf(buf2,"  Ht: %d cm, Wt: %d lbs\n\r",GET_HEIGHT(k),GET_WEIGHT(k));
            strcat(buf,buf2);
            send_to_char(buf,ch);
            sprintf(buf,"Str:[%d/%d]  Int:[%d]  Wis:[%d]  Dex:[%d]  Con:[%d]\n\r",
                    GET_STR(k), GET_ADD(k), GET_INT(k),
                    GET_WIS(k), GET_DEX(k), GET_CON(k) );
            send_to_char(buf,ch);
            sprintf(buf,"Mana: %d/%d+%d, Hits: %d/%d+%d, Moves: %d/%d+%d\n\r",
                    GET_MANA(k),mana_limit(k),mana_gain(k),
                    GET_HIT(k),hit_limit(k),hit_gain(k),
                    GET_MOVE(k),move_limit(k),move_gain(k) );
            send_to_char(buf,ch);
            sprintf(buf,"AC: %d/10, Gold: %d, Exp: %d, Hitroll: %d, Damroll: %d, Regen: %d\n\r",
                    GET_AC(k), GET_GOLD(k), GET_EXP(k),
                    k->points.hitroll, k->points.damroll, k->regeneration );
            send_to_char(buf,ch);
            if (IS_NPC(k)) {
                strcpy(buf,"Special: ");
                strcat(buf,(mob_index[k->nr].func ? "Exists\n\r" : "None\n\r"));
                send_to_char(buf, ch);
            }
            if (IS_NPC(k)) {
                sprintf(buf, "NPC Bare Hand Damage %dd%d.\n\r",
                        k->specials.damnodice, k->specials.damsizedice);
                send_to_char(buf, ch);
            }
            sprintf(buf,"Carried weight: %d   Carried items: %d\n\r",
                    IS_CARRYING_W(k),
                    IS_CARRYING_N(k) );
            send_to_char(buf,ch);
            for(i=0,j=k->carrying;j;j=j->next_content,i++);
            sprintf(buf,"Items in inv: %d, ",i);
            for(i=0,i2=0;i<MAX_WEAR;i++)
                if (k->equipment[i]) i2++;
            sprintf(buf2,"Items in equ: %d\n\r", i2);
            strcat(buf,buf2);
            send_to_char(buf, ch);
            if(k->desc){
                sprinttype(k->desc->connected,connected_types,buf2);
                strcpy(buf,"Connected: ");
                strcat(buf,buf2);
                sprintf(buf2," %s (%d)\n\r",k->desc->host,k->desc->descriptor);
                strcat(buf,buf2);
                send_to_char(buf,ch);
            }
            if(IS_NPC(k)) {
                strcpy(buf,"NPC flags: ");
                sprintbit(k->specials.act,action_bits,buf2);
            } else {
                strcpy(buf,"PC flags: ");
                sprintbit(k->specials.act,player_bits,buf2);
            }
            strcat(buf,buf2);
            strcat(buf,"\n\r");
            send_to_char(buf, ch);

            if(!IS_NPC(k)){
                sprintf(buf, "Thirst: %d, Hunger: %d, Drunk: %d\n\r",
                        k->specials.conditions[THIRST],
                        k->specials.conditions[FULL],
                        k->specials.conditions[DRUNK]);
                sprintf(buf2,"Practices: %d\n\r",k->specials.spells_to_learn);
                strcat(buf,buf2);
                send_to_char(buf, ch);
            }
            sprintf(buf,"Master is '%s'\n\r",
                    ((k->master) ? GET_NAME(k->master) : "NOBODY"));
            send_to_char(buf, ch);
            send_to_char("Followers are:\n\r", ch);
            for(fol=k->followers; fol; fol = fol->next)
                act("    $N", FALSE, ch, 0, fol->follower, TO_CHAR);
            /* Showing the bitvector */
            sprintbit(k->specials.affected_by,affected_bits,buf);
            send_to_char("Affected by: ", ch);
            strcat(buf,"\n\r");
            send_to_char(buf, ch);
            if (k->affected) {
                send_to_char("Affecting Spells:\n\r", ch);
                for(aff = k->affected; aff; aff = aff->next) {
                    sprintf(buf,"%s: %s by %d, %d hrs, bits: ",spells[aff->type-1],
                            apply_types[aff->location],aff->modifier,aff->duration);
                    sprintbit(aff->bitvector,affected_bits,buf2);
                    strcat(buf,buf2);
                    strcat(buf,"\n\r");
                    send_to_char(buf, ch);
                }
            }
            if(!IS_NPC(k)){
                buf[0]=0;
                for(i=0;i<MAX_SKILLS;++i){
                    sprintf(buf2,"%3d",k->skills[i].learned);
                    strcat(buf,buf2);
                    if(!((i+1)%24)) strcat(buf,"\n\r");
                }
                strcat(buf,"\n\r") ;
                send_to_char(buf,ch);
            }
            if (!IS_NPC(k)) {	/* print solved quest */
                send_to_char("Quest Solved:\n\r", ch) ;
                send_to_char(what_is_solved(k), ch) ;
			}
            return;
        } else {
            send_to_char("No mobile or object by that name in the world\n\r", ch);
        }
    }
}

void do_shutdow(struct char_data *ch, char *argument, int cmd)
{
    send_to_char("If you want to shut something down - say so!\n\r", ch);
}

void do_shutdown(struct char_data *ch, char *argument, int cmd)
{
    extern int shutdowngame;
    char arg[MAX_INPUT_LENGTH];

    if (IS_NPC(ch) || GET_LEVEL(ch) > (IMO+3))
        return;
    one_argument(argument, arg);
    if (!*arg) {
        send_to_all("Shutting down immediately.\n\r");
        shutdowngame = 1;
    } else
        send_to_char("Go shut down someone your own size.\n\r", ch);
}
void do_snoop(struct char_data *ch, char *argument, int cmd)
{
    static char arg[MAX_STRING_LENGTH];
    struct char_data *victim;
    int diff;

    if (!ch->desc)
        return;
    if (IS_NPC(ch))
        return;
    one_argument(argument, arg);
    if(!*arg) {
        send_to_char("Snoop who ?\n\r",ch);
        return;
    }
    if(!(victim=get_char_vis(ch, arg))) {
        send_to_char("No such person around.\n\r",ch);
        return;
    }
    if(!victim->desc) {
        send_to_char("There's no link.. nothing to snoop.\n\r",ch);
        return;
    }
    if(victim == ch) {
        send_to_char("Ok, you just snoop yourself.\n\r",ch);
        if(ch->desc->snoop.snooping) {
            ch->desc->snoop.snooping->desc->snoop.snoop_by = 0;
            ch->desc->snoop.snooping = 0;
        }
        return;
    }
    if(victim->desc->snoop.snoop_by){
        send_to_char("Busy already. \n\r",ch);
        return;
    }
    diff=GET_LEVEL(victim)-GET_LEVEL(ch);
    if(diff >= 0){
        send_to_char("You failed.\n\r",ch);
        return;
    }
    send_to_char("Ok. \n\r",ch);
    if(ch->desc->snoop.snooping)
        ch->desc->snoop.snooping->desc->snoop.snoop_by = 0;
    ch->desc->snoop.snooping = victim;
    victim->desc->snoop.snoop_by = ch;
    return;
}
void do_switch(struct char_data *ch, char *argument, int cmd)
{
    static char arg[MAX_STRING_LENGTH];
    char buf[70];
    struct char_data *victim;

    if (IS_NPC(ch))
        return;
    one_argument(argument, arg);
    if (!*arg) {
        send_to_char("Switch with who?\n\r", ch);
    } else {
        if (!(victim = get_char(arg)))
            send_to_char("They aren't here.\n\r", ch);
        else {
            if (ch == victim) {
                send_to_char("He he he... We are jolly funny today, eh?\n\r", ch);
                return;
            }
            if (!ch->desc||ch->desc->snoop.snoop_by || ch->desc->snoop.snooping) {
                send_to_char(
                    "You can't do that, the body is already in use.\n\r",ch);
                return;
            }
            if(victim->desc || (!IS_NPC(victim))) {
                if(GET_LEVEL(victim) > GET_LEVEL(ch))
                    send_to_char("They aren't here.\n\r", ch);
                else
                    send_to_char(
                        "You can't do that, the body is already in use!\n\r",ch);
            } else {
                send_to_char("Ok. Type 'return' at the end of the trip.\n\r", ch);
                ch->desc->character = victim;
                ch->desc->original = ch;
                victim->desc = ch->desc;
                ch->desc = 0;
            }
        }
    }
}

void do_return(struct char_data *ch, char *argument, int cmd)
{
    static char arg[MAX_STRING_LENGTH];
    char buf[70];

    if(!ch->desc)
        return;

    if(!ch->desc->original)
    { 
        send_to_char("Eh?\n\r", ch);
        return;
    }
    else
    {
        send_to_char("You return to your original body.\n\r",ch);

        ch->desc->character = ch->desc->original;
        ch->desc->original = 0;

        ch->desc->character->desc = ch->desc; 
        ch->desc = 0;
    }
}


void do_force(struct char_data *ch, char *argument, int cmd)
{
    struct descriptor_data *i;
    struct char_data *vict;
    char name[100], to_force[100],buf[100]; 
    int diff;

    if (IS_NPC(ch) || GET_LEVEL(ch) > (IMO+3))
        return;

    half_chop(argument, name, to_force);

    if (!*name || !*to_force)
        send_to_char("Who do you wish to force to do what?\n\r", ch);
    else if (str_cmp("all", name)) {
        if (!(vict = get_char_vis(ch, name)))
            send_to_char("No-one by that name here..\n\r", ch);
        else {
            diff=GET_LEVEL(ch)-GET_LEVEL(vict);
            if ((!IS_NPC(vict)) && (diff <= 0)){
                send_to_char("Oh no you don't!!\n\r", ch);
            } else {
                if(GET_LEVEL(ch) < (IMO+3))
                    sprintf(buf, "$n has forced you to '%s'.", to_force);
                else
                    buf[0]=0;
                act(buf, FALSE, ch, 0, vict, TO_VICT);
                send_to_char("Ok.\n\r", ch);
                command_interpreter(vict, to_force);
            }
        }
    } else { /* force all */
        if(GET_LEVEL(ch) < (IMO+3)){
            send_to_char("Force all's are a bad idea these days.\n\r",ch);
            return;
        }
        for (i = descriptor_list; i; i = i->next)
            if (i->character != ch && i->connected == CON_PLYNG ) {
                vict = i->character;
                command_interpreter(vict, to_force);
            }
        send_to_char("Ok.\n\r", ch);
    }
}

void do_load(struct char_data *ch, char *argument, int cmd)
{
    struct char_data *mob;
    struct obj_data *obj;
    char type[100], num[100], buf[100];
    int number, r_num;

    extern int top_of_mobt;
    extern int top_of_objt;

    if (IS_NPC(ch) || GET_LEVEL(ch) > (IMO+3))
        return;

    argument_interpreter(argument, type, num);

    if (!*type || !*num || !ISDIGIT(*num)) {
        send_to_char("Syntax:\n\rload <'char' | 'obj'> <number>.\n\r", ch);
        return;
    }

    if ((number = atoi(num)) < 0) {
        send_to_char("A NEGATIVE number??\n\r", ch);
        return;
    }
    if (is_abbrev(type, "char")) {
        if ((r_num = real_mobile(number)) < 0) {
            send_to_char("There is no monster with that number.\n\r", ch);
            return;
        }
        mob = read_mobile(r_num, REAL);
        char_to_room(mob, ch->in_room);
        act("$n makes a quaint, magical gesture with one hand.", TRUE, ch,
            0, 0, TO_ROOM);
        act("$n has created $N!", FALSE, ch, 0, mob, TO_ROOM);
        send_to_char("Done.\n\r", ch);
        sprintf(buf,"%s loaded char %d",ch->player.name,number);
        log(buf);
    }
    else if (is_abbrev(type, "obj")) {
        if ((obj = read_object(number, VIRTUAL)) == 0) {
            send_to_char("There is no object with that number.\n\r", ch);
            return;
        }
        if(GET_LEVEL(ch) < (IMO+2))
            if(IS_OBJ_STAT(obj,ITEM_NOLOAD)){
                send_to_char("That item is not loadable.\n\r",ch);
                extract_obj(obj);
                sprintf(buf,"%s tried to load %d",ch->player.name,number);
                log(buf);
                return;
            }
        act("$n makes a strange magical gesture.", TRUE, ch, 0, 0, TO_ROOM);
        act("$n has created $p!", FALSE, ch, obj, 0, TO_ROOM);
        if(IS_SET(obj->obj_flags.wear_flags,ITEM_TAKE)){
            obj_to_char(obj,ch);
            act("$n snares $p!\n\r",FALSE,ch,obj,0,TO_ROOM);
        } else
            obj_to_room(obj, ch->in_room);
        send_to_char("Ok.\n\r", ch);
        sprintf(buf,"%s loaded object %d",ch->player.name,number);
        log(buf);
    } else
        send_to_char("That'll have to be either 'char' or 'obj'.\n\r", ch);
}

/* clean a room of all mobiles and objects */
void do_purge(struct char_data *ch, char *argument, int cmd)
{
    struct char_data *vict, *next_v;
    struct obj_data *obj, *next_o;
    int i;
    char name[100], buf[100];

    if (IS_NPC(ch) || GET_LEVEL(ch) > (IMO+3))
        return;
    one_argument(argument, name);
    if (*name) {
        if (vict = get_char_room_vis(ch, name)) {
            if(!IS_NPC(vict)){
                if(GET_LEVEL(ch) < GET_LEVEL(vict)){
                    sprintf(buf,"%s tried to purge you.\n\r",ch->player.name);
                    send_to_char(buf,vict);
                    return;
                }
                if ( stash_char(vict,0) < 0 ) {
                    send_to_char("saving victim failed.\n\r", ch) ;
                }
                move_stashfile(vict->player.name);
                for(i=0;i<MAX_WEAR;i++)
                    if(vict->equipment[i]){
                        extract_obj(unequip_char(vict,i));
                        vict->equipment[i]=0;
                    }
                wipe_obj(vict->carrying);
                vict->carrying=0;
                if(vict->desc)
                    close_socket(vict->desc);
                extract_char(vict);
                return;
            }
            act("$n disintegrates $N.", FALSE, ch, 0, vict, TO_NOTVICT);
            if (IS_NPC(vict)) {
                extract_char(vict);
            }
        } else if(obj=get_obj_in_list_vis(ch,name,world[ch->in_room].contents)) {
            act("$n destroys $p.", FALSE, ch, obj, 0, TO_ROOM);
            extract_obj(obj);
        } else {
            send_to_char("I don't know anyone or anything by that name.\n\r", ch);
            return;
        }
        send_to_char("Ok.\n\r", ch);
    }
    else { /* no argument. clean out the room */
        if (IS_NPC(ch)) {
            send_to_char("Don't... You would only kill yourself..\n\r", ch);
            return;
        }
        act("$n gestures... You are surrounded by scorching flames!", 
            FALSE, ch, 0, 0, TO_ROOM);
        send_to_room("The world seems a little cleaner.\n\r", ch->in_room);
        for (vict = world[ch->in_room].people; vict; vict = next_v) {
            next_v = vict->next_in_room;
            if (IS_NPC(vict))
                extract_char(vict);
        }
        for (obj = world[ch->in_room].contents; obj; obj = next_o) {
            next_o = obj->next_content;
            extract_obj(obj);
        }
    }
}

/* Give pointers to the five abilities */
void roll_abilities(struct char_data *ch)
{
    int i, j, k, temp, sum, count;
    ubyte table[5];
    ubyte rools[4];

#ifdef DEATHFIGHT
	table[0] = 18 ;
	table[1] = 16 ;
	table[2] = 15 ;
	table[3] = 14 ;
	table[4] = 13 ;
#else
    count = 50 ;
    while(--count > 0) {
        for(i=0; i<5; table[i++]=0)  ;

        sum = 0 ;
        for(i=0; i<5; i++) {

            for(j=0; j<4; j++)
                rools[j] = number(1,6);
            temp = MIN( 18, rools[0]+rools[1]+rools[2]+rools[3] );
            sum += temp ;
            for(k=0; k<5; k++)
                if (table[k] < temp)
                    SWITCH(temp, table[k]);
		}
        if ( sum >= 70 && sum <= 75 )			/*  It's average 14-15 */
            break ;
	}
#endif

    ch->abilities.str_add = 0;

    switch (GET_CLASS(ch)) {
    case CLASS_MAGIC_USER: {
        ch->abilities.intel = table[0];
        ch->abilities.wis = table[1];
        ch->abilities.dex = table[2];
        ch->abilities.str = table[3];
        ch->abilities.con = table[4];
    }  break;
    case CLASS_CLERIC: {
        ch->abilities.wis = table[0];
        ch->abilities.intel = table[1];
        ch->abilities.str = table[2];
        ch->abilities.dex = table[3];
        ch->abilities.con = table[4];
    } break;
    case CLASS_THIEF: {
        ch->abilities.dex = table[0];
        ch->abilities.str = table[1];
        ch->abilities.con = table[2];
        ch->abilities.intel = table[3];
        ch->abilities.wis = table[4];
    } break;
    case CLASS_WARRIOR: {
        ch->abilities.str = table[0];
        ch->abilities.dex = table[1];
        ch->abilities.con = table[2];
        ch->abilities.wis = table[3];
        ch->abilities.intel = table[4];
#ifdef DEATHFIGHT
        ch->abilities.str_add = 50;
#else
        if (ch->abilities.str == 18)
            ch->abilities.str_add = number(0,100);
#endif 
    } break;
    }
    ch->tmpabilities = ch->abilities;
}

void do_start(struct char_data *ch)
{
#ifdef DEATHFIGHT
	extern int deathfight ;
#endif 
    int i, j;
    byte table[5];
    byte rools[4];

    extern struct dex_skill_type dex_app_skill[];
    void advance_level(struct char_data *ch);

	send_to_char("========================\n\r", ch);
    send_to_char("환영합니다....\n\r", ch);
	send_to_char("========================\n\r", ch);

    GET_LEVEL(ch) = 1;
    GET_EXP(ch) = 1;

    set_title(ch);

    roll_abilities(ch);

    ch->points.max_hit  = 10;  /* These are BASE numbers   */
    ch->points.max_move  = 2;  /* These are BASE numbers   */
    ch->points.max_mana  = 0;  /* These are BASE numbers   */

    for ( i = 0 ; i < MAX_SKILLS ; i++ ) {
        ch->skills[i].learned = 0 ;
    }

	/* clear all quest .. */
	clear_all_quest(ch) ;

    switch (GET_CLASS(ch)) {

    case CLASS_MAGIC_USER : {
    } break;

    case CLASS_CLERIC : {
    } break;

    case CLASS_THIEF : {
        ch->skills[SKILL_SNEAK].learned = 10;
        ch->skills[SKILL_HIDE].learned =  5;
        ch->skills[SKILL_STEAL].learned = 15;
        ch->skills[SKILL_BACKSTAB].learned = 10;
        ch->skills[SKILL_PICK_LOCK].learned = 10;
    } break;

    case CLASS_WARRIOR: {
    } break;
    }
    advance_level(ch);
    GET_HIT(ch) = hit_limit(ch);
    GET_MANA(ch) = mana_limit(ch);
    GET_MOVE(ch) = move_limit(ch);
    GET_COND(ch,THIRST) = 24;
    GET_COND(ch,FULL) = 24;
    GET_COND(ch,DRUNK) = 0;
    ch->player.time.played = 0;
    ch->player.time.logon = time(0);

#ifdef DEATHFIGHT
	if ( deathfight == 0 ) {
		/* exp enought to advance to 20 level */
		GET_EXP(ch) = titles[GET_CLASS(ch)-1][10].exp ;
		GET_GOLD(ch) = 50000 ;
		ch->points.hit = ch->points.max_hit = 200 ;
		if ( GET_CLASS(ch) == CLASS_WARRIOR )
			ch->points.damroll= 12 ;
    }
#endif 
}

void do_advance(struct char_data *ch, char *argument, int cmd)
{
    struct char_data *victim;
    char name[100], level[100], buf[240], cyb[100];
    int i, adv, newlevel;

    void gain_exp(struct char_data *ch, int gain);
    if (IS_NPC(ch))
        return;
    sprintf(cyb, "%s advanced %s", GET_NAME(ch), argument);
    log(cyb);
    if(GET_LEVEL(ch) < (IMO+3)){
        send_to_char("You can only do that in a guild.\n\r",ch);
        return;
    }
    argument_interpreter(argument, name, level);
    if (*name) {
        if (!(victim = get_char_room_vis(ch, name))) {
            send_to_char("That player is not here.\n\r", ch);
            return;
        }
    } else {
        send_to_char("Advance who?\n\r", ch);
        return;
    }
    if (IS_NPC(victim)) {
        send_to_char("NO! Not on NPC's.\n\r", ch);
        return;
    }
    if (GET_LEVEL(victim) == 0)
        adv = 1;
    else if (!*level) {
        send_to_char("You must supply a level number.\n\r", ch);
        return;
    } else {
        if (!ISDIGIT(*level)) {
            send_to_char("Second argument must be a positive integer.\n\r",ch);
            return;
        }
        if((newlevel=atoi(level)) < GET_LEVEL(victim)) {
            if(GET_LEVEL(ch) <= GET_LEVEL(victim)){
                send_to_char("Very amusing.\n\r",ch);
                return;
            }
            victim->player.level=newlevel;
            if(newlevel < IMO){
                for(i=0;i<3;++i)
                    victim->specials.conditions[i]=0;
            }
            victim->tmpabilities = victim->abilities;
            send_to_char("The poor soul...\n\r", ch);
            send_to_char("You have been punished.\n\r",victim);
            return;
        }
    }
    adv = newlevel - GET_LEVEL(victim);
    if (( newlevel  >= 1) && (GET_LEVEL(ch) < (IMO+3))
        && (GET_LEVEL(ch) > (IMO+3))) {
        send_to_char("Thou art not godly enough.\n\r", ch);
        return;
    }
    if (newlevel  > (IMO+2)) {
        send_to_char("Too high a level.\n\r",ch);
        return;
    }

    send_to_char("You feel generous.\n\r", ch);
/* cyb
   act("$n makes some strange gestures.\n\rA strange feeling comes uppon you,"
   "\n\rLike a giant hand, light comes down from\n\rabove, grabbing your "
   "body, that begins\n\rto pulse with coloured lights from inside.\n\rYo"
   "ur head seems to be filled with deamons\n\rfrom another plane as your"
   " body dissolves\n\rto the elements of time and space itself.\n\rSudde"
   "nly a silent explosion of light snaps\n\ryou back to reality. You fee"
   "l slightly\n\rdifferent.",FALSE,ch,0,victim,TO_VICT);
*/
    act("$n makes some strange gestures.\n\rA strange feeling comes uppon you,\
\n\rLike a giant hand, light comes down from\n\rabove, grabbing your \
body, that begins\n\rto pulse with coloured lights from inside.\n\rYo\
ur head seems to be filled with deamons\n\rfrom another plane as your\
 body dissolves\n\rto the elements of time and space itself.\n\rSudde\
nly a silent explosion of light snaps\n\ryou back to reality. You fee\
l slightly\n\rdifferent.",FALSE,ch,0,victim,TO_VICT);
    sprintf(cyb, "%s advances %s to %d", GET_NAME(ch),GET_NAME(victim),newlevel);
    log(cyb);
    if (GET_LEVEL(victim) == 0) {
        do_start(victim);
    } else {
        if (GET_LEVEL(ch) < (IMO+3) || GET_LEVEL(victim) < (IMO+3)) {
            victim->points.exp=1;
            gain_exp_regardless(victim, (titles[GET_CLASS(victim)-1][
                GET_LEVEL(victim)+adv].exp)-GET_EXP(victim));
        } else {
            send_to_char("Some idiot just tried to advance your level.\n\r",
                         victim);
            send_to_char("IMPOSSIBLE! IDIOTIC!\n\r", ch);
        }
    }
}

void do_reroll(struct char_data *ch, char *argument, int cmd)
{
    struct char_data *victim;
    char buf[100];
    sh_int target;

    if (IS_NPC(ch)) return;
	if (GET_LEVEL(ch) != IMO+3 && GET_LEVEL(ch) != 1) {
		if (GET_LEVEL(ch) != 1) {
			send_to_char("You can reroll your stats during level 1\n\r",ch);
			return;
		}
	}

	if (GET_LEVEL(ch) == IMO +3) {
		one_argument(argument,buf);
		if (!*buf) {
			send_to_char("Who do you wish to reroll?\n\r",ch);
			return;
		}
		else {
			if(!(victim = get_char(buf))) {
				send_to_char("No-one by that name in the world.\n\r",ch);
				return;
			}
		}
	}
	else if (GET_LEVEL(ch) == 1) {
		victim = ch;
	}

	roll_abilities(victim);
	sprintf(buf,"Rerolled: %d/%d %d %d %d %d\n",victim->abilities.str,
			victim->abilities.str_add,
			victim->abilities.intel,
			victim->abilities.wis,
			victim->abilities.dex,
			victim->abilities.con);
	send_to_char(buf,ch);
}

void do_restore(struct char_data *ch, char *argument, int cmd)
{
    struct char_data *victim;
    char buf[100];
    int i;

    void update_pos( struct char_data *victim );

    if (IS_NPC(ch) || GET_LEVEL(ch) > (IMO+3))
        return;
    one_argument(argument,buf);
    if (!*buf)
        send_to_char("Who do you wish to restore?\n\r",ch);
    else
        if(!(victim = get_char_vis(ch,buf)))
            send_to_char("No-one by that name in the world.\n\r",ch);
        else {
            GET_MANA(victim) = GET_MAX_MANA(victim);
            GET_HIT(victim) = GET_MAX_HIT(victim);
            GET_MOVE(victim) = GET_MAX_MOVE(victim);

            if (GET_LEVEL(victim) >= IMO) {
                for (i = 0; i < MAX_SKILLS; i++) {
                    victim->skills[i].learned = 100;
                    victim->skills[i].recognise = TRUE;
                }

                if (GET_LEVEL(victim) >= (IMO+3)) {
                    victim->abilities.str_add = 100;
                    victim->abilities.intel = 18;
                    victim->abilities.wis = 18;
                    victim->abilities.dex = 18;
                    victim->abilities.str = 18;
                    victim->abilities.con = 18;
                }
                victim->tmpabilities = victim->abilities;

            }
            update_pos( victim );
            send_to_char("Done.\n\r", ch);
            act("You have been fully healed by $N!", FALSE, victim, 0, ch, TO_CHAR);
        }
}

do_noshout(struct char_data *ch, char *argument, int cmd)
{
    struct char_data *vict;
    struct obj_data *dummy;
    char buf[MAX_INPUT_LENGTH];

    if (IS_NPC(ch))
        return;
    one_argument(argument, buf);
    if (!*buf) {
        if (IS_SET(ch->specials.act, PLR_EARMUFFS)) {
            send_to_char("You can now hear shouts again.\n\r", ch);
            REMOVE_BIT(ch->specials.act, PLR_EARMUFFS);
        } else {
            send_to_char("From now on, you won't hear shouts.\n\r", ch);
            SET_BIT(ch->specials.act, PLR_EARMUFFS);
        }
        return;
    }
    if(GET_LEVEL(ch) < IMO)
        return;
    if (!generic_find(argument, FIND_CHAR_WORLD, ch, &vict, &dummy))
        send_to_char("Couldn't find any such creature.\n\r", ch);
    else if (IS_NPC(vict))
        send_to_char("Can't do that to a beast.\n\r", ch);
    else if (GET_LEVEL(vict) > GET_LEVEL(ch))
        act("$E might object to that.. better not.", 0, ch, 0, vict, TO_CHAR);
    else if (IS_SET(vict->specials.act, PLR_NOSHOUT)) {
        send_to_char("You can shout again.\n\r", vict);
        send_to_char("NOSHOUT removed.\n\r", ch);
        REMOVE_BIT(vict->specials.act, PLR_NOSHOUT);
    } else {
        send_to_char("The gods take away your ability to shout!\n\r", vict);
        send_to_char("NOSHOUT set.\n\r", ch);
        SET_BIT(vict->specials.act, PLR_NOSHOUT);
    }
}
void do_wiznet(struct char_data *ch, char *argument, int cmd)
{
    struct descriptor_data *i;
    struct char_data *victim;
    char buf[MAX_STRING_LENGTH];

    if (IS_NPC(ch))
        return;
    sprintf(buf,"%s: %s\n\r",ch->player.name,argument);
    for (i = descriptor_list; i; i = i->next)
        if (!i->connected) {
            if(i->original) continue;
            victim = i->character;
            if((GET_LEVEL(victim) >= IMO) && (GET_LEVEL(victim) <= (IMO+4)))
                send_to_char(buf,victim);
        }
    send_to_char("Ok.\n\r",ch);
}
void do_chat(struct char_data *ch, char *argument, int cmd)
{
    void lastchat_add(char *str);
    struct descriptor_data *i;
    struct char_data *victim;
    char buf[MAX_STRING_LENGTH];
    extern int nochatflag;

    if(nochatflag){
        send_to_char("chat is forbidened now.\n\r",ch);
        return;
    }
    if (IS_NPC(ch))
        return;
    sprintf(buf,"%s> %s",ch->player.name,argument);
    lastchat_add(buf) ;  /* cyb */

	strcat(buf, "\n\r");
    for (i = descriptor_list; i; i = i->next)
        if (!i->connected) {
            if(i->original) continue;
            victim = i->character;
            if(!IS_SET(victim->specials.act,PLR_NOCHAT))
                send_to_char(buf,victim);
        }
    send_to_char("Ok.\n\r",ch);
}
void do_noaffect(struct char_data *ch, char *argument, int cmd)
{
    struct char_data *vict;
    struct obj_data *dummy;
    struct affected_type *hjp;
    char buf[MAX_INPUT_LENGTH];

    if (IS_NPC(ch))
        return;
    one_argument(argument, buf);
    if (!*buf){
        send_to_char("Remove affects from whom?\n\r", ch);
        return;
    } else {
        if (!generic_find(argument, FIND_CHAR_WORLD, ch, &vict, &dummy))
            send_to_char("Couldn't find any such creature.\n\r", ch);
        else if (IS_NPC(vict))
            send_to_char("Can't do that to a beast.\n\r", ch);
        else if (GET_LEVEL(vict) > GET_LEVEL(ch))
            act("$E might object to that.. better not.",0,ch,0,vict,TO_CHAR);
        else{
            send_to_char("You are normal again.\n\r", vict);
            for(hjp = vict->affected; hjp; hjp = hjp->next)
                affect_remove( vict, hjp );
        }
    }
    send_to_char("Ok.\n\r",ch);
}
void do_wall(struct char_data *ch, char *argument, int cmd)
{
    struct descriptor_data *i;
    struct char_data *victim;
    char buf[MAX_STRING_LENGTH];

    if(IS_NPC(ch)||(! *argument) || GET_LEVEL(ch) > (IMO+3))
        return;
    sprintf(buf,"%s\n\r",argument+1);
    send_to_all(buf);
    send_to_char("Ok.\n\r",ch);
}

void do_set(struct char_data *ch, char *argument, int cmd)
{
	struct char_data	*victim ;
	char	mess[BUFSIZ], buf[128], buf2[32], buf3[32], buf4[32] ;
	char	mbuf[BUFSIZ] ;
	int	k, i ;
	extern int	nokillflag,nostealflag,nonewplayers,noshoutflag,nochatflag;
	extern int	nologin ;
#ifdef DEATHFIGHT
	extern int	deathfight ;
#endif 

	if (IS_NPC(ch))
		return;

	sprintf(buf, "%s set %s", GET_NAME(ch), argument);
	log(buf) ;

	half_chop(argument,buf,buf2);
	if(!*buf){	/* no argument */
		sprintf(mbuf, "%10s = %2d    %10s = %2d    %10s = %2d\n\r",
                "nokill", nokillflag, "nosteal", nostealflag, "freeze", nonewplayers);
		strcpy(mess, mbuf) ;
		sprintf(mbuf, "%10s = %2d    %10s = %2d    %10s = %2d\n\r",
                "noshout", noshoutflag, "nochat", nochatflag, "nologin", nologin) ;
		strcat(mess, mbuf) ;
#ifdef DEATHFIGHT
		sprintf(mbuf, "%10s = %2d    ", "deathfight", deathfight) ;
		strcat(mess, mbuf) ;
#endif 
		sprintf(mbuf, "%10s = %s\n\r", "baddomain", baddomain[0]) ;
		strcat(mess, mbuf) ;

        /*
          sprintf(mess,
          " nokill=%d\n\rnosteal=%d\n\rfreeze=%d\n\rnoshout=%d\n\rnochat=%d\n\rbaddomain=%s\n\rnologin=%d\n\r",
          nokillflag,nostealflag,nonewplayers,noshoutflag,
          nochatflag,baddomain[0],nologin);
        */
		send_to_char(mess,ch);
		return;
    }	/* end .. no argument */
	else if (( victim = get_char_room_vis(ch, buf)) == NULL) {
		/* set system flags */
		if(strcmp(buf,"nokill")==0){
			one_argument(buf2,buf3);
			if(*buf3)
				nokillflag=atoi(buf3);
			sprintf(mess,"No-kill flag is %d.\n\r",nokillflag);
			send_to_char(mess,ch);
			return ;
        }
		if(strcmp(buf,"baddomain")==0){
			one_argument(buf2,buf3);
			if(*buf3)
				strcpy(baddomain[0],buf3);
			sprintf(mess,"Bad Domain is %s.\n\n",baddomain[0]);
			send_to_char(mess,ch);
			return ;
        }
		if(strcmp(buf,"nosteal")==0){
			one_argument(buf2,buf3);
			if(*buf3)
				nostealflag=atoi(buf3);
			sprintf(mess,"No-steal flag is %d.\n\r",nostealflag);
			send_to_char(mess,ch);
			return ;
        }
		if(strcmp(buf,"freeze")==0){
			one_argument(buf2,buf3);
			if(*buf3)
				nonewplayers=atoi(buf3);
			sprintf(mess,"Freeze flag is %d.\n\r",nonewplayers);
			send_to_char(mess,ch);
			return ;
        }
		if(strcmp(buf,"noshout")==0){
			one_argument(buf2,buf3);
			if(*buf3)
				noshoutflag=atoi(buf3);
			sprintf(mess,"NoShout flag is %d.\n\r",noshoutflag);
			send_to_char(mess,ch);
			return ;
        }
		if(strcmp(buf,"nologin")==0){
			one_argument(buf2,buf3);
			if(*buf3)
				nologin=atoi(buf3);
			sprintf(mess,"No Login flag is %d.\n\r",nologin);
			send_to_char(mess,ch);
			return ;
        }
		if(strcmp(buf,"nochat")==0){
			one_argument(buf2,buf3);
			if(*buf3)
				nochatflag=atoi(buf3);
			sprintf(mess,"Nochat flag is %d.\n\r",nochatflag);
			send_to_char(mess,ch);
			return;
        }
#ifdef DEATHFIGHT
		if(strcmp(buf,"deathfight")==0){
			one_argument(buf2,buf3);
			if(*buf3)
				deathfight=atoi(buf3);
			sprintf(mess,"Death Fight flag is %d.\n\r", deathfight);
			send_to_char(mess,ch);
			return;
        }
#endif 
		/* not found ! (system flags) */
    }	/* end.. system flags */

	/* not yet returned ? then check character.. */
	if ( victim == NULL ) {	/* none here */
		if(!(victim = get_char(buf))) {
			send_to_char("No-one by that name in the world.\n\r",ch);
			return ;
        }
    }
	half_chop(buf2,buf3,buf4);
	k=atoi(buf4);
	if ((GET_LEVEL(ch) < (IMO+2))&&(strcmp(buf3,"gold"))) {
		send_to_char("You can't touch gold.", ch) ;
		return;
    }

	/* checking fields */
	if(strcmp(buf3,"exp")==0)
		victim->points.exp=k;
	else if (strcmp(buf3,"skill")==0) {
		for (i=0;i<MAX_SKILLS;i++)
			victim->skills[i].learned=0;		/* clear skills */
    }
	else if (strcmp(buf3,"quest")==0){
		clear_all_quest(victim) ;
    }
	else if(strcmp(buf3,"lev")==0 ) {
		if (implementor(GET_NAME(ch)))
			GET_LEVEL(victim)=k;
    }
	else if(strcmp("hit",buf3)==0)
		victim->points.hit=victim->points.max_hit=k;
	else if(strcmp("age",buf3)==0){
		victim->player.time.birth = time(0);
		victim->player.time.played = 0;
		victim->player.time.logon = time(0);
    }
	else if(strcmp("mana",buf3)==0)
		victim->points.mana=victim->points.max_mana=k;
	else if(strcmp("move",buf3)==0)
		victim->points.move=victim->points.max_move=k;
	else if(strcmp("gold",buf3)==0)
		victim->points.gold=k;
	else if(strcmp("bank",buf3)==0)
		victim->bank=k;
	else if(strcmp("align",buf3)==0)
		victim->specials.alignment=k;
	else if(strcmp("str",buf3)==0)
		victim->abilities.str=k;
	else if(strcmp("str_add",buf3)==0)
		victim->abilities.str_add=k;
	else if(strcmp("dex",buf3)==0)
		victim->abilities.dex=k;
	else if(strcmp("wis",buf3)==0)
		victim->abilities.wis=k;
	else if(strcmp("con",buf3)==0)
		victim->abilities.con=k;
	else if(strcmp("int",buf3)==0)
		victim->abilities.intel=k;
	else if(strcmp("pra",buf3)==0)
		victim->specials.spells_to_learn=k;
	else if(strcmp("hunger",buf3)==0)
		victim->specials.conditions[FULL]=k;
	else if(strcmp("thirsty",buf3)==0)
		victim->specials.conditions[THIRST]=k;
	else if(strcmp("drunk",buf3)==0)
		victim->specials.conditions[0]=k;
	else if(strcmp("dr",buf3)==0)
		victim->points.damroll=k;
	else if(strcmp("hr",buf3)==0)
		victim->points.hitroll=k;
	else if(strcmp("ac",buf3)==0)
		victim->points.armor=k;
#ifdef DEATHFIGHT
	else if(strcmp("super",buf3)==0) {
		GET_LEVEL(victim)= 40 ;
		GET_REGENERATION(victim) = 10000 ;
		victim->points.hit=victim->points.max_hit= 50000 ;
		victim->points.mana=victim->points.max_mana= 32000 ;
		victim->points.move=victim->points.max_move= 32000 ;
		for (i = 0; i < MAX_SKILLS; i++) {
			victim->skills[i].learned = 100;
			victim->skills[i].recognise = TRUE;
        }
		victim->abilities.str_add = 100;
		victim->abilities.intel = 18;
		victim->abilities.wis = 18;
		victim->abilities.dex = 18;
		victim->abilities.str = 18;
		victim->abilities.con = 18;
		victim->points.damroll= 200 ;
		victim->points.hitroll= 40 ;
		victim->points.armor= -200 ;
    }
#endif 
	else if(strcmp("sex",buf3)==0) {
		k = k % 3 ;	/* neutral, male, female */
		GET_SEX(victim)=k;
    }
	else {
		send_to_char("Huh?\n\r",ch);
		return ;
    }
	victim->tmpabilities = victim->abilities;
}

void do_invis(struct char_data *ch, char *argument, int cmd)
{
    if (IS_NPC(ch) || GET_LEVEL(ch) > (IMO+3))
        return;
    if(IS_SET(ch->specials.act,PLR_WIZINVIS)){
        REMOVE_BIT(ch->specials.act,PLR_WIZINVIS);
        send_to_char("You are visible again.\n\r",ch);
    } else {
        SET_BIT(ch->specials.act,PLR_WIZINVIS);
        send_to_char("You vanish.\n\r",ch);
    }
    send_to_char("Ok.\n\r",ch);
}
do_banish(struct char_data *ch, char *argument, int cmd)
{
    struct char_data *vict;
    struct obj_data *dummy;
    char buf[MAX_INPUT_LENGTH];
    int location;
    extern int top_of_world;

    if (IS_NPC(ch))
        return;
    one_argument(argument, buf);
    if (!*buf)
        send_to_char("Banish whom?\n\r", ch);
    else if (!generic_find(argument, FIND_CHAR_WORLD, ch, &vict, &dummy))
        send_to_char("Couldn't find any such creature.\n\r", ch);
    else if (IS_NPC(vict))
        send_to_char("Can't do that to a beast.\n\r", ch);
    else if (GET_LEVEL(vict) >= IMO)
        send_to_char("It's pointless to banish an immortal.\n\r",ch);
    else if (IS_SET(vict->specials.act, PLR_BANISHED)) {
        REMOVE_BIT(vict->specials.act, PLR_BANISHED);
        send_to_char("You feel forgiven?\n\r", vict);
        act("$N is forgiven.",FALSE,ch,0,vict,TO_CHAR);
    } else {
        SET_BIT(vict->specials.act, PLR_BANISHED);
        for (location = 0; location <= top_of_world; location++)
            if (world[location].number == 6999)
                break;
        if(location == top_of_world){
            send_to_char("Death Room is gone?\n\r",ch);
        } else {
            act("$n disappears in a puff of smoke.",FALSE,vict,0,0,TO_ROOM);
            char_from_room(vict);
            char_to_room(vict,location);
            act("$n appears with an ear-splitting bang.",FALSE,vict,0,0,TO_ROOM);
        }
        send_to_char("You smell fire and brimstone?\n\r", vict);
        act("$N is banished.",FALSE,ch,0,vict,TO_CHAR);
    }
    send_to_char("OK.\n\r",ch);
}

do_flag(struct char_data *ch, char *argument, int cmd)
{
    struct char_data *vict;
    struct obj_data *dummy;
    char buf[MAX_INPUT_LENGTH];
    int f;

    if (IS_NPC(ch))
        return;
    one_argument(argument, buf);
    if (!*buf)
        send_to_char("Flag whom?\n\r", ch);
    else if (!generic_find(argument, FIND_CHAR_WORLD, ch, &vict, &dummy))
        send_to_char("Couldn't find any such creature.\n\r", ch);
    else if (IS_NPC(vict))
        send_to_char("Can't do that to a beast.\n\r", ch);
    else if ((GET_LEVEL(vict) >= IMO)&&(cmd != 231))
        send_to_char("It's pointless to flag an immortal.\n\r",ch);
    else {
        f=(cmd == 231) ? IS_SET(vict->specials.act, PLR_XYZZY) :
            IS_SET(vict->specials.act, PLR_CRIMINAL) ;
        if (f) {
            if(cmd == 231){
                REMOVE_BIT(vict->specials.act, PLR_XYZZY);
                send_to_char("Log removed.\n\r",ch);
            } else {
                REMOVE_BIT(vict->specials.act, PLR_CRIMINAL);
                send_to_char("Flag removed.\n\r",ch);
            }
        } else {
            if(cmd == 231){
                SET_BIT(vict->specials.act, PLR_XYZZY);
                send_to_char("Log set.\n\r",ch);
            } else {
                SET_BIT(vict->specials.act, PLR_CRIMINAL);
                send_to_char("Flag set.\n\r",ch);
            }
        }
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
    } else if (victim == ch) {
        send_to_char("Odd?\n\r", ch);
        return;
    } else if(GET_LEVEL(ch) <= GET_LEVEL(victim)){
        send_to_char("Bad idea.\n\r",ch);
        return;
    }
    if (!(obj = get_obj_in_list_vis(victim, obj_name, victim->carrying))) {
        for (eq_pos = 0; (eq_pos < MAX_WEAR); eq_pos++)
            if(victim->equipment[eq_pos] &&
               (isname(obj_name, victim->equipment[eq_pos]->name))){
                obj = victim->equipment[eq_pos];
                break;
            }
        if (!obj) {
            send_to_char("Can't find that item.\n\r",ch);
            return;
        } else { /* It is equipment */
            obj_to_char(unequip_char(victim, eq_pos), ch);
            send_to_char("Done.\n\r", ch);
        }
    } else {  /* obj found in inventory */
        obj_from_char(obj);
        obj_to_char(obj, ch);
        send_to_char("Done.\n\r", ch);
    }
}
void do_sys(struct char_data *ch, char *argument, int cmd)
{
    struct rusage xru;
    char buffer[BUFSIZ], buf[MAX_STRING_LENGTH] ;
    extern int boottime;
    extern struct char_data *character_list;
    extern struct obj_data *object_list;
    extern struct descriptor_data *descriptor_list;
    struct char_data *i;
    struct obj_data *k;
    struct descriptor_data *d;
    static int nits,nics,nids;
	int	stack_size, dummy, used, struct_size ;
	int	j ;

	getrusage(0,&xru);
	sprintf(buffer,
            "sys time: %d secs     usr time: %d secs     run time: %d secs\n\r",
            xru.ru_stime.tv_sec, xru.ru_utime.tv_sec, time(0)-boottime);
	strcpy(buf, buffer) ;

	if (GET_LEVEL(ch) <= (IMO+1)) {
		send_to_char(buf, ch) ;
		return ;
    }

	nits = nics = nids = 0;
	for (k=object_list;k;k=k->next) ++nits;
	for (i=character_list;i;i=i->next) ++nics;
	for (d=descriptor_list;d;d=d->next) ++nids;

	sprintf(buffer," objects: %d     chars: %d     players: %d\n\r",
            nits, nics, nids);
	strcat(buf, buffer) ;
	send_to_char(buf, ch) ;

	strcpy(buf, "\n\r < Memory Management >\n\r");
	strcat(buf,"stack  remains  ratio    byte      stack  remains  ratio     byte\n\r") ;
	for ( j = 0 ; j < MAX_MEMORY_MANAGE_SIZE ; j ++ ) {
		stack_size = used = struct_size = 1 ;
		if ( malloc_memory_info(j, &stack_size, &dummy, &used, &struct_size) <0)
			continue ;
		sprintf(buffer, "%3d %8d %6d%% %9d%s", j,
                used, 100*used/stack_size, used*struct_size,
                ((j % 2 == 0) ? "  |  " : "\n\r")) ;
		strcat(buf, buffer) ;
    }
	strcat(buf, "\r\n") ;
    send_to_char(buf, ch);
}

void do_which(struct char_data *ch, char *argument, int cmd)
{
	char	name[BUFSIZ], second_name[BUFSIZ] ;
	char	buf[BUFSIZ], buf2[BUFSIZ];
	struct char_data	*i;
	struct obj_data	*k;
	int	j, n, virtual;

	if (GET_LEVEL(ch) < IMO) {
		send_to_char("You can't do that yet.\n\r", ch) ;
		return ;
    }
	argument = one_argument(argument, name);
	if ( *name == 0 ) {
		send_to_char("usage: which <item> { < item >}\n\r", ch) ;
		return ;
    }
	one_argument(argument, second_name) ;
	n=0;
	*buf = 0 ;

	/* carring */
	for (k = object_list ; k ; k = k->next) {
		if ( *second_name ) {
			if ( !isname(second_name, k->name) )
				continue ;
        }
		if(isname(name, k->name)){
			if ( k->item_number >= 0 )
				virtual = obj_index[k->item_number].virtual ;
			else
				virtual = 0 ;
			if (k->rented_by) {
				sprintf(buf,"%2d: %s (%d) rented by %s.\n\r", ++n,
                        k->short_description, virtual, GET_NAME(k->rented_by));
				send_to_char(buf,ch);
            }
			else if(k->carried_by) {
				sprintf(buf,"%2d: %s (%d) carried by %s.\n\r", ++n,
                        k->short_description, virtual, GET_NAME(k->carried_by));
				send_to_char(buf,ch);
            }
			else if(k->in_obj) {
				sprintf(buf,"%2d: %s (%d) in %s", ++n, k->short_description,
                        virtual, k->in_obj->short_description);
				if( k->in_obj->rented_by ){
					sprintf(buf2," rented by %s\n\r", GET_NAME(k->in_obj->rented_by));
					strcat(buf,buf2);
                }
				else if( k->in_obj->carried_by ){
					sprintf(buf2," carried by %s\n\r", GET_NAME(k->in_obj->carried_by));
					strcat(buf,buf2);
                }
				else
					strcat(buf,"\n\r");
				send_to_char(buf,ch);
            }
			else if(k->in_room != NOWHERE) {
				sprintf(buf, "%2d: %s (%d) in %s [%d]\n\r",++n, k->short_description,
                        virtual, world[k->in_room].name, world[k->in_room].number);
				send_to_char(buf, ch);
            }
			else {	/* uncertain place - worn or really uncertain */
				sprintf(buf, "%2d: %s (%d) in uncertain.\n\r",++n,
                        k->short_description, virtual);
				send_to_char(buf, ch);
            }
        }
    }

	/* wearing */
	for (i=character_list;i;i=i->next) {
		for(j=0;j<MAX_WEAR;++j) {
			if(i->equipment[j]) {
				if ( *second_name ) {
					if ( !isname(second_name, i->equipment[j]->name))
						continue ;
                }
				if(isname(name, i->equipment[j]->name)){
					if ( i->equipment[j]->item_number >= 0 )
						virtual = obj_index[i->equipment[j]->item_number].virtual ;
					else
						virtual = 0 ;
					sprintf(buf, "%2d: %s (%d) worn by %s\n\r", ++n,
                            i->equipment[j]->short_description,
                            virtual, GET_NAME(i)) ;
					send_to_char(buf, ch);
                }
            }
        }
    }

    if (!*buf)
        send_to_char("Couldn't find any such thing.\n\r", ch);
}


#define	IMMO_NUMBER	1 
char *immo_members[] = {
	"Timber", ""
} ;

int implementor(char *name)
{
	int	i ;
	char	*p ;

	if ( *name == 0 )
		return 0 ;

	for ( i = 0 ; i < IMMO_NUMBER && immo_members[i] != NULL ; i ++ ) {
		p = immo_members[i] ;
		if ( strcmp(p, name) == 0 )
			return 1 ;
    }

	return 0 ;
}

/* command by cold */
void do_reconfig(struct char_data *ch, char *argument, int cmd)
{
	void	reconfig_news() ;
	char	buf[BUFSIZ] ;

	if ( IS_NPC(ch) )
		return ;

	send_to_char("Reconficuration news and motd file.\n\r", ch) ;
	sprintf(buf, "reconfig by %s", GET_NAME(ch)) ;
	log(buf) ;

	reconfig_news() ;	/* in db.c */
}

/* command by cold - for trasure-finding */
void do_scatter(struct char_data *ch, char *argument, int cmd)
{
	extern int	top_of_world ;
	struct obj_data	*obj;
	char	type[BUFSIZ], num[BUFSIZ] ;
	int	item_number, destiny, count ;

	if (IS_NPC(ch) || GET_LEVEL(ch) < (IMO+2)) {
		send_to_char("You're so powerless.\n\r", ch) ;
		return;
    }

	argument_interpreter(argument, type, num);

	if (!*type || !*num || !ISDIGIT(*num)) {
		send_to_char("Syntax: scatter <'local' | 'global'> <item V-number>.\n\r", ch);
		return;
    }

	if ((item_number = atoi(num)) < 0) {
		send_to_char("A NEGATIVE number??\n\r", ch);
		return;
    }

	if ((obj = read_object(item_number, VIRTUAL)) == 0) {
		send_to_char("There is no object with that number.\n\r", ch);
		return;
    }

	count = 0 ;
	if (is_abbrev(type, "local")) {
		do {
			if ( ++ count > 30 ) {
				send_to_char("Failed.\n\r", ch) ;
				return ;
            }
			destiny = ch->in_room + number(0, 100) - 50 ;
        } while ( IS_SET(world[destiny].room_flags, PRIVATE)
                  || destiny < 0 || destiny > top_of_world );
    }
	else if ( is_abbrev(type, "global")) {
		do {
			if ( ++ count > 30 ) {
				send_to_char("Failed.\n\r", ch) ;
				return ;
            }
			destiny = number(0, top_of_world) ;
        } while ( IS_SET(world[destiny].room_flags, PRIVATE));
    }
	else {
		send_to_char("That'll have to be either 'local' or 'global'.\n\r", ch);
		return ;
    }

	act("$n makes a strange magical gesture.", TRUE, ch, 0, 0, TO_ROOM) ;
	act("$n has created $p and throws it away !", FALSE, ch, obj, 0, TO_ROOM) ;
	obj_to_room(obj, destiny) ;
	act("You have created $p and throw it away !", FALSE, ch, obj, 0, TO_CHAR) ;
	send_to_char("Done.\n\r", ch);

}
