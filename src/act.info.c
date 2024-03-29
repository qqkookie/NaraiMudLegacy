/* **************************************************************************
   *  file: act.info.c , Implementation of commands.        Part of DIKUMUD *
   *  Usage : Commands about world, players info. help, time and weather    *
   *	NOTE: Merged OLD weather.c and help related procs.		    *
   ************************************************************************* */
/*
   this file is for version control of mud.
   and fileid is also used in 'show version'.
 */
char version_id[] =
"@(#) KIT Clasic version v4.00  2019/10/05  by Cookie (cantata@gmail.com)\r\n"
" New Narai 1998  v2.92.3   97/12/09  by Cookie.\r\n"
" Forked from Narai  v2.90   97/09/11  by cookie (cookie0@chollian.net)\r\n"
" Legacy v2.00   94/10/10  Greatly Improved by process(wdshin@eve)\r\n"
"        v1.13   94/04/13  by Source Manager Cold.\r\n" ;

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <time.h>

#include "char.h"
#include "object.h"
#include "global.h"
#include "comm.h"
#include "play.h"
#include "gamedb.h"


void do_who(struct char_data *ch, char *argument, int cmd)
{
    struct descriptor_data *d;
    char buf[256], buf2[256], class;
    int num_player = 0, tlev, lev, tz, z, flag, condtype;
    char page_buffer[MAX_STRING_LENGTH];
    char *name = NULL;

    tlev = lev = tz = z = flag = 0;

    condtype = 0;
    if (argument) {
	while (argument[0] == ' ') ++argument;

	if (!*argument)
	    condtype = 0;
	else if (isalpha(*argument)) {
	    /* name search */
	    condtype = 5;
	    name = argument;
	}
	else {
	    switch (argument[0]) {
	    case '+':
		condtype = 1; ++argument; break;
	    case '-':
		condtype = 2; ++argument; break;
	    case '=':
		condtype = 3; ++argument; break;
	    case '*':
		condtype = 4; ++argument;
		z = world[ch->in_room].zone;
		break;
	    default:
		condtype = 1;
	    }
	}
	if (condtype > 0 && condtype < 4)
	    lev = atoi(argument);
    }

    strcpy(page_buffer,
	    STRHAN( "Players\r\n-------\r\n", "사람들\r\n-------\r\n", ch));

    for (d = descriptor_list; d; d = d->next) {
	if (!d->connected && CAN_SEE(ch, d->character)) {
	    if (condtype) {
		if (condtype < 4)
		    tlev = d->original ? GET_LEVEL(d->original) :
			GET_LEVEL(d->character);
		else if (condtype == 4)
		    tz = d->original ? world[d->original->in_room].zone :
			world[d->character->in_room].zone;
		else {	/* name find */
		    name = d->original ? GET_NAME(d->original) :
			GET_NAME(d->character);
		}

		switch (condtype) {
		case 1:
		    flag = (tlev >= lev); break;
		case 2:
		    flag = (tlev <= lev); break;
		case 3:
		    flag = (tlev == lev); break;
		case 4:
		    flag = (tz == z); break;
		case 5:
		    flag = (isname(argument, name));
		}
		if (!flag)
		    continue;
	    }
#ifdef UNUSED_CODE
/* NOTE: following code is obsolete, so it is removed */
	    if (d->character) {
		switch (GET_CLASS(d->character)) {
		case CLASS_MAGIC_USER:
		    class = 'M';
		    break;
		case CLASS_CLERIC:
		    class = 'C';
		    break;
		case CLASS_THIEF:
		    class = 'T';
		    break;
		case CLASS_WARRIOR:
		    class = 'W';
		    break;
		default:
		    class = 'U';
		}
/*NOTE: following code is obsolete, so it is removed */
/*
	       if (is_korean)
		   sprintf(buf, "< %c %2d > %s %s", class,
		   GET_LEVEL(d->character), GET_NAME(d->character),
		   d->character->player.title);
	       else
 */
		sprintf(buf, "< %c %2d > %s %s", class, GET_LEVEL(d->character),
			  GET_NAME(d->character), GET_TITLE(d->character));
	    }
 #endif		/* UNUSED_CODE */

	    /*NOTE: BUG FIX: GET_TITLE(ch) may be null */
	    if (d->original) {  /* If switched */
		/* NOTE: simplified CLASS code printing */
		class = "UMCTWU"[GET_CLASS(d->original)];
		sprintf(buf, "< %c %2d > %s %s", class,
		    GET_LEVEL(d->original), GET_NAME(d->original),
		    (GET_TITLE(d->original) ? GET_TITLE(d->original) : ""));
	    }
	    else if (d->character) {
		    class = "UMCTWU"[GET_CLASS(d->character)];
		    sprintf(buf, "< %c %2d > %s %s", class,
		    GET_LEVEL(d->character), GET_NAME(d->character),
		    (GET_TITLE(d->character) ? GET_TITLE(d->character): ""));

		    if (IS_SET(d->character->specials.act, PLR_CRIMINAL))
			strcat(buf, " (CRIMINAL)");

		if (IS_MORTAL(ch)) {
		    sprintf(buf2, " PK#(%d)\r\n", d->character->player.pk_num);
		    strcat(buf, buf2);
		}
		else if ( d->character->player.guild <= MAX_GUILD_LIST) {
		    sprintf(buf2," PK#(%d) %s\r\n", d->character->player.pk_num,
			    guild_names[d->character->player.guild]);
		    strcat(buf, buf2);
		}

	    }
	    if (!IS_AFFECTED(d->character, AFF_SHADOW_FIGURE))
		strcat(page_buffer, buf);
	    else {
		if (IS_MORTAL(ch))
		    strcat(page_buffer, "A shadow figure\r\n");
		else
		    strcat(page_buffer, buf);
	    }
	    num_player++;
	}
    }
    sprintf( buf, STRHAN("\r\nYou can see %d players.\r\n",
			 "\r\n%d 명이 있습니다.\r\n", ch), num_player);
    strcat(page_buffer, buf );
    page_string(ch->desc, page_buffer, 1);
}

int list_users(struct char_data *ch, char *line );

void do_users(struct char_data *ch, char *argument, int cmd)
{
    char line[MAX_STRING_LENGTH];
    int	t;
    extern time_t	boottime;

    one_argument(argument, line);
    if (IS_WIZARD(ch) && (strcmp("-t", line) != 0)) {
	strcpy( line, "------------ ------------\r\n" );
	list_users( ch, line + strlen(line) );
	strcat(line , "\r\n" );
	page_string(ch->desc, line, 1);
    }

/* NOTE: do_users(), recored_player_number() have similar code.
	Common code is merged into list_users().	*/
#ifdef	UNUSED_CODE
    line[0] = 0;
    for (d = descriptor_list; d; d = d->next) {
	++m;
	if (flag)
	    continue;
	if (d->original) {
	    if (!CAN_SEE(ch, d->original))
		continue;
	    sprintf(line + strlen(line), "%3d%2d:", d->descriptor,
		    d->original->specials.timer);
	    sprintf(line + strlen(line), "%-14s%2d ",
		    GET_NAME(d->original), GET_LEVEL(d->original));
	}
	else if (d->character) {
	    if (!CAN_SEE(ch, d->character))
		continue;
	    sprintf(line + strlen(line), "%3d%2d:", d->descriptor,
		    d->character->specials.timer);
	    sprintf(line + strlen(line), "%-14s%2d ",
		    (d->connected == CON_PLYNG)
		    ? GET_NAME(d->character) : "Not in game",
		    GET_LEVEL(d->character));
	}
	else
	    sprintf(line + strlen(line), "%3d%9s%10s ",
		 d->descriptor, "  UNDEF  ", connected_types[d->connected]);
	sprintf(line + strlen(line), "%-15s", d->host);
	if (!(n % 2)) {
	    strcat(line, "|");
	}
	else {
	    strcat(line, "\r\n");
	    send_to_char(line, ch);
	    line[0] = 0;
	}
	++n;
    }
    if ((!flag) && (n % 2)) {
	strcat(line, "\r\n");
	send_to_char(line, ch);
    }
    if (m > most)
	most = m;
    sprintf(line, "%s%d/%d active connections\r\n",
	    (n % 2) ? "\r\n" : "", m, most);
    if (IS_GOD(ch))
	send_to_char(line, ch);
#endif		/* UNUSED_CODE */

    t = 30 + time(0) - boottime;
    sprintf(line, STRHAN("Running time:   %d:%02d\r\n",
	"현재  %d시간 %02d분 지났습니다.\r\n", ch), t / 3600, (t % 3600) / 60);
    send_to_char(line, ch);
}

/* NOTE: NEW! list active user and thier connection.
	Merge common function of do_users() and recored_players_number(). */
int list_users(struct char_data *ch, char *line )
{
    char room[30], *buf;
    struct descriptor_data *d;
    struct char_data *pl ;
    /* struct in_addr  addr;
    struct hostent  *h_ent; */
    static int most = 0;
    extern char *connected_types[];

    int m = 0 ;
    buf = line;

    for (d = descriptor_list; d; d = d->next) {
	++m;
	/* NOTE: Old was too wide. list one connection per line, not two. */
	if ( d->original )
	    pl = d->original;
	else if ( d->character )
	    pl = d->character;
	else
	    pl = NULL ;

	if ( pl ) {
	    if ( ch && !(CAN_SEE(ch, pl ))) 	/* NOTE: for do_user() */
		continue;
	    /* NOTE: Show room name  : Truncated at length 24.    */
	    if (( d->connected == CON_PLYNG ) && ( pl->in_room > NOWHERE )) {
		    strncpy ( room, world[pl->in_room].name, 24 );
		    room[24] = '\0' ;
	    }
	    else
		    strcpy( room, "No Where" );
	    sprintf(buf, "%2d: < %c %2d > %-12s (%2d) %-24s ",
		    d->descriptor, "UMCTWU"[GET_CLASS(pl)], GET_LEVEL(pl),
		    ( d->connected == CON_PLYNG ) ?
			GET_NAME(d->character) : "Not in game",
		    pl->specials.timer, room );
	}
	else
	    sprintf(buf , "%2d: %5s    %-17s %-24s ",
		d->descriptor, "UNDEF", connected_types[d->connected], "");
	buf += strlen(buf);

#ifdef UNUSED_CODE
	/* NOTE: Show domain name instead of numeric IP addr, if possible.*/
	/* NOTE: inet_makeaddr() is more portable than inet_aton().    */

	addr = inet_makeaddr(inet_network(d->host),inet_addr(d->host));
	if ( h_ent = gethostbyaddr((char*) &addr, sizeof(addr), AF_INET))
	    strcat(buf, h_ent->h_name );
	else
	    strcat(buf, d->host );
#endif 		/*  UNUSED_CODE  */
	/* NOTE: d->host is set to DNS name in new_descriptor() in comm.c */
	strncpy(buf, (d->host[0] ? d->host : "Unknown Host" ), sizeof(d->host));
	buf[24] = '\0' ; 	/* Truncate */
	strcat(buf, (ch ? "\r\n" : "\n"));

	/* NOTE: Prevent buffer overflow */
	buf += strlen(buf);
	if ( (buf - line + 100 ) > MAX_STRING_LENGTH )  {
	    strcat(buf, "And too many users to list....\r\n");
	    buf += strlen(buf);
	    break;
	}

/* NOTE: Now, it is meaningless to distinguish foreign/domestic player */
/*  if(is_korean(d)) in_d++; else out_d++; */
    }
    if (m > most)
	most = m;
    if (ch && IS_DIVINE(ch))
	sprintf( buf, "\nConnections:    Active: %d    Peak: %d", m, most);

    return(m);
}

char *how_good(int p1, int p2)
{
    static char buf[64];

    sprintf(buf, "(%3d,%3d)", p1, p2);
    return (buf);
}

bool do_practice(struct char_data *ch, char *arg, int cmd)
{
    int i, nlist;
    char buffer[ MAX_SKILLS * 40 + MAX_BUFSIZ ], *buf;
    char tmp[MAX_BUFSIZ];
    char victim_name[MAX_INPUT_LENGTH];
    struct char_data *victim;

    buf = buffer; *buf = '\0';
    if (IS_WIZARD(ch)) {
	arg = one_argument(arg, victim_name);
	if (*victim_name) {
	    victim = get_char_vis(ch, victim_name);
	    if (victim) {
		sprintf(buf, "Practice of %s\r\n", victim->player.name);
		buf += strlen(buf);
	    }
	    else {
		send_to_char("There's no such a person or mob.\n", ch);
		return FALSE;
	    }
	}
	else
	    victim = ch;
    }
    else
	victim = ch;

    nlist = 0;
    /* NOTE: spells index starts from 1, not zero */
    for (i = 1; *spells[i] != '\n'; i++) {
	if ( !*spells[i] || !victim->skills[i].learned )
	    continue;
	if (!(spell_info[i].min_level[GET_CLASS(victim) - 1] >
	      GET_LEVEL(victim))) {
	    if (victim->skills[i].learned <= 0)
		continue;
	    /* NOTE: Added spell number for cross ref.   */
	    /* sprintf(tmp, "%-20s %-4s", */
	    sprintf(tmp, "%3d %-20s %-9s    ", i,
		    /* NOTE: spells index starts from 1, not zero */
		    spells[i], how_good(victim->skills[i].learned,
					victim->skills[i].skilled));
	    /* NOTE: show two skills per line */
	    if (!(++nlist % 2))
		strcat(tmp, "\r\n");
	    strcat(buf, tmp);
	    buf += strlen(buf);
	}
    /* NOTE: Second entry of a line : Duplicated code commented out.    */
    }
    if (!(++nlist % 2))
	strcat(buf, "\r\n");
    page_string(ch->desc, buffer, 1);
    return TRUE;
}

/* NOTE: Separated from do_spell() for single line */
void one_spell(struct char_data *ch, int no, char *buf)
{
    /* Added your practice value display for easier cross refernce */
    /* sprintf(buf,"%3d %-20s%3d%3d%3d%3d  %4d%4d%4d%4d%8d\r\n", */
    sprintf(buf, "%3d %-20s%3d%3d%3d%3d  %4d%4d%4d%4d    %4d    %-9s%c\r\n",
	    no, spells[no],
	spell_info[no].min_level[0], spell_info[no].min_level[1],
	spell_info[no].min_level[2], spell_info[no].min_level[3],
	spell_info[no].max_skill[0], spell_info[no].max_skill[1],
	spell_info[no].max_skill[2], spell_info[no].max_skill[3],
	spell_info[no].min_usesmana,
	/* NOTE: Append player's learned, skill for that spell/skill.
	 * Spell player can use/learn is marked with '*' */
	how_good(ch->skills[no].learned, ch->skills[no].skilled),
	((spell_info[no].min_level[GET_CLASS(ch) - 1]
	    <= GET_LEVEL(ch)) ? '*' : ' '));
}

void do_spells(struct char_data *ch, char *argument, int cmd)
{
    char buffer[80 * MAX_SKILLS+MAX_BUFSIZ], *buf;
    int  no;

    if (IS_NPC(ch))
	return;
    buf = buffer;
    sprintf(buf, "NUM     SPELL NAME       MU CL TH WA    MU  CL  TH  WA  MIN_MANA  PRACTICE\r\n");
    buf += strlen(buf);

    while (isspace(*argument))
	argument++;	/* NOTE: Strip leading blank */
    if (!*argument) {
	/* NOTE: spells index starts from 1, not zero   */
	for (no = 1; *spells[no] != '\n'; no++) {
	    /* NOTE: check pointer instead of spells[] string to skip */
	    if (spell_info[no].spell_pointer == 0 )
		continue;
	    one_spell(ch, no, buf);
	    buf += strlen(buf);
	}
	page_string(ch->desc, buffer, TRUE);
    }
    else {
	/* NOTE: I striped leading blank, and don't want exact match  */
	/* NOTE: Use search_block() instead of old_search_block()     */
	no = search_block(argument, spells, FALSE);
	if (no == -1 || spell_info[no].spell_pointer == 0 ) {
	    send_to_char("There's no such skill.\r\n", ch);
	    return;
	}
	one_spell(ch, no, buf);
	send_to_char(buffer, ch);
    }
}

void do_levels(struct char_data *ch, char *argument, int cmd)
{
    int i;
    char buf[MAX_STRING_LENGTH];

    if (IS_NPC(ch)) {
	send_to_char("You ain't nothin' but a hound-dog.\r\n", ch);
	return;
    }
    *buf = '\0';
    for (i = 1; i <= LEVEL_LIMIT; i++) {
	sprintf(buf + strlen(buf), "%2d: %10lld to %10lld: ", i,
		titles[GET_CLASS(ch) - 1][i].exp,
		titles[GET_CLASS(ch) - 1][i + 1].exp);
	switch (GET_SEX(ch)) {
	case SEX_MALE:
	    strcat(buf, titles[GET_CLASS(ch) - 1][i].title_m);
	    break;
	case SEX_FEMALE:
	    strcat(buf, titles[GET_CLASS(ch) - 1][i].title_f);
	    break;
	default:
	    send_to_char("Oh dear.\r\n", ch);
	    break;
	}
	strcat(buf, "\r\n");
    }
    /* NOTE: Use paging */
    page_string(ch->desc, buf, 1);
}


struct help_index_element {
    char *keyword;
        long pos;
};

struct help_index_element *help_index = 0;
int top_of_helpt = 0;		/* top of help index table	*/

FILE *help_fl;			/* file for help texts (HELP <kwd>) */

/* NOTE: Removed HELP_PAGE_FILE and help[]. */
/* NOTE: Main help page "lib/help" file is inserted into beginning of
	 "help_table" file itself under "MASTER HELP" keyword.  */
void do_help(struct char_data *ch, char *argument, int cmd)
{
    int chk, bot, top, mid, minlen;
    char buf[MAX_STRING_LENGTH], buffer[MAX_STRING_LENGTH];


    if (!ch->desc)
	return;
    for (; isspace(*argument); argument++) ;
    /* NOTE: If 'help' has no arg, search topic "MASTER HELP".	*/
    if (!*argument)
	argument = "MASTER HELP";
    if (!help_index) {
	send_to_char("No help available.\r\n", ch);
	return;
    }
    bot = 0;
    top = top_of_helpt;

    for (;;) {
	mid = (bot + top) / 2;
	minlen = strlen(argument);

	if (!(chk = strn_cmp(argument, help_index[mid].keyword, minlen))) {
	    fseek(help_fl, help_index[mid].pos, 0);
	    *buffer = '\0';
	    for (;;) {
		fgets(buf, 80, help_fl);
		if (*buf == '#')
		    break;
		strcat(buffer, buf);
		strcat(buffer, "\r");
	    }
	    page_string(ch->desc, buffer, 0);
	    return;
	}
	else if (bot >= top) {
	    send_to_char("There is no help on that word.\r\n", ch);
	    return;
	}
	else if (chk > 0)
	    bot = ++mid;
	else
	    top = --mid;
    }
    return;
}

void build_help_index(void)
{
    int nr = -1, issorted, i;
    struct help_index_element *list = 0, mem;
    char buf[MAX_BUFSIZ], tmp[81], *scan;
    long pos;
    FILE *fl;

    /* NOTE: Now, help file is opened here, not in boot_db()  */
    if (!(help_fl = fopen(lookup_db("help"), "r"))) {
	log("   Could not open help file.");
	return;
    }
    fl = help_fl;

    for (;;) {
	pos = ftell(fl);
	fgets(buf, 81, fl);
	*(buf + strlen(buf) - 1) = '\0';
	scan = buf;
	for (;;) {
	    /* extract the keywords */
	    scan = one_word(scan, tmp);

	    if (!*tmp)
		break;

	    if (!list) {
		CREATE(list, struct help_index_element, 1);
		nr = 0;
	    }
	    else
		RECREATE(list, struct help_index_element, ++nr + 1);

	    list[nr].pos = pos;
	    CREATE(list[nr].keyword, char, strlen(tmp) + 1);

	    strcpy(list[nr].keyword, tmp);
	}
	/* skip the text */
	do
	    fgets(buf, 81, fl);
	while (*buf != '#');
	if (*(buf + 1) == '~')
	    break;
    }
    /* we might as well sort the stuff */
    do {
	issorted = 1;
	for (i = 0; i < nr; i++)
	    if (str_cmp(list[i].keyword, list[i + 1].keyword) > 0) {
		mem = list[i];
		list[i] = list[i + 1];
		list[i + 1] = mem;
		issorted = 0;
	    }
    }
    while (!issorted);

    top_of_helpt = nr;	/* top of help index table	*/
    help_index = list;
}

/* NOTE: Commands 'credit', 'news', 'NEWS', 'plan', 'wizlist', 'version'
	are merged into single command 'show'. like 'show news', 'show credit'.
	do_credit(), do_news(), do_plan(), do_wizards(), do_version()
	are merged to do_show(). 	*/
/* NOTE: news, news_old, plan, credits, wizards is read on the fly. */
void do_show(struct char_data *ch, char *argument, int cmd)
{
    char arg[100], *fs, *mark, *head, *tail;
    char file_str[MAX_STRING_LENGTH];
    int topic;
    static char *show_list[] = {
	"news", "NEWS", "oldnews", "plans", "wizards", "wizlists",
       	"credits", "motd", "versions" , "help", "?", "", "\n" };
#define TOPICS "help, news, old, plan, wizards, credits, motd and version"
    extern char motd[];			/* the messages of today	*/

    one_argument(argument, arg );

    switch( topic = search_block(arg, show_list, 0 )) {
	case 0:	case 1:    case 2:  case 3:
	/* NOTE: NEW! "old_news" Yesterday's news: archived news articles  */
	/* NOTE:  static news[], news_old[] was too small. Use dynamic heap. */
	strcpy( arg, NEWS_FILE );
	fs = file_to_string(arg, NULL );
	if (!fs)
	    return;
	if ( topic == 2  )
	    mark = "\n#OLDNEWS\n";
	else if ( topic == 3 )
	    mark = "\n#PLAN\n";
	else
	    mark = "\n#NEWS\n";
	head = strstr(fs, mark);
	if ( !head )
	    return;
	head += strlen(mark);
	tail = strstr(head+1, "\n#");
	if ( tail )
	    *tail = '\0';

	page_string(ch->desc, head, 1);
	free(fs);
	break;

    case 4:	case 5:	    case 6:
	/* NOTE: Wizard list/Credits page are merged to single "wizards" */
	/* NOTE:    files are separated by form-feed(^L) char	*/
	if(!file_to_string(CREDITS_FILE, file_str))
	    return;

	mark = (topic == 6) ? "\n#WIZARDS\n" : "\n#CREDITS\n";
	head = strstr(file_str, mark);
	if ( !head )
	    return;
	head += strlen(mark);
	tail = strstr(head+1, "\n#");
	if ( tail )
	    *tail = '\0';

	send_to_char( head, ch );
	break;

    case 7:
	send_to_char( motd, ch );
	break;

    case 8:
	if (IS_WIZARD(ch)) {
	    send_to_char(version_id, ch);
	    /* NOTE: show compile time */
	    send_to_char("Compiled on : " __DATE__ "  " __TIME__ ".\r\n" , ch);
	}
	else
	    send_to_char("This topic is for Immortals only.\r\n", ch);
	break;

    default:
	send_to_char( "Available topics: " TOPICS ".\r\n" , ch);
	break;
    }
}

/* NOTE: Commands 'credit', 'news', 'NEWS', 'plan', 'wizlist' 'version' are
	merged into single command 'show'. like 'show news', 'show credit'..
    do_credit(), do_news(), do_plan(), do_wizards() and do_version()
    are replaced by do_show().  See do_show() in newcom.c for details	*/
/*
void do_credits(struct char_data *ch, char *argument, int cmd)
{
    page_string(ch->desc, credits, 0);
}
void do_news(struct char_data *ch, char *argument, int cmd)
{
    page_string(ch->desc, news, 0);
}
void do_plan(struct char_data *ch, char *argument, int cmd)
{
    page_string(ch->desc, plan, 0);
}
void do_wizards(struct char_data *ch, char *argument, int cmd)
{
    page_string(ch->desc, wizards, 0);
}

void do_version(struct char_data *ch, char *argument, int cmd)
{
    extern char fileid[];
    send_to_char(fileid, ch);
}
*/

/* ************************************************************************
   *  file: weather.c , Weather and time module              Part of DIKUMUD *
   *  Usage: Performing the clock and the weather                            *
   ************************************************************************* */

struct time_info_data time_info;	/* the infomation about the time   */
struct weather_data weather_info;	/* the infomation about the weather */

extern void send_to_outdoor(char *mesg);

void do_weather(struct char_data *ch, char *argument, int cmd)
{
    char buf[MAX_LINE_LEN], buf2[MAX_LINE_LEN];
    static char *sky_look[4] =
    {
	"cloudless",
	"cloudy",
	"rainy",
	"lit by flashes of lightning"
    };
    static char *sky_look_han[4] =
    {
	"하늘이 구름 한점 없이 맑습니다.",
	"하늘에 구름이 조금 있습니다.",
	"비가 오고 있습니다.",
	"천둥 번개가 칩니다."
    };

    void weather_change(int change);

    one_argument(argument, buf2);
    if (IS_GOD(ch) && buf2[0] != '\0') {
	if (strcmp(buf2, "cloud") == 0)
	    weather_change(1);
	else if (strcmp(buf2, "rain") == 0)
	    weather_change(2);
	else if (strcmp(buf2, "cloudless") == 0)
	    weather_change(3);
	else if (strcmp(buf2, "lightning") == 0)
	    weather_change(4);
	return;
    }
    if (OUTSIDE(ch)) {
	sprintf(buf, "The sky is %s and %s.\r\n", sky_look[weather_info.sky],
	     (weather_info.change >= 0 ? "you feel a warm wind from south" :
	      "your foot tells you bad weather is due"));
	sprintf(buf2, "%s %s.\r\n", sky_look_han[weather_info.sky],
	 (weather_info.change >= 0 ? "서편하늘부터 구름이 개이고 있습니다" :
	  "더 궂은 날이 되것 같습니다"));
	send_to_char_han(buf, buf2, ch);
    }
    else
	send_to_char_han("You have no feeling about the weather at all.\r\n",
			 "여기선 날씨가 어떤지 알 수 없습니다.\r\n", ch);
}

void do_time(struct char_data *ch, char *argument, int cmd)
{
    char buf[100], *suf, *ts;
    int weekday, day;
    time_t  tm;
    extern char *weekdays[];
    extern char *month_name[];

    sprintf(buf, "It is %d o'clock %s, on ",
	    ((time_info.hours % 12 == 0) ? 12 : ((time_info.hours) % 12)),
	    ((time_info.hours >= 12) ? "pm" : "am"));

    /* 35 days in a month */
    weekday = ((35 * time_info.month) + time_info.day + 1) % 7;

    strcat(buf, weekdays[weekday]);
    strcat(buf, "\r\n");
    send_to_char(buf, ch);

    day = time_info.day + 1;	/* day in [1..35] */

    if (day == 1) suf = "st";
    else if (day == 2) suf = "nd";
    else if (day == 3) suf = "rd";
    else if (day < 20) suf = "th";
    else if ((day % 10) == 1) suf = "st";
    else if ((day % 10) == 2) suf = "nd";
    else if ((day % 10) == 3) suf = "rd";
    else suf = "th";

    sprintf(buf, "The %d%s Day of the %s, Year %d.\r\n",
	    day, suf, month_name[time_info.month], time_info.year);

    send_to_char(buf, ch);

    /* NOTE: Show Real World calendar clock, too */
    tm = time(0);
    ts = asctime(localtime(&tm));
    ts[16] = '\0';
    sprintf( buf, "But your broken wrist watch tells it's %s.\r\n", ts );
    send_to_char(buf, ch);
}

void another_hour(int mode)
{
    time_info.hours++;

    if (mode) {
	switch (time_info.hours) {
	case 5:
		weather_info.sunlight = SUN_RISE;
		send_to_outdoor("The sun rises in the east.\r\n");
		break;
	case 6:
		weather_info.sunlight = SUN_LIGHT;
		send_to_outdoor("The day has begun.\r\n");
		break;
	case 21:
		weather_info.sunlight = SUN_SET;
		send_to_outdoor( "The sun slowly disappears in the west.\r\n");
		break;
	case 22:
		weather_info.sunlight = SUN_DARK;
		send_to_outdoor("The night has begun.\r\n");
		break;
	default:
	    break;
	}
    }

    if (time_info.hours > 23) {		/* Changed by HHS due to bug ??? */
	time_info.hours -= 24;
	time_info.day++;

	if (time_info.day > 34) {
	    time_info.day = 0;
	    time_info.month++;

	    if (time_info.month > 16) {
		time_info.month = 0;
		time_info.year++;
	    }
	}
    }
}

void weather_change(int change)
{
    int diff;

    if ((time_info.month >= 9) && (time_info.month <= 16))
	diff = (weather_info.pressure > 985 ? -2 : 2);
    else
	diff = (weather_info.pressure > 1015 ? -2 : 2);

    weather_info.change += (dice(1, 4) * diff + dice(2, 6) - dice(2, 6));

    weather_info.change = MIN(weather_info.change, 12);
    weather_info.change = MAX(weather_info.change, -12);

    weather_info.pressure += weather_info.change;

    weather_info.pressure = MIN(weather_info.pressure, 1040);
    weather_info.pressure = MAX(weather_info.pressure, 960);

    if (change == 0) {
	switch (weather_info.sky) {
	case SKY_CLOUDLESS:
		if (weather_info.pressure < 990)
		    change = 1;
		else if (weather_info.pressure < 1010)
		    if (dice(1, 4) == 1)
			change = 1;
		break;
	case SKY_CLOUDY:
		if (weather_info.pressure < 970)
		    change = 2;
		else if (weather_info.pressure < 990)
		    if (dice(1, 4) == 1)
			change = 2;
		    else
			change = 0;
		else if (weather_info.pressure > 1030)
		    if (dice(1, 4) == 1)
			change = 3;

		break;
	case SKY_RAINING:
		if (weather_info.pressure < 970)
		    if (dice(1, 4) == 1)
			change = 4;
		    else
			change = 0;
		else if (weather_info.pressure > 1030)
		    change = 5;
		else if (weather_info.pressure > 1010)
		    if (dice(1, 4) == 1)
			change = 5;
		break;
	case SKY_LIGHTNING:
		if (weather_info.pressure > 1010)
		    change = 6;
		else if (weather_info.pressure > 990)
		    if (dice(1, 4) == 1)
			change = 6;

		break;
	default:
		change = 0;
		weather_info.sky = SKY_CLOUDLESS;
		break;
	}
    }

    switch (change) {
    case 0:
	break;
    case 1:
	    send_to_outdoor( "The sky is getting cloudy.\r\n");
	    weather_info.sky = SKY_CLOUDY;
	    break;
    case 2:
	    send_to_outdoor( "It starts to rain.\r\n");
	    weather_info.sky = SKY_RAINING;
	    break;
    case 3:
	    send_to_outdoor( "The clouds disappear.\r\n");
	    weather_info.sky = SKY_CLOUDLESS;
	    break;
    case 4:
	    send_to_outdoor( "Lightning starts to show in the sky.\r\n");
	    weather_info.sky = SKY_LIGHTNING;
	    break;
    case 5:
	    send_to_outdoor( "The rain stopped.\r\n");
	    weather_info.sky = SKY_CLOUDY;
	    break;
    case 6:
	    send_to_outdoor( "The lightning has stopped.\r\n");
	    weather_info.sky = SKY_RAINING;
	    break;
    default:
	break;
    }
}

void weather_and_time(int mode)
{
    another_hour(mode);
    if (mode)
	weather_change(0);
}

int is_solved_quest(struct char_data *ch, int quest)
{
    return (ch->quest.flag & (0x1 << quest));
}

void set_solved_quest(struct char_data *ch, int quest)
{
	assert( (ch->quest.flag & (0x1 << quest)) == 0 );
	ch->quest.flag |= (0x1 << quest);
}


