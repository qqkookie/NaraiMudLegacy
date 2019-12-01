KIT MUD Classic CHANGE LOG
==================

## TODO

#define RETURN_TO_QUIT

move House room # 7700 => ???

argument with dash/underscore (president-robot)

Perhaps -> narai

Check 4th jangro give_gold

o Add typo or talk to imple command
o
정시 사용자 리스트 출력에서
list user Room title  깨지는 문제 해결
Process or any where?


================================================================
----------------------------------
 0 -1 3001
 E
 poster~
-지저분 하게 뜯긴 HITE 맥주 포스터.
-쭉쭉 빵빵 늘씬한 미녀가 앙증맞게 작은 비키니를 입고 섹시하게 웃고 있다.
+앞으로 여기에 붙어있는 포스터를 자주 눈여겨 보세요. 특별한 일이 있으면,
+여기에 적어 놓을 것이거든요. 현제 page1 까지 있습니다.
+  예: look page1
+
+      언제나 여러분과 함께   Cold
+~
+E
+page1~
+먹을 것중에 어떤 것은 회복하는 데 도움을 줍니다. 근데 그냥 눈으로 보거나
+identify 해도 그건 알 수가 없어요.
~
 S
 #3081
 The Quest Room~
    You are standing in the quest room. There is a small poster on the wall.
 ~
-30 2060 0
+30 2204 0
 D5
 You see the Midgaard.
 ~
----------------------------

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

revert some IS_NPC() to IS_MOB()

give_gold() : dont give gold to NPC

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

IMO + 3		-> LEV_GOD
IMO + 2		-> LEV_DEMI
IMO + 1		-> LEV_LESSER
level < IMO	-> IS_MORTAL()
level >= IMO	-> IS_IMMORTAL()

LEVEL_LIMIT	= IMO -1
LEVEL_SIZE

LEV_GOD, LEV_DEMI, LEV_LESSER, LEV_IMMO
LEVEL_MAX, LEVEL_LIMIT,

NEW: IS_GOD  level == LEV_GOD
     NOT_GOD  level < LEV_GOD
     IS_DIVINE level >= LEV_LESSER
     IS_WIZARD

+ CARRY_WIGHT BASE 500

+ CARRY_NUM BASE 5 -> 15

wizard violence control

act.info.c: file_id -> version_id

added feedback message to wizset char

struct title_type exp -> LONGLONG

gain_gold(), gain_exp(), gain_exp_regard_less () gain arg LONGLONG

=================================================================

2019. 10. 09. (수) 20:08:16 KST

zone renumbering restored to old (pre-1998) zone numbering

lib/mob.quest is revert to old mob.quest

lib/tinyworld.mob lib/tineyworld.obj renumbered

CHANGE lib/tinyword.obj:

Weapon magic (val[0]) restored to 1-9,1000
TRASH (obj type =13 ) restored

Chuck of coal now obj type OTHERS

MacDonald coke and cheeseburger qualtity to 36->33
price back to 50
type pen to type others

ticket name change

chest -> chest treasure box

daggers 21d{1~3}, 18d4


CHANGE lib/tinyword.mob:

DK Mob  revert to old (pre-1998) levels
May be need to upgrade to Lecacy 2003 levels

Need to clean thess:
#19000,1,2 => #19801, 2, 3
Wang Comicky

Stronger mushashi

Checked for change of tineyworld.mob tineyworld.obj, world/, zone/

call command => taxi kaist, taxi process

do_exits => no perterbation for divines

shop keeper junks obj type : item_trash and item_other


MooDang zone -> EVERYZONE flag 제거
#200100  -> NORELO flag 제거

퀘스트룸 플래그 2204 -> 2060


-----------------
rename/merge info files

lib/news + news_old => lib/NEWS
lib/wizards + credit => lib/CREDITS
lib/motd -> lib/MOTD

GREETING BANNER and MENU moved ifrom "gamedb.h" to "comm.c"


=================================================================

2019. 10. 11. (금) 20:40:26 KST

ZONE RENUMERING MOVE

MooDang 20000~20199 => 18000 ~18199

KAIST   31000~31199 => 19000 ~19199

Kingdom of Chok : 2800~2899 => 15800~15899

Removed all exits from LIMBO #27 Secret Room

REmoved zone :
	The jale (police jail) to Midgaard Village (#1800 =>#1852)

Quest Room (#3081 => #3061)

Remortal room (#3039 => #3062)

Meta Physician (#3065)

Hospital room (#3060 -> #3066)

NEW ZONE Midgaard Village

Locker ROOM (#3000 => #1841)

Police Jale #1800 => #1852

Police 3095-3099 => 1855-1859

Outlaw 3221~3226 => 1871~1876

Assasin 9050~ 9053 => 1880~1883

Utility 13300-13399
 #27 => #13387
Secret 7500~7599  => 13400-13499
EasyLand 12000~12099 => 13500-13599
Pride 7600~7699    => 13600-13699

Process 2000-2099
Process 2000~2099 => 13800~13899




