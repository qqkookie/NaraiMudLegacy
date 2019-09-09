/******************************************************************************
* board.c                                                                     *
* bulletin board in mud                                                       *
******************************************************************************/

#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <ctype.h>

#include "structs.h"
#include "utils.h"
#include "comm.h"
#include "db.h"

#define MAX_MSGS 100
#define WRITER_NAME_LEN 20
#define BOARD_DIR "boards"

struct board_data {
    char *head[MAX_MSGS];  /* head of board */
    char *msgs[MAX_MSGS];  /* msg of board */
    char writer[MAX_MSGS][WRITER_NAME_LEN]; /* writer of that mesg */
    int m_num;  /* msg number that board contains */
    FILE *fp;   /* file pointer */
    int room_num;  /* room number that board is in */
    char bfile[50];  /* board file name */
    struct board_data *next;  /* next */
};

struct board_data *board_list;
extern struct room_data *world;

char *one_argument(char *arg, char *first);
int isname(char *name, char *namelist);
void page_string(struct descriptor_data *d, char *str, int keep);
void half_chop(char *str, char *arg1, char *arg2);
void extract_obj(struct obj_data *o);


struct board_data *init_a_board(struct char_data *ch);
struct board_data *find_board(struct char_data *ch);
void load_board( struct board_data *cb );
void save_board( struct board_data *cb );
void log(char *str);
int show_board(struct char_data *ch, struct board_data *cb, char *arg);
int post_board(struct char_data *ch, struct board_data *cb, char *arg);
int remove_board(struct char_data *ch, struct board_data *cb, char *arg);
int write_board(struct char_data *ch, struct board_data *cb, char *arg);
int read_board(struct char_data *ch, struct board_data *cb, char *arg);

/* got this from mobact.c */
void obj_to_char(struct obj_data *o, struct char_data *ch);

/* init one board and return its pointer */
struct board_data *init_a_board(struct char_data *ch)
{
    struct board_data *cr_board;

    cr_board = (struct board_data *)malloc(sizeof(struct board_data));
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

    for(ind = 0; ind < m_num; ind++) {
        fread(&size, sizeof(int), 1, cb->fp);
        cb->head[ind] = (char *)malloc(size + 1);
        if (!cb->head[ind]) {
            log("malloc failure in board.c");
			return;
        }
        fread(cb->head[ind], size, 1, cb->fp);
        cb->head[ind][size] = NUL;
        fread(cb->writer[ind], WRITER_NAME_LEN, 1, cb->fp);
        fread(&size, sizeof(int), 1, cb->fp);
        cb->msgs[ind] = (char *)malloc(size + 1);
        if (!cb->msgs[ind]) {
            log("malloc failure in board.c");
			return;
        }
        fread(cb->msgs[ind], size, 1, cb->fp);
        cb->msgs[ind][size] = NUL;
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
    if (cmd != 15 && cmd != 246 && cmd != 247 && cmd != 66 && cmd != 63 )
		return FALSE;

    cur_board = find_board(ch);
    if (!cur_board) {
		log("malloc failure in board.c");
        return FALSE;
    }

    switch(cmd){
      case 15:
        return show_board(ch, cur_board, arg);
      case 246:
        return write_board(ch, cur_board, arg);
      case 247:
        return post_board(ch, cur_board, arg);
      case 66:
        return remove_board(ch, cur_board, arg);
      case 63:
        return read_board(ch, cur_board, arg);
      default:
		log("cmd is infeasible! (board.c)");
        return FALSE;
    }
}

/* show message number, header, write to player.
   this is called when you look board    */
int show_board( struct char_data *ch, struct board_data *cb, char *arg )
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
            sprintf(tmp, "%-2d : %s ( %s )\n\r", 
                i + 1, cb->head[i], cb->writer[i]);
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
    if(!*number || !(msg = atoi(number)))
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
    	if (GET_LEVEL(ch) < IMO) {
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
    for(; ind < cb->m_num; ind++){
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
    if(!*number || !(msg = atoi(number)))
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

/*	write message routine
	this will be called when you write <header>			*/
int write_board(struct char_data *ch, struct board_data *cb, char *arg)
{
  struct obj_data *paper;
  
  if(!arg) return 0;
  
  while(*arg && isspace(*arg)) arg++;
  
  /* delete "board " */
  /* while(*arg && !isspace(*arg)) arg++; */ 
  while(*arg && isspace(*arg)) arg++;
  
  /* no title */
  if(!*arg){
    send_to_char("No title?#@\n\r", ch);
    return 1;
  }
  
  /* load paper */
  if(!(paper = read_object(3035, VIRTUAL))){
    return 0;
  }
  
  /* give paper, pen to ch */
  obj_to_char(paper, ch);
  
  /* write paper with pen */
#define MAX_NOTE_LENGTH MAX_STRING_LENGTH
  ch->desc->str = &paper->action_description;
  ch->desc->max_str = MAX_NOTE_LENGTH;
  CREATE(ch->desc->title, char, strlen(arg) + 10);
  sprintf(ch->desc->title, "paper %s", arg);
  
  return 1;
}

/*  posting message routine.
    this will be called when you post <paper> <header>   */
int post_board(struct char_data *ch, struct board_data *cb, char *arg)
{
    char papername[MAX_INPUT_LENGTH], header[MAX_INPUT_LENGTH],
        buf[MAX_STRING_LENGTH];
    struct obj_data *paper;
    extern struct obj_data *get_obj_in_list_vis(
        struct char_data *ch, char *name,struct obj_data *list);

    half_chop(arg, papername, header);
    if (*papername == 0 || *header == 0)
		return FALSE;
    paper = get_obj_in_list_vis(ch, papername, ch->carrying);

    if (!paper) {
        sprintf( buf, "You can't find %s in your inventory.\n\r", papername);
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
        cb->head[cb->m_num] = (char *)malloc(strlen(header) + 1);
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
    	send_to_char(buf, ch);
    	save_board(cb);
    }

    return TRUE;
}

