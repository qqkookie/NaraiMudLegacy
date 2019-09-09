/* Bitvector for 'affected_by' */
#define AFF_BLIND             1
#define AFF_INVISIBLE         2
#define AFF_DETECT_EVIL       4
#define AFF_DETECT_INVISIBLE  8
#define AFF_INFRAVISION       16
#define AFF_SENSE_LIFE        32
#define AFF_DEAF              64 /* by process */
#define AFF_SANCTUARY         128
#define AFF_GROUP             256
#define AFF_DUMB              512 /* by process */
#define AFF_CURSE             1024
#define AFF_MIRROR_IMAGE      2048
#define AFF_POISON            4096
#define AFF_PROTECT_EVIL      8192
#define AFF_REFLECT_DAMAGE    16384 /* by process */
#define AFF_HOLY_SHIELD       32768
#define AFF_SPELL_BLOCK       65536 /* by process */
#define AFF_SLEEP             131072
#define AFF_SHADOW_FIGURE     262144 /* by process */
#define AFF_SNEAK             524288
#define AFF_HIDE              1048576
#define AFF_DEATH             2097152
#define AFF_CHARM             4194304
#define AFF_FOLLOW            8388608
#define AFF_HASTE             16777216
#define AFF_IMPROVED_HASTE    33554432
#define AFF_LOVE              67108864 /* by perhaps */
#define AFF_RERAISE           134217728 /* by chase */
#define AFF_ARREST            268435456 /* by sypark */

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
#define CLASS_DEMON	  6
#define CLASS_INSECT      8

/* sex */
#define SEX_NEUTRAL	0
#define SEX_MALE	1
#define SEX_FEMALE	2

/* for mobile actions: specials.act */
#define ACT_SPEC		1     /* special routine to be called if exist   */
#define ACT_SENTINEL		2     /* this mobile not to be moved             */
#define ACT_SCAVENGER		4     /* pick up stuff lying around              */
#define ACT_ISNPC		8     /* This bit is set for use with IS_NPC()   */
#define ACT_NICE_THIEF		16     /* Set if a thief should NOT be killed     */
#define ACT_AGGRESSIVE		32     /* Set if automatic attack on NPC's        */
#define ACT_STAY_ZONE		64     /* MOB Must stay inside its own zone       */
#define ACT_WIMPY		128     /* MOB Will flee when injured, and if      */
                               /* aggressive only attack sleeping players */
#define ACT_FIGHTER		256
#define ACT_MAGE		512
#define ACT_CLERIC		1024
#define ACT_THIEF		2048
#define ACT_PALADIN		4096
#define ACT_DRAGON		8192
#define ACT_SPITTER		16384
#define ACT_SHOOTER		32768
#define ACT_GUARD		65536
#define ACT_SUPERGUARD		131072
#define ACT_GIANT		262144
#define ACT_HELPER		524288         /* changed by cyb */
#define ACT_ORC			524288         /* changed by cyb */
#define ACT_RESCUER		1048576
#define ACT_SPELL_BLOCKER	2097152
#define ACT_FINISH_BLOW		4194304

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


/*
#V-NUM
name list~
short desc~
long desc~
~
mob desc
~
special_act	affected_by	alignment
class	level	hitpoint	armor	hr	dr
gold	xp	position	sex

*/
struct char_data {
  int vnum;
  char *mob_name;
  char *short_descr;
  char *long_descr;
  char *mob_descr;
  long special_act;
  long affected_by;
  int alignment;
  char class;
  char level;
  int hitpoint;
  int armor;
  int hr;
  int dr;
  long long  gold;
  long long xp;
  char position;
  char sex;
};
