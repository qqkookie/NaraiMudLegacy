
/* ************************************************************************
   *  file: special.c , Special module.                   Part of DIKUMUD *
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

    mob_index[real_mobile(MOB_LIMBO_PUFF)].func = puff;

    /* Moksha */
    mob_index[real_mobile(MOB_GOBIND)].func = spell_blocker;
    mob_index[real_mobile(MOB_HWATA_GUARD)].func = spell_blocker;	/* Big 94,3,26 */

    /* Wasteland */
    mob_index[real_mobile(MOB_LARGE_HOUND)].func = fido;	/* larger hound */

    /* Midgaard */
    mob_index[real_mobile(MOB_MID_JANITOR)].func = janitor;
    mob_index[real_mobile(MOB_MID_FIDO1)].func = fido;
    mob_index[real_mobile(MOB_MID_FIDO2)].func = fido;

    mob_index[real_mobile(MOB_MID_GUILDMASTER)].func = guild;
    mob_index[real_mobile(MOB_MID_MAYOR)].func = mayor;
    mob_index[real_mobile(MOB_MID_DEATHCURE)].func = deathcure;
    mob_index[real_mobile(MOB_MID_SUPERDC)].func = super_deathcure;
    mob_index[real_mobile(MOB_MID_MASS)].func = mud_message;
    mob_index[real_mobile(MOB_MID_MASS+1)].func = mud_message;
    mob_index[real_mobile(MOB_MID_MASS+2)].func = mud_message;
    mob_index[real_mobile(MOB_MID_MASS+3)].func = mud_message;
    mob_index[real_mobile(MOB_MID_MASS+4)].func = mud_message;
    mob_index[real_mobile(MOB_MID_MASS+5)].func = mud_message;
    mob_index[real_mobile(MOB_MID_MASS+6)].func = mud_message;
    mob_index[real_mobile(MOB_MID_MUSASHI)].func = musashi;
    mob_index[real_mobile(MOB_MID_SUPERMUSASHI)].func = super_musashi;
    mob_index[real_mobile(MOB_MID_BOMBARD)].func = Quest_bombard;
    mob_index[real_mobile(MOB_MID_MOM)].func = mom;
    mob_index[real_mobile(MOB_MID_SINGER)].func = singer;
    mob_index[real_mobile(MOB_MID_ARCHMAGE)].func = archmage;

    /* Perhaps */
    mob_index[real_mobile(MOB_MID_HELPER)].func = perhaps;

    /* MORIA */
    mob_index[real_mobile(MOB_MORIA_SNAKE1)].func = snake;
    mob_index[real_mobile(MOB_MORIA_SNAKE2)].func = snake;
    mob_index[real_mobile(MOB_MORIA_SNAKE3)].func = snake;
    mob_index[real_mobile(MOB_MORIA_SNAKE4)].func = snake;
    mob_index[real_mobile(MOB_DESERT_WORM)].func = snake;
    // NOTE: Missing 1660
    // mob_index[real_mobile(1660)].func = snake;

    /* SEWERS */
    mob_index[real_mobile(MOB_SEWER_SNAKE)].func = snake;

    /* FOREST */
    mob_index[real_mobile(MOB_FOREST_SPIDER1)].func = snake;
    mob_index[real_mobile(MOB_FOREST_SPIDER2)].func = snake;


    /* Death Kingdom : cyb */
    mob_index[real_mobile(MOB_DK_BARBOR)].func = helper;		/* barbaror */
    mob_index[real_mobile(MOB_DK_YURIA)].func = helper;		/* yuria */

    /* Robo city : big cyb */
    mob_index[real_mobile(MOB_RC_GREAT_MAZINGA)].func = great_mazinga;	/* great mazinga */

    /* DaeRimSa */
    /* son_ogong = mob_index[real_mobile(SON_OGONG)]; */
    /* fourth_jangro = mob_index[real_mobile(FOURTH_JANGRO)]; */

    mob_index[real_mobile(DRS_SON_OGONG)].func = son_ogong_func;
    mob_index[real_mobile(DRS_FOURTH_JANGRO)].func = fourth_jangro_func;
    mob_index[real_mobile(DRS_SON_OGONG_CLONE)].func = son_ogong_mirror_func;

    /* GoodBadIsland */
    mob_index[real_mobile(GBI_SAINT_MIRROR)].func = gbisland_saint_mirror;
    mob_index[real_mobile(GBI_LANESSA)].func = gbisland_lanessa;
    mob_index[real_mobile(GBI_CARPIE)].func = gbisland_carpie;
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

    obj_index[real_object(OBJ_BOARD)].func = board;
    obj_index[real_object(OBJ_MAILBOX)].func = mbox;
    obj_index[real_object(OBJ_SLOTMACHINE)].func = slot_machine;
    obj_index[real_object(OBJ_STRINGMACHINE)].func = string_machine;

    obj_index[real_object(OBJ_UNKNOWNSEED)].func = magicseed;
    obj_index[real_object(OBJ_TOTEM)].func = totem;

    obj_index[real_object(DRS_SCROLL)].func = teleport_daerimsa_tower;
    obj_index[real_object(DRS_SAINT_WATER)].func = saint_water;

    /* GoodBadIsland */
    obj_index[real_object(GBI_MAGIC_PAPER)].func = gbisland_magic_paper;
    obj_index[real_object(GBI_SEED_EVIL_POWER)].func = gbisland_seed_evil_power;
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

    world[real_room(ROOM_LIMBO)].funct = safe_house;
    world[real_room(MID_PORTAL)].funct = safe_house;
    world[real_room(ROOM_BOARD)].funct = safe_house;
    world[real_room(ROOM_RECEPTION)].funct = safe_house;

    /* quest room */
    world[real_room(ROOM_QUEST)].funct = quest_room;

    world[real_room(ROOM_METAPHYSICIAN)].funct = metahospital;
    world[real_room(ROOM_REMORTAL)].funct = remortal;
    world[real_room(ROOM_HOSPITAL)].funct = hospital;
    world[real_room(ROOM_MID_PETSHOP)].funct = pet_shops;

    world[real_room(WASTELAND_LABORATORY)].funct = portal;
    world[real_room(NEVERLAND_CLOUD)].funct = neverland;
    world[real_room(ROOM_MID_BANK)].funct = bank;
    world[real_room(ROOM_MID_DUMP)].funct = dump;

    /* taxi */
    world[real_room(TAXI_START_MKTSQ)].funct = taxi;
    world[real_room(TAXI_START_TP)].funct = taxi;

    /* GoodBadIsland */
    world[real_room(GBI_SEA)].funct = gbisland_sea;

    world[real_room(LEVGATE_CLUBSAFARI)].funct = level_gate;
    world[real_room(LEVGATE_ARENA)].funct = level_gate;

/*
   world[real_room(LEVGATE_TB8TH)].funct = level_gate;
   world[real_room(LEVGATE_EASTCASTLE)].funct = level_gate;
   world[real_room(LEVGATE_THALOS)].funct = level_gate;
   world[real_room(LEVGATE_SHIRE)].funct = level_gate;
 */
   // NOTE: unused levelgate
   // world[real_room(LEVGATE_MAGETOWER)].funct = level_gate;

    /* guild entry */
    world[real_room(GUILD_POLICE_GATE)].funct = guild_entry;
    world[real_room(GUILD_POLICE_PRACTICE)].funct = guild_practice_yard;
    world[real_room(GUILD_POLICE_LOCKER)].funct = locker_room;
    /* JAIL ROOM */
    world[real_room(ROOM_POLICE_JAIL)].funct = jail_room;

    world[real_room(GUILD_OUTLAW_GATE)].funct = guild_entry;
    world[real_room(GUILD_OUTLAW_PRACTICE)].funct = guild_practice_yard;
    world[real_room(GUILD_OUTLAW_LOCKER)].funct = locker_room;

    world[real_room(GUILD_ASSASSIN_GATE)].funct = guild_entry;
    world[real_room(GUILD_ASSASSIN_PRACTICE)].funct = guild_practice_yard;
    world[real_room(GUILD_ASSASSIN_LOCKER)].funct = locker_room;

    /* locker room */
    world[real_room(ROOM_MID_LOCKER)].funct = locker_room;

    /* NOTE: jail for banished PC is now re-modeled as jail_room() */
    /* world[real_room(6999)].funct = safe_house; */
    world[real_room(ROOM_JAIL)].funct = jail_room;

    int RC_elecfield[] = {
	125, 135, 138, 148, 116, 117, 126, 127, 128, 129,
	136, 140, 139, 149, 150, 151, 152, 162, -1
    };

    for ( int ii = 0; RC_elecfield[ii] > 0 ; ii++)
	world[real_room(ROBOCITY_BASE + RC_elecfield[ii])].funct = electric_shock;

    /*
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
    */
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

int level_quest[LEVEL_SIZE] =
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


static struct {
    int number;
    char *name;
} zone_data[] = {

    /* NOTE: zone/mob renumbered */
    { 2999,	"The LIMBO" },
    { 3199,	"Northern Midgaard MainCity" },
    { 3299,	"Slum of Midgaard" },
    { 3399,	"Southern Midgaard" },
    { 3499,	"Midgaard Crossroads" },
    { 3899,	"TownHouse" },
    { 14199,	"Graveyard" },
    { 14299,	"Knight's Residence" },
    { 14399,	"Sewer" },
    { 14499,	"Second Sewer" },
    { 14599,	"Sewer MAZE" },
    { 14699,	"The Tunnels" },
    { 14799,	"Muncie Loftwick" },
    { 15299,	"The Corporation" },
    { 15399,	"The East Castle" },
    { 15499,	"Dirk's Castle" },
    { 15599,	"Moria Light" },
    { 15699,	"Moria Cave" },
    { 15799,	"Dragon Tower" },
    { 15899,	"The Dwarven Kingdom" },
    { 16199,	"The Zyekian Lands" },
    { 16299,	"SHIRE" },
    { 16399,	"Haon Dor Light" },
    { 16499,	"Haon Dor Dark" },
    { 16599,	"Arachnos" },
    { 16699,	"Galaxy" },
    { 16799,	"The Death Star" },
    { 16899,	"The Keep of Mahn-Tor" },
    { 17199,	"The Wasteland" },
    { 17299,	"The Great Eastern Desert" },
    { 17399,	"Drow City" },
    { 17499,	"The City of Thalos" },
    { 17699,	"The Wamphyri Aerie" },
    { 17899,	"Death Kingdom" },
    { 17999,	"Mount Olympus" },
    { 18199,	"Dae Rim Sa" },
    { 18299,	"Good-Bad-Island" },
    { 18399,	"Kingdom of Chok" },
    { 18499,	"Kingdom of Wee" },
    { 18699,	"Kingdom of O" },
    { 18899,	"Moo Dang" },
    { 19399,	"Robot City" },
    { 19599,	"KAIST" },
    { 19699,	"Process' Castle" },
    { 19799,	"Easy Land" },
    // { 1499,	"the Houses" },
    // { 1899,	"Village of Midgaard" },
    // { 1399,	"the Mel's Dog-House" },
    // { 2799,	"The NeverLand" },
    { -1,	NULL }
};

char *find_zone(int number)
{
    int i;

    for (i = 0; zone_data[i].number > 0; i++) {
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
    if (IS_ALL_REMOED(ch)) {
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

    if (IS_WIZARD(ch)) {		/* IMO */
	send_to_char_han("QUEST : You can do something.\r\n",
			 "QUEST : 당신은 무엇이든 할 수 있습니다.\r\n", ch);
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
	if (IS_ALL_REMOED(ch)) {
	    if (ch->quest.solved < 30) {

		send_to_char_han("QUEST : You can't request.\r\n",
			   "QUEST : 다른 임무를 맡을 수 없습니다.\r\n", ch);
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
		send_to_char_han("QUEST : You can't request.\r\n",
			   "QUEST : 다른 임무를 맡을 수 없습니다.\r\n", ch);
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
	send_to_char_han("QUEST : Congratulations, You made! Go to QM.\r\n",
		"QUEST : 성공했군요, 축하합니다. QM에게 가보세요.\r\n", ch);
	return;
    }
}

void do_hint(struct char_data *ch, char *arg, int cmd)
{
    int num;
    char buf1[MAX_STRING_LENGTH], buf2[MAX_STRING_LENGTH];
    char *zone;

    if (IS_WIZARD(ch)) {		/* IMO */
	send_to_char_han("QUEST : You can do something.\r\n",
			 "QUEST : 당신은 무엇이든 할 수 있습니다.\r\n", ch);
	return;
    }

    if (IS_MOB(ch)) 	/* MOBILE */
	return;

    /* not initialized */
    if (ch->quest.type == 0) {
	send_to_char_han("QUEST : First, you should type quest.\r\n",
			 "QUEST : 먼저 quest라고 해 보세요.\r\n", ch);
	return;
    }

    /* ch solved quest */
    if (ch->quest.type < 0) {
	send_to_char_han("QUEST : Congratulations, You made! Go to QM.\r\n",
		"QUEST : 성공했군요, 축하합니다. QM에게 가보세요.\r\n", ch);
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

    if (IS_WIZARD(ch)) {		/* IMO */
	send_to_char_han("QUEST : You can do something.\r\n",
			 "QUEST : 당신은 무엇이든 할 수 있습니다.\r\n", ch);
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
	send_to_char_han("QUEST : Congratulations, You made! Go to QM.\r\n",
		"QUEST : 성공했군요, 축하합니다. QM에게 가보세요.\r\n", ch);
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
    char buf[MAX_LINE_LEN], *name;
    int num, size;

    if (!(fp = fopen(QUEST_FILE, "r"))) {
	log("init quest (quest_file)");
	exit(1);
    }

    topQM = 0;
    while (1) {
	fgets(buf, MAX_LINE_LEN - 1, fp);
	if (buf[0] == '$') {	/* end of file */
	    fclose(fp);
	    return;
	}
	// fscanf(fp, " %d ", &(QM[topQM].level));

	sscanf(buf, " %d %d %n", &num, &(QM[topQM].level), &size);
	name = buf + size;

	QM[topQM].virtual = num;

	// NOTE: BUG FIX!!!
	size = strlen(name) +1;
	name[size - 2] = '\0';	//  chop off '\n'
	CREATE(QM[topQM].name, char, size);
	strcpy(QM[topQM].name, name);

	topQM++;

	if (topQM > MaxQuest) {
	    log("Quest Mobiles are overflow.");
	    fclose(fp);
	    return;
	}
	/* NOTE: Check virtual number of mob at boot time. */
	if ( real_mobile(num) < 0 ) {
	    char buf2[MAX_OUT_LEN];
	    sprintf(buf2, "Oops! non-existent quest mob: %s", buf);
	    log(buf2);
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
	    send_to_char_han("QUEST : Congratulations, You made! Go to QM.\r\n",
		"QUEST : 성공했군요, 축하합니다. QM에게 가보세요.\r\n", ch);
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
		if (IS_ALL_REMOED(ch))
		    send_to_char("You can't use that ticket .\r\n", ch);
		else if (obj_index[tmp_obj->item_number].virtual == 7994) {
		    ch->quest.type = 0;
		    do_quest(ch, arg, 302);
		    extract_obj(tmp_obj);
		}
		else {
		    send_to_char("You can't use that ticket here.\r\n", ch);
		}
	    }
	    else {
		send_to_char("You do not have that item.\r\n", ch);
	    }

	    return TRUE;
	}

	return FALSE;
    }

    if (IS_WIZARD(ch)) {		/* IMO */
	send_to_char_han("QUEST : You can do something.\r\n",
			 "QUEST : 당신은 무엇이든 할 수 있습니다.\r\n", ch);
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
			"QUEST : QM gives some coins for your success.\r\n",
		"QUEST : 당신의 성공을 축하하며 QM이 돈을 줍니다.\r\n", ch);
	    break;
	case 3:	/* request ticket */
	    obj = read_object(7994, VIRTUAL);
	    obj_to_char(obj, ch);
	    send_to_char_han(
	      "QUEST : QM gives a ticket for request for your success.\r\n",
				"QUEST : 당신의 성공을 축하하며 QM이 REQUEST용 티켓을 줍니다.\r\n", ch);
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
			  "QUEST : QM gives an armor for your success.\r\n",
	      "QUEST : 당신의 성공을 축하하며 QM이 갑옷을 줍니다.\r\n", ch);
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
			  "QUEST : QM gives a weapon for your success.\r\n",
	      "QUEST : 당신의 성공을 축하하며 QM이 무기를 줍니다.\r\n", ch);
	    break;
	case 7:	/* ticket for AC */
	    obj = read_object(7991, VIRTUAL);
	    obj_to_char(obj, ch);
	    send_to_char_han(
		 "QUEST : QM gives a ticket for meta for your success.\r\n",
				"QUEST : 당신의 성공을 축하하며 QM이 메타용 티켓을 줍니다.\r\n", ch);
	    break;
	case 8:	/* ticket for HR */
	    obj = read_object(7992, VIRTUAL);
	    obj_to_char(obj, ch);
	    send_to_char_han(
		 "QUEST : QM gives a ticket for meta for your success.\r\n",
				"QUEST : 당신의 성공을 축하하며 QM이 메타용 티켓을 줍니다.\r\n", ch);
	    break;
	case 9:	/* ticket for DR */
	    obj = read_object(7993, VIRTUAL);
	    obj_to_char(obj, ch);
	    send_to_char_han(
		 "QUEST : QM gives a ticket for meta for your success.\r\n",
				"QUEST : 당신의 성공을 축하하며 QM이 메타용 티켓을 줍니다.\r\n", ch);
	    break;
	case 0:
	case 2:
	case 4:
	default:
	    send_to_char_han(
		  "QUEST : Hmm. I can't find a gift for you this time.\r\n",
		      "QUEST : 음. 이번에는 줄만한 선물이 없네요.\r\n", ch);
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
    send_to_char_han("QUEST : Come to me, if you made your quest!\r\n",
		     "QUEST : quest를 마치고 제게 오세요!\r\n", ch);
    return TRUE;
}
