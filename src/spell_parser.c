/* ************************************************************************
*  file: spell_parser.c , Basic routines and parsing      Part of DIKUMUD *
*  Usage : Interpreter of spells                                          *
*  Copyright (C) 1990, 1991 - see 'license.doc' for complete information. *
************************************************************************* */

#include <stdio.h>
#include <string.h>
#include <assert.h>

#include "structs.h"
#include "utils.h"
#include "comm.h"
#include "db.h"
#include "interpreter.h" 
#include "spells.h"
#include "handler.h"


/* Global data */

extern struct room_data *world;
extern struct char_data *character_list;
extern char *spell_wear_off_msg[];

/* Extern procedures */

void log(char *str);
int number(int from, int to);
int MIN(int a, int b);
int MAX(int a, int b);
int str_cmp(char *arg1, char *arg2);

/* Extern procedures */
void cast_armor( byte level, struct char_data *ch, char *arg, int si,
   struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_teleport( byte level, struct char_data *ch, char *arg, int si,
   struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_bless( byte level, struct char_data *ch, char *arg, int si,
   struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_damage_up( byte level, struct char_data *ch, char *arg, int si,
   struct char_data *tar_ch,struct obj_data *tar_obj);
void cast_crush_armor( byte level, struct char_data *ch, char *arg, int si,
   struct char_data *tar_ch,struct obj_data *tar_obj);
void cast_blindness( byte level, struct char_data *ch, char *arg, int si,
   struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_burning_hands( byte level, struct char_data *ch, char *arg, int si,
   struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_call_lightning( byte level, struct char_data *ch, char *arg, int si,
   struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_charm_person( byte level, struct char_data *ch, char *arg, int si,
   struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_chill_touch( byte level, struct char_data *ch, char *arg, int si,
   struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_shocking_grasp( byte level, struct char_data *ch, char *arg, int si,
   struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_color_spray( byte level, struct char_data *ch, char *arg, int si,
   struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_create_food( byte level, struct char_data *ch, char *arg, int si,
   struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_create_water( byte level, struct char_data *ch, char *arg, int si,
   struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_cure_blind( byte level, struct char_data *ch, char *arg, int si,
   struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_cure_critic( byte level, struct char_data *ch, char *arg, int si,
   struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_cause_critic( byte level, struct char_data *ch, char *arg, int si,
   struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_mana_boost( byte level, struct char_data *ch, char *arg, int si,
   struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_vitalize( byte level, struct char_data *ch, char *arg, int si,
   struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_cure_light( byte level, struct char_data *ch, char *arg, int si,
   struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_cause_light( byte level, struct char_data *ch, char *arg, int si,
   struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_curse( byte level, struct char_data *ch, char *arg, int si,
   struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_detect_evil( byte level, struct char_data *ch, char *arg, int si,
   struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_detect_invisibility(byte level,struct char_data *ch,char *arg,int si,
   struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_recharger( byte level, struct char_data *ch, char *arg, int si,
   struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_preach( byte level, struct char_data *ch, char *arg, int si,
   struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_dispel_evil( byte level, struct char_data *ch, char *arg, int si,
   struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_earthquake( byte level, struct char_data *ch, char *arg, int si,
   struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_all_heal( byte level, struct char_data *ch, char *arg, int si,
   struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_hand_of_god( byte level, struct char_data *ch, char *arg, int si,
   struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_fire_storm( byte level, struct char_data *ch, char *arg, int si,
   struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_enchant_weapon( byte level, struct char_data *ch, char *arg, int si,
   struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_enchant_armor( byte level, struct char_data *ch, char *arg, int si,
   struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_enchant_person( byte level, struct char_data *ch, char *arg, int si,
   struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_pray_for_armor( byte level, struct char_data *ch, char *arg, int si,
   struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_energy_drain( byte level, struct char_data *ch, char *arg, int si,
   struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_fireball( byte level, struct char_data *ch, char *arg, int si,
   struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_full_fire( byte level, struct char_data *ch, char *arg, int si,
   struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_throw( byte level, struct char_data *ch, char *arg, int si,
   struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_energyflow( byte level, struct char_data *ch, char *arg, int si,
   struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_sunburst( byte level, struct char_data *ch, char *arg, int si,
   struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_disintegrate( byte level, struct char_data *ch, char *arg, int si,
   struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_harm( byte level, struct char_data *ch, char *arg, int si,
   struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_heal( byte level, struct char_data *ch, char *arg, int si,
   struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_full_heal( byte level, struct char_data *ch, char *arg, int si,
   struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_entire_heal( byte level, struct char_data *ch, char *arg, int si,
   struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_invisibility( byte level, struct char_data *ch, char *arg, int si,
   struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_lightning_bolt( byte level, struct char_data *ch, char *arg, int si,
   struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_locate_object( byte level, struct char_data *ch, char *arg, int si,
   struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_magic_missile( byte level, struct char_data *ch, char *arg, int si,
   struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_poison( byte level, struct char_data *ch, char *arg, int si,
   struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_protection_from_evil(byte level,struct char_data *ch,char *arg,
   int si, struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_remove_curse( byte level, struct char_data *ch, char *arg, int si,
   struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_sanctuary( byte level, struct char_data *ch, char *arg, int si,
   struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_infravision( byte level, struct char_data *ch, char *arg, int si,
   struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_sleep( byte level, struct char_data *ch, char *arg, int si,
   struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_strength( byte level, struct char_data *ch, char *arg, int si,
   struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_summon( byte level, struct char_data *ch, char *arg, int si,
   struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_relocate( byte level, struct char_data *ch, char *arg, int si,
   struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_far_look( byte level, struct char_data *ch, char *arg, int si,
   struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_ventriloquate( byte level, struct char_data *ch, char *arg, int si,
   struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_word_of_recall( byte level, struct char_data *ch, char *arg, int si,
   struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_remove_poison( byte level, struct char_data *ch, char *arg, int si,
   struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_sense_life( byte level, struct char_data *ch, char *arg, int si,
   struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_identify( byte level, struct char_data *ch, char *arg, int si,
   struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_reanimate( byte level, struct char_data *ch, char *arg, int si,
   struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_clone( byte level, struct char_data *ch, char *arg, int si,
   struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_trick( byte level, struct char_data *ch, char *arg, int si,
   struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_haste( byte level, struct char_data *ch, char *arg, int si,
   struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_improved_haste( byte level, struct char_data *ch, char *arg, int si,
   struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_create_nectar( byte level, struct char_data *ch, char *arg, int si,
   struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_create_golden_nectar( byte level, struct char_data *ch, char *arg, int si,
   struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_corn_of_ice( byte level, struct char_data *ch, char *arg, int si,
   struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_phase( byte level, struct char_data *ch, char *arg, int si,
   struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_mirror_image( byte level, struct char_data *ch, char *arg, int si,
   struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_love( byte level, struct char_data *ch, char *arg, int si,
   struct char_data *tar_ch, struct obj_data *tar_obj);
   /* LOVE spell perhaps written */
void cast_reraise( byte level, struct char_data *ch, char *arg, int si,
   struct char_data *tar_ch, struct obj_data *tar_obj);
   /* reraise spell chase written */
void cast_death( byte level, struct char_data *ch,char *arg,int si,
   struct char_data *tar_ch,struct obj_data *tar_obj);
void cast_spell_block(byte level,struct char_data *ch,char *arg,int si,
	struct char_data *tar_ch,struct obj_data *tar_obj);
void cast_mana_transfer(byte level,struct char_data *ch,char *arg,int si,
	struct char_data *tar_ch,struct obj_data *tar_obj);
void cast_self_heal(byte level,struct char_data *ch,char *arg,int si,
	struct char_data *tar_ch,struct obj_data *tar_obj);
void cast_holy_shield(byte level,struct char_data *ch,char *arg,int si,
	struct char_data *tar_ch,struct obj_data *tar_obj);
void cast_restore_move(byte level,struct char_data *ch,char *arg,int si,
	struct char_data *tar_ch,struct obj_data *tar_obj);
void cast_kiss_of_process(byte level,struct char_data *ch,char *arg,int si,
	struct char_data *tar_ch,struct obj_data *tar_obj);
void cast_reflect_damage(byte level,struct char_data *ch,char *arg,int si,
	struct char_data *tar_ch,struct obj_data *tar_obj);
void cast_dumb(byte level,struct char_data *ch,char *arg,int si,
	struct char_data *tar_ch,struct obj_data *tar_obj);
void cast_thunder_bolt(byte level,struct char_data *ch,char *arg,int si,
	struct char_data *tar_ch,struct obj_data *tar_obj);
void cast_sanctuary_cloud(byte level,struct char_data *ch,char *arg,int si,
	struct char_data *tar_ch,struct obj_data *tar_obj);

struct spell_info_type spell_info[MAX_SPL_LIST];

char *spells[]=
{
  "armor",               /* 1 */
  "teleport",
  "bless",
  "blindness",
  "burning hands",
  "call lightning",
  "charm person",
  "chill touch",
  "reanimate",
  "color spray",
  "relocate",     /* 11 */
  "create food",
  "create water",
  "cure blind",
  "cure critic",
  "cure light",
  "curse",
  "detect evil",
  "detect invisibility",
  "recharger",
  "preach",       /* 21 */
  "dispel evil",
  "earthquake",
  "enchant weapon",
  "energy drain",
  "fireball",
  "harm",
  "heal",
  "invisibility",
  "lightning bolt",
  "locate object",      /* 31 */
  "magic missile",
  "poison",
  "protection from evil",
  "remove curse",
  "sanctuary",
  "shocking grasp",
  "sleep",
  "strength",
  "summon",
  "ventriloquate",      /* 41 */
  "word of recall",
  "remove poison",
  "sense life",         /* 44 */
  "sunburst",
  "clone",
  "",
  "",
  "",
  "",
  "", /* 51 */
  "",
  "identify",
  "",
  "sneak",        /* 55 */
  "hide",
  "steal",
  "backstab",
  "pick",
  "kick",         /* 60 */
  "bash", /* 61 */
  "rescue",
  "double attack",
  "quadruple attack",
  "extra damaging",
  "",
  "",
  "",
  "",
  "",
  "damage up", /* 71 */
  "energy flow",
  "mana boost",
  "vitalize",
  "full fire",
  "throw",		/* 76 */
  "firestorm",
  "crush armor",
  "full heal",
  "trick",
  "haste",      /* 81 */
  "improved haste",
  "far look",
  "all heal",
  "tornado",        /* 85  */
  "lightning move",
  "parry",
  "flash",
  "multi kick",
  "enchant armor",  /* 90 */
  "pray for armor", /* 91 */
  "infravision",
  "create nectar",
  "create golden nectar",
  "cone of ice",
  "disintegrate",
  "phase",
  "mirror image",
  "hand of god",
  "cause light", /* 100 */
  "cause critic", /* 101 */
  "entire heal",
  "octa attack",
  "love",
  "disarm", /* 105 */
  "reraise",
  "shouryuken", /* 107 */
  "throw object", /* 108 */
  "punch", /* 109 */
  "death", /* 110 */
  "enchant person", /* 111 */
  "spell block", /* 112 */
  "temptation", /* 113 */
  "shadow figure", /* 114 */
  "mana transfer",
  "self heal",
  "holy shield",
  "restore move",
  "heal the world",
  "reflect damage",
  "dumb",
  "spin bird kick",
  "thunderbolt",
  "arrest",
  "sanctuary cloud", /* 125 */
  "\n"
};


/*
  modified by ares
  saving_throws[class][type][level]
  class :
  MAGIC_USER, CLERIC, THIEF, WARRIOR
  type :
  SAVING_PARA, SAVING_HIT_SKILL, SAVING_PETRI, SAVING_BREATH, SAVING_SPELL
  lower value is more good
  worst value must be lower than 100
  best value must be higher than 19
  currently, SAVING_PETRI has no effect.
  
  current stat :
  classification : very good (20 ~ 30 for level 40)
  good      (30 ~ 40 for level 40)
  medium    (40 ~ 50 for level 40)
  bad       (50 ~ 60 for level 40)
  poor      (60 ~ 70 for level 40)
  type :       PARA       HIT SKILL  PETRI      BREATH      SPELL
  MAGIC USER : medium     medium     0          medium      good
  CLERIC     : medium     poor       0          good        very good
  THIEF      : very good  good       0          medium      poor
  WARRIOR    : bad        very good  0          medium      medium
  */

#define SAVING_VERY_GOOD \
{ 59, 58, 57, 56, 55, 54, 53, 52, 51, 50, \
    49, 48, 47, 46, 45, 44, 43, 42, 41, 40, \
    39, 38, 37, 36, 35, 34, 33, 32, 31, 30, \
    29, 28, 27, 26, 25, 24, 23, 22, 21, 20, \
    0, }
#define SAVING_GOOD \
{ 69, 68, 67, 66, 65, 64, 63, 62, 61, 60, \
    59, 58, 57, 56, 55, 54, 53, 52, 51, 50, \
    49, 48, 47, 46, 45, 44, 43, 42, 41, 40, \
    39, 38, 37, 36, 35, 34, 33, 32, 31, 30, \
    0, }
#define SAVING_MEDIUM \
{ 79, 78, 77, 76, 75, 74, 73, 72, 71, 70, \
    69, 68, 67, 66, 65, 64, 63, 62, 61, 60, \
    59, 58, 57, 56, 55, 54, 53, 52, 51, 50, \
    49, 48, 47, 46, 45, 44, 43, 42, 41, 40, \
    0, }
#define SAVING_BAD \
{ 89, 88, 87, 86, 85, 84, 83, 82, 81, 80, \
    79, 78, 77, 76, 75, 74, 73, 72, 71, 70, \
    69, 68, 67, 66, 65, 64, 63, 62, 61, 60, \
    59, 58, 57, 56, 55, 54, 53, 52, 51, 50, \
    0, }
#define SAVING_POOR \
{ 99, 98, 97, 96, 95, 94, 93, 92, 91, 90, \
    89, 88, 87, 86, 85, 84, 83, 82, 81, 80, \
    79, 78, 77, 76, 75, 74, 73, 72, 71, 70, \
    69, 68, 67, 66, 65, 64, 63, 62, 61, 60, \
    0, }
#define SAVING_RESERVED \
{ 0, }

byte saving_throws[4][5][IMO+10] = {
  /* for magic user */
  { SAVING_MEDIUM,
    SAVING_MEDIUM,
    SAVING_RESERVED,
    SAVING_MEDIUM,
    SAVING_GOOD },
  
  /* for cleric */
  { SAVING_MEDIUM,
    SAVING_POOR,
    SAVING_RESERVED,
    SAVING_GOOD,
    SAVING_VERY_GOOD },
  
  /* for thief */
  { SAVING_VERY_GOOD,
    SAVING_GOOD, 
    SAVING_RESERVED,
    SAVING_MEDIUM,
    SAVING_POOR },
  
  /* for warrior */
  { SAVING_BAD,
    SAVING_VERY_GOOD,
    SAVING_RESERVED,
    SAVING_MEDIUM,
    SAVING_MEDIUM }
};	/* end of saving throw */

#undef SAVING_VERY_GOOD
#undef SAVING_GOOD
#undef SAVING_MEDIUM
#undef SAVING_BAD
#undef SAVING_POOR
#undef SAVING_RESERVED


int use_mana(struct char_data *ch, int sn)
{
  int min;
  int lev;
  
  lev = GET_LEVEL(ch) / 5 + 2;
  lev += spell_info[sn].min_level[GET_CLASS(ch) - 1];
  lev = MIN(IMO, lev);
  min = spell_info[sn].min_usesmana * lev;
  min /= (1 + GET_LEVEL(ch));
  min = MAX(spell_info[sn].min_usesmana, min);
  
  return min;
}

void affect_update( void )
{
  static struct affected_type *af, *next_af_dude;
  static struct char_data *i;
  
  for (i = character_list; i; i = i->next) {
    for (af = i->affected; af; af = next_af_dude) {
      next_af_dude = af->next;
      if (af->duration >= 1)
	af->duration--;
      else {
	if (!af->next || (af->next->type != af->type) ||
	    (af->next->duration < 0))
	  /*       (af->next->duration > 0))			*/
	  {
	    send_to_char(spell_wear_off_msg[af->type], i);
	    send_to_char("\n\r", i);
	  }
	affect_remove(i, af);
      }
    }
  }
}


/* Check if making CH follow VICTIM will create an illegal */
/* Follow "Loop/circle"                                    */
bool circle_follow(struct char_data *ch, struct char_data *victim)
{
  struct char_data *k;
  
  for(k=victim; k; k=k->master) {
    if (k == ch)
      return(TRUE);
  }
  
  return(FALSE);
}



/* Called when stop following persons, or stopping charm */
/* This will NOT do if a character quits/dies!!          */
void stop_follower(struct char_data *ch)
{
  struct follow_type *j, *k;
  
  assert(ch->master);
  
  if (IS_AFFECTED(ch, AFF_CHARM)) {
    act("You realize that $N is a jerk!", FALSE, ch, 0, ch->master, TO_CHAR);
    act("$n realizes that $N is a jerk!", FALSE, ch, 0, ch->master, TO_NOTVICT);
    act("$n hates your guts!", FALSE, ch, 0, ch->master, TO_VICT);
    if (affected_by_spell(ch, SPELL_CHARM_PERSON))
      affect_from_char(ch, SPELL_CHARM_PERSON);
  } else {
    act("You stop following $N.", FALSE, ch, 0, ch->master, TO_CHAR);
    act("$n stops following $N.", FALSE, ch, 0, ch->master, TO_NOTVICT);
    act("$n stops following you.", FALSE, ch, 0, ch->master, TO_VICT);
  }
  
  if (ch->master->followers->follower == ch) { /* Head of follower-list? */
    k = ch->master->followers;
    ch->master->followers = k->next;
    free(k);
    k=NULL;
  } else { /* locate follower who is not head of list */
    for(k = ch->master->followers; k->next->follower!=ch; k=k->next)  ;
    
    j = k->next;
    k->next = j->next;
    free(j);
    j=NULL;
  }
  
  ch->master = 0;
  REMOVE_BIT(ch->specials.affected_by, AFF_CHARM | AFF_GROUP);
}



/* Called when a character that follows/is followed dies */
void die_follower(struct char_data *ch)
{
  struct follow_type *j, *k;
  
  if (ch->master)
    stop_follower(ch);
  
  for (k=ch->followers; k; k=j) {
    j = k->next;
    stop_follower(k->follower);
  }
}



/* Do NOT call this before having checked if a circle of followers */
/* will arise. CH will follow leader                               */
void add_follower(struct char_data *ch, struct char_data *leader)
{
  struct follow_type *k;
  
  assert(!ch->master);
  
  ch->master = leader;
  
  CREATE(k, struct follow_type, 1);
  
  k->follower = ch;
  k->next = leader->followers;
  leader->followers = k;
  
  act("You now follow $N.", FALSE, ch, 0, leader, TO_CHAR);
  act("$n starts following you.", TRUE, ch, 0, leader, TO_VICT);
  act("$n now follows $N.", TRUE, ch, 0, leader, TO_NOTVICT);
}



void say_spell( struct char_data *ch, int si )
{
  char buf[100], splwd[256];
  char buf2[MAX_OUTPUT_LENGTH];
  
  int j, offs;
  struct char_data *temp_char;
  
  
  struct syllable {
    char org[10];
    char new[10];
  };
  
  static struct syllable syls[] = {
    { " ", " " },
    { "ar", "abra"   },
    { "au", "kada"    },
    { "bless", "fido" },
    { "blind", "nose" },
    { "bur", "mosa" },
    { "cu", "judi" },
    { "de", "oculo"},
    { "en", "unso" },
    { "light", "dies" },
    { "lo", "hi" },
    { "mor", "zak" },
    { "move", "sido" },
    { "ness", "lacri" },
    { "ning", "illa" },
    { "per", "duda" },
    { "ra", "gru"   },
    { "re", "candus" },
    { "son", "sabru" },
    { "tect", "infra" },
    { "tri", "cula" },
    { "ven", "nofo" },
    {"a", "a"},{"b","b"},{"c","q"},{"d","e"},{"e","z"},{"f","y"},{"g","o"},
    {"h", "p"},{"i","u"},{"j","y"},{"k","t"},{"l","r"},{"m","w"},{"n","i"},
    {"o", "a"},{"p","s"},{"q","d"},{"r","f"},{"s","g"},{"t","h"},{"u","j"},
    {"v", "z"},{"w","x"},{"x","n"},{"y","l"},{"z","k"}, {"",""}
  };
  
  
  
  strcpy(buf, "");
  strcpy(splwd, spells[si-1]);
  
  offs = 0;
  
  while(*(splwd+offs)) {
    for(j=0; *(syls[j].org); j++)
      if (strncmp(syls[j].org, splwd+offs, strlen(syls[j].org))==0) {
	strcat(buf, syls[j].new);
	if (strlen(syls[j].org))
	  offs+=strlen(syls[j].org);
	else
	  ++offs;
      }
  }
  sprintf(buf2,"$n utters the words, '%s'", buf);
  sprintf(buf, "$n utters the words, '%s'", spells[si-1]);
  for(temp_char = world[ch->in_room].people;
      temp_char;
      temp_char = temp_char->next_in_room)
    if(temp_char != ch) {
      if (GET_CLASS(ch) == GET_CLASS(temp_char))
	act(buf, FALSE, ch, 0, temp_char, TO_VICT);
      else
	act(buf2, FALSE, ch, 0, temp_char, TO_VICT);
    }
}

bool saves_spell(struct char_data *ch, int save_type)
{
  int save;
  
  /* Negative apply_saving_throw makes saving throw better! */
  save = ch->specials.apply_saving_throw[save_type];
  
  if (!IS_NPC(ch))
    save += saving_throws[GET_CLASS(ch)-1][save_type][GET_LEVEL(ch)-1];
  
  return (MAX(1,save) < number(1, 100));
}

char *skip_spaces(char *string)
{
  for(;*string && (*string)==' ';string++);
  return(string);
}
/* Assumes that *argument does start with first letter of chopped string */
void do_cast(struct char_data *ch, char *argument, int cmd)
{
  struct obj_data *tar_obj;
  struct char_data *tar_char;
  char name[MAX_STRING_LENGTH];
  int qend, spl, i;
  bool target_ok;
  
  if (!ch)
    return;
  
  /* 41 level can cast but cannot enchant(see magic2.c) */
  /*
    if(!IS_NPC(ch) && GET_LEVEL(ch) == IMO){
    send_to_char("Uk! You can cast nothing.\n\r",ch);
    return;
    }
    */
  
  if (GET_LEVEL(ch) < IMO && IS_SET(world[ch->in_room].room_flags,NO_MAGIC)) {
    send_to_char("Your magical power can't be summoned!\n\r",ch);
    return;
  }
  
  if (IS_AFFECTED(ch, AFF_DUMB) && number(1,10) > 5) {
    send_to_char("Eh? You are DUMBED!!!",ch);
    return;
  }
  argument = skip_spaces(argument);
  
  /* If there is no chars in argument */
  if (!(*argument)) {
    if(!IS_NPC(ch))
      send_to_char("Cast which what where?\n\r", ch);
    return;
  }
  
  if (*argument != '\'') {
    if(!IS_NPC(ch))
      send_to_char("Magic must always be enclosed by ", ch);
    send_to_char("the magic symbols: '\n\r", ch);
    return;
  }
  
  /* Locate the last quote && lowercase the magic words (if any) */
  
  for (qend=1; *(argument+qend) && (*(argument+qend) != '\'') ; qend++)
    *(argument+qend) = LOWER(*(argument+qend));
  
  if (*(argument+qend) != '\'') {
    if(!IS_NPC(ch))
      send_to_char("Magic must always be enclosed by ", ch);
    send_to_char("the magic symbols: '\n\r", ch);
    return;
  }
  
  spl = old_search_block(argument, 1, qend-1,spells, 0);
  
  if (!spl) {
    if(!IS_NPC(ch))
      send_to_char("Your lips do not move, no magic appears.\n\r", ch);
    return;
  }
  
#define ISASPELL ((spl > 0) && (spl < MAXSPELL))
  
  if(ISASPELL && spell_info[spl].spell_pointer) {
    if (GET_POS(ch) < spell_info[spl].minimum_position) {
      if(!IS_NPC(ch)) {
	switch(GET_POS(ch)) {
	case POSITION_SLEEPING :
	  send_to_char("You dream about great ", ch);
	  send_to_char("magical powers.\n\r", ch);
	  break;
	case POSITION_RESTING :
	  send_to_char("You can't concentrate enough ", ch);
	  send_to_char("while resting.\n\r",ch);
	  break;
	case POSITION_SITTING :
	  send_to_char("You can't do this sitting!\n\r", ch);
	  break;
	case POSITION_FIGHTING :
	  send_to_char("Impossible! You can't concentrate ", ch);
	  send_to_char("enough!.\n\r", ch);
	  break;
	default:
	  send_to_char("It seems like you're in ", ch);
	  send_to_char("a pretty bad shape!\n\r",ch);
	  break;
	} /* switch */
      } /* !IS_NPC(ch) */
    } /* end of checking minimum position */
    else {
      int ch_class;
      
      ch_class = GET_CLASS(ch) - 1;
      
      if(spell_info[spl].min_level[ch_class] > GET_LEVEL(ch)) {
	if(!IS_NPC(ch))
	  send_to_char("Sorry, you can't do that yet.\n\r", ch);
	return;
      }
      argument += qend+1; /* Point to the last ' */
      for(;*argument == ' '; argument++);
      target_ok = FALSE;
      tar_char = 0;
      tar_obj = 0;
      
      if (!IS_SET(spell_info[spl].targets, TAR_IGNORE)) {
	argument = one_argument(argument, name);
	if (*name) {
	  if (IS_SET(spell_info[spl].targets, TAR_CHAR_ROOM))
	    if ((tar_char = get_char_room_vis(ch, name)))
	      target_ok = TRUE;
	  if (!target_ok &&
	      IS_SET(spell_info[spl].targets, TAR_CHAR_WORLD))
	    if ((tar_char = get_char_vis(ch, name)))
	      target_ok = TRUE;
	  if (!target_ok &&
	      IS_SET(spell_info[spl].targets, TAR_CHAR_ZONE)) { 
	    if (IS_SET(world[ch->in_room].room_flags, EVERYZONE)) {
	      if ((tar_char = get_char_vis(ch, name)))
		target_ok = TRUE;
	    }
	    else {
	      if ((tar_char = get_char_vis_zone(ch, name)))
		target_ok = TRUE;
	    }
	  }
	  if (!target_ok &&
	      IS_SET(spell_info[spl].targets, TAR_OBJ_INV))
	    if ((tar_obj = get_obj_in_list_vis(ch, name, ch->carrying)))
	      target_ok = TRUE;
	  if (!target_ok &&
	      IS_SET(spell_info[spl].targets, TAR_OBJ_ROOM))
	    if ((tar_obj = get_obj_in_list_vis(ch, name,
					      world[ch->in_room].contents)))
	      target_ok = TRUE;
	  if (!target_ok &&
	      IS_SET(spell_info[spl].targets, TAR_OBJ_WORLD))
	    if ((tar_obj = get_obj_vis(ch, name)))
	      target_ok = TRUE;
	  if (!target_ok &&
	      IS_SET(spell_info[spl].targets, TAR_OBJ_EQUIP)) {
	    for (i=0; i<MAX_WEAR && !target_ok; i++)
	      if (ch->equipment[i] && str_cmp(name,
					      ch->equipment[i]->name) == 0) {
		tar_obj = ch->equipment[i];
		target_ok = TRUE;
	      }
	  }
	  if (!target_ok &&
	      IS_SET(spell_info[spl].targets, TAR_SELF_ONLY))
	    if (str_cmp(GET_NAME(ch), name) == 0) {
	      tar_char = ch;
	      target_ok = TRUE;
	    }
	}
	else { /* No argument was typed */
	  if (IS_SET(spell_info[spl].targets, TAR_FIGHT_SELF))
	    if (ch->specials.fighting) {
	      tar_char = ch;
	      target_ok = TRUE;
	    }
	  if (!target_ok &&
	      IS_SET(spell_info[spl].targets, TAR_FIGHT_VICT))
	    if (ch->specials.fighting) {
	      /* WARNING, MAKE INTO POINTER */
	      tar_char = ch->specials.fighting;
	      target_ok = TRUE;
	    }
	  if (!target_ok &&
	      IS_SET(spell_info[spl].targets, TAR_SELF_ONLY)) {
	    tar_char = ch;
	    target_ok = TRUE;
	  }
	}
      }
      else {
	target_ok = TRUE; /* No target, is a good target */
      }
      
      if (!target_ok) {
	if (*name) {
	  if(!IS_NPC(ch))
	    send_to_char("Say what?\n\r", ch);
	}
	else { /* Nothing was given as argument */
	  if (spell_info[spl].targets < TAR_OBJ_INV) {
	    if(!IS_NPC(ch)) {
	      send_to_char("Who should the spell ", ch);
	      send_to_char("be cast upon?\n\r", ch);
	    }
	    else
	      if(!IS_NPC(ch)) {
		send_to_char("What should the spell be ", ch);
		send_to_char("cast upon?\n\r", ch);
	      }
	  }
	}
	return;
      }
      else if (IS_NPC(ch) || GET_LEVEL(ch) < (IMO+3)) {
	if ((tar_char == ch) &&
	    IS_SET(spell_info[spl].targets, TAR_SELF_NONO)) {
	  if(!IS_NPC(ch)) {
	    send_to_char("You can not cast this spell ", ch);
	    send_to_char("upon yourself.\n\r", ch);
	  }
	  return;
	}
	else if ((tar_char!=ch) &&
		 IS_SET(spell_info[spl].targets,TAR_SELF_ONLY)) {
	  if(!IS_NPC(ch)) {
	    send_to_char("You can only cast this spell ", ch);
	    send_to_char("upon yourself.\n\r", ch);
	  }
	  return;
	}
	else if (IS_AFFECTED(ch, AFF_CHARM) &&
		 (ch->master == tar_char)) {
	  if(!IS_NPC(ch)) {
	    send_to_char("You are afraid that it could ", ch);
	    send_to_char("harm your master.\n\r", ch);
	  }
	  return;
	}
      }
      if (IS_NPC(ch) || GET_LEVEL(ch) < IMO) {
	if (GET_MANA(ch) < use_mana(ch, spl)) {
	  if(!IS_NPC(ch)) {
	    send_to_char("You can't summon enough ", ch);
	    send_to_char("energy to cast the spell.\n\r", ch);
	  }
	  else {
	    act("$n could not summon enough energy to cast.",
		TRUE,ch,0,0,TO_ROOM);
	  }
	  return;
	}
      }
      if (spl != SPELL_VENTRILOQUATE)  /* :-) */
	say_spell(ch, spl);
      WAIT_STATE(ch, spell_info[spl].beats);
      if ((spell_info[spl].spell_pointer == 0) && spl>0) {
	send_to_char("Sorry, this magic has not yet ", ch);
	send_to_char("been implemented.\n\r", ch);
      }
      else {
	if (number(1,101) > ch->skills[spl].learned) {
	  if(!IS_NPC(ch))
	    send_to_char("You lost your concentration!\n\r", ch);
	  else
	    act("$n lost the concentration!!!",TRUE,ch,0,0,TO_ROOM);
	  GET_MANA(ch) -= (use_mana(ch, spl)>>1);
	  return;
	}
	if (!IS_NPC(ch))
	  send_to_char("Ok.\n\r",ch);
	if (tar_char && (IS_AFFECTED(tar_char,AFF_SPELL_BLOCK) &&
			 number(1,10) > 3 )) {
	  if(!IS_NPC(ch))
	    send_to_char("Your magical power is blocked!", ch);
	  else
	    act("$n failed to cast by spell block!!",
		TRUE,ch,0,0,TO_ROOM);
	  return;	
	}
	if (tar_char && (IS_AFFECTED(tar_char,AFF_DEATH)) &&
	    number(1,10) > 2 ) {
	  if(!IS_NPC(ch))
	    send_to_char("He will die. Let him die. \n\r",ch);
	  else
	    act("$n failed to cast by DEATH!!",
		TRUE,ch,0,0,TO_ROOM);
	  return;
	}
	((*spell_info[spl].spell_pointer) (GET_LEVEL(ch),ch,argument,
					   SPELL_TYPE_SPELL, tar_char, tar_obj));
	GET_MANA(ch) -= (use_mana(ch, spl));
      }
    } /* if GET_POS < min_pos */
    return;
  }
  
  if(!IS_NPC(ch))
    send_to_char("Eh?\n\r", ch);
}

void SPELLO( int nr, byte beat, byte pos, int l0, int l1, int l2, int l3,
             int s0, int s1, int s2, int s3, int mana, int tar,
             void (*func) (byte level, struct char_data *ch, char *arg,
                           int type, struct char_data *tar_ch,
                           struct obj_data *tar_obj))
{
  spell_info[nr].spell_pointer = (func);
  spell_info[nr].beats = (beat);
  spell_info[nr].minimum_position = (pos);
  spell_info[nr].min_usesmana = (mana);
  spell_info[nr].min_level[0] = (l0);
  spell_info[nr].min_level[1] = (l1);
  spell_info[nr].min_level[2] = (l2);
  spell_info[nr].min_level[3] = (l3);
  spell_info[nr].max_skill[0] = (s0);
  spell_info[nr].max_skill[1] = (s1);
  spell_info[nr].max_skill[2] = (s2);
  spell_info[nr].max_skill[3] = (s3);
  spell_info[nr].targets = (tar);
}

void assign_spell_pointers(void)
{
  int i;
  
  for (i=0; i<MAX_SPL_LIST; i++)
    spell_info[i].spell_pointer = 0;
  
  SPELLO(32, 12, POSITION_FIGHTING, 1, IMO, IMO, IMO, 95, 30, 30, 30, 50, 
	 TAR_CHAR_ROOM | TAR_FIGHT_VICT | TAR_SELF_NONO, cast_magic_missile);
  SPELLO(8, 12, POSITION_FIGHTING, 3, 5, 13, 13, 95, 60, 30, 30, 15,
	 TAR_CHAR_ROOM | TAR_FIGHT_VICT | TAR_SELF_NONO, cast_chill_touch);
  SPELLO(5, 12, POSITION_FIGHTING, 5, 8, 15, 15, 95, 60, 30, 30, 15,
	 TAR_CHAR_ROOM | TAR_FIGHT_VICT | TAR_SELF_NONO, cast_burning_hands);
  SPELLO(37, 12, POSITION_FIGHTING, 7, 10, 17, 17, 95, 60, 30, 30, 15,
	 TAR_CHAR_ROOM | TAR_FIGHT_VICT | TAR_SELF_NONO, cast_shocking_grasp);
  SPELLO(30, 12, POSITION_FIGHTING, 9, 13, 19, 19, 95, 60, 30, 30, 20,
	 TAR_CHAR_ROOM | TAR_FIGHT_VICT | TAR_SELF_NONO, cast_lightning_bolt);
  SPELLO(10, 12, POSITION_FIGHTING, 11, 15, 21, 21, 95, 60, 30, 30, 25,
	 TAR_CHAR_ROOM | TAR_FIGHT_VICT | TAR_SELF_NONO, cast_color_spray);
  SPELLO(25, 12, POSITION_FIGHTING, 13, 20, 25, 25, 95, 40, 30, 30, 35,
	 TAR_CHAR_ROOM | TAR_FIGHT_VICT | TAR_SELF_NONO, cast_energy_drain);
  SPELLO(26, 12, POSITION_FIGHTING, 15, IMO, IMO, IMO, 95, 30, 30, 30, 75,
	 TAR_CHAR_ROOM | TAR_FIGHT_VICT | TAR_SELF_NONO, cast_fireball);
  SPELLO(23, 12, POSITION_FIGHTING, IMO, 7, IMO, IMO, 30, 95, 30, 30, 75,
	 TAR_IGNORE, cast_earthquake);
  SPELLO(22, 12, POSITION_FIGHTING, 25, 10, IMO, IMO, 40, 95, 30, 30, 35,
	 TAR_CHAR_ROOM | TAR_FIGHT_VICT | TAR_SELF_NONO, cast_dispel_evil);
  SPELLO(6, 12, POSITION_FIGHTING, 20, IMO, IMO, IMO, 95, 30, 30, 30, 100,
	 TAR_CHAR_ROOM | TAR_FIGHT_VICT | TAR_SELF_NONO, cast_call_lightning);
  SPELLO(27, 12, POSITION_FIGHTING, IMO, 15, IMO, IMO, 30, 95, 30, 30, 50,
	 TAR_CHAR_ROOM | TAR_FIGHT_VICT | TAR_SELF_NONO, cast_harm);
  SPELLO(45, 12, POSITION_FIGHTING, 22, 25, IMO, IMO, 80, 95, 30, 30, 45,
	 TAR_CHAR_ROOM | TAR_FIGHT_VICT | TAR_SELF_NONO, cast_sunburst);
  SPELLO(46, 12, POSITION_STANDING, 30, IMO, IMO, IMO, 95, 30, 30, 30, 50,
	 TAR_CHAR_ROOM | TAR_FIGHT_VICT | TAR_SELF_NONO, cast_clone);
  SPELLO(72, 12, POSITION_FIGHTING, 10, IMO, IMO, IMO, 95, 30, 30, 30, 50,
	 TAR_CHAR_ROOM | TAR_FIGHT_VICT | TAR_SELF_NONO, cast_energyflow);
  SPELLO(75, 36, POSITION_FIGHTING, 17, IMO, IMO, IMO, 95, 30, 30, 30, 150,
	 TAR_CHAR_ROOM | TAR_FIGHT_VICT | TAR_SELF_NONO, cast_full_fire);
  SPELLO(76, 12, POSITION_FIGHTING, 22, 33, IMO, IMO, 95, 60, 30, 30, 150,
	 TAR_CHAR_ROOM | TAR_FIGHT_VICT | TAR_SELF_NONO, cast_throw);
  SPELLO(77, 12, POSITION_FIGHTING, 18, 18, IMO, IMO, 90, 90, 30, 30, 100,
	 TAR_IGNORE, cast_fire_storm);
  
  
  SPELLO(1, 12, POSITION_STANDING, 5, 1, IMO, IMO, 75, 95, 30, 30, 15,
	 TAR_CHAR_ROOM, cast_armor);
  SPELLO(2, 12, POSITION_STANDING, 8, 18, 40, 40, 95, 95, 30, 30, 50,
	 TAR_SELF_ONLY, cast_teleport);
  SPELLO(3, 12, POSITION_STANDING, 25, 5, IMO, IMO, 40, 95, 30, 30, 25,
	 TAR_OBJ_INV | TAR_CHAR_ROOM , cast_bless);
  SPELLO(4, 12, POSITION_STANDING, 8, 6, IMO, IMO, 75, 95, 30, 30, 25,
	 TAR_CHAR_ROOM | TAR_FIGHT_VICT | TAR_SELF_NONO, cast_blindness);
  SPELLO(7, 12, POSITION_STANDING, 20, 15, IMO, IMO, 60, 95, 30, 30, 50,
	 TAR_CHAR_ROOM | TAR_SELF_NONO, cast_charm_person);
  SPELLO(9, 12, POSITION_STANDING, 15, IMO, IMO, IMO, 95, 30, 30, 30, 100,
	 TAR_OBJ_ROOM, cast_reanimate);
  SPELLO(11, 12, POSITION_STANDING, 24, 20, 30, 30, 80, 90, 30, 30, 150,
	 TAR_CHAR_ZONE, cast_relocate);
  SPELLO(12, 12, POSITION_STANDING, 10, 3, 20, 20, 50, 95, 30, 30, 5,
	 TAR_IGNORE, cast_create_food);
  SPELLO(13, 12, POSITION_STANDING, 10, 3, 20, 20, 50, 95, 30, 30, 5,
	 TAR_OBJ_INV, cast_create_water);
  SPELLO(14, 12, POSITION_STANDING, 10, 4, IMO, IMO, 80, 95, 30, 30, 80,
	 TAR_CHAR_ROOM, cast_cure_blind);
  SPELLO(15, 12, POSITION_FIGHTING, 11, 7, 20, 20, 80, 95, 70, 70, 85,
	 TAR_CHAR_ROOM, cast_cure_critic);
  SPELLO(16, 12, POSITION_FIGHTING, 4, 1, 10, 10, 80, 95, 70, 70, 55,
	 TAR_CHAR_ROOM, cast_cure_light);
  SPELLO(17, 12, POSITION_STANDING, 12, 20, IMO, IMO, 90, 60, 30, 30, 50,
	 TAR_CHAR_ROOM | TAR_OBJ_ROOM | TAR_OBJ_INV | TAR_SELF_NONO, cast_curse);
  SPELLO(18, 12, POSITION_STANDING, 20, 4, 30, 30, 50, 95, 30, 30, 50,
	 TAR_SELF_ONLY, cast_detect_evil);
  SPELLO(19, 12, POSITION_STANDING, 2, 5, 20, 20, 95, 95, 70, 70, 50,
	 TAR_SELF_ONLY, cast_detect_invisibility);
  SPELLO(20, 12, POSITION_STANDING, 11, 9, IMO, IMO, 95, 95, 30, 30, 200,
	 TAR_OBJ_INV, cast_recharger);
  SPELLO(21, 12, POSITION_STANDING, IMO, 25, IMO, IMO, 30, 95, 30, 30, 100,
	 TAR_CHAR_ROOM, cast_preach);
  /*	SPELLO(24, 12, POSITION_STANDING, 12, 41, 41, 41, 90, 50, 30, 30, 255,
	TAR_OBJ_INV, cast_enchant_weapon);		*/
  SPELLO(24, 12, POSITION_STANDING, 12, 30, 40, 40, 90, 50, 30, 30, 255,
	 TAR_OBJ_INV, cast_enchant_weapon);
  SPELLO(28, 12, POSITION_FIGHTING, 23, 10, IMO, IMO, 80, 95, 30, 30, 100,
	 TAR_CHAR_ROOM, cast_heal);
  SPELLO(29, 12, POSITION_STANDING, 4, 10, IMO, IMO, 95, 70, 30, 30, 50,
	 TAR_CHAR_ROOM | TAR_OBJ_INV | TAR_OBJ_ROOM, cast_invisibility);
  SPELLO(31, 12, POSITION_STANDING, 6, 10, IMO, IMO, 95, 95, 30, 30, 50,
	 TAR_OBJ_WORLD, cast_locate_object);
  SPELLO(33, 12, POSITION_STANDING, 20, 8, 30, IMO, 70, 95, 60, 30, 70,
	 TAR_CHAR_ROOM | TAR_SELF_NONO | TAR_OBJ_INV, cast_poison);
  SPELLO(34, 12, POSITION_STANDING, 20, 6, IMO, IMO, 60, 95, 30, 30, 50,
	 TAR_SELF_ONLY, cast_protection_from_evil);
  SPELLO(35, 12, POSITION_STANDING, IMO, 12, IMO, IMO, 30, 95, 30, 30, 50,
	 TAR_CHAR_ROOM | TAR_OBJ_INV | TAR_OBJ_ROOM, cast_remove_curse);
  SPELLO(36, 12, POSITION_STANDING, 25, 13, IMO, IMO, 50, 95, 30, 30, 100,
	 TAR_CHAR_ROOM, cast_sanctuary);
  SPELLO(38, 12, POSITION_STANDING, 20, 14, IMO, IMO, 75, 90, 30, 30, 50,
	 TAR_CHAR_ROOM | TAR_SELF_NONO, cast_sleep);
  SPELLO(39, 12, POSITION_STANDING, 10, 7, IMO, IMO, 75, 90, 30, 30, 90,
	 TAR_SELF_ONLY, cast_strength);
  SPELLO(40, 12, POSITION_STANDING, 20, 8, IMO, IMO, 60, 95, 30, 30, 200,
	 TAR_CHAR_WORLD, cast_summon);
  SPELLO(41, 12, POSITION_STANDING, 1, 1, 1, 1, 95, 95, 95, 95, 5,
	 TAR_CHAR_ROOM | TAR_OBJ_ROOM | TAR_SELF_NONO, cast_ventriloquate);
  SPELLO(42, 12, POSITION_STANDING, 20, 11, IMO, IMO, 50, 95, 30, 30, 200,
	 TAR_SELF_ONLY, cast_word_of_recall);
  SPELLO(43, 12, POSITION_STANDING, IMO, 9, 33, IMO, 30, 95, 50, 30, 50,
	 TAR_CHAR_ROOM | TAR_OBJ_INV | TAR_OBJ_ROOM, cast_remove_poison);
  SPELLO(44, 12, POSITION_STANDING, 15, 7, 7, 17, 70, 95, 95, 30, 50,
	 TAR_SELF_ONLY, cast_sense_life);
  SPELLO(71, 12, POSITION_STANDING, 9, 13, 40, 40, 95, 55, 30, 30, 125,
	 TAR_SELF_ONLY, cast_damage_up);
  SPELLO(73, 12, POSITION_STANDING, 15, 12, IMO, IMO, 75, 90, 30, 30, 200,
	 TAR_CHAR_ROOM, cast_mana_boost);
  SPELLO(74, 12, POSITION_STANDING, 17, 15, IMO, IMO, 75, 90, 30, 30, 200,
	 TAR_CHAR_ROOM, cast_vitalize);
  SPELLO(78, 12, POSITION_STANDING, 19, 18, IMO, IMO, 75, 90, 30, 30, 100,
	 TAR_CHAR_ROOM | TAR_FIGHT_VICT | TAR_SELF_NONO, cast_crush_armor);
  SPELLO(79, 12, POSITION_FIGHTING, IMO, 21, IMO, IMO, 30, 95, 30, 30, 200,
	 TAR_CHAR_ROOM, cast_full_heal);
  SPELLO(80, 12, POSITION_STANDING, 25, IMO, IMO, IMO, 95, 30, 30, 30, 100,
	 TAR_CHAR_ROOM | TAR_FIGHT_VICT | TAR_SELF_NONO, cast_trick);
  SPELLO(81, 12, POSITION_STANDING, 17, 20, 40, 40, 95, 95, 30, 30, 50,
	 TAR_CHAR_ROOM, cast_haste);
  SPELLO(82, 12, POSITION_STANDING, 30, IMO, IMO, IMO, 95, 95, 30, 30, 100,
	 TAR_SELF_ONLY, cast_improved_haste);
  SPELLO(83, 12, POSITION_STANDING, 30, 5, 40, 40, 65, 95, 30, 30, 50,
	 TAR_CHAR_WORLD, cast_far_look);
  SPELLO(84, 12, POSITION_FIGHTING, IMO, 30, IMO, IMO, 30, 95, 30, 30, 200,
	 TAR_IGNORE, cast_all_heal);
  SPELLO(53, 1, POSITION_STANDING, 7, 5, 10, 10, 95, 95, 95, 95, 30,
	 TAR_OBJ_INV, cast_identify);
  
  /* 55 : sneak */
  SPELLO(55, 1, POSITION_STANDING, IMO, IMO, 1, IMO, 30, 30, 95, 30, 0,
	 TAR_IGNORE, 0);
  /* 56 : hide */
  SPELLO(56, 1, POSITION_STANDING, IMO, IMO, 1, IMO, 30, 30, 95, 30, 0,
	 TAR_IGNORE, 0);
  /* 57 : steal */
  SPELLO(57, 1, POSITION_STANDING, IMO, IMO, 1, IMO, 30, 30, 95, 30, 0,
	 TAR_IGNORE, 0);
  /* 58 : backstab */
  SPELLO(58, 1, POSITION_STANDING, IMO, IMO, 1, IMO, 30, 30, 95, 30, 0,
	 TAR_IGNORE, 0);
  /* 59 : pick */
  SPELLO(59, 1, POSITION_STANDING, 20, 20, 1, 10, 30, 30, 95, 30, 0,
	 TAR_IGNORE, 0);
  /* 60 : kick */
  SPELLO(60, 1, POSITION_STANDING, IMO, IMO, IMO, 1, 30, 30, 30, 90, 0,
	 TAR_IGNORE, 0);
  /* 61 : bash */
  SPELLO(61, 1, POSITION_STANDING, IMO, IMO, IMO, 1, 30, 30, 30, 90, 0,
	 TAR_IGNORE, 0);
  /* 62 : rescue */
  SPELLO(62, 1, POSITION_STANDING, 30, 30, 10, 1, 30, 30, 50, 90, 0,
	 TAR_IGNORE, 0);
  /* 63 : double attack */
  SPELLO(63, 1, POSITION_STANDING, IMO, IMO, 10, 8, 30, 30, 100, 100, 0,
	 TAR_IGNORE, 0);
  /* 64 : quad attack */
  SPELLO(64, 1, POSITION_STANDING, IMO, IMO, 27, 15, 30, 30, 100, 100, 0,
	 TAR_IGNORE, 0);
  /* 65 : extra damaging */
  SPELLO(65, 1, POSITION_STANDING, IMO, IMO, IMO, 25, 30, 30, 30, 100, 0,
	 TAR_IGNORE, 0);
  /* 85 : tornado */
  SPELLO(85, 1, POSITION_STANDING, IMO, IMO, 20, 13, 30, 30, 75, 90, 0,
	 TAR_IGNORE, 0);
  /* 86 : lightning move */
  SPELLO(86, 1, POSITION_STANDING, IMO, IMO, 1, IMO, 30, 30, 95, 30, 0,
	 TAR_IGNORE, 0);
  /* 87 : parry */
  SPELLO(87, 1, POSITION_STANDING, IMO, IMO, 1, 19, 30, 30, 95, 50, 0,
	 TAR_IGNORE, 0);
  /* 88 : flash */
  SPELLO(88, 1, POSITION_STANDING, IMO, IMO, 5, IMO, 30, 30, 95, 30, 0,
	 TAR_IGNORE, 0);
  /* 89 : multi kick */
  SPELLO(89, 1, POSITION_STANDING, IMO, IMO, IMO, 17, 30, 30, 30, 95, 0,
	 TAR_IGNORE, 0);
  
  SPELLO(90, 12, POSITION_STANDING, 30, IMO, IMO, IMO, 90, 30, 30, 30, 255,
	 TAR_OBJ_INV, cast_enchant_armor);
  SPELLO(91, 12, POSITION_STANDING, IMO, 30, IMO, IMO, 30, 90, 30, 30, 255,
	 TAR_OBJ_INV, cast_pray_for_armor);
  SPELLO(92, 12, POSITION_STANDING, 7, 5, IMO, IMO, 95, 95, 30, 30, 30,
	 TAR_SELF_ONLY, cast_infravision);
  SPELLO(93, 12, POSITION_STANDING, IMO, 25, IMO, IMO, 30, 95, 30, 30, 150,
	 TAR_OBJ_INV, cast_create_nectar);
  SPELLO(94, 12, POSITION_FIGHTING, IMO, 33, IMO, IMO, 30, 95, 30, 30, 255,
	 TAR_OBJ_INV, cast_create_golden_nectar);
  SPELLO(95, 12, POSITION_FIGHTING, 30, IMO, IMO, IMO, 95, 30, 30, 30, 150,
	 TAR_CHAR_ROOM | TAR_FIGHT_VICT | TAR_SELF_NONO, cast_corn_of_ice);
  SPELLO(96, 12, POSITION_FIGHTING, 33, IMO, IMO, IMO, 95, 30, 30, 30, 150,
	 TAR_CHAR_ROOM | TAR_FIGHT_VICT | TAR_SELF_NONO, cast_disintegrate);
  SPELLO(97, 12, POSITION_STANDING, 20, 14, IMO, IMO, 75, 95, 30, 30, 50,
	 TAR_IGNORE, cast_phase);
  SPELLO(98, 12, POSITION_STANDING, 35, IMO, IMO, IMO, 95, 30, 30, 30, 100,
	 TAR_SELF_ONLY, cast_mirror_image);
  SPELLO(99, 12, POSITION_FIGHTING, IMO, 35, IMO, IMO, 30, 95, 30, 30, 200,
	 TAR_IGNORE, cast_hand_of_god);
  SPELLO(100, 12, POSITION_FIGHTING, IMO, 1, IMO, IMO, 30, 95, 30, 30, 15,
	 TAR_CHAR_ROOM | TAR_FIGHT_VICT | TAR_SELF_NONO, cast_cause_light);
  SPELLO(101, 12, POSITION_FIGHTING, IMO, 7, IMO, IMO, 30, 95, 30, 30, 15,
	 TAR_CHAR_ROOM | TAR_FIGHT_VICT | TAR_SELF_NONO, cast_cause_critic);
  SPELLO(102, 19, POSITION_FIGHTING, IMO, IMO, IMO, IMO, 30, 30, 30, 30, 255,
	 TAR_CHAR_ROOM, cast_entire_heal);
  /*
    SPELLO(102, 19, POSITION_FIGHTING, IMO, 30, IMO, IMO, 30, 30, 30, 30, 255,
    TAR_CHAR_ROOM, cast_entire_heal);
    */
  
  /* 103 : octa attack */
  SPELLO(103, 1, POSITION_STANDING, IMO, IMO, IMO, 20, 30, 30, 30, 100, 0,
	 TAR_IGNORE, 0);
  SPELLO(104, 12, POSITION_STANDING, IMO, 13, IMO, IMO, 30, 95, 30, 30, 100,
	 TAR_CHAR_ROOM, cast_love);
  /* 105 : disarm */
  SPELLO(105, 1, POSITION_FIGHTING, IMO, IMO, 1, 10, 30, 30, 39, 30, 0,
	 TAR_CHAR_ROOM, 0);
  SPELLO(106, 12, POSITION_STANDING, IMO, 40, IMO, IMO, 30, 75, 30, 30, 255,
	 TAR_SELF_ONLY, cast_reraise);
  /* 107 : shouryuken */
  SPELLO(107, 1, POSITION_FIGHTING, IMO, IMO, IMO, 30, 30, 30, 30, 95, 0,
	 TAR_CHAR_ROOM, 0);
  /* 108 : throw object */
  SPELLO(108, 1, POSITION_FIGHTING, IMO, IMO, 30, IMO, 30, 30, 95, 30, 0,
	 TAR_CHAR_ROOM, 0);
  /* 109 : punch */
  SPELLO(109, 1, POSITION_FIGHTING, IMO, IMO, 30, 25, 30, 30, 50, 95, 0,
	 TAR_CHAR_ROOM, 0);
  SPELLO(110, 12, POSITION_STANDING, 30, 30, 30, 30, 95, 95, 95, 95, 250,
	 TAR_SELF_ONLY, cast_death); /* death by process */
  SPELLO(111, 12, POSITION_STANDING, 30, 30, 40, 40, 70, 70, 30, 30, 200,
	 TAR_SELF_ONLY, cast_enchant_person); /* by wings */
  SPELLO(112, 12, POSITION_STANDING, 30, 20, IMO, IMO, 70, 80, 30, 30, 200,
	 TAR_CHAR_ROOM, cast_spell_block); /* by process */
  /* 113 : temptation */
  SPELLO(113, 1, POSITION_STANDING, 10, 10, 10, 10, 39, 39, 39, 39, 0,
	 TAR_CHAR_ROOM, 0); /* temptation by process */
  SPELLO(115, 12, POSITION_SITTING, 30, 20, IMO, IMO, 70, 90, 30, 30, 255,
	 TAR_CHAR_ROOM, cast_mana_transfer); /* by process */
  SPELLO(116, 12, POSITION_FIGHTING, 17, 13, 20, 20, 80, 95, 70, 70, 100,
	 TAR_SELF_ONLY, cast_self_heal); /* by process */
  SPELLO(117, 12, POSITION_STANDING, IMO, 40, IMO, IMO, 30, 80, 30, 30, 120,
	 TAR_SELF_ONLY, cast_holy_shield); /* by process */
  SPELLO(118, 12, POSITION_FIGHTING, IMO, IMO, 30, 40, 30, 30, 49, 30, 200,
	 TAR_SELF_ONLY, cast_restore_move); /* by ares */
  SPELLO(119, 12, POSITION_STANDING, IMO, IMO, IMO, IMO, 30, 39, 30, 30, 200,
	 TAR_IGNORE,cast_kiss_of_process); /* by process */
  SPELLO(120, 12, POSITION_STANDING, 35, 30, IMO, IMO, 50, 80, 30, 30, 200,
	 TAR_SELF_ONLY, cast_reflect_damage); /* by process */
  SPELLO(121, 12, POSITION_STANDING, 30, 30, IMO, IMO, 39, 39, 30, 30, 255,
	 TAR_CHAR_ROOM, cast_dumb);
  /* 122 : spin bird kick */
  SPELLO(122, 1, POSITION_FIGHTING, IMO, IMO, IMO, 30, 30, 30, 30, 95, 0,
	 TAR_CHAR_ROOM, 0);
  SPELLO(123, 12, POSITION_STANDING, 30, IMO, IMO, IMO, 39, 30, 30, 30, 200,
	 TAR_IGNORE, cast_thunder_bolt);
  /* 125 : sanctuary cloud */
  SPELLO(125, 12, POSITION_STANDING, IMO, 40, IMO, IMO, 30, 80, 30, 30, 255,
	 TAR_IGNORE, cast_sanctuary_cloud);
}
