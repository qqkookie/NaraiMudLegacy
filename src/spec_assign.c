/* ************************************************************************
*  file: spec_assign.c , Special module.                  Part of DIKUMUD *
*  Usage: Procedures assigning function pointers.                         *
*  Copyright (C) 1990, 1991 - see 'license.doc' for complete information. *
************************************************************************* */

#include <stdio.h>
#include "structs.h"
#include "db.h"

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
    int super_deathcure(struct char_data *ch, int cmd, char *arg);
    int mom(struct char_data *ch, int cmd, char *arg);
    int seraphim(struct char_data *ch, int cmd, char *arg);
    int musashi(struct char_data *ch, int cmd, char *arg);
    int lag_monster(struct char_data *ch, int cmd, char *arg);
    int snake(struct char_data *ch, int cmd, char *arg);
    int singer(struct char_data *ch, int cmd, char *arg);
    int thief(struct char_data *ch, int cmd, char *arg);
    int magic_user(struct char_data *ch, int cmd, char *arg);
    int superguard(struct char_data *ch, int cmd, char *arg);
    int dragon(struct char_data *ch, int cmd, char *arg);
    int kickbasher(struct char_data *ch, int cmd, char *arg);
    int spitter(struct char_data *ch, int cmd, char *arg);
    int shooter(struct char_data *ch, int cmd, char *arg);
    int spell_blocker(struct char_data *ch, int cmd, char *arg);
    int yuiju(struct char_data *ch, int cmd, char *arg);
    int archmage(struct char_data *ch, int cmd, char *arg);
    int helper(struct char_data *ch, int cmd, char *arg);
    int great_mazinga(struct char_data *ch, int cmd, char *arg);
    int teleport_machine(struct char_data *ch, int cmd, char *arg);
    int school_cold(struct char_data *ch, int cmd, char *arg);
    int sumiae_healer(struct char_data *ch, int cmd, char *arg);
    int school_gracia(struct char_data *ch, int cmd, char *arg);
    int school_nara(struct char_data *ch, int cmd, char *arg);
    int okse_sunin(struct char_data *ch, int cmd, char *arg);
	int donjonkeeper(struct char_data *ch, int cmd, char *arg) ;
	int teleport_demon(struct char_data *ch, int cmd, char *arg) ;
	int bogan(struct char_data *ch, int cmd, char *arg) ;
	int smart_thief(struct char_data *ch, int cmd, char *arg) ;
	int sonogong(struct char_data *ch, int cmd, char *arg) ;
	int son_boonsin(struct char_data *ch, int cmd, char *arg) ;
	int mob_defender (struct char_data *ch, int cmd, char *argument);
    int zorro(struct char_data *ch, int cmd, char *arg);
	int smith(struct char_data *ch, int cmd, char *arg);

    mob_index[real_mobile(1   )].func = puff;

    /* Moksha */
    mob_index[real_mobile(1550)].func = spell_blocker;
    mob_index[real_mobile(2836)].func = spell_blocker; /* Big 94/3/26 */
    mob_index[real_mobile(17002)].func = yuiju; 
	/* blue dragon, jmjeong*/

    /* Wasteland */
    mob_index[real_mobile(2123)].func = fido;	/* larger hound */
  
    /* Midgaard */
    mob_index[real_mobile(3061)].func = janitor;
    mob_index[real_mobile(3066)].func = fido;
    mob_index[real_mobile(3062)].func = fido;

    mob_index[real_mobile(3020)].func = guild;
    mob_index[real_mobile(3143)].func = mayor;
#ifndef DEATHFIGHT
    mob_index[real_mobile(3125)].func = deathcure;
#endif 
    mob_index[real_mobile(3135)].func = super_deathcure;
    mob_index[real_mobile(3126)].func = mud_message;
    mob_index[real_mobile(3127)].func = mud_message;
    mob_index[real_mobile(3128)].func = mud_message;
    mob_index[real_mobile(3129)].func = mud_message;
    mob_index[real_mobile(3130)].func = mud_message;
    mob_index[real_mobile(3131)].func = mud_message;
    mob_index[real_mobile(3132)].func = mud_message;
#ifndef DEATHFIGHT
    mob_index[real_mobile(3133)].func = musashi; 
    mob_index[real_mobile(3096)].func = lag_monster; 
#endif 
    mob_index[real_mobile(3134)].func = Quest_bombard; 
    mob_index[real_mobile(3145)].func = mom;
    mob_index[real_mobile(1455)].func = singer;
    mob_index[real_mobile(3072)].func = archmage;
    mob_index[real_mobile(3101)].func = seraphim;
    mob_index[real_mobile(3007)].func = smith;

    mob_index[real_mobile(3151)].func = bogan;	/* bogan's steal */

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
    mob_index[real_mobile(15023)].func = teleport_machine ;	/* teleport machine */

    /* Mud School : big cyb */
    mob_index[real_mobile(2904)].func = school_cold ;	/* Cold */
    mob_index[real_mobile(2903)].func = school_gracia ;	/* Gracia */
    mob_index[real_mobile(2905)].func = school_nara ;	/* Gracia */

	/* Rest room : healer(sumiae) */
    mob_index[real_mobile(3099)].func = sumiae_healer ;	/* sumiae */

	/* Wee, chok, onara */
    mob_index[real_mobile(2836)].func = okse_sunin ;	/* kangyu */
    mob_index[real_mobile(17010)].func = okse_sunin ;	/* woogil */
    mob_index[real_mobile(16012)].func = okse_sunin ;	/* jwaja */

	/* donjonkeeper */
    mob_index[real_mobile(9012)].func = donjonkeeper ;

	/* neverland */
    mob_index[real_mobile(2703)].func = teleport_demon ;
    mob_index[real_mobile(2704)].func = teleport_demon ;
    mob_index[real_mobile(2705)].func = teleport_demon ;
    mob_index[real_mobile(2706)].func = teleport_demon ;
    mob_index[real_mobile(2707)].func = smart_thief ;

	/* rome */
    mob_index[real_mobile(14030)].func = mob_defender ;	/* Jupiter*/
    mob_index[real_mobile(14031)].func = mob_defender ;	/* Venus */
    mob_index[real_mobile(14032)].func = mob_defender ;	/* Mars */
    mob_index[real_mobile(14038)].func = mob_defender ;/* Jupiter */
    mob_index[real_mobile(14033)].func = mob_defender ;	/* Mercury */

	/* Daerim Temple */
    // mob_index[real_mobile(18000)].func = son_boonsin ;	/* 분신 */
    // mob_index[real_mobile(18001)].func = sonogong ;	/* Son ogong */
    // mob_index[real_mobile(18017)].func = snake;	/* King snake */

    /* 조로 존 */
    mob_index[real_mobile(18010)].func = zorro;	    /* Zorro */
	
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
    int chamber(struct char_data *ch, int cmd, char *arg); /* 재활용 기계 */

    obj_index[real_object(3099)].func = board;
    obj_index[real_object(3098)].func = mbox;
    obj_index[real_object(1311)].func = totem;
    obj_index[real_object(2157)].func = magicseed;
    obj_index[real_object(12100)].func = chamber;
}

/* assign special procedures to rooms */
void assign_rooms(void)
{
    int dump(struct char_data *ch, int cmd, char *arg);
    int pet_shops(struct char_data *ch, int cmd, char *arg);
    int pet_shops2(struct char_data *ch, int cmd, char *arg);
    int hospital(struct char_data *ch, int cmd, char *arg);
    int metahospital(struct char_data *ch, int cmd, char *arg);
    int metahospital2(struct char_data *ch, int cmd, char *arg);
    int safe_house(struct char_data *ch, int cmd, char *arg);
    int level_gate(struct char_data *ch, int cmd, char *arg);
    int bank(struct char_data *ch, int cmd, char *arg);
    int portal(struct char_data *ch, int cmd, char *arg);
    int neverland(struct char_data *ch, int cmd, char *arg);
    int electric_shock(struct char_data *ch, int cmd, char *arg);
    int dark_sand(struct char_data *ch, int cmd, char *arg);
    int control_center(struct char_data *ch, int cmd, char *arg);
    int terminator_shops(struct char_data *ch, int cmd, char *arg);
	int check_APT_room(struct char_data *ch, int cmd, char *arg);
    int apart_manage_room(struct char_data *ch, int cmd, char *arg);
    int apart_buy_room(struct char_data *ch, int cmd, char *arg);
    int apart_room(struct char_data *ch, int cmd, char *arg) ;
	int gomulsang(struct char_data *ch, int cmd, char *arg) ;
	int slotmachine(struct char_data *ch, int cmd, char *arg);
    int remortal_house(struct char_data *ch, int cmd, char *arg);
    int baseball(struct char_data *ch, int cmd, char *arg);

    world[real_room(3030)].funct = dump;
    world[real_room(3002)].funct = bank;
#ifndef DEATHFIGHT
    /* in death fight, there is no level gate */
    world[real_room(1453)].funct = level_gate;
    world[real_room(2535)].funct = level_gate;
    world[real_room(3500)].funct = level_gate;
    world[real_room(5200)].funct = level_gate;
    world[real_room(6001)].funct = level_gate;
    world[real_room(9400)].funct = level_gate;
    world[real_room(5027)].funct = level_gate;	/* rome */

    world[real_room(18061)].funct = level_gate;	/* just spell block */
#endif 

    world[real_room(3031)].funct = pet_shops;
    world[real_room(3076)].funct = pet_shops2;
    world[real_room(3060)].funct = hospital;
    world[real_room(3065)].funct = metahospital;
    world[real_room(13798)].funct = metahospital2;
    world[real_room(13799)].funct = remortal_house;

#ifndef DEATHFIGHT
    world[real_room(   1)].funct = safe_house;
    world[real_room(3001)].funct = safe_house;	/* now dressing room */

    world[real_room(3072)].funct = safe_house;	/* new midgaard */
    /*world[real_room(3078)].funct = safe_house;	A Rounge Room */
/* cyb  world[real_room(3008)].funct = safe_house; reception */
    world[real_room(6999)].funct = safe_house;
#endif 

    world[real_room(3079)].funct = slotmachine;
    world[real_room(3080)].funct = baseball;
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

    world[real_room(2710)].funct = dark_sand;
    world[real_room(2711)].funct = dark_sand;
    world[real_room(2712)].funct = dark_sand;
    world[real_room(2713)].funct = dark_sand;
    world[real_room(2714)].funct = dark_sand;
    world[real_room(2715)].funct = dark_sand;
    world[real_room(2716)].funct = dark_sand;
    world[real_room(2720)].funct = dark_sand;
    world[real_room(2721)].funct = dark_sand;
    world[real_room(2722)].funct = dark_sand;
    world[real_room(2723)].funct = dark_sand;
    world[real_room(2724)].funct = dark_sand;
#ifndef DEATHFIGHT
	/* mud control center */
    world[real_room(3095)].funct = control_center;
#endif 

	/* terminator shop in market */
	world[real_room(12118)].funct = terminator_shops ;

#ifndef DEATHFIGHT
	world[real_room(3075)].funct = gomulsang ;
#endif 

	/* apartment check room */
	world[real_room(8001)].funct = check_APT_room ;
    world[real_room(8002)].funct = apart_buy_room ;
	world[real_room(8003)].funct = apart_manage_room ;
    

	world[real_room(8010)].funct = apart_room ;
	world[real_room(8011)].funct = apart_room ;
	world[real_room(8012)].funct = apart_room ;
	world[real_room(8013)].funct = apart_room ;
	world[real_room(8014)].funct = apart_room ;
	world[real_room(8015)].funct = apart_room ;
	world[real_room(8016)].funct = apart_room ;
	world[real_room(8017)].funct = apart_room ;
	world[real_room(8018)].funct = apart_room ;
	world[real_room(8019)].funct = apart_room ;
	world[real_room(8020)].funct = apart_room ;
}
