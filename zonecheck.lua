#!/usr/bin/lua
-- usage: zonecheck [lib-directory]
-- To renum, set G_renum true and 'mkdir renum renum/zone renum/world' for output

require('KZIOLib')
require('KZoneCheck')

-----------------------------------------------------------

function main()
    -- check and renumber zone files

    G_renum = true

    Zone_renum_tab = Zone_renum_tab_master

    print('================================================================')

    G_zonepath = (#arg >= 1) and arg[1] or '.'

    Zone_index = {}

    Object_index = {}
    Mobile_index = {}

    Room_index = {}
    Exit_index = {}

    if G_renum and not pathexists('renum/zone/') then
        G_renum = false
    end

    load_zone_index()

    load_assets_index( Object_index, 'tinyworld.obj')

    load_assets_index( Mobile_index, 'tinyworld.mob')

    -- check_zone_list()

    for iz, zinfo in ipairs(Zone_index) do
        local wf = 'world/' .. zinfo[4]

        zone_number(iz)
        load_world(wf)
    end

    if G_renum then
        print("--------------- Renumbering Zone and world:")
        renum_index(Object_index, 'OBJ')
        renum_index(Mobile_index, 'MOB')
        renum_index(Room_index, 'ROOM')

        renum_special_cases()
    else
        print("--------------- Checking Zone and world:")
    end

    for iz, zinfo in ipairs_sorted(Zone_index, G_renum and renum_cmp or nil ) do
        local zf = 'zone/' .. zinfo[3]
        local wf = 'world/' .. zinfo[4]

        zone_number(iz)
        check_zone(zf)
        check_world(wf)

        local zbuf = string.format(' %3d %-30s %6d %4d (%4d %3d %6d -%6d )',
            iz, zone_name() , zone_base(), zone_top() - zone_base()+1,
            G_room_max - G_room_min+1, G_room_min - zone_base(), G_room_min, G_room_max)
        print(zbuf)
    end

    zone_number(-1)

    print("--------------- Checking Unused obj, mob, room:")
    check_unreferenced()

    print("--------------- Checking World room exit and key:")
    check_exits(false)

    print("---------------- Checking Shops:")
    check_shops('tinyworld.shp')

    print("---------------- Checking Quest mob level: ")
    check_questmob('mob.quest')

    if G_renum then
        print("---------------- Renumbering Objects and mobs: ")

        renum_assets = renum_mob
        renum_assets_index( Mobile_index, 'tinyworld.mob')

        renum_assets = renum_obj
        renum_assets_index( Object_index, 'tinyworld.obj')

        print("--------------- Renumbering world:")
        renum_world()

        renum_zone_list()
    end
    print('All Done.')

end


function renum_special_cases()

    MID = 3000
    ORPHAN = 11110       -- unreferenced/unused items

    SLUM = 3210         -- club safari + Archmage + Guild rooms
    RIVER= 3310         -- River of MID
    HOUSE = 3810        -- RB building +Utility + House

    GRAVE = 11210       -- Graveyard/Mels dog house/Neverland
    TUNNEL = 11710      -- The tunnel
    GKP =  11910        -- Grand Knight Paladin (Redferne Residence)
    WASTE = 12200       -- Waste land
    DESERT = 12410      -- Great eastern desert
    SHIRE = 15100       -- shire
    HDK = 15410         -- haondor dark
    DWARVEN = 14610

    DROW = 12710
    ARACH = 15500
    LAND = 15200
    DRAGON = 14510
    AERIE = 12510
    MUNCIE = 11810
    EASY = 18210        -- Easyland
    SOUTH = 3510        -- Sothern MID

    TOJAIL = SLUM+89      -- Realm of the Dead (to 3299)

    special_obj =
    {
        { 1301, ORPHAN+1 },      -- UNREF: OBJ	#1301	immo wooden Kal
        { 1304, ORPHAN+2 },     -- immo mail UNREF: OBJ	#1304	immo vulcan mail
        { 1002, ORPHAN+3 },     -- champ belt UNREF: OBJ	#1002	champion belt
        { 1300, ORPHAN+4 },     -- immo UNREF: OBJ	#1300	immo gloves magic
        
        { 7213, ORPHAN+11},    -- large helmet  UNREF: OBJ	#7213	helmet large
        { 7214, ORPHAN+12 },    -- chaos helmet  UNREF: OBJ	#7214	helmet chaos
        
        { 1312, ORPHAN+13 },      --- claw of faust  UNREF: OBJ	#1312	claws faust
        { 1321, ORPHAN+14 },     -- UNREF: OBJ	#1321	blade chaos
        
        { 5050, ORPHAN+15 },    -- cloak blue -- -- UNREF: OBJ	#5050	cloak blue
        { 5051, ORPHAN+16 },    -- rifle assualt  -- UNREF: OBJ	#5051	rifle assualt
        
        { 1310, ORPHAN+21 },     -- nothing UNREF: OBJ	#1310	nothing        
        { 1314, ORPHAN+22 },     -- white dove -  UNREF: OBJ	#1314	white dove      
        { 3300, ORPHAN+23 },    -- a metal key~     UNREF: OBJ	#3300	key metal
        { 3301, ORPHAN+24 },    --  a brass key~    UNREF: OBJ	#3301	key brass
        

        -- silver epee (The tunnel)/halbard (shire) {to tunnel (7400)} TODO: CHECK
        { 1305, TUNNEL+1 },      -- silver epee TO The tunnel

        { 1309, DESERT+41 },    -- ancient plate (great eastern deset)
 

        { 1318, DROW+21 },       -- banner bloody Drow City(5100)
        { 1319, DESERT+42 },       -- green cresent banner Great easten desert

-- Grave yard has many unref items

--[[
        #3400-3430

UNREF: OBJ	#3400	candlestick
UNREF: OBJ	#3401	oops
UNREF: OBJ	#3402	tickler
UNREF: OBJ	#3403	plasma webbing
UNREF: OBJ	#3410	rod key
UNREF: OBJ	#3411	large black dirty key
UNREF: OBJ	#3412	head jubal mummy
UNREF: OBJ	#3413	black marble ring marblering
UNREF: OBJ	#3414	thin rapier
UNREF: OBJ	#3415	sarcophagus coffin
UNREF: OBJ	#3416	iron heavy key
UNREF: OBJ	#3417	flask wineflask
UNREF: OBJ	#3418	coins
UNREF: OBJ	#3419	key
UNREF: OBJ	#3420	rat'hcor rathcor
UNREF: OBJ	#3421	rusty sword curved scimitar
UNREF: OBJ	#3422	piece slime
UNREF: OBJ	#3423	smock
UNREF: OBJ	#3424	scroll tightly rolled
UNREF: OBJ	#3425	auburn orb
UNREF: OBJ	#3426	bar gold
UNREF: OBJ	#3427	cape dark black
]]
    --[[
        { 3400, ORPHAN+31 },  -- candlestick
        { 3401, ORPHAN+32 }, 	-- oops
        { 3402, ORPHAN+33 },  -- tickler
        { 3403, ORPHAN+34 }, 	-- 	plasma webbing
    ]]

        { 3121, GRAVE+41 },     -- key rusty : from Southern MID to Grave
        -- { 6110, GRAVE?? }    -- Strenth potion(yellow)  to Grave yard
        { 6110, GRAVE+42 },     --> strength potion (yellow) ONLY GRAVE
        { 1303, GRAVE+43 },     -- seraphim protector to grave yard
        { 1331, GRAVE+44 },     -- white cape to grave yard
        { 6111, GRAVE+45 },     -- large round shield

        { 2700, GRAVE+51 },     -- #2700 깃털
        { 1325, GRAVE+52 },     -- Mauve stone to Grave (1300)
        

        { 1327, HOUSE+1 },      -- magnolias garland~ (House)

        { 1335, ARACH+11 },     -- speckled potion to arachnos(9000)
        { 1462, LAND+31 },     -- red potion to the land (1900) ** check over**

        { 1306, HDK+21 },       -- large scythe (handor Dark)
        { 1316, HDK+22 },       -- armored plate evil roy (6137) haodor dark
        { 1620, HDK+23 },       -- colt 45
        { 1621, HDK+24 },       -- bullet
        { 5999, HDK+25 },       -- black leather boost, Lumber jack boot

        { 6910, HDK+31 },       -- a black dragon scale shirt
        { 6911, HDK+32 },       -- a black dragon scale skirt
        { 6912, HDK+33 },       -- cloak black hooded
        { 6913, HDK+34 },       -- a broad silver belt
        { 6914, HDK+35 },       -- a long, slender sword

        { 7215, HDK+36 },       -- light libre HAODOR DARK ONLY

        { 1311, WASTE+62 },     -- totem (waste land)
        { 2157, WASTE+61 },     -- unknown seed
        { 2400, WASTE+63 },     -- wooden kal

        --[[
        -- 2200 = Dragon tower has lots of unref objs
        -- 2220 (diana bow ) ~ 2247 (jug wine) : UNREF: OBJ	#2220	bow diana
        -- folowings are given to Deathcure
        -- { 2222, MID },      -- diana legging (used by DC)
        -- { 2223, MID },      -- fur sash (used by DC)
        -- { 2224, MID },      -- fur hat (used by DC)
        -- { 2244, MID },      -- brass ring (used by DC)

        UNREF: OBJ	#2220	bow diana
        UNREF: OBJ	#2221	arrow
        UNREF: OBJ	#2226	loin cloth
        UNREF: OBJ	#2227	horn
        UNREF: OBJ	#2228	spear
        UNREF: OBJ	#2230	staff echo
        UNREF: OBJ	#2231	berries
        UNREF: OBJ	#2232	a nut
        UNREF: OBJ	#2233	jug wine
        UNREF: OBJ	#2240	sling
        UNREF: OBJ	#2241	rock
        UNREF: OBJ	#2242	cape green
        UNREF: OBJ	#2243	necklace emerald
        UNREF: OBJ	#2245	broadsword black
        UNREF: OBJ	#2246	crooked dagger
        UNREF: OBJ	#2247	bonnet
        ]]

        -- exceed DT range, Move to 2258, 2259 position
        { 2298, DRAGON+58 },     -- DT, a dragon winged cloak~
        { 2299, DRAGON+59 },     -- DT, key gold tarnished~


        { 6999, TOJAIL },          -- Jail

        -- global potions
        -- { 6109, HDK+?? }     -- blue potion (?? ) remain in hoandor dark
        { 6115, MUNCIE+31 },     -- haste/ powerful potion to Muncie (2300)
        -- { 6116, SLUM+??}     -- improved state potion to Library
        { 6118, AERIE+31 },     -- wand of vitalize to Aerie

        { 6117, GKP+21 },    -- mana boost staff to Paladin
        { 7090, GKP+22 },    -- waybread  to residence ONLY
        { 7190, TUNNEL+2 },  -- glinting ring of silver: (The tunnel Red dragon) or Paladin
        { 7191, SHIRE+21 },  -- Goldring : Shire(1700) or Paladin
        { 7192, TUNNEL+3 },  -- magic boot (the Tunnel Jones ONLY)
        { 7207, GKP+26 },    -- muddy gauntlets to Paladin ONLY
        { 7212, GKP+27 },    -- Bastard sword, TO Paladin


        -- GK Paladin ONLY
        { 7216, GKP+31 },    -- two-handed sword
        { 7217, GKP+32 },    -- breast plate
        { 7218, GKP+33 },    -- leg plate
        { 7219, GKP+34 },    -- pair of gloves
        { 7220, GKP+35 },    -- silvery helmet
        { 7221, GKP+36 },    -- shield of rose
        { 7222, GKP+37 },    -- a pair of heavily plated boot
        { 7223, GKP+38 },    -- some arm plates
        { 7224, GKP+39 },    -- a sparkling ring of innocence

        { 7903, DESERT +61 },    -- 'new key' to desert(5000) brass dragon

        ----------------------------------------------

        { 1461, SLUM+1 },      -- solar flair
        { 1455, SLUM+2 },      -- boddari
        { 1460, SLUM+3 },      -- nectar glass
        { 1454, SLUM+4 },      -- iron-banded scroll
        { 1462, SLUM+5 },      -- red portion

        { 1450, SLUM+6 },      -- bras knuckle
        { 1451, SLUM+7},      -- dockers levis
        { 1452, SLUM+8 },      -- shirts chambreay
        { 1453, SLUM+9 },      -- reebok pumps

        { 3090, SLUM+11 },      -- archmage robe
        { 3091, SLUM+12 },      -- magi staff
        { 6116, SLUM+13 },      -- imp haste powerful potion

        { 1871, SLUM+61 },      -- 88 light
        { 1872, SLUM+62 },      -- sky light
        { 1873, SLUM+63 },      -- drug

        ----------------------------------------------

        { 1000, MID+2 },     -- sundew fountain
        { 1001, MID+3 },     -- altar

        { 3099, MID+5 },     --  board
        { 3098, MID+6 },     --  mail box
        { 7999, MID+7 },     -- slot machine
        { 8000, MID+8 },     -- string machine

        { 3052, MID+11 },     --  scroll recall
        { 3050, MID+12 },     --  scroll id
        { 3137, MID+13 },     --  rechrge scroll
        { 3051, MID+14 },     --  potion see inv
        { 3138, MID+15 },     --  opaque potion
        { 3053, MID+16 },     --  inv wand
        { 3054, MID+17 },     --  staff sleep UNREF: OBJ	#3054	staff

        { 3022, MID+21 },     --  sword long
        { 3021, MID+22 },     --  small sword
        { 3020, MID+33 },     --  dagger
        { 3023, MID+24 },     --  club wooden
        { 3024, MID+25 },     --  war hammer

        { 3007, MID+27 },     --  coke
        { 3006, MID+28 },     --  cheese

        { 1317, MID+31 },     --  anti-dc
        { 1308, MID+32 },     --  vault key
        { 3133, MID+33 },     --  City Key  (from Southern MID)

        { 3061, MID+35 },     --  canoe
        { 3060, MID+36 },     --  raft
        { 1313, MID+37 },     --  map
        { 1322, MID+38 },     --  align

        -- Reservered for extention: 40-49

        -- TICKET:  7991 7992, 7993 7994
        -- QUEST weapon armor 7997 7998
        { 7997, MID+51 },     --  quest weapon
        { 7998, MID+52 },     --  quest armor
        { 7994, MID+54 },     --  ticket req
        { 7991, MID+55 },     --  ticket ac
        { 7993, MID+56 },     --  ticket dr
        { 7992, MID+57 },     --  ticket hr

        { 3032, MID+62 },     --  bag
        { 3033, MID+63 },     --  box
        { 3031, MID+64 },     --  lantern
        { 3030, MID+65 },     --  torch
        { 3035, MID+66 },     --  paper
        { 3034, MID+67 },     --  pen

        { 3026, MID+71 },     --  swod saurai
        { 3027, MID+72 },     --  sword bombard
        -- { XXX, MID+73 },      -- sowrd bombard minor.
        { 3025, MID+74 },     --  sword particle

        { 1605, MID+76 },     -- brass shield
        { 1320, MID+77 },     -- sunshade
        { 1330, MID+78 },     -- baseball cap
        { 1307, MID+79 },     --  carpenter hammer
        
        { 1326, MID+81 },     --  pendant
        { 1302, MID+82 },     --  bagrd boot
        { 1622, MID+83 },     --  shotgun
        { 1623, MID+84 },     --  shell ammo
        { 1315, MID+85 },     --  broom

        ---------------------------------------------

        { 3000, MID+111 },     --  beer barrel
        { 3001, MID+112 },     --  beer bottle
        { 3002, MID+113 },     --  ale bottle
        { 3003, MID+114 },     --  firebreather
        { 3004, MID+115 },     --  local bott
        { 3005, MID+116 },     --  fancy bottle
        { 3008, MID+117 },     --  condition

        { 3010, MID+118 },     --  bread
        { 3011, MID+119 },     --  danis

        { 7923, MID+131 },     --  bronze knee
        { 7922, MID+132 },     --  leather knee

        { 3040, MID+135 },     --  breast plate (18K)
        { 3041, MID+136 },     --  chain mail shirt (2.5k)
        { 3044, MID+137 },     --  jacket studd (0.5K)
        { 3042, MID+138 },     --  wooden shield
        { 3045, MID+139 },     --  scale mail UNREF: OBJ	#3045	jacket scale mail

        { 3075, MID+140 },     --  helmet bronze
        { 3046, MID+141 },     --  bronze breast
        { 3085, MID+142 },     --  sleeve bronze
        { 3070, MID+143 },     --  gauntlt bronze
        { 3080, MID+144 },     --  legging bronze

        { 3076, MID+145 },     --  cap leather
        { 3043, MID+146 },     --  jacket leather
        { 3086, MID+147 },     --  sleeve leather
        { 3071, MID+148 },     --  glove leather
        { 3081, MID+149 },     --  pant leather

        -- perhaps: 7924, 7925, 26, 27, 28,29
        { 7924, MID+151 },     -- perhaps
        { 7925, MID+152 },     -- perhaps
        { 7926, MID+153 },     -- perhaps
        { 7927, MID+154 },     -- perhaps
        { 7928, MID+155 },     -- perhaps
        { 7929, MID+156 },     -- perhaps

--[[

UNREF: OBJ	#1002	champion belt
UNREF: OBJ	#1300	immo gloves magic
UNREF: OBJ	#1301	immo wooden Kal
UNREF: OBJ	#1304	immo vulcan mail
UNREF: OBJ	#1310	nothing
UNREF: OBJ	#1312	claws faust
UNREF: OBJ	#1314	white dove
UNREF: OBJ	#1321	blade chaos
UNREF: OBJ	#1553	kirpan dagger
UNREF: OBJ	#1871	ciga cigarette tabacco 88
UNREF: OBJ	#1872	ciga cigarrette sky
UNREF: OBJ	#1873	drug marijuana
UNREF: OBJ	#2115	amulet
UNREF: OBJ	#2116	shield pallas
UNREF: OBJ	#2141	oboe golden
------------------ DT items
UNREF: OBJ	#2208	robe black
UNREF: OBJ	#2211	axe heavy
UNREF: OBJ	#2220	bow diana
UNREF: OBJ	#2221	arrow
UNREF: OBJ	#2226	loin cloth
UNREF: OBJ	#2227	horn
UNREF: OBJ	#2228	spear
UNREF: OBJ	#2230	staff echo
UNREF: OBJ	#2231	berries
UNREF: OBJ	#2232	a nut
UNREF: OBJ	#2233	jug wine
UNREF: OBJ	#2240	sling
UNREF: OBJ	#2241	rock
UNREF: OBJ	#2242	cape green
UNREF: OBJ	#2243	necklace emerald
UNREF: OBJ	#2245	broadsword black
UNREF: OBJ	#2246	crooked dagger
UNREF: OBJ	#2247	bonnet
---------------- DT END
UNREF: OBJ	#2317	flail
UNREF: OBJ	#2921	broach ivory
UNREF: OBJ	#2922	manual arts healing book
UNREF: OBJ	#2924	symbol holy golden
UNREF: OBJ	#2931	horn life
UNREF: OBJ	#2933	stone blood red
UNREF: OBJ	#2981	seste huge spiked brass knuckles
UNREF: OBJ	#2987	club ogre war
UNREF: OBJ	#2988	cloak bear skin bearskin
UNREF: OBJ	#2989	ear ring earring feathered
UNREF: OBJ	#3027	sword bombard
UNREF: OBJ	#3033	box
UNREF: OBJ	#3045	jacket scale mail
UNREF: OBJ	#3054	staff
UNREF: OBJ	#3300	key metal
UNREF: OBJ	#3301	key brass
------------ GRAVE items
UNREF: OBJ	#3400	candlestick
UNREF: OBJ	#3401	oops
UNREF: OBJ	#3402	tickler
UNREF: OBJ	#3403	plasma webbing
UNREF: OBJ	#3410	rod key
UNREF: OBJ	#3411	large black dirty key
UNREF: OBJ	#3412	head jubal mummy
UNREF: OBJ	#3413	black marble ring marblering
UNREF: OBJ	#3414	thin rapier
UNREF: OBJ	#3415	sarcophagus coffin
UNREF: OBJ	#3416	iron heavy key
UNREF: OBJ	#3417	flask wineflask
UNREF: OBJ	#3418	coins
UNREF: OBJ	#3419	key
UNREF: OBJ	#3420	rat'hcor rathcor
UNREF: OBJ	#3421	rusty sword curved scimitar
UNREF: OBJ	#3422	piece slime
UNREF: OBJ	#3423	smock
UNREF: OBJ	#3424	scroll tightly rolled
UNREF: OBJ	#3425	auburn orb
UNREF: OBJ	#3426	bar gold
UNREF: OBJ	#3427	cape dark black
-------------- Grave END
UNREF: OBJ	#5050	cloak blue
UNREF: OBJ	#5051	rifle assualt
UNREF: OBJ	#5209	shield black dark
UNREF: OBJ	#6101	sign
UNREF: OBJ	#6515	magic boots
UNREF: OBJ	#6551	trapdoor
UNREF: OBJ	#7213	helmet large
UNREF: OBJ	#7214	helmet chaos
UNREF: OBJ	#7909	treasure coins
------------- special function
UNREF: OBJ	#7991	ticket ac
UNREF: OBJ	#7992	ticket hr
UNREF: OBJ	#7993	ticket dr
UNREF: OBJ	#7994	ticket request
UNREF: OBJ	#7997	quest weapon
UNREF: OBJ	#7998	quest armor
UNREF: OBJ	#9814	power capsule engine
UNREF: OBJ	#11126	여의봉 bong
UNREF: OBJ	#11127	금테 golden rim
UNREF: OBJ	#11132	scroll teleport
UNREF: OBJ	#11134	saint water bottle 성수병
------ Process catsle
UNREF: OBJ	#13801	bullets ammo
UNREF: OBJ	#13802	sword damaging
UNREF: OBJ	#13803	mail good really
UNREF: OBJ	#13804	ring cap
UNREF: OBJ	#13805	knee band
UNREF: OBJ	#13806	gloves
UNREF: OBJ	#13807	bracelet
UNREF: OBJ	#13808	dagger norent
UNREF: OBJ	#13809	staff zizon mage
UNREF: OBJ	#13810	shield
UNREF: OBJ	#13811	cloak
UNREF: OBJ	#13812	boots speed
UNREF: OBJ	#13813	pendant light
UNREF: OBJ	#13814	light
UNREF: OBJ	#13815	legging
UNREF: OBJ	#13816	belt
UNREF: OBJ	#13817	plate
UNREF: OBJ	#13818	cap
------- END process
UNREF: OBJ	#15036	zizon dagger
UNREF: OBJ	#15806	yubi's BIG knee
UNREF: OBJ	#23308	magic paper 부적
UNREF: OBJ	#23309	seed evil power 악마 씨앗

]]

    }

    special_mob =
    {
        -- #2                   -- UNREF: MOB	#2	zombie

        { 1300, GRAVE+1 },     -- alley cat
        { 1303, GRAVE+2 },     -- ant
        { 1305, GRAVE+5 },     -- dragon white
        { 1307, GRAVE+7 },     -- peter pan
        { 2700, GRAVE+11 },    -- bird
        { 2701, GRAVE+12 },    -- 고운 새

        { 3149, HDK+9 },       -- ranger (haondor dark)
        { 1465, HDK+21 },      -- roy slade
        { 6007, HDK+22 },      -- Jim Lumberjack (haondor dark)
        { 6910, HDK+23 },      -- isha elf : (haondor dark)

        { 1454, SLUM+1 },       -- too too
        { 1455, SLUM+2 },       -- bob dylan
        { 1450, SLUM+4 },       -- bouncer

        { 3070, SLUM+11 },   -- crank lib
        { 3071, SLUM+12 },   -- furniture
        { 3072, SLUM+13 },   -- grand mage

        -- { 3004, SLUM+21 },   -- armorer

        { 1470, HOUSE+1 },     -- labbie
        { 1400, HOUSE+3 },     -- cinnamon
        { 1924, HOUSE+5 },     -- Charon: deathreaper

        { 6999, TOJAIL },          -- Jail
        -- { 13799, }

        { 7009, GKP+1 },      -- Grnd Knight Paladin to Redfrens Residance (7900)

        -- #4104   -- orc 4th in Moria 3-4 UNREF: MOB	#4104	orc
        -- #6003    UNREF: MOB	#6003	rabbit
        --  UNREF: MOB	#7008	homonculus homo
        --  UNREF: MOB	#7012	esther madam
        --  #7007     -- UNREF: MOB	#7007	wanderer

        { 7040, TUNNEL+1 },     -- red dragon
        { 7010, TUNNEL+2 },     -- chr-eff'n chr
        { 7041, TUNNEL+3 },     -- sea hag
        { 7042, TUNNEL+4 },     -- guardain naga
        { 7043, TUNNEL+5 },     -- scaled basilis
        { 7044, TUNNEL+6 },     -- lemure blob
        { 7045, TUNNEL+7 },     -- jones cruncher ettin
        { 7046, TUNNEL+8 },     -- herald mouse killer ettin

        { 6250, ORPHAN+1 },     -- com bug UNREF: MOB	#6250	bug
        { 8010, ORPHAN+2 },      -- cowboy UNREF: MOB	#8010	cowboy

        -- Unref mobs in Pride land (easyzone)
        { 13601, EASY+71 },     -- UNREF: MOB	#13601	wang
        { 13602, EASY+72 },     -- UNREF: MOB	#13602	cap
        { 13603, EASY+73 },     -- UNREF: MOB	#13603	Mac

------------------------------------------------------------------------------

        { 1000, MID+03 },    -- perhaps

        { 3000, MID+11 },   -- magic shop
        { 3003, MID+12 },   -- weapon shp
        { 3004, MID+13 },   -- armorer

        { 3020, MID+14 },   -- pr room master

        { 3002, MID+15 },   -- gen store
        { 3001, MID+16 },   -- baker
        { 3044, MID+17 },   -- Mac boy
        { 3006, MID+18 },   -- captain
        { 3007, MID+19 },   -- sailor

        { 3152, MID+21 },   -- bank examiner
        { 3153, MID+22 },   -- bank president
        { 1460, MID+23 },   -- grand ma
        { 8000, MID+24 },   -- bag lady
        { 3154, MID+25 },    -- undercover

        { 3046, MID+27 },   -- filthy
        { 3040, MID+28 },   -- bartender
        { 3042, MID+29 },   -- waiter Ron
        { 3024, MID+30 },   -- guild ent
        { 3026, MID+31 },   -- UNREF: MOB	#3026	Chester Wilson

        { 3133, MID+32 },   -- mushashi
        { 3145, MID+33 },   -- mom
        { 3125, MID+34 },   -- DC
        { 3135, MID+35 },   -- super DC

        { 3134, MID+40 },   -- bombard
        -- 3126 ~ 3132 = --  7 hit series
        { 3126, MID+41 },
        { 3127, MID+42 },
        { 3128, MID+43 },
        { 3129, MID+44 },
        { 3130, MID+45 },
        { 3131, MID+46 },
        { 3132, MID+47 },

        { 3060, MID+51 },   -- city guard 1
        { 3067, MID+52 },   -- cityguard 2
        { 3148, MID+53 },   -- unarmed city
        { 3150, MID+54 },   -- security guard

        { 3061, MID+55 },   -- janitor
        { 3062, MID+56 },   -- fido
        { 3066, MID+57 },   -- odif yltsaeb~
        { 3063, MID+58 },   -- mercenary
        { 3064, MID+59 },   -- drunk
        { 3065, MID+60 },   -- begger
        { 3147, MID+61 },   -- urchin

        { 3151, MID+62 },   -- bogan
        { 3005, MID+63 },   -- Doris smith
        { 3146, MID+64 },   -- suzy
        { 3045, MID+65 },   -- carpenter

        -- pet shop
        { 3090, MID+90 },   -- kitten
        { 3091, MID+91 },   -- puppy
        { 3092, MID+92 },   -- beagle~
        { 3093, MID+93 },   -- rottweiler
        { 3094, MID+94 },   -- wolf
        { 8006, MID+95 },   -- juggnut

        { 19999, MID+99 },  -- super mushashi  UNREF: MOB	#19999	super musashi

--[[
UNREF: MOB	#2	zombie
UNREF: MOB	#3026	Chester Wilson
UNREF: MOB	#4104	orc
UNREF: MOB	#6003	rabbit
UNREF: MOB	#6250	bug
UNREF: MOB	#7007	wanderer
UNREF: MOB	#7008	homonculus homo
UNREF: MOB	#7012	esther madam
UNREF: MOB	#8010	cowboy
UNREF: MOB	#9521	month sura
UNREF: MOB	#11141	mirror son ogong
-- Process
UNREF: MOB	#13807	2nd keeper
UNREF: MOB	#13808	3rd keeper
UNREF: MOB	#13809	4th keeper
UNREF: MOB	#13810	5th keeper
UNREF: MOB	#13811	6th keeper
UNREF: MOB	#15836	secret guard
-- MooDang
UNREF: MOB	#18020	guest
UNREF: MOB	#18021	servant
UNREF: MOB	#18022	cook
UNREF: MOB	#18023	jumo
UNREF: MOB	#18024	waiter
UNREF: MOB	#19999	super musashi
        ]]
    }

    special_room =
    {
        -- Club Safari
        { 1453, SLUM+1 },      -- enterance
        { 1451, SLUM+2 },      -- club
        { 1454, SLUM+3 },      --  bar
        { 1452, SLUM+4 },      -- small room
        { 1450, SLUM+5 },      -- office

        { 3090, SLUM+11 },     -- library/archmage
        { 3091, SLUM+12 },
        { 3092, SLUM+13 },
        { 3093, SLUM+14 },

        -- { 3020, SLUM+21 },     -- armory

        -- { 1852, SLUM+59 },     -- Police jail room

        { 1855, SLUM+52 },    -- police enter
        { 1856, SLUM+53 },    -- police pr
        { 1857, SLUM+54 },    -- police Lounge
        { 1859, SLUM+55 },    -- police locker(board)
        { 1858, SLUM+56 },    -- police shop
        { 1852, SLUM+58 },    -- police jail

        { 1871, SLUM+62 },    -- outlaw enter
        { 1872, SLUM+63 },    -- outlaw pr(board)
        { 1874, SLUM+64 },    -- outlaw Lounge (locker)
        { 1876, SLUM+65 },    -- outlaw board room
        { 1873, SLUM+66 },    -- outlaw boss
        { 1875, SLUM+68 },    -- outlaw exit

        { 1880, SLUM+72 },    -- assassin enter
        { 1881, SLUM+73 },    -- assassin pr
        { 1883, SLUM+74 },    -- assassin Lounge
        { 1884, SLUM+75 },    -- assassin dark room (board)
        { 1882, SLUM+76 },    -- assassin locker
        -- { 1885, GUILD+78 },    -- assassin new room

        { 1841, SLUM+85 },     -- locker room

        { 6999, TOJAIL },       -- Jail to Slum

        -- To Southern MID West park enterance
        { 1425, SOUTH+51 },       -- Tree
        { 1426, SOUTH+52 },       -- Nest

        -- Robert bell building
        { 1470, HOUSE+1 },     -- building
        { 1471, HOUSE+2 },     -- rom 134
        { 1472, HOUSE+3 },     -- BSU Dorm

        -- here comes Utility as 3220 ~3220+41(3261)

        { 13310, HOUSE+11 },
        { 13311, HOUSE+12 },
        { 13312, HOUSE+13 },
        { 13313, HOUSE+14 },
        { 13314, HOUSE+15 },
        { 13301, HOUSE+21 },
        { 13302, HOUSE+22 },
        { 13331, HOUSE+24 },
        { 13332, HOUSE+25 },
        { 13333, HOUSE+26 },
        { 13341, HOUSE+27 },
        { 13342, HOUSE+28 },

        -- after util ( 3280~)
        { 1403, HOUSE+71 },     -- Mansion
        { 2369, HOUSE+72 },     -- from Muncie
        { 2370, HOUSE+73 },
        { 1999, HOUSE+74 },     -- Scott's House

        { 1400, HOUSE+76 },       -- toot
        { 1404, HOUSE+77 },      -- Elbo
        { 1407, HOUSE+78 },      -- Pokerthief
        { 1419, HOUSE+79 },      -- God
        { 1416, HOUSE+80 },      -- Syn
        { 1417, HOUSE+81 },      -- Dave
        { 1402, HOUSE+82 },      -- Norton under con

        -- turning point to River of MID +30
        { 3503, RIVER+31 },     -- city enternace
        { 3502, RIVER+32 },     -- turning point
        { 3501, RIVER+33 },
        { 3500, RIVER+34 },


        { 13427, EASY+41 },     -- the secret room
        
        { 2720, GRAVE+60+10 },   -- NeverLand sand beach =   11280
        { 2721, GRAVE+60+11 },
        { 2722, GRAVE+60+12 },
        { 2723, GRAVE+60+13 },
        { 2724, GRAVE+60+14 },
        
        -- Dwarven Mine #6542 : error, shift by one
        { 6555, DWARVEN+42 },   -- inside mine
          
        { 6542, DWARVEN+43 },   -- Bottom of mine
        { 6543, DWARVEN+44 },
        { 6544, DWARVEN+45 },
        { 6545, DWARVEN+46 },
        { 6551, DWARVEN+47 },   -- Mining eq. room
        
        { 6546, DWARVEN+50 },
        { 6547, DWARVEN+51 },
        { 6548, DWARVEN+52 },
        { 6549, DWARVEN+53 },
        { 6550, DWARVEN+54 },
        { 6552, DWARVEN+55 },
        { 6553, DWARVEN+56 },
        
        --[[TODO
            13799 Olypus key
        ]]

        { 3001, MID+01 },     -- temple
        -- temple replace 3003

        { 3054, MID+04 },     -- altar
        { 3070, MID+05 },     -- board
        { 3071, MID+06 },     -- mail
        { 3061, MID+07 },     -- quest
        { 3008, MID+08 },     -- reception

        { 3005, MID+11 },     -- temple sq
        { 3003, MID+12 },     -- enter bank
        { 3002, MID+13 },     -- bank
        { 3004, MID+14 },     -- valut
        { 3065, MID+15 },     -- meta
        { 3062, MID+16 },     -- remortal
        { 3066, MID+17 },     -- hospital

        { 3019, MID+21 },     -- pr room
        { 3030, MID+22 },     -- dump
        { 3031, MID+23 },     -- pet shop
        { 3032, MID+24 },     -- pet shop store
        -- used shop
        -- rounge romm
        -- game room

        { 3033, MID+31 },     -- magic shop
        { 3011, MID+32 },     -- weapon store
        { 3020, MID+33 },     -- armory
        { 3010, MID+34 },     -- gen store
        { 3009, MID+35 },     -- bakery
        { 3006, MID+36 },     -- ent inn
        { 3007, MID+37 },     -- inn
        { 3017, MID+38 },     -- enter pr
        { 3018, MID+39 },     -- guild bar

        { 3027, MID+41 },     -- Mac
        { 3049, MID+42 },     -- Levee
        { 3051, MID+43 },     -- on the bridge
        { 3048, MID+44 },     -- groovy inn
        { 3050, MID+45 },     -- warehouse
        { 3035, MID+46 },     -- bombard
        { 3094, MID+47 },     -- storage
        { 3034, MID+48 },     -- Super DC

        { 3014, MID+60 },      -- market sq
        { 3012, MID+61 },      -- main str (magic)
        { 3013, MID+62 },      -- main str (bakery)
        { 3015, MID+63 },      -- main (gen store)
        { 3016, MID+64 },      -- main (weapon)
        { 3021, MID+65 },      -- ent TB

        { 3025, MID+66 },     -- comm sq
        { 3044, MID+67 },     -- poor alley
        { 3024, MID+68 },     -- Eastern end of poor ally
        { 3026, MID+69 },     -- dark alley
        { 3045, MID+70 },     -- ally at levee
        { 3046, MID+71 },     -- eastern end of alley

        { 3040, MID+72 },     -- in west gate
        { 3052, MID+73 },     -- outside west gate
        { 3041, MID+74 },     -- in east gate
        { 3053, MID+75 },     -- outside east gate
        { 3042, MID+76 },     -- wall road 1
        { 3043, MID+77 },     -- wall road 2
        { 3047, MID+78 },     -- wall road 3

        -- UNREF: ROOM	#0
    }

    for ix,  renum in ipairs(special_obj) do
      Object_index[renum[1]][RENUMINDEX] = renum[2]
    end
    
    --[[
    -- unref'ed Graveyard items
    for ix in range(0, 17) do
      Object_index[3410+ix][RENUMINDEX] = ORPHAN + 41 + ix
    end
    ]]
    
    for ix, renum in ipairs(special_mob) do
        Mobile_index[renum[1]][RENUMINDEX] = renum[2]
    end

    -- 이지렌드 (10-38)
    for ix in range(0, 28) do
        Room_index[13500+ix][RENUMINDEX] = EASY + 1 + ix
    end

    -- SecretZone (50-79)
    for ix in range(0, 25) do
        Room_index[13400+ix][RENUMINDEX] = EASY + 43 + ix
    end

    -- Pride land (80-99)
    for ix in range(0, 13) do
        Room_index[13601+ix][RENUMINDEX] = EASY + 71 + ix
    end

    for ix, renum in ipairs(special_room) do
        if renum[1] ~= 6555 or Room_index[6555] then
            Room_index[renum[1]][RENUMINDEX] = renum[2]
        end
    end
    
end

-- New Narai (partally renumbered)

Zone_renum_tab_master = {
    --                                   ORIG  RENUM SPAN
    { "LIMBO",                              0,  2900, 20 },
    { "The castle",                       200,  14110, 90 },
    { "Mel's Doghouse",                  1300,  11260, 0 },   -- To Grave
    { "Houses",                          1400,  3210, 90 },     -- Here is SLUM
    { "Dirk's Castle",                   1500,  14210, 90 },
    { "SHIRE",                           1700,  15100, 100 },
    { "Village of Midgaard",             1840,  3240, 0 },    -- To SLUM
    { "The lands",                       1900,  15200, 100 },
    { "The Wasteland",                   2100,  12200, 100 },
    { "Dragon Tower",                    2200,  14510, 90 },
    { "Muncie",                          2300,  11810, 90 },
    { "The Corporation",                 2500,  14700, 200 },
    { "The Never Land",                  2700,  11270, 0 },   -- To Grave
    { "The Keep of Mahn-Tor",            2900,  15800, 100 },
    { "Northern Midgaard Main City",     3000,  3000, 200 },
    { "Southern part of Midgaard",       3100,  3510, 90 },
    { "River of Midgaard",               3200,  3310, 90 },
    { "Graveyard",                       3400,  11210, 90 },
    { "Moria level 1-2",                 4000,  14310, 90 }, -- special 3500-3503
    { "Moria level 3-4",                 4100,  14410, 90 },
    { "The Wamphyri Aerie",              4200,  12510, 190 },
    { "The Great Eastern Desert",        5000,  12410, 90 },
    { "drow city",                       5100,  12710, 90 },
    { "The City of Thalos",              5200,  12810, 90 },
    { "HAON-DOR-LIGHT",                  6000,  15310, 90 },
    { "HAON-DOR-DARK",                   6100,  15410, 90 },
    { "The Dwarven Kingdom",             6500,  14610, 90 }, -- special 6999
    { "Sewer",                           7000,  11300, 100 },
    { "SecondSewer",                     7100,  11400, 100 },
    { "SEWERMAZE",                       7200,  11500, 200 },
    { "The tunnels",                     7400,  11710, 90 },
    { "Redferne's Residence",            7900,  11910, 90 },
    { "Arachnos",                        9000,  15500, 100 },
    { "Arena",                           9400,  12110, 90 },
    { "DeathKingdom",                    9500,  16710, 190 },
    { "Galaxy",                          9700,  15610, 90 },
    { "The Death Star",                  9800,  15710, 90 },
    { "Dae Rim Sa",                     11100,  16100, 100 },
    { "Utility Zone",                   13300,   3810, 90 },  -- HERE is HOUSE
    { "Easy zone",                      13400,  18210, 90 },
    { "Mount Olympus",                  13780,  12310, 90 },
    { "Process' castle",                13800,  18100, 100 },
    { "Robot city",                     15000,  17110, 290 },
    { "Kingdom Of Chok",                15800,  16310, 90 },
    { "Kingdom of Wee",                 16000,  16410, 190 },
    { "o-kingdom",                      17000,  16600, 100 },
    { "MooDang",                        18000,  17610, 190 },
    { "KAIST",                          19000,  17410, 190 },
    { "Good-Bad-Island",                23300,  16200, 100 },

}

-- Legacy Narai
Zone_renum_tab_legacy = {
    --                                   ORIG  RENUM
    { "LIMBO",                              0,     0 },
    { "The castle",                       200,   200 },
    { "Mel's Doghouse",                  1300,  1300 },
    { "Houses",                          1400,  1400 },
    { "Dirk's Castle",                   1500,  1500 },
    { "SHIRE",                           1700,  1700 },
--    { "Village of Midgaard",             1800,  1800 },
    { "The jale",                        1800,  1800 },
    { "The lands",                       1900,  1900 },
    { "Process' castle",                 2000, 13800 },
    { "The Wasteland",                   2100,  2100 },
    { "Dragon Tower",                    2200,  2200 },
    { "Muncie",                          2300,  2300 },
    { "The Corporation",                 2500,  2500 },
    { "The Never Land",                  2700,  2700 },
    { "Kingdom Of Chok",                 2800, 15800 },
    { "The Keep of Mahn-Tor",            2900,  2900 },
    { "Northern Midgaard Main City",     3000,  3000 },
    { "Southern part of Midgaard",       3100,  3100 },
    { "River of Midgaard",               3200,  3200 },
    { "Graveyard",                       3400,  3400 },
--    { "XXX Turning Point",               3500,  3500 },
--    { "Moria level 1-2",                 4000,  4000 },
    { "Moria level 1-2",                 3500,  4000 },
    { "Moria level 3-4",                 4100,  4100 },
    { "The Wamphyri Aerie",              4200,  4200 },
    { "The Great Eastern Desert",        5000,  5000 },
    { "drow city",                       5100,  5100 },
    { "The City of Thalos",              5200,  5200 },
    { "HAON-DOR-LIGHT",                  6000,  6000 },
    { "HAON-DOR-DARK",                   6100,  6100 },
    { "The Dwarven Kingdom",             6500,  6500 },
--    { "XXX Dwarven Jail",                6990,  6990 },
    { "Sewer",                           7000,  7000 },
    { "SecondSewer",                     7100,  7100 },
    { "SEWERMAZE",                       7200,  7200 },
    { "The tunnels",                     7400,  7400 },
    { "Redferne's Residence",            7900,  7900 },
    { "Arachnos",                        9000,  9000 },
    { "Arena",                           9400,  9400 },
    { "DeathKingdom",                    9500,  9500 },
    { "Galaxy",                          9700,  9700 },
    { "The Death Star",                  9800,  9800 },
    { "Dae Rim Sa",                     11100, 11100 },
--    { "Utility Zone",                   13300, 13300 },
--    { "Easy zone",                      13400, 13400 },
    { "Mount Olympus",                  13700, 13700 },
--    { "Process' castle",                13800, 13800 },
    { "Robot city",                     15000, 15000 },
--    { "Kingdom Of Chok",                15800, 15800 },
    { "Kingdom of Wee",                 16000, 16000 },
    { "o-kingdom",                      17000, 17000 },
    { "Utility Zone",                   19000, 13300 },
    { "MooDang",                        20000, 18000 },
    { "Good-Bad-Island",                23300, 23300 },
    { "A Mi Dae Jun",                   25000, 23300 },
    { "KAIST",                          30000, 19000 },
--    { "Good-Bad-Island",                23300, 23300 },

}

main()


function test()

    local a, b = 1, 2
    a = pathexists('d/')
    print (a)

end

-- test()



--[[

2900-2999
    { "LIMBO",                          0,     0 },

3000-3199
    { "Northern Midgaard Main City",    3000,  3000 },


3200-3299
   3200 (Slum /Street of Mid
 -- 3210 (30)
    { "Houses2",                         1450,  1400 },

    3240 (10){ Library 3090 ~ 3093  }
    3250 (40)
    { "Village of Midgaard",            1850,  1800 },
    3290 (10)
    { "XXX Dwarven Jail",               6990,  6990

3300-3399
        3310 (30)
    { "River of Midgaard",              3200,  3200 },

    3340 (10)
    { "XXX Turning Point",              3500,  3500 },


3400-3499
-- KIT School zone


3500-3599
    { "Southern part of Midgaard",      3100,  3100 },

3700

3800-3899
    Village of
    3810 (10)
    House3
    3820 (50)
    { "Utility Zone",                   13300, 13300 },
    3870 (30)
    { "Houses",                         1400,  1400 },


======================================================
--------------------- 10000  ----


 ( Midgaard )

-------



11100 Orphan Area

11200-11299
11210(40)-12
    { "Graveyard",                      3400,  3400 },
11260   (10)
    { "Mel's Doghouse",                 1300,  1300 }
11270 (30)

    { "The Never Land",                 2700,  2700 },

11300-1399
    { "Sewer",                          7000,  7000 },
11400-1499
    { "SecondSewer",                    7100,  7100 },
11500-1699
    { "SEWERMAZE",                      7200,  7200 },

11700-11799
    { "The tunnels",                    7400,  7400 },


11800-11899
    { "Muncie",                         2300,  2300 },

11900-11999 (30)
    { "Redferne's Residence",           7900,  7900 },

--------------- 12000


12100-12199    
    { "Arena",                          9400,  9400 }

12200-12299
    { "The Wasteland",                  2100,  2100 },
12300-12399
    { "Mount Olympus",                  13700, 13700 },

12400-12499
    { "The Great Eastern Desert",       5000,  5000 },

12500-12699
    { "The Wamphyri Aerie",             4200,  4200 },
12700-12799
    { "drow city",                      5100,  5100 },
12800-12899
    { "The City of Thalos",             5200,  5200 },

--------------- 13000

RESERVED

------------------- 14000-------------------------------------

EAST 14000

14100-14199
    { "The castle",                     200,   200 },
14200-14299
    { "Dirk's Castle",                  1500,  1500 },
14300-14399
    { "Moria level 1-2",                4000,  4000 },
14400-14499
    { "Moria level 3-4",                4100,  4100 },
14500-14599
    { "Dragon Tower",                   2200,  2200 },
14600-14699
    { "The Dwarven Kingdom",            6500,  6500 },

14700-14899
    { "The Corporation",                2500,  2500 }

---------------------15000 -------
WEST 15000
15100-15199
    { "SHIRE",                          1700,  1700 },
15200-15299
    { "The lands",                      1900,  1900 },

15300-15399
    { "HAON-DOR-LIGHT",                 6000,  6000 },
15400-15499
    { "HAON-DOR-DARK",                  6100,  6100 },

15500-15599
    { "Arachnos",                       9000,  9000 },
    
15600-15699
    { "Galaxy",                         9700,  9700 },
15700-15799
    { "The Death Star",                 9800,  9800 },
15800-15899
    { "The Keep of Mahn-Tor",           2900,  2900 }
    
------------- 16000 ---------------------

16100-16199
    { "Dae Rim Sa",                     11100, 11100 },

16200-16299
    { "Good-Bad-Island",                23300, 23300 },
    
16300-16399  
    { "Kingdom Of Chok",                15800, 15800 },
16400-16599
    { "Kingdom of Wee",                 16000, 16000 },
16600-16699
    { "o-kingdom",                      17000, 17000 },

16700-16899
    { "DeathKingdom",                   9500,  9500 },
   
----------- 17000

17100-17399
    { "Robot city",                     15000, 15000 }
    
17400-17599
    { "KAIST",                          19000, 19000 }

17600-17799
    { "MooDang",                        18000, 18000 },
17800-17899
     {아미대전 }
     
---------- 18000  
18100-18199
    { "Process' castle",                13800, 13800 },

18200-18299
    { "Easy zone",                      13400, 13400 },

--------------------------------------

   1 LIMBO                               0  100 (  28   0      0 -    27 )
   2 The castle                        200  100 (  73   1    201 -   273 )
   3 Mel's Doghouse                   1300  100 (   3   1   1301 -  1303 )
   4 Houses                           1400  100 (  73   0   1400 -  1472 )
   5 Dirk's Castle                    1500  100 (  52   0   1500 -  1551 )
   6 SHIRE                            1700  100 ( 100   0   1700 -  1799 )
   7 Village of Midgaard              1840   60 (  44   1   1841 -  1884 )
   8 The lands                        1900  100 (  99   1   1901 -  1999 )
   9 The Wasteland                    2100  100 (  97   0   2100 -  2196 )
  10 Dragon Tower                     2200  100 (  44   1   2201 -  2244 )
  11 Muncie                           2300  100 (  71   0   2300 -  2370 )
  12 The Corporation                  2500  200 ( 193   0   2500 -  2692 )
  13 The Never Land                   2700  100 (  25   0   2700 -  2724 )
  14 The Keep of Mahn-Tor             2900  100 ( 100   0   2900 -  2999 )
  15 Northern Midgaard Main City      3000  100 (  94   1   3001 -  3094 )
  16 Southern part of Midgaard        3100  100 (  42   0   3100 -  3141 )
  17 River of Midgaard                3200  100 (  21   0   3200 -  3220 )
  18 Graveyard                        3400  100 (   6   0   3400 -  3405 )
  19 Moria level 1-2                  3500  600 ( 575   0   3500 -  4074 )
  20 Moria level 3-4                  4100  100 (  26   0   4100 -  4125 )
  21 The Wamphyri Aerie               4200  131 ( 127   1   4201 -  4327 )
  22 The Great Eastern Desert         5000  100 (  72   1   5001 -  5072 )
  23 drow city                        5100  100 (  51   0   5100 -  5150 )
  24 The City of Thalos               5200  100 (  51   0   5200 -  5250 )
  25 HAON-DOR-LIGHT                   6000  100 (  24   0   6000 -  6023 )
  26 HAON-DOR-DARK                    6100  400 (  44   0   6100 -  6143 )
  27 The Dwarven Kingdom              6500  500 ( 500   0   6500 -  6999 )
  28 Sewer                            7000  100 (  99   1   7001 -  7099 )
  29 SecondSewer                      7100  100 (  99   1   7101 -  7199 )
  30 SEWERMAZE                        7200  200 ( 200   0   7200 -  7399 )
  31 The tunnels                      7400  100 (  46   0   7400 -  7445 )
  32 Redferne's Residence             7900  100 (  21   0   7900 -  7920 )
  33 Arachnos                         9000  100 (  99   1   9001 -  9099 )
  34 Arena                            9400  100 (  46   0   9400 -  9445 )
  35 DeathKingdom                     9500  200 ( 123   0   9500 -  9622 )
  36 Galaxy                           9700   72 (  71   1   9701 -  9771 )
  37 The Death Star                   9800   52 (  52   0   9800 -  9851 )
  38 Dae Rim Sa                      11100  100 (  99   1  11101 - 11199 )
  39 Utility Zone                    13300  100 (  42   1  13301 - 13342 )
  40 Easy zone                       13400  300 ( 215   0  13400 - 13614 )
  41 Mount Olympus                   13700  100 (  19  80  13780 - 13798 )
  42 Process' castle                 13800  100 ( 100   0  13800 - 13899 )
  43 Robot city                      15000  300 ( 227   0  15000 - 15226 )
  44 Kingdom Of Chok                 15800  100 (  36   0  15800 - 15835 )
  45 Kingdom of Wee                  16000  200 ( 120   0  16000 - 16119 )
  46 o-kingdom                       17000  100 (  95   0  17000 - 17094 )
  47 MooDang                         18000  200 ( 151   0  18000 - 18150 )
  48 KAIST                           19000  200 ( 158   1  19001 - 19158 )
  49 Good-Bad-Island                 23300  100 (  90   1  23301 - 23390 )

]]


