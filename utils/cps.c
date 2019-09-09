#include <stdio.h>
#include <string.h>
#include <ctype.h>

#ifndef __FreeBSD__
#include <crypt.h>
#endif 
#include <unistd.h>

#include "structs.h"

extern char *spells[];

#define PROMPT			fprintf(stdout, "CHECK PLAYER >> ");

#define ERROR(s)		fprintf(stderr, "        ERROR : %s", s);
#define MESSAGE(f,s)	fprintf(stdout, "      MESSAGE : ");fprintf(stdout, f, s);
#define DEBUG(f,s)		/* fprintf(stderr, "        DEBUG : ");fprintf(stderr, f, s); */

typedef struct {
	char *name;
	int (*func)();
} COMMAND;

int changed = 0;
int loaded = 0;

FILE *fp;
int offset;
int fd;

/* Used in CHAR_FILE_U *DO*NOT*CHANGE* */
struct char_old_skill_data {
	ubyte learned;           /* % chance for success 0 = not learned   */
	bool recognise;         /* If you can recognise the scroll etc.   */
};

/* for new */
/* check do_new(arguments) */
/*
struct char_old_file_u {
};
struct char_old_file_u ch;
*/

struct char_file_u ch;

char sex[4] = "NMF";
char class[5] = "MCTW";

int do_bank(char *);
int do_find(char *);
int do_passwd(char *);
int do_quit(char *);
int do_room(char *);
int do_save(char *);
int do_skill(char *);
int do_show(char *);
int do_level(char *);
int do_set(char *);
int do_help(char *);
int do_temp(char *);
int do_new(char *);
int do_affected(char *);

char *first(char *str, char *buf);

#define MaxCommand	13
COMMAND commands[MaxCommand] =
{
	{"affected", do_affected},
	{"bank", do_bank},
	{"find", do_find},
	{"help", do_help},
	{"new", do_new},
	{"passwd", do_passwd},
	{"quit", do_quit},
	{"room", do_room},
	{"save", do_save},
	{"set", do_set},
	{"skill", do_skill},
	{"show", do_show},
	{"temp", do_temp},
};

void new_char(struct char_file_u *ch, struct char_file_u *ch2)
{
	int i;

	bcopy(ch, ch2, sizeof(struct char_file_u));

	for(i=0;i<MAX_AFFECT;i++){
		ch2->affected[i].type = 0;
		ch2->affected[i].duration = 0;
		ch2->affected[i].modifier = 0;
		ch2->affected[i].location = 0;
		ch2->affected[i].bitvector = 0;
		ch2->affected[i].next = 0;
	}

#ifdef TTTT
	bzero(ch2, sizeof(struct char_file_u));

	ch2->sex = ch->sex;
	ch2->class = ch->class;
	ch2->level = ch->level;

	ch2->birth = ch->birth;
	ch2->played = ch->life;
	ch2->weight = ch->weight;
	ch2->height = ch->height;
	bcopy(ch->title, ch2->title, 80);

	/* guild skill */
	for(i = 0; i < MAX_GUILD_SKILLS; i ++){
		ch2->guild_skills[i] = ch->guild_skills[i];
	}

	bcopy(ch->description, ch2->description, 300);

	ch2->pked_num = ch->pked_num;
	ch2->no_of_change_guild = ch->no_of_change_guild;
	ch2->guild = ch->guild;
	ch2->pk_num = ch->pk_num;
	ch2->load_room = ch->load_room;

	/* abilities */
	bcopy((char *)&(ch->abilities), (char *)&(ch2->abilities),
			sizeof(struct char_ability_data));
	/* points */
	bcopy((char *)&(ch->points), (char *)&(ch2->points),
			sizeof(struct char_point_data));
	/* skills */
	for(i = 0; i < MAX_SKILLS; i ++){
		ch2->skills[i].learned = ch->skills[i].learned;
		ch2->skills[i].recognise = ch->skills[i].recognise;

		ch2->skills[i].skilled = 0;
	}
	/* affected */
	for(i = 0; i < MAX_AFFECT; i ++){
		ch2->affected[i].location = ch->affected[i].location;
		ch2->affected[i].modifier = ch->affected[i].modifier;
	}

	ch2->spells_to_learn = ch->spells_to_learn;
	ch2->alignment = ch->alignment;
	ch2->last_logon = ch->last_logon;
	ch2->act = ch->act;
	ch2->bank = ch->bank;
	bcopy(ch->name, ch2->name, 20);
	bcopy(ch->pwd, ch2->pwd, 11);

	/* conditions */
	for(i = 0; i < 3; i ++){
		ch2->conditions[i] = ch->conditions[i];
	}

	/* quest */
	ch2->quest.type = 0;
	ch2->quest.data = 0;
	ch2->quest.solved = 0;

	/* reserved */
#endif /* TTTT */
}

int do_affected(char *str)
{
	int i;

	if(!loaded){
		ERROR("not loaded\n");
		return -1;
	}

	for(i=0;i<MAX_AFFECT;i++){
		printf("\ttype : %d\n", ch.affected[i].type);
		printf("\t\tduration : %d,", ch.affected[i].duration);
		printf("modifier : %d\n", ch.affected[i].modifier);
	}
	return 0;
}

int do_help(char *str)
{
	printf("\tbank : \n");
	printf("\tfind : \n");
	printf("\thelp : \n");
	printf("\tnew : \n");
	printf("\tpasswd : \n");
	printf("\tquit : \n");
	printf("\troom : \n");
	printf("\tsave : \n");
	printf("\tset : \n");
	printf("\t\tclass, level, hit, mana, move, hitroll, damroll, abilities, conditions \n");
	printf("\tskill : \n");
	printf("\tshow : \n");
	return 0;
}

int do_bank(char *str)
{
	int num;

	if(!loaded){
		ERROR("not loaded\n");
		return -1;
	}

	if(!*str){
		ERROR("argument is needed.\n");
	}

	num = atoi(str);

	ch.bank = num;
	changed = 1;
	return 0;
}

int do_set(char *str)
{
	int num;
	char buf[256];

	if(!loaded){
		ERROR("not loaded\n");
		return -1;
	}

	if(!*str){
		ERROR("argument is needed.\n");
	}

	str = first(str, buf);
	if(!strncmp("level", buf, strlen(buf))){
		num = atoi(str);
		ch.level = num;
		changed = 1;
	}
	else if(!strncmp("class", buf, strlen(buf))){
		changed = 1;
		if(*str == 'M') ch.class = 1;
		else if(*str == 'C') ch.class = 2;
		else if(*str == 'T') ch.class = 3;
		else if(*str == 'W') ch.class = 4;
		else {
			ERROR("invalid class.\n");
			changed = 0;
		}
	}
	else if(!strncmp("hit", buf, strlen(buf))){
		num = atoi(str);
		ch.points.max_hit = num;
		ch.points.hit = num;
		changed = 1;
	}
	else if(!strncmp("mana", buf, strlen(buf))){
		num = atoi(str);
		ch.points.max_mana = num;
		ch.points.mana = num;
		changed = 1;
	}
	else if(!strncmp("move", buf, strlen(buf))){
		num = atoi(str);
		ch.points.max_move = num;
		ch.points.move = num;
		changed = 1;
	}
	else if(!strncmp("hitroll", buf, strlen(buf))){
		num = atoi(str);
		ch.points.hitroll = num;
		changed = 1;
	}
	else if(!strncmp("damroll", buf, strlen(buf))){
		num = atoi(str);
		ch.points.damroll = num;
		changed = 1;
	}
	else if(!strncmp("hit", buf, strlen(buf))){
		num = atoi(str);
		ch.points.max_hit = num;
		ch.points.hit = num;
		changed = 1;
	}
	else if(!strncmp("abilities", buf, strlen(buf))){
		num = atoi(str);
		ch.abilities.str = 18;
		ch.abilities.str_add = 100;
		ch.abilities.intel = 18;
		ch.abilities.wis = 18;
		ch.abilities.dex = 18;
		ch.abilities.con = 18;
		changed = 1;
	}
	else if(!strncmp("handno", buf, strlen(buf))){
		num = atoi(str);
		ch.damnodice = num;
		changed = 1;
	}
	else if(!strncmp("handsize", buf, strlen(buf))){
		num = atoi(str);
		ch.damsizedice = num;
		changed = 1;
	}
	else if(!strncmp("quest", buf, strlen(buf))){
		num = atoi(str);
		ch.quest.solved = num;
		changed = 1;
	}
	else if(!strncmp("conditions", buf, strlen(buf))){
		ch.conditions[0] = -1;
		ch.conditions[1] = -1;
		ch.conditions[2] = -1;
		changed = 1;
	}
	else {
		changed = 0;
	}
	return 0;
}

int do_temp(char *str)
{
	// long of;

	fp = fopen("players", "r+");
	if(!fp){
		ERROR("can't open players\n");
		return -1;
	}

	while(!feof(fp)){
		/* of = */ ftell(fp);
		fread(&ch, sizeof(struct char_file_u), 1, fp);
DEBUG("Name = %s\n", ch.name);
DEBUG("\tAct = %d\n", ch.act);
		if(ch.act & 1) ERROR("hmm.\n");
		/*
DEBUG("\tNew Act = %d\n", ch.act);
		fseek(fp, of, 0);
		*/
		fwrite(&ch, sizeof(struct char_file_u), 1, fp);
	}
	fclose(fp);

	return 1;
}

int do_find(char *str)
{
	// int num;

	if(!*str){
		ERROR("argument is needed.\n");
	}

	fp = fopen("players", "rb");
	if(!fp){
		ERROR("can't open players\n");
		return -1;
	}

	while(!feof(fp)){
		if(fread(&ch, sizeof(struct char_file_u), 1, fp) == 0) continue;
DEBUG("Name = %s\n", ch.name);
		if(!strcmp(str, ch.name)){
			MESSAGE("%s Found\n", ch.name);
			offset = ftell(fp);
			offset -= sizeof(struct char_file_u);
			fclose(fp);
			loaded = 1;
			return 0;
		}
	}

	fclose(fp);

	ERROR("can't find player.\n");

	return -1;
}

int do_new(char *str)
{
	// int num;
	FILE *fp2;
	struct char_file_u ch2;

	fp = fopen("players", "rb");
	fp2 = fopen("players.new", "wb");

	if(!fp){
		ERROR("can't open players\n");
		return -1;
	}
	if(!fp2){
		ERROR("can't open players.new\n");
		return -1;
	}

	while(!feof(fp)){
		fread(&ch, sizeof(struct char_file_u), 1, fp);
		if(!feof(fp)){
DEBUG("Name = %s\n", ch.name);
			new_char(&ch, &ch2);
			fwrite(&ch2, sizeof(struct char_file_u), 1, fp2);
		}
	}

	fclose(fp);
	fclose(fp2);

	MESSAGE("%s is made\n", "players.new");

	return 0;
}

int do_passwd(char *str)
{
	//  int num;

	if(!loaded){
		ERROR("not loaded\n");
		return -1;
	}

	if(!*str){
		ERROR("argument is needed.\n");
	}

	strcpy(ch.pwd, (char *)crypt(str, ch.name));

	changed = 1;

	return 0;
}

int do_quit(char *str)
{
	// int num;

	if(changed){
		do_save(str);
	}

	exit(0);
}

int do_room(char *str)
{
	int num;

	if(!loaded){
		ERROR("not loaded\n");
		return -1;
	}

	if(!*str){
		ERROR("argument is needed.\n");
	}

	num = atoi(str);

	ch.load_room = num;
	changed = 1;

	return 0;
}

int do_save(char *str)
{
	// int num;

	if(!loaded){
		ERROR("not loaded\n");
		return -1;
	}

	fp = fopen("players", "r+");
	if(!fp){
		ERROR("can't open players\n");
		return -1;
	}

	fseek(fp, offset, 0);
	fwrite(&ch, sizeof(struct char_file_u), 1, fp);
	fclose(fp);

	changed = 0;

	return 0;
}

int do_skill(char *str)
{
	int /* num, */ i;

	if(!loaded){
		ERROR("not loaded\n");
		return -1;
	}

	if ( !*str ) {
		/* show skills */
		fprintf(stdout, "SHOW SKILLS ...\n\n");
		for(i = 1; *spells[i-1] != '\n'; i ++){
			if (*spells[i-1]==0) continue;
			fprintf(stdout, "[%3d] %20s : (%3d,%3d), %s\n",
				i, spells[i-1], ch.skills[i].learned, ch.skills[i].skilled,
				(ch.skills[i].recognise)?"RECOGNISE" : "NOT RECOGNISE");
		}
	}
	else {
		int	len, flag = 0;
		len = strlen(str);
		if ( strcmp( str, "all") == 0 ) {
			for(i = 1; *spells[i-1] != '\n'; i ++){
				if (*spells[i-1]==0) continue;
				ch.skills[i].learned = 100;
				ch.skills[i].skilled = 255;
			}
		}
		else {
			for(i = 1; *spells[i-1] != '\n'; i ++){
				if (*spells[i-1]==0) continue;
				if ( strncmp( str, spells[i-1], len ) == 0 ) {
					flag = 1;
					break;
				}
			}
			if ( flag == 0 ) {
				fprintf( stderr, "'%s' does not exist\n", str );
				return	-1;
			}
			fprintf(stdout, "[%3d] %20s : (%3d,%3d), %s\n",
					i, spells[i-1], ch.skills[i].learned, ch.skills[i].skilled,
					(ch.skills[i].recognise)?"RECOGNISE" : "NOT RECOGNISE");
			printf("Learned: ");
			scanf("%d", &flag);
			ch.skills[i].learned = flag;
			fflush(stdin);
			printf("Skilled: ");
			scanf("%d", &flag);
			ch.skills[i].skilled = flag;
			fflush(stdin);
		}
	}

	return 0;
}

int do_show(char *str)
{
	// int num;
	// char buf[256];

	if(!loaded){
		ERROR("not loaded\n");
		return -1;
	}

	fprintf(stdout, "Name      : %s\n", ch.name);
	fprintf(stdout, "Title     : %s\n", ch.title);
	fprintf(stdout, "Sex       : %c, Class : %c, Level : %d\n",
			sex[(int)ch.sex], class[ch.class - 1], ch.level);
	fprintf(stdout, "Load Room : %d, Exp: %lld, Bank : %lld\n",
			ch.load_room, ch.points.exp, ch.bank);
	fprintf(stdout, "Wimpyness : %d\n", ch.wimpyness);
	fprintf(stdout, "Str: %d/%d Int:%d Wis %d Dex:%d Con:%d\n", 
			ch.abilities.str, ch.abilities.str_add, ch.abilities.intel,
			ch.abilities.wis, ch.abilities.dex, ch.abilities.con);
	fprintf(stdout, "Hitroll: %d, Damroll: %d\n",
			ch.points.hitroll, ch.points.damroll);
	fprintf(stdout, "bare hand dice : %dd%d\n", ch.damnodice, ch.damsizedice);
	/* fprintf(stdout, "Remortal : %d\n", ch.remortal); */
	fprintf(stdout, "Remortal : ");
	if(ch.remortal & REMORTAL_MAGIC_USER) fprintf(stdout, "M");
	if(ch.remortal & REMORTAL_CLERIC) fprintf(stdout, "C");
	if(ch.remortal & REMORTAL_THIEF) fprintf(stdout, "T");
	if(ch.remortal & REMORTAL_WARRIOR) fprintf(stdout, "W");
	fprintf(stdout,"\n");
	fprintf(stdout, "Quest Solved : %ld\n", ch.quest.solved);

	return 0;
}

char *first(char *str, char *buf)
{
	int n;

	n = 0;
	while(str[n] && !isspace(str[n])) n++;
	strncpy(buf, str, n);
	buf[n] = 0;

	/* Clear Space Character */
	while(str[n] && isspace(str[n])) n++;

	return (&str[n]);
}

void doCommand(char *str)
{
	int i;
	char buf[256];

	str = first(str, buf);

	for(i = 0; i < MaxCommand; i ++){
		if(!strncmp(commands[i].name, buf, strlen(buf))){
			(commands[i].func)(str);
			break;
		}
	}
}

//main(void)
//{
//	char str[256];
//
//	changed = 0;
//	loaded = 0;
//
//	while(1){
//		PROMPT;
//		gets(str);
//		if(!*str) continue;
//
//		doCommand(str);
//	}
//}

int main(void)
{
	char str[256];
	FILE *fp;

	changed = 0;
	loaded = 0;

	fp = stdin;
	while(1){
		PROMPT;
		fgets(str,256,fp);
		if(!*str) continue;

		doCommand(str);
	}
}

// include "spells_array.c"
// from src/spell_parser.c

char *spells[]=
{
   "armor",               /* 1 */
   "teleport",
   "bless",
   "blindness",
   "burning hands",
   "call lightning",
   "charm person",
   "chill touch",
   "reanimate",
   "color spray",
   "relocate",     /* 11 */
   "create food",
   "create water",
   "cure blind",
   "cure critic",
   "cure light",
   "curse",
   "detect evil",
   "detect invisibility",
   "recharger",
   "preach",       /* 21 */
   "dispel evil",
   "earthquake",
   "enchant weapon",
   "energy drain",
   "fireball",
   "harm",
   "heal",
   "invisibility",
   "lightning bolt",
   "locate object",      /* 31 */
   "magic missile",
   "poison",
   "protection from evil",
   "remove curse",
   "sanctuary",
   "shocking grasp",
   "sleep",
   "strength",
   "summon",
   "ventriloquate",      /* 41 */
   "word of recall",
   "remove poison",
   "sense life",         /* 44 */
   "sunburst",
   "clone",
   "",
   "",
   "",
   "",
   "", /* 51 */
   "",
   "identify",
   "",
   "sneak",        /* 55 */
   "hide",
   "steal",
   "backstab",
   "pick",
   "kick",         /* 60 */
   "bash", /* 61 */
   "rescue",
   "double attack",
   "quadruple attack",
   "extra damaging",
   "",
   "",
   "",
   "",
   "",
   "damage up", /* 71 */
   "energy flow",
   "mana boost",
   "vitalize",
   "full fire",
   "throw",		/* 76 */
   "firestorm",
   "crush armor",
   "full heal",
   "trick",
   "haste",      /* 81 */
   "improved haste",
   "far look",
   "all heal",
   "tornado",        /* 85  */
   "lightning move",
   "parry",
   "flash",
   "multi kick",
   "enchant armor",  /* 90 */
   "pray for armor", /* 91 */
   "infravision",
   "create nectar",
   "create golden nectar",
   "cone of ice",
   "disintegrate",
   "phase",
   "mirror image",
   "hand of god",
   "cause light", /* 100 */
   "cause critic", /* 101 */
   "entire heal",
   "octa attack",
   "love",
   "disarm", /* 105 */
   "reraise",
   "shouryuken", /* 107 */
   "throw object", /* 108 */
   "punch", /* 109 */
   "death", /* 110 */
   "enchant person", /* 111 */
   "spell block", /* 112 */
   "temptation", /* 113 */
   "shadow figure", /* 114 */
   "mana transfer",
   "self heal",
   "holy shield",
   "restore move",
   "heal the world",
   "reflect damage",
   "dumb",
   "spin bird kick",
   "thunderbolt",
   "arrest",
   "sanctuary cloud", /* 125 */
   "\n"
};
