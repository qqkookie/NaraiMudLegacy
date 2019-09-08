#include "structs.h"
#include "limit.h"
#include "guild_list.h"

char *guild_names[] =
{
	"NONE",
	"POLICE",
	"OUTLAW",
	"ASSASSIN"
};

/*
	useless guild skills
*/
int guild_skill_nums[] =
{
	0,
	0,
	0
};
/*
int guild_skill_nums[] =
{
	4,
	5,
	4
};
*/

char *police_skills[] =
{
	"broadcast",
	"simultaneous",
	"arrest",
	"power bash"
};

int police_skill_costs[] =
{
	5000,
	10000,
	10000,
	20000,
};

char *outlaw_skills[] =
{
	"angry yell",
	"assault",
	"smoke",
	"inject",
	"charge"
};

int outlaw_skill_costs[] =
{
	10000,
	12000,
	1000,
	2000,
	50000
};
char *assasin_skills[] =
{
	"solace",
	"shadow figure",
	"evil strike",
	"improved reflect damage"
};

int assasin_skill_costs[] =
{
	12000,
	15000,
	20000,
	100000
};
