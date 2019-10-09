#include <stdio.h>
#include "char.h"
#include "global.h"
#include "play.h"

char *perhaps_words[] = {
	"나래 머드에 오신 걸 환영합니다.",
	"나래 머드에 오신 걸 환영합니다.",
	"나래 머드에 와 주셔서 감사합니다.",
	"초보자이시면, 'help'를 쳐보세요.",
	"reimburse는 해줄 사람도 없대요...",
	"reimburse는 안해준데요...",
	"나래 머드에 관한 의견과 제안이 있으면 board에 써주세요.",
	"버그가 발견되면 보드에 적어주세요. 사례를 드립니다.",
	"NEWS를 자주 보세요... type 'show news'.",
	"새로운 소식은 NEWS에 난대요. type 'show news'.",
	"지나간 NEWS 를 보시려면, type 'show old'.",
/*      "길드에 가입하면 level-down이 안된답니다.",     */
	"경찰길드와 깡패길드는 midgaard근처에 있습니다.",
	"암살자 길드는 DeathKingdom에 있습니다.",

	"여기서 나가시려면 s e u 에 있는 Reception에서 type 'rent'.",
	"기술을 익히려면 Practice Room 에서 'prac'. 여기서 s s w w s s e.",
	"배가 고프시면 여기 sundew를 드세요. drink sundew.",
	"여기 북쪽의 Altar에 약간의 아이템이 있을 지도 몰라요.",
	"Altar 아래쪽은 함정이니 초보들은 빠지지않게 조심.",
	"사파리 클럽에 가면 비싼 약도 판다는데 뽕이 아닐까?",
	"마법가게에서 파는 두루말이는 꼭 사서 다니세요, 값도 싸요..",
	"가게에서 파는 지도는 허접하지만 초보한테는 약간 도움이 될지도..",
	"아주 낮은 레벨은 Arena 서 신병훈련을 받으세여. 여기서 6번 남쪽.",
	"초보용 존으로는 Tall Building, Shire, East/West Castle 등등  ",
	"Tall Building도 초보는 20 층이상은 올라가지 마세요.",
	"돈이나 아이템을 덤프에 버리면 경험치가 올라요. 단 조금씩.",
	"돈은 은행에 저금하세요. 요즘 소매치기가 많아서...",
	"어떤 마법이나 기술은 쓰면 위력이 더욱 강해져요. 많이 쓰세요.",
	"전사는 칼, 도둑은 단도, 성직자와 마법사는 도끼나 막대기가 좋아요.",
	"전사는 맨손이 쎄야돼요. 칼을 들지말고 싸워보셔요.",
	"도둑은 단도를 들고 기습하고 빠져나오기가 좋죠. back & lig...",
	"당신의 상태를 보려면 score, 소지품은 inv, 입고있는 것은 equip.",
	"상대를 보려면 look, 공격전에 consider, 죽이려면 kill 하세요.",
	"한글로 보시려면 set han yes. 그러나 아직 명령어는 한글이 안돼요.",
	"말씀을 하시려면 say 라고 하세요. 그외 tell, chat도 있어요.",
	"칼을 손에 쥐려면 wield, Holding 가방을 등에 메려면 hold.",
	"마법을 쓰려면 \"cast '마법이름' 대상이름\" 이렇게 하면 됩니다.",
	"마법이나 기술의 종류를 보려면 spell. 다 쓸 수 있는건 아녀요.",
	"가지고 있는 물건을 더 자세히 알아보려면 cast 'identify' object",

/*      "메타에서 AC나 DR, HR을 사시려면 표가 있어야 되는데 단군만 준대요", */
	"SEWER는 Dump 아래에 있는데 길이 복잡하고 빠져나오기가 어려워요.",
	"Deathcure는 붙잡히면 힘을 다 뺏으니 보이면 얼른 피하세요.",
	"11레벨 이상은 quest를 해야 합니다. type 'quest', 'hint'",
	"quest가 어려우시면 request하거나 request ticket을 사용하세요.",
	"단군들은 quest를 70개 씩이나 했다는데 믿기지 않는군..",
	"QUERY 하시면 동물들의 상태를 아실 수 있대요.",
	"모든 직업을 다 거치기 전에는 메타를 어느 이상 살 수 없데나...",
	"모든 직업을 다 거치기 전엔 선악도에 가지마셔요. 뼈도 못추려요.",
	"혼자 다니시지 말고 그룹으로 몰려 다니시면 더 재밌어요.",
	"고수들은 초보들 좀 잘 가르쳐 주세요. 퀘스트만 도와주지말고...",
	"KAIST 3호관은 문 좀 꼭꼭 닫고 다니세요. 연구에 지장이 많아요 :(",
#ifdef MID_HELPER	
	"어려운 일은 " MID_HELPER "님에게 부탁해 보셔요. 잘(?) 도와줄겁니다.", 
#endif	    
	"귀여운 희선이가 보고시포... 전화해서 만나자고 해야징.. :)",
	"음.. 배고프다.. 신라면이나 끓여먹자..",
	"...", "...", "END_OF_TIPS", "\n"
};

char *spell_wear_off_msg[] =
{
    "RESERVED DB.C",
    "You feel less protected.",		/* 1 */
    "!Teleport!",
    "You feel less righteous.",
    "You feel a cloak of blindness dissolve.",
    "!Burning Hands!",
    "!Call Lightning",
    "You feel more self-confident.",
    /* NOTE:  "!Chill Touch!", */
    "Your feel coldness of your body vanish.",
    "!Reanimate!",
    "!Color Spray!",
    "!Relocate!",	/* 11 */
    "!Create Food!",
    "!Create Water!",
    "!Cure Blind!",
    "!Cure Critic!",
    "!Cure Light!",
    "You feel better.",
    "You sense the red in your vision disappear.",
    "The detect invisible wears off.",
    "The detect magic wears off.",
    "!PREACH!",		/* 21 */
    "!Dispel Evil!",
    "!Earthquake!",
    "!Enchant Weapon!",
    "!Energy Drain!",
    "!Fireball!",
    "!Harm!",
    "!Heal",
    "You feel yourself exposed.",
    "!Lightning Bolt!",
    "!Locate object!",	/* 31 */
    "!Magic Missile!",
    "You feel less sick.",
    "You feel less protected.",
    "!Remove Curse!",
    "The white aura around your body fades.",
    "!Shocking Grasp!",
    "You feel less tired.",
    "You feel weaker.",
    "!Summon!",
    "!Ventriloquate!",	/* 41 */
    "!Word of Recall!",
    "!Remove Poison!",
    "You feel less aware of your surroundings.",
    "!Sunburst!",
    "!UNUSED!",
    "!UNUSED!",
    "!UNUSED!",
    "!UNUSED!",
    "!UNUSED!",
    "!UNUSED!",		/* 51 */
    "!UNUSED!",
    "!Identify!",
    "!UNUSED!",
    "",			/* NO MESSAGE FOR SNEAK */
    "!Hide!",
    "!Steal!",
    "!Backstab!",
    "!Pick Lock!",
    "!Kick!",
    "!Bash!",		/* 61 */
    "!Rescue!",
    "!UNUSED!",
    "!UNUSED!",
    "!UNUSED!",
    "!UNUSED!",
    "!UNUSED!",
    "!UNUSED!",
    "!UNUSED!",
    "!UNUSED!",
    "You feel less brave.",	/* 71 */
    "!energy flow!",
    "!mana boost!",
    "!vitalize!",
    "!full fire!",
    "!throw!",
    "!fire storm!",
    "You feel less shirink.",
    "!full heal!",
    "!trick!",
    "You feel things return to normal.",	/* 81 */
    "You feel things return to boring.",
    "!far look!",
    "!all heal!",
    "!tornado!",
    "!light move!",
    "!parry!",
    "!flash!",
    "!multi kick!",
    "!UNUSED90!",
    "!UNUSED91!",	/* 91 */
    "Your eyes are less sensitive now.",	/* 92 ; infra */
    "!UNUSED93!",
    "!UNUSED94!",
    "!UNUSED95!",
    "!UNUSED96!",
    "!UNUSED97!",	/* 97 ; mirror image */
    "Your illusion color fades.",	/* 98 ; mirror image */
    "!UNUSED99!",
    "!UNUSED100!",	/* 100 */
    "!UNUSED101!",
    "!UNUSED102!",
    "!UNUSED103!",
    "You are normalized again!",	/* 104: love */
    "!UNUSED105!",
    "You feel less angelic!",	/* reraise 106  */
    "!UNUSED107!",
    "!UNUSED108!",
    "!UNUSED109!",
    "You have been killed!!",
    "!UNUSED110!",
    "Your blocking clouds got fade!!",
    "!UNUSED112!",
    "Your shadow fades!",
    "!UNUSED114!",
    "!UNUSED115!",
    "Your holy shield fades!",
    "Your reflecting shield fades!",
    "!UNUSED118",
    "Your reflecting shield fades!",
    "You can speak now!!",
    "!jjup!",
    "!jjup!",
    "!jjup!",
    "\n",
};


int rev_dir[] =
{
    2, 3, 0, 1, 5, 4
};

int movement_loss[] =
{
    1,			/* Inside     */
    2,			/* City       */
    2,			/* Field      */
    3,			/* Forest     */
    4,			/* Hills      */
    6,			/* Mountains  */
    4,			/* Swimming   */
    1,			/* Unswimable */
    1			/* Sky */
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

char *weekdays[7] =
{
    "the Day of the Moon",
    "the Day of the Bull",
    "the Day of the Deception",
    "the Day of Thunder",
    "the Day of Freedom",
    "the day of the Great Gods",
    "the Day of the Sun"};

char *month_name[17] =
{
    "Month of Winter",	/* 0 */
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

int sharp[] =
{
    0,
    0,
    0,
    1,			/* Slashing */
    0,
    0,
    0,
    0,			/* Bludgeon */
    0,
    0,
    0,
    0};			/* Pierce   */

char *where[] =
{
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
    "<worn on back>       ",
};

char *drinks[] =
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
    "sundew",
    "nectar",
    "golden nectar",
    "makoli",
    "condition"
};

char *drinknames[] =
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
    "sundew",
    "nectar",
    "golden nectar",
    "makoli",
    "condition"
};


char *fullness[] =
{
    "less than half ",
    "about half ",
    "more than half ",
    ""
};


char *color_liquid[] =
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
    "white",
    "yellow",
    "golden",
    "white",
    /* NOTE: color of condition(TM) drink :
	BUG Reported by  HaengSook Lee <siesta@soback.kornet.nm.kr> */
    "pale green",
};

int drink_aff[][3] =
{
    {0, 1, 10},		/* Water    */
    {2, 2, 5},		/* beer     */
    {1, 3, 4},		/* wine     */
    {3, 2, 5},		/* ale      */
    {3, 2, 5},		/* ale      */
    {6, 1, 4},		/* Whiskey  */
    {0, 1, 8},		/* lemonade */
    {10, 0, 0},		/* firebr   */
    {3, 3, 3},		/* local    */
    {0, 4, -8},		/* juice    */
    {0, 3, 6},		/* milk     */
    {0, 1, 6},		/* tea      */
    {0, 1, 6},		/* coffee   */
    {0, 2, -1},		/* blood    */
    {0, 1, -2},		/* salt     */
    {0, 3, 8},		/* sundew   */
    {5, 5, 5},		/* nectar   */
    {8, 8, 8},		/* golden nectar */
    {5, 1, 2},		/* makoli 막걸리 */
    /* {-4, 1, 1} */	/* NOTE: condition: New formula! */
    {-8, 1, 1}		/* condition */
};


struct title_type titles[4][LEVEL_SIZE] =
{
    {
	{"the Man", "the Woman", 0},
	{"the Apprentice of Magic", "the Apprentice of Magic", 1},	/* 1 */
	{"the Spell Student", "the Spell Student", 1250},
	{"the Scholar of Magic", "the Scholar of Magic", 2500},
	{"the Delver in Spells", "the Delveress in Spells", 4000},
	{"the Medium of Magic", "the Medium of Magic", 8000},
	{"the Scribe of Magic", "the Scribess of Magic", 16000},
	{"the Seer", "the Seeress", 30000},
	{"the Sage", "the Sage", 50000},
	{"the Illusionist", "the Illusionist", 85000},
	{"the Abjurer", "the Abjuress", 150000},	/* 10 */
	{"the Invoker", "the Invoker", 350000},
	{"the Enchanter", "the Enchantress", 650000},
	{"the Conjurer", "the Conjuress", 1000000},
	{"the Magician", "the Witch", 1300000},
	{"the Creator", "the Creator", 1600000},
	{"the Savant", "the Savant", 2000000},
	{"the Magus", "the Craftess", 2500000},
	{"the Wizard", "the Wizard", 3000000},
	{"the Warlock", "the War Witch", 3500000},
	{"the Sorcerer", "the Sorceress", 4000000},	/* 20 */
	{"the Man of Hecate", "the Lady of Hecate", 5000000},
	{"the Apparantice Archmage", "the Apparantice Archmage", 7000000},
	{"the Archmage of Fire", "the Archmage of Fire", 10000000},
	{"the Archmage of Water", "the Archmage of Water", 15000000},
	{"the Archmage of Earth", "the Archmage of Earth", 22000000},
	{"the Archmage of Wind", "the Archmage of Wind", 35000000},
	{"the Mage Guru", "the Mage Guru", 50000000},
	{"the Apparantice MageLord", "the Apparantice MageLord", 70000000},
	{"the MageLord of Fire", "the MageLord of Fire", 85000000},
	{"the MageLord of Water", "the MageLord of Water", 100000000},	/* 30 
									 */
	{"the MageLord of Earth", "the MageLord of Earth", 125000000},
	{"the MageLord of Wind", "the MageLord of Wind", 150000000},
	{"the MageLord of Moon", "the MageLord of Moon", 175000000},
	{"the MageLord of Sun", "the MageLord of Sun", 200000000},
	{"the MageLord of Galaxy", "the MageLord of Galaxy", 225000000},
	{"the MageLord of Universe", "the MageLord of Universe", 250000000},
  {"the MageLord of Metaphysics", "the MageLord of Metaphysics", 275000000},
	{"the Metaphysician", "the Metaphysicians", 300000000},
	{"the Master of MageLord", "the Master of MageLord", 325000000},
	{"the Dangun", "the Dangun", 350000000},	/* 40 */
	{"the Immortal Warlock", "the Immortal Enchantress", 400000000},
	{"the Avatar of Magic", "the Empress of Magic", 500000000},
	{"the God of magic", "the Goddess of magic", 600000000},
	{"the Implementator", "the Implementress", 700000000}},

    {
	{"the Man", "the Woman", 0},
	{"the Believer", "the Believer", 1},	/* 1 */
	{"the Attendant", "the Attendant", 500},
	{"the Acolyte", "the Acolyte", 1000},
	{"the Novice", "the Novice", 2000},
	{"the Missionary", "the Missionary", 3000},
	{"the Adept", "the Adept", 5000},
	{"the Deacon", "the Deaconess", 10000},
	{"the Vicar", "the Vicaress", 15000},
	{"the Priest", "the Priestess", 25000},
	{"the Minister", "the Lady Minister", 35000},	/* 10 */
	{"the Canon", "the Canon", 57000},
	{"the Levite", "the Levitess", 90000},
	{"the Curate", "the Curess", 125000},
	{"the Monk", "the Nunne", 130000},
	{"the Healer", "the Healess", 155000},
	{"the Chaplain", "the Chaplain", 180000},
	{"the Expositor", "the Expositress", 225000},
	{"the Bishop", "the Bishop", 220000},
	{"the Arch Bishop", "the Arch Lady of the Church", 250000},
	{"the Patriarch", "the Matriarch", 300000},	/* 20 */
	{"the Lesser Doctor", "the Lesser Doctor", 600000},
	{"the Doctor", "the Doctor", 1200000},
	{"the Master Doctor", "the Master Doctor", 2500000},
	{"the Healer of GOD", "the Healer of GOD", 4000000},
	{"the Cleric", "the Cleric", 6000000},
	{"the Cleric for people", "the Cleric for people", 7500000},
	{"the Chief Doctor", "the Chief Doctor", 9300000},
	{"the Great Doctor", "the Great Doctor", 15000000},
	{"the Doctor in Wind", "the Doctor in Wind", 30000000},
	{"the Doctor in Fire", "the Doctor in Fire", 75000000},		/* 30 
									 */
	{"the Doctor of earth", "the Doctor of earth", 90000000},
	{"the Doctor of Universe", "the Doctor of Universe", 105000000},
	{"the Famous Doctor", "the Famous Doctor", 120000000},
	{"the Prayer", "the Prayer", 140000000},
	{"the Healer", "the Healer", 160000000},
	{"the Healer of Lords", "the Healer of Lords", 180000000},
	{"the Healer of God", "the Healer of God", 200000000},
	{"the Great Prayer", "the Great Prayer", 220000000},
	{"the Prayer for Dangun", "the Prayer for Dangun", 240000000},
	{"the Dangun", "the Dangun", 280000000},	/* 40 */
	{"the Immortal Cardinal", "the Immortal Priestess", 300000000},
	{"the Inquisitor", "the Inquisitress", 500000000},
    {"the God of good and evil", "the Goddess of good and evil", 600000000},
	{"the Implementator", "the Implementress", 700000000}},

    {
	{"the Man", "the Woman", 0},
	{"the Pilferer", "the Pilferess", 1},	/* 1 */
	{"the Footpad", "the Footpad", 1400},
	{"the Filcher", "the Filcheress", 2800},
	{"the Pick-Pocket", "the Pick-Pocket", 5600},
	{"the Sneak", "the Sneak", 11000},
	{"the Pincher", "the Pincheress", 22000},
	{"the Cut-Purse", "the Cut-Purse", 40000},
	{"the Snatcher", "the Snatcheress", 80000},
	{"the Sharper", "the Sharpress", 140000},
	{"the Rogue", "the Rogue", 180000},	/* 10 */
	{"the Robber", "the Robber", 280000},
	{"the Magsman", "the Magswoman", 500000},
	{"the Highwayman", "the Highwaywoman", 660000},
	{"the Burglar", "the Burglaress", 920000},
	{"the Thief", "the Thief", 1180000},
	{"the Knifer", "the Knifer", 1420000},
	{"the Quick-Blade", "the Quick-Blade", 1660000},
	{"the Killer", "the Murderess", 1900000},
	{"the Brigand", "the Brigand", 2300000},
	{"the Cut-Throat", "the Cut-Throat", 3000000},	/* 20 */
	{"the Urchin Father", "the Urchin Mother", 4000000},
	{"the Bully", "the Bully", 6000000},
	{"the Snifer", "the Snifer", 8500000},
	{"the famous Killer", "the Famous Killer", 12000000},
	{"the Best Snifer", "the Best Snifer", 16000000},
	{"the Quick Knife", "the Quick Knife", 20000000},
	{"the Stealer", "the Stealer", 30000000},
	{"the Stealer of breath", "the Stealer of Breath", 45000000},
	{"the Good Thief", "the Good Thief", 66000000},
	{"the Lord of Thieves", "the Lady of Thieves", 90000000},	/* 30 
									 */
	{"the King of Thieves", "the Queen of Thieves", 115000000},
	{"the Urchin in world", "the Urchin in world", 140000000},
	{"the Shadow", "the Shadow", 165000000},
	{"the Thief of Fire", "the Thief of Fire", 190000000},
	{"the Thief of Wind", "the Thief of Wind", 215000000},
	{"the Thief of Earth", "the Thief of Earth", 240000000},
	{"the Thief of Universe", "the Thief of Universe", 270000000},
	{"the God-Father", "the God-Mother", 300000000},
	{"the Greatest Bully", "the Greatest Bully", 330000000},
	{"the Dangun", "the Dangun", 360000000},	/* 40 */
	{"the Immortal Assassin", "the Immortal Assassin", 400000000},
      {"the Demi God of thieves", "the Demi Goddess of thieves", 500000000},
	{"the God of Thieves", "the Goddess of Thieves", 600000000},
	{"the Implementator", "the Implementress", 700000000}},

    {
	{"the Man", "the Woman", 0},
	{"the Swordpupil", "the Swordpupil", 1},	/* 1 */
	{"the Recruit", "the Recruit", 2000},
	{"the Sentry", "the Sentress", 5000},
	{"the Fighter", "the Fighter", 10000},
	{"the Soldier", "the Soldier", 20000},
	{"the Warrior", "the Warrior", 40000},
	{"the Veteran", "the Veteran", 75000},
	{"the Swordsman", "the Swordswoman", 150000},
	{"the Fencer", "the Fenceress", 300000},
	{"the Combatant", "the Combatess", 650000},	/* 10 */
	{"the Hero", "the Heroine", 1000000},
	{"the Myrmidon", "the Myrmidon", 1400000},
	{"the Swashbuckler", "the Swashbuckleress", 1800000},
	{"the Mercenary", "the Mercenaress", 2200000},
	{"the Swordmaster", "the Swordmistress", 2600000},
	{"the Lieutenant", "the Lieutenant", 3000000},
	{"the Champion", "the Lady Champion", 3500000},
	{"the Dragoon", "the Lady Dragoon", 4000000},
	{"the Cavalier", "the Cavalier", 4500000},
	{"the Knight", "the Lady Knight", 5000000},	/* 20 */
	{"the Knight of Fire", "the Knight of Fire", 6500000},
	{"the Knight of Wind", "the Knight of Wind", 8500000},
	{"the Knight of Ice", "the Knight of Ice", 12000000},
	{"the Knight of Water", "the Knight of Water", 17000000},
	{"the Powerful Knight", "the Powerful Knight", 24000000},
	{"the Lord of Wind", "the Lady of Wind", 34000000},
	{"the Lord of Fire", "the Lady of Fire", 50000000},
	{"the Lord of Water", "the Lady of Water", 70000000},
	{"the Lord of Earth", "the Lord of Earth", 95000000},
	{"the Bodyguard", "the Bodyguard", 120000000},	/* 30 */
	{"the Famous knight", "the Famous Knight", 150000000},
	{"the Best Knight", "the Best Knight", 180000000},
	{"the Lord of Knight", "The Lady of Knight", 210000000},
	{"The King of Knight", "the King of Knight", 240000000},
	{"the Adventurer", "the Adventurer", 270000000},
	{"the Best kid", "the Best lady", 300000000},
	{"the Paladin", "the Paladin", 330000000},
	{"the Best Swordman", "the Best Swordwoman", 360000000},
      {"the Adventurer for Dangun", "the Adventurer for Dangun", 400000000},
	{"the Dangun", "the Dangun", 430000000},	/* 40 */
	{"the Immortal Warlord", "the Immortal Lady of War", 500000000},
	{"the Extirpator", "the Queen of Destruction", 600000000},
	{"the God of war", "the Goddess of war", 700000000},
	{"the Implementator", "the Implementress", 800000000}}
};

/* NOTE: Commnet out types no longer supported. */
char *item_types[] =
{
    "UNDEFINED",
    "LIGHT",
    "SCROLL",
    "WAND",
    "STAFF",
    "WEAPON",
    "FIRE WEAPON",
    "UNDEFINED", /* "MISSILE",*/
    "TREASURE",
    "ARMOR",
    "POTION",
    "WORN",
    "OTHER",
    "UNDEFINED", /* "TRASH", */
    "UNDEFINED", /* "TRAP", */
    "CONTAINER",
    "NOTE",
    "LIQUID CONTAINER",
    "KEY",
    "FOOD",
    "MONEY",
    "UNDEFINED", /* "PEN", */
    "UNDEFINED", /* "BOAT", */
    "UNDEFINED", /* "CIGA", */
    "UNDEFINED", /* "DRUG", */
    "\n"
};

char *wear_bits[] =
{
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
    "BACKPACK",
    "\n"
};

char *extra_bits[] =
{
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
    "ANTI-POLICE",
    "ANTI-OUTLAW",
    "ANTI-ASSASSIN",
    "NOCOPY",
    "\n"
};

char *room_bits[] =
{
    "DARK",
    "NOSUMMON",
    "NO_MOB",
    "INDOORS",
    "LAWFUL",
    "NEUTRAL",
    "CHAOTIC",
    "NO_MAGIC",
    "TUNNEL",
    "PRIVATE",
    "OFF_LIMITS",
    "RENT",
    "NORELOCATE",
    "EVERYZONE",
    "JAIL",
    "RESTROOM",
    "TEST",
    "TEST2",
    "TEST3",
    "TEST4",
    /* NOTE: Additional bits for "house.c"  */
    "HOUSE",
    "H_CRASH",
    "BFS_MARK", /* NOTE: NEW! Youshould *never see romm with this bit set */
    "\n"
};

char *exit_bits[] =
{
    "IS-DOOR",
    "CLOSED",
    "LOCKED",
    "KK",
    "LL",
    "PICKPROOF",
    "NOPHASE",
    "\n"
};

char *sector_types[] =
{
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

char *equipment_types[] =
{
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
    "Worn on back",
    "\n"
};

char *affected_bits[] =
{
 "BLIND",
 "INVISIBLE",
 "DETECT-EVIL",
 "DETECT-INVISIBLE",
 "INFRAVISION",
 "SENSE-LIFE",
 "DEAF",
 "SANCTUARY",
 "GROUP",
 "DUMB",
 "CURSE",
 "MIRROR-IMAGE",
 "POISON",
 "PROTECT-EVIL",
 "REFLECT-DAMAGE",
 "HOLY-SHIELD",
 "SPELL-BLOCK",
 "SLEEP",
 "SHADOW-FIGURE",
 "SNEAK",
 "HIDE",
 "DEATH",
 "CHARM",
 "FOLLOW",
 "HASTE",
 "IMPROVED_HASTE",
 "LOVE",
 "RERAISE",
 "\n"
};

char *apply_types[] =
{
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
    "SAVING_HIT_SKILL",
    "SAVING_PETRI",
    "SAVING_BREATH",
    "SAVING_SPELL",
    "REGENERATION",
    "INVISIBLE",
    "\n"
};

char *pc_class_types[] =
{
    "UNDEFINED",
    "Magic User",
    "Cleric",
    "Thief",
    "Warrior",
    "\n"
};

char *npc_class_types[] =
{
    "Normal",
    "Undead",
    "Humanoid",
    "Animal",
    "Dragon",
    "Giant",
    "Demon",
    "Insect",
    "\n"
};

char *action_bits[] =
{
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
    "HELPER",
    "FINISHER",
    "\n"
};

/* NOTE: Re-organized PLR_???? bit position to match "char.h" */
char *player_bits[] =
{
    "RSV1",
    "WIZINVIS",
    "XYZZY",
    "DONTSET",
    "RSV2",
    "SHUTUP",	/* NOTE: Old name "NOSHOUT" */
    "BANISHED",
    "DUMB-BY-WIZ",

    "WIMPY",
    "SOLO",
    "NOSHOUT",
    "NOCHAT",
    "NOTELL",
    "BRIEF",
    "COMPACT",
    "KOREAN",

    "AUTOOPEN",	/* NOTE: NEW! PLR_AUTOOPEN */
    "CRIMINAL",
    "RSV4",
    "RSV5",
    "\n"
}; 

char *position_types[] =
{
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

char *connected_types[] =
{
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
    /* NOTE: Added additional connection state type */
    "Link dead",
    "Change password2",
    "Change password1",
    "Change password done",
    "Get Improvement",
    "Setting New player",
    "Delete player",
    "Delete player Confirm",
    "Read Immortal MOTD",
    "New player MOTD",
    "Entering game",
    "Closing connection",
    "\n"
};

/* [class], [level] (all) M, C, T, W */
/* cyb const int thaco[4][LEVEL_SIZE] = { */
/* probability of attack */
int thaco[4][LEVEL_SIZE] =
{
    {0,
     1, 2, 3, 4, 5, 6, 7, 8, 9, 10,
     11, 12, 13, 14, 15, 16, 17, 18, 19, 20,
     21, 22, 23, 24, 25, 26, 27, 28, 29, 30,
     31, 32, 33, 34, 35, 36, 37, 38, 39, 40,
     0},
    {0,
     3, 4, 5, 6, 7, 8, 9, 10, 11, 12,
     13, 14, 15, 16, 17, 18, 19, 20, 21, 22,
     23, 24, 25, 26, 27, 28, 29, 30, 31, 32,
     33, 34, 35, 36, 37, 38, 39, 40, 41, 42,
     0},
    {0,
     7, 8, 9, 10, 11, 12, 13, 14, 15, 16,
     17, 18, 19, 20, 21, 22, 23, 24, 25, 26,
     27, 28, 29, 30, 31, 32, 33, 34, 35, 36,
     37, 38, 39, 40, 41, 42, 43, 44, 45, 46,
     0},
    {0,
     11, 12, 13, 14, 15, 16, 17, 18, 19, 20,
     21, 22, 23, 24, 25, 26, 27, 28, 29, 30,
     31, 32, 33, 34, 35, 36, 37, 38, 39, 40,
     41, 42, 43, 44, 45, 46, 47, 48, 49, 50,
     0},
};

/* [ch] strength apply (all) */
/* struct = { short tohit, todam, carry_w, wield_w } */
struct str_app_type str_app[31] =
{
    {-5, 0, 0, 0},	/* 0  */
    {-5, 1, 3, 1},	/* 1  */
    {-3, 1, 3, 2},
    {-3, 1, 10, 3},	/* 3  */
    {-2, 1, 25, 4},
    {-2, 1, 55, 5},	/* 5  */
    {-1, 2, 80, 6},
    {-1, 2, 90, 7},
    {0, 2, 100, 8},
    {0, 3, 100, 9},
    {0, 3, 115, 10},	/* 10  */
    {0, 4, 115, 11},
    {0, 5, 130, 12},
    {0, 6, 140, 13},
    {0, 7, 160, 14},
    {0, 8, 170, 15},	/* 15  */
    {0, 9, 195, 16},
    {1, 10, 220, 18},
    {1, 11, 255, 20},	/* 18  */
    {3, 20, 640, 40},
    {3, 21, 700, 40},	/* 20  */
    {4, 22, 810, 40},
    {4, 23, 970, 40},
    {5, 24, 1130, 40},
    {6, 25, 1440, 40},
    {7, 26, 1750, 40},	/* 25            */
    {1, 12, 280, 22},	/* 18/01-50      */
    {1, 13, 305, 24},	/* 18/51-75      */
    {2, 14, 330, 26},	/* 18/76-90      */
    {2, 15, 380, 28},	/* 18/91-99      */
    {3, 16, 480, 30},	/* 18/100   (30) */
};

/* [dex] skill apply (thieves only) */
/* struct = {short p_pocket, p_locks, traps, sneak, hide } */
struct dex_skill_type dex_app_skill[26] =
{
    {-99, -99, -90, -99, -60},	/* 0 */
    {-90, -90, -60, -90, -50},	/* 1 */
    {-80, -80, -40, -80, -45},
    {-70, -70, -30, -70, -40},
    {-60, -60, -30, -60, -35},
    {-50, -50, -20, -50, -30},	/* 5 */
    {-40, -40, -20, -40, -25},
    {-30, -30, -15, -30, -20},
    {-20, -20, -15, -20, -15},
    {-15, -10, -10, -20, -10},
    {-10, -5, -10, -15, -5},	/* 10 */
    {-5, 0, -5, -10, 0},
    {0, 0, 0, -5, 0},
    {0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0},	/* 15 */
    {0, 5, 0, 0, 0},
    {5, 10, 0, 5, 5},
    {10, 15, 5, 10, 10},
    {15, 20, 10, 15, 15},
    {15, 20, 10, 15, 15},	/* 20 */
    {20, 25, 10, 15, 20},
    {20, 25, 15, 20, 20},
    {25, 25, 15, 20, 20},
    {25, 30, 15, 25, 25},
    {25, 30, 15, 25, 25}
};

/* [level] backstab multiplyer (thieves only) */
byte backstab_mult[LEVEL_SIZE] =
{
    1,			/* 0 */
    2,			/* 1 */
    2,
    3,
    3,
    3,			/* 5 */
    4,
    4,
    4,
    4,
    5,			/* 10 */
    5,
    5,
    5,
    5,
    6,			/* 15 */
    6,
    6,
    6,
    6,
    6,			/* 20 */
    7,
    7,
    7,
    7,			/* 25 */
    7,
    7,
    7,
    8,
    8,			/* 30 */
    8, 8, 8, 8, 8,	/* 35 */
    8, 9, 9, 9, 10,	/* 40 */
    10, 10, 12, 20, 45	/* 45 */
};

/* [dex] apply (all) */
/* struct = { short reaction, attack, defensive } */
/* attack : possibility of attack */
/* defensive : possibility of defensive */
struct dex_app_type dex_app[26] =
{
    {-7, 0, 0},		/* 0 */
    {-6, 1, 2},		/* 1 */
    {-4, 2, 4},
    {-3, 3, 6},
    {-2, 4, 8},
    {-1, 5, 10},	/* 5 */
    {0, 6, 12},
    {0, 7, 14},
    {0, 8, 16},
    {0, 9, 18},
    {0, 10, 20},	/* 10 */
    {0, 12, 22},
    {0, 14, 24},
    {0, 16, 26},
    {0, 18, 28},
    {0, 20, 30},	/* 15 */
    {1, 23, 33},
    {2, 26, 36},
    {2, 30, 40},	/* 18 */
    {3, 35, 45},
    {3, 40, 50},	/* 20 */
    {4, 45, 55},
    {4, 50, 60},
    {4, 55, 65},
    {5, 60, 70},
    {5, 65, 75}
};

/* [con] apply (all) */
/* struct = { short hitp, shock }  */
struct con_app_type con_app[26] =
{
    {-4, 20},		/* 0 */
    {-3, 25},		/* 1 */
    {-2, 30},
    {-2, 35},
    {-1, 40},
    {-1, 45},		/* 5 */
    {-1, 50},
    {0, 55},
    {1, 65},
    {2, 70},
    {3, 75},		/* 10 */
    {4, 78},
    {5, 80},
    {6, 85},
    {7, 88},
    {8, 90},		/* 15 */
    {9, 95},
    {10, 97},
    {10, 99},
    {10, 99},
    {10, 99},		/* 20 */
    {10, 99},
    {10, 99},
    {10, 99},
    {10, 99},
    {10, 100}		/* 25 */
};

/* [int] apply (all) */
struct int_app_type int_app[26] =
{
    {1},
    {2},  {3},  {4},  {7},  {8},  {9},  {10}, {20}, {28}, {30},	/* 10 */
    {35}, {38}, {40}, {41}, {42}, {46}, {50}, {54}, {99}, {99},	/* 20 */
    {99}, {99}, {99}, {99}, {99}
};

/* [wis] apply (all) struct = { byte bonus } */
struct wis_app_type wis_app[26] =
{
    {0},
    {1}, {1}, {1}, {1}, {1}, {2}, {2}, {2}, {3}, {3},
    {3}, {3}, {3}, {3}, {4}, {4}, {4}, {6}, {8}, {8},
    {8}, {8}, {8}, {8}, {8}
};


/* NOTE: moved from magic.c */
/*
   modified by ares
   saving_throws[class][type][level]
   class :
   MAGIC_USER, CLERIC, THIEF, WARRIOR
   type :
   SAVING_PARA, SAVING_HIT_SKILL, SAVING_PETRI, SAVING_BREATH, SAVING_SPELL
   lower value is more good
   worst value must be lower than 100
   best value must be higher than 19
   currently, SAVING_PETRI has no effect.

   current stat :
   classification : very good (20 ~ 30 for level 40)
   good      (30 ~ 40 for level 40)
   medium    (40 ~ 50 for level 40)
   bad       (50 ~ 60 for level 40)
   poor      (60 ~ 70 for level 40)
   type :       PARA       HIT SKILL  PETRI      BREATH      SPELL
   MAGIC USER : medium     medium     0          medium      good
   CLERIC     : medium     poor       0          good        very good
   THIEF      : very good  good       0          medium      poor
   WARRIOR    : bad        very good  0          medium      medium
 */

#define SAVING_VERY_GOOD \
	  { 59, 58, 57, 56, 55, 54, 53, 52, 51, 50, \
	    49, 48, 47, 46, 45, 44, 43, 42, 41, 40, \
	    39, 38, 37, 36, 35, 34, 33, 32, 31, 30, \
	    29, 28, 27, 26, 25, 24, 23, 22, 21, 20, \
		0, }
#define SAVING_GOOD \
	  { 69, 68, 67, 66, 65, 64, 63, 62, 61, 60, \
	    59, 58, 57, 56, 55, 54, 53, 52, 51, 50, \
	    49, 48, 47, 46, 45, 44, 43, 42, 41, 40, \
	    39, 38, 37, 36, 35, 34, 33, 32, 31, 30, \
		0, }
#define SAVING_MEDIUM \
	  { 79, 78, 77, 76, 75, 74, 73, 72, 71, 70, \
	    69, 68, 67, 66, 65, 64, 63, 62, 61, 60, \
	    59, 58, 57, 56, 55, 54, 53, 52, 51, 50, \
	    49, 48, 47, 46, 45, 44, 43, 42, 41, 40, \
		0, }
#define SAVING_BAD \
	  { 89, 88, 87, 86, 85, 84, 83, 82, 81, 80, \
	    79, 78, 77, 76, 75, 74, 73, 72, 71, 70, \
	    69, 68, 67, 66, 65, 64, 63, 62, 61, 60, \
	    59, 58, 57, 56, 55, 54, 53, 52, 51, 50, \
		0, }
#define SAVING_POOR \
	  { 99, 98, 97, 96, 95, 94, 93, 92, 91, 90, \
	    89, 88, 87, 86, 85, 84, 83, 82, 81, 80, \
	    79, 78, 77, 76, 75, 74, 73, 72, 71, 70, \
	    69, 68, 67, 66, 65, 64, 63, 62, 61, 60, \
		0, }
#define SAVING_RESERVED \
	  { 0, }

byte saving_throws[4][5][LEVEL_SIZE] =
{
	/* for magic user */
    {SAVING_MEDIUM,
     SAVING_MEDIUM,
     SAVING_RESERVED,
     SAVING_MEDIUM,
     SAVING_GOOD},

	/* for cleric */
    {SAVING_MEDIUM,
     SAVING_POOR,
     SAVING_RESERVED,
     SAVING_GOOD,
     SAVING_VERY_GOOD},

	/* for thief */
    {SAVING_VERY_GOOD,
     SAVING_GOOD,
     SAVING_RESERVED,
     SAVING_MEDIUM,
     SAVING_POOR},

	/* for warrior */
    {SAVING_BAD,
     SAVING_VERY_GOOD,
     SAVING_RESERVED,
     SAVING_MEDIUM,
     SAVING_MEDIUM}
};			/* end of saving throw */

#undef SAVING_VERY_GOOD
#undef SAVING_GOOD
#undef SAVING_MEDIUM
#undef SAVING_BAD
#undef SAVING_POOR
#undef SAVING_RESERVED

/* NOTE: NEW! Longer item description text */
char *item_type_desc[] = { 
    "Item type 0: Invalid",
    "It is too bright to stare for long.",
    "A old and worn parchment with something scribed on it.",
    "The stick seems to bear arcane magic power.",
    "The stick seems to bear arcane magic power.",
    "It looks like weapon crying for blood", 
    "It looks like a gun with threatening barrel.",
    "Item type 7: Invalid",
    "It looks like something very valuable.",
    "You feel like wearing it. Try on.",
    "You can see strange fluid swirling in it.",
    "It looks like a ring fit for your finger.",
    "Item type OTHER", 
    "Item type 13: Invalid", 
    "Item type 14: Invalid", 
    "It seems there are somthing in it.",
    "It is yellowish worn paper", 
    "It looks like a drink container.",
    "It seems an intricate key for some lock.",
    "Item type FOOD",
    "Item type MONEY",
    "Item type 21: Invalid",
    "Item type 22: Invalid",
    "Item type 23: Invalid",
    "Item type 24: Invalid",
    "\n"
};

/* NOTE: NEW! Longer wepon type description text */
char *weapon_type_desc[] = {
    "It looks like weapon to hit.", 
    "The sword beams shimmering splendor from long blade.",
    "It has a short and slender blade thirsty for blood.",
    "It looks like axe or hammer with blood-stained heavy heads.",
    "It looks like club heavy enough to crush enimies's head bone.",
    "It has a long flexible tail which will rip off your flesh.",
    "bite",
    "sting",
    "shoot",
    "\n",
};


/* NOTE: moved from OLD fight.c */ 

/* NOTE: Currently, all damages messages are brief. So no need of data below */
#ifdef NO_DEF

     struct dam_weapon_type dam_weapons[] =
    {

	{"$n misses $N with $s #W.",	/* 0    */
	 "You miss $N with your #W.",
	 "$n misses you with $s #W."},

	{"$n tickles $N with $s #W.",	/* 1.. 4  */
	 "You tickle $N as you #W $M.",
	 "$n tickle you as $e #W you."},

	{"$n barely #W $N.",	/* 5.. 8  */
	 "You barely #W $N.",
	 "$n barely #W you."},

	{"$n #W $N.",	/* 9.. 12  */
	 "You #W $N.",
	 "$n #W you."},

	{"$n #W $N hard.",	/* 13..20  */
	 "You #W $N hard.",
	 "$n #W you hard."},

	{"$n #W $N very hard.",		/* 21..30  */
	 "You #W $N very hard.",
	 "$n #W you very hard."},

	{"$n #W $N extremely hard.",	/* 31..40  */
	 "You #W $N extremely hard.",
	 "$n #W you extremely hard."},

	{"$n massacre $N to small fragments with $s #W.",	/* > 40    */
	 "You massacre $N to small fragments with your #W.",
	 "$n massacre you to small fragments with $s #W."},

	{"$n annihilates $N to micro molecules with $s #W.",	/* > 65    */
	 "You annihilate $N to micro molecules with your #W.",
	 "$n annihilates you to micro molecules with $s #W."},

	{"$n disintegrates $N with $s #W.",	/* > 110 */
	 "You disintegrate $N with your #W.",
	 "$n disintegrates you with $s #W."},

	{"$n sends $N void with $s #W.",	/* > 150 */
	 "You send $N to void with your #W.",
	 "$n sends you to void with $s #W."},

	{"$n tickles $N void with $s #W.",	/* > 200 */
	 "You tickle $N to void with your #W.",
	 "$n tickles you to void with $s #W."},

    /* Furfuri made massage */

	{"$n shoots $N with laser power $s #W.",	/* > 250 */
	 "You shoot $N with laser power #W.",
	 "$n shoots you with laser power $s #W."},

	{"$n strikes $N with NUCLEAR power $s #W.",
	 "You strike $N with NUCLEAR power #W.",
	 "$n strikes you with NUCLEAR power $s #W."},

	{"$n CRASHES $N with -= S P A C E   P O W E R =- $s #W.",
	 "YOU CRASH $N with -= S P A C E   P O W E R =- #W.",
	 "$n CRASHES YOU with -= S P A C E   P O W E R =- $s #W."},

	{"$n PROCESSES $N into ---==<< V A C U U M >>==--- with $s #W.",
	 "YOU PROCESS $N into ---==<< V A C U U M >>==--- with your #W.",
	 "$n PROCESSES YOU into ---==<< V A C U U M >>==--- with $s #W."}
    };

#endif				/* NO_DEF */

/* brief mode */
    struct dam_weapon_type brief_dam_weapons[] =
    {
	{"$n -miss- $N.",	/* 0    */
	 "You -miss- $N.",
	 "$n -miss- you."},

	{"$n -tickle- $N.",	/* 1.. 4  */
	 "You -tickle- $N.",
	 "$n -tickle- you."},

	{"$n -barely- $N.",	/* 5.. 8  */
	 "You -barely- $N.",
	 "$n -barely- you."},

	{"$n -hit- $N.",	/* 9.. 12  */
	 "You -hit- $N.",
	 "$n -hit- you."},

	{"$n -hard- $N.",	/* 13..20  */
	 "You -hard- $N.",
	 "$n -hard- you."},

	{"$n -very hard- $N.",	/* 21..30  */
	 "You -very hard- $N.",
	 "$n -very hard- you."},

	{"$n -extremely hard- $N.",	/* 31..40  */
	 "You -extremely hard- $N.",
	 "$n -extremely hard- you."},

	{"$n -massacre- $N.",	/* > 40    */
	 "You -massacre- $N.",
	 "$n -massacre- you."},

	{"$n -annihilate- $N.",		/* > 65    */
	 "You -annihilate- $N.",
	 "$n -annihilate- you."},

	{"$n -disintegrate- $N.",	/* > 110 */
	 "You -disintegrate- $N.",
	 "$n -disintegrate- you."},

	{"$n -send void- $N.",	/* > 150 */
	 "You -send void- $N.",
	 "$n -send void- you."},

	{"$n -tickle void- $N.",	/* > 200 */
	 "You -tickle void- $N.",
	 "$n -tickle void- you."},

    /* Furfuri made massage */

	{"$n -= laser power =- $N.",	/* > 250 */
	 "You -= laser power =- $N.",
	 "$n -= laser power =- you."},

	{"$n -= NUCLEAR power =- $N.",
	 "You -= NUCLEAR power =- $N.",
	 "$n -= NUCLEAR power =- YOU."},

	{"$n -= S P A C E   P O W E R =- $N.",
	 "YOU -= S P A C E   P O W E R =- $N.",
	 "$n -= S P A C E   P O W E R =- YOU."},

	{"$n ---==<< V A C U U M >>==--- $N.",
	 "YOU ---==<< V A C U U M >>==--- $N.",
	 "$n ---==<< V A C U U M >>==--- YOU."}
    };

    /* NOTE: When Player is in group fighting and English mode, prepend '*'
       char to 'You', 'you', 'YOU' word to help they be easily recognized
       among long and rapid damage messages scroll */

    struct dam_weapon_type you_dam_weapons[] =
    {
	{"$n -miss- $N.",	/* 0    */
	 "*You -miss- $N.",
	 "$n -miss- *you."},

	{"$n -tickle- $N.",	/* 1.. 4  */
	 "*You -tickle- $N.",
	 "$n -tickle- *you."},

	{"$n -barely- $N.",	/* 5.. 8  */
	 "*You -barely- $N.",
	 "$n -barely- *you."},

	{"$n -hit- $N.",	/* 9.. 12  */
	 "*You -hit- $N.",
	 "$n -hit- *you."},

	{"$n -hard- $N.",	/* 13..20  */
	 "*You -hard- $N.",
	 "$n -hard- *you."},

	{"$n -very hard- $N.",	/* 21..30  */
	 "*You -very hard- $N.",
	 "$n -very hard- *you."},

	{"$n -extremely hard- $N.",	/* 31..40  */
	 "*You -extremely hard- $N.",
	 "$n -extremely hard- *you."},

	{"$n -massacre- $N.",	/* > 40    */
	 "*You -massacre- $N.",
	 "$n -massacre- *you."},

	{"$n -annihilate- $N.",		/* > 65    */
	 "*You -annihilate- $N.",
	 "$n -annihilate- *you."},

	{"$n -disintegrate- $N.",	/* > 110 */
	 "*You -disintegrate- $N.",
	 "$n -disintegrate- *you."},

	{"$n -send void- $N.",	/* > 150 */
	 "*You -send void- $N.",
	 "$n -send void- *you."},

	{"$n -tickle void- $N.",	/* > 200 */
	 "*You -tickle void- $N.",
	 "$n -tickle void- *you."},

    /* Furfuri made massage */

	{"$n -= laser power =- $N.",	/* > 250 */
	 "*You -= laser power =- $N.",
	 "$n -= laser power =- *you."},

	{"$n -= NUCLEAR power =- $N.",
	 "*You -= NUCLEAR power =- $N.",
	 "$n -= NUCLEAR power =- *YOU."},

	{"$n -= S P A C E   P O W E R =- $N.",
	 "*YOU -= S P A C E   P O W E R =- $N.",
	 "$n -= S P A C E   P O W E R =- *YOU."},

	{"$n ---==<< V A C U U M >>==--- $N.",
	 "*YOU ---==<< V A C U U M >>==--- $N.",
	 "$n ---==<< V A C U U M >>==--- *YOU."}
    };

/* NOTE: Currently, all damages messages are brief. So no need of data below */
#ifdef NO_DEF

    struct dam_weapon_type han_weapons[] =
    {
	{"$n 님이 $N 님을 때리려다 빗나갔습니다.",	/* 0    */
	 "당신은 $N 님을 때리지 못했습니다.",
	 "$n 님이 당신을 때리려다 실패했습니다."},

	{"$n 님이 $N 님을 간지럽게 #W.",	/* 1.. 4  */
	 "당신은 $N 님을 간지럽게 #W.",
	 "$n 님이 당신을 간지럽게 #W."},

	{"$n 님이 $N 님을 살살 #W.",	/* 5.. 8  */
	 "당신은 $N 님을 살살 #W.",
	 "$n 님이 당신을 살살 #W."},

	{"$n 님이 $N 님을 #W.",		/* 9.. 12  */
	 "당신은 $N 님을 #W.",
	 "$n 님이 당신을 #W."},

	{"$n 님이 $N 님을 세게 #W.",	/* 13..20  */
	 "당신은 $N 님을 세게 #W.",
	 "$n 님이 당신을 세게 #W."},

	{"$n 님이 $N 님을 매우 세게 #W.",	/* 21..30  */
	 "당신은 $N 님을 매우 세게 #W.",
	 "$n 님이 당신을 매우 세게 #W."},

	{"$n 님이 $N 님을 무지막지하게 #W.",	/* 31..40  */
	 "당신은 $N 님을 무지막지하게 #W.",
	 "$n 님이 당신을 무지막지하게 #W."},

	{"$n 님이 $N 님을 뼈가 으스러지게 #W.",		/* > 40    */
	 "당신은 $N 님을 뼈가 으스러지게 #W.",
	 "$n 님이 당신을 뼈가 으스러지게 #W."},

	{"$n 님이 $N 님을 몸이 산산조각이 나게 #W.",	/* > 65    */
	 "당신은 $N 님을 몸이 산산조각이 나게 #W.",
	 "$n 님이 당신을 몸이 산산조각이 나게 #W."},

	{"$n 님이 $N 님을 몸이 가루가 되게 #W.",	/* > 110    */
	 "당신은 $N 님을 몸이 가루가 되게 #W.",
	 "$n 님이 당신을 몸이 가루가 되게 #W."},

	{"$n 님이 $N 님을 눈앞에 별이 돌게 #W.",	/* > 150 */
	 "당신은 $N 님을 눈앞에 별이 돌게 #W.",
	 "$n 님이 당신을 눈앞에 별이 돌게 #W."},

	{"$n 님이 $N 님을 눈앞에 병아리가 돌게 #W.",	/* > 200 */
	 "당신은 $N 님을 눈앞에 병아리가 돌게 #W.",
	 "$n 님이 당신을 눈앞에 병아리가 돌게 #W."},

	{"$n 님이 $N 님을 소리없이 #W.",	/* > 250 */
	 "당신은 $N 님을 소리없이 #W.",
	 "$n 님이 당신을 소리없이 #W."},

	{"$n 님이 $N 님을 레이져 파워로 #W.",
	 "당신은 $N 님을 레이져 파워로 #W.",
	 "$n 님이 당신을 레이져 파워로 #W."},

	{"$n 님이 $N 님을 전..우주의 힘을 빌어 #W.",
	 "당신은 $N 님을 전 ..우주의 힘을 빌어 #W.",
	 "$n 님이 당신을 전 ..우주의 힘을 빌어 #W."},

	{"$n 님이 $N 님을 진공상태로 되도록 #W.",
	 "당신은 $N 님을 진공상태로 되도록 #W.",
	 "$n 님이 당신을 진공상태가 되도록 #W."}
    };

#endif				/* NO_DEF */

    /* brief mode */
    struct dam_weapon_type brief_han_weapons[] =
    {
	{"$n, $N 님을 못침.",	/* 0    */
	 "당신, $N 님을 못침.",
	 "$n, 당신을 못침."},

	{"$n, $N 님을 간지럽게.",	/* 1.. 4  */
	 "당신, $N 님을 간지럽게.",
	 "$n, 당신을 간지럽게."},

	{"$n, $N 님을 살살.",	/* 5.. 8  */
	 "당신, $N 님을 살살.",
	 "$n, 당신을 살살."},

	{"$n, $N 님을 침.",	/* 9.. 12  */
	 "당신, $N 님을 침.",
	 "$n, 당신을 침."},

	{"$n, $N 님을 세게.",	/* 13..20  */
	 "당신, $N 님을 세게.",
	 "$n, 당신을 세게."},

	{"$n, $N 님을 매우 세게.",	/* 21..30  */
	 "당신, $N 님을 매우 세게.",
	 "$n, 당신을 매우 세게."},

	{"$n, $N 님을 무지막지하게.",	/* 31..40  */
	 "당신, $N 님을 무지막지하게.",
	 "$n, 당신을 무지막지하게."},

	{"$n, $N 님을 뼈가 으스러지게.",	/* > 40    */
	 "당신, $N 님을 뼈가 으스러지게.",
	 "$n, 당신을 뼈가 으스러지게."},

	{"$n, $N 님을 몸이 산산조각이 나게.",	/* > 65    */
	 "당신, $N 님을 몸이 산산조각이 나게.",
	 "$n, 당신을 몸이 산산조각이 나게."},

	{"$n, $N 님을 몸이 가루가 되게.",	/* > 110    */
	 "당신, $N 님을 몸이 가루가 되게.",
	 "$n, 당신을 몸이 가루가 되게."},

	{"$n, $N 님을 눈앞에 별이 돌게.",	/* > 150 */
	 "당신, $N 님을 눈앞에 별이 돌게.",
	 "$n, 당신을 눈앞에 별이 돌게."},

	{"$n, $N 님을 눈앞에 병아리가 돌게.",	/* > 200 */
	 "당신, $N 님을 눈앞에 병아리가 돌게.",
	 "$n, 당신을 눈앞에 병아리가 돌게."},

	{"$n, $N 님을 소리없이.",	/* > 250 */
	 "당신, $N 님을 소리없이.",
	 "$n, 당신을 소리없이."},

	{"$n, $N 님을 레이져 파워로.",
	 "당신, $N 님을 레이져 파워로.",
	 "$n, 당신을 레이져 파워로."},

	{"$n, $N 님을 전..우주의 힘을 빌어.",
	 "당신, $N 님을 전 ..우주의 힘을 빌어.",
	 "$n, 당신을 전 ..우주의 힘을 빌어."},

	{"$n, $N 님을 진공상태로.",
	 "당신, $N 님을 진공상태로.",
	 "$n, 당신을 진공상태로."}
    }; 

#ifdef OLD_DAM_MESSAGE

    struct dam_weapon_type dam_weapons[] =
    {

	{"$n misses $N with $s #W.",	/* 0    */
	 "You miss $N with your #W.",
	 "$n miss you with $s #W."},

	{"$n tickles $N with $s #W.",	/* 1.. 4  */
	 "You tickle $N as you #W $M.",
	 "$n tickle you as $e #W you."},

	{"$n barely #W $N.",	/* 5.. 8  */
	 "You barely #W $N.",
	 "$n barely #W you."},

	{"$n #W $N.",	/* 9.. 12  */
	 "You #W $N.",
	 "$n #W you."},

	{"$n #W $N hard.",	/* 13..20  */
	 "You #W $N hard.",
	 "$n #W you hard."},

	{"$n #W $N very hard.",		/* 21..30  */
	 "You #W $N very hard.",
	 "$n #W you very hard."},

	{"$n #W $N extremely hard.",	/* 31..40  */
	 "You #W $N extremely hard.",
	 "$n #W you extremely hard."},

	{"$n massacre $N to small fragments with $s #W.",	/* > 40    */
	 "You massacre $N to small fragments with your #W.",
	 "$n massacre you to small fragments with $s #W."},

	{"$n annihilates $N to micro molecules with $s #W.",	/* > 65    */
	 "You annihilate $N to micro molecules with your #W.",
	 "$n annihilates you to micro molecules with $s #W."},

	{"$n disintegrates $N with $s #W.",	/* > 110 */
	 "You disintegrate $N with your #W.",
	 "$n disintegrates you with $s #W."},

	{"$n sends $N void with $s #W.",	/* > 150 */
	 "You send $N to void with your #W.",
	 "$n sends you to void with $s #W."},

	{"$n tickles $N void with $s #W.",	/* > 200 */
	 "You tickle $N to void with your #W.",
	 "$n tickles you to void with $s #W."},

    /* Furfuri made massage */

	{"$n shoots $N with laser power $s #W.",	/* > 250 */
	 "You shoot $N with laser power #W.",
	 "$n shoots you with laser power $s #W."},

	{"$n strikes $N with NUCLEAR power $s #W.",
	 "You strike $N with NUCLEAR power #W.",
	 "$n strikes you with NUCLEAR power $s #W."},

	{"$n CRASHES $N with -= S P A C E   P O W E R =- $s #W.",
	 "YOU CRASH $N with -= S P A C E   P O W E R =- #W.",
	 "$n CRASHES YOU with -= S P A C E   P O W E R =- $s #W."},

	{"$n PROCESSES $N into ---==<< V A C U U M >>==--- with $s #W.",
	 "YOU PROCESS $N into ---==<< V A C U U M >>==--- with your #W.",
	 "$n PROCESSES YOU into ---==<< V A C U U M >>==--- with $s #W."}
    };

    struct dam_weapon_type han_weapons[] =
    {
	{"$n 님이 $N 님을 때리려다 빗나갔습니다.",	/* 0    */
	 "당신은 $N 님을 때리지 못했습니다.",
	 "$n 님이 당신을 때리려다 실패했습니다."},

	{"$n 님이 $N 님을 간지럽게 #W.",	/* 1.. 4  */
	 "당신은 $N 님을 간지럽게 #W.",
	 "$n 님이 당신을 간지럽게 #W."},

	{"$n 님이 $N 님을 살살 #W.",	/* 5.. 8  */
	 "당신은 $N 님을 살살 #W.",
	 "$n 님이 당신을 살살 #W."},

	{"$n 님이 $N 님을 #W.",		/* 9.. 12  */
	 "당신은 $N 님을 #W.",
	 "$n 님이 당신을 #W."},

	{"$n 님이 $N 님을 세게 #W.",	/* 13..20  */
	 "당신은 $N 님을 세게 #W.",
	 "$n 님이 당신을 세게 #W."},

	{"$n 님이 $N 님을 매우 세게 #W.",	/* 21..30  */
	 "당신은 $N 님을 매우 세게 #W.",
	 "$n 님이 당신을 매우 세게 #W."},

	{"$n 님이 $N 님을 무지막지하게 #W.",	/* 31..40  */
	 "당신은 $N 님을 무지막지하게 #W.",
	 "$n 님이 당신을 무지막지하게 #W."},

	{"$n 님이 $N 님을 뼈가 으스러지게 #W.",		/* > 40    */
	 "당신은 $N 님을 뼈가 으스러지게 #W.",
	 "$n 님이 당신을 뼈가 으스러지게 #W."},

	{"$n 님이 $N 님을 몸이 산산조각이 나게 #W.",	/* > 65    */
	 "당신은 $N 님을 몸이 산산조각이 나게 #W.",
	 "$n 님이 당신을 몸이 산산조각이 나게 #W."},

	{"$n 님이 $N 님을 몸이 가루가 되게 #W.",	/* > 110    */
	 "당신은 $N 님을 몸이 가루가 되게 #W.",
	 "$n 님이 당신을 몸이 가루가 되게 #W."},

	{"$n 님이 $N 님을 눈앞에 별이 돌게#W.",		/* > 150 */
	 "당신은 $N 님을 눈앞에 별이 돌게#W.",
	 "$n 님이 당신을 눈앞에 별이 돌게#W."},

	{"$n 님이 $N 님을 눈앞에 병아리가 돌게#W.",	/* > 200 */
	 "당신은 $N 님을 눈앞에 병아리가 돌게#W.",
	 "$n 님이 당신을 눈앞에 병아리가 돌게#W."},

	{"$n 님이 $N 님을 소리없이#W.",		/* > 250 */
	 "당신은 $N 님을 소리없이#W.",
	 "$n 님이 당신을 소리없이#W."},

	{"$n 님이 $N 님을 레이져 파워로 #W.",
	 "당신은 $N 님을 레이져 파워로 #W.",
	 "$n 님이 당신을 레이져 파워로 #W."},

	{"$n 님이 $N 님을 전..우주의 힘을 빌어 #W.",
	 "당신은 $N 님을 전 ..우주의 힘을 빌어 #W.",
	 "$n 님이 당신을 전 ..우주의 힘을 빌어 #W."},

	{"$n 님이 $N 님을 진공상태로 되도록 #W.",
	 "당신은 $N 님을 진공상태로 되도록 #W.",
	 "$n 님이 당신을 진공상태가 되도록 #W."}
    };

/* NOTE: Weapon attack texts from damage() in "fight.c" */
struct attack_hit_type attack_hit_text[] =
{
    {"hit", "hits"},		/* TYPE_HIT      */
    {"slash", "slashes"},	/* TYPE_SLASH    */
    {"pierce", "pierces"},	/* TYPE_PIERCE   */
    {"pound", "pounds"},	/* TYPE_BLUDGEON */
    {"crush", "crushs"}		/* TYPE_CRUSH    */
    {"whip", "whips"},		/* TYPE_WHIP     */
    {"bite", "bites"},		/* TYPE_BITE     */
    {"sting", "stings"},	/* TYPE_STING    */
    {"shoot", "shoots"},	/* TYPE_SHOOT    */
};

struct attack_hit_type attack_hit_han[] =
{
    {"칩니다", "칩니다"},		/* TYPE_HIT      */
    {"벱니다", "벱니다"},		/* TYPE_SLASH    */
    {"찌릅니다", "찌릅니다"},		/* TYPE_PIERCE   */
    {"난타 합니다", "난타 합니다"},	/* TYPE_BLUDGEON */
    {"부숩니다", "부숩니다"}		/* TYPE_CRUSH    */
    {"채찍질 합니다", "채찍질 합니다"},	/* TYPE_WHIP     */
    {"물어 뜯습니다", "물어 뜯습니다"},	/* TYPE_BITE     */
    {"톡 쏩니다", "톡 쏩니다"},		/* TYPE_STING    */
    {"쏩니다", "쏩니다"},		/* TYPE_SHOOT    */
};

#endif	/* OLD_DAM_MESSAGE */

