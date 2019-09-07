/* ************************************************************************
*  file: spec_assign.c , Special module.                  Part of DIKUMUD *
*  Usage: Procedures assigning function pointers.                         *
*  Copyright (C) 1990, 1991 - see 'license.doc' for complete information. *
************************************************************************* */

#include <stdio.h>
#include "structs.h"
#include "db.h"
#include "guild_list.h"

extern struct room_data *world;
extern struct index_data *mob_index;
extern struct index_data *obj_index;
void boot_the_shops();
void assign_the_shopkeepers();

/* ********************************************************************
*  Assignments                                                        *
******************************************************************** */

/* assign special procedures to mobiles */
void assign_mobiles(void)
{
  int cityguard(struct char_data *ch, int cmd, char *arg);
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
  int thief(struct char_data *ch, int cmd, char *arg);
  int magic_user(struct char_data *ch, int cmd, char *arg);
  int superguard(struct char_data *ch, int cmd, char *arg);
  int dragon(struct char_data *ch, int cmd, char *arg);
  int kickbasher(struct char_data *ch, int cmd, char *arg);
  int spitter(struct char_data *ch, int cmd, char *arg);
  int shooter(struct char_data *ch, int cmd, char *arg);
  int finisher(struct char_data *ch, int cmd, char *arg);
  int spell_blocker(struct char_data *ch, int cmd, char *arg);
  int archmage(struct char_data *ch, int cmd, char *arg);
  int helper(struct char_data *ch, int cmd, char *arg);
  int great_mazinga(struct char_data *ch, int cmd, char *arg);
  int son_ogong_func(struct char_data *ch, int cmd, char *arg);
  int son_ogong_func2(struct char_data *ch, int cmd, char *arg);
  int fourth_jangro_func(struct char_data *ch, int cmd, char *arg);

  /* GoodBadIsland */
  int gbisland_saint_mirror(struct char_data *ch, int cmd, char *arg);
  int gbisland_lanessa(struct char_data *ch, int cmd, char *arg);
  int gbisland_carpie(struct char_data *ch, int cmd, char *arg);

  mob_index[real_mobile(1   )].func = puff;

  /* Moksha */
  mob_index[real_mobile(1550)].func = spell_blocker;
  mob_index[real_mobile(2836)].func = spell_blocker; /* Big 94,3,26 */

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
  mob_index[real_mobile(9528)].func = helper;   /* barbaror */
  mob_index[real_mobile(9562)].func = helper;	/* yuria */

  /* Robo city : big cyb */
  mob_index[real_mobile(15182)].func = great_mazinga ;	/* great mazinga */

  /* DaeRimSa */
#define SON_OGONG			11101
#define FOURTH_JANGRO		11132
  mob_index[real_mobile(SON_OGONG)].func = son_ogong_func;
  mob_index[real_mobile(FOURTH_JANGRO)].func = fourth_jangro_func;

  /* GoodBadIsland */
#define SAINT_MIRROR        23304
#define LANESSA             23303
#define CARPIE              23320
  mob_index[real_mobile(SAINT_MIRROR)].func = gbisland_saint_mirror;
  mob_index[real_mobile(LANESSA)].func = gbisland_lanessa;
  mob_index[real_mobile(CARPIE)].func = gbisland_carpie;

  boot_the_shops();
  assign_the_shopkeepers();
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
  obj_index[real_object(23308)].func = gbisland_magic_paper;
  obj_index[real_object(23309)].func = gbisland_seed_evil_power;
}

/* assign special procedures to rooms */
void assign_rooms(void)
{
  int dump(struct char_data *ch, int cmd, char *arg);
  int pet_shops(struct char_data *ch, int cmd, char *arg);
  int hospital(struct char_data *ch, int cmd, char *arg);
  int metahospital(struct char_data *ch, int cmd, char *arg);
  int remortal(struct char_data *ch, int cmd, char *arg);
  int safe_house(struct char_data *ch, int cmd, char *arg);
  int level_gate(struct char_data *ch, int cmd, char *arg);
  int bank(struct char_data *ch, int cmd, char *arg);
  int portal(struct char_data *ch, int cmd, char *arg);
  int neverland(struct char_data *ch, int cmd, char *arg);
  int electric_shock(struct char_data *ch, int cmd, char *arg);
  int guild_entry(struct char_data *ch,int cmd,char *arg);
  int locker_room(struct char_data *ch,int cmd,char *arg);
  int guild_practice_yard(struct char_data *ch,int cmd,char *arg);
  int taxi(struct char_data *ch,int cmd,char *arg);

  /* quest */
  int quest_room(struct char_data *ch,int cmd,char *arg);

  /* jale */
  int jale_room(struct char_data *ch, int cmd, char *arg);

  /* GoodBadIsland */
  int gbisland_sea(struct char_data *ch, int cmd, char *arg);

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
  world[real_room(   1)].funct = safe_house;
  world[real_room(3001)].funct = safe_house;
  world[real_room(3008)].funct = safe_house;
  world[real_room(6999)].funct = safe_house;
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

  /* JALE ROOM */
  world[real_room(1800)].funct = jale_room;

  /* quest room */
  world[real_room(3081)].funct = quest_room;

  /* GoodBadIsland */
  world[real_room(23301)].funct = gbisland_sea;
}
