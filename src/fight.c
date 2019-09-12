/* ************************************************************************
*  File: fight.c , Combat module.                         Part of DIKUMUD *
*  Usage: Combat system and messages.                                     *
*  Copyright (C) 1990, 1991 - see 'license.doc' for complete information. *
************************************************************************* */

#include <stdio.h>
#include <string.h>
#include <assert.h>

#include "structs.h"
#include "utils.h"
#include "comm.h"
#include "handler.h"
#include "interpreter.h"
#include "db.h"
#include "spells.h"

#include "guild_list.h"

/* Structures */

struct char_data *combat_list = 0;     /* head of l-list of fighting chars */
struct char_data *combat_next_dude = 0; /* Next dude global trick           */


/* External structures */

extern struct room_data *world;
extern struct message_list fight_messages[MAX_MESSAGES];
extern struct obj_data  *object_list;
extern struct index_data *obj_index;
extern struct index_data *mob_index;

/* External procedures */

char *fread_string(FILE *f1);
void stop_follower(struct char_data *ch);
void do_flee(struct char_data *ch, char *argument, int cmd);
void hit(struct char_data *ch, struct char_data *victim, int type);
void wipe_stash(char *filename);
void log(char *str);
int number(int from, int to);
int MAX(int a, int b);
int MIN(int a, int b);
void gain_exp(struct char_data *ch, int gain);
int dice(int num, int size);
void save_char_nocon(struct char_data *ch, sh_int load_room );

/* quest */
void check_quest_mob_die(struct char_data *ch, int mob);


/* Weapon attack texts */
struct attack_hit_type attack_hit_text[] =
{
  {"hit",   "hits"},             /* TYPE_HIT      */
  {"pound", "pounds"},           /* TYPE_BLUDGEON */
  {"pierce", "pierces"},         /* TYPE_PIERCE   */
  {"slash", "slashes"},          /* TYPE_SLASH    */
  {"whip", "whips"},             /* TYPE_WHIP     */
  {"shoot", "shoots"},           /* TYPE_SHOOT    */
  {"bite", "bites"},             /* TYPE_BITE     */
  {"sting", "stings"},           /* TYPE_STING    */
  {"crush", "crushs"}           /* TYPE_CRUSH    */
};

struct attack_hit_type attack_hit_han[] =
{
  {"칩니다",   "칩니다"},             /* TYPE_HIT      */
  {"난타 합니다", "난타 합니다"},           /* TYPE_BLUDGEON */
  {"찌릅니다", "찌릅니다"},         /* TYPE_PIERCE   */
  {"벱니다", "벱니다"},          /* TYPE_SLASH    */
  {"채찍질 합니다", "채찍질 합니다"},             /* TYPE_WHIP     */
  {"쏩니다", "쏩니다"},           /* TYPE_SHOOT    */
  {"물어 뜯습니다", "물어 뜯습니다"},             /* TYPE_BITE     */
  {"톡 쏩니다", "톡 쏩니다"},           /* TYPE_STING    */
  {"부숩니다", "부숩니다"}           /* TYPE_CRUSH    */
};

/* The Fight related routines */
void appear(struct char_data *ch)
{
  act("$n slowly fade into existence.", FALSE, ch,0,0,TO_ROOM);
  if (affected_by_spell(ch, SPELL_INVISIBLE))
    affect_from_char(ch, SPELL_INVISIBLE);
  REMOVE_BIT(ch->specials.affected_by, AFF_INVISIBLE);
}

void load_messages(void)
{
  FILE *f1;
  int i,type;
  struct message_type *messages;
  char chk[100];
  
  if (!(f1 = fopen(MESS_FILE, "r"))){
    perror("read messages");
    exit(0);
  }
  
  for (i = 0; i < MAX_MESSAGES; i++) { 
    fight_messages[i].a_type = 0;
    fight_messages[i].number_of_attacks=0;
    fight_messages[i].msg = 0;
  }
  
  fscanf(f1, " %s \n", chk);
  
  while(*chk == 'M') {
    fscanf(f1," %d\n", &type);
    for (i = 0; (i < MAX_MESSAGES) && (fight_messages[i].a_type!=type) &&
	   (fight_messages[i].a_type); i++);
    if(i>=MAX_MESSAGES){
      log("Too many combat messages.");
      exit(0);
    }
    
    CREATE(messages,struct message_type,1);
    fight_messages[i].number_of_attacks++;
    fight_messages[i].a_type=type;
    messages->next=fight_messages[i].msg;
    fight_messages[i].msg=messages;
    messages->die_msg.attacker_msg      = fread_string(f1);
    messages->die_msg.victim_msg        = fread_string(f1);
    messages->die_msg.room_msg          = fread_string(f1);
    messages->miss_msg.attacker_msg     = fread_string(f1);
    messages->miss_msg.victim_msg       = fread_string(f1);
    messages->miss_msg.room_msg         = fread_string(f1);
    messages->hit_msg.attacker_msg      = fread_string(f1);
    messages->hit_msg.victim_msg        = fread_string(f1);
    messages->hit_msg.room_msg          = fread_string(f1);
    messages->god_msg.attacker_msg      = fread_string(f1);
    messages->god_msg.victim_msg        = fread_string(f1);
    messages->god_msg.room_msg          = fread_string(f1);
    fscanf(f1, " %s \n", chk);
  }
  fclose(f1);
}

void update_pos( struct char_data *victim )
{
  
  if ((GET_HIT(victim) > 0) && (GET_POS(victim) > POSITION_STUNNED)) return;
  else if (GET_HIT(victim) > 0 ) GET_POS(victim) = POSITION_STANDING;
  else if (GET_HIT(victim) <= -11) GET_POS(victim) = POSITION_DEAD;
  else if (GET_HIT(victim) <= -6) GET_POS(victim) = POSITION_MORTALLYW;
  else if (GET_HIT(victim) <= -3) GET_POS(victim) = POSITION_INCAP;
  else GET_POS(victim) = POSITION_STUNNED;
  
}


/* start one char fighting another (yes, it is horrible, I know... )  */
void set_fighting(struct char_data *ch, struct char_data *vict)
{
  /*
    assert(!ch->specials.fighting); */
  
  ch->next_fighting = combat_list;
  combat_list = ch;
  
  if(IS_AFFECTED(ch,AFF_SLEEP))
    affect_from_char(ch,SPELL_SLEEP);
  
  ch->specials.fighting = vict;
  GET_POS(ch) = POSITION_FIGHTING;
}



/* remove a char from the list of fighting chars */
void stop_fighting(struct char_data *ch)
{
  struct char_data *tmp;
  
  if (!ch) return;
  /*
    assert(ch->specials.fighting);
    */
  if (!ch->specials.fighting)
    return;
  
  if (ch == combat_next_dude)
  combat_next_dude = ch->next_fighting;
  
  if (combat_list == ch)
    combat_list = ch->next_fighting;
  else {
    for (tmp = combat_list; tmp && (tmp->next_fighting != ch); 
	 tmp = tmp->next_fighting);
    if (!tmp) {
      log("Char fighting not found Error (fight.c, stop_fighting)");
      /*
	abort();
	*/
      goto next;
    }
    tmp->next_fighting = ch->next_fighting;
  }
  
next:
  ch->next_fighting = 0;
  ch->specials.fighting = 0;
  GET_POS(ch) = POSITION_STANDING;
  update_pos(ch);
}

#define MAX_NPC_CORPSE_TIME 15
#define MAX_PC_CORPSE_TIME 40

void make_corpse(struct char_data *ch, int level)
{
  struct obj_data *corpse, *o;
  struct obj_data *otmp; 
  char buf[MAX_STRING_LENGTH];
  int i;
  
  struct obj_data *create_money( int amount );
  
  CREATE(corpse, struct obj_data, 1);
  clear_object(corpse);
  
  corpse->item_number = NOWHERE;
  corpse->in_room = NOWHERE;
  sprintf(buf, "corpse %s", 
	  (IS_NPC(ch) ? ch->player.short_descr : GET_NAME(ch)));
  corpse->name = strdup(buf);
  
  sprintf(buf, "Corpse of %s is lying here.", 
	  (IS_NPC(ch) ? ch->player.short_descr : GET_NAME(ch)));
  corpse->description = strdup(buf);
  
  sprintf(buf, "Corpse of %s",
	  (IS_NPC(ch) ? ch->player.short_descr : GET_NAME(ch)));
  corpse->short_description = strdup(buf);
  
  corpse->contains = ch->carrying;
  corpse->obj_flags.type_flag = ITEM_CONTAINER;
  corpse->obj_flags.wear_flags = ITEM_TAKE;
  corpse->obj_flags.value[0] = 0; /* You can't store stuff in a corpse */
  /* corpse->obj_flags.value[3] = 1; */ /* corpse identifyer */
  corpse->obj_flags.weight = GET_WEIGHT(ch)+IS_CARRYING_W(ch);
  
  /* norent */
  corpse->obj_flags.extra_flags = ITEM_NORENT;
  
  /* changed by jhpark */
  if (IS_NPC(ch))
    {
      corpse->obj_flags.timer = MAX_NPC_CORPSE_TIME;
      corpse->obj_flags.value[3]=1;  /* corpse identifyer : NPC */
    }
  else
    {
      corpse->obj_flags.timer = MAX_PC_CORPSE_TIME;
      corpse->obj_flags.value[3] = 2; /* corpse identifyer : PC */
    }
  
  for (i=0; i<MAX_WEAR; i++) {
    if (ch->equipment[i]) {
      /* 손오공 */
      if(mob_index[ch->nr].virtual == 11101 && i == WEAR_HEAD){
	otmp = unequip_char(ch,i);
	extract_obj(otmp);
      }
      else {
	otmp = unequip_char(ch,i);
	obj_to_obj(otmp, corpse);
      }
    }
  }
  
  ch->carrying = 0;
  IS_CARRYING_N(ch) = 0;
  IS_CARRYING_W(ch) = 0;
  
  corpse->next = object_list;
  object_list = corpse;
  
  for (o = corpse->contains; o; o->in_obj = corpse, o = o->next_content);
  object_list_new_owner(corpse, 0);
  
  /*
    삼장법사(11111) (put 금테(11127), 성수병(11134) Into corpse)
    */
  if(mob_index[ch->nr].virtual == 11111){
    o = read_object(11127, VIRTUAL);
    obj_to_obj(o, corpse);
    o = read_object(11134, VIRTUAL);
    obj_to_obj(o, corpse);
  }
  
  /* GoodBadIsland */
  /*
    IRON GOLEM(23323)
    */
  if(mob_index[ch->nr].virtual == 23323){
    act("거대한 철문이 열리고, 위층으로 올라가는 계단이 보입니다.",
	FALSE, ch, 0, 0, TO_ROOM);
    REMOVE_BIT(EXIT(ch, 4)->exit_info, EX_LOCKED);
    REMOVE_BIT(EXIT(ch, 4)->exit_info, EX_CLOSED);
  }
  
  /*
    KAALM(23301)
    */
  if(mob_index[ch->nr].virtual == 23301){
    o = read_object(23309, VIRTUAL);
    obj_to_obj(o, corpse);
  }
  
  obj_to_room(corpse, ch->in_room);
  
  /* ch is freed in extact_char */
  /*
    if (IS_NPC(ch) && ch != NULL) {
    free(ch);
    ch = NULL;
    }
    */
}

/* When ch kills victim */
void change_alignment(struct char_data *ch, struct char_data *victim)
{
  int al, al_v;
  
  /*
    al=(7*GET_ALIGNMENT(ch)-GET_ALIGNMENT(victim))/8;
    al=(al-GET_ALIGNMENT(ch))/10;
    */
  al_v = -GET_ALIGNMENT(victim);
  al = al_v / 200;
  if(al_v > 0){
    al += number(1, 3);
  }
  else {
    al -= number(1, 3);
  }
  
  GET_ALIGNMENT(ch)+=al;
  if(GET_ALIGNMENT(ch) < -1000) GET_ALIGNMENT(ch) = -1000;
  if(GET_ALIGNMENT(ch) >  1000) GET_ALIGNMENT(ch) =  1000;
}

void death_cry(struct char_data *ch)
{
  int door, was_in;
  
  acthan("Your blood freezes as you hear $ns death cry.",
	 "$n님의 처절한 비명소리가 들립니다. 으 소름끼쳐 ~~",  FALSE, ch,0,0,TO_ROOM);
  was_in = ch->in_room;
  
  for (door = 0; door <= 5; door++) {
    if (CAN_GO(ch, door)) {
      ch->in_room = world[was_in].dir_option[door]->to_room;
      acthan("Your blood freezes as you hear someones death cry.",
	     "어디선가 소름끼치는 비명소리가 들려옵니다.", FALSE,ch,0,0,TO_ROOM);
      ch->in_room = was_in;
    }
  }
}

void raw_kill(struct char_data *ch, int level)
{
  if (ch->specials.fighting)
    stop_fighting(ch);
  
  death_cry(ch);
  make_corpse(ch, level);
  extract_char(ch);
}

void die(struct char_data *ch, int level,struct char_data *who)
{
  struct affected_type *af;
  int exp;
  
  if (!ch) return;
  /* chase modified this for reraise */
  if (!IS_NPC(ch) && IS_AFFECTED(ch, AFF_RERAISE)) {
    GET_POS(ch) = POSITION_STANDING;
    act("$n is RERAISED by Almighty Power!",TRUE,ch,0,0,TO_ROOM);
    act("God blesses you with Almighty Power!",TRUE,ch,0,0,TO_CHAR);
    if(ch->specials.fighting)
      stop_fighting(ch);
    GET_HIT(ch)=GET_PLAYER_MAX_HIT(ch);
    GET_MANA(ch)=MAX(50,GET_MANA(ch));
    GET_MOVE(ch)=MAX(50,GET_MOVE(ch));
    if ( number(1,5)!=4 ) {
      GET_ALIGNMENT(ch) = MIN(-1000,GET_ALIGNMENT(ch)-200);
      act("You feel sacred!",TRUE,ch,0,0,TO_CHAR);
    }
    else {
      GET_ALIGNMENT(ch) = MAX(GET_ALIGNMENT(ch)+200,1000);
      act("You feel peaceful!",TRUE,ch,0,0,TO_CHAR);
    }
  }
  /* to forbid 2 calls, by ares */
  else if (!IS_NPC(ch) && IS_AFFECTED(ch, AFF_DEATH)) {
    for (af = ch->affected; af; af = af->next)
      if (af->type == SPELL_DEATH)
	break;
    affect_remove(ch, af);
  }
  else {
    if (!IS_NPC(ch)) {
      wipe_stash(GET_NAME(ch));
      GET_GOLD(ch) = 0;
      save_char(ch, ch->in_room);
      /* killed number(no only pk) */
      /*
	if(who && !IS_NPC(who))
	*/
      ch->player.pked_num++;
      /*
	GET_PLAYER_MAX_HIT(ch) -= GET_LEVEL(ch);
	GET_PLAYER_MAX_MANA(ch) -= GET_LEVEL(ch);
	GET_PLAYER_MAX_MOVE(ch) -= GET_LEVEL(ch);
	*/
      /*
	if (IS_SET(ch->specials.act, PLR_CRIMINAL))
	REMOVE_BIT(ch->specials.act, PLR_CRIMINAL);
	home = 0;
	switch (GET_GUILD(ch)) {
	case POLICE:
	home = real_room(ROOM_GUILD_POLICE_LOCKER);
	break;
	case OUTLAW:
	home = real_room(ROOM_GUILD_OUTLAW_LOCKER);
	break;
	case ASSASSIN:
	home = real_room(ROOM_GUILD_ASSASSIN_LOCKER);
	break;
	}
	if (home != 0 && who) {
	if (!IS_NPC(who))
	{
	if (ch->in_room != NOWHERE)
	char_from_room(ch);
	char_to_room(ch,home);
	act("$n appears in the room BLEEDING!!!",
	TRUE,ch,0,0,TO_ROOM);
	}
	}
	*/
    }
    /*
      gain_exp(ch, -(GET_EXP(ch)*1/10));
      */
    exp = GET_LEVEL(ch)*GET_LEVEL(ch)*level*200;
    gain_exp(ch, -exp);
    /* For coin copy bug , fixed by dsshin */
    
    if(!IS_NPC(ch)) {
      save_char_nocon(ch, world[ch->in_room].number);
    }
    
    /***************************************/

    raw_kill(ch, level);
  }
}

void group_gain(struct char_data *ch, struct char_data *victim)
{
  char buf[256], buf2[BUFSIZ];
  unsigned no_members, share, money;
  struct char_data *k;
  struct follow_type *f;
  unsigned high_level;
  unsigned total_level;
  unsigned level_exp;
  
  if (!(k = ch->master))
    k = ch;
  
  if (IS_AFFECTED(k, AFF_GROUP) && (k->in_room == ch->in_room))
    no_members = 1;
  else
    no_members = 0;
  
  high_level = GET_LEVEL(k);
  total_level = GET_LEVEL(k);
  
  /* find highest level in same room */
  for (f = k->followers; f; f = f->next)
    if (IS_AFFECTED(f->follower, AFF_GROUP) &&
	(f->follower->in_room == ch->in_room))
      high_level = MAX(high_level, GET_LEVEL(f->follower));
  
  /* calculate total member, total level */
  for (f = k->followers; f; f = f->next)
    if (IS_AFFECTED(f->follower, AFF_GROUP) &&
	!IS_NPC(f->follower) &&
	(f->follower->in_room == ch->in_room)) {
      no_members++;
      total_level += GET_LEVEL(f->follower);
    }
  
  /* assert(no_members) for divide by zero */
  no_members = MAX(1, no_members);
  
  /*
    money = GET_GOLD(victim) * ((no_members >> 1) + 1);
    money += (GET_LEVEL(victim) * GET_LEVEL(victim) * GET_LEVEL(victim));
    money /= no_members;
    level_exp = GET_EXP(victim) * ((no_members >> 1) + 1) / total_level;	*/
  
  /* group advantage */
  money = GET_GOLD(victim) * ((no_members >> 1) + 1);
  money += (GET_LEVEL(victim) * GET_LEVEL(victim) * GET_LEVEL(victim));
  money /= ( no_members * no_members );
  level_exp = GET_EXP(victim) * ((no_members >> 1) + 1) / total_level;
  
  if (IS_AFFECTED(k, AFF_GROUP) && (k->in_room == ch->in_room)) {
    share = level_exp * GET_LEVEL(k);
    sprintf(buf, "You receive %d experience and %d gold coins.",
	    share, money);
    sprintf(buf2, "당신은 %d 점의 경험치와 %d의 금을 얻었습니다.",
	    share, money);
    acthan(buf, buf2, FALSE, k, 0, 0, TO_CHAR);
    if (!IS_NPC(k)) {
      gain_exp(k, share); /* Perhaps modified for mob's exp*/
      GET_GOLD(k) += money;
    }
    change_alignment(k, victim);
  }
  
  /* followers gain exp and gold */
  for (f = k->followers; f; f = f->next) {
    if (IS_AFFECTED(f->follower, AFF_GROUP) &&
	f->follower->in_room == ch->in_room) {
      share = level_exp * GET_LEVEL(f->follower);
      sprintf(buf, "You receive %d experience and %d gold coins.",
	      share, money);
      sprintf(buf2, "당신은 %d 점의 경험치와 %d의 금을 얻었습니다.",
	      share, money);
      acthan(buf, buf2, FALSE, f->follower, 0, 0, TO_CHAR);
      if(!IS_NPC(f->follower)) {
	gain_exp(f->follower, share); /* Perhaps modified */
	GET_GOLD(f->follower) += money;
      }
      change_alignment(f->follower, victim);
    }
  }
}

char *replace_string(char *str, char *weapon)
{
  static char buf[3][256];
  static int count = 0;
  char *rtn;
  char *cp;
  
  cp = rtn = buf[count % 3];
  count ++ ;
  
  for (; *str; str++) {
    if (*str == '#') {
      switch(*(++str)) {
    case 'W' : 
      for (; *weapon; *(cp++) = *(weapon++));
      break;
      default :
	*(cp++) = '#';
	break;
      }
    } else {
      *(cp++) = *str;
    }
    *cp = 0;
  } /* For */
  return(rtn);
}

struct dam_weapon_type {
  char *to_room;
  char *to_char;
  char *to_victim;
} ;

void dam_message(int dam, struct char_data *ch, struct char_data *victim,
                 int w_type)
{
  struct obj_data *wield;
  char *buf, *buf2;
  int  msg_index ;

#ifdef UNUSED_CODE 
  static struct dam_weapon_type dam_weapons[] = {
    
    {"$n misses $N with $s #W.",                           /*    0    */
     "You miss $N with your #W.",
     "$n misses you with $s #W." },
    
    {"$n tickles $N with $s #W.",                          /*  1.. 4  */
     "You tickle $N as you #W $M.",
     "$n tickle you as $e #W you." },
    
    {"$n barely #W $N.",                                   /*  5.. 8  */
     "You barely #W $N.",
     "$n barely #W you."},
    
    {"$n #W $N.",                                          /*  9.. 12  */
     "You #W $N.",
     "$n #W you."}, 
    
    {"$n #W $N hard.",                                     /*  13..20  */
     "You #W $N hard.",
     "$n #W you hard."},
    
    {"$n #W $N very hard.",                                /* 21..30  */
     "You #W $N very hard.",
     "$n #W you very hard."},
    
    {"$n #W $N extremely hard.",                          /* 31..40  */
     "You #W $N extremely hard.",
     "$n #W you extremely hard."},
    
    {"$n massacre $N to small fragments with $s #W.",     /* > 40    */
     "You massacre $N to small fragments with your #W.",
     "$n massacre you to small fragments with $s #W."},
    
    {"$n annihilates $N to micro molecules with $s #W.",   /* > 65    */
     "You annihilate $N to micro molecules with your #W.",
     "$n annihilates you to micro molecules with $s #W."},
    
    { "$n disintegrates $N with $s #W.",                    /* > 110 */
      "You disintegrate $N with your #W.",
      "$n disintegrates you with $s #W." },
    
    { "$n sends $N void with $s #W.",  /* > 150 */
      "You send $N to void with your #W.",
      "$n sends you to void with $s #W." },
    
    { "$n tickles $N void with $s #W.",  /* > 200 */
      "You tickle $N to void with your #W.",
      "$n tickles you to void with $s #W." },
    
    /* Furfuri made massage */
    
    { "$n shoots $N with laser power $s #W.",  /* > 250 */
      "You shoot $N with laser power #W.",
      "$n shoots you with laser power $s #W." },
    
    { "$n strikes $N with NUCLEAR power $s #W.",
      "You strike $N with NUCLEAR power #W.",
      "$n strikes you with NUCLEAR power $s #W."},
    
    { "$n CRASHES $N with -= S P A C E   P O W E R =- $s #W.",  
      "YOU CRASH $N with -= S P A C E   P O W E R =- #W.",
      "$n CRASHES YOU with -= S P A C E   P O W E R =- $s #W." },
    
    { "$n PROCESSES $N into ---==<< V A C U U M >>==--- with $s #W.",
      "YOU PROCESS $N into ---==<< V A C U U M >>==--- with your #W.",
      "$n PROCESSES YOU into ---==<< V A C U U M >>==--- with $s #W." }
  };
#endif // UNUSED_CODE
  
  /* brief mode */
  static struct dam_weapon_type brief_dam_weapons[] = {
    {"$n -miss- $N.",                           /*    0    */
     "You -miss- $N.",
     "$n -miss- you." },
    
    {"$n -tickle- $N.",                          /*  1.. 4  */
     "You -tickle- $N.",
     "$n -tickle- you." },
    
    {"$n -barely- $N.",                                   /*  5.. 8  */
     "You -barely- $N.",
     "$n -barely- you."},
    
    {"$n -hit- $N.",                                          /*  9.. 12  */
     "You -hit- $N.",
     "$n -hit- you."}, 
    
    {"$n -hard- $N.",                                     /*  13..20  */
     "You -hard- $N.",
     "$n -hard- you."},
    
    {"$n -very hard- $N.",                                /* 21..30  */
     "You -very hard- $N.",
     "$n -very hard- you."},
    
    {"$n -extremely hard- $N.",                          /* 31..40  */
     "You -extremely hard- $N.",
     "$n -extremely hard- you."},
    
    {"$n -massacre- $N.",     /* > 40    */
     "You -massacre- $N.",
     "$n -massacre- you."},
    
    {"$n -annihilate- $N.",   /* > 65    */
     "You -annihilate- $N.",
     "$n -annihilate- you."},
    
    { "$n -disintegrate- $N.",                    /* > 110 */
      "You -disintegrate- $N.",
      "$n -disintegrate- you." },
    
    { "$n -send void- $N.",  /* > 150 */
      "You -send void- $N.",
      "$n -send void- you." },
    
    { "$n -tickle void- $N.",  /* > 200 */
      "You -tickle void- $N.",
      "$n -tickle void- you." },
    
    /* Furfuri made massage */
    
    { "$n -= laser power =- $N.",  /* > 250 */
      "You -= laser power =- $N.",
      "$n -= laser power =- you." },
    
    { "$n -= NUCLEAR power =- $N.",
      "You -= NUCLEAR power =- $N.",
      "$n -= NUCLEAR power =- YOU."},
    
    { "$n -= S P A C E   P O W E R =- $N.",  
      "YOU -= S P A C E   P O W E R =- $N.",
      "$n -= S P A C E   P O W E R =- YOU." },
    
    { "$n ---==<< V A C U U M >>==--- $N.",
      "YOU ---==<< V A C U U M >>==--- $N.",
      "$n ---==<< V A C U U M >>==--- YOU." }
  };

#ifdef UNUSED_CODE 
  static struct dam_weapon_type han_weapons[] = {
    {"$n 님이 $N 님을 때리려다 빗나갔습니다.",        /*    0    */
     "당신은 $N 님을 때리지 못했습니다.",
     "$n 님이 당신을 때리려다 실패했습니다." },
    
    {"$n 님이 $N 님을 간지럽게 #W.",                          /*  1.. 4  */
     "당신은 $N 님을 간지럽게 #W.",
     "$n 님이 당신을 간지럽게 #W." },
    
    {"$n 님이 $N 님을 살살 #W.",                                   /*  5.. 8  */
     "당신은 $N 님을 살살 #W.",
     "$n 님이 당신을 살살 #W."},
    
    {"$n 님이 $N 님을 #W.",                                       /*  9.. 12  */
     "당신은 $N 님을 #W.",
     "$n 님이 당신을 #W."}, 
    
    {"$n 님이 $N 님을 세게 #W.",                                /*  13..20  */
     "당신은 $N 님을 세게 #W.",
     "$n 님이 당신을 세게 #W."},
    
    {"$n 님이 $N 님을 매우 세게 #W.",                                /* 21..30  */
     "당신은 $N 님을 매우 세게 #W.",
     "$n 님이 당신을 매우 세게 #W."},
    
    {"$n 님이 $N 님을 무지막지하게 #W.",                          /* 31..40  */
     "당신은 $N 님을 무지막지하게 #W.",
     "$n 님이 당신을 무지막지하게 #W."},
    
    {"$n 님이 $N 님을 뼈가 으스러지게 #W.",     /* > 40    */
     "당신은 $N 님을 뼈가 으스러지게 #W.",
     "$n 님이 당신을 뼈가 으스러지게 #W."},
    
    {"$n 님이 $N 님을 몸이 산산조각이 나게 #W.",     /* > 65    */
     "당신은 $N 님을 몸이 산산조각이 나게 #W.",
     "$n 님이 당신을 몸이 산산조각이 나게 #W."},
    
    {"$n 님이 $N 님을 몸이 가루가 되게 #W.",     /* > 110    */
     "당신은 $N 님을 몸이 가루가 되게 #W.",
     "$n 님이 당신을 몸이 가루가 되게 #W."},
    
    {"$n 님이 $N 님을 눈앞에 별이 돌게 #W.",       /* > 150 */
     "당신은 $N 님을 눈앞에 별이 돌게 #W.",
     "$n 님이 당신을 눈앞에 별이 돌게 #W." },
    
    {"$n 님이 $N 님을 눈앞에 병아리가 돌게 #W.",       /* > 200 */
     "당신은 $N 님을 눈앞에 병아리가 돌게 #W.",
     "$n 님이 당신을 눈앞에 병아리가 돌게 #W." },
    
    {"$n 님이 $N 님을 소리없이 #W.",       /* > 250 */
     "당신은 $N 님을 소리없이 #W.",
     "$n 님이 당신을 소리없이 #W." },
    
    {"$n 님이 $N 님을 레이져 파워로 #W.",
     "당신은 $N 님을 레이져 파워로 #W.",
     "$n 님이 당신을 레이져 파워로 #W."},
    
    {"$n 님이 $N 님을 전..우주의 힘을 빌어 #W.",
     "당신은 $N 님을 전 ..우주의 힘을 빌어 #W.",
     "$n 님이 당신을 전 ..우주의 힘을 빌어 #W."},
    
    {"$n 님이 $N 님을 진공상태로 되도록 #W.",
     "당신은 $N 님을 진공상태로 되도록 #W.",
     "$n 님이 당신을 진공상태가 되도록 #W."} 
  };
#endif  // UNUSED_CODE

  /* brief mode */
  static struct dam_weapon_type brief_han_weapons[] = {
    {"$n, $N 님을 못침.",        /*    0    */
     "당신, $N 님을 못침.",
     "$n, 당신을 못침." },

    {"$n, $N 님을 간지럽게.",                          /*  1.. 4  */
     "당신, $N 님을 간지럽게.",
     "$n, 당신을 간지럽게." },
    
    {"$n, $N 님을 살살.",                                   /*  5.. 8  */
     "당신, 님을 살살.",
     "$n, 당신을 살살."},
    
    {"$n, $N 님을 침.",                                       /*  9.. 12  */
     "당신, $N 님을 침.",
     "$n, 당신을 침."}, 
    
    {"$n, $N 님을 세게.",                                /*  13..20  */
     "당신, $N 님을 세게.",
     "$n, 당신을 세게."},
    
    {"$n, $N 님을 매우 세게.",                                /* 21..30  */
     "당신, $N 님을 매우 세게.",
     "$n, 당신을 매우 세게."},
    
    {"$n, $N 님을 무지막지하게.",                          /* 31..40  */
     "당신, $N 님을 무지막지하게.",
     "$n, 당신을 무지막지하게."},
    
    {"$n, $N 님을 뼈가 으스러지게.",     /* > 40    */
     "당신, $N 님을 뼈가 으스러지게.",
     "$n, 당신을 뼈가 으스러지게."},
    
    {"$n, $N 님을 몸이 산산조각이 나게.",     /* > 65    */
     "당신, $N 님을 몸이 산산조각이 나게.",
     "$n, 당신을 몸이 산산조각이 나게."},
    
    {"$n, $N 님을 몸이 가루가 되게.",     /* > 110    */
     "당신, $N 님을 몸이 가루가 되게.",
     "$n, 당신을 몸이 가루가 되게."},
    
    {"$n, $N 님을 눈앞에 별이 돌게.",       /* > 150 */
     "당신, $N 님을 눈앞에 별이 돌게.",
     "$n, 당신을 눈앞에 별이 돌게." },
    
    {"$n, $N 님을 눈앞에 병아리가 돌게.",       /* > 200 */
     "당신, $N 님을 눈앞에 병아리가 돌게.",
     "$n, 당신을 눈앞에 병아리가 돌게." },
    
    {"$n, $N 님을 소리없이.",       /* > 250 */
     "당신, $N 님을 소리없이.",
     "$n, 당신을 소리없이." },
    
    {"$n, $N 님을 레이져 파워로.",
     "당신, $N 님을 레이져 파워로.",
     "$n, 당신을 레이져 파워로."},
    
    {"$n, $N 님을 전..우주의 힘을 빌어.",
     "당신, $N 님을 전 ..우주의 힘을 빌어.",
     "$n, 당신을 전 ..우주의 힘을 빌어."},
    
    {"$n, $N 님을 진공상태로.",
     "당신, $N 님을 진공상태로.",
     "$n, 당신을 진공상태로."} 
  };
  
  w_type -= TYPE_HIT;   /* Change to base of table with text */
  
  wield = ch->equipment[WIELD];
  
  /* updated by atre */
  if (dam == 0)   		 msg_index = 0 ;
  else if ( dam <= 20 )	 msg_index = 1 ;
  else if ( dam <= 50 )	 msg_index = 2 ;
  else if ( dam <= 100)	 msg_index = 3 ;
  else if ( dam <= 200)	 msg_index = 4 ;
  else if ( dam <= 400)	 msg_index = 5 ;
  else if ( dam <= 600)  msg_index = 6 ;
  else if ( dam <= 800)	 msg_index = 7 ;
  else if ( dam <= 1000) msg_index = 8 ;
  else if ( dam <= 1200) msg_index = 9 ;
  else if ( dam <= 1300) msg_index = 10 ;
  else if ( dam <= 1500) msg_index = 11 ;
  else if ( dam <= 2000) msg_index = 12 ;
  else if ( dam <= 3000) msg_index = 13;
  else if ( dam <= 5000) msg_index = 14;
  else msg_index=15;
  
  /*
    all messages are brief...
    */
  if ( msg_index == 3 || msg_index == 4 || msg_index == 5 || msg_index == 7 ) {
    buf = replace_string ( brief_dam_weapons[msg_index].to_room,
			   attack_hit_text[w_type].plural);
  }
  else {
    buf = replace_string ( brief_dam_weapons[msg_index].to_room,
			   attack_hit_text[w_type].singular);
  }
  
  /* no miss */
  if ( msg_index > 0) {
    buf2 = replace_string ( brief_han_weapons[msg_index].to_room,
			    attack_hit_han[w_type].singular);
    acthan(buf,buf2, FALSE, ch, wield, victim, TO_NOTVICT);
  }
  
  buf = replace_string ( brief_dam_weapons[msg_index].to_char,
			 attack_hit_text[w_type].singular);
  buf2 = replace_string ( brief_han_weapons[msg_index].to_char,
			  attack_hit_han[w_type].singular);
  acthan(buf,buf2, FALSE, ch, wield, victim, TO_CHAR);
  
  if ( msg_index == 1 || msg_index == 2 || msg_index == 3 || msg_index == 5
       || msg_index == 7 ) {
    buf = replace_string ( brief_dam_weapons[msg_index].to_victim,
			   attack_hit_text[w_type].plural);
  }
  else {
    buf = replace_string ( brief_dam_weapons[msg_index].to_victim,
			   attack_hit_text[w_type].singular);
  }
  buf2 = replace_string ( brief_han_weapons[msg_index].to_victim,
			  attack_hit_han[w_type].singular);
  acthan(buf,buf2, FALSE, ch, wield, victim, TO_VICT);
}

#ifdef OLD_DAM_MESSAGE
void dam_message(int dam, struct char_data *ch, struct char_data *victim,
                 int w_type)
{
  struct obj_data *wield;
  char *buf, *buf2;
  int  msg_index ;

  static struct dam_weapon_type dam_weapons[] = {
    
    {"$n misses $N with $s #W.",                           /*    0    */
     "You miss $N with your #W.",
     "$n miss you with $s #W." },
    
    {"$n tickles $N with $s #W.",                          /*  1.. 4  */
     "You tickle $N as you #W $M.",
     "$n tickle you as $e #W you." },
    
    {"$n barely #W $N.",                                   /*  5.. 8  */
     "You barely #W $N.",
     "$n barely #W you."},
    
    {"$n #W $N.",                                          /*  9.. 12  */
     "You #W $N.",
     "$n #W you."}, 
    
    {"$n #W $N hard.",                                     /*  13..20  */
     "You #W $N hard.",
     "$n #W you hard."},
    
    {"$n #W $N very hard.",                                /* 21..30  */
     "You #W $N very hard.",
     "$n #W you very hard."},
    
    {"$n #W $N extremely hard.",                          /* 31..40  */
     "You #W $N extremely hard.",
     "$n #W you extremely hard."},
    
    {"$n massacre $N to small fragments with $s #W.",     /* > 40    */
     "You massacre $N to small fragments with your #W.",
     "$n massacre you to small fragments with $s #W."},
    
    {"$n annihilates $N to micro molecules with $s #W.",   /* > 65    */
     "You annihilate $N to micro molecules with your #W.",
     "$n annihilates you to micro molecules with $s #W."},
    
    { "$n disintegrates $N with $s #W.",                    /* > 110 */
      "You disintegrate $N with your #W.",
      "$n disintegrates you with $s #W." },
    
    { "$n sends $N void with $s #W.",  /* > 150 */
      "You send $N to void with your #W.",
      "$n sends you to void with $s #W." },
    
    { "$n tickles $N void with $s #W.",  /* > 200 */
      "You tickle $N to void with your #W.",
      "$n tickles you to void with $s #W." },
    
    /* Furfuri made massage */
    
    { "$n shoots $N with laser power $s #W.",  /* > 250 */
      "You shoot $N with laser power #W.",
      "$n shoots you with laser power $s #W." },
    
    { "$n strikes $N with NUCLEAR power $s #W.",
      "You strike $N with NUCLEAR power #W.",
      "$n strikes you with NUCLEAR power $s #W."},
    
    { "$n CRASHES $N with -= S P A C E   P O W E R =- $s #W.",  
      "YOU CRASH $N with -= S P A C E   P O W E R =- #W.",
      "$n CRASHES YOU with -= S P A C E   P O W E R =- $s #W." },
    
    { "$n PROCESSES $N into ---==<< V A C U U M >>==--- with $s #W.",
      "YOU PROCESS $N into ---==<< V A C U U M >>==--- with your #W.",
      "$n PROCESSES YOU into ---==<< V A C U U M >>==--- with $s #W." }
  };

  static struct dam_weapon_type han_weapons[] = {
    {"$n 님이 $N 님을 때리려다 빗나갔습니다.",        /*    0    */
     "당신은 $N 님을 때리지 못했습니다.",
     "$n 님이 당신을 때리려다 실패했습니다." },
    
    {"$n 님이 $N 님을 간지럽게 #W.",                          /*  1.. 4  */
     "당신은 $N 님을 간지럽게 #W.",
     "$n 님이 당신을 간지럽게 #W." },
    
    {"$n 님이 $N 님을 살살 #W.",                                   /*  5.. 8  */
     "당신은 $N 님을 살살 #W.",
     "$n 님이 당신을 살살 #W."},
    
    {"$n 님이 $N 님을 #W.",                                       /*  9.. 12  */
     "당신은 $N 님을 #W.",
     "$n 님이 당신을 #W."}, 
    
    {"$n 님이 $N 님을 세게 #W.",                                /*  13..20  */
     "당신은 $N 님을 세게 #W.",
     "$n 님이 당신을 세게 #W."},
    
    {"$n 님이 $N 님을 매우 세게 #W.",                                /* 21..30  */
     "당신은 $N 님을 매우 세게 #W.",
     "$n 님이 당신을 매우 세게 #W."},
    
    {"$n 님이 $N 님을 무지막지하게 #W.",                          /* 31..40  */
     "당신은 $N 님을 무지막지하게 #W.",
     "$n 님이 당신을 무지막지하게 #W."},
    
    {"$n 님이 $N 님을 뼈가 으스러지게 #W.",     /* > 40    */
     "당신은 $N 님을 뼈가 으스러지게 #W.",
     "$n 님이 당신을 뼈가 으스러지게 #W."},
    
    {"$n 님이 $N 님을 몸이 산산조각이 나게 #W.",     /* > 65    */
     "당신은 $N 님을 몸이 산산조각이 나게 #W.",
     "$n 님이 당신을 몸이 산산조각이 나게 #W."},
    
    {"$n 님이 $N 님을 몸이 가루가 되게 #W.",     /* > 110    */
     "당신은 $N 님을 몸이 가루가 되게 #W.",
     "$n 님이 당신을 몸이 가루가 되게 #W."},
    
    {"$n 님이 $N 님을 눈앞에 별이 돌게#W.",       /* > 150 */
     "당신은 $N 님을 눈앞에 별이 돌게#W.",
     "$n 님이 당신을 눈앞에 별이 돌게#W." },
    
    {"$n 님이 $N 님을 눈앞에 병아리가 돌게#W.",       /* > 200 */
     "당신은 $N 님을 눈앞에 병아리가 돌게#W.",
     "$n 님이 당신을 눈앞에 병아리가 돌게#W." },
    
    {"$n 님이 $N 님을 소리없이#W.",       /* > 250 */
     "당신은 $N 님을 소리없이#W.",
     "$n 님이 당신을 소리없이#W." },
    
    {"$n 님이 $N 님을 레이져 파워로 #W.",
     "당신은 $N 님을 레이져 파워로 #W.",
     "$n 님이 당신을 레이져 파워로 #W."},
    
    {"$n 님이 $N 님을 전..우주의 힘을 빌어 #W.",
     "당신은 $N 님을 전 ..우주의 힘을 빌어 #W.",
     "$n 님이 당신을 전 ..우주의 힘을 빌어 #W."},
    
    {"$n 님이 $N 님을 진공상태로 되도록 #W.",
     "당신은 $N 님을 진공상태로 되도록 #W.",
     "$n 님이 당신을 진공상태가 되도록 #W."} 
  };
  
  w_type -= TYPE_HIT;   /* Change to base of table with text */
  
  wield = ch->equipment[WIELD];
  
  /* updated by atre */
  /*
    if (dam == 0)		 msg_index = 0 ;
    else if ( dam <= 10 )	msg_index = 1 ;
    else if ( dam <= 20 )	msg_index = 2 ;
    else if ( dam <= 30 )	msg_index = 3 ;
    else if ( dam <= 40 )	msg_index = 4 ;
    else if ( dam <= 50 )	msg_index = 5 ;
    else if ( dam <= 70 )	msg_index = 6 ;
    else if ( dam <= 110)	msg_index = 7 ;
    else if ( dam <= 160)	msg_index = 8 ;
    else if ( dam <= 200) msg_index = 9 ;
    else if ( dam <= 300) msg_index = 10 ;
    else if ( dam <= 500) msg_index = 11 ;
    else if ( dam <= 700) msg_index = 12 ;
    else if ( dam <= 1000) msg_index = 13;
    else if ( dam <= 2000) msg_index = 14;
    else msg_index=15;
    */
  if (dam == 0)   		 msg_index = 0 ;
  else if ( dam <= 20 )	 msg_index = 1 ;
  else if ( dam <= 50 )	 msg_index = 2 ;
  else if ( dam <= 100)	 msg_index = 3 ;
  else if ( dam <= 200)	 msg_index = 4 ;
  else if ( dam <= 400)	 msg_index = 5 ;
  else if ( dam <= 600)  msg_index = 6 ;
  else if ( dam <= 800)	 msg_index = 7 ;
  else if ( dam <= 1000) msg_index = 8 ;
  else if ( dam <= 1200) msg_index = 9 ;
  else if ( dam <= 1300) msg_index = 10 ;
  else if ( dam <= 1500) msg_index = 11 ;
  else if ( dam <= 2000) msg_index = 12 ;
  else if ( dam <= 3000) msg_index = 13;
  else if ( dam <= 5000) msg_index = 14;
  else msg_index=15;
  
  if ( msg_index == 3 || msg_index == 4 || msg_index == 5 || msg_index == 7 ) {
    buf = replace_string ( dam_weapons[msg_index].to_room,
			   attack_hit_text[w_type].plural);
  }
  else {
    buf = replace_string ( dam_weapons[msg_index].to_room,
			   attack_hit_text[w_type].singular);
  }
  buf2 = replace_string ( han_weapons[msg_index].to_room,
			  attack_hit_han[w_type].singular);
  acthan(buf,buf2, FALSE, ch, wield, victim, TO_NOTVICT);
  
  buf = replace_string ( dam_weapons[msg_index].to_char,
			 attack_hit_text[w_type].singular);
  buf2 = replace_string ( han_weapons[msg_index].to_char,
			  attack_hit_han[w_type].singular);
  acthan(buf,buf2, FALSE, ch, wield, victim, TO_CHAR);
  
  if ( msg_index == 1 || msg_index == 2 || msg_index == 3 || msg_index == 5
       || msg_index == 7 ) {
    buf = replace_string ( dam_weapons[msg_index].to_victim,
			   attack_hit_text[w_type].plural);
  }
  else {
    buf = replace_string ( dam_weapons[msg_index].to_victim,
			   attack_hit_text[w_type].singular);
  }
  buf2 = replace_string ( han_weapons[msg_index].to_victim,
			  attack_hit_han[w_type].singular);
  acthan(buf,buf2, FALSE, ch, wield, victim, TO_VICT);
}
#endif

void damage(struct char_data *ch, struct char_data *victim,
            int dam, int attacktype)
{
  char buf[MAX_STRING_LENGTH];
  struct message_type *messages;
  int i,j,nr,max_hit,exp;
  extern int nokillflag;
  
  /* for quest */
  struct follow_type *f;
  
  int hit_limit(struct char_data *ch);
  
  if (!victim || !ch)
    return;
  
#ifdef GHOST
  /* connectionless PC can't damage or be damaged */
  if ((!IS_NPC(ch) && !ch->desc) || (!IS_NPC(victim) && !victim->desc)) 
    return;
#endif /* GHOST */
  
  if (nokillflag)
    if (!IS_NPC(ch) && !IS_NPC(victim))
      return;
  
  /* can't hit same guild member */
  if (ch->player.guild == victim->player.guild &&
      (ch->player.guild || victim->player.guild)) {
    send_to_char("Wanna hit your guild member?  CAN'T!\n\r", ch);
    return;
  }
  
  if (GET_POS(victim) <= POSITION_DEAD)
    return;
  
  if (GET_LEVEL(victim) >=IMO && !IS_NPC(victim))
    dam = 0;
  
  if (victim != ch) {
    if (GET_POS(victim) > POSITION_STUNNED) {
      if (!victim->specials.fighting)
	set_fighting(victim, ch);
      GET_POS(victim) = POSITION_FIGHTING;
    }
    
    if (GET_POS(ch) > POSITION_STUNNED) {
      if (!ch->specials.fighting && ch->in_room == victim->in_room)
	set_fighting(ch, victim);
      
      /* forbid charmed person damage charmed person */
      /*
	if (IS_AFFECTED(ch, AFF_CHARM) && IS_AFFECTED(victim,AFF_CHARM)) {
	if(ch->specials.fighting)
	stop_fighting(ch);
	if(victim->specials.fighting)
	stop_fighting(victim);
	return;
	}
	*/
      
      /* charmed mob can't ALWAYS kill another mob for player */
      if (IS_NPC(ch) && IS_NPC(victim) && victim->master &&
	  !number(0,4) && IS_AFFECTED(victim, AFF_CHARM) &&
	  (victim->master->in_room == ch->in_room)) {
	if (ch->specials.fighting)
	  stop_fighting(ch);
	if (ch->in_room == victim->master->in_room)
	  hit(ch, victim->master, TYPE_UNDEFINED);
	return;
      } 
    }
  }
  
  if (victim->master == ch)
    stop_follower(victim);
  if (IS_AFFECTED(ch, AFF_INVISIBLE))
    appear(ch);
  if (IS_AFFECTED(victim, AFF_SANCTUARY))
    dam /= 2;
  if (IS_AFFECTED(victim, AFF_LOVE))
    dam *= 2;
  /*
    if (IS_NPC(ch))
    dam *= 2;
    */
  
  dam = MAX(dam, -dam);
  /* MAX DAMAGE */
  dam = MIN(dam,100000);
  GET_HIT(victim) -= dam; /* this is the maximum damage */
  
  /* exp gained by damage */
  if (ch != victim)
    gain_exp(ch, GET_LEVEL(victim) * dam / 2);
  
  update_pos(victim);
  if (attacktype >= TYPE_HIT && attacktype <= TYPE_SLASH) {
    if (!ch->equipment[WIELD])
      dam_message(dam, ch, victim, TYPE_HIT);
    else
      dam_message(dam, ch, victim, attacktype);
  }
  else if (attacktype != TYPE_SHOOT) {
    if (attacktype) {
      for(i = 0; i < MAX_MESSAGES; i++) {
	if (fight_messages[i].a_type == attacktype) {
	  nr = dice(1, fight_messages[i].number_of_attacks);
	  for(j = 1, messages = fight_messages[i].msg;
	      j < nr && messages; j++)
	    messages = messages->next;
	  
	  if (!IS_NPC(victim) && (GET_LEVEL(victim) >= IMO)) {
	    act(messages->god_msg.attacker_msg, FALSE, ch,
		ch->equipment[WIELD], victim, TO_CHAR);
	    act(messages->god_msg.victim_msg, FALSE, ch,
		ch->equipment[WIELD], victim, TO_VICT);
	    act(messages->god_msg.room_msg, FALSE, ch,
		ch->equipment[WIELD], victim, TO_NOTVICT);
	  }
	  else if (dam != 0) {
	    if (GET_POS(victim) == POSITION_DEAD) {
	      act(messages->die_msg.attacker_msg, FALSE, ch,
		  ch->equipment[WIELD], victim, TO_CHAR);
	      act(messages->die_msg.victim_msg, FALSE, ch,
		  ch->equipment[WIELD], victim, TO_VICT);
	      act(messages->die_msg.room_msg, FALSE, ch,
		  ch->equipment[WIELD], victim, TO_NOTVICT);
	    }
	    else {
	      act(messages->hit_msg.attacker_msg, FALSE, ch,
		  ch->equipment[WIELD], victim, TO_CHAR);
	      act(messages->hit_msg.victim_msg, FALSE, ch,
		  ch->equipment[WIELD], victim, TO_VICT);
	      act(messages->hit_msg.room_msg, FALSE, ch,
		  ch->equipment[WIELD], victim, TO_NOTVICT);
	    }
	  }
	  else { /* Dam == 0 */
	    act(messages->miss_msg.attacker_msg, FALSE, ch,
		ch->equipment[WIELD], victim, TO_CHAR);
	    act(messages->miss_msg.victim_msg, FALSE, ch,
		ch->equipment[WIELD], victim, TO_VICT);
	    act(messages->miss_msg.room_msg, FALSE, ch,
		ch->equipment[WIELD], victim, TO_NOTVICT);
	  }
	  break;
	} /* end of if */
      } /* end of for */
    } /* end of if */
  } /* end of else */
  
  switch (GET_POS(victim)) {
  case POSITION_MORTALLYW:
    act("$n is mortally wounded, and will die soon, if not aided.",
	TRUE, victim, 0, 0, TO_ROOM);
    act("You are mortally wounded, and will die soon, if not aided.",
	FALSE, victim, 0, 0, TO_CHAR);
    break;
  case POSITION_INCAP:
    act("$n is incapacitated and will slowly die, if not aided.",
	TRUE, victim, 0, 0, TO_ROOM);
    act("You are incapacitated an will slowly die, if not aided.",
	FALSE, victim, 0, 0, TO_CHAR);
    break;
  case POSITION_STUNNED:
    act("$n is stunned, but could regain consciousness again.",
	TRUE, victim, 0, 0, TO_ROOM);
    act("You're stunned, but could regain consciousness again.",
	FALSE, victim, 0, 0, TO_CHAR);
    break;
  case POSITION_DEAD:
    act("$n is dead! R.I.P.", TRUE, victim, 0, 0, TO_ROOM);
    act("You are dead!  Sorry...", FALSE, victim, 0, 0, TO_CHAR);
    break;
  default:  /* >= POSITION SLEEPING */
    max_hit = hit_limit(victim);
    if (dam > max_hit /10)
      act("That Really did HURT!", FALSE, victim, 0, 0, TO_CHAR);
    if (IS_NPC(victim) && IS_SET(victim->specials.act, ACT_WIMPY) &&
	GET_HIT(victim) < max_hit / 30){
      act("You wish that your wounds would stop BLEEDING that much!",
	  FALSE, victim, 0, 0, TO_CHAR);
      do_flee(victim, "", 0);
    }
    else if(!IS_NPC(victim)&&IS_SET(victim->specials.act,PLR_WIMPY)&&
	    GET_HIT(victim) < victim->specials.wimpyness){
      act("You wish that your wounds would stop BLEEDING that much!",
	  FALSE, victim, 0, 0, TO_CHAR);
      do_flee(victim, "", 0);
    }
    break;  
  }
  
  if (GET_POS(victim) < POSITION_MORTALLYW)
    if (ch->specials.fighting == victim)
      stop_fighting(ch);
  
  if (!AWAKE(victim))
    if (victim->specials.fighting)
      stop_fighting(victim);
  
  if (GET_POS(victim) == POSITION_DEAD) {
    if (IS_NPC(victim)) {
      if (IS_AFFECTED(ch, AFF_RERAISE))
	gain_exp(ch,0);
      else {
	if (IS_AFFECTED(ch, AFF_GROUP))
	  group_gain(ch, victim);
	else { 
	  /* Calculate level-difference bonus? */
	  exp = GET_EXP(victim);
	  exp = MAX(exp, 1);
	  gain_exp(ch, exp);
	  GET_GOLD(ch) += GET_GOLD(victim);
	  GET_GOLD(ch) += (GET_LEVEL(victim)*GET_LEVEL(victim)*500);
	  change_alignment(ch, victim);
	}
	
	/* quest : check quest */
	if(ch->master){
	  f = ch->master->followers;
	  /* check master */
	  /*
	    if(IS_AFFECTED(ch->master, AFF_GROUP) &&
	    */
	  if(ch->master->in_room == ch->in_room)
	    check_quest_mob_die(ch->master, victim->nr);
	}
	else {
	  f = ch->followers;
	  
	  /* check ch */
	  check_quest_mob_die(ch, victim->nr);
	}
	
	while(f){
	  /* check followers */
	  /*
	    if(IS_AFFECTED(f->follower, AFF_GROUP) &&
	    */
	  if(f->follower->in_room == ch->in_room)
	    check_quest_mob_die(f->follower, victim->nr);
	  f = f->next;
	}
      }
    }
    else { /* if player killed . */
      ch->player.pk_num++;
      if (!IS_NPC(ch) && !IS_AFFECTED(victim, AFF_RERAISE)) {
	exp = ((GET_EXP(victim) / 10) * 4 / 9);
	gain_exp(ch, exp);
	GET_ALIGNMENT(ch) -= 100;
	GET_GOLD(ch) += GET_GOLD(victim);
      }
    }
    
    /* just for log */
    if (!IS_NPC(victim)) {
      if (!IS_AFFECTED(victim, AFF_RERAISE)) {
	sprintf(buf, "%s killed by %s at %s", GET_NAME(victim),
		(IS_NPC(ch) ? ch->player.short_descr : GET_NAME(ch)),
		world[victim->in_room].name);
      }
      else {
	sprintf(buf, "%s was reraised at killing of %s at %s",
		GET_NAME(victim),
		(IS_NPC(ch) ? ch->player.short_descr : GET_NAME(ch)),
		world[victim->in_room].name);
      }
      log(buf);
    }
    
    die(victim, GET_LEVEL(ch), ch);
  }
}

void hit(struct char_data *ch, struct char_data *victim, int type)
{
  struct obj_data *wielded = 0;
  //  struct obj_data *held = 0;
  int w_type;
  int dam, prf;
  int parry_num;
  int miss;
  int limit_nodice, limit_sizedice;
  // char buffer[MAX_STRING_LENGTH];

  extern int thaco[4][IMO+4];
  extern byte backstab_mult[];
  extern struct str_app_type str_app[];
  extern struct dex_app_type dex_app[];
  
  if (ch == NULL || victim == NULL)
    return;
  
  if (ch->in_room != victim->in_room) {
    log("NOT SAME ROOM WHEN FIGHTING!");
    return;
  }
  
  prf = 0;
  /* parry for NPC */
  if (GET_LEARNED(victim, SKILL_PARRY)){
    /* new mechanism by atre */
    parry_num =  GET_LEARNED(victim, SKILL_PARRY);
    parry_num += (GET_SKILLED(victim, SKILL_PARRY) << 1);
    parry_num += (GET_DEX(victim) << 1);
    
    if(GET_CLASS(victim)==CLASS_THIEF){
      parry_num += GET_LEVEL(victim);
    }
    
    if(number(0, parry_num) > (GET_HITROLL(ch) << 1) + GET_LEVEL(ch) - GET_LEVEL(victim) + 43){
      prf = 1;
    }
    
#ifdef OLD_PARRY
    if (IS_NPC(victim))
      parry_num = GET_LEVEL(victim)*50;
    /*
      parry_num = GET_LEVEL(victim)*35;
      */
    /* parry for PC */
    else {
      /*
	parry_num = GET_LEVEL(victim)*6;
	*/
      parry_num = GET_LEVEL(victim)*((GET_SKILLED(victim, SKILL_PARRY)>>3)+6);
    }
    
    if (GET_CLASS(victim)==CLASS_THIEF)
      parry_num -= 2*GET_LEVEL(victim);
    
    if (victim->skills[SKILL_PARRY].learned + 2 * GET_DEX(victim) >
	number(1, parry_num) - ((GET_LEVEL(victim) - GET_LEVEL(ch)) << 2))
      prf = 1;
#endif /* OLD_PARRY */
  }
  /* mirror image */
#ifdef OLD_MIRROR_IMAGE
  if (IS_AFFECTED(victim, AFF_MIRROR_IMAGE) &&
      3 * number(1, GET_LEVEL(ch)) < number(1, GET_LEVEL(victim)))
    prf = 2;
#endif /* OLD_MIRROR_IMAGE */
  /* new mechanism by atre */
  if (!prf && IS_AFFECTED(victim, AFF_MIRROR_IMAGE)){
    parry_num = GET_LEVEL(victim) + GET_LEARNED(victim, SPELL_MIRROR_IMAGE);
    parry_num += (GET_SKILLED(victim, SPELL_MIRROR_IMAGE) << 1);
    if(number(0, parry_num) > 3 * GET_LEVEL(ch)){
      prf = 2;
    }
  }
  
  if (prf) {
    if (prf == 1) {
      send_to_char("You parry successfully.\n\r", victim);
      act("$N parries successfully.",
	  FALSE, ch, 0, victim, TO_CHAR);
      INCREASE_SKILLED1(victim, ch, SKILL_PARRY);
    }
    else if (prf == 2) {
      send_to_char("You hit illusion. You are confused.\n\r", ch);
      act("$n hits illusion, looks confused.",
	  FALSE, ch, 0, victim, TO_VICT);
      INCREASE_SKILLED1(victim, ch, SPELL_MIRROR_IMAGE);
    }
    return;
	}
  
  // if (ch->equipment[HOLD])
  //     held = ch->equipment[HOLD];
  
  if (ch->equipment[WIELD] &&
      (ch->equipment[WIELD]->obj_flags.type_flag == ITEM_WEAPON)) {
    wielded = ch->equipment[WIELD];
    switch (wielded->obj_flags.value[3]) {
    case 0:
    case 1:
    case 2:
      w_type = TYPE_WHIP; break;
    case 3:
      w_type = TYPE_SLASH; break;
    case 4:
    case 5:
    case 6:
      w_type = TYPE_CRUSH; break;
    case 7:
      w_type = TYPE_BLUDGEON; break;
    case 8:
    case 9:
    case 10:
    case 11:
      w_type = TYPE_PIERCE; break;
    default:
      w_type = TYPE_HIT; break;
    }
  }
  else {
    if (IS_NPC(ch) && (ch->specials.attack_type >= TYPE_HIT))
      w_type = ch->specials.attack_type;
    else
      w_type = TYPE_HIT;
  }
  
  miss = -GET_HITROLL(ch);
  miss -= number(0, dex_app[GET_DEX(ch)].attack);
  miss -= number(0, thaco[(int) GET_CLASS(ch) - 1][(int) GET_LEVEL(ch)]);
  miss += dex_app[GET_DEX(victim)].defensive;
  if (type == SKILL_BACKSTAB){
    miss -= (GET_LEVEL(ch) >> 2);
    miss -= ((GET_LEVEL(ch) >> 2) + (GET_SKILLED(ch, SKILL_BACKSTAB) >> 2));
  }
  
  /* attack misses */
  if (AWAKE(victim) && miss > 0) {
    if (type == SKILL_BACKSTAB)
      damage(ch, victim, 0, SKILL_BACKSTAB);
    else
      damage(ch, victim, 0, w_type);
    return;
  }
  
  dam = str_app[STRENGTH_APPLY_INDEX(ch)].todam;
  dam += GET_DAMROLL(ch);
  if (!wielded ){
    if(GET_CLASS(ch) == CLASS_WARRIOR)
      dam += dice(ch->specials.damnodice, ch->specials.damsizedice);
    else dam += 0;
    
    /* increase dice of bare hand */
    if(GET_CLASS(ch) == CLASS_WARRIOR){

      if(number(0, 49 + ((ch->specials.damnodice * ch->specials.damsizedice)<<4)) == 37){
	/* Check for all remortaled by dsshin 	*/
	/* Limit is changed by epochal		  	*/
	/*
	  limit_nodice = 21;
	  limit_sizedice = 31;
	  */
	limit_nodice = 15;
	limit_sizedice = 15;
	if((ch->player.level >= (IMO -1)) && (ch->player.remortal >= 15))
	  {
	    /*limit_nodice = 40;
	      limit_sizedice = 40;*/
	    limit_nodice = 1000;
	    limit_sizedice = 1000;
	  } 
	/*  to here */				
	if(number(0,2) == 1){
	  if(ch->specials.damnodice < limit_nodice){
	    send_to_char("Your bare hand dice is added!!!\n\r", ch);
	    ch->specials.damnodice ++;
	  }
	}
	else {
	  if(ch->specials.damsizedice < limit_sizedice){
	    send_to_char("Your bare hand dice is enlarged!!!\n\r", ch);
	    ch->specials.damsizedice ++;
	  }
	}
      }
    }
  }
  else {
    dam += dice(wielded->obj_flags.value[1],
		wielded->obj_flags.value[2]);
    if (ch->equipment[HOLD] && number(1,10) > 4 &&
	CAN_WEAR(ch->equipment[HOLD], ITEM_WIELD) &&
	GET_CLASS(ch) == CLASS_WARRIOR)
      dam += dice(ch->equipment[HOLD]->obj_flags.value[1],
		  ch->equipment[HOLD]->obj_flags.value[2]);
  }
  
  /* 여의봉 */
  if (ch->equipment[WIELD]
      && ch->equipment[WIELD]->item_number >= 0
      && ch->equipment[HOLD]
      && ch->equipment[HOLD]->item_number >= 0
      && obj_index[ch->equipment[WIELD]->item_number].virtual == 11126
      && obj_index[ch->equipment[HOLD]->item_number].virtual == 11126
      && number(1,10) > 8)
    dam += dice(ch->equipment[HOLD]->obj_flags.value[1],
		ch->equipment[HOLD]->obj_flags.value[2]);
  
  /* lss belt sword */
  if (ch->equipment[WEAR_WAISTE]
      && ch->equipment[WEAR_WAISTE]->item_number >= 0
      && obj_index[ch->equipment[WEAR_WAISTE]->item_number].virtual == 9508
      && number(1,10) > 6)
    dam += dice(ch->equipment[WEAR_WAISTE]->obj_flags.value[1],
		ch->equipment[WEAR_WAISTE]->obj_flags.value[2]);
  
  if (GET_AC(victim) > 0)
    dam -= (GET_AC(victim) >> 2);
  else
    dam += (GET_AC(victim) >> 1);
  
  if (GET_POS(victim) < POSITION_FIGHTING)
    dam *= 1 + (POSITION_FIGHTING - GET_POS(victim)) / 3;
  /* Position  sitting  x 1.33 */
  /* Position  resting  x 1.66 */
  /* Position  sleeping x 2.00 */
  /* Position  stunned  x 2.33 */
  /* Position  incap    x 2.66 */
  /* Position  mortally x 3.00 */
  
  if (ch->skills[SKILL_EXTRA_DAMAGING].learned) {	
    if (ch->skills[SKILL_EXTRA_DAMAGING].learned >
	number(1,500) - 4 * GET_LEVEL(ch)) {
      /* warrior's extra damaging is more powerful */
      INCREASE_SKILLED1(ch, victim, SKILL_EXTRA_DAMAGING);
      if (ch->player.class == CLASS_WARRIOR)
	dam += (dam * ((GET_SKILLED(ch, SKILL_EXTRA_DAMAGING) >> 5) + 1));
      else
	dam += number(dam >> 1, dam * ((GET_SKILLED(ch, SKILL_EXTRA_DAMAGING) >> 5) + 1));
    }
  }
  
  if (IS_AFFECTED(ch, AFF_LOVE))
    dam *= 2;
  if (IS_AFFECTED(ch, AFF_DEATH))
    dam *= 4;
  
  /* each class has dedicated weapon */
  if (GET_CLASS(ch) == CLASS_THIEF &&
      w_type == TYPE_PIERCE && number(1, 10) > 5)
    dam *= 2;
  if (GET_CLASS(ch) == CLASS_MAGIC_USER &&
      w_type == TYPE_BLUDGEON && number(1,10) > 5)
    dam *= 2;
  if (GET_CLASS(ch) == CLASS_WARRIOR &&
      w_type == TYPE_SLASH && number(1,10) > 5)
    dam *= 2;
  if (GET_CLASS(ch) == CLASS_CLERIC &&
      w_type == TYPE_BLUDGEON && number (1,10) > 5)
    dam *= 2;
  
  if (IS_AFFECTED(ch, AFF_SHADOW_FIGURE) && number(1, 10) > 6)
    dam *= 3/2;
  
  dam = MAX(1, dam); 
  
  if (type == SKILL_BACKSTAB) {
    if(IS_AFFECTED(ch,AFF_HIDE)) {
      log("backstab+hide");
      dam <<= 1;
    }
    if(IS_NPC(ch)){
      dam *= backstab_mult[GET_LEVEL(ch) / 2];
    }
    else {	/* PC */
      dam*=(backstab_mult[(int) GET_LEVEL(ch)]+(GET_SKILLED(ch,SKILL_BACKSTAB)>>2));
    }
    if (!IS_AFFECTED(victim, AFF_REFLECT_DAMAGE))
      damage(ch, victim, dam<<1, SKILL_BACKSTAB);
    else {
      if(ch->skills[SPELL_REFLECT_DAMAGE].learned > number(1, 500)) {
	INCREASE_SKILLED1(victim, ch, SPELL_REFLECT_DAMAGE);
	act("You reflect damage on $N succesfully.",
	    TRUE, ch, 0, victim, TO_CHAR);
	act("$n reflects damage on $N succesfully.",
	    TRUE, victim, 0, ch, TO_ROOM);
	dam >>= 2;
	dam = MAX(1, dam);
	/*
	  if (GET_GUILD(victim) == ASSASSIN && number(1,300) < 50 )
	  dam += dice(((GET_LEVEL(ch) - 20) >> 2) + 1,
	  GET_GUILD_SKILLS(victim,
	  ASSASSIN_SKILL_IMPROVED_REFLECT_DAMAGE));
	  */
	damage(victim, ch, dam, w_type);
      }
      else
	damage(ch,victim,dam<<1,SKILL_BACKSTAB);
    }
  } 
  else {
    if (!IS_AFFECTED(victim, AFF_REFLECT_DAMAGE))
      damage(ch, victim, dam, w_type);
    else {
      if (ch->skills[SPELL_REFLECT_DAMAGE].learned > number(1,300)) {
	INCREASE_SKILLED1(victim, ch, SPELL_REFLECT_DAMAGE);
	act("You reflect damage on $N successfully.",
	    TRUE, ch, 0, victim, TO_CHAR);
	act("$n reflects damage on $N successfully.",
	    TRUE, victim, 0, ch, TO_ROOM);
	dam >>= 2;
	/*
	  if (GET_GUILD(victim) == ASSASSIN && number(1,100) < 50 )
	  dam += GET_GUILD_SKILLS(victim,
	  ASSASSIN_SKILL_IMPROVED_REFLECT_DAMAGE);
	  */
	damage(victim, ch, dam, w_type);
      }
      else
	damage(ch, victim, dam, w_type);
    }
  }
}

/* control the fights going on */
void perform_violence(void)
{
  struct char_data *ch;
  int i,dat=100;
  extern void magic_weapon_hit();
  struct obj_data *weapon,*held;
  int percent;
  
  for (ch = combat_list; ch; ch=combat_next_dude) {
    combat_next_dude = ch->next_fighting;
    assert(ch->specials.fighting);
    
    if(AWAKE(ch) && (ch->in_room==ch->specials.fighting->in_room)) {
      weapon = ch->equipment[WIELD];
	if( weapon && weapon->obj_flags.gpd > 0 && weapon->obj_flags.value[0] > 0 &&
	    IS_SET( weapon->obj_flags.extra_flags, ITEM_MAGIC ) )
	  magic_weapon_hit(ch, ch->specials.fighting, weapon );
	held = ch->equipment[HOLD];
	if (held && number(1,10) > 5 && held->obj_flags.gpd > 0 &&
	    held->obj_flags.value[0] > 0 && IS_SET(held->obj_flags.extra_flags,
						   ITEM_MAGIC) && CAN_WEAR(ch->equipment[HOLD], ITEM_WIELD) &&
	    GET_CLASS(ch) == CLASS_WARRIOR )
	  magic_weapon_hit(ch, ch->specials.fighting, held);
	hit(ch, ch->specials.fighting, TYPE_UNDEFINED);
	if(!IS_NPC(ch)){
	  /* double attack */
	  if(ch->skills[SKILL_DOUBLE].learned>0){
	    percent = GET_LEARNED(ch, SKILL_DOUBLE)
	      + GET_SKILLED(ch, SKILL_DOUBLE)
	      + 3*GET_LEVEL(ch);
	    if(percent > number(1, 200)){
	      /*
		if(ch->skills[SKILL_DOUBLE].learned>number(1,200)-4*GET_LEVEL(ch))
		*/
	      hit(ch,ch->specials.fighting,TYPE_UNDEFINED);
	      INCREASE_SKILLED1(ch, ch, SKILL_DOUBLE);
	    }
	  }
	  
	  if(ch->skills[SKILL_QUADRUPLE].learned>0) {
	    if(GET_CLASS(ch)==CLASS_MAGIC_USER||GET_CLASS(ch)==CLASS_CLERIC)
	      if(number(1,40)>20+(GET_SKILLED(ch, SKILL_QUADRUPLE)>>3))
		goto octa;
	    
	    for(i=0;i<2;i++){
	      percent = GET_LEARNED(ch, SKILL_QUADRUPLE)
             	+ (GET_SKILLED(ch, SKILL_QUADRUPLE) << 1)
             	+ 6*GET_LEVEL(ch);
	      if(percent > number(1, 450)){
		hit(ch,ch->specials.fighting,TYPE_UNDEFINED);
		INCREASE_SKILLED1(ch, ch, SKILL_QUADRUPLE);
	      }
	    }
	  }
	octa:
       if(ch->skills[SKILL_OCTA].learned>0){
	 if(GET_CLASS(ch)==CLASS_MAGIC_USER||GET_CLASS(ch)==CLASS_CLERIC)
	   if(number(1,40)>10+(GET_SKILLED(ch, SKILL_OCTA)>>3))
	     goto octa;
	 
	 for(i=0;i<4;i++){
	   percent = GET_LEARNED(ch, SKILL_OCTA)
	     + (GET_SKILLED(ch, SKILL_OCTA) << 1)
	     + 6*GET_LEVEL(ch);
	   if(percent > number(1, 450)){
	     hit(ch,ch->specials.fighting,TYPE_UNDEFINED);
	     INCREASE_SKILLED1(ch, ch, SKILL_OCTA);
	   }
	 }
       }
	}
	/*    next: */
	if(IS_NPC(ch)){
	  if(!IS_SET(ch->specials.act,ACT_CLERIC) &&
	     !IS_SET(ch->specials.act,ACT_MAGE) ) 
	    dat=7;
	  else dat=15;
	  for(i=dat;i<GET_LEVEL(ch);i+=dat)
	    hit(ch,ch->specials.fighting,TYPE_UNDEFINED);
	}
	
	if(IS_AFFECTED(ch,AFF_HASTE))
	  {
	    hit(ch,ch->specials.fighting,TYPE_UNDEFINED);
	    hit(ch,ch->specials.fighting,TYPE_UNDEFINED);
	  }
	if(IS_AFFECTED(ch,AFF_IMPROVED_HASTE)){
	  hit(ch,ch->specials.fighting,TYPE_UNDEFINED);
	  hit(ch,ch->specials.fighting,TYPE_UNDEFINED);
	  hit(ch,ch->specials.fighting,TYPE_UNDEFINED);
	}
   
	if(ch->equipment[WEAR_FEET]&&ch->equipment[WEAR_FEET]->item_number>=0&&obj_index[ch->equipment[WEAR_FEET]->item_number].virtual==2012)
	  { /* SPEED boots */
	    hit(ch,ch->specials.fighting,TYPE_UNDEFINED);
	    hit(ch,ch->specials.fighting,TYPE_UNDEFINED);
	    hit(ch,ch->specials.fighting,TYPE_UNDEFINED);
	  }
    } else { /* Not in same room */
      stop_fighting(ch);
    }
  }
}

