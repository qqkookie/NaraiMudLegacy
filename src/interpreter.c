/* ************************************************************************
*  file: Interpreter.c , Command interpreter module.      Part of DIKUMUD *
*  Usage: Procedures interpreting user command                            *
*  Copyright (C) 1990, 1991 - see 'license.doc' for complete information. *
************************************************************************* */

#include <string.h>
#include <ctype.h>
#include <stdio.h>
#include <sys/types.h>
#include <termio.h>
/* cyb #include "termio.h" */
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
#include "limits.h"
#include "quest.h"
#include "command.h"
#include "memory.h"

/* #define ISLETTER(c)	(c > ' ' || c < 0) */
#define ISLETTER(c)	(c > ' ' || is_hangul(c))

#define COMMANDO(number,min_pos,pointer,min_level) {      \
  cmd_info[(number)].command_pointer = (pointer);         \
  cmd_info[(number)].minimum_position = (min_pos);        \
  cmd_info[(number)].minimum_level = (min_level); }

#define NOT !
#define AND &&
#define OR ||

#define STATE(d) ((d)->connected)

int no_echo = 0;
extern struct title_type titles[4][IMO+4];
extern char motd[MAX_STRING_LENGTH];
extern struct char_data *character_list;
extern struct player_index_element *player_table;
extern int top_of_p_table;
extern struct index_data *mob_index;
extern struct index_data *obj_index;
extern struct room_data *world;
struct command_info cmd_info[MAX_CMD_LIST];


/* external fcntls */

int stash_char_new(struct char_data *ch);
void unstash_char(struct char_data *ch, char *name);
int stash_char(struct char_data *ch, char *name);
void set_title(struct char_data *ch);
void init_char(struct char_data *ch);
void store_to_char(struct char_file_u *st, struct char_data *ch);
int create_entry(char *name);
int special(struct char_data *ch, int cmd, char *arg);
void log(char *str);

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
void do_qui(struct char_data *ch, char *argument, int cmd);
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
void do_typo(struct char_data *ch, char *arg, int cmd);
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
void do_jun(struct char_data *ch, char *arg, int cmd);
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
void do_examine(struct char_data *ch, char *arg, int cmd);
void do_info(struct char_data *ch, char *arg, int cmd);
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
void do_lastchat(struct char_data *ch, char *arg, int cmd);
void do_date(struct char_data *ch, char *arg, int cmd);
void do_mail(struct char_data *ch, char *argument, int cmd);
void do_throw(struct char_data *ch, char *argument, int cmd); /* newcom.c */
void do_commands(struct char_data *ch, char *argument, int cmd); /* newcom.c */
void do_nothing(struct char_data *ch, char *argument, int cmd); /* newcom.c */
void do_quest(struct char_data *ch, char *argument, int cmd); /* newcom.c */
void do_which(struct char_data *ch, char *argument, int cmd); /* act.wizard.c */
void do_autoexit(struct char_data *ch, char *arg, int cmd);
void do_reconfig(struct char_data *ch, char *arg, int cmd);	/* act.wizard.c */
void do_offer(struct char_data *ch, char *arg, int cmd);	/* newcom.c */
void do_scatter(struct char_data *ch, char *arg, int cmd);	/* act.wizard.c */
void do_sacrifice(struct char_data *ch, char *arg, int cmd);	/* newcom.c */
void do_display(struct char_data *ch, char *arg, int cmd);	/* newcom.c */
void do_retrieve(struct char_data *ch, char *arg, int cmd);	/* newcom.c */

void do_berserk(struct char_data *ch, char *arg, int cmd);	/* newcom.c */
void do_cookie(struct char_data *ch, char *arg, int cmd);	/* newcom.c */

/* Do not change the order !!  It is defined in command.h */
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
  "typo",        /* 81 */
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
  "jun",
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
  "info",
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
  "누구",
  "어디",
  "바이",
  "잠",
  "히히",
  "끄덕",
  "감사",  /* 261 */
  "assist",
  "ungroup",
  "wizards",
  "hangul",
  "NEWS",
  "version",
  "lastchat",
  "date",
  "mail",	/* 270 */
  "throw",	/* 271 */
  "commands",
  "press",
  "pull", /* 274 */
  "quest",
  "which",
  "autoexit",
  "reconfig",	/* 278 */
  "offer",	/* 279 */
  "scatter",	/* 280 */
  "sacrifice",
  "display",
  "retrieve",
  "junk",
  "도리도리",
  "절",
  "쩝쩝",
  "identify",
  "berserk",
  "cookie",
  "\n"	/* end of list */
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

void echo_local(int fd)
   {
  /* cyb struct termios io; */
   struct termio io;

  /* cyb  ioctl(fd, TCGETS, &io); */
   ioctl(fd, TCGETA, &io);
   io.c_line = 0;
   io.c_lflag |= ECHO;
  /* ioctl(fd, TCSETS, &io); */
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
  } else {
    while ( NOT found AND *(list[guess]) != '\n' ) {
      found=1;
      for(search=0;( search < length AND found );search++)
        found=(*(argument+begin+search)== *(list[guess]+search));
      guess++;
    }
  }
  return ( found ? guess : -1 ); 
}


int command_interpreter(struct char_data *ch, char *argument) 
{
	int search_hash_block(char *name, int leng, int mode) ;
    int look_at, cmd, begin, lev;
    extern int no_specials;
    char buf[MAX_STRING_LENGTH];

    REMOVE_BIT(ch->specials.affected_by, AFF_HIDE);
    /* Find first non blank */
    for (begin = 0 ; (*(argument + begin ) == ' ' ) ; begin++ );
    /* Find length of first word */
    for (look_at = 0; ISLETTER( *(argument + begin + look_at )) ; look_at++)
        /*cyb just iterate */;
  
	/*
      cmd = old_search_block(argument,begin,look_at,command,0);
	*/
	cmd = search_hash_block(argument+begin, look_at, 0) ;
  
    if (!cmd)
        return(1);

    if ( cmd>0 && GET_LEVEL(ch)<cmd_info[cmd].minimum_level ) {
        Huh(ch) ;
        return(1);
    }
    if ( cmd>0 && (cmd_info[cmd].command_pointer != 0)) {
        if( GET_POS(ch) < cmd_info[cmd].minimum_position )
            switch(GET_POS(ch)) {
            case POSITION_DEAD:
                send_to_char("Lie still; you are DEAD!!! :-( \n\r", ch);
                break;
            case POSITION_INCAP:
            case POSITION_MORTALLYW:
                send_to_char(
                    "You are in a pretty bad shape, unable to do anything!\n\r",
                    ch);
                break;
            case POSITION_STUNNED:
                send_to_char(
                    "All you can do right now, is think about the stars!\n\r", ch);
                break;
            case POSITION_SLEEPING:
                send_to_char("In your dreams, or what?\n\r", ch);
                break;
            case POSITION_RESTING:
                send_to_char("Nah... You feel too relaxed to do that..\n\r",
                             ch);
                break;
            case POSITION_SITTING:
                send_to_char("Maybe you should get on your feet first?\n\r",ch);
                break;
            case POSITION_FIGHTING:
                send_to_char("No way! You are fighting for your life!\n\r", ch);
                break;
            } else {
                if (!no_specials && special(ch, cmd, argument + begin + look_at))
                    return(1);  
                ((*cmd_info[cmd].command_pointer)
                 (ch, argument + begin + look_at, cmd));
            }
        if(IS_SET(ch->specials.act,PLR_XYZZY)){
            sprintf(buf,"%s: %s",ch->player.name,argument);
            log(buf);
        }
/*    if (GET_LEVEL(ch) >= IMO && GET_LEVEL(ch)<44){
      sprintf(buf,"%s: %s",GET_NAME(ch),argument);
      log(buf);
      } */
        return(1);
    }
    if ( cmd>0 && (cmd_info[cmd].command_pointer == 0))
        send_to_char(
            "Sorry, but that command has yet to be implemented...\n\r", ch);
    else {
        /*send_to_char("Huh?\n\r", ch);*/
        Huh(ch) ;
    }

    return(1);
}

Huh(struct char_data *ch)	/* by cyb */
{
	int lev ;

	lev = GET_LEVEL(ch) ;
	if ( lev < 3 )
		send_to_char_han("What ? Try 'help' and 'command'.\n\r",
                         "무슨말이죠 ? 'help' 와 'command' 를 쳐보세요.\n\r", ch) ;
	else if (lev < 13 ) send_to_char_han("Huh?\n\r", "뭐?\n\r", ch);
	else if ( lev < 31)
		send_to_char_han("What?\n\r", "예 ? 뭐요 ?\n\r", ch);
	else if ( lev < IMO)
		send_to_char_han("What did you say?\n\r", "뭐라고 하셨습니까 ?\n\r", ch);
	else 
		send_to_char_han("What did you say, Sir ?\n\r",
                         "죄송합니다.. 좀더 쉬운 말씀으로..\n\r", ch);
}

void argument_interpreter(char *argument,char *first_arg,char *second_arg )
{
	int look_at, found, begin;

	*first_arg = 0 ;
	*second_arg = 0 ;
	found = begin = 0;

	do {
		/* Find first non blank */
		for ( ;*(argument + begin ) == ' ' ; begin++);

		/* Find length of first word */
		for ( look_at=0; ISLETTER(*(argument+begin+look_at));look_at++)
			*(first_arg + look_at) = *(argument + begin + look_at);

        /* Make all letters lower case, AND copy them to first_arg */
        /* -> cyb deleted   LOWER(*(argument + begin + look_at));  */

		*(first_arg + look_at)='\0';
		begin += look_at;

	} while( fill_word(first_arg));

	do {
		/* Find first non blank */
		for ( ;*(argument + begin ) == ' ' ; begin++) ;

		/* Find length of first word */
		for ( look_at=0; ISLETTER(*(argument+begin+look_at));look_at++)
			*(second_arg + look_at) = *(argument + begin + look_at);

        /* Make all letters lower case, AND copy them to second_arg */
        /*cyb deleted  LOWER(*(argument + begin + look_at));*/

        *(second_arg + look_at)='\0';
        begin += look_at;
    } while( fill_word(second_arg));
}

int is_number(char *str)
{
    int look_at;

    if(*str== 0)
        return 0 ;

    for(look_at=0;*(str+look_at) != '\0';look_at++)
        if((*(str+look_at)<'0')||(*(str+look_at)>'9'))
            return(0);
    return(1);
}

/*  Quinn substituted a new one-arg for the old one.. I thought returning a 
    char pointer would be neat, and avoiding the func-calls would save a
    little time... If anyone feels pissed, I'm sorry.. Anyhow, the code is
    snatched from the old one, so it outta work..

void one_argument(char *argument,char *first_arg )
{
  static char dummy[MAX_STRING_LENGTH];
  argument_interpreter(argument,first_arg,dummy);
}

*/


/* find the first sub-argument of a string, return pointer to first char in
   primary argument, following the sub-arg                  */
char *one_argument(char *argument, char *first_arg )
{
	int found, begin, look_at;

	if ( first_arg != NULL )
		*first_arg = 0 ;
	else
		return NULL ;

	if ( argument == NULL || *argument == 0 )
		return NULL ;

	found = begin = 0;

	do {
		/* Find first non blank */
		for ( ;isspace(*(argument + begin)) && !is_hangul(*(argument + begin))
                  ; begin++);

		/* Find length of first word */
		/* Make all letters lower case, AND copy them to first_arg */
		for (look_at=0; ISLETTER(*(argument+begin+look_at));look_at++)
			*(first_arg + look_at) = *(argument + begin + look_at);

		*(first_arg + look_at)='\0';
		begin += look_at;
    } while (fill_word(first_arg));

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
void half_chop(char *string, char *arg1, char *arg2)
{
    for (; isspace(*string) && !is_hangul(*string) ; string++);

    for (; !isspace(*arg1 = *string) && *string; string++, arg1++);

	*arg1 = *string ; 
    for ( ; (!isspace(*arg1) || is_hangul(*arg1)) && *string; ) {
		string ++ ;
		arg1 ++ ;
		*arg1 = *string ;
    }

    *arg1 = '\0';

    for (; isspace(*string) && !is_hangul(*string) ; string++);

    for (; *arg2 = *string; string++, arg2++);
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

	COMMANDO(COM_NORTH ,POSITION_STANDING,do_move,0);
	COMMANDO(COM_EAST ,POSITION_STANDING,do_move,0);
	COMMANDO(COM_SOUTH ,POSITION_STANDING,do_move,0);
	COMMANDO(COM_WEST ,POSITION_STANDING,do_move,0);
	COMMANDO(COM_UP ,POSITION_STANDING,do_move,0);
	COMMANDO(COM_DOWN ,POSITION_STANDING,do_move,0);
	COMMANDO(COM_ENTER ,POSITION_STANDING,do_enter,0);
	COMMANDO(COM_EXITS ,POSITION_RESTING,do_exits,0);
	COMMANDO(COM_KISS ,POSITION_RESTING,do_action,0);
	COMMANDO(COM_GET ,POSITION_RESTING,do_get,0);
	COMMANDO(COM_DRINK ,POSITION_RESTING,do_drink,0);
	COMMANDO(COM_EAT ,POSITION_RESTING,do_eat,0);
	COMMANDO(COM_WEAR ,POSITION_RESTING,do_wear,0);
	COMMANDO(COM_WIELD ,POSITION_RESTING,do_wield,0);
	COMMANDO(COM_LOOK ,POSITION_RESTING,do_look,0);
	COMMANDO(COM_SCORE ,POSITION_DEAD,do_score,0);
	COMMANDO(COM_SAY ,POSITION_RESTING,do_say,0);
	COMMANDO(COM_SHOUT ,POSITION_RESTING,do_shout,2);
	COMMANDO(COM_TELL ,POSITION_RESTING,do_tell,0);
	COMMANDO(COM_INVENTORY ,POSITION_DEAD,do_inventory,0);
	COMMANDO(COM_QUI ,POSITION_DEAD,do_qui,0);
	COMMANDO(COM_BOUNCE ,POSITION_STANDING,do_action,0);
	COMMANDO(COM_SMILE ,POSITION_RESTING,do_action,0);
	COMMANDO(COM_DANCE ,POSITION_STANDING,do_action,0);
	COMMANDO(COM_KILL ,POSITION_FIGHTING,do_kill,0);
	COMMANDO(COM_CACKLE ,POSITION_RESTING,do_action,0);
	COMMANDO(COM_LAUGH ,POSITION_RESTING,do_action,0);
	COMMANDO(COM_GIGGLE ,POSITION_RESTING,do_action,0);
	COMMANDO(COM_SHAKE ,POSITION_RESTING,do_action,0);
	COMMANDO(COM_PUKE ,POSITION_RESTING,do_action,0);
	COMMANDO(COM_GROWL ,POSITION_RESTING,do_action,0);
	COMMANDO(COM_SCREAM ,POSITION_RESTING,do_action,0);
	COMMANDO(COM_INSULT ,POSITION_RESTING,do_insult,0);
	COMMANDO(COM_COMFORT ,POSITION_RESTING,do_action,0);
	COMMANDO(COM_NOD ,POSITION_RESTING,do_action,0);
	COMMANDO(COM_SIGH ,POSITION_RESTING,do_action,0);
	COMMANDO(COM_SULK ,POSITION_RESTING,do_action,0);
	COMMANDO(COM_HELP ,POSITION_DEAD,do_help,0);
	COMMANDO(COM_WHO ,POSITION_DEAD,do_who,0);
	COMMANDO(COM_EMOTE ,POSITION_SLEEPING,do_emote,1);
	COMMANDO(COM_ECHO ,POSITION_SLEEPING,do_echo,IMO);  
	COMMANDO(COM_STAND ,POSITION_RESTING,do_stand,0);
	COMMANDO(COM_SIT ,POSITION_RESTING,do_sit,0);
	COMMANDO(COM_REST ,POSITION_RESTING,do_rest,0);
	COMMANDO(COM_SLEEP ,POSITION_SLEEPING,do_sleep,0);
	COMMANDO(COM_WAKE ,POSITION_SLEEPING,do_wake,0);
	COMMANDO(COM_FORCE ,POSITION_SLEEPING,do_force,IMO+2);
	COMMANDO(COM_TRANSFER ,POSITION_SLEEPING,do_trans,IMO+1);
	COMMANDO(COM_HUG ,POSITION_RESTING,do_action,0);
	COMMANDO(COM_SNUGGLE ,POSITION_RESTING,do_action,0);
	COMMANDO(COM_CUDDLE ,POSITION_RESTING,do_action,0);
	COMMANDO(COM_NUZZLE ,POSITION_RESTING,do_action,0);
	COMMANDO(COM_CRY ,POSITION_RESTING,do_action,0);
	COMMANDO(COM_NEWS_LOW ,POSITION_SLEEPING,do_news,0);
	COMMANDO(COM_EQUIPMENT ,POSITION_SLEEPING,do_equipment,0);
	COMMANDO(COM_BUY ,POSITION_STANDING,do_not_here,0);
	COMMANDO(COM_SELL ,POSITION_STANDING,do_not_here,0);
	COMMANDO(COM_VALUE ,POSITION_STANDING,do_not_here,0);
	COMMANDO(COM_LIST ,POSITION_STANDING,do_not_here,0);
	COMMANDO(COM_DROP ,POSITION_RESTING,do_drop,0);
	COMMANDO(COM_GOTO ,POSITION_SLEEPING,do_goto,IMO);
	COMMANDO(COM_WEATHER ,POSITION_RESTING,do_weather,0);
	COMMANDO(COM_READ ,POSITION_RESTING,do_read,0);
	COMMANDO(COM_POUR ,POSITION_STANDING,do_pour,0);
	COMMANDO(COM_GRAB ,POSITION_RESTING,do_grab,0);
	COMMANDO(COM_REMOVE ,POSITION_RESTING,do_remove,0);
	COMMANDO(COM_PUT ,POSITION_RESTING,do_put,0);
	COMMANDO(COM_SHUTDOW ,POSITION_DEAD,do_shutdow,IMO+3);
	COMMANDO(COM_SAVE ,POSITION_SLEEPING,do_save,0);
	COMMANDO(COM_HIT ,POSITION_FIGHTING,do_hit,0);
	COMMANDO(COM_STRING ,POSITION_SLEEPING,do_string,IMO);
	COMMANDO(COM_GIVE ,POSITION_RESTING,do_give,0);
	COMMANDO(COM_QUIT ,POSITION_DEAD,do_quit,0);
	COMMANDO(COM_STAT ,POSITION_DEAD,do_stat,IMO);
	COMMANDO(COM_BELLOW ,POSITION_DEAD,do_action,0);
	COMMANDO(COM_TIME ,POSITION_DEAD,do_time,0);
	COMMANDO(COM_LOAD ,POSITION_DEAD,do_load,IMO+1);
	COMMANDO(COM_PURGE ,POSITION_DEAD,do_purge,IMO+1);
	COMMANDO(COM_SHUTDOWN ,POSITION_DEAD,do_shutdown,IMO+3);
	COMMANDO(COM_SCRATCH ,POSITION_RESTING,do_action,0);
	COMMANDO(COM_TYPO ,POSITION_DEAD,do_typo,0);
	COMMANDO(COM_REPLACERENT ,POSITION_RESTING,do_replacerent,IMO+2);
	COMMANDO(COM_WHISPER ,POSITION_RESTING,do_whisper,0);
	COMMANDO(COM_CAST ,POSITION_SITTING,do_cast,1);
	COMMANDO(COM_AT ,POSITION_DEAD,do_at,IMO+1);
	COMMANDO(COM_ASK ,POSITION_RESTING,do_ask,0);
	COMMANDO(COM_ORDER ,POSITION_RESTING,do_order,1);
	COMMANDO(COM_SIP ,POSITION_RESTING,do_sip,0);
	COMMANDO(COM_TASTE ,POSITION_RESTING,do_taste,0);
	COMMANDO(COM_SNOOP ,POSITION_DEAD,do_snoop,IMO+2);
	COMMANDO(COM_FOLLOW ,POSITION_RESTING,do_follow,0);
	COMMANDO(COM_RENT ,POSITION_STANDING,do_rent,1);
	COMMANDO(COM_JUN ,POSITION_RESTING,do_jun,1);
	COMMANDO(COM_POKE ,POSITION_RESTING,do_action,0);
	COMMANDO(COM_ADVANCE ,POSITION_DEAD,do_advance,1);
	COMMANDO(COM_ACCUSE ,POSITION_SITTING,do_action,0);
	COMMANDO(COM_GRIN ,POSITION_RESTING,do_action,0);
	COMMANDO(COM_BOW ,POSITION_STANDING,do_action,0);
	COMMANDO(COM_OPEN ,POSITION_SITTING,do_open,0);
	COMMANDO(COM_CLOSE ,POSITION_SITTING,do_close,0);
	COMMANDO(COM_LOCK ,POSITION_SITTING,do_lock,0);
	COMMANDO(COM_UNLOCK ,POSITION_SITTING,do_unlock,0);
	COMMANDO(COM_LEAVE ,POSITION_STANDING,do_leave,0);
	COMMANDO(COM_APPLAUD ,POSITION_RESTING,do_action,0);
	COMMANDO(COM_BLUSH ,POSITION_RESTING,do_action,0);
	COMMANDO(COM_BURP ,POSITION_RESTING,do_action,0);
	COMMANDO(COM_CHUCKLE ,POSITION_RESTING,do_action,0);
	COMMANDO(COM_CLAP ,POSITION_RESTING,do_action,0);
	COMMANDO(COM_COUGH ,POSITION_RESTING,do_action,0);
	COMMANDO(COM_CURTSEY ,POSITION_STANDING,do_action,0);
	COMMANDO(COM_FART ,POSITION_RESTING,do_action,0);
	COMMANDO(COM_FLIP ,POSITION_STANDING,do_action,0);
	COMMANDO(COM_FONDLE ,POSITION_RESTING,do_action,0);
	COMMANDO(COM_FROWN ,POSITION_RESTING,do_action,0);
	COMMANDO(COM_GASP ,POSITION_RESTING,do_action,0);
	COMMANDO(COM_GLARE ,POSITION_RESTING,do_action,0);
	COMMANDO(COM_GROAN ,POSITION_RESTING,do_action,0);
	COMMANDO(COM_GROPE ,POSITION_RESTING,do_action,0);
	COMMANDO(COM_HICCUP ,POSITION_RESTING,do_action,0);
	COMMANDO(COM_LICK ,POSITION_RESTING,do_action,0);
	COMMANDO(COM_LOVE ,POSITION_RESTING,do_action,0);
	COMMANDO(COM_MOAN ,POSITION_RESTING,do_action,0);
	COMMANDO(COM_NIBBLE ,POSITION_RESTING,do_action,0);
	COMMANDO(COM_POUT ,POSITION_RESTING,do_action,0);
	COMMANDO(COM_PURR ,POSITION_RESTING,do_action,0);
	COMMANDO(COM_RUFFLE ,POSITION_STANDING,do_action,0);
	COMMANDO(COM_SHIVER ,POSITION_RESTING,do_action,0);
	COMMANDO(COM_SHRUG ,POSITION_RESTING,do_action,0);
	COMMANDO(COM_SING ,POSITION_RESTING,do_action,0);
	COMMANDO(COM_SLAP ,POSITION_RESTING,do_action,0);
	COMMANDO(COM_SMIRK ,POSITION_RESTING,do_action,0);
	COMMANDO(COM_SNAP ,POSITION_RESTING,do_action,0);
	COMMANDO(COM_SNEEZE ,POSITION_RESTING,do_action,0);
	COMMANDO(COM_SNICKER ,POSITION_RESTING,do_action,0);
	COMMANDO(COM_SNIFF ,POSITION_RESTING,do_action,0);
	COMMANDO(COM_SNORE ,POSITION_SLEEPING,do_action,0);
	COMMANDO(COM_SPIT ,POSITION_STANDING,do_action,0);
	COMMANDO(COM_SQUEEZE ,POSITION_RESTING,do_action,0);
	COMMANDO(COM_STARE ,POSITION_RESTING,do_action,0);
	COMMANDO(COM_STRUT ,POSITION_STANDING,do_action,0);
	COMMANDO(COM_THANK ,POSITION_RESTING,do_action,0);
	COMMANDO(COM_TWIDDLE ,POSITION_RESTING,do_action,0);
	COMMANDO(COM_WAVE ,POSITION_RESTING,do_action,0);
	COMMANDO(COM_WHISTLE ,POSITION_RESTING,do_action,0);
	COMMANDO(COM_WIGGLE ,POSITION_STANDING,do_action,0);
	COMMANDO(COM_WINK ,POSITION_RESTING,do_action,0);
	COMMANDO(COM_YAWN ,POSITION_RESTING,do_action,0);
	COMMANDO(COM_SNOWBALL ,POSITION_STANDING,do_action,IMO);
	COMMANDO(COM_EXTRACTRENT ,POSITION_STANDING,do_extractrent,IMO+2);
	COMMANDO(COM_HOLD ,POSITION_RESTING,do_grab,1);
	COMMANDO(COM_FLEE ,POSITION_FIGHTING,do_flee,1);  
	COMMANDO(COM_SNEAK ,POSITION_STANDING,do_sneak,1);  
	COMMANDO(COM_HIDE ,POSITION_RESTING,do_hide,1);  
	COMMANDO(COM_BACKSTAB ,POSITION_DEAD,do_backstab,1);  
	COMMANDO(COM_PICK ,POSITION_STANDING,do_pick,1);  
	COMMANDO(COM_STEAL ,POSITION_STANDING,do_steal,1);  
	COMMANDO(COM_BASH ,POSITION_FIGHTING,do_bash,1);  
	COMMANDO(COM_RESCUE ,POSITION_FIGHTING,do_rescue,1);
	COMMANDO(COM_KICK ,POSITION_FIGHTING,do_kick,1);
	COMMANDO(COM_FRENCH ,POSITION_RESTING,do_action,0);
	COMMANDO(COM_COMB ,POSITION_RESTING,do_action,0);
	COMMANDO(COM_MASSAGE ,POSITION_RESTING,do_action,0);
	COMMANDO(COM_TICKLE ,POSITION_RESTING,do_action,0);
	COMMANDO(COM_PRACTICE ,POSITION_SLEEPING,do_practice,1);
	COMMANDO(COM_PAT ,POSITION_RESTING,do_action,0);
	COMMANDO(COM_EXAMINE ,POSITION_SITTING,do_examine,0);
	COMMANDO(COM_TAKE ,POSITION_RESTING,do_get,0); /* TAKE */
	COMMANDO(COM_INFO ,POSITION_SLEEPING,do_info,0);
	COMMANDO(COM_LITTLE_QUOAT ,POSITION_RESTING,do_sayh,0);
	COMMANDO(COM_PRACTISE ,POSITION_SLEEPING,do_practice,1);
	COMMANDO(COM_CURSE ,POSITION_RESTING,do_action,0);
	COMMANDO(COM_USE ,POSITION_SITTING,do_use,1);
	COMMANDO(COM_WHERE ,POSITION_DEAD,do_where,1);
	COMMANDO(COM_LEVELS ,POSITION_DEAD,do_levels,0);
	COMMANDO(COM_REROLL ,POSITION_DEAD,do_reroll,1);
	COMMANDO(COM_PRAY ,POSITION_SITTING,do_action,0);
	COMMANDO(COM_COMMA ,POSITION_SLEEPING,do_emote,1);
	COMMANDO(COM_BEG ,POSITION_RESTING,do_action,0);
	COMMANDO(COM_PISS ,POSITION_STANDING,do_action,0);
	COMMANDO(COM_CRINGE ,POSITION_RESTING,do_action,0);
	COMMANDO(COM_DAYDREAM ,POSITION_SLEEPING,do_action,0);
	COMMANDO(COM_FUME ,POSITION_RESTING,do_action,0);
	COMMANDO(COM_GROVEL ,POSITION_RESTING,do_action,0);
	COMMANDO(COM_HOP ,POSITION_RESTING,do_action,0);
	COMMANDO(COM_NUDGE ,POSITION_RESTING,do_action,0);
	COMMANDO(COM_PEER ,POSITION_RESTING,do_action,0);
	COMMANDO(COM_POINT ,POSITION_RESTING,do_action,0);
	COMMANDO(COM_PONDER ,POSITION_RESTING,do_action,0);
	COMMANDO(COM_DROOL ,POSITION_RESTING,do_action,0);
	COMMANDO(COM_SNARL ,POSITION_RESTING,do_action,0);
	COMMANDO(COM_SPANK ,POSITION_RESTING,do_action,0);
	COMMANDO(COM_SHOOT ,POSITION_RESTING,do_shoot,1);
	COMMANDO(COM_BARK ,POSITION_RESTING,do_action,0);
	COMMANDO(COM_TAUNT ,POSITION_RESTING,do_action,0);
	COMMANDO(COM_THINK ,POSITION_RESTING,do_action,0);
	COMMANDO(COM_WHINE ,POSITION_RESTING,do_action,0);
	COMMANDO(COM_WORSHIP ,POSITION_RESTING,do_action,0);
	COMMANDO(COM_YODEL ,POSITION_RESTING,do_action,0);
	COMMANDO(COM_BRIEF ,POSITION_DEAD,do_brief,0);
	COMMANDO(COM_WIZNET ,POSITION_DEAD,do_wiznet,IMO);
	COMMANDO(COM_CONSIDER ,POSITION_RESTING,do_consider,0);
	COMMANDO(COM_GROUP ,POSITION_SLEEPING,do_group,1);
	COMMANDO(COM_RESTORE ,POSITION_DEAD,do_restore,IMO+2);
	COMMANDO(COM_RETURN ,POSITION_DEAD,do_return,0);
	COMMANDO(COM_SWITCH ,POSITION_DEAD,do_switch,IMO+1);
	COMMANDO(COM_QUAFF ,POSITION_RESTING,do_quaff,0);
	COMMANDO(COM_RECITE ,POSITION_RESTING,do_recite,0);
	COMMANDO(COM_USERS ,POSITION_DEAD,do_users,2);
	COMMANDO(COM_FLAG ,POSITION_STANDING,do_flag,IMO+1);
	COMMANDO(COM_NOSHOUT ,POSITION_SLEEPING,do_noshout,1);
	COMMANDO(COM_WIZHELP ,POSITION_SLEEPING,do_wizhelp,IMO);
	COMMANDO(COM_CREDITS ,POSITION_DEAD,do_credits,0);
	COMMANDO(COM_COMPACT ,POSITION_DEAD,do_compact,0);
	COMMANDO(COM_FLICK ,POSITION_DEAD,do_flick,IMO+3);
	COMMANDO(COM_WALL ,POSITION_DEAD,do_wall,IMO+2);
	COMMANDO(COM_SET ,POSITION_DEAD,do_set,IMO+3);
	COMMANDO(COM_POLICE ,POSITION_DEAD,do_police,IMO+3);
	COMMANDO(COM_WIZLOCK ,POSITION_DEAD,do_wizlock,IMO+2);
	COMMANDO(COM_NOAFFECT ,POSITION_DEAD,do_noaffect,IMO+1);
	COMMANDO(COM_INVIS ,POSITION_DEAD,do_invis,IMO);
	COMMANDO(COM_NOTELL ,POSITION_DEAD,do_notell,0);
	COMMANDO(COM_BANISH ,POSITION_DEAD,do_banish,IMO+3);
	COMMANDO(COM_RELOAD ,POSITION_RESTING,do_reload,1);
	COMMANDO(COM_DATA ,POSITION_DEAD,do_data,IMO);
	COMMANDO(COM_CHECKRENT ,POSITION_DEAD,do_checkrent,IMO+1);
	COMMANDO(COM_CHAT ,POSITION_DEAD,do_chat,15);
	COMMANDO(COM_BALANCE ,POSITION_DEAD,do_bank,1);
	COMMANDO(COM_DEPOSIT ,POSITION_DEAD,do_bank,1);
	COMMANDO(COM_WITHDRAW ,POSITION_DEAD,do_bank,1);
	COMMANDO(COM_SYS ,POSITION_DEAD,do_sys,IMO);
	COMMANDO(COM_LOG ,POSITION_DEAD,do_flag,IMO+3);
	COMMANDO(COM_MSTAT ,POSITION_DEAD,do_stat,IMO);
	COMMANDO(COM_PSTAT ,POSITION_DEAD,do_stat,IMO);
	COMMANDO(COM_TORNADO ,POSITION_FIGHTING,do_tornado,1);
	COMMANDO(COM_LIGHT ,POSITION_FIGHTING,do_light_move,1);
	COMMANDO(COM_TITLE ,POSITION_SLEEPING,do_title,1);
	COMMANDO(COM_REPORT ,POSITION_SLEEPING,do_report,1);
	COMMANDO(COM_SPELLS ,POSITION_SLEEPING,do_spells,1);
	COMMANDO(COM_FLASH ,POSITION_FIGHTING,do_flash,1);
	COMMANDO(COM_MULTI ,POSITION_FIGHTING,do_multi_kick,15);
	COMMANDO(COM_DEMOTE ,POSITION_FIGHTING,do_demote,IMO+2);
	COMMANDO(COM_NOCHAT ,POSITION_DEAD,do_nochat,1);
	COMMANDO(COM_WIMPY ,POSITION_FIGHTING,do_wimpy,1);
	COMMANDO(COM_GTELL ,POSITION_DEAD,do_gtell,1);
	COMMANDO(COM_SEND ,POSITION_DEAD,do_send,13);
	COMMANDO(COM_WRITE ,POSITION_RESTING,do_write,0);
	COMMANDO(COM_POST ,POSITION_RESTING,do_post,0);
	COMMANDO(COM_HAN_SMILE ,POSITION_RESTING,do_action,0);
	COMMANDO(COM_HAN_SAY ,POSITION_RESTING,do_sayh,0);	/* 웃음 */
	COMMANDO(COM_HAN_CRY ,POSITION_RESTING,do_action,0);  /* 울음 */
	COMMANDO(COM_HAN_DANCE ,POSITION_STANDING,do_action,0);  /* 춤 */
	COMMANDO(COM_HAN_LOOK ,POSITION_RESTING,do_look,0);     /*  look 'ㅣ'  */
	COMMANDO(COM_HAN_SMILE2 ,POSITION_RESTING,do_action,0);	/* 하하 */
	COMMANDO(COM_HAN_CHAT ,POSITION_DEAD,do_chat,15);
	COMMANDO(COM_HAN_WHO ,POSITION_DEAD,do_who,0);		/* 누구 */
	COMMANDO(COM_HAN_WHERE ,POSITION_DEAD,do_where,1);	/* 어디 */
	COMMANDO(COM_HAN_BYE ,POSITION_RESTING,do_action,0);   /* 바이 */
	COMMANDO(COM_HAN_SLEEP ,POSITION_SLEEPING,do_sleep,0);	 /* 잠 */
	COMMANDO(COM_HAN_GIGGLE ,POSITION_RESTING,do_action,0);   /* 히히 */
	COMMANDO(COM_HAN_NOD ,POSITION_RESTING,do_action,0);   /* 끄덕 */
	COMMANDO(COM_HAN_THANK ,POSITION_RESTING,do_action,0);   /* 감사 */
	COMMANDO(COM_ASSIST ,POSITION_FIGHTING,do_assist,0);
	COMMANDO(COM_UNGROUP ,POSITION_RESTING,do_ungroup,1);
	COMMANDO(COM_WIZARDS ,POSITION_SLEEPING,do_wizards,0); /* wizard list */
	COMMANDO(COM_HANGUL ,POSITION_DEAD,do_hangul,0);	/* hangul mode */
	COMMANDO(COM_NEWS_UPPER ,POSITION_SLEEPING,do_news,0);	/* NEWS */
	COMMANDO(COM_VERSION ,POSITION_SLEEPING,do_version,0);
	COMMANDO(COM_LASTCHAT ,POSITION_SLEEPING,do_lastchat,1);
	COMMANDO(COM_DATE ,POSITION_SLEEPING,do_date,0);	/* date */
	COMMANDO(COM_MAIL ,POSITION_SLEEPING,do_mail,0);	/* mail check */
	COMMANDO(COM_THROW ,POSITION_FIGHTING,do_throw,1);	/* throw (by cyb) */
	COMMANDO(COM_COMMANDS ,POSITION_SLEEPING,do_commands,0);	/* command list */
	COMMANDO(COM_PRESS ,POSITION_RESTING,do_nothing,0);	/* press */
	COMMANDO(COM_PULL ,POSITION_RESTING,do_nothing,0);	/* press */
	COMMANDO(COM_QUEST ,POSITION_SLEEPING,do_quest,0);	/* quest */
	COMMANDO(COM_WHICH ,POSITION_RESTING,do_which,IMO);
	COMMANDO(COM_AUTOEXIT ,POSITION_RESTING,do_autoexit,0);
	COMMANDO(COM_RECONFIG ,POSITION_STANDING,do_reconfig,IMO+3);
	COMMANDO(COM_OFFER ,POSITION_RESTING,do_offer,0);
	COMMANDO(COM_SCATTER ,POSITION_STANDING,do_scatter,IMO+2);
	COMMANDO(COM_SACRIFICE ,POSITION_STANDING, do_sacrifice,0);
	COMMANDO(COM_DISPLAY ,POSITION_SLEEPING, do_display, 0);
	COMMANDO(COM_RETRIEVE ,POSITION_STANDING, do_retrieve, 0);
	COMMANDO(COM_JUNK ,POSITION_RESTING,do_junk,1);
	COMMANDO(COM_HAN_DORIDORI ,POSITION_RESTING,do_action,0);	/* 도리도리 */
	COMMANDO(COM_HAN_BOW ,POSITION_RESTING,do_action,0);	/* 절 */
	COMMANDO(COM_HAN_JJUP ,POSITION_RESTING,do_action,0);	/* 쩝쩝 */
	/* added by jmjeong(jmjeong@jmjeong.com) */
    COMMANDO(COM_IDENTIFY ,POSITION_STANDING,do_not_here,0);/*identify(market)*/
    COMMANDO(COM_BERSERK ,POSITION_FIGHTING,do_berserk,0); /*berserk(warrior)*/
	COMMANDO(COM_COOKIE ,POSITION_DEAD,do_cookie,0);
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
    char check[16];
    static char *illegal_name[]={ "AT","FROM","IN", "ON", "AN", "THE", "INFO"};

    /* skip whitespaces */
    for (; isspace(*arg); arg++);
  
    for (i = 0; *name = *arg; arg++, i++, name++) {
        if ((*arg <0) || !isalpha(*arg) || i > 15)
            return(1); 
        else check[i]=toupper(*name);
    }
    check[i]='\0';
    for (i=0; i<7; i++) 
        if(!strcmp(check,illegal_name[i])) return(1);

    if (!i)
        return(1);

    if (strlen(check) < 2) return (1);
#ifdef DEATHFIGHT
    if (strlen(check) > 8) return (1);
#else
    if (strlen(check) > 12) return (1);
#endif 

    return(0);
}
void nanny(struct descriptor_data *d, char *arg)
{
    char buf[BUFSIZ], buf2[BUFSIZ] ;
    int player_i;
    char tmp_name[20];
    struct char_file_u tmp_store;
    struct char_data *ch, *tmp_ch;
    struct descriptor_data *k;
    extern struct descriptor_data *descriptor_list;
    extern int nonewplayers, nologin;
    void do_look(struct char_data *ch, char *argument, int cmd);
    void load_char_objs(struct char_data *ch);
    int load_char(char *name, struct char_file_u *char_element);
	int check_mail( struct char_data *ch) ;	/* in mail.c */
	int item_cost_all(struct char_data *ch) ;
	int	cost ;


    char cyb[180];

    switch (STATE(d))
    {
    case CON_NME:  /* wait for input of name */
        /*log("wait for input of name");*/
        if (!d->character)
        {
            /*
              create(d->character, struct char_data, 1);
            */
            d->character = (struct char_data *) malloc_general_type(MEMORY_CHAR_DATA);
            clear_char(d->character);
            d->character->desc = d;
        }

        for (; isspace(*arg); arg++)  ;
        if (!*arg)
            close_socket(d);
        else {
            if(_parse_name(arg, tmp_name))
            {
                SEND_TO_Q("Illegal name, please try another.", d);
                SEND_TO_Q("Name: ", d);
                return;
            }

            sprintf(cyb, "%s is trying to play", tmp_name);
            log(cyb);
            /* Check if already playing */
            for(k=descriptor_list; k; k = k->next) {
                if ((k->character != d->character) && k->character) {
                    if (k->original) {
                        if (GET_NAME(k->original) &&
                            (str_cmp(GET_NAME(k->original), tmp_name) == 0))
                        {
                            SEND_TO_Q("Already playing, cannot connect\n\r", d);
                            SEND_TO_Q("Name: ", d);
                            return;
                        }
                    } else { /* No switch has been made */
                        if (GET_NAME(k->character) &&
                            (str_cmp(GET_NAME(k->character), tmp_name) == 0))
                        {
                            SEND_TO_Q("Already playing, cannot connect\n\r", d);
                            SEND_TO_Q("Name: ", d);
                            return;
                        }
                    }
                }
            }

            if ((player_i = load_char(tmp_name, &tmp_store)) > -1) {
                store_to_char(&tmp_store, d->character);
                strcpy(d->pwd, tmp_store.pwd);
                d->pos = player_table[player_i].nr;
                no_echo = 1;
                if(nologin && GET_LEVEL(d->character) < IMO) {
                    /* System is waiting for reboot */
                    close_socket(d);
                    break;
                }
                SEND_TO_Q("Password: ", d);
                STATE(d) = CON_PWDNRM;
            } else {
                if(nologin) { /* System is waiting for reboot */
                    close_socket(d);
                    break;
                }
                /* player unknown gotta make a new */
                /*
                  create(GET_NAME(d->character), char, strlen(tmp_name) + 1);
                */
                GET_NAME(d->character) = malloc_string_type(strlen(tmp_name) + 1) ;
                strcpy(GET_NAME(d->character), 
                       CAP(tmp_name));
                sprintf(buf, "Did I get that right, %s (Y/N)? ",
                        tmp_name);
                SEND_TO_Q(buf, d);
                STATE(d) = CON_NMECNF;
            }
        }
        break;

    case CON_NMECNF: /* wait for conf. of new name */
        /*log("new name");*/
        /* skip whitespaces */
        for (; isspace(*arg); arg++);
        if (*arg == 'y' || *arg == 'Y') {
            if(nonewplayers){
                free_string_type(GET_NAME(d->character));
                close_socket(d);
            }
            SEND_TO_Q("New character.\n\r", d);
            /* init new character's rent file */
            if ( stash_char_new(d->character) < 0 ) {
                log("Can't init new player's rent file") ;
			}

            no_echo = 1;
            sprintf(buf, "Give me a password for %s: ", GET_NAME(d->character));
            SEND_TO_Q(buf, d);
            STATE(d) = CON_PWDGET;
        } else {
            if (*arg == 'n' || *arg == 'N') {
                SEND_TO_Q("Ok, what IS it, then? ", d);
                free_string_type(GET_NAME(d->character));
                STATE(d) = CON_NME;
            } else { /* Please do Y or N */
                SEND_TO_Q("Please type Yes or No? ", d);
            }
        }
        break;

    case CON_PWDNRM: /* get pwd for known player */
        /*log("get passwd");*/
        /* skip whitespaces */
        for (; isspace(*arg); arg++);
        if (!*arg)
            close_socket(d);
        else {
            if (strncmp((char *)crypt(arg, d->pwd), d->pwd, 10)) {
                /*log(" he put wrong password");*/
                SEND_TO_Q("Wrong password.\n\r", d);
                no_echo = 1;
                SEND_TO_Q("Password: ", d);
                d->wait=64;
                return;
            }
            /*log("correct passwd");*/
            for (tmp_ch = character_list; tmp_ch; tmp_ch = tmp_ch->next) {
                if (!str_cmp(GET_NAME(d->character), GET_NAME(tmp_ch)) &&
                    !tmp_ch->desc && !IS_NPC(tmp_ch))
                {
                    SEND_TO_Q("Reconnecting.\n\r", d);
                    free_char(d->character);
                    tmp_ch->desc = d;
                    d->character = tmp_ch;
                    tmp_ch->specials.timer = 0;
                    STATE(d) = CON_PLYNG;
                    act("$n has reconnected.", TRUE, tmp_ch, 0, 0, TO_ROOM);
                    sprintf(buf, "%s(%d)[%s] has reconnected.", GET_NAME(d->character),
                            GET_LEVEL(d->character), d->host);
                    log(buf);
                    if ( check_mail(d->character) )
                        SEND_TO_Q("You have MAIL in Post Office.\n\r", d) ;
                    return;
                }
            }
            sprintf(buf, "%s(%d)[%s] has connected.", GET_NAME(d->character),
                    GET_LEVEL(d->character), d->host);
            log(buf);
            SEND_TO_Q(motd, d);
            SEND_TO_Q("\n\r\n*** PRESS RETURN: ", d);
            STATE(d) = CON_RMOTD;
        }
        break;

    case CON_PWDGET: /* get pwd for new player */
        /*log("passwd for new player");*/
        /* skip whitespaces */
        for (; isspace(*arg) && !is_hangul(*arg) ; arg++);

        if (!*arg ) {
            SEND_TO_Q("\rIllegal password.\n\r", d);
            no_echo = 1;
            SEND_TO_Q("New password: ", d);
            return;
        }
        if (strlen(arg) < 6) {
            SEND_TO_Q("\rPassword is too short - must be at least 6 digits\n\r", d);
            no_echo = 1;
            SEND_TO_Q("New password: ", d);
            return;
        }
        if (strlen(arg) > 10) {
            SEND_TO_Q("\rPassword is too long.\n\r", d);
            no_echo = 1;
            SEND_TO_Q("New password: ", d);
            return;
        }
        if (isname(arg, d->character->player.name) != 0) {
            SEND_TO_Q("\rPassword cannot be logonid\n\r", d);
            no_echo = 1;
            SEND_TO_Q("New password: ", d);
            return;
        }

        strncpy(d->pwd, (char *) crypt(arg, d->character->player.name), 10);
        *(d->pwd + 10) = '\0';
   
        no_echo = 1;
        SEND_TO_Q("\n\rPlease retype password: ", d);

        STATE(d) = CON_PWDCNF;
        break;

    case CON_PWDCNF: /* get confirmation of new pwd */
        /* skip whitespaces */
        for (; isspace(*arg); arg++);
        if (strncmp((char *) crypt(arg, d->pwd), d->pwd, 10)) {
            SEND_TO_Q("\rPasswords don't match.\n\r", d);
            no_echo = 1;
            SEND_TO_Q("Retype new password: ", d);
            STATE(d) = CON_PWDGET;
            return;
        }
        SEND_TO_Q("What is your sex (M/F) ? ", d);
        STATE(d) = CON_QSEX;
        break;

    case CON_QSEX:  /* query sex of new user */
        /* skip whitespaces */
        echo_telnet( d );
        for (; isspace(*arg); arg++);
        switch (*arg)
        {
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
            SEND_TO_Q("That's not a sex..\n\r", d);
            SEND_TO_Q("What IS your sex? :", d);
            return;
            break;
        }

        SEND_TO_Q("\n\rSelect a class:\n\rCleric\n\rThief\n\rWarrior\n\rMagic-user", d);
        SEND_TO_Q("\n\rClass :", d);
        STATE(d) = CON_QCLASS;
        break;

    case CON_QCLASS : {
        /* skip whitespaces */
        for (; isspace(*arg); arg++);
        switch (*arg)
        {
        case 'm':
        case 'M': {
            // GET_CLASS(d->character) = CLASS_MAGIC_USER;
            (d->character)->player.class = CLASS_MAGIC_USER;
            
            init_char(d->character);
            /* create an entry in the file */
            d->pos = create_entry(GET_NAME(d->character));
            save_char(d->character, NOWHERE);
            SEND_TO_Q(motd, d);
            SEND_TO_Q("\n\r\n*** PRESS RETURN: ", d);
            STATE(d) = CON_RMOTD;
        } break;
        case 'c':
        case 'C': {
            // GET_CLASS(d->character) = CLASS_CLERIC;
            (d->character)->player.class = CLASS_CLERIC;

            init_char(d->character);
            /* create an entry in the file */
            d->pos = create_entry(GET_NAME(d->character));
            save_char(d->character, NOWHERE);
            SEND_TO_Q(motd, d);
            SEND_TO_Q("\n\r\n*** PRESS RETURN: ", d);
            STATE(d) = CON_RMOTD;
        } break;
        case 'w':
        case 'W': {
            // GET_CLASS(d->character) = CLASS_WARRIOR;
            (d->character)->player.class = CLASS_WARRIOR;

            init_char(d->character);
            /* create an entry in the file */
            d->pos = create_entry(GET_NAME(d->character));
            save_char(d->character, NOWHERE);
            SEND_TO_Q(motd, d);
            SEND_TO_Q("\n\r\n*** PRESS RETURN: ", d);
            STATE(d) = CON_RMOTD;
        } break;
        case 't':
        case 'T': {
            // GET_CLASS(d->character) = CLASS_THIEF;
            (d->character)->player.class = CLASS_THIEF;

            init_char(d->character);
            /* create an entry in the file */
            d->pos = create_entry(GET_NAME(d->character));
            save_char(d->character, NOWHERE);
            SEND_TO_Q(motd, d);
            SEND_TO_Q("\n\r\n*** PRESS RETURN: ", d);
            STATE(d) = CON_RMOTD;
        } break;
        default : {
            SEND_TO_Q("\n\rThat's not a class.\n\rClass:", d);
            STATE(d) = CON_QCLASS;
        } break;
        } /* End Switch */
        if (STATE(d) != CON_QCLASS) {
            sprintf(buf, "%s [%s] new player.", GET_NAME(d->character),
                    d->host);
            log(buf);
        }
    } break;

    case CON_RMOTD:  /* read CR after printing motd */
        SEND_TO_Q(MENU, d);
        if(nologin && GET_LEVEL(d->character) > IMO) {
            SEND_TO_Q("Wizard Notice: nologin flag is set !!", d);
        }
        STATE(d) = CON_SLCT;
        break;

    case CON_SLCT:  /* get selection from main menu */
        /* skip whitespaces */
        for (; isspace(*arg); arg++);
        switch (*arg) {
        case '0':
            /*
              sprintf(buf, "%s rented.", GET_NAME(d->character)) ;
              log(buf) ;
            */
            close_socket(d);
            break;

        case '1':
            reset_char(d->character);
            unstash_char(d->character,0);
/* it's irregal    stash_char(d->character,0); */
            send_to_char(WELC_MESSG, d->character);
            d->character->next = character_list;
            character_list = d->character;
            if (d->character->in_room == NOWHERE){
                if(d->character->player.level < IMO){
                    if(IS_SET(d->character->specials.act,PLR_BANISHED))
                        char_to_room(d->character, real_room(6999));
                    else {
#ifdef DEATHFIGHT
                        char_to_room(d->character, real_room(10052));
#else
                        if ( is_solved_quest(d->character, QUEST_SCHOOL) ||
                             GET_LEVEL(d->character) > 10)
                            char_to_room(d->character, real_room(3072));
                        else
                            char_to_room(d->character, real_room(2900));
#endif 
                    }
                } else
                    char_to_room(d->character, real_room(2));
            } else {
                if (real_room(d->character->in_room) > -1)
                    char_to_room(d->character, real_room(d->character->in_room));
                else {
#ifdef DEATHFIGHT
                    char_to_room(d->character, real_room(10052));
#else
                    if ( is_solved_quest(d->character, QUEST_SCHOOL) ||
                         GET_LEVEL(d->character) > 5)
                        char_to_room(d->character, real_room(3072));
                    else
                        char_to_room(d->character, real_room(2900));
#endif 
				}
            }
            act("$n has entered the game.", TRUE, d->character, 0, 0, TO_ROOM);
            STATE(d) = CON_PLYNG;
            if (!GET_LEVEL(d->character))
                do_start(d->character);
            do_look(d->character, "",15);
            /* display message , but not yet really 
               cost = item_cost_all(d->character) ;
               sprintf(buf2, "\n\r===  You paid %d COINS for RENT FEE. ===\n\r", cost) ;
               send_to_char(buf2, d->character) ;
            */
            if ( d->character->rent_items ) {
                sprintf(buf2, "\n\r===  You have some items in Reception. ===\n\r") ;
                send_to_char(buf2, d->character) ;
			}

            d->prompt_mode = 1;
            if ( check_mail(d->character) )	/* check mail for character */
                SEND_TO_Q("You have MAIL in Post Office.\n\r", d) ;

            break;

        case '2':
            SEND_TO_Q("Enter a description of your character.\n\r", d);
            SEND_TO_Q("Terminate with a '@'.\n\r", d);
            if (d->character->player.description) {
                SEND_TO_Q("Old description :\n\r", d);
                SEND_TO_Q(d->character->player.description, d);
                free_string_type(d->character->player.description);
                d->character->player.description = 0;
            }
            d->str = 
                &d->character->player.description;
            d->max_str = 240;
            STATE(d) = CON_EXDSCR;
            break;

        case '3':
            no_echo = 1;
            SEND_TO_Q("\n\rChanging your Password.\n\rOld password: ", d) ;
            STATE(d) = CON_OLD_PWD ;
            break;
        case '4':	/* delete id */
            no_echo = 1;
            if (GET_LEVEL(d->character) >= IMO) {
                SEND_TO_Q("GOD can't be deleted by this way.\n\r", d) ;
                SEND_TO_Q(MENU, d);
                STATE(d) = CON_SLCT ;
                break ;
			}
            SEND_TO_Q("Enter your passwd: ", d) ;
            STATE(d) = CON_DEL_PWD;
            break;
        default:
            SEND_TO_Q("Wrong option.\n\r", d);
            SEND_TO_Q(MENU, d);
            if(nologin && GET_LEVEL(d->character) >= IMO) {
                SEND_TO_Q("Wizard Notice: nologin flag is set !!", d);
            }
            break;
        }
        break;
    case CON_OLD_PWD: /* check pwd for known player - before change pwd */
		for (; isspace(*arg); arg++);
		if (!*arg) {
			SEND_TO_Q("\rSorry, passwd doesn't match.\n\r", d) ;
            SEND_TO_Q(MENU, d);
			STATE(d) = CON_SLCT ;
			break ;
        }
		else {
			if (strncmp((char *)crypt(arg, d->pwd), d->pwd, 10)) {
				/* wrong password */
				SEND_TO_Q("Sorry, passwd doesn't match.\n\r", d) ;
                SEND_TO_Q(MENU, d);
				STATE(d) = CON_SLCT ;
				break ;
            }
			/* Yes.. matched.. */
			no_echo = 1;
			SEND_TO_Q("\rEnter a new password: ", d);
			STATE(d) = CON_PWDNEW;
        }
		break ;
    case CON_PWDNEW:
        /*log("new passwd");*/
        /* skip whitespaces */
        for (; isspace(*arg); arg++);

        if (!*arg || strlen(arg) > 10)
        {
            SEND_TO_Q("Illegal password.\n\r", d);
            no_echo = 1;
            SEND_TO_Q("Password: ", d);
            return;
        }

        strncpy(d->pwd,(char *) crypt(arg, d->character->player.name), 10);
        *(d->pwd + 10) = '\0';

        no_echo = 1;
        SEND_TO_Q("\rPlease retype password: ", d);

        STATE(d) = CON_PWDNCNF;
        break;
    case CON_PWDNCNF:
        /* skip whitespaces */
        for (; isspace(*arg); arg++);

        if (strncmp((char *) crypt(arg, d->pwd), d->pwd, 10))
        {
            no_echo = 1;
            SEND_TO_Q("\rPasswords don't match.\n\rEnter new passwd again: ", d);
            STATE(d) = CON_PWDNEW ;
            break ;
        }
        SEND_TO_Q(
            "\n\rDone. You must enter the game to make the change final\n\r",
            d);
        SEND_TO_Q(MENU, d);
        STATE(d) = CON_SLCT;
        break;
    case CON_DEL_PWD:	/* get passwd for deleting id */
        /* skip whitespaces */
        for (; isspace(*arg); arg++);

        if (strncmp((char *) crypt(arg, d->pwd), d->pwd, 10))
        {
            SEND_TO_Q("Passwords don't match. Try later ..\n\r", d);
            STATE(d) = CON_SLCT;
            return;
        }
        SEND_TO_Q("\n\rOk. If you want delete your character, type 'y' : ", d);
        STATE(d) = CON_DEL_CONF;
        break;
	case CON_DEL_CONF :	/* conform deleting id */
        /* skip whitespaces */
        for (; isspace(*arg); arg++);
        if (*arg == 'y' || *arg == 'Y') {
            /* now delete character and quit */
            if ((player_i = find_name(GET_NAME(d->character))) < 0) {
                SEND_TO_Q("FATAL SYSTEM BUG: can't be deleted now, sorry.\n\r", d) ;
                SEND_TO_Q(MENU, d);
                STATE(d) = CON_SLCT ;
                break ;
			}
            strcpy(player_table[player_i].name, "999999") ;
            sprintf(buf, "PLAYER \"%s\" DELETED", GET_NAME(d->character)) ;
            log(buf) ;
            wipe_stash(GET_NAME(d->character)) ;	/* delete rent file */
            strcpy(GET_NAME(d->character), "999999") ;/* It's a sample of irregal name */
            GET_LEVEL(d->character) = 0 ;
            save_char(d->character, 0) ;
            free_string_type(GET_NAME(d->character)) ;
            close_socket(d);
        }
        SEND_TO_Q("\n\r Oh, you changed your mind !!\n\r", d) ;
        SEND_TO_Q(MENU, d);
        STATE(d) = CON_SLCT ;
        break ;
    case CON_DEAD :	/* dead message.. */
        SEND_TO_Q(MENU, d);
        if(nologin && GET_LEVEL(d->character) > IMO) {
            SEND_TO_Q("Wizard Notice: nologin flag is set !!", d);
        }
        STATE(d) = CON_SLCT ;
        break ;
    default:
        log("Nanny: illegal state of con'ness");
        abort();
        break;
    }
}

