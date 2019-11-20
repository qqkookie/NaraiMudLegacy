/************************************************************************
 * File : etc.h : Not so gerneral definition.				*
 * Definitions for specific mob / object/ room in some zone.		*
 ************************************************************************/

#define MID_BASE		3000

#define ROOM_VOID		2900
#define ROOM_WIZ_LOUNGE		2902	

// safe house
#define MID_PORTAL		(MID_BASE+1)
#define ROOM_LIMBO		2901
#define ROOM_RECEPTION		(MID_BASE+8)
#define ROOM_BOARD		(MID_BASE+5)

#define ROOM_QUEST		(MID_BASE+7)
#define ROOM_MID_BANK		(MID_BASE+13)
#define ROOM_METAPHYSICIAN	(MID_BASE+15)
#define ROOM_REMORTAL		(MID_BASE+16)
#define ROOM_HOSPITAL		(MID_BASE+17)

#define ROOM_MID_DUMP		(MID_BASE+22)
#define	ROOM_MID_PETSHOP	(MID_BASE+23)

#define ROOM_MID_BOMBARD	(MID_BASE+46)
#define ROOM_MID_STORAGE	(MID_BASE+47)

// taxi stop
#define TAXI_START_MKTSQ	(MID_BASE+60)	// Market Squre
#define TAXI_START_TP		3342	// Turning point

#define TAXI_STOP_KAIST		17411	// NOTE: KAIST zone renumber 31001->17411
#define TAXI_STOP_PROCESS	18100	// 2000

#define NEVERLAND_CLOUD		11277	    // 섬이 보이는 구름 위
#define NEVERLAND_SANDBEACH	11280	    // 모래밭

#define WASTELAND_LABORATORY	12258	    // Wasteland, Large laboratory

// RC electric shock
#define ROBOCITY_BASE		17110

// level gate
#define LEVGATE_CLUBSAFARI	3211	    // south
#define LEVGATE_ARENA		12110	    // south
#define LEVGATE_TB8TH		14735	    // south, TB 8th floor, chemist
#define LEVGATE_EASTCASTLE	3344	    // west, East Castle 
#define LEVGATE_THALOS		12810	    // west, City of Thalos
#define LEVGATE_SHIRE		12810	    // south, Shire
#define LEVGATE_MAGETOWER	(MID_BASE+74)	    // south, Mage Tower

/* ---------------    GUILD Rooms    ---------------- */
/*	From guild_list.h		*/
#define GUILD_BASE		3250

#define GUILD_POLICE_GATE	(GUILD_BASE+12)	// 3095
#define GUILD_POLICE_PRACTICE 	(GUILD_BASE+13)	// 3096
#define GUILD_POLICE_LOCKER	(GUILD_BASE+15)	// 3099

/* NOTE: moved from JALE_ROOM definition in guild_skills.c */
#define ROOM_POLICE_JAIL    		(GUILD_BASE+18)	// 1800

#define GUILD_OUTLAW_GATE	(GUILD_BASE+22)	// 3221
#define GUILD_OUTLAW_PRACTICE	(GUILD_BASE+23)	// 3222
#define GUILD_OUTLAW_LOCKER	(GUILD_BASE+24)	// 3224

#define GUILD_ASSASSIN_GATE	(GUILD_BASE+32)	// 9650
#define GUILD_ASSASSIN_PRACTICE (GUILD_BASE+33)	// 9651
#define GUILD_ASSASSIN_LOCKER	(GUILD_BASE+36)	// 9652

#define ROOM_MID_LOCKER		3295	// 3000

#define ROOM_JAIL		3299

// ----------------------------------------------------

#define OBJ_BOARD		(MID_BASE+5)	// bulletin board
#define OBJ_MAILBOX		(MID_BASE+6)	// mail box 
#define OBJ_SLOTMACHINE		(MID_BASE+7)
#define OBJ_STRINGMACHINE	(MID_BASE+8)
#define OBJ_REPAIRMACHINE

#define OBJ_UNKNOWNSEED		12261
#define OBJ_TOTEM		12262
#define OBJ_ANTI_DEATHCURE	(MID_BASE+31)

#define POTION_SOLAR		3211

// TODO: change to 2D8 dagger
#define WEAPON_BASIC_THIEF	11511	/* 2D4 pierce short sword (dagger) */
#define WEAPON_BASIC_WARRIOR	(MID_BASE+21)	/* 1D8 slash long sword */

#define OBJ_BIRD_FEATHER	11261
#define OBJ_PEGASUS_WING	15613

#define OBJ_LSS_BELT		16118

#define OBJ_SWORD_BOMBARD	(MID_BASE+72)
#define OBJ_BOMBARD_MINOR	(MID_BASE+73)
#define OBJ_SWORD_PARTICLE	(MID_BASE+74)

// ----------------------------------------------------

#define MOB_LIMBO_PUFF		2901
#define MOB_MID_HELPER		(MID_BASE+3)	    // Perhaps

#define MOB_MID_MAYOR	    	3553
#define MOB_MID_GUILDMASTER	(MID_BASE+14)	    // Practice room
#define MOB_MID_MUSASHI		(MID_BASE+32)
#define MOB_MID_SUPERMUSASHI	(MID_BASE+99)
#define MOB_MID_MOM		(MID_BASE+33)
#define MOB_MID_DEATHCURE    	(MID_BASE+34)
#define MOB_MID_SUPERDC		(MID_BASE+35)
#define MOB_MID_BOMBARD		(MID_BASE+40)

#define MOB_MID_MASS		(MID_BASE+41)	    // Hit 7 series (Massacre, ... , Miss)

#define MOB_MID_JANITOR		(MID_BASE+55)
#define MOB_MID_FIDO1		(MID_BASE+56)
#define MOB_MID_FIDO2		(MID_BASE+57)

#define MOB_MID_SINGER		3212	    // Bob Dylan
#define MOB_MID_ARCHMAGE	3223

/*
#define KITTEN		3090
#define PUPPY		3091
#define BEAGLE		3092
#define ROTTWEILER	3093
#define WOLF		3094
#define JUGGERNAUT	8006 

if (mob_nr == KITTEN || mob_nr == PUPPY
    || mob_nr == BEAGLE || mob_nr == ROTTWEILER
    || mob_nr == WOLF || mob_nr == JUGGERNAUT)
*/

#define MOB_MID_KITTEN		(MID_BASE+90)
#define MOB_MID_JUGGERNAUT	(MID_BASE+95)

#define IS_PETSHOP_MOB(nr) ((nr) >= MOB_MID_KITTEN && (nr) <= MOB_MID_JUGGERNAUT)

// spell blockers
#define MOB_GOBIND		14260	    // Mokcha gobind
#define MOB_HWATA_GUARD		16346	    // hwata secret guard

// helper
#define MOB_DK_BARBOR		16738	    // DK Barbor
#define MOB_DK_YURIA		16772	    // DK Yuria

// misc.
#define MOB_RC_GREAT_MAZINGA	17292	    // Great Mazinga
#define MOB_LARGE_HOUND		12223	    // large hound of hades

// Super DC summon mobs
#define MOB_SDC_HELPER1		12717	    // 2nd Matron Mother
#define MOB_SDC_HELPER2		12410	    // dracolich lich
#define MOB_SDC_HELPER3		12414	    // giant purple worm
#define MOB_SDC_HELPER4		12209	    // Medusa gorgon
#define MOB_SDC_HELPER5		12218	    // Hecate in Greek
#define MOB_SDC_HELPER6		17217	    // Semi robot in RC
#define MOB_SDC_HELPER7		16424	    // Wee Head Soldier

// snake
#define MOB_MORIA_SNAKE1	14310	    // dreadful snake
#define MOB_MORIA_SNAKE2	14311	    // small green snake
#define MOB_MORIA_SNAKE3	14363	    // brown snake
#define MOB_MORIA_SNAKE4	14412	    // large green snake
#define MOB_DESERT_WORM		12414	    // iant purple worm
#define MOB_SNAKE_ERROR		1660	    // Missing snake 1660?	

#define MOB_SEWER_SNAKE		11306	    // slithering snake
#define MOB_FOREST_SPIDER1	15423	    // poisonous spider
#define MOB_FOREST_SPIDER2	15424	    // Queen spider

/* -------------    for 대림사    ------------------------- */
#define DRS_BASE			16100

#define DRS_DARK_ROOM		(DRS_BASE+34)
#define DRS_TOWER		(DRS_BASE+61)

#define DRS_SON_OGONG		(DRS_BASE+1)
#define DRS_SON_OGONG_CLONE	(DRS_BASE+41)
#define DRS_FOURTH_JANGRO	(DRS_BASE+32)

#define DRS_PEN			(DRS_BASE+23)
#define DRS_PAPER		(DRS_BASE+24)
#define DRS_SCROLL		(DRS_BASE+32)
#define DRS_SAINT_WATER		(DRS_BASE+34)

#define DRS_GOLDEN_RIM		(DRS_BASE+27)
#define DRS_SON_OGONG_BONG	(DRS_BASE+26)

/*
#define SON_OGONG_STEP(son_ogong->quest.solved)
#define FOURTH_JANGRO_STEP(fourth_jangro->quest.solved)
*/

/* --------------      Good-Bad Island	   ----------------- */

#define GBI_BASE			16200

#define GBI_SEA			(GBI_BASE+1)
#define GBI_SEASHORE		(GBI_BASE+2)
#define GBI_MIRROR_SAINT	(GBI_BASE+21)
#define GBI_MAGIC_BARRIER_OUT   (GBI_BASE+31)

#define GBI_KAALM		(GBI_BASE+1)
#define GBI_LANESSA		(GBI_BASE+3)
#define GBI_SAINT_MIRROR	(GBI_BASE+4)
#define GBI_CARPIE		(GBI_BASE+20)
#define GBI_GOLEM		(GBI_BASE+23)

#define GBI_MAGIC_PAPER1	(GBI_BASE+6)
#define GBI_MAGIC_PAPER2	(GBI_BASE+7)
#define GBI_MAGIC_PAPER		(GBI_BASE+8)

#define GBI_SEED_EVIL_POWER	(GBI_BASE+9) 
#define GBI_BOTTLE		(GBI_BASE+12)

/* --------------------------------------------------------- */

