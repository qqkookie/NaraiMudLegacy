/* ************************************************************************
   *  file: Interpreter.c , Command interpreter module.      Part of DIKUMUD *
   *  Usage: Procedures interpreting user command                            *
   *  Copyright (C) 1990, 1991 - see 'license.doc' for complete information. *
   ************************************************************************* */

#include <stdio.h>
#include <string.h>

#include "char.h"
#include "object.h"
#include "global.h"
#include "comm.h"
#include "actions.h"

#define MAX_CMD_LIST 250        /* max command list modified by chase */

struct command_info {
    void (*command_pointer) (struct char_data * ch, char *argument, int cmd);
    /* NOTE: Include command string and command number */
    char *command_str;
    sh_int command_nr;
    byte minimum_position;
    byte minimum_level[4];      /* modified by ares */
};

/* global varible */
struct command_info *cmd_info[MAX_CMD_LIST];
char *CMD_LINE; /* NOTE: New! to get full command line from action proc. */

/* ---------------------------------------------------------------- */

#ifdef UNUSED_CODE

char *command[] =
{ "",
 "north",		/* 1 */
 "east",
 "south",
 "west",
 "up",
 "down",
 "enter",
 "exits",
 /* "kiss", */
 "get",
 "drink",		/* 11 */
 "eat",
 "wear",
 "wield",
 "look",
 "score",
 "say",
 "shout",
 "tell",
 "inventory",
 "qui",			/* 21 */
 /* "bounce", */
 "smile",		// social special 
 /* "dance", */
 "kill",
 /* "cackle", */
 /* "laugh", */
 /* "giggle", */
 /* "shake", */
 /* "puke", */
 /* "growl", */		/* 31 */
 /* "scream", */
 "insult",
 /* "comfort", */
 /* "nod", */
 /* "sigh", */
 /* "sulk", */
 "help",
 "who",
 "emote",
 "echo",		/* 41 */
 "stand",
 "sit",
 "rest",
 "sleep",
 "wake",
 "force",
 "transfer",
 /* "hug", */
 /* "snuggle", */
 /* "cuddle", */		/* 51 */
 /* "nuzzle", */
 /* "cry", */
 "news",
 "equipment",
 "buy",
 "sell",
 "value",
 "list",
 "drop",
 "goto",		/* 61 */
 "weather",
 "read",
 "pour",
 "grab",
 "remove",
 "put",
 "shutdow",
 "save",
 "hit",
 "string",		/* 71 */
 "give",
 "quit",
 "stat",
 /* "bellow", */
 "time",
 "load",
 "purge",
 "shutdown",
 /* "scratch", */
 "",			/* 81 */
 "replacerent",
 "whisper",
 "cast",
 "at",
 "ask",
 "order",
 "sip",
 "taste",
 "snoop",
 "follow",		/* 91 */
 "rent",
 "junk",
 /* "poke", */
 "advance",
 /* "accuse", */
 /* "grin", */
 "bow",		// social special
 "open",
 "close",
 "lock",		/* 101 */
 "unlock",
 "leave",
 /* "applaud", */
 /* "blush", */
 /* "burp", */
 /* "chuckle", */
 /* "clap", */
 /* "cough", */
 /* "curtsey", */
 /* "fart", */		/* 111 */
 /* "flip", */
 /* "fondle", */
 /* "frown", */
 /* "gasp", */
 /* "glare", */
 /* "groan", */
 /* "grope", */
 /* "hiccup", */
 /* "lick", */
 /* "love", */		/* 121 */
 /* "moan", */
 /* "nibble", */
 /* "pout", */
 /* "purr", */
 /* "ruffle", */
 /* "shiver", */
 /* "shrug", */
 /* "sing", */
 /* "slap", */
 /* "smirk", */		/* 131 */
 /* "snap", */
 /* "sneeze", */
 /* "snicker", */
 /* "sniff", */
 /* "snore", */
 /* "spit", */
 /* "squeeze", */
 /* "stare", */
 /* "strut", */
 /* "thank", */		/* 141 */
 /* "twiddle", */
 /* "wave", */
 /* "whistle", */
 /* "wiggle", */
 /* "wink", */
 /* "yawn", */
 /* "snowball", */
 "extractrent",
 "hold",
 "flee",		/* 151 */
 "sneak",
 "hide",
 "backstab",
 "pick",
 "steal",
 "bash",
 "rescue",
 "kick",
 /* "french", */
 /* "comb", */		/* 161 */
 /* "massage", */
 /* "tickle", */
 "practice",
 /* "pat", */
 "examine",
 "take",
 "",
 "'",
 "practise",
 /* "curse", */		/* 171 */
 "use",
 "where",
 "levels",
 "reroll",
 /* "pray", */
 "~",			/* NOTE: Abbr: emote  ',' -> '~'  */
 /* "beg", */
 /* "piss", */
 /* "cringe", */
 /* "daydream", */		/* 181 */
 /* "fume", */
 /* "grovel", */
 /* "hop", */
 /* "nudge", */
 /* "peer", */
 "point",		/* NOTE: 'point' is no longer just a social action. */
 /* "ponder", */
 /* "drool", */
 /* "snarl", */
 /* "spank", */		/* 191 */
 "shoot",
 /* "bark", */
 /* "taunt", */
 /* "think", */
 /* "whine", */
 /* "worship", */
 /* "yodel", */		/* 198 */
 "brief",
 "wiznet",
 "consider",		/* 201 */
 "group",
 "restore",
 "return",
 "switch",		/* 205 */
 "quaff",
 "recite",
 "users",
 "flag",
 "noshout",
 "wizhelp",		/* 211 */
 "credits",
 "compact",
 "flick",
 "wall",
 "wizset",		/* NOTE: OLD 'set' command renamed 'wizset' */
 "police",
 "wizlock",
 "noaffect",
 "invis",
 "notell",
 "banish",
 "reload",
 "data",
 "checkrent",		/* 225 */
 "chat",
 "balance",
 "deposit",
 "withdraw",
 "sys",
 "log",			/* 231 */
 "mstat",
 "pstat",
 "tornado",
 "light",
 "title",
 "report",
 "spells",
 "flash",
 "multi kick",
 "demote",		/* 241 */
 "nochat",
 "wimpy",
 "gtell",
 "send",
 "write",
 "post",
 /* "웃음", */
 /* "말", */
 /* "울음", */
 /* "춤", */			/* 251 */
 "ㅣ",
 /* "하하", */
 /* "체팅", */
 /* "haha", */
 /* "jjup", */
 /* "wow", */			/* 257 */
 /* "bye", */
 /* "hee", */
 /* "brb", */
 /* "hmm", */			/* 261 */
 "assist",
 "ungroup",
 "wizlist",
 "hangul",
 "NEWS",
 "set",			/* NOTE: NEW! set player parameter. old 'version' pos */
 "disarm",		/* by chase 268 */
 "shouryuken",		/* chase 269 */
 "throw",		/* chase 270 */
 "punch",
 "assault",		/* process 272 */
 "JOIN",		/* guild join 273 */
 "LEAVE",		/* guild leave 274 */
 "train",		/* guild skill practice 275 */
 "cant",		/* guild chat 276 */
 "SAVE",		/* 277 SAVE at locker room */
 "LOAD",		/* 278 LOAD at locker room */
 "QUERY",		/* 279 QUERY,querys player's guild */
 "broadcast",		/* 280 for police */
 "simultaneous",	/* 281 for police */
 "arrest",		/* 282 for police */
 "angry yell",		/* 283 for outlaws */
 "solace",		/* 284 for assasins */
 "unwield",
 "unhold",
 "temptation",		/* 287 */
 "shadow figure",	/* 288 for assasins */
 "smoke",		/* 289 for outlaws */
 "inject",		/* 290 for outlaws */
 "show",
 "power bash",
 "evil strike",
 "call",		/* 294 for board shuttle bus */
 "charge",
 "solo",
 "dumb",
 "spin bird kick",
 "view",		/* used in new_shop.c */
 "reply",
 "",			/* 301 */
 "quest",
 "request",
 "hint",
 "pull",
 "change",
 ":",
 "lastchat",
 "\n",
};

#endif		/* UNUSED_CODE */

char *alias[] = {
    "",		"",
    "a",	"assist",
    "b",	"backstab",
    "c",	"cast",
    "f",	"flee",
    "g",	"get",
    "h",	"h",
    "j",	"j",
    "k",	"kill",
    "m",	"multi",
    "o",	"open",
    "p",	"put",
    "q",	"quaff",
    "r",	"remove",
    "t",	"tornado",
    "v",	"v",
    "x",	"examine",
    "y",	"y",
    "z",	"z",

    "/",	"chat",
    "\"",	"gtell",
    "gi",	"give",
    "gr",	"group",
    "wr",	"wear",
    "gk",	"get key",
    "gkc",	"get key corpse",
    "dk",	"drop key",
    "ul",	"unlock",
    "od",	"open door",
    "og",	"open gate",
    "xc",	"examine corpse",
    "gac",	"get all corpse",
    "res",	"rescue",
    "drs",	"drink sundew",
    "xh",	"examine holding",
    "wh",	"wear holding",
    "rh",	"remove holding",
    "xu",	"examin urn",
    "xb",	"examin boddari",
    "xal",	"examine altar",
    "lt",	"look target",
    "asl",	"assist leader",

    "id",	"cast 'identify'",
    "solar",	"quaff solar",
    "rrr",	"recite recall",
    "word",	"cast 'word'",
    "zzz",	"cast 'sleep' ",
    "hh",	"cast 'self heal'",
    "he",	"cast 'heal' ",
    "hhh",	"cast 'full heal' ",
    "sanc",	"cast 'sanc' ",
    "armor",	"cast 'armor' ",
    "has",	"cast 'haste' ",
    "harm",	"cast 'harm' ",
    "cone",	"cast 'cone' ",
    "fire",	"cast 'firestorm'",
    "li",	"light",
    "sho",	"shouryuken",
    "spin",	"spin",
    "\n",	"\n",
};

extern int find_social(char *action_name, int *min_lev, int *min_pos );
/* internal proc's */
int special(struct char_data *ch, int cmd, char *arg );
void log_command(struct char_data *ch, char *argument, int cmd);
void do_social_special(struct char_data *ch, char *argument, int cmd);

/* NOTE: NEW! Break from hide state after executing command.
    This needs much refinement like determine breaking hide depending
    on command and action. */
void break_hide(struct char_data *ch, char *argument, int cmd)
{
	REMOVE_BIT(ch->specials.affected_by, AFF_HIDE);
}

int command_interpreter(struct char_data *ch, char *argument)
{
    int cmd_nr, req_level, req_pos ;
    void (*cmd_proc)(struct char_data *ch, char *arg, int cmd );
    int lev, alias_found, len , i;
    char *args, *to, *msg ;
    char cmdbuf[MAX_BUFSIZ] ;
    extern int no_specials;

    /* NOTE: Break from hide state *after* executing command. */

#ifdef UNUSED_CODE
    /* Find first non blank */
    for (begin = 0; (*(argument + begin) == ' '); begin++) ;
    /* Find length of first word */
    for (look_at = 0; ISLETTER(*(argument + begin + look_at)); look_at++) ;
#endif    /* UNUSED_CODE */

    for ( args = argument; isspace(*args) ; args++ ) ;
    /* NOTE: NEW GLOBAL varable to access full command line */
    CMD_LINE = args;
    for ( to = cmdbuf ; *args && !isspace(*args) ; args++, to++ )
	*to = *args;
    *to = '\0' ;

    /* NOTE: Expand aliases on cmdbuf[] , parse it again	*/
    /* NOTE: match parameter 2  means keyword-data pair exact match  */
    if(( alias_found = search_block( cmdbuf, alias, 2 )) > 0 ) {
	strcpy( cmdbuf, alias[alias_found+1] );
	strcat( cmdbuf, args );
        for( args = cmdbuf ; *args && !isspace(*args); args++ ) ;
	if( *args)
	    *(args++)= '\0' ;
    }
    /* NOTE: Strip blanks between command and arguments */
    while( isspace(*args)) args++ ;
/*
    cmd = old_search_block(argument, begin, look_at, command, 0);
*/
    /* NOTE: find command in cmd_info.command_str[]: inexact match */
    cmd_nr = 0;
    len = strlen(cmdbuf);
    if(len == 0)
	return(0);
    for( i = 1 ; i < MAX_CMD_LIST ; i++) {
	if ( !cmd_info[i] || cmd_info[i]->command_nr <=0 )
	    break;
	if ( !strncmp(cmdbuf, cmd_info[i]->command_str, len )) {
	    cmd_nr = cmd_info[i]->command_nr;
	    req_level = cmd_info[i]->minimum_level[GET_CLASS(ch)-1];
	    req_pos = cmd_info[i]->minimum_position;
	    cmd_proc = cmd_info[i]->command_pointer;
	    goto found;
	}
    }
    /* NOTE: if not found in command list, it may be social action */
    /* NOTE: If it is social action, find_social will return
	positive index to soc_mess[] table, and set req_level, req_pos.	*/
    cmd_nr = find_social( cmdbuf, &req_level, &req_pos );
    cmd_proc = do_social;

found:
    if ( cmd_nr <= 0) {
	lev = GET_LEVEL(ch);
	if (lev > LEVEL_LIMIT)
	    msg = STRHAN("What did you say, Sir ?\r\n",
	"오 신이시여 무식한 제가 잘못입니다.. 좀더 쉬운 말씀으로..\r\n", ch);
	else if (lev >= 31)
	    msg = STRHAN("What did you say?\r\n","뭐라고 하셨습니까 ?\r\n",ch);
	else if (lev >= 13)
	    msg = STRHAN("What?\r\n", "예 ? 뭐요 ?\r\n", ch);
	else
	    msg = STRHAN( "Huh?\r\n", "뭐라고?\r\n", ch);
    }
    else if ( cmd_proc == 0 )
	msg = "Sorry, but that command has yet to be implemented...\r\n";
    else if ( GET_LEVEL(ch) < req_level )
	msg = "Huh?\r\n";
    else if ( GET_POS(ch) < req_pos ) {
	switch (GET_POS(ch)) {
	case POS_DEAD:
	    msg = "Lie still; you are DEAD!!! :-( \r\n"; break;
	case POS_INCAP:
	case POS_MORTALLYW:
	    msg = "You are in a pretty bad shape, unable to do anything!\r\n";
	    break;
	case POS_STUNNED:
	    msg = "All you can do right now, is think about the stars!\r\n";
	    break;
	case POS_SLEEPING:
	    msg = "In your dreams, or what?\r\n"; break;
	case POS_RESTING:
	    msg = "Nah... You feel too relaxed to do that..\r\n"; break;
	case POS_SITTING:
	    msg = "Maybe you should get on your feet first?\r\n"; break;
	case POS_FIGHTING:
	    msg = "No way! You are fighting for your life!\r\n"; break;
	default:
	    msg = "You are not proper position to do that!\r\n";
	}
    }
    else if ( cmd_proc == do_social ) {
	do_social( ch, args, cmd_nr );

	/* NOTE: BUG: char is still hided after action command : fixed it. */
	return (1);
    }
    else {
	log_command(ch, argument, cmd_nr);
	if (no_specials || !special(ch, cmd_nr, args))
	((*cmd_proc) (ch, args, cmd_nr));

	/* NOTE: char breaks hide *after* executing first command  */
	break_hide(ch, args, cmd_nr);
	return (1);
    }
    send_to_char(msg, ch);
    return(0);
}

void log_command(struct char_data *ch, char *argument, int cmd)
{
    char buf[MAX_BUFSIZ];
    extern int loglevel;
/* level  0 : No command/extract/stash log.
 * level  1 : shout, chat, cant, echo, wall, wizset, demote, shutdown, banish.
 * level  2 : level 1 + say, tell, ', reply, ask. (except gtell, whisper.)
 * level  3 : All commands except simple movement.
 * level  4 : level 3 + extract_char/ stash_char.
 * level  5 : level 1 + debugging info (marked with "DEBUG:").  */

    if ( loglevel <= 0 || cmd <= CMD_DOWN )
	return;
    /* NOTE: log PC player only, not mob */
    if ( !IS_NPC(ch) && IS_SET(ch->specials.act, PLR_XYZZY)) {
	sprintf(buf, "%s:: %s", GET_NAME(ch), argument);
	log(buf);
    }
    /* NOTE: Log some commands according to log level   */
    else if(( cmd==CMD_SHOUT || cmd==CMD_CHAT || cmd==CMD_CANT || cmd==CMD_ECHO
	    || cmd==CMD_WALL || cmd==CMD_WIZSET || cmd==CMD_DEMOTE
	    || cmd==CMD_SHUTDOWN || cmd==CMD_BANISH || cmd==CMD_FLAG )
	|| (loglevel == 2 && ( cmd == CMD_SAY || cmd == CMD_TELL
	    || cmd == CMD_QUOTE || cmd == CMD_REPLY || cmd == CMD_ASK ))
       || (loglevel == 3 || loglevel == 4)) {

	sprintf(buf,"$ %-8s %s", GET_NAME(ch), argument);
	log(buf);
    }
}

/* NOTE: special() has changed argument order  */
int special(struct char_data *ch, int cmd, char *arg )
{
    register struct obj_data *i;
    register struct char_data *k;
    int j;

    /* special in room? */
    if (world[ch->in_room].funct)
	if ((*world[ch->in_room].funct) (ch, cmd, arg))
	    return (1);

    /* special in equipment list? */
    for (j = 0; j <= (MAX_WEAR - 1); j++)
	if (ch->equipment[j] && ch->equipment[j]->item_number >= 0)
	    if (obj_index[ch->equipment[j]->item_number].func)
		if ((*obj_index[ch->equipment[j]->item_number].func)
		    (ch, cmd, arg))
		    return (1);

    /* special in inventory? */
    for (i = ch->carrying; i; i = i->next_content)
	if (i->item_number >= 0)
	    if (obj_index[i->item_number].func)
		if ((*obj_index[i->item_number].func) (ch, cmd, arg))
		    return (1);

    /* special in mobile present? */
    for (k = world[ch->in_room].people; k; k = k->next_in_room)
	if (IS_MOB(k) && k->nr >= 0 )	/* NOTE: not IS_NPC(ch) */
	    if (mob_index[k->nr].func)
		if ((*mob_index[k->nr].func) (ch, cmd, arg))
		    return (1);

    /* special in object present? */
    for (i = world[ch->in_room].contents; i; i = i->next_content)
	if (i->item_number >= 0)
	    if (obj_index[i->item_number].func)
		if ((*obj_index[i->item_number].func) (ch, cmd, arg))
		    return (1);

    return (0);
}

void do_social_special(struct char_data *ch, char *arg, int cmd_nr )
{
    if( cmd_nr == CMD_SMILE )
	cmd_nr = find_social("smile", NULL, NULL);
    else if( cmd_nr == CMD_BOW )
	cmd_nr = find_social("bow", NULL, NULL);

    do_social(ch, arg, cmd_nr);
}

/* NOTE: subst_show() will notify do_show() will substitute
	do_news(), do_credits(), do_wizards(), do_plan(), do_version(). */
void subst_show(struct char_data *ch, char *argument, int cmd)
{
    send_to_char("This command is removed. Try 'show <topic>'.\r\n", ch );
}
/* NOTE: subst_set() will notify do_set() will substitute
	do_nochat(), do_notell(), doshout(), do_compact(), do_brief(),
	do_hangul(), do_solo(), do_title(). */
void subst_set(struct char_data *ch, char *argument, int cmd)
{
    send_to_char("This command is removed. "
	"Try 'set <attribute> <yes/no/value>'.\r\n", ch );
}
/* NOTE: subst_holdwield() will notify do_hold(), do_wield() will substitute
	do_unhold(), do_unwield()	*/
void subst_holdwield(struct char_data *ch, char *argument, int cmd)
{
    send_to_char("This command is removed. "
	"Try 'hold' or 'wield' with no argument.\r\n", ch );
}
/* NOTE: subst_flag() will notify do_flag()will substitute
	do_log(), do_dumb() */
void subst_flag(struct char_data *ch, char *argument, int cmd)
{
    send_to_char("This command is removed. "
	"Try 'flag <char> <shutup|dumb|log|invisi|criminal>'.\r\n", ch );
}

void do_not_here(struct char_data *ch, char *argument, int cmd)
{
    send_to_char("Sorry, but you cannot do that here!\r\n", ch);
}

/* NOTE: Now it is function. not macro */
void COMMANDO(char *str, sh_int number, byte min_pos,
	void (*pointer)(), byte min_lm, byte min_lc, byte min_lt, byte min_lw)
{
    static int counter = 0;
    struct command_info *command;
    CREATE(command, struct command_info, 1 );
    cmd_info[++counter] = command;
    command->command_pointer = (pointer);
    command->minimum_position = (min_pos);
    command->minimum_level[0] = (min_lm);
    command->minimum_level[1] = (min_lc);
    command->minimum_level[2] = (min_lt);
    command->minimum_level[3] = (min_lw);
    /* NOTE: Include command number and command string */
    command->command_nr = number;
    command->command_str = str;
}

void assign_command_pointers(void)
{
    int position;
    void commando1(void), commando2(void), commando3(void);
    for (position = 0; position < MAX_CMD_LIST; position++)
	cmd_info[position] = NULL;

/* NOTE: cmd_info should be sorted by cmd_nr,
	in strictly increasing order, but no need to be sequential. */

#define IMO	(LEVEL_LIMIT+1)

/* NOTE: Original commando was too big to compile with small swap sapce */
    COMMANDO( "north", CMD_NORTH, POS_STANDING, do_move, 0, 0, 0, 0);
    COMMANDO( "east", CMD_EAST, POS_STANDING, do_move, 0, 0, 0, 0);
    COMMANDO( "south", CMD_SOUTH, POS_STANDING, do_move, 0, 0, 0, 0);
    COMMANDO( "west", CMD_WEST, POS_STANDING, do_move, 0, 0, 0, 0);
    COMMANDO( "up", CMD_UP, POS_STANDING, do_move, 0, 0, 0, 0);
    COMMANDO( "down", CMD_DOWN, POS_STANDING, do_move, 0, 0, 0, 0);
    COMMANDO( "enter", CMD_ENTER, POS_STANDING, do_enter, 0, 0, 0, 0);
    COMMANDO( "exits", CMD_EXITS, POS_RESTING, do_exits, 0, 0, 0, 0);
    /* COMMANDO( "kiss", ACTN_KISS,  POS_RESTING, do_action, 0, 0, 0, 0); */
    COMMANDO( "get", CMD_GET, POS_RESTING, do_get, 0, 0, 0, 0);
    COMMANDO( "drink", CMD_DRINK, POS_RESTING, do_drink, 0, 0, 0, 0);
    COMMANDO( "eat", CMD_EAT, POS_RESTING, do_eat, 0, 0, 0, 0);
    COMMANDO( "wear", CMD_WEAR, POS_RESTING, do_wear, 0, 0, 0, 0);
    COMMANDO( "wield", CMD_WIELD, POS_RESTING, do_wield, 0, 0, 0, 0);
    /* NOTE: do_look() has its own minimum position checking */
    /* COMMANDO(15, POS_RESTING, do_look, 0, 0, 0, 0); */
    COMMANDO( "look", CMD_LOOK, POS_STUNNED, do_look, 0, 0, 0, 0);
    COMMANDO( "score", CMD_SCORE, POS_DEAD, do_score, 0, 0, 0, 0);
    COMMANDO( "say", CMD_SAY, POS_RESTING, do_say, 0, 0, 0, 0);
    COMMANDO( "shout", CMD_SHOUT, POS_RESTING, do_shout, 1, 1, 1, 1);
    COMMANDO( "tell", CMD_TELL, POS_RESTING, do_tell, 0, 0, 0, 0);
    COMMANDO( "inventory", CMD_INVENTORY, POS_DEAD, do_inventory, 0, 0, 0, 0);
    /* COMMANDO( "bounce", ACTN_BOUNCE,  POS_STANDING, do_action, 0, 0, 0, 0); */
    COMMANDO( "smile", CMD_SMILE,  POS_RESTING, do_social_special, 0, 0, 0, 0);
    /* COMMANDO( "dance", ACTN_DANCE,  POS_STANDING, do_action, 0, 0, 0, 0); */
    COMMANDO( "kill", CMD_KILL, POS_FIGHTING, do_kill, 0, 0, 0, 0);
    /* COMMANDO( "cackle", ACTN_CACKLE,  POS_RESTING, do_action, 0, 0, 0, 0); */
    /* COMMANDO( "laugh", ACTN_LAUGH,  POS_RESTING, do_action, 0, 0, 0, 0); */
    /* COMMANDO( "giggle", ACTN_GIGGLE,  POS_RESTING, do_action, 0, 0, 0, 0); */
    /* COMMANDO( "shake", ACTN_SHAKE,  POS_RESTING, do_action, 0, 0, 0, 0); */
    /* COMMANDO( "puke", ACTN_PUKE,  POS_RESTING, do_action, 0, 0, 0, 0); */
    /* COMMANDO( "growl", ACTN_GROWL,  POS_RESTING, do_action, 0, 0, 0, 0); */
    /* COMMANDO( "scream", ACTN_SCREAM,  POS_RESTING, do_action, 0, 0, 0, 0); */
    COMMANDO( "insult", CMD_INSULT, POS_RESTING, do_insult, 0, 0, 0, 0);
    /* COMMANDO( "comfort", ACTN_COMFORT,  POS_RESTING, do_action, 0, 0, 0, 0); */
    /* COMMANDO( "nod", ACTN_NOD,  POS_RESTING, do_action, 0, 0, 0, 0); */
    /* COMMANDO( "sigh", ACTN_SIGH,  POS_RESTING, do_action, 0, 0, 0, 0); */
    /* COMMANDO( "sulk", ACTN_SULK,  POS_RESTING, do_action, 0, 0, 0, 0); */
    COMMANDO( "help", CMD_HELP, POS_DEAD, do_help, 0, 0, 0, 0);
    COMMANDO( "who", CMD_WHO, POS_DEAD, do_who, 0, 0, 0, 0);
    COMMANDO( "emote", CMD_EMOTE, POS_SLEEPING, do_emote, 1, 1, 1, 1);
    COMMANDO( "echo", CMD_ECHO, POS_SLEEPING, do_echo, IMO, IMO, IMO, IMO);
    /* NOTE: do_stand(), do_sit(), do_rest()
    	have their own minimum position checking 	*/
    COMMANDO( "stand", CMD_STAND, POS_SLEEPING, do_stand, 0, 0, 0, 0);
    /* NOTE: do_sit() have new meaning. Sitting on some viecle or mob?? */
    COMMANDO( "sit", CMD_SIT, POS_SLEEPING, do_sit, 0, 0, 0, 0);
    COMMANDO( "rest", CMD_REST, POS_SLEEPING, do_rest, 0, 0, 0, 0);

    COMMANDO( "sleep", CMD_SLEEP, POS_SLEEPING, do_sleep, 0, 0, 0, 0);
    COMMANDO( "wake", CMD_WAKE, POS_SLEEPING, do_wake, 0, 0, 0, 0);
    COMMANDO( "force", CMD_FORCE, POS_SLEEPING, do_force, IMO + 2, IMO + 2, IMO + 2, IMO + 2);
    COMMANDO( "transfer", CMD_TRANSFER, POS_SLEEPING, do_trans, IMO + 1, IMO + 1, IMO + 1, IMO + 1);
    /* COMMANDO( "hug", ACTN_HUG,  POS_RESTING, do_action, 0, 0, 0, 0); */
    /* COMMANDO( "snuggle", ACTN_SNUGGLE,  POS_RESTING, do_action, 0, 0, 0, 0); */
    /* COMMANDO( "cuddle", ACTN_CUDDLE,  POS_RESTING, do_action, 0, 0, 0, 0); */
    /* COMMANDO( "nuzzle", ACTN_NUZZLE,  POS_RESTING, do_action, 0, 0, 0, 0); */
    /* COMMANDO( "cry", ACTN_CRY,  POS_RESTING, do_action, 0, 0, 0, 0); */
    /* NOTE: 'news' command is substituted by 'show news'.  */
    /* COMMANDO(54, POS_SLEEPING, do_news, 0, 0, 0, 0); */
    COMMANDO( "news", CMD_NEWS, POS_SLEEPING, subst_show, 0, 0, 0, 0);
    COMMANDO( "equipment", CMD_EQUIPMENT, POS_SLEEPING, do_equipment, 0, 0, 0, 0);
    COMMANDO( "buy", CMD_BUY, POS_STANDING, do_not_here, 0, 0, 0, 0);
    COMMANDO( "sell", CMD_SELL, POS_STANDING, do_not_here, 0, 0, 0, 0);
    COMMANDO( "value", CMD_VALUE, POS_STANDING, do_not_here, 0, 0, 0, 0);
    COMMANDO( "list", CMD_LIST, POS_STANDING, do_not_here, 0, 0, 0, 0);
    COMMANDO( "drop", CMD_DROP, POS_RESTING, do_drop, 0, 0, 0, 0);
    COMMANDO( "goto", CMD_GOTO, POS_SLEEPING, do_goto, IMO, IMO, IMO, IMO);
    COMMANDO( "weather", CMD_WEATHER, POS_RESTING, do_weather, 0, 0, 0, 0);
    COMMANDO( "read", CMD_READ, POS_RESTING, do_read, 0, 0, 0, 0);
    COMMANDO( "pour", CMD_POUR, POS_STANDING, do_pour, 0, 0, 0, 0);
    COMMANDO( "grab", CMD_GRAB, POS_RESTING, do_grab, 0, 0, 0, 0);
    COMMANDO( "remove", CMD_REMOVE, POS_RESTING, do_remove, 0, 0, 0, 0);
    COMMANDO( "put", CMD_PUT, POS_RESTING, do_put, 0, 0, 0, 0);
    /* NOTE: do_shutdown() will check it */
   /*  COMMANDO( "shutdow", CMD_SHUTDOW, POS_DEAD, do_shutdow, IMO + 3, IMO + 3, IMO + 3, IMO + 3); */
    COMMANDO( "save", CMD_SAVE, POS_SLEEPING, do_save, 0, 0, 0, 0);
    COMMANDO( "hit", CMD_HIT, POS_FIGHTING, do_hit, 0, 0, 0, 0);
    COMMANDO( "string", CMD_STRING, POS_SLEEPING, do_string, IMO, IMO, IMO, IMO);
    COMMANDO( "give", CMD_GIVE, POS_RESTING, do_give, 0, 0, 0, 0);
    COMMANDO( "quit", CMD_QUIT, POS_DEAD, do_quit, 0, 0, 0, 0);
    COMMANDO( "stat", CMD_STAT, POS_DEAD, do_stat, IMO, IMO, IMO, IMO);
    /* COMMANDO( "bellow", ACTN_BELLOW,  POS_DEAD, do_action, 0, 0, 0, 0); */
    COMMANDO( "time", CMD_TIME, POS_DEAD, do_time, 0, 0, 0, 0);
    COMMANDO( "load", CMD_LOAD, POS_DEAD, do_load, IMO + 2, IMO + 2, IMO + 2, IMO + 2);
    COMMANDO( "purge", CMD_PURGE, POS_DEAD, do_purge, IMO + 1, IMO + 1, IMO + 1, IMO + 1);
    COMMANDO( "shutdown", CMD_SHUTDOWN, POS_DEAD, do_shutdown, IMO + 3, IMO + 3, IMO + 3, IMO + 3);
    /* COMMANDO( "scratch", ACTN_SCRATCH,  POS_RESTING, do_action, 0, 0, 0, 0); */
    COMMANDO( "replacerent", CMD_REPLACERENT, POS_RESTING, do_replacerent,
	     IMO + 3, IMO + 3, IMO + 3, IMO + 3);
    COMMANDO( "whisper", CMD_WHISPER, POS_RESTING, do_whisper, 0, 0, 0, 0);
    /* NOTE: do_cast() has its own minimum position checking */
    /* COMMANDO(84, POS_SITTING, do_cast, 1, 1, 1, 1); */
    COMMANDO( "cast", CMD_CAST, POS_STUNNED, do_cast, 1, 1, 1, 1);
    COMMANDO( "at", CMD_AT, POS_DEAD, do_at, IMO, IMO, IMO, IMO);
    /* NOTE: do_ask() is essentailly same function with do_whisper() */
    /* COMMANDO(86, POS_RESTING, do_ask, 1, 1, 1, 1); */
    COMMANDO( "ask", CMD_ASK, POS_RESTING, do_whisper, 1, 1, 1, 1);
    COMMANDO( "order", CMD_ORDER, POS_RESTING, do_order, 1, 1, 1, 1);
    /* NOTE: do_sip(), do_taste() are replaced by do_drink(), do_eat() */
    /* COMMANDO( "sip", CMD_SIP, POS_RESTING, do_sip, 0, 0, 0, 0); */
    COMMANDO( "sip", CMD_SIP, POS_RESTING, do_drink, 0, 0, 0, 0);
    /* COMMANDO( "taste", CMD_TASTE, POS_RESTING, do_taste, 0, 0, 0, 0); */
    COMMANDO( "taste", CMD_TASTE, POS_RESTING, do_eat, 0, 0, 0, 0);
    COMMANDO( "snoop", CMD_SNOOP, POS_DEAD, do_snoop, IMO, IMO, IMO, IMO);
    COMMANDO( "follow", CMD_FOLLOW, POS_RESTING, do_follow, 0, 0, 0, 0);
    COMMANDO( "rent", CMD_RENT, POS_STANDING, do_rent, 1, 1, 1, 1);
    COMMANDO( "junk", CMD_JUNK, POS_RESTING, do_junk, 1, 1, 1, 1);
    /* COMMANDO( "poke", ACTN_POKE,  POS_RESTING, do_action, 0, 0, 0, 0); */
    COMMANDO( "advance", CMD_ADVANCE, POS_DEAD, do_advance, 1, 1, 1, 1);
    /* COMMANDO( "accuse", ACTN_ACCUSE,  POS_SITTING, do_action, 0, 0, 0, 0); */
    /* COMMANDO( "grin", ACTN_GRIN,  POS_RESTING, do_action, 0, 0, 0, 0); */
    COMMANDO( "bow", CMD_BOW,  POS_STANDING, do_social_special, 0, 0, 0, 0);
    COMMANDO( "open", CMD_OPEN, POS_SITTING, do_open, 0, 0, 0, 0);

/* ------------------------------------------------------------------------- */

    COMMANDO( "close", CMD_CLOSE, POS_SITTING, do_close, 0, 0, 0, 0);
    COMMANDO( "lock", CMD_LOCK, POS_SITTING, do_lock, 0, 0, 0, 0);
    COMMANDO( "unlock", CMD_UNLOCK, POS_SITTING, do_unlock, 0, 0, 0, 0);
    COMMANDO( "leave", CMD_LEAVE, POS_STANDING, do_leave, 0, 0, 0, 0);
    /* COMMANDO( "applaud", ACTN_APPLAUD,  POS_RESTING, do_action, 0, 0, 0, 0); */
    /* COMMANDO( "blush", ACTN_BLUSH,  POS_RESTING, do_action, 0, 0, 0, 0); */
    /* COMMANDO( "burp", ACTN_BURP,  POS_RESTING, do_action, 0, 0, 0, 0); */
    /* COMMANDO( "chuckle", ACTN_CHUCKLE,  POS_RESTING, do_action, 0, 0, 0, 0); */
    /* COMMANDO( "clap", ACTN_CLAP,  POS_RESTING, do_action, 0, 0, 0, 0); */
    /* COMMANDO( "cough", ACTN_COUGH,  POS_RESTING, do_action, 0, 0, 0, 0); */
    /* COMMANDO( "curtsey", ACTN_CURTSEY,  POS_STANDING, do_action, 0, 0, 0, 0); */
    /* COMMANDO( "fart", ACTN_FART,  POS_RESTING, do_action, 0, 0, 0, 0); */
    /* COMMANDO( "flip", ACTN_FLIP,  POS_STANDING, do_action, 0, 0, 0, 0); */
    /* COMMANDO( "fondle", ACTN_FONDLE,  POS_RESTING, do_action, 0, 0, 0, 0); */
    /* COMMANDO( "frown", ACTN_FROWN,  POS_RESTING, do_action, 0, 0, 0, 0); */
    /* COMMANDO( "gasp", ACTN_GASP,  POS_RESTING, do_action, 0, 0, 0, 0); */
    /* COMMANDO( "glare", ACTN_GLARE,  POS_RESTING, do_action, 0, 0, 0, 0); */
    /* COMMANDO( "groan", ACTN_GROAN,  POS_RESTING, do_action, 0, 0, 0, 0); */
    /* COMMANDO( "grope", ACTN_GROPE,  POS_RESTING, do_action, 0, 0, 0, 0); */
    /* COMMANDO( "hiccup", ACTN_HICCUP,  POS_RESTING, do_action, 0, 0, 0, 0); */
    /* COMMANDO( "lick", ACTN_LICK,  POS_RESTING, do_action, 0, 0, 0, 0); */
    /* COMMANDO( "love", ACTN_LOVE,  POS_RESTING, do_action, 0, 0, 0, 0); */
    /* COMMANDO( "moan", ACTN_MOAN,  POS_RESTING, do_action, 0, 0, 0, 0); */
    /* COMMANDO( "nibble", ACTN_NIBBLE,  POS_RESTING, do_action, 0, 0, 0, 0); */
    /* COMMANDO( "pout", ACTN_POUT,  POS_RESTING, do_action, 0, 0, 0, 0); */
    /* COMMANDO( "purr", ACTN_PURR,  POS_RESTING, do_action, 0, 0, 0, 0); */
    /* COMMANDO( "ruffle", ACTN_RUFFLE,  POS_STANDING, do_action, 0, 0, 0, 0); */
    /* COMMANDO( "shiver", ACTN_SHIVER,  POS_RESTING, do_action, 0, 0, 0, 0); */
    /* COMMANDO( "shrug", ACTN_SHRUG,  POS_RESTING, do_action, 0, 0, 0, 0); */
    /* COMMANDO( "sing", ACTN_SING,  POS_RESTING, do_action, 0, 0, 0, 0); */
    /* COMMANDO( "slap", ACTN_SLAP,  POS_RESTING, do_action, 0, 0, 0, 0); */
    /* COMMANDO( "smirk", ACTN_SMIRK,  POS_RESTING, do_action, 0, 0, 0, 0); */
    /* COMMANDO( "snap", ACTN_SNAP,  POS_RESTING, do_action, 0, 0, 0, 0); */
    /* COMMANDO( "sneeze", ACTN_SNEEZE,  POS_RESTING, do_action, 0, 0, 0, 0); */
    /* COMMANDO( "snicker", ACTN_SNICKER,  POS_RESTING, do_action, 0, 0, 0, 0); */
    /* COMMANDO( "sniff", ACTN_SNIFF,  POS_RESTING, do_action, 0, 0, 0, 0); */
    /* COMMANDO( "snore", ACTN_SNORE,  POS_SLEEPING, do_action, 0, 0, 0, 0); */
    /* COMMANDO( "spit", ACTN_SPIT,  POS_STANDING, do_action, 0, 0, 0, 0); */
    /* COMMANDO( "squeeze", ACTN_SQUEEZE,  POS_RESTING, do_action, 0, 0, 0, 0); */
    /* COMMANDO( "stare", ACTN_STARE,  POS_RESTING, do_action, 0, 0, 0, 0); */
    /* COMMANDO( "strut", ACTN_STRUT,  POS_STANDING, do_action, 0, 0, 0, 0); */
    /* COMMANDO( "thank", ACTN_THANK,  POS_RESTING, do_action, 0, 0, 0, 0); */
    /* COMMANDO( "twiddle", ACTN_TWIDDLE,  POS_RESTING, do_action, 0, 0, 0, 0); */
    /* COMMANDO( "wave", ACTN_WAVE,  POS_RESTING, do_action, 0, 0, 0, 0); */
    /* COMMANDO( "whistle", ACTN_WHISTLE,  POS_RESTING, do_action, 0, 0, 0, 0); */
    /* COMMANDO( "wiggle", ACTN_WIGGLE,  POS_STANDING, do_action, 0, 0, 0, 0); */
    /* COMMANDO( "wink", ACTN_WINK,  POS_RESTING, do_action, 0, 0, 0, 0); */
    /* COMMANDO( "yawn", ACTN_YAWN,  POS_RESTING, do_action, 0, 0, 0, 0); */
    /* COMMANDO( "snowball", ACTN_SNOWBALL,  POS_STANDING, do_action, IMO, IMO, IMO, IMO); */
    COMMANDO( "extractrent", CMD_EXTRACTRENT, POS_STANDING, do_extractrent,
	     IMO + 2, IMO + 2, IMO + 2, IMO + 2);
    COMMANDO( "hold", CMD_HOLD, POS_RESTING, do_grab, 1, 1, 1, 1);
    COMMANDO( "flee", CMD_FLEE, POS_FIGHTING, do_flee, 1, 1, 1, 1);
    COMMANDO( "sneak", CMD_SNEAK, POS_STANDING, do_sneak, IMO, IMO, 1, IMO);
    COMMANDO( "hide", CMD_HIDE, POS_RESTING, do_hide, IMO, IMO, 1, IMO);
    /* COMMANDO(154, POS_DEAD, do_backstab, IMO, IMO, 1, IMO);  */
    COMMANDO( "backstab", CMD_BACKSTAB, POS_STANDING, do_backstab, IMO, IMO, 1, IMO);/* jhpark */
    COMMANDO( "pick", CMD_PICK, POS_STANDING, do_pick, 20, 20, 1, 10);
    COMMANDO( "steal", CMD_STEAL, POS_STANDING, do_steal, IMO, IMO, 1, IMO);
    COMMANDO( "bash", CMD_BASH, POS_FIGHTING, do_bash, IMO, IMO, IMO, 1);
    COMMANDO( "rescue", CMD_RESCUE, POS_FIGHTING, do_rescue, 30, 30, 10, 1);
    COMMANDO( "kick", CMD_KICK, POS_FIGHTING, do_kick, IMO, IMO, IMO, 1);
    /* COMMANDO( "french", ACTN_FRENCH,  POS_RESTING, do_action, 0, 0, 0, 0); */
    /* COMMANDO( "comb", ACTN_COMB,  POS_RESTING, do_action, 0, 0, 0, 0); */
    /* COMMANDO( "massage", ACTN_MASSAGE,  POS_RESTING, do_action, 0, 0, 0, 0); */
    /* COMMANDO( "tickle", ACTN_TICKLE,  POS_RESTING, do_action, 0, 0, 0, 0); */
    COMMANDO( "practice", CMD_PRACTICE, POS_RESTING, do_practice, 1, 1, 1, 1);
    /* COMMANDO( "pat", ACTN_PAT,  POS_RESTING, do_action, 0, 0, 0, 0); */
    COMMANDO( "examine", CMD_EXAMINE, POS_SITTING, do_examine, 0, 0, 0, 0);
    COMMANDO( "take", CMD_TAKE, POS_RESTING, do_get, 0, 0, 0, 0);
    /* NOTE: do_sayh() is renamed to do_say() */
    /* COMMANDO(169, POS_RESTING, do_sayh, 0, 0, 0, 0);*/
    COMMANDO( "'", CMD_QUOTE, POS_RESTING, do_say, 0, 0, 0, 0);
    /* NOTE: command 'practise' is misspelling of 'practice'	*/
    /* COMMANDO( "practise", CMD_PRACTISE, POS_RESTING, do_practice, 1, 1, 1, 1); */
    /* COMMANDO( "curse", ACTN_CURSE,  POS_RESTING, do_action, 0, 0, 0, 0); */
    COMMANDO( "use", CMD_USE, POS_SITTING, do_use, 1, 1, 1, 1);
    COMMANDO( "where", CMD_WHERE, POS_DEAD, do_where, 1, 1, 1, 1);
    COMMANDO( "levels", CMD_LEVELS, POS_DEAD, do_levels, 0, 0, 0, 0);
    COMMANDO( "reroll", CMD_REROLL, POS_DEAD, do_reroll, IMO + 3, IMO + 3, IMO + 3, IMO + 3);
    /* COMMANDO( "pray", ACTN_PRAY,  POS_SITTING, do_action, 0, 0, 0, 0); */
    COMMANDO( "~", CMD_TILDE, POS_SLEEPING, do_emote, 1, 1, 1, 1);
    /* COMMANDO( "beg", ACTN_BEG,  POS_RESTING, do_action, 0, 0, 0, 0); */
    /* COMMANDO( "piss", ACTN_PISS,  POS_STANDING, do_action, 0, 0, 0, 0); */
    /* COMMANDO( "cringe", ACTN_CRINGE,  POS_RESTING, do_action, 0, 0, 0, 0); */
    /* COMMANDO( "daydream", ACTN_DAYDREAM,  POS_SLEEPING, do_action, 0, 0, 0, 0); */
    /* COMMANDO( "fume", ACTN_FUME,  POS_RESTING, do_action, 0, 0, 0, 0); */
    /* COMMANDO( "grovel", ACTN_GROVEL,  POS_RESTING, do_action, 0, 0, 0, 0); */
    /* COMMANDO( "hop", ACTN_HOP,  POS_RESTING, do_action, 0, 0, 0, 0); */
    /* COMMANDO( "nudge", ACTN_NUDGE,  POS_RESTING, do_action, 0, 0, 0, 0); */
    /* COMMANDO( "peer", ACTN_PEER,  POS_RESTING, do_action, 0, 0, 0, 0);  */
    /* NOTE: Now, 'point' action will set target. If not, just social action. */
    /* COMMANDO(187, POS_RESTING, do_action, 0, 0, 0, 0); */
    COMMANDO( "point", CMD_POINT, POS_RESTING, do_point, 0, 0, 0, 0);

    /* COMMANDO( "ponder", ACTN_PONDER,  POS_RESTING, do_action, 0, 0, 0, 0); */
    /* COMMANDO( "drool", ACTN_DROOL,  POS_RESTING, do_action, 0, 0, 0, 0); */
    /* COMMANDO( "snarl", ACTN_SNARL,  POS_RESTING, do_action, 0, 0, 0, 0); */
    /* COMMANDO( "spank", ACTN_SPANK,  POS_RESTING, do_action, 0, 0, 0, 0); */
    COMMANDO( "shoot", CMD_SHOOT, POS_RESTING, do_shoot, 1, 1, 1, 1);
    /* COMMANDO( "bark", ACTN_BARK,  POS_RESTING, do_action, 0, 0, 0, 0); */
    /* COMMANDO( "taunt", ACTN_TAUNT,  POS_RESTING, do_action, 0, 0, 0, 0); */
    /* COMMANDO( "think", ACTN_THINK,  POS_RESTING, do_action, 0, 0, 0, 0); */
    /* COMMANDO( "whine", ACTN_WHINE,  POS_RESTING, do_action, 0, 0, 0, 0); */
    /* COMMANDO( "worship", ACTN_WORSHIP,  POS_RESTING, do_action, 0, 0, 0, 0); */
    /* COMMANDO( "yodel", ACTN_YODEL,  POS_RESTING, do_action, 0, 0, 0, 0); */
    /* NOTE: 'brief' command is replaced by 'set brief yes/no'. */
    /* COMMANDO(199, POS_DEAD, do_brief, 0, 0, 0, 0); */
    COMMANDO( "brief", CMD_BRIEF, POS_DEAD, subst_set, 0, 0, 0, 0);

/* ------------------------------------------------------------------------- */

    COMMANDO( "wiznet", CMD_WIZNET, POS_DEAD, do_wiznet, IMO, IMO, IMO, IMO);
    COMMANDO( "consider", CMD_CONSIDER, POS_RESTING, do_consider, 0, 0, 0, 0);
    COMMANDO( "group", CMD_GROUP, POS_RESTING, do_group, 1, 1, 1, 1);
    COMMANDO( "restore", CMD_RESTORE, POS_DEAD, do_restore, IMO + 2, IMO + 2, IMO + 2, IMO + 2);
    COMMANDO( "return", CMD_RETURN, POS_DEAD, do_return, 0, 0, 0, 0);
    COMMANDO( "switch", CMD_SWITCH, POS_DEAD, do_switch, IMO + 2, IMO + 2, IMO + 2, IMO + 2);
    COMMANDO( "quaff", CMD_QUAFF, POS_RESTING, do_quaff, 0, 0, 0, 0);
    COMMANDO( "recite", CMD_RECITE, POS_RESTING, do_recite, 0, 0, 0, 0);
    COMMANDO( "users", CMD_USERS, POS_DEAD, do_users, 2, 2, 2, 2);
    COMMANDO( "flag", CMD_FLAG, POS_STANDING, do_flag, IMO + 1, IMO + 1, IMO + 1, IMO + 1);
    /* NOTE: Now, 'noshout' command is replaced by 'set' and 'flag <char> shutup'.  */
    /* COMMANDO( "noshout", CMD_NOSHOUT, POS_SLEEPING, do_noshout, 0, 0, 0, 0); */
    COMMANDO( "noshout", CMD_NOSHOUT, POS_SLEEPING, subst_set, 0, 0, 0, 0);
    COMMANDO( "wizhelp", CMD_WIZHELP, POS_SLEEPING, do_wizhelp, IMO, IMO, IMO, IMO);
    /* NOTE: 'credits' command is substituted by 'show credits'.  */
    /* COMMANDO(212, POS_DEAD, do_credits, 0, 0, 0, 0); */
    COMMANDO( "credits", CMD_CREDITS, POS_DEAD, subst_show, 0, 0, 0, 0);
    /* NOTE: 'compact' command is replaced by 'set tell yes/no'. */
    /* COMMANDO(213, POS_DEAD, do_compact, 0, 0, 0, 0); */
    COMMANDO( "compact", CMD_COMPACT, POS_DEAD, subst_set, 0, 0, 0, 0);
    COMMANDO( "flick", CMD_FLICK, POS_DEAD, do_flick, IMO + 3, IMO + 3, IMO + 3, IMO + 3);
    COMMANDO( "wall", CMD_WALL, POS_DEAD, do_wall, IMO + 3, IMO + 3, IMO + 3, IMO + 3);
    /* NOTE: OLD 'set' command renamed as 'wizset'   */
    /* COMMANDO(216, POS_DEAD, do_set, IMO + 3, IMO + 3, IMO + 3, IMO + 3); */
    COMMANDO( "wizset", CMD_WIZSET, POS_DEAD, do_wizset, IMO + 3, IMO + 3, IMO + 3, IMO + 3);
    COMMANDO( "police", CMD_POLICE, POS_DEAD, do_police, IMO + 3, IMO + 3, IMO + 3, IMO + 3);
    COMMANDO( "wizlock", CMD_WIZLOCK, POS_DEAD, do_wizlock, IMO + 3, IMO + 3, IMO + 3, IMO + 3);
    COMMANDO( "noaffect", CMD_NOAFFECT, POS_DEAD, do_noaffect, IMO + 1, IMO + 1, IMO + 1, IMO + 1);
    COMMANDO( "invis", CMD_INVIS, POS_DEAD, do_invis, IMO, IMO, IMO, IMO);
    /* NOTE: 'notell' command is replaced by 'set tell yes/no'. */
    /* COMMANDO(221, POS_DEAD, do_notell, 0, 0, 0, 0); */
    COMMANDO( "notell", CMD_NOTELL, POS_DEAD, subst_set, 0, 0, 0, 0);
    COMMANDO( "banish", CMD_BANISH, POS_DEAD, do_banish, IMO + 3, IMO + 3, IMO + 3, IMO + 3);
    COMMANDO( "reload", CMD_RELOAD, POS_RESTING, do_reload, 1, 1, 1, 1);
    COMMANDO( "data", CMD_DATA, POS_DEAD, do_data, IMO, IMO, IMO, IMO);
    COMMANDO( "checkrent", CMD_CHECKRENT, POS_DEAD, do_checkrent, IMO, IMO, IMO, IMO);
    COMMANDO( "chat", CMD_CHAT, POS_DEAD, do_chat, 10, 10, 10, 10);
    COMMANDO( "balance", CMD_BALANCE, POS_DEAD, do_bank, 1, 1, 1, 1);
    COMMANDO( "deposit", CMD_DEPOSIT, POS_DEAD, do_bank, 1, 1, 1, 1);
    COMMANDO( "withdraw", CMD_WITHDRAW, POS_DEAD, do_bank, 1, 1, 1, 1);
    COMMANDO( "sys", CMD_SYS, POS_DEAD, do_sys, IMO, IMO, IMO, IMO);
    /* NOTE: 'log' command is replace by 'flag <char> log'.  */
    /* COMMANDO( "log", CMD_LOG, POS_DEAD, do_flag, IMO + 3, IMO + 3, IMO + 3, IMO + 3); */
    COMMANDO( "log", CMD_LOG, POS_DEAD, subst_flag, IMO + 3, IMO + 3, IMO + 3, IMO + 3);
    COMMANDO( "mstat", CMD_MSTAT, POS_DEAD, do_stat, IMO, IMO, IMO, IMO);
    COMMANDO( "pstat", CMD_PSTAT, POS_DEAD, do_stat, IMO, IMO, IMO, IMO);
    COMMANDO( "tornado", CMD_TORNADO, POS_FIGHTING, do_tornado, IMO, IMO, 20, 13);
    COMMANDO( "light", CMD_LIGHT, POS_FIGHTING, do_light_move, IMO, IMO, 1, IMO);
    /* NOTE: 'title' command is replaced by 'set title <title>'. */
    /* COMMANDO(236, POS_SLEEPING, do_title, 1, 1, 1, 1); */
    COMMANDO( "title", CMD_TITLE, POS_SLEEPING, subst_set, 1, 1, 1, 1);
    COMMANDO( "report", CMD_REPORT, POS_SLEEPING, do_report, 1, 1, 1, 1);
    COMMANDO( "spells", CMD_SPELLS, POS_SLEEPING, do_spells, 1, 1, 1, 1);
    COMMANDO( "flash", CMD_FLASH, POS_FIGHTING, do_flash, IMO, IMO, 5, IMO);
    COMMANDO( "multi kick", CMD_MULTI_KICK, POS_FIGHTING, do_multi_kick, IMO, IMO, IMO, 17);
    COMMANDO( "demote", CMD_DEMOTE, POS_FIGHTING, do_demote,
	     IMO + 3, IMO + 3, IMO + 3, IMO + 3);
    /* NOTE: 'nochat' command is replaced by 'set chat yes/no'. */
    /* COMMANDO(242, POS_DEAD, do_nochat, 1, 1, 1, 1); */
    COMMANDO( "nochat", CMD_NOCHAT, POS_DEAD, subst_set, 1, 1, 1, 1);
    COMMANDO( "wimpy", CMD_WIMPY, POS_FIGHTING, do_wimpy, 1, 1, 1, 1);
    /* COMMANDO( "wimpy", CMD_WIMPY, POS_FIGHTING, subst_set, 1, 1, 1, 1);*/
    COMMANDO( "gtell", CMD_GTELL, POS_DEAD, do_gtell, 1, 1, 1, 1);
    COMMANDO( "send", CMD_SEND, POS_RESTING, do_send, 19, 19, 19, 19);
    COMMANDO( "write", CMD_WRITE, POS_RESTING, do_write, 0, 0, 0, 0);
    COMMANDO( "post", CMD_POST, POS_RESTING, do_post, 0, 0, 0, 0);
    /* COMMANDO( "웃음", ACTN_웃음,  POS_RESTING, do_action, 0, 0, 0, 0); */
    /* COMMANDO( "말", ACTN_말,  POS_RESTING, do_action, 0, 0, 0, 0); */
    /* COMMANDO( "울음", ACTN_울음,  POS_RESTING, do_action, 0, 0, 0, 0); */
    /* COMMANDO( "춤", ACTN_춤,  POS_STANDING, do_action, 0, 0, 0, 0); */
    COMMANDO( "ㅣ", CMD_HLOOK, POS_RESTING, do_look, 0, 0, 0, 0);
    /* COMMANDO( "하하", ACTN_하하,  POS_RESTING, do_action, 0, 0, 0, 0); */
    /* COMMANDO( "체팅", ACTN_체팅,  POS_RESTING, do_action, 0, 0, 0, 0); */
    /* COMMANDO( "haha", ACTN_HAHA,  POS_RESTING, do_action, 0, 0, 0, 0); */
    /* COMMANDO( "jjup", ACTN_JJUP,  POS_RESTING, do_action, 0, 0, 0, 0); */
    /* COMMANDO( "wow", ACTN_WOW,  POS_RESTING, do_action, 0, 0, 0, 0); */
    /* COMMANDO( "bye", ACTN_BYE,  POS_RESTING, do_action, 0, 0, 0, 0); */
    /* COMMANDO( "hee", ACTN_HEE,  POS_RESTING, do_action, 0, 0, 0, 0); */
    /* COMMANDO( "brb", ACTN_BRB,  POS_RESTING, do_action, 0, 0, 0, 0); */
    /* COMMANDO( "hmm", ACTN_HMM,  POS_RESTING, do_action, 0, 0, 0, 0); */
    COMMANDO( "assist", CMD_ASSIST, POS_FIGHTING, do_assist, 0, 0, 0, 0);
    COMMANDO( "ungroup", CMD_UNGROUP, POS_RESTING, do_ungroup, 1, 1, 1, 1);
    /* NOTE: 'wizlist' command is substituted by 'show wizards'. */
    /* COMMANDO(264, POS_SLEEPING, do_wizards, 0, 0, 0, 0);  */
    COMMANDO( "wizlist", CMD_WIZLIST, POS_SLEEPING, subst_show, 0, 0, 0, 0);
    /* NOTE: 'hangul' command is replaced by 'set hangul yes/no'. */
    /* COMMANDO(265, POS_DEAD, do_hangul, 0, 0, 0, 0); */
    COMMANDO( "hangul", CMD_HANGUL, POS_DEAD, subst_set, 0, 0, 0, 0);
    /* NOTE: 'NEWS' command is substituted by 'show news'.  */
    /* COMMANDO(266, POS_SLEEPING, do_news, 0, 0, 0, 0); */
    COMMANDO( "NEWS", CMD_NEWS, POS_SLEEPING, subst_show, 0, 0, 0, 0);
    /* NOTE: NEW! do_set() replace do_version position. */
    /* COMMANDO(267, POPOSNG, do_version, IMO, IMO, IMO, IMO); */
    COMMANDO( "set", CMD_SET, POS_SLEEPING, do_set, 0, 0, 0, 0);
    COMMANDO( "disarm", CMD_DISARM, POS_FIGHTING, do_disarm, IMO, IMO, 1, 10);
    COMMANDO( "shouryuken", CMD_SHOURYUKEN, POS_FIGHTING, do_shouryuken, IMO, IMO, IMO, 30);
    COMMANDO( "throw", CMD_THROW, POS_FIGHTING, do_throw_object, IMO, IMO, 30, IMO);
    COMMANDO( "punch", CMD_PUNCH, POS_FIGHTING, do_punch, IMO, IMO, 30, 25);
    COMMANDO( "assault", CMD_ASSAULT, POS_FIGHTING, do_assault, 1, 1, 1, 1);
    COMMANDO( "JOIN", CMD_JOIN, POS_STANDING, do_not_here, 10, 10, 10, 10);
    COMMANDO( "LEAVE", CMD_LEAVE, POS_STANDING, do_not_here, 10, 10, 10, 10);
    COMMANDO( "train", CMD_TRAIN, POS_STANDING, do_not_here, 10, 10, 10, 10);
    COMMANDO( "cant", CMD_CANT, POS_SLEEPING, do_cant, 10, 10, 10, 10);
    /* COMMANDO( "SAVE", CMD_SAVE, POS_STANDING, do_not_here, 10, 10, 10, 10); */
    /* COMMANDO( "LOAD", CMD_LOAD, POS_STANDING, do_not_here, 10, 10, 10, 10); */
    COMMANDO( "QUERY", CMD_QUERY, POS_STANDING, do_query, 10, 10, 10, 10);
    COMMANDO( "broadcast", CMD_BROADCAST, POS_STANDING, do_whistle, 15, 15, 15, 15);
    COMMANDO( "simultaneous", CMD_SIMULTANEOUS, POS_FIGHTING, do_simultaneous, 15, 15, 15, 15);
    COMMANDO( "arrest", CMD_ARREST, POS_FIGHTING, do_arrest, 15, 15, 15, 15);
    COMMANDO( "angry yell", CMD_ANGRY_YELL, POS_FIGHTING, do_angry_yell, 20, 20, 20, 20);
    COMMANDO( "solace", CMD_SOLACE, POS_FIGHTING, do_solace, 25, 25, 25, 25);
    /* COMMANDO( "unwield", CMD_UNWIELD, POS_FIGHTING, do_unwield, 0, 0, 0, 0); */
    COMMANDO( "unwield", CMD_UNWIELD, POS_FIGHTING, subst_holdwield, 0, 0, 0, 0);
    /* COMMANDO( "unhold", CMD_UNHOLD, POS_FIGHTING, do_unhold, 0, 0, 0, 0); */
    COMMANDO( "unhold", CMD_UNHOLD, POS_FIGHTING, subst_holdwield, 0, 0, 0, 0);
    COMMANDO( "temptation", CMD_TEMPTATION, POS_FIGHTING, do_temptation, 10, 10, 10, 10);
    COMMANDO( "shadow figure", CMD_SHADOW_FIGURE, POS_STANDING, do_shadow, 25, 25, 25, 25);
    COMMANDO( "smoke", CMD_SMOKE, POS_STANDING, do_smoke, 20, 20, 20, 20);
    COMMANDO( "inject", CMD_INJECT, POS_STANDING, do_inject, 20, 20, 20, 20);
    /* NOTE: 'plan' command is substituted by 'show plan'.  */
    /* COMMANDO(291, POS_SLEEPING, do_plan, 1, 1, 1, 1); */
    /* COMMANDO(291, POS_SLEEPING, subst_show, 1, 1, 1, 1); */
    COMMANDO( "show", CMD_SHOW, POS_SLEEPING, do_show, 1, 1, 1, 1);
    COMMANDO( "power bash", CMD_POWER_BASH, POS_FIGHTING, do_power_bash, 15, 15, 15, 15);
    COMMANDO( "evil strike", CMD_EVIL_STRIKE, POS_FIGHTING, do_evil_strike, 25, 25, 25, 25);
    // NOTE: 'call' => 'taxi <zone>' ('taxi kaist', 'taxi process')
    COMMANDO( "taxi", CMD_TAXI, POS_STANDING, do_not_here, 1, 1, 1, 1);
    COMMANDO( "charge", CMD_CHARGE, POS_FIGHTING, do_charge, 20, 20, 20, 20);
    /* NOTE: 'solo' command is substituted by 'set solo on/off'.  */
    /* COMMANDO(296, POS_STANDING, do_solo, 1, 1, 1, 1); */
    COMMANDO( "solo", CMD_SOLO, POS_STANDING, subst_set, 1, 1, 1, 1);
    /* NOTE: 'dumb' command is replace by 'flag <char> dumb'.  */
    /* COMMANDO( "dumb", CMD_DUMB, POS_STANDING, do_flag, IMO + 3, IMO + 3, IMO + 3, IMO + 3); */
    COMMANDO( "dumb", CMD_DUMB, POS_STANDING, subst_flag, IMO + 3, IMO + 3, IMO + 3, IMO + 3);
    COMMANDO( "spin bird kick", CMD_SPIN_BIRD_KICK, POS_FIGHTING, do_spin_bird_kick, IMO, IMO, IMO, 30);
    COMMANDO( "view", CMD_VIEW, POS_STANDING, do_not_here, 1, 1, 1, 1);
    COMMANDO( "reply", CMD_REPLY, POS_SLEEPING, do_reply, 1, 1, 1, 1);
    COMMANDO( "quest", CMD_QUEST, POS_SLEEPING, do_quest, 10, 10, 10, 10);
    COMMANDO( "request", CMD_REQUEST, POS_SLEEPING, do_request, 10, 10, 10, 10);
    COMMANDO( "hint", CMD_HINT, POS_SLEEPING, do_hint, 10, 10, 10, 10);
    COMMANDO( "pull", CMD_PULL, POS_SLEEPING, do_not_here, 1, 1, 1, 1);
    COMMANDO( "change", CMD_CHANGE, POS_SLEEPING, do_not_here, 1, 1, 1, 1);
    COMMANDO( ":", CMD_COLON, POS_DEAD, do_wiznet, IMO, IMO, IMO, IMO);
    COMMANDO( "lastchat", CMD_LASTCHAT, POS_SLEEPING, do_lastchat, 1, 1, 1, 1);
    /* NOTE: NEW!!! house command. */
    COMMANDO( "house", CMD_HOUSE, POS_STANDING, do_house, 1, 1, 1, 1);
    COMMANDO( "hcontrol", CMD_HCONTROL, POS_STANDING, do_hcontrol, IMO +3, IMO + 3, IMO + 3, IMO + 3);
    /* NOTE: NEW!!! track command. */
    COMMANDO( "track", CMD_TRACK, POS_STANDING, do_track, 20, 20, 1, 10);
    // NOTE: call command is obsolete.
    COMMANDO( "call", CMD_TAXI, POS_STANDING, do_not_here, 1, 1, 1, 1);
    /* NOTE: Mark end of valid cmd_info[] */
    COMMANDO(NULL, -1, 0, NULL, 0, 0, 0, 0);
}

void do_wizhelp(struct char_data *ch, char *argument, int cmd)
{
    char buf[MAX_STRING_LENGTH];
    int no, i, j;

    if (IS_NPC(ch))
	return;
    send_to_char("The following privileged commands are available:\r\n", ch);
    *buf = '\0';
    no = 1;
    /* NOTE: Modified according to changue of struct command_info. */
    for ( i = 1; i < MAX_CMD_LIST ; i++) {
	if ( !cmd_info[i]
		|| GET_LEVEL(ch) < cmd_info[i]->minimum_level[GET_CLASS(ch)-1])
	    continue;
	for (j = 0; j < 4; j++)
	    if ( LEVEL_LIMIT >= cmd_info[i]->minimum_level[j])
		goto no_wizcmd;

	sprintf(buf + strlen(buf), "%-15s", cmd_info[i]->command_str);
	if (!(no % 4))
	    strcat(buf, "\r\n");
	no++;
no_wizcmd:;
    }
    strcat(buf, "\r\n");
    page_string(ch->desc, buf, 1);
}

#ifdef UNUSED_CODE
/* NOTE: NEW to access command_info structure.
	Check ch's class, minimum level, minimum position for that command. */
int cando_action( struct char_data *ch, int cmd_nr )
{
    int mid;
    int bot = 0;
    int top = MAX_CMD_LIST;

    /* NOTE: cmd_info should be sorted, but dont need to be sequential */
    for(;;) {
	mid = (top + bot)/2;
	ASSERT(cmd_info[mid]);
	if( cmd_info[mid]->command_nr == cmd_nr )
	    break;
	if( bot >= top )
	    return(FALSE);
	if ( cmd_info[mid]->command_nr > cmd_nr )
	    top = --mid;
	else
	    bot = ++mid;
    }
    if( ( cmd_info[mid]->minimum_position > GET_POS(ch))
	|| (cmd_info[mid]->minimum_level[GET_CLASS(ch)-1] > GET_LEVEL(ch)))
	return(FALSE);
    return(TRUE);
}
#endif 		/* UNUSED_CODE */
