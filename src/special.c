
/* ************************************************************************
   *  file: spec_procs.c , Special module.                   Part of DIKUMUD *
   *  Usage: Procedures handling special procedures for object/room/mobile   *
   *  Copyright (C) 1990, 1991 - see 'license.doc' for complete information. *
   ************************************************************************* */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "char.h"
#include "object.h"
#include "global.h"
#include "comm.h"
#include "actions.h"
#include "etc.h"


/* ************************************************************************
   *  file: spec_assign.c , Special module.                  Part of DIKUMUD *
   *  Usage: Procedures assigning function pointers.                         *
   *  Copyright (C) 1990, 1991 - see 'license.doc' for complete information. *
   ************************************************************************* */

/* ********************************************************************
   *  Assignments                                                        *
   ******************************************************************** */

/* assign special procedures to mobiles */
void assign_mobiles(void)
{
    /* extern struct char_data *fourth_jangro; */
    /* extern struct char_data *son_ogong; */

    int guild(struct char_data *ch, int cmd, char *arg);
    int puff(struct char_data *ch, int cmd, char *arg);
    int fido(struct char_data *ch, int cmd, char *arg);
    int janitor(struct char_data *ch, int cmd, char *arg);
    int mayor(struct char_data *ch, int cmd, char *arg);
    int mud_message(struct char_data *ch, int cmd, char *arg);
    int Quest_bombard(struct char_data *ch, int cmd, char *arg);
    int deathcure(struct char_data *ch, int cmd, char *arg);
    int perhaps(struct char_data *ch, int cmd, char *arg);
    int super_deathcure(struct char_data *ch, int cmd, char *arg);
    int mom(struct char_data *ch, int cmd, char *arg);
    int musashi(struct char_data *ch, int cmd, char *arg);
    int super_musashi(struct char_data *ch, int cmd, char *arg);
    int snake(struct char_data *ch, int cmd, char *arg);
    int singer(struct char_data *ch, int cmd, char *arg);
    int spell_blocker(struct char_data *ch, int cmd, char *arg);
    int archmage(struct char_data *ch, int cmd, char *arg);
    int helper(struct char_data *ch, int cmd, char *arg);
    int great_mazinga(struct char_data *ch, int cmd, char *arg);

    /* DaeRimSa */
    int son_ogong_func(struct char_data *ch, int cmd, char *arg);
    int fourth_jangro_func(struct char_data *ch, int cmd, char *arg);
    int son_ogong_mirror_func(struct char_data *ch, int cmd, char *arg);

    /* GoodBadIsland */
    int gbisland_saint_mirror(struct char_data *ch, int cmd, char *arg);
    int gbisland_lanessa(struct char_data *ch, int cmd, char *arg);
    int gbisland_carpie(struct char_data *ch, int cmd, char *arg);

    mob_index[real_mobile(1)].func = puff;

    /* Moksha */
    mob_index[real_mobile(1550)].func = spell_blocker;
    mob_index[real_mobile(2836)].func = spell_blocker;	/* Big 94,3,26 */

    /* Wasteland */
    mob_index[real_mobile(2123)].func = fido;	/* larger hound */

    /* Midgaard */
    mob_index[real_mobile(3061)].func = janitor;
    mob_index[real_mobile(3066)].func = fido;
    mob_index[real_mobile(3062)].func = fido;

    mob_index[real_mobile(3020)].func = guild;
    mob_index[real_mobile(3143)].func = mayor;
    mob_index[real_mobile(3125)].func = deathcure;
    mob_index[real_mobile(3135)].func = super_deathcure;
    mob_index[real_mobile(3126)].func = mud_message;
    mob_index[real_mobile(3127)].func = mud_message;
    mob_index[real_mobile(3128)].func = mud_message;
    mob_index[real_mobile(3129)].func = mud_message;
    mob_index[real_mobile(3130)].func = mud_message;
    mob_index[real_mobile(3131)].func = mud_message;
    mob_index[real_mobile(3132)].func = mud_message;
    mob_index[real_mobile(3133)].func = musashi;
    mob_index[real_mobile(19999)].func = super_musashi;
    mob_index[real_mobile(3134)].func = Quest_bombard;
    mob_index[real_mobile(3145)].func = mom;
    mob_index[real_mobile(1455)].func = singer;
    mob_index[real_mobile(3072)].func = archmage;

    /* Perhaps */
    mob_index[real_mobile(1000)].func = perhaps;

    /* MORIA */
    mob_index[real_mobile(4000)].func = snake;
    mob_index[real_mobile(4001)].func = snake;
    mob_index[real_mobile(4053)].func = snake;
    mob_index[real_mobile(1660)].func = snake;
    mob_index[real_mobile(5004)].func = snake;
    mob_index[real_mobile(4102)].func = snake;

    /* SEWERS */
    mob_index[real_mobile(7006)].func = snake;

    /* FOREST */
    mob_index[real_mobile(6113)].func = snake;
    mob_index[real_mobile(6114)].func = snake;


    /* Death Kingdom : cyb */
    mob_index[real_mobile(9528)].func = helper;		/* barbaror */
    mob_index[real_mobile(9562)].func = helper;		/* yuria */

    /* Robo city : big cyb */
    mob_index[real_mobile(15182)].func = great_mazinga;	/* great mazinga */

    /* DaeRimSa */
    /* son_ogong = mob_index[real_mobile(SON_OGONG)]; */
    /* fourth_jangro = mob_index[real_mobile(FOURTH_JANGRO)]; */

    mob_index[real_mobile(MOB_SON_OGONG)].func = son_ogong_func;
    mob_index[real_mobile(MOB_FOURTH_JANGRO)].func = fourth_jangro_func;
    mob_index[real_mobile(MOB_SON_OGONG_CLONE)].func = son_ogong_mirror_func;

    /* GoodBadIsland */
    mob_index[real_mobile(SAINT_MIRROR)].func = gbisland_saint_mirror;
    mob_index[real_mobile(LANESSA)].func = gbisland_lanessa;
    mob_index[real_mobile(CARPIE)].func = gbisland_carpie; 
}

/* assign special procedures to objects */
void assign_objects(void)
{
    int totem(struct char_data *ch, int cmd, char *arg);
    int board(struct char_data *ch, int cmd, char *arg);
    int mbox(struct char_data *ch, int cmd, char *arg);
    int magicseed(struct char_data *ch, int cmd, char *arg);
    int teleport_daerimsa_tower(struct char_data *ch, int cmd, char *arg);
    int slot_machine(struct char_data *ch, int cmd, char *arg);
    int string_machine(struct char_data *ch, int cmd, char *arg);
    int saint_water(struct char_data *ch, int cmd, char *arg);

    /* GoodBadIsland */
    int gbisland_magic_paper(struct char_data *ch, int cmd, char *arg);
    int gbisland_seed_evil_power(struct char_data *ch, int cmd, char *arg);

    obj_index[real_object(3099)].func = board;
    obj_index[real_object(3098)].func = mbox;
    obj_index[real_object(1311)].func = totem;
    obj_index[real_object(2157)].func = magicseed;

    obj_index[real_object(11132)].func = teleport_daerimsa_tower;

    obj_index[real_object(7999)].func = slot_machine;
    obj_index[real_object(8000)].func = string_machine;
    obj_index[real_object(11134)].func = saint_water;

    /* GoodBadIsland */
    obj_index[real_object(GBISLAND_MAGIC_PAPER)].func = gbisland_magic_paper;
    obj_index[real_object(GBISLAND_SEED_EVIL_POWER)].func = gbisland_seed_evil_power;
}

/* assign special procedures to rooms */
void assign_rooms(void)
{
    /* quest */
    int quest_room(struct char_data *ch, int cmd, char *arg);

    extern int dump(struct char_data *ch, int cmd, char *arg);
    extern int pet_shops(struct char_data *ch, int cmd, char *arg);
    extern int hospital(struct char_data *ch, int cmd, char *arg);
    extern int metahospital(struct char_data *ch, int cmd, char *arg);
    extern int remortal(struct char_data *ch, int cmd, char *arg);
    extern int safe_house(struct char_data *ch, int cmd, char *arg);
    extern int level_gate(struct char_data *ch, int cmd, char *arg);
    extern int bank(struct char_data *ch, int cmd, char *arg);
    extern int portal(struct char_data *ch, int cmd, char *arg);
    extern int neverland(struct char_data *ch, int cmd, char *arg);
    extern int electric_shock(struct char_data *ch, int cmd, char *arg);
    extern int guild_entry(struct char_data *ch, int cmd, char *arg);
    extern int locker_room(struct char_data *ch, int cmd, char *arg);
    extern int guild_practice_yard(struct char_data *ch, int cmd, char *arg);
    extern int taxi(struct char_data *ch, int cmd, char *arg); 

    /* jail */
    extern int jail_room(struct char_data *ch, int cmd, char *arg);

    /* GoodBadIsland */
    extern int gbisland_sea(struct char_data *ch, int cmd, char *arg);

    world[real_room(3030)].funct = dump;
    world[real_room(3002)].funct = bank;

    /* guild entry */
    world[real_room(ROOM_GUILD_POLICE)].funct = guild_entry;
    world[real_room(ROOM_GUILD_OUTLAW)].funct = guild_entry;
    world[real_room(ROOM_GUILD_ASSASSIN)].funct = guild_entry;

    /* taxi */
    world[real_room(3014)].funct = taxi;
    world[real_room(3502)].funct = taxi;
    /* locker room */
    world[real_room(ROOM_GUILD_POLICE_LOCKER)].funct = locker_room;
    world[real_room(ROOM_GUILD_OUTLAW_LOCKER)].funct = locker_room;
    world[real_room(ROOM_GUILD_ASSASSIN_LOCKER)].funct = locker_room;
    world[real_room(3000)].funct = locker_room;

    /* guild practice yard */
    world[real_room(ROOM_GUILD_POLICE_PRACTICE)].funct = guild_practice_yard;
    world[real_room(ROOM_GUILD_OUTLAW_PRACTICE)].funct = guild_practice_yard;
    world[real_room(ROOM_GUILD_ASSASSIN_PRACTICE)].funct = guild_practice_yard;

    world[real_room(1453)].funct = level_gate;
    world[real_room(9400)].funct = level_gate;

/*
   world[real_room(2535)].funct = level_gate;
   world[real_room(3500)].funct = level_gate;
   world[real_room(5200)].funct = level_gate;
   world[real_room(6001)].funct = level_gate;
 */

    world[real_room(3031)].funct = pet_shops;
    world[real_room(3039)].funct = remortal;
    world[real_room(3060)].funct = hospital;
    world[real_room(3065)].funct = metahospital;
    world[real_room(1)].funct = safe_house;
    world[real_room(MID_TEMPLE)].funct = safe_house;
    world[real_room(3008)].funct = safe_house;
    /* NOTE: jail for banished PC is now re-modeled jail_room() */
    /* world[real_room(6999)].funct = safe_house; */
    world[real_room(JAIL_ROOM)].funct = jail_room;
    world[real_room(3070)].funct = safe_house;
    world[real_room(2158)].funct = portal;
    world[real_room(2707)].funct = neverland;

    world[real_room(15125)].funct = electric_shock;
    world[real_room(15135)].funct = electric_shock;
    world[real_room(15138)].funct = electric_shock;
    world[real_room(15148)].funct = electric_shock;
    world[real_room(15116)].funct = electric_shock;
    world[real_room(15117)].funct = electric_shock;
    world[real_room(15126)].funct = electric_shock;
    world[real_room(15127)].funct = electric_shock;
    world[real_room(15128)].funct = electric_shock;
    world[real_room(15129)].funct = electric_shock;
    world[real_room(15136)].funct = electric_shock;
    world[real_room(15140)].funct = electric_shock;
    world[real_room(15139)].funct = electric_shock;
    world[real_room(15149)].funct = electric_shock;
    world[real_room(15150)].funct = electric_shock;
    world[real_room(15151)].funct = electric_shock;
    world[real_room(15152)].funct = electric_shock;
    world[real_room(15162)].funct = electric_shock;

    /* JAIL ROOM */
    world[real_room(ROOM_POLICE_JAIL)].funct = jail_room;

    /* quest room */
    world[real_room(3081)].funct = quest_room;

    /* GoodBadIsland */
    world[real_room(GBISLAND_SEA)].funct = gbisland_sea;
}
/* ================================================================ */
/* 
   Quest Management module(source file)
   made by atre@paradise.kaist.ac.kr at 1995/11/09
 */


#define MaxQuest		10000
#define QUEST_FILE		"mob.quest"

struct {
    int virtual;
    int level;
    char *name;
} QM[MaxQuest];

int topQM;

int level_quest[45] =
{
    0,
    0, 0, 0, 0, 0,
    0, 0, 0, 0, 0,	/* 10 */
    2, 3, 4, 5, 6,
    8, 10, 12, 14, 16,	/* 20 */
    18, 20, 22, 24, 26,
    29, 32, 35, 38, 41,	/* 30 */
    44, 47, 50, 53, 56,
    59, 62, 65, 70, 75,	/* 40 */
    101, 110, 120, 150
};


#define ZONE_NUMBER 45
static struct {
    int number;
    char *name;
} zone_data[ZONE_NUMBER] = {

    { 99,	"the LIMBO" },
    { 299,	"the East Castle" },
    { 1399,	"the Mel's Dog-House" },
    { 1499,	"the Houses" },
    { 1599,	"Dirk's Castle" },
    { 1799,	"SHIRE" },
    { 1899,	"The Police Jail" },
    { 1999,	"The Lands" },
    { 2099,	"Process' Castle" },
    { 2199,	"The Wasteland" },
    { 2300,	"Dragon Tower" },
    { 2399,	"Muncie" },
    { 2699,	"The Corporation" },
    { 2799,	"The NeverLand" },
    { 2899,	"Kingdom Of Chok" },
    { 2999,	"The Keep Of MahnTor" },
    { 3099,	"Northern Midgaard MainCity" },
    { 3199,	"Southern Part Of Midgaard" },
    { 3299,	"River Of Midgaard" },
    { 3499,	"Graveyard" },
    { 4199,	"Moria" },
    { 4330,	"The Wamphyri Aerie" },
    { 5099,	"The Great Eastern Desert" },
    { 5199,	"Drow City" },
    { 5299,	"The City Of Thalos" },
    { 6099,	"HAON DOR LIGHT" },
    { 6499,	"HAON DOR DARK" },
    { 6999,	"The Dwarven Kingdom" },
    { 7099,	"SEWER" },
    { 7199,	"Second SEWER" },
    { 7399,	"SEWER MAZE" },
    /* NOTE: { 7899,	"The Tunnels" }, 7500-7899 : Secret Room */
    { 7499,	"The Tunnels" },
    { 7999,	"Redfernes Residence" },
    { 9099,	"Arachnos" },
    { 9499,	"Arena" },
    { 9699,	"Death Kingdom" },
    { 9771,	"Galaxy" },
    { 9851,	"The Death Star" },
    { 12099,	"Easy Zone" },
    /* NOTE: zone/mob renumbered */
    { 12599,	"KAIST" },
    { 12899,	"Moo Dang" },
    { 13199,	"Kingdom Of Wee" },
    { 13399,	"O Kingdom" },
    { 13798,	"Mount Olympus" },
    { 15299,	"Robot City" },
};

char *find_zone(int number)
{
    int i;

    for (i = 0; i < ZONE_NUMBER; i++) {
	if (zone_data[i].number > number)
	    return zone_data[i].name;
    }

    return NULL;
}

int get_quest(struct char_data *ch)
{
    static int width[9] =
    {
	0,		/* 0 *//* 10 level */
	63,		/* 10 *//* 16 level */
	129,		/* 20 *//* 20 level */
	170,		/* 30 *//* 24 level */
	188,		/* 40 *//* 27 level */
	263,		/* 50 *//* 30 level */
	/* 304, */	/* 60 *//* 32 level */
	/* NOTE: quest mob adjustment */ 
	302, 		/* 60 *//* 36 level */
	344,		/* 70 *//* 38 level */
	360,		/* 80 *//* 39 level */
    };

#define END_QUEST_MOBILE 509

    int low, high;
    int t;
    int num;

    /* NOTE: All remortal dangun */
    if (GET_LEVEL(ch) >= 40 && ch->player.remortal == 15 ) {
	low = 284;
	high = END_QUEST_MOBILE;
    }
    /* NOTE: plain dangun */
    else if (GET_LEVEL(ch) == 40  ) {
	low = 345;	/* 39 level */
	high = 430;	/* 40 level */
    }
    else if (ch->quest.solved >= 80 ) {
	low = 267;	/* 34 level mobile */
	high = 348;	/* 38 level mobile */
    }
    else {
	t = ch->quest.solved / 10;
	low = width[t];
	high = width[t + 1];
    }

    do {
	num = number(low, high);
    } while (num == ch->quest.data);

    return num;
}

void do_request(struct char_data *ch, char *arg, int cmd)
{
    int num;
    char buf1[MAX_STRING_LENGTH], buf2[MAX_STRING_LENGTH];

    if (GET_LEVEL(ch) >= IMO) {		/* IMO */
	send_to_char_han("QUEST : You can do something.\n\r",
			 "QUEST : 당신은 무엇이든 할 수 있습니다.\n\r", ch);
	return;
    }

    if (IS_MOB(ch)) 	/* MOBILE */
	return;

    /* request */
    if (ch->quest.type > 0) {

        /*      if error occur, can do request. */
	if (ch->quest.data == 0 ) {
	    ch->quest.type = 0;
	    return;
	}

	/* All remortal player can't do request. */
	/* NOTE: by dyaus : Not so, all remo player can do request with
	   severe panalty . Cost: 10 quest      */
	if ((ch->player.level >= (IMO - 1)) && (ch->player.remortal >= 15)) {
	    if (ch->quest.solved < 30) {

		send_to_char_han("QUEST : You can't request.\n\r",
			   "QUEST : 다른 임무를 맡을 수 없습니다.\n\r", ch);
		return;
	    }
	    else {
		(ch->quest.solved) = (ch->quest.solved) - 10;
		ch->quest.type = 0;
		return;
	    }
	    /* to here by dyaus */
	}

	ch->quest.type = 0;
	if (ch->quest.solved > 0) {
	    (ch->quest.solved)--;
	}
	else {
	    /* 올리모 안한 단군의 request penalty */
	    int xp = number(5000000, 10000000);

	    if (GET_EXP(ch) > xp) {
		GET_EXP(ch) -= xp;
	    }
	    else {
		send_to_char_han("QUEST : You can't request.\n\r",
			   "QUEST : 다른 임무를 맡을 수 없습니다.\n\r", ch);
		return;
	    }
	}
    }

    if (ch->quest.type == 0) {
	/* initialize quest */
	num = get_quest(ch);

	sprintf(buf1, "QUEST : QM proposes you should kill %s.\n", QM[num].name);
	sprintf(buf2, "QUEST : QM은 당신이 %s을 죽일 것을 제안합니다.\n", QM[num].name);
	send_to_char_han(buf1, buf2, ch);

	ch->quest.data = num;
	ch->quest.type = 1;

	return;
    }

    /* ch solved quest */
    if (ch->quest.type < 0) {
	send_to_char_han("QUEST : Congratulations, You made! Go to QM.\n\r",
		"QUEST : 성공했군요, 축하합니다. QM에게 가보세요.\n\r", ch);
	return;
    }
}

void do_hint(struct char_data *ch, char *arg, int cmd)
{
    int num;
    char buf1[MAX_STRING_LENGTH], buf2[MAX_STRING_LENGTH];
    char *zone;

    if (GET_LEVEL(ch) >= IMO) {		/* IMO */
	send_to_char_han("QUEST : You can do something.\n\r",
			 "QUEST : 당신은 무엇이든 할 수 있습니다.\n\r", ch);
	return;
    }

    if (IS_MOB(ch)) 	/* MOBILE */
	return;

    /* not initialized */
    if (ch->quest.type == 0) {
	send_to_char_han("QUEST : First, you should type quest.\n\r",
			 "QUEST : 먼저 quest라고 해 보세요.\n\r", ch);
	return;
    }

    /* ch solved quest */
    if (ch->quest.type < 0) {
	send_to_char_han("QUEST : Congratulations, You made! Go to QM.\n\r",
		"QUEST : 성공했군요, 축하합니다. QM에게 가보세요.\n\r", ch);
	return;
    }

    num = ch->quest.data;

    zone = find_zone(QM[num].virtual);
    if (!zone) {
	sprintf(buf1, "QUEST : Where %s is, I don't know, either.\n", QM[num].name);
	sprintf(buf2, "QUEST : %s? 어디 있는 걸까? 모르겠는데...\n", QM[num].name);
	log("QUEST : INVALID mobile");
    }
    else {
	sprintf(buf1, "QUEST : %s is in %s probably.\n", QM[num].name, zone);
	sprintf(buf2, "QUEST : 아마도 %s는 %s에 있을 걸요.\n", QM[num].name, zone);
    }

    send_to_char_han(buf1, buf2, ch);
}

void do_quest(struct char_data *ch, char *arg, int cmd)
{
    int num;
    char buf1[MAX_STRING_LENGTH], buf2[MAX_STRING_LENGTH];

    if (GET_LEVEL(ch) >= IMO) {		/* IMO */
	send_to_char_han("QUEST : You can do something.\n\r",
			 "QUEST : 당신은 무엇이든 할 수 있습니다.\n\r", ch);
	return;
    }

    if (IS_MOB(ch))	/* MOBILE */
	return;

    /* not initialized */
    if (ch->quest.type == 0) {
	/* initialize quest */
	num = get_quest(ch);

	sprintf(buf1, "QUEST : QM proposes you should kill %s.\n", QM[num].name);
	sprintf(buf2, "QUEST : QM은 당신이 %s을 죽일 것을 제안합니다.\n", QM[num].name);
	send_to_char_han(buf1, buf2, ch);

	ch->quest.data = num;
	ch->quest.type = 1;

	return;
    }

    /* ch solved quest */
    if (ch->quest.type < 0) {
	send_to_char_han("QUEST : Congratulations, You made! Go to QM.\n\r",
		"QUEST : 성공했군요, 축하합니다. QM에게 가보세요.\n\r", ch);
	return;
    }

    num = ch->quest.data;

    sprintf(buf1, "QUEST : QM proposes you should kill %s.\n", QM[num].name);
    sprintf(buf2, "QUEST : QM은 당신이 %s을 죽일 것을 제안합니다.\n", QM[num].name);

    send_to_char_han(buf1, buf2, ch);
}

void init_quest(void)
{
    FILE *fp;
    char buf[MAX_STRING_LENGTH];
    int num, size;

    if (!(fp = fopen(QUEST_FILE, "r"))) {
	log("init quest (quest_file)");
	exit(0);
    }

    topQM = 0;
    while (1) {
	fscanf(fp, "%s", buf);
	if (buf[0] == '$') {	/* end of file */
	    fclose(fp);
	    return;
	}

	num = atoi(buf);
	/* NOTE: Check virtual number of mob at boot time. */
	if ( real_mobile(num) < 0 )
	    log("Oops! non-existent quest mob.");

	QM[topQM].virtual = num;

	fscanf(fp, " %d ", &(QM[topQM].level));

	fgets(buf, MAX_STRING_LENGTH - 1, fp);
	// NOTE: BUG FIX!!!
	size = strlen(buf) +1;
	buf[size - 1] = 0;
	CREATE(QM[topQM].name, char, size);
	strcpy(QM[topQM].name, buf);

	topQM++;

	if (topQM > MaxQuest) {
	    log("Quest Mobiles are overflow.");
	    fclose(fp);
	    return;
	}

    }
}


#ifdef OLD_PRIZE
int give_gift_for_quest(int level)
{
    static int gift_array[100] =
    {
	0, 1, 1, 1, 1, 1, 1, 1, 1, 5,
	1, 1, 1, 1, 1, 1, 1, 1, 1, 3,
	1, 1, 1, 1, 1, 1, 1, 1, 1, 6,
	1, 1, 1, 1, 1, 1, 1, 1, 1, 3,
	1, 1, 1, 1, 1, 1, 1, 1, 1, 7,
	1, 1, 1, 1, 1, 1, 1, 1, 1, 3,
	1, 1, 1, 1, 1, 1, 1, 1, 1, 8,
	1, 1, 1, 1, 1, 1, 1, 1, 3, 3,
	1, 1, 1, 1, 1, 1, 1, 3, 3, 9,
	5, 5, 5, 5, 6, 6, 7, 8, 9, 3
    };
    int num;

    num = number(level * 3 / 2, 99);

    return (gift_array[num]);
}
#else
/* NOTE: Argument type int level -> char_data */
int give_gift_for_quest(struct char_data *ch)
{
    int num;
    static int gift_array[100] =
    {
	0, 1, 1, 1, 1, 1, 1, 1, 1, 5,
	1, 1, 1, 1, 1, 1, 1, 1, 1, 3,
	1, 1, 1, 1, 1, 1, 1, 1, 1, 6,	/* 1 = gold */
	1, 1, 1, 1, 1, 1, 1, 1, 1, 3,	/* 3 = request ticket */
	1, 1, 1, 1, 1, 1, 1, 1, 1, 5,	/* 5 = quest armor */
	1, 1, 1, 1, 1, 1, 1, 1, 1, 3,	/* 6 = quest weapon */
	1, 1, 1, 1, 1, 1, 1, 1, 1, 6,	/* 7 = ticket for AC */
	1, 1, 1, 1, 1, 1, 1, 1, 3, 3,	/* 8 = ticket for HR */
	1, 1, 1, 1, 1, 1, 1, 3, 3, 5,	/* 9 = ticket for DR */
	5, 5, 5, 5, 6, 6, 3, 3, 6, 3
    };
    static int gift_array_dangun[40] =
    {
	1, 1, 1, 1, 1, 1, 1, 1, 1, 8,
	1, 1, 1, 1, 1, 1, 1, 1, 7, 8,
	1, 1, 1, 1, 1, 1, 1, 9, 5, 9,
	5, 5, 5, 5, 6, 6, 7, 8, 9, 1
    };

    if (GET_LEVEL(ch) < 40) {
	num = number(GET_LEVEL(ch) * 3 / 2, 99);
	return (gift_array[num]);
    }
    /* NOTE: Half quest prize for negative solved quest number */
    else if ( ch->quest.solved < 0 && number(0,9) < 5) 
	return(0);
    else {
	num = number(0, 39);
	return (gift_array_dangun[num]);
    }
}
#endif


/* 
NOTE: OLD: void check_quest_mob_die(struct char_data *ch, int mob )
      OLD check ch's quest number with real mobile num
      NEW: Change type of 2nd argument. (mob number -> pointer to mob itself)
 */
void check_quest_mob_die(struct char_data *ch, struct char_data *victim)
{
    if ( !ch || !victim )
	return ;

    if ( IS_NPC(victim)  && (ch->quest.type > 0 )) 
	if ( real_mobile(QM[ch->quest.data].virtual) == victim->nr ) {
	    ch->quest.type = -1;
	    send_to_char_han("QUEST : Congratulations, You made! Go to QM.\n\r",
		"QUEST : 성공했군요, 축하합니다. QM에게 가보세요.\n\r", ch);
	} 
    /* NOTE: checking char's hunting target died moved to adjust_gain()  */ 
}

int quest_room(struct char_data *ch, int cmd, char *arg)
{
    int num;
    char buf[MAX_STRING_LENGTH];
    char buf1[MAX_STRING_LENGTH], buf2[MAX_STRING_LENGTH];
    struct obj_data *obj;

    extern int ac_applicable(struct obj_data *obj_object);

    static char *weapon[] = {
	"STAFF",
	"CLUB",
	"DAGGER",
	"SWORD"
    };
    static int weapon_type[] = {
	7,
	7,
	11,
	3
    };
    static char *wear[] = {
	"RING",
	"PENDANT",
	"ROBE",
	"HELMET",
	"LEGGINGS",
	"BOOTS",
	"GLOVES",
	"ARMPLATES",
	"SHIELD",
	"CLOAK",
	"BELT",
	"BRACELET",
	"MASCOT",
	"KNEEBAND",
	"PLATE"
    };
    static int wear_flags[] = {
	2,
	4,
	8,
	16,
	32,
	64,
	128,
	256,
	512,
	1024,
	2048,
	4096,
	16384,
	65536,
	131072
    };
    struct obj_data *tmp_obj;

    /* quest or use  */
    if (cmd != CMD_QUEST && cmd != CMD_USE) {
	return FALSE;
    }

    if (cmd == CMD_USE) {
	half_chop(arg, buf, buf2);
	if (*buf) {
	    if (strncmp("tickets", buf, strlen(buf))) {
		/* not ticket */
		return FALSE;
	    }

	    tmp_obj = get_obj_in_list_vis(ch, buf, ch->carrying);
	    if (tmp_obj) {
		if ((ch->player.level >= (IMO - 1)) && (ch->player.remortal >= 15))
		    send_to_char("You can't use that ticket .\n\r", ch);
		else if (obj_index[tmp_obj->item_number].virtual == 7994) {
		    ch->quest.type = 0;
		    do_quest(ch, arg, 302);
		    extract_obj(tmp_obj);
		}
		else {
		    send_to_char("You can't use that ticket here.\n\r", ch);
		}
	    }
	    else {
		send_to_char("You do not have that item.\n\r", ch);
	    }

	    return TRUE;
	}

	return FALSE;
    }

    if (GET_LEVEL(ch) >= IMO) {		/* IMO */
	send_to_char_han("QUEST : You can do something.\n\r",
			 "QUEST : 당신은 무엇이든 할 수 있습니다.\n\r", ch);
	return TRUE;
    }

    if (ch->quest.type < 0) {
	switch (give_gift_for_quest(ch)) {
	    int lev2;

	case 1:	/* some gold */
	    /* 
	       GET_GOLD(ch) += number(100000, (500000 * GET_LEVEL(ch)) >> 2); */
	    /* NOTE: Too much quest prize money for low level */
	    assert(GET_LEVEL(ch) > 9);
	    lev2 = (GET_LEVEL(ch) - 7) * (GET_LEVEL(ch) - 6);
	    /* NOTE: level 20 = [ 182K,782K] lev 40 = [1.1M,4.5M] */
	    gain_gold(ch, number(lev2 * 1000, lev2 * 4000));

	    send_to_char_han(
			"QUEST : QM gives some coins for your success.\n\r",
		"QUEST : 당신의 성공을 축하하며 QM이 돈을 줍니다.\n\r", ch);
	    break;
	case 3:	/* request ticket */
	    obj = read_object(7994, VIRTUAL);
	    obj_to_char(obj, ch);
	    send_to_char_han(
	      "QUEST : QM gives a ticket for request for your success.\n\r",
				"QUEST : 당신의 성공을 축하하며 QM이 REQUEST용 티켓을 줍니다.\n\r", ch);
	    break;
	case 5:	/* armor */
	    obj = read_object(7998, VIRTUAL);

	    /* wear flags */
	    num = number(0, 14);
	    obj->obj_flags.wear_flags = wear_flags[num] + 1;

	    /* name */
	    free(obj->name);
	    sprintf(buf1, "%s QUEST ARMOR", wear[num]);
	    CREATE(obj->name, char, strlen(buf1) + 1);

	    strcpy(obj->name, buf1);

	    /* short description */
	    free(obj->short_description);
	    sprintf(buf1, "%s's QUEST %s",
		    GET_NAME(ch), wear[num]);
	    CREATE(obj->short_description, char, strlen(buf1) + 1);

	    strcpy(obj->short_description, buf1);

	    /* armor class */
	    obj->obj_flags.value[0] = number(10, GET_LEVEL(ch) / 2);

	    /* affected */
	    obj->affected[0].location = APPLY_HITROLL;
	    obj->affected[0].modifier = number(10, GET_LEVEL(ch) / 2);
	    obj->affected[1].location = APPLY_DAMROLL;
	    obj->affected[1].modifier = number(10, GET_LEVEL(ch) / 2);

	    /* NOTE: if new quest object is not AC applicable  object like
	       ring, necklace, light,  affact APPLY_ ARMOR in stead of
	       APPLY_HITROLL */

	    /* NOTE: APPLY_AC should be negative for better AC */
	    if (!ac_applicable(obj)) {
		obj->affected[0].location = APPLY_ARMOR;
		obj->affected[0].modifier = -(obj->obj_flags.value[0]);
		obj->obj_flags.value[0] = 0;
	    }

	    obj_to_char(obj, ch);
	    send_to_char_han(
			  "QUEST : QM gives an armor for your success.\n\r",
	      "QUEST : 당신의 성공을 축하하며 QM이 갑옷을 줍니다.\n\r", ch);
	    break;
	case 6:	/* weapon */
	    obj = read_object(7997, VIRTUAL);

	    /* name */
	    free(obj->name);
	    sprintf(buf1, "%s QUEST WEAPON", weapon[GET_CLASS(ch) - 1]);
	    CREATE(obj->name, char, strlen(buf1) + 1);

	    strcpy(obj->name, buf1);

	    /* short description */
	    free(obj->short_description);
	    sprintf(buf1, "%s's QUEST %s",
		    GET_NAME(ch), weapon[GET_CLASS(ch) - 1]);
	    CREATE(obj->short_description, char, strlen(buf1) + 1);

	    strcpy(obj->short_description, buf1);

	    /* dice  obj->obj_flags.value[1] =
	       number(5+(GET_LEVEL(ch)>>3),5+(GET_LEVEL(ch)>>1));
	       obj->obj_flags.value[2] =
	       number(5+(GET_LEVEL(ch)>>3),5+(GET_LEVEL(ch)>>1)); */
	    /* dice */
	    obj->obj_flags.value[1] = number(GET_LEVEL(ch) / 2, GET_LEVEL(ch) / 10);
	    obj->obj_flags.value[2] = number(5 + (GET_LEVEL(ch) >> 3), 5 + (GET_LEVEL(ch) >> 1));

	    /* weapon type */
	    obj->obj_flags.value[3] = weapon_type[GET_CLASS(ch) - 1];

	    /* affected */
	    obj->affected[0].location = 18;
	    obj->affected[0].modifier = number(10, 5 + GET_LEVEL(ch) / 2);
	    obj->affected[1].location = 19;
	    obj->affected[1].modifier = number(10, 5 + GET_LEVEL(ch) / 2);

	    obj_to_char(obj, ch);
	    send_to_char_han(
			  "QUEST : QM gives a weapon for your success.\n\r",
	      "QUEST : 당신의 성공을 축하하며 QM이 무기를 줍니다.\n\r", ch);
	    break;
	case 7:	/* ticket for AC */
	    obj = read_object(7991, VIRTUAL);
	    obj_to_char(obj, ch);
	    send_to_char_han(
		 "QUEST : QM gives a ticket for meta for your success.\n\r",
				"QUEST : 당신의 성공을 축하하며 QM이 메타용 티켓을 줍니다.\n\r", ch);
	    break;
	case 8:	/* ticket for HR */
	    obj = read_object(7992, VIRTUAL);
	    obj_to_char(obj, ch);
	    send_to_char_han(
		 "QUEST : QM gives a ticket for meta for your success.\n\r",
				"QUEST : 당신의 성공을 축하하며 QM이 메타용 티켓을 줍니다.\n\r", ch);
	    break;
	case 9:	/* ticket for DR */
	    obj = read_object(7993, VIRTUAL);
	    obj_to_char(obj, ch);
	    send_to_char_han(
		 "QUEST : QM gives a ticket for meta for your success.\n\r",
				"QUEST : 당신의 성공을 축하하며 QM이 메타용 티켓을 줍니다.\n\r", ch);
	    break;
	case 0:
	case 2:
	case 4:
	default:
	    send_to_char_han(
		  "QUEST : Hmm. I can't find a gift for you this time.\n\r",
		      "QUEST : 음. 이번에는 줄만한 선물이 없네요.\n\r", ch);
	}
	ch->quest.type = 0;
	ch->quest.solved++;
	return TRUE;
    }

    if (ch->quest.type == 0) {
	/* initialize quest */
	num = get_quest(ch);

	sprintf(buf1, "QUEST : QM proposes you should kill %s.\n", QM[num].name);
	sprintf(buf2, "QUEST : QM은 당신이 %s을 죽일 것을 제안합니다.\n", QM[num].name);
	send_to_char_han(buf1, buf2, ch);

	ch->quest.type = 1;
	ch->quest.data = num;

	return TRUE;
    }

    /* not made */
    send_to_char_han("QUEST : Come to me, if you made your quest!\n\r",
		     "QUEST : quest를 마치고 제게 오세요!\n\r", ch);
    return TRUE;
}
