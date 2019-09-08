/* ************************************************************************
*  file: guild_command.c , Special module.                  Part of DIKUMUD *
*  Usage: Procedures handling special procedures for object/room/mobile   *
*  Made by Shin Won-dong  in KAIST                                        *
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
#include "limit.h"

#include "guild_list.h" /* by process */
/*   external vars  */
extern struct room_data *world;
extern struct char_data *character_list;
extern struct descriptor_data *descriptor_list;
extern struct index_data *obj_index;
extern struct time_info_data time_info;
extern struct title_type titles[4][IMO+4];
extern struct index_data *mob_index;

extern char *guild_names[];
extern int guild_skill_nums[];
extern char *police_skills[];
extern char *outlaw_skills[];
extern char *assasin_skills[];
extern int police_skill_costs[];
extern int outlaw_skill_costs[];
extern int assasin_skill_costs[];



void do_look(struct char_data *ch, char *arg, int cmd);
void log(char *str);
void stash_char(struct char_data *ch);
void unstash_char(struct char_data *ch, char *filename);
int MIN(int a, int b);



/* shuttle bus to KAIST */

char *msg_for_taxi[]=
{
  "NOT-DEFINED",
  "Welcome to KAIST!!!\n\r",
	"Welcome to Process' house\n\r"
};

char *where_to_taxi[]=
{
  "NOT-DEFINED",
  "KAIST",
  "PROCESS House",
};

int charge_taxi[]=
{
  0,
  2000,
  3000,
};

int room_taxi[]=
{
  0,
  31001,
  2000
};

int level_taxi[]=
{
  0,
  10,
  10
};
int taxi(struct char_data *ch,int cmd,char *arg)
{
  char buf[MAX_STRING_LENGTH];
  int room_number;
  int taxi_num;
  int to_room;
  
  if(IS_NPC(ch))
    return FALSE;
  room_number=world[ch->in_room].number;
#define TO_KAIST 3014
#define TO_PROCESS 3502
  switch(room_number) {
  case TO_KAIST:
    taxi_num=1;
    break;
  case TO_PROCESS:
    taxi_num=2;
    break;			
  default:
    return FALSE;
	}
  to_room=real_room(room_taxi[taxi_num]);
  
  if(cmd==294) {/* call */
    
    if(GET_LEVEL(ch)<level_taxi[taxi_num]) {
      send_to_char("You cannot ride this taxi.sorry",ch);
      return TRUE;
    }
    send_to_char(msg_for_taxi[taxi_num],ch);
    send_to_char("Sit down on your seat,and wait for a while\n\r",ch);
    sprintf(buf,"The taxi with %s starts to leave for %s.",GET_NAME(ch),where_to_taxi[taxi_num]);
    act(buf,TRUE,ch,0,0,TO_ROOM);
    char_from_room(ch);
    sprintf(buf,"OK. Here is %s.\n\r",where_to_taxi[taxi_num]);
    send_to_char(buf,ch);
    sprintf(buf,"The charge is %d.\n\r",charge_taxi[taxi_num]);
    send_to_char(buf,ch);
    if(GET_GOLD(ch)<charge_taxi[taxi_num]) {
      send_to_char("The taxi driver slaps you!!!\n\r",ch);
      send_to_char("Get the fuck out!!n\r",ch);
      GET_HIT(ch)=1;
      GET_MOVE(ch)=1;
      char_to_room(ch,real_room(3001));
      do_look(ch,"",15);
      return TRUE;
    }
    GET_GOLD(ch)-=charge_taxi[taxi_num];
    char_to_room(ch,to_room);
    do_look(ch,"",15);
    return TRUE;
  }
  return FALSE;
}

/* always set exit NORTH to guild from entry */
int guild_entry(struct char_data *ch,int cmd,char *arg)
{
  char buf[MAX_STRING_LENGTH],buf2[MAX_STRING_LENGTH];
  int i,room_number;
  char guild_number;
  
  if(IS_NPC(ch))
    return (FALSE);
  
  /* guild_number = */
  room_number=world[ch->in_room].number;
  
  switch(room_number) {
  case ROOM_GUILD_POLICE:
    guild_number=1;
    break;
  case ROOM_GUILD_OUTLAW:
    guild_number=2;
    break;
  case ROOM_GUILD_ASSASSIN:
    guild_number=3;
    break;
  default:
    return FALSE;
  }
  if( cmd==273 ) {/* JOIN */

    /* find out room-number and figure out guild number which the
       character want to join */
    switch(guild_number) {
    case POLICE:
      if( (GET_LEVEL(ch)<15)&&GET_LEVEL(ch)<IMO ) {
	send_to_char("당신은 경찰이 될만한 자격이 없군요.\n\r",ch);
	return TRUE;	
      }
      break;
    case OUTLAW:
      if( (GET_LEVEL(ch)<20 )&&GET_LEVEL(ch)<IMO ) {
	send_to_char("당신은 깡패가 될만한 자격이 없군요.\n\r",ch);
	return TRUE;
      }
      break;
    case ASSASSIN:
      if( (GET_LEVEL(ch)<25 )&&GET_LEVEL(ch)<IMO ) {
	send_to_char("당신은 암살자가 될만한 자격이 없군요.\n\r",ch);
	return TRUE;
      }
      break;
    default:
      return TRUE;
    }
    
    if ( guild_number == ch->player.guild ) {
      send_to_char("You are joining that guild already!!!",ch);
      return TRUE;
    }
    if ( ch->player.guild != 0 ) {
      send_to_char("You must leave your guild first!!!",ch);
      return TRUE;
    }
    ch->player.guild=guild_number;
    sprintf(buf, "%s JOINED %s guild\n\r",
	    GET_NAME(ch),
	    guild_names[(int) guild_number]);
    send_to_all(buf);
    log(buf);
    for(i=0;i<MAX_GUILD_SKILLS;i++) {
      ch->player.guild_skills[i]=0;
    }
    return TRUE;
  }
  else if ( cmd == 274 ) {/* LEAVE */
    if ( guild_number != ch->player.guild ) {
      send_to_char("You can't leave this guild as you are not a member!!",ch);
      return TRUE;
    }
    sprintf(buf, "%s LEFT %s guild\n\r",
	    GET_NAME(ch),
	    guild_names[(int) ch->player.guild]);
    send_to_all(buf);
    log(buf);
    ch->player.guild=0;
    for(i=0;i<MAX_GUILD_SKILLS;i++) {
      ch->player.guild_skills[i]=0;
    }
    return TRUE;
  }
  else if ( cmd == 1 ) {/* north */
    if ( (guild_number != ch->player.guild)&&GET_LEVEL(ch)<IMO ) {
      sprintf(buf,"The guild guard humiliates you,and block your way.\n\r");
      sprintf(buf2,"The guard humilates $n,and blocks $s way.");
      send_to_char(buf,ch);
      act(buf2,FALSE,ch,0,0,TO_ROOM);
      return TRUE;
    }
  }
  else if (cmd == 84) { /* to prevent cast 'phase' */
    if ((guild_number != ch->player.guild) && GET_LEVEL(ch) < IMO) {
      sprintf(buf, "The guild guard screams, SHUT UP~~!!\n\r");
      send_to_char(buf,ch);
      return TRUE;
    }
  }
  
  return (FALSE);
}

int locker_room(struct char_data *ch,int cmd,char *arg)
{
  char buf[MAX_STRING_LENGTH];
  int i,room_number;
  char guild_number;
  static int count=0;
#define LIMIT_LOAD 10
  static char load_names[LIMIT_LOAD+1][30];
  int equiped_something = 0;
  
  void extract_carrying_obj(struct obj_data *p);
  
  if(IS_NPC(ch)) {
    return (FALSE);
  }
  room_number=world[ch->in_room].number;
  switch(room_number) {
  case 3000:
    guild_number=0;
    break;
  case ROOM_GUILD_POLICE_LOCKER:
    guild_number=1;
    break;
  case ROOM_GUILD_OUTLAW_LOCKER:
    guild_number=2;
    break;
  case ROOM_GUILD_ASSASSIN_LOCKER:
    guild_number=3;
    break;
  default:
    return FALSE;
  }
  
  if ( guild_number < 0 || guild_number > MAX_GUILD_LIST ) {
    return FALSE;
  }
  if ( guild_number != ch->player.guild && GET_LEVEL(ch) < IMO)	{
    send_to_char("But you are not the member of this guild!",ch);
    return FALSE;
  }
  if ( cmd == 277 ) {/* SAVE */
    return FALSE;
#define COST	10000
    if(GET_GOLD(ch) < GET_LEVEL(ch)*GET_LEVEL(ch)*COST ) {
      sprintf(buf,"You need %d coins to SAVE your items.",
	      GET_LEVEL(ch)*GET_LEVEL(ch)*COST);
      send_to_char(buf,ch);	
      return TRUE;
    }
    
    /* only allow to save equiped item */
    if (ch->carrying) {
      send_to_char("You can SAVE only equiped item.\n\r", ch);
      send_to_char("Drop all carrying item!\n\r", ch);
      return TRUE;
    }
    /* only allow to save container which DOSEN'T contain something */
    for (i = 0; i < MAX_WEAR; i++) {
      if (ch->equipment[i]) {
	equiped_something = 1;
	if (ch->equipment[i]->contains) {
	  send_to_char("You can't SAVE container which ", ch);
	  send_to_char("contains something.\n\r", ch);
	  return TRUE;
	}
      }
    }
    /* But, need not to save! what a waste of coins! */
    if (!equiped_something) {
      send_to_char("But, you don't have anything to save!\n\r", ch);
      return TRUE;
    }
    
    GET_GOLD(ch)=GET_GOLD(ch)-(GET_LEVEL(ch)*GET_LEVEL(ch)*COST);
    stash_char(ch);
    send_to_char("Ok. When you want to LOAD items, type LOAD.\n\r",ch);
    return TRUE;
  }
  else if ( cmd == 278 ) {/* LOAD */
    /* only allow 1 load per reboot */
    for (i = 0; i < count; i++) {
      if (strcmp(load_names[i], GET_NAME(ch)) == 0) {
	send_to_char("You already used ONE chance of LOAD!!\n\r",ch);
	return TRUE;
      }
    }
    /* limit number of LOAD in guild */
    if (count >= LIMIT_LOAD) {
      send_to_char("All Lockers already used by other persons!\n\r", ch);
      send_to_char("The following persons used locker!!\n\r",ch);
      for (i = 0; i < LIMIT_LOAD; i++) {
	sprintf(buf, "%s\n\r", load_names[i]);
	send_to_char(buf,ch);
      }
      return TRUE;
    }
    
    strcpy(load_names[i], GET_NAME(ch));
    
    if (ch->carrying) {
      send_to_char("You must carry nothing to LOAD!!\n\r",ch);
      return TRUE;
    }
    for (i = 0; i < MAX_WEAR; i++) {
      if(ch->equipment[i]) {
	send_to_char("You must equip nothing to LOAD!!\n\r",ch);
	return TRUE;
      }
    }
    count++;
    unstash_char(ch,GET_NAME(ch));
    send_to_char("Ok. Your items restored...\n\r",ch);
    return TRUE;
  }
  return FALSE;
}

int guild_practice_yard(struct char_data *ch,int cmd,char *arg)
{
  char buf[MAX_STRING_LENGTH];
  int i,room_number;
  char guild_number;
  int number;
  int cost=0;
  
  if(IS_NPC(ch))
    return FALSE;
  room_number=world[ch->in_room].number;
  switch(room_number) {
  case ROOM_GUILD_POLICE_PRACTICE:
    guild_number=1;
    break;
  case ROOM_GUILD_OUTLAW_PRACTICE:
    guild_number=2;
    break;
  case ROOM_GUILD_ASSASSIN_PRACTICE:
    guild_number=3;
    break;
  default:
    return FALSE;
  }
  if(guild_number!=GET_GUILD(ch) && GET_LEVEL(ch)<IMO) {
    send_to_char("But you are not a member of this guild.\n\r",ch);
    return FALSE;
  }
  if(cmd==275) { /* train */
    number=atoi(arg)-1;
    if(number >= guild_skill_nums[guild_number-1] || number < 0 ) {
      send_to_char("There's no such skill\n\r",ch);
      return TRUE;
    }
    if((GET_GUILD_SKILL(ch,number))>=90) {
      send_to_char("You trained enough!!!\n\r",ch);
      return TRUE;
    }
    switch(guild_number) {
    case POLICE:
      cost=police_skill_costs[number]*((GET_GUILD_SKILL(ch,number)/10)+1)*GET_LEVEL(ch);
      sprintf(buf,"You train %s.\n\r",police_skills[number]);
      send_to_char(buf,ch);
      break;
    case OUTLAW:
      cost=outlaw_skill_costs[number]*((GET_GUILD_SKILL(ch,number)/10)+1)*GET_LEVEL(ch);
      sprintf(buf,"You train %s.\n\r",outlaw_skills[number]);
      send_to_char(buf,ch);
      break;
    case ASSASSIN:
      cost=assasin_skill_costs[number]*((GET_GUILD_SKILL(ch,number)/10)+1)*GET_LEVEL(ch);
      sprintf(buf,"You train %s.\n\r",assasin_skills[number]);
      send_to_char(buf,ch);
      break;
    default:
      log("shit in train");
      break;
    }
    if(GET_EXP(ch)>cost) {		
      GET_EXP(ch)=GET_EXP(ch)-cost;
      GET_GUILD_SKILL(ch,number)=MIN(GET_GUILD_SKILL(ch,number)+10,90);
    }
    else {
      sprintf(buf,"You need %d exp.\n\r",cost);
      send_to_char(buf,ch);
    }
    return TRUE;
  }
  else if (cmd==59) {/* list */
    if(guild_skill_nums[guild_number-1]<1) {
      send_to_char("There's no skill now\n\r",ch);
    }
    send_to_char("You can train skills by typing 'train #'\n\r",ch);
    switch(guild_number) {
    case POLICE:
      for(i=0; i < guild_skill_nums[guild_number-1]; i++) {
	sprintf(buf,"%d: %s %d (now %d%%)\n\r",
		i+1,
		police_skills[i],
		police_skill_costs[i] * ((GET_GUILD_SKILL(ch,i)/10)+1) * GET_LEVEL(ch),
		GET_GUILD_SKILL(ch,i));
	send_to_char(buf,ch);
      }
      break;
    case OUTLAW:
      for(i=0;i<guild_skill_nums[guild_number-1];i++) {
	sprintf(buf,"%d: %s %d (now %d%%)\n\r",i+1,outlaw_skills[i],
		outlaw_skill_costs[i]*((GET_GUILD_SKILL(ch,i)/10)+1)
		*GET_LEVEL(ch),
		GET_GUILD_SKILL(ch,i));
	send_to_char(buf,ch);
      }
      break;
    case ASSASSIN:
      for(i=0;i<guild_skill_nums[guild_number-1];i++) {
	sprintf(buf,"%d: %s %d (now %d%%)\n\r",i+1,assasin_skills[i],
		assasin_skill_costs[i]*((GET_GUILD_SKILL(ch,i)/10)+1)
		*GET_LEVEL(ch),
		GET_GUILD_SKILL(ch,i));
	send_to_char(buf,ch);
      }
      break;
    }
    return TRUE;
  }
  return FALSE;
}
