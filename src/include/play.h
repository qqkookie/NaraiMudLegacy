
#include "typedef.h"

#define TAR_IGNORE        1
#define TAR_CHAR_ROOM     2
#define TAR_CHAR_WORLD    4
#define TAR_FIGHT_SELF    8
#define TAR_FIGHT_VICT   16
#define TAR_SELF_ONLY    32	/* Only a check, use with ei. TAR_CHAR_ROOM */
#define TAR_SELF_NONO    64	/* Only a check, use with ei. TAR_CHAR_ROOM */
#define TAR_SELF_DEF    128  	/* NOTE: Default is myself + TAR_CHAR_ROOM */
#define TAR_OBJ_INV     256
#define TAR_OBJ_ROOM    512
#define TAR_OBJ_WORLD  1024
#define TAR_OBJ_EQUIP  2048
#define TAR_CHAR_ZONE  4096

struct spell_info_type {
    void (*spell_pointer) (byte level, struct char_data * ch, char *arg,
    	    int type, struct char_data * tar_ch, struct obj_data * tar_obj);
/* cyb
   void (*spell_pointer) ();
 */
    byte minimum_position;	/* Position for caster              */
    ubyte min_usesmana;		/* Amount of mana used by a spell   */
    byte beats;			/* Heartbeats until ready for next */
    byte min_level[4];
    byte max_skill[4];
    int targets;		/* See below for use with TAR_XXX  */
    char *name;			/* NOTE: Spell name string. Not used yet. */
};

/* Possible Targets:

   bit 0 : IGNORE TARGET
   bit 1 : PC/NPC in room
   bit 2 : PC/NPC in world
   bit 3 : Object held
   bit 4 : Object in inventory
   bit 5 : Object in room
   bit 6 : Object in world
   bit 7 : If fighting, and no argument, select tar_char as self
   bit 8 : If fighting, and no argument, select tar_char as victim (fighting)
   bit 9 : If no argument, select self, if argument check that it IS self.

 */

#define SPELL_TYPE_SPELL   0
#define SPELL_TYPE_POTION  1
#define SPELL_TYPE_WAND    2
#define SPELL_TYPE_STAFF   3
#define SPELL_TYPE_SCROLL  4

/* ----------------------------------------------------------------- */
/* NOTE: Also defined in "spells.h"	*/ 

#define TYPE_UNDEFINED               -1
#define SPELL_CHARM_PERSON            7
#define SPELL_POISON                 33
#define SPELL_SLEEP                  38
#define SKILL_SNEAK                  55
#define SKILL_HIDE                   56
#define SKILL_STEAL                  57
#define SKILL_PICK_LOCK              59
#define SKILL_BASH                   61
#define SKILL_RESCUE                 62
#define SKILL_LIGHT_MOVE             86
#define SKILL_DISARM		    105
#define SPELL_DEATH		    110		/* by process */
#define SPELL_SHADOW_FIGURE	    114		/* by process */
#define SKILL_ARREST		    124

#define TYPE_HIT		    150
#define TYPE_MISC		    191		/* NO MESSAGES  */ 
#define TYPE_SUFFERING		    200 

/* ----------------------------------------------------------------- */
/* NOTE: Move following type, defintion to "fight.c"
	#define MAX_MESSAGES          61
	struct msg_type ;
	struct message_type ;
	struct message_list;
	struct attack_hit_type;
*/ 

int WAIT_STATE(struct char_data *ch, int cycle);

/* NOTE: PULSE_VIOLENCE defined in "comm.c"	*/ 
#define PULSE_VIOLENCE		12 

/* skilled */
#define GET_LEARNED(ch, sk_no)		((ch)->skills[(sk_no)].learned)
#define GET_SKILLED(ch, sk_no)		((ch)->skills[(sk_no)].skilled)

/* defined in utility.c */
extern void print_increased_skilled(struct char_data *ch, int sk_no);
extern void increase_skilled(struct char_data *ch, struct char_data *victim,
			     int sk_no, int mul, int add);

/* NOTE: INCREASE_SKILLED(ch,victim, sk_no)'s are too big for macro. 
   Use function version for space efficency             */
/*      probability = 1/( skilled(ch, sk_no) * mul + add )      */
/*  NOTE: Guild membership check for PK is deleted. It is pointless */
/*  NOTE: It checks victim is pet to prevent skill drill by robot play 
   in petshop   - by Hjpark */
/*  NOTE: increase_skilled() is in utility.c. See FYI: there for details */

#define INCREASE_SKILLED(ch, victim, sk_no) \
	increase_skilled(ch, victim, sk_no, 2, 99 )

/* more hard increased */
#define INCREASE_SKILLED1(ch, victim, sk_no) \
	increase_skilled(ch, victim, sk_no, 4, 250 )

/* more easily increased */
#define INCREASE_SKILLED2(ch, victim, sk_no)	\
	increase_skilled(ch, victim, sk_no, 1, 29 )

/*******************  Define for MOB magic ********************/
/*
   From: mob_magic.h
   this file is for mob's efficient attact..
   by choi yeon boem
 */

/* fight mode */
#define	VIC_FIGHTING	0	/* victim who is fighting */
#define VIC_WATCHING	1	/* victim who just watching */
#define	VIC_ALL		2	/* victim all fight or not .. */

/* mode */
#define MODE_RANDOM	0	/* choose anyone */
#define	MODE_HIT_MIN	1	/* whose hit point is minimum */
#define	MODE_MANA_MIN	2	/* whose mana point is minimum */
#define	MODE_MOVE_MIN	3	/* whose move point is minimum */
#define	MODE_AC_MAX	4	/* poorest armor */
#define MODE_HR_MIN	5	/* low hitroll */
#define MODE_DR_MIN	6	/* low damroll */

#define	MODE_HIT_MAX	7	/* whose hit point is minimum */
#define	MODE_MANA_MAX	8	/* whose mana point is minimum */
#define	MODE_MOVE_MAX	9	/* whose move point is minimum */
#define	MODE_AC_MIN	10	/* poorest armor */
#define MODE_HR_MAX	11	/* low hitroll */
#define MODE_DR_MAX	12	/* low damroll */

#define MODE_HIGH_LEVEL	13	/* level 31 - 40 */
#define MODE_MID_LEVEL	14	/* level 13 - 30 */
#define MODE_LOW_LEVEL	15	/* level 1 - 12 */

#define MODE_LEVEL_MAX	16	/* max level.. */
#define MODE_LEVEL_MIN	16	/* min level.. */ 

/* ------------  for PLAYER / MOB balancing ------------------- */ 
/* NOTE: from constant.c	*/

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

struct wis_app_type {
    byte bonus;		/* how many bonus skills a player can */
			/* practice pr. level                 */
};

struct int_app_type {
    byte learn;		/* how many % a player learns a spell/skill */
};

struct con_app_type {
    int hitp;
    int shock;
};

struct dam_weapon_type {
    char *to_room;
    char *to_char;
    char *to_victim;
};

/* ----------------------------------------------------------------- */

extern struct dex_skill_type dex_app_skill[]; 
extern struct int_app_type int_app[];
extern struct wis_app_type wis_app[];
extern struct dex_app_type dex_app[];
extern struct con_app_type con_app[];

extern struct spell_info_type spell_info[]; 

extern void hit(struct char_data *ch, struct char_data *victim, int type); 
extern void stop_fighting(struct char_data *ch);

extern void first_attack(struct char_data *ch, struct char_data *victim); 
extern void die(struct char_data *ch, int level, struct char_data *who);
extern void damage(struct char_data *ch, struct char_data *victim, 
	int damage, int weapontype);

extern bool saves_spell(struct char_data *ch, int spell);

extern bool prohibit_violence(struct char_data *ch);

/* ----------------------------------------------------------------- */
