/* ************************************************************************
 *  file: spells.h , Implementation of magic spells.       Part of DIKUMUD *
 *  Usage : Spells                                                         *
 ************************************************************************* */

/* NOTE: Somme of spell constant symbol name changed.
	Ex) SPELL_DETECT_EVIL -> SPELL_DETECT_ALIGN	*/

#define TYPE_UNDEFINED               -1
#define SPELL_RESERVED_DBC            0		/* SKILL NUMBER ZERO */
#define SPELL_ARMOR                   1
#define SPELL_TELEPORT                2
#define SPELL_BLESS                   3
#define SPELL_BLINDNESS               4
#define SPELL_BURNING_HANDS           5
#define SPELL_CALL_LIGHTNING          6
#define SPELL_CHARM_PERSON            7
#define SPELL_CHILL_TOUCH             8
#define SPELL_REANIMATE               9
#define SPELL_COLOR_SPRAY            10
#define SPELL_RELOCATE               11
#define SPELL_CREATE_FOOD            12
#define SPELL_CREATE_WATER           13
#define SPELL_CURE_BLIND             14
#define SPELL_CURE_CRITIC            15
#define SPELL_CURE_LIGHT             16
#define SPELL_CURSE                  17
#define SPELL_DETECT_ALIGN           18
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
#define SPELL_PROTECT_FROM_ALIGN     34
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
#define SPELL_FIRESTORM              77
#define SPELL_CRUSH_ARMOR            78
#define SPELL_FULL_HEAL		     79
#define SPELL_TRICK                  80
#define SPELL_HASTE                  81
#define SPELL_IMPROVED_HASTE         82
#define SPELL_FAR_LOOK               83
#define SPELL_ALL_HEAL               84

#define SPELL_IDENTIFY               53

#define SKILL_SNEAK                  55
#define SKILL_HIDE                   56
#define SKILL_STEAL                  57
#define SKILL_BACKSTAB               58
#define SKILL_PICK_LOCK              59

#define SKILL_KICK                   60
#define SKILL_BASH                   61
#define SKILL_RESCUE                 62
#define SKILL_DOUBLE_ATTACK          63
#define SKILL_QUADRUPLE_ATTACK       64
#define SKILL_EXTRA_DAMAGING         65

#define SPELL_FIRE_BREATH            66
#define SPELL_GAS_BREATH             67
#define SPELL_FROST_BREATH           68
#define SPELL_ACID_BREATH            69
#define SPELL_LIGHTNING_BREATH       70

#define SKILL_TORNADO                85
#define SKILL_LIGHT_MOVE             86
#define SKILL_PARRY                  87
#define SKILL_FLASH		     88
#define SKILL_MULTI_KICK             89

#define SPELL_ENCHANT_ARMOR          90
#define SPELL_PRAY_FOR_ARMOR         91
#define SPELL_INFRAVISION            92
#define SPELL_CREATE_NECTAR          93
#define SPELL_CREATE_GOLDEN_NECTAR   94
#define SPELL_CONE_OF_ICE            95
#define SPELL_DISINTEGRATE           96
#define SPELL_PHASE                  97
#define SPELL_MIRROR_IMAGE           98
#define SPELL_HAND_OF_GOD            99
#define SPELL_CAUSE_LIGHT           100
#define SPELL_CAUSE_CRITIC          101
#define SPELL_ENTIRE_HEAL	    102
#define SKILL_OCTA_ATTACK	    103
#define SPELL_LOVE  	            104
#define SKILL_DISARM		    105
#define SPELL_RERAISE		    106		/* by chase */
#define SKILL_SHOURYUKEN	    107		/* by chase */
#define SKILL_THROW_OBJECT	    108		/* by chase */
#define SKILL_PUNCH		    109
#define SPELL_DEATH		    110		/* by process */
#define SPELL_ENCHANT_PERSON	    111		/* by wings */
#define SPELL_SPELL_BLOCK	    112		/* by process */
#define SKILL_TEMPTATION	    113		/* by process */
#define SPELL_SHADOW_FIGURE	    114		/* by process */
#define SPELL_MANA_TRANSFER	    115		/* by process */
#define SPELL_SELF_HEAL		    116		/* by process */
#define SPELL_HOLY_SHIELD	    117		/* by process */
#define SPELL_RESTORE_MOVE	    118		/* by ares */
#define SPELL_KISS_OF_PROCESS 	    119		/* by process */
#define SPELL_REFLECT_DAMAGE	    120		/* by process */
#define SPELL_DUMB		    121		/* by process */
#define SKILL_SPIN_BIRD_KICK	    122		/* by ares */
#define SPELL_THUNDERBOLT	    123
#define SKILL_ARREST		    124
#define SPELL_SANCTUARY_CLOUD	    125		/* by atre */
#define SKILL_TRACK		    126		/* NOTE: NEW! */

/* --------------------------------------------------------------- */
/* NOTE: spells related definition: also defined in spells.h  */

#define MAXSPELL 		150

/* NOTE: #define MAX_SPL_LIST	200 : to accomodate TYPE_SUFFERING */
#define MAX_SPL_LIST		210 

/* NOTE: Damage type order rearrangement */
#define TYPE_HIT		    150
#define TYPE_SLASH		    151
#define TYPE_PIERCE		    152
#define TYPE_BLUDGEON		    153
#define TYPE_CRUSH		    154	/* NO MESSAGES WRITTEN YET! */ 
#define TYPE_WHIP		    155	/* EXAMPLE */
#define TYPE_BITE		    156	/* NO MESSAGES WRITTEN YET! */
#define TYPE_STING		    157	/* NO MESSAGES WRITTEN YET! */
#define TYPE_SHOOT		    158	/* NO MESSAGES  */ 

/* #define WEAPON_*		    (170-179) */
/****************************************************************/
/* NOTE: This is OLD magic_weapon.h (from magic.c)		*/
/*******************  Define for magic weapon *******************/
/* NOTE: weapon special magic value : 1-9, 1000 -> 171-179, 170	*/

#define	WEAPON_ANY_MAGIC 		170
#define WEAPON_SMASH 			171
#define WEAPON_FLAME 			172
#define WEAPON_ICE 			173
#define WEAPON_BOMBARD 			174
#define WEAPON_SHOT 			175
#define WEAPON_DRAGON_SLAYER 		176
#define WEAPON_ANTI_EVIL_WEAPON 	177
#define WEAPON_ANTI_GOOD_WEAPON 	178
#define WEAPON_ANTI_ALIGN	 	179

/* NOTE: NO MESSAGES (will replace TYPE_UNDEFINED in damage().)  */ 
#define TYPE_MISC		    191	/* NO MESSAGES  */ 
#define TYPE_NOATTACK		    199	/* NO MESSAGES  */ 
#define TYPE_SUFFERING              200 

/* More anything but spells and weapontypes can be inserted here! */ 

/* #define MAX_TYPES		70 */



/* =================================================================== */
/* NOTE: casting magic proc.	*/
/* NOTE: Spell/cast function name change.
        Ex) void cast_detect_evil()         -> void cast_detect_align().
        Ex) void spell_detect_evil()         -> void spell_detect_align(). */


void cast_armor(byte level, struct char_data *ch, char *arg, int si,
		struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_teleport(byte level, struct char_data *ch, char *arg, int si,
		   struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_bless(byte level, struct char_data *ch, char *arg, int si,
		struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_blindness(byte level, struct char_data *ch, char *arg, int si,
		    struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_burning_hands(byte level, struct char_data *ch, char *arg, int si,
			struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_call_lightning(byte level, struct char_data *ch, char *arg, int si,
			 struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_charm_person(byte level, struct char_data *ch, char *arg, int si,
		       struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_chill_touch(byte level, struct char_data *ch, char *arg, int si,
		      struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_reanimate(byte level, struct char_data *ch, char *arg, int si,
		    struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_color_spray(byte level, struct char_data *ch, char *arg, int si,
		      struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_relocate(byte level, struct char_data *ch, char *arg, int si,
		   struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_create_food(byte level, struct char_data *ch, char *arg, int si,
		      struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_create_water(byte level, struct char_data *ch, char *arg, int si,
		       struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_cure_blind(byte level, struct char_data *ch, char *arg, int si,
		     struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_cure_critic(byte level, struct char_data *ch, char *arg, int si,
		      struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_cure_light(byte level, struct char_data *ch, char *arg, int si,
		     struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_curse(byte level, struct char_data *ch, char *arg, int si,
		struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_detect_align(byte level, struct char_data *ch, char *arg, int si,
		      struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_detect_invisible(byte level, struct char_data *ch, char *arg, 
		int si, struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_recharger(byte level, struct char_data *ch, char *arg, int si,
		    struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_preach(byte level, struct char_data *ch, char *arg, int si,
		 struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_dispel_evil(byte level, struct char_data *ch, char *arg, int si,
		      struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_earthquake(byte level, struct char_data *ch, char *arg, int si,
		     struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_enchant_weapon(byte level, struct char_data *ch, char *arg, int si,
			 struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_energy_drain(byte level, struct char_data *ch, char *arg, int si,
		       struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_fireball(byte level, struct char_data *ch, char *arg, int si,
		   struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_harm(byte level, struct char_data *ch, char *arg, int si,
	       struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_heal(byte level, struct char_data *ch, char *arg, int si,
	       struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_invisible(byte level, struct char_data *ch, char *arg, int si,
		       struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_lightning_bolt(byte level, struct char_data *ch, char *arg, int si,
			 struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_locate_object(byte level, struct char_data *ch, char *arg, int si,
			struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_magic_missile(byte level, struct char_data *ch, char *arg, int si,
			struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_poison(byte level, struct char_data *ch, char *arg, int si,
		 struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_protect_from_align(byte level, struct char_data *ch, char *arg,
		int si, struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_remove_curse(byte level, struct char_data *ch, char *arg, int si,
		       struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_sanctuary(byte level, struct char_data *ch, char *arg, int si,
		    struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_shocking_grasp(byte level, struct char_data *ch, char *arg, int si,
			 struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_sleep(byte level, struct char_data *ch, char *arg, int si,
		struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_strength(byte level, struct char_data *ch, char *arg, int si,
		   struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_summon(byte level, struct char_data *ch, char *arg, int si,
		 struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_ventriloquate(byte level, struct char_data *ch, char *arg, int si,
			struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_word_of_recall(byte level, struct char_data *ch, char *arg, int si,
			 struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_remove_poison(byte level, struct char_data *ch, char *arg, int si,
			struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_sense_life(byte level, struct char_data *ch, char *arg, int si,
		     struct char_data *tar_ch, struct obj_data *tar_obj); 
void cast_sunburst(byte level, struct char_data *ch, char *arg, int si,
		   struct char_data *tar_ch, struct obj_data *tar_obj); 
void cast_clone(byte level, struct char_data *ch, char *arg, int si,
		struct char_data *tar_ch, struct obj_data *tar_obj); 
void cast_damage_up(byte level, struct char_data *ch, char *arg, int si,
		    struct char_data *tar_ch, struct obj_data *tar_obj); 
void cast_energyflow(byte level, struct char_data *ch, char *arg, int si,
		     struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_mana_boost(byte level, struct char_data *ch, char *arg, int si,
		     struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_vitalize(byte level, struct char_data *ch, char *arg, int si,
		   struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_full_fire(byte level, struct char_data *ch, char *arg, int si,
		    struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_throw(byte level, struct char_data *ch, char *arg, int si,
		struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_firestorm(byte level, struct char_data *ch, char *arg, int si,
		     struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_crush_armor(byte level, struct char_data *ch, char *arg, int si,
		      struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_full_heal(byte level, struct char_data *ch, char *arg, int si,
		    struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_trick(byte level, struct char_data *ch, char *arg, int si,
		struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_haste(byte level, struct char_data *ch, char *arg, int si,
		struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_improved_haste(byte level, struct char_data *ch, char *arg, int si,
			 struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_far_look(byte level, struct char_data *ch, char *arg, int si,
		   struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_all_heal(byte level, struct char_data *ch, char *arg, int si,
		   struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_identify(byte level, struct char_data *ch, char *arg, int si,
		   struct char_data *tar_ch, struct obj_data *tar_obj);


void cast_enchant_armor(byte level, struct char_data *ch, char *arg, int si,
			struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_pray_for_armor(byte level, struct char_data *ch, char *arg, int si,
			 struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_infravision(byte level, struct char_data *ch, char *arg, int si,
		      struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_create_nectar(byte level, struct char_data *ch, char *arg, int si,
			struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_create_golden_nectar(byte level, struct char_data *ch, char *arg,
		int si, struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_cone_of_ice(byte level, struct char_data *ch, char *arg, int si,
		      struct char_data *tar_ch, struct obj_data *tar_obj); 
void cast_disintegrate(byte level, struct char_data *ch, char *arg, int si,
		       struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_phase(byte level, struct char_data *ch, char *arg, int si,
		struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_mirror_image(byte level, struct char_data *ch, char *arg, int si,
		       struct char_data *tar_ch, struct obj_data *tar_obj); 
void cast_hand_of_god(byte level, struct char_data *ch, char *arg, int si,
		      struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_cause_critic(byte level, struct char_data *ch, char *arg, int si,
		       struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_cause_light(byte level, struct char_data *ch, char *arg, int si,
		      struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_entire_heal(byte level, struct char_data *ch, char *arg, int si,
		      struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_love(byte level, struct char_data *ch, char *arg, int si,
	       struct char_data *tar_ch, struct obj_data *tar_obj);
		/* LOVE spell perhaps written */
void cast_reraise(byte level, struct char_data *ch, char *arg, int si,
		  struct char_data *tar_ch, struct obj_data *tar_obj); 
		/* reraise spell chase written */ 
void cast_death(byte level, struct char_data *ch, char *arg, int si,
		struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_enchant_person(byte level, struct char_data *ch, char *arg, int si,
			 struct char_data *tar_ch, struct obj_data *tar_obj); 
void cast_spell_block(byte level, struct char_data *ch, char *arg, int si,
		      struct char_data *tar_ch, struct obj_data *tar_obj);

/*  void cast_shadow_figure(byte level, struct char_data *ch, char *arg, int si,
		    struct char_data *tar_ch, struct obj_data *tar_obj); */ 

void cast_mana_transfer(byte level, struct char_data *ch, char *arg, int si,
			struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_self_heal(byte level, struct char_data *ch, char *arg, int si,
		    struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_holy_shield(byte level, struct char_data *ch, char *arg, int si,
		      struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_restore_move(byte level, struct char_data *ch, char *arg, int si,
		       struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_kiss_of_process(byte level, struct char_data *ch, char *arg, int si,
			struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_reflect_damage(byte level, struct char_data *ch, char *arg, int si,
			 struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_dumb(byte level, struct char_data *ch, char *arg, int si,
	       struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_thunderbolt(byte level, struct char_data *ch, char *arg, int si,
		       struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_sanctuary_cloud(byte level, struct char_data *ch, char *arg, int si,
			struct char_data *tar_ch, struct obj_data *tar_obj);


/* --------------------------------------------------------------- */
/* NOTE: spell proc.		*/


void spell_armor(byte level, struct char_data *ch,
		 struct char_data *victim, struct obj_data *obj);
void spell_teleport(byte level, struct char_data *ch,
		    struct char_data *victim, struct obj_data *obj);
void spell_bless(byte level, struct char_data *ch,
		 struct char_data *victim, struct obj_data *obj);
void spell_blindness(byte level, struct char_data *ch,
		     struct char_data *victim, struct obj_data *obj);
void spell_burning_hands(byte level, struct char_data *ch,
			 struct char_data *victim, struct obj_data *obj);
void spell_call_lightning(byte level, struct char_data *ch,
			  struct char_data *victim, struct obj_data *obj); 
void spell_charm_person(byte level, struct char_data *ch,
			struct char_data *victim, struct obj_data *obj);
void spell_chill_touch(byte level, struct char_data *ch,
		       struct char_data *victim, struct obj_data *obj); 
void spell_reanimate(byte level, struct char_data *ch,
		     struct char_data *victim, struct obj_data *obj);
void spell_color_spray(byte level, struct char_data *ch,
		       struct char_data *victim, struct obj_data *obj);
void spell_relocate(byte level, struct char_data *ch,
		    struct char_data *victim, struct obj_data *obj);
void spell_create_food(byte level, struct char_data *ch,
		       struct char_data *victim, struct obj_data *obj);
void spell_create_water(byte level, struct char_data *ch,
			struct char_data *victim, struct obj_data *obj);
void spell_cure_blind(byte level, struct char_data *ch,
		      struct char_data *victim, struct obj_data *obj);
void spell_cure_critic(byte level, struct char_data *ch,
		       struct char_data *victim, struct obj_data *obj);
void spell_cure_light(byte level, struct char_data *ch,
		      struct char_data *victim, struct obj_data *obj);
void spell_curse(byte level, struct char_data *ch,
		 struct char_data *victim, struct obj_data *obj);
void spell_detect_align(byte level, struct char_data *ch,
		       struct char_data *victim, struct obj_data *obj);
void spell_detect_invisible(byte level, struct char_data *ch,
			    struct char_data *victim, struct obj_data *obj);
void spell_recharger(byte level, struct char_data *ch,
		     struct char_data *victim, struct obj_data *obj); 
void spell_preach(byte level, struct char_data *ch,
		  struct char_data *victim, struct obj_data *obj);
void spell_dispel_evil(byte level, struct char_data *ch,
		       struct char_data *victim, struct obj_data *obj);
void spell_earthquake(byte level, struct char_data *ch,
		      struct char_data *victim, struct obj_data *obj); 
void spell_enchant_weapon(byte level, struct char_data *ch,
			  struct char_data *victim, struct obj_data *obj);
void spell_energy_drain(byte level, struct char_data *ch,
			struct char_data *victim, struct obj_data *obj);
void spell_fireball(byte level, struct char_data *ch,
		    struct char_data *victim, struct obj_data *obj);
void spell_harm(byte level, struct char_data *ch,
		struct char_data *victim, struct obj_data *obj);
void spell_heal(byte level, struct char_data *ch,
		struct char_data *victim, struct obj_data *obj);
void spell_invisible(byte level, struct char_data *ch,
			struct char_data *victim, struct obj_data *obj);
void spell_lightning_bolt(byte level, struct char_data *ch,
			  struct char_data *victim, struct obj_data *obj);
void spell_locate_object(byte level, struct char_data *ch,
			 struct char_data *victim, struct obj_data *obj); 
void spell_magic_missile(byte level, struct char_data *ch,
			 struct char_data *victim, struct obj_data *obj);
void spell_poison(byte level, struct char_data *ch,
		  struct char_data *victim, struct obj_data *obj);
void spell_protect_from_align(byte level, struct char_data *ch,
			    struct char_data *victim, struct obj_data *obj);
void spell_remove_curse(byte level, struct char_data *ch,
			struct char_data *victim, struct obj_data *obj);
void spell_sanctuary(byte level, struct char_data *ch,
		     struct char_data *victim, struct obj_data *obj);
void spell_shocking_grasp(byte level, struct char_data *ch,
			  struct char_data *victim, struct obj_data *obj); 
void spell_sleep(byte level, struct char_data *ch,
		 struct char_data *victim, struct obj_data *obj);
void spell_strength(byte level, struct char_data *ch,
		    struct char_data *victim, struct obj_data *obj);
void spell_summon(byte level, struct char_data *ch,
		  struct char_data *victim, struct obj_data *obj);
void spell_ventriloquate(byte level, struct char_data *ch,
			 struct char_data *victim, struct obj_data *obj); 
void spell_word_of_recall(byte level, struct char_data *ch,
			  struct char_data *victim, struct obj_data *obj);
void spell_remove_poison(byte level, struct char_data *ch,
			 struct char_data *victim, struct obj_data *obj);
void spell_sense_life(byte level, struct char_data *ch,
		      struct char_data *victim, struct obj_data *obj);
void spell_sunburst(byte level, struct char_data *ch,
		    struct char_data *victim, struct obj_data *obj);
void spell_clone(byte level, struct char_data *ch,
		 struct char_data *victim, struct obj_data *obj);
void spell_damage_up(byte level, struct char_data *ch,
		     struct char_data *victim, struct obj_data *obj);
void spell_energyflow(byte level, struct char_data *ch,
		      struct char_data *victim, struct obj_data *obj);
void spell_mana_boost(byte level, struct char_data *ch,
		      struct char_data *victim, struct obj_data *obj);
void spell_vitalize(byte level, struct char_data *ch,
		    struct char_data *victim, struct obj_data *obj);
void spell_full_fire(byte level, struct char_data *ch,
		     struct char_data *victim, struct obj_data *obj);
void spell_throw(byte level, struct char_data *ch,
		 struct char_data *victim, struct obj_data *obj);
void spell_firestorm(byte level, struct char_data *ch,
		      struct char_data *victim, struct obj_data *obj);
void spell_crush_armor(byte level, struct char_data *ch,
		       struct char_data *victim, struct obj_data *obj);
void spell_full_heal(byte level, struct char_data *ch,
		     struct char_data *victim, struct obj_data *obj);
void spell_trick(byte level, struct char_data *ch,
		 struct char_data *victim, struct obj_data *obj);
void spell_haste(byte level, struct char_data *ch,
		 struct char_data *victim, struct obj_data *obj);
void spell_improved_haste(byte level, struct char_data *ch,
			  struct char_data *victim, struct obj_data *obj);
void spell_far_look(byte level, struct char_data *ch,
		    struct char_data *victim, struct obj_data *obj);
void spell_all_heal(byte level, struct char_data *ch,
		    struct char_data *victim, struct obj_data *obj);
void spell_identify(byte level, struct char_data *ch,
		    struct char_data *victim, struct obj_data *obj);

void spell_fire_breath(byte level, struct char_data *ch,
		       struct char_data *victim, struct obj_data *obj);
void spell_frost_breath(byte level, struct char_data *ch,
			struct char_data *victim, struct obj_data *obj);
void spell_gas_breath(byte level, struct char_data *ch,
		      struct char_data *victim, struct obj_data *obj);
void spell_lightning_breath(byte level, struct char_data *ch,
			    struct char_data *victim, struct obj_data *obj);

void spell_enchant_armor(byte level, struct char_data *ch,
			 struct char_data *victim, struct obj_data *obj);
void spell_pray_for_armor(byte level, struct char_data *ch,
			  struct char_data *victim, struct obj_data *obj);
void spell_infravision(byte level, struct char_data *ch,
		       struct char_data *victim, struct obj_data *obj); 
void spell_create_nectar(byte level, struct char_data *ch,
			 struct char_data *victim, struct obj_data *obj);
void spell_create_golden_nectar(byte level, struct char_data *ch,
			    struct char_data *victim, struct obj_data *obj);
void spell_cone_of_ice(byte level, struct char_data *ch,
		       struct char_data *victim, struct obj_data *obj);
void spell_disintegrate(byte level, struct char_data *ch,
			struct char_data *victim, struct obj_data *obj); 
void spell_phase(byte level, struct char_data *ch,
		 struct char_data *victim, struct obj_data *obj);
void spell_mirror_image(byte level, struct char_data *ch,
			struct char_data *victim, struct obj_data *obj);
void spell_hand_of_god(byte level, struct char_data *ch,
		       struct char_data *victim, struct obj_data *obj);
void spell_cause_light(byte level, struct char_data *ch,
		       struct char_data *victim, struct obj_data *obj);
void spell_cause_critic(byte level, struct char_data *ch,
			struct char_data *victim, struct obj_data *obj);
void spell_entire_heal(byte level, struct char_data *ch,
		       struct char_data *victim, struct obj_data *obj);
void spell_love(byte level, struct char_data *ch,
		struct char_data *victim, struct obj_data *obj);
void spell_reraise(byte level, struct char_data *ch,
		   struct char_data *victim, struct obj_data *obj);
void spell_death(byte level, struct char_data *ch,
		 struct char_data *victim, struct obj_data *obj);
void spell_enchant_person(byte level, struct char_data *ch,
			  struct char_data *victim, struct obj_data *obj);
void spell_spell_block(byte level, struct char_data *ch,
			struct char_data *tar_ch, struct obj_data *tar_obj);
void spell_mana_transfer(byte level, struct char_data *ch,
			struct char_data *tar_ch, struct obj_data *tar_obj);
void spell_self_heal(byte level, struct char_data *ch,
		     struct char_data *victim, struct obj_data *obj);
void spell_holy_shield(byte level, struct char_data *ch, 
			struct char_data *tar_ch, struct obj_data *tar_obj);
void spell_restore_move(byte level, struct char_data *ch,
			struct char_data *victim, struct obj_data *obj); 
void spell_kiss_of_process(byte level, struct char_data *ch, 
			struct char_data *tar_ch, struct obj_data *tar_obj);
void spell_reflect_damage(byte level, struct char_data *ch, 
			struct char_data *tar_ch, struct obj_data *tar_obj);
void spell_dumb(byte level, struct char_data *ch, struct char_data *tar_ch,
		struct obj_data *tar_obj);
void spell_thunderbolt(byte level, struct char_data *ch,
			struct char_data *tar_ch, struct obj_data *tar_obj);
void spell_sanctuary_cloud(byte level, struct char_data *ch,
			   struct char_data *victim, struct obj_data *obj);

/* --------------------------------------------------------------- */
