/* ************************************************************************
*  file: act.comm.c , Implementation of commands.         Part of DIKUMUD *
*  Usage : Communication. and displaying message to PC player		  *
*  Copyright (C) 1990, 1991 - see 'license.doc' for complete information. *
*									  *
* NOTE: Merged sources from OLD act.socail.c and part of OLD comm.c       *
************************************************************************* */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

#include "char.h"
#include "object.h"
#include "global.h"
#include "comm.h"
#include "actions.h"

/* Locals */
/* NOTE: Now symbolic constant. LASTCHAT_SIZE */
#define LASTCHAT_SIZE 40
static char history[LASTCHAT_SIZE][MAX_LINE_LEN];
static int his_start = 0, his_end = 0;

void do_lastchat(struct char_data *ch, char *argument, int cmd);
void chat_history(char *str);

int is_dumb(struct char_data *ch)
{
    return(IS_ACTPLR(ch, PLR_DUMB_BY_WIZ) && NOT_GOD(ch));
}

#ifdef UNUSED_CODE
/* 말하기 */
void do_say(struct char_data *ch, char *argument, int cmd)
{
    int i;
    char buf[MAX_STRING_LENGTH];

    if (IS_SET(ch->specials.act, PLR_DUMB_BY_WIZ) && NOT_GOD(ch)) {
	return;
    }
    for (i = 0; *(argument + i) == ' '; i++) ;

    if (!*(argument + i))
	send_to_char("Yes, but WHAT do you want to say?\r\n", ch);
    else {
	sprintf(buf, "You say '%s'\r\n", argument + i);
	send_to_char(buf, ch);
	sprintf(buf, "$n says '%s'", argument + i);
	act(buf, FALSE, ch, 0, 0, TO_ROOM);
    }
}
#endif	    // UNUSED_CODE

/* NOTE: do_sayh() is renamed do_say() */
void do_say(struct char_data *ch, char *argument, int cmd)
{			/* hangul say */
    char buf[MAX_BUFSIZ], buf2[MAX_BUFSIZ];

    if ( is_dumb(ch)) return;

    argument = skip_spaces(argument);
    if (!*argument)
	send_to_char(STRHAN("Yes, but WHAT do you want to say?\r\n",
			 "예? 뭐라고 말해요 ?\r\n", ch), ch );
    else {
	/* English - Korean display act() , english text first.. */
	sprintf(buf,
	    STRHAN("You say '%s'\r\n", "'%s' 라고 말합니다\r\n", ch), argument);
	send_to_char(buf, ch);

	sprintf(buf, "$n says '%s'", argument);
	sprintf(buf2, "$n 님이 '%s' 라고 말합니다", argument);
	/* English - Korean display act() , english text first.. */
	acthan(buf, buf2, FALSE, ch, 0, 0, TO_ROOM);
    }
}

void do_chat(struct char_data *ch, char *argument, int cmd)
{
    struct descriptor_data *i;
    char buf[MAX_BUFSIZ], *cp;

    extern int nochatflag;

    if (IS_NPC(ch))
	return;
    /* NOTE: Did little code clean up */
    cp = skip_spaces(argument);
    if (is_dumb(ch) || ( nochatflag && IS_MORTAL(ch)))
	send_to_char("chat is forbiddened now.\r\n", ch);
    /* NOTE: Player forbidden shouting by wizard can't chat, too. */
    else if (IS_SET(ch->specials.act, PLR_SHUTUP))
	send_to_char("Shut up your mouth!!\r\n", ch);
    /* NOTE: "chat" with blank argument is same as "chat /last" */
    else if (*cp == '\0' || strncmp(cp, "/last", 5) == 0)
	do_lastchat(ch, argument, cmd);
    else {
	sprintf(buf, "%s > %s\r\n", GET_NAME(ch), argument);
	for (i = descriptor_list; i; i = i->next)
	    if ((i->connected == CON_PLYNG) && (!i->original)
		    && !IS_ACTPLR(i->character, PLR_NOCHAT))
		send_to_char(buf, i->character );

	chat_history(buf);
    }
}

/* 외치기 */
void do_shout(struct char_data *ch, char *argument, int cmd)
{
    char buf[MAX_BUFSIZ];
    struct descriptor_data *i;
    extern int noshoutflag;

    /*  NOTE:  NPC shoud not shout too laudly. Ex) Archmage's SUNFIRE!.  */
    if (IS_NPC(ch)) {
	sprintf(buf, "$n shouts '%s'", argument);
	act(buf, 0, ch, 0, 0, TO_ROOM);
	return;
    }

    argument = skip_spaces(argument);
    if (is_dumb(ch) || ( noshoutflag && IS_MORTAL(ch)))
	send_to_char("I guess you can't shout now?\r\n", ch);
    else if (IS_SET(ch->specials.act, PLR_SHUTUP))
	send_to_char("Shut up your mouth!!\r\n", ch);
    else if (!*argument)
	send_to_char("Shout? Yes! Fine! Shout we must, but WHAT??\r\n", ch);
    else {
	/* send_to_char("Ok.\r\n", ch);*/
	sprintf(buf, "%s shouts '%s'\r\n", GET_NAME(ch), argument);

	for (i = descriptor_list; i; i = i->next)
	    /* NOTE: I'd like to hear my own shout. */
	    if ( /* i->character != ch && */
		/* NOTE: Sleeping people can't hear shout. */
		!i->connected && i->character && AWAKE(ch)
		    && !IS_SET(i->character->specials.act, PLR_NOSHOUT))
		send_to_char(buf, i->character);

	// NOTE: exclude NPC shouting from chat history
	if (!IS_NPC(ch))
	    chat_history(buf) ;
    }
}

/* NOTE: Record history of chat and shout in ring buffer. */
/*	Excepted from do_chat() and added logging.	*/
void chat_history(char *str)
{
    /* assert(his_end >= 0 && his_end < LASTCHAT_SIZE); */
    // NOTE: add date time
    char stime[MAX_NAME_LEN], buf[MAX_LINE_LEN];
    time_t tt = time(0);
    strftime(stime, MAX_NAME_LEN-1, "%F %H:%M", localtime(&tt));
    sprintf(buf, "%s %s", stime, str);

    // NOTE: skip year and month part. They are for chatlog only.
    strcpy(history[his_end], buf+8); // or 11 to cut date

    his_end++;
    if ((his_end % LASTCHAT_SIZE) == (his_start % LASTCHAT_SIZE)) {
	his_end = his_start % LASTCHAT_SIZE;
	his_start = (his_start + 1) % LASTCHAT_SIZE;
    }

#ifdef	CHATLOG
    // NOTE: log chat and shout
    static FILE *chatlogfp = NULL;
    if (chatlogfp == NULL)
	chatlogfp = fopen(CHATLOG, "a");
    fputs(buf, chatlogfp);
    fflush(chatlogfp);
#endif

}

void do_lastchat(struct char_data *ch, char *argument, int cmd)
{
    int j;

    for (j = his_start; j != his_end; j = (j + 1) % LASTCHAT_SIZE) {
	/* assert(history[j]); */
	send_to_char(history[j], ch);
    }
    return;
}

/* 얘기하기 */
void do_tell(struct char_data *ch, char *argument, int cmd)
{
    struct char_data *vict;
    char *s, name[100], message[MAX_LINE_LEN], buf[MAX_OUT_LEN];

    if (is_dumb(ch)) return;
    half_chop(argument, name, message);
    if (!*name || !*message) {
	send_to_char("Who do you wish to tell what??\r\n", ch);
	return;
    }

    /* NOTE: "!" as listener name means telling to whom you told  or replied
       last time. This applies to telling to mob, too      */
    if (!strcmp(name, "!") && strlen(ch->specials.reply_who) > 1)
	strncpy(name, ch->specials.reply_who, 19);
    else
	strncpy(ch->specials.reply_who, name, 19);

    if (!(vict = get_char_vis(ch, name)))
	send_to_char(MSGSTR("No-one by that name here..\r\n", ch), ch);
    else if (ch == vict)
	send_to_char("You try to tell yourself something.\r\n", ch);

    else if (GET_POS(vict) == POS_SLEEPING && IS_MORTAL(ch)) {
	/* NOTE: Identify listener you tell even when it is sleeping.  */
	act("$N can't hear you.", FALSE, ch, 0, vict, TO_CHAR);
	/* NOTE: If victim player is sleeping, just notify.. */
	if (!IS_NPC(vict) && !IS_SET(vict->specials.act, PLR_NOTELL)) {
	    sprintf(buf, "%s has something to tell you. Wake up!\r\n",
		    GET_NAME(ch));
	    send_to_char(buf, vict);
	    strncpy(vict->specials.reply_who, GET_NAME(ch), 19);
	}
    }
    else if (!IS_ACTPLR(ch, PLR_NOTELL) ||
	     (IS_WIZARD(ch) && HIGHER_LEV(ch, vict))) {
	if (IS_NPC(ch)) {
	    s = ch->player.short_descr;
	    sprintf(buf, "%s tells you '%s'\r\n", s, message);
	}
	else {
	    s = CAN_SEE(vict, ch) ? GET_NAME(ch) : "Someone";
	    /* NOTE: Distinguish PC player tell from Shopkeeper tell   */
	    sprintf(buf, "%s tells YOU '%s'\r\n", s, message);
	}
	/* NOTE: If message starts with '!' char, beep bell on terminal */
	if (message[0] == '!' )
	    send_to_char("\a\a** BEEP ** " , vict);
	send_to_char(buf, vict);
	sprintf(vict->specials.reply_who, "%s", GET_NAME(ch));
	sprintf(buf, "You tell %s '%s'\r\n", GET_NAME(vict), message);
	send_to_char(buf, ch);
    }
    else
	act("$E isn't listening now.", FALSE, ch, 0, vict, TO_CHAR);
}

void do_reply(struct char_data *ch, char *argument, int cmd)
{
    char buf[MAX_BUFSIZ];

    sprintf(buf, "%s %s", ch->specials.reply_who, argument);
    do_tell(ch, buf, 0);
}

/* 보내기 */
void do_send(struct char_data *ch, char *argument, int cmd)
{
    struct char_data *vict;
    char *s, name[MAX_NAME_LEN], message[MAX_NAME_LEN], paint_name[MAX_LINE_LEN];
    char  buf[MAX_LINE_LEN], paint[MAX_STR_LEN];

    if (is_dumb(ch)) return;
    half_chop(argument, name, message);
    /* NOTE: Fix potential security hole. Prevent going up directory.	*/
    /*		Imagine command like "send cookie ../../../../etc/passwd"  */
    if (strstr( message, ".." ))
    	return;

    if (!*name || !*message)
	send_to_char("Who do you wish to tell what??\r\n", ch);
    else if (!(vict = get_char_vis(ch, name)))
	send_to_char("No-one by that name here..\r\n", ch);
    else {
	/* NOTE: get name of file to send from db.c module */
	sprintf(paint_name, "%s/%s", lookup_db("paints") , message);
	if (file_to_string(paint_name, paint) == NULL)
	    send_to_char("No such paint prepared.\r\n", ch);
	else if ((IS_SET(vict->specials.act, PLR_NOTELL))  &&
		(IS_MORTAL(ch) || !HIGHER_LEV (ch, vict)))
	    act("$E isn't listening now.", FALSE, ch, 0, vict, TO_CHAR);
	else {
	    send_to_char("\r\n", vict);
	    send_to_char(paint, vict);
	    sprintf(buf, "You send %s %s\r\n", GET_NAME(vict), message);
	    send_to_char(buf, ch);
	    if (IS_NPC(ch))
		s = ch->player.short_descr;
	    else
		s = CAN_SEE(vict, ch) ? GET_NAME(ch) : "Someone";
	    sprintf(buf, "%s sends you '%s'\r\n", s, message);
	    send_to_char(buf, vict);
	}
     }
}

/*  그룹 말하기 */
void do_gtell(struct char_data *ch, char *argument, int cmd)
{
    struct char_data *k;
    struct follow_type *f;
    char *s, buf[MAX_BUFSIZ];

    if (is_dumb(ch)) return;
    /* NOTE: Ungrouped solo player can 'gtell' to him/herself. */
    /*	This is to use 'gtell' in tintin "#action{<pattern>} {gtell <msg>}". */
    /* NOTE: if (!(k = ch->master)) */
    if (!(k = ch->master) || !IS_AFFECTED(ch, AFF_GROUP))
	k = ch;
    if (IS_NPC(ch))
	s = ch->player.short_descr;
    else
	s = GET_NAME(ch);
    sprintf(buf, "** %s ** '%s'\r\n", s, argument);
    /* NOTE: if (k && IS_AFFECTED(k, AFF_GROUP))  	*/
    if (k)
	send_to_char(buf, k);
    /* NOTE: make sure leader is grouped */
    if (k && IS_AFFECTED(k, AFF_GROUP))
	for (f = k->followers; f; f = f->next)
	    if (f->follower && IS_AFFECTED(f->follower, AFF_GROUP))
		send_to_char(buf, f->follower);
    /* send_to_char("Ok.\r\n", ch); */
}

/* NOTE: do_whisper() and do_ask() does essentially same function
	except action messages. Merge two function in do_whisper().  */
/* 속삭이기 */
void do_whisper(struct char_data *ch, char *argument, int cmd)
{
    struct char_data *vict;
    char name[100], message[MAX_LINE_LEN], buf[MAX_LINE_LEN];
    char **actstr ;

    static char  *ask_str[] = {
	"Who do you want to ask something.. and what??\r\n",
	"$n quietly asks $mself a question.",
	"You think about it for a while...\r\n",
	"$n asks you '%s'",
	"$n asks $N a question.",
    };
    static char  *whisper_str[] = {
	"Who do you want to whisper to.. and what??\r\n",
	"$n whispers quietly to $mself.",
	"You can't seem to get your mouth close enough to your ear...\r\n",
	"$n whispers to you, '%s'",
	"$n whispers something to $N.",
    };

    if (is_dumb(ch)) return;
    /* NOTE:  Select as do_whisper() or do_ask() */
    actstr = ( cmd == CMD_ASK ) ? ask_str : whisper_str ;

    half_chop(argument, name, message);

    if (!*name || !*message)
	send_to_char( actstr[0], ch);
    else if (!(vict = get_char_room_vis(ch, name)))
	send_to_char(MSGSTR("No-one by that name here..\r\n", ch), ch);
    else if (vict == ch) {
	act( actstr[1], FALSE, ch, 0, 0, TO_ROOM);
	send_to_char( actstr[2], ch);
    }
    else {
	sprintf(buf, actstr[3], message);
	act(buf, FALSE, ch, 0, vict, TO_VICT);
	/*    send_to_char("Ok.\r\n", ch); */
	act( actstr[4], FALSE, ch, 0, vict, TO_NOTVICT);
    }
}

#ifdef UNUSED_CODE
/* 물어보기 */
void do_ask(struct char_data *ch, char *argument, int cmd)
{
    struct char_data *vict;
    char name[100], message[MAX_STRING_LENGTH], buf[MAX_STRING_LENGTH];

    if (IS_SET(ch->specials.act, PLR_DUMB_BY_WIZ) && NOT_GOD(ch)) {
	return;
    }
    half_chop(argument, name, message);

    if (!*name || !*message)
	send_to_char("Who do you want to ask something.. and what??\r\n", ch);
    else if (!(vict = get_char_room_vis(ch, name)))
	send_to_char("No-one by that name here..\r\n", ch);
    else if (vict == ch) {
	act("$n quietly asks $mself a question.", FALSE, ch, 0, 0, TO_ROOM);
	send_to_char("You think about it for a while...\r\n", ch);
    }
    else {
	sprintf(buf, "$n asks you '%s'", message);
	act(buf, FALSE, ch, 0, vict, TO_VICT);
	/*    send_to_char("Ok.\r\n", ch); */
	act("$n asks $N a question.", FALSE, ch, 0, vict, TO_NOTVICT);
    }
}
#endif		/* UNUSED_CODE */

void do_emote(struct char_data *ch, char *argument, int cmd)
{
    static char buf[MAX_BUFSIZ];

    if (is_dumb(ch)) return;

    /* NOTE: Now, NPC mob can emote...  Use do_emote() in mob special proc.*/
    /* if (IS_NPC(ch)) return; */
    argument = skip_spaces(argument);
    if (!*argument)
	send_to_char("Yes.. But what?\r\n", ch);
    else {
	sprintf(buf, "$n %s", argument);
	act(buf, FALSE, ch, 0, 0, TO_ROOM);

	sprintf(buf, "You %s.\r\n", argument);
	send_to_char(buf, ch);
    }
}

void do_echo(struct char_data *ch, char *argument, int cmd)
{
    struct descriptor_data *i;
    char buf[MAX_BUFSIZ];

    if (IS_NPC(ch))
	return;

    argument = skip_spaces(argument);
    if (!*argument )
	send_to_char("That must be a mistake...\r\n", ch);
    else {
	sprintf(buf, "%s\r\n", argument);
	/* NOTE: God and Demi-God's echo will be heard by all players       */
	/* NOTE: Don't use send_to_except()   */
	if ( GET_LEVEL(ch) >= LEV_DEMI ) {
	    for (i = descriptor_list; i; i = i->next)
		if ( i->character && ch->desc != i && !i->connected)
		    send_to_char( argument, i->character );
	}
	else
	    send_to_room_except(buf, ch->in_room, ch);
	send_to_char("Ok.\r\n", ch);
    }
}

void do_wall(struct char_data *ch, char *argument, int cmd)
{
    char buf[MAX_BUFSIZ];

    argument = skip_spaces(argument);
    if (IS_NPCLEV(ch) || (!*argument))
	return;
    sprintf(buf, "%s\r\n", argument);
    send_to_all(buf);
    /* send_to_char("Ok.\r\n", ch); */
}

void do_wiznet(struct char_data *ch, char *argument, int cmd)
{
    struct descriptor_data *i;
    struct char_data *victim;
    char buf[MAX_BUFSIZ];

    if (IS_NPC(ch))
	return;
    sprintf(buf, "%s: %s\r\n", ch->player.name, argument);
    for (i = descriptor_list; i; i = i->next)
	if (!i->connected) {
	    if (i->original)
		continue;
	    victim = i->character;
	    if (IS_WIZARD(victim))
		send_to_char(buf, victim);
	}
    send_to_char("Ok.\r\n", ch);
}

#ifdef UNUSED_CODE
/* NOTE: do_nochat(), do_notell() are replaced by NEW do_set(). */
/*	Command 'nochat' etc. is now 'set chat <yes/no>'. etc.  */
void do_nochat(struct char_data *ch, char *argument, int cmd)
{
    if (IS_NPC(ch))
	return;

    if (IS_SET(ch->specials.act, PLR_NOCHAT)) {
	send_to_char("You can now hear chats again.\r\n", ch);
	REMOVE_BIT(ch->specials.act, PLR_NOCHAT);
    }
    else {
	send_to_char("From now on, you won't hear chats.\r\n", ch);
	SET_BIT(ch->specials.act, PLR_NOCHAT);
    }
}

void do_notell(struct char_data *ch, char *argument, int cmd)
{
    if (IS_NPC(ch))
	return;

    if (IS_SET(ch->specials.act, PLR_NOTELL)) {
	send_to_char("You can now hear tells again.\r\n", ch);
	REMOVE_BIT(ch->specials.act, PLR_NOTELL);
    }
    else {
	send_to_char("From now on, you won't hear tells.\r\n", ch);
	SET_BIT(ch->specials.act, PLR_NOTELL);
    }
}

#endif		/* UNUSED_CODE */

/* ************************************************************************
   *  OLD file: act.social.c ,   Usage : Social action commands.
   ************************************************************************* */

#define MAX_SOC_MESS 150

struct social_messg {
    /* int act_nr; */
    char *action_name;		/* NOTE: New! string of action command name. */
    int hide;
    int min_level;		/* NOTE: Minimum level of char */
    int min_char_position;	/* NOTE: New! Min position of char */
    int min_victim_position;	/* Position of victim */

    /* No argument was supplied */
    char *char_no_arg;
    char *others_no_arg;

    /* An argument was there, and a victim was found */
    char *char_found;	/* if NULL, read no further, ignore args */
    char *others_found;
    char *vict_found;

    /* An argument was there, but no victim was found */
    char *not_found;

    /* The victim turned out to be the character */
    char *char_auto;
    char *others_auto;
};

/* NOTE:  soc_mess_list is now table of pointers to struct social_messg  */
struct social_messg *soc_mess_list[MAX_SOC_MESS];
static int list_top  = 0 ;

/* NOTE: cmd arg has index to messge table, not regular command number. */
void do_action(struct char_data *ch, char *argument, int cmd)
{
    char buf[MAX_LINE_LEN];
    struct social_messg *action;
    struct char_data *vict;

/*
    if ((act_nr = find_action(cmd)) < 0) {
	send_to_char("That action is not supported.\r\n", ch);
	return;
    }
*/
    /* NOTE : soc_mess[] table was searched in command_interpreter(). */
    if ( cmd <= 0 || cmd >= MAX_SOC_MESS ) {
	send_to_char("That action is not supported.\r\n", ch);
	return;
    }
    action = soc_mess_list[cmd];

    if (action->char_found)
	one_argument(argument, buf);
    else
	*buf = '\0';

    if (!*buf) {
	send_to_char(action->char_no_arg, ch);
	send_to_char("\r\n", ch);
	act(action->others_no_arg, action->hide, ch, 0, 0, TO_ROOM);
	return;
    }

    if (!(vict = get_char_room_vis(ch, buf))) {
	send_to_char(action->not_found, ch);
	send_to_char("\r\n", ch);
    }
    else if (vict == ch) {
	send_to_char(action->char_auto, ch);
	send_to_char("\r\n", ch);
	act(action->others_auto, action->hide, ch, 0, 0, TO_ROOM);
    }
    else {
	/* NOTE: ch's level and position is check in command_interpreter() */
	if (GET_POS(vict) < action->min_victim_position) {
	    act("$N is not in a proper position for that.",
		FALSE, ch, 0, vict, TO_CHAR);
	}
	else {
	    act(action->char_found, 0, ch, 0, vict, TO_CHAR);
	    act(action->others_found, action->hide, ch, 0, vict, TO_NOTVICT);
	    act(action->vict_found, action->hide, ch, 0, vict, TO_VICT);
	}
    }
}

/*
int find_action(int cmd)
{
    int bot, top, mid;

    bot = 0;
    top = list_top;

    if (top < 0)
	return (-1);

    for (;;) {
	mid = (bot + top) / 2;

	if (soc_mess_list[mid].act_nr == cmd)
	    return (mid);
	if (bot >= top)
	    return (-1);

	if (soc_mess_list[mid].act_nr > cmd)
	    top = --mid;
	else
	    bot = ++mid;
    }
}
*/

/* NOTE: Used in command_interpreter. Searches for action_name.
	If the action  is found, returns index (> 0) to soc_mess table
	and will set value of min_level min_pos. 	 */
/* NOTE: pmin_lev, pmin_pos are not iput args, it is output args  */
int find_action(char *action_name, int *pmin_lev, int *pmin_pos )
{
    int i, len;
    if (list_top <= 0)
	return (-1);

    len = strlen(action_name);
    if (len == 0 ) len = 1;

    /* NOTE: sequencial search. inexact match to name string */
    for (i = 1; i <= list_top; i++)
	if (!strncmp( action_name, (soc_mess_list[i])->action_name, len )){
	    /* NOTE: To check ch's min level and position */
	    if ( pmin_lev )
		*pmin_lev = soc_mess_list[i]->min_level;
	    if ( pmin_pos )
		*pmin_pos = soc_mess_list[i]->min_char_position;
	    return (i);
	}
    return (-1);
}

char *fread_action(FILE * fl)
{
    char buf[MAX_LINE_LEN], *rslt;

    for (;;) {
	if ( fgets(buf, MAX_LINE_LEN-1, fl) == NULL ) {
	    perror("Fread_action - unexpected EOF.");
	    exit(2);
	}

	if (*buf == '#')
	    return (0);
	else {
	    *(buf + strlen(buf) - 1) = '\0';
	    CREATE(rslt, char, strlen(buf) + 1);
	    strcpy(rslt, buf);
	    return (rslt);
	}
    }
}

void boot_social_messages(void)
{
    FILE *fl ;
    int hide, min_lev, min_ch_pos, min_vic_pos;
    char action_name[MAX_LINE_LEN];
    struct social_messg *action;

    /* NOTE: use lookup_db() to get socail message file name.(SOCMESS_FILE) */
    if (!(fl = fopen(lookup_db("social"), "r"))) {
	perror("boot_social_messages");
	exit(2);
    }

    for (;;) {
	fscanf(fl, " %s ", action_name);
	if (!strcmp(action_name, "END"))
	    break;
	fscanf(fl, " %d %d ", &hide, &min_lev );
	fscanf(fl, " %d %d \n", &min_ch_pos, &min_vic_pos);
	ASSERT(action_name[0]);

	CREATE( action, struct social_messg, 1 );
	if ( !action || ++list_top >= MAX_SOC_MESS ) {
	    perror("boot_social_messages. over/alloc");
	    exit(1);
	}
	/* NOTE: soc_mess_list index starts from 1, not zero */
	soc_mess_list[list_top] = action;

	/* read the stuff */
	action->hide = hide;
	action->min_level = min_lev;
	action->min_char_position = min_ch_pos;
	action->min_victim_position = min_vic_pos;

	action->action_name = strdup(action_name);

	action->char_no_arg = fread_action(fl);
	action->others_no_arg = fread_action(fl);
	action->char_found = fread_action(fl);

	/* if no char_found, the rest is to be ignored */
	if (!action->char_found)
	    continue;

	action->others_found = fread_action(fl);
	action->vict_found = fread_action(fl);
	action->not_found = fread_action(fl);
	action->char_auto = fread_action(fl);
	action->others_auto = fread_action(fl);

    }
    fclose(fl);
}

void do_insult(struct char_data *ch, char *argument, int cmd)
{
    char buf[MAX_BUFSIZ], *msg;
    struct char_data *victim;
    static char *insult_msg[] = {
	"$n accuses you of fighting like a woman!",
	"$n says that women can't fight.",
	"$n accuses you of having the smallest.... (brain?)",
	"$n tells you that you'd loose a beautycontest against a troll.",
	/* NOTE: more insultive messages */
	"$n scoffs that you are coward with chicken heart and rat's eys.",
	"$n says that he will rape your dirty pussy hole with his big dick.",
	"$n threatens you she will chop off your pennis and feed it to fido.",
	"$n shouts you are bitch and have fucked everyone in this mud.",
    };

    one_argument(argument, buf);

    if (!*buf) {
	send_to_char("Sure you don't want to insult everybody.\r\n", ch);
	return;
    }
    if (!(victim = get_char_room_vis(ch, buf))) {
	send_to_char("Can't hear you!\r\n", ch);
	return;
    }
    if (victim == ch) {
	send_to_char("You feel insulted.\r\n", ch);
	return;
    }
    /* ch != victim */
    sprintf(buf, "You insult %s.\r\n", GET_NAME(victim));
    send_to_char(buf, ch);

    msg = *insult_msg ;
    /* NOTE: simplify msg selection code */
    switch ( random() % 4) {
    case 1:
	msg += 4 ; /* NOTE: more damaging insult */
	/* FALL THRU */
    case 0:
	if (GET_SEX(ch) == SEX_FEMALE )
	    msg += 2;
	if (GET_SEX(victim) == SEX_FEMALE)
	    msg ++ ;
	act( buf, FALSE, ch, 0, victim, TO_VICT);
	break;
    case 2:
	act("$n calls your mother a bitch!", FALSE, ch, 0, victim, TO_VICT);
	break;
    default:
	act("$n tells you to get lost!", FALSE, ch, 0, victim, TO_VICT);
	break;
    }	/* end switch */

    act("$n insults $N.", TRUE, ch, 0, victim, TO_NOTVICT);
}

/* ****************************************************************
   *  Public routines for system-to-player-communication        *
   **************************************************************** */

void send_to_char(char *messg, struct char_data *ch)
{
    if (ch->desc && messg && *messg)
	write_to_q(messg, &ch->desc->output);
}

void send_to_char_han(char *msgeng, char *msghan, struct char_data *ch)
{
    /* NOTE: if Korean message unavialable, send english message instead. */
    if (ch->desc) {
	if ( IS_ACTPLR(ch, PLR_KOREAN) && msghan && *msghan )	/* korean */
	    write_to_q(msghan, &ch->desc->output);
	else if (msgeng && *msgeng )	/* english */
	    write_to_q(msgeng, &ch->desc->output);
    }
}

void send_to_all(char *messg)
{
    struct descriptor_data *i;

    if (messg && *messg )
	for (i = descriptor_list; i; i = i->next)
	    if (!i->connected)
		write_to_q(messg, &i->output);
}

void send_to_outdoor(char *messg)
{
    struct descriptor_data *i;

    if (messg)
	for (i = descriptor_list; i; i = i->next)
	    if (!i->connected)
		if (OUTSIDE(i->character))
		    write_to_q(messg, &i->output);
}

/* NOTE: send_to_except() is not used, now. Comment out */
/*
void send_to_except(char *messg, struct char_data *ch)
{
    struct descriptor_data *i;

    if (messg)
	for (i = descriptor_list; i; i = i->next)
	    if (ch->desc != i && !i->connected)
		write_to_q(messg, &i->output);
}
*/

void send_to_room(char *messg, int room)
{
    struct char_data *i;

    if (messg && *messg)
	for (i = world[room].people; i; i = i->next_in_room)
	    if (i->desc)
		write_to_q(messg, &i->desc->output);
}

void send_to_room_except(char *messg, int room, struct char_data *ch)
{
    struct char_data *i;

    if (messg)
	for (i = world[room].people; i; i = i->next_in_room)
	    if (i != ch && i->desc)
		write_to_q(messg, &i->desc->output);
}

/* NOTE: send_to_except() is not used in anywhere. Comment out */
/*
void send_to_room_except_two
    (char *messg, int room, struct char_data *ch1, struct char_data *ch2) {
    struct char_data *i;

    if (messg)
	for (i = world[room].people; i; i = i->next_in_room)
	    if (i != ch1 && i != ch2 && i->desc)
		write_to_q(messg, &i->desc->output);
}
*/

/* NOTE: NEW!  Return hangul translated message for stock message. */
char * MSGSTR(char *msg, struct char_data *ch)
{
    int find;
    char buf[MAX_BUFSIZ];

    static char *catalog[] = { "", "",
	"No-one by that name here..\r\n", "그런 이름은 없는데요?\r\n",
	"\n", "\n",
    };

    if (!IS_ACTPLR(ch, PLR_KOREAN))
	return(msg);
    if ((find = search_block(msg, catalog, 2)) >= 0 ) /* pair-wise match */
	return(catalog[find+1]);
    sprintf(buf, "MSGSTR: %s", msg); /* No translation */
    log(buf);
    return(msg);
}
/* ============    MACROS from OLD structs.h for act()   =============== */

#define HSHR(ch) ((ch)->player.sex ?          \
  (((ch)->player.sex == 1) ? "his" : "her") : "its")

#define HSSH(ch) ((ch)->player.sex ?          \
  (((ch)->player.sex == 1) ? "he" : "she") : "it")

#define HMHR(ch) ((ch)->player.sex ?           \
  (((ch)->player.sex == 1) ? "him" : "her") : "it")

#define ANA(obj) (index("aeiouyAEIOUY", *(obj)->name) ? "An" : "A")

#define SANA(obj) (index("aeiouyAEIOUY", *(obj)->name) ? "an" : "a")

/* char name/short_desc(for mobs) or someone?  */
#define PERS(ch, vict)   ( CAN_SEE(vict, ch) ? \
    (!IS_NPC(ch) ? (ch)->player.name : (ch)->player.short_descr) : "someone" )

/*
   new $-code for backstab
 */
#define WEAP(obj, vict) ((obj == NULL) ? "finger" : OBJS(obj, vict))

#define OBJS(obj, vict) (CAN_SEE_OBJ((vict), (obj)) ? \
  (obj)->short_description  : "something")

#define OBJN(obj, vict) (CAN_SEE_OBJ((vict), (obj)) ? \
  fname((obj)->name) : "something")

/* higher-level communication */

void expand_actvar( char *buf, char *str, struct char_data *ch,
    struct obj_data *obj, void *vict_obj, struct char_data *to);

/* NOTE: act(), act_han() do same macro expansion. */
void act(char *str, int hide_invisible, struct char_data *ch,
	 struct obj_data *obj, void *vict_obj, int type)
{
    struct char_data *to;
    char buf[MAX_BUFSIZ];

    if (!str || !*str)
	return;

    if (type == TO_VICT)
	to = (struct char_data *) vict_obj;
    else if (type == TO_CHAR)
	to = ch;
    else
	to = world[ch->in_room].people;

    for (; to; to = to->next_in_room) {
	if (to->desc && ((to != ch) || (type == TO_CHAR)) &&
	    (CAN_SEE(to, ch) || !hide_invisible) && AWAKE(to) &&
	 !((type == TO_NOTVICT) && (to == (struct char_data *) vict_obj))) {
	    expand_actvar( buf, str, ch, obj, (void *) vict_obj, to);

	    CAP(buf);
	    write_to_q(buf, &to->desc->output);
	}
	if ((type == TO_VICT) || (type == TO_CHAR))
	    return;
    }
}

void acthan(char *streng, char *strhan, int hide_invisible,
	struct char_data *ch, struct obj_data *obj, void *vict_obj, int type)
{
    char *str ;
    struct char_data *to;
    char buf[MAX_BUFSIZ];

    if (!strhan || !*strhan) /* NOTE:  if no korean msg, use english */
	strhan = streng;
    if (!streng || !*streng )
	return;

    if (type == TO_VICT)
	to = (struct char_data *) vict_obj;
    else if (type == TO_CHAR)
	to = ch;
    else
	to = world[ch->in_room].people;

    for (; to; to = to->next_in_room) {
	if (to->desc && ((to != ch) || (type == TO_CHAR)) &&
	    (CAN_SEE(to, ch) || !hide_invisible) && AWAKE(to) &&
	 !((type == TO_NOTVICT) && (to == (struct char_data *) vict_obj))) {
	    str = IS_SET(to->specials.act, PLR_KOREAN) ? strhan : streng ;
	    expand_actvar(buf, str, ch, obj, (void *) vict_obj, to);

	    CAP(buf);
	    write_to_q(buf, &to->desc->output);
	}
	if ((type == TO_VICT) || (type == TO_CHAR))
	    return;
    }
}

/* NOTE: Type of 5th arg : From struct char_data * to void *   */
void expand_actvar(char *buf, char *str, struct char_data *ch,
	struct obj_data *obj, void *vict_obj, struct char_data *to)
{
    register char *point, *i;

    for( point = buf; ((point - buf) < MAX_BUFSIZ -3 ) && *str; str++ ) {
	if (*str != '$') {
	    *(point++) = *str ;
	    continue ;
	}
	switch (*(++str)) {
	case 'n':
	    i = PERS(ch, to); break;
	case 'N':
	    i = PERS((struct char_data *) vict_obj, to); break;
	case 'm':
	    i = HMHR(ch); break;
	case 'M':
	    i = HMHR((struct char_data *) vict_obj); break;
	case 's':
	    i = HSHR(ch); break;
	case 'S':
	    i = HSHR((struct char_data *) vict_obj); break;
	case 'e':
	    i = HSSH(ch); break;
	case 'E':
	    i = HSSH((struct char_data *) vict_obj); break;
	case 'o':
	    i = OBJN(obj, to); break;
	case 'O':
	    i = OBJN((struct obj_data *) vict_obj, to); break;
	case 'p':
	    i = OBJS(obj, to); break;
	case 'P':
	    i = OBJS((struct obj_data *) vict_obj, to); break;
	case 'a':
	    i = SANA(obj); break;
	case 'A':
	    i = SANA((struct obj_data *) vict_obj); break;
	case 'T':
	    i = (char *) vict_obj; break;
	case 'F':
	    i = fname((char *) vict_obj); break;
	    /* new $-code by atre */
	    /* for backstab by mobile if(no-weapon) WEAP = finger */
	case 'w':
	    i = WEAP(obj, to); break;
	case 'W':
	    i = WEAP((struct obj_data *) vict_obj, to); break;
	case '$':
	    i = "$"; break;
	default:
	    i = NULL;
	    log("Illegal $-code to act():");
	    log(str);
	    break;
	}
	/* NOTE: Check replacement is valid. (for DEBUGGING)   */
	if(!i){
	    log(str);
	    log("DEBUG: expand_actvar(): invalid expansion.");
	    break;
	}
	while ( *i && ((point - buf) < MAX_BUFSIZ-3 ))
	    *(point++) = *(i++);
    }

    *(point++) = '\n';
    *(point++) = '\r';
    *(point) = '\0';
}

#ifdef UNUSED_CODE
void acthan(char *streng, char *strhan, int hide_invisible,
	struct char_data *ch, struct obj_data *obj, void *vict_obj, int type)
{
    register char *strp, *str, *point, *i;
    struct char_data *to;
    char buf[MAX_STRING_LENGTH];

    if (!streng || !strhan)
	return;
    if (!*streng || !*strhan)
	return;

    if (type == TO_VICT)
	to = (struct char_data *) vict_obj;
    else if (type == TO_CHAR)
	to = ch;
    else
	to = world[ch->in_room].people;

    for (; to; to = to->next_in_room) {
	if (to->desc && ((to != ch) || (type == TO_CHAR)) &&
	    (CAN_SEE(to, ch) || !hide_invisible) && AWAKE(to) &&
	 !((type == TO_NOTVICT) && (to == (struct char_data *) vict_obj))) {
	    if ((IS_SET(to->specials.act, PLR_KOREAN))) {
		str = strhan;
	    }
	    else {
		str = streng;
	    }
	    for (strp = str, point = buf; (point - buf) <= MAX_STRING_LENGTH - 1;)
		if (*strp == '$') {
		    strp++;
		    switch (*strp) {
		    case 'n':
			i = PERS(ch, to);
			break;
		    case 'N':
			i = PERS((struct char_data *) vict_obj, to);
			break;
		    case 'm':
			i = HMHR(ch);
			break;
		    case 'M':
			i = HMHR((struct char_data *) vict_obj);
			break;
		    case 's':
			i = HSHR(ch);
			break;
		    case 'S':
			i = HSHR((struct char_data *) vict_obj);
			break;
		    case 'e':
			i = HSSH(ch);
			break;
		    case 'E':
			i = HSSH((struct char_data *) vict_obj);
			break;
		    case 'o':
			i = OBJN(obj, to);
			break;
		    case 'O':
			i = OBJN((struct obj_data *) vict_obj, to);
			break;
		    case 'p':
			i = OBJS(obj, to);
			break;
		    case 'P':
			i = OBJS((struct obj_data *) vict_obj, to);
			break;
		    case 'a':
			i = SANA(obj);
			break;
		    case 'A':
			i = SANA((struct obj_data *) vict_obj);
			break;
		    case 'T':
			i = (char *) vict_obj;
			break;
		    case 'F':
			i = fname((char *) vict_obj);
			break;
		    case '$':
			i = "$";
			break;
		    default:
			i = NULL;
			log("Illegal $-code to act():");
			log(str);
			break;
		    }
		    while (*i && ((point - buf) <= MAX_STRING_LENGTH - 1))
			*point++ = *i++;
		    ++strp;
		}
		else if (!(*(point++) = *(strp++)))
		    break;

	    *(--point) = '\n';
	    *(++point) = '\r';
	    *(++point) = '\0';

	    CAP(buf);
	    write_to_q(buf, &to->desc->output);
	}
	if ((type == TO_VICT) || (type == TO_CHAR))
	    return;
    }
}

#endif		/* UNUSED_CODE */

void show_string(struct descriptor_data *d, char *input)
{
    char buffer[MAX_STRING_LENGTH*2], buf[MAX_LINE_LEN];
    register char *scan, *chk;
    int lines = 0, toggle = 1;
    int pl;	/* NOTE: page length */

    one_argument(input, buf);
    if (*buf) {
	if (d->showstr_head) {
	    free(d->showstr_head);
	    d->showstr_head = 0;
	}
	d->showstr_point = 0;
	return;
    }

    /* NOTE: Reduced defaukt page length from 22 to 21 lines per page */
    /* NOTE: Now! varible page length set by user ('set pagelength 30') */
    pl = ( d->page_len < 10 || d->page_len > 50 ) ? 21 : d->page_len;

    /* show a chunk */
    for (scan = buffer;; scan++, d->showstr_point++)
	if ((((*scan = *d->showstr_point) == '\n') || (*scan == '\r')) &&
	    ((toggle = -toggle) < 0))
	    lines++;
	else if (!*scan || (lines >= pl)) {
	    *scan = '\0';
	    SEND_TO_Q(buffer, d);

	    /* see if this is the end (or near the end) of the string */
	    for (chk = d->showstr_point; isspace(*chk); chk++) ;
	    if (!*chk) {
		if (d->showstr_head) {
		    free(d->showstr_head);
		    d->showstr_head = 0;
		}
		d->showstr_point = 0;
	    }
	    return;
	}
}

void page_string(struct descriptor_data *d, char *str, int keep_internal)
{
    int line = 0;
    char *ps = str;

    if (!d)
	return;

    /* NOTE: If string  is short enough, don't paging. Just send out  */
    while (*ps)
	if (*ps++ == '\n')
	    line++;
    /* NOTE: Accomodate varibale page length */
    if (line < MAX(15, d->page_len - 5 )) {
	SEND_TO_Q(str, d);
	return;
    }

    if (keep_internal) {
	CREATE(d->showstr_head, char, strlen(str) + 1);

	strcpy(d->showstr_head, str);
	d->showstr_point = d->showstr_head;
    }
    else
	d->showstr_point = str;

    show_string(d, "");
}

