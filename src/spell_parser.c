/* ************************************************************************
   *  file: spell_parser.c , Basic routines and parsing      Part of DIKUMUD *
   *  Usage : Interpreter of spells                                          *
   *  Copyright (C) 1990, 1991 - see 'license.doc' for complete information. *
   ************************************************************************* */

#include <stdio.h>
#include <string.h>

#include "char.h"
#include "object.h"
#include "global.h"
#include "comm.h"
#include "play.h"
#include "spells.h"

#define NOTSPELL ((void *) -1)

struct spell_info_type spell_info[MAX_SPL_LIST];

char *spells[MAX_SPL_LIST] ;

#ifdef UNUSED_CODE
char *spells[MAX_SPL_LIST] =
{   "", 		/* NOTE: spells[] index starts from 1, not zero.   */
    "armor",		/* 1 */
    "teleport",
    "bless",
    "blindness",
    "burning hands",
    "call lightning",
    "charm person",
    "chill touch",
    "reanimate",
    "color spray",
    "relocate",		/* 11 */
    "create food",
    "create water",
    "cure blind",
    "cure critic",
    "cure light",
    "curse",
    "detect align",
    "detect invisible",
    "recharger",
    "preach",		/* 21 */
    "dispel evil",
    "earthquake",
    "enchant weapon",
    "energy drain",
    "fireball",
    "harm",
    "heal",
    "invisible",
    "lightning bolt",
    "locate object",	/* 31 */
    "magic missile",
    "poison",
    "protection from evil",
    "remove curse",
    "sanctuary",
    "shocking grasp",
    "sleep",
    "strength",
    "summon",
    "ventriloquate",	/* 41 */
    "word of recall",
    "remove poison",
    "sense life",	/* 44 */
    "sunburst",
    "clone",
    "",
    "",
    "",
    "",
    "",			/* 51 */
    "",
    "identify",
    "",
    "sneak",		/* 55 */
    "hide",
    "steal",
    "backstab",
    "pick",
    "kick",		/* 60 */
    "bash",		/* 61 */
    "rescue",
    "double attack",
    "quadruple attack",
    "extra damaging",
    "",
    "",
    "",
    "",
    "",
    "damage up",	/* 71 */
    "energy flow",
    "mana boost",
    "vitalize",
    "full fire",
    "throw",		/* 76 */
    "firestorm",
    "crush armor",
    "full heal",
    "trick",
    "haste",		/* 81 */
    "improved haste",
    "far look",
    "all heal",
    "tornado",		/* 85  */
    "lightning move",
    "parry",
    "flash",
    "multi kick",
    "enchant armor",	/* 90 */
    "pray for armor",	/* 91 */
    "infravision",
    "create nectar",
    "create golden nectar",
    "cone of ice",
    "disintegrate",
    "phase",
    "mirror image",
    "hand of god",
    "cause light",	/* 100 */
    "cause critic",	/* 101 */
    "entire heal",
    "octa attack",
    "love",
    "disarm",		/* 105 */
    "reraise",
    "shouryuken",	/* 107 */
    "throw object",	/* 108 */
    "punch",		/* 109 */
    "death",		/* 110 */
    "enchant person",	/* 111 */
    "spell block",	/* 112 */
    "temptation",	/* 113 */
    "shadow figure",	/* 114 */
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
    "sanctuary cloud",	/* 125 */
    "track",
    "\n"
};
#endif 		/*  UNUSED_CODE */

char *spell_alias[] = {
    "",		"",
    "co",	"cone of ice",
    "fh",	"full heal",
    "full",	"full heal",
    "dali",	"detect align",
    "dinv",	"detect invisible",
    "enwe",	"enchant weapon",
    "enar",	"enchant armor",
    "remv",	"restore move",
    "sacl",	"sanctuary cloud",
    "\n"	"\n"
};

int use_mana(struct char_data *ch, int sn)
{
    int min;
    int lev;

    lev = GET_LEVEL(ch) / 5 + 2;
    lev += spell_info[sn].min_level[GET_CLASS(ch) - 1];
    lev = MIN(LEVEL_LIMIT+1, lev);
    min = spell_info[sn].min_usesmana * lev;
    min /= (1 + GET_LEVEL(ch));
    min = MAX(spell_info[sn].min_usesmana, min);

    return min;
}

void say_spell(struct char_data *ch, int si)
{
    char buf[MAX_LINE_LEN], buf2[MAX_LINE_LEN];
    char splwd[MAX_NAME_LEN], obfus[100];
    int j, offs;
    struct char_data *temp_char;


    struct syllable {
	char org[10];
	char new[10];
    };

    static struct syllable syls[] =
    {
	{" ", " "},
	{"ar", "abra"},
	{"au", "kada"},
	{"bless", "fido"},
	{"blind", "nose"},
	{"bur", "mosa"},
	{"cu", "judi"},
	{"de", "oculo"},
	{"en", "unso"},
	{"light", "dies"},
	{"lo", "hi"},
	{"mor", "zak"},
	{"move", "sido"},
	{"ness", "lacri"},
	{"ning", "illa"},
	{"per", "duda"},
	{"ra", "gru"},
	{"re", "candus"},
	{"son", "sabru"},
	{"tect", "infra"},
	{"tri", "cula"},
	{"ven", "nofo"},
	{"a", "a"},
	{"b", "b"},
	{"c", "q"},
	{"d", "e"},
	{"e", "z"},
	{"f", "y"},
	{"g", "o"},
	{"h", "p"},
	{"i", "u"},
	{"j", "y"},
	{"k", "t"},
	{"l", "r"},
	{"m", "w"},
	{"n", "i"},
	{"o", "a"},
	{"p", "s"},
	{"q", "d"},
	{"r", "f"},
	{"s", "g"},
	{"t", "h"},
	{"u", "j"},
	{"v", "z"},
	{"w", "x"},
	{"x", "n"},
	{"y", "l"},
	{"z", "k"},
	{"", ""}
    };


    strcpy(buf, "");
    /* NOTE: spells index starts from 1, not zero.   */
    strcpy(splwd, spells[si]);

    offs = 0;

    while (*(splwd + offs)) {
	for (j = 0; *(syls[j].org); j++)
	    if (strncmp(syls[j].org, splwd + offs, strlen(syls[j].org)) == 0) {
		strcat(obfus, syls[j].new);
		if (strlen(syls[j].org))
		    offs += strlen(syls[j].org);
		else
		    ++offs;
	    }
    }
    sprintf(buf2, "$n utters the words, '%s'", obfus);
    /* NOTE: spells index starts from 1, not zero.   */
    sprintf(buf, "$n utters the words, '%s'", spells[si]);
    for (temp_char = world[ch->in_room].people;
	 temp_char;
	 temp_char = temp_char->next_in_room)
	if (temp_char != ch) {
	    if (GET_CLASS(ch) == GET_CLASS(temp_char))
		act(buf, FALSE, ch, 0, temp_char, TO_VICT);
	    else
		act(buf2, FALSE, ch, 0, temp_char, TO_VICT);
	}
}

/* Assumes that *argument does start with first letter of chopped string */
void do_cast(struct char_data *ch, char *argument, int cmd)
{
    struct obj_data *tar_obj;
    struct char_data *tar_char;
    char name[MAX_BUFSIZ];
    int spl, i;
    extern struct char_data *get_char_vis_zone(struct char_data *ch,
    		char *name);

    if (!ch)
	return;

    /* 41 level can cast but cannot enchant(see magic2.c) */
    /* if(!IS_NPC(ch) && GET_LEVEL(ch) == IMO) {
	send_to_char("Uk! You can cast nothing.\r\n",ch);
	return;
    } */

    if (IS_MORTAL(ch) && IS_SET(world[ch->in_room].room_flags, NO_MAGIC)) {
	send_to_char("Your magical power can't be summoned!\r\n", ch);
	return;
    }

    if (IS_AFFECTED(ch, AFF_DUMB) && number(1, 10) > 5) {
	send_to_char("Eh? You are DUMBED!!!", ch);
	return;
    }

#ifdef UNUSED_CODE
    argument = skip_spaces(argument);

    /* If there is no chars in argument */
    if (!(*argument)) {
	if (!IS_NPC(ch))
	    send_to_char("Cast which what where?\r\n", ch);
	return;
    }

    if (*argument != '\'') {
	if (!IS_NPC(ch))
	    send_to_char("Magic must always be enclosed by ", ch);
	send_to_char("the magic symbols: '\r\n", ch);
	return;
    }

    /* Locate the last quote && lowercase the magic words (if any) */

    for (qend = 1; *(argument + qend) && (*(argument + qend) != '\''); qend++)
	*(argument + qend) = LOWER(*(argument + qend));

    if (*(argument + qend) != '\'') {
	if (!IS_NPC(ch))
	    send_to_char("Magic must always be enclosed by "
			 "the magic symbols: '\r\n", ch);
	return;
    }
#endif		/*  UNUSED_CODE  */

    /* NOTE: one_argument() will handle quoatation	*/
    /*  SIDE EFFECTS: Single word spell don't need quation. (cast sanc)
	   No complaint about missing quatation. */
    argument = one_argument(argument, name);
    // NOTE: Replace '-', '_' with space, 'full-heal' => 'full heal'
    char *cp = argument;
    while(*++cp)
	if (*cp == '-'  || *cp == '_')
	    *cp = ' ';

    if (!*name ) {
	send_to_char("Cast which what where?\r\n", ch);
	return;
    }

    /* NOTE: Spell name alias : Exact match only.	*/
    /* NOTE: match parameter 2 means keyword-data pair exact match  */
    if ((spl = search_block( name, spell_alias, 2)) > 0 )
	strcpy(name, spell_alias[spl+1]);

    /* NOTE: spells index starts from 1, not zero.   */
    /* NOTE: use serch_block() instead of old_search_block()    */
    /* spl = old_search_block(argument, 1, qend - 1, spells, 0); */
    spl = search_block( name, spells, 0);

    if (spl <= 0) {
	send_to_char("Your lips do not move, no magic appears.\r\n", ch);
	return;
    }

    if (( spl <= 0 || spl >= MAXSPELL) || !(spell_info[spl].spell_pointer)
	    || spell_info[spl].spell_pointer == NOTSPELL) {
	send_to_char("Eh?\r\n", ch);
	return;
    }

    if (GET_POS(ch) < spell_info[spl].minimum_position) {
	if (!IS_NPC(ch)) {
	    switch (GET_POS(ch)) {
	    case POS_SLEEPING:
		send_to_char("You dream about great ", ch);
		send_to_char("magical powers.\r\n", ch);
		break;
	    case POS_RESTING:
		send_to_char("You can't concentrate enough ", ch);
		send_to_char("while resting.\r\n", ch);
		break;
	    case POS_SITTING:
		send_to_char("You can't do this sitting!\r\n", ch);
		break;
	    case POS_FIGHTING:
		send_to_char("Impossible! You can't concentrate ", ch);
		send_to_char("enough!.\r\n", ch);
		break;
	    default:
		send_to_char("It seems like you're in ", ch);
		send_to_char("a pretty bad shape!\r\n", ch);
		break;
	    }	/* switch */
	}		/* !IS_NPC(ch) */
	return;
    }		/* end of checking minimum position */

    if (spell_info[spl].min_level[GET_CLASS(ch)-1] > GET_LEVEL(ch)) {
	send_to_char("Sorry, you can't do that yet.\r\n", ch);
	return;
    }
    /* NOTE: *argument is next char of end of first, now  */
    /* argument += qend + 1; */		/* Point to the last ' */
    for (; *argument == ' '; argument++) ;
    tar_char = 0;
    tar_obj = 0;

    if (!IS_SET(spell_info[spl].targets, TAR_IGNORE)) {
	argument = one_argument(argument, name);
	if (*name) {
	    /* NOTE: Code simplification */
	    if (IS_SET(spell_info[spl].targets, TAR_CHAR_ROOM)
		&& (tar_char = get_char_room_vis(ch, name)));
	    else if ( IS_SET(spell_info[spl].targets, TAR_CHAR_WORLD)
		&& (tar_char = get_char_vis(ch, name)));
	    else if ( IS_SET(spell_info[spl].targets, TAR_CHAR_ZONE)) {
		if (IS_SET(world[ch->in_room].room_flags, EVERYZONE))
		    tar_char = get_char_vis(ch, name);
		else
		    tar_char = get_char_vis_zone(ch, name);
	    }

	    if ( !tar_char && IS_SET(spell_info[spl].targets, TAR_OBJ_INV)
		 && (tar_obj = get_obj_in_list_vis(ch, name, ch->carrying)));
	    else if ( IS_SET(spell_info[spl].targets, TAR_OBJ_ROOM)
		 && (tar_obj = get_obj_in_list_vis(ch, name,
				       world[ch->in_room].contents)));
	    else if ( IS_SET(spell_info[spl].targets, TAR_OBJ_WORLD)
		 && (tar_obj = get_obj_vis(ch, name)));
	    else if ( IS_SET(spell_info[spl].targets, TAR_OBJ_EQUIP)) {
		for (i = 0; i < MAX_WEAR && !tar_obj; i++)
		    if (ch->equipment[i] && str_cmp(name,
				     ch->equipment[i]->name) == 0)
			tar_obj = ch->equipment[i];
	    }
	    if ( !tar_char && !tar_obj
		&& IS_SET(spell_info[spl].targets, TAR_SELF_ONLY))
		if (str_cmp(GET_NAME(ch), name) == 0)
		    tar_char = ch;
	}
	else {	/* No argument was typed */
	    if (IS_SET(spell_info[spl].targets, TAR_FIGHT_SELF)
		&& ch->specials.fighting && ( tar_char = ch ));
	    else if ( IS_SET(spell_info[spl].targets, TAR_FIGHT_VICT)
		&& ( tar_char = ch->specials.fighting ));
		    /* WARNING, MAKE INTO POINTER */
	    /* NOTE: If TAR_SELF_DEF is set, default is myself */
	    else if ( IS_SET(spell_info[spl].targets, TAR_SELF_ONLY|TAR_SELF_DEF))
		tar_char = ch;
	}
    }

    if (!tar_char && !tar_obj && !IS_SET(spell_info[spl].targets, TAR_IGNORE)) {
	if (*name)
		send_to_char("Say what?\r\n", ch);
	// else {	/* Nothing was given as argument */
	else if (!IS_NPC(ch)) {
	    if (spell_info[spl].targets < TAR_OBJ_INV)
		send_to_char("Who should the spell be cast upon?\r\n", ch);
	    else
		send_to_char("What should the spell be cast upon?\r\n", ch);
	}
	return;
    }

    if (IS_NPC(ch) || NOT_GOD(ch)) {
	if ((tar_char == ch) &&
	    IS_SET(spell_info[spl].targets, TAR_SELF_NONO)) {
	    send_to_char("You can not cast this spell upon yourself.\r\n", ch);
	    return;
	}
	else if ((tar_char != ch) &&
		 IS_SET(spell_info[spl].targets, TAR_SELF_ONLY)) {
	    send_to_char("You can only cast this spell upon yourself.\r\n", ch);
	    return;
	}
	else if (IS_AFFECTED(ch, AFF_CHARM) && (ch->master == tar_char)
	     && IS_SET(spell_info[spl].targets, TAR_SELF_NONO|TAR_FIGHT_VICT)){
	    send_to_char("You are afraid that it could harm your master.\r\n", ch);
	    return;
	}
    }

    if (IS_NPC(ch) || IS_MORTAL(ch))
	if (GET_MANA(ch) < use_mana(ch, spl)) {
	    if (!IS_NPC(ch))
		send_to_char("You can't summon enough energy to cast the spell.\r\n", ch);
	    else
		act("$n could not summon enough energy to cast.",
		    TRUE, ch, 0, 0, TO_ROOM);
	    return;
	}

    if (spl != SPELL_VENTRILOQUATE)	/* :-) */
	say_spell(ch, spl);
    WAIT_STATE(ch, spell_info[spl].beats);

    if ((spell_info[spl].spell_pointer == 0) && spl > 0) {
	send_to_char("Sorry, this magic has not yet ", ch);
	send_to_char("been implemented.\r\n", ch);
    }
    else {
	if (number(1, 101) > ch->skills[spl].learned) {
	    if (!IS_MOB(ch))
		send_to_char("You lost your concentration!\r\n", ch);
	    else
		act("$n lost the concentration!!!", TRUE, ch, 0, 0, TO_ROOM);
	    GET_MANA(ch) -= (use_mana(ch, spl) >> 1);
	    return;
	}
	if (!IS_NPC(ch))
	    send_to_char("Ok.\r\n", ch);
	/* NOTE: Don't block non-harmful spell and spell casted by myself. */
	if (tar_char && tar_char != ch
	    && IS_SET(spell_info[spl].targets, TAR_SELF_NONO|TAR_FIGHT_VICT)) {
	    if( IS_AFFECTED(tar_char, AFF_SPELL_BLOCK) && number(1, 10) > 3 ) {
		if (!IS_MOB(ch))
		    send_to_char("Your magical power is blocked!\r\n", ch);
		else
		    act("$n failed to cast spell by spell block!!",
			TRUE, ch, 0, 0, TO_ROOM);
		return;
	    }
	    if( IS_AFFECTED(tar_char, AFF_DEATH) && number(1, 10) > 2 ) {
		if (!IS_MOB(ch))
		    send_to_char("He will die. Let him die. \r\n", ch);
		else
		    act("$n failed to cast by DEATH!!",
			TRUE, ch, 0, 0, TO_ROOM);
		return;
	    }
	}
	((*spell_info[spl].spell_pointer) (GET_LEVEL(ch), ch, argument,
			      SPELL_TYPE_SPELL, tar_char, tar_obj));
	GET_MANA(ch) -= (use_mana(ch, spl));
    }
}

void SPELLO(int nr, char *name, byte beat,
    byte pos, int l0, int l1, int l2, int l3,
    int s0, int s1, int s2, int s3, int mana, int tar,
    void (*func) (byte level, struct char_data * ch, char *arg, int type,
		struct char_data * tar_ch, struct obj_data * tar_obj))
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
    spell_info[nr].name = name;
    spells[nr] = name;
    /*
    if (strcmp(name, spells[nr]))
	fprintf(stderr, "hmm.. wrong spell name.. %s %s.\n", name, spells[nr]);
    */
}

#define IMO	(LEVEL_LIMIT+1)

void assign_spell_pointers(void)
{
    int i;

    for (i = 0; i < MAX_SPL_LIST; i++) {
	spell_info[i].spell_pointer = 0;
	spells[i] = "";
    }
    /* NOTE:  Add TAR_SELF_DEF to target type  of following spells:
	'armor', 'bless', 'cure blind', 'cure critic', 'cure light',
	'heal', 'remove curse', 'sanctuary', 'remove poison', 'love',
	'full heal', 'haste', 'entire heal'. */
    /* NOTE: 'mana boost', 'vitalize' is excluded from TAR_SELF_DEF */
    /* NOTE: Add min mana to 'backstab', 'shouryuken', 'spin bird kick': 250 */

    SPELLO(SPELL_ARMOR, "armor", 12,
	    POS_STANDING, 5, 1, IMO, IMO, 75, 95, 30, 30, 15,
	    TAR_CHAR_ROOM | TAR_SELF_DEF, cast_armor);
    SPELLO(SPELL_TELEPORT, "teleport", 12,
	    POS_STANDING, 8, 18, 40, 40, 95, 95, 30, 30, 50,
	    TAR_SELF_ONLY, cast_teleport);
    SPELLO(SPELL_BLESS, "bless", 12,
	    POS_STANDING, 25, 5, IMO, IMO, 40, 95, 30, 30, 25,
	    TAR_OBJ_INV | TAR_CHAR_ROOM | TAR_SELF_DEF, cast_bless);
    SPELLO(SPELL_BLINDNESS, "blindness", 12,
	    POS_STANDING, 8, 6, IMO, IMO, 75, 95, 30, 30, 25,
	    TAR_CHAR_ROOM | TAR_FIGHT_VICT | TAR_SELF_NONO, cast_blindness);
    SPELLO(SPELL_BURNING_HANDS, "burning hands", 12,
	    POS_FIGHTING, 5, 8, 15, 41, 60, 60, 30, 30, 100,
	  TAR_CHAR_ROOM | TAR_FIGHT_VICT | TAR_SELF_NONO, cast_burning_hands);
    SPELLO(SPELL_CALL_LIGHTNING, "call lightning", 12,
	    POS_FIGHTING, 20, IMO, IMO, IMO, 95, 30, 30, 30, 100,
	  TAR_CHAR_ROOM | TAR_FIGHT_VICT | TAR_SELF_NONO, cast_call_lightning);
    SPELLO(SPELL_CHARM_PERSON, "charm person", 12,
	   POS_STANDING, 20, 15, IMO, IMO, 60, 95, 30, 30, 50,
	   TAR_CHAR_ROOM | TAR_SELF_NONO, cast_charm_person);
    /* NOTE: lower max lerned of 'chill touch' spell for mage : 95 -> 60. */
    /* SPELLO(8, 12, POS_FIGHTING, 3, 5, 13, 13, 95, 60, 30, 30, 15, */
    SPELLO(SPELL_CHILL_TOUCH, "chill touch", 12,
	    POS_FIGHTING, 3, 5, 13, 13, 95, 60, 30, 30, 15,
	    TAR_CHAR_ROOM | TAR_FIGHT_VICT | TAR_SELF_NONO, cast_chill_touch);
    SPELLO(SPELL_REANIMATE, "reanimate", 12,
	    POS_STANDING, 15, IMO, IMO, IMO, 95, 30, 30, 30, 100,
	    TAR_OBJ_ROOM, cast_reanimate);

    SPELLO(SPELL_COLOR_SPRAY, "color spray", 12,
	    POS_FIGHTING, 11, 15, 21, 21, 95, 60, 30, 30, 25,
	   TAR_CHAR_ROOM | TAR_FIGHT_VICT | TAR_SELF_NONO, cast_color_spray);
    SPELLO(SPELL_RELOCATE, "relocate", 12,
	    POS_STANDING, 24, 20, 30, 30, 80, 90, 30, 30, 150,
	    TAR_CHAR_ZONE, cast_relocate);
    SPELLO(SPELL_CREATE_FOOD, "create food", 12,
	    POS_STANDING, 10, 3, 20, 20, 50, 95, 30, 30, 5,
	    TAR_IGNORE, cast_create_food);
    SPELLO(SPELL_CREATE_WATER, "create water", 12,
	    POS_STANDING, 10, 3, 20, 20, 50, 95, 30, 30, 5,
	    TAR_OBJ_INV, cast_create_water);
    SPELLO(SPELL_CURE_BLIND, "cure blind", 12,
	    POS_STANDING, 10, 4, IMO, IMO, 80, 95, 30, 30, 80,
	    TAR_CHAR_ROOM | TAR_SELF_DEF, cast_cure_blind);
    SPELLO(SPELL_CURE_CRITIC, "cure critic", 12,
	    POS_FIGHTING, 11, 7, 20, 20, 80, 95, 70, 70, 85,
	    TAR_CHAR_ROOM | TAR_SELF_DEF, cast_cure_critic);
    SPELLO(SPELL_CURE_LIGHT, "cure light", 12,
	    POS_FIGHTING, 4, 1, 10, 10, 80, 95, 70, 70, 55,
	    TAR_CHAR_ROOM | TAR_SELF_DEF, cast_cure_light);
    /* NOTE: Magic User can't cast spell 'curse'. */
    /* SPELLO(17, 12, POS_STANDING, 12, 20, IMO, IMO, 90, 60, 30, 30, 50,*/
    SPELLO(SPELL_CURSE, "curse", 12,
	    POS_STANDING, IMO, 20, IMO, IMO, 90, 60, 30, 30, 50,
	TAR_CHAR_ROOM | TAR_OBJ_ROOM | TAR_OBJ_INV | TAR_SELF_NONO, cast_curse);
    SPELLO(SPELL_DETECT_ALIGN, "detect align", 12,
	    POS_STANDING, 20, 4, 30, 30, 50, 95, 30, 30, 50,
	    TAR_SELF_ONLY, cast_detect_align);
    SPELLO(SPELL_DETECT_INVISIBLE, "detect invisible", 12,
	    POS_STANDING, 2, 5, 20, 20, 95, 95, 70, 70, 50,
	    TAR_SELF_ONLY, cast_detect_invisible);

    SPELLO(SPELL_RECHARGER, "recharger", 12,
	    POS_STANDING, 11, 9, IMO, IMO, 95, 95, 30, 30, 200,
	    TAR_OBJ_INV, cast_recharger);
    SPELLO(SPELL_PREACH, "preach", 12,
	    POS_STANDING, IMO, 25, IMO, IMO, 30, 95, 30, 30, 100,
	    TAR_CHAR_ROOM | TAR_SELF_NONO, cast_preach);
    SPELLO(SPELL_DISPEL_EVIL, "dispel evil", 12,
	    POS_FIGHTING, 25, 10, IMO, IMO, 40, 95, 30, 30, 35,
	    TAR_CHAR_ROOM | TAR_FIGHT_VICT | TAR_SELF_NONO, cast_dispel_evil);
    SPELLO(SPELL_EARTHQUAKE, "earthquake", 12,
	    POS_FIGHTING, IMO, 7, IMO, IMO, 30, 95, 30, 30, 75,
	    TAR_IGNORE, cast_earthquake);
    /* SPELLO(24, 12, POS_STANDING, 12, 41, 41, 41, 90, 50, 30, 30, 255,*/
    SPELLO(SPELL_ENCHANT_WEAPON, "enchant weapon", 12,
	    POS_STANDING, 12, 30, 40, 40, 90, 50, 30, 30, 255,
	    TAR_OBJ_INV, cast_enchant_weapon);
    SPELLO(SPELL_ENERGY_DRAIN, "energy drain", 12,
	    POS_FIGHTING, 13, 20, 25, 25, 95, 40, 30, 30, 35,
	    TAR_CHAR_ROOM | TAR_FIGHT_VICT | TAR_SELF_NONO, cast_energy_drain);
    SPELLO(SPELL_FIREBALL, "fireball", 12,
	    POS_FIGHTING, 15, IMO, IMO, IMO, 95, 30, 30, 30, 75,
	    TAR_CHAR_ROOM | TAR_FIGHT_VICT | TAR_SELF_NONO, cast_fireball);
    SPELLO(SPELL_HARM, "harm", 12,
	    POS_FIGHTING, IMO, 15, IMO, IMO, 30, 95, 30, 30, 50,
	    TAR_CHAR_ROOM | TAR_FIGHT_VICT | TAR_SELF_NONO, cast_harm);
    SPELLO(SPELL_HEAL, "heal", 12,
	    POS_FIGHTING, 23, 10, IMO, IMO, 80, 95, 30, 30, 100,
	    TAR_CHAR_ROOM | TAR_SELF_DEF, cast_heal);
    SPELLO(SPELL_INVISIBLE, "invisible", 12,
	    POS_STANDING, 4, 10, IMO, IMO, 95, 70, 30, 30, 50,
	    TAR_CHAR_ROOM | TAR_OBJ_INV | TAR_OBJ_ROOM, cast_invisible);

    SPELLO(SPELL_LIGHTNING_BOLT, "lightning bolt", 12,
	    POS_FIGHTING, 9, 13, 19, 19, 95, 60, 30, 30, 20,
	TAR_CHAR_ROOM | TAR_FIGHT_VICT | TAR_SELF_NONO, cast_lightning_bolt);
    SPELLO(SPELL_LOCATE_OBJECT, "locate object", 12,
	    POS_STANDING, 6, 10, IMO, IMO, 95, 95, 30, 30, 50,
	    TAR_OBJ_WORLD, cast_locate_object);
    SPELLO(SPELL_MAGIC_MISSILE, "magic missile", 12,
	    POS_FIGHTING, 1, IMO, IMO, IMO, 95, 30, 30, 30, 50,
	    TAR_CHAR_ROOM | TAR_FIGHT_VICT | TAR_SELF_NONO, cast_magic_missile);
    SPELLO(SPELL_POISON, "poison", 12,
	    POS_STANDING, 20, 8, 30, IMO, 70, 95, 60, 30, 70,
	    TAR_CHAR_ROOM | TAR_SELF_NONO | TAR_OBJ_INV, cast_poison);
    SPELLO(SPELL_PROTECT_FROM_ALIGN, "protect from align", 12,
	    POS_STANDING, 20, 6, IMO, IMO, 60, 95, 30, 30, 50,
	    TAR_SELF_ONLY, cast_protect_from_align);
    SPELLO(SPELL_REMOVE_CURSE, "remove curse", 12,
	    POS_STANDING, IMO, 12, IMO, IMO, 30, 95, 30, 30, 50,
	    TAR_CHAR_ROOM | TAR_OBJ_INV | TAR_OBJ_ROOM | TAR_SELF_DEF, cast_remove_curse);
    SPELLO(SPELL_SANCTUARY, "sanctuary", 12,
	    POS_STANDING, 25, 13, IMO, IMO, 50, 95, 30, 30, 100,
	    TAR_CHAR_ROOM | TAR_SELF_DEF, cast_sanctuary);
    SPELLO(SPELL_SHOCKING_GRASP, "shocking grasp", 12,
	    POS_FIGHTING, 7, 10, 17, 17, 95, 60, 30, 30, 15,
	TAR_CHAR_ROOM | TAR_FIGHT_VICT | TAR_SELF_NONO, cast_shocking_grasp);
    SPELLO(SPELL_SLEEP, "sleep", 12,
	    POS_STANDING, 20, 14, IMO, IMO, 75, 90, 30, 30, 50,
	    TAR_CHAR_ROOM | TAR_SELF_NONO, cast_sleep);
    SPELLO(SPELL_STRENGTH, "strength", 12,
	    POS_STANDING, 10, 7, IMO, IMO, 75, 90, 30, 30, 90,
	    TAR_SELF_ONLY, cast_strength);

    SPELLO(SPELL_SUMMON, "summon", 12,
	    POS_STANDING, 20, 8, IMO, IMO, 60, 95, 30, 30, 200,
	    TAR_CHAR_WORLD, cast_summon);
    SPELLO(SPELL_VENTRILOQUATE, "ventriloquate", 12,
	    POS_STANDING, 1, 1, 1, 1, 95, 95, 95, 95, 5,
	    TAR_CHAR_ROOM | TAR_OBJ_ROOM | TAR_SELF_NONO, cast_ventriloquate);
    SPELLO(SPELL_WORD_OF_RECALL, "word of recall", 12,
	    POS_STANDING, 20, 11, IMO, IMO, 50, 95, 30, 30, 200,
	    TAR_SELF_ONLY, cast_word_of_recall);
    SPELLO(SPELL_REMOVE_POISON, "remove poison", 12,
	    POS_STANDING, IMO, 9, 33, IMO, 30, 95, 50, 30, 50,
	    TAR_CHAR_ROOM | TAR_OBJ_INV | TAR_OBJ_ROOM | TAR_SELF_DEF, cast_remove_poison);
    SPELLO(SPELL_SENSE_LIFE, "sense life", 12,
	    POS_STANDING, 15, 7, 7, 17, 70, 95, 95, 30, 50,
	    TAR_SELF_ONLY, cast_sense_life);
    SPELLO(SPELL_SUNBURST, "sunburst", 12,
	    POS_FIGHTING, 22, 25, IMO, IMO, 80, 95, 30, 30, 45,
	    TAR_CHAR_ROOM | TAR_FIGHT_VICT | TAR_SELF_NONO, cast_sunburst);
    SPELLO(SPELL_CLONE, "clone", 12,
	    POS_STANDING, 30, IMO, IMO, IMO, 95, 30, 30, 30, 50,
	    TAR_CHAR_ROOM | TAR_FIGHT_VICT | TAR_SELF_NONO, cast_clone);

    /* SPELL NO 47 - 52 is not used. */

    SPELLO(SPELL_IDENTIFY, "identify", 1,
	    POS_STANDING, 7, 5, 10, 10, 95, 95, 95, 95, 30,
	    TAR_OBJ_INV, cast_identify);

    /* SPELL NO 54. is not used. */

    SPELLO(SKILL_SNEAK, "sneak", 1,
	    POS_STANDING, IMO, IMO, 1, IMO, 30, 30, 95, 30, 0,
	    TAR_IGNORE, NOTSPELL);
    SPELLO(SKILL_HIDE, "hide", 1,
	    POS_STANDING, IMO, IMO, 1, IMO, 30, 30, 95, 30, 0,
	    TAR_IGNORE, NOTSPELL);
    SPELLO(SKILL_STEAL, "steal", 1,
	    POS_STANDING, IMO, IMO, 1, IMO, 30, 30, 95, 30, 0,
	    TAR_IGNORE, NOTSPELL);
    /* NOTE: Less learned max for 'backstab' */
    /* SPELLO(58, 1, POS_STANDING, IMO, IMO, 1, IMO, 30, 30, 95, 30, 0, */
    SPELLO(SKILL_BACKSTAB, "backstab", 1,
	    POS_STANDING, IMO, IMO, 1, IMO, 30, 30, 75, 30, 250,
	    TAR_IGNORE, NOTSPELL);
    SPELLO(SKILL_PICK_LOCK, "pick", 1,
	    POS_STANDING, 20, 20, 1, 10, 30, 30, 95, 30, 0,
	    TAR_IGNORE, NOTSPELL);
    SPELLO(SKILL_KICK, "kick", 1,
	    POS_STANDING, IMO, IMO, IMO, 1, 30, 30, 30, 90, 0,
	    TAR_IGNORE, NOTSPELL);
    SPELLO(SKILL_BASH, "bash", 1,
	    POS_STANDING, IMO, IMO, IMO, 1, 30, 30, 30, 90, 0,
	    TAR_IGNORE, NOTSPELL);
    SPELLO(SKILL_RESCUE, "rescue", 1,
	    POS_STANDING, 30, 30, 10, 1, 30, 30, 50, 90, 0,
	    TAR_IGNORE, NOTSPELL);
    SPELLO(SKILL_DOUBLE_ATTACK, "double attack", 1,
	    POS_STANDING, IMO, IMO, 10, 8, 30, 30, 100, 100, 0,
	    TAR_IGNORE, NOTSPELL);
    SPELLO(SKILL_QUADRUPLE_ATTACK, "quadruple attack", 1,
	    POS_STANDING, IMO, IMO, 27, 15, 30, 30, 100, 100, 0,
	    TAR_IGNORE, NOTSPELL);
    SPELLO(SKILL_EXTRA_DAMAGING, "extra damaging", 1,
	    POS_STANDING, IMO, IMO, IMO, 25, 30, 30, 30, 100, 0,
	    TAR_IGNORE, NOTSPELL);

    /* SPELL NO 66-70. is mobile breath spells */
    spells[SPELL_FIRE_BREATH]	= "fire breath";
    spells[SPELL_GAS_BREATH]	= "gas breath";
    spells[SPELL_FROST_BREATH]	= "frost breath";
    spells[SPELL_ACID_BREATH]	= "acid breath";
    spells[SPELL_LIGHTNING_BREATH]	= "lightning breath";

    SPELLO(SPELL_DAMAGE_UP, "damage up", 12,
	    POS_STANDING, 9, 13, 40, 40, 95, 55, 30, 30, 125,
	    TAR_SELF_ONLY, cast_damage_up);
    SPELLO(SPELL_ENERGY_FLOW, "energy flow", 12,
	    POS_FIGHTING, 10, IMO, IMO, IMO, 95, 30, 30, 30, 50,
	    TAR_CHAR_ROOM | TAR_FIGHT_VICT | TAR_SELF_NONO, cast_energyflow);
    SPELLO(SPELL_MANA_BOOST, "mana boost", 12,
	    POS_STANDING, 15, 12, IMO, IMO, 75, 90, 30, 30, 200,
	    TAR_CHAR_ROOM, cast_mana_boost);
    SPELLO(SPELL_VITALIZE, "vitalize", 12,
	    POS_STANDING, 17, 15, IMO, IMO, 75, 90, 30, 30, 200,
	    TAR_CHAR_ROOM, cast_vitalize);
    SPELLO(SPELL_FULL_FIRE, "full fire", 36,
	    POS_FIGHTING, 17, IMO, IMO, IMO, 95, 30, 30, 30, 150,
	    TAR_CHAR_ROOM | TAR_FIGHT_VICT | TAR_SELF_NONO, cast_full_fire);
    SPELLO(SPELL_THROW, "throw", 12,
	    POS_FIGHTING, 22, 33, IMO, IMO, 95, 60, 30, 30, 150,
	    TAR_CHAR_ROOM | TAR_FIGHT_VICT | TAR_SELF_NONO, cast_throw);
    SPELLO(SPELL_FIRESTORM, "firestorm", 12,
	    POS_FIGHTING, 18, 18, IMO, IMO, 90, 90, 30, 30, 100,
	    TAR_IGNORE, cast_firestorm);
    SPELLO(SPELL_CRUSH_ARMOR, "crush armor", 12,
	    POS_STANDING, 19, 18, IMO, IMO, 75, 90, 30, 30, 100,
	    TAR_CHAR_ROOM | TAR_FIGHT_VICT | TAR_SELF_NONO, cast_crush_armor);
    SPELLO(SPELL_FULL_HEAL, "full heal", 12,
	    POS_FIGHTING, IMO, 21, IMO, IMO, 30, 95, 30, 30, 200,
	    TAR_CHAR_ROOM | TAR_SELF_DEF, cast_full_heal);

    SPELLO(SPELL_TRICK, "trick", 12,
	    POS_STANDING, 25, IMO, IMO, IMO, 95, 30, 30, 30, 100,
	    TAR_CHAR_ROOM | TAR_FIGHT_VICT | TAR_SELF_NONO, cast_trick);
    SPELLO(SPELL_HASTE, "haste", 12,
	    POS_STANDING, 17, 20, 40, 40, 95, 95, 30, 30, 50,
	    TAR_CHAR_ROOM | TAR_SELF_DEF, cast_haste);
    SPELLO(SPELL_IMPROVED_HASTE, "improved haste", 12,
	    POS_STANDING, 30, IMO, IMO, IMO, 95, 95, 30, 30, 100,
	    TAR_SELF_ONLY, cast_improved_haste);
    SPELLO(SPELL_FAR_LOOK, "far look", 12,
	    POS_STANDING, 30, 5, 40, 40, 65, 95, 30, 30, 50,
	    TAR_CHAR_WORLD, cast_far_look);
    SPELLO(SPELL_ALL_HEAL, "all heal", 12,
	    POS_FIGHTING, IMO, 30, IMO, IMO, 30, 95, 30, 30, 200,
	    TAR_IGNORE, cast_all_heal);
    SPELLO(SKILL_TORNADO, "tornado", 1,
	    POS_STANDING, IMO, IMO, 20, 13, 30, 30, 75, 90, 0,
	    TAR_IGNORE, NOTSPELL);
    SPELLO(SKILL_LIGHT_MOVE, "lightning move", 1,
	    POS_STANDING, IMO, IMO, 1, IMO, 30, 30, 95, 30, 0,
	    TAR_IGNORE, NOTSPELL);
    SPELLO(SKILL_PARRY, "parry", 1,
	    POS_STANDING, IMO, IMO, 1, 19, 30, 30, 95, 50, 0,
	    TAR_IGNORE, NOTSPELL);
    /* 88 : flash */ /* NOTE: delay: 1 -> 36 */
    SPELLO(SKILL_FLASH, "flash", 36,
	    POS_STANDING, IMO, IMO, 5, IMO, 30, 30, 95, 30, 0,
	    TAR_IGNORE, NOTSPELL);
    /* 89 : multi kick */ /* NOTE: delay: 1 -> 12 */
    SPELLO(SKILL_MULTI_KICK, "multi kick", 12,
	    POS_STANDING, IMO, IMO, IMO, 17, 30, 30, 30, 95, 0,
	    TAR_IGNORE, NOTSPELL);

    SPELLO(SPELL_ENCHANT_ARMOR, "enchant armor", 12,
	    POS_STANDING, 30, IMO, IMO, IMO, 90, 30, 30, 30, 255,
	    TAR_OBJ_INV, cast_enchant_armor);
    SPELLO(SPELL_PRAY_FOR_ARMOR, "pray for armor", 12,
	    POS_STANDING, IMO, 30, IMO, IMO, 30, 90, 30, 30, 255,
	    TAR_OBJ_INV, cast_pray_for_armor);
    SPELLO(SPELL_INFRAVISION, "infravision", 12,
	    POS_STANDING, 7, 5, IMO, IMO, 95, 95, 30, 30, 30,
	    TAR_SELF_ONLY, cast_infravision);
    SPELLO(SPELL_CREATE_NECTAR, "create nectar", 12,
	    POS_STANDING, IMO, 25, IMO, IMO, 30, 95, 30, 30, 150,
	    TAR_OBJ_INV, cast_create_nectar);
    SPELLO(SPELL_CREATE_GOLDEN_NECTAR, "create golden nectar", 12,
	    POS_FIGHTING, IMO, 33, IMO, IMO, 30, 95, 30, 30, 255,
	    TAR_OBJ_INV, cast_create_golden_nectar);
    /* NOTE: less learned max and more mana consumption for 'cone of ice' */
/* SPELLO(95, 12, POS_FIGHTING, 30, IMO, IMO, IMO, 95, 30, 30, 30, 150,*/
    SPELLO(SPELL_CONE_OF_ICE, "cone of ice", 12,
	    POS_FIGHTING, 30, IMO, IMO, IMO, 75, 30, 30, 30, 250,
	    TAR_CHAR_ROOM | TAR_FIGHT_VICT | TAR_SELF_NONO, cast_cone_of_ice);
    SPELLO(SPELL_DISINTEGRATE, "disintegrate", 12,
	    POS_FIGHTING, 33, IMO, IMO, IMO, 95, 30, 30, 30, 150,
	 TAR_CHAR_ROOM | TAR_FIGHT_VICT | TAR_SELF_NONO, cast_disintegrate);
    SPELLO(SPELL_PHASE, "phase", 12,
	    POS_STANDING, 20, 14, IMO, IMO, 75, 95, 30, 30, 50,
	    TAR_IGNORE, cast_phase);
    SPELLO(SPELL_MIRROR_IMAGE, "mirror image", 12,
	    POS_STANDING, 35, IMO, IMO, IMO, 95, 30, 30, 30, 100,
	    TAR_SELF_ONLY, cast_mirror_image);
    SPELLO(SPELL_HAND_OF_GOD, "hand of god", 12,
	    POS_FIGHTING, IMO, 35, IMO, IMO, 30, 95, 30, 30, 200,
	    TAR_IGNORE, cast_hand_of_god);
    SPELLO(SPELL_CAUSE_LIGHT, "cause light", 12,
	    POS_FIGHTING, IMO, 1, IMO, IMO, 30, 95, 30, 30, 15,
	    TAR_CHAR_ROOM | TAR_FIGHT_VICT | TAR_SELF_NONO, cast_cause_light);
    SPELLO(SPELL_CAUSE_CRITIC, "cause critic", 12,
	    POS_FIGHTING, IMO, 7, IMO, IMO, 30, 95, 30, 30, 15,
	 TAR_CHAR_ROOM | TAR_FIGHT_VICT | TAR_SELF_NONO, cast_cause_critic);
    /* SPELLO(102, 19, POS_FIGHTING, IMO, 30, IMO, IMO, 30, 30, 30, 30,
       255, TAR_CHAR_ROOM, cast_entire_heal); */
    SPELLO(SPELL_ENTIRE_HEAL, "entire heal", 24,
	    POS_FIGHTING, IMO, IMO, IMO, IMO, 30, 30, 30, 30, 255,
	    TAR_CHAR_ROOM | TAR_SELF_DEF, cast_entire_heal);

    SPELLO(SKILL_OCTA_ATTACK, "octa attack", 1,
	    POS_STANDING, IMO, IMO, IMO, 20, 30, 30, 30, 100, 0,
	    TAR_IGNORE, NOTSPELL);
    SPELLO(SPELL_LOVE, "love", 12,
	    POS_STANDING, IMO, 13, IMO, IMO, 30, 95, 30, 30, 100,
	    TAR_CHAR_ROOM | TAR_SELF_DEF, cast_love);
    SPELLO(SKILL_DISARM, "disarm", 1,
	    POS_FIGHTING, IMO, IMO, 1, 10, 30, 30, 39, 30, 0,
	    TAR_CHAR_ROOM, NOTSPELL);
    SPELLO(SPELL_RERAISE, "reraise", 12,
	    POS_STANDING, IMO, 40, IMO, IMO, 30, 75, 30, 30, 255,
	    TAR_SELF_ONLY, cast_reraise);
    SPELLO(SKILL_SHOURYUKEN, "shouryuken", 1,
	    POS_FIGHTING, IMO, IMO, IMO, 30, 30, 30, 30, 95, 250,
	    TAR_CHAR_ROOM, NOTSPELL);
    SPELLO(SKILL_THROW_OBJECT, "throw object", 1,
	    POS_FIGHTING, IMO, IMO, 30, IMO, 30, 30, 95, 30, 0,
	    TAR_CHAR_ROOM, NOTSPELL);
    SPELLO(SKILL_PUNCH, "punch", 1,
	    POS_FIGHTING, IMO, IMO, 30, 25, 30, 30, 50, 95, 0,
	    TAR_CHAR_ROOM, NOTSPELL);
    SPELLO(SPELL_DEATH, "death", 12,
	    POS_STANDING, 30, 30, 30, 30, 95, 95, 95, 95, 250,
	    TAR_SELF_ONLY, cast_death);	/* death by process */
    SPELLO(SPELL_ENCHANT_PERSON, "enchant person", 12,
	    POS_STANDING, 30, 30, 40, 40, 70, 70, 30, 30, 200,
	    TAR_SELF_ONLY, cast_enchant_person);		/* by wings */
    SPELLO(SPELL_SPELL_BLOCK, "spell block", 12,
	    POS_STANDING, 30, 20, IMO, IMO, 70, 80, 30, 30, 200,
	    TAR_CHAR_ROOM, cast_spell_block);	/* by process */
    SPELLO(SKILL_TEMPTATION, "temptation", 1,
	    POS_STANDING, 10, 10, 10, 10, 39, 39, 39, 39, 0,
	    TAR_CHAR_ROOM, NOTSPELL);	/* temptation by process */

    /* SPELL NO. 114 : shadow figure */

    SPELLO(SPELL_MANA_TRANSFER, "mana transfer", 12,
	    POS_SITTING, 30, 20, IMO, IMO, 70, 90, 30, 30, 255,
	    TAR_CHAR_ROOM, cast_mana_transfer);	/* by process */
    SPELLO(SPELL_SELF_HEAL, "self heal", 12,
	    POS_FIGHTING, 17, 13, 20, 20, 80, 95, 70, 70, 100,
	    TAR_SELF_ONLY, cast_self_heal);	/* by process */
    SPELLO(SPELL_HOLY_SHIELD, "holy shield", 12,
	    POS_STANDING, IMO, 40, IMO, IMO, 30, 80, 30, 30, 120,
	    TAR_SELF_ONLY, cast_holy_shield);	/* by process */
    SPELLO(SPELL_RESTORE_MOVE, "restore move", 12,
	    POS_FIGHTING, IMO, IMO, 30, 40, 30, 30, 49, 30, 200,
	    TAR_SELF_ONLY, cast_restore_move);	/* by ares */
    SPELLO(SPELL_KISS_OF_PROCESS, "heal the world", 12,
	    POS_STANDING, IMO, IMO, IMO, IMO, 30, 39, 30, 30, 200,
	    TAR_IGNORE, cast_kiss_of_process);	/* by process */
    SPELLO(SPELL_REFLECT_DAMAGE, "reflect damage", 12,
	    POS_STANDING, 35, 30, IMO, IMO, 50, 80, 30, 30, 200,
	    TAR_SELF_ONLY, cast_reflect_damage);		/* by process */
    SPELLO(SPELL_DUMB, "dumb", 12,
	    POS_STANDING, 30, 30, IMO, IMO, 39, 39, 30, 30, 255,
	    TAR_CHAR_ROOM | TAR_SELF_NONO, cast_dumb);
    SPELLO(SKILL_SPIN_BIRD_KICK, "spin bird kick", 1,
	    POS_FIGHTING, IMO, IMO, IMO, 30, 30, 30, 30, 95, 250,
	    TAR_CHAR_ROOM, NOTSPELL);
    SPELLO(SPELL_THUNDERBOLT, "thunderbolt", 12,
	    POS_STANDING, 30, IMO, IMO, IMO, 39, 30, 30, 30, 200,
	    TAR_IGNORE, cast_thunderbolt);

    /* SPELL NO. 124 : arrest */

    SPELLO(SPELL_SANCTUARY_CLOUD, "sanctuary cloud", 12,
	    POS_STANDING, IMO, 40, IMO, IMO, 30, 80, 30, 30, 255,
	    TAR_IGNORE, cast_sanctuary_cloud);
    SPELLO(SKILL_TRACK, "track", 12,
	    POS_STANDING, 20, 20, 1, 10, 39, 39, 70, 39, 100,
	    TAR_IGNORE, NOTSPELL);

    spells[TYPE_HIT]		= "type hit";
    spells[TYPE_SLASH]		= "type slash";
    spells[TYPE_PIERCE]		= "type pierce";
    spells[TYPE_BLUDGEON]	= "type bludgeon";
    spells[TYPE_CRUSH]		= "type crush";
    spells[TYPE_WHIP]		= "type whip";
    spells[TYPE_BITE]		= "type bite";
    spells[TYPE_STING]		= "type sting";
    spells[TYPE_SHOOT]		= "type shoot";

    spells[WEAPON_ANY_MAGIC]	= "weapon any magic";
    spells[WEAPON_SMASH]	= "weapon smash";
    spells[WEAPON_FLAME]	= "weapon flame";
    spells[WEAPON_ICE]		= "weapon ice";
    spells[WEAPON_BOMBARD]	= "weapon bombard";
    spells[WEAPON_SHOT]		= "weapon shot";
    spells[WEAPON_DRAGON_SLAYER]	= "weapon dragon slayer";
    spells[WEAPON_ANTI_EVIL_WEAPON]	= "weapon anti evil";
    spells[WEAPON_ANTI_GOOD_WEAPON]	= "weapon anti good";
    spells[WEAPON_ANTI_ALIGN]	= "weapon anti align" ;

    spells[TYPE_SUFFERING] 	= "type suffering";

    spells[MAXSPELL] =  "\n";
}

#ifdef UNUSED_CODE
/* NOTE: NEW to access spell_info structure. get minimum level of that class.*/
int min_level(int spell , int class)
{
    ASSERT( spell >= 0 && spell <= MAXSPELL);
    return(spell_info[spell].min_level[class-1]);
}
#endif		/* UNUSED_CODE */
