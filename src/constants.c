#include "structs.h"
#include "limits.h"

char *spell_wear_off_msg[] = {
    "RESERVED DB.C",
    "You feel less protected.", /* 1 */
    "!Teleport!",
    "You feel less righteous.",
    "You feel a cloak of blindness dissolve.",
    "!Burning Hands!",
    "!Call Lightning",
    "You feel more self-confident.",
    "!Chill Touch!",
    "!Reanimate!",
    "!Color Spray!",
    "!Relocate!",  /*11*/
    "!Create Food!",
    "!Create Water!",
    "!Cure Blind!",
    "!Cure Critic!",
    "!Cure Light!",
    "You feel better.",
    "You sense the red in your vision disappear.",
    "The detect invisible wears off.",
    "The detect magic wears off.",
    "!PREACH!",  /* 21 */
    "!Dispel Evil!",
    "!Earthquake!",
    "!Enchant Weapon!",
    "!Energy Drain!",
    "!Fireball!",
    "!Harm!",
    "!Heal",
    "You feel yourself exposed.",
    "!Lightning Bolt!",
    "!Locate object!",  /* 31 */
    "!Magic Missile!",
    "You feel less sick.",
    "You feel less protected.",
    "!Remove Curse!",
    "The white aura around your body fades.",
    "!Shocking Grasp!",
    "You feel les tired.",
    "You feel weaker.",
    "!Summon!",
    "!Ventriloquate!",  /* 41 */
    "!Word of Recall!",
    "!Remove Poison!",
    "You feel less aware of your surroundings.",
    "!Sunburst!",
    "!UNUSED!",
    "!UNUSED!",
    "!UNUSED!",
    "!UNUSED!",
    "!UNUSED!",
    "!UNUSED!",    /* 51 */
    "!UNUSED!",
    "!Identify!",
    "!UNUSED!",
    "",  /* NO MESSAGE FOR SNEAK*/
    "!Hide!",
    "!Steal!",
    "!Backstab!",
    "!Pick Lock!",
    "!Kick!",
    "!Bash!",    /* 61 */
    "!Rescue!",
    "!UNUSED!",
    "!UNUSED!",
    "!UNUSED!",
    "!UNUSED!",
    "!UNUSED!",
    "!UNUSED!",
    "!UNUSED!",
    "!UNUSED!",
    "You feel less brave.",  /* 71 */
    "!energy flow!",
    "!mana boost!",
    "!vitalize!",
    "!full fire!",
    "!throw!",
    "!fire storm!",
    "You feel less shirink.",
    "!full heal!",
    "!trick!",
    "You feel things return to normal.",  /* 81 */
    "You feel things return to normal.", 
    "!far look!",
    "!all heal!",
    "!tornado!",
    "!light move!",
    "!parry!",
    "!flash!",
    "!multi kick!",
    "!UNUSED!",
    "!UNUSED!",         /* 91 */
    "Your eyes are less sensitive now.",		/* 92 ; infra */
    "!UNUSED!",
    "!UNUSED!",
    "!UNUSED!",
    "!UNUSED!",
    "!UNUSED!",
    "Your illusion color fades.",	/* 98; mirror image */
    "!UNUSED!",
    "!UNUSED!",
    "!UNUSED!",						/* 101 */
    "!UNUSED!",
    "!UNUSED!",
	"You feel less freezing",
    "!UNUSED!",
    "\n"
};


int rev_dir[] = 
{
    2, 3, 0, 1, 5, 4
}; 

/* If you fix movement_loss, you must change db.c sector check */
/* in db.c boot_world() , there is limit checking.. */
int movement_loss[]=
{
    1,  /* Inside     */
    2,  /* City       */
    2,  /* Field      */
    3,  /* Forest     */
    4,  /* Hills      */
    6,  /* Mountains  */
    4,  /* Swimming   */
    1,  /* Unswimable */
    1   /* Sky */
};

char *dirs[] = 
{
    "north",
    "east",
    "south",
    "west",
    "up",
    "down",
    "\n"
};

char *weekdays[7] = { 
    "the Day of the Moon",
    "the Day of the Bull",
    "the Day of the Deception",
    "the Day of Thunder",
    "the Day of Freedom",
    "the day of the Great Gods",
    "the Day of the Sun" };

char *month_name[17] = {
    "Month of Winter",           /* 0 */
    "Month of the Winter Wolf",
    "Month of the Frost Giant",
    "Month of the Old Forces",
    "Month of the Grand Struggle",
    "Month of the Spring",
    "Month of Nature",
    "Month of Futility",
    "Month of the Dragon",
    "Month of the Sun",
    "Month of the Heat",
    "Month of the Battle",
    "Month of the Dark Shades",
    "Month of the Shadows",
    "Month of the Long Shadows",
    "Month of the Ancient Darkness",
    "Month of the Great Evil"
};

int sharp[] = {
    0,
    0,
    0,
    1,    /* Slashing */
    0,
    0,
    0,
    0,    /* Bludgeon */
    0,
    0,
    0,
    0 };  /* Pierce   */

char *where[] = {
    "<used as light>      ",
    "<worn on finger>     ",
    "<worn on finger>     ",
    "<worn around neck>   ",
    "<worn around neck>   ",
    "<worn on body>       ",
    "<worn on head>       ",
    "<worn on legs>       ",
    "<worn on feet>       ",
    "<worn on hands>      ",
    "<worn on arms>       ",
    "<worn as shield>     ",
    "<worn about body>    ",
    "<worn about waist>   ",
    "<worn around wrist>  ",
    "<worn around wrist>  ",
    "<wielded>            ",
    "<held>               ",
    "<worn around knee>   ",
    "<worn around knee>   ", 
    "<worn about legs>    ",
}; 

char *drinks[]=
{
    "water",
    "beer",
    "wine",
    "ale",
    "dark ale",
    "whisky",
    "lemonade",
    "firebreather",
    "local speciality",
    "slime mold juice",
    "milk",
    "tea",
    "coffee",
    "blood",
    "salt water",
    "coke",
    "nectar",
    "golden nectar",
    "makoli",
    "sundew"
};

char *drinknames[]=
{
    "water",
    "beer",
    "wine",
    "ale",
    "ale",
    "whisky",
    "lemonade",
    "firebreather",
    "local",
    "juice",
    "milk",
    "tea",
    "coffee",
    "blood",
    "salt",
    "coke",
    "nectar",
    "golden nectar",
    "makoli",
    "sundew"
};

/* effect of drinks on drunk, hunger, thirst-- see values.doc */
int drink_aff[][3] = {
    { 0,1,10 },  /* Water    */
    { 2,2,5 },   /* beer     */
    { 1,3,4 },   /* wine     */
    { 3,2,5 },   /* ale      */
    { 3,2,5 },   /* ale      */
    { 6,1,4 },   /* Whiskey  */
    { 0,1,8 },   /* lemonade */
    { 10,0,0 },  /* firebr   */
    { 3,3,3 },   /* local    */
    { 0,4,-8 },  /* juice    */
    { 0,3,6 },   /* milk     */
    { 0,1,6 },   /* tea      */
    { 0,1,6 },   /* coffee   */
    { 0,2,-1 },  /* blood    */
    { 0,1,-2 },  /* salt     */
    { 0,3,8 },   /* coke     */
    { 5,5,5 },   /* nectar   */
    { 8,8,8 },   /* golden nectar */
    { 5,1,2 },   /* makoli 막걸리 */
    { 0,9,20 },   /* sundew   */
};

char *color_liquid[]=
{
    "clear",
    "brown",
    "clear",
    "brown",
    "dark",
    "golden",
    "red",
    "green",
    "clear",
    "light green",
    "white",
    "brown",
    "black",
    "red",
    "clear",
    "clear brown",	/* coke ?? */
    "yellow",
    "golden",
    "white",
    "milky"
};

char *fullness[] =
{
    "less than half ",
    "about half ",
    "more than half ",
    ""
};

struct title_type titles[4][IMO+4] = {
    { {"the Man","the Woman",0},
      {"the Apprentice of Magic","the Apprentice of Magic",1}, /* 1 */
      {"the Spell Student","the Spell Student",2250},
      {"the Scholar of Magic","the Scholar of Magic",4500},
      {"the Delver in Spells","the Delveress in Spells",9000},
      {"the Medium of Magic","the Medium of Magic",18000},
      {"the Scribe of Magic","the Scribess of Magic",36000},
      {"the Seer","the Seeress",60000},
      {"the Sage","the Sage",90000},
      {"the Illusionist","the Illusionist",135000},
      {"the Abjurer","the Abjuress",250000}, /* 10 */
      {"the Invoker","the Invoker",450000},
      {"the Enchanter","the Enchantress",650000},
      {"the Conjurer","the Conjuress",1000000},
      {"the Magician","the Witch",1300000},
      {"the Creator","the Creator",1600000},
      {"the Savant","the Savant",2000000},
      {"the Magus","the Craftess",2500000},
      {"the Wizard","the Wizard",3000000},
      {"the Warlock","the War Witch",3500000},
      {"the Sorcerer","the Sorceress",4000000}, /* 20 */
      {"the Man of Hecate","the Lady of Hecate",  5000000},
      {"the Apparantice Archmage","the Apparantice Archmage",  7000000},
      {"the Archmage of Fire","the Archmage of Fire", 10000000},
      {"the Archmage of Water","the Archmage of Water", 15000000},
      {"the Archmage of Earth","the Archmage of Earth", 22000000},
      {"the Archmage of Wind","the Archmage of Wind", 35000000},
      {"the Mage Guru","the Mage Guru", 50000000},
      {"the Apparantice MageLord","the Apparantice MageLord", 70000000},
      {"the MageLord of Fire","the MageLord of Fire",85000000},
      {"the MageLord of Water","the MageLord of Water",100000000}, /* 30 */
      {"the MageLord of Earth","the MageLord of Earth",125000000}, 
      {"the MageLord of Wind","the MageLord of Wind",150000000},
      {"the MageLord of Moon","the MageLord of Moon",175000000},
      {"the MageLord of Sun","the MageLord of Sun",200000000},
      {"the MageLord of Galaxy","the MageLord of Galaxy",225000000},
      {"the MageLord of Universe","the MageLord of Universe",250000000},
      {"the MageLord of Metaphysics","the MageLord of Metaphysics",275000000},
      {"the Metaphysician","the Metaphysician",300000000},
      {"the Master of MageLord","the Master of MageLord",325000000},
      {"the Dangun","the Dangun",350000000}, /* 40 */
      {"the Immortal Warlock","the Immortal Enchantress",400000000},
      {"the Avatar of Magic","the Empress of Magic",500000000},
      {"the God of Magic","the Goddess of Magic",600000000},
      {"the Implementator","the Implementress",700000000} },

    { {"the Man","the Woman",0},
      {"the Believer","the Believer",1}, /* 1 */
      {"the Attendant","the Attendant",1500},
      {"the Acolyte","the Acolyte",3000},
      {"the Novice","the Novice",6000},
      {"the Missionary","the Missionary",13000},
      {"the Adept","the Adept",27500},
      {"the Deacon","the Deaconess",55000},
      {"the Vicar","the Vicaress",110000},
      {"the Priest","the Priestess",225000},
      {"the Minister","the Lady Minister",450000}, /* 10 */
      {"the Canon","the Canon",675000},
      {"the Levite","the Levitess",900000},
      {"the Curate","the Curess",1125000},
      {"the Monk","the Nunne",1350000},
      {"the Healer","the Healess",1575000},
      {"the Chaplain","the Chaplain",1800000},
      {"the Expositor","the Expositress",2025000},
      {"the Bishop","the Bishop",2250000},
      {"the Arch Bishop","the Arch Lady of the Church",2500000},
      {"the Patriarch","the Matriarch",3000000}, /* 20 */
      {"the Lesser Doctor","the Lesser Doctor",  4000000},
      {"the Doctor","the Doctor",  6000000},
      {"the Master Doctor","the Master Doctor", 8500000},
      {"the Doctor of GOD","the Doctor of GOD", 12000000},
      {"the Cleric","the Cleric", 16000000},
      {"the Cleric for people","the Cleric for people", 22000000},
      {"the Chief Doctor","the Chief Doctor", 33000000},
      {"the Great Doctor","the Great Doctor", 45000000},
      {"the Doctor in Wind","the Doctor in Wind", 60000000},
      {"the Doctor in Fire","the Doctor in Fire", 75000000},        /* 30 */
      {"the Doctor of earth","the Doctor of earth", 90000000},
      {"the Doctor of Universe","the Doctor of Universe",105000000},
      {"the Famous Doctor","the Famous Doctor",120000000},
      {"the Prayer","the Prayer",140000000},
      {"the Healer","the Healer",160000000},
      {"the Healer of Lords","the Healer of Lords",180000000},
      {"the Healer of God","the Healer of God",200000000},
      {"the Great Prayer","the Great Prayer",220000000},
      {"the Prayer for Dangun","the Prayer for Dangun",240000000},
      {"the Dangun","the Dangun",280000000},        /* 40 */
      {"the Immortal Cardinal","the Immortal Priestess",300000000},
      {"the Inquisitor","the Inquisitress",500000000},
      {"the God of good and evil","the Goddess of good and evil",600000000},
      {"the Implementator","the Implementress",700000000} },

    { {"the Man","the Woman",0},
      {"the Pilferer","the Pilferess",1}, /* 1 */
      {"the Footpad","the Footpad",1400},
      {"the Filcher","the Filcheress",2800},
      {"the Pick-Pocket","the Pick-Pocket",5600},
      {"the Sneak","the Sneak",11000},
      {"the Pincher","the Pincheress",22000},
      {"the Cut-Purse","the Cut-Purse",40000},
      {"the Snatcher","the Snatcheress",80000},
      {"the Sharper","the Sharpress",140000},
      {"the Rogue","the Rogue",180000}, /* 10 */
      {"the Robber","the Robber",280000},
      {"the Magsman","the Magswoman",500000},
      {"the Highwayman","the Highwaywoman",660000},
      {"the Burglar","the Burglaress",920000},
      {"the Thief","the Thief",1180000},
      {"the Knifer","the Knifer",1420000},
      {"the Quick-Blade","the Quick-Blade",1660000},
      {"the Killer","the Murderess",1900000},
      {"the Brigand","the Brigand",2300000},
      {"the Cut-Throat","the Cut-Throat",3000000}, /* 20 */
      {"the Urchin Father","the Urchin Mother",  4000000},
      {"the Bully","the Bully",  6000000},
      {"the Snifer","the Snifer",8500000},
      {"the Famous Killer","the Famous Killer",12000000},
      {"the Best Snifer","the Best Snifer",16000000},
      {"the Quick Knife","the Quick Knife",20000000},
      {"the Stealer","the Stealer",30000000},
      {"the Stealer of Breath","the Stealer of Breath",45000000},
      {"the Good Thief","the Good Thief",66000000},
      {"the Lord of Thieves","the Lady of Thieves",90000000},       /* 30 */
      {"the King of Thieves","the Queen of Thieves",115000000},
      {"the Urchin in World","the Urchin in World",140000000},
      {"the Shadow","the Shadow", 165000000},
      {"the Thief of Fire","the Thief of Fire", 190000000},
      {"the Thief of Wind","the Thief of Wind", 215000000},
      {"the Thief of Earth","the Thief of Earth", 240000000},
      {"the Thief of Universe","the Thief of Universe", 270000000},
      {"the God-Father","the God-Mother",300000000},
      {"the Greatest Bully","the Greatest Bully",330000000},
      {"the Dangun","the Dangun",360000000},	/* 40 */
      {"the Immortal Assassin","the Immortal Assassin",400000000},
      {"the Demi God of Thieves","the Demi Goddess of Thieves",500000000},
      {"the God of Thieves","the Goddess of Thieves",600000000},
      {"the Implementator","the Implementress",700000000} },

    { {"the Man","the Woman",0},
      {"the Swordpupil","the Swordpupil",1}, /* 1 */
      {"the Recruit","the Recruit",2000},
      {"the Sentry","the Sentress",5000},
      {"the Fighter","the Fighter",10000},
      {"the Soldier","the Soldier",20000},
      {"the Warrior","the Warrior",40000},
      {"the Veteran","the Veteran",75000},
      {"the Swordsman","the Swordswoman",150000},
      {"the Fencer","the Fenceress",300000},
      {"the Combatant","the Combatess",650000}, /* 10 */
      {"the Hero","the Heroine",1000000},
      {"the Myrmidon","the Myrmidon",1400000},
      {"the Swashbuckler","the Swashbuckleress",1800000},
      {"the Mercenary","the Mercenaress",2200000},
      {"the Swordmaster","the Swordmistress",2600000},
      {"the Lieutenant","the Lieutenant",3000000},
      {"the Champion","the Lady Champion",3500000},
      {"the Dragoon","the Lady Dragoon",4000000},
      {"the Cavalier","the Cavalier",4500000},
      {"the Knight","the Lady Knight",5000000}, /* 20 */
      {"the Knight of Fire","the Knight of Fire",  6500000},
      {"the Knight of Wind","the Knight of Wind",  8500000},
      {"the Knight of Ice","the Knight of Ice", 12000000},
      {"the Knight of Water","the Knight of Water", 17000000},
      {"the Powerful Knight","the Powerful Knight", 24000000},
      {"the Lord of Wind","the Lady of Wind", 34000000},
      {"the Lord of Fire","the Lady of Fire", 50000000},
      {"the Lord of Water","the Lady of Water", 70000000},
      {"the Lord of Earth","the Lady of Earth", 95000000},
      {"the Bodyguard","the Bodyguard",120000000},  /* 30 */
      {"the Famous knight","the Famous Knight",150000000},
      {"the Best Knight","the Best Knight",180000000},
      {"the Lord of Knight","The Lady of Knight",210000000},
      {"The King of Knight","the Queen of Knight",240000000},
      {"the Adventurer","the Adventurer",270000000},
      {"the Best Kid","the Best lady",300000000},
      {"the Paladin","the Paladin",330000000},
      {"the Best Swordman","the Best Swordwoman",360000000},
      {"the Adventurer for Dangun","the Adventurer for Dangun",400000000},
      {"the Dangun","the Dangun",430000000},	/* 40 */
      {"the Immortal Warlord","the Immortal Lady of War",500000000},
      {"the Extirpator","the Queen of Destruction",600000000},
      {"the God of War","the Goddess of War",700000000},
      {"the Implementator","the Implementress",800000000} }
};

char *item_types[] = {
    "UNDEFINED",
    "LIGHT",	/* 1 */
    "SCROLL",
    "WAND",
    "STAFF",
    "WEAPON",
    "FIRE WEAPON",
    "MISSILE",
    "TREASURE",
    "ARMOR",
    "POTION",	/* 10 */
    "WORN",
    "OTHER",
    "TRASH",
    "TRAP",
    "CONTAINER",
    "NOTE",
    "LIQUID CONTAINER",
    "KEY",
    "FOOD",
    "MONEY",	/* 20 */
    "PEN",
    "BOAT",
    "BOMB",
    "BROKEN",
    "\n"
};

char *wear_bits[] = {
    "TAKE",
    "FINGER",
    "NECK",
    "BODY",
    "HEAD",
    "LEGS",
    "FEET",
    "HANDS",
    "ARMS",
    "SHIELD",
    "ABOUT",
    "WAISTE",
    "WRIST",
    "WIELD",
    "HOLD",
    "THROW",
    "KNEE",
    "ABOUTLEGS",
    "LIGHT-SOURCE",
    "\n"
};

char *extra_bits[] = {
    "GLOW",
    "HUM",
    "DARK",
    "LOCK",
    "EVIL",
    "INVISIBLE",
    "MAGIC",
    "NODROP",
    "BLESS",
    "ANTI-GOOD",
    "ANTI-EVIL",
    "ANTI-NEUTRAL",
    "NOLOAD",
    "ANTI-MAGE",
    "ANTI-CLERIC",
    "ANTI-THIEF",
    "ANTI-WARRIOR",
    "NORENT",
    "\n"
};

char *room_bits[] = {
    "DARK",
    "NOSUMMON",
    "NO_MOB",
    "INDOORS",
    "NO_RECALL",
    "NEUTRAL",
    "CHAOTIC",
    "NO_MAGIC",
    "TUNNEL",
    "PRIVATE",
    "OFF_LIMITS",
    "RENT",
    "NORELOCATE",
    "EVERYZONE",
    "DEATH_PLACE",
    "\n"
};

char *exit_bits[] = {
    "IS-DOOR",
    "CLOSED",
    "LOCKED",
    "KK",
    "LL",
    "PICKPROOF",
    "NOPHASE",
    "\n"
};

/*  If you add in sector_types, please add in movement_loss[]  */
/*  in this file, and fix boot_world() in db.c  - find 'sector'  */
char *sector_types[] = {
    "Inside",
    "City",
    "Field",
    "Forest",
    "Hills",
    "Mountains",
    "Water Swim",
    "Water NoSwim",
    "Sky",
    "\n"
};

char *equipment_types[] = {
    "Special",
    "Worn on right finger",
    "Worn on left finger",
    "First worn around Neck",
    "Second worn around Neck",
    "Worn on body",
    "Worn on head",
    "Worn on legs",
    "Worn about legs",
    "Worn on right knee",
    "Worn on left knee",
    "Worn on feet",
    "Worn on hands",
    "Worn on arms",
    "Worn as shield",
    "Worn about body",
    "Worn around waiste",
    "Worn around right wrist",
    "Worn around left wrist",
    "Wielded",
    "Held",
    "\n"
};
  
char *affected_bits[] = 
{  "BLIND",
   "INVISIBLE",
   "DETECT-ALIGN",
   "DETECT-INVISIBLE",
   "INFRAVISION",
   "SENSE-LIFE",
   "HOLD",
   "SANCTUARY",
   "GROUP",
   "UNUSED",
   "CURSE",
   "MIRROR-IMAGE",
   "POISON",
   "PROTECT-EVIL",
   "PARALYSIS",
   "MORDENS-SWORD",
   "FLAMING-SWORD",
   "SLEEP",
   "UNUSED",
   "SNEAK",
   "HIDE",
   "FEAR",
   "CHARM",
   "FOLLOW",
   "HASTE",
   "IMPROVED_HASTE",
   "NO_SUMMON",	/* for NPC */
   "NO_CHARM", /* for NPC */
   "BERSERK",
   "\n"
};

char *apply_types[] = {
    "NONE",
    "STR",
    "DEX",
    "INT",
    "WIS",
    "CON",
    "SEX",
    "CLASS",
    "LEVEL",
    "AGE",
    "CHAR_WEIGHT",
    "CHAR_HEIGHT",
    "MANA",
    "HIT",
    "MOVE",
    "GOLD",
    "EXP",
    "ARMOR",
    "HITROLL",
    "DAMROLL",
    "SAVING_PARA",
    "SAVING_ROD",
    "SAVING_PETRI",
    "SAVING_BREATH",
    "SAVING_SPELL",
    "REGENERATION",
    "INVISIBLE",
    "\n"
};

char *pc_class_types[] = {
    "UNDEFINED",
    "Magic User",
    "Cleric",
    "Thief",
    "Warrior",
    "\n"
};

char *npc_class_types[] = {
    "Normal",
    "Undead",
    "\n"
};

char *action_bits[] = {
    "SPEC",
    "SENTINEL",
    "SCAVENGER",
    "ISNPC",
    "NICE-THIEF",
    "AGGRESSIVE",
    "STAY-ZONE",
    "WIMPY",
    "FIGHTER",
    "MAGE",
    "CLERIC",
    "THIEF",
    "PALADIN",
    "DRAGON",
    "SPITTER",
    "SHOOTER",
    "GUARD",
    "SUPERGUARD",
    "GIANT",
    "HELPER",
    "RESCUER",
    "SPELLBLOCKER",
    "\n"
};


char *player_bits[] = {
    "BRIEF",
    "NOSHOUT",
    "COMPACT",
    "DONTSET",
    "NOTELL",
    "BANISHED",
    "CRIMINAL",
    "WIZINVIS",
    "EARMUFFS",
    "XYZZY",
    "AGGR",
    "KOREAN",
    "NOCHAT",
    "WIMPY",
    "AUTOEXIT",
    "\n"
};


char *position_types[] = {
    "Dead",
    "Mortally wounded",
    "Incapacitated",
    "Stunned",
    "Sleeping",
    "Resting",
    "Sitting",
    "Fighting",
    "Standing",
    "\n"
};

char *connected_types[]  =  {
    "Playing",
    "Get name",
    "Confirm name",
    "Read Password",
    "Get new password",
    "Confirm new password",
    "Get sex",
    "Read messages of today",
    "Read Menu",
    "Get extra description",
    "Get class",
    "\n"
};

/* [class], [level] (all) M, C, T, W*/
/* cyb const int thaco[4][IMO+4] = { */
int thaco[4][IMO+4] = {
    { 100,20,20,20,19,19,19,18,18,18,17,17,17,16,16,16, /* 0-15 */
      15,15,15,14,14,14,13,13,13,13,13,12,12,12,12,11, /* 16-31 */
      11,11,10,10,10, 9, 9, 9, 8, 8, 8, 8, 8},	       /* 32-44 */
    { 100,20,20,20,19,19,18,18,17,17,16,16,15,15,15,14,
      14,14,13,13,12,12,11,11,11,10,10, 9, 9, 8, 8, 7,
      7, 6, 6, 5, 5, 5, 4, 4, 4, 4, 3, 3, 3},
    { 100,20,20,19,19,18,18,17,17,16,16,15,15,14,14,13,
      13,13,12,12,12,11,11,11,11,10,10,10,10, 9, 9, 9,
      8, 8, 7, 7, 6, 6, 5, 5, 4, 4, 3, 3, 2},
    { 100,20,20,19,19,18,17,16,15,14,13,12,12,11,10, 9,
      8, 7, 7, 6, 6, 6, 5, 5, 5, 5, 4, 4, 4, 4, 3, 3,
      3, 3, 3, 2, 2, 2, 2, 1, 1, 1, 1, 1, 0}
};

/* [ch] strength apply (all) */
/* struct = { short tohit, todam, carry_w, wield_w } */
struct str_app_type str_app[31] = {
    { -5,-4,   0,  0 },  /* 0  */
    { -5,-4,   3,  1 },  /* 1  */
    { -3,-2,   3,  2 },
    { -3,-1,  10,  3 },  /* 3  */
    { -2,-1,  25,  4 },
    { -2,-1,  55,  5 },  /* 5  */
    { -1, 0,  80,  6 },
    { -1, 0,  90,  7 },
    {  0, 0, 100,  8 },
    {  0, 0, 100,  9 },
    {  0, 0, 115, 10 }, /* 10  */
    {  0, 0, 115, 11 },
    {  0, 0, 130, 12 },
    {  0, 0, 140, 13 },
    {  0, 0, 160, 14 },
    {  0, 0, 170, 15 }, /* 15  */
    {  0, 0, 195, 16 },
    {  1, 1, 220, 18 },
    {  1, 1, 255, 20 }, /* 18  */
    {  3, 7, 640, 40 },
    {  3, 8, 700, 40 }, /* 20  */
    {  4, 9, 810, 40 },
    {  4,10, 970, 40 },
    {  5,11,1130, 40 },
    {  6,12,1440, 40 },
    {  7,14,1750, 40 }, /* 25            */
    {  1, 2, 280, 22 }, /* 18/01-50      */
    {  1, 3, 305, 24 }, /* 18/51-75      */
    {  2, 4, 330, 26 }, /* 18/76-90      */
    {  2, 4, 380, 28 }, /* 18/91-99      */
    {  3, 5, 480, 30 }, /* 18/100   (30) */
};

/* [dex] skill apply (thieves only) */
/* struct = {short p_pocket, p_locks, traps, sneak, hide } */
struct dex_skill_type dex_app_skill[26] = {
    {-99,-99,-90,-99,-60},   /* 0 */
    {-90,-90,-60,-90,-50},   /* 1 */
    {-80,-80,-40,-80,-45},
    {-70,-70,-30,-70,-40},
    {-60,-60,-30,-60,-35},
    {-50,-50,-20,-50,-30},   /* 5 */
    {-40,-40,-20,-40,-25},
    {-30,-30,-15,-30,-20},
    {-20,-20,-15,-20,-15},
    {-15,-10,-10,-20,-10},
    {-10, -5,-10,-15, -5},   /* 10 */
    { -5,  0, -5,-10,  0},
    {  0,  0,  0, -5,  0},
    {  0,  0,  0,  0,  0},
    {  0,  0,  0,  0,  0},
    {  0,  0,  0,  0,  0},   /* 15 */
    {  0,  5,  0,  0,  0},
    {  5, 10,  0,  5,  5},
    { 10, 15,  5, 10, 10},
    { 15, 20, 10, 15, 15},
    { 15, 20, 10, 15, 15},   /* 20 */
    { 20, 25, 10, 15, 20},
    { 20, 25, 15, 20, 20},
    { 25, 25, 15, 20, 20},
    { 25, 30, 15, 25, 25},
    { 25, 30, 15, 25, 25}
};

/* [level] backstab multiplyer (thieves only) */
byte backstab_mult[IMO+4] = {
    1,   /* 0 */
    2,   /* 1 */
    2,
    2,
    3,
    3,   /* 5 */
    3,
    3,
    3,
    4,
    4,   /* 10 */
    4,
    4,
    4,
    4,
    5,   /* 15 */
    5,
    5,
    5,
    5,
    5,   /* 20 */
    5,
    5,
    5,
    5,   /* 25 */
    5,
    5,
    6,
    6,
    6,  /* 30 */
    6, 6, 6, 7, 7, /* 35 */
    7, 7, 7, 7, 8, /* 40 */
    8, 8, 9,20,45	 /* 45 */
};

/* [dex] apply (all) */
/* struct = { short reaction, miss_att, defensive } */
struct dex_app_type dex_app[26] = {
    {-7,-7, 6},   /* 0 */
    {-6,-6, 5},   /* 1 */
    {-4,-4, 5},
    {-3,-3, 4},
    {-2,-2, 3},
    {-1,-1, 2},   /* 5 */
    { 0, 0, 1},
    { 0, 0, 0},
    { 0, 0, 0},
    { 0, 0, 0},
    { 0, 0, 0},   /* 10 */
    { 0, 0, 0},
    { 0, 0, 0},
    { 0, 0, 0},
    { 0, 0, 0},
    { 0, 0,-1},   /* 15 */
    { 1, 1,-2},
    { 2, 2,-3},
    { 2, 2,-4},
    { 3, 3,-4},
    { 3, 3,-4},   /* 20 */
    { 4, 4,-5},
    { 4, 4,-5},
    { 4, 4,-5},
    { 5, 5,-6},
    { 5, 5,-6}
};

/* [con] apply (all) */
/* struct = { short hitp, shock }  */
struct con_app_type con_app[26] = {
    {-4,20},   /* 0 */
    {-3,25},   /* 1 */
    {-2,30},
    {-2,35},
    {-1,40},
    {-1,45},   /* 5 */
    {-1,50},
    { 0,55},
    { 1,65},
    { 2,70},
    { 3,75},   /* 10 */
    { 4,78},
    { 5,80},
    { 6,85},
    { 7,88},
    { 8,90},   /* 15 */
    { 9,95},
    { 10,97},
    { 10,99},
    { 10,99},
    { 10,99},   /* 20 */
    { 10,99},
    { 10,99},
    { 10,99},
    { 10,99},
    { 10,100}  /* 25 */
};

/* [int] apply (all) */
struct int_app_type int_app[26] = {
    1,
    2,  3,  4,  7,  8,  9, 10, 20, 28, 30, /* 10 */
    35, 38, 40, 41, 42, 46, 50, 54, 99, 99, /* 20 */
    99, 99, 99, 99, 99
};

/* [wis] apply (all) struct = { byte bonus } */
struct wis_app_type wis_app[26] = {
    0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 2, 2, 3, 3, 3, 4, 5, 6, 6,
    6, 6, 6, 6, 6
};
