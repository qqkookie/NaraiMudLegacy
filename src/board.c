/* ************************************************************************
   *  file: board.c : read/write on board/letter.	Part of DIKUMUD *
   *  Usage: Run-time modification (by users) of game variables              *
   *  Copyright (C) 1990, 1991 - see 'license.doc' for complete information. *
   ************************************************************************ */


#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "char.h"
#include "object.h"
#include "global.h"
#include "comm.h"
#include "actions.h"


/************************************************************************
* board.c    * * bulletin board in mud 					*
*************************************************************************/

#define MAX_MSGS 100
#define WRITER_NAME_LEN 20
#define BOARD_DIR "boards"

struct board_data {
    char *head[MAX_MSGS];	/* head of board */
    char *msgs[MAX_MSGS];	/* msg of board */
    char writer[MAX_MSGS][WRITER_NAME_LEN];	/* writer of that mesg */
    int m_num;		/* msg number that board contains */
    FILE *fp;		/* file pointer */
    int room_num;	/* room number that board is in */
    char bfile[50];	/* board file name */
    struct board_data *next;	/* next */
};

struct board_data *board_list;

void load_board(struct board_data *cb);
int show_board(struct char_data *ch, struct board_data *cb, char *arg);
int post_board(struct char_data *ch, struct board_data *cb, char *arg);
int write_board(struct char_data *ch, struct board_data *cb, char *arg);
int remove_board(struct char_data *ch, struct board_data *cb, char *arg);
int read_board(struct char_data *ch, struct board_data *cb, char *arg);

#define MAX_NOTE_LENGTH MAX_STR_LEN	/* arbitrary */

/* 쓰기 */
void do_write(struct char_data *ch, char *argument, int cmd)
{
    struct obj_data *paper = 0;		/* , *pen = 0; */
    char papername[MAX_NAME_LEN], penname[MAX_NAME_LEN];
    char buf[MAX_LINE_LEN]; 

    argument_interpreter(argument, papername, penname);

    if (!ch->desc)
	return;

    if (!*papername) {	/* nothing was delivered */
	send_to_char(
	  "Write? with what? ON what? what are you trying to do??\n\r", ch);
	return;
    }
/* NOTE: Ignore pen, it has no useful meaning */
#ifdef USE_PEN
    if (*penname) {	/* there were two arguments */
	if (!(paper = get_obj_in_list_vis(ch, papername, ch->carrying))) {
	    sprintf(buf, "You have no %s.\n\r", papername);
	    send_to_char(buf, ch);
	    return;
	}
	if (!(pen = get_obj_in_list_vis(ch, penname, ch->carrying))) {
	    sprintf(buf, "You have no %s.\n\r", penname);
	    send_to_char(buf, ch);
	    return;
	}
    }
    else {		/* there was one arg.let's see what we can find */
#endif				/* USE_PEN */
	if (!(paper = get_obj_in_list_vis(ch, papername, ch->carrying))) {
	    sprintf(buf, "There is no %s in your inventory.\n\r", papername);
	    send_to_char(buf, ch);
	    return;
	}
/* NOTE: Ignore pen, it has no useful meaning */
#ifdef USE_PEN
	if (paper->obj_flags.type_flag == ITEM_PEN) {	/* oops, a pen.. */
	    pen = paper;
	    paper = 0;
	}
	else if (paper->obj_flags.type_flag != ITEM_NOTE) {
	    send_to_char("That thing has nothing to do with writing.\n\r", ch);
	    return;
	}

	/* one object was found. Now for the other one. */
	if (!ch->equipment[HOLD]) {
	    sprintf(buf, "You can't write with a %s alone.\n\r", papername);
	    send_to_char(buf, ch);
	    return;
	}
	if (!CAN_SEE_OBJ(ch, ch->equipment[HOLD])) {
	    send_to_char("The stuff in your hand is invisible! Yeech!!\n\r", ch);
	    return;
	}

	if (pen)
	    paper = ch->equipment[HOLD];
	else
	    pen = ch->equipment[HOLD];
    }

    /* ok.. now let's see what kind of stuff we've found */
    if (pen->obj_flags.type_flag != ITEM_PEN) {
	act("$p is no good for writing with.", FALSE, ch, pen, 0, TO_CHAR);
    }
#endif				/* USE_PEN */
    else if (paper->obj_flags.type_flag != ITEM_NOTE)
	act("You can't write on $p.", FALSE, ch, paper, 0, TO_CHAR);
    else if (paper->action_description)
	send_to_char("There's something written on it already.\n\r", ch);
    else {
	/* we can write - hooray! */

	send_to_char("Ok.. go ahead and write.. end the note with a @.\n\r",
		     ch);
	act("$n begins to jot down a note.", TRUE, ch, 0, 0, TO_ROOM);
	ch->desc->str = &paper->action_description;
	ch->desc->max_str = MAX_NOTE_LENGTH;
	ch->desc->scratch[0] = '\0';
    }
} 

/* init one board and return its pointer */
struct board_data *init_a_board(struct char_data *ch)
{
    struct board_data *cr_board;

    cr_board = (struct board_data *) malloc(sizeof(struct board_data));

    if (cr_board) {
	bzero(cr_board, sizeof(struct board_data));

	cr_board->room_num = world[ch->in_room].number;
	sprintf(cr_board->bfile, "%s/%dboard",
		BOARD_DIR, cr_board->room_num);
	cr_board->next = board_list;
	board_list = cr_board;
	load_board(cr_board);
    }
    return cr_board;
}

/* find board that is in the same room */
struct board_data *find_board(struct char_data *ch)
{
    struct board_data *tmp_board;
    int ch_rnum;

    ch_rnum = world[ch->in_room].number;
    if (board_list) {
	for (tmp_board = board_list; tmp_board; tmp_board = tmp_board->next) {
	    if (tmp_board->room_num == ch_rnum) {
		return tmp_board;
	    }
	}
    }
    tmp_board = init_a_board(ch);
    return tmp_board;
}

/* load board from disk. 
   this routine is called once when board is initialized. */
void load_board(struct board_data *cb)
{
    int ind, m_num, size;

    cb->fp = fopen(cb->bfile, "r+");
    if (!cb->fp) {
	cb->fp = fopen(cb->bfile, "w");
	fclose(cb->fp);
	cb->fp = fopen(cb->bfile, "r");
    }

    if (fread(&m_num, sizeof(int), 1, cb->fp) <= 0)
	    return;

    cb->m_num = m_num;

    for (ind = 0; ind < m_num; ind++) {
	fread(&size, sizeof(int), 1, cb->fp);

	cb->head[ind] = (char *) malloc(size + 1);
	if (!cb->head[ind]) {
	    log("malloc failure in board.c");
	    return;
	}
	fread(cb->head[ind], size, 1, cb->fp);
	cb->head[ind][size] = '\0';
	fread(cb->writer[ind], WRITER_NAME_LEN, 1, cb->fp);
	fread(&size, sizeof(int), 1, cb->fp);

	cb->msgs[ind] = (char *) malloc(size + 1);
	if (!cb->msgs[ind]) {
	    log("malloc failure in board.c");
	    return;
	}
	fread(cb->msgs[ind], size, 1, cb->fp);
	cb->msgs[ind][size] = '\0';
    }
    fclose(cb->fp);
    return;
}

/* save board on disk.
   this routine is called when message is posted on board 
   or message is removed from board.     */
void save_board(struct board_data *cb)
{
    int ind, m_num, size;

    if (cb->m_num == 0) {
	log("cb->m_num is infeasible (board.c save_board())");
	return;
    }

    cb->fp = fopen(cb->bfile, "w");
    if (!cb->fp) {
	log("unable to open board file.\n\r");
	return;
    }

    m_num = cb->m_num;
    fwrite(&m_num, sizeof(int), 1, cb->fp);

    for (ind = 0; ind < m_num; ind++) {
	size = strlen(cb->head[ind]);
	fwrite(&size, sizeof(int), 1, cb->fp);

	fwrite(cb->head[ind], size, 1, cb->fp);
	fwrite(cb->writer[ind], WRITER_NAME_LEN, 1, cb->fp);
	size = strlen(cb->msgs[ind]);
	fwrite(&size, sizeof(int), 1, cb->fp);

	fwrite(cb->msgs[ind], size, 1, cb->fp);
    }
    fclose(cb->fp);
}

/* board main routine */
int board(struct char_data *ch, int cmd, char *arg)
{
    struct board_data *cur_board;

    if (!ch->desc)
	return FALSE;
    /* if (cmd != 15 && cmd != 246 && cmd != 247 && cmd != 66 && cmd != 63) */
    if (cmd != CMD_LOOK && cmd !=  CMD_WRITE && cmd !=  CMD_POST 
	    && cmd != CMD_REMOVE && cmd != CMD_READ)
	return FALSE;

    cur_board = find_board(ch);
    if (!cur_board) {
	log("malloc failure in board.c");
	return FALSE;
    }

    switch (cmd) {
    case CMD_LOOK:
	return show_board(ch, cur_board, arg);
    case CMD_WRITE:
	return write_board(ch, cur_board, arg);
    case CMD_POST:
	return post_board(ch, cur_board, arg);
    case CMD_REMOVE:
	return remove_board(ch, cur_board, arg);
    case CMD_READ:
	return read_board(ch, cur_board, arg);
    default:
	log("cmd is infeasible! (board.c)");
	return FALSE;
    }
}

/* show message number, header, write to player.
   this is called when you look board    */
int show_board(struct char_data *ch, struct board_data *cb, char *arg)
{
    int i;
    char buf[MAX_STRING_LENGTH], tmp[256];

    one_argument(arg, tmp);
    if (!*tmp || !isname(tmp, "board bulletin"))
	return FALSE;

    act("$n studies the board.", TRUE, ch, 0, 0, TO_ROOM);
    strcpy(buf, "This is a bulletin board.\n\r");
    strcat(buf, "Usage: READ/REMOVE <messg #>, POST <paper> <header>\n\r");
    if (!cb->m_num)
	strcat(buf, "The board is empty.\n\r");
    else {
	sprintf(tmp, "There are %d messages on the board.\n\r", cb->m_num);
	strcat(buf, tmp);
	for (i = 0; i < cb->m_num; i++) {
	    /* NOTE: Align board display  */
	    sprintf(tmp, "%2d  %-8s :  %s \n\r",
		    i + 1, cb->writer[i], cb->head[i]);
	    strcat(buf, tmp);
	}
    }
    page_string(ch->desc, buf, 1);

    return TRUE;
}

/* routien that remove message from board.
   this is caleed when you type remove #     */
int remove_board(struct char_data *ch, struct board_data *cb, char *arg)
{
    int ind, msg;
    char number[MAX_INPUT_LENGTH];

    one_argument(arg, number);
    if (!*number || !(msg = atoi(number)))
	return FALSE;

    if (!cb->m_num) {
	send_to_char("The board is empty!\n\r", ch);
	return TRUE;
    }
    if (msg < 1 || msg > cb->m_num) {
	send_to_char("That message exists only in your imagination.\n\r", ch);
	return TRUE;
    }

    if (strcmp(cb->writer[msg - 1], GET_NAME(ch))) {
	if (IS_MORTAL(ch)) {
	    send_to_char("Only Immortal can remove other player's ", ch);
	    send_to_char("messages from board.\n\r", ch);
	    return TRUE;
	}
    }

    cb->m_num--;

    ind = msg - 1;
    free(cb->head[ind]);
    if (cb->msgs[ind])
	free(cb->msgs[ind]);
    for (; ind < cb->m_num; ind++) {
	cb->head[ind] = cb->head[ind + 1];
	cb->msgs[ind] = cb->msgs[ind + 1];
	strcpy(cb->writer[ind], cb->writer[ind + 1]);
    }

    send_to_char("Message is removed.\n\r", ch);
    save_board(cb);
    return TRUE;
}

/*  reading message from board routine.
   this will be called when you read #    */
int read_board(struct char_data *ch, struct board_data *cb, char *arg)
{
    char buffer[MAX_STRING_LENGTH], number[MAX_INPUT_LENGTH];
    int msg;

    one_argument(arg, number);
    if (!*number || !(msg = atoi(number)))
	return FALSE;

    if (!cb->m_num) {
	send_to_char("The board is empty!\n\r", ch);
	return TRUE;
    }
    if (msg < 1 || msg > cb->m_num) {
	send_to_char("That message exists only in your imagination.\n\r", ch);
	return TRUE;
    }

    sprintf(buffer, "message %d : %s by %s\n\r\n\r%s",
	    msg, cb->head[msg - 1], cb->writer[msg - 1], cb->msgs[msg - 1]);
    page_string(ch->desc, buffer, 1);
    return TRUE;
}

/*      write message routine
   this will be called when you write <header>                  */
int write_board(struct char_data *ch, struct board_data *cb, char *arg)
{
    struct obj_data *paper;
    char arg1[100], *arg2;

    if (!arg)
	return 0;

    arg2 = one_argument(arg, arg1);

    /* delete "board " */
    /* NOTE: delete it only if first arg is "board" */
    /* NOTE: If 1st arg is not "board" it is considered part of title */
    if (isname(arg1, "board bulletin"))
        arg =  arg2;

    /* no title */
    if (!*arg) {
	send_to_char("No title?#@\n\r", ch);
	return 1;
    }

    /* load paper */
    if (!(paper = read_object(3035, VIRTUAL))) {
	return 0;
    }

    /* give paper, pen to ch */
    obj_to_char(paper, ch);

    /* write paper with pen */
// #define MAX_NOTE_LENGTH MAX_STRING_LENGTH
    ch->desc->str = &paper->action_description;
    ch->desc->max_str = MAX_NOTE_LENGTH;
    sprintf(ch->desc->scratch, "paper %s", arg);

    return 1;
}

/*  posting message routine.
   this will be called when you post <paper> <header>   */
int post_board(struct char_data *ch, struct board_data *cb, char *arg)
{
    char papername[MAX_INPUT_LENGTH], header[MAX_INPUT_LENGTH];
    char buf[MAX_STRING_LENGTH];
    struct obj_data *paper;

    half_chop(arg, papername, header);
    if (*papername == 0 || *header == 0){
	/* NOTE: Complain it. Retrun TRUE */
	send_to_char("Post what? or what title?\r\n", ch);
	return TRUE;
    }
    paper = get_obj_in_list_vis(ch, papername, ch->carrying);

    if (!paper) {
	sprintf(buf, "You can't find %s in your inventory.\n\r", papername);
    }
    else if (paper->obj_flags.type_flag != ITEM_NOTE) {
	sprintf(buf, "You can't post %s on board.\n\r", papername);
    }
    else if (paper->action_description == 0) {
	sprintf(buf, "%s is empty.\n\r", papername);
    }
    else {
	if (cb->m_num == MAX_MSGS) {
	    send_to_char("Sorry...board is full.\n\r", ch);
	    return TRUE;
	}
	cb->head[cb->m_num] = (char *) malloc(strlen(header) + 1);
	if (cb->head[cb->m_num] == 0) {
	    sprintf(buf, "Your %s is fallen from board.\n\r", papername);
	    send_to_char(buf, ch);
	    return TRUE;
	}
	strcpy(cb->head[cb->m_num], header);
	strcpy(cb->writer[cb->m_num], GET_NAME(ch));
	cb->msgs[cb->m_num] = paper->action_description;
	paper->action_description = 0;
	cb->m_num++;
	extract_obj(paper);
	sprintf(buf, "Ok. You posted %s on board.\n\r", papername);
	/* send_to_char(buf, ch); */
	save_board(cb);
    }
    /* NOTE: give posting ok/error message about board */
    send_to_char(buf, ch);

    return TRUE;
}

#ifdef NO_DEF
/* NOTE: NEW! Wrapper proc of post_board() for string_add() in modify.c */
/* NOTE:        This proc don't take current board argument.    */
int post_board_char(struct char_data *ch, char *arg)
{
    struct board_data *cur_board;

    cur_board = find_board(ch);
    if (cur_board)
	return (post_board(ch, cur_board, arg));
    log("board.c:post_board_char(): can't find board.");
    return (FALSE);
}
#endif		/* NO_DEF */

/******************************************************************************
* mail.c ( mail routine of KIT Mud a.k.a. jinsil mud )                      *
* mail.c is coded by Kim Jiho ( Nara of Baram )                              *
******************************************************************************/
/* this is not so good code. you had better modify this one. */
/* NOTE: primary mail box in POST is loaded on boot time - Cookie  */

#define MAX_MAILS 200
/* NOTE: mbox files don't need separate directory. Share Boards directory */
/* #define MBOARD_DIR "mboxes" */
#define MBOARD_DIR "boards"

struct mbox_data {
    char *head[MAX_MAILS];	/* head of mbox */
    char *msgs[MAX_MAILS];	/* msg of mbox */
    char sender[MAX_MAILS][20];		/* sender of that mesg */
    char receiver[MAX_MAILS][20];	/* receiver of that mesg */
    int m_num;		/* msg number that mbox contains */
    FILE *fp;		/* file pointer */
    int room_num;	/* room number that mbox is in */
    char bfile[50];	/* mbox file name */
    struct mbox_data *next;	/* next */
};

struct mbox_data *mbox_list;
/*
extern struct room_data *world;

void page_string(struct descriptor_data *d, char *str, int keep);
int number(int from, int to);
*/


void init_mbox(void);
struct mbox_data *init_a_mbox(struct char_data *ch);
struct mbox_data *find_mbox(struct char_data *ch);
void load_mbox(struct mbox_data *cb);
void save_mbox(struct mbox_data *cb);
void mail_error_log(char *str);
void remove_mail(struct mbox_data *cb, int m_num);
int show_mail(struct char_data *ch, struct mbox_data *cb, char *arg);
int post_mail(struct char_data *ch, struct mbox_data *cb, char *arg);
int get_mail(struct char_data *ch, struct mbox_data *cb, char *arg);

/* NOTE: Check unread mail in POST mbox when entering game. */
int check_mail(struct char_data *ch);

#define MID_POST	3071	/* MID Post Office room number */

/* this function will be inserted into db.c */
/* init all mbox, and load primary mbox     */
void init_mbox(void)
{
    mbox_list = 0;
    /* init and load primary mbox in Post Office in MID(3071)  */
    if ((mbox_list = init_a_mbox((struct char_data *) 0)))
	log("POST mbox init ok.");
    else
	mail_error_log("POST mbox init failed.\r\n");
}

/* init one mbox and return its pointer */
struct mbox_data *init_a_mbox(struct char_data *ch)
{
    struct mbox_data *cr_mbox;

    cr_mbox = (struct mbox_data *) malloc(sizeof(struct mbox_data));

    if (cr_mbox) {
	bzero(cr_mbox, sizeof(*cr_mbox));
	/* NOTE: when ch is NULL, init primary mbox */
	if (ch)
	    cr_mbox->room_num = world[ch->in_room].number;
	else
	    cr_mbox->room_num = MID_POST;
	sprintf(cr_mbox->bfile, "%s/%dmbox",
		MBOARD_DIR, cr_mbox->room_num);
	cr_mbox->next = mbox_list;
	mbox_list = cr_mbox;
	load_mbox(cr_mbox);
    }
    return cr_mbox;
}

struct mbox_data *find_mbox(struct char_data *ch)
{
    struct mbox_data *tmp_mbox;
    int ch_rnum;

    ch_rnum = world[ch->in_room].number;
    if (mbox_list) {
	for (tmp_mbox = mbox_list; tmp_mbox; tmp_mbox = tmp_mbox->next) {
	    if (tmp_mbox->room_num == ch_rnum) {
		return tmp_mbox;
	    }
	}
    }
    tmp_mbox = init_a_mbox(ch);
    return tmp_mbox;
}

void load_mbox(struct mbox_data *cb)
{
    int ind, m_num, size;

    if ((cb->fp = fopen(cb->bfile, "r+")) == NULL) {
	/* NOTE: if mbox is vanished, create new one */
	mail_error_log("mbox file vanished. Will create new one.\n\r");
	save_mbox(cb);
	if ((cb->fp = fopen(cb->bfile, "w+")))
	    rewind(cb->fp);
	else
	    return;
    }

    fread(&m_num, sizeof(int), 1, cb->fp);

    cb->m_num = m_num;
    /* NOTE: m_num == 0 is valid state. i.e. empty mailbox */
    if (m_num < 0 || m_num > MAX_MAILS || feof(cb->fp)) {
	mail_error_log("mbox msg file corrupt or nonexistent.\n\r");
	cb->m_num = 0;
	fclose(cb->fp);
	return;
    }

    for (ind = 0; ind < m_num; ind++) {
	fread(&size, sizeof(int), 1, cb->fp);

	cb->head[ind] = (char *) malloc(size + 1);
	if (!cb->head[ind]) {
	    mail_error_log("mbox malloc failure.\n\r");
	    cb->m_num = m_num = ind;
	}
	fread(cb->head[ind], sizeof(char), size, cb->fp);

	cb->head[ind][size] = '\0';
	fread(cb->sender[ind], sizeof(char), 20, cb->fp);
	fread(cb->receiver[ind], sizeof(char), 20, cb->fp);
	fread(&size, sizeof(int), 1, cb->fp);

	cb->msgs[ind] = (char *) malloc(size + 1);
	if (!cb->msgs[ind]) {
	    mail_error_log("mbox malloc failure.\n\r");
	    cb->m_num = m_num = ind;
	}
	fread(cb->msgs[ind], sizeof(char), size, cb->fp);

	cb->msgs[ind][size] = '\0';
    }
    fclose(cb->fp);
    return;
}

void save_mbox(struct mbox_data *cb)
{
    int ind, m_num, size;

    /* NOTE: If no mail, write header only  */
    if (cb->m_num < 0 || cb->m_num > MAX_MAILS)
	return;
    cb->fp = fopen(cb->bfile, "r+");
    if (cb->fp == 0) {
	mail_error_log("unable to open mbox file.\n\r");
	return;
    }

    m_num = cb->m_num;
    fwrite(&m_num, sizeof(int), 1, cb->fp);

    for (ind = 0; ind < m_num; ind++) {
	size = strlen(cb->head[ind]) + 1;
	fwrite(&size, sizeof(int), 1, cb->fp);
	fwrite(cb->head[ind], sizeof(char), size, cb->fp);
	fwrite(cb->sender[ind], sizeof(char), 20, cb->fp);
	fwrite(cb->receiver[ind], sizeof(char), 20, cb->fp);

	size = strlen(cb->msgs[ind]) + 1;
	fwrite(&size, sizeof(int), 1, cb->fp);
	fwrite(cb->msgs[ind], sizeof(char), size, cb->fp);
    }
    fclose(cb->fp);
}


int mbox(struct char_data *ch, int cmd, char *arg)
{
    struct mbox_data *cur_mbox;

    if (!ch->desc)
	return (FALSE);
    if (cmd != CMD_LOOK && cmd != CMD_POST && cmd != CMD_GET )
	return (FALSE);

    cur_mbox = find_mbox(ch);
    if (cur_mbox == 0) {
	mail_error_log(" mbox malloc failure\n\r.");
	return (FALSE);
    }

    switch (cmd) {
    case CMD_LOOK:
	return (show_mail(ch, cur_mbox, arg));
    case CMD_POST:
	return (post_mail(ch, cur_mbox, arg));
    case CMD_GET:
	return (get_mail(ch, cur_mbox, arg));
    default:
	return (FALSE);
    }
}

int show_mail(struct char_data *ch, struct mbox_data *cb, char *arg)
{
    int i, pm_num = 0;
    char buf[MAX_STRING_LENGTH], tmp[MAX_INPUT_LENGTH];

    one_argument(arg, tmp);
    if (!*tmp || !isname(tmp, "mail mbox board"))
	return (FALSE);
    strcpy(buf, "This is a mail box.\n\r"
	   "Usage: GET letter mbox, POST <receiver> <paper> <header>\n\r");

    /* NOTE: Algned mail list form (left justified) */
    for (i = 0; i < cb->m_num; i++) {
	if (isname(GET_NAME(ch), cb->receiver[i]) || IS_GOD(ch)) {
	    sprintf(buf + strlen(buf), "    from %-8s to %-8s : %s\r\n",
		    cb->sender[i], cb->receiver[i], cb->head[i]);
	    pm_num++;
	}
    }
    if (!pm_num)
	strcat(buf, "The mbox is empty.\n\r");
    page_string(ch->desc, buf, 0);

    return (TRUE);
}

void remove_mail(struct mbox_data *cb, int m_num)
{
    int ind;

    ind = m_num;
    cb->m_num--;
    free(cb->head[ind]);
    if (cb->msgs[ind])
	free(cb->msgs[ind]);
    for (; ind < cb->m_num; ind++) {
	cb->head[ind] = cb->head[ind + 1];
	cb->msgs[ind] = cb->msgs[ind + 1];
	strcpy(cb->sender[ind], cb->sender[ind + 1]);
	strcpy(cb->receiver[ind], cb->receiver[ind + 1]);
    }

    save_mbox(cb);
}

int get_mail(struct char_data *ch, struct mbox_data *cb, char *arg)
{
    char buffer[MAX_STRING_LENGTH], lt[MAX_INPUT_LENGTH], mb[MAX_INPUT_LENGTH];
    int pm_num = 0, i, paper_num = 3035;
    struct obj_data *letter;

    argument_interpreter(arg, lt, mb);
    if (!*arg || !isname(mb, "mail mbox board") ||
	!isname(lt, "letter mail"))
	return (FALSE);

    for (i = 0; i < cb->m_num; i++) {
	if (isname(GET_NAME(ch), cb->receiver[i])) {
	    sprintf(buffer, "letter titled [%s] from %s\n\r\n\r%s",
		    cb->head[i], cb->sender[i], cb->msgs[i]);
	    pm_num++;
	    if ((paper_num = real_object(paper_num)) < 0) {
		mail_error_log("no such paper item");
		return (FALSE);
	    }
	    remove_mail(cb, i);
	    letter = read_object(paper_num, REAL);
	    obj_to_char(letter, ch);
	    // NOTE: BUG FIX!
	    CREATE(letter->action_description, char, strlen(buffer)+1);
	    strcpy(letter->action_description, buffer);
	    send_to_char("You got letter from mail box.\n\r", ch);
	    return (TRUE);
	}
    }

    send_to_char("Your mail box is empty!\n\r", ch);
    return (TRUE);
}

int post_mail(struct char_data *ch, struct mbox_data *cb, char *arg)
{
    char papername[MAX_INPUT_LENGTH], header[MAX_INPUT_LENGTH],
	buf[MAX_STRING_LENGTH], receiver[MAX_INPUT_LENGTH];
    struct obj_data *paper;

    half_chop(arg, receiver, buf);
    half_chop(buf, papername, header);
    if (*papername == 0 || *header == 0 || *receiver == 0)
	return (FALSE);
    paper = get_obj_in_list_vis(ch, papername, ch->carrying);

    if (!paper) {
	sprintf(buf, "You can't find %s in your inventory.\n\r", papername);
    }
    else if (paper->obj_flags.type_flag != ITEM_NOTE) {
	sprintf(buf, "You can't post %s on mbox.\n\r", papername);
    }
    else if (paper->action_description == 0) {
	sprintf(buf, "%s is empty.\n\r", papername);
    }
    else {
	sprintf(buf, "Ok. You posted %s on mbox.\n\r", papername);
	cb->head[cb->m_num] = (char *) malloc(strlen(header) + 1);
	if (cb->head[cb->m_num] == 0) {
	    sprintf(buf, "Your %s is fallen from mbox.\n\r", papername);
	    send_to_char(buf, ch);
	    return (TRUE);
	}
	strcpy(cb->head[cb->m_num], header);
	strcpy(cb->sender[cb->m_num], GET_NAME(ch));
	strcpy(cb->receiver[cb->m_num], receiver);
	cb->msgs[cb->m_num] = paper->action_description;
	paper->action_description = 0;
	cb->m_num++;
	extract_obj(paper);
    }

    send_to_char(buf, ch);
    save_mbox(cb);
    return (TRUE);
}

void mail_error_log(char *str)
{
    fputs("mbox : ", stderr);
    fputs(str, stderr);
    return;
}

/* NOTE: check unread mail in POST mbox sent to player chacacter */
/* Returns number of received mail. You can check in out of post office */
int check_mail(struct char_data *ch)
{
    int i, pm_num = 0;

    if (!ch || IS_NPC(ch) || !mbox_list)
	return (0);
    /* *m_list  is primary mbox in mid post office      */
    for (i = 0; i < mbox_list->m_num; i++)
	if (isname(GET_NAME(ch), mbox_list->receiver[i]))
	    pm_num++;

    return (pm_num);
}

/************************************************************************
* OLD modify.c    * Modifying string value of player/mobile/item	*
*************************************************************************/

#define TP_MOB    0
#define TP_OBJ     1
#define TP_ERROR  2 

/* NOTE: field = 1 for name , 2 for short ... */
char *string_fields[] = {   
    "",
    "name",
    "short",
    "long",
    "description",
    "title",
    "delete-description",
    "\n"
};

/* NOTE: Max length of field 1 is length[1]...  */
/* maximum length for text field */
int length[] = {
    0,
    40,
    60,
    256,
    240,
    60
};


/* NOTE: skill_field[], max_value[] not used yet */
/*
char *skill_fields[] = {
    "learned",
    "affected",
    "duration",
    "recognize",
    "\n"
};

int max_value[] = {
    255,
    255,
    10000,
    1
}; 
*/

/* ************************************************************************
   *  modification of malloc'ed strings                                      *
   ************************************************************************ */

/* Add user input to the 'current' string (as defined by d->str) */
void string_add(struct descriptor_data *d, char *str)
{
    int terminator = 0;

    struct board_data *cur_board;

    /* determine if this is the terminal string, and truncate if so */
    if ((terminator = (*str == '@'))) {
	*str = '\0';
    }

    if (!(*d->str)) {
	if ((int)strlen(str) > d->max_str) {
	    send_to_char("String too long - Truncated.\n\r",
			 d->character);
	    *(str + d->max_str) = '\0';
	    terminator = 1;
	}
	CREATE(*d->str, char, strlen(str) + 3);

	strcpy(*d->str, str);
    }
    else {
	if ((int)(strlen(str) + strlen(*d->str)) > d->max_str) {
	    send_to_char("String too long. Last line skipped.\n\r",
			 d->character);
	    terminator = 1;
	}
	else {
	    if (!(*d->str = (char *) realloc(*d->str, strlen(*d->str) +
					     strlen(str) + 3))) {
		perror("string_add");
		exit(1);
	    }
	    strcat(*d->str, str);
	}
    }

    if (terminator) {
	d->str = 0;
	if (d->connected == CON_EXDSCR) {
	    SEND_TO_Q("\r\n*** PRESS RETURN : ", d);
	    /* NOTE: save modified player description immediately */
	    /* NOTE: Here, d->character->in_room is NOWHERE.      */
	    save_char(d->character);
	    d->connected = CON_RMOTD;
	}

	/* post paper */
	/* NOTE: scratch is used as 'title' of board message */
	if (!strncmp(d->scratch,"paper", 5)) { 
	    cur_board = find_board(d->character);
	    post_board(d->character, cur_board, d->scratch);
#ifdef NO_DEF
	    post_board_char(d->character, d->scratch);
#endif
	    d->scratch[0] = 0;
	}
    }
    else
	strcat(*d->str, "\n\r");
}


#undef MAX_STR

/* interpret an argument for do_string */
void quad_arg(char *arg, int *type, char *name, int *field, char *string)
{
    char buf[MAX_STRING_LENGTH];

    /* determine type */
    arg = one_argument(arg, buf);
    if (is_abbrev(buf, "char"))
	*type = TP_MOB;
    else if (is_abbrev(buf, "obj"))
	*type = TP_OBJ;
    else {
	*type = TP_ERROR;
	return;
    }

    /* find name */
    arg = one_argument(arg, name);

    /* field name and number */
    arg = one_argument(arg, buf);
    /* NOTE: Use search_block() instead of old_search_block() */
  /* if (!(*field = old_search_block(buf, 0, strlen(buf), string_fields, 0)))*/
    if ((*field = search_block(buf, string_fields, 0)) <= 0 )
	return;

    /* string */
    for (; isspace(*arg); arg++) ;
    for (; (*string = *arg); arg++, string++) ;

    return;
}


/* modification of malloc'ed strings in chars/objects */
void do_string(struct char_data *ch, char *arg, int cmd)
{
    char name[MAX_STRING_LENGTH], string[MAX_STRING_LENGTH];
    int field, type;
    struct char_data *mob;
    struct obj_data *obj;
    struct extra_descr_data *ed, *tmp;

    if (IS_NPC(ch))
	return;

    quad_arg(arg, &type, name, &field, string);

    if (type == TP_ERROR) {
	send_to_char(
	"Syntax: string ('obj'|'char') <name> <field> [<string>].\n\r", ch);
	return;
    }

    if (!field) {
	send_to_char("No field by that name. Try 'help string'.\n\r", ch);
	return;
    }

    if (type == TP_MOB) {
	/* locate the beast */
	if (!(mob = get_char_vis(ch, name))) {
	    send_to_char("I don't know anyone by that name...\n\r", ch);
	    return;
	}

	switch (field) {
	case 1:
	    if (!IS_NPC(mob) && NOT_GOD(ch)) {
		send_to_char("You can't change that field for players.", ch);
		return;
	    }
	    ch->desc->str = &GET_NAME(mob);
	    if (!IS_NPC(mob))
		send_to_char(
		 "WARNING: You have changed the name of a player.\n\r", ch);
	    break;
	case 2:
	    if (!IS_NPC(mob)) {
		send_to_char( "That field is for monsters only.\n\r", ch);
		return;
	    }
	    ch->desc->str = &mob->player.short_descr;
	    break;
	case 3:
	    if (!IS_NPC(mob)) {
		send_to_char( "That field is for monsters only.\n\r", ch);
		return;
	    }
	    ch->desc->str = &mob->player.long_descr;
	    break;
	case 4:
	    ch->desc->str = &mob->player.description;
	    break;
	case 5:
	    if (IS_NPC(mob)) { 
		send_to_char("Monsters have no titles.\n\r", ch);
		return;
	    }
	    ch->desc->str = &GET_TITLE(mob);
	    break;
	default:
	    send_to_char( "That field is undefined for monsters.\n\r", ch);
	    return;
	    break;
	}
    }
    else {		/* type == TP_OBJ */
	/* locate the object */
	if (!(obj = get_obj_vis(ch, name))) {
	    send_to_char("Can't find such a thing here..\n\r", ch);
	    return;
	}
	switch (field) {
	case 1:
	    ch->desc->str = &obj->name;
	    break;
	case 2:
	    ch->desc->str = &obj->short_description;
	    break;
	case 3:
	    ch->desc->str = &obj->description;
	    break;
	case 4:
	    if (!*string) {
		send_to_char("You have to supply a keyword.\n\r", ch);
		return;
	    }
	    /* try to locate extra description */
	    for (ed = obj->ex_description;; ed = ed->next)
		if (!ed) {  /* the field was not found. create a new one.   */
		    CREATE(ed, struct extra_descr_data, 1);

		    ed->next = obj->ex_description;
		    obj->ex_description = ed;
		    CREATE(ed->keyword, char, strlen(string) + 1);

		    strcpy(ed->keyword, string);
		    ed->description = 0;
		    ch->desc->str = &ed->description;
		    send_to_char("New field.\n\r", ch);
		    break;
		}
		else if (!str_cmp(ed->keyword, string)) {
		    /* the field exists */
		    free(ed->description);
		    ed->description = 0;
		    ch->desc->str = &ed->description;
		    send_to_char(
				    "Modifying description.\n\r", ch);
		    break;
		}
	    ch->desc->max_str = MAX_STRING_LENGTH;
	    return;	/* the stndrd (see below) procedure does not apply
			   here */
	    break;
	case 6:
	    if (!*string) {
		send_to_char("You must supply a field name.\n\r", ch);
		return;
	    }
	    /* try to locate field */
	    for (ed = obj->ex_description;; ed = ed->next)
		if (!ed) {
		    send_to_char("No field with that keyword.\n\r", ch);
		    return;
		}
		else if (!str_cmp(ed->keyword, string)) {
		    free(ed->keyword);
		    if (ed->description)
			free(ed->description);

		    /* delete the entry in the desr list */
		    if (ed == obj->ex_description)
			obj->ex_description = ed->next;
		    else {
			for (tmp = obj->ex_description; tmp->next != ed;
			     tmp = tmp->next) ;
			tmp->next = ed->next;
		    }
		    free(ed);

		    send_to_char("Field deleted.\n\r", ch);
		    return;
		}
	    break;
	default:
	    send_to_char( "That field is undefined for objects.\n\r", ch);
	    return;
	    break;
	}
    }

    if (*ch->desc->str) {
	free(*ch->desc->str);
    }

    if (*string) {	/* there was a string in the argument array */
	/* NOTE: Max length of field 1 is length[1].. */
	if ((int) strlen(string) > length[field]) {
	    send_to_char("String too long - truncated.\n\r", ch);
	    *(string + length[field]) = '\0';
	}
	CREATE(*ch->desc->str, char, strlen(string) + 1);

	strcpy(*ch->desc->str, string);
	ch->desc->str = 0;
	send_to_char("Ok.\n\r", ch);
    }
    else {		/* there was no string. enter string mode */
	send_to_char("Enter string. terminate with '@'.\n\r", ch);
	*ch->desc->str = 0;
	ch->desc->max_str = length[field];
    }
}
