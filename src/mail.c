/******************************************************************************
* mail.c ( mail routine of KIT Mud a.k.a. jinsil mud )                      *
* mail.c is coded by Kim Jiho ( Nara of Baram )                              *
******************************************************************************/
/* this is not so good code. you had better modify this one. */

#include <stdio.h>
#include <string.h>
/*
#include <sys/types.h>
#include <sys/stat.h> 
*/
#include <fcntl.h>
#include "structs.h"
#include "utils.h"
#include "comm.h"
#include "db.h"
#include "handler.h"
#include "interpreter.h"

#define MAX_MAILS 200
#define MBOARD_DIR "mboxes"

struct mbox_data {
    char *head[MAX_MAILS];  /* head of mbox */
    char *msgs[MAX_MAILS];  /* msg of mbox */
    char sender[MAX_MAILS][20]; /* sender of that mesg */
    char receiver[MAX_MAILS][20]; /* receiver of that mesg */
    int m_num;  /* msg number that mbox contains */
    FILE *fp;   /* file pointer */
    int room_num;  /* room number that mbox is in */
    char bfile[50];  /* mbox file name */
    struct mbox_data *next;  /* next */
};

struct mbox_data *mbox_list;
extern struct room_data *world;

void page_string(struct descriptor_data *d, char *str, int keep);
int number(int from, int to);


void init_mbox();
struct mbox_data *init_a_mbox(struct char_data *ch);
struct mbox_data *find_mbox(struct char_data *ch);
void load_mbox( struct mbox_data *cb );
void save_mbox( struct mbox_data *cb );
void mail_error_log( char *str );
void remove_mail(struct mbox_data *cb, int m_num);
int show_mail( struct char_data *ch, struct mbox_data *cb, char *arg);
int post_mail(struct char_data *ch, struct mbox_data *cb, char *arg);
int get_mail(struct char_data *ch, struct mbox_data *cb, char *arg);

/* this function will be inserted into db.c 
   init all mbox  */
void init_mbox()
{
    mbox_list = 0;
}

/* init one mbox and return its pointer */
struct mbox_data *init_a_mbox(struct char_data *ch)
{
    struct mbox_data *cr_mbox;

    cr_mbox = (struct mbox_data *) malloc( sizeof( struct mbox_data ) );
    if( cr_mbox ){
	// BUG FIX!!
        bzero(cr_mbox,sizeof(*cr_mbox));
        cr_mbox->room_num = world[ch->in_room].number;
        sprintf( cr_mbox->bfile, "%s/%dmbox", 
            MBOARD_DIR, cr_mbox->room_num );
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
    if( mbox_list ){
        for( tmp_mbox = mbox_list; tmp_mbox; tmp_mbox = tmp_mbox->next ){
            if( tmp_mbox->room_num == ch_rnum ){
                return tmp_mbox;
            }
        }
    }
    tmp_mbox = init_a_mbox(ch);
    return tmp_mbox;
}

void load_mbox(struct mbox_data *cb )
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
    if (m_num < 1 || m_num > MAX_MAILS || feof(cb->fp)) {
        mail_error_log("mbox msg file corrupt or nonexistent.\n\r");
        cb->m_num = 0;
        fclose(cb->fp);
        return;
    }

    for( ind = 0; ind < m_num; ind++ ){
        fread(&size, sizeof(int), 1, cb->fp);
        cb->head[ind] = (char *) malloc( size+1 );
        if( !cb->head[ind] ){
            mail_error_log("mbox malloc failure.\n\r");
            cb->m_num = m_num = ind;
        }
        fread(cb->head[ind],sizeof(char),size,cb->fp);
        cb->head[ind][size] = NUL;
        fread(cb->sender[ind],sizeof(char),20,cb->fp);
        fread(cb->receiver[ind],sizeof(char),20,cb->fp);
        fread(&size,sizeof(int), 1, cb->fp);
        cb->msgs[ind] = (char *)malloc( size+1 );
        if( !cb->msgs[ind] ){
            mail_error_log("mbox malloc failure.\n\r");
            cb->m_num = m_num = ind;
        }
        fread(cb->msgs[ind],sizeof(char),size,cb->fp);
        cb->msgs[ind][size] = NUL;
    }
    fclose(cb->fp);
    return;
}

void save_mbox( struct mbox_data *cb )
{
    int ind, m_num, size;

    if( cb->m_num == 0 ) return;
    cb->fp = fopen( cb->bfile, "r+" );
    if( cb->fp == 0 ){
        mail_error_log("unable to open mbox file.\n\r");
        return;
    }

    m_num = cb->m_num;
    fwrite(&m_num, sizeof(int), 1, cb->fp);
    for( ind = 0; ind < m_num; ind++ ){
        size = strlen(cb->head[ind]) + 1;
        fwrite(&size, sizeof(int), 1, cb->fp);
        fwrite(cb->head[ind],sizeof(char),size,cb->fp);
        fwrite(cb->sender[ind],sizeof(char),20,cb->fp);
        fwrite(cb->receiver[ind],sizeof(char),20,cb->fp);
        size = strlen(cb->msgs[ind]) + 1;
        fwrite(&size, sizeof(int), 1, cb->fp);
        fwrite(cb->msgs[ind], sizeof(char), size, cb->fp);
    }
    fclose(cb->fp);
}


int mbox( struct char_data *ch, int cmd, char *arg )
{
    struct mbox_data *cur_mbox;

    if(!ch->desc) return(FALSE);
    if( cmd != 15 && cmd != 247 && cmd != 10 ) return(FALSE);

    cur_mbox = find_mbox(ch);
    if( cur_mbox == 0 ){
        mail_error_log( " mbox malloc failure\n\r." );
        return( FALSE );
    }

    switch(cmd){
      case 15:
        return(show_mail(ch,cur_mbox,arg));
      case 247:
        return(post_mail(ch,cur_mbox,arg));
      case 10:
        return(get_mail(ch,cur_mbox,arg));
      default:
        return(FALSE);
    }
}

int show_mail( struct char_data *ch, struct mbox_data *cb, char *arg )
{
    int i, pm_num=0;
    char buf[MAX_STRING_LENGTH], tmp[MAX_INPUT_LENGTH];

    one_argument(arg, tmp);
    if( !*tmp || !isname(tmp, "mail mbox board")) return(FALSE);
    strcpy(buf, "This is a mail box.\n\r");
    strcat(buf,"Usage: GET letter mbox, POST <receiver> <paper> <header>\n\r");

    for (i = 0; i < cb->m_num; i++){
        if( isname(GET_NAME(ch), cb->receiver[i]) || GET_LEVEL(ch) > IMO+2 ){
            sprintf(buf+strlen(buf),"   %s from %s\n\r",
                cb->head[i],cb->sender[i]);
            sprintf(buf+strlen(buf)," to %s\n\r", cb->receiver[i]);
            pm_num++;
        }
    }
    if (!pm_num)
        strcat(buf, "The mbox is empty.\n\r");
    page_string(ch->desc, buf, 0);

    return(TRUE);
}

void remove_mail(struct mbox_data *cb, int m_num)
{
    int ind;

    ind = m_num; cb->m_num--;
    free(cb->head[ind]);
    if(cb->msgs[ind]) free(cb->msgs[ind]);
    for(; ind < cb->m_num; ind++){
        cb->head[ind] = cb->head[ind+1];
        cb->msgs[ind] = cb->msgs[ind+1];
        strcpy(cb->sender[ind], cb->sender[ind+1]);
        strcpy(cb->receiver[ind], cb->receiver[ind+1]);
    }

    save_mbox(cb);
}

int get_mail(struct char_data *ch, struct mbox_data *cb, char *arg)
{
    char buffer[MAX_STRING_LENGTH], lt[MAX_INPUT_LENGTH],
        mb[MAX_INPUT_LENGTH];
    int pm_num=0, i, paper_num = 3035;
    struct obj_data *letter;
    argument_interpreter(arg, lt, mb);
    if( !*arg || !isname(mb, "mail mbox board") ||
        !isname(lt, "letter mail") ) return(FALSE);

    for (i = 0; i < cb->m_num; i++){
        if( isname(GET_NAME(ch), cb->receiver[i]) ){
            sprintf(buffer,"letter %s from %s\n\r\n\r%s",
                cb->head[i], cb->sender[i], cb->msgs[i] );
            pm_num++;
            if( (paper_num = real_object(paper_num)) < 0 ){
                mail_error_log("no such paper item");
                return(FALSE);
            }
            remove_mail(cb,i);
            letter = read_object(paper_num,REAL);
            obj_to_char(letter,ch);
            CREATE(letter->action_description,char,strlen(buffer)+1);
            strcpy( letter->action_description, buffer );
            send_to_char( "You got letter from mail box.\n\r", ch );
            return(TRUE);
        }
    }
        
    send_to_char("Your mail box is empty!\n\r", ch);
    return(TRUE);
}

int post_mail(struct char_data *ch, struct mbox_data *cb, char *arg)
{
    char papername[MAX_INPUT_LENGTH], header[MAX_INPUT_LENGTH],
        buf[MAX_STRING_LENGTH], receiver[MAX_INPUT_LENGTH];
    struct obj_data *paper;
    extern struct obj_data *get_obj_in_list_vis(
        struct char_data *ch, char *name,struct obj_data *list );

    half_chop(arg,receiver,buf);
    half_chop(buf,papername,header);
    if( *papername == 0 || *header == 0 || *receiver == 0 ) return(FALSE);
    paper = get_obj_in_list_vis(ch,papername,ch->carrying);

    if( !paper ){
        sprintf( buf, "You can't find %s in your inventory.\n\r", papername );
    }
    else if( paper->obj_flags.type_flag != ITEM_NOTE ){
        sprintf( buf, "You can't post %s on mbox.\n\r", papername );
    }
    else if( paper->action_description == 0 ){
        sprintf(buf,"%s is empty.\n\r",papername);
    }
    else{
        sprintf(buf,"Ok. You posted %s on mbox.\n\r",papername);
        cb->head[cb->m_num] = (char *)malloc(strlen(header)+1);
        if( cb->head[cb->m_num] == 0 ){
            sprintf(buf,"Your %s is fallen from mbox.\n\r",papername);
            send_to_char(buf, ch);
            return(TRUE);
        }
        strcpy(cb->head[cb->m_num],header);
        strcpy(cb->sender[cb->m_num],GET_NAME(ch));
        strcpy(cb->receiver[cb->m_num],receiver);
        cb->msgs[cb->m_num] = paper->action_description;
        paper->action_description = 0;
        cb->m_num++;
        extract_obj(paper);
    }

    send_to_char(buf,ch);
    save_mbox(cb);
    return(TRUE);
}

void mail_error_log(char *str)
{
    fputs("mbox : ", stderr);
    fputs( str, stderr );
    return;
}

