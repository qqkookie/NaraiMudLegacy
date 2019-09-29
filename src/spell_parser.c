/* ************************************************************************
*  file: spell_parser.c , Basic routines and parsing      Part of DIKUMUD *
*  Usage : Interpreter of spells                                          *
*  Copyright (C) 1990, 1991 - see 'license.doc' for complete information. *
************************************************************************* */

#include <stdio.h>
#include <assert.h>

#include "structs.h"
#include "utils.h"
#include "comm.h"
#include "db.h"
#include "interpreter.h" 
#include "spells.h"
#include "handler.h"
#include "memory.h"
#include "quest.h"

#define MANA_MU 1
#define MANA_CL 1

/* cyb :  SPELLO macro is transformed to function  ..  */

#define SPELL_LEVEL(ch,sn) (spell_info[sn].min_level[GET_CLASS(ch)-1])

/* Global data */

extern struct room_data *world;
extern struct char_data *character_list;
extern char *spell_wear_off_msg[];

/* Extern procedures */

char *string_dup(char *str);

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
void cast_colour_spray( byte level, struct char_data *ch, char *arg, int si,
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
void cast_detect_align( byte level, struct char_data *ch, char *arg, int si,
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
void cast_control_weather( byte level, struct char_data *ch, char *arg, int si,
                           struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_group_sanctuary( byte level, struct char_data *ch, char *arg, int si,
                           struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_paralysis( byte level, struct char_data *ch, char *arg, int si,
                           struct char_data *tar_ch, struct obj_data *tar_obj);
void cast_cure_paralysis( byte level, struct char_data *ch, char *arg, int si,
                           struct char_data *tar_ch, struct obj_data *tar_obj);
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
    "colour spray",
    "relocate",     /* 11 */
    "create food",
    "create water",
    "cure blind",
    "cure critic",
    "cure light",
    "curse",
    "detect align",
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
    "",
    "",
    "identify",
    "berserk",
    "sneak",        /* 55 */
    "hide",
    "steal",
    "backstab",
    "pick",
    "kick",         /* 60 */
    "bash",
    "rescue",
    "double attack",
    "quadruple attack",
    "extra damaging",
    "",
    "",
    "",
    "",
    "",
    "damage up",
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
    "pray for armor",
    "infravision",
    "create nectar",
    "create golden nectar",
    "corn of ice",
    "disintegrate",
    "phase",
    "mirror image",
    "hand of god",
    "cause light", /* 100 */
    "cause critic",
    "control weather",
	"group sanctuary",
	"paralysis",
	"cure paralysis",
    "\n"				/*  end of spells must be "\n"  */
};


/*   byte saving_throws[4][5][IMO+5] = { */
byte saving_throws[4][5][IMO+10] = {
    {
        {16,14,14,14,14,14,13,13,13,13,13,11,11,11,11,11,
         10, 9, 9, 9, 7, 7, 7, 5, 5, 5, 4, 4, 4, 4, 3,
         3, 3, 3, 2, 2, 2, 1, 1, 1, 1, 1, 1, 1, 1, 1,
         1, 1, 1, 1, 0},
        {13,11,11,11,11,11, 9, 9, 9, 9, 9, 7, 7, 7, 7, 7, 
         6, 6, 6, 6, 6, 6, 5, 5, 5, 5, 5, 3, 3, 3, 3,
         3, 2, 2, 2, 2, 2, 1, 1, 1, 1, 1, 1, 1, 1, 1,
         1, 1, 1, 1, 0},
        {15,13,13,13,13,13,11,11,11,11,11, 9, 9, 9, 9, 9,
         7, 7, 7, 7, 6, 6, 6, 5, 5, 5, 4, 4, 4, 3, 3,
         3, 3, 2, 2, 2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
         1, 1, 1, 1, 0},
        {17,15,15,15,15,15,13,13,13,13,13,11,11,11,11,11,
         10,10,10,10, 9, 9, 9, 9, 8, 8, 8, 7, 7, 7, 7,
         3, 3, 3, 3, 2, 2, 2, 1, 1, 1, 1, 1, 1, 1, 1,
         1, 1, 1, 1, 0},
        {14,12,12,12,12,12,10,10,10,10,10, 8, 8, 8, 8, 8,
         7, 7, 7, 7, 6, 6, 6, 6, 5, 5, 4, 4, 4, 4, 3,
         3, 3, 3, 3, 4, 3, 3, 3, 3, 2, 2, 2, 2, 2, 1,
         1, 1, 1, 1, 0}
    }, {
        {11,10,10,10, 9, 9, 9, 8, 8, 8, 8, 7, 7, 7, 7, 6,
         6, 6, 6, 6, 5, 5, 5, 5, 4, 4, 4, 4, 3, 3, 3,
         3, 3, 3, 3, 2, 2, 2, 1, 1, 1, 1, 1, 1, 1, 1,
         1, 1, 1, 1, 0},
        {16,14,14,14,13,13,13,11,11,11,10,10,10, 9, 9, 9,
         9, 8, 8, 8, 8, 6, 6, 6, 5, 5, 5, 4, 4, 4, 4,
         3, 3, 3, 3, 2, 2, 2, 1, 1, 1, 1, 1, 1, 1, 1,
         1, 1, 1, 1, 0},
        {15,13,13,13,12,12,12,11,11,11,10,10,10, 9, 9, 9,
         8, 8, 8, 7, 7, 7, 6, 6, 6, 5, 5, 5, 4, 4, 4,
         3, 3, 3, 3, 2, 2, 2, 1, 1, 1, 1, 1, 1, 1, 1,
         1, 1, 1, 1, 0},
        {18,16,16,16,15,15,15,13,13,13,12,12,12,11,11,11,
         10,10,10, 9, 9, 8, 8, 8, 7, 7, 6, 6, 5, 5, 4,
         3, 3, 3, 3, 2, 2, 2, 1, 1, 1, 1, 1, 1, 1, 1,
         1, 1, 1, 1, 0},
        {17,15,15,15,14,14,14,12,12,12,11,11,11,10,10,10,
         9, 9, 9, 7, 7, 7, 6, 6, 6, 5, 5, 5, 4, 4, 4,
         3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 2,
         2, 2, 1, 1, 0}
    }, {
        {15,13,13,13,13,12,12,12,12,11,11,11,11,10,10,10,
         10, 9, 9, 9, 8, 8, 7, 7, 6, 6, 5, 5, 4, 4, 4,
         3, 3, 3, 3, 2, 2, 2, 1, 1, 1, 1, 1, 1, 1, 1,
         1, 1, 1, 1, 0},
        {16,14,14,14,14,12,12,12,12,10,10,10,10, 8, 8, 8,
         8, 7, 7, 7, 6, 6, 6, 5, 5, 5, 5, 4, 4, 4, 4,
         3, 3, 3, 3, 2, 2, 2, 1, 1, 1, 1, 1, 1, 1, 1,
         1, 1, 1, 1, 0},
        {14,12,12,12,12,11,11,11,11,10,10,10,10, 9, 9, 9,
         9, 8, 8, 8, 7, 7, 7, 6, 6, 5, 5, 5, 4, 4, 4,
         3, 3, 3, 3, 2, 2, 2, 1, 1, 1, 1, 1, 1, 1, 1,
         1, 1, 1, 1, 0},
        {18,16,16,16,16,15,15,15,15,14,14,14,14,13,13,13,
         13,11,10, 9, 9, 9, 8, 8, 8, 7, 7, 7, 6, 5, 4,
         3, 3, 3, 3, 2, 2, 2, 1, 1, 1, 1, 1, 1, 1, 1,
         1, 1, 1, 1, 0},
        {17,15,15,15,15,13,13,13,13,11,11,11,11, 9, 9, 9,
         9, 8, 8, 7, 7, 7, 6, 6, 6, 5, 5, 5, 4, 4, 4,
         3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 2,
         2, 2, 1, 1, 0}
    }, {
        {16,14,14,14,14,11,11,10,10, 8, 8, 7, 7, 5, 5, 4,
         4, 4, 4, 4, 4, 4, 4, 4, 3, 3, 3, 3, 3, 3, 3,
         3, 3, 3, 3, 2, 2, 2, 1, 1, 1, 1, 1, 1, 1, 1,
         1, 1, 1, 1, 0},
        {18,16,16,15,15,13,13,12,12,10,10, 9, 9, 7, 7, 6,
         6, 5, 5, 5, 5, 5, 4, 4, 4, 4, 4, 4, 3, 3, 3,
         3, 3, 3, 3, 2, 2, 2, 1, 1, 1, 1, 1, 1, 1, 1,
         1, 1, 1, 1, 0},
        {17,15,15,14,14,12,12,11,11, 9, 9, 8, 8, 6, 6, 5,
         5, 5, 5, 5, 4, 4, 4, 4, 4, 4, 3, 3, 3, 3, 3,
         3, 3, 3, 3, 2, 2, 2, 1, 1, 1, 1, 1, 1, 1, 1,
         1, 1, 1, 1, 0},
        {20,17,17,16,16,13,13,12,12, 9, 9, 8, 8, 5, 5, 4,
         4, 4, 4, 4, 4, 4, 4, 3, 3, 3, 3, 3, 3, 3, 3,
         3, 3, 3, 3, 2, 2, 2, 1, 1, 1, 1, 1, 1, 1, 1,
         1, 1, 1, 1, 0},
        {19,17,17,16,16,14,14,13,13,11,11,10,10, 8, 8, 7,
         7, 6, 6, 6, 6, 6, 5, 5, 5, 5, 4, 4, 4, 4, 4,
         3, 3, 3, 3, 2, 2, 2, 1, 1, 1, 1, 1, 1, 1, 1,
         1, 1, 1, 1, 0}
    }
};	/* end of saving throw */


void affect_update( void )
{
    static struct affected_type *af, *next_af_dude;
    static struct char_data *i;

    for (i = character_list; i; i = i->next)
        for (af = i->affected; af; af = next_af_dude) {
            next_af_dude = af->next;
            if (af->duration >= 1)
                af->duration--;
            else {
                if (!af->next || (af->next->type != af->type) ||
                    (af->next->duration > 0))
                    if (*spell_wear_off_msg[af->type]) {
                        send_to_char(spell_wear_off_msg[af->type], i);
                        send_to_char("\n\r", i);
                    }

                affect_remove(i, af);
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
        free_general_type((char *)k, MEMORY_FOLLOW_TYPE) ;
    } else { /* locate follower who is not head of list */
        for(k = ch->master->followers; k->next->follower!=ch; k=k->next)  ;

        j = k->next;
        k->next = j->next;
        free_general_type((char *)j, MEMORY_FOLLOW_TYPE) ;
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

	/*
      create(k, struct follow_type, 1);
	*/
	k = (struct follow_type *) malloc_general_type(MEMORY_FOLLOW_TYPE) ;

    k->follower = ch;
    k->next = leader->followers;
    leader->followers = k;

    act("You now follow $N.", FALSE, ch, 0, leader, TO_CHAR);
    act("$n starts following you.", TRUE, ch, 0, leader, TO_VICT);
    act("$n now follows $N.", TRUE, ch, 0, leader, TO_NOTVICT);
}



say_spell( struct char_data *ch, int si )
{
    char buf[MAX_STRING_LENGTH], splwd[MAX_BUF_LENGTH];
    char buf2[MAX_STRING_LENGTH];

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

bool saves_spell(struct char_data *ch, sh_int save_type)
{
    int save;

    /* Negative apply_saving_throw makes saving throw better! */
    save = ch->specials.apply_saving_throw[save_type];
    if (!IS_NPC(ch)) {
        save += saving_throws[GET_CLASS(ch)-1][save_type][GET_LEVEL(ch)];
        if (GET_LEVEL(ch) >= IMO)
            return(TRUE);
    }
	else {
		/* if mob, minimum save must be 2 */
		if ( save > 0 ) save = MAX(2, save);
	}
    return(MAX(1,save) < number(0,20));
}
char *skip_spaces(char *string)
{
    for(;*string && (*string)==' ';string++);
    return(string);
}

int has_ULTIMATE_SCROLL(struct char_data *ch)
{
	extern struct index_data	*obj_index ;
    char buf[MAX_STRING_LENGTH];
	struct obj_data *obj;

	for (obj = ch->carrying; obj; obj = obj->next_content) {
		if (obj_index[obj->item_number].virtual == 12104) {
			/* scroll of ULTIMATE */

			/*
			sprintf(buf, "INFO> %s has ULTIMATE", ch->player.name);
			log(buf);
			*/
			return 1;
		}
	}
	return 0;
}

/* 100 is the MAX_MANA for a character */
// #define USE_MANA(ch, sn)                            \
//  MAX(spell_info[sn].min_usesmana, 100/(2+GET_LEVEL(ch)-SPELL_LEVEL(ch,sn)))

int calc_manause(struct char_data *ch, int sn)
{
	int min_mana;
	int calc_lev;
	int old_spell_level = IMO, new_spell_level;
	int base_max = 100;
	int min_usemana;

#define SPELL_LEVEL(ch,sn) (spell_info[sn].min_level[GET_CLASS(ch)-1])

	// remortal한 경우 예전 spell 필요 level
	if (is_solved_quest(ch, QUEST_REMORTAL)) {
		old_spell_level = spell_info[sn].min_level[GET_CLASS_O(ch) -1];
	}

	// 현재 spell 필요 level
	new_spell_level = SPELL_LEVEL(ch,sn);
	min_usemana = spell_info[sn].min_usesmana;
	
	if (is_solved_quest(ch, QUEST_REMORTAL)) {
		// 새로 배운 magic 이라면 마나 소모율 증가
		//
		if (old_spell_level > GET_LEVEL(ch)) {
			min_usemana = min_usemana * 2;
		}
	}

	calc_lev = 2 + GET_LEVEL(ch) - MIN(old_spell_level, new_spell_level);
	if (calc_lev <= 0) calc_lev = 1;

	return MAX(min_usemana, base_max/calc_lev);
}

/* Assumes that *argument does start with first letter of chopped string */
void do_cast(struct char_data *ch, char *argument, int cmd)
{
	extern struct index_data	*obj_index ;
    struct obj_data *tar_obj;
    struct char_data *tar_char;
    char name[MAX_STRING_LENGTH];
    int qend, spl, i, fail_rate, learn_rate;
    bool target_ok;
	int use_mana;

    if (IS_NPC(ch))
        return;

    if(GET_LEVEL(ch)>=IMO && GET_LEVEL(ch)<IMO+3) return;

	if (IS_SET(world[ch->in_room].room_flags, NO_MAGIC)) {
	    send_to_char("Your magic fizzles out and dies.\r\n", ch);
		act("$n's magic fizzles out and dies.", FALSE, ch, 0, 0,
			TO_ROOM);
		return;
	}

    argument = skip_spaces(argument);

    /* If there is no chars in argument */
    if (!(*argument)) {
        send_to_char("Cast which what where?\n\r", ch);
        return;
    }

    if (*argument != '\'') {
        send_to_char("Magic must always be enclosed by the magic symbols: '\n\r",ch);
        return;
    }

    /* Locate the last quote && lowercase the magic words (if any) */

    for (qend=1; *(argument+qend) && (*(argument+qend) != '\'') ; qend++)
        *(argument+qend) = LOWER(*(argument+qend));

    if (*(argument+qend) != '\'') {
        send_to_char("Magic must always be enclosed by the magic symbols: '\n\r",ch);
        return;
    }

    spl = old_search_block(argument, 1, qend-1,spells, 0);

    if (!spl) {
        send_to_char("Your lips do not move, no magic appears.\n\r",ch);
        return;
    }

#define ISASPELL(c) ((c > 0) && (c < MAXSPELL))

    if(ISASPELL(spl) && spell_info[spl].spell_pointer) {
        if (GET_POS(ch) < spell_info[spl].minimum_position) {
            switch(GET_POS(ch)) {
            case POSITION_SLEEPING :
                send_to_char("You dream about great magical powers.\n\r", ch);
                break;
            case POSITION_RESTING :
                send_to_char("You can't concentrate enough while resting.\n\r",ch);
                break;
            case POSITION_SITTING :
                send_to_char("You can't do this sitting!\n\r", ch);
                break;
            case POSITION_FIGHTING :
                send_to_char("Impossible! You can't concentrate enough!.\n\r", ch);
                break;
            default:
                send_to_char("It seems like you're in a pretty bad shape!\n\r",ch);
                break;
            } /* Switch */
        } else {
            if (spell_info[spl].min_level[GET_CLASS(ch)-1] > GET_LEVEL(ch)){
				if (is_solved_quest(ch, QUEST_REMORTAL) &&
					spell_info[spl].min_level[GET_CLASS_O(ch)-1] 
						<= GET_LEVEL(ch)) {
				}
				else {
					send_to_char("Sorry, you can't do that yet.\n\r", ch);
					return;
				}
            }
            argument+=qend+1; /* Point to the last ' */
            for(;*argument == ' '; argument++);
            target_ok = FALSE;
            tar_char = 0;
            tar_obj = 0;

            if (!IS_SET(spell_info[spl].targets, TAR_IGNORE)) {
                argument = one_argument(argument, name);
                if (*name) {
                    if (IS_SET(spell_info[spl].targets, TAR_CHAR_ROOM))
                        if (tar_char = get_char_room_vis(ch, name))
                            target_ok = TRUE;
                    if (!target_ok && IS_SET(spell_info[spl].targets, TAR_CHAR_WORLD))
                        if (tar_char = get_char_vis(ch, name))
                            target_ok = TRUE;
                    if (!target_ok && IS_SET(spell_info[spl].targets, TAR_CHAR_ZONE)){ 
                        if(IS_SET(world[ch->in_room].room_flags,EVERYZONE)){
                            if (tar_char = get_char_vis(ch, name))
                                target_ok = TRUE;
                        }else{
                            if (tar_char = get_char_vis_zone(ch, name))
                                target_ok = TRUE;
                        }
                    }
                    if (!target_ok && IS_SET(spell_info[spl].targets, TAR_OBJ_INV))
                        if (tar_obj = get_obj_in_list_vis(ch, name, ch->carrying))
                            target_ok = TRUE;
                    if (!target_ok && IS_SET(spell_info[spl].targets, TAR_OBJ_ROOM))
                        if (tar_obj = get_obj_in_list_vis(ch, name, world[ch->in_room].contents))
                            target_ok = TRUE;
                    if (!target_ok && IS_SET(spell_info[spl].targets, TAR_OBJ_WORLD))
                        if (tar_obj = get_obj_vis(ch, name))
                            target_ok = TRUE;
                    if (!target_ok && IS_SET(spell_info[spl].targets, TAR_OBJ_EQUIP)) {
                        for(i=0; i<MAX_WEAR && !target_ok; i++)
                            if (ch->equipment[i] && str_cmp(name, ch->equipment[i]->name) == 0) {
                                tar_obj = ch->equipment[i];
                                target_ok = TRUE;
                            }
                    }
                    if (!target_ok && IS_SET(spell_info[spl].targets, TAR_SELF_ONLY))
                        if (str_cmp(GET_NAME(ch), name) == 0) {
                            tar_char = ch;
                            target_ok = TRUE;
                        }
                } else { /* No argument was typed */
                    if (IS_SET(spell_info[spl].targets, TAR_FIGHT_SELF))
                        if (ch->specials.fighting) {
                            tar_char = ch;
                            target_ok = TRUE;
                        }
                    if (!target_ok && IS_SET(spell_info[spl].targets, TAR_FIGHT_VICT))
                        if (ch->specials.fighting) {
                            /* WARNING, MAKE INTO POINTER */
                            tar_char = ch->specials.fighting;
                            target_ok = TRUE;
                        }
                    if (!target_ok && 
					( IS_SET(spell_info[spl].targets, TAR_SELF_ONLY) ||
					  IS_SET(spell_info[spl].targets, TAR_SELF_PERMIT)) ) {
                        tar_char = ch;
                        target_ok = TRUE;
                    }
                }
            } else {
                target_ok = TRUE; /* No target, is a good target */
            }
            if (!target_ok) {
                if (*name) {
                    send_to_char("Say what?\n\r", ch);
                } else { /* Nothing was given as argument */
                    if (spell_info[spl].targets < TAR_OBJ_INV)
                        send_to_char("Who should the spell be cast upon?\n\r", ch);
                    else
                        send_to_char("What should the spell be cast upon?\n\r", ch);
                }
                return;
            } else if(GET_LEVEL(ch) < (IMO+3)){
                if((tar_char == ch) && IS_SET(spell_info[spl].targets, TAR_SELF_NONO)) {
                    send_to_char("You can not cast this spell upon yourself.\n\r", ch);
                    return;
                }
                else if ((tar_char!=ch)&&IS_SET(spell_info[spl].targets,TAR_SELF_ONLY)) {
                    send_to_char("You can only cast this spell upon yourself.\n\r", ch);
                    return;
                } else if (IS_AFFECTED(ch, AFF_CHARM) && (ch->master == tar_char)) {
                    send_to_char("You are afraid that it could harm your master.\n\r", ch);
                    return;
                }
            }

			// bluedragon yuiju
			//
			if ( ch->equipment[HOLD] &&
            	obj_index[ch->equipment[HOLD]->item_number].virtual == 17002) {
				use_mana = (calc_manause(ch, spl) * 4) / 5;
			}
			else use_mana = calc_manause(ch, spl);

            if (GET_LEVEL(ch) < IMO) {
                if (GET_MANA(ch) < use_mana) {
                    send_to_char("You can't summon enough energy to cast the spell.\n\r", ch);
                    return;
                }
            }
            if (spl != SPELL_VENTRILOQUATE)  /* :-) */
                say_spell(ch, spl);

			if (GET_LEVEL(ch) < IMO) {   /* add by timber(jmjeong@jmjeong.com) */
				if (has_ULTIMATE_SCROLL(ch)) {
					WAIT_STATE(ch, spell_info[spl].beats*4/5);
				}
				else  {
					WAIT_STATE(ch, spell_info[spl].beats);
				}
			}

            if ((spell_info[spl].spell_pointer == 0) && spl>0)
                send_to_char("Sorry, this magic has not yet been implemented :(\n\r", ch);
            else {
                fail_rate = number(1, 101) ;	/* 101% is complete failure */
                learn_rate = ch->skills[spl].learned ;
                if ( ch->equipment[HOLD] ) {
                    switch( obj_index[ch->equipment[HOLD]->item_number].virtual ) {
                    case 2707 :	/* power stone for mage */
                    case 2708 :	/* for cleric */
                        fail_rate -= 5 ;
                        fail_rate -= MIN(15, (101-learn_rate)/2) ;
                        break ;
                    case 2709 :	/* for thief */
                    case 2710 :	/* for warrior */
                        fail_rate -= 5 ;
                        fail_rate -= MIN(10, (101-learn_rate)/2) ;
                        break ;
                    default : break ;
                    }
                }

                if ( fail_rate > learn_rate ) {
                    send_to_char("You lost your concentration!\n\r", ch);
                    GET_MANA(ch) -= (use_mana>>1);
                    return;
                }
                send_to_char("Ok.\n\r",ch);
                ((*spell_info[spl].spell_pointer) (GET_LEVEL(ch),ch,argument,
                                                   SPELL_TYPE_SPELL, tar_char, tar_obj));
                GET_MANA(ch) -= (use_mana);
            }
        } /* if GET_POS < min_pos */
        return;
    }
    send_to_char("Eh?\n\r", ch);
}

void SPELLO( int nr, byte beat, byte pos, int l0, int l1, int l2, int l3,
             int s0, int s1, int s2, int s3, int mana, int tar,
             void (*func) (byte level, struct char_data *ch, char *arg,
                           int type, struct char_data *tar_ch,
                           struct obj_data *tar_obj))
{
    spell_info[nr].spell_pointer = (func);
    spell_info[nr].beats = (int)(beat) * NEW_TICK_GAP; /* new tick gap */
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

    for(i=0; i<MAX_SPL_LIST; i++)
        spell_info[i].spell_pointer = 0;

    /* From spells1.c */

    SPELLO(32,12,POSITION_FIGHTING, 1, 2, 3, 3,95,60,30,30,15, 
           TAR_CHAR_ROOM | TAR_FIGHT_VICT, cast_magic_missile);
    SPELLO( 8,12,POSITION_FIGHTING, 3, 5, 6, 6,95,60,30,30,15,
            TAR_CHAR_ROOM | TAR_FIGHT_VICT, cast_chill_touch);
    SPELLO( 5,12,POSITION_FIGHTING, 5, 8,10,10,95,60,30,30,15,
            TAR_CHAR_ROOM | TAR_FIGHT_VICT, cast_burning_hands);
    SPELLO(37,12,POSITION_FIGHTING, 7,10,12,12,95,60,30,30,15,
           TAR_CHAR_ROOM | TAR_FIGHT_VICT, cast_shocking_grasp);
    SPELLO(30,12,POSITION_FIGHTING, 9,13,16,16,95,60,30,30,15,
           TAR_CHAR_ROOM | TAR_FIGHT_VICT, cast_lightning_bolt);
    SPELLO(10,12,POSITION_FIGHTING,11,15,18,18,95,60,30,30,15,
           TAR_CHAR_ROOM | TAR_FIGHT_VICT, cast_colour_spray);
    SPELLO(25,12,POSITION_FIGHTING,13,20,15,20,95,40,65,40,35,
           TAR_CHAR_ROOM | TAR_FIGHT_VICT, cast_energy_drain);
    SPELLO(26,12,POSITION_FIGHTING,15,IMO,IMO,IMO,95,30,30,30,15,
           TAR_CHAR_ROOM | TAR_FIGHT_VICT, cast_fireball);
    SPELLO(23,12,POSITION_FIGHTING,15, 7,IMO,IMO,80,95,25,50,15,
           TAR_IGNORE, cast_earthquake);
    SPELLO(22,12,POSITION_FIGHTING,25,10,IMO,IMO,30,95,10,30,15,
           TAR_CHAR_ROOM | TAR_FIGHT_VICT, cast_dispel_evil);
    SPELLO( 6,12,POSITION_FIGHTING,20,15,IMO,IMO,80,95,10,10,15,
            TAR_CHAR_ROOM | TAR_FIGHT_VICT, cast_call_lightning);
    SPELLO(27,12,POSITION_FIGHTING,IMO,15,IMO,IMO,80,95,10,10,15,
           TAR_CHAR_ROOM | TAR_FIGHT_VICT, cast_harm);
    SPELLO(45,12,POSITION_FIGHTING,22,25,IMO,IMO,85,95,25,25,15,
           TAR_CHAR_ROOM | TAR_FIGHT_VICT, cast_sunburst);
    SPELLO(46,12,POSITION_STANDING,30,IMO,IMO,IMO,95,75,75,10,30,
           TAR_CHAR_ROOM | TAR_FIGHT_VICT, cast_clone);
    SPELLO(72,12,POSITION_FIGHTING,4,IMO,IMO,IMO,95,30,30,30,3,
           TAR_CHAR_ROOM | TAR_FIGHT_VICT, cast_energyflow);
    SPELLO(75,36,POSITION_FIGHTING,17,IMO,IMO,IMO,95,30,30,30,70,
           TAR_CHAR_ROOM | TAR_FIGHT_VICT, cast_full_fire);
    SPELLO(76,18,POSITION_FIGHTING,22,IMO,IMO,IMO,95,37,37,37,15,
           TAR_CHAR_ROOM | TAR_FIGHT_VICT, cast_throw);
    SPELLO(77,12,POSITION_FIGHTING,23,18,IMO,IMO,90,95,50,50,30,
           TAR_IGNORE, cast_fire_storm);

    /* Spells2.c */

    SPELLO( 1,12,POSITION_STANDING, 5, 1,IMO,IMO,95,95,60,10, 5,
            TAR_CHAR_ROOM, cast_armor);
    SPELLO( 2,12,POSITION_FIGHTING, 8,18,IMO,IMO,95,95,50,10,35,
            TAR_SELF_ONLY, cast_teleport);
    SPELLO( 3,12,POSITION_STANDING,25, 5,IMO,IMO,25,95,10,25, 5,
            TAR_OBJ_INV | TAR_OBJ_EQUIP | TAR_CHAR_ROOM, cast_bless);
    SPELLO( 4,12,POSITION_STANDING, 8, 6,IMO,IMO,95,95,50,10,15,
            TAR_CHAR_ROOM, cast_blindness);
    SPELLO( 7,12,POSITION_STANDING,14,26,IMO,IMO,95,10,25,20,20,
            TAR_CHAR_ROOM | TAR_SELF_NONO, cast_charm_person);
    SPELLO( 9,12,POSITION_STANDING, 6,12,IMO,IMO,95,95,20,10,10,
            TAR_OBJ_ROOM,cast_reanimate);
    SPELLO(11,12,POSITION_STANDING,20,24,25,25,90,80,60,40,50,
           TAR_CHAR_ZONE,cast_relocate);
    SPELLO(12,12,POSITION_STANDING,10, 3,20,20,40,95,25,10, 5,
           TAR_IGNORE, cast_create_food);
    SPELLO(13,12,POSITION_STANDING,10, 2,20,20,40,95,25,10, 5,
           TAR_OBJ_INV | TAR_OBJ_EQUIP, cast_create_water);
    SPELLO(14,12,POSITION_STANDING,20, 4,IMO,IMO,50,95,10,10, 5,
           TAR_CHAR_ROOM, cast_cure_blind);
    SPELLO(15,12,POSITION_FIGHTING,16, 9,20,18,60,95,45,50,15,
           TAR_CHAR_ROOM, cast_cure_critic);
    SPELLO(16,12,POSITION_FIGHTING, 4, 1,10,10,50,95,45,45,15,
           TAR_CHAR_ROOM, cast_cure_light);
    SPELLO(17,12,POSITION_STANDING,12,20,IMO,IMO,95,30,50,10,20,
           TAR_CHAR_ROOM | TAR_OBJ_ROOM | TAR_OBJ_INV | TAR_OBJ_EQUIP, cast_curse);
    SPELLO(18,12,POSITION_STANDING,20, 4,20,25,50,95,75,25, 5,
           TAR_CHAR_ROOM | TAR_SELF_ONLY, cast_detect_align);
    SPELLO(19,12,POSITION_STANDING, 2, 5,20,30,95,95,60,10, 5,
           TAR_CHAR_ROOM | TAR_SELF_PERMIT, cast_detect_invisibility);
    SPELLO(20,12,POSITION_STANDING,9,11,IMO,IMO,95,95,50,20,50,
           TAR_OBJ_INV | TAR_OBJ_EQUIP, cast_recharger);
    SPELLO(21,12,POSITION_STANDING,IMO,30,IMO,IMO,60,95,40,30,50,
           TAR_CHAR_ROOM, cast_preach);
    SPELLO(24,12,POSITION_STANDING,12,IMO,IMO,IMO,90,50,30,20,100,
           TAR_OBJ_INV | TAR_OBJ_EQUIP, cast_enchant_weapon);
    SPELLO(28,12,POSITION_FIGHTING,24,14,25,25,40,95,30,30,50,
           TAR_CHAR_ROOM | TAR_SELF_PERMIT, cast_heal);
    SPELLO(29,12,POSITION_STANDING, 4,20,IMO,IMO,95,50,60,10, 5,
           TAR_CHAR_ROOM|TAR_OBJ_INV|TAR_OBJ_ROOM|TAR_OBJ_EQUIP,cast_invisibility);
    SPELLO(31,12,POSITION_STANDING, 6,10,IMO,IMO,95,95,60,10,20,
           TAR_OBJ_WORLD, cast_locate_object);
    SPELLO(33,12,POSITION_STANDING,20, 8,IMO,IMO,60,95,60,10,20,
           TAR_CHAR_ROOM | TAR_SELF_NONO | TAR_OBJ_INV | TAR_OBJ_EQUIP, cast_poison);
    SPELLO(34,12,POSITION_STANDING,20, 6,IMO,IMO,60,95,10,10, 5,
           TAR_CHAR_ROOM | TAR_SELF_ONLY, cast_protection_from_evil);
    SPELLO(35,12,POSITION_STANDING,IMO,12,IMO,IMO,60,95,10,10, 5,
           TAR_CHAR_ROOM|TAR_OBJ_INV|TAR_OBJ_EQUIP|TAR_OBJ_ROOM, cast_remove_curse);
    SPELLO(36,12,POSITION_STANDING,25,13,IMO,IMO,40,95,10,10,33,
           TAR_CHAR_ROOM, cast_sanctuary);
    SPELLO(38,12,POSITION_STANDING,14,20,IMO,IMO,95,80,50,10,20,
           TAR_CHAR_ROOM, cast_sleep);
    SPELLO(39,12,POSITION_STANDING, 7,20,IMO,IMO,95,50,20,40,20,
           TAR_CHAR_ROOM | TAR_SELF_ONLY, cast_strength);
    SPELLO(40,12,POSITION_STANDING,20, 8,IMO,IMO,60,95,10,10,50,
           TAR_CHAR_WORLD, cast_summon);
    SPELLO(41,12,POSITION_STANDING, 1,20,20,20,95,95,95,95, 5,
           TAR_CHAR_ROOM | TAR_OBJ_ROOM | TAR_SELF_NONO, cast_ventriloquate);
    SPELLO(42,12,POSITION_STANDING,20,11,IMO,IMO,50,95,50,10, 5,
           TAR_CHAR_ROOM | TAR_SELF_ONLY, cast_word_of_recall);
    SPELLO(43,12,POSITION_STANDING,IMO, 9,IMO,IMO,50,95,40,30, 5,
           TAR_CHAR_ROOM | TAR_OBJ_INV | TAR_OBJ_ROOM, cast_remove_poison);
    SPELLO(44,12,POSITION_STANDING,15, 7,17,19,50,95,40,30, 5,
           TAR_CHAR_ROOM | TAR_SELF_ONLY,cast_sense_life);
    SPELLO(71,12,POSITION_STANDING,9,23,IMO,IMO,95,25,20,15,25,
           TAR_SELF_ONLY, cast_damage_up);
    SPELLO(73,12,POSITION_STANDING,15,22,IMO,IMO,95,20,10,20,10,
           TAR_CHAR_ROOM | TAR_SELF_ONLY, cast_mana_boost);
    SPELLO(74,12,POSITION_STANDING,17,IMO,IMO,IMO,95,40,35,30,25,
           TAR_CHAR_ROOM , cast_vitalize);
    SPELLO(78,12,POSITION_STANDING,19,28,IMO,IMO,95,35,30,25,45,
           TAR_CHAR_ROOM , cast_crush_armor);
    SPELLO(79,12,POSITION_FIGHTING,IMO,21,IMO,IMO,40,95,30,30,70,
           TAR_CHAR_ROOM, cast_full_heal);
    SPELLO(80,12,POSITION_FIGHTING,25,IMO,IMO,IMO,95,20,20,20,50,
           TAR_CHAR_ROOM, cast_trick);
    SPELLO(81,12,POSITION_STANDING,17,23,IMO,IMO,95,95,35,20,30,
           TAR_CHAR_ROOM, cast_haste);
    SPELLO(82,12,POSITION_STANDING,25,IMO,IMO,IMO,95,95,35,20,40,
           TAR_SELF_ONLY, cast_improved_haste);
    SPELLO(83,12,POSITION_STANDING,7,5,IMO,IMO,95,95,35,20,40,
           TAR_CHAR_WORLD, cast_far_look);
    SPELLO(84,12,POSITION_FIGHTING,IMO,30,IMO,IMO,80,95,25,50,85,
           TAR_IGNORE, cast_all_heal);
    SPELLO(47,1,POSITION_STANDING,IMO+4,IMO+4,IMO+4,IMO+4,0,0,0,0,10,TAR_IGNORE,0);
    SPELLO(48,1,POSITION_STANDING,IMO+4,IMO+4,IMO+4,IMO+4,0,0,0,0,10,TAR_IGNORE,0);
    SPELLO(49,1,POSITION_STANDING,IMO+4,IMO+4,IMO+4,IMO+4,0,0,0,0,10,TAR_IGNORE,0);
    SPELLO(50,1,POSITION_STANDING,IMO+4,IMO+4,IMO+4,IMO+4,0,0,0,0,10,TAR_IGNORE,0);
    SPELLO(51,1,POSITION_STANDING,IMO+4,IMO+4,IMO+4,IMO+4,0,0,0,0,10,TAR_IGNORE,0);
    SPELLO(52,1,POSITION_STANDING,IMO+4,IMO+4,IMO+4,IMO+4,0,0,0,0,10,TAR_IGNORE,0);
    SPELLO(53,1,POSITION_FIGHTING,17,IMO,IMO,IMO,95,5,5,5,25,
		TAR_OBJ_INV|TAR_CHAR_ROOM, cast_identify);
	/* jmjeong */
    SPELLO(54,1,POSITION_STANDING,IMO+4,IMO+4,33,25,0,0,20,95,100,TAR_IGNORE,0);
	/* jmjeong */
    SPELLO(55,1,POSITION_STANDING,1,1,1,1,30, 1,95,20,10,TAR_IGNORE,0);
    SPELLO(56,1,POSITION_STANDING,1,1,1,1,30, 1,95,20,100,TAR_IGNORE,0);
    SPELLO(57,1,POSITION_STANDING,1,1,1,1,30, 1,95,20,100,TAR_IGNORE,0);
    SPELLO(58,1,POSITION_STANDING,1,1,1,1,30, 1,95,30,100,TAR_IGNORE,0);
    SPELLO(59,1,POSITION_STANDING,1,1,1,1,30, 1,95,20,100,TAR_IGNORE,0);
    SPELLO(60,1,POSITION_STANDING,1,1,1,1,30,10,60,90,100,TAR_IGNORE,0);
    SPELLO(61,1,POSITION_STANDING,1,1,1,1,30,10,30,90,100,TAR_IGNORE,0);
    SPELLO(62,1,POSITION_STANDING,1,1,1,1,30,40,30,90,100,TAR_IGNORE,0);
    SPELLO(63,1,POSITION_STANDING,IMO,IMO,10,8,0,0,100,100,0,TAR_IGNORE,0);
    SPELLO(64,1,POSITION_STANDING,IMO,IMO,27,15,0,0,100,100,0,TAR_IGNORE,0);
    SPELLO(65,1,POSITION_STANDING,IMO,29,IMO,25,0,70,0,100,0,TAR_IGNORE,0);
    SPELLO(85,1,POSITION_STANDING,IMO,IMO,1,33,0,0,95,80,0,TAR_IGNORE,0);
    SPELLO(86,1,POSITION_STANDING,IMO,IMO,1,IMO,0,0,95,0,0,TAR_IGNORE,0);
    SPELLO(87,1,POSITION_STANDING,IMO,IMO,1,29,0,0,95,50,0,TAR_IGNORE,0);
    SPELLO(88,1,POSITION_STANDING,IMO,IMO,5,IMO,0,0,95,0,0,TAR_IGNORE,0);
    SPELLO(89,1,POSITION_STANDING,IMO,IMO,IMO,17,0,0,0,90,0,TAR_IGNORE,0);
    SPELLO(90,12,POSITION_STANDING,30,IMO,IMO,IMO,90,50,30,20,100,
           TAR_OBJ_INV | TAR_OBJ_EQUIP, cast_enchant_armor);
    SPELLO(91,12,POSITION_STANDING,IMO,34,IMO,IMO,90,90,30,20,100,
           TAR_OBJ_INV | TAR_OBJ_EQUIP, cast_pray_for_armor);
    SPELLO(92,12,POSITION_STANDING,5,7,IMO,IMO,95,95,10,10,5,
           TAR_CHAR_ROOM, cast_infravision);
    SPELLO(93,12,POSITION_STANDING,IMO,25,IMO,IMO,40,95,25,10,50,
           TAR_OBJ_INV | TAR_OBJ_EQUIP, cast_create_nectar);
    SPELLO(94,12,POSITION_STANDING,IMO,33,IMO,IMO,40,95,25,10,70,
           TAR_OBJ_INV | TAR_OBJ_EQUIP, cast_create_golden_nectar);
    SPELLO(95,12,POSITION_FIGHTING,30,IMO,IMO,IMO,95,30,30,30,20,
           TAR_CHAR_ROOM | TAR_FIGHT_VICT, cast_corn_of_ice);
    SPELLO(96,20,POSITION_FIGHTING,33,IMO,IMO,IMO,95,30,30,30,50,
           TAR_CHAR_ROOM | TAR_FIGHT_VICT, cast_disintegrate);
    SPELLO(97,12,POSITION_STANDING,20,24,IMO,IMO,95,95,30,30,10,
           TAR_IGNORE, cast_phase);
    SPELLO(98,12,POSITION_STANDING,35,IMO,IMO,IMO,95,95,35,20,70,
           TAR_SELF_ONLY, cast_mirror_image);
    SPELLO(99,12,POSITION_FIGHTING,IMO,35,IMO,IMO,90,95,50,50,33,
           TAR_IGNORE, cast_hand_of_god);
    SPELLO(100,12,POSITION_FIGHTING, 4, 1,IMO,IMO,50,95,45,45,15,
           TAR_CHAR_ROOM, cast_cause_light);
    SPELLO(101,12,POSITION_FIGHTING,16, 9,IMO,IMO,60,95,45,50,15,
           TAR_CHAR_ROOM, cast_cause_critic);
    SPELLO(102,12,POSITION_STANDING,IMO,IMO,IMO,IMO,95,95,20,20,10,
           TAR_IGNORE, cast_control_weather);
    SPELLO(103,12,POSITION_STANDING,IMO,35,IMO,IMO,10,95,10,10,120,
           TAR_SELF_ONLY, cast_group_sanctuary);
    SPELLO(104,12,POSITION_STANDING,20,32,IMO,IMO,95,30,10,10, 30,
           TAR_CHAR_ROOM, cast_paralysis);
    SPELLO(105,12,POSITION_STANDING,25,10,IMO,IMO,45,95,40,30, 10,
           TAR_CHAR_ROOM, cast_cure_paralysis);
}
