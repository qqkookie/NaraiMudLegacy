/* ************************************************************************
*  file: actwiz.c , Implementation of commands.           Part of DIKUMUD *
*  Usage : Wizard Commands.                                               *
*  Copyright (C) 1990, 1991 - see 'license.doc' for complete information. *
************************************************************************* */

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <assert.h>

#include <time.h>
#include <sys/time.h>
#include <sys/resource.h>

#include "structs.h"
#include "utils.h"
#include "comm.h"
#include "interpreter.h"
#include "handler.h"
#include "db.h"
#include "spells.h"
#include "limit.h"

/*   external vars  */

extern struct room_data *world;
extern struct char_data *character_list;
extern struct descriptor_data *descriptor_list;
extern struct title_type titles[4][IMO+4];
extern struct index_data *mob_index;
extern struct index_data *obj_index;
extern struct int_app_type int_app[26];
extern struct player_index_element *player_table;
extern int regen_time_percent;
extern int regen_time;
extern int regen_percent;
extern u_long reboot_time;

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
int load_char(char *name, struct char_file_u *char_element);
void store_to_char_for_transform(struct char_file_u *st, struct char_data *ch);
void stash_char(struct char_data *ch);
void unstash_char(struct char_data *ch, char *filename);
void page_string(struct descriptor_data *d, char *str, int keep);
void move_stashfile(char *name);
void wipe_obj(struct obj_data *o);
void close_socket(struct descriptor_data *d);
int number(int from, int to);
int MIN(int a, int b);
void gain_exp_regardless(struct char_data *ch, int gain);
void log(char *str);

char history[20][MAX_STRING_LENGTH];
int his_start=0,his_end=0; 

void do_emote(struct char_data *ch, char *argument, int cmd)
{
  int i;
  static char buf[MAX_STRING_LENGTH];

  if(IS_SET(ch->specials.act,PLR_DUMB_BY_WIZ)&&GET_LEVEL(ch)<IMO+3)
  {
	return;
  }
  if (IS_NPC(ch))
    return;
  for (i = 0; *(argument + i) == ' '; i++);
  if (!*(argument + i))
    send_to_char("Yes.. But what?\n\r", ch);
  else {
    sprintf(buf,"$n %s", argument + i);
    act(buf,FALSE,ch,0,0,TO_ROOM);

    sprintf(buf, "You %s.\n\r", argument + i);
    send_to_char(buf, ch);
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
  stash_char(ch);
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
  int target;

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
  else if ((target_mob = get_char_vis(ch, loc_str)))
    location = target_mob->in_room;
  else if ((target_obj = get_obj_vis(ch, loc_str)))
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

  if (IS_NPC(ch) || GET_LEVEL(ch) > (IMO+4))
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
  else if ((target_mob = get_char_vis(ch, buf)))
    location = target_mob->in_room;
  else if ((target_obj = get_obj_vis(ch, buf)))
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
	void init_char(struct char_data *ch);
	void do_start(struct char_data *ch);

	sprintf(buf, "%s demoted %s", GET_NAME(ch), argument);
	log(buf);

	if (IS_NPC(ch))
		return;

	one_argument(argument, name);
	if (*name) {
		if (!(victim = get_char_room_vis(ch, name))) {
			send_to_char("That player is not here.\n\r", ch);
			return;
		}
	}
	else {
		send_to_char("Demote who?\n\r", ch);
		return;
	}
	if (IS_NPC(victim)) {
		send_to_char("NO! Not on NPC's.\n\r", ch);
		return;
	}
	if (GET_LEVEL(victim) > GET_LEVEL(ch)) {
		send_to_char("Impossible!\n\r", ch);
		return;
	}

	GET_AC(ch) += GET_LEVEL(ch);
	GET_HITROLL(ch) -= GET_LEVEL(ch);
	GET_DAMROLL(ch) -= GET_LEVEL(ch);
	
	init_char(victim);
	do_start(victim);
}

void do_stat(struct char_data *ch, char *argument, int cmd)
{
  extern char *spells[];
  struct affected_type *aff;
  char arg1[100];
  char buf[512];
  char buf2[256];
  struct room_data *rm=0;
  struct char_data *k=0;
  struct obj_data  *j=0;
  struct extra_descr_data *desc;
  struct follow_type *fol;
  int i,i2,t,virtual;
  char page_buffer[10000];
  
  /* for objects */
  extern char *item_types[];
  extern char *wear_bits[];
  extern char *extra_bits[];
  
  /* for rooms */
  extern char *dirs[];
  extern char *room_bits[];
  extern char *exit_bits[];
  extern char *sector_types[];
  
  /* for chars */
  extern char *affected_bits[];
  extern char *apply_types[];
  extern char *pc_class_types[];
  extern char *action_bits[];
  extern char *player_bits[];
  extern char *connected_types[];
  struct char_data *get_specific_vis(struct char_data *ch,
				     char *name,int type);
  
  if (IS_NPC(ch))
    return;
  argument = one_argument(argument, arg1);
  
  if (!*arg1) {
    send_to_char("Stats on who or what?\n\r", ch);
    return;
  }
  else {
    /* stats on room */
    if ((cmd != 232) && (cmd != 233) && (!str_cmp("room", arg1))) {
      rm = &world[ch->in_room];
      sprintf(page_buffer, "Room: %s, Zone: %d. V-Num: %d, ",
	      rm->name, rm->zone, rm->number);
      sprintf(buf, "R-num: %d, Light: %d.\n\r",
	      ch->in_room, rm->light);
      strcat(page_buffer, buf);
      sprinttype(rm->sector_type, sector_types, buf2);
      sprintf(buf, "Sector type : %s\n\r", buf2);
      strcat(page_buffer, buf);
      strcpy(buf, "Special procedure : ");
      strcat(buf,(rm->funct) ? "Exists\n\r" : "No\n\r");
      strcat(page_buffer, buf);
      strcat(page_buffer, "Room flags: ");
      sprintbit((long) rm->room_flags,room_bits,buf);
      strcat(buf,"\n\r");
      strcat(page_buffer, buf);
      strcat(page_buffer, "Description:\n\r");
      strcat(page_buffer, rm->description);
      strcpy(buf, "Extra description keywords(s): ");
      if (rm->ex_description) {
	strcat(buf, "\n\r");
	for (desc = rm->ex_description; desc; desc = desc->next) {
	  strcat(buf, desc->keyword);
	  strcat(buf, "\n\r");
	}
	strcat(buf, "\n\r");
	strcat(page_buffer, buf);
      }
      else {
	strcat(buf, "None\n\r");
	strcat(page_buffer, buf);
      }
      strcpy(buf, "Chars present:\n\r");
      for (k = rm->people; k; k = k->next_in_room) {
	if(!CAN_SEE(ch,k))
	  continue;
	strcat(buf, GET_NAME(k));
	strcat(buf, (!IS_NPC(k) ? "(PC)\n\r" :
		     (!IS_MOB(k) ? "(NPC)\n\r" : "(MOB)\n\r")));
      }
      strcat(buf, "\n\r");
      strcat(page_buffer, buf);
      strcpy(buf, "Contents:\n\r");
      for (j = rm->contents; j; j = j->next_content) {
	strcat(buf, j->name);
	strcat(buf, "\n\r");
      }
      strcat(buf, "\n\r");
      strcat(page_buffer, buf);
      strcat(page_buffer, "Exits:\n\r");
      for (i = 0; i <= 5; i++) {
	if (rm->dir_option[i]) {
	  sprintf(buf,"Direction %s. Keyword : %s\n\r",
		  dirs[i], rm->dir_option[i]->keyword);
	  strcat(page_buffer, buf);
	  strcpy(buf, "Description:\n\r  ");
	  if (rm->dir_option[i]->general_description)
	    strcat(buf, rm->dir_option[i]->general_description);
	  else
	    strcat(buf,"UNDEFINED\n\r");
	  strcat(page_buffer, buf);
	  sprintbit(rm->dir_option[i]->exit_info,exit_bits,buf2);
	  sprintf(buf, "Exit flag: %s\n\rKey #: %d\n\r",
		  buf2, rm->dir_option[i]->key);
	  sprintf(buf2, "To room(R-Num): %d\n\r",
		  rm->dir_option[i]->to_room);
	  strcat(buf, buf2);
	  strcat(page_buffer, buf);
	}
      }
      page_string(ch->desc, page_buffer, 1);
      return;
    } /* end of stat room */
    
    
    
    /* stat obj */
    if ((cmd != 232) && (cmd != 233) && (j = get_obj_vis(ch, arg1))) {
      virtual = (j->item_number >= 0) ?
	obj_index[j->item_number].virtual : 0;
      sprintf(page_buffer, "Object name: [%s], R-num: [%d], ",
	      j->name, j->item_number);
      sprintf(buf, "V-number: [%d]\n\rItem type: ", virtual);
      strcat(page_buffer, buf);
      sprinttype(GET_ITEM_TYPE(j),item_types,buf2);
      strcat(page_buffer,buf2);
      strcat(page_buffer,"\n\r");
      sprintf(buf, "Short desc: %s\n\rLong desc:\n\r%s\n\r",
	      ((j->short_description) ? j->short_description : "None"),
	      ((j->description) ? j->description : "None") );
      strcat(page_buffer, buf);
      if (j->ex_description) {
	strcpy(buf, "\n\rExtra desc keyword(s):\n\r");
	for (desc = j->ex_description; desc; desc = desc->next) {
	  strcat(buf, desc->keyword);
	  strcat(buf, "\n\r");
	}
	strcat(page_buffer, buf);
      }
      else {
	strcpy(buf,"\n\rExtra desc keyword(s): None\n\r");
	strcat(page_buffer, buf);
      }
      strcat(page_buffer, "\n\rCan be worn on :");
      sprintbit(j->obj_flags.wear_flags,wear_bits,buf);
      strcat(buf,"\n\r");
      strcat(page_buffer, buf);
      strcat(page_buffer, "Extra flags: ");
      sprintbit(j->obj_flags.extra_flags,extra_bits,buf);
      strcat(buf,"\n\r\n\r");
      strcat(page_buffer, buf);
      sprintf(buf,"Weight: %d, Value: %d, Timer: %d\n\r",
	      j->obj_flags.weight,j->obj_flags.cost, j->obj_flags.timer);
      strcat(page_buffer, buf);
      sprintf(buf,"Values 0-3 : [%d] [%d] [%d] [%d]\n\r",
	      j->obj_flags.value[0], j->obj_flags.value[1],
	      j->obj_flags.value[2], j->obj_flags.value[3]);
      strcat(page_buffer, buf);
      strcat(page_buffer, "Can affect char :\n\r");
      for (i=0;i<MAX_OBJ_AFFECT;++i) {
	sprinttype(j->affected[i].location,apply_types,buf2);
	sprintf(buf, "  Affects: %s by %d\n\r",
		buf2,j->affected[i].modifier);
	strcat(page_buffer, buf);
      }
      page_string(ch->desc, page_buffer, 1);
      return;
    }
    /* end of stat obj */
    
    
    /* pstat PC */
    if (cmd==233)
      k = get_specific_vis(ch,arg1,0);
    /* mstat mob */
    else if (cmd==232)
      k = get_specific_vis(ch,arg1,1);
    /* stat PC or mob */
    else
      k = get_char_vis(ch, arg1);
    
    if (k) {
      sprintf(page_buffer,"Sex: %d,",k->player.sex);
      sprintf(buf, " %s, Name: %s\n\r",
	      (!IS_NPC(k) ? "PC" : (!IS_MOB(k) ? "NPC" : "MOB")),
	      GET_NAME(k));
      strcat(page_buffer, buf);
      if (IS_NPC(k)) {
	sprintf(buf, "V-Number [%d]\n\r", mob_index[k->nr].virtual);
	strcat(page_buffer, buf);
	strcpy(buf,"Short desc: ");
	strcat(buf, (k->player.short_descr ?
		     k->player.short_descr : "None"));
	strcat(buf,"\n\r");
	strcat(page_buffer, buf);
	strcat(page_buffer, "Long desc: ");
	if (k->player.long_descr)
	  strcat(page_buffer, k->player.long_descr);
	else
	  strcat(page_buffer, "None.\n\r");
      }
      else {
	strcpy(buf,"Title: ");
	strcat(buf, (k->player.title ? k->player.title : "None"));
	strcat(buf,"\n\r");
	strcat(page_buffer, buf);
      }
      strcpy(buf, "Class: ");
      sprinttype(k->player.class, pc_class_types, buf2);
      strcat(buf, buf2);
      sprintf(buf2,"  Level [%d] Alignment[%d]\n\r",k->player.level,
	      k->specials.alignment);
      strcat(buf, buf2);
      strcat(page_buffer, buf);
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
      strcat(page_buffer, buf);
      sprintf(buf,"Age: %d Y, %d M, %d D, %d H.",
	      age(k).year, age(k).month, age(k).day, age(k).hours);
      sprintf(buf2, "  Ht: %d cm, Wt: %d lbs\n\r",
	      GET_HEIGHT(k),GET_WEIGHT(k));
      strcat(buf,buf2);
      strcat(page_buffer, buf);
      sprintf(buf,
	      "Str:[%d/%d]  Int:[%d]  Wis:[%d]  Dex:[%d]  Con:[%d]\n\r",
	      GET_STR(k), GET_ADD(k), GET_INT(k),
	      GET_WIS(k), GET_DEX(k), GET_CON(k));
      strcat(page_buffer, buf);
      sprintf(buf,"Mana: %d/%d+%d, Hits: %d/%d+%d, Moves: %d/%d+%d\n\r",
	      GET_MANA(k),mana_limit(k),mana_gain(k),
	      GET_HIT(k),hit_limit(k),hit_gain(k),
	      GET_MOVE(k),move_limit(k),move_gain(k) );
      strcat(page_buffer, buf);
      
      sprintf(buf, "AC: %d/10, Hitroll: %d, Damroll: %d, Regen: %d\n\r",
	      GET_AC(k), k->points.hitroll, k->points.damroll, k->regeneration);
      strcat(page_buffer, buf);
      sprintf(buf, "Gold: %lld, Bank: %lld, Exp: %lld\n\r",
	      GET_GOLD(k), k->bank, GET_EXP(k));
      strcat(page_buffer, buf);
      
      if (IS_NPC(k)) {
	strcpy(buf,"Special: ");
	strcat(buf,(mob_index[k->nr].func ? "Exists\n\r" : "None\n\r"));
	strcat(page_buffer, buf);
      }
      
      sprintf(buf, "Remortal : ");
      if(k->player.remortal & REMORTAL_MAGIC_USER)
	strcat(buf, "M");
      if(k->player.remortal & REMORTAL_CLERIC)
	strcat(buf, "C");
      if(k->player.remortal & REMORTAL_THIEF)
	strcat(buf, "T");
      if(k->player.remortal & REMORTAL_WARRIOR)
	strcat(buf, "W");
      strcat(buf, "\n\r");
      strcat(page_buffer, buf);
      
      sprintf(buf, "Bare Hand Damage %dd%d.\n\r",
	      k->specials.damnodice, k->specials.damsizedice);
      strcat(page_buffer, buf);
      sprintf(buf,"Carried weight: %d   Carried items: %d\n\r",
	      IS_CARRYING_W(k), IS_CARRYING_N(k));
      strcat(page_buffer, buf);
      for (i = 0, j = k->carrying; j; j = j->next_content, i++) ;
      sprintf(buf,"Items in inv: %d, ",i);
      for (i = 0, i2 = 0; i < MAX_WEAR; i++)
	if (k->equipment[i])
	  i2++;
      sprintf(buf2,"Items in equ: %d\n\r", i2);
      strcat(buf,buf2);
      strcat(page_buffer, buf);
      if (k->desc) {
	sprinttype(k->desc->connected,connected_types,buf2);
	strcpy(buf,"Connected: ");
	strcat(buf,buf2);
	sprintf(buf2," %s (%d)\n\r",k->desc->host,k->desc->descriptor);
	strcat(buf,buf2);
	strcat(page_buffer, buf);
      }
      if (IS_NPC(k)) {
	strcpy(buf,"NPC flags: ");
	sprintbit(k->specials.act,action_bits,buf2);
      }
      else {
	strcpy(buf,"PC flags: ");
	sprintbit(k->specials.act,player_bits,buf2);
      }
      strcat(buf,buf2);
      strcat(buf,"\n\r");
      strcat(page_buffer, buf);
      
      if (!IS_NPC(k)) {
	sprintf(buf, "Thirst: %d, Hunger: %d, Drunk: %d\n\r",
		k->specials.conditions[THIRST],
		k->specials.conditions[FULL],
		k->specials.conditions[DRUNK]);
	sprintf(buf2,"Practices: %d\n\r",k->specials.spells_to_learn);
	strcat(buf,buf2);
	strcat(page_buffer, buf);
      }
      sprintf(buf,"Master is '%s'\n\r",
	      ((k->master) ? GET_NAME(k->master) : "NOBODY"));
      strcat(page_buffer, buf);
      strcat(page_buffer, "Followers are:\n\r");
      for (fol = k->followers; fol; fol = fol->next) {
	sprintf(buf, "  %s", GET_NAME(fol->follower));
	strcat(page_buffer, buf);
      }
      /* Showing the bitvector */
      strcat(page_buffer, "Affected by: ");
      sprintbit(k->specials.affected_by,affected_bits,buf);
      strcat(buf,"\n\r");
      strcat(page_buffer, buf);
      if (k->affected) {
	strcat(page_buffer, "Affecting Spells:\n\r");
	for (aff = k->affected; aff; aff = aff->next) {
	  sprintf(buf, "%s: %s by %d, %d hrs, bits: ",
		  spells[aff->type-1],
		  apply_types[(int) aff->location],
		  aff->modifier,aff->duration);
	  sprintbit(aff->bitvector,affected_bits,buf2);
	  strcat(buf,buf2);
	  strcat(buf,"\n\r");
	  strcat(page_buffer, buf);
	}
      }
      page_string(ch->desc, page_buffer, 1);
      return;
    }
    else {
      send_to_char("No mobile or object by that name in the world\n\r",
		   ch);
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
	victim = ch;
  }
  else if(!(victim=get_char_vis(ch, arg))) {
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
        send_to_char("Ok.\n\r", ch);
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
  if(!ch->desc)
    return;

  if(!ch->desc->original)
   { 
    send_to_char("Eh?\n\r", ch);
    return;
  }
  else
  {
    send_to_char("You return to your originaly body.\n\r",ch);

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
  char name[100], to_force[100],buf[200]; 
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
      if (i->character != ch && !i->connected) {
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

  if (IS_NPC(ch) || GET_LEVEL(ch) > (IMO+4))
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
    if ((r_num = real_object(number)) < 0) {
      send_to_char("There is no object with that number.\n\r", ch);
      return;
    }
    obj = read_object(r_num, REAL);
    if(GET_LEVEL(ch) < (IMO+2))
    if(IS_SET(obj->obj_flags.extra_flags,ITEM_NOLOAD)){
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
    if ((vict = get_char_room_vis(ch, name))) {
      if(!IS_NPC(vict)){
        if(GET_LEVEL(ch) < GET_LEVEL(vict)){
          sprintf(buf,"%s tried to purge you.\n\r",ch->player.name);
          send_to_char(buf,vict);
          return;
        }
        stash_char(vict);
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
    } else if((obj = get_obj_in_list_vis(ch,name,world[ch->in_room].contents))) {
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
  int i, j, k, temp;
  ubyte table[5];
  ubyte rools[4];

  for(i=0; i<5; table[i++]=0)  ;

  for(i=0; i<5; i++) {

    for(j=0; j<4; j++)
      rools[j] = number(1,6);
    
    temp = MIN( 18, rools[0]+rools[1]+rools[2]+rools[3] );

    for(k=0; k<5; k++)
      if (table[k] < temp)
        SWITCH(temp, table[k]);
  }

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
      if (ch->abilities.str == 18)
        ch->abilities.str_add = number(0,100);
    } break;
  }
  ch->tmpabilities = ch->abilities;
}

void do_start(struct char_data *ch)
{
	void advance_level(struct char_data *ch, int level_up);

	/*
	send_to_char("Welcome. This is now your character in MUD.\n\r", ch);
	*/

	GET_LEVEL(ch) = 1;
	GET_EXP(ch) = 1;
	set_title(ch);

	/*
	switch (GET_CLASS(ch)) {
		case CLASS_MAGIC_USER:
			break;
		case CLASS_CLERIC:
			break;
		case CLASS_THIEF:
			ch->skills[SKILL_SNEAK].learned = 10;
			ch->skills[SKILL_HIDE].learned = 5;
			ch->skills[SKILL_STEAL].learned = 15;
			ch->skills[SKILL_BACKSTAB].learned = 10;
			ch->skills[SKILL_PICK_LOCK].learned = 10;
			break;
		case CLASS_WARRIOR:
			break;
	}
	*/
	advance_level(ch, 1);

	GET_HIT(ch) = GET_PLAYER_MAX_HIT(ch);
	GET_MANA(ch) = GET_PLAYER_MAX_MANA(ch);
	GET_MOVE(ch) = GET_PLAYER_MAX_MOVE(ch);

	GET_COND(ch,THIRST) = 24;
	GET_COND(ch,FULL) = 24;
	GET_COND(ch,DRUNK) = 0;

	ch->specials.spells_to_learn=3;
	ch->player.time.played = 0;
	ch->player.time.logon = time(0);
}

void do_advance(struct char_data *ch, char *argument, int cmd)
{
	struct char_data *victim;
	char name[100], level[100];
	int i, newlevel;

	void gain_exp(struct char_data *ch, int gain);
	void init_char(struct char_data *ch);

	if (IS_NPC(ch))
		return;

	if (GET_LEVEL(ch) < (IMO+3)) {
		send_to_char("You can only do that in a guild.\n\r",ch);
		return;
	}
	argument_interpreter(argument, name, level);
	if (*name) {
		if (!(victim = get_char_room_vis(ch, name))) {
			send_to_char("That player is not here.\n\r", ch);
			return;
		}
	}
	else {
		send_to_char("Advance who?\n\r", ch);
		return;
	}

	if (IS_NPC(victim)) {
		send_to_char("NO! Not on NPC's.\n\r", ch);
		return;
	}

	if (!*level) {
		send_to_char("You must supply a level number.\n\r", ch);
		return;
	}
	else {
		if (!ISDIGIT(*level)) {
			send_to_char("Second argument must be a positive integer.\n\r",ch);
			return;
		}
		if ((newlevel=atoi(level)) < GET_LEVEL(victim)) {
			if (GET_LEVEL(ch) <= GET_LEVEL(victim)) {
				send_to_char("Very amusing.\n\r",ch);
				return;
			}
			victim->player.level=newlevel;
			if (newlevel < IMO) {
				for (i = 0; i < 3; ++i)
					victim->specials.conditions[i]=0;
			}
			victim->tmpabilities = victim->abilities;
			send_to_char("The poor soul...\n\r", ch);
			send_to_char("You have been punished.\n\r",victim);
			return;
		}
	}

	if (newlevel > IMO) {
		send_to_char("Too high a level.\n\r",ch);
		return;
	}
	send_to_char("You feel generous.\n\r", ch);

	if (GET_LEVEL(victim) == 0) {
		init_char(victim);
		do_start(victim);
	}
	else {
		victim->points.exp=1;
		gain_exp_regardless(victim, (titles[GET_CLASS(victim)-1][
			newlevel].exp)-GET_EXP(victim));
	}
}

void do_reroll(struct char_data *ch, char *argument, int cmd)
{
  struct char_data *victim;
  char buf[100];

  if (IS_NPC(ch) || GET_LEVEL(ch) > (IMO+3))
    return;

  one_argument(argument,buf);
  if (!*buf)
    send_to_char("Who do you wish to reroll?\n\r",ch);
  else
    if(!(victim = get_char(buf)))
      send_to_char("No-one by that name in the world.\n\r",ch);
    else {
      roll_abilities(victim);
      sprintf(buf,"Rerolled: %d/%d %d %d %d %d\n",victim->abilities.str,
        victim->abilities.str_add,
        victim->abilities.intel,
        victim->abilities.wis,
        victim->abilities.dex,
        victim->abilities.con);
      send_to_char(buf,ch);
    }
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
      GET_MANA(victim) = GET_PLAYER_MAX_MANA(victim);
      GET_HIT(victim) = GET_PLAYER_MAX_HIT(victim);
      GET_MOVE(victim) = GET_PLAYER_MAX_MOVE(victim);

      if (GET_LEVEL(victim) >= IMO) {
        for (i = 0; i < MAX_SKILLS; i++) {
          victim->skills[i].learned = 100;
          victim->skills[i].recognise = 0;
        }

        if (GET_LEVEL(victim) >= (IMO+3)) {
          victim->abilities.str_add = 100;
          victim->abilities.intel = 25;
          victim->abilities.wis = 25;
          victim->abilities.dex = 25;
          victim->abilities.str = 25;
          victim->abilities.con = 25;
        }
        victim->tmpabilities = victim->abilities;

      }
      update_pos( victim );
      send_to_char("Done.\n\r", ch);
      act("You have been fully healed by $N!", FALSE, victim, 0, ch, TO_CHAR);
    }
}

void do_noshout(struct char_data *ch, char *argument, int cmd)
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
FILE *chatlogfp = NULL;
void do_chat(struct char_data *ch, char *argument, int cmd)
{
  struct descriptor_data *i;
  struct char_data *victim;
  char buf[MAX_STRING_LENGTH];
  int j;

  extern int nochatflag;

  if(IS_SET(ch->specials.act,PLR_DUMB_BY_WIZ)&&GET_LEVEL(ch)<IMO+3)
  {
	return;
  }
  if(nochatflag){
    send_to_char("chat is forbidened now.\n\r",ch);
    return;
  }
  if (IS_NPC(ch))
    return;
  if(strcmp(" /last",argument))
  {
	time_t tt = time(NULL);
	strftime(buf, 127, "%F %H:%M", localtime(&tt));
	sprintf(buf + strlen(buf)," %s >%s\n\r", GET_NAME(ch), argument);
	assert(his_end>=0&&his_end<20);
	strcpy(history[his_end], &buf[5]);	// omit year part
	his_end++;
	if((his_end%20)==(his_start%20))
	{
		his_end=his_start%20;
		his_start=(his_start+1)%20;
	}

#ifdef CHATLOG
	if (chatlogfp == NULL)
	    chatlogfp = fopen(CHATLOG, "a");
	fputs(buf, chatlogfp);
	fflush(chatlogfp);
	/* 20110117 by Moon */
#endif

  }
  else 
  {
	for(j=his_start;j!=his_end;j=(j+1)%20)
	{
		assert(history[j]);
		send_to_char(history[j],ch);
	}
	return;
  }
  sprintf(buf,"%s> %s\n\r",ch->player.name,argument);
  for (i = descriptor_list; i; i = i->next)
    if (!i->connected) {
      if(i->original) continue;
      victim = i->character;
      if(!IS_SET(victim->specials.act,PLR_NOCHAT))
        send_to_char(buf,victim);
    }
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
	}
	else {
		if (!generic_find(argument, FIND_CHAR_WORLD, ch, &vict, &dummy))
			send_to_char("Couldn't find any such creature.\n\r", ch);
		else if (IS_NPC(vict) && vict->in_room != ch->in_room)
			send_to_char("Only can do that to a mob in same room.\n\r", ch);
		else if (!IS_NPC(vict) && GET_LEVEL(vict) > GET_LEVEL(ch))
			act("$E might object to that.. better not.",0,ch,0,vict,TO_CHAR);
		else {
			send_to_char("You are normal again.\n\r", vict);
			for (hjp = vict->affected; hjp; hjp = hjp->next)
				affect_remove( vict, hjp );
		}
	}
	send_to_char("Ok.\n\r",ch);
}
void do_wall(struct char_data *ch, char *argument, int cmd)
{
  char buf[MAX_STRING_LENGTH];

  if(IS_NPC(ch)||(! *argument) || GET_LEVEL(ch) > (IMO+3))
    return;
  sprintf(buf,"%s\n\r",argument+1);
  send_to_all(buf);
  send_to_char("Ok.\n\r",ch);
}

void do_set(struct char_data *ch, char *argument, int cmd)
{
  struct char_data *victim;
  char mess[256],buf[64],buf2[32],buf3[32],buf4[32],buf5[32],buf6[32];
  int i,j;
  unsigned int k;
  extern int nokillflag,nostealflag,nonewplayers,noshoutflag,nochatflag;
  extern int nodisarmflag; /* chase written */
  bool	target_ok = FALSE;
  struct obj_data *tar_obj = NULL;

  if (IS_NPC(ch))
    return;
  if (GET_LEVEL(ch)<(IMO+3))
	return;
  half_chop(argument,buf,buf2);
  if(!*buf){
    sprintf(mess,
      " nokill=%d\n\rnosteal=%d\n\rfreeze=%d\n\rnoshout=%d\n\rnochat=%d\n\rbaddomain=%s\n\rnodisarm=%d\n\rregen percent=%d\n\rregen time percent=%d\n\rregen time=%d\n\rreboot time=%ld\n\r",
      nokillflag,nostealflag,nonewplayers,noshoutflag,nochatflag,baddomain[0],
	  nodisarmflag,regen_percent,regen_time_percent,regen_time,reboot_time); /* chase added nodisarm */
    send_to_char(mess,ch);
    return;
  } else {
    if(strcmp(buf,"nokill")==0){
      one_argument(buf2,buf3);
      if(*buf3)
        nokillflag=atoi(buf3);
      sprintf(mess,"No-kill flag is %d.\n\r",nokillflag);
      send_to_char(mess,ch);
      return;
    }
    if(strcmp(buf,"baddomain")==0){
      one_argument(buf2,buf3);
      if(*buf3)
        strcpy(baddomain[0],buf3);
      sprintf(mess,"Bad Domain is %s.\n\n",baddomain[0]);
      send_to_char(mess,ch);
      return;
    }
    if(strcmp(buf,"nosteal")==0){
      one_argument(buf2,buf3);
      if(*buf3)
        nostealflag=atoi(buf3);
      sprintf(mess,"No-steal flag is %d.\n\r",nostealflag);
      send_to_char(mess,ch);
      return;
    }
	if(strcmp(buf,"nodisarm")==0){ /* Chase written */
		one_argument(buf2,buf3);
		if(*buf3)
			nodisarmflag=atoi(buf3);
		sprintf(mess,"No-disarm flag is %d.\n\r",nodisarmflag);
		send_to_char(mess,ch);
		return;
	}
    if(strcmp(buf,"freeze")==0){
      one_argument(buf2,buf3);
      if(*buf3)
        nonewplayers=atoi(buf3);
      sprintf(mess,"Freeze flag is %d.\n\r",nonewplayers);
      send_to_char(mess,ch);
      return;
    }
    if(strcmp(buf,"noshout")==0){
      one_argument(buf2,buf3);
      if(*buf3)
        noshoutflag=atoi(buf3);
      sprintf(mess,"NoShout flag is %d.\n\r",noshoutflag);
      send_to_char(mess,ch);
      return;
    }
    if(strcmp(buf,"nochat")==0){
      one_argument(buf2,buf3);
      if(*buf3)
        nochatflag=atoi(buf3);
      sprintf(mess,"Nochat flag is %d.\n\r",nochatflag);
      send_to_char(mess,ch);
      return;
    }
    if(strcmp(buf,"regen_percent")==0){
      one_argument(buf2,buf3);
      if(*buf3)
        regen_percent= atoi(buf3);
      sprintf(mess,"Regeneration Percentage is %d.\n\n",regen_percent);
      send_to_char(mess,ch);
      return;
    }
    if(strcmp(buf,"regen_time")==0){
      one_argument(buf2,buf3);
      if(*buf3)
        regen_time = atoi(buf3);
      sprintf(mess,"Equipment-Regeneration-Time is %d.\n\n", regen_time);
      send_to_char(mess,ch);
      return;
    }
    if(strcmp(buf,"regen_time_percent")==0){
      one_argument(buf2,buf3);
      if(*buf3)
        regen_time_percent= atoi(buf3);
      sprintf(mess,"Regeneration Time is %d / 100 of original.\n\n",
		regen_time_percent);
      send_to_char(mess,ch);
      return;
    }
    if(strcmp(buf,"reboot_time")==0){
      one_argument(buf2,buf3);
      if(*buf3)
          reboot_time= atoi(buf3);
      sprintf(mess,"Reboot Time is %ld seconds after current boot time .\n\n",
		reboot_time);
      send_to_char(mess,ch);
      return;
    }
    if(!(victim = get_char(buf))) {
      if (!target_ok)
	if ((tar_obj 
	     = get_obj_in_list_vis(ch, buf, ch->carrying)))
	  target_ok = TRUE;
      if (!target_ok)
	if ((tar_obj 
	     = get_obj_in_list_vis(ch, buf,
				   world[ch->in_room].contents)))
	  target_ok = TRUE;
      if (!target_ok)
	if ((tar_obj = get_obj_vis(ch, buf)))
	  target_ok = TRUE;
      if (!target_ok)
	for (i=0; i<MAX_WEAR && !target_ok; i++)
	  if (ch->equipment[i] 
	      && str_cmp(buf, ch->equipment[i]->name) == 0) {
	    tar_obj = ch->equipment[i];
	    target_ok = TRUE;
	  }
      if (!target_ok) {
	send_to_char("No-object by that name in the world.\n\r",ch);
	return;
      }
      half_chop(buf2,buf3,buf4);
      k = atoi(buf4);
      if (GET_LEVEL(ch) < (IMO+3)) return;
      if (strcmp(buf3,"value1") == 0)
	tar_obj->obj_flags.value[0] = k;
      else if (strcmp(buf3,"value2") == 0)
	tar_obj->obj_flags.value[1] = k;
      else if (strcmp(buf3,"value3") == 0)
	tar_obj->obj_flags.value[2] = k;
      else if (strcmp(buf3,"value4") == 0)
	tar_obj->obj_flags.value[3] = k;
      else if (strcmp(buf3,"weight") == 0)
	tar_obj->obj_flags.weight = k;
      else if (strcmp(buf3,"type") == 0)
	tar_obj->obj_flags.type_flag = k;
      else if (strcmp(buf3,"extra") == 0)
	tar_obj->obj_flags.extra_flags = k;
      else if (strcmp(buf3,"wear") == 0)
	tar_obj->obj_flags.wear_flags = k;
      else if (strcmp(buf3,"cost") == 0)
	tar_obj->obj_flags.cost = k;
      else if (strcmp(buf3,"magic") == 0)
	tar_obj->obj_flags.gpd = k;
      else if (strcmp(buf3,"affect1") == 0) {
	half_chop(buf4,buf5,buf6);
	k = atoi(buf5);
	j = atoi(buf6);
	tar_obj->affected[0].location = k;
	tar_obj->affected[0].modifier = j;
      }
      else if (strcmp(buf3,"affect2") == 0) {
	half_chop(buf4,buf5,buf6);
	k = atoi(buf5);
	j = atoi(buf6);
	tar_obj->affected[1].location = k;
	tar_obj->affected[1].modifier = j;
      }
      else {
	send_to_char("Huh?\n\r",ch);
      }
    }
    else {
      half_chop(buf2,buf3,buf4);
      k=atoi(buf4);
      LONGLONG kk=atoll(buf4);
      if((GET_LEVEL(ch) < (IMO+2))&&(strcmp(buf3,"gold"))) return;
      if(strcmp(buf3,"exp")==0 && (GET_LEVEL(ch) > (IMO+2)))
        victim->points.exp=kk;
      else if(strcmp(buf3,"skill")==0) {
        for(i=0;i<MAX_SKILLS;i++)
          victim->skills[i].learned=k;		/* clear skills */
      }
      else if(strcmp(buf3,"recognise")==0) {
	k = atoi(buf4);
	for(i=0;i<MAX_SKILLS;i++)
	  victim->skills[i].recognise=k;
      }
      else if(strcmp(buf3,"lev")==0)
	GET_LEVEL(victim)=k;
      else if(strcmp("hit",buf3)==0)
	victim->points.hit=victim->points.max_hit=k;
      else if(strcmp("age",buf3)==0){
	victim->player.time.birth = time(0);
	victim->player.time.played = 0;
	victim->player.time.logon = time(0); }
      else if(strcmp("mana",buf3)==0)
	victim->points.mana=victim->points.max_mana=k;
      else if(strcmp("move",buf3)==0)
	victim->points.move=victim->points.max_move=k;
      else if(strcmp("bank",buf3)==0)
        victim->bank=kk;
      else if(strcmp("gold",buf3)==0)
        victim->points.gold=kk;
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
      else if(strcmp("handno",buf3)==0)
	victim->specials.damnodice=k;
      else if(strcmp("handsize",buf3)==0)
	victim->specials.damsizedice=k;
      else if(strcmp("quest",buf3)==0)
        victim->quest.solved=k;
      else if(strcmp("sex",buf3)==0) {
	if(strncmp("m",buf4,1)==0 || strncmp("M",buf4,1)==0 )
	  victim->player.sex = 1;
	else if(strncmp("f",buf4,1)==0 || strncmp("F",buf4,1)==0 )
	  victim->player.sex = 2;
	else
	  send_to_char("Invalid sex\n\r",ch);
      }
      else if(strcmp("ac",buf3)==0)
	victim->points.armor=k;
      else if(strcmp("remortal", buf3)==0) {
	if(strncmp("w",buf4,1)==0 || strncmp("W",buf4,1)==0 ){
	  victim->player.remortal |= REMORTAL_WARRIOR;
	  send_to_char("WARRIOR. Done.\n\r",ch);
	}
	else if(strncmp("c",buf4,1)==0 || strncmp("C",buf4,1)==0 ){
	  victim->player.remortal |= REMORTAL_CLERIC;
	  send_to_char("CLERIC. Done.\n\r",ch);
	}
	else if(strncmp("m",buf4,1)==0 || strncmp("M",buf4,1)==0 ){
	  victim->player.remortal |= REMORTAL_MAGIC_USER;
	  send_to_char("MAGIC USER. Done.\n\r",ch);
	}
	else if(strncmp("t",buf4,1)==0 || strncmp("T",buf4,1)==0 ){
	  victim->player.remortal |= REMORTAL_THIEF;
	  send_to_char("THIEF. Done.\n\r",ch);
	}
	else {
	  victim->player.remortal = 0;
	  send_to_char("Invalid class. remortal is resetted.\n\r",ch);
      	}
      }
      else if(strcmp("class",buf3)==0) {
	if(strncmp("w",buf4,1)==0 || strncmp("W",buf4,1)==0 )
	  victim->player.class = 4;
	else if(strncmp("c",buf4,1)==0 || strncmp("C",buf4,1)==0 )
	  victim->player.class = 2;
	else if(strncmp("m",buf4,1)==0 || strncmp("M",buf4,1)==0 )
	  victim->player.class = 1;
	else if(strncmp("t",buf4,1)==0 || strncmp("T",buf4,1)==0 )
	  victim->player.class = 3;
	else
	  send_to_char("Invalid class\n\r",ch);
      }
      else if(strcmp("passwd",buf3)==0) {
	strcpy(victim->desc->pwd,buf4);
#ifdef  RETURN_TO_QUIT  
	save_char(ch,world[ch->in_room].number);
#else
	save_char(ch,ch->in_room);
#endif
      }
      else
        send_to_char("Huh?\n\r",ch);
      victim->tmpabilities = victim->abilities;
    }
  }
}
void do_invis(struct char_data *ch, char *argument, int cmd)
{
  if (IS_NPC(ch) || GET_LEVEL(ch) > (IMO+4))
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
void do_banish(struct char_data *ch, char *argument, int cmd)
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
void do_flag(struct char_data *ch, char *argument, int cmd)
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
		if(cmd!=297)
		{
        REMOVE_BIT(vict->specials.act, PLR_CRIMINAL);
        send_to_char("Flag removed.\n\r",ch);
		}
      }
    } else {
      if(cmd == 231){
        SET_BIT(vict->specials.act, PLR_XYZZY);
        send_to_char("Log set.\n\r",ch);
      } else {
		if(cmd!=297)
		{
        SET_BIT(vict->specials.act, PLR_CRIMINAL);
        send_to_char("Flag set.\n\r",ch);
		}
      }
    }
	if(cmd==297) /* wizdumb */
	{
	  if(IS_SET(vict->specials.act,PLR_DUMB_BY_WIZ))
	  {
		REMOVE_BIT(vict->specials.act,PLR_DUMB_BY_WIZ);
		send_to_char("DUMB removed.\n\r",ch);
	  }
	  else
	  {
		SET_BIT(vict->specials.act,PLR_DUMB_BY_WIZ);
		send_to_char("DUMB set.\n\r",ch);
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
  char buffer[256];
  extern int boottime;
  extern struct char_data *character_list;
  extern struct obj_data *object_list;
  extern struct descriptor_data *descriptor_list;
  struct char_data *i;
  struct obj_data *k;
  struct descriptor_data *d;
  static int nits,nics,nids;

  getrusage(0,&xru);
  sprintf(buffer,
    "sys time: %d secs\n\rusr time: %d secs\n\rrun time: %d secs\n\r",
    (int) xru.ru_stime.tv_sec, (int) xru.ru_utime.tv_sec, (int) (time(0)-boottime));
  send_to_char(buffer,ch);
  if(GET_LEVEL(ch) >= (IMO+2)){
    nits=0;
    for (k=object_list;k;k=k->next) ++nits;
    nics=0;
    for (i=character_list;i;i=i->next) ++nics;
    nids=0;
    for (d=descriptor_list;d;d=d->next) ++nids;
  }
  sprintf(buffer," objects: %d\n\r   chars: %d\n\r players: %d\n\r",
    nits,nics,nids);
  send_to_char(buffer,ch);
}
