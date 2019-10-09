KIT MUD Classic CHANGE LOG
==================

## TODO

#define RETURN_TO_QUIT

move House room # 7700 => ???

argument with dash/underscore (president-robot)

Perhaps -> narai

Check 4th jangro give_gold

=====================================================================

CHANGE : 2019. 10. 03. (금) 

-Wall -Wextra -Werror  관련 에러 수정 
ex) FALLTHRU, 
    
#pragma GCC diagnostic ignored "-Wunused-parameter"

MUD user directory :  $mudhome/lib -> $mudhone/users 

chdir fail -> not error

Move include dir -> src/include

PID file nud-port#.pid + unlink

long users/plan  file관련 buffer overflow 수정

Deathcure 공갈 message  수정

무사시 crush armor 수정

머드 이름 KIT Classic MUD

--------------------------------------------------------

CHANGES : 2019. 10. 06. (일) 08:45:26 KST

lib/zone/zone_files:

Import Legacy version utils 

--- a/lib/zone/DragonTower.zon
+++ b/lib/zone/DragonTower.zon
@@ -1,5 +1,5 @@
 Dragon Tower~
-2300 30 1
+2290 30 1
 M 0 2200 5 2241
 M 0 2200 5 2239

Remove zone/EasyZone.zon entry

--- a/lib/zone/zone_files
+++ b/lib/zone/zone_files
@@ -39,7 +39,6 @@ zone/DeathKingdom.zon
 zone/Galaxy.zon
 zone/TheDeathStar.zon
 zone/DaeRimSa.zon
-zone/EasyZone.zon
 zone/GoodBadIsland.zon
 zone/KAIST.zon
 zone/MooDang.zon

Utils rename

dangun			-> chlist
purge			-> chdelete
what_item_in_zone	-> zoneitem
list			-> zonelist
test_skill		-> testskill

Added from legacy

chcheck			<- player_check.c
zoneprint		<- zone_print
mobbalance		<- mob_balance 
mobcreate		<- mob_create
mobedit			<- mob_edit
dodtashform		<- chstashform
undostashform		<- unchstashform

Removed dupname.c

Makefile dependancy update
 
Remove IMO=41 macro



Check for NPC && level > IMO +3 -> ISNPCLEV()

do_shout():
	Excude Archimage shouting from chat history

Added date/time in chat hitoryi display

Added CHATLOG

OBJ_FEATHER	    -> OBJ_BIRD_FEATHER
POLICE_JAIL_ROOM    -> ROOM_POLICE_JAIL

revert some IS_NPC to IS_MOB

give_gold() : dont give gold to NPC

added feedback message to wizset char

struct title_type exp -> LONGLONG

gain_gold(), gain_exp(), gain_exp_regard_less () gain arg LONGLONG

db.c: read_mobile()
    commnted out experimental mob balancing

CMD_CALL -> CMD_TAXI

taxi() code clean

Add MID_HELPER

Add REBOOT_TIME

Add REBOOT_WHEN

Add spell name like full-heal full_heal 

src/utils

chpass.c: Add password check menu 

chdelete: Add empty name delete

special.c init_quest() BUG FIX

------------------------------------------------------

Remove IMO

IMO +3		-> LEV_GOD
IMO +2		-> LEV_DEMI
level < IMO	-> IS_MORTAL()
level >= IMO	-> IS_IMMORTAL()
LEVEL_LIMIT	= IMO -1

LEV_GOD, LEV_DEMI, LEV_LESSER, LEV_IMMO
LEVEL_MAX, LEVEL_LIMIT, 

NEW: IS_GOD  level == LEV_GOD
     NOT_GOD  level < LEV_GOD
     IS_DIVINE level >= LEV_LESSER
     IS_

   consider NPC && IMMO

+ CARRY_WIGHT BASE 500

+ CARRY_NUM BASE 5 -> 15

wizard violence control

=================================================================

2019. 10. 09. (수) 20:08:16 KST
