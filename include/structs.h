#include <sys/types.h>
#include <stdlib.h>

// Changed all bytes and short to int or unsigned
typedef short sbyte;
typedef unsigned ubyte;
typedef int sh_int;
typedef unsigned ush_int;
typedef char bool;
typedef unsigned short byte;

typedef long long LONGLONG;

#define IMO		41

/*
#define PULSE_MOBILE    41
*/
#define PULSE_MOBILE    39
#define PULSE_MOBILE2   13

#define PULSE_ZONE     240
#define PULSE_VIOLENCE  12
#define WAIT_SEC       4
#define WAIT_ROUND     4

#define MAX_STRING_LENGTH   2000
#define MAX_OUTPUT_LENGTH    512
#define MAX_INPUT_LENGTH     200
#define MAX_MESSAGES          61
#define MAX_ITEMS            153

#define MESS_ATTACKER 1
#define MESS_VICTIM   2
#define MESS_ROOM     3

#define SECS_PER_REAL_MIN  60
#define SECS_PER_REAL_HOUR (60*SECS_PER_REAL_MIN)
#define SECS_PER_REAL_DAY  (24*SECS_PER_REAL_HOUR)
#define SECS_PER_REAL_YEAR (365*SECS_PER_REAL_DAY)

#define SECS_PER_MUD_HOUR  60
#define SECS_PER_MUD_DAY   (24*SECS_PER_MUD_HOUR)
#define SECS_PER_MUD_MONTH (35*SECS_PER_MUD_DAY)
#define SECS_PER_MUD_YEAR  (17*SECS_PER_MUD_MONTH)

/* The following defs are for obj_data  */

/* For 'type_flag' */

#define ITEM_LIGHT      1
#define ITEM_SCROLL     2
#define ITEM_WAND       3
#define ITEM_STAFF      4
#define ITEM_WEAPON     5
#define ITEM_FIREWEAPON 6
#define ITEM_MISSILE    7
#define ITEM_TREASURE   8
#define ITEM_ARMOR      9
#define ITEM_POTION    10
#define ITEM_WORN      11
#define ITEM_OTHER     12
#define ITEM_TRASH     13
#define ITEM_TRAP      14
#define ITEM_CONTAINER 15
#define ITEM_NOTE      16
#define ITEM_DRINKCON  17
#define ITEM_KEY       18
#define ITEM_FOOD      19
#define ITEM_MONEY     20
#define ITEM_PEN       21
#define ITEM_BOAT      22
#define ITEM_CIGA	   23
#define ITEM_DRUG      24

/* Bitvector For 'wear_flags' */

#define ITEM_TAKE              1
#define ITEM_WEAR_FINGER       2
#define ITEM_WEAR_NECK         4
#define ITEM_WEAR_BODY         8
#define ITEM_WEAR_HEAD        16
#define ITEM_WEAR_LEGS        32
#define ITEM_WEAR_FEET        64
#define ITEM_WEAR_HANDS      128
#define ITEM_WEAR_ARMS       256
#define ITEM_WEAR_SHIELD     512
#define ITEM_WEAR_ABOUT     1024
#define ITEM_WEAR_WAISTE    2048
#define ITEM_WEAR_WRIST     4096
#define ITEM_WIELD          8192
#define ITEM_HOLD          16384
#define ITEM_THROW         32768
#define ITEM_WEAR_KNEE     65536
#define ITEM_WEAR_ABOUTLEGS 131072
#define ITEM_WEAR_BACK	262144 /* added by process */

/* Bitvector for 'extra_flags' */

#define ITEM_GLOW            1
#define ITEM_HUM             2
#define ITEM_DARK            4
#define ITEM_LOCK            8
#define ITEM_EVIL           16
#define ITEM_INVISIBLE      32
#define ITEM_MAGIC          64
#define ITEM_NODROP        128
#define ITEM_BLESS         256
#define ITEM_ANTI_GOOD     512 /* not usable by good people    */
#define ITEM_ANTI_EVIL    1024 /* not usable by evil people    */
#define ITEM_ANTI_NEUTRAL 2048 /* not usable by neutral people */
#define ITEM_NOLOAD       4096
#define ITEM_ANTI_MAGE    8192 /* not usable by mage */
#define ITEM_ANTI_CLERIC 16384 /* not usable by cleric */
#define ITEM_ANTI_THIEF  32768 /* not usable by thief */
#define ITEM_ANTI_WARRIOR 65536 /* not usable by warrior */
#define ITEM_NORENT		131072 /* cannot rent ,added by process */
#define ITEM_ANTI_POLICE	262144 /* not usable by police */
#define ITEM_ANTI_OUTLAW	524288 /* not usable by outlaw */
#define ITEM_ANTI_ASSASSIN	1048576/* not usable by assasin */
#define ITEM_NOCOPY		2097152 /* do not SAVE in the locker room */

/* Some different kind of liquids */
#define LIQ_WATER      0
#define LIQ_BEER       1
#define LIQ_WINE       2
#define LIQ_ALE        3
#define LIQ_DARKALE    4
#define LIQ_WHISKY     5
#define LIQ_LEMONADE   6
#define LIQ_FIREBRT    7
#define LIQ_LOCALSPC   8
#define LIQ_SLIME      9
#define LIQ_MILK       10
#define LIQ_TEA        11
#define LIQ_COFFE      12
#define LIQ_BLOOD      13
#define LIQ_SALTWATER  14
#define LIQ_SUNDEW     15
#define LIQ_NECTAR     16
#define LIQ_GOLDEN_NECTAR     17
#define LIQ_MAKOLI	18
#define LIQ_CONDITIOIN 19

/* for containers  - value[1] */

#define CONT_CLOSEABLE      1
#define CONT_PICKPROOF      2
#define CONT_CLOSED         4
#define CONT_LOCKED         8

struct extra_descr_data
{
  char *keyword;                 /* Keyword in look/examine          */
  char *description;             /* What to see                      */
  struct extra_descr_data *next; /* Next in list                     */
};

#define MAX_OBJ_AFFECT 2         /* Used in OBJ_FILE_ELEM *DO*NOT*CHANGE* */
#define OBJ_NOTIMER    -7000000

struct obj_flag_data
{
  int value[4];       /* Values of the item (see list)    */
  byte type_flag;     /* Type of item                     */
  int wear_flags;     /* Where you can wear it            */
  int extra_flags;    /* If it hums,glows etc             */
  int weight;         /* Weigt what else                  */
  int cost;           /* Value when sold (gp.)            */
  int timer;          /* Timer for object                 */
  long bitvector;     /* To set chars bits                */
  int gpd;            /* general purpose data             */
};

/* Used in OBJ_FILE_ELEM *DO*NOT*CHANGE* */
struct obj_affected_type {
  byte location;      /* Which ability to change (APPLY_XXX) */
  sbyte modifier;     /* How much it changes by              */
};

/* ======================== Structure for object ========================= */
struct obj_data
{
  sh_int item_number;            /* Where in data-base               */
  sh_int in_room;                /* In what room -1 when conta/carr  */
  struct obj_flag_data obj_flags;/* Object information               */
  struct obj_affected_type
      affected[MAX_OBJ_AFFECT];  /* Which abilities in PC to change  */
  char *name;                    /* Title of object :get etc.        */
  char *description ;            /* When in room                     */
  char *short_description;       /* when worn/carry/in cont.         */
  char *action_description;      /* What to write when used          */
  struct extra_descr_data *ex_description; /* extra descriptions     */
  struct char_data *carried_by;  /* Carried by :NULL in room/conta   */

  struct obj_data *in_obj;       /* In what object NULL when none    */
  struct obj_data *contains;     /* Contains objects                 */

  struct obj_data *next_content; /* For 'contains' lists             */
  struct obj_data *next;         /* For the object list              */
};
/* ======================================================================= */

/* The following defs are for room_data  */

#define NOWHERE    -1    /* nil reference for room-database    */

/* Bitvector For 'room_flags' */

#define DARK           1
#define NOSUMMON       2
#define NO_MOB         4
#define INDOORS        8
#define LAWFUL        16
#define NEUTRAL       32
#define CHAOTIC       64
#define NO_MAGIC     128
#define TUNNEL       256
#define PRIVATE      512
#define OFF_LIMITS  1024
#define RENT        2048
#define NORELOCATE  4096
#define EVERYZONE   8192    /* tar char to all zone */
#define JAIL		16384 /* added by process */
#define RESTROOM	32768 /* added by process */

/* For 'dir_option' */

#define NORTH          0
#define EAST           1
#define SOUTH          2
#define WEST           3
#define UP             4
#define DOWN           5

#define EX_ISDOOR    1
#define EX_CLOSED    2
#define EX_LOCKED    4
#define EX_RSCLOSED  8
#define EX_RSLOCKED  16
#define EX_PICKPROOF 32
#define EX_NOPHASE   64

/* For 'sector types' */

#define SECT_INSIDE          0
#define SECT_CITY            1
#define SECT_FIELD           2
#define SECT_FOREST          3
#define SECT_HILLS           4
#define SECT_MOUNTAIN        5
#define SECT_WATER_SWIM      6
#define SECT_WATER_NOSWIM    7
#define SECT_SKY             8

struct room_direction_data
{
  char *general_description;       /* When look DIR.                  */
  char *keyword;                   /* for open/close                  */
  sh_int exit_info;                /* Exit info                       */
  sh_int key;                       /* Key's number (-1 for no key)    */
  sh_int to_room;                  /* Where direction leeds (NOWHERE) */
};

/* ========================= Structure for room ========================== */
struct room_data
{
  sh_int number;               /* Rooms number                       */
  sh_int zone;                 /* Room zone (for resetting)          */
  int sector_type;             /* sector type (move/hide)            */
  char *name;                  /* Rooms name 'You are ...'           */
  char *description;           /* Shown when entered                 */
  struct extra_descr_data *ex_description; /* for examine/look       */
  struct room_direction_data *dir_option[6]; /* Directions           */
  int room_flags;           /* DEATH,DARK ... etc                 */
  byte light;                  /* Number of lightsources in room     */
  int (*funct)();              /* special procedure                  */

  struct obj_data *contents;   /* List of items in room              */
  struct char_data *people;    /* List of NPC / PC in room           */
};
/* ======================================================================== */

/* The following defs and structures are related to char_data   */

/* For 'equipment' */

#define WEAR_LIGHT      0
#define WEAR_FINGER_R   1
#define WEAR_FINGER_L   2
#define WEAR_NECK_1     3
#define WEAR_NECK_2     4
#define WEAR_BODY       5
#define WEAR_HEAD       6
#define WEAR_LEGS       7
#define WEAR_FEET       8
#define WEAR_HANDS      9
#define WEAR_ARMS      10
#define WEAR_SHIELD    11
#define WEAR_ABOUT     12
#define WEAR_WAISTE    13
#define WEAR_WRIST_R   14
#define WEAR_WRIST_L   15
#define WIELD          16
#define HOLD           17
#define WEAR_KNEE_R    18
#define WEAR_KNEE_L    19
#define WEAR_ABOUTLEGS 20
#define WEAR_BACK      21

/* For 'char_payer_data' */
#define MAX_GUILD_SKILLS 30
#define MAX_SKILLS  200  /* Used in CHAR_FILE_U DO NOT CHANGE */
#define MAX_WEAR    22
#define MAX_AFFECT	32    /* Used in CHAR_FILE_U *DO*NOT*CHANGE* */

#define DRUNK        0
#define FULL         1
#define THIRST       2

/* Bitvector for 'affected_by' */
#define AFF_BLIND             1
#define AFF_INVISIBLE         2
#define AFF_DETECT_EVIL       4
#define AFF_DETECT_INVISIBLE  8
#define AFF_INFRAVISION       16
#define AFF_SENSE_LIFE        32
#define AFF_DEAF				64 /* by process */
#define AFF_SANCTUARY         128
#define AFF_GROUP             256
#define AFF_DUMB			  512 /* by process */
#define AFF_CURSE             1024
#define AFF_MIRROR_IMAGE      2048
#define AFF_POISON            4096
#define AFF_PROTECT_EVIL      8192
#define AFF_REFLECT_DAMAGE	16384 /* by process */
#define AFF_HOLY_SHIELD			32768
#define AFF_SPELL_BLOCK       65536 /* by process */
#define AFF_SLEEP             131072
#define AFF_SHADOW_FIGURE     262144 /* by process */
#define AFF_SNEAK             524288
#define AFF_HIDE              1048576
#define AFF_DEATH			  2097152
#define AFF_CHARM             4194304
#define AFF_FOLLOW            8388608
#define AFF_HASTE	          16777216
#define AFF_IMPROVED_HASTE    33554432
#define AFF_LOVE              67108864 /* by perhaps */
#define AFF_RERAISE           134217728 /* by chase */
#define AFF_ARREST            268435456 /* by sypark */
/* modifiers to char's abilities */

#define APPLY_NONE              0
#define APPLY_STR               1
#define APPLY_DEX               2
#define APPLY_INT               3
#define APPLY_WIS               4
#define APPLY_CON               5
#define APPLY_SEX               6
#define APPLY_CLASS             7
#define APPLY_LEVEL             8
#define APPLY_AGE               9
#define APPLY_CHAR_WEIGHT      10
#define APPLY_CHAR_HEIGHT      11
#define APPLY_MANA             12
#define APPLY_HIT              13
#define APPLY_MOVE             14
#define APPLY_GOLD             15
#define APPLY_EXP              16
#define APPLY_AC               17
#define APPLY_ARMOR            17
#define APPLY_HITROLL          18
#define APPLY_DAMROLL          19
#define APPLY_SAVING_PARA      20
#define APPLY_SAVING_HIT_SKILL 21
#define APPLY_SAVING_PETRI     22
#define APPLY_SAVING_BREATH    23
#define APPLY_SAVING_SPELL     24
#define APPLY_REGENERATION     25
#define APPLY_INVISIBLE        26

/* 'class' for PC's */
#define CLASS_MAGIC_USER  1
#define CLASS_CLERIC      2
#define CLASS_THIEF       3
#define CLASS_WARRIOR     4

/* 'class' for NPC's */
#define CLASS_OTHER       0 /* These are not yet used!   */
#define CLASS_UNDEAD      1 /* But may soon be so        */
#define CLASS_HUMANOID    2 /* Idea is to use general    */
#define CLASS_ANIMAL      3 /* monster classes           */
#define CLASS_DRAGON      4 /* Then for example a weapon */
#define CLASS_GIANT       5 /* of dragon slaying is pos. */
#define CLASS_DEMON		  6
#define CLASS_INSECT	  8

/* sex */
#define SEX_NEUTRAL   0
#define SEX_MALE      1
#define SEX_FEMALE    2

/* positions */
#define POSITION_DEAD       0
#define POSITION_MORTALLYW  1
#define POSITION_INCAP      2
#define POSITION_STUNNED    3
#define POSITION_SLEEPING   4
#define POSITION_RESTING    5
#define POSITION_SITTING    6 /* POSITION_VEHICLE??? */
#define POSITION_FIGHTING   7
#define POSITION_STANDING   8
#define POSITION_SWIMMING   9 /* swim should be before stand, */
#define POSITION_LEVITATED  10 /* just a bit above the ground */
#define POSITION_FLYING     11 /* go upwards */

/* for mobile actions: specials.act */
#define ACT_SPEC         1     /* special routine to be called if exist   */
#define ACT_SENTINEL     2     /* this mobile not to be moved             */
#define ACT_SCAVENGER    4     /* pick up stuff lying around              */
#define ACT_ISNPC        8     /* This bit is set for use with IS_NPC()   */
#define ACT_NICE_THIEF  16     /* Set if a thief should NOT be killed     */
#define ACT_AGGRESSIVE  32     /* Set if automatic attack on NPC's        */
#define ACT_STAY_ZONE   64     /* MOB Must stay inside its own zone       */
#define ACT_WIMPY      128     /* MOB Will flee when injured, and if      */
                               /* aggressive only attack sleeping players */
#define ACT_FIGHTER    256
#define ACT_MAGE       512
#define ACT_CLERIC    1024
#define ACT_THIEF     2048
#define ACT_PALADIN   4096
#define ACT_DRAGON    8192
#define ACT_SPITTER  16384
#define ACT_SHOOTER  32768
#define ACT_GUARD    65536
#define ACT_SUPERGUARD   131072
#define ACT_GIANT        262144
#define ACT_HELPER       524288		/* changed by cyb */
#define ACT_ORC          524288		/* changed by cyb */
#define ACT_RESCUER     1048576
#define ACT_SPELL_BLOCKER  2097152
#define ACT_FINISH_BLOW    4194304

/* For players : specials.act */
#define PLR_BRIEF        1
#define PLR_NOSHOUT      2
#define PLR_COMPACT      4
#define PLR_DONTSET      8   /* Dont EVER set */
#define PLR_NOTELL      16
#define PLR_BANISHED    32
#define PLR_CRIMINAL    64
#define PLR_WIZINVIS   128
#define PLR_EARMUFFS   256
#define PLR_XYZZY      512
#define PLR_AGGR      1024
#define PLR_KOREAN    2048
#define PLR_NOCHAT      4096
#define PLR_WIMPY       8192
#define PLR_DUMB_BY_WIZ 16384
#define PLR_NOCHAT_BY_WIZ 32768
#define PLR_SOLO		65536

struct time_info_data {
  byte hours, day, month;
  sh_int year;
};

struct time_data {
  time_t birth;
  time_t logon;
  int played;
};

/* remortal */
#define REMORTAL_MAGIC_USER		1
#define REMORTAL_CLERIC			2
#define REMORTAL_THIEF			4
#define REMORTAL_WARRIOR		8

struct char_player_data {
  char *name;
  char *short_descr;
  char *long_descr;
  char *description;
  char *title;
  byte guild; /* which guild you are joining */
  int no_of_change_guild;
  int skilled;
  ubyte guild_skills[MAX_GUILD_SKILLS]; /* by process */
  int pk_num; /* how many did you kill assholes! */
  int pked_num; /* how many have you been killed!! */
  byte sex;
  byte class;         /* PC s class or NPC alignment */
  byte level;         /* PC / NPC s level                     */
  struct time_data time;  /* PC s AGE in days                 */
  ubyte weight;
  ubyte height;
  byte remortal;	/* remortal to class */
};


/* Used in CHAR_FILE_U *DO*NOT*CHANGE* */
struct char_ability_data {
  sbyte str;
  sbyte str_add;
  sbyte intel;
  sbyte wis;
  sbyte dex;
  sbyte con;
};

/* Used in CHAR_FILE_U *DO*NOT*CHANGE* */
struct char_point_data {
  int mana;
  int max_mana;     /* Not useable may be erased upon player file renewal */
  int hit;
  int max_hit;      /* Max hit for NPC                         */
  int move;
  int max_move;     /* Max move for NPC                        */
  int armor;        /* Internal -100..100, external -10..10 AC */
  LONGLONG gold;            /* Money carried                           */
  LONGLONG exp;             /* The experience of the player            */
  sbyte hitroll;       /* Any bonus or penalty to the hit roll    */
  sbyte damroll;       /* Any bonus or penalty to the damage roll */
};

struct char_special_data {
  struct char_data *fighting;
  struct char_data *hunting;
  long affected_by;
  byte position;
  byte default_pos;
  unsigned act;
  ubyte spells_to_learn;    /* How many can you learn yet this level   */
  int carry_weight;
  byte carry_items;
  int timer;               /* Timer for update                        */
  sh_int was_in_room;      /* storage of location for linkdead people */
  int apply_saving_throw[5];
  int conditions[3];      /* Drunk full etc.                        */
  ubyte damnodice;           /* The number of damage dice's            */
  ubyte damsizedice;         /* The size of the damage dice's          */
  byte last_direction;      /* The last direction the monster went    */
  int attack_type;          /* The Attack Type Bitvector for NPC's    */
  int alignment;            /* +-1000 for alignments                  */
  char reply_who[30];
  /* belows are for future use */
  int arena; 	/* arena flag */
  int arena_move;
  int arena_mana;
  int arena_hits;
  int jail_time;
  struct char_data *arrest_by;
  struct char_data *arrest_link;
  int wimpyness;
};

/* Used in CHAR_FILE_U *DO*NOT*CHANGE* */
struct char_skill_data {
  ubyte learned;           /* % chance for success 0 = not learned   */
  ubyte skilled;
  bool recognise;         /* If you can recognise the scroll etc.   */
};

/* Used in CHAR_FILE_U *DO*NOT*CHANGE* */
struct affected_type {
  sbyte type;           /* The type of spell that caused this      */
  int duration;      /* For how long its effects will last      */
  sbyte modifier;       /* This is added to apropriate ability     */
  byte location;        /* Tells which ability to change(APPLY_XXX)*/
  long bitvector;       /* Tells which bits to set (AFF_XXX)       */
  struct affected_type *next;
};

struct follow_type {
  struct char_data *follower;
  struct follow_type *next;
};

/* quest data */
struct quest_data {
	int type;
	int data;
	long solved;
};

struct char_data {
  int nr;
  sh_int in_room;
  int magic_number;
//  unsigned long bank;
  LONGLONG bank;
  int life,regeneration;

  /* for mobile */
  /* it's regened... */
  int regened;

  struct char_player_data player;
  struct char_ability_data abilities;
  struct char_ability_data tmpabilities;
  struct char_point_data points;
  struct char_special_data specials;
  struct char_skill_data skills[MAX_SKILLS];
  struct quest_data quest;
  struct affected_type *affected;
  struct obj_data *equipment[MAX_WEAR];
  struct obj_data *carrying;
  struct descriptor_data *desc;
  struct char_data *next_in_room;
  struct char_data *next;
  struct char_data *next_fighting;
  struct follow_type *followers;
  struct char_data *master;
};

#define SUN_DARK  0
#define SUN_RISE  1
#define SUN_LIGHT  2
#define SUN_SET    3

/* And how is the sky ? */

#define SKY_CLOUDLESS  0
#define SKY_CLOUDY  1
#define SKY_RAINING  2
#define SKY_LIGHTNING  3

struct weather_data {
  int pressure;  /* How is the pressure ( Mb ) */
  int change;  /* How fast and what way does it change. */
  int sky;  /* How is the sky. */
  int sunlight;  /* And how much sun. */
};

struct char_file_u {
  byte sex,class,level;
  time_t birth;
  int played,life;
  ubyte weight,height;
  char title[80];
  ubyte guild_skills[MAX_GUILD_SKILLS];
  char description[300];
  int pked_num;
  int no_of_change_guild;
  byte guild;
  int pk_num;
  sh_int load_room;
  struct char_ability_data abilities;
  struct char_point_data points;
  struct char_skill_data skills[MAX_SKILLS];
  struct affected_type affected[MAX_AFFECT];
  struct quest_data quest;
  ubyte spells_to_learn;
  int alignment;
  time_t last_logon;
  unsigned act;
  LONGLONG bank;
  char name[20];
  char pwd[11];
  int conditions[3];
  byte remortal;
  int wimpyness;

  /* new attribute */
  byte damnodice;
  byte damsizedice;

  /* 60 - 2 byte */
  byte reserved[58];
};

struct txt_block {
  char *text;
  struct txt_block *next;
};

struct txt_q {
  struct txt_block *head;
  struct txt_block *tail;
};

/* modes of connectedness */

#define CON_PLYNG   0
#define CON_NME     1
#define CON_NMECNF  2
#define CON_PWDNRM  3
#define CON_PWDGET  4
#define CON_PWDCNF  5
#define CON_QSEX    6
#define CON_RMOTD   7
#define CON_SLCT    8
#define CON_EXDSCR  9
#define CON_QCLASS  10
#define CON_LDEAD   11
#define CON_PWDNEW  12
#define CON_PWDNEW1 13
#define CON_PWDNCNF 14
#define CON_QSTATE	15
#define CON_SET		16
#define CON_DELCNF	17
#define CON_DELCNF2	18
#define CON_IMOTD   19

struct snoop_data {
  struct char_data *snooping;
  struct char_data *snoop_by;
};

#define MAX_LAST_INPUT_COUNT 10

struct descriptor_data {
  int descriptor;        /* file descriptor for socket */
  char host[16];         /* hostname                   */
  int ncmds,contime;
  char pwd[12];          /* password                   */
  int pos;               /* position in player-file    */
  int connected;         /* mode of 'connectedness'    */
  int wait;              /* wait for how many loops    */
  char *showstr_head;    /* for paging through texts  */
  char *showstr_point;

  char **str;            /* for the modify-str system  */
  int max_str;           /* -                          */

  /* new attribute!!! for posting */
  char *title;

  int prompt_mode;       /* control of prompt-printing */
  char buf[MAX_STRING_LENGTH];  /* buffer for raw input       */
  char last_input[MAX_LAST_INPUT_COUNT][MAX_INPUT_LENGTH]; /* for use of !! */
  int last_input_count;
  struct txt_q output;          /* q of strings to send       */
  struct txt_q input;           /* q of unprocessed input     */
  struct char_data *character;  /* linked to char             */
  struct char_data *original;   /* original char              */
  struct snoop_data snoop;      /* to snoop people.           */
  struct descriptor_data *next; /* link to next descriptor    */
};

struct msg_type {
  char *attacker_msg;  /* message to attacker */
  char *victim_msg;    /* message to victim   */
  char *room_msg;      /* message to room     */
};

struct message_type {
  struct msg_type die_msg;      /* messages when death            */
  struct msg_type miss_msg;     /* messages when miss             */
  struct msg_type hit_msg;      /* messages when hit              */
  struct msg_type sanctuary_msg;/* messages when hit on sanctuary */
  struct msg_type god_msg;      /* messages when hit on god       */
  struct message_type *next;/* to next messages of this kind.*/
};

struct message_list {
  int a_type;               /* Attack type              */
  int number_of_attacks;    /* How many attack messages to chose from. */
  struct message_type *msg; /* List of messages.         */
};

struct dex_skill_type {
  int p_pocket;
  int p_locks;
  int traps;
  int sneak;
  int hide;
};

struct dex_app_type {
  int reaction;
  int attack;
  int defensive;
};

struct str_app_type {
  int tohit;    /* To Hit (THAC0) Bonus/Penalty        */
  int todam;    /* Damage Bonus/Penalty                */
  int carry_w;  /* Maximum weight that can be carrried */
  int wield_w;  /* Maximum weight that can be wielded  */
};

struct wis_app_type {
  byte bonus;       /* how many bonus skills a player can */
                    /* practice pr. level                 */
};

struct int_app_type {
  byte learn;       /* how many % a player learns a spell/skill */
};

struct con_app_type {
  int hitp;
  int shock;
};
