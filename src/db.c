/**************************************************************************
*  file: db.c , Database module.                          Part of DIKUMUD *
*  Usage: Loading/Saving chars, booting world, resetting etc.             *
*  Copyright (C) 1990, 1991 - see 'license.doc' for complete information. *
***************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>

#include "char.h"
#include "object.h"
#include "global.h"
#include "comm.h"
#include "play.h"
#include "gamedb.h" 

/**************************************************************************
*  declarations of most of the 'global' variables                         *
************************************************************************ */

struct room_data *world;		/* dyn alloc'ed array of rooms     */
int top_of_world = 0;			/* ref to the top element of world */

struct obj_data *object_list = 0;	/* the global linked list of obj's */
struct char_data *character_list = 0;	/* global l-list of chars          */

struct zone_data *zone_table;	/* table of reset data		*/
int top_of_zone_table = 0;

struct player_index_element *player_table;  /* index to player file */
int top_of_p_table = -1;		/* ref to top of table		*/

#ifdef    NO_DEF
char credits[MAX_STRING_LENGTH];	/* the Credits List		*/
char news[MAX_STRING_LENGTH];		/* the news			*/
char imotd[MAX_STRING_LENGTH];		/* MOTD for immortals		*/
char motd[MAX_STRING_LENGTH];		/* the messages of today	*/
char plan[MAX_STRING_LENGTH];		/* the info text		*/
char wizards[MAX_STRING_LENGTH];	/* the wizards text		*/
char help[MAX_STRING_LENGTH];		/* the main help page		*/
#endif		/* NO_DEF */

/* NOTE: Old credits[], news[], plan[], wizard[] was not used now. 
	news, plan, credits, wizards is read on the fly.	*/

char motd[MAX_STRING_LENGTH];		/* the messages of today	*/
char *imotd = motd;			/* MOTD for immortals		*/

FILE *mob_f,				/* file containing mob prototypes  */
   *obj_f;				/* obj prototypes                  */

struct index_data *mob_index;		/* index table for mobile file	*/
struct index_data *obj_index;		/* index table for object file	*/

int top_of_mobt = 0;			/* top of mobile index table	*/
int top_of_objt = 0;			/* top of object index table	*/ 


/*************************************************************************
*  routines for booting the system                                       *
*********************************************************************** */

void reset_time(void);
struct index_data *generate_indices(FILE * fl, int *top);

/* body of the booting system */
void boot_db(void)
{
    extern int no_specials;
    extern void reset_all_zone(void);	/* NOTE: in db.zone.c */

    extern void build_help_index(void);
    extern void init_quest(void); /* quest */
    extern void boot_zones(void);
    extern void boot_world(void);
    extern void renum_world(void);
    extern void renum_zone_table(void);
    extern void build_player_index(void);
    extern void load_messages(void);
    extern void boot_social_messages(void);
    extern void boot_the_shops(void);

    extern void check_zone_data(void);

    extern void assign_mobiles(void);
    extern void assign_objects(void);
    extern void assign_rooms(void);
    extern void assign_command_pointers(void);
    extern void assign_spell_pointers(void);
    extern void assign_the_shopkeepers(void); 

    extern void House_boot(void);

    log("Boot db -- BEGIN.");

    log("Resetting the game time:");
    reset_time();

/*
    log("Reading news, credits, help-page, plan, wizards, and motd.");
    file_to_string(NEWS_FILE, news);
    file_to_string(CREDITS_FILE, credits);
    file_to_string(IMOTD_FILE, imotd);
    file_to_string(MOTD_FILE, motd);
    file_to_string(HELP_PAGE_FILE, help);
    file_to_string(PLAN_FILE, plan);
    file_to_string(WIZARDS_FILE, wizards);
*/
    /* NOTE: news, plan, credits, wizards is not loaded at boot time.	*/
    /* NOTE: Main help page is merged to "help_table" file itself. 	*/ 
    /* NOTE: Motd/Immortal motd are merged to single "motd" file 	*/ 
    /* 	     separated by form-feed(^L) char				*/
    log("Reading motd.");
    if( file_to_string(MOTD_FILE, motd)) {
	for( imotd = motd; *imotd && *imotd != '\f' ; imotd++ ) ;
	if ( *imotd )
	    *(imotd++) = '\0' ;
	else
	    imotd = motd;
    }

    log("Opening mobile, object and help files.");
    if (!(mob_f = fopen(MOB_FILE, "r"))) {
	perror("boot");
	exit(2);
    }
    log("Mob done");

    if (!(obj_f = fopen(OBJ_FILE, "r"))) {
	perror("boot");
	exit(2);
    }
    log("Obj done");

    /* NOTE: Opening help file moved to build_help_index()  */
    build_help_index();
    log("help index done.");

    log("Loading zone table.");
    boot_zones();

    log("Loading rooms.");
    boot_world();

    log("Generating index tables for mobile files.");
    mob_index = generate_indices(mob_f, &top_of_mobt);
    log("Generating index tables for object files.");
    obj_index = generate_indices(obj_f, &top_of_objt);

    /* NOTE: Renumber room *after* loading object file */
    log("Renumbering rooms.");
    renum_world(); 
    check_zone_data();

    log("Renumbering zone table.");
    renum_zone_table();

    log("Generating player index.");
    build_player_index();

    log("Loading fight messages.");
    load_messages();

    log("Loading social messages.");
    boot_social_messages();

    log("Loading shops.");
    boot_the_shops();

    /* NOTE: Init quest after loading mobile file */
    log("quest manager : initialize.");
    init_quest(); 

    /* NOTE: In "house.c". Init house rooms and load items in it */
    log("Loading Houses.");
    House_boot();

    log("Assigning function pointers:");
    if (!no_specials) {
	log("   Mobiles.");
	assign_mobiles();
	log("   Objects.");
	assign_objects();
	log("   Room.");
	assign_rooms();
	log("   Shops.");
	assign_the_shopkeepers(); /* NOTE: shopkeeper is here */
    }

    log("   Commands.");
    assign_command_pointers();
    log("   Spells.");
    assign_spell_pointers();

    reset_all_zone();		/* NOTE: Boot time zone reset in "db.zone.c" */
    log("Boot db -- DONE."); 
}

/* reset the time in the game from file */
void reset_time(void)
{
    char buf[MAX_BUFSIZ];

    /* long beginning_of_time = 650336715; */
    /* NOTE: b.o.t = 0 In Real World: Sat Sep 27 16:05:40 KST 1997, Mud Time: 
       The 33rd Day of the Month of the Heat, Year 1021.  */
    long beginning_of_time = 0;
    struct time_info_data mud_time_passed(time_t t2, time_t t1);

    time_info = mud_time_passed(time(0), beginning_of_time);

    switch (time_info.hours) {
    case 0: case 1: case 2: case 3: case 4:
	    weather_info.sunlight = SUN_DARK;
	    break;
    case 5:
	    weather_info.sunlight = SUN_RISE;
	    break;
    case 6: case 7: case 8: case 9: case 10:
    case 11: case 12: case 13: case 14: case 15:
    case 16: case 17: case 18: case 19: case 20:
	    weather_info.sunlight = SUN_LIGHT;
	    break;
    case 21:
	    weather_info.sunlight = SUN_SET;
	    break;
    case 22: case 23:
    default:
	    weather_info.sunlight = SUN_DARK;
	    break;
    }

    sprintf(buf, "   Current Gametime: %dH %dD %dM %dY.",
	    time_info.hours, time_info.day,
	    time_info.month, time_info.year);
    log(buf);

    weather_info.pressure = 960;
    if ((time_info.month >= 7) && (time_info.month <= 12))
	weather_info.pressure += dice(1, 50);
    else
	weather_info.pressure += dice(1, 80);

    weather_info.change = 0;

    if (weather_info.pressure <= 980)
	weather_info.sky = SKY_LIGHTNING;
    else if (weather_info.pressure <= 1000)
	weather_info.sky = SKY_RAINING;
    else if (weather_info.pressure <= 1020)
	weather_info.sky = SKY_CLOUDY;
    else
	weather_info.sky = SKY_CLOUDLESS;
}

/* generate index table for object or monster file */
struct index_data *generate_indices(FILE * fl, int *top)
{
    int i = 0;
    struct index_data *index = NULL;
    char buf[MAX_BUFSIZ];

    rewind(fl);

    for (;;) {
	if (fgets(buf, 81, fl)) {
	    if (*buf == '#') {
		/* allocate new cell */

		if (!i)	/* first cell */
		    CREATE(index, struct index_data, 1);

		else if (!(index =
			   (struct index_data *) realloc(index,
				    (i + 1) * sizeof(struct index_data)))) {
		    perror("load indices");
		    exit(2);
		}
		sscanf(buf, "#%d", &index[i].virtual);
		index[i].pos = ftell(fl);
		index[i].number = 0;
		index[i].func = 0;
		i++;
	    }
	    else if (*buf == '$')	/* EOF */
		break;
	}
	else {
	    perror("generate indices");
	    exit(2);
	}
    }
    *top = i - 2;
    return (index);
} 

/*************************************************************************
*  procedures for resetting, both play-time and boot-time      *
*********************************************************************** */

/* read a mobile from MOB_FILE */
struct char_data *read_mobile(int nr, int type)
{
    int i;
    struct char_data *mob;
    long tmp, tmp2, tmp3;
    char letter;
    int level, level2, level3, abil, class;
    char buf[MAX_BUFSIZ];
    /* #include "mob_bal.c" */

    extern int mob_bal_hit[][20]; 
    extern int mob_bal_ac[][20]; 
    extern int mob_bal_hit[][20]; 
    extern int mob_bal_hr[][20]; 
    extern int mob_bal_dr[][20]; 
    extern int mob_bal_exp[][20]; 

	i = nr;
    if (type == VIRTUAL)
	if ((nr = real_mobile(nr)) < 0) {
	    sprintf(buf, "Mobile (V) %d does not exist in database.", i);
	    log(buf);
	    return (0);
	}

    fseek(mob_f, mob_index[nr].pos, 0);
    CREATE(mob, struct char_data, 1);
    clear_char(mob);

    /***** String data *** */

    mob->player.name = fread_string(mob_f);
    mob->player.short_descr = fread_string(mob_f);
    mob->player.long_descr = fread_string(mob_f);
    mob->player.description = fread_string(mob_f);
    GET_TITLE(mob)= 0;

    /* NOTE: Default description is same as long description.  */
    if(!mob->player.description && mob->player.long_descr) 
	mob->player.description = strdup(mob->player.long_descr); 

    /* *** Numeric data *** */

    fscanf(mob_f, "%ld ", &tmp);
    mob->specials.act = tmp;
    SET_BIT(mob->specials.act, ACT_ISNPC);

    fscanf(mob_f, " %ld ", &tmp);
    mob->specials.affected_by = tmp;

    fscanf(mob_f, " %ld \n", &tmp);
    mob->specials.alignment = tmp;

    /* set mob's class */
    fscanf(mob_f, " %c ", &letter);
    switch (letter) {
    case 'M':
	GET_CLASS(mob) = CLASS_MAGIC_USER;
	break;
    case 'C':
	GET_CLASS(mob) = CLASS_CLERIC;
	break;
    case 'T':
	GET_CLASS(mob) = CLASS_THIEF;
	break;
    case 'W':
	GET_CLASS(mob) = CLASS_WARRIOR;
	break;
    default:
	/* random selection */
	GET_CLASS(mob) = number(1, 4);
    }
    class = GET_CLASS(mob);

    /* set mob's level */
    fscanf(mob_f, " %ld ", &tmp);
    level = MIN(43, tmp);
    level2 = level * level;
    level3 = level2 * level;

    GET_LEVEL(mob) = level;

    /* level dependent data */
    /* NOTE: Enhence mobile's abilities: OLD: abil = level / 4 + 5; */
    abil = level / 3 + 5;
    switch (class) {
    case CLASS_MAGIC_USER:
	mob->abilities.str = number(abil >> 1, abil);
	mob->abilities.str_add = 0;
	mob->abilities.intel = number(abil, 18);
	mob->abilities.wis = number(abil, 17);
	mob->abilities.dex = number(abil >> 1, abil);
	mob->abilities.con = number(abil >> 1, abil);
	break;
    case CLASS_CLERIC:
	mob->abilities.str = number(abil >> 1, abil);
	mob->abilities.str_add = 0;
	mob->abilities.intel = number(abil, 17);
	mob->abilities.wis = number(abil, 18);
	mob->abilities.dex = number(abil >> 1, abil);
	mob->abilities.con = number(abil >> 1, abil);
	break;
    case CLASS_THIEF:
	mob->abilities.str = number(abil, 18);
	mob->abilities.str_add = 0;
	mob->abilities.intel = number(abil >> 1, abil);
	mob->abilities.wis = number(abil >> 1, abil);
	mob->abilities.dex = number(abil, 18);
	mob->abilities.con = number(abil, 17);
	break;
    case CLASS_WARRIOR:
	mob->abilities.str = 18;
	mob->abilities.str_add = number(1, MAX(abil << 3, 100));
	mob->abilities.intel = number(abil >> 1, abil);
	mob->abilities.wis = number(abil >> 1, abil);
	mob->abilities.dex = number(abil, 17);
	mob->abilities.con = number(abil, 18);
	break;
    }
    /* NOTE: mob's mana, move is about half of same level player's  */
    mob->points.mana = mob->points.max_mana =
	number(level2 * mob->abilities.wis / 3,
	    level * mob->abilities.wis * mob->abilities.intel);
    mob->points.move = mob->points.max_move =
	number(level2 * mob->abilities.dex / 3,
	level * mob->abilities.dex * mob->abilities.con);

    /* set hit */
    fscanf(mob_f, " %ld ", &tmp);
    if (tmp < 2) {	/* tmp == 1 */
	mob->points.hit = mob->points.max_hit =
	    number(level3, mob_bal_hit[level - 1][0]);
    }
    else if (tmp > 20) {
	mob->points.hit = mob->points.max_hit =
	    number(mob_bal_hit[level - 1][19], level3 * tmp);
    }
    else {		/* 1 < tmp <= 20 */
	mob->points.hit = mob->points.max_hit =
	    number(mob_bal_hit[level - 1][tmp - 2], mob_bal_hit[level - 1][tmp - 1]);
    }

    /* set armor */
    fscanf(mob_f, " %ld ", &tmp);
    if (tmp < 2) {	/* tmp == 1 */
	tmp2 = 100 - mob_bal_ac[level - 1][0];
	mob->points.armor = 100 - number(level, tmp2);
    }
    else if (tmp > 20) {	/* tmp > 20 */
	tmp2 = 100 - mob_bal_ac[level - 1][19];
	mob->points.armor = 100 - number(tmp2, level * tmp);
    }
    else {		/* 1 < tmp <= 20 */
	tmp2 = 100 - mob_bal_ac[level - 1][tmp - 2];
	tmp3 = 100 - mob_bal_ac[level - 1][tmp - 1];
	mob->points.armor = 100 - number(tmp2, tmp3);
    }

    /* set hitroll */
    fscanf(mob_f, " %ld ", &tmp);
    if (tmp < 2) {	/* tmp == 1 */
	mob->points.hitroll = number(level, mob_bal_hr[level - 1][0]);
    }
    else if (tmp > 20) {
	mob->points.hitroll =
	    number(mob_bal_hr[level - 1][19], level * tmp);
    }
    else {		/* 1 < tmp <= 20 */
	mob->points.hitroll =
	    number(mob_bal_hr[level - 1][tmp - 2], mob_bal_hr[level - 1][tmp - 1]);
    }

    /* set damdice */
    mob->specials.damnodice = number(level / 3, level / 2) + 1;
    mob->specials.damsizedice = number(level / 4, level / 3) + 1;

    /* set damroll */
    fscanf(mob_f, " %ld \n", &tmp);
    if (tmp < 2) {	/* tmp == 1 */
	mob->points.damroll = number(level, mob_bal_dr[level - 1][0]);
    }
    else if (tmp > 20) {
	mob->points.damroll =
	    number(mob_bal_dr[level - 1][19], level * tmp);
    }
    else {		/* 1 < tmp <= 20 */
	mob->points.damroll =
	    number(mob_bal_dr[level - 1][tmp - 2], mob_bal_dr[level - 1][tmp - 1]);
    }

    /* set gold */
    fscanf(mob_f, " %ld ", &tmp);
    mob->points.gold = tmp;

    /* set XP */
    fscanf(mob_f, " %ld ", &tmp);
    if (tmp < 2) {	/* tmp == 1 */
	mob->points.exp = number(level3, mob_bal_exp[level - 1][0]);
    }
    else if (tmp > 20) {
	mob->points.exp =
	    number(mob_bal_exp[level - 1][19], level3 * level * tmp / 10);
    }
    else {		/* 1 < tmp <= 20 */
	mob->points.exp =
	    number(mob_bal_exp[level - 1][tmp - 2], mob_bal_exp[level - 1][tmp - 1]);
    }

    /* set position */
    fscanf(mob_f, " %ld ", &tmp);
    mob->specials.default_pos = mob->specials.position = tmp;

    /* set sex */
    fscanf(mob_f, " %c \n", &letter);
    switch (letter) {
    case 'N':
	mob->player.sex = 0;
	break;
    case 'M':
	mob->player.sex = 1;
	break;
    case 'F':
	mob->player.sex = 2;
	break;
    default:
	mob->player.sex = number(0, 2);
    }

    mob->player.guild = 0;
    mob->player.pk_num = 0;
    mob->player.pked_num = 0;

    mob->player.weight = number(100, 200);
    mob->player.height = number(100, 200);

    for (i = 0; i < 3; i++)
	GET_COND(mob, i) = -1;

    mob->specials.apply_saving_throw[SAVING_PARA] =
	100 - number(GET_LEVEL(mob) >> 1, GET_LEVEL(mob));
    mob->specials.apply_saving_throw[SAVING_PETRI] =
	100 - number(GET_LEVEL(mob) >> 1, GET_LEVEL(mob));
    mob->specials.apply_saving_throw[SAVING_BREATH] =
	100 - number(GET_LEVEL(mob) >> 1, GET_LEVEL(mob));

    /* NOTE: More well prepared mobile with spells */
    switch (class) {
    case CLASS_MAGIC_USER:
	if (level >= 35 && (number(1,10) <= 5 )) {
	    SET_BIT(mob->specials.affected_by, AFF_MIRROR_IMAGE);
	    SET_BIT(mob->specials.affected_by, AFF_REFLECT_DAMAGE);
	}
	if (level >= 30 && (number(1,10) <= 6)) {
	    SET_BIT(mob->specials.affected_by, AFF_IMPROVED_HASTE);
	}
	if (level >= 25 && (number(1,10) <= 7)) {
	    SET_BIT(mob->specials.affected_by, AFF_SANCTUARY);
	}
	if (level >= 17 && (number(1,10) <= 8)) {
	    SET_BIT(mob->specials.affected_by, AFF_HASTE);
	}
	break;
    case CLASS_CLERIC:
	if (level >= 30 && (number(1,10) <= 7 )) {
	    SET_BIT(mob->specials.affected_by, AFF_REFLECT_DAMAGE);
	} 
	if (level >= 13 && (number(1,10) <= 8 )) {
	    SET_BIT(mob->specials.affected_by, AFF_SANCTUARY);
	}
	if (level >= 13 && (number(1,10) <= 5 )) {
	    SET_BIT(mob->specials.affected_by, AFF_LOVE);
	}
	if (level >= 20 && (number(1,10) <= 8)) {
	    SET_BIT(mob->specials.affected_by, AFF_HASTE);
	}
	break;
    case CLASS_THIEF:
	if (level >= 40 && (number(1,10) <= 8)) {
	    SET_BIT(mob->specials.affected_by, AFF_HASTE);
	}
	break;
    case CLASS_WARRIOR:
	if (level >= 40 && (number(1,10) <= 8)) {
	    SET_BIT(mob->specials.affected_by, AFF_HASTE);
	}
	break;
    }
	
    if ( class == CLASS_MAGIC_USER || class == CLASS_CLERIC ) {
	mob->specials.apply_saving_throw[SAVING_HIT_SKILL] =
	    100 - number(GET_LEVEL(mob) >> 1, GET_LEVEL(mob));
	mob->specials.apply_saving_throw[SAVING_SPELL] =
	    100 - number(GET_LEVEL(mob), GET_LEVEL(mob) << 1);
    }
    else if ( class ==  CLASS_THIEF || class == CLASS_WARRIOR ) {
	mob->specials.apply_saving_throw[SAVING_HIT_SKILL] =
	    100 - number(GET_LEVEL(mob), GET_LEVEL(mob) << 1);
	mob->specials.apply_saving_throw[SAVING_SPELL] =
	    100 - number(GET_LEVEL(mob) >> 1, GET_LEVEL(mob));
    }

    mob->tmpabilities = mob->abilities;

    mob->player.time.birth = time(0);
    mob->player.time.played = 0;
    mob->player.time.logon = time(0);

    for (i = 0; i < MAX_WEAR; i++)	/* Initialisering Ok */
	mob->equipment[i] = 0;

    mob->nr = nr;
    mob->desc = 0;

    for (i = 0; i < MAX_SKILLS; i++) {
	if (spell_info[i].min_level[class - 1] <= level) {
	    /* NOTE: More well learned mobs */ 
	    mob->skills[i].learned
		= (level > 40) ? 99 : spell_info[i].max_skill[class - 1];
	}
	else {
	    mob->skills[i].learned = 0;
	}
	/* NOTE: More well skilled mobs */ 
	/* mob->skills[i].skilled = (level > 40) ? level : 0; */
	mob->skills[i].skilled = (level >= 30) ? 30 + (level-30)*5 : level;
	mob->skills[i].recognise = 0;
    }

    if (level > 40)
	mob->regeneration = level << 4;
    else if (level > 35)
	mob->regeneration = level << 2;
    else
	mob->regeneration = level << 1;

    /* quest */
    mob->quest.type = 0;
    mob->quest.data = 0;
    mob->quest.solved = 0;

    /* insert in list */
    mob->next = character_list;
    character_list = mob;

    mob_index[nr].number++;

    /* NOTE: this code moved to assign_mobile() */ 
/*
    if (mob_index[nr].virtual == SON_OGONG)
	son_ogong = mob;
    else if (mob_index[nr].virtual == FOURTH_JANGRO)
	fourth_jangro = mob;
 */ 
    mob->regened = 0;
    mob->specials.fighting = 0;

    /* NOTE: VERY IMPORTANT!!! EXPERIMENTAL!!!     Sun Dec 28 1997
	Mobile hit, maxhit, HR, DR, AC Adjustment. May be controversal. */
    /*
    mob->points.hit = mob->points.max_hit =  mob->points.max_hit /4;
    GET_HITROLL(mob) *= 2; 
    GET_DAMROLL(mob) *= 2;
    GET_AC(mob) -=  level*2 + 50 ;
    */

    return (mob);
}

#ifdef OldMobileFormat
struct char_data *read_mobile(int nr, int type)
{
    int i;
    long tmp, tmp2, tmp3;
    struct char_data *mob;
    char buf[100];
    char letter;
    int data;
    int abil;
    static int mob_abil[44] =
    {
	4,
	5, 5, 5, 6, 6, 6, 7, 7, 7, 7,
	8, 8, 8, 9, 9, 9, 10, 10, 10, 10,
	11, 11, 12, 12, 13, 13, 14, 14, 15, 15,
	15, 15, 15, 16, 16, 16, 17, 17, 18, 18,
	18, 18, 18};

    i = nr;
    if (type == VIRTUAL)
	if ((nr = real_mobile(nr)) < 0) {
	    sprintf(buf, "Mobile (V) %d does not exist in database.", i);
	    return (0);
	}

    fseek(mob_f, mob_index[nr].pos, 0);

    CREATE(mob, struct char_data, 1);
    clear_char(mob);

    /***** String data *** */

    mob->player.name = fread_string(mob_f);
    mob->player.short_descr = fread_string(mob_f);
    mob->player.long_descr = fread_string(mob_f);
    mob->player.description = fread_string(mob_f);
    mob->player.title = 0;

    /* *** Numeric data *** */

    fscanf(mob_f, "%d ", &tmp);
    mob->specials.act = tmp;
    SET_BIT(mob->specials.act, ACT_ISNPC);

    fscanf(mob_f, " %d ", &tmp);
    mob->specials.affected_by = tmp;

    fscanf(mob_f, " %d ", &tmp);
    mob->specials.alignment = tmp;

    fscanf(mob_f, " %c \n", &letter);

    /* new easy monster, for easy balancing */
    if (letter == 'X') {
	/* set mob's class */
	fscanf(mob_f, " %c ", &letter);
	switch (letter) {
	case 'M':
	    mob->player.class = CLASS_MAGIC_USER;
	    break;
	case 'C':
	    mob->player.class = CLASS_CLERIC;
	    break;
	case 'T':
	    mob->player.class = CLASS_THIEF;
	    break;
	case 'W':
	    mob->player.class = CLASS_WARRIOR;
	    break;
	default:
	    log("mob must have a class.");
	    exit(2);
	}

	/* set mob's level */
	fscanf(mob_f, " %ld ", &tmp);
	tmp = MIN(43, tmp);
	GET_LEVEL(mob) = tmp;

	/* level dependent data */
	abil = mob_abil[tmp];
	mob->abilities.str = number(abil >> 1, abil);
	mob->abilities.str_add = 0;
	mob->abilities.intel = number(abil >> 1, abil);
	mob->abilities.wis = number(abil >> 1, abil);
	mob->abilities.dex = number(abil >> 1, abil);
	mob->abilities.con = number(abil >> 1, abil);

	mob->points.mana = mob->points.max_mana =
	    number(GET_LEVEL(mob) * 10, GET_LEVEL(mob) * 20);
	mob->points.move = mob->points.max_move =
	    number(GET_LEVEL(mob) * 20, GET_LEVEL(mob) * 30);
	/* end of level dependent data */

	/* set hardness */
	fscanf(mob_f, " %c \n", &letter);
	switch (letter) {
	    /* A : most hard, E : mose easy */
	case 'A':
	    abil = dice(GET_LEVEL(mob), 3);
	    mob->points.hitroll = abil;
	    mob->points.damroll = abil;
	    abil = dice(GET_LEVEL(mob), 5);
	    mob->points.armor = 100 - abil;
	    if (GET_LEVEL(mob) < 10)
		abil = GET_LEVEL(mob);
	    else if (GET_LEVEL(mob) < 20)
		abil = GET_LEVEL(mob) << 1;
	    else if (GET_LEVEL(mob) < 30)
		abil = GET_LEVEL(mob) << 2;
	    else if (GET_LEVEL(mob) < 40)
		abil = GET_LEVEL(mob) << 3;
	    else
		abil = GET_LEVEL(mob) << 4;
	    abil = dice(abil, 200);
	    GET_PLAYER_MAX_HIT(mob) = GET_HIT(mob) = abil;
	    abil = MAX(5, GET_LEVEL(mob) >> 1);
	    abil = number(5, abil);
	    mob->specials.damnodice = abil;
	    mob->specials.damsizedice = abil;
	    break;
	case 'B':
	    abil = dice(GET_LEVEL(mob), 2);
	    mob->points.hitroll = abil;
	    mob->points.damroll = abil;
	    abil = dice(GET_LEVEL(mob), 4);
	    mob->points.armor = 100 - abil;
	    if (GET_LEVEL(mob) < 10)
		abil = GET_LEVEL(mob);
	    else if (GET_LEVEL(mob) < 20)
		abil = GET_LEVEL(mob) << 1;
	    else if (GET_LEVEL(mob) < 30)
		abil = GET_LEVEL(mob) << 2;
	    else if (GET_LEVEL(mob) < 40)
		abil = GET_LEVEL(mob) << 3;
	    else
		abil = GET_LEVEL(mob) << 4;
	    abil = dice(abil, 100);
	    GET_PLAYER_MAX_HIT(mob) = GET_HIT(mob) = abil;
	    abil = MAX(4, GET_LEVEL(mob) >> 2);
	    abil = number(4, abil);
	    mob->specials.damnodice = abil;
	    mob->specials.damsizedice = abil;
	    break;
	case 'C':
	    abil = GET_LEVEL(mob);
	    mob->points.hitroll = abil;
	    mob->points.damroll = abil;
	    abil = dice(GET_LEVEL(mob), 4);
	    mob->points.armor = 100 - abil;
	    if (GET_LEVEL(mob) < 10)
		abil = GET_LEVEL(mob);
	    else if (GET_LEVEL(mob) < 20)
		abil = GET_LEVEL(mob) << 1;
	    else if (GET_LEVEL(mob) < 30)
		abil = GET_LEVEL(mob) << 2;
	    else if (GET_LEVEL(mob) < 40)
		abil = GET_LEVEL(mob) << 3;
	    else
		abil = GET_LEVEL(mob) << 4;
	    abil = dice(abil, 50);
	    GET_PLAYER_MAX_HIT(mob) = GET_HIT(mob) = abil;
	    abil = MAX(1, GET_LEVEL(mob) >> 3);
	    abil = number(1, abil);
	    mob->specials.damnodice = abil;
	    mob->specials.damsizedice = abil;
	    break;
	case 'D':
	    abil = number(GET_LEVEL(mob) >> 1, GET_LEVEL(mob));
	    mob->points.hitroll = abil;
	    mob->points.damroll = abil;
	    abil = dice(GET_LEVEL(mob), 3);
	    mob->points.armor = 100 - abil;
	    if (GET_LEVEL(mob) < 10)
		abil = GET_LEVEL(mob);
	    else if (GET_LEVEL(mob) < 20)
		abil = GET_LEVEL(mob) << 1;
	    else if (GET_LEVEL(mob) < 30)
		abil = GET_LEVEL(mob) << 2;
	    else if (GET_LEVEL(mob) < 40)
		abil = GET_LEVEL(mob) << 3;
	    else
		abil = GET_LEVEL(mob) << 4;
	    abil = dice(abil, 30);
	    GET_PLAYER_MAX_HIT(mob) = GET_HIT(mob) = abil;
	    abil = MAX(1, GET_LEVEL(mob) >> 3);
	    abil = number(1, abil);
	    mob->specials.damnodice = abil;
	    abil >>= 1;
	    abil = MAX(1, abil);
	    mob->specials.damsizedice = abil;
	    break;
	case 'E':
	    abil = number(1, GET_LEVEL(mob));
	    mob->points.hitroll = abil;
	    mob->points.damroll = abil;
	    abil = dice(GET_LEVEL(mob), 2);
	    mob->points.armor = 100 - abil;
	    if (GET_LEVEL(mob) < 10)
		abil = GET_LEVEL(mob);
	    else if (GET_LEVEL(mob) < 20)
		abil = GET_LEVEL(mob) << 1;
	    else if (GET_LEVEL(mob) < 30)
		abil = GET_LEVEL(mob) << 2;
	    else if (GET_LEVEL(mob) < 40)
		abil = GET_LEVEL(mob) << 3;
	    else
		abil = GET_LEVEL(mob) << 4;
	    abil = dice(abil, 20);
	    GET_PLAYER_MAX_HIT(mob) = GET_HIT(mob) = abil;
	    abil = MAX(1, GET_LEVEL(mob) >> 3);
	    abil = number(1, abil);
	    abil >>= 1;
	    abil = MAX(1, abil);
	    mob->specials.damnodice = abil;
	    mob->specials.damsizedice = abil;
	    break;
	default:
	    log("mob's hardness is unknown.");
	    exit(2);
	}

	fscanf(mob_f, " %ld ", &tmp);
	mob->points.gold = tmp;
	fscanf(mob_f, " %ld \n", &tmp);
	GET_EXP(mob) = tmp;
	fscanf(mob_f, " %ld ", &tmp);
	mob->specials.position = tmp;
	fscanf(mob_f, " %ld ", &tmp);
	mob->specials.default_pos = tmp;
	fscanf(mob_f, " %ld \n", &tmp);
	mob->player.sex = tmp;

	mob->player.guild = 0;
	mob->player.pk_num = 0;
	mob->player.pked_num = 0;

	mob->player.weight = number(100, 200);
	mob->player.height = number(100, 200);

	for (i = 0; i < 3; i++)
	    GET_COND(mob, i) = -1;
	for (i = 0; i < 5; i++)
	    mob->specials.apply_saving_throw[i] =
		100 - number(GET_LEVEL(mob) >> 1, GET_LEVEL(mob) << 1);
    }
    else if (letter == 'S') {
	/* set mob's level */
	fscanf(mob_f, " %ld ", &tmp);
	tmp = MIN(43, tmp);
	GET_LEVEL(mob) = tmp;

	abil = mob_abil[tmp];
	mob->abilities.str = number(abil >> 1, abil);
	mob->abilities.str_add = 0;
	mob->abilities.intel = number(abil >> 1, abil);
	mob->abilities.wis = number(abil >> 1, abil);
	mob->abilities.dex = number(abil >> 1, abil);
	mob->abilities.con = number(abil >> 1, abil);
	/* 
	   mob->abilities.str   = 18; mob->abilities.intel = 11; 
	   mob->abilities.wis   = 11; mob->abilities.dex   = 18;
	   mob->abilities.con   = 11; */

	mob->player.guild = 0;
	mob->player.pk_num = 0;
	mob->player.pked_num = 0;
	fscanf(mob_f, " %ld ", &tmp);
	mob->points.hitroll = 20 - tmp;

	fscanf(mob_f, " %ld ", &tmp);
	mob->points.armor = 10 * tmp;

	fscanf(mob_f, " %ldd%ld+%ld ", &tmp, &tmp2, &tmp3);
	mob->points.max_hit = dice(tmp, tmp2) + tmp3;
	mob->points.hit = mob->points.max_hit;

	fscanf(mob_f, " %ldd%ld+%ld \n", &tmp, &tmp2, &tmp3);
	mob->points.damroll = tmp3;
	mob->specials.damnodice = tmp;
	mob->specials.damsizedice = tmp2;

	mob->points.mana = mob->points.max_mana =
	    number(GET_LEVEL(mob) * 10, GET_LEVEL(mob) * 20);
	mob->points.move = mob->points.max_move =
	    number(GET_LEVEL(mob) * 20, GET_LEVEL(mob) * 30);

	fscanf(mob_f, " %ld ", &tmp);
	mob->points.gold = tmp;
	fscanf(mob_f, " %ld \n", &tmp);
	GET_EXP(mob) = tmp;
	fscanf(mob_f, " %ld ", &tmp);
	mob->specials.position = tmp;
	fscanf(mob_f, " %ld ", &tmp);
	mob->specials.default_pos = tmp;
	fscanf(mob_f, " %ld \n", &tmp);
	mob->player.sex = tmp;
	mob->player.class = number(1, 4);
	mob->player.weight = 200;
	mob->player.height = 198;
	for (i = 0; i < 3; i++)
	    GET_COND(mob, i) = -1;
	for (i = 0; i < 5; i++)
	    mob->specials.apply_saving_throw[i] =
		100 - number(GET_LEVEL(mob) >> 1, GET_LEVEL(mob) << 1);
    }
    else {		/* The old monsters are down below here */
	perror("Old monster exit! (db.c, read_mobile)");
	log("Old monster exit in mob file! (db.c, read_mobile)");
	exit(2);
    }

    mob->tmpabilities = mob->abilities;

    mob->player.time.birth = time(0);
    mob->player.time.played = 0;
    mob->player.time.logon = time(0);

    for (i = 0; i < MAX_WEAR; i++)	/* Initialisering Ok */
	mob->equipment[i] = 0;

    mob->nr = nr;
    mob->desc = 0;

    data = MIN(95, GET_LEVEL(mob) * 2);
    for (i = 0; i < MAX_SKILLS; i++) {
	mob->skills[i].learned = data;
	mob->skills[i].recognise = 0;
    }
    mob->regeneration = 0;

    /* insert in list */

    mob->next = character_list;
    character_list = mob;

    mob_index[nr].number++;

    return (mob);
}
#endif				/* OldMobileFormat */

/* read an object from OBJ_FILE */
struct obj_data *read_object(int nr, int type)
{
    struct obj_data *obj;
    int tmp, i;
    char chk[50], buf[100];
    struct extra_descr_data *new_descr;

    i = nr;
    if (type == VIRTUAL)
	if ((nr = real_object(nr)) < 0) {
	    sprintf(buf, "Object (V) %d does not exist in database.", i);
	    return (0);
	}

    fseek(obj_f, obj_index[nr].pos, 0);

    CREATE(obj, struct obj_data, 1);

    clear_object(obj);

    /* *** string data *** */

    obj->name = fread_string(obj_f);
    obj->short_description = fread_string(obj_f);
    obj->description = fread_string(obj_f);
    obj->action_description = fread_string(obj_f);

    /* *** numeric data *** */

    fscanf(obj_f, " %d ", &tmp);
    obj->obj_flags.type_flag = tmp;
    fscanf(obj_f, " %d ", &tmp);
    obj->obj_flags.extra_flags = tmp;
    fscanf(obj_f, " %d ", &tmp);
    obj->obj_flags.wear_flags = tmp;
    fscanf(obj_f, " %d ", &tmp);
    obj->obj_flags.value[0] = tmp;
    fscanf(obj_f, " %d ", &tmp);
    obj->obj_flags.value[1] = tmp;
    fscanf(obj_f, " %d ", &tmp);
    obj->obj_flags.value[2] = tmp;
    fscanf(obj_f, " %d ", &tmp);
    obj->obj_flags.value[3] = tmp;
    fscanf(obj_f, " %d ", &tmp);
    obj->obj_flags.weight = tmp;
    fscanf(obj_f, " %d \n", &tmp);
    obj->obj_flags.cost = tmp;
    fscanf(obj_f, " %d \n", &tmp);
    obj->obj_flags.gpd = tmp;

    /* *** extra descriptions *** */

    obj->ex_description = 0;

    while (fscanf(obj_f, " %s \n", chk), *chk == 'E') {
	CREATE(new_descr, struct extra_descr_data, 1);

	new_descr->keyword = fread_string(obj_f);
	new_descr->description = fread_string(obj_f);

	new_descr->next = obj->ex_description;
	obj->ex_description = new_descr;
    }

    for (i = 0; (i < MAX_OBJ_AFFECT) && (*chk == 'A'); i++) {
	fscanf(obj_f, " %d ", &tmp);
	obj->affected[i].location = tmp;
	fscanf(obj_f, " %d \n", &tmp);
	obj->affected[i].modifier = tmp;
	fscanf(obj_f, " %s \n", chk);
    }

    for (; (i < MAX_OBJ_AFFECT); i++) {
	obj->affected[i].location = APPLY_NONE;
	obj->affected[i].modifier = 0;
    }

    obj->in_room = NOWHERE;
    obj->next_content = 0;
    obj->carried_by = 0;
    obj->in_obj = 0;
    obj->contains = 0;
    obj->item_number = nr;

    obj->next = object_list;
    object_list = obj;

    obj_index[nr].number++;


    return (obj);
} 


/* returns the real number of the room with given virtual number */
int real_room(int virtual)
{
    int bot, top, mid;

    bot = 0;
    top = top_of_world;

    /* perform binary search on world-table */
    for (;;) {
	mid = (bot + top) / 2;

	if ((world + mid)->number == virtual)
	    return (mid);
	if (bot >= top) {
	    fprintf(stderr, "Room %d does not exist in database\n", virtual);
	    return (-1);
	}
	if ((world + mid)->number > virtual)
	    top = mid - 1;
	else
	    bot = mid + 1;
    }
}

/* returns the real number of the monster with given virtual number */
int real_mobile(int virtual)
{
    int bot, top, mid;

    bot = 0;
    top = top_of_mobt;

    /* perform binary search on mob-table */
    for (;;) {
	mid = (bot + top) / 2;

	if ((mob_index + mid)->virtual == virtual)
	    return (mid);
	if (bot >= top)
	    return (-1);
	if ((mob_index + mid)->virtual > virtual)
	    top = mid - 1;
	else
	    bot = mid + 1;
    }
}

/* returns the real number of the object with given virtual number */
int real_object(int virtual)
{
    int bot, top, mid;

    bot = 0;
    top = top_of_objt;

    /* perform binary search on obj-table */
    for (;;) {
	mid = (bot + top) / 2;

	if ((obj_index + mid)->virtual == virtual)
	    return (mid);
	if (bot >= top)
	    return (-1);
	if ((obj_index + mid)->virtual > virtual)
	    top = mid - 1;
	else
	    bot = mid + 1;
    }
} 

/* read and allocate space for a '~'-terminated string from a given file */
char *fread_string(FILE * fl)
{
    char buf[MAX_STR_LEN], tmp[MAX_STR_LEN+100];
    char *rslt;
    register char *point;
    int flag;

    bzero(buf, MAX_STR_LEN);
    do {
	if (!fgets(tmp, MAX_STRING_LENGTH, fl)) {
	    /* NOTE: more informative log on file read error. */
	    sprintf(tmp, "fread_str(): buf = [%s]", buf);
	    perror(tmp);
	    return(NULL);
	}
	if (strlen(tmp) + strlen(buf) > MAX_STR_LEN) {
	    log("fread_string: string too large (db.c)");
	    buf[70] = 0;
	    fprintf(stderr, "%s\n", buf);
	    exit(2);
	}
	else
	    strcat(buf, tmp);

	for (point = buf + strlen(buf) - 2; point >= buf && isspace(*point);
	     point--) ;
	if ((flag = (*point == '~')))
	    if (*(buf + strlen(buf) - 3) == '\n') {
		*(buf + strlen(buf) - 2) = '\r';
		*(buf + strlen(buf) - 1) = '\0';
	    }
	    else
		*(buf + strlen(buf) - 2) = '\0';
	else {
	    *(buf + strlen(buf) + 1) = '\0';
	    *(buf + strlen(buf)) = '\r';
	}
    }
    while (!flag);

    /* do the allocate boogie  */

    if (strlen(buf) > 0) {
	CREATE(rslt, char, strlen(buf) + 1);

	strcpy(rslt, buf);
    }
    else
	rslt = 0;
    return (rslt);
} 

/* NOTE: Increase max buf size to 10-fold (20K), and save memory, too  */
/*     If 2nd parameter is null, use malloc()'ed memory and return it.   */
/* read contents of a text file, and place in buf */
char *file_to_string(char *name, char *sbuf)
{
    FILE *fl;
    char *buf, *bp, tmp[100] ; 
    int buflen;

    if(sbuf) {
	buf = sbuf ;
	buflen = MAX_STRING_LENGTH;
    }
    else {	/* NOTE: Alloc *BIG* enough buffer */
	CREATE( buf, char, MAX_STRING_LENGTH*10 );
	buflen = MAX_STRING_LENGTH*10;
    }
    *buf = '\0';
    bp = buf; 

    if (!(fl = fopen(name, "r"))) {
	/* NOTE: Log failed file name. */
	sprintf(tmp, "file-to-string: %s", name ); 
	perror(tmp); 
	return (NULL);
    }

    while( fgets( bp, MAX_LINE_LEN-3, fl) != NULL ) { 
	bp += strlen(bp);
	strcpy( bp, "\r" );
	bp ++;
	if ( bp > buf + buflen - MAX_LINE_LEN ) {
	    if ( sbuf ) {
		log("fl->strng: string too big (db.c, file_to_string)");
		buf[40] = '\0';
		log(buf);
		*buf = '\0';
		return (NULL);
	    }
	} 
    }

    fclose(fl); 
    /* NOTE: Shrink buf to return back unused core	*/ 
    if ( !sbuf )
	RECREATE(buf, char, ( bp - buf + 8 ) ); 

    return (buf);
} 

/*
int compare(struct player_index_element *arg1, struct player_index_element
	    *arg2)
{
    return (str_cmp(arg1->name, arg2->name));
}
*/

char *lookup_db(char *keyword)
{
    if( !strcmp(keyword,"paints"))
	return(PAINT_DIR);
    if( !strcmp(keyword,"shop"))
	return(SHOP_FILE);
    if( !strcmp(keyword,"social"))
	return(SOCMESS_FILE);
    if( !strcmp(keyword,"hitmsg"))
	return(MESS_FILE);
    if( !strcmp(keyword,"help"))
	return(HELP_KWRD_FILE);
    else 
	return(NULL);
}

