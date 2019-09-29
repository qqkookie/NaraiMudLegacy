/******************************************************************************
* mail.c ( mail routine of KIT Mud a.k.a. jinsil mud )                      *
* mail.c is coded by Kim Jiho ( Nara of Baram )                              *
******************************************************************************/
/* this is not so good code. you had better modify this one. */

#include <stdio.h>
/*
#include <sys/types.h>
#include <sys/stat.h> 
*/
#include <fcntl.h>
#include <time.h>
#include "structs.h"
#include "utils.h"
#include "comm.h"
#include "db.h"
#include "handler.h"
#include "interpreter.h"
#include "command.h"
#include "memory.h"

#define MAX_MAILS 200
#define MBOARD_DIR "mboxes"
#define	POST_OFFICE	3071

struct mbox_data {
	char *head[MAX_MAILS];  /* head of mbox */
	char *msgs[MAX_MAILS];  /* msg of mbox */
	char sender[MAX_MAILS][20]; /* sender of that mesg */
	char receiver[MAX_MAILS][20]; /* receiver of that mesg */
	char create_time[MAX_MAILS][30] ;	/* posting time */
	int m_num;  /* msg number that mbox contains */
	FILE *fp;   /* file pointer */
	int room_num;  /* room number that mbox is in */
	char bfile[50];  /* mbox file name */
	struct mbox_data *next;  /* next */
};

struct mbox_data *mbox_list;
extern struct room_data *world;

void init_mbox();
struct mbox_data *init_a_mbox(int char_in_room);
struct mbox_data *find_mbox(struct char_data *ch);
int load_mbox( struct mbox_data *cb );
int save_mbox( struct mbox_data *cb );
void mail_error_log( char *str );
int remove_mail(struct mbox_data *cb, int m_num);
int show_mail( struct char_data *ch, struct mbox_data *cb, char *arg ) ;
int post_mail(struct char_data *ch, struct mbox_data *cb, char *arg) ;
int get_mail(struct char_data *ch, struct mbox_data *cb, char *arg) ;

/* this function will be inserted into db.c 
   init all mbox  */
void init_mbox()
{
	init_a_mbox(real_room(POST_OFFICE)) ;
}

/* init one mbox and return its pointer */
struct mbox_data *init_a_mbox(int char_in_room)
{
    struct mbox_data *cr_mbox;

    cr_mbox = (struct mbox_data *) malloc( sizeof( struct mbox_data ) );
    if( cr_mbox ){
        bzero(cr_mbox,sizeof(cr_mbox));
        cr_mbox->room_num = world[char_in_room].number;
        sprintf( cr_mbox->bfile, "%s/%dmbox", 
                 MBOARD_DIR, cr_mbox->room_num );
        cr_mbox->next = mbox_list;
        mbox_list = cr_mbox;
        if ( load_mbox(cr_mbox) < 0 )
			return NULL ;
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

	/* if return value of init_a_mbox is NULL, also find_mbox returns NULL */
    tmp_mbox = init_a_mbox(ch->in_room);
    return tmp_mbox;
}

int load_mbox(struct mbox_data *cb )
{
    int ind, m_num, size;

    cb->fp = fopen( cb->bfile, "r+" );
    if( cb->fp == 0 ){
        creat( cb->bfile, 448 );
        cb->fp = fopen( cb->bfile, "r+" );
    }
    if( cb->fp == 0 ){
        return (-1) ;	/* load fail */
    }

    fread(&m_num, sizeof(int), 1, cb->fp);
    cb->m_num = m_num;
    if (m_num < 1 || m_num > MAX_MAILS || feof(cb->fp)) {
        mail_error_log("mbox msg file corrupt or nonexistent.\n\r");
        cb->m_num = 0;
        fclose(cb->fp);
        return (-1) ;	/* load fail */
    }

    for( ind = 0; ind < m_num; ind++ ){
        fread(&size, sizeof(int), 1, cb->fp);
        cb->head[ind] = (char *) malloc( size+1 );
        if( !cb->head[ind] ){
            mail_error_log("mbox malloc failure.\n\r");
            cb->m_num = m_num = ind;
        }
        fread(cb->head[ind],sizeof(char),size,cb->fp);
        cb->head[ind][size] = 0;
        fread(cb->sender[ind],sizeof(char),20,cb->fp);
        fread(cb->receiver[ind],sizeof(char),20,cb->fp);
        fread(cb->create_time[ind],sizeof(char),30,cb->fp);
        fread(&size,sizeof(int), 1, cb->fp);
        cb->msgs[ind] = (char *)malloc( size+1 );
        if( !cb->msgs[ind] ){
            mail_error_log("mbox malloc failure.\n\r");
            cb->m_num = m_num = ind;
        }
        fread(cb->msgs[ind],sizeof(char),size,cb->fp);
        cb->msgs[ind][size] = 0;
    }
    fclose(cb->fp);
    return 0 ;	/* load success */
}

int save_mbox( struct mbox_data *cb )
{
    int ind, m_num, size;

    if( cb->m_num == 0 ) return (-1) ;
    cb->fp = fopen( cb->bfile, "r+" );
    if( cb->fp == 0 ){
        mail_error_log("unable to open mbox file.\n\r");
        return (-1) ;	/* save fail */
    }

    m_num = cb->m_num;
    fwrite(&m_num, sizeof(int), 1, cb->fp);
    for( ind = 0; ind < m_num; ind++ ){
        size = strlen(cb->head[ind]) + 1;
        fwrite(&size, sizeof(int), 1, cb->fp);
        fwrite(cb->head[ind],sizeof(char),size,cb->fp);
        fwrite(cb->sender[ind],sizeof(char),20,cb->fp);
        fwrite(cb->receiver[ind],sizeof(char),20,cb->fp);
		fwrite(cb->create_time[ind], sizeof(char), 30, cb->fp) ;
        size = strlen(cb->msgs[ind]) + 1;
        fwrite(&size, sizeof(int), 1, cb->fp);
        fwrite(cb->msgs[ind], sizeof(char), size, cb->fp);
    }
    fclose(cb->fp);

	return 0 ;	/* save successful */
}

int mbox( struct char_data *ch, int cmd, char *arg )
{
    struct mbox_data *cur_mbox;
	char	buf[BUFSIZ] ;

	if ( !ch ) return (FALSE) ;
    if(!ch->desc) return(FALSE);
    if( cmd != COM_LOOK && cmd != COM_POST && cmd != COM_GET ) return(FALSE);

    cur_mbox = find_mbox(ch);
    if( cur_mbox == NULL ){
		sprintf(buf, "mbox: Player %s's mbox malloc failure\n\r", GET_NAME(ch)) ;
        mail_error_log( buf );
        return( FALSE );
    }

    switch(cmd){
    case COM_LOOK :	/* look mbox */
        return(show_mail(ch,cur_mbox,arg));
    case COM_POST :	/* post paper ... */
        return(post_mail(ch,cur_mbox,arg));
    case COM_GET :		/* get letter mbox */
        return(get_mail(ch,cur_mbox,arg));
    default:
        return(FALSE);
    }
}

/*  check if 'ch' have mail or not  */
int check_mail( struct char_data *ch)	/* by cyb */
{
	struct mbox_data *cur_mbox, *tmp_mbox ;
	char	buf[BUFSIZ] ;
	int	i, ch_rnum ;

	if ( !ch ) return (FALSE) ;

	cur_mbox = NULL ;
	/* room number of post office */
	ch_rnum = world[real_room(POST_OFFICE)].number;
	if( mbox_list ){
		for( tmp_mbox = mbox_list; tmp_mbox; tmp_mbox = tmp_mbox->next ){
			if( tmp_mbox->room_num == ch_rnum ){
				cur_mbox = tmp_mbox;
				break ;
            }
        }
    }
	else {
		mail_error_log("check_mail: There is no mbox list.\n\r") ;
		return FALSE ;
    }

	if( cur_mbox == NULL ){
		mail_error_log("check_mail: There is no mbox.\n\r") ;
		return( FALSE );
    }

	/* find mail */
	for (i = 0 ; i < cur_mbox->m_num; i++){
		if( isname(GET_NAME(ch), cur_mbox->receiver[i]) ){
            return (POST_OFFICE) ;	/* mail found */
        }
    }

	return FALSE ;	/* have no mail for you */
}

int show_mail( struct char_data *ch, struct mbox_data *cb, char *arg )
{
    int i, pm_num=0, count;
    char buf[MAX_STRING_LENGTH], tmp[MAX_INPUT_LENGTH];

	/* 'look mail' or 'look mbox' or 'look board' is valid */
    one_argument(arg, tmp);
    if( !*tmp || !isname(tmp, "mail mbox board")) return(FALSE);
    strcpy(buf, "This is a mail box.\n\r");
    strcat(buf,"Usage: GET letter mbox, POST <receiver> <paper> <header>\n\r");

    for (i = 0, count = 0 ; i < cb->m_num; i++){
        if( GET_LEVEL(ch) > IMO+2 ||
            isname(GET_NAME(ch), cb->receiver[i]) ||
            isname(GET_NAME(ch), cb->sender[i]) ){
            count ++ ;
            sprintf(buf+strlen(buf),"[%2d] \"%s\" from %s\n\r",
                    count, cb->head[i], cb->sender[i]);
            sprintf(buf+strlen(buf),"  to %s (%s)\n\r", cb->receiver[i],
					cb->create_time[i]);
            pm_num++;
        }
    }
    if (!pm_num)
        strcat(buf, "The mbox is empty.\n\r");
    page_string(ch->desc, buf, 1);

    return(TRUE);
}

int remove_mail(struct mbox_data *cb, int m_num)
{
    int ind;
    char number[MAX_INPUT_LENGTH];

    ind = m_num; cb->m_num--;
    free_string_type(cb->head[ind]);
    if(cb->msgs[ind]) free_string_type(cb->msgs[ind]);
    for(; ind < cb->m_num; ind++){
        cb->head[ind] = cb->head[ind+1];
        cb->msgs[ind] = cb->msgs[ind+1];
        strcpy(cb->sender[ind], cb->sender[ind+1]);
        strcpy(cb->receiver[ind], cb->receiver[ind+1]);
    }

    if ( save_mbox(cb) < 0 ) {	/* save fail */
		mail_error_log("remove_mail: save failed ") ;
		return (-1) ;
    }

	return 0 ;	/* successful */
}

int get_mail(struct char_data *ch, struct mbox_data *cb, char *arg)
{
	char buffer[MAX_STRING_LENGTH+200], lt[MAX_INPUT_LENGTH],
		mb[MAX_INPUT_LENGTH];
	char	buf2[MAX_STRING_LENGTH] ;
	int msg, pm_num=0, i ;
	int original_paper_num = 3035, paper_num ;
	struct obj_data *letter;
	long	ct ;
	char	current_time[BUFSIZ] ;

	argument_interpreter(arg, lt, mb);
	if( !*arg || !isname(mb, "mail mbox board") ||
        !isname(lt, "letter mail") ) return(FALSE);

	for (i = 0; i < cb->m_num; i++){
        if( isname(GET_NAME(ch), cb->receiver[i]) ){
            ct = time(0) ;	/* received time */
            strcpy(current_time, asctime(localtime(&ct))) ;
            current_time[strlen(current_time) -1] = 0 ;
            sprintf(buffer,"Letter \"%s\" from %s\n\r",
                    cb->head[i], cb->sender[i]);
            sprintf(buf2,"Send: %s\n\rRecieved: %s\n\r\n\r%s",
                    cb->create_time[i], current_time, cb->msgs[i] );
            strcat(buffer, buf2) ;
            pm_num++;
            if( (paper_num = real_object(original_paper_num)) < 0 ){
                mail_error_log("no such paper item");
                return(FALSE);
            }
            if ( remove_mail(cb,i) < 0 ) {	/* remove fail */
                send_to_char("Mail system corrupted, sorry.\n\r", ch) ;
                return(TRUE) ;
            }
            letter = read_object(paper_num,REAL);
            obj_to_char(letter,ch);
            /*
              create(letter->action_description,char,strlen(buffer+1));
            */
            letter->action_description = malloc_string_type(strlen(buffer+1)) ;
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
	int	find_name(char *name) ;		/* check player's existance */
	long	ct ;
	char	time_str[BUFSIZ] ;
	char papername[MAX_INPUT_LENGTH], header[MAX_INPUT_LENGTH],
		buf[MAX_STRING_LENGTH], receiver[MAX_INPUT_LENGTH];
	struct obj_data *paper;
	extern struct obj_data *get_obj_in_list_vis(
        struct char_data *ch, char *name,struct obj_data *list );

	half_chop(arg,receiver,buf);
	half_chop(buf,papername,header);
	if( *papername == 0 || *header == 0 || *receiver == 0 ) {
		send_to_char("Post what ?  Look mbox for usage.\n\r", ch) ;
		return(FALSE);
    }
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
	else if ( find_name(receiver) < 0 ) {
		sprintf(buf, "There is no such receiver in this MUD.\n\r") ;
    }
	else{
		if ( cb->m_num >= MAX_MAILS ) {
			send_to_char("Too much mail is posted already, try later,\n\r", ch);
			return TRUE ;
        }
        sprintf(buf,"Ok. You posted %s on mbox.\n\r",papername);
        cb->head[cb->m_num] = (char *)malloc(strlen(header)+1);
        if( cb->head[cb->m_num] == 0 ){
            sprintf(buf, "Your %s is fallen from mbox.\n\r", papername);
            send_to_char(buf, ch);
            return(TRUE);
        }
        strcpy(cb->head[cb->m_num],header);
        strcpy(cb->sender[cb->m_num],GET_NAME(ch));
        strcpy(cb->receiver[cb->m_num],receiver);
		ct = time(0) ;
		strcpy(time_str, asctime(localtime(&ct))) ;
		time_str[strlen(time_str) - 1] = 0 ;
		strcpy(cb->create_time[cb->m_num], time_str) ;
		free_string_type(time_str) ;
        cb->msgs[cb->m_num] = paper->action_description;
        paper->action_description = 0;
        cb->m_num++;
        extract_obj(paper);
    }

    send_to_char(buf,ch);
    if ( save_mbox(cb) < 0 ) {	/* save fail */
		send_to_char("SERVER: Can't save your mail. Ask to wizard.\n\r", ch) ;
    }
    return(TRUE);
}

void do_mail(struct char_data *ch, char *argument, int cmd)
{
	int check_mail(struct char_data *ch) ;

	if ( ch == NULL ) return ;

	if ( check_mail(ch) )
		send_to_char("You have MAIL in Post Office.\n\r", ch) ;
	else
		send_to_char("No mail for you.\n\r", ch) ;

	return ;
}

void mail_error_log(char *str)
{
    fputs("MAIL SYSTEM: ", stderr);
    fputs( str, stderr );
    return;
}

