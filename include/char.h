/*********************************************************************
 *  File : char.h  char_data related structures and definitions      *
 *********************************************************************/

#include "typedef.h"

/* The following defs and structures are related to char_data   */ 

/* ---------------     For 'char_payer_data'     --------------*/

#define MAX_SKILLS		200	/* Used in CHAR_FILE_U DO NOT CHANGE */
#define MAX_WEAR    		22
#define MAX_AFFECT		32	/* Used in CHAR_FILE_U *DO*NOT*CHANGE */
#define MAX_GUILD_SKILLS 	30
#define MAX_GUILD_LIST		3

struct time_data {
    time_t birth;
    time_t logon;
    int played;
};

struct char_player_data {
    char *name;
    char *short_descr;
    char *long_descr;
    char *description;
    char *title;
    ubyte guild;			/* which guild you are joining */
    /* NOTE: Not used member. */
    /* ubyte no_of_change_guild; */ 
    /* int skilled; */
    ubyte guild_skills[MAX_GUILD_SKILLS];	/* by process */
    sh_int pk_num;			/* how many did you kill assholes! */
    sh_int pked_num;		/* how many have you been killed!! */
    ubyte sex;
    ubyte class;			/* PC s class or NPC alignment */
    ubyte level;			/* PC / NPC s level*/
    struct time_data time;	/* PC s AGE in days */
    ubyte weight;
    ubyte height;
    ubyte remortal;		/* remortal to class */
}; 

/* 'class' for PC's */
#define CLASS_MAGIC_USER	1
#define CLASS_CLERIC		2
#define CLASS_THIEF		3
#define CLASS_WARRIOR		4

/* 'class' for NPC's */
#define CLASS_OTHER		0	/* These are not yet used!   */
#define CLASS_UNDEAD		1	/* But may soon be so        */
#define CLASS_HUMANOID		2	/* Idea is to use general    */
#define CLASS_ANIMAL		3	/* monster classes           */
#define CLASS_DRAGON		4	/* Then for example a weapon */
#define CLASS_GIANT		5	/* of dragon slaying is pos. */
#define CLASS_DEMON		6
#define CLASS_INSECT		8

/* sex */
#define SEX_NEUTRAL		0
#define SEX_MALE		1
#define SEX_FEMALE		2

/* remortal */
#define REMORTAL_MAGIC_USER	1
#define REMORTAL_CLERIC		2
#define REMORTAL_THIEF		4
#define REMORTAL_WARRIOR	8

/* guild */
#define POLICE			1
#define OUTLAW  		2
#define ASSASSIN 		3

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
    int max_mana;	/* Not useable may be erased upon player file renewal */
    int hit;
    int max_hit;	/* Max hit for NPC			   */
    int move;
    int max_move;	/* Max move for NPC			   */
    int armor;		/* Internal -100..100, external -10..10 AC */
    unsigned long gold;	/* Money carried			   */
    unsigned long exp;	/* The experience of the player		   */
    sbyte hitroll;	/* Any bonus or penalty to the hit roll    */
    sbyte damroll;	/* Any bonus or penalty to the damage roll */
};

/* ---------------     struct char_special_data     --------------- */

struct char_special_data {
    struct char_data *fighting;
    struct char_data *hunting;
    /* NOTE: Now used. Set by pointing or target flee while fighting */
    long affected_by;
    ubyte position;
    ubyte default_pos;
    unsigned long act;
    sbyte spells_to_learn;	/* How many can you learn yet this level   */
    sh_int carry_weight;
    byte carry_items;
    int timer;		/* Timer for update			  */
    sh_int was_in_room;	/* storage of location for linkdead people */
    sbyte apply_saving_throw[5];
    sbyte conditions[3];	/* Drunk full etc.			  */
    ubyte damnodice;	/* The number of damage dice's		  */
    ubyte damsizedice;	/* The size of the damage dice's	  */
    ubyte last_direction;	/* The last direction the monster went    */
    ubyte attack_type;	/* The Attack Type Bitvector for NPC's    */
    sh_int alignment;	/* +-1000 for alignments		  */
    /* char reply_who[30]; */
    /* NOTE: sizeof( struct char_file_u.name ) == 20 */
    char reply_who[20];
    time_t jail_time;	/* NOTE: Now used. Time to be released from jail  */
    /* int jail_time ; */
    int wimpyness;
#ifdef NO_DEF
    /* belows are for future use */
    int arena;		/* arena flag */
    int arena_move;
    int arena_mana;
    int arena_hits;
    struct char_data *arrest_by;
    struct char_data *arrest_link;
#endif
}; 

#define SAVING_PARA   		0
#define SAVING_HIT_SKILL	1
#define SAVING_PETRI		2
#define SAVING_BREATH		3
#define SAVING_SPELL		4

#define DRUNK        0
#define FULL         1
#define THIRST       2

/* positions */
/* NOTE: #define POSITION_XXX -> POS_XXX */
#define POS_DEAD		0
#define POS_MORTALLYW		1
#define POS_INCAP		2
#define POS_STUNNED		3
#define POS_SLEEPING		4
#define POS_RESTING		5
#define POS_SITTING		6    /* POS_VEHICLE??? */
#define POS_FIGHTING		7
#define POS_STANDING		8
#define POS_SWIMMING		9     /* swim should be before stand,*/
#define POS_LEVITATED		10    /* just a bit above the ground */
#define POS_FLYING		11    /* go upwards */

/* Bitvector for 'affected_by' */

#define AFF_BLIND		1
#define AFF_INVISIBLE		2
#define AFF_DETECT_EVIL		4
#define AFF_DETECT_INVISIBLE	8
#define AFF_INFRAVISION		16
#define AFF_SENSE_LIFE		32
#define AFF_DEAF		64		/* by process */
#define AFF_SANCTUARY		128
#define AFF_GROUP		256
#define AFF_DUMB		512		/* by process */
#define AFF_CURSE		1024
#define AFF_MIRROR_IMAGE	2048
#define AFF_POISON		4096
#define AFF_PROTECT_EVIL	8192
#define AFF_REFLECT_DAMAGE	16384		/* by process */
#define AFF_HOLY_SHIELD		32768
#define AFF_SPELL_BLOCK		65536		/* by process */
#define AFF_SLEEP		131072
#define AFF_SHADOW_FIGURE	262144		/* by process */
#define AFF_SNEAK		524288
#define AFF_HIDE		1048576
#define AFF_DEATH		2097152
#define AFF_CHARM		4194304
#define AFF_FOLLOW		8388608
#define AFF_HASTE		16777216
#define AFF_IMPROVED_HASTE	33554432
#define AFF_LOVE		67108864	/* by perhaps */
#define AFF_RERAISE		134217728	/* by chase */
#define AFF_ARREST		268435456	/* by sypark */
#define AFF_NOTRACK		536870912	/* NOTE: Woun't be 'track'ed */

/* for mobile actions: specials.act */

#define ACT_SPEC		1    /* special routine to be called if exist */
#define ACT_SENTINEL		2    /* this mobile not to be moved	      */
#define ACT_SCAVENGER		4    /* pick up stuff lying around	      */
#define ACT_ISNPC		8    /* This bit is set for use with IS_NPC() */
#define ACT_NICE_THIEF		16   /* Set if a thief should NOT be killed   */
#define ACT_AGGRESSIVE		32   /* Set if automatic attack on NPC's      */
#define ACT_STAY_ZONE		64   /* MOB Must stay inside its own zone     */
#define ACT_WIMPY		128  /* MOB Will flee when injured, and if    */
				/* aggressive only attack sleeping players    */
#define ACT_FIGHTER		256
#define ACT_MAGE  		512
#define ACT_CLERIC		1024
#define ACT_THIEF		2048
#define ACT_PALADIN		4096
#define ACT_DRAGON		8192
#define ACT_SPITTER		16384
#define ACT_SHOOTER		32768
#define ACT_GUARD		65536
#define ACT_SUPERGUARD		131072
#define ACT_GIANT		262144
#define ACT_HELPER		524288		/* changed by cyb */
#define ACT_ORC			524288		/* changed by cyb */
#define ACT_RESCUER		1048576
#define ACT_SPELL_BLOCKER	2097152
#define ACT_FINISH_BLOW		4194304

/* For players : specials.act */

/* NOTE: Re-organized PLR_???? bit position.  */
#define PLR_RESERVED1		1
#define PLR_WIZINVIS		2
#define PLR_XYZZY		4
#define PLR_DONTSET		8		/* Dont EVER set */
#define PLR_RESERVED2		16
/* NOTE: PLR_SHUTUP is OLD PLR_NOSHOUT (prohibit shouting) */
#define PLR_SHUTUP		32 
#define PLR_BANISHED		64
#define PLR_DUMB_BY_WIZ		128

#define PLR_WIMPY		256
#define PLR_SOLO		512 
/* NOTE: PLR_NOSHOUT is OLD PLR_EARMUFFS (Won't hear shouting) */
#define PLR_NOSHOUT		1024
#define PLR_NOCHAT		2048
#define PLR_NOTELL		4096
#define PLR_BRIEF		8192
#define PLR_COMPACT		16384
#define PLR_KOREAN		32768 

/* NOTE: NEW! Player will open door upon unlocking it. */
#define PLR_AUTOOPEN		65536 
#define PLR_CRIMINAL		131072
#define PLR_RESERVED4		262144
#define PLR_RESERVED5		524288 

/* #define PLR_AGGR		0 */
/* #define PLR_NOCHAT_BY_WIZ	0 */

/* -----------   end of struct char_special_data  -------------- */ 

/* Used in CHAR_FILE_U *DO*NOT*CHANGE* */
struct affected_type {
    sbyte type;		/* The type of spell that caused this      */
    int duration;	/* For how long its effects will last      */
    sbyte modifier;	/* This is added to apropriate ability     */
    /* NOTE: byte -> ubyte.  Hmm. */
    ubyte location;	/* Tells which ability to change(APPLY_XXX) */
    long bitvector;	/* Tells which bits to set (AFF_XXX)       */
    struct affected_type *next;
};

/*  For 'struct affected_type.location'  modifiers to char's abilities */

#define APPLY_NONE		0
#define APPLY_STR		1
#define APPLY_DEX		2
#define APPLY_INT		3
#define APPLY_WIS		4
#define APPLY_CON		5
#define APPLY_SEX		6
#define APPLY_CLASS		7
#define APPLY_LEVEL		8
#define APPLY_AGE		9
#define APPLY_CHAR_WEIGHT	10
#define APPLY_CHAR_HEIGHT	11
#define APPLY_MANA		12
#define APPLY_HIT		13
#define APPLY_MOVE		14
#define APPLY_GOLD		15
#define APPLY_EXP		16
#define APPLY_AC		17
#define APPLY_ARMOR		17
#define APPLY_HITROLL		18
#define APPLY_DAMROLL		19
#define APPLY_SAVING_PARA	20
#define APPLY_SAVING_HIT_SKILL  21
#define APPLY_SAVING_PETRI	22
#define APPLY_SAVING_BREATH	23
#define APPLY_SAVING_SPELL	24
#define APPLY_REGENERATION	25
#define APPLY_INVISIBLE		26

/* -------------    End of 'struct affected_type  --------------- */
/* -------------    Other char_type member        --------------- */

/* Used in CHAR_FILE_U *DO*NOT*CHANGE* */
struct char_skill_data {
    ubyte learned;	/* % chance for success 0 = not learned   */
    ubyte skilled;
    bool recognise;	/* If you can recognise the scroll etc.   */
};

/* Used in CHAR_FILE_U *DO*NOT*CHANGE* */
/* quest data */
struct quest_data {
    int type;
    int data;
    int solved;
};

struct follow_type {
    struct char_data *follower;
    struct follow_type *next;
}; 

struct char_data {
    int nr;
    sh_int in_room;
    int magic_number;
    unsigned long bank;
    int life, regeneration;

    /* for mobile */ /* it's regened... */
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

/* For 'struct char_data.equipment' */

#define WEAR_LIGHT		0
#define WEAR_FINGER_R		1
#define WEAR_FINGER_L		2
#define WEAR_NECK_1		3
#define WEAR_NECK_2		4
#define WEAR_BODY		5
#define WEAR_HEAD		6
#define WEAR_LEGS		7
#define WEAR_FEET		8
#define WEAR_HANDS		9
#define WEAR_ARMS		10
#define WEAR_SHIELD		11
#define WEAR_ABOUT		12
#define WEAR_WAISTE		13
#define WEAR_WRIST_R		14
#define WEAR_WRIST_L		15
#define WIELD			16
#define HOLD			17
#define WEAR_KNEE_R		18
#define WEAR_KNEE_L		19
#define WEAR_ABOUTLEGS		20
#define WEAR_BACK		21


/*************    END of struct char_data    *****************************/
/* ---------    struct char_data member acccess macros      ------------ */

#define GET_POS(ch)     ((ch)->specials.position)

#define GET_COND(ch, i) ((ch)->specials.conditions[(i)])

#define GET_NAME(ch)    ((ch)->player.name)

#define GET_TITLE(ch)   ((ch)->player.title)

#define GET_LEVEL(ch)   ((ch)->player.level)

#define GET_CLASS(ch)   ((ch)->player.class)

#define GET_STR(ch)     ((ch)->tmpabilities.str)

#define GET_ADD(ch)     ((ch)->tmpabilities.str_add)

#define GET_DEX(ch)     ((ch)->tmpabilities.dex)

#define GET_INT(ch)     ((ch)->tmpabilities.intel)

#define GET_WIS(ch)     ((ch)->tmpabilities.wis)

#define GET_CON(ch)     ((ch)->tmpabilities.con)

#define GET_AC(ch)      ((ch)->points.armor)

#define GET_HIT(ch)     ((ch)->points.hit)

#define GET_MAX_HIT(ch) (hit_limit(ch))

#define GET_PLAYER_MAX_HIT(ch) ((ch)->points.max_hit)

#define GET_MOVE(ch)    ((ch)->points.move)

#define GET_MAX_MOVE(ch) (move_limit(ch))

#define GET_PLAYER_MAX_MOVE(ch) ((ch)->points.max_move)

#define GET_MANA(ch)    ((ch)->points.mana)

#define GET_MAX_MANA(ch) (mana_limit(ch))

#define GET_PLAYER_MAX_MANA(ch) ((ch)->points.max_mana)

#define GET_GOLD(ch)    ((ch)->points.gold)

#define GET_EXP(ch)     ((ch)->points.exp)

#define GET_SEX(ch)     ((ch)->player.sex)

#define GET_HITROLL(ch) ((ch)->points.hitroll)

#define GET_DAMROLL(ch) ((ch)->points.damroll)

#define GET_ALIGNMENT(ch) ((ch)->specials.alignment)

#define GET_ACT(ch) ((ch)->specials.act)

/* NOTE: NEW! */
#define GET_FIGHTING(ch) ((ch)->specials.fighting)
#define GET_HUNTING(ch) ((ch)->specials.hunting)

/* NOTE: Remove rarely used macros */
/* #define GET_REGENERATION(ch) ((ch)->regeneration) */ 
/* #define GET_HEIGHT(ch)  ((ch)->player.height) */ 
/* #define GET_WEIGHT(ch)  ((ch)->player.weight) */
/* #define GET_AGE(ch)     (age(ch).year)  */ 

/* ------------       Predicates for char      ---------------- */

#define IS_NPC(ch)  (IS_SET((ch)->specials.act, ACT_ISNPC))

/* #define IS_MOB(ch)  (IS_SET((ch)->specials.act, ACT_ISNPC) && ((ch)->nr >-1))*/
/* NOTE: IS_MOB(): Real mobile, Not wizard switched to NPC.  */
#define IS_MOB(ch)  (IS_SET((ch)->specials.act, ACT_ISNPC) && !((ch)->desc))

#define AWAKE(ch) (GET_POS(ch) > POS_SLEEPING)

#define IS_AFFECTED(ch,skill) ( IS_SET((ch)->specials.affected_by, (skill)) )

#define IS_ACTPLR( ch, flag ) ( IS_SET(GET_ACT(ch), flag )) 

#define IS_GOOD(ch)    (GET_ALIGNMENT(ch) >= 350)
#define IS_EVIL(ch)    (GET_ALIGNMENT(ch) <= -350)
#define IS_NEUTRAL(ch) (!IS_GOOD(ch) && !IS_EVIL(ch))

/* ----------------------------------------------------------- */
/* guild related definitions made by Process */

#define GET_GUILD(x) ((x)->player.guild)
#define GET_GUILD_SKILL(x,i) ((x)->player.guild_skills[(i)]) 
/* NOTE: Remove unused macros */
/* #define GET_GUILD_SKILLS(x,i) ((x)->player.guild_skills[(i)]) */
/* #define GET_GUILD_NAME(i) (guild_names[(i)]) */
#define GET_PK(x) ((x)->player.pk_num)

/* quest */
#define GET_QUEST_TYPE(ch) (ch->quest.type)
#define GET_QUEST_DATA(ch) (ch->quest.data)
#define GET_QUEST_SOLVED(ch) (ch->quest.solved)

#define STRENGTH_APPLY_INDEX(ch) \
        ( ((GET_ADD(ch)==0) || (GET_STR(ch) != 18)) ? GET_STR(ch) :\
          (GET_ADD(ch) <= 50) ? 26 :( \
          (GET_ADD(ch) <= 75) ? 27 :( \
          (GET_ADD(ch) <= 90) ? 28 :( \
          (GET_ADD(ch) <= 99) ? 29 :  30 ) ) ))


/* NOTE: NEW! get virtual number of mob */
#define GET_MOB_VIRTUAL(ch) (mob_index[(ch)->nr].virtual)

/* NOTE: CAN_SEE() is now function, not macro.  See library.c */
#ifdef NO_DEF
/* Can subject see character "obj"? */
#define OMNI(sub) (!IS_NPC(sub) && (GET_LEVEL(sub) >= IMO))
#define X99(sub, obj)   ( ((!IS_AFFECTED((obj),AFF_INVISIBLE) || \
                                IS_AFFECTED((sub),AFF_DETECT_INVISIBLE)) && \
                               !IS_AFFECTED((sub),AFF_BLIND) ) && \
               (IS_NPC(obj) || !IS_SET((obj)->specials.act,PLR_WIZINVIS)) && \
   (OMNI(sub) || IS_LIGHT(sub->in_room) || IS_AFFECTED(sub,AFF_INFRAVISION)))

#define CAN_SEE(sub,obj) (X99(sub,obj)||(OMNI(sub) && (IS_NPC(obj) || \
   (GET_LEVEL(sub) >= GET_LEVEL(obj)))))
#endif		/* NO_DEF  */

/*************    External variables and proc()'s    *****************/

extern struct index_data *mob_index;
extern struct char_data *character_list;

/* Public Procedures */
extern int mana_limit(struct char_data *ch);
extern int hit_limit(struct char_data *ch);
extern int move_limit(struct char_data *ch);

extern struct time_info_data age(struct char_data *ch);

/* ----------------------------------------------------------------- */
