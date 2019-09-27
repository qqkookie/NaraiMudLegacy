
/* ************************************************************************
*  file: spec_procs.c , Special module.                   Part of DIKUMUD *
*  Usage: Procedures handling special procedures for object/room/mobile   *
*  Copyright (C) 1990, 1991 - see 'license.doc' for complete information. *
************************************************************************* */

#define FUDGE (100+dice(6,20))

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

/*   external vars  */

extern struct room_data *world;
extern struct char_data *character_list;
extern struct descriptor_data *descriptor_list;
extern struct index_data *obj_index;
extern struct time_info_data time_info;
extern struct title_type titles[4][IMO+4];
extern struct index_data *mob_index;

/* extern procedures */

void hit(struct char_data *ch, struct char_data *victim, int type);
void gain_exp(struct char_data *ch, int gain);
void advance_level(struct char_data *ch, int level_up);
void stop_fighting(struct char_data *ch);
void set_title(struct char_data *ch);
int MIN(int a, int b);
int number(int from, int to);
int dice(int num, int size);
void page_string(struct descriptor_data *d, char *str, int keep);
int do_simple_move(struct char_data *ch, int cmd, int following);
void do_flash(struct char_data *ch, char *arg, int cmd);
void do_cast(struct char_data *ch, char *arg, int cmd);
void do_shouryuken(struct char_data *ch, char *arg, int cmd);
void do_spin_bird_kick(struct char_data *ch, char *arg, int cmd);
void do_backstab(struct char_data *ch, char *arg, int cmd);
void do_punch(struct char_data *ch,char *argument,int cmd);
void do_bash(struct char_data *ch, char *arg, int cmd);
void do_light_move(struct char_data *ch,char *argument,int cmd);


void cast_sleep( byte level, struct char_data *ch, char *arg, int si,
   struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_cure_light(byte level,struct char_data *ch,char *arg,int type,
    struct char_data *victim, struct obj_data *tar_obj );
void cast_cure_critic(byte level,struct char_data *ch,char *arg,int type,
    struct char_data *victim, struct obj_data *tar_obj );
void cast_heal(byte level, struct char_data *ch, char *arg, int type,
    struct char_data *tar_ch, struct obj_data *tar_obj );
void cast_full_heal(byte level, struct char_data *ch, char *arg, int type,
    struct char_data *tar_ch, struct obj_data *tar_obj );
void cast_sunburst(byte level, struct char_data *ch, char *arg, int type,
    struct char_data *victim, struct obj_data *tar_obj );
void cast_fireball(byte level, struct char_data *ch, char *arg, int type,
    struct char_data *victim, struct obj_data *tar_obj );
void cast_color_spray(byte level, struct char_data *ch, char *arg, int type,
    struct char_data *victim, struct obj_data *tar_obj );

char *how_good(int p1, int p2)
{
  static char buf[64];

  sprintf(buf,"(%3d, %3d)",p1, p2);
  return (buf);
}

int guild(struct char_data *ch, int cmd, char *arg)
{
  char buf[MAX_STRING_LENGTH], buf2[MAX_STRING_LENGTH];
  char buf3[MAX_STRING_LENGTH*MAX_SKILLS];
  char tmp[MAX_STRING_LENGTH];
  int number,i,percent;
  int lev,cla;
  extern char *spells[];
  extern struct spell_info_type spell_info[MAX_SPL_LIST];
  extern struct int_app_type int_app[26];
  extern int level_quest[];

  /*for(i=0;i<MAX_STRING_LENGTH*MAX_SKILLS;i++) buf3[i]=NULL;*/
  strcpy(buf3,"");
  if ((cmd!=95)&&(cmd!=164)&&(cmd!=170)) return(FALSE);

  if(cmd==95){ /* advance */
    if (!IS_NPC(ch)) {
      for (i = 0; titles[GET_CLASS(ch)-1][i].exp <= GET_EXP(ch); i++) {
        if(i >= IMO){
          send_to_char_han("Immortality cannot be gained here.\n\r",
             "여기서는 신이 될 수 없습니다.\n\r",ch);
          return(TRUE);
        }
        if((i > GET_LEVEL(ch)) && (GET_QUEST_SOLVED(ch) >= level_quest[(int) GET_LEVEL(ch)])){
          send_to_char_han("You raise a level\n\r",
             "레벨을 올렸습니다\n\r", ch);
          GET_LEVEL(ch) = i; advance_level(ch, 1); set_title(ch);
          return(TRUE);
        }
      }
      send_to_char_han("You need more experience.\n\r",
         "경험이 더 필요합니다.\n\r",ch);
      return(TRUE);
    }
  }
  lev=GET_LEVEL(ch); cla=GET_CLASS(ch)-1;
  for(; *arg==' '; arg++);
  if (!*arg) {
    send_to_char_han("You can practice any of these skills:\n\r",
      "당신은 다음과 같은 기술을 익힐 수 있습니다:\n\r", ch);
	  /* Perhaps modified 
    for(i=0; *spells[i] != '\n'; i++){
      if(*spells[i] &&
          (spell_info[i+1].min_level[cla] <= lev)) {
        send_to_char(spells[i], ch);
        send_to_char(how_good(ch->skills[i+1].learned, ch->skills[i+1].skilled), ch);
        send_to_char("\n\r", ch);
      }
    }
	*/
    for(i=0; *spells[i] != '\n'; i++){
      if(*spells[i] &&
          (spell_info[i+1].min_level[cla] <= lev)) {
			sprintf(tmp,"%-20s %-4s\n\r",spells[i],how_good(ch->skills[i+1].learned, ch->skills[i+1].skilled));
			strcat(buf3,tmp);
	  }
	}
    sprintf(buf,"You have %d practices left.\n\r",
      ch->specials.spells_to_learn);
    sprintf(buf2,"지금 %d 번 기술을 연마(practice)할 수 있습니다. \n\r",
      ch->specials.spells_to_learn);
    send_to_char_han(buf,buf2,ch);
	page_string(ch->desc,buf3,0);
    return(TRUE);
  }
  number = old_search_block(arg,0,strlen(arg),spells,FALSE);
  if(number == -1) {
    send_to_char_han("You do not know of this spell...\n\r",
      "그런 기술은 모르는데요 ...\n\r", ch);
    return(TRUE);
  }
  if (lev < spell_info[number].min_level[cla]) {
    send_to_char_han("Your level is too low.\n\r",
      "아직은 레벨이 낮아 안됩니다...\n\r",ch);
    return(TRUE);
  }
  if (ch->specials.spells_to_learn <= 0) {
    send_to_char_han("You do not seem to be able to practice now.\n\r",
      "지금은 더이상 배울 수 없습니다.\n\r", ch);
    return(TRUE);
  }
  if(ch->skills[number].learned >= spell_info[number].max_skill[cla]){
    send_to_char_han("You know this area as well as possible.\n\r",
      "그 분야는 배울 수 있는만큼 배웠습니다.\n\r",ch);
    return(TRUE);
  }
  send_to_char_han("You Practice for a while...\n\r",
     "기술이 늘고 있습니다...\n\r", ch);
  ch->specials.spells_to_learn--;
  percent=ch->skills[number].learned+1+
          ((int)int_app[GET_INT(ch)].learn
          *(int)spell_info[number].max_skill[cla])/FUDGE;
  ch->skills[number].learned=
    MIN(spell_info[number].max_skill[cla],percent);
  if(ch->skills[number].learned >= spell_info[number].max_skill[cla]){
    send_to_char_han("You're now as proficient as possible.\n\r",
       "이 분야에는 이미 배울만큼 다 배웠습니다.\n\r",ch);
    return(TRUE);
  }
  return TRUE;
}


int dump(struct char_data *ch, int cmd, char *arg) 
{
   struct obj_data *k;
   char buf[100];
   struct char_data *tmp_char;
   int value=0;

  void do_drop(struct char_data *ch, char *argument, int cmd);
  char *fname(char *namelist);

  for(k = world[ch->in_room].contents; k ; k = world[ch->in_room].contents) {
    sprintf(buf, "The %s vanish in a puff of smoke.\n\r" ,fname(k->name));
    for(tmp_char = world[ch->in_room].people; tmp_char;
      tmp_char = tmp_char->next_in_room)
      if (CAN_SEE_OBJ(tmp_char, k))
        send_to_char(buf,tmp_char);
    extract_obj(k);
  }
  if(cmd!=60) return(FALSE);
  do_drop(ch, arg, cmd);
  value = 0;
  for(k=world[ch->in_room].contents; k ; k = world[ch->in_room].contents) {
    sprintf(buf, "The %s vanish in a puff of smoke.\n\r",fname(k->name));
    for(tmp_char = world[ch->in_room].people; tmp_char;
      tmp_char = tmp_char->next_in_room) {
      if (CAN_SEE_OBJ(tmp_char, k))
        send_to_char(buf,tmp_char);
    }
    value += k->obj_flags.cost ;
    extract_obj(k);
  }
  if (value > 0) 
  {
    acthan("You are awarded for outstanding performance.",
       "당신은 시장으로부터 선행상을 받았습니다.",  FALSE, ch, 0, 0, TO_CHAR);
    acthan("$n has been awarded for being a good citizen.",
       "$n님이 착한 시민상을 받았습니다", TRUE, ch, 0,0, TO_ROOM);

    ch->points.exp+= value/3;
  }
  return TRUE;
}

int mayor(struct char_data *ch, int cmd, char *arg)
{
  static char open_path[] =
    "W3a3003b33000c111d0d111Oe333333Oe22c222112212111a1S.";

  static char close_path[] =
    "W3a3003b33000c111d0d111CE333333CE22c222112212111a1S.";

  static char *path;
  static int index;
  static bool move = FALSE;

  void do_move(struct char_data *ch, char *argument, int cmd);
  void do_open(struct char_data *ch, char *argument, int cmd);
  void do_lock(struct char_data *ch, char *argument, int cmd);
  void do_unlock(struct char_data *ch, char *argument, int cmd);
  void do_close(struct char_data *ch, char *argument, int cmd);


  if (!move) {
    if (time_info.hours == 6) {
      move = TRUE;
      path = open_path;
      index = 0;
    } else if (time_info.hours == 20) {
      move = TRUE;
      path = close_path;
      index = 0;
    }
  }

  if (cmd || !move || (GET_POS(ch) < POSITION_SLEEPING) ||
    (GET_POS(ch) == POSITION_FIGHTING))
    return FALSE;

  switch (path[index]) {
    case '0' :
    case '1' :
    case '2' :
    case '3' :
      do_move(ch,"",path[index]-'0'+1);
      break;

    case 'W' :
      GET_POS(ch) = POSITION_STANDING;
      acthan("$n awakens and groans loudly.",
        "$n님이 일어나 기지개를 켜십니다", FALSE,ch,0,0,TO_ROOM);
      break;

    case 'S' :
      GET_POS(ch) = POSITION_SLEEPING;
      acthan("$n lies down and instantly falls asleep.",
        "$n님이 자리에 눕더니 금새 잠이 듭니다.", FALSE,ch,0,0,TO_ROOM);
      break;

    case 'a' :
      acthan("$n says 'Hello Honey!'",
         "$n님이 '안녕 내사랑!' 이라고 말합니다.", FALSE,ch,0,0,TO_ROOM);
      acthan("$n smirks.","$n님이 점잔빼며 웃습니다.", FALSE,ch,0,0,TO_ROOM);
      break;

    case 'b' :
      acthan("$n says 'What a view! I must get something done about dump!'",
        "$n님이 '하! 쓰레기에 대해서 뭔가 조치를 취해야 겠네!' 라고 말합니다",
        FALSE,ch,0,0,TO_ROOM);
      break;

    case 'c' :
      acthan("$n says 'Vandals! Youngsters nowadays have no respect for anything!'",
        "$n님이 '이런 이런! 젊은것들은 요즘 인사를 안한단말야!' 라고 말합니다.",
        FALSE,ch,0,0,TO_ROOM);
      break;

    case 'd' :
      acthan("$n says 'Good day, citizens!'",
        "$n님이 '시민 여러분 안녕들 하십니까!'라고 말합니다",  FALSE, ch, 0,0,TO_ROOM);
      break;

    case 'e' :
      acthan("$n says 'I hereby declare the bazaar open!'",
         "$n님이 '자 이제 시장을 열겠습니다 !' 라고 말합니다", FALSE,ch,0,0,TO_ROOM);
      break;

    case 'E' :
      acthan("$n says 'I hereby declare Midgaard closed!'",
         "$n님이 '자 이제 문을 닫았습니다!' 라고 말합니다", FALSE,ch,0,0,TO_ROOM);
      break;

    case 'O' :
      do_unlock(ch, "gate", 0);
      do_open(ch, "gate", 0);
      break;

    case 'C' :
      do_close(ch, "gate", 0);
      do_lock(ch, "gate", 0);
      break;

    case '.' :
      move = FALSE;
      break;
  }
  index++;
  return FALSE;
}

void npc_tornado(struct char_data *ch)
{
    struct char_data *tch;

    acthan("$n says 'Ya haa haa hap'.", "$n님이 '야하햐합' 하고 말합니다",
        1, ch, 0, 0, TO_ROOM);

    for (tch = world[ch->in_room].people; tch; tch = tch->next_in_room) {
        if (!IS_NPC(tch) && ch->points.move > 0) {
            if (ch->skills[SKILL_TORNADO].learned > number(1,99)) {
                hit(ch, tch, TYPE_UNDEFINED);
            }
        }
    }

    ch->points.move -= (IMO - GET_LEVEL(ch) + 2);
}

void npc_steal(struct char_data *ch,struct char_data *victim)
{
  int dir,gold;

  if(IS_NPC(victim)) return;
  if(GET_LEVEL(victim) >= IMO) return;
  if (AWAKE(victim) && (number(0,GET_LEVEL(ch)) == 0)) {
    acthan("You discover that $n has $s hands in your wallet.",
      "앗! $n님이 당신의 지갑에 손을 넣습니다.",
      FALSE,ch,0,victim,TO_VICT);
    acthan("$n tries to steal gold from $N.",
      "$n님이 $N님으로부터 돈을 훔치려고 합니다.",TRUE,ch,0,victim,TO_NOTVICT);
  } else {
    /* Steal some gold coins */
    acthan("$n suppresses a laugh.","$n님이 키득키득 웃습니다.(왜그렇까?)",
      TRUE,ch,0,0,TO_NOTVICT);
    gold =(int)((GET_GOLD(victim)*number(1,10))/25);
    if (gold > 0) {
      GET_GOLD(ch) += gold/2;
      GET_GOLD(victim) -= gold;
      dir=number(0,5);
      if(CAN_GO(ch,dir))
        do_simple_move(ch,dir,FALSE);
    }
  }
}

int snake(struct char_data *ch, int cmd, char *arg)
{
  void cast_poison( byte level, struct char_data *ch, char *arg, int type,
    struct char_data *tar_ch, struct obj_data *tar_obj );

  if(cmd) return FALSE;
  if(GET_POS(ch)!=POSITION_FIGHTING) return FALSE;
  if(ch->specials.fighting && 
    (ch->specials.fighting->in_room == ch->in_room) &&
    (number(0,32-GET_LEVEL(ch))==0))
    {
      acthan("$n bites $N!", "$n님이 $N님의 다리를 물었습니다!",
         1, ch, 0, ch->specials.fighting, TO_NOTVICT);
      acthan("$n bites you!", "$n님에게 다리를 물렸습니다!",
         1, ch, 0, ch->specials.fighting, TO_VICT);
      cast_poison( GET_LEVEL(ch), ch, "", SPELL_TYPE_SPELL,
         	ch->specials.fighting, 0);
      return TRUE;
    }
  return FALSE;
}

void first_attack(struct char_data *ch, struct char_data *victim)
{
	char buf[80], sbuf[100];
	int num;

	sprintf(buf, "%s", GET_NAME(victim));

	if(GET_LEVEL(ch) < 10){
		hit(ch, victim, 0);
		return;
	}

	num = number(GET_LEVEL(ch) / 2, GET_LEVEL(ch));
	switch(GET_CLASS(ch)){
		case CLASS_MAGIC_USER :
				if(number(0, 1)){
				  if(num > 33){
					sprintf(sbuf, " 'disintegrate' %s", buf);
				  }
				  else if(num > 30){
					sprintf(sbuf, " 'cone of ice' %s", buf);
				  }
				  else if(num > 26){
					sprintf(sbuf, " 'full fire' %s", buf);
				  }
				  else if(num > 20){
					sprintf(sbuf, " 'throw' %s", buf);
				  }
				  else if(num > 15){
					sprintf(sbuf, " 'energy drain' %s", buf);
				  }
				  else {
					sprintf(sbuf, " 'lightning bolt' %s", buf);
				  }
				  do_cast(ch, sbuf, 0);
				}
				else {
				  hit(ch, victim, 0);
				}
				break;
		case CLASS_CLERIC     :
				if(number(0, 1)){
				  if(num > 35){
					sprintf(sbuf, " 'throw' %s", buf);
				  }
				  else if(num > 25){
					sprintf(sbuf, " 'sunburst' %s", buf);
				  }
				  else if(num > 15){
					sprintf(sbuf, " 'harm' %s", buf);
				  }
				  else {
					sprintf(sbuf, " 'call lightning' %s", buf);
				  }
				  do_cast(ch, sbuf, 0);
				}
				else {
				  hit(ch, victim, 0);
				}
				break;
		case CLASS_THIEF      :
				if(num > 30) do_backstab(ch, buf, 0);
				else if(num > 20) do_flash(ch, buf, 0);
				else hit(ch, victim, 0);
				break;
		case CLASS_WARRIOR    :
				if(num > 30){
					if(GET_SEX(ch) > 1){	/* female */
						do_spin_bird_kick(ch, buf, 0);
					}
					else if(GET_SEX(ch) > 0){ /* male */
						do_shouryuken(ch, buf, 0);
					}
					else {
						do_punch(ch, buf, 0);
					}
				}
				else if(num > 20) {
					do_bash(ch, buf, 0);
				}
				else hit(ch, victim, 0);
	}
}

struct char_data *select_victim(struct char_data *ch)
{
	return ch->specials.fighting;
}

/* new mob act by atre */
/*
	warrior
*/
int warrior(struct char_data *ch, int cmd, char *arg)
{
	struct char_data *victim;
	/*
		157 : bash
		159 : kick
		240 : multi kick
		271 : punch
		269 : shouryuken
		298 : spin bird kick
	*/
	static int do_what_cmd[50] = {
			157, 159, 157, 159, 240,
			157, 159, 157, 159, 240,
			157, 159, 157, 159, 240,
			157, 240, 157, 240, 271,    /* level 20 */
			157, 240, 240, 271, 271,
			157, 240, 271, 269, 298,    /* level 30 */
			159, 240, 271, 269, 298,
			240, 271, 298, 269, 298,    /* level 40 */
			269, 298, 269, 298, 269,
			298, 269, 298, 269, 298     /* level 50 */
	};
	int do_what;
	extern struct command_info cmd_info[];

	if(cmd) return FALSE;

	if(GET_POS(ch)==POSITION_FIGHTING ) {
	  /* select victim */
	  victim = select_victim(ch);
	  if(!victim) return FALSE;

	  do_what = number(GET_LEVEL(ch) / 2, GET_LEVEL(ch));

	  if(cmd_info[do_what_cmd[do_what]].minimum_level[3] < GET_LEVEL(ch)){
		((*cmd_info[do_what_cmd[do_what]].command_pointer)(ch, GET_NAME(victim), 0));
		return TRUE;
	  }
	}

	return FALSE;
}

/*
	backstab
	lig
	flash
	steal
	tornado
*/
int thief(struct char_data *ch, int cmd, char *arg)
{
  struct char_data *victim, *cons;
  int do_what;

  if(cmd) return FALSE;
  if(GET_POS(ch)==POSITION_FIGHTING ) {
    victim = select_victim(ch);
    if(!victim) return FALSE;
    if(GET_LEVEL(ch) >= 30) do_what = number(1, 5);
    else do_what = number(1, 4);
    switch(do_what){
          case 1  : do_backstab(ch, GET_NAME(victim), 0); break;
          case 2  : do_light_move(ch, GET_NAME(victim), 0); break;
          case 3  : do_flash(ch, GET_NAME(victim), 0); break;
          case 4  : npc_tornado(ch);
          case 5  : do_punch(ch, GET_NAME(victim), 0); break;
    }
    return TRUE;
  }
  else {
    if(GET_LEVEL(ch) < 5) return FALSE;

    for(cons = world[ch->in_room].people; cons; cons = cons->next_in_room ){
      if((!IS_NPC(cons)) && (GET_LEVEL(cons)<IMO) &&
            (GET_LEVEL(cons)>=GET_LEVEL(ch)) && (number(1,3)==1)){
      	npc_steal(ch,cons); 
      	return TRUE;
      }
    }
  }
  return FALSE;
}

int cleric(struct char_data *ch, int cmd, char *arg)
{
  struct char_data *victim;
  char buf[80];

  if(cmd) return FALSE;
  if(GET_POS(ch)!=POSITION_FIGHTING){
    return FALSE;
  }

  victim = select_victim(ch);
  if(!victim) return FALSE;

  switch (number(1, GET_LEVEL(ch))) {
        case 0  :
        case 1  :
        case 2  :
        case 3  :
        case 4  :
        case 5  :
			sprintf(buf," 'magic missile' %s",GET_NAME(victim)); break;
        case 6  :
        case 7  :
        case 8  :
			sprintf(buf," 'chill touch' %s",GET_NAME(victim)); break;
        case 9  :
        case 10 :
			sprintf(buf," 'burning hands' %s",GET_NAME(victim)); break;
        case 11 :
        case 12 :
			sprintf(buf," 'earthquake' %s",GET_NAME(victim)); break;
        case 13 :
        case 14 :
			sprintf(buf," 'shocking grasp' %s",GET_NAME(victim)); break;
        case 15 :
        case 16 :
        case 17 :
			sprintf(buf," 'harm' %s",GET_NAME(victim)); break;
        case 18 :
        case 19 :
        case 20 :
			sprintf(buf," 'color spray' %s",GET_NAME(victim)); break;
        case 21 :
        case 22 :
        case 23 :
        case 24 :
			sprintf(buf," 'call lightning' %s",GET_NAME(victim)); break;
        case 25 :
        case 26 :
        case 27 :
        case 28 :
			sprintf(buf," 'fireball' %s",GET_NAME(victim)); break;
        case 29 :
        case 30 :
        case 31 :
        case 32 :
			sprintf(buf," 'firestorm' %s",GET_NAME(victim)); break;
        case 33 :
        case 34 :
        case 35 :
        case 36 :
			sprintf(buf," 'sunburst' %s",GET_NAME(victim)); break;
        case 37 :
        case 38 :
        case 39 :
		case 40 :
		case 41 :
		case 42 :
		case 43 :
			sprintf(buf," 'throw' %s",GET_NAME(victim)); break;
  }
  do_cast(ch,buf,84);

  return TRUE;
}

int magic_user(struct char_data *ch, int cmd, char *arg)
{
  struct char_data *victim;
  char buf[80];

  if(cmd) return FALSE;
  if(GET_POS(ch)!=POSITION_FIGHTING){
    return FALSE;
  }

  victim = select_victim(ch);
  if(!victim) return FALSE;

  switch (number(1, GET_LEVEL(ch))) {
    	case 1:
    	case 2:
			sprintf(buf," 'magic missile' %s",GET_NAME(victim)); break;
    	case 3:
    	case 4:
			sprintf(buf," 'chill touch' %s",GET_NAME(victim)); break;
    	case 5:
    	case 6:
			sprintf(buf," 'energy flow' %s",GET_NAME(victim)); break;
    	case 7:
    	case 8:
			sprintf(buf," 'burning hands' %s",GET_NAME(victim)); break;
    	case 9:
    	case 10:
    	case 11:
			sprintf(buf," 'shocking grasp' %s",GET_NAME(victim)); break;
    	case 12:
    	case 13:
			sprintf(buf," 'lightning bolt' %s",GET_NAME(victim)); break;
    	case 14:
    	case 15:
    	case 16:
			sprintf(buf," 'colour spray' %s",GET_NAME(victim)); break;
    	case 17:
    	case 18:
    	case 19:
    	case 20:
			sprintf(buf," 'fireball' %s",GET_NAME(victim)); break;
    	case 21:
    	case 22:
    	case 23:
			sprintf(buf," 'energy drain' %s",GET_NAME(victim)); break;
    	case 24:
    	case 25:
    	case 26:
			sprintf(buf," 'firestorm' %s",GET_NAME(victim)); break;
    	case 27:
    	case 28:
    	case 29:
    	case 30:
			sprintf(buf," 'cone of ice' %s",GET_NAME(victim)); break;
		case 31:
		case 32:
		case 33:
		case 34:
			sprintf(buf," 'throw' %s",GET_NAME(victim)); break;
		case 35:
		case 36:
		case 37:
		case 38:
			sprintf(buf," 'disintegrate' %s",GET_NAME(victim)); break;
		case 39:
		case 40:
		case 41:
		case 42:
		case 43:
			sprintf(buf," 'full fire' %s",GET_NAME(victim));
			break;
  }
  do_cast(ch,buf,84);

  return TRUE;
}

/* modified by atre */
#ifdef OldMobAct
int thief(struct char_data *ch, int cmd, char *arg)
{
  struct char_data *cons;

  if(cmd) return FALSE;
  if(GET_POS(ch)==POSITION_FIGHTING ) {
    if (number(1,4) > 1) {
      acthan("$n says 'Ya haa haa hap'.", "$n님이 '야하햐합' 하고 말합니다",
        1, ch, 0, 0, TO_ROOM);
      for(cons = world[ch->in_room].people; cons; cons = cons->next_in_room ) {
        if ( number(10,IMO+3) < GET_LEVEL(ch) && GET_LEVEL(cons) < IMO)
          if (!IS_NPC(cons) && ch!=cons)
            hit(ch,cons,TYPE_UNDEFINED) ;	/* It is tornado */
        }
      }
    else if ( GET_LEVEL(ch) > 10 &&(GET_HIT(ch) * 100/GET_PLAYER_MAX_HIT(ch))< 47) {
      acthan("$n says 'Quu ha cha cha'.", "$n님이 '크하차차' 하고 외칩니다",
        1, ch, 0, 0, TO_ROOM);
      do_flash(ch, "", 0);
      }
    }
  else if (GET_POS(ch)!=POSITION_STANDING) return FALSE;
  else {
    for(cons = world[ch->in_room].people; cons; cons = cons->next_in_room ) {
      if((!IS_NPC(cons)) && (GET_LEVEL(cons)<IMO) &&
        (GET_LEVEL(cons)>=GET_LEVEL(ch)) && (number(1,3)==1))
        npc_steal(ch,cons); 
      }
    }
  return TRUE;
}

int cleric(struct char_data *ch, int cmd, char *arg)
{
  struct char_data *vict;
  char buf[80];

  if(cmd) return FALSE;
  if(GET_POS(ch)!=POSITION_FIGHTING){
    return FALSE;
  }
  if(!ch->specials.fighting) return FALSE;
  for(vict=world[ch->in_room].people; vict; vict = vict->next_in_room )
    if(vict->specials.fighting==ch)
      break;
  if(!vict)
	return FALSE;
  switch (number(1,GET_LEVEL(ch))) {
    case 1:
		sprintf(buf," 'armor' %s",GET_NAME(ch));
		break;
    case 2:
		sprintf(buf," 'cure light' %s",GET_NAME(ch));
    case 3:
    case 4:
    case 5:
		sprintf(buf," 'bless' %s",GET_NAME(ch));
		break;
    case 6:
		sprintf(buf," 'blindness' %s",GET_NAME(vict));
		break;
    case 7:
    case 8:
		sprintf(buf," 'poison' %s",GET_NAME(ch));
		break;
    case 9:
		sprintf(buf," 'cure critic' %s",GET_NAME(ch));
		break;
    case 10:
		sprintf(buf," 'dispel evil' %s",GET_NAME(ch));
		break;
    case 11:
		sprintf(buf," 'remove poison' %s",GET_NAME(ch));
		break;
    case 12:
		sprintf(buf," 'remove curse' %s",GET_NAME(ch));
		break;
    case 13:
		sprintf(buf," 'sancuary' %s",GET_NAME(ch));
    case 14:
		sprintf(buf," 'heal' %s",GET_NAME(ch));
		break;
    case 15:
		sprintf(buf," 'call lightning' %s",GET_NAME(vict));
		break;
    case 16:
		sprintf(buf," 'harm' %s",GET_NAME(vict));
		break;
    case 17:
    case 18:
		sprintf(buf," 'love' %s",GET_NAME(ch));
		break;
    case 19:
		sprintf(buf," 'love' %s",GET_NAME(vict));
      	break;
    case 20:
		sprintf(buf," 'curse' %s",GET_NAME(vict));
		break;
    case 21:
		sprintf(buf," 'full heal' %s",GET_NAME(ch));
		break;
    case 22:
    case 23:
		sprintf(buf," 'haste' %s",GET_NAME(ch));
		break;
    case 24:
    case 25:
		sprintf(buf," 'sunburst' %s",GET_NAME(vict));
		break;
    case 26:
    case 27:
    case 28:
    case 29:
    case 30:
	case 31:
	case 32:
		sprintf(buf," 'full heal' %s",GET_NAME(vict));
		break;
	case 33:
	case 34:
		sprintf(buf," 'full heal' %s",GET_NAME(ch));
		break;
	case 35:
		sprintf(buf," 'throw' %s",GET_NAME(vict));
		break;
	case 36:
		sprintf(buf," 'full fire' %s",GET_NAME(vict));
		break;
	case 37:
	case 38:
	case 39:
	case 40:
	default:
		sprintf(buf," 'improved haste'");
		do_cast(ch,buf,84);
		sprintf(buf," 'mirror'");
		break;
  }
  do_cast(ch,buf,84);
  return TRUE;
}

int magic_user(struct char_data *ch, int cmd, char *arg)
{
  struct char_data *vict;
  char buf[80];

  if(cmd) return FALSE;
  if(GET_POS(ch)!=POSITION_FIGHTING){
    if(GET_LEVEL(ch) != 27)   /* Hypnos */
      return(FALSE);
    for(vict=world[ch->in_room].people; vict; vict = vict->next_in_room )
      if((!IS_NPC(vict))&&(GET_LEVEL(vict) < IMO)){
        act("$n utters the words 'Hypnos Thanatos'.",1,ch,0,0,TO_ROOM);
		cast_sleep(GET_LEVEL(ch), ch, "", SPELL_TYPE_SPELL, vict, 0);
        return(TRUE);
      }
    return FALSE;
  }
  if(!ch->specials.fighting) return FALSE;
  for(vict=world[ch->in_room].people; vict; vict = vict->next_in_room )
    if(vict->specials.fighting==ch )
      break;
  if (!vict)
    return FALSE;
  if (!ch)
	return FALSE;
  switch ( number(1,GET_LEVEL(ch)) ) {
    case 1:
		sprintf(buf," 'magic missile' %s",GET_NAME(vict));
		break;
    case 2:
    case 3:
		sprintf(buf," 'chill touch' %s",GET_NAME(vict));
		break;
    case 4:
		sprintf(buf," 'invisibility' %s",GET_NAME(ch));
		do_cast(ch,buf,84);
		sprintf(buf," 'energy flow' %s",GET_NAME(vict));
		break;
    case 5:
		sprintf(buf," 'armor' %s",GET_NAME(ch));
		do_cast(ch,buf,84);
		sprintf(buf," 'burning hands' %s",GET_NAME(vict));
		break;
    case 6:
    case 7:
		sprintf(buf," 'shocking grasp' %s",GET_NAME(vict));
		break;
    case 8:
		sprintf(buf," 'blindness' %s",GET_NAME(ch));
		break;
    case 9:
		sprintf(buf," 'lightning bolt' %s",GET_NAME(vict));
		do_cast(ch,buf,84);
		sprintf(buf," 'damage up'");
		break;
    case 10:
		sprintf(buf," 'self heal'");
		break;
    case 11:
		sprintf(buf," 'colour spray' %s",GET_NAME(vict));
		break;
    case 12:
		sprintf(buf," 'curse' %s",GET_NAME(vict));
		break;
    case 13:
		sprintf(buf," 'energy drain' %s",GET_NAME(vict));
		break;
    case 14:
		sprintf(buf," 'sleep' %s",GET_NAME(vict));
		break;
    case 15:
		sprintf(buf," 'fireball' %s",GET_NAME(vict));
		do_cast(ch,buf,84);
		sprintf(buf," 'mana boost' %s",GET_NAME(ch));
		break;
    case 16:
		sprintf(buf," 'mana boost' %s",GET_NAME(vict));
		break;
    case 17:
		sprintf(buf," 'vitalize' %s",GET_NAME(ch));
		do_cast(ch,buf,84);
		sprintf(buf," 'throw' %s",GET_NAME(vict));
		break;
    case 18:
    case 19:
		sprintf(buf," 'crush aromr' %s",GET_NAME(vict));
		break;
    case 20:
    case 21:
    case 22:
		sprintf(buf," 'haste' %s",GET_NAME(ch));
		break;
    case 23:
    case 24:
    case 25:
		sprintf(buf," 'improved haste'");
		break;
    case 26:
		sprintf(buf," 'full fire' %s",GET_NAME(vict));
		break;
    case 27:
    case 28:
    case 29:
    case 30:
		sprintf(buf," 'cone of ice' %s",GET_NAME(vict));
		break;
	case 31:
		sprintf(buf," 'vitalize' %s",GET_NAME(vict));
		break;
	case 32:
	case 33:
		sprintf(buf," 'disintegrate' %s",GET_NAME(vict));
		break;
	case 34:
	case 35:
		sprintf(buf," 'mirror image'");
		break;
	case 36:
	case 37:
	case 38:
	case 39:
	case 40:
		sprintf(buf," 'full heal' %s",GET_NAME(ch));
		break;
	default:
		sprintf(buf," 'disintegrate' %s",GET_NAME(vict));
		break;
  }
  do_cast(ch,buf,84);
  return TRUE;
}
#endif /* OldMobAct */

int paladin(struct char_data *ch, int cmd, char *arg)
{
  struct char_data *vict;

  void do_bash(struct char_data *ch,char *argument,int cmd);
  void do_multi_kick(struct char_data *ch,char *argument,int cmd);
  void do_kick(struct char_data *ch,char *argument,int cmd);
  void do_punch(struct char_data *ch,char *argument,int cmd);
  void do_light_move(struct char_data *ch,char *argument,int cmd);
  void do_disarm(struct char_data *ch,char *argument,int cmd);

  if(cmd) return FALSE;
  if(GET_POS(ch)!=POSITION_FIGHTING){
    return FALSE;
  }
  if(!ch->specials.fighting) return FALSE;
  for(vict=world[ch->in_room].people; vict; vict = vict->next_in_room )
    if(vict->specials.fighting==ch )
      break;
  if(!vict)
	return FALSE;
  switch (number(1,GET_LEVEL(ch))) {
    case 1:
    case 2:
    case 3:
		do_kick(ch,GET_NAME(vict),0);
		break;
    case 4:
    case 5:
    case 6:
    case 7:
		do_bash(ch,GET_NAME(vict),0);
		break;
    case 8:
    case 9:
    case 10:
    case 11:
    case 12:
    case 13:
      act("$n utters the words 'asdghjkasdgi'.", 1, ch, 0, 0, TO_ROOM);
      if(!IS_AFFECTED(ch,AFF_SPELL_BLOCK))
		cast_cure_light(GET_LEVEL(ch), ch, "", SPELL_TYPE_SPELL, ch, 0);
	else
		act("$n failed to cure light!!!",TRUE,ch,0,0,TO_ROOM);
      break;
    case 14:
    case 15:
    case 16:
		do_multi_kick(ch,GET_NAME(vict),0);
		break;
    case 17:
    case 18:
    case 19:
    case 20:
      act("$n utters the words 'saghkasdlghui'.",1,ch,0,0,TO_ROOM);
      if(!IS_AFFECTED(ch,AFF_SPELL_BLOCK))
		cast_cure_critic(GET_LEVEL(ch), ch,"",SPELL_TYPE_SPELL,ch,0);
	else
		act("$n failed to cure critic!!!",TRUE,ch,0,0,TO_ROOM);
      break;
    case 21:
    case 22:
    case 23:
		do_light_move(ch,"",0);
    case 24:
    case 25:
      act("$n utters the words 'heal'.", 1, ch, 0, 0, TO_ROOM);
      if(!IS_AFFECTED(ch,AFF_SPELL_BLOCK))
		cast_heal(GET_LEVEL(ch), ch, "", SPELL_TYPE_SPELL, ch, 0);
	else
		act("$n failed to heal!!!",TRUE,ch,0,0,TO_ROOM);
      break;
    case 26:
    case 27:
		do_punch(ch,GET_NAME(vict),0);
    case 28:
    case 29:
		break;
    case 30:
		do_flash(ch,GET_NAME(vict),0);
		break;
    case 31:
    case 32:
		break;
    case 33:
    case 34:
	case 35:
		do_disarm(ch,GET_NAME(vict),0);
		break;
	case 36:
	case 37:
		break;
	case 38:
		if (GET_SEX(ch) == SEX_MALE)
			do_shouryuken(ch,GET_NAME(vict),0);
		else
			do_spin_bird_kick(ch,GET_NAME(vict),0);
		break;
	case 39:
	case 40:
      act("$n utters the words 'sdagh'.", 1, ch, 0, 0, TO_ROOM);
      if(!IS_AFFECTED(ch,AFF_SPELL_BLOCK))
		cast_full_heal(GET_LEVEL(ch), ch, "", SPELL_TYPE_SPELL, ch, 0);
	else
		act("$n failed to full heal!!!",TRUE,ch,0,0,TO_ROOM);
      break;
	case 41:
	default:
		break;
  }
  return TRUE;
}

int dragon(struct char_data *ch, int cmd, char *arg)
{
  int mh;
  struct char_data *vict;
  void cast_fire_breath( byte level, struct char_data *ch, char *arg,int type,
    struct char_data *tar_ch, struct obj_data *tar_obj );
  void cast_frost_breath( byte level, struct char_data *ch, char *arg,int type,
    struct char_data *tar_ch, struct obj_data *tar_obj );
  void cast_gas_breath(byte level, struct char_data *ch,char *arg,int type,
    struct char_data *tar_ch, struct obj_data *tar_obj );
  void cast_lightning_breath(byte level,struct char_data *ch,char *arg,
   int type, struct char_data *tar_ch, struct obj_data *tar_obj );

  if(cmd) return FALSE;
  vict=ch->specials.fighting;
  if(!vict) return FALSE;
  mh=GET_HIT(vict);
  switch(number(1,4)){
   case 1:
    act("$n utters the words 'qassir plaffa'.", 1, ch, 0, 0, TO_ROOM);
	cast_fire_breath(GET_LEVEL(ch), ch, "", SPELL_TYPE_SPELL, vict, 0);
    if(mh < 400) return TRUE;
   case 2:
    act("$n utters the words 'qassir porolo'.", 1, ch, 0, 0, TO_ROOM);
	cast_gas_breath(GET_LEVEL(ch), ch, "", SPELL_TYPE_SPELL, vict, 0);
    if(mh < 400) return TRUE;
   case 3:
    act("$n utters the words 'qassir relata'.", 1, ch, 0, 0, TO_ROOM);
	cast_lightning_breath(GET_LEVEL(ch), ch, "", SPELL_TYPE_SPELL, vict, 0);
    if(mh < 400) return TRUE;
   case 4:
    act("$n utters the words 'qassir moolim'.", 1, ch, 0, 0, TO_ROOM);
	cast_frost_breath(GET_LEVEL(ch), ch, "", SPELL_TYPE_SPELL, vict, 0);
    return TRUE;
  }
  return TRUE;
}

