/******************************************************************************
* board.c ( board routine of KIT Mud a.k.a. jinsil mud )                      *
* board.c is coded by Kim Jiho ( Nara of Baram )                              *
******************************************************************************/

#include <stdio.h>
/* cyb #include <sys/stat.h> */
#include <fcntl.h>
#include "structs.h"
#include "utils.h"
#include "comm.h"

#define MAX_MSGS 800
#define BOARD_DIR "boards"

struct board_data {
    char *head[MAX_MSGS];  /* head of board */
    char *msgs[MAX_MSGS];  /* msg of board */
    char writer[MAX_MSGS][20]; /* writer of that mesg */
    int m_num;  /* msg number that board contains */
    FILE *fp;   /* file pointer */
    int room_num;  /* room number that board is in */
    char bfile[50];  /* board file name */
    struct board_data *next;  /* next */
};

struct board_data *board_list;
extern struct room_data *world;

void init_board();
struct board_data *init_a_board(struct char_data *ch);
struct board_data *find_board(struct char_data *ch);
void load_board( struct board_data *cb );
void save_board( struct board_data *cb );
void error_log( char *str );

/* this function will be inserted into db.c 
   init all board  */
void init_board()
{
    board_list = 0;
}

/* init one board and return its pointer */
struct board_data *init_a_board(struct char_data *ch)
{
    struct board_data *cr_board;

    cr_board = (struct board_data *) malloc( sizeof( struct board_data ) );
    if( cr_board ){
        bzero(cr_board,sizeof(cr_board));
        cr_board->room_num = world[ch->in_room].number;
        sprintf( cr_board->bfile, "%s/%dboard", 
                 BOARD_DIR, cr_board->room_num );
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
    if( board_list ){
        for( tmp_board = board_list; tmp_board; tmp_board = tmp_board->next ){
            if( tmp_board->room_num == ch_rnum ){
                return tmp_board;
            }
        }
    }
    tmp_board = init_a_board(ch);
    return tmp_board;
}

/* load board from disk. 
   this routine is called once when board is initialized. */
void load_board(struct board_data *cb )
{
    int ind, m_num, size;

    cb->fp = fopen( cb->bfile, "r+" );
    if( cb->fp == 0 ){
        creat( cb->bfile, 448 );
        cb->fp = fopen( cb->bfile, "r+" );
    }
    if( cb->fp == 0 ){
        return;
    }

    fread(&m_num, sizeof(int), 1, cb->fp);
    cb->m_num = m_num;
    if (m_num < 0 || m_num > MAX_MSGS || feof(cb->fp)) {
        error_log("board msg file corrupt or nonexistent.\n\r");
        cb->m_num = 0;
        fclose(cb->fp);
        return;
    }

    for( ind = 0; ind < m_num; ind++ ){
        fread(&size, sizeof(int), 1, cb->fp);
        cb->head[ind] = (char *) malloc( size+1 );
        if( !cb->head[ind] ){
            error_log("board malloc failure.\n\r");
            cb->m_num = m_num = ind;
        }
        fread(cb->head[ind],sizeof(char),size,cb->fp);
        cb->head[ind][size] = 0;
        fread(cb->writer[ind],sizeof(char),20,cb->fp);
        fread(&size,sizeof(int), 1, cb->fp);
        cb->msgs[ind] = (char *)malloc( size+1 );
        if( !cb->msgs[ind] ){
            error_log("board malloc failure.\n\r");
            cb->m_num = m_num = ind;
        }
        fread(cb->msgs[ind],sizeof(char),size,cb->fp);
        cb->msgs[ind][size] = 0;
    }
    fclose(cb->fp);
    return;
}

/* save board on disk.
   this routine is called when message is posted on board 
   or message is removed from board.     */
void save_board( struct board_data *cb )
{
    int ind, m_num, size;

    if( cb->m_num == 0 ) return;
    cb->fp = fopen( cb->bfile, "r+" );
    if( cb->fp == 0 ){
        error_log("unable to open board file.\n\r");
        return;
    }

    m_num = cb->m_num;
    fwrite(&m_num, sizeof(int), 1, cb->fp);
    for( ind = 0; ind < m_num; ind++ ){
        size = strlen(cb->head[ind])+1;
        fwrite(&size, sizeof(int), 1, cb->fp);
        fwrite(cb->head[ind],sizeof(char),size,cb->fp);
        fwrite(cb->writer[ind],sizeof(char),20,cb->fp);
        size = strlen(cb->msgs[ind])+1;
        fwrite(&size, sizeof(int), 1, cb->fp);
        fwrite(cb->msgs[ind], sizeof(char), size, cb->fp);
    }
    fclose(cb->fp);
}

/* board main routine */
int board( struct char_data *ch, int cmd, char *arg )
{
    struct board_data *cur_board;

    if(!ch->desc) return(FALSE);
    if( cmd != 15 && cmd != 247 && cmd != 66 && cmd != 63 ) return(FALSE);

    cur_board = find_board(ch);
    if( cur_board == 0 ){
        error_log( " board malloc failure\n\r." );
        return( FALSE );
    }

    switch(cmd){
    case 15:
        return(show_board(ch,cur_board,arg));
    case 247:
        return(post_board(ch,cur_board,arg));
    case 66:
        return(remove_board(ch,cur_board,arg));
    case 63:
        return(read_board(ch,cur_board,arg));
    default:
        return(FALSE);
    }
}

/* show message number, header, write to player.
   this is called when you look board    */
int show_board( struct char_data *ch, struct board_data *cb, char *arg )
{
    int i;
    char buf[MAX_STRING_LENGTH], tmp[MAX_INPUT_LENGTH];

    one_argument(arg, tmp);
    if( !*tmp || !isname(tmp, "board bullerin")) return(FALSE);
    act("$n studies the board.", TRUE, ch, 0, 0, TO_ROOM);
    strcpy(buf, "This is a bulletin board.\n\r");
    strcat(buf, "Usage: READ/REMOVE <messg #>, POST <paper> <header>\n\r");
    if (!cb->m_num)
        strcat(buf, "The board is empty.\n\r");
    else {
        sprintf(buf + strlen(buf), "There are %d messages on the board.\n\r",
                cb->m_num);
        for (i = cb->m_num-1; i >= 0; i--)  {
            sprintf(buf + strlen(buf), "%-2d : %s ( %s )\n\r", 
                    i + 1, cb->head[i], cb->writer[i]);
			if (strlen(buf) > MAX_STRING_LENGTH - 80) break;
		}
    }
    page_string(ch->desc, buf, 1);

    return(TRUE);
}

/* routien that remove message from board.
   this is caleed when you type remove #     */
int remove_board(struct char_data *ch, struct board_data *cb, char *arg)
{
    int ind, msg;
    char number[MAX_INPUT_LENGTH];

    one_argument(arg, number);
    if(!*number || !ISDIGIT(*number)) return(FALSE);
    if(!(msg = atoi(number))) return(FALSE);

    if(GET_LEVEL(ch) < IMO){
        send_to_char("Only Immortal can remove messages from board.\n\r",ch);
        return(TRUE);
    }
    if(cb->m_num==0){
        send_to_char("The board is empty!\n\r", ch);
        return(TRUE);
    }
    if(msg < 1 || msg > cb->m_num){
        send_to_char("That message exists only in your imagination.\n\r",ch);
        return(TRUE);
    }

    ind = msg;  ind--; cb->m_num--;
    free_string_type(cb->head[ind]);
    if(cb->msgs[ind]) free_string_type(cb->msgs[ind]);
    for(; ind < cb->m_num; ind++){
        cb->head[ind] = cb->head[ind+1];
        cb->msgs[ind] = cb->msgs[ind+1];
        strcpy(cb->writer[ind], cb->writer[ind+1]);
    }

    send_to_char("Message is removed.\n\r", ch);
    save_board(cb);
    return(TRUE);
}

/*  reading message from board routine.
    this will be called when you read #    */
int read_board(struct char_data *ch, struct board_data *cb, char *arg)
{
    char buffer[MAX_STRING_LENGTH+256], number[MAX_INPUT_LENGTH+256];
    int msg;

    one_argument(arg, number);
    if(!*number || !ISDIGIT(*number)) return(FALSE);
    if(!(msg = atoi(number))) return(FALSE);

    if(cb->m_num == 0){
        send_to_char("The board is empty!\n\r", ch);
        return(TRUE);
    }
    if( msg < 1 || msg > cb->m_num ){
        send_to_char("That message exists only in your imagination.\n\r",ch);
        return(TRUE);
    }

    sprintf(buffer,"message %d : %s by %s\n\r\n\r%s",
            msg, cb->head[msg-1], cb->writer[msg-1], cb->msgs[msg-1] );
    page_string(ch->desc, buffer, 1);
    return(TRUE);
}

/*  posting message routine.
    this will be called when you post <paper> <header>   */
int post_board(struct char_data *ch, struct board_data *cb, char *arg)
{
    char papername[MAX_INPUT_LENGTH], header[MAX_INPUT_LENGTH],
        buf[MAX_STRING_LENGTH];
    struct obj_data *paper;
    extern struct obj_data *get_obj_in_list_vis(
        struct char_data *ch, char *name,struct obj_data *list );

    half_chop(arg,papername,header);
    if( *papername == 0 || *header == 0 ) return(FALSE);
    paper = get_obj_in_list_vis(ch,papername,ch->carrying);

    if( !paper ){
        sprintf( buf, "You can't find %s in your inventory.\n\r", papername );
    }
    else if( paper->obj_flags.type_flag != ITEM_NOTE ){
        sprintf( buf, "You can't post %s on board.\n\r", papername );
    }
    else if( paper->action_description == 0 ){
        sprintf(buf,"%s is empty.\n\r",papername);
    }
    else{
        sprintf(buf,"Ok. You posted %s on board.\n\r",papername);
        cb->head[cb->m_num] = (char *)malloc(strlen(header)+1);
        if( cb->head[cb->m_num] == 0 ){
            sprintf(buf,"Your %s is fallen from board.\n\r",papername);
            send_to_char(buf, ch);
            return(TRUE);
        }
        strcpy(cb->head[cb->m_num],header);
        strcpy(cb->writer[cb->m_num],GET_NAME(ch));
        cb->msgs[cb->m_num] = paper->action_description;
        paper->action_description = 0;
        cb->m_num++;
        extract_obj(paper);
    }

    send_to_char(buf,ch);
    save_board(cb);
    return(TRUE);
}

/* board error log routine */
void error_log(char *str)
{
    fputs("board : ", stderr);
    fputs( str, stderr );
    return;
}

