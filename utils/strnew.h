#include <sys/types.h>

typedef short sbyte;
typedef unsigned char ubyte;
typedef short sh_int;
typedef unsigned short ush_int;
typedef char bool;
typedef char byte;
#define MAX_SKILLS  150
#define MAX_AFFECT  25

struct char_ability_data {
  sbyte str;
  sbyte str_add;
  sbyte intel;
  sbyte wis;
  sbyte dex;
  sbyte con;
};

struct char_point_data {
  sh_int mana;
  sh_int max_mana;     /* Not useable may be erased upon player file renewal */
  int hit;
  int max_hit;      /* Max hit for NPC                         */
  sh_int move;
  sh_int max_move;     /* Max move for NPC                        */
  sh_int armor;        /* Internal -100..100, external -10..10 AC */
  unsigned int gold;            /* Money carried                           */
  int exp;             /* The experience of the player            */
  sbyte hitroll;       /* Any bonus or penalty to the hit roll    */
  sbyte damroll;       /* Any bonus or penalty to the damage roll */
};
struct char_skill_data {
  byte learned;           /* % chance for success 0 = not learned   */
  bool recognise;         /* If you can recognise the scroll etc.   */
};
struct affected_type {
  sbyte type;           /* The type of spell that caused this      */
  sh_int duration;      /* For how long its effects will last      */
  sbyte modifier;       /* This is added to apropriate ability     */
  byte location;        /* Tells which ability to change(APPLY_XXX)*/
  int bitvector;       /* Tells which bits to set (AFF_XXX)       */
  int next;
};
struct new_affected_type {
  sbyte type;           /* The type of spell that caused this      */
  sh_int duration;      /* For how long its effects will last      */
  sbyte modifier;       /* This is added to apropriate ability     */
  byte location;        /* Tells which ability to change(APPLY_XXX)*/
  int bitvector;       /* Tells which bits to set (AFF_XXX)       */
  struct affected_type *next;
};

struct char_file_u {
  byte sex,class,level;
  time_t birth;
  int played,life;
  ubyte weight,height;
  char title[80];
  char description[240];
  sh_int load_room;
  struct char_ability_data abilities;
  struct char_point_data points;
  struct char_skill_data skills[MAX_SKILLS];
  struct affected_type affected[MAX_AFFECT];
  byte spells_to_learn;
  int alignment;
  time_t last_logon;
  unsigned act;
  int bank;
  char name[20];
  char pwd[11];
  int conditions[3];
  int	quest[2] ;
  int reserved[18];	/* reserved field for extension */
};

struct new_char_file_u {
  byte sex,class,level;
  time_t birth;
  int played,life;
  ubyte weight,height;
  char title[80];
  char description[240];
  sh_int load_room;
  struct char_ability_data abilities;
  struct char_point_data points;
  struct char_skill_data skills[MAX_SKILLS];
  struct new_affected_type affected[MAX_AFFECT];
  byte spells_to_learn;
  int alignment;
  time_t last_logon;
  unsigned act;
  int bank;
  char name[20];
  char pwd[11];
  int conditions[3];
  int	quest[2] ;
  int reserved[18];	/* reserved field for extension */
};
