/*
guild_list.h
made by Process
*/
#define MAX_GUILD_LIST	3
#define POLICE	1
#define OUTLAW  2
#define ASSASSIN 3

#define ROOM_GUILD_POLICE	3095
#define ROOM_GUILD_OUTLAW	3221
#define ROOM_GUILD_ASSASSIN	9650

#define ROOM_GUILD_POLICE_LOCKER	3099
#define ROOM_GUILD_OUTLAW_LOCKER	3224 
#define ROOM_GUILD_ASSASSIN_LOCKER	9652

#define ROOM_GUILD_POLICE_PRACTICE 	3096
#define ROOM_GUILD_OUTLAW_PRACTICE	3222
#define ROOM_GUILD_ASSASSIN_PRACTICE 9651

#define GET_GUILD(x) ((x)->player.guild)
#define GET_GUILD_SKILLS(x,i) ((x)->player.guild_skills[(i)])
#define GET_GUILD_NAME(i) (guild_names[(i)])
#define GET_PK(x) ((x)->player.pk_num)
#define GET_GUILD_SKILL(x,i) ((x)->player.guild_skills[(i)])

/* start of police */
#define POLICE_SKILL_WHISTLE	0
#define POLICE_SKILL_SIMULTANEOUS	1
#define POLICE_SKILL_ARREST		2
#define POLICE_SKILL_POWER_BASH	3
/* start of outlaw */
#define OUTLAW_SKILL_ANGRYYELL	0
#define OUTLAW_SKILL_ASSAULT    1
#define OUTLAW_SKILL_SMOKE		2
#define OUTLAW_SKILL_INJECT		3
#define OUTLAW_SKILL_CHARGE		4
/* start of assasin */
#define ASSASSIN_SKILL_SOLACE	0
#define ASSASSIN_SKILL_SHADOW_FIGURE	1
#define ASSASSIN_SKILL_EVIL_STRIKE 2
#define ASSASSIN_SKILL_IMPROVED_REFLECT_DAMAGE 3
