/* 
	Quest Management module(source file)
		made by atre@paradise.kaist.ac.kr at 1995/11/09
*/

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
#include <stdlib.h>

#include "structs.h"
#include "utils.h"
#include "db.h"

#define MaxQuest		10000
#define QUEST_FILE		"mob.quest"

int number(int from, int to);
void send_to_char_han(char *msgeng, char *msghan, struct char_data *ch);
void log(char *str);
void half_chop(char *string, char *arg1, char *arg2);
struct obj_data *get_obj_in_list_vis(struct char_data *ch, char *name,
  struct obj_data *list);
void send_to_char(char *messg, struct char_data *ch);
void extract_obj(struct obj_data *obj);
void obj_to_char(struct obj_data *o, struct char_data *ch);


struct {
  int virtual;
  int level;
  char *name;
} QM[MaxQuest];
int topQM;

int level_quest[45] = {
  0,
  0, 0, 0, 0, 0,
  0, 0, 0, 0, 0,	/* 10 */
  2, 3, 4, 5, 6,
  8, 10, 12, 14, 16,  /* 20 */
  18, 20, 22, 24, 26,
  29, 32, 35, 38, 41,  /* 30 */
  44, 47, 50, 53, 56,
  59, 62, 65, 70, 75, /* 40 */
  101, 110, 120, 150
};

extern struct index_data *mob_index;

#define ZONE_NUMBER 45
static struct {
  int number;
  char *name;
} zone_data[ZONE_NUMBER] = {
  { 99, "the LIMBO" },
  { 299, "the East Castle" },
  { 1399, "the Mel's Dog-House" },
  { 1499, "the Houses" },
  { 1599, "Dirk's Castle" },
  { 1799, "SHIRE" },
  { 1899, "The Jale" },
  { 1999, "The Lands" },
  { 2099, "Process' Castle" },
  { 2199, "The Wasteland" },
  { 2300, "Dragon Tower" },
  { 2399, "Muncie" },
  { 2699, "The Corporation" },
  { 2799, "The NeverLand" },
  { 2899, "Kingdom Of Chok" },
  { 2999, "The Keep Of MahnTor" },
  { 3099, "Northern Midgaard MainCity" },
  { 3199, "Southern Part Of Midgaard" },
  { 3299, "River Of Midgaard" },
  { 3499, "Graveyard" },
  { 4199, "Moria" },
  { 4330, "The Wamphyri Aerie" },
  { 5099, "The Great Eastern Desert" },
  { 5199, "Drow City" },
  { 5299, "The City Of Thalos" },
  { 6099, "HAON DOR LIGHT" },
  { 6499, "HAON DOR DARK" },
  { 6999, "The Dwarven Kingdom" },
  { 7099, "SEWER" },
  { 7199, "Second SEWER" },
  { 7399, "SEWER MAZE" },
  { 7899, "The Tunnels" },
  { 7999, "Redfernes Residence" },
  { 9099, "Arachnos" },
  { 9499, "Arena" },
  { 9699, "Death Kingdom" },
  { 9771, "Galaxy" },
  { 9851, "The Death Star" },
  { 12099, "Easy Zone" },
  { 13798, "Mount Olympus" },
  { 15299, "Robot City" },
  { 16199, "Kingdom Of Wee" },
  { 17099, "O Kingdom" },
  { 18099, "Moo Dang" },
  { 20000, "KAIST" }
};

char *find_zone(int number)
{
  int i;
  
  for(i = 0; i < ZONE_NUMBER; i ++){
    if(zone_data[i].number > number) return zone_data[i].name;
  }
  
  return NULL;
}

int get_quest(struct char_data *ch)
{
  static int width[8] = {
    0,			/* 0 */		/* 10 level */
    63,			/* 10 */	/* 16 level */
    129,		/* 20 */	/* 20 level */
    170,		/* 30 */	/* 24 level */
    188,		/* 40 */	/* 27 level */
    263,		/* 50 */	/* 30 level */
    304,		/* 60 */	/* 32 level */
  };
#define END_QUEST_MOBILE 538
  
  int low, high;
  int t;
  int num;
  
  if(GET_LEVEL(ch) == 40){
    low = 284;
    high = END_QUEST_MOBILE;
  }
  else if(ch->quest.solved >= 60){
    low = 267;		/* 34 level mobile */
    high = 348;		/* 38 level mobile */
  }
  else {
    t = ch->quest.solved / 10;
    low = width[t];
    high = width[t + 1];
  }
  
  do {
    num = number(low, high);
  } while(num == ch->quest.data);
  
  return num;
}

void do_request(struct char_data *ch, char *arg, int cmd)
{
  int num;
  char buf1[MAX_STRING_LENGTH], buf2[MAX_STRING_LENGTH];
  
  if(GET_LEVEL(ch) >= IMO){	/* IMO */
    send_to_char_han("QUEST : You can do something.\n\r",
		     "QUEST : 당신은 무엇이든 할 수 있습니다.\n\r", ch);
    return;
  }
  
  if(IS_MOB(ch)){				/* MOBILE */
    return;
  }
  
  /* request */
  if(ch->quest.type > 0){
    
	/*	if error occur, can do request.	*/
	if (ch->quest.data == NUL) {
		ch->quest.type = 0;
		return;
	}
  
    /* All remotal player cann't do request.	*/
    if((ch->player.level >= (IMO -1)) && (ch->player.remortal >= 15)){
      send_to_char_han("QUEST : You can't request.\n\r",
		       "QUEST : 다른 임무를 맡을 수 없습니다.\n\r", ch);
      return;
    }
    
    ch->quest.type = 0;
    if(ch->quest.solved > 0){
      (ch->quest.solved) --;
    }
    else {
      /* 단군의 request penalty */
      int xp = number(5000000, 10000000);
      
      if(GET_EXP(ch) > xp){
	GET_EXP(ch) -= xp;
      }
      else {
	send_to_char_han("QUEST : You can't request.\n\r",
			 "QUEST : 다른 임무를 맡을 수 없습니다.\n\r", ch);
	return;
      }
    }
  }

  if(ch->quest.type == 0){
    /* initialize quest */
    num = get_quest(ch);
    
    sprintf(buf1, "QUEST : QM proposes you should kill %s.\n", QM[num].name);
    sprintf(buf2, "QUEST : QM은 당신이 %s을 죽일 것을 제안합니다.\n", QM[num].name);
    send_to_char_han(buf1, buf2, ch);
    
    ch->quest.data = num;
    ch->quest.type = 1;
    
    return;
  }
  
  /* ch solved quest */
  if(ch->quest.type < 0){
    send_to_char_han("QUEST : Congratulations, You made! Go to QM.\n\r",
		     "QUEST : 성공했군요, 축하합니다. QM에게 가보세요.\n\r", ch);
    return;
  }
}

void do_hint(struct char_data *ch, char *arg, int cmd)
{
  int num;
  char buf1[MAX_STRING_LENGTH], buf2[MAX_STRING_LENGTH];
  char *zone;
  
  if(GET_LEVEL(ch) >= IMO){	/* IMO */
    send_to_char_han("QUEST : You can do something.\n\r",
		     "QUEST : 당신은 무엇이든 할 수 있습니다.\n\r", ch);
    return;
  }
  
  if(IS_MOB(ch)){				/* MOBILE */
    return;
  }
  
  /* not initialized */
  if(ch->quest.type == 0){
    send_to_char_han("QUEST : First, you should type quest.\n\r",
		     "QUEST : 먼저 quest라고 해 보세요.\n\r", ch);
    return;
  }
  
  /* ch solved quest */
  if(ch->quest.type < 0){
    send_to_char_han("QUEST : Congratulations, You made! Go to QM.\n\r",
		     "QUEST : 성공했군요, 축하합니다. QM에게 가보세요.\n\r", ch);
    return;
  }
  
  num = ch->quest.data;
  
  zone = find_zone(QM[num].virtual);
  if(!zone){
    sprintf(buf1, "QUEST : Where %s is, I don't know, either.\n", QM[num].name);
    sprintf(buf2, "QUEST : %s? 어디 있는 걸까? 모르겠는데...\n", QM[num].name);
    log("QUEST : INVALID mobile");
  }
  else {
    sprintf(buf1, "QUEST : %s is in %s probably.\n", QM[num].name, zone);
    sprintf(buf2, "QUEST : 아마도 %s는 %s에 있을 걸요.\n", QM[num].name, zone);
  }
  
  send_to_char_han(buf1, buf2, ch);
}

void do_quest(struct char_data *ch, char *arg, int cmd)
{
  int num;
  char buf1[MAX_STRING_LENGTH], buf2[MAX_STRING_LENGTH];
  
  if(GET_LEVEL(ch) >= IMO){	/* IMO */
    send_to_char_han("QUEST : You can do something.\n\r",
		     "QUEST : 당신은 무엇이든 할 수 있습니다.\n\r", ch);
    return;
  }
  
  if(IS_MOB(ch)){				/* MOBILE */
    return;
  }
  
  /* not initialized */
  if(ch->quest.type == 0){
    /* initialize quest */
    num = get_quest(ch);
    
    sprintf(buf1, "QUEST : QM proposes you should kill %s.\n", QM[num].name);
    sprintf(buf2, "QUEST : QM은 당신이 %s을 죽일 것을 제안합니다.\n", QM[num].name);
    send_to_char_han(buf1, buf2, ch);
    
    ch->quest.data = num;
    ch->quest.type = 1;
    
    return;
  }
  
  /* ch solved quest */
  if(ch->quest.type < 0){
    send_to_char_han("QUEST : Congratulations, You made! Go to QM.\n\r",
		     "QUEST : 성공했군요, 축하합니다. QM에게 가보세요.\n\r", ch);
    return;
  }
  
  num = ch->quest.data;
  
  sprintf(buf1, "QUEST : QM proposes you should kill %s.\n", QM[num].name);
  sprintf(buf2, "QUEST : QM은 당신이 %s을 죽일 것을 제안합니다.\n", QM[num].name);
  
  send_to_char_han(buf1, buf2, ch);
}

void init_quest(void)
{
  FILE *fp;
  char buf[MAX_STRING_LENGTH];
  int num, size;
  
  if(!(fp = fopen(QUEST_FILE, "r"))){
    log("init quest (quest_file)");
    exit(0);
  }
  
  topQM = 0;
  while(1){
    fscanf(fp, "%s", buf);
    if(buf[0] == '$'){	/* end of file */
      fclose(fp);
      return ;
    }
    
    num = atoi(buf);
    QM[topQM].virtual = num;
    
    fscanf(fp, " %d ", &(QM[topQM].level));
    
    fgets(buf, MAX_STRING_LENGTH - 1, fp);
    size = strlen(buf)+1;
    buf[size - 1] = 0;
    CREATE(QM[topQM].name, char, size);
    strcpy(QM[topQM].name, buf);
    
    topQM ++;
    
    if(topQM > MaxQuest){
      log("Quest Mobiles are overflow.");
      fclose(fp);
      return ;
    }
    
  }
}

int give_gift_for_quest(int level)
{
  static int gift_array[100] = {
    0, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 6,
    1, 1, 1, 1, 1, 1, 1, 1, 3, 3,
    1, 1, 1, 1, 1, 1, 1, 1, 5, 3,
    1, 1, 1, 1, 1, 3, 1, 1, 1, 3,
    1, 1, 1, 1, 1, 3, 1, 1, 1, 8,
    1, 1, 1, 1, 1, 1, 1, 1, 3, 7,
    1, 1, 1, 1, 1, 1, 1, 1, 8, 9,
    5, 5, 5, 5, 6, 6, 7, 6, 9, 1,
  };
  int num;
  
  num = number(level*3/2, 99);
  
  return(gift_array[num]);
}

/* 
   check ch's quest number with real mobile num
   */
void check_quest_mob_die(struct char_data *ch, int mob)
{
  int num;
  
  num = ch->quest.data;
  
  if(ch->quest.type < 1){
    return ;
  }
  
  num = real_mobile(QM[num].virtual);
  if(num == mob){
    ch->quest.type = -1;
    send_to_char_han("QUEST : Congratulations, You made! Go to QM.\n\r",
		     "QUEST : 성공했군요, 축하합니다. QM에게 가보세요.\n\r", ch);
  }
}

int quest_room(struct char_data *ch, int cmd, char *arg)
{
  int num;
  char buf[MAX_STRING_LENGTH];
  char buf1[MAX_STRING_LENGTH], buf2[MAX_STRING_LENGTH];
  struct obj_data *obj;
  static char *weapon[] = {
    "STAFF",
    "CLUB",
    "DAGGER",
    "SWORD"
  };
  static int weapon_type[] = {
    7,
    7,
    11,
    3
  };
  static char *wear[] = {
    "RING",
    "PENDANT",
    "ROBE",
    "HELMET",
    "LEGGINGS",
    "BOOTS",
    "GLOVES",
    "ARMPLATES",
    "SHIELD",
    "CLOAK",
    "BELT",
    "BRACELET",
    "MASCOT",
    "KNEEBAND",
    "PLATE"
  };
  static int wear_flags[] = {
    2,
    4,
    8,
    16,
    32,
    64,
    128,
    256,
    512,
    1024,
    2048,
    4096,
    16384,
    65536,
    131072
  };
  struct obj_data *tmp_obj;
  extern struct index_data *obj_index;
  
  /* quest or use  */
  if(cmd != 302 && cmd != 172){
    return FALSE;
  }
  
  if(cmd == 172){
    half_chop(arg, buf, buf2);
    if(*buf){
      if(strncmp("tickets", buf, strlen(buf))){
	/* not ticket */
	return FALSE;
      }
      
      tmp_obj = get_obj_in_list_vis(ch, buf, ch->carrying);
      if(tmp_obj){
	if((ch->player.level >= (IMO -1)) && (ch->player.remortal >= 15))
	  send_to_char("You can't use that ticket .\n\r", ch);
	else if(obj_index[tmp_obj->item_number].virtual == 7994){
	  ch->quest.type = 0;
	  do_quest(ch, arg, 302);
	  extract_obj(tmp_obj);
	}
	else {
	  send_to_char("You can't use that ticket here.\n\r", ch);
	}
      }
      else {
	send_to_char("You do not have that item.\n\r", ch);
      }
      
      return TRUE;
    }
    
    return FALSE;
  }
  
  if(GET_LEVEL(ch) >= IMO){   /* IMO */
    send_to_char_han("QUEST : You can do something.\n\r",
		     "QUEST : 당신은 무엇이든 할 수 있습니다.\n\r", ch);
    return TRUE;
  }
  
  if(ch->quest.type < 0){
    switch(give_gift_for_quest(GET_LEVEL(ch))){
    case 1  : /* some gold */
      GET_GOLD(ch) += number(100000, (500000 * GET_LEVEL(ch)) >> 2);
      send_to_char_han(
		       "QUEST : QM gives some coins for your success.\n\r",
		       "QUEST : 당신의 성공을 축하하며 QM이 돈을 줍니다.\n\r", ch);
      break;
    case 3  : /* request ticket */
      obj = read_object(7994, VIRTUAL);
      obj_to_char(obj, ch);
      send_to_char_han(
		       "QUEST : QM gives a ticket for request for your success.\n\r",
		       "QUEST : 당신의 성공을 축하하며 QM이 REQUEST용 티켓을 줍니다.\n\r", ch);
      break;
    case 5  : /* armor */
      obj = read_object(7998, VIRTUAL);
      
      /* wear flags */
      num = number(0, 14);
      obj->obj_flags.wear_flags = wear_flags[num] + 1;
      
      /* name */
      free(obj->name);
      sprintf(buf1, "%s QUEST ARMOR", wear[num]);
      CREATE(obj->name, char, strlen(buf1) + 1);
      strcpy(obj->name, buf1);
      
      /* short description */
      free(obj->short_description);
      sprintf(buf1, "%s's QUEST %s",
	      GET_NAME(ch), wear[num]);
      CREATE(obj->short_description, char, strlen(buf1) + 1);
      strcpy(obj->short_description, buf1);
      
      /* armor class */
      obj->obj_flags.value[0] = number(10,GET_LEVEL(ch) / 2);
      
      /* affected */
      obj->affected[0].location = 18;
      obj->affected[0].modifier = number(10,GET_LEVEL(ch) / 2);
      obj->affected[1].location = 19;
      obj->affected[1].modifier = number(10,GET_LEVEL(ch) / 2);
      
      obj_to_char(obj, ch);
      send_to_char_han(
		       "QUEST : QM gives an armor for your success.\n\r",
		       "QUEST : 당신의 성공을 축하하며 QM이 갑옷을 줍니다.\n\r", ch);
      break;
    case 6  : /* weapon */
      obj = read_object(7997, VIRTUAL);
      
      /* name */
      free(obj->name);
      sprintf(buf1, "%s QUEST WEAPON", weapon[GET_CLASS(ch) - 1]);
      CREATE(obj->name, char, strlen(buf1) + 1);
      strcpy(obj->name, buf1);
      
      /* short description */
      free(obj->short_description);
      sprintf(buf1, "%s's QUEST %s",
	      GET_NAME(ch), weapon[GET_CLASS(ch) - 1]);
      CREATE(obj->short_description, char, strlen(buf1) + 1);
      strcpy(obj->short_description, buf1);
      
      /* dice 
	 obj->obj_flags.value[1] = number(5+(GET_LEVEL(ch)>>3),5+(GET_LEVEL(ch)>>1));
	 obj->obj_flags.value[2] = number(5+(GET_LEVEL(ch)>>3),5+(GET_LEVEL(ch)>>1));
	 */
      /* dice */
      obj->obj_flags.value[1] = number(GET_LEVEL(ch)/2,GET_LEVEL(ch)/10);
      obj->obj_flags.value[2] = number(5+(GET_LEVEL(ch)>>3),5+(GET_LEVEL(ch)>>1));
      
      /* weapon type */
      obj->obj_flags.value[3] = weapon_type[GET_CLASS(ch) - 1];
      
      /* affected */
      obj->affected[0].location = 18;
      obj->affected[0].modifier = number(10,5+GET_LEVEL(ch) / 2);
      obj->affected[1].location = 19;
      obj->affected[1].modifier = number(10,5+GET_LEVEL(ch) / 2);
      
      obj_to_char(obj, ch);
      send_to_char_han(
		       "QUEST : QM gives a weapon for your success.\n\r",
		       "QUEST : 당신의 성공을 축하하며 QM이 무기를 줍니다.\n\r", ch);
      break;
    case 7  : /* ticket for AC */
      obj = read_object(7991, VIRTUAL);
      obj_to_char(obj, ch);
      send_to_char_han(
		       "QUEST : QM gives a ticket for meta for your success.\n\r",
		       "QUEST : 당신의 성공을 축하하며 QM이 메타용 티켓을 줍니다.\n\r", ch);
      break;
    case 8  : /* ticket for HR */
      obj = read_object(7992, VIRTUAL);
      obj_to_char(obj, ch);
      send_to_char_han(
		       "QUEST : QM gives a ticket for meta for your success.\n\r",
		       "QUEST : 당신의 성공을 축하하며 QM이 메타용 티켓을 줍니다.\n\r", ch);
      break;
    case 9  : /* ticket for DR */
      obj = read_object(7993, VIRTUAL);
      obj_to_char(obj, ch);
      send_to_char_han(
		       "QUEST : QM gives a ticket for meta for your success.\n\r",
		       "QUEST : 당신의 성공을 축하하며 QM이 메타용 티켓을 줍니다.\n\r", ch);
      break;
    case 0  :
    case 2  :
    case 4  :
    default :
      send_to_char_han(
		       "QUEST : Hmm. I can't find a gift for you this time.\n\r",
		       "QUEST : 음. 이번에는 줄만한 선물이 없네요.\n\r", ch);
    }
    ch->quest.type = 0;
    ch->quest.solved ++;
    return TRUE;
  }
  
  if(ch->quest.type == 0){
    /* initialize quest */
    num = get_quest(ch);
    
    sprintf(buf1, "QUEST : QM proposes you should kill %s.\n", QM[num].name);
    sprintf(buf2, "QUEST : QM은 당신이 %s을 죽일 것을 제안합니다.\n", QM[num].name);
    send_to_char_han(buf1, buf2, ch);
    
    ch->quest.type = 1;
    ch->quest.data = num;
    
    return TRUE;
  }
  
  /* not made */
  send_to_char_han("QUEST : Come to me, if you made your quest!\n\r",
		   "QUEST : quest를 마치고 제게 오세요!\n\r", ch);
  return TRUE;
}
