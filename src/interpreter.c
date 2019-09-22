/* ************************************************************************
*  file: Interpreter.c , Command interpreter module.      Part of DIKUMUD *
*  Usage: Procedures interpreting user command                            *
*  Copyright (C) 1990, 1991 - see 'license.doc' for complete information. *
************************************************************************* */

#include <string.h>
#include <ctype.h>
#include <stdio.h>
#include <sys/types.h>

// Don't use obsolete termio.h, use modern termios.h
#undef DONT_HAVE_TERMIOS_H

#ifdef DONT_HAVE_TERMIOS_H
#include <termio.h>
#else
#include <termios.h>
#endif

#ifndef __FreeBSD__
#include <crypt.h>
#endif

#include <sys/uio.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#define	TELOPTS
#include <arpa/telnet.h>
#include "structs.h"
#include "comm.h"
#include "interpreter.h"
#include "db.h"
#include "utils.h"
#include "limit.h"

#define ISLETTER(c)	(c > ' ' || c < 0)

#define COMMANDO(number, min_pos, pointer, min_lm, min_lc, min_lt, min_lw) { \
  cmd_info[(number)].command_pointer = (pointer); \
  cmd_info[(number)].minimum_position = (min_pos); \
  cmd_info[(number)].minimum_level[0] = (min_lm); \
  cmd_info[(number)].minimum_level[1] = (min_lc); \
  cmd_info[(number)].minimum_level[2] = (min_lt); \
  cmd_info[(number)].minimum_level[3] = (min_lw); }

#define NOT !
#define AND &&
#define OR ||

#define STATE(d) ((d)->connected)
#define MAX_CMD_LIST 350 /*  max command list modified by chase */

int no_echo = 0;
extern struct title_type titles[4][IMO+4];
extern char motd[MAX_STRING_LENGTH];
extern char imotd[MAX_STRING_LENGTH];
extern struct char_data *character_list;
extern struct player_index_element *player_table;
extern int top_of_p_table;
extern struct index_data *mob_index;
extern struct index_data *obj_index;
extern struct room_data *world;
struct command_info cmd_info[MAX_CMD_LIST];


/* external fcntls */

void unstash_char(struct char_data *ch, char *name);
void stash_char(struct char_data *ch);
void set_title(struct char_data *ch);
void init_char(struct char_data *ch);
void store_to_char(struct char_file_u *st, struct char_data *ch);
int create_entry(char *name);
int special(struct char_data *ch, int cmd, char *arg);
void log(char *str);
int MIN(int a, int b);
int number(int from, int to);
int str_cmp(char *arg1, char *arg2);
void char_to_room(struct char_data *ch, int room);
void do_start(struct char_data *ch);

int remove_entry(struct char_data *ch);
void delete_char(struct char_data *ch);

void close_socket(struct descriptor_data *d);

void do_shoot(struct char_data *ch, char *argument, int cmd);
void do_wimpy(struct char_data *ch, char *argument, int cmd);
void do_move(struct char_data *ch, char *argument, int cmd);
void do_look(struct char_data *ch, char *argument, int cmd);
void do_read(struct char_data *ch, char *argument, int cmd);
void do_write(struct char_data *ch, char *argument, int cmd);
void do_say(struct char_data *ch, char *argument, int cmd);
void do_exit(struct char_data *ch, char *argument, int cmd);
void do_snoop(struct char_data *ch, char *argument, int cmd);
void do_insult(struct char_data *ch, char *argument, int cmd);
void do_quit(struct char_data *ch, char *argument, int cmd);
void do_help(struct char_data *ch, char *argument, int cmd);
void do_who(struct char_data *ch, char *argument, int cmd);
void do_emote(struct char_data *ch, char *argument, int cmd);
void do_echo(struct char_data *ch, char *argument, int cmd);
void do_trans(struct char_data *ch, char *argument, int cmd);
void do_kill(struct char_data *ch, char *argument, int cmd);
void do_stand(struct char_data *ch, char *argument, int cmd);
void do_sit(struct char_data *ch, char *argument, int cmd);
void do_rest(struct char_data *ch, char *argument, int cmd);
void do_sleep(struct char_data *ch, char *argument, int cmd);
void do_wake(struct char_data *ch, char *argument, int cmd);
void do_force(struct char_data *ch, char *argument, int cmd);
void do_get(struct char_data *ch, char *argument, int cmd);
void do_drop(struct char_data *ch, char *argument, int cmd);
void do_news(struct char_data *ch, char *argument, int cmd);
void do_score(struct char_data *ch, char *argument, int cmd);
void do_title(struct char_data *ch, char *argument, int cmd);
void do_spells(struct char_data *ch, char *argument, int cmd);
void do_report(struct char_data *ch, char *argument, int cmd);
void do_inventory(struct char_data *ch, char *argument, int cmd);
void do_equipment(struct char_data *ch, char *argument, int cmd);
void do_shout(struct char_data *ch, char *argument, int cmd);
void do_not_here(struct char_data *ch, char *argument, int cmd);
void do_tell(struct char_data *ch, char *argument, int cmd);
void do_send(struct char_data *ch, char *argument, int cmd);
void do_gtell(struct char_data *ch, char *argument, int cmd);
void do_wear(struct char_data *ch, char *argument, int cmd);
void do_wield(struct char_data *ch, char *argument, int cmd);
void do_grab(struct char_data *ch, char *argument, int cmd);
void do_remove(struct char_data *ch, char *argument, int cmd);
void do_put(struct char_data *ch, char *argument, int cmd);
void do_shutdown(struct char_data *ch, char *argument, int cmd);
void do_save(struct char_data *ch, char *argument, int cmd);
void do_hit(struct char_data *ch, char *argument, int cmd);
void do_string(struct char_data *ch, char *arg, int cmd);
void do_give(struct char_data *ch, char *arg, int cmd);
void do_stat(struct char_data *ch, char *arg, int cmd);
void do_time(struct char_data *ch, char *arg, int cmd);
void do_weather(struct char_data *ch, char *arg, int cmd);
void do_load(struct char_data *ch, char *arg, int cmd);
void do_purge(struct char_data *ch, char *arg, int cmd);
void do_shutdow(struct char_data *ch, char *arg, int cmd);
void do_whisper(struct char_data *ch, char *arg, int cmd);
void do_cast(struct char_data *ch, char *arg, int cmd);
void do_at(struct char_data *ch, char *arg, int cmd);
void do_goto(struct char_data *ch, char *arg, int cmd);
void do_ask(struct char_data *ch, char *arg, int cmd);
void do_drink(struct char_data *ch, char *arg, int cmd);
void do_eat(struct char_data *ch, char *arg, int cmd);
void do_pour(struct char_data *ch, char *arg, int cmd);
void do_sip(struct char_data *ch, char *arg, int cmd);
void do_taste(struct char_data *ch, char *arg, int cmd);
void do_order(struct char_data *ch, char *arg, int cmd);
void do_follow(struct char_data *ch, char *arg, int cmd);
void do_rent(struct char_data *ch, char *arg, int cmd);
void do_junk(struct char_data *ch, char *arg, int cmd);
void do_advance(struct char_data *ch, char *arg, int cmd);
void do_close(struct char_data *ch, char *arg, int cmd);
void do_open(struct char_data *ch, char *arg, int cmd);
void do_lock(struct char_data *ch, char *arg, int cmd);
void do_unlock(struct char_data *ch, char *arg, int cmd);
void do_exits(struct char_data *ch, char *arg, int cmd);
void do_enter(struct char_data *ch, char *arg, int cmd);
void do_leave(struct char_data *ch, char *arg, int cmd);
void do_flee(struct char_data *ch, char *arg, int cmd);
void do_sneak(struct char_data *ch, char *arg, int cmd);
void do_hide(struct char_data *ch, char *arg, int cmd);
void do_backstab(struct char_data *ch, char *arg, int cmd);
void do_pick(struct char_data *ch, char *arg, int cmd);
void do_steal(struct char_data *ch, char *arg, int cmd);
void do_bash(struct char_data *ch, char *arg, int cmd);
void do_rescue(struct char_data *ch, char *arg, int cmd);
void do_kick(struct char_data *ch, char *arg, int cmd);
void do_punch(struct char_data *ch, char *arg, int cmd);
void do_examine(struct char_data *ch, char *arg, int cmd);
void do_users(struct char_data *ch, char *arg, int cmd);
void do_where(struct char_data *ch, char *arg, int cmd);
void do_levels(struct char_data *ch, char *arg, int cmd);
void do_reroll(struct char_data *ch, char *arg, int cmd);
void do_brief(struct char_data *ch, char *arg, int cmd);
void do_wiznet(struct char_data *ch, char *arg, int cmd);
void do_consider(struct char_data *ch, char *arg, int cmd);
void do_group(struct char_data *ch, char *arg, int cmd);
void do_restore(struct char_data *ch, char *arg, int cmd);
void do_return(struct char_data *ch, char *argument, int cmd);
void do_switch(struct char_data *ch, char *argument, int cmd);
void do_quaff(struct char_data *ch, char *argument, int cmd);
void do_recite(struct char_data *ch, char *argument, int cmd);
void do_use(struct char_data *ch, char *argument, int cmd);
void do_flag(struct char_data *ch, char *argument, int cmd);
void do_noshout(struct char_data *ch, char *argument, int cmd);
void do_wizhelp(struct char_data *ch, char *argument, int cmd);
void do_credits(struct char_data *ch, char *argument, int cmd);
void do_compact(struct char_data *ch, char *argument, int cmd);
void do_action(struct char_data *ch, char *arg, int cmd);
void do_practice(struct char_data *ch, char *arg, int cmd);
void do_flick(struct char_data *ch, char *arg, int cmd);
void do_wall(struct char_data *ch, char *arg, int cmd);
void do_set(struct char_data *ch, char *arg, int cmd);
void do_police(struct char_data *ch, char *arg, int cmd);
void do_wizlock(struct char_data *ch, char *arg, int cmd);
void do_noaffect(struct char_data *ch, char *arg, int cmd);
void do_invis(struct char_data *ch, char *arg, int cmd);
void do_notell(struct char_data *ch, char *arg, int cmd);
void do_banish(struct char_data *ch, char *arg, int cmd);
void do_reload(struct char_data *ch, char *arg, int cmd);
void do_data(struct char_data *ch, char *arg, int cmd);
void do_checkrent(struct char_data *ch, char *arg, int cmd);
void do_chat(struct char_data *ch, char *arg, int cmd);
void do_lastchat(struct char_data *ch, char *arg, int cmd);
void do_bank(struct char_data *ch, char *arg, int cmd);
void do_sys(struct char_data *ch, char *arg, int cmd);
void do_extractrent(struct char_data *ch, char *arg, int cmd);
void do_replacerent(struct char_data *ch, char *arg, int cmd);
void do_tornado(struct char_data *ch, char *arg, int cmd);
void do_light_move(struct char_data *ch, char *arg, int cmd);
void do_flash(struct char_data *ch, char *arg, int cmd);
void do_multi_kick(struct char_data *ch, char *arg, int cmd);
void do_demote(struct char_data *ch, char *arg, int cmd);
void do_nochat(struct char_data *ch, char *arg, int cmd);
void do_post(struct char_data *ch, char *arg, int cmd);
void do_sayh(struct char_data *ch, char *argument, int cmd);
void do_assist(struct char_data *ch, char *argument, int cmd);
void do_ungroup(struct char_data *ch, char *argument, int cmd);
void do_wizards(struct char_data *ch, char *argument, int cmd);
void do_hangul(struct char_data *ch, char *arg, int cmd);
void do_version(struct char_data *ch, char *arg, int cmd);
void do_disarm(struct char_data *ch, char *arg, int cmd); /* chase */
void do_shouryuken(struct char_data *ch, char *arg, int cmd); /* chase */
void do_throw_object(struct char_data *ch, char *arg, int cmd); /* chase */
void do_assault(struct char_data *ch,char *arg,int cmd); /* by process */
void do_cant(struct char_data *ch,char *arg,int cmd); /* by process */
void do_query(struct char_data *ch,char *arg,int cmd);/* by process */
void do_whistle(struct char_data *ch,char *arg,int cmd); /* by process */
void do_simultaneous(struct char_data *ch,char *arg,int cmd); /* by process */
void do_arrest(struct char_data *ch,char *arg,int cmd); /* by process */
void do_angry_yell(struct char_data *ch,char *arg,int cmd); /* by process */
void do_solace(struct char_data *ch,char *arg,int cmd); /* by process */
void do_unwield(struct char_data *ch,char *arg,int cmd); /* by process */
void do_unhold(struct char_data *ch,char *arg,int cmd); /* by process */
void do_temptation(struct char_data *ch,char *arg,int cmd); /* by process */
void do_shadow(struct char_data *ch,char *arg,int cmd); /* by process */
void do_smoke(struct char_data *ch,char *arg,int cmd); /* by process */
void do_inject(struct char_data *ch,char *arg,int cmd); /* by process */
void do_plan(struct char_data *ch,char *arg,int cmd); /* by process */
void do_power_bash(struct char_data *ch,char *arg,int cmd); /* by process */
void do_evil_strike(struct char_data *ch,char *arg,int cmd); /* by process */
void do_charge(struct char_data *ch,char *arg,int cmd); /* by process */
void do_solo(struct char_data *ch,char *arg,int cmd); /* by process */
void do_auto_assist(struct char_data *ch,char *arg,int cmd); /* by process */
void do_spin_bird_kick(struct char_data *ch,char *arg,int cmd); /* by ares */
void do_reply(struct char_data *ch,char *arg,int cmd); /* by process */

/* quest */
void do_quest(struct char_data *ch,char *arg,int cmd); /* by atre */
void do_request(struct char_data *ch,char *arg,int cmd); /* by atre */
void do_hint(struct char_data *ch,char *arg,int cmd); /* by atre */

#ifdef UNUSED_CODE
char *command[]=
{ "north",        /* 1 */
  "east",
  "south",
  "west",
  "up",
  "down",
  "enter",
  "exits",
  "kiss",
  "get",
  "drink",       /* 11 */
  "eat",
  "wear",
  "wield",
  "look",
  "score",
  "say",
  "shout",
  "tell",
  "inventory",
  "qui",         /* 21 */
  "bounce",
  "smile",
  "dance",
  "kill",
  "cackle",
  "laugh",
  "giggle",
  "shake",
  "puke",
  "growl",       /* 31 */    
  "scream",
  "insult",
  "comfort",
  "nod",
  "sigh",
  "sulk",
  "help",
  "who",
  "emote",
  "echo",        /* 41 */
  "stand",
  "sit",
  "rest",
  "sleep",
  "wake",
  "force",
  "transfer",
  "hug",
  "snuggle",
  "cuddle",       /* 51 */
  "nuzzle",
  "cry",
  "news",
  "equipment",
  "buy",
  "sell",
  "value",
  "list",
  "drop",
  "goto",         /* 61 */
  "weather",
  "read",
  "pour",
  "grab",
  "remove",
  "put",
  "shutdow",
  "save",
  "hit",
  "string",      /* 71 */
  "give",
  "quit",
  "stat",
  "bellow",
  "time",
  "load",
  "purge",
  "shutdown",
  "scratch",
  "",        /* 81 */
  "replacerent",
  "whisper",
  "cast",
  "at",
  "ask",
  "order",
  "sip",
  "taste",
  "snoop",
  "follow",      /* 91 */
  "rent",
  "junk",
  "poke",
  "advance",
  "accuse",
  "grin",
  "bow",
  "open",
  "close",
  "lock",        /* 101 */
  "unlock",
  "leave",
  "applaud",
  "blush",
  "burp",
  "chuckle",
  "clap",
  "cough",
  "curtsey",
  "fart",        /* 111 */
  "flip",
  "fondle",
  "frown",
  "gasp",
  "glare",
  "groan",
  "grope",
  "hiccup",
  "lick",
  "love",        /* 121 */
  "moan",
  "nibble",
  "pout",
  "purr",
  "ruffle",
  "shiver",
  "shrug",
  "sing",
  "slap",
  "smirk",       /* 131 */
  "snap",
  "sneeze",
  "snicker",
  "sniff",
  "snore",
  "spit",
  "squeeze",
  "stare",
  "strut",
  "thank",       /* 141 */
  "twiddle",
  "wave",
  "whistle",
  "wiggle",
  "wink",
  "yawn",
  "snowball",
  "extractrent",
  "hold",
  "flee",        /* 151 */
  "sneak",
  "hide",
  "backstab",
  "pick",
  "steal",
  "bash",
  "rescue",
  "kick",
  "french",
  "comb",        /* 161 */
  "massage",
  "tickle",
  "practice",
  "pat",
  "examine",
  "take",
  "",
  "'",
  "practise",
  "curse",       /* 171 */
  "use",
  "where",
  "levels",
  "reroll",
  "pray",
  ",",
  "beg",
  "piss",
  "cringe",
  "daydream",    /* 181 */
  "fume",
  "grovel",
  "hop",
  "nudge",
  "peer",
  "point",
  "ponder",
  "drool",
  "snarl",
  "spank",       /* 191 */
  "shoot",
  "bark",
  "taunt",
  "think",
  "whine",
  "worship",
  "yodel",    /* 198 */
  "brief",
  "wiznet",
  "consider",    /* 201 */
  "group",
  "restore",
  "return",
  "switch",      /* 205 */
  "quaff",
  "recite",
  "users",
  "flag",
  "noshout",
  "wizhelp",   /* 211 */
  "credits",
  "compact",
  "flick",
  "wall",
  "set",
  "police",
  "wizlock",
  "noaffect",
  "invis",
  "notell",
  "banish",
  "reload",
  "data",
  "checkrent",   /* 225 */
  "chat",
  "balance",
  "deposit",
  "withdraw",
  "sys",
  "log",   /* 231 */
  "mstat",
  "pstat",
  "tornado",
  "light",
  "title",
  "report",
  "spells",
  "flash",
  "multi kick",
  "demote", /*241 */
  "nochat",
  "wimpy",
  "gtell",
  "send",
  "write",
  "post",
  "웃음",
  "말",
  "울음",
  "춤",   /* 251 */
  "ㅣ",
  "하하",
  "체팅",
  "haha",
  "jjup",
  "wow", /* 257 */
  "bye",
  "hee",
  "brb",
  "hmm",  /* 261 */
  "assist",
  "ungroup",
  "wizlist",
  "hangul",
  "NEWS",
  "version",
  "disarm", /* by chase 268 */
  "shouryuken", /* chase 269 */
  "throw", /* chase 270 */
  "punch", 
  "assault", /* process 272 */
  "JOIN", /* guild join 273*/
  "LEAVE", /* guild leave 274*/
  "train", /* guild skill practice 275*/
  "cant",/* guild chat 276 */
  "SAVE", /* 277 SAVE at locker room */
  "LOAD", /* 278 LOAD at locker room */
  "QUERY",	/* 279 QUERY,querys player's guild */
  "broadcast", /* 280 for police */
  "simultaneous", /* 281 for police */
  "arrest",/* 282 for police */
  "angry yell", /* 283 for outlaws */
  "solace", /* 284 for assasins */
  "unwield",
  "unhold",
  "temptation", /* 287 */
  "shadow figure", /* 288 for assasins */
  "smoke", /* 289 for outlaws */
  "inject", /* 290 for outlaws */
  "plan",
  "power bash",
  "evil strike",
  "call", /* 294 for board shuttle bus */
  "charge",
  "solo",
  "dumb",
  "spin bird kick",
  "view", /* used in new_shop.c */
  "reply",
  "", /* 301 */
  "quest",
  "request",
  "hint",
  "pull",
  "change",
  ":",
  "lastchat",
  "\n"
};
#endif	// UNUSED_CODE

char *command[]=
{ "north",        /* 1 */
  "east",
  "south",
  "west",
  "up",
  "down",
  "enter",
  "exits",
  "kill",
  "get",
  "drink",       /* 11 */
  "eat",
  "wear",
  "wield",
  "look",
  "score",
  "say",
  "shout",
  "tell",
  "inventory",
  "qui",         /* 21 */
  "backstab",
  "smile",
  "dance",
  "kiss",
  "chat",
  "laugh",
  "giggle",
  "shake",
  "puke",
  "group",       /* 31 */    
  "scream",
  "insult",
  "comfort",
  "nod",
  "sigh",
  "sulk",
  "help",
  "who",
  "emote",
  "echo",        /* 41 */
  "stand",
  "sit",
  "rest",
  "sleep",
  "wake",
  "follow",
  "transfer",
  "hug",
  "snuggle",
  "cuddle",       /* 51 */
  "nuzzle",
  "cry",
  "news",
  "equipment",
  "buy",
  "sell",
  "value",
  "list",
  "drop",
  "goto",         /* 61 */
  "weather",
  "read",
  "pour",
  "grab",
  "remove",
  "put",
  "shutdow",
  "save",
  "hit",
  "string",      /* 71 */
  "give",
  "quit",
  "stat",
  "bellow",
  "time",
  "load",
  "purge",
  "shutdown",
  "scratch",
  "",        /* 81 */
  "replacerent",
  "whisper",
  "cast",
  "at",
  "ask",
  "order",
  "sip",
  "taste",
  "snoop",
  "force",      /* 91 */
  "rent",
  "junk",
  "poke",
  "advance",
  "accuse",
  "grin",
  "bow",
  "open",
  "close",
  "lock",        /* 101 */
  "unlock",
  "leave",
  "applaud",
  "blush",
  "burp",
  "cackle",
  "clap",
  "cough",
  "curtsey",
  "fart",        /* 111 */
  "flee",
  "fondle",
  "frown",
  "gasp",
  "glare",
  "groan",
  "grope",
  "hiccup",
  "lick",
  "love",        /* 121 */
  "moan",
  "nibble",
  "pout",
  "purr",
  "ruffle",
  "shiver",
  "shrug",
  "sing",
  "slap",
  "smirk",       /* 131 */
  "snap",
  "sneeze",
  "snicker",
  "sniff",
  "snore",
  "spit",
  "squeeze",
  "stare",
  "strut",
  "thank",       /* 141 */
  "twiddle",
  "wave",
  "whistle",
  "wiggle",
  "wink",
  "yawn",
  "snowball",
  "extractrent",
  "hold",
  "flip",        /* 151 */
  "sneak",
  "hide",
  "bounce",
  "pick",
  "steal",
  "bash",
  "rescue",
  "kick",
  "french",
  "comb",        /* 161 */
  "massage",
  "tickle",
  "practice",
  "pat",
  "examine",
  "take",
  "",
  "'",
  "practise",
  "curse",       /* 171 */
  "use",
  "where",
  "levels",
  "reroll",
  "pray",
  ",",
  "beg",
  "piss",
  "cringe",
  "daydream",    /* 181 */
  "fume",
  "grovel",
  "hop",
  "nudge",
  "peer",
  "point",
  "ponder",
  "drool",
  "snarl",
  "spank",       /* 191 */
  "shoot",
  "bark",
  "taunt",
  "think",
  "whine",
  "worship",
  "yodel",    /* 198 */
  "brief",
  "wiznet",
  "consider",    /* 201 */
  "growl",
  "restore",
  "return",
  "switch",      /* 205 */
  "quaff",
  "recite",
  "users",
  "flash",
  "noshout",
  "wizhelp",   /* 211 */
  "credits",
  "compact",
  "flick",
  "wall",
  "set",
  "police",
  "wizlock",
  "noaffect",
  "invis",
  "notell",
  "banish",
  "reload",
  "data",
  "checkrent",   /* 225 */
  "chuckle",
  "balance",
  "deposit",
  "withdraw",
  "sys",
  "log",   /* 231 */
  "mstat",
  "pstat",
  "tornado",
  "light",
  "title",
  "report",
  "spells",
  "flag",
  "multi kick",
  "demote", /*241 */
  "nochat",
  "wimpy",
  "gtell",
  "send",
  "write",
  "post",
  "웃음",
  "말",
  "울음",
  "춤",   /* 251 */
  "ㅣ",
  "하하",
  "체팅",
  "haha",
  "jjup",
  "wow", /* 257 */
  "bye",
  "hee",
  "brb",
  "hmm",  /* 261 */
  "assist",
  "ungroup",
  "wizlist",
  "hangul",
  "NEWS",
  "version",
  "disarm", /* by chase 268 */
  "shouryuken", /* chase 269 */
  "throw", /* chase 270 */
  "punch", 
  "assault", /* process 272 */
  "JOIN", /* guild join 273*/
  "LEAVE", /* guild leave 274*/
  "train", /* guild skill practice 275*/
  "cant",/* guild chat 276 */
  "SAVE", /* 277 SAVE at locker room */
  "LOAD", /* 278 LOAD at locker room */
  "QUERY",	/* 279 QUERY,querys player's guild */
  "broadcast", /* 280 for police */
  "simultaneous", /* 281 for police */
  "arrest",/* 282 for police */
  "angry yell", /* 283 for outlaws */
  "solace", /* 284 for assasins */
  "unwield",
  "unhold",
  "temptation", /* 287 */
  "shadow figure", /* 288 for assasins */
  "smoke", /* 289 for outlaws */
  "inject", /* 290 for outlaws */
  "plan",
  "power bash",
  "evil strike",
  "call", /* 294 for board shuttle bus */
  "charge",
  "solo",
  "dumb",
  "spin bird kick",
  "view", /* used in new_shop.c */
  "reply",
  "", /* 301 */
  "quest",
  "request",
  "hint",
  "pull",
  "change",
  ":",
  "lastchat",
  "\n"
};

char *fill[]=
{ "in",
  "from",
  "with",
  "the",
  "on",
  "at",
  "to",
  "\n"
};

/*********************************************************************
	This is for try to control the echo on telnet
	running on diku system:
	First call the no_echo_local when boot
	then call (no_)echo_telnet in the function nanny in the
	interpreter.c

	(don't ask me to explain you all the system,
	it's just shit but it works!)

	copyright (C) Big Cat rochat_p@elde.epfl.ch
			      balelec@eldi.epfl.ch

	Was performed on MUME : lbdsun4.epfl.ch 4000
				128.178.77.5 4000

(This head in comments is part of the code, and can't be removed !!!)
**********************************************************************/

#ifdef DONT_HAVE_TERMIOS_H

void echo_local(int fd)
   {
   struct termio io;

   ioctl(fd, TCGETA, &io);
   io.c_line = 0;
   io.c_lflag |= ECHO;
   ioctl(fd, TCSETA, &io);
   }

void no_echo_local(int fd)	
   {
   struct termio io;

   ioctl(fd, TCGETA, &io); 
   io.c_cc[VMIN] = 1;
   io.c_cc[VTIME] = 0;
   io.c_line = 0;
   io.c_lflag &= ~ECHO;
   ioctl(fd, TCSETA, &io);
   }  

#else

void echo_local(int fd)
{
   struct termios io;
   
   tcgetattr(fd, &io);
   io.c_lflag |= ECHO;
   tcsetattr(fd, TCSANOW, &io);
}

void no_echo_local(int fd)
{  
   struct termios io;
   
   tcgetattr(fd, &io);
   io.c_cc[VMIN] = 1; 
   io.c_cc[VTIME] = 0;
   io.c_lflag &= ~ECHO;
   tcsetattr(fd, TCSANOW, &io);
}

#endif


void no_echo_telnet(struct descriptor_data *d)
{
	char buf[5];

	sprintf(buf,"%c%c%c",IAC,WILL,TELOPT_ECHO);
	write(d->descriptor,buf,3);
}

void echo_telnet(struct descriptor_data *d)
{
	char buf[5];

	sprintf(buf,"%c%c%c",IAC,WONT,TELOPT_ECHO);
	write(d->descriptor,buf,3);
}
/*--------------------------------------------------*/

int search_block(char *arg, char **list, bool exact)
{
  register int i,l;

  /* Make into lower case, and get length of string */
  for(l=0; *(arg+l); l++)
    /*cyb   *(arg+l)=LOWER(*(arg+l));*/
    /*cyb just iterate */  ;

  if (exact) {
    for(i=0; **(list+i) != '\n'; i++)
      if (!strcmp(arg, *(list+i)))
        return(i);
  } else {
    if (!l)
      l=1; /* Avoid "" to match the first available string */
    for(i=0; **(list+i) != '\n'; i++)
      if (!strncmp(arg, *(list+i), l))
        return(i);
  }

  return(-1);
}

int old_search_block(char *argument,int begin,int length,char **list,int mode)
{
	int guess, found, search;
        
	/* If the word contain 0 letters, then a match is already found */
	found = (length < 1);
	guess = 0;
	/* Search for a match */
	if(mode)
		while ( NOT found AND *(list[guess]) != '\n' ) {
			found=(length==strlen(list[guess]));
			for(search=0;( search < length AND found );search++)
				found=(*(argument+begin+search)== *(list[guess]+search));
			guess++;
		}
	else {
		while ( NOT found AND *(list[guess]) != '\n' ) {
			found=1;
			for(search=0;( search < length AND found );search++)
				found=(*(argument+begin+search)== *(list[guess]+search));
			guess++;
		}
	}
	return ( found ? guess : -1 ); 
}

int improve_status(struct char_data *ch, char arg)
{
	int is_possible = 1;

	switch (arg) {
		case 'c':
			if (ch->abilities.con < 18)
				ch->abilities.con++;
			else
				is_possible = 0;
			break;
		case 'i':
			if (ch->abilities.intel < 18)
				ch->abilities.intel++;
			else
				is_possible = 0;
			break;
		case 'd':
			if (ch->abilities.dex < 18)
				ch->abilities.dex++;
			else
				is_possible = 0;
			break;
		case 'w':
			if (ch->abilities.wis < 18)
				ch->abilities.wis++;
			else
				is_possible = 0;
			break;
		case 's':
			if (ch->abilities.str < 18)
				ch->abilities.str++;
			else if (ch->abilities.str == 18 && ch->abilities.str_add < 100) {
				ch->abilities.str_add += number(10, 25);
				ch->abilities.str_add = MIN(ch->abilities.str_add,100);
				/*2010.12.22 ch->abilities.str_add 들어가야 할 둣? by Moon*/
			}
			else
				is_possible = 0;
			break;
		default:
			is_possible = 0;
			break;
	}

	return is_possible;
}

int command_interpreter(struct char_data *ch, char *argument) 
{
	int look_at, cmd, begin, lev;
	extern int no_specials;
	char buf[MAX_STRING_LENGTH];
	int ch_class;

	REMOVE_BIT(ch->specials.affected_by, AFF_HIDE);

	/* Find first non blank */
	for (begin = 0 ; argument[begin] == ' '; begin++ );
	/* Find length of first word */
	for (look_at = 0; ISLETTER( *(argument + begin + look_at )) ; look_at++) ;

	cmd = old_search_block(argument,begin,look_at,command,0);
  
	if (!cmd)
		return(1);

	ch_class = GET_CLASS(ch) - 1;

	if (cmd > 0 && GET_LEVEL(ch) < cmd_info[cmd].minimum_level[ch_class]) {
		lev = GET_LEVEL(ch) ;
		if (lev < 13)
			send_to_char_han("Huh?\n\r", "뭐라고?\n\r", ch);
		else if (lev < 31)
			send_to_char_han("What?\n\r", "예 ? 뭐요 ?\n\r", ch);
		else if (lev < IMO)
			send_to_char_han("What did you say?\n\r",
				"뭐라고 하셨습니까 ?\n\r", ch);
		else 
			send_to_char_han("What did you say, Sir ?\n\r",
				"오 신이시여 무식한 제가 잘못입니다.. 좀더 쉬운 말씀으로..\n\r",
				ch);
		return(1);
	}

	if (cmd > 0 && (cmd_info[cmd].command_pointer != 0)) {
		if (GET_POS(ch) < cmd_info[cmd].minimum_position) {
			switch (GET_POS(ch)) {
				case POSITION_DEAD:
					send_to_char("Lie still; you are DEAD!!! :-( \n\r", ch);
					break;
				case POSITION_INCAP:
				case POSITION_MORTALLYW:
					send_to_char("You are in a pretty bad shape, ", ch);
					send_to_char("unable to do anything!\n\r", ch);
					break;
				case POSITION_STUNNED:
					send_to_char("All you can do right now, ", ch);
					send_to_char("is think about the stars!\n\r", ch);
					break;
				case POSITION_SLEEPING:
					send_to_char("In your dreams, or what?\n\r", ch);
					break;
				case POSITION_RESTING:
					send_to_char("Nah... You feel too relaxed to do that..\n\r",
						ch);
					break;
				case POSITION_SITTING:
					send_to_char("Maybe you should get ", ch);
					send_to_char("on your feet first?\n\r", ch);
					break;
				case POSITION_FIGHTING:
					send_to_char("No way! You are ", ch);
					send_to_char("fighting for your life!\n\r", ch);
					break;
			}
		}
		else {
			if (!no_specials &&
				special(ch, cmd, argument + begin + look_at))
				return(1);  
			((*cmd_info[cmd].command_pointer)
				(ch, argument + begin + look_at, cmd));
		}

		if (IS_SET(ch->specials.act, PLR_XYZZY)) {
			sprintf(buf,"%s: %s",ch->player.name,argument);
			log(buf);
		}
		return(1);
	}

	if (cmd > 0 && (cmd_info[cmd].command_pointer == 0)) {
		send_to_char("Sorry, but that command ", ch);
		send_to_char("has yet to be implemented...\n\r", ch);
	}
	else 
		send_to_char("Huh?\n\r", ch);
	return(1);
}

void argument_interpreter(char *argument,char *first_arg,char *second_arg )
{
        int look_at, /*found,*/ begin;

        /* found = */ begin = 0;

        do
        {
                /* Find first non blank */
                for ( ;*(argument + begin ) == ' ' ; begin++);

                /* Find length of first word */
                for ( look_at=0; ISLETTER(*(argument+begin+look_at));look_at++)

                        /* Make all letters lower case,
                           AND copy them to first_arg */
                        *(first_arg + look_at) =
                        /*cyb   LOWER(*(argument + begin + look_at));  */
                        *(argument + begin + look_at);

                *(first_arg + look_at)='\0';
                begin += look_at;

        }
        while( fill_word(first_arg));

        do
        {
                /* Find first non blank */
                for ( ;*(argument + begin ) == ' ' ; begin++);

                /* Find length of first word */
                for ( look_at=0; ISLETTER(*(argument+begin+look_at));look_at++)

                        /* Make all letters lower case,
                           AND copy them to second_arg */
                        *(second_arg + look_at) =
                        /*cyb   LOWER(*(argument + begin + look_at));*/
                        *(argument + begin + look_at);

                *(second_arg + look_at)='\0';
                begin += look_at;

        }
        while( fill_word(second_arg));
}

int is_number(char *str)
{
  int look_at;

  if(*str=='\0')
    return(0);

  for(look_at=0;*(str+look_at) != '\0';look_at++)
    if((*(str+look_at)<'0')||(*(str+look_at)>'9'))
      return(0);
  return(1);
}


/* find the first sub-argument of a string, return pointer to first char in
   primary argument, following the sub-arg                  */
char *one_argument(char *argument, char *first_arg )
{
  int /* found, */ begin, look_at;

        /* found = */ begin = 0;

        do
        {
                /* Find first non blank */
                for ( ;isspace(*(argument + begin)); begin++);

                /* Find length of first word */
                for (look_at=0; ISLETTER(*(argument+begin+look_at));look_at++)

                        /* Make all letters lower case,
                           AND copy them to first_arg */
                        *(first_arg + look_at) =
                        /*cyb LOWER(*(argument + begin + look_at));*/
                        *(argument + begin + look_at);

                *(first_arg + look_at)='\0';
    begin += look_at;
  }
        while (fill_word(first_arg));

  return(argument+begin);
}

int fill_word(char *argument)
{
  return ( search_block(argument,fill,TRUE) >= 0);
}

/* determine if a given string is an abbreviation of another */
int is_abbrev(char *arg1, char *arg2)
{
  if (!*arg1)
     return(0);

  for (; *arg1; arg1++, arg2++)
     if (LOWER(*arg1) != LOWER(*arg2))
        return(0);

  return(1);
}




/* return first 'word' plus trailing substring of input string */
#define isnhspace(ch) (!(((ch) & 0xE0) > 0x90) && isspace(ch))

void half_chop(char *string, char *arg1, char *arg2)
{
  for (; isnhspace(*string); string++);

  for (; !isnhspace(*arg1 = *string) && *string; string++, arg1++);

  *arg1 = '\0';  
  
  for (; isnhspace(*string); string++);

  for (; (*arg2 = *string); string++, arg2++);
}


int special(struct char_data *ch, int cmd, char *arg)
{
  register struct obj_data *i;
  register struct char_data *k;
  int j;

  /* special in room? */
  if (world[ch->in_room].funct)
     if ((*world[ch->in_room].funct)(ch, cmd, arg))
        return(1);

  /* special in equipment list? */
  for (j = 0; j <= (MAX_WEAR - 1); j++)
     if (ch->equipment[j] && ch->equipment[j]->item_number>=0)
        if (obj_index[ch->equipment[j]->item_number].func)
           if ((*obj_index[ch->equipment[j]->item_number].func)
              (ch, cmd, arg))
              return(1);

  /* special in inventory? */
  for (i = ch->carrying; i; i = i->next_content)
    if (i->item_number>=0)
      if (obj_index[i->item_number].func)
          if ((*obj_index[i->item_number].func)(ch, cmd, arg))
             return(1);

  /* special in mobile present? */
  for (k = world[ch->in_room].people; k; k = k->next_in_room)
     if ( IS_MOB(k) )
        if (mob_index[k->nr].func)
           if ((*mob_index[k->nr].func)(ch, cmd, arg))
              return(1);

  /* special in object present? */
  for (i = world[ch->in_room].contents; i; i = i->next_content)
     if (i->item_number>=0)
        if (obj_index[i->item_number].func)
           if ((*obj_index[i->item_number].func)(ch, cmd, arg))
              return(1);


  return(0);
}

void assign_command_pointers ( void )
{
  int position;

  for (position = 0 ; position < MAX_CMD_LIST; position++)
    cmd_info[position].command_pointer = 0;

#ifdef UNUSED_CODE
  COMMANDO(1, POSITION_STANDING, do_move, 0, 0, 0, 0);
  COMMANDO(2, POSITION_STANDING, do_move, 0, 0, 0, 0);
  COMMANDO(3, POSITION_STANDING, do_move, 0, 0, 0, 0);
  COMMANDO(4, POSITION_STANDING, do_move, 0, 0, 0, 0);
  COMMANDO(5, POSITION_STANDING, do_move, 0, 0, 0, 0);
  COMMANDO(6, POSITION_STANDING, do_move, 0, 0, 0, 0);
  COMMANDO(7, POSITION_STANDING, do_enter, 0, 0, 0, 0);
  COMMANDO(8, POSITION_RESTING, do_exits, 0, 0, 0, 0);
  COMMANDO(9, POSITION_RESTING, do_action, 0, 0, 0, 0);
  COMMANDO(10, POSITION_RESTING, do_get, 0, 0, 0, 0);
  COMMANDO(11, POSITION_RESTING, do_drink, 0, 0, 0, 0);
  COMMANDO(12, POSITION_RESTING, do_eat, 0, 0, 0, 0);
  COMMANDO(13, POSITION_RESTING, do_wear, 0, 0, 0, 0);
  COMMANDO(14, POSITION_RESTING, do_wield, 0, 0, 0, 0);
  COMMANDO(15, POSITION_RESTING, do_look, 0, 0, 0, 0);
  COMMANDO(16, POSITION_DEAD, do_score, 0, 0, 0, 0);
  COMMANDO(17, POSITION_RESTING, do_say, 0, 0, 0, 0);
  COMMANDO(18, POSITION_RESTING, do_shout, 1, 1, 1, 1);
  COMMANDO(19, POSITION_RESTING, do_tell, 0, 0, 0, 0);
  COMMANDO(20, POSITION_DEAD, do_inventory, 0, 0, 0, 0);
  COMMANDO(22, POSITION_STANDING, do_action, 0, 0, 0, 0);
  COMMANDO(23, POSITION_RESTING, do_action, 0, 0, 0, 0);
  COMMANDO(24, POSITION_STANDING, do_action, 0, 0, 0, 0);
  COMMANDO(25, POSITION_FIGHTING, do_kill, 0, 0, 0, 0);
  COMMANDO(26, POSITION_RESTING, do_action, 0, 0, 0, 0);
  COMMANDO(27, POSITION_RESTING, do_action, 0, 0, 0, 0);
  COMMANDO(28, POSITION_RESTING, do_action, 0, 0, 0, 0);
  COMMANDO(29, POSITION_RESTING, do_action, 0, 0, 0, 0);
  COMMANDO(30, POSITION_RESTING, do_action, 0, 0, 0, 0);
  COMMANDO(31, POSITION_RESTING, do_action, 0, 0, 0, 0);
  COMMANDO(32, POSITION_RESTING, do_action, 0, 0, 0, 0);
  COMMANDO(33, POSITION_RESTING, do_insult, 0, 0, 0, 0);
  COMMANDO(34, POSITION_RESTING, do_action, 0, 0, 0, 0);
  COMMANDO(35, POSITION_RESTING, do_action, 0, 0, 0, 0);
  COMMANDO(36, POSITION_RESTING, do_action, 0, 0, 0, 0);
  COMMANDO(37, POSITION_RESTING, do_action, 0, 0, 0, 0);
  COMMANDO(38, POSITION_DEAD, do_help, 0, 0, 0, 0);
  COMMANDO(39, POSITION_DEAD, do_who, 0, 0, 0, 0);
  COMMANDO(40, POSITION_SLEEPING, do_emote, 1, 1, 1, 1);
  COMMANDO(41, POSITION_SLEEPING, do_echo, IMO, IMO, IMO, IMO);  
  COMMANDO(42, POSITION_RESTING, do_stand, 0, 0, 0, 0);
  COMMANDO(43, POSITION_RESTING, do_sit, 0, 0, 0, 0);
  COMMANDO(44, POSITION_RESTING, do_rest, 0, 0, 0, 0);
  COMMANDO(45, POSITION_SLEEPING, do_sleep, 0, 0, 0, 0);
  COMMANDO(46, POSITION_SLEEPING, do_wake, 0, 0, 0, 0);
  COMMANDO(47, POSITION_SLEEPING, do_force, IMO + 2, IMO + 2, IMO + 2, IMO + 2);
  COMMANDO(48, POSITION_SLEEPING, do_trans, IMO + 1, IMO + 1, IMO + 1, IMO + 1);
  COMMANDO(49, POSITION_RESTING, do_action, 0, 0, 0, 0);
  COMMANDO(50, POSITION_RESTING, do_action, 0, 0, 0, 0);
  COMMANDO(51, POSITION_RESTING, do_action, 0, 0, 0, 0);
  COMMANDO(52, POSITION_RESTING, do_action, 0, 0, 0, 0);
  COMMANDO(53, POSITION_RESTING, do_action, 0, 0, 0, 0);
  COMMANDO(54, POSITION_SLEEPING, do_news, 0, 0, 0, 0);
  COMMANDO(55, POSITION_SLEEPING, do_equipment, 0, 0, 0, 0);
  COMMANDO(56, POSITION_STANDING, do_not_here, 0, 0, 0, 0);
  COMMANDO(57, POSITION_STANDING, do_not_here, 0, 0, 0, 0);
  COMMANDO(58, POSITION_STANDING, do_not_here, 0, 0, 0, 0);
  COMMANDO(59, POSITION_STANDING, do_not_here, 0, 0, 0, 0);
  COMMANDO(60, POSITION_RESTING, do_drop, 0, 0, 0, 0);
  COMMANDO(61, POSITION_SLEEPING, do_goto, IMO, IMO, IMO, IMO);
  COMMANDO(62, POSITION_RESTING, do_weather, 0, 0, 0, 0);
  COMMANDO(63, POSITION_RESTING, do_read, 0, 0, 0, 0);
  COMMANDO(64, POSITION_STANDING, do_pour, 0, 0, 0, 0);
  COMMANDO(65, POSITION_RESTING, do_grab, 0, 0, 0, 0);
  COMMANDO(66, POSITION_RESTING, do_remove, 0, 0, 0, 0);
  COMMANDO(67, POSITION_RESTING, do_put, 0, 0, 0, 0);
  COMMANDO(68, POSITION_DEAD, do_shutdow, IMO + 3, IMO + 3, IMO + 3, IMO + 3);
  COMMANDO(69, POSITION_SLEEPING, do_save, 0, 0, 0, 0);
  COMMANDO(70, POSITION_FIGHTING, do_hit, 0, 0, 0, 0);
  COMMANDO(71, POSITION_SLEEPING, do_string, IMO, IMO, IMO, IMO);
  COMMANDO(72, POSITION_RESTING, do_give, 0, 0, 0, 0);
  COMMANDO(73, POSITION_DEAD, do_quit, 0, 0, 0, 0);
  COMMANDO(74, POSITION_DEAD, do_stat, IMO, IMO, IMO, IMO);
  COMMANDO(75, POSITION_DEAD, do_action, 0, 0, 0, 0);
  COMMANDO(76, POSITION_DEAD, do_time, 0, 0, 0, 0);
  COMMANDO(77, POSITION_DEAD, do_load, IMO + 2, IMO + 2, IMO + 2, IMO + 2);
  COMMANDO(78, POSITION_DEAD, do_purge, IMO + 1, IMO + 1, IMO + 1, IMO + 1);
  COMMANDO(79, POSITION_DEAD, do_shutdown, IMO + 3, IMO + 3, IMO + 3, IMO + 3);
  COMMANDO(80, POSITION_RESTING, do_action, 0, 0, 0, 0);
  COMMANDO(82, POSITION_RESTING, do_replacerent,
	IMO + 3, IMO + 3, IMO + 3, IMO + 3);
  COMMANDO(83, POSITION_RESTING, do_whisper, 0, 0, 0, 0);
  COMMANDO(84, POSITION_SITTING, do_cast, 1, 1, 1, 1);
  COMMANDO(85, POSITION_DEAD, do_at, IMO, IMO, IMO, IMO);
  COMMANDO(86, POSITION_RESTING, do_ask, 1, 1, 1, 1);
  COMMANDO(87, POSITION_RESTING, do_order, 1, 1, 1, 1);
  COMMANDO(88, POSITION_RESTING, do_sip, 0, 0, 0, 0);
  COMMANDO(89, POSITION_RESTING, do_taste, 0, 0, 0, 0);
  COMMANDO(90, POSITION_DEAD, do_snoop, IMO, IMO, IMO, IMO);
  COMMANDO(91, POSITION_RESTING, do_follow, 0, 0, 0, 0);
  COMMANDO(92, POSITION_STANDING, do_rent, 1, 1, 1, 1);
  COMMANDO(93, POSITION_RESTING, do_junk, 1, 1, 1, 1);
  COMMANDO(94, POSITION_RESTING, do_action, 0, 0, 0, 0);
  COMMANDO(95, POSITION_DEAD, do_advance, 1, 1, 1, 1);
  COMMANDO(96, POSITION_SITTING, do_action, 0, 0, 0, 0);
  COMMANDO(97, POSITION_RESTING, do_action, 0, 0, 0, 0);
  COMMANDO(98, POSITION_STANDING, do_action, 0, 0, 0, 0);
  COMMANDO(99, POSITION_SITTING, do_open, 0, 0, 0, 0);
  COMMANDO(100, POSITION_SITTING, do_close, 0, 0, 0, 0);
  COMMANDO(101, POSITION_SITTING, do_lock, 0, 0, 0, 0);
  COMMANDO(102, POSITION_SITTING, do_unlock, 0, 0, 0, 0);
  COMMANDO(103, POSITION_STANDING, do_leave, 0, 0, 0, 0);
  COMMANDO(104, POSITION_RESTING, do_action, 0, 0, 0, 0);
  COMMANDO(105, POSITION_RESTING, do_action, 0, 0, 0, 0);
  COMMANDO(106, POSITION_RESTING, do_action, 0, 0, 0, 0);
  COMMANDO(107, POSITION_RESTING, do_action, 0, 0, 0, 0);
  COMMANDO(108, POSITION_RESTING, do_action, 0, 0, 0, 0);
  COMMANDO(109, POSITION_RESTING, do_action, 0, 0, 0, 0);
  COMMANDO(110, POSITION_STANDING, do_action, 0, 0, 0, 0);
  COMMANDO(111, POSITION_RESTING, do_action, 0, 0, 0, 0);
  COMMANDO(112, POSITION_STANDING, do_action, 0, 0, 0, 0);
  COMMANDO(113, POSITION_RESTING, do_action, 0, 0, 0, 0);
  COMMANDO(114, POSITION_RESTING, do_action, 0, 0, 0, 0);
  COMMANDO(115, POSITION_RESTING, do_action, 0, 0, 0, 0);
  COMMANDO(116, POSITION_RESTING, do_action, 0, 0, 0, 0);
  COMMANDO(117, POSITION_RESTING, do_action, 0, 0, 0, 0);
  COMMANDO(118, POSITION_RESTING, do_action, 0, 0, 0, 0);
  COMMANDO(119, POSITION_RESTING, do_action, 0, 0, 0, 0);
  COMMANDO(120, POSITION_RESTING, do_action, 0, 0, 0, 0);
  COMMANDO(121, POSITION_RESTING, do_action, 0, 0, 0, 0);
  COMMANDO(122, POSITION_RESTING, do_action, 0, 0, 0, 0);
  COMMANDO(123, POSITION_RESTING, do_action, 0, 0, 0, 0);
  COMMANDO(124, POSITION_RESTING, do_action, 0, 0, 0, 0);
  COMMANDO(125, POSITION_RESTING, do_action, 0, 0, 0, 0);
  COMMANDO(126, POSITION_STANDING, do_action, 0, 0, 0, 0);
  COMMANDO(127, POSITION_RESTING, do_action, 0, 0, 0, 0);
  COMMANDO(128, POSITION_RESTING, do_action, 0, 0, 0, 0);
  COMMANDO(129, POSITION_RESTING, do_action, 0, 0, 0, 0);
  COMMANDO(130, POSITION_RESTING, do_action, 0, 0, 0, 0);
  COMMANDO(131, POSITION_RESTING, do_action, 0, 0, 0, 0);
  COMMANDO(132, POSITION_RESTING, do_action, 0, 0, 0, 0);
  COMMANDO(133, POSITION_RESTING, do_action, 0, 0, 0, 0);
  COMMANDO(134, POSITION_RESTING, do_action, 0, 0, 0, 0);
  COMMANDO(135, POSITION_RESTING, do_action, 0, 0, 0, 0);
  COMMANDO(136, POSITION_SLEEPING, do_action, 0, 0, 0, 0);
  COMMANDO(137, POSITION_STANDING, do_action, 0, 0, 0, 0);
  COMMANDO(138, POSITION_RESTING, do_action, 0, 0, 0, 0);
  COMMANDO(139, POSITION_RESTING, do_action, 0, 0, 0, 0);
  COMMANDO(140, POSITION_STANDING, do_action, 0, 0, 0, 0);
  COMMANDO(141, POSITION_RESTING, do_action, 0, 0, 0, 0);
  COMMANDO(142, POSITION_RESTING, do_action, 0, 0, 0, 0);
  COMMANDO(143, POSITION_RESTING, do_action, 0, 0, 0, 0);
  COMMANDO(144, POSITION_RESTING, do_action, 0, 0, 0, 0);
  COMMANDO(145, POSITION_STANDING, do_action, 0, 0, 0, 0);
  COMMANDO(146, POSITION_RESTING, do_action, 0, 0, 0, 0);
  COMMANDO(147, POSITION_RESTING, do_action, 0, 0, 0, 0);
  COMMANDO(148, POSITION_STANDING, do_action, IMO, IMO, IMO, IMO);
  COMMANDO(149, POSITION_STANDING, do_extractrent,
	IMO + 2, IMO + 2, IMO + 2, IMO + 2);
  COMMANDO(150, POSITION_RESTING, do_grab, 1, 1, 1, 1);
  COMMANDO(151, POSITION_FIGHTING, do_flee, 1, 1, 1, 1);  
  COMMANDO(152, POSITION_STANDING, do_sneak, IMO, IMO, 1, IMO);  
  COMMANDO(153, POSITION_RESTING, do_hide, IMO, IMO, 1, IMO);  
  /* COMMANDO(154, POSITION_DEAD, do_backstab, IMO, IMO, 1, IMO);  */
  COMMANDO(154,	POSITION_STANDING, do_backstab, IMO, IMO, 1, IMO);/* jhpark */
  COMMANDO(155, POSITION_STANDING, do_pick, 20, 20, 1, 10);  
  COMMANDO(156, POSITION_STANDING, do_steal, IMO, IMO, 1, IMO);  
  COMMANDO(157, POSITION_FIGHTING, do_bash, IMO, IMO, IMO, 1);  
  COMMANDO(158, POSITION_FIGHTING, do_rescue, 30, 30, 10, 1);
  COMMANDO(159, POSITION_FIGHTING, do_kick, IMO, IMO, IMO, 1);
  COMMANDO(160, POSITION_RESTING, do_action, 0, 0, 0, 0);
  COMMANDO(161, POSITION_RESTING, do_action, 0, 0, 0, 0);
  COMMANDO(162, POSITION_RESTING, do_action, 0, 0, 0, 0);
  COMMANDO(163, POSITION_RESTING, do_action, 0, 0, 0, 0);
  COMMANDO(164, POSITION_RESTING, do_practice, 1, 1, 1, 1);
  COMMANDO(165, POSITION_RESTING, do_action, 0, 0, 0, 0);
  COMMANDO(166, POSITION_SITTING, do_examine, 0, 0, 0, 0);
  COMMANDO(167, POSITION_RESTING, do_get, 0, 0, 0, 0);
  COMMANDO(169, POSITION_RESTING, do_sayh, 0, 0, 0, 0);
  COMMANDO(170, POSITION_RESTING, do_practice, 1, 1, 1, 1);
  COMMANDO(171, POSITION_RESTING, do_action, 0, 0, 0, 0);
  COMMANDO(172, POSITION_SITTING, do_use, 1, 1, 1, 1);
  COMMANDO(173, POSITION_DEAD, do_where, 1, 1, 1, 1);
  COMMANDO(174, POSITION_DEAD, do_levels, 0, 0, 0, 0);
  COMMANDO(175, POSITION_DEAD, do_reroll, IMO + 3, IMO + 3, IMO + 3, IMO + 3);
  COMMANDO(176, POSITION_SITTING, do_action, 0, 0, 0, 0);
  COMMANDO(177, POSITION_SLEEPING, do_emote, 1, 1, 1, 1);
  COMMANDO(178, POSITION_RESTING, do_action, 0, 0, 0, 0);
  COMMANDO(179, POSITION_STANDING, do_action, 0, 0, 0, 0);
  COMMANDO(180, POSITION_RESTING, do_action, 0, 0, 0, 0);
  COMMANDO(181, POSITION_SLEEPING, do_action, 0, 0, 0, 0);
  COMMANDO(182, POSITION_RESTING, do_action, 0, 0, 0, 0);
  COMMANDO(183, POSITION_RESTING, do_action, 0, 0, 0, 0);
  COMMANDO(184, POSITION_RESTING, do_action, 0, 0, 0, 0);
  COMMANDO(185, POSITION_RESTING, do_action, 0, 0, 0, 0);
  COMMANDO(186, POSITION_RESTING, do_action, 0, 0, 0, 0);
  COMMANDO(187, POSITION_RESTING, do_action, 0, 0, 0, 0);
  COMMANDO(188, POSITION_RESTING, do_action, 0, 0, 0, 0);
  COMMANDO(189, POSITION_RESTING, do_action, 0, 0, 0, 0);
  COMMANDO(190, POSITION_RESTING, do_action, 0, 0, 0, 0);
  COMMANDO(191, POSITION_RESTING, do_action, 0, 0, 0, 0);
  COMMANDO(192, POSITION_RESTING, do_shoot, 1, 1, 1, 1);
  COMMANDO(193, POSITION_RESTING, do_action, 0, 0, 0, 0);
  COMMANDO(194, POSITION_RESTING, do_action, 0, 0, 0, 0);
  COMMANDO(195, POSITION_RESTING, do_action, 0, 0, 0, 0);
  COMMANDO(196, POSITION_RESTING, do_action, 0, 0, 0, 0);
  COMMANDO(197, POSITION_RESTING, do_action, 0, 0, 0, 0);
  COMMANDO(198, POSITION_RESTING, do_action, 0, 0, 0, 0);
  COMMANDO(199, POSITION_DEAD, do_brief, 0, 0, 0, 0);
  COMMANDO(200, POSITION_DEAD, do_wiznet, IMO, IMO, IMO, IMO);
  COMMANDO(201, POSITION_RESTING, do_consider, 0, 0, 0, 0);
  COMMANDO(202, POSITION_RESTING, do_group, 1, 1, 1, 1);
  COMMANDO(203, POSITION_DEAD, do_restore, IMO + 2, IMO + 2, IMO + 2, IMO + 2);
  COMMANDO(204, POSITION_DEAD, do_return, 0, 0, 0, 0);
  COMMANDO(205, POSITION_DEAD, do_switch, IMO + 2, IMO + 2, IMO + 2, IMO + 2);
  COMMANDO(206, POSITION_RESTING, do_quaff, 0, 0, 0, 0);
  COMMANDO(207, POSITION_RESTING, do_recite, 0, 0, 0, 0);
  COMMANDO(208, POSITION_DEAD, do_users, 2, 2, 2, 2);
  COMMANDO(209, POSITION_STANDING, do_flag, IMO + 1, IMO + 1, IMO + 1, IMO + 1);
  COMMANDO(210, POSITION_SLEEPING, do_noshout, 1, 1, 1, 1);
  COMMANDO(211, POSITION_SLEEPING, do_wizhelp, IMO, IMO, IMO, IMO);
  COMMANDO(212, POSITION_DEAD, do_credits, 0, 0, 0, 0);
  COMMANDO(213, POSITION_DEAD, do_compact, 0, 0, 0, 0);
  COMMANDO(214, POSITION_DEAD, do_flick, IMO + 3, IMO + 3, IMO + 3, IMO + 3);
  COMMANDO(215, POSITION_DEAD, do_wall, IMO + 3, IMO + 3, IMO + 3, IMO + 3);
  COMMANDO(216, POSITION_DEAD, do_set, IMO + 3, IMO + 3, IMO + 3, IMO + 3);
  COMMANDO(217, POSITION_DEAD, do_police, IMO + 3, IMO + 3, IMO + 3, IMO + 3);
  COMMANDO(218, POSITION_DEAD, do_wizlock, IMO + 3, IMO + 3, IMO + 3, IMO + 3);
  COMMANDO(219, POSITION_DEAD, do_noaffect, IMO + 1, IMO + 1, IMO + 1, IMO + 1);
  COMMANDO(220, POSITION_DEAD, do_invis, IMO, IMO, IMO, IMO);
  COMMANDO(221, POSITION_DEAD, do_notell, 0, 0, 0, 0);
  COMMANDO(222, POSITION_DEAD, do_banish, IMO + 3, IMO + 3, IMO + 3, IMO + 3);
  COMMANDO(223, POSITION_RESTING, do_reload, 1, 1, 1, 1);
  COMMANDO(224, POSITION_DEAD, do_data, IMO, IMO, IMO, IMO);
  COMMANDO(225, POSITION_DEAD, do_checkrent, IMO, IMO, IMO, IMO);
  COMMANDO(226, POSITION_DEAD, do_chat, 10, 10, 10, 10);
  COMMANDO(227, POSITION_DEAD, do_bank, 1, 1, 1, 1);
  COMMANDO(228, POSITION_DEAD, do_bank, 1, 1, 1, 1);
  COMMANDO(229, POSITION_DEAD, do_bank, 1, 1, 1, 1);
  COMMANDO(230, POSITION_DEAD, do_sys, IMO, IMO, IMO, IMO);
  COMMANDO(231, POSITION_DEAD, do_flag, IMO + 3, IMO + 3, IMO + 3, IMO + 3);
  COMMANDO(232, POSITION_DEAD, do_stat, IMO, IMO, IMO, IMO);
  COMMANDO(233, POSITION_DEAD, do_stat, IMO, IMO, IMO, IMO);
  COMMANDO(234, POSITION_FIGHTING, do_tornado, IMO, IMO, 20, 13);
  COMMANDO(235, POSITION_FIGHTING, do_light_move, IMO, IMO, 1, IMO);
  COMMANDO(236, POSITION_SLEEPING, do_title, 1, 1, 1, 1);
  COMMANDO(237, POSITION_SLEEPING, do_report, 1, 1, 1, 1);
  COMMANDO(238, POSITION_SLEEPING, do_spells, 1, 1, 1, 1);
  COMMANDO(239, POSITION_FIGHTING, do_flash, IMO, IMO, 5, IMO);
  COMMANDO(240, POSITION_FIGHTING, do_multi_kick, IMO, IMO, IMO, 17);
  COMMANDO(241, POSITION_FIGHTING, do_demote,
	IMO + 3, IMO + 3, IMO + 3, IMO + 3);
  COMMANDO(242, POSITION_DEAD, do_nochat, 1, 1, 1, 1);
  COMMANDO(243, POSITION_FIGHTING, do_wimpy, 1, 1, 1, 1);
  COMMANDO(244, POSITION_DEAD, do_gtell, 1, 1, 1, 1);
  COMMANDO(245, POSITION_RESTING, do_send, 19, 19, 19, 19);
  COMMANDO(246, POSITION_RESTING, do_write, 0, 0, 0, 0);
  COMMANDO(247, POSITION_RESTING, do_post, 0, 0, 0, 0);
  COMMANDO(248, POSITION_RESTING, do_action, 0, 0, 0, 0);
  COMMANDO(249, POSITION_RESTING, do_action, 0, 0, 0, 0);
  COMMANDO(250, POSITION_RESTING, do_action, 0, 0, 0, 0);
  COMMANDO(251, POSITION_STANDING, do_action, 0, 0, 0, 0);
  COMMANDO(252, POSITION_RESTING, do_look, 0, 0, 0, 0);
  COMMANDO(253, POSITION_RESTING, do_action, 0, 0, 0, 0);
  COMMANDO(254, POSITION_RESTING, do_action, 0, 0, 0, 0);
  COMMANDO(255, POSITION_RESTING, do_action, 0, 0, 0, 0);
  COMMANDO(256, POSITION_RESTING, do_action, 0, 0, 0, 0);
  COMMANDO(257, POSITION_RESTING, do_action, 0, 0, 0, 0);
  COMMANDO(258, POSITION_RESTING, do_action, 0, 0, 0, 0);
  COMMANDO(259, POSITION_RESTING, do_action, 0, 0, 0, 0);
  COMMANDO(260, POSITION_RESTING, do_action, 0, 0, 0, 0);
  COMMANDO(261, POSITION_RESTING, do_action, 0, 0, 0, 0);
  COMMANDO(262, POSITION_FIGHTING, do_assist, 0, 0, 0, 0);
  COMMANDO(263, POSITION_RESTING, do_ungroup, 1, 1, 1, 1);
  COMMANDO(264, POSITION_SLEEPING, do_wizards, 0, 0, 0, 0);
  COMMANDO(265, POSITION_DEAD, do_hangul, 0, 0, 0, 0);
  COMMANDO(266, POSITION_SLEEPING, do_news, 0, 0, 0, 0);
  COMMANDO(267, POSITION_SLEEPING, do_version, IMO, IMO, IMO, IMO);
  COMMANDO(268, POSITION_FIGHTING, do_disarm, IMO, IMO, 1, 10);
  COMMANDO(269, POSITION_FIGHTING, do_shouryuken, IMO, IMO, IMO, 30);
  COMMANDO(270, POSITION_FIGHTING, do_throw_object, IMO, IMO, 30, IMO);
  COMMANDO(271, POSITION_FIGHTING, do_punch, IMO, IMO, 30, 25); 
  COMMANDO(272, POSITION_FIGHTING, do_assault, 1, 1, 1, 1);
  COMMANDO(273, POSITION_STANDING, do_not_here, 10, 10, 10, 10);
  COMMANDO(274, POSITION_STANDING, do_not_here, 10, 10, 10, 10);
  COMMANDO(275, POSITION_STANDING, do_not_here, 10, 10, 10, 10);
  COMMANDO(276, POSITION_SLEEPING, do_cant, 10, 10, 10, 10);
  COMMANDO(277, POSITION_STANDING, do_not_here, 10, 10, 10, 10);
  COMMANDO(278, POSITION_STANDING, do_not_here, 10, 10, 10, 10); 
  COMMANDO(279, POSITION_STANDING, do_query, 10, 10, 10, 10); 
  COMMANDO(280, POSITION_STANDING, do_whistle, 15, 15, 15, 15);
  COMMANDO(281, POSITION_FIGHTING, do_simultaneous, 15, 15, 15, 15);
  COMMANDO(282, POSITION_FIGHTING, do_arrest, 15, 15, 15, 15);
  COMMANDO(283, POSITION_FIGHTING, do_angry_yell, 20, 20, 20, 20);
  COMMANDO(284, POSITION_FIGHTING, do_solace, 25, 25, 25, 25);
  COMMANDO(285, POSITION_FIGHTING, do_unwield, 0, 0, 0, 0);
  COMMANDO(286, POSITION_FIGHTING, do_unhold, 0, 0, 0, 0);
  COMMANDO(287, POSITION_FIGHTING, do_temptation, 10, 10, 10, 10); 
  COMMANDO(288, POSITION_STANDING, do_shadow, 25, 25, 25, 25);
  COMMANDO(289, POSITION_STANDING, do_smoke, 20, 20, 20, 20);
  COMMANDO(290, POSITION_STANDING, do_inject, 20, 20, 20, 20);
  COMMANDO(291, POSITION_SLEEPING, do_plan, 1, 1, 1, 1);
  COMMANDO(292, POSITION_FIGHTING, do_power_bash, 15, 15, 15, 15);
  COMMANDO(293, POSITION_FIGHTING, do_evil_strike, 25, 25, 25, 25);
  COMMANDO(294, POSITION_STANDING, do_not_here, 1, 1, 1, 1);
  COMMANDO(295, POSITION_FIGHTING, do_charge, 20, 20, 20, 20);
  COMMANDO(296, POSITION_STANDING, do_solo, 1, 1, 1, 1);
  COMMANDO(297, POSITION_STANDING, do_flag, IMO + 3, IMO + 3, IMO + 3, IMO + 3);
  COMMANDO(298, POSITION_FIGHTING, do_spin_bird_kick, IMO, IMO, IMO, 30); 
  COMMANDO(299, POSITION_STANDING, do_not_here, 1, 1, 1, 1);
  COMMANDO(300, POSITION_SLEEPING, do_reply, 1, 1, 1, 1);
  COMMANDO(302, POSITION_SLEEPING, do_quest, 10, 10, 10, 10);
  COMMANDO(303, POSITION_SLEEPING, do_request, 10, 10, 10, 10);
  COMMANDO(304, POSITION_SLEEPING, do_hint, 10, 10, 10, 10);
  COMMANDO(305, POSITION_SLEEPING, do_not_here, 1, 1, 1, 1);
  COMMANDO(306, POSITION_SLEEPING, do_not_here, 1, 1, 1, 1);
  COMMANDO(307, POSITION_DEAD, do_wiznet, IMO, IMO, IMO, IMO);
  COMMANDO(308, POSITION_SLEEPING, do_lastchat, 2, 2, 2, 2);

#endif // UNUSED_CODE

  COMMANDO(1, POSITION_STANDING, do_move, 0, 0, 0, 0);
  COMMANDO(2, POSITION_STANDING, do_move, 0, 0, 0, 0);
  COMMANDO(3, POSITION_STANDING, do_move, 0, 0, 0, 0);
  COMMANDO(4, POSITION_STANDING, do_move, 0, 0, 0, 0);
  COMMANDO(5, POSITION_STANDING, do_move, 0, 0, 0, 0);
  COMMANDO(6, POSITION_STANDING, do_move, 0, 0, 0, 0);
  COMMANDO(7, POSITION_STANDING, do_enter, 0, 0, 0, 0);
  COMMANDO(8, POSITION_RESTING, do_exits, 0, 0, 0, 0);
  COMMANDO(25, POSITION_RESTING, do_action, 0, 0, 0, 0);
  COMMANDO(10, POSITION_RESTING, do_get, 0, 0, 0, 0);
  COMMANDO(11, POSITION_RESTING, do_drink, 0, 0, 0, 0);
  COMMANDO(12, POSITION_RESTING, do_eat, 0, 0, 0, 0);
  COMMANDO(13, POSITION_RESTING, do_wear, 0, 0, 0, 0);
  COMMANDO(14, POSITION_RESTING, do_wield, 0, 0, 0, 0);
  COMMANDO(15, POSITION_RESTING, do_look, 0, 0, 0, 0);
  COMMANDO(16, POSITION_DEAD, do_score, 0, 0, 0, 0);
  COMMANDO(17, POSITION_RESTING, do_say, 0, 0, 0, 0);
  COMMANDO(18, POSITION_RESTING, do_shout, 1, 1, 1, 1);
  COMMANDO(19, POSITION_RESTING, do_tell, 0, 0, 0, 0);
  COMMANDO(20, POSITION_DEAD, do_inventory, 0, 0, 0, 0);
  COMMANDO(154, POSITION_STANDING, do_action, 0, 0, 0, 0);
  COMMANDO(23, POSITION_RESTING, do_action, 0, 0, 0, 0);
  COMMANDO(24, POSITION_STANDING, do_action, 0, 0, 0, 0);
  COMMANDO(9, POSITION_FIGHTING, do_kill, 0, 0, 0, 0);
  COMMANDO(107, POSITION_RESTING, do_action, 0, 0, 0, 0);
  COMMANDO(27, POSITION_RESTING, do_action, 0, 0, 0, 0);
  COMMANDO(28, POSITION_RESTING, do_action, 0, 0, 0, 0);
  COMMANDO(29, POSITION_RESTING, do_action, 0, 0, 0, 0);
  COMMANDO(30, POSITION_RESTING, do_action, 0, 0, 0, 0);
  COMMANDO(202, POSITION_RESTING, do_action, 0, 0, 0, 0);
  COMMANDO(32, POSITION_RESTING, do_action, 0, 0, 0, 0);
  COMMANDO(33, POSITION_RESTING, do_insult, 0, 0, 0, 0);
  COMMANDO(34, POSITION_RESTING, do_action, 0, 0, 0, 0);
  COMMANDO(35, POSITION_RESTING, do_action, 0, 0, 0, 0);
  COMMANDO(36, POSITION_RESTING, do_action, 0, 0, 0, 0);
  COMMANDO(37, POSITION_RESTING, do_action, 0, 0, 0, 0);
  COMMANDO(38, POSITION_DEAD, do_help, 0, 0, 0, 0);
  COMMANDO(39, POSITION_DEAD, do_who, 0, 0, 0, 0);
  COMMANDO(40, POSITION_SLEEPING, do_emote, 1, 1, 1, 1);
  COMMANDO(41, POSITION_SLEEPING, do_echo, IMO, IMO, IMO, IMO);  
  COMMANDO(42, POSITION_RESTING, do_stand, 0, 0, 0, 0);
  COMMANDO(43, POSITION_RESTING, do_sit, 0, 0, 0, 0);
  COMMANDO(44, POSITION_RESTING, do_rest, 0, 0, 0, 0);
  COMMANDO(45, POSITION_SLEEPING, do_sleep, 0, 0, 0, 0);
  COMMANDO(46, POSITION_SLEEPING, do_wake, 0, 0, 0, 0);
  COMMANDO(91, POSITION_SLEEPING, do_force, IMO + 2, IMO + 2, IMO + 2, IMO + 2);
  COMMANDO(48, POSITION_SLEEPING, do_trans, IMO + 1, IMO + 1, IMO + 1, IMO + 1);
  COMMANDO(49, POSITION_RESTING, do_action, 0, 0, 0, 0);
  COMMANDO(50, POSITION_RESTING, do_action, 0, 0, 0, 0);
  COMMANDO(51, POSITION_RESTING, do_action, 0, 0, 0, 0);
  COMMANDO(52, POSITION_RESTING, do_action, 0, 0, 0, 0);
  COMMANDO(53, POSITION_RESTING, do_action, 0, 0, 0, 0);
  COMMANDO(54, POSITION_SLEEPING, do_news, 0, 0, 0, 0);
  COMMANDO(55, POSITION_SLEEPING, do_equipment, 0, 0, 0, 0);
  COMMANDO(56, POSITION_STANDING, do_not_here, 0, 0, 0, 0);
  COMMANDO(57, POSITION_STANDING, do_not_here, 0, 0, 0, 0);
  COMMANDO(58, POSITION_STANDING, do_not_here, 0, 0, 0, 0);
  COMMANDO(59, POSITION_STANDING, do_not_here, 0, 0, 0, 0);
  COMMANDO(60, POSITION_RESTING, do_drop, 0, 0, 0, 0);
  COMMANDO(61, POSITION_SLEEPING, do_goto, IMO, IMO, IMO, IMO);
  COMMANDO(62, POSITION_RESTING, do_weather, 0, 0, 0, 0);
  COMMANDO(63, POSITION_RESTING, do_read, 0, 0, 0, 0);
  COMMANDO(64, POSITION_STANDING, do_pour, 0, 0, 0, 0);
  COMMANDO(65, POSITION_RESTING, do_grab, 0, 0, 0, 0);
  COMMANDO(66, POSITION_RESTING, do_remove, 0, 0, 0, 0);
  COMMANDO(67, POSITION_RESTING, do_put, 0, 0, 0, 0);
  COMMANDO(68, POSITION_DEAD, do_shutdow, IMO + 3, IMO + 3, IMO + 3, IMO + 3);
  COMMANDO(69, POSITION_SLEEPING, do_save, 0, 0, 0, 0);
  COMMANDO(70, POSITION_FIGHTING, do_hit, 0, 0, 0, 0);
  COMMANDO(71, POSITION_SLEEPING, do_string, IMO, IMO, IMO, IMO);
  COMMANDO(72, POSITION_RESTING, do_give, 0, 0, 0, 0);
  COMMANDO(73, POSITION_DEAD, do_quit, 0, 0, 0, 0);
  COMMANDO(74, POSITION_DEAD, do_stat, IMO, IMO, IMO, IMO);
  COMMANDO(75, POSITION_DEAD, do_action, 0, 0, 0, 0);
  COMMANDO(76, POSITION_DEAD, do_time, 0, 0, 0, 0);
  COMMANDO(77, POSITION_DEAD, do_load, IMO + 2, IMO + 2, IMO + 2, IMO + 2);
  COMMANDO(78, POSITION_DEAD, do_purge, IMO + 1, IMO + 1, IMO + 1, IMO + 1);
  COMMANDO(79, POSITION_DEAD, do_shutdown, IMO + 3, IMO + 3, IMO + 3, IMO + 3);
  COMMANDO(80, POSITION_RESTING, do_action, 0, 0, 0, 0);
  COMMANDO(82, POSITION_RESTING, do_replacerent,
	IMO + 3, IMO + 3, IMO + 3, IMO + 3);
  COMMANDO(83, POSITION_RESTING, do_whisper, 0, 0, 0, 0);
  COMMANDO(84, POSITION_SITTING, do_cast, 1, 1, 1, 1);
  COMMANDO(85, POSITION_DEAD, do_at, IMO, IMO, IMO, IMO);
  COMMANDO(86, POSITION_RESTING, do_ask, 1, 1, 1, 1);
  COMMANDO(87, POSITION_RESTING, do_order, 1, 1, 1, 1);
  COMMANDO(88, POSITION_RESTING, do_sip, 0, 0, 0, 0);
  COMMANDO(89, POSITION_RESTING, do_taste, 0, 0, 0, 0);
  COMMANDO(90, POSITION_DEAD, do_snoop, IMO, IMO, IMO, IMO);
  COMMANDO(47, POSITION_RESTING, do_follow, 0, 0, 0, 0);
  COMMANDO(92, POSITION_STANDING, do_rent, 1, 1, 1, 1);
  COMMANDO(93, POSITION_RESTING, do_junk, 1, 1, 1, 1);
  COMMANDO(94, POSITION_RESTING, do_action, 0, 0, 0, 0);
  COMMANDO(95, POSITION_DEAD, do_advance, 1, 1, 1, 1);
  COMMANDO(96, POSITION_SITTING, do_action, 0, 0, 0, 0);
  COMMANDO(97, POSITION_RESTING, do_action, 0, 0, 0, 0);
  COMMANDO(98, POSITION_STANDING, do_action, 0, 0, 0, 0);
  COMMANDO(99, POSITION_SITTING, do_open, 0, 0, 0, 0);
  COMMANDO(100, POSITION_SITTING, do_close, 0, 0, 0, 0);
  COMMANDO(101, POSITION_SITTING, do_lock, 0, 0, 0, 0);
  COMMANDO(102, POSITION_SITTING, do_unlock, 0, 0, 0, 0);
  COMMANDO(103, POSITION_STANDING, do_leave, 0, 0, 0, 0);
  COMMANDO(104, POSITION_RESTING, do_action, 0, 0, 0, 0);
  COMMANDO(105, POSITION_RESTING, do_action, 0, 0, 0, 0);
  COMMANDO(106, POSITION_RESTING, do_action, 0, 0, 0, 0);
  COMMANDO(226, POSITION_RESTING, do_action, 0, 0, 0, 0);
  COMMANDO(108, POSITION_RESTING, do_action, 0, 0, 0, 0);
  COMMANDO(109, POSITION_RESTING, do_action, 0, 0, 0, 0);
  COMMANDO(110, POSITION_STANDING, do_action, 0, 0, 0, 0);
  COMMANDO(111, POSITION_RESTING, do_action, 0, 0, 0, 0);
  COMMANDO(151, POSITION_STANDING, do_action, 0, 0, 0, 0);
  COMMANDO(113, POSITION_RESTING, do_action, 0, 0, 0, 0);
  COMMANDO(114, POSITION_RESTING, do_action, 0, 0, 0, 0);
  COMMANDO(115, POSITION_RESTING, do_action, 0, 0, 0, 0);
  COMMANDO(116, POSITION_RESTING, do_action, 0, 0, 0, 0);
  COMMANDO(117, POSITION_RESTING, do_action, 0, 0, 0, 0);
  COMMANDO(118, POSITION_RESTING, do_action, 0, 0, 0, 0);
  COMMANDO(119, POSITION_RESTING, do_action, 0, 0, 0, 0);
  COMMANDO(120, POSITION_RESTING, do_action, 0, 0, 0, 0);
  COMMANDO(121, POSITION_RESTING, do_action, 0, 0, 0, 0);
  COMMANDO(122, POSITION_RESTING, do_action, 0, 0, 0, 0);
  COMMANDO(123, POSITION_RESTING, do_action, 0, 0, 0, 0);
  COMMANDO(124, POSITION_RESTING, do_action, 0, 0, 0, 0);
  COMMANDO(125, POSITION_RESTING, do_action, 0, 0, 0, 0);
  COMMANDO(126, POSITION_STANDING, do_action, 0, 0, 0, 0);
  COMMANDO(127, POSITION_RESTING, do_action, 0, 0, 0, 0);
  COMMANDO(128, POSITION_RESTING, do_action, 0, 0, 0, 0);
  COMMANDO(129, POSITION_RESTING, do_action, 0, 0, 0, 0);
  COMMANDO(130, POSITION_RESTING, do_action, 0, 0, 0, 0);
  COMMANDO(131, POSITION_RESTING, do_action, 0, 0, 0, 0);
  COMMANDO(132, POSITION_RESTING, do_action, 0, 0, 0, 0);
  COMMANDO(133, POSITION_RESTING, do_action, 0, 0, 0, 0);
  COMMANDO(134, POSITION_RESTING, do_action, 0, 0, 0, 0);
  COMMANDO(135, POSITION_RESTING, do_action, 0, 0, 0, 0);
  COMMANDO(136, POSITION_SLEEPING, do_action, 0, 0, 0, 0);
  COMMANDO(137, POSITION_STANDING, do_action, 0, 0, 0, 0);
  COMMANDO(138, POSITION_RESTING, do_action, 0, 0, 0, 0);
  COMMANDO(139, POSITION_RESTING, do_action, 0, 0, 0, 0);
  COMMANDO(140, POSITION_STANDING, do_action, 0, 0, 0, 0);
  COMMANDO(141, POSITION_RESTING, do_action, 0, 0, 0, 0);
  COMMANDO(142, POSITION_RESTING, do_action, 0, 0, 0, 0);
  COMMANDO(143, POSITION_RESTING, do_action, 0, 0, 0, 0);
  COMMANDO(144, POSITION_RESTING, do_action, 0, 0, 0, 0);
  COMMANDO(145, POSITION_STANDING, do_action, 0, 0, 0, 0);
  COMMANDO(146, POSITION_RESTING, do_action, 0, 0, 0, 0);
  COMMANDO(147, POSITION_RESTING, do_action, 0, 0, 0, 0);
  COMMANDO(148, POSITION_STANDING, do_action, IMO, IMO, IMO, IMO);
  COMMANDO(149, POSITION_STANDING, do_extractrent,
	IMO + 2, IMO + 2, IMO + 2, IMO + 2);
  COMMANDO(150, POSITION_RESTING, do_grab, 1, 1, 1, 1);
  COMMANDO(112, POSITION_FIGHTING, do_flee, 1, 1, 1, 1);  
  COMMANDO(152, POSITION_STANDING, do_sneak, IMO, IMO, 1, IMO);  
  COMMANDO(153, POSITION_RESTING, do_hide, IMO, IMO, 1, IMO);  
  /* COMMANDO(154, POSITION_DEAD, do_backstab, IMO, IMO, 1, IMO);  */
  COMMANDO(22,	POSITION_STANDING, do_backstab, IMO, IMO, 1, IMO);/* jhpark */
  COMMANDO(155, POSITION_STANDING, do_pick, 20, 20, 1, 10);  
  COMMANDO(156, POSITION_STANDING, do_steal, IMO, IMO, 1, IMO);  
  COMMANDO(157, POSITION_FIGHTING, do_bash, IMO, IMO, IMO, 1);  
  COMMANDO(158, POSITION_FIGHTING, do_rescue, 30, 30, 10, 1);
  COMMANDO(159, POSITION_FIGHTING, do_kick, IMO, IMO, IMO, 1);
  COMMANDO(160, POSITION_RESTING, do_action, 0, 0, 0, 0);
  COMMANDO(161, POSITION_RESTING, do_action, 0, 0, 0, 0);
  COMMANDO(162, POSITION_RESTING, do_action, 0, 0, 0, 0);
  COMMANDO(163, POSITION_RESTING, do_action, 0, 0, 0, 0);
  COMMANDO(164, POSITION_RESTING, do_practice, 1, 1, 1, 1);
  COMMANDO(165, POSITION_RESTING, do_action, 0, 0, 0, 0);
  COMMANDO(166, POSITION_SITTING, do_examine, 0, 0, 0, 0);
  COMMANDO(167, POSITION_RESTING, do_get, 0, 0, 0, 0);
  COMMANDO(169, POSITION_RESTING, do_sayh, 0, 0, 0, 0);
  COMMANDO(170, POSITION_RESTING, do_practice, 1, 1, 1, 1);
  COMMANDO(171, POSITION_RESTING, do_action, 0, 0, 0, 0);
  COMMANDO(172, POSITION_SITTING, do_use, 1, 1, 1, 1);
  COMMANDO(173, POSITION_DEAD, do_where, 1, 1, 1, 1);
  COMMANDO(174, POSITION_DEAD, do_levels, 0, 0, 0, 0);
  COMMANDO(175, POSITION_DEAD, do_reroll, IMO + 3, IMO + 3, IMO + 3, IMO + 3);
  COMMANDO(176, POSITION_SITTING, do_action, 0, 0, 0, 0);
  COMMANDO(177, POSITION_SLEEPING, do_emote, 1, 1, 1, 1);
  COMMANDO(178, POSITION_RESTING, do_action, 0, 0, 0, 0);
  COMMANDO(179, POSITION_STANDING, do_action, 0, 0, 0, 0);
  COMMANDO(180, POSITION_RESTING, do_action, 0, 0, 0, 0);
  COMMANDO(181, POSITION_SLEEPING, do_action, 0, 0, 0, 0);
  COMMANDO(182, POSITION_RESTING, do_action, 0, 0, 0, 0);
  COMMANDO(183, POSITION_RESTING, do_action, 0, 0, 0, 0);
  COMMANDO(184, POSITION_RESTING, do_action, 0, 0, 0, 0);
  COMMANDO(185, POSITION_RESTING, do_action, 0, 0, 0, 0);
  COMMANDO(186, POSITION_RESTING, do_action, 0, 0, 0, 0);
  COMMANDO(187, POSITION_RESTING, do_action, 0, 0, 0, 0);
  COMMANDO(188, POSITION_RESTING, do_action, 0, 0, 0, 0);
  COMMANDO(189, POSITION_RESTING, do_action, 0, 0, 0, 0);
  COMMANDO(190, POSITION_RESTING, do_action, 0, 0, 0, 0);
  COMMANDO(191, POSITION_RESTING, do_action, 0, 0, 0, 0);
  COMMANDO(192, POSITION_RESTING, do_shoot, 1, 1, 1, 1);
  COMMANDO(193, POSITION_RESTING, do_action, 0, 0, 0, 0);
  COMMANDO(194, POSITION_RESTING, do_action, 0, 0, 0, 0);
  COMMANDO(195, POSITION_RESTING, do_action, 0, 0, 0, 0);
  COMMANDO(196, POSITION_RESTING, do_action, 0, 0, 0, 0);
  COMMANDO(197, POSITION_RESTING, do_action, 0, 0, 0, 0);
  COMMANDO(198, POSITION_RESTING, do_action, 0, 0, 0, 0);
  COMMANDO(199, POSITION_DEAD, do_brief, 0, 0, 0, 0);
  COMMANDO(200, POSITION_DEAD, do_wiznet, IMO, IMO, IMO, IMO);
  COMMANDO(201, POSITION_RESTING, do_consider, 0, 0, 0, 0);
  COMMANDO(31, POSITION_RESTING, do_group, 1, 1, 1, 1);
  COMMANDO(203, POSITION_DEAD, do_restore, IMO + 2, IMO + 2, IMO + 2, IMO + 2);
  COMMANDO(204, POSITION_DEAD, do_return, 0, 0, 0, 0);
  COMMANDO(205, POSITION_DEAD, do_switch, IMO + 2, IMO + 2, IMO + 2, IMO + 2);
  COMMANDO(206, POSITION_RESTING, do_quaff, 0, 0, 0, 0);
  COMMANDO(207, POSITION_RESTING, do_recite, 0, 0, 0, 0);
  COMMANDO(208, POSITION_DEAD, do_users, 2, 2, 2, 2);
  COMMANDO(239, POSITION_STANDING, do_flag, IMO + 1, IMO + 1, IMO + 1, IMO + 1);
  COMMANDO(210, POSITION_SLEEPING, do_noshout, 1, 1, 1, 1);
  COMMANDO(211, POSITION_SLEEPING, do_wizhelp, IMO, IMO, IMO, IMO);
  COMMANDO(212, POSITION_DEAD, do_credits, 0, 0, 0, 0);
  COMMANDO(213, POSITION_DEAD, do_compact, 0, 0, 0, 0);
  COMMANDO(214, POSITION_DEAD, do_flick, IMO + 3, IMO + 3, IMO + 3, IMO + 3);
  COMMANDO(215, POSITION_DEAD, do_wall, IMO + 3, IMO + 3, IMO + 3, IMO + 3);
  COMMANDO(216, POSITION_DEAD, do_set, IMO + 3, IMO + 3, IMO + 3, IMO + 3);
  COMMANDO(217, POSITION_DEAD, do_police, IMO + 3, IMO + 3, IMO + 3, IMO + 3);
  COMMANDO(218, POSITION_DEAD, do_wizlock, IMO + 3, IMO + 3, IMO + 3, IMO + 3);
  COMMANDO(219, POSITION_DEAD, do_noaffect, IMO + 1, IMO + 1, IMO + 1, IMO + 1);
  COMMANDO(220, POSITION_DEAD, do_invis, IMO, IMO, IMO, IMO);
  COMMANDO(221, POSITION_DEAD, do_notell, 0, 0, 0, 0);
  COMMANDO(222, POSITION_DEAD, do_banish, IMO + 3, IMO + 3, IMO + 3, IMO + 3);
  COMMANDO(223, POSITION_RESTING, do_reload, 1, 1, 1, 1);
  COMMANDO(224, POSITION_DEAD, do_data, IMO, IMO, IMO, IMO);
  COMMANDO(225, POSITION_DEAD, do_checkrent, IMO, IMO, IMO, IMO);
  COMMANDO(26, POSITION_DEAD, do_chat, 10, 10, 10, 10);
  COMMANDO(227, POSITION_DEAD, do_bank, 1, 1, 1, 1);
  COMMANDO(228, POSITION_DEAD, do_bank, 1, 1, 1, 1);
  COMMANDO(229, POSITION_DEAD, do_bank, 1, 1, 1, 1);
  COMMANDO(230, POSITION_DEAD, do_sys, IMO, IMO, IMO, IMO);
  COMMANDO(231, POSITION_DEAD, do_flag, IMO + 3, IMO + 3, IMO + 3, IMO + 3);
  COMMANDO(232, POSITION_DEAD, do_stat, IMO, IMO, IMO, IMO);
  COMMANDO(233, POSITION_DEAD, do_stat, IMO, IMO, IMO, IMO);
  COMMANDO(234, POSITION_FIGHTING, do_tornado, IMO, IMO, 20, 13);
  COMMANDO(235, POSITION_FIGHTING, do_light_move, IMO, IMO, 1, IMO);
  COMMANDO(236, POSITION_SLEEPING, do_title, 1, 1, 1, 1);
  COMMANDO(237, POSITION_SLEEPING, do_report, 1, 1, 1, 1);
  COMMANDO(238, POSITION_SLEEPING, do_spells, 1, 1, 1, 1);
  COMMANDO(209, POSITION_FIGHTING, do_flash, IMO, IMO, 5, IMO);
  COMMANDO(240, POSITION_FIGHTING, do_multi_kick, IMO, IMO, IMO, 17);
  COMMANDO(241, POSITION_FIGHTING, do_demote,
	IMO + 3, IMO + 3, IMO + 3, IMO + 3);
  COMMANDO(242, POSITION_DEAD, do_nochat, 1, 1, 1, 1);
  COMMANDO(243, POSITION_FIGHTING, do_wimpy, 1, 1, 1, 1);
  COMMANDO(244, POSITION_DEAD, do_gtell, 1, 1, 1, 1);
  COMMANDO(245, POSITION_RESTING, do_send, 19, 19, 19, 19);
  COMMANDO(246, POSITION_RESTING, do_write, 0, 0, 0, 0);
  COMMANDO(247, POSITION_RESTING, do_post, 0, 0, 0, 0);
  COMMANDO(248, POSITION_RESTING, do_action, 0, 0, 0, 0);
  COMMANDO(249, POSITION_RESTING, do_action, 0, 0, 0, 0);
  COMMANDO(250, POSITION_RESTING, do_action, 0, 0, 0, 0);
  COMMANDO(251, POSITION_STANDING, do_action, 0, 0, 0, 0);
  COMMANDO(252, POSITION_RESTING, do_look, 0, 0, 0, 0);
  COMMANDO(253, POSITION_RESTING, do_action, 0, 0, 0, 0);
  COMMANDO(254, POSITION_RESTING, do_action, 0, 0, 0, 0);
  COMMANDO(255, POSITION_RESTING, do_action, 0, 0, 0, 0);
  COMMANDO(256, POSITION_RESTING, do_action, 0, 0, 0, 0);
  COMMANDO(257, POSITION_RESTING, do_action, 0, 0, 0, 0);
  COMMANDO(258, POSITION_RESTING, do_action, 0, 0, 0, 0);
  COMMANDO(259, POSITION_RESTING, do_action, 0, 0, 0, 0);
  COMMANDO(260, POSITION_RESTING, do_action, 0, 0, 0, 0);
  COMMANDO(261, POSITION_RESTING, do_action, 0, 0, 0, 0);
  COMMANDO(262, POSITION_FIGHTING, do_assist, 0, 0, 0, 0);
  COMMANDO(263, POSITION_RESTING, do_ungroup, 1, 1, 1, 1);
  COMMANDO(264, POSITION_SLEEPING, do_wizards, 0, 0, 0, 0);
  COMMANDO(265, POSITION_DEAD, do_hangul, 0, 0, 0, 0);
  COMMANDO(266, POSITION_SLEEPING, do_news, 0, 0, 0, 0);
  COMMANDO(267, POSITION_SLEEPING, do_version, IMO, IMO, IMO, IMO);
  COMMANDO(268, POSITION_FIGHTING, do_disarm, IMO, IMO, 1, 10);
  COMMANDO(269, POSITION_FIGHTING, do_shouryuken, IMO, IMO, IMO, 30);
  COMMANDO(270, POSITION_FIGHTING, do_throw_object, IMO, IMO, 30, IMO);
  COMMANDO(271, POSITION_FIGHTING, do_punch, IMO, IMO, 30, 25); 
  COMMANDO(272, POSITION_FIGHTING, do_assault, 1, 1, 1, 1);
  COMMANDO(273, POSITION_STANDING, do_not_here, 10, 10, 10, 10);
  COMMANDO(274, POSITION_STANDING, do_not_here, 10, 10, 10, 10);
  COMMANDO(275, POSITION_STANDING, do_not_here, 10, 10, 10, 10);
  COMMANDO(276, POSITION_SLEEPING, do_cant, 10, 10, 10, 10);
  COMMANDO(277, POSITION_STANDING, do_not_here, 10, 10, 10, 10);
  COMMANDO(278, POSITION_STANDING, do_not_here, 10, 10, 10, 10); 
  COMMANDO(279, POSITION_STANDING, do_query, 10, 10, 10, 10); 
  COMMANDO(280, POSITION_STANDING, do_whistle, 15, 15, 15, 15);
  COMMANDO(281, POSITION_FIGHTING, do_simultaneous, 15, 15, 15, 15);
  COMMANDO(282, POSITION_FIGHTING, do_arrest, 15, 15, 15, 15);
  COMMANDO(283, POSITION_FIGHTING, do_angry_yell, 20, 20, 20, 20);
  COMMANDO(284, POSITION_FIGHTING, do_solace, 25, 25, 25, 25);
  COMMANDO(285, POSITION_FIGHTING, do_unwield, 0, 0, 0, 0);
  COMMANDO(286, POSITION_FIGHTING, do_unhold, 0, 0, 0, 0);
  COMMANDO(287, POSITION_FIGHTING, do_temptation, 10, 10, 10, 10); 
  COMMANDO(288, POSITION_STANDING, do_shadow, 25, 25, 25, 25);
  COMMANDO(289, POSITION_STANDING, do_smoke, 20, 20, 20, 20);
  COMMANDO(290, POSITION_STANDING, do_inject, 20, 20, 20, 20);
  COMMANDO(291, POSITION_SLEEPING, do_plan, 1, 1, 1, 1);
  COMMANDO(292, POSITION_FIGHTING, do_power_bash, 15, 15, 15, 15);
  COMMANDO(293, POSITION_FIGHTING, do_evil_strike, 25, 25, 25, 25);
  COMMANDO(294, POSITION_STANDING, do_not_here, 1, 1, 1, 1);
  COMMANDO(295, POSITION_FIGHTING, do_charge, 20, 20, 20, 20);
  COMMANDO(296, POSITION_STANDING, do_solo, 1, 1, 1, 1);
  COMMANDO(297, POSITION_STANDING, do_flag, IMO + 3, IMO + 3, IMO + 3, IMO + 3);
  COMMANDO(298, POSITION_FIGHTING, do_spin_bird_kick, IMO, IMO, IMO, 30); 
  COMMANDO(299, POSITION_STANDING, do_not_here, 1, 1, 1, 1);
  COMMANDO(300, POSITION_SLEEPING, do_reply, 1, 1, 1, 1);
  COMMANDO(302, POSITION_SLEEPING, do_quest, 10, 10, 10, 10);
  COMMANDO(303, POSITION_SLEEPING, do_request, 10, 10, 10, 10);
  COMMANDO(304, POSITION_SLEEPING, do_hint, 10, 10, 10, 10);
  COMMANDO(305, POSITION_SLEEPING, do_not_here, 1, 1, 1, 1);
  COMMANDO(306, POSITION_SLEEPING, do_not_here, 1, 1, 1, 1);
  COMMANDO(307, POSITION_DEAD, do_wiznet, IMO, IMO, IMO, IMO);
  COMMANDO(308, POSITION_SLEEPING, do_lastchat, 2, 2, 2, 2);
}

void query_status(struct descriptor_data *d)
{
	char buf[128];
	struct char_data *ch;

	ch = d->character;
	sprintf(buf, "You status is STR:%d/%d WIS:%d INT:%d DEX:%d CON:%d\n",
			ch->abilities.str, ch->abilities.str_add,
			ch->abilities.wis, ch->abilities.intel,
			ch->abilities.dex, ch->abilities.con);
	SEND_TO_Q(buf, d);
	SEND_TO_Q("\n\rDo you want to reroll your stat or improve?", d);
	SEND_TO_Q("\n\rTo reroll, type R", d);
	SEND_TO_Q("\n\rTo improve, type 2 initials of each stat.", d);
	SEND_TO_Q("\n\rEx. R (to reroll)", d);
	SEND_TO_Q("\n\rEx. S W (to improve)", d);
	SEND_TO_Q("Type ... >>>> ", d);
}

int find_name(char *name)
{
  int i;

  for (i = 0; i <= top_of_p_table; i++) {
     if (!str_cmp((player_table + i)->name, name))
        return(i);
  }
  return(-1);
}

int _parse_name(char *arg, char *name)
{
  int i;
  char check[18];

  /* skip whitespaces */
  for (; isspace(*arg); arg++);
  
  for (i = 0; (*name = *arg); arg++, i++, name++) {
     if ((*arg <0) || !isalpha(*arg) || i > 16)
        return(1); 
     else check[i]=toupper(*name);
  }
  check[i]='\0';
  /*for (i=0; i<6; i++)
  	if(!strcmp(check,illegal_name[i])) return(1);
	*/

  if (!i)
     return(1);

  if (strlen(check) < 2) return (1);

  return(0);
}

void nanny(struct descriptor_data *d, char *arg)
{
  char buf[100];
  int player_i;
  char tmp_name[20];
  struct char_file_u tmp_store;
  struct char_data *tmp_ch;
  struct descriptor_data *k;
  extern struct descriptor_data *descriptor_list;
  extern int nonewplayers;
  void do_look(struct char_data *ch, char *argument, int cmd);
  void load_char_objs(struct char_data *ch);
  int load_char(char *name, struct char_file_u *char_element);
  char improved_stat[2];
  int i;
  char new_connection[180];
  
  switch (STATE(d)) {
  case CON_NME:  /* wait for input of name */
    if (!d->character) {
      CREATE(d->character, struct char_data, 1);
      clear_char(d->character);
      d->character->desc = d;
    }
    for (; isspace(*arg); arg++) ;
    if (!*arg)
      close_socket(d);
    else {
      if(_parse_name(arg, tmp_name)) {
	SEND_TO_Q("Illegal name, please try another.", d);
	SEND_TO_Q("Name : ", d);
	return;
      }
      sprintf(new_connection, "%s is trying to play", tmp_name);
      log(new_connection);
      /* Check if already playing */
      for(k=descriptor_list; k; k = k->next) {
	if ((k->character != d->character) && k->character) {
	  if (k->original) {
	    if (GET_NAME(k->original) &&
		(str_cmp(GET_NAME(k->original),
			 tmp_name) == 0)) {
	      SEND_TO_Q("Already playing!\n\r", d);
	      SEND_TO_Q("Name : ", d);
	      return;
	    }
	  }
	  else { /* No switch has been made */
	    if (GET_NAME(k->character) &&
		(str_cmp(GET_NAME(k->character),
			 tmp_name) == 0)) {
	      SEND_TO_Q("Already playing!\n\r", d);
	      SEND_TO_Q("Name : ", d);
	      return;
	    }
	  }
	}
      } /* end of for */
      
      if ((player_i = load_char(tmp_name, &tmp_store)) > -1) {
	store_to_char(&tmp_store, d->character);
	strcpy(d->pwd, tmp_store.pwd);
	d->pos = player_table[player_i].nr;
	no_echo = 1;
	SEND_TO_Q("Password : ", d);
	STATE(d) = CON_PWDNRM;
      }
      else {
	/* player unknown gotta make a new */
	CREATE(GET_NAME(d->character), char, strlen(tmp_name) + 1);
	CAP(tmp_name);
	strcpy(GET_NAME(d->character), tmp_name);
	sprintf(buf, "Did I get that right, %s (Y/N)? ", tmp_name);
	SEND_TO_Q(buf, d);
	STATE(d) = CON_NMECNF;
      }
    }
    break;
    
  case CON_NMECNF: /* wait for conf. of new name */
    /* skip whitespaces */
    for (; isspace(*arg); arg++) ;
    if (*arg == 'y' || *arg == 'Y') {
      if (nonewplayers) {
	free(GET_NAME(d->character));
	GET_NAME(d->character)=NULL;
	close_socket(d);
      }
      SEND_TO_Q("New character.\n\r", d);
      no_echo = 1;
      sprintf(buf, "Give me a password for %s : ",
	      GET_NAME(d->character));
      SEND_TO_Q(buf, d);
      STATE(d) = CON_PWDGET;
    }
    else {
      if (*arg == 'n' || *arg == 'N') {
	SEND_TO_Q("Ok, what IS it, then? ", d);
	free(GET_NAME(d->character));
	GET_NAME(d->character)=NULL;
	STATE(d) = CON_NME;
      }
      else { /* Please do Y or N */
	SEND_TO_Q("Please type Yes or No? ", d);
      }
    }
    break;
    
  case CON_PWDNRM: /* get pwd for known player */
    /* skip whitespaces */
    for (; isspace(*arg); arg++) ;
    if (!*arg)
      close_socket(d);
    else {
      if (strncmp((char *)crypt(arg, d->pwd), d->pwd, 10)) {
	SEND_TO_Q("Wrong password.\n\r", d);
	no_echo = 1;
	SEND_TO_Q("Password : ", d);
	d->wait=20;
	return;
      }
      for (tmp_ch = character_list; tmp_ch; tmp_ch = tmp_ch->next) {
	if (!str_cmp(GET_NAME(d->character), GET_NAME(tmp_ch)) &&
	    !tmp_ch->desc && !IS_NPC(tmp_ch)) {
	  SEND_TO_Q("Reconnecting.\n\r", d);
	  free_char(d->character);
	  tmp_ch->desc = d;
	  d->character = tmp_ch;
	  tmp_ch->specials.timer = 0;
	  STATE(d) = CON_PLYNG;
	  act("$n has reconnected.", TRUE, tmp_ch, 0, 0, TO_ROOM);
	  sprintf(buf, "%s(%d)[%s] has reconnected.",
		  GET_NAME(d->character),
		  GET_LEVEL(d->character), d->host);
	  log(buf);
	  return;
	}
      }
      sprintf(buf, "%s(%d)[%s] has connected.",
	      GET_NAME(d->character), GET_LEVEL(d->character), d->host);
      log(buf);
      SEND_TO_Q(motd, d);
      SEND_TO_Q("\n\r\n*** PRESS RETURN : ", d);
      STATE(d) = CON_RMOTD;
    }
    break;
    
  case CON_PWDGET: /* get pwd for new player */
    /* skip whitespaces */
    for (; isspace(*arg); arg++) ;
    
    if (!*arg || strlen(arg) > 10) {
      SEND_TO_Q("Illegal password.\n\r", d);
      no_echo = 1;
      SEND_TO_Q("Password : ", d);
      return;
    }
    
    strncpy(d->pwd, (char *) crypt(arg, d->character->player.name), 10);
    *(d->pwd + 10) = '\0';
    no_echo = 1;
    SEND_TO_Q("Please retype password : ", d);
    STATE(d) = CON_PWDCNF;
    break;
    
  case CON_PWDCNF: /* get confirmation of new pwd */
    /* skip whitespaces */
    for (; isspace(*arg); arg++) ;
    if (strncmp((char *) crypt(arg, d->pwd), d->pwd, 10)) {
      SEND_TO_Q("\n\rPassword doesn't match!\n\r", d);
      no_echo = 1;
      SEND_TO_Q("Retype password : ", d);
      STATE(d) = CON_PWDGET;
      return;
    }
    SEND_TO_Q("What is your sex (M/F) ? ", d);
    STATE(d) = CON_QSEX;
    break;
    
  case CON_QSEX:  /* query sex of new user */
    echo_telnet(d);
    for (; isspace(*arg); arg++) ;
    switch (*arg) {
    case 'm':
    case 'M':
      /* sex MALE */
      d->character->player.sex = SEX_MALE;
      break;
    case 'f':
    case 'F':
      /* sex FEMALE */
      d->character->player.sex = SEX_FEMALE;
      break;
    default:
      SEND_TO_Q("That's not a sex...\n\r", d);
      SEND_TO_Q("What IS your sex? ", d);
      return;
    }
    SEND_TO_Q("\n\rSelect a class :\n\r", d);
    SEND_TO_Q("\tCleric\n\r\tThief\n\r\tWarrior\n\r\tMagic user", d);
    SEND_TO_Q("\n\rClass : ", d);
    STATE(d) = CON_QCLASS;
    break;
    
  case CON_QCLASS :
    /* skip whitespaces */
    for (; isspace(*arg); arg++) ;
    switch (*arg) {
    case 'm':
    case 'M':
      GET_CLASS(d->character) = CLASS_MAGIC_USER;
      init_char(d->character);
      STATE(d) = CON_QSTATE;
      break;
    case 'c':
    case 'C':
      GET_CLASS(d->character) = CLASS_CLERIC;
      init_char(d->character);
      STATE(d) = CON_QSTATE;
      break;
    case 'w':
    case 'W':
      GET_CLASS(d->character) = CLASS_WARRIOR;
      init_char(d->character);
      STATE(d) = CON_QSTATE;
      break;
    case 't':
    case 'T':
      GET_CLASS(d->character) = CLASS_THIEF;
      init_char(d->character);
      STATE(d) = CON_QSTATE;
      break;
    default :
      SEND_TO_Q("\n\rThat's not a class.\n\rClass : ", d);
      STATE(d) = CON_QCLASS;
      break;
    } /* End Switch */
    if (STATE(d) != CON_QCLASS) {
      sprintf(buf, "%s [%s] new player.", GET_NAME(d->character),
	      d->host);
      log(buf);
      SEND_TO_Q("\n\r*** PRESS RETURN : ", d);
    }
    break;
    
  case CON_RMOTD:  /* read CR after printing motd */
    if(GET_LEVEL(d->character) >= IMO) {
      SEND_TO_Q(imotd, d);
      SEND_TO_Q("\n\r*** PRESS RETURN : ", d);
      STATE(d) = CON_IMOTD;
      break;
    }
    SEND_TO_Q(MENU, d);
    STATE(d) = CON_SLCT;
    break;
    
  case CON_IMOTD: /* read CR after printing imotd */
    SEND_TO_Q(MENU, d);
    STATE(d) = CON_SLCT;
    break;
    
  case CON_SLCT:  /* get selection from main menu */
    /* skip whitespaces */
    for (; isspace(*arg); arg++) ;
    switch (*arg) {
    case '0':
      close_socket(d);
      break;
    case '1':
      reset_char(d->character);
      send_to_char(WELC_MESSG, d->character);
      d->character->next = character_list;
      character_list = d->character;
      if (d->character->in_room == NOWHERE) {
	if(d->character->player.level < IMO)
	  if(IS_SET(d->character->specials.act,PLR_BANISHED))
	    char_to_room(d->character, real_room(6999));
	  else 
	    char_to_room(d->character, real_room(3001)); 
	else
	  char_to_room(d->character, real_room(2));
      }
      else {
	if (real_room(d->character->in_room) > -1)
	  char_to_room(d->character,
		       real_room(d->character->in_room));
	else
	  char_to_room(d->character, real_room(3001));
      }
      unstash_char(d->character,0);
      act("$n has entered the game.",
	  TRUE, d->character, 0, 0, TO_ROOM);
      STATE(d) = CON_PLYNG;
      if (!GET_LEVEL(d->character))
	do_start(d->character);
      do_look(d->character, "",15);
      d->prompt_mode = 1;
      break;
    case '2':
      SEND_TO_Q("Enter a description of your character.\n\r", d);
      SEND_TO_Q("Terminate with a '@'.\n\r", d);
      if (d->character->player.description) {
	SEND_TO_Q("Old description :\n\r", d);
	SEND_TO_Q(d->character->player.description, d);
	free(d->character->player.description);
	d->character->player.description = 0;
      }
      d->str = &d->character->player.description;
      d->max_str = 240;
      STATE(d) = CON_EXDSCR;
      break;
    case '3':
      no_echo = 1;
      SEND_TO_Q("Please type password : ", d);
      STATE(d) = CON_PWDNEW1;
      break;
      /* Delete character is removed for player bug by dsshin */
      /*
	case '4':
	no_echo = 1;
	SEND_TO_Q("Please type password : ", d);
	STATE(d) = CON_DELCNF;
	break;
	*/
    default:
      SEND_TO_Q("Wrong option.\n\r", d);
      SEND_TO_Q(MENU, d);
      break;
    }
    break;
  case CON_PWDNEW1:
    /* check passwd */
    while(*arg && isspace(*arg))
      arg ++;
    
    if (strncmp((char *) crypt(arg, d->pwd), d->pwd, 10)) {
      SEND_TO_Q("Password doesn't match!\n\r", d);
      SEND_TO_Q(MENU, d);
      STATE(d) = CON_SLCT;
    }
    else {
      SEND_TO_Q("Enter a new password : ", d);
      STATE(d) = CON_PWDNEW;
    }
    break;
  case CON_PWDNEW:
    /* skip whitespaces */
    for (; isspace(*arg); arg++) ;
    if (!*arg || strlen(arg) > 10) {
      SEND_TO_Q("Illegal password.\n\r", d);
      no_echo = 1;
      SEND_TO_Q("Password : ", d);
      return;
    }
    strncpy(d->pwd,(char *) crypt(arg, d->character->player.name), 10);
    *(d->pwd + 10) = '\0';
    no_echo = 1;
    SEND_TO_Q("Please retype password : ", d);
    STATE(d) = CON_PWDNCNF;
    break;
    
  case CON_PWDNCNF:
    /* skip whitespaces */
    for (; isspace(*arg); arg++) ;
    if (strncmp((char *) crypt(arg, d->pwd), d->pwd, 10)) {
      SEND_TO_Q("Password doesn't match!\n\r", d);
      no_echo = 1;
      SEND_TO_Q("Retype password : ", d);
      STATE(d) = CON_PWDNEW;
      return;
    }
    SEND_TO_Q("\n\rDone. You must enter the game ", d);
    SEND_TO_Q("to make the change final.\n\r", d);
    SEND_TO_Q(MENU, d);
    STATE(d) = CON_SLCT;
    break;
    
  case CON_QSTATE:
    query_status(d);
    STATE(d) = CON_SET;
    break;
    
  case CON_SET:
    while (*arg && isspace(*arg))
      arg++;
    improved_stat[0] = tolower(*arg);
    while (*arg && !isspace(*arg))
      arg++;
    while (*arg && isspace(*arg))
      arg++;
    improved_stat[1] = tolower(*arg);
    
    for (i = 0; i < 2; i++) {
      switch (improved_stat[i]) {
      case 's':
      case 'w':
      case 'i':
      case 'd':
      case 'c':
	STATE(d) = CON_RMOTD;
	break;
      case 'r':
	init_char(d->character);
	SEND_TO_Q("\n\rYour stat is rerolled.\n\r", d);
	SEND_TO_Q("\n\r*** PRESS RETURN : ", d);
	STATE(d) = CON_QSTATE;
	return;
      default:
	SEND_TO_Q("\n\rCan't you understand? Retry!\n\r", d);
	SEND_TO_Q("\n\r*** PRESS RETURN : ", d);
	STATE(d) = CON_QSTATE;
	return;
      }
    }
    for (i = 0; i < 2; i++) {
      switch (improved_stat[i]) {
      case 's':
      case 'w':
      case 'i':
      case 'd':
      case 'c':
	if (!improve_status(d->character, improved_stat[i]))
	  SEND_TO_Q("\n\rAlas! you lost chance...", d);
	break;
      default:
	break;
      }
    }
    
    /* create an entry in the file */
    d->pos = create_entry(GET_NAME(d->character));
    save_char(d->character, NOWHERE);
    SEND_TO_Q(motd, d);
    SEND_TO_Q("\n\r\n*** PRESS RETURN : ", d);
    break;
  case CON_DELCNF:
    /* check passwd */
    while(*arg && isspace(*arg))
      arg ++;
    
    if (strncmp((char *) crypt(arg, d->pwd), d->pwd, 10)) {
      SEND_TO_Q("Password doesn't match!\n\r", d);
      SEND_TO_Q(MENU, d);
      STATE(d) = CON_SLCT;
    }
    else {
      SEND_TO_Q("\n\rReally want to DELETE your character? ", d);
      STATE(d) = CON_DELCNF2;
    }
    break;
  case CON_DELCNF2:
    /* skip whitespaces */
    for (; isspace(*arg); arg++) ;
    
    if(*arg == 'y' || *arg == 'Y'){
      if(remove_entry(d->character)){
	delete_char(d->character);
	close_socket(d);
      }
    }
    else {
      SEND_TO_Q(MENU, d);
      STATE(d) = CON_SLCT;
    }
    break;
  default:
    log("Nanny: illegal state of con'ness");
    abort();
    break;
  }
}
