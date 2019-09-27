/* ************************************************************************
*  file: spells.h , Implementation of magic spells.       Part of DIKUMUD *
*  Usage : Spells                                                         *
************************************************************************* */

#define TYPE_UNDEFINED               -1
#define SPELL_RESERVED_DBC            0  /* SKILL NUMBER ZERO */
#define SPELL_ARMOR                   1
#define SPELL_TELEPORT                2
#define SPELL_BLESS                   3
#define SPELL_BLINDNESS               4
#define SPELL_BURNING_HANDS           5
#define SPELL_CALL_LIGHTNING          6
#define SPELL_CHARM_PERSON            7
#define SPELL_CHILL_TOUCH             8
#define SPELL_REANIMATE               9
#define SPELL_COLOUR_SPRAY           10 
#define SPELL_RELOCATE               11 
#define SPELL_CREATE_FOOD            12 
#define SPELL_CREATE_WATER           13 
#define SPELL_CURE_BLIND             14 
#define SPELL_CURE_CRITIC            15 
#define SPELL_CURE_LIGHT             16 
#define SPELL_CURSE                  17 
#define SPELL_DETECT_EVIL            18 
#define SPELL_DETECT_INVISIBLE       19 
#define SPELL_RECHARGER              20 
#define SPELL_PREACH                 21 
#define SPELL_DISPEL_EVIL            22 
#define SPELL_EARTHQUAKE             23 
#define SPELL_ENCHANT_WEAPON         24 
#define SPELL_ENERGY_DRAIN           25 
#define SPELL_FIREBALL               26 
#define SPELL_HARM                   27 
#define SPELL_HEAL                   28 
#define SPELL_INVISIBLE              29 
#define SPELL_LIGHTNING_BOLT         30 
#define SPELL_LOCATE_OBJECT          31 
#define SPELL_MAGIC_MISSILE          32 
#define SPELL_POISON                 33 
#define SPELL_PROTECT_FROM_EVIL      34 
#define SPELL_REMOVE_CURSE           35 
#define SPELL_SANCTUARY              36 
#define SPELL_SHOCKING_GRASP         37 
#define SPELL_SLEEP                  38 
#define SPELL_STRENGTH               39 
#define SPELL_SUMMON                 40 
#define SPELL_VENTRILOQUATE          41 
#define SPELL_WORD_OF_RECALL         42
#define SPELL_REMOVE_POISON          43
#define SPELL_SENSE_LIFE             44
#define SPELL_SUNBURST               45
#define SPELL_CLONE                  46

#define SPELL_DAMAGE_UP              71
#define SPELL_ENERGY_FLOW            72
#define SPELL_MANA_BOOST             73
#define SPELL_VITALIZE               74
#define SPELL_FULL_FIRE              75
#define SPELL_THROW                  76
#define SPELL_FIRE_STORM             77
#define SPELL_CRUSH_ARMOR            78
#define SPELL_FULL_HEAL		     79
#define SPELL_TRICK                  80
#define SPELL_HASTE                  81
#define SPELL_IMPROVED_HASTE         82
#define SPELL_FAR_LOOK               83
#define SPELL_ALL_HEAL               84

#define MAXSPELL 150

#define SPELL_IDENTIFY               53

#define SKILL_SNEAK                  55
#define SKILL_HIDE                   56
#define SKILL_STEAL                  57
#define SKILL_BACKSTAB               58
#define SKILL_PICK_LOCK              59

#define SKILL_KICK                   60
#define SKILL_BASH                   61
#define SKILL_RESCUE                 62
#define SKILL_DOUBLE                 63
#define SKILL_QUADRUPLE              64
#define SKILL_EXTRA_DAMAGING         65

#define SPELL_FIRE_BREATH            66
#define SPELL_GAS_BREATH             67
#define SPELL_FROST_BREATH           68
#define SPELL_ACID_BREATH            69
#define SPELL_LIGHTNING_BREATH       70

#define SKILL_TORNADO                85
#define SKILL_LIGHT_MOVE             86
#define SKILL_PARRY                  87
#define SKILL_FLASH	                 88
#define SKILL_MULTI_KICK             89

#define SPELL_ENCHANT_ARMOR          90
#define SPELL_PRAY_FOR_ARMOR         91
#define SPELL_INFRAVISION            92
#define SPELL_CREATE_NECTAR          93
#define SPELL_CREATE_GOLDEN_NECTAR   94
#define SPELL_CORN_OF_ICE            95
#define SPELL_DISINTEGRATE           96
#define SPELL_PHASE                  97
#define SPELL_MIRROR_IMAGE           98
#define SPELL_HAND_OF_GOD            99
#define SPELL_CAUSE_LIGHT           100
#define SPELL_CAUSE_CRITIC          101
#define SPELL_ENTIRE_HEAL			102
#define SKILL_OCTA					103
#define SPELL_LOVE  	            104 
#define SKILL_DISARM				105
#define SPELL_RERAISE				106 /* by chase */
#define SKILL_SHOURYUKEN			107 /* by chase */
#define SKILL_THROW_OBJECT			108 /* by chase */
#define SKILL_PUNCH					109
#define SPELL_DEATH					110 /* by process */
#define SPELL_ENCHANT_PERSON		111 /* by wings */
#define SPELL_SPELL_BLOCK           112 /* by process */
#define SKILL_TEMPTATION			113 /* by process */
#define SPELL_SHADOW_FIGURE			114 /* by process */
#define SPELL_MANA_TRANSFER			115 /* by process */
#define SPELL_SELF_HEAL			    116 /* by process */
#define SPELL_HOLY_SHIELD			117 /* by process */
#define SPELL_RESTORE_MOVE	        118 /* by ares */
#define SPELL_KISS_OF_PROCESS       119 /* by process */
#define SPELL_REFLECT_DAMAGE	    120 /* by process */
#define SPELL_DUMB					121 /* by process */
#define SKILL_SPIN_BIRD_KICK		122 /* by ares */
#define SPELL_THUNDERBOLT			123
#define SKILL_ARREST				124
#define SPELL_SANCTUARY_CLOUD		125 /* by atre */

#define TYPE_HIT                     150
#define TYPE_BLUDGEON                151
#define TYPE_PIERCE                  152
#define TYPE_SLASH                   153
#define TYPE_WHIP                    154 /* EXAMPLE */
#define TYPE_SHOOT                   155  /* NO MESSAGES WRITTEN YET! */
#define TYPE_BITE                    156  /* NO MESSAGES WRITTEN YET! */
#define TYPE_STING                   157  /* NO MESSAGES WRITTEN YET! */
#define TYPE_CRUSH                   158  /* NO MESSAGES WRITTEN YET! */



#define TYPE_SUFFERING               200
/* More anything but spells and weapontypes can be insterted here! */





#define MAX_TYPES 70

#define SAVING_PARA   0
#define SAVING_HIT_SKILL    1
#define SAVING_PETRI  2
#define SAVING_BREATH 3
#define SAVING_SPELL  4


#define MAX_SPL_LIST 200


#define TAR_IGNORE        1
#define TAR_CHAR_ROOM     2
#define TAR_CHAR_WORLD    4
#define TAR_FIGHT_SELF    8
#define TAR_FIGHT_VICT   16
#define TAR_SELF_ONLY    32 /* Only a check, use with ei. TAR_CHAR_ROOM */
#define TAR_SELF_NONO    64 /* Only a check, use with ei. TAR_CHAR_ROOM */
#define TAR_OBJ_INV     128
#define TAR_OBJ_ROOM    256
#define TAR_OBJ_WORLD   512
#define TAR_OBJ_EQUIP  1024
#define TAR_CHAR_ZONE  2048

struct spell_info_type
{
  void (*spell_pointer) (byte level, struct char_data *ch, char *arg, int type,
                         struct char_data *tar_ch, struct obj_data *tar_obj);
/* cyb
  void (*spell_pointer) ();
 */
  byte minimum_position;  /* Position for caster              */
  ubyte min_usesmana;     /* Amount of mana used by a spell   */
  byte beats;             /* Heartbeats until ready for next */
  byte min_level[4];
  byte max_skill[4];
  int targets;         /* See below for use with TAR_XXX  */
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


/* Attacktypes with grammar */

struct attack_hit_type {
  char *singular;
  char *plural;
};

