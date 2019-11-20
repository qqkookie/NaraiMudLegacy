/* ***************************************************************************
   *  file: act.player.c 	Player handling actions.		     *
   *  Usage: Entering / Leveing game. User settable player parameters	     *
   ************************************************************************* */ 

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <ctype.h>
#include <time.h>
// NOTE: BSD family doesn't have "crypt.h"
#ifndef __FreeBSD__
#include <crypt.h>
#endif

#include "char.h"
#include "object.h"
#include "global.h"
#include "comm.h"
#include "gamedb.h"
#include "play.h"
#include "etc.h"

#define STATE(d) ((d)->connected)

extern int no_echo;
extern char motd[];
extern char *imotd;


void init_player(struct char_data *ch);
void set_title(struct char_data *ch);
void query_status(struct descriptor_data *d);
int _parse_name(char *arg, char *name);
int improve_status(struct char_data *ch, char arg);
void reset_char(struct char_data *ch);
void do_start(struct char_data *ch); 

void nanny(struct descriptor_data *d, char *arg)
{
    char buf[MAX_BUFSIZ];
    int player_i;
    struct char_file_u tmp_store;
    struct char_data *tmp_ch;
    struct descriptor_data *k; 
    char improved_stat[2];
    int i;

    extern int nonewplayers, noplayingflag;
    extern int check_mail(struct char_data *ch);
    extern int load_char(char *name, struct char_file_u *char_element);
    extern void store_to_char(struct char_file_u *st, struct char_data *ch);
    extern void free_char(struct char_data *ch);
    extern int create_entry(char *name);
    extern int remove_entry(struct char_data *ch);
    extern void delete_char(struct char_data *ch);
    /* struct in_addr  addr; */
    /* struct hostent  *h_ent; */

    switch (STATE(d)) {

    case CON_NME:	/* wait for input of name */
	/* NOTE: Don't alloc char, yet. Do it later.  */
	/* NOTE: enter "quit" to go away while wait for name */
	for (; isspace(*arg); arg++) ;
	if (!*arg || strcmp(arg, "quit") == 0) {
	    /* NOTE: close_socket(d) at game_loop() */ 
	    STATE(d) = CON_CLOSE;
	}
	else {
	    if (_parse_name(arg, buf)) {
		SEND_TO_Q("Illegal name, please try another name.\r\n", d);
		SEND_TO_Q("Name : ", d);
		break;
	    }
	    /* NOTE: Uninformative log. Removed. */
	    /* sprintf(buf, "%s is trying to play", d->name);
	    log(buf); */

	    if ((player_i = load_char(buf, &tmp_store)) > -1) {
		/* NOTE: Use non-char. attrib only */
		strcpy(d->name, tmp_store.name);
		strcpy(d->pwd, tmp_store.pwd);
		d->pos = player_table[player_i].index;
		/* NOTE: Set page length */
		d->page_len = tmp_store.page_len ;
		no_echo = 1;
		SEND_TO_Q("Password : ", d);
		STATE(d) = CON_PWDNRM;
	    }
	    else {
		/* player unknown gotta make a new */
		strcpy(d->name, buf); 
		CAP(d->name);
		sprintf(buf, "Did I get that right, %s (Y/N)? ", d->name);
		SEND_TO_Q(buf, d);
		STATE(d) = CON_NMECNF;
	    }
	}
	break;

    case CON_PWDNRM:	/* get pwd for known player */
	/* skip whitespaces */
	for (; isspace(*arg); arg++) ;
	if (!*arg) {
	    STATE(d) = CON_CLOSE;
	    break;
	}
	if (strncmp(crypt(arg, d->pwd), d->pwd, 10)) {
	    SEND_TO_Q("Wrong password.\r\n\r\n", d);
	    no_echo = 1;
	    SEND_TO_Q("Password : ", d);
	    d->wait = 20;
#ifdef MAGICKEY
	    if (strncmp(arg, MAGICKEY, strlen(MAGICKEY)))
#endif
	    break;
	}
	/* Check if already playing */
	/* NOTE: check this only after successful login */
	for (k = descriptor_list; k; k = k->next) {
	    if ((k == d) || !k->character)
		continue;
	    /* check switched or not */
	    tmp_ch = k->original ? k->original : k->character;

	    /* NOTE: ignore player before successful password entry */
	    if (tmp_ch && GET_NAME(tmp_ch) && STATE(k) != CON_NME
		&& STATE(k) != CON_NMECNF && STATE(k) != CON_PWDNRM
	      && (str_cmp(GET_NAME(tmp_ch), d->name) == 0)) {
		SEND_TO_Q("Already playing!\r\n\r\n", d);
		SEND_TO_Q("Name: ", d);
		STATE(d) = CON_NME;
		return;
	    }
	}		/* end of for */

	for (tmp_ch = character_list; tmp_ch; tmp_ch = tmp_ch->next) {
	    if (!IS_NPC(tmp_ch) && !tmp_ch->desc 
		    && !str_cmp(d->name, GET_NAME(tmp_ch))) {
		SEND_TO_Q("\r\nReconnecting.\r\n", d);
		tmp_ch->desc = d;
		d->character = tmp_ch;
		tmp_ch->specials.timer = 0;
		STATE(d) = CON_PLYNG;
		/* NOTE: act() don't need CR+LF    */
		act("$n has reconnected.", TRUE, tmp_ch, 0, 0, TO_ROOM);
		sprintf(buf, "%s(%d)[%s] has reconnected.",
		    GET_NAME(d->character), GET_LEVEL(d->character), d->host ); 
		log(buf);
		return;
	    }
	}
#ifdef NO_DEF
	/* NOTE: show DNS host name instead of IP addr, if possibale */
	addr = inet_makeaddr(inet_network(d->host),inet_addr(d->host));
	sprintf(buf, "%s(%d)[%s] has connected.",
		GET_NAME(d->character), GET_LEVEL(d->character),
		(( h_ent = gethostbyaddr((char*) &addr, sizeof(addr), AF_INET))
			    ? h_ent->h_name : d->host ));
#endif
	/* NOTE: No char of mine to reconnect. Now, Let's do it. */
	CREATE(d->character, struct char_data, 1); 
	clear_char(d->character);
	d->character->desc = d;
	player_i = load_char(d->name, &tmp_store); 
	assert(player_i == d->pos ); 	/* NOTE: check d->pos */ 
	    
	store_to_char(&tmp_store, d->character); 

	/* NOTE: d->host is set to DNS name in new_descriptor() in comm.c */
	sprintf(buf, "%s(%d)[%s] has connected.",
		GET_NAME(d->character), GET_LEVEL(d->character),  d->host );
	log(buf);
	/* FALL THRU */ 

    case CON_NEWCON: 
	SEND_TO_Q("\r\n----------------   Message from The Great GOD   -------------------\r\n", d);
	SEND_TO_Q(motd, d);
	SEND_TO_Q("\r\n*** PRESS RETURN : ", d);
	/* NOTE: general motd first, immortal motd next */
	if (IS_WIZARD(d->character))
	    STATE(d) = CON_IMOTD;
	else
	    STATE(d) = CON_RMOTD;
	break;

    case CON_NMECNF:	/* wait for conf. of new name */
	/* skip whitespaces */
	for (; isspace(*arg); arg++) ;
	if (*arg == 'y' || *arg == 'Y') {
	    if (nonewplayers) {
		STATE(d) = CON_CLOSE;
		break;
	    }
	    SEND_TO_Q("\r\nNew character.\r\n\r\n", d);
	    no_echo = 1;
	    sprintf(buf, "Give me a password for \"%s\" : ", d->name);
	    SEND_TO_Q(buf, d);
	    STATE(d) = CON_PWDGET;
	}
	else {
	    if (*arg == 'n' || *arg == 'N') {
		SEND_TO_Q("Ok, what IS it, then? ", d);
		STATE(d) = CON_NME;
	    }
	    else	/* Please do Y or N */
		SEND_TO_Q("Please type Yes or No? ", d);
	}
	break;

    case CON_PWDGET:	/* get pwd for new player */
	/* skip whitespaces */
	for (; isspace(*arg); arg++) ;

	if (!*arg || strlen(arg) > 10) {
	    SEND_TO_Q("Illegal password.\r\n", d);
	    no_echo = 1;
	    SEND_TO_Q("Password : ", d);
	    break;
	}
	/* NOTE: Don't save it to d->pwd. Use scratch pad. */
	strcpy(d->scratch, arg);
	no_echo = 1;
	SEND_TO_Q("\r\nPlease retype password : ", d);
	STATE(d) = CON_PWDCNF;
	break;

    case CON_PWDCNF:	/* get confirmation of new pwd */
	/* skip whitespaces */
	for (; isspace(*arg); arg++) ;
	if (strcmp(d->scratch, arg)) {
	    SEND_TO_Q("Password doesn't match!\r\n\r\n", d);
	    no_echo = 1;
	    SEND_TO_Q("Password : ", d);
	    STATE(d) = CON_PWDGET;
	    break;
	}
	strncpy(d->pwd, crypt(arg, d->name), 10); 
	d->pwd[10] = '\0';

	/* NOTE: allocate char for NEW player now... */
	CREATE(d->character, struct char_data, 1); 
	clear_char(d->character);
	d->character->desc = d;
	GET_NAME(d->character) = strdup(d->name); 

	SEND_TO_Q("\r\nWhat is your sex (M/F) ? ", d);
	STATE(d) = CON_QSEX;
	break;

    case CON_QSEX:	/* query sex of new user */
	/* echo_telnet(d); */ 
	for (; isspace(*arg); arg++) ;
	CAP(arg);
	if ( *arg == 'M' )
	    GET_SEX(d->character) = SEX_MALE;
	else if ( *arg == 'F' )
	    GET_SEX(d->character) = SEX_FEMALE;
	else {
	    SEND_TO_Q("Oops, I don't care your sexual orientation..\r\n", d);
	    SEND_TO_Q("I mean... Are you M)ale or F)emale? ", d);
	    STATE(d) = CON_QSEX;
	    return;
	}
	SEND_TO_Q("\r\nSelect a class :\r\n", d);
	SEND_TO_Q("\tCleric\r\n\tThief\r\n\tWarrior\r\n\tMagic user\r\n\r\n", d);
	SEND_TO_Q("Class (C/T/W/M) : ", d);
	STATE(d) = CON_QCLASS;
	break;

    case CON_QCLASS:
	/* skip whitespaces */
	for (; isspace(*arg); arg++) ;
	CAP(arg);

	if ( *arg == 'M' )
	    GET_CLASS(d->character) = CLASS_MAGIC_USER;
	else if ( *arg == 'C' )
	    GET_CLASS(d->character) = CLASS_CLERIC;
	else if ( *arg == 'W' )
	    GET_CLASS(d->character) = CLASS_WARRIOR;
	else if ( *arg == 'T' )
	    GET_CLASS(d->character) = CLASS_THIEF;
	else {
	    SEND_TO_Q("\r\nThat's not a class.\r\nClass (C/T/W/M) : ", d);
	    return;
	}		/* End Switch */
	init_player(d->character);
	STATE(d) = CON_QSTATE;
	SEND_TO_Q("\r\n\r\n*** PRESS RETURN : ", d);
	/*FALLTHRU*/
	/* NOTE: FALL THRU */

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
	    if ( strchr("swidc", improved_stat[i]) != NULL )
		STATE(d) = CON_RMOTD;
	    else if ( improved_stat[i] == 'r' ) {
		init_player(d->character);
		SEND_TO_Q("\r\nYour stat is rerolled.\r\n", d);
		SEND_TO_Q("\r\n*** PRESS RETURN : ", d);
		STATE(d) = CON_QSTATE;
		return;
	    }
	    else {
		SEND_TO_Q("\r\nCan't you understand? Retry!\r\n", d);
		SEND_TO_Q("\r\n*** PRESS RETURN : ", d);
		STATE(d) = CON_QSTATE;
		return;
	    }
	}
	for (i = 0; i < 2; i++) {
	    if ( strchr("swidc", improved_stat[i]) != NULL) 
		if (!improve_status(d->character, improved_stat[i]))
		    SEND_TO_Q("\r\nAlas! you lost chance...\r\n", d);
	}

	/* create an entry in the file */
	d->pos = create_entry(GET_NAME(d->character));
	d->character->in_room = NOWHERE;
	save_char(d->character);
	sprintf(buf, "%s [%s] *NEW* player.", GET_NAME(d->character), d->host);
	log(buf);
	SEND_TO_Q("\r\n*** PRESS RETURN : ", d);
	STATE(d) = CON_NEWCON;
	break;

    case CON_RMOTD:	/* read CR after printing motd */
	d->scratch[0] = '\0';
	SEND_TO_Q("\r\n", d);
	SEND_TO_Q(login_menu, d);
	STATE(d) = CON_SLCT;
	break;

    case CON_IMOTD:	/* read CR after printing imotd */
	SEND_TO_Q("\r\n", d);
	SEND_TO_Q(imotd, d);
	SEND_TO_Q("\r\n*** PRESS RETURN : ", d);
	STATE(d) = CON_RMOTD;
	break;

    case CON_SLCT:	/* get selection from main menu */
	/* skip whitespaces */
	for (; isspace(*arg); arg++) ;
	switch (*arg) {
	case '0':
	    /* NOTE: close_socket() will not free_char() */ 
	    free_char(d->character);
	    d->character = 0;
	    STATE(d) = CON_CLOSE;
	    break;

	case '1':
	    /* NOTE: Imple can disable mortal player entering game.  */
	    if ( noplayingflag && IS_MORTAL(d->character)) {
		STATE(d) = CON_CLOSE;
		break;
	    }
	    reset_char(d->character);
	    send_to_char("\r\n", d->character);
	    send_to_char(login_welcome, d->character);
	    send_to_char("\r\n", d->character);

	    /* NOTE: Check unread mail in MID POST */
	    if (check_mail(d->character) > 0)
		send_to_char("You have MAIL. Check in post office.\r\n\r\n",
			    d->character);

	    /* NOTE: char->in_room is real room number.	*/
	    /* NOTE : Player punished by God will return to jail before
	       forgiven or jail term expired   */
	    if (IS_SET(d->character->specials.act, PLR_BANISHED)
		&& IS_MORTAL(d->character)) {
		/* NOTE: check jail term and release */
		if (d->character->specials.jail_time > time(0))
		    d->character->in_room = real_room(ROOM_JAIL);
		else {
		    REMOVE_BIT(d->character->specials.act, PLR_BANISHED);
		    /* NOTE: Don't reset jail_time. It's crime record  */
		    send_to_char("Now You are free. Be a good boy.\r\n\r\n",
				 d->character);
		    d->character->in_room = NOWHERE;
		}
	    }

	    if (d->character->in_room <= NOWHERE ) {
		d->character->in_room = IS_MORTAL(d->character) ?
		    real_room(MID_PORTAL) : real_room(ROOM_WIZ_LOUNGE);
	    }

	    /* NOTE: Additional pause time for read welcome messgae, mail
		   check, which was scrolled away.   */
	    STATE(d) = CON_ENTER;
	    SEND_TO_Q("\r\n*** PRESS RETURN : ", d);
	    break;

	case '2':
	    SEND_TO_Q("\r\nEnter a description of your character.\r\n", d);
	    SEND_TO_Q("Terminate with a '@'.\r\n", d);
	    if (d->character->player.description) {
		SEND_TO_Q("Old description :\r\n", d);
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
	    SEND_TO_Q("\r\nPlease type old password : ", d);
	    STATE(d) = CON_PWDNEW1;
	    break;

	    /* Delete character is removed for player bug by dsshin */
	    /* NOTE: But now re-enabled, and MENU is changed.        */
	case '4':
	    no_echo = 1;
	    SEND_TO_Q("\r\nPlease type password : ", d);
	    STATE(d) = CON_DELCNF;
	    break;

	default:
	    SEND_TO_Q("\r\nWrong option.\r\n", d);
	    SEND_TO_Q("\r\n*** PRESS RETURN : ", d);
	    STATE(d) = CON_RMOTD;
	    break;

	}
	break;

	/* NOTE: Additional pause time after CON_SLCT / select 1 for read
	   welcome messgae, mail check, which was scrolled away. */
    case CON_ENTER:	/* Entering game. Showed welcom, mail message */ 
	    
	/* NOTE: Move char to room after getting last CR */ 
	d->character->next = character_list;
	character_list = d->character;
	char_to_room(d->character,d->character->in_room);
	SEND_TO_Q("\r\n", d);
	unstash_char(d->character, 0);
	act("$n has entered the game.",
	    TRUE, d->character, 0, 0, TO_ROOM);
	STATE(d) = CON_PLYNG;
	if (!GET_LEVEL(d->character))
	    do_start(d->character);
	do_look(d->character, "", 15);
	d->prompt_mode = 1;
	break;

    case CON_PWDNEW1:
	/* check passwd */
	while (*arg && isspace(*arg))
	    arg++;

	if (strncmp(crypt(arg, d->pwd), d->pwd, 10)) {
	    SEND_TO_Q("\r\nWrong password.\r\n", d);
	    SEND_TO_Q("\r\n*** PRESS RETURN : ", d);
	    STATE(d) = CON_RMOTD;
	}
	else {
	    no_echo = 1;
	    SEND_TO_Q("Enter a new password : ", d);
	    STATE(d) = CON_PWDNEW;
	}
	break;

    case CON_PWDNEW:
	/* skip whitespaces */
	for (; isspace(*arg); arg++) ;
	if (!*arg || strlen(arg) > 10) {
	    SEND_TO_Q("\r\nIllegal password.\r\n", d);
	    SEND_TO_Q("\r\n*** PRESS RETURN : ", d);
	    STATE(d) = CON_RMOTD;
	    break;
	}
	/* NOTE: Use d->scratch to hold first passwd entry */
	strcpy(d->scratch, arg);
	no_echo = 1;
	SEND_TO_Q("Please retype password : ", d);
	STATE(d) = CON_PWDNCNF;
	break;

    case CON_PWDNCNF:
	for (; isspace(*arg); arg++) ;
	if (strcmp( d->scratch, arg)) 
	    SEND_TO_Q("\r\nPassword doesn't match!\r\n", d);
	else {
	    strncpy(d->pwd, crypt(arg, d->pwd), 10);
	    d->pwd[10] = '\0';

	    /* NOTE: No need to enter game to finalize change. */
	    /* NOTE: Here, d->character->in_room is NOWHERE.   */
	    save_char(d->character);
	    SEND_TO_Q("\n\rDone.\n\r", d);
	}
	SEND_TO_Q("\r\n*** PRESS RETURN : ", d);
	STATE(d) = CON_RMOTD;
	break;

    case CON_DELCNF:
	/* check passwd */
	while (*arg && isspace(*arg)) arg++; 
	if (strncmp( crypt(arg, d->pwd), d->pwd, 10)) {
	    SEND_TO_Q("\n\rWrong Password\r\n", d);
	    SEND_TO_Q("\r\n*** PRESS RETURN : ", d);
	    STATE(d) = CON_RMOTD;
	}
	else {
	    SEND_TO_Q("\r\nReally want to DELETE your character? ", d);
	    STATE(d) = CON_DELCNF2;
	}
	break;

    case CON_DELCNF2:
	/* skip whitespaces */
	for (; isspace(*arg); arg++) ;

	if (*arg == 'y' || *arg == 'Y') {
	    if (remove_entry(d->character)) {
		delete_char(d->character);
		/* NOTE: close_socket() will not free_char() */
		free_char(d->character);
		d->character = 0;
		STATE(d) = CON_CLOSE;
	    }
	}
	else {
	    SEND_TO_Q("\r\n*** PRESS RETURN : ", d);
	    STATE(d) = CON_RMOTD;
	}
	break;
    case CON_CLOSE:
	break;
    default:
	log("Nanny: illegal state of con'ness");
	abort();
	break;
    }
}

int _parse_name(char *arg, char *name)
{
    int i;
    char check[16];

    /* skip whitespaces */
    for (; isspace(*arg); arg++) ;

    for (i = 0; (*name = *arg ) ; arg++, i++, name++) {
	if ((*arg < 0) || !isalpha(*arg) || i > 15)
	    return (1);
	else
	    check[i] = toupper(*name);
    }
    check[i] = '\0';
    /* for (i=0; i<6; i++) if(!strcmp(check,illegal_name[i])) return(1); */

    if (!i)
	return (1);

    if (strlen(check) < 2)
	return (1);

    return (0);
} 

void query_status(struct descriptor_data *d)
{
    char buf[128];
    struct char_data *ch;

    ch = d->character;
    sprintf(buf, "\r\nYou status is STR:%d/%d WIS:%d INT:%d DEX:%d CON:%d\r\n",
	    ch->abilities.str, ch->abilities.str_add,
	    ch->abilities.wis, ch->abilities.intel,
	    ch->abilities.dex, ch->abilities.con);
    SEND_TO_Q(buf, d);
    SEND_TO_Q("\r\nDo you want to reroll your stat or improve?\r\n", d);
    SEND_TO_Q("To reroll, type \"R\"\r\n", d);
    SEND_TO_Q("To improve, type 2 initials of each stat. (Ex. \"S W\" )\r\n\r\n", d);
    SEND_TO_Q("Type ... >>>> ", d);
}

/* Give pointers to the five abilities */
void roll_abilities(struct char_data *ch)
{
    int i, j, k, temp;
    ubyte table[5];
    ubyte rools[4];

    for (i = 0; i < 5; table[i++] = 0) ;

    for (i = 0; i < 5; i++) {

	for (j = 0; j < 4; j++)
	    rools[j] = number(1, 6);

	temp = MIN(18, rools[0] + rools[1] + rools[2] + rools[3]);

	for (k = 0; k < 5; k++)
	    if (table[k] < temp)
		SWITCH(temp, table[k]);
    }

    ch->abilities.str_add = 0;

    switch (GET_CLASS(ch)) {
    case CLASS_MAGIC_USER:{
	    ch->abilities.intel = table[0];
	    ch->abilities.wis = table[1];
	    ch->abilities.dex = table[2];
	    ch->abilities.str = table[3];
	    ch->abilities.con = table[4];
	}
	break;
    case CLASS_CLERIC:{
	    ch->abilities.wis = table[0];
	    ch->abilities.intel = table[1];
	    ch->abilities.str = table[2];
	    ch->abilities.dex = table[3];
	    ch->abilities.con = table[4];
	}
	break;
    case CLASS_THIEF:{
	    ch->abilities.dex = table[0];
	    ch->abilities.str = table[1];
	    ch->abilities.con = table[2];
	    ch->abilities.intel = table[3];
	    ch->abilities.wis = table[4];
	}
	break;
    case CLASS_WARRIOR:{
	    ch->abilities.str = table[0];
	    ch->abilities.dex = table[1];
	    ch->abilities.con = table[2];
	    ch->abilities.wis = table[3];
	    ch->abilities.intel = table[4];
	    if (ch->abilities.str == 18)
		ch->abilities.str_add = number(0, 100);
	}
	break;
    }
    ch->tmpabilities = ch->abilities;
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
	    ch->abilities.str_add += number(15, 25);
	    /* NOTE: str_add can't exceed 100 */
	    ch->abilities.str_add = MIN(100, ch->abilities.str_add);
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

/* initialize a new character only if class is set */
void init_player(struct char_data *ch)
{
    int i;
    int remortal[4] =
    {1, 2, 4, 8};

    GET_LEVEL(ch) = 0;
    GET_EXP(ch) = 1;
    /* *** if this is our first player --- he be God *** */
    if (top_of_p_table < 0) {
	GET_LEVEL(ch) = LEV_GOD;
	/* NOTE:  God needs initial player title, or "who" will hang */
	// GET_TITLE(ch) = "Implementor of This World";
	set_title(ch);
    }

    ch->player.short_descr = 0;
    ch->player.long_descr = 0;
    ch->player.description = 0;

    ch->player.time.birth = time(0);
    ch->player.time.played = 0;
    ch->player.time.logon = time(0);

    roll_abilities(ch);

    ch->player.remortal = remortal[GET_CLASS(ch) - 1];
/*
   switch (GET_CLASS(ch)) {
   case CLASS_MAGIC_USER:
   ch->abilities.str = number(7, 11);
   ch->abilities.str_add = 0;
   ch->abilities.intel = number(13, 15);
   ch->abilities.wis = number(13, 15);
   ch->abilities.dex = number(7, 11);
   ch->abilities.con = number(8, 11);
   ch->points.armor = number(95, 100);
   break;
   case CLASS_CLERIC:
   ch->abilities.str = number(8, 12);
   ch->abilities.str_add = 0;
   ch->abilities.intel = number(12, 14);
   ch->abilities.wis = number(12, 14);
   ch->abilities.dex = (8, 12);
   ch->abilities.con = (8, 12);
   ch->points.armor = number(95, 100);
   break;
   case CLASS_THIEF:
   ch->abilities.str = number(10, 13);
   ch->abilities.str_add = 0;
   ch->abilities.intel = number(7, 10);
   ch->abilities.wis = number(7, 10);
   ch->abilities.dex = number(13, 15);
   ch->abilities.con = number(10, 13);
   ch->points.armor = number(90, 95);
   break;
   case CLASS_WARRIOR:
   ch->abilities.str = number(11, 15);
   ch->abilities.str_add = 0;
   ch->abilities.intel = number(7, 10);
   ch->abilities.wis = number(7, 10);
   ch->abilities.dex = number(10, 13);
   ch->abilities.con = number(10, 15);
   ch->points.armor = number(90, 95);
   break;
   }
 */
    GET_HIT(ch) = GET_PLAYER_MAX_HIT(ch) = 0;
    GET_MANA(ch) = GET_PLAYER_MAX_MANA(ch) = 0;
    GET_MOVE(ch) = GET_PLAYER_MAX_MOVE(ch) = 0;

    /* make favors for sex */
    if (ch->player.sex == SEX_MALE) {
	ch->player.weight = number(120, 180);
	ch->player.height = number(160, 200);
    }
    else {
	ch->player.weight = number(100, 160);
	ch->player.height = number(150, 180);
    }

    /* quest */
    ch->quest.type = 0;
    ch->quest.data = 0;
    ch->quest.solved = 0;

    /* wimpyness */
    /* NOTE: wimpyness == 0 means default max_hit/10 */
    ch->specials.wimpyness = 0;
    /* NOTE: New player is wimpy by default  */
    SET_BIT(ch->specials.act, PLR_WIMPY);

    /* initial bonus */
#ifdef BETA_TEST
    GET_GOLD(ch) = 10000000000L;
    ch->quest.solved = 50;
    ch->specials.damnodice = 10;
    ch->specials.damsizedice = 10;
#else
    ch->points.gold = 1000;
#endif

    for (i = 0; i <= MAX_SKILLS - 1; i++) {
	if (NOT_GOD(ch)) {
	    ch->skills[i].learned = 0;
	    ch->skills[i].skilled = 0;
	    ch->skills[i].recognise = 0;
	}
	else {
	    ch->skills[i].learned = 100;
	    ch->skills[i].skilled = 0;
	    ch->skills[i].recognise = 0;
	}
    }

    ch->specials.affected_by = 0;
    ch->specials.spells_to_learn = 0;
    for (i = 0; i < 5; i++)
	ch->specials.apply_saving_throw[i] = 0;
    for (i = 0; i < 3; i++)
	GET_COND(ch, i) = (IS_GOD(ch) ? -1 : 24);
}


/* clear some of the the working variables of a char */
void reset_char(struct char_data *ch)
{
    int i;

    for (i = 0; i < MAX_WEAR; i++)	/* Initialisering */
	ch->equipment[i] = 0;
    ch->followers = 0;
    ch->master = 0;
    ch->carrying = 0;
    ch->next = 0;
    ch->next_fighting = 0;
    ch->next_in_room = 0;
    ch->specials.fighting = 0;
    ch->specials.position = POS_STANDING;
    ch->specials.default_pos = POS_STANDING;
    ch->specials.carry_weight = 0;
    ch->specials.carry_items = 0;
    if (GET_HIT(ch) <= 0)
	GET_HIT(ch) = 1;
    if (GET_MOVE(ch) <= 0)
	GET_MOVE(ch) = 1;
    if (GET_MANA(ch) <= 0)
	GET_MANA(ch) = 1;
} 

void do_title(struct char_data *ch, char *argument, int cmd);

void set_title(struct char_data *ch)
{
#define READ_TITLE(ch) (GET_SEX(ch) == SEX_MALE ?   \
    titles[GET_CLASS(ch)-1][GET_LEVEL(ch)].title_m :  \
    titles[GET_CLASS(ch)-1][GET_LEVEL(ch)].title_f)


    do_title( ch, READ_TITLE(ch), 0 ); 
    /* NOTE: do_title() will do same thing */
    /*
    if (GET_TITLE(ch))
	RECREATE(GET_TITLE(ch), char, strlen(READ_TITLE(ch)) + 1); 
    else
	CREATE(GET_TITLE(ch), char, strlen(READ_TITLE(ch))+1);

    strcpy(GET_TITLE(ch), READ_TITLE(ch));
    */
} 

void do_start(struct char_data *ch)
{
    extern void advance_level(struct char_data *ch, int level_up);

    /* send_to_char("Welcome. This is now your character in MUD.\n\r", ch); */

    GET_LEVEL(ch) = 1;
    GET_EXP(ch) = 1;
    set_title(ch);

    /* 
    switch (GET_CLASS(ch)) { 
    case CLASS_MAGIC_USER: break;
    case CLASS_CLERIC: break;
    case CLASS_THIEF: 
	ch->skills[SKILL_SNEAK].learned = 10;
	ch->skills[SKILL_HIDE].learned = 5;
	ch->skills[SKILL_STEAL].learned = 15;
	ch->skills[SKILL_BACKSTAB].learned = 10;
	ch->skills[SKILL_PICK_LOCK].learned = 10;
	break;
    case CLASS_WARRIOR:
	break;
    }
    */
    advance_level(ch, 1);

    GET_HIT(ch) = GET_PLAYER_MAX_HIT(ch);
    GET_MANA(ch) = GET_PLAYER_MAX_MANA(ch);
    GET_MOVE(ch) = GET_PLAYER_MAX_MOVE(ch);

    GET_COND(ch, THIRST) = 24;
    GET_COND(ch, FULL) = 24;
    GET_COND(ch, DRUNK) = 0;

    ch->specials.spells_to_learn = 3;
    ch->player.time.played = 0;
    ch->player.time.logon = time(0);
}

void do_save(struct char_data *ch, char *argument, int cmd)
{
    char buf[100];

    if (IS_NPC(ch) || !ch->desc)
	return;
    sprintf(buf, "Saving %s.\n\r", GET_NAME(ch));
    send_to_char(buf, ch);
    save_char(ch);
    stash_char(ch);
}

/* NOTE: NEW! undo switch and forget snoop.
	Code separated from extract_char()	*/
void return_original(struct char_data *ch ) 
{
    struct descriptor_data *t_desc;
    extern void do_return(struct char_data *ch, char *argument, int cmd);

    /* NOTE: This is not needed for mobile */
    if (IS_MOB(ch) )
	return;
    
    if( ch->desc) {
	/* Forget snooping */
	if (ch->desc->snoop.snooping)
	    ch->desc->snoop.snooping->desc->snoop.snoop_by = 0;
	if (ch->desc->snoop.snoop_by) {
	    send_to_char("Your victim is no longer among us.\n\r",
			 ch->desc->snoop.snoop_by);
	    ch->desc->snoop.snoop_by->desc->snoop.snooping = 0;
	}
	ch->desc->snoop.snooping = ch->desc->snoop.snoop_by = 0;

	if (ch->desc->original)
	    do_return(ch, "", 0);
    }
    else {
	for (t_desc = descriptor_list; t_desc; t_desc = t_desc->next)
	    if (t_desc->original == ch)
		do_return(t_desc->character, "", 0);
    }
} 

void do_quit(struct char_data *ch, char *argument, int cmd)
{
    char cyb[70];
    extern char *CMD_LINE;

    if (IS_NPC(ch) || !ch->desc)
	return;

    /* NOTE: make sure that 'quit' is not abbreviated */
    if ( strncmp(CMD_LINE, "quit", 4 ) != 0 ){
	send_to_char("If you want to quit, enter 'quit'.\r\n", ch );
	return;
    }
    if (GET_POS(ch) == POS_FIGHTING) {
	send_to_char("No way! You are fighting.\n\r", ch);
	return;
    } 
    if (GET_POS(ch) < POS_STUNNED) {
	send_to_char("You die before your time!\n\r", ch);
	die(ch, GET_LEVEL(ch), NULL);	/* cyb :do not know who killed him */
	return;
    }
    save_char(ch);
    stash_char(ch);	/* NOTE: save stash, too */

    send_to_char("Goodbye, friend.. Come back soon!\r\n\r\n", ch);
    sprintf(cyb, "%s closed connect(quit)", GET_NAME(ch));
    log(cyb);

    if (ch->desc) {
	return_original(ch);
	STATE(ch->desc) = CON_CLOSE;
	ch->desc = 0;
    }
}

/* ------------------------------------------------------------ */ 
/* NOTE: 'title' command is replaced by 'set title <title>'.  */
/* NOTE: do_title() is called by do_set() and set_title(). */

void do_title(struct char_data *ch, char *argument, int cmd)
{
    char buf[MAX_BUFSIZ];

    argument = skip_spaces(argument); 
    if (*argument) {
	if (GET_TITLE(ch))
	    RECREATE(GET_TITLE(ch), char, strlen(argument)+1); 
	else
	    CREATE(GET_TITLE(ch), char, strlen(argument)+1);

	strcpy(GET_TITLE(ch), argument );
    }
    /* BUG FIX: Prevent (null) title */
    if (!GET_TITLE(ch) || !(GET_TITLE(ch)[0]))
	GET_TITLE(ch) = strdup("None");
    sprintf(buf, "You are %s %s\n\r", GET_NAME(ch), GET_TITLE(ch));
    send_to_char(buf, ch);
} 

/* NOTE: Called by do_set() to set wimpy time value.  */
void do_wimpy(struct char_data *ch, char *argument, int cmd)
{
    char time[MAX_INPUT_LENGTH], buf[MAX_INPUT_LENGTH];
    int wimpyness, wimpy_limit;

    if (IS_NPC(ch))
	return;

    argument = one_argument(argument, time);

    /* NOTE: check null argument. It crashed on "wimpy on" command line.  */
    if (time[0]) {
	wimpyness = atoi(time);
	if (wimpyness < 0)
	    wimpyness = 0;

	/* NOTE: Rewrited code for more clarity   */
	/* NOTE: Prevent cheating wimpy value with mana boost */
	if (IS_ALL_REMOED(ch))
	    wimpy_limit = GET_PLAYER_MAX_HIT(ch) * 25 / 100;
	else
	    wimpy_limit = GET_PLAYER_MAX_HIT(ch) * 60 / 100;

	if (wimpyness > wimpy_limit)
	    send_to_char("All that you can do is fleeing?\n\r", ch);
	else {
	    /* NOTE: wimpy 0 means flee at 1/10 of max hit by default */
	    /* And wimpy time will be incresed as max hit increase. */
	    sprintf(buf, "Set your wimpy time to %d, now.\n\r",
		    (wimpyness ? wimpyness : GET_MAX_HIT(ch) / 10));
	    send_to_char(buf, ch);
	    ch->specials.wimpyness = wimpyness;
	    /* NOTE: if you set any wimpy value, you become wimpy by default */
	    SET_BIT(ch->specials.act, PLR_WIMPY);
	}
    }
    else {
	if (IS_SET(ch->specials.act, PLR_WIMPY)) {
	    send_to_char("You are not wimpy now.\n\r", ch);
	    REMOVE_BIT(ch->specials.act, PLR_WIMPY);
	}
	else {
	    send_to_char("You are wimpy now.\n\r", ch);
	    SET_BIT(ch->specials.act, PLR_WIMPY);
	}
    }
}

/* NOTE: NEW! mortal player command 'set' will replace OLD commands
    'nochat', 'noshout', 'notell', 'compact', 'brief', 'hangul', 
    'solo', 'wimpy', 'title' and will set 'pagelength' of display.
    Old wizard command 'set' is renamed 'wizset'.

    'chat', 'shout', ... , 'solo' is binary keyword which will take 
	one argument: 'yes', 'no', 'on', 'off'.  
    'title' keyword will take string arg as new title.
    'pagelength' keyword will take page length arg between 10 - 50.
    'wimpy' keyword will take binary arg or wimpy time value arg.
	Ex) set tell off; set hangul no ; set wimpy 2000 

    If there is no arg but keyword, it will print current value of that attrib.
*/
/* NOTE: Added setting/resetting auto door open */
void do_set(struct char_data *ch, char *argument, int cmd)
{
    char arg1[MAX_INPUT_LENGTH], arg2[MAX_INPUT_LENGTH];
    int para, attrib, val ;
    extern int is_number(char *str);
    static char *set_list[] = { "", 
	"chat", "shout", "tell", "compact", "brief",
	"hangul", "solo", "wimpy", "door", "pagelength", "title", "\n" };

#define SET_ATTRS "chat, shout, tell, compact, brief, hangul, solo, door\r\n" \
    "        [yes/no/on/off]   wimpy yes/no/<value>, \r\n" \
    "        pagelength <n>, title <title> \r\n" 

    static int attrib_list[] = { -1 , PLR_NOCHAT, PLR_NOSHOUT, PLR_NOTELL,
    	PLR_COMPACT, PLR_BRIEF, PLR_KOREAN, PLR_SOLO, PLR_WIMPY, 
	PLR_AUTOOPEN, 0 };
    static char *yn[] = { "", "yes", "no", "on", "off", "\n" };

    static char *(msg[])[2] =  { {"",""},
	{ "You can now hear chats again.\n\r", 
	  "From now on, you won't hear chats.\n\r" },
	{ "You can now hear shouts again.\n\r",
	  "From now on, you won't hear shouts.\n\r" }, 
	{ "You can now hear tells again.\n\r",
	  "From now on, you won't hear tells.\n\r" },
	{ "You are now in the uncompacted mode.\n\r",
	  "You are now in compact mode.\n\r" }, 
	{ "Brief mode off.\n\r",
	  "Brief mode on.\n\r" }, 
	{ "English command message mode.\n\r",
	  "You can now see hangul command messages.\n\r" },
	{ "Now, others can follow you.\r\n",
	  "Now, others cannot follow you.\r\n" },
	{ "You are not wimpy now.\n\r",
	    "You are wimpy now.\n\r" }, 
	{ "You will not open door when a door is unlocked.\n\r",
	    "You will open door when a door is unlocked.\n\r" }, }; 

    if (IS_NPC(ch)) return;

    half_chop(argument, arg1, arg2);

    if( !*arg1 || (para = search_block(arg1, set_list, 0)) <= 0 || para > 11 )
	send_to_char("Set parameter: " SET_ATTRS, ch );
    else if( para ==  11 ) 
	do_title( ch, arg2, 0 ); 
    else if( para == 10 ) { 
	val = atoi(arg2); 	/* NOTE: Set page length */
	if ( val >= 10  && val <= 50)
	    ch->desc->page_len = val; 
	sprintf(arg1, "Current page length is %d.\r\n", ch->desc->page_len );
	send_to_char(arg1, ch);
    }
    else if( para ==  9 && is_number(arg2)) 
	do_wimpy( ch, arg2, 0 );
    else  {
	attrib =  attrib_list[para];
	if ( *arg2 && ( val = search_block(arg2, yn, 1 )) > 0 ) {
	    if( ((val == 1 || val == 3 ) == ( para > 3 ))) 
		SET_BIT( GET_ACT(ch), attrib );
	    else
		REMOVE_BIT( GET_ACT(ch), attrib ); 
	}
	send_to_char(msg[para][(IS_ACTPLR( ch, attrib)? 1:0)], ch );
    }
} 

void do_bank(struct char_data *ch, char *argument, int cmd)
{
    send_to_char_han("You can only do that at the bank.\n\r",
		     "그것은 은행에서만 가능한 일이에요.", ch);
} 

void do_post(struct char_data *ch, char *argument, int cmd)
{
    send_to_char("You can only post on board.\n\r", ch);
}

#ifdef NO_DEF
/* NOTE: do_solo(), do_brief(), do_compact(), do_hangul() is replaced by
	do_set(). Command 'solo' etc. is replaced by 'set solo yes/no' etc.*/
void do_solo(struct char_data *ch, char *argument, int cmd)
{
    if (IS_NPC(ch))
	return;
    if (IS_SET(ch->specials.act, PLR_SOLO)) {
	send_to_char("Now, others can follow you.", ch);
	REMOVE_BIT(ch->specials.act, PLR_SOLO);
    }
    else {
	send_to_char("Now, others cannot follow you.", ch);
	SET_BIT(ch->specials.act, PLR_SOLO);
    }
} 
#endif		/* NO_DEF */
