#!/usr/bin/lua

require('KZIOLib')
require('KZDatabase')


-----------------------------------------------------------

function main()

    -- check and renumber zone files
    -- To renum, do 'mkdir renum renum/zone renum/world' for output
    G_renum = false

    Zone_renum_tab = Zone_renum_tab3

    print('================================================================')

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
        local wf = 'world/' .. zinfo[3]

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
        local zf = 'zone/' .. zinfo[2]
        local wf = 'world/' .. zinfo[3]

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

        renum_zone_list()
    end
    print('All Done.')

end


function renum_special_cases()

    MID = 13000
    ORPHAN = 2010       -- unreferenced/unused items

    SLUM = 3210         -- club safari + Archmage + Guild rooms
    HOUSE = 3810        -- House + Rebert bell building

    GRAVE = 11210       -- Graveyard/Mels dog house/Neverland
    TUNNEL = 11710      -- The tunnel
    GKP =  11910        -- Grand Knight Paladin (Redferne Residence)
    WASTE = 12200       -- Waste land
    DESERT = 12410      -- Great eastern desert
    SHIRE = 15100       -- shire
    HDK = 15510         -- haondor dark

    special_obj =
    {

        { 1002, ORPHAN+1 },     -- champ belt UNREF: OBJ	#1002	champion belt
        { 1300, ORPHAN+2 },     -- immo UNREF: OBJ	#1300	immo gloves magic
        { 1301, ORPHAN+3 },      -- UNREF: OBJ	#1301	immo wooden Kal

        { 1304, ORPHAN+4 },     -- immo mail UNREF: OBJ	#1304	immo vulcan mail
        -- silver epee (The tunnel)/halbard (shire) {to tunnel (7400)} TODO: CHECK
        { 1305, TUNNEL+1 },      -- silver epee TO The tunnel

        { 1309, DESERT+41 },    -- ancient plate (great eastern deset)
        { 1310, ORPHAN+5 },     -- nothing UNREF: OBJ	#1310	nothing

        { 1312, ORPHAN+6 },      --- claw of faust  UNREF: OBJ	#1312	claws faust
        { 1314, ORPHAN+7 },     -- white dove -  UNREF: OBJ	#1314	white dove

        { 1318, 12710+21 },       -- banner bloody Drow City(5100)
        { 1319, DESERT+42 },       -- green cresent banner Great easten desert

        { 1321, ORPHAN+8 },     -- UNREF: OBJ	#1321	blade chaos

        { 3121, GRAVE+41 },     -- key rusty : from Southern MID to Grave
        -- { 6110, GRAVE?? }    -- Strenth potion(yellow)  to Grave yard
        { 6110, GRAVE+42 },     --> strength potion (yellow) ONLY GRAVE
        { 1303, GRAVE+43 },     -- seraphim protector to grave yard
        { 1331, GRAVE+44 },     -- white cape to grave yard
        { 6111, GRAVE+45 },     -- large round shield

        { 2700, GRAVE+51 },     -- #2700 깃털
        { 1325, GRAVE+52 },     -- Mauve stone to Grave (1300)

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

        { 1327, HOUSE+1 },      -- magnolias garland~ (House)

        { 1335, 15600+11 },     -- speckled potion to arachnos(9000)
        { 1462, 15300+31 },     -- red potion to the land (1900) ** check over**

        { 1306, HDK+21 },       -- large scythe (handor Dark)
        { 1316, HDK+22 },       -- armored plate evil roy (6137) haodor dark
        { 1620, HDK+23 },       -- colot 45
        { 1621, HDK+24 },       -- bullet
        { 5999, HDK+25 },       -- black leather boost, Lumber jack boot

        { 6910, HDK+31 },       -- a black dragon scale shirt
        { 6911, HDK+32 },       -- a black dragon scale skirt
        { 6912, HDK+33 },       -- cloak black hooded
        { 6913, HDK+34 },       -- a broad silver belt
        { 6914, HDK+35 },       -- a long, slender sword

        { 7215, HDK+36 },       -- light libre HAODOR DARK ONLY

        { 1871, SLUM+51 },      -- 88 light
        { 1872, SLUM+52 },      -- sky light
        { 1873, SLUM+53 },      -- drug

        { 1311, WASTE+62 },     -- totem (waste land) DO SEED
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
        { 2298, 14510+58 },     -- DT, a dragon winged cloak~
        { 2299, 14510+59 },     --  DT, key gold tarnished~

        { 3300, ORPHAN+11 },    -- a metal key~     UNREF: OBJ	#3300	key metal
        { 3301, ORPHAN+12 },    --  a brass key~    UNREF: OBJ	#3301	key brass
        { 5050, ORPHAN+13 },    -- cloak blue -- -- UNREF: OBJ	#5050	cloak blue
        { 5051, ORPHAN+14 },    -- rifle assualt  -- UNREF: OBJ	#5051	rifle assualt

        -- global potions
        -- { 6109, HDK+?? }     -- blue potion (?? ) remain in hoandor dark
        { 6115, 11810+31 },     -- haste/ powerful potion to Muncie (2300)
        -- { 6116, SLUM+??}     -- improved state potion to Library
        { 6118, 12510+31 },     -- wand of vitalize to Aerie

        { 6117, GKP+21 },    -- mana boost staff to Paladin
        { 7090, GKP+22 },    -- waybread  to residence ONLY
        { 7190, TUNNEL+2 },  -- glinting ring of silver: (The tunnel Red dragon) or Paladin
        { 7191, SHIRE+21 },  -- Goldring : Shire(1700) or Paladin
        { 7192, TUNNEL+3 },  -- magic boot (the Tunnel Jones ONLY)
        { 7207, GKP+26 },    -- muddy gauntlets to Paladin ONLY
        { 7212, GKP+27 },    -- Bastard sword, TO Paladin

        { 7213, ORPHAN+15 },    -- large helmet  UNREF: OBJ	#7213	helmet large
        { 7214, ORPHAN+16 },    -- chaos helmet  UNREF: OBJ	#7214	helmet chaos

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

        { 1000, MID+11 },     -- sundew fountain
        { 1001, MID+12 },     -- altar

        { 3099, MID+13 },     --  board
        { 3098, MID+14 },     --  mail box
        { 7999, MID+15 },     -- slot machine
        { 8000, MID+16 },     -- string machine

        { 3050, MID+21 },     --  scroll id
        { 3051, MID+22 },     --  potion see inv
        { 3052, MID+23 },     --  scroll recall
        { 3053, MID+24 },     --  inv wand
        { 3054, MID+25 },     --  staff sleep
        { 3137, MID+26 },     --  opaque potion

        { 1317, MID+31 },     --  anti-dc
        { 1308, MID+32 },     --  vault key
        { 3133, MID+33 },     --  City Key  (from Southern MID)

        { 3000, MID+41 },     --  beer barrel
        { 3001, MID+42 },     --  beer bottle
        { 3002, MID+43 },     --  ale bottle
        { 3003, MID+44 },     --  firebreather
        { 3004, MID+45 },     --  local bott
        { 3005, MID+46 },     --  fancy bottle
        { 3008, MID+47 },     --  condition

        { 3007, MID+51 },     --  coke
        { 3006, MID+52 },     --  cheese
        { 3010, MID+53 },     --  bread
        { 3011, MID+54 },     --  danish :

        { 3061, MID+55 },     --  canoe
        { 3060, MID+56 },     --  raft
        { 1313, MID+57 },     --  map
        { 1322, MID+58 },     -- align

        { 3032, MID+61 },     --  bag
        { 3033, MID+62 },     --  box
        { 3031, MID+63 },     --  lantern
        { 3030, MID+64 },     --  torch
        { 3035, MID+65 },     --  paper
        { 3034, MID+66 },     --  pen

        --- TICKET:  7991 7992, 7993 7994
        -- QUEST weap. armor 7997 7998
        { 7997, MID+71 },     --  quest weapon
        { 7998, MID+72 },     --  quest armor
        { 7994, MID+74 },     --  ticket req
        { 7991, MID+75 },     --  ticket ac
        { 7993, MID+76 },     --  ticket dr
        { 7992, MID+77 },     --  ticket hr

        { 3022, MID+111 },     --  sword long
        { 3021, MID+112 },     --  small sword
        { 3020, MID+113 },     --  dagger
        { 3023, MID+114 },     --  club wooden
        { 3024, MID+115 },     --  war hammer

        { 3040, MID+121 },     --  breast plate
        { 3041, MID+122 },     --  chain mail shirt
        { 3042, MID+123 },     --  wooden shield
        { 3043, MID+124 },     --  jacket leather
        { 3044, MID+125 },     --  jacket studd
        { 3045, MID+126 },     --  scale mail
        { 3046, MID+127 },     --  bronze breast
        { 3070, MID+128 },     --  gauntlt bronze
        { 3071, MID+129 },     --  glove leather
        { 3075, MID+130 },     --  helmet bronze
        { 3076, MID+131 },     --  cap leather
        { 3080, MID+132 },     --  legging bronze
        { 3081, MID+133 },     --  pant leather
        { 3085, MID+134 },     --  sleeve bronze
        { 3086, MID+135 },     --  sleeve leather
        { 7922, MID+136 },     --  leather knee
        { 7923, MID+137 },     --  bronze knee

        { 3026, MID+141 },     --  swod saurai
        { 3027, MID+142 },     --  sword bombard
        { 3025, MID+143 },     --  sword particle
        { 1307, MID+144 },     --  carpenter hammer

        { 1326, MID+145 },     --  pendant
        { 1302, MID+146 },     --  bagrd boot
        { 1622, MID+147 },     --  shotgun
        { 1623, MID+148 },     --  shell ammo
        { 1315, MID+149 },     --  broom

        { 1605, MID+150 },     -- brass shild
        { 1320, MID+151 },     -- sunshade
        { 1330, MID+152 },     -- baseball cap

        -- perhaps: 7924, 7925, 26, 27, 28,29
        { 7924, MID+161 },     -- perhaps
        { 7925, MID+162 },     -- perhaps
        { 7926, MID+163 },     -- perhaps
        { 7927, MID+164 },     -- perhaps
        { 7928, MID+165 },     -- perhaps
        { 7929, MID+166 },     -- perhaps

        { 3090, SLUM+42 },      -- archmage robe
        { 3091, SLUM+43 },      -- magi staff
        { 6116, SLUM+41 },      -- imp haste powerful potion

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

        { 1470, HOUSE+1 },     -- labbie
        { 1400, HOUSE+2 },     -- cinnamon
        { 1924, HOUSE+5 },     -- Charon: deathreaper

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
        { 13601, 18710+1 },     -- UNREF: MOB	#13601	wang
        { 13602, 18710+2 },     -- UNREF: MOB	#13602	cap
        { 13603, 18710+3 },     -- UNREF: MOB	#13603	Mac

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

        { 3040, MID+21 },   -- bartender
        { 3042, MID+22 },   -- waiter Ron
        { 3024, MID+23 },   -- guild ent
        { 3026, MID+24 },   -- UNREF: MOB	#3026	Chester Wilson
        { 3046, MID+25 },   -- filthy

        { 3152, MID+31 },   -- bank examiner
        { 3153, MID+32 },   -- bank president
        { 1460, MID+33 },   -- grand ma
        { 8000, MID+34 },   -- bag lady
        { 3154, MID+35 },    -- undercover

        { 3133, MID+41 },   -- mushashi
        { 3145, MID+43 },   -- mom
        { 3125, MID+44 },   -- DC
        { 3135, MID+45 },   -- super DC

        { 3134, MID+50 },   -- bombard
        -- 3126 ~ 3132 = --  7 hit series
        { 3126, MID+51 },
        { 3127, MID+52 },
        { 3128, MID+53 },
        { 3129, MID+54 },
        { 3130, MID+55 },
        { 3131, MID+56 },
        { 3132, MID+57 },

        { 3060, MID+61 },   -- city guard 1
        { 3067, MID+62 },   -- cityguard 2
        { 3148, MID+63 },   -- unarmed city
        { 3150, MID+64 },   -- security guard

        { 3061, MID+65 },   -- janitor
        { 3062, MID+66 },   -- fido
        { 3066, MID+67 },   -- odif yltsaeb~
        { 3063, MID+68 },   -- mercenary
        { 3064, MID+69 },   -- drunk
        { 3065, MID+70 },   -- begger
        { 3147, MID+71 },   -- urchin

        { 3005, MID+75 },   -- Doris smith
        { 3151, MID+76 },   -- bogan
        { 3146, MID+77 },   -- suzy
        { 3045, MID+78 },   -- carpenter

        -- pet shop
        { 3090, MID+90 },   -- kitten
        { 3091, MID+91 },   -- puppy
        { 3092, MID+92 },   -- beagle~
        { 3093, MID+93 },   -- rottweiler
        { 3094, MID+94 },   -- wolf
        { 8006, MID+95 },   -- juggnut

        { 19999, MID+99 },         -- super mushashi  UNREF: MOB	#19999	super musashi

        { 3070, SLUM+11 },   -- crank lib
        { 3071, SLUM+12 },   -- furniture
        { 3072, SLUM+13 },   -- grand mage

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
UNREF: MOB	#13807	2nd keeper
UNREF: MOB	#13808	3rd keeper
UNREF: MOB	#13809	4th keeper
UNREF: MOB	#13810	5th keeper
UNREF: MOB	#13811	6th keeper
UNREF: MOB	#15836	secret guard
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
        { 1861, SLUM+85 },     -- locker room shift up

        { 1862, SLUM+87 },     -- Police jail room to besides The_Jail

        { 1999 , HOUSE+60+9 }, -- Scott's

        --[[
            street of MID
            3210-3214  safari club
            3221 - library
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

        { 3090, SLUM+31 },     -- library/archmage
        { 3091, SLUM+32 },
        { 3092, SLUM+33 },
        { 3093, SLUM+34 },

        -- UNREF: ROOM	#0
    }

    for ix,  renum in ipairs(special_obj) do
        Object_index[renum[1]][RENUMINDEX] = renum[2]
    end

    for ix, renum in ipairs(special_mob) do
        Mobile_index[renum[1]][RENUMINDEX] = renum[2]
    end

    for ix, renum in ipairs(special_room) do
        Room_index[renum[1]][RENUMINDEX] = renum[2]
    end


end


function test()

    local a, b = 1, 2
    a = pathexists('d/')
    print (a)

end

-- test()

Zone_renum_tab1 = {
    --                                   ORIG  RENUM
    { "LIMBO",                              0,  2000 },
    { "The castle",                       200,  14110 },
    { "Mel's Doghouse",                  1300,  11250 },
    { "Houses",                          1400,  3870 },
    { "XXX Houses2",                     1450,  3210 },
    { "XXX Houses3",                     1470,  3810 },
    { "Dirk's Castle",                   1500,  14210 },
    { "SHIRE",                           1700,  15100 },
    { "Village of Midgaard",             1860,  3260 },
    { "The lands",                       1900,  15300 },
    { "The Wasteland",                   2100,  12200 },
    { "Dragon Tower",                    2200,  14510 },
    { "Muncie",                          2300,  11810 },
    { "The Corporation",                 2500,  11300 },
    { "The Never Land",                  2700,  11260 },
    { "The Keep of Mahn-Tor",            2900,  16100 },
    { "Northern Midgaard Main City",     3000,  3000 },
    { "Southern part of Midgaard",       3100,  3510 },
    { "River of Midgaard",               3200,  3310 },
    { "Graveyard",                       3400,  11210 },
    { "XXX Turning Point",               3500,  3340 },
    { "Moria level 1-2",                 4000,  14310 },
    { "Moria level 3-4",                 4100,  14410 },
    { "The Wamphyri Aerie",              4200,  12510 },
    { "The Great Eastern Desert",        5000,  12410 },
    { "drow city",                       5100,  12710 },
    { "The City of Thalos",              5200,  12810 },
    { "HAON-DOR-LIGHT",                  6000,  15510 },
    { "HAON-DOR-DARK",                   6100,  15510 },
    { "The Dwarven Kingdom",             6500,  14610 },
    { "XXX Dwarven Jail",                6990,  3290 },
    { "Sewer",                           7000,  11300 },
    { "SecondSewer",                     7100,  11400 },
    { "SEWERMAZE",                       7200,  11500 },
    { "The tunnels",                     7400,  11710 },
    { "Redferne's Residence",            7900,  11910 },
    { "Arachnos",                        9000,  15600 },
    { "Arena",                           9400,  11110 },
    { "DeathKingdom",                    9500,  16310 },
    { "Galaxy",                          9700,  15810 },
    { "The Death Star",                  9800,  16210 },
    { "Dae Rim Sa",                     11100,  17100 },
    { "Utility Zone",                   13300,  3820 },
    { "XXX Easy zone 1",                      13400,  18740 },
    { "XXX Easy zone 2",                      13500,  18710 },
    { "XXX Easy zone 3",                      13600,  18770 },
    { "Mount Olympus",                  13780,  12310 },
    { "Process' castle",                13800,  18600 },
    { "Robot city",                     15000,  18110 },
    { "Kingdom Of Chok",                15800,  16510 },
    { "Kingdom of Wee",                 16000,  16610 },
    { "o-kingdom",                      17000,  16800 },
    { "MooDang",                        18000,  17510 },
    { "KAIST",                          19000,  18410 },
    { "Good-Bad-Island",                23300,  17300 },

}


--[[

2000-
    { "LIMBO",                          0,     0 },

3000
    { "Northern Midgaard Main City",    3000,  3000 },


3200
   3200 (Slum /Street of Mid
 -- 3210 (30)
    { "Houses2",                         1450,  1400 },

    3240 (10){ Library 3090 ~ 3093  }
    3250 (40)
    { "Village of Midgaard",            1850,  1800 },
    3290 (10)
    { "XXX Dwarven Jail",               6990,  6990

3300
        3310 (30)
    { "River of Midgaard",              3200,  3200 },

    3340 (10)
    { "XXX Turning Point",              3500,  3500 },


3400
-- KIT School zone


3510
    { "Southern part of Midgaard",      3100,  3100 },


3800
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



11200

11210(40)
    { "Graveyard",                      3400,  3400 },
11250   (10)
    { "Mel's Doghouse",                 1300,  1300 }
11260 (30)

    { "The Never Land",                 2700,  2700 },

11300
    { "Sewer",                          7000,  7000 },
11400
    { "SecondSewer",                    7100,  7100 },
11500
    { "SEWERMAZE",                      7200,  7200 },

11700
    { "The tunnels",                    7400,  7400 },


11800
    { "Muncie",                         2300,  2300 },

11900 (30)
    { "Redferne's Residence",           7900,  7900 },

--------------- 12000


11100    { "Arena",                          9400,  9400 }

12200
    { "The Wasteland",                  2100,  2100 },
12300
    { "Mount Olympus",                  13700, 13700 },

12400
    { "The Great Eastern Desert",       5000,  5000 },

12500
    { "The Wamphyri Aerie",             4200,  4200 },
12700
    { "drow city",                      5100,  5100 },
12800
    { "The City of Thalos",             5200,  5200 },

--------------- 13000

RESERVED

------------------- 14000-------------------------------------

EAST 14000



14100
    { "The castle",                     200,   200 },
14200
    { "Dirk's Castle",                  1500,  1500 },
14300
    { "Moria level 1-2",                4000,  4000 },
14400
    { "Moria level 3-4",                4100,  4100 },
14500
    { "Dragon Tower",                   2200,  2200 },
14600
    { "The Dwarven Kingdom",            6500,  6500 },

14700
    { "The Corporation",                2500,  2500 }

---------------------15000 -------
WEST 22000
15100
    { "SHIRE",                          1700,  1700 },
15300
    { "The lands",                      1900,  1900 },

15400

    { "HAON-DOR-LIGHT",                 6000,  6000 },
15500
    { "HAON-DOR-DARK",                  6100,  6100 },

15600
    { "Arachnos",                       9000,  9000 },

15800
    { "Galaxy",                         9700,  9700 },


------------- 16000 ---------------------

16100
    { "The Keep of Mahn-Tor",           2900,  2900 },

16200
    { "The Death Star",                 9800,  9800 },

16300
    { "DeathKingdom",                   9500,  9500 },

16500
    { "Kingdom Of Chok",                15800, 15800 },
16600
    { "Kingdom of Wee",                 16000, 16000 },
16800,
    { "o-kingdom",                      17000, 17000 },


----------- 17000
17100
    { "Dae Rim Sa",                     11100, 11100 },
17300
    { "Good-Bad-Island",                23300, 23300 },
17500
    { "MooDang",                        18000, 18000 },
17700
     {아미대전 }

----------- 18000

18100
    { "Robot city",                     15000, 15000 },

18400
    { "KAIST",                          19000, 19000 }


18600
    { "Process' castle",                13800, 13800 },

18700
    { "Easy zone",                      13400, 13400 },

--------------------------------------




   1 LIMBO                               0   99 (  27   0      0 -    27 )
   2 The castle                        200   99 (  72   1    201 -   273 )
   3 Mel's Doghouse                   1300   99 (   2   1   1301 -  1303 )
   4 Houses                           1400   99 (  72   0   1400 -  1472 )
   5 Dirk's Castle                    1500   99 (  51   0   1500 -  1551 )
   6 SHIRE                            1700   99 (  99   0   1700 -  1799 )
   7 Village of Midgaard              1800   99 (  43  41   1841 -  1884 )
   8 The lands                        1900   99 (  98   1   1901 -  1999 )
   9 The Wasteland                    2100   99 (  96   0   2100 -  2196 )
  10 Dragon Tower                     2200   90 (  43   1   2201 -  2244 )
  11 Muncie                           2300   99 (  70   0   2300 -  2370 )
  12 The Corporation                  2500  199 ( 192   0   2500 -  2692 )
  13 The Never Land                   2700   99 (  24   0   2700 -  2724 )
  14 The Keep of Mahn-Tor             2900   99 (  99   0   2900 -  2999 )
  15 Northern Midgaard Main City      3000   99 (  93   1   3001 -  3094 )
  16 Southern part of Midgaard        3100   99 (  41   0   3100 -  3141 )
  17 River of Midgaard                3200   99 (  20   0   3200 -  3220 )
  18 Graveyard                        3400   99 (   5   0   3400 -  3405 )
  19 XXX Turning Point                3500    9 (   3   0   3500 -  3503 )
  20 Moria level 1-2                  4000   99 (  74   0   4000 -  4074 )
  21 Moria level 3-4                  4100   99 (  25   0   4100 -  4125 )
  22 The Wamphyri Aerie               4200  130 ( 126   1   4201 -  4327 )
  23 The Great Eastern Desert         5000   99 (  71   1   5001 -  5072 )
  24 drow city                        5100   99 (  50   0   5100 -  5150 )
  25 The City of Thalos               5200   99 (  50   0   5200 -  5250 )
  26 HAON-DOR-LIGHT                   6000   99 (  23   0   6000 -  6023 )
  27 HAON-DOR-DARK                    6100   99 (  43   0   6100 -  6143 )
  28 The Dwarven Kingdom              6500   99 (  53   0   6500 -  6553 )
  29 XXX Dwarven Jail                 6990    9 (   0   9   6999 -  6999 )
  30 Sewer                            7000   99 (  98   1   7001 -  7099 )
  31 SecondSewer                      7100   99 (  98   1   7101 -  7199 )
  32 SEWERMAZE                        7200  199 ( 199   0   7200 -  7399 )
  33 The tunnels                      7400   99 (  45   0   7400 -  7445 )
  34 Redferne's Residence             7900   99 (  20   0   7900 -  7920 )
  35 Arachnos                         9000   99 (  98   1   9001 -  9099 )
  36 Arena                            9400   99 (  45   0   9400 -  9445 )
  37 DeathKingdom                     9500  199 ( 122   0   9500 -  9622 )
  38 Galaxy                           9700   71 (  70   1   9701 -  9771 )
  39 The Death Star                   9800   51 (  51   0   9800 -  9851 )
  40 Dae Rim Sa                      11100  199 (  98   1  11101 - 11199 )
  41 Utility Zone                    13300   99 (  41   1  13301 - 13342 )
  42 Easy zone                       13400  299 ( 214   0  13400 - 13614 )
  43 Mount Olympus                   13700   98 (  18  80  13780 - 13798 )
  44 Process' castle                 13800   99 (  99   0  13800 - 13899 )
  45 Robot city                      15000  299 ( 226   0  15000 - 15226 )
  46 Kingdom Of Chok                 15800   99 (  35   0  15800 - 15835 )
  47 Kingdom of Wee                  16000  199 ( 119   0  16000 - 16119 )
  48 o-kingdom                       17000   99 (  94   0  17000 - 17094 )
  49 MooDang                         18000  199 ( 150   0  18000 - 18150 )
  50 KAIST                           19000  199 ( 157   1  19001 - 19158 )
  51 Good-Bad-Island                 23300   99 (  89   1  23301 - 23390 )
]]





--[[


(3011)
(3012)

OOZ OBJ  #1000 'sundew bubble machine'  @ Northern Midgaard Main City (3000)
OOZ OBJ  #1000 'sundew bubble machine'  @ Utility Zone (13300)
OOZ OBJ  #1000 'sundew bubble machine'  @ Village of Midgaard (1840)
OOZ OBJ  #1001 'altar'  @ Northern Midgaard Main City (3000)
OOZ OBJ  #1001 'altar'  @ Village of Midgaard (1840)




#3099  (3013)
board bulletin~
a bulletin board~

#3098  (3014)
board mbox mail~
a mail box~

(301, 3016)

OOZ OBJ  #7999 'slot machine'  @ Northern Midgaard Main City (3000)
OOZ OBJ  #8000 'string machine'  @ Northern Midgaard Main City (3000)

------------- 3021

OOZ OBJ  #1308 'key vault'  @ Northern Midgaard Main City (3000)



OOZ OBJ  #1317 'anti deathcure'  @ Southern part of Midgaard (3100)


----- 3030


#3007 (3031)
coke cup~
A large cup~


#3006  (3032)
cheeseburger~
A cheeseburger~



#3000 () (3033)
beer barrel~
a barrel~


#3001 (3034)
beer bottle~
a bottle~

-
#3002  (3035)
ale bottle dark~
a bottle~


#3003 (3036)
firebreather bottle~
a firebreather~


#3004 (3037)
local bottle~
a bottle~


#3005 (3038)
water fancy bottle~
a fancy bottle~


#3010 (3041)
bread~
a bread~

#3011 (3042)
danish pastry~
a danish pastry~


------------- 3040


#3045
canoe~
a canoe~


#3060 (3046)
raft~
a raft~




(3051, 3052)

OOZ OBJ  #1313 'map'  @ Northern Midgaard Main City (3000)
OOZ OBJ  #1322 'align keywords great super sundew supersundew nogada'  @ Arena (9400)
OOZ OBJ  #1322 'align keywords great super sundew supersundew nogada'  @ Northern Midgaard Main

#3055
lantern~
a lantern~

#3030 (3056)
torch~
a torch~



#3032 (3057)
bag~
a bag~

#3033 (3058)
box~
a box~
A small box.~

#3034 (3059)
pen~
a pen~

#3035
paper white~
a white paper~





-------------------3060




#3050    (3050)
scroll identify~
a scroll of identify~





#3052
scroll recall~
a scroll of recall~

#3051
potion yellow~
a yellow potion of see invisible~

OOZ OBJ  #3137 'scroll recharge'  @ Northern Midgaard Main City (3000)


#3053
wand~
a wand of invisibility~

#3054
staff~
a staff of sleep~

OOZ OBJ  #3138 'potion opaque'  @ Northern Midgaard Main City (3000)

#3008   (3056)
bottle condition~
A bottle of condition(TM)~
A bottle of condition(TM) has been left here.~



OOZ OBJ  #1335 'potion speckled'  @ Arachnos (9000)
OOZ OBJ  #1462 'potion red'  @ The lands (1900)
OOZ OBJ  #6109 'potion blue'  @ Arachnos (9000)
OOZ OBJ  #6109 'potion blue'  @ The castle (200)

OOZ OBJ  #6108 'coins gold'  @ Graveyard (3400)
OOZ OBJ  #6108 'coins gold'  @ Redferne's Residence (7900)

OOZ OBJ  #6110 'potion yellow'  @ Graveyard (3400)
OOZ OBJ  #6115 'potion haste powerful'  @ Muncie (2300)



OOZ OBJ  #6116 'potion improve haste great powerful'  @ Northern Midgaard Main City (3000)
OOZ OBJ  #6117 'staff mana'  @ Redferne's Residence (7900)
OOZ OBJ  #6118 'wand vitalize'  @ The Wamphyri Aerie (4200)

---------2060
#3027
sword bombard~
a Bombard sword~
A Bombard sword has been left here.~



#3025
sword particle~
a particle of bombard sword~
A particle of bombard sword has been left here.~

#3026
sword samurai~
Samurai sword~
A Samurai sword has been left here.~





#3020
dagger~
a dagger~

#3021
sword small~
a small sword~


#3022
sword long~
a long sword~

#3023
club wooden~
a wooden club~

#3024
warhammer hammer~
a warhammer~


--- 2070 -----------------

#3040
breast plate~
a breast plate~

#3041
chain mail shirt~

#3042
shield~
a shield~

#3043
jacket soft leather~
a leather jacket~


#3044
jacket studded~
a studded leather jacket~

#3045
jacket scale mail~
a scale mail jacket~

#3046
bronze breast plate~
a bronze breast plate~



#3070
gauntlets bronze~
a pair of bronze gauntlets~

#3071
gloves leather~
a pair of leather gloves~

#3075
helmet bronze~
a bronze helmet~


#3076
cap leather~
a leather cap~

#3080
leggings bronze~
a pair of bronze leggings~

#3081
pants leather~
a pair of leather pants~

#3085
sleeves bronze~
a pair of bronze sleeves~

#3086
sleeves leather~
a pair of leather sleeves~


OOZ OBJ  #7922 'knee protecter leather'  @ Dirk's Castle (1500)
OOZ OBJ  #7922 'knee protecter leather'  @ Northern Midgaard Main City (3000)
OOZ OBJ  #7923 'knee protecter bronze'  @ Northern Midgaard Main City (3000)


-- 3101-------------------

OOZ OBJ  #7924 '청잠바 잠바 shirt shirts'  @ Northern Midgaard Main City (3000)
OOZ OBJ  #7925 '안경 퍼햅스 glass glasses'  @ Northern Midgaard Main City (3000)
OOZ OBJ  #7926 '바지 청바지 pants blue jeans'  @ Northern Midgaard Main City (3000)
OOZ OBJ  #7927 '잠바 청잠바 coat'  @ Northern Midgaard Main City (3000)
OOZ OBJ  #7928 'belt 벨트'  @ Northern Midgaard Main City (3000)
OOZ OBJ  #7929 '신발 shoes'  @ Northern Midgaard Main City (3000)






-- 3111-------------------


OOZ OBJ  #1326 'pendant diamond'  @ Southern part of Midgaard (3100)

OOZ OBJ  #1302 'Boots of Bangard'  @ Southern part of Midgaard (3100)

OOZ OBJ  #1622 'shotgun'  @ Southern part of Midgaard (3100)
OOZ OBJ  #1623 'shells ammo'  @ XXX Houses2 (1450)


OOZ OBJ  #1315 'broom'  @ Southern part of Midgaard (3100)



-- 3121----

OOZ OBJ  #1605 'brass shield brassshield'  @ Southern part of Midgaard (3100)

OOZ OBJ  #2222 'leggings'  @ Southern part of Midgaard (3100)
OOZ OBJ  #2223 'sash'  @ Southern part of Midgaard (3100)
OOZ OBJ  #2224 'fur hat'  @ Southern part of Midgaard (3100)
OOZ OBJ  #2244 'brass ring brassring'  @ Southern part of Midgaard (3100)

--2123

OOZ OBJ  #1320 'shades sunglasses'  @ Southern part of Midgaard (3100)

OOZ OBJ  #1330 'baseball cap'  @ HAON-DOR-LIGHT (6000)
OOZ OBJ  #1330 'baseball cap'  @ Southern part of Midgaard (3100)
OOZ OBJ  #1307 'hammer'  @ Northern Midgaard Main City (3000)



#3090
robe archmage~
Robe of the Archmage~

#3091
gildenwood staff magi~
Staff of the Magi~

OOZ OBJ  #9002 'hat wizard's'  @ The castle (200)





OOZ OBJ  #3000 'beer barrel'  @ The Dwarven Kingdom (6500)
OOZ OBJ  #3002 'ale bottle dark'  @ SHIRE (1700)
OOZ OBJ  #3003 'firebreather bottle'  @ The Dwarven Kingdom (6500)
OOZ OBJ  #3031 'lantern'  @ The Dwarven Kingdom (6500)
OOZ OBJ  #3032 'bag'  @ SHIRE (1700)

OOZ OBJ  #3025 'sword particle'  @ Southern part of Midgaard (3100)
OOZ OBJ  #3026 'sword samurai'  @ Southern part of Midgaard (3100)

OOZ OBJ  #3133 'city key'  @ Northern Midgaard Main City (3000)


OOZ OBJ  #3010 'bread'  @ Arena (9400)
OOZ OBJ  #3010 'bread'  @ The Dwarven Kingdom (6500)
OOZ OBJ  #3011 'danish pastry'  @ The Dwarven Kingdom (6500)
OOZ OBJ  #3020 'dagger'  @ Arena (9400)
OOZ OBJ  #3020 'dagger'  @ The City of Thalos (5200)
OOZ OBJ  #3021 'sword small'  @ Arena (9400)
OOZ OBJ  #3021 'sword small'  @ SHIRE (1700)
OOZ OBJ  #3022 'sword long'  @ KAIST (19000)
OOZ OBJ  #3022 'sword long'  @ SHIRE (1700)
OOZ OBJ  #3022 'sword long'  @ The Great Eastern Desert (5000)


OOZ OBJ  #3043 'jacket soft leather'  @ SHIRE (1700)
OOZ OBJ  #3046 'bronze breast plate'  @ SHIRE (1700)
OOZ OBJ  #3046 'bronze breast plate'  @ The castle (200)
OOZ OBJ  #3070 'gauntlets bronze'  @ The castle (200)
OOZ OBJ  #3070 'gauntlets bronze'  @ The tunnels (7400)
OOZ OBJ  #3075 'helmet bronze'  @ SHIRE (1700)
OOZ OBJ  #3075 'helmet bronze'  @ The castle (200)
OOZ OBJ  #3080 'leggings bronze'  @ SHIRE (1700)
OOZ OBJ  #3080 'leggings bronze'  @ The castle (200)
OOZ OBJ  #3080 'leggings bronze'  @ The tunnels (7400)
OOZ OBJ  #3085 'sleeves bronze'  @ The castle (200)
OOZ OBJ  #3085 'sleeves bronze'  @ The tunnels (7400)
OOZ OBJ  #3099 'board bulletin'  @ LIMBO (0)
OOZ OBJ  #3099 'board bulletin'  @ Village of Midgaard (1840)

==================================================

OOZ OBJ  #1303 'Protecter of Seraphim'  @ Graveyard (3400)
OOZ OBJ  #1305 'silver epee'  @ SHIRE (1700)
OOZ OBJ  #1305 'silver epee'  @ The tunnels (7400)
OOZ OBJ  #1306 'scythe'  @ HAON-DOR-LIGHT (6000)

OOZ OBJ  #1309 'plates ancient'  @ The Great Eastern Desert (5000)

OOZ OBJ  #1311 'totem'  @ The Wasteland (2100)
OOZ OBJ  #1316 'plates armored'  @ XXX Houses2 (1450)

OOZ OBJ  #1318 'banner bloody cross'  @ drow city (5100)
OOZ OBJ  #1319 'banner green crescent'  @ The Great Eastern Desert (5000)
City (3000)
OOZ OBJ  #1325 'stone mauve'  @ Mel's Doghouse (1300)

OOZ OBJ  #1327 'magnolias garland'  @ Houses (1400)

OOZ OBJ  #1331 'cape white'  @ Graveyard (3400)



OOZ OBJ  #13799 'key olympus'  @ The Wasteland (2100)
OOZ OBJ  #13799 'key olympus'  @ The lands (1900)



OOZ OBJ  #1620 'colt gun'  @ XXX Houses2 (1450)
OOZ OBJ  #1621 'bullets ammo'  @ XXX Houses2 (1450)


OOZ OBJ  #1704 'silvery cloak'  @ Dirk's Castle (1500)
OOZ OBJ  #1911 'belt leather leatherbelt'  @ SHIRE (1700)
OOZ OBJ  #2207 'necklace symbol holy'  @ The castle (200)

OOZ OBJ  #2298 'cloak winged'  @ Dragon Tower (2200)
OOZ OBJ  #2299 'key gold tarnished'  @ Dragon Tower (2200)
OOZ OBJ  #2400 'wooden Kal'  @ The Wasteland (2100)


OOZ OBJ  #7190 'ring silver silverring'  @ Redferne's Residence (7900)
OOZ OBJ  #7190 'ring silver silverring'  @ The tunnels (7400)
OOZ OBJ  #7191 'ring gold goldring'  @ Redferne's Residence (7900)
OOZ OBJ  #7191 'ring gold goldring'  @ SHIRE (1700)



OOZ OBJ  #3100 'tea cup'  @ SHIRE (1700)
OOZ OBJ  #3121 'key rusty'  @ Graveyard (3400)

OOZ OBJ  #3428 'flaming sword long longsword'  @ Graveyard (3400)
OOZ OBJ  #3430 'suit golden armor'  @ Graveyard (3400)
OOZ OBJ  #5107 'whip snake sixheaded'  @ The castle (200)
OOZ OBJ  #5999 'boots'  @ HAON-DOR-LIGHT (6000)
OOZ OBJ  #6004 'lantern'  @ SHIRE (1700)
OOZ OBJ  #6013 'water barrel'  @ Redferne's Residence (7900)


OOZ OBJ  #6111 'shield large'  @ Graveyard (3400)


OOZ OBJ  #6910 'shirt scale'  @ HAON-DOR-DARK (6100)
OOZ OBJ  #6911 'skirt scale'  @ HAON-DOR-DARK (6100)
OOZ OBJ  #6912 'cloak black hooded'  @ HAON-DOR-DARK (6100)
OOZ OBJ  #6913 'belt silver silverbelt'  @ HAON-DOR-DARK (6100)
OOZ OBJ  #6914 'sword long slender'  @ HAON-DOR-DARK (6100)



OOZ OBJ  #7192 'boots magic'  @ The tunnels (7400)
OOZ OBJ  #7203 'cloak purple'  @ The castle (200)


OOZ OBJ  #7211 'sword short'  @ Arena (9400)
OOZ OBJ  #7211 'sword short'  @ The Dwarven Kingdom (6500)
OOZ OBJ  #7211 'sword short'  @ The Great Eastern Desert (5000)

OOZ OBJ  #7215 'sabre light'  @ HAON-DOR-DARK (6100)

OOZ OBJ  #7207 'gauntlets muddy'  @ Redferne's Residence (7900)

OOZ OBJ  #7208 'stick small light'  @ Arena (9400)
OOZ OBJ  #7208 'stick small light'  @ Redferne's Residence (7900)

OOZ OBJ  #6999 'scythe'  @ Mel's Doghouse (1300)
OOZ OBJ  #7090 'waybread food'  @ Redferne's Residence (7900)
OOZ OBJ  #7212 'sword bastard'  @ Arena (9400)
OOZ OBJ  #7212 'sword bastard'  @ Redferne's Residence (7900)
OOZ OBJ  #7216 'sword two-handed'  @ Redferne's Residence (7900)
OOZ OBJ  #7217 'plate breast'  @ Redferne's Residence (7900)
OOZ OBJ  #7218 'plates leg'  @ Redferne's Residence (7900)
OOZ OBJ  #7219 'gloves'  @ Redferne's Residence (7900)
OOZ OBJ  #7220 'helmet'  @ Redferne's Residence (7900)
OOZ OBJ  #7221 'shield'  @ Redferne's Residence (7900)
OOZ OBJ  #7222 'boots'  @ Redferne's Residence (7900)
OOZ OBJ  #7223 'plates arm'  @ Redferne's Residence (7900)
OOZ OBJ  #7224 'ring innocence innocencering'  @ Redferne's Residence (7900)



OOZ OBJ  #7903 'new key'  @ The Great Eastern Desert (5000)

--------------- Checking Unused obj, mob, room:
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
UNREF: OBJ	#5050	cloak blue
UNREF: OBJ	#5051	rifle assualt
UNREF: OBJ	#5209	shield black dark
UNREF: OBJ	#6101	sign
UNREF: OBJ	#6515	magic boots
UNREF: OBJ	#6551	trapdoor
UNREF: OBJ	#7213	helmet large
UNREF: OBJ	#7214	helmet chaos
UNREF: OBJ	#7909	treasure coins
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
UNREF: OBJ	#15036	zizon dagger
UNREF: OBJ	#15806	yubi's BIG knee
UNREF: OBJ	#23308	magic paper 부적
UNREF: OBJ	#23309	seed evil power 악마 씨앗
UNREF: MOB	#2	zombie
UNREF: MOB	#3026	Chester Wilson
UNREF: MOB	#4104	orc
UNREF: MOB	#6003	rabbit
UNREF: MOB	#6250	bug
UNREF: MOB	#7007	wanderer
UNREF: MOB	#7008	homonculus homo
UNREF: MOB	#7012	esther madam
UNREF: MOB	#8010	cowboy
UNREF: MOB	#9520	rainbow sura
UNREF: MOB	#9521	month sura
UNREF: MOB	#11141	mirror son ogong
UNREF: MOB	#13601	wang
UNREF: MOB	#13602	cap
UNREF: MOB	#13603	Mac
UNREF: MOB	#13807	2nd keeper
UNREF: MOB	#13808	3rd keeper
UNREF: MOB	#13809	4th keeper
UNREF: MOB	#13810	5th keeper
UNREF: MOB	#13811	6th keeper
UNREF: MOB	#15836	secret guard
UNREF: MOB	#18020	guest
UNREF: MOB	#18021	servant
UNREF: MOB	#18022	cook
UNREF: MOB	#18023	jumo
UNREF: MOB	#18024	waiter
UNREF: MOB	#19999	super musashi
UNREF: ROOM	#0
]]

-- partially split  temporary

Zone_renum_tab2 = {
    --                                   ORIG  RENUM
    { "LIMBO",                              0,     0 },
    { "The castle",                       200,   200 },
    { "Mel's Doghouse",                  1300,  1300 },
    { "Houses",                          1400,  1400 },
    { "Dirk's Castle",                   1500,  1500 },
    { "SHIRE",                           1700,  1700 },
    { "Village of Midgaard",             1840,  1800 },
    { "The lands",                       1900,  1900 },
    { "The Wasteland",                   2100,  2100 },
    { "Dragon Tower",                    2200,  2200 },
    { "Muncie",                          2300,  2300 },
    { "The Corporation",                 2500,  2500 },
    { "The Never Land",                  2700,  2700 },
    { "The Keep of Mahn-Tor",            2900,  2900 },
    { "Northern Midgaard Main City",     3000,  3000 },
    { "Southern part of Midgaard",       3100,  3100 },
    { "River of Midgaard",               3200,  3200 },
    { "Graveyard",                       3400,  3400 },
    { "XXX Turning Point",               3500,  3500 },
    { "Moria level 1-2",                 4000,  4000 },
    { "Moria level 3-4",                 4100,  4100 },
    { "The Wamphyri Aerie",              4200,  4200 },
    { "The Great Eastern Desert",        5000,  5000 },
    { "drow city",                       5100,  5100 },
    { "The City of Thalos",              5200,  5200 },
    { "HAON-DOR-LIGHT",                  6000,  6000 },
    { "HAON-DOR-DARK",                   6100,  6100 },
    { "The Dwarven Kingdom",             6500,  6500 },
    { "XXX Dwarven Jail",                6990,  6990 },
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
    { "Utility Zone",                   13300, 13300 },
    { "Easy zone",                      13400, 13400 },
    { "Mount Olympus",                  13700, 13700 },
    { "Process' castle",                13800, 13800 },
    { "Robot city",                     15000, 15000 },
    { "Kingdom Of Chok",                15800, 15800 },
    { "Kingdom of Wee",                 16000, 16000 },
    { "o-kingdom",                      17000, 17000 },
    { "MooDang",                        18000, 18000 },
    { "KAIST",                          19000, 19000 },
    { "Good-Bad-Island",                23300, 23300 },

}


-- New Narai (partally renumbered)

Zone_renum_tab3 = {
    --                                   ORIG  RENUM
    { "LIMBO",                              0,     0 },
    { "The castle",                       200,   200 },
    { "Mel's Doghouse",                  1300,  1300 },
    { "Houses",                          1400,  1400 },
    { "Dirk's Castle",                   1500,  1500 },
    { "SHIRE",                           1700,  1700 },
    { "Village of Midgaard",             1800,  1800 },
    { "The lands",                       1900,  1900 },
    { "The Wasteland",                   2100,  2100 },
    { "Dragon Tower",                    2200,  2200 },
    { "Muncie",                          2300,  2300 },
    { "The Corporation",                 2500,  2500 },
    { "The Never Land",                  2700,  2700 },
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
    { "Utility Zone",                   13300, 13300 },
    { "Easy zone",                      13400, 13400 },
    { "Mount Olympus",                  13700, 13700 },
    { "Process' castle",                13800, 13800 },
    { "Robot city",                     15000, 15000 },
    { "Kingdom Of Chok",                15800, 15800 },
    { "Kingdom of Wee",                 16000, 16000 },
    { "o-kingdom",                      17000, 17000 },
    { "MooDang",                        18000, 18000 },
    { "KAIST",                          19000, 19000 },
    { "Good-Bad-Island",                23300, 23300 },

}

--[[

-- Legacy Narai
Zone_renum_tab4 = {
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

]]

main()
