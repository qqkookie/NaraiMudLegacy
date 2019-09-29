/*
  apartment.c - private apartment for dangun player

  code by jmjeong@jmjeong.com(12/4/2001)
*/
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <assert.h>
#include <stdlib.h>

#include "structs.h"
#include "utils.h"
#include "comm.h"
#include "handler.h"
#include "db.h"
#include "interpreter.h"
#include "command.h"

/* #include "interpreter.h" */
/* #include "handler.h" */
/* #include "db.h" */
/* #include "spells.h" */
/* #include "limits.h" */
#include "command.h"

#define APARTMENT_DIR	"apartment"
#define	CONTROL_FILE    "control"

#define MAX_APARTMENT   10

extern struct room_data *world;
extern char *string_dup(char *source);
extern int is_all_digit(char *str);
extern int id_to_hash(struct obj_data *obj);
extern void page_string(struct descriptor_data *d,
                        char *str, int keep_internal);

int apart_list(struct char_data *ch, int idx, int narg, int value, char *name);
int apart_deposit(struct char_data *ch, int room_idx, int narg, char *name);
int apart_withdraw(struct char_data *ch, int room_idx,
                   int narg, int value, char *name);
int apart_identify(struct char_data *ch, int room_idx,
                   int narg, int value, char *name);
int apart_write(struct char_data *ch, int room_idx);

void apart_item_save(int room_idx);
struct apart_item_data *find_item_from_apart(struct obj_data *obj, int idx);
int apart_item_to_str(char *str, struct apart_item_data *mar);
void init_room(int room_idx);

char *malloc_string_type(int size);
int free_string_type(char *mem);

int check_apart_room(int virtual);

struct apart_item_data {
	int	n;
	int	id ;
	int	hash ;
	struct obj_data	*obj ;
	struct apart_item_data	*next;
} ;

#define APT_SUNDEW          1
#define APT_IDENTIFY        2

int apart_control_save_bit = 0;     // control정보 save bit

struct apart_control_data
{
    char    *owner;                 // 아파트 주인
    char    *description;           // 아파트 방 내부 묘사
    sh_int  room_num;               // room number
    int     max_item;               // 총 저장할 수 있는 item 수
    int     room_flags;             // roomflag(sundew, identify)
    time_t  visit_time;             // 마지막 아파트 방문 시기
    int     item_num;               // 현재 저장중인 item 수
    int     item_save_bit;          // item 변경이 저장되었는지
    struct apart_item_data item;    // 아파트에 보관중인 item
} dangun_apart_list[MAX_APARTMENT];

extern char *fread_string(FILE *fl);

int apart_room(struct char_data *ch, int cmd, char *arg)
{
    char  name[BUFSIZ] ;
	int	narg, nvalue = 0, status = 0;
    int room_idx;

    if ( cmd != COM_LIST && cmd != COM_DEPOSIT && cmd != COM_WITHDRAW
         && cmd != COM_IDENTIFY && cmd != COM_WRITE)
        return FALSE ;

    one_argument(arg, name) ;
    if ( *name == 0 )
		narg = 0 ;
	else {
		narg = 1 ;
		nvalue = 0 ;
		if ( is_all_digit(name) )
			nvalue = atoi(name) ;
    }

    room_idx = check_apart_room(world[ch->in_room].number);
    
	switch(cmd) {
    case COM_LIST :
        status = apart_list(ch, room_idx, narg, nvalue, name) ;
        break ;
    case COM_DEPOSIT :
        status = apart_deposit(ch, room_idx, narg, name) ;
        break ;
    case COM_WITHDRAW : 
        status = apart_withdraw(ch, room_idx, narg, nvalue, name) ;
        break ;
    case COM_IDENTIFY :
        status = apart_identify(ch, room_idx, narg, nvalue, name);
        break;
    case COM_WRITE:
        status = apart_write(ch, room_idx);
        break;
    }

	return status ;
}

/*
	apart에 똑 같은 물품이 있는지 찾아본다
	있으면 해당되는 item pointer를 반환한다.
*/
struct apart_item_data *find_item_from_apart(struct obj_data *obj, int idx)
{
	extern struct index_data *obj_index;
	struct apart_item_data *p;
	int hash;

	/* 같은 물건이 있는지 찾는다.. */
	p = dangun_apart_list[idx].item.next ;

	hash = id_to_hash(obj) ;

	for ( ; p != NULL ; p = p->next ) {
		if ( p->id == obj_index[obj->item_number].virtual ) {
			if ( hash == p->hash) {
				/* found */
				return p;
            }
        }
    }
	return NULL;
}

#define	ONE_PAGE_LINES	18

int apart_write(struct char_data *ch, int room_idx)
{
    send_to_char("방의 descrition의 변경은 바로 이루어지지 않습니다.\n\r", ch);
    send_to_char("인테리어에 시간이 걸리기 때문에 다음 부팅때 이루어집니다 .\n\r", ch);
    send_to_char("Ok.. go ahead and write.. end the note with a @.\n\r",
                 ch);
    act("$n begins to decorate $s room.", TRUE, ch, 0,0,TO_ROOM);

	if (dangun_apart_list[room_idx].description)
		free_string_type(dangun_apart_list[room_idx].description);

    dangun_apart_list[room_idx].description = 0;
    ch->desc->str = &dangun_apart_list[room_idx].description;
    ch->desc->max_str = MAX_STRING_LENGTH;

//    init_room(room_idx);

    return TRUE;
}

int apart_identify(struct char_data *ch, int room_idx,
                 int narg, int value, char *name)
{
	struct apart_item_data	*p;
	int	n;
    void spell_identify(byte level, struct char_data *ch,
                        struct char_data *victim, struct obj_data *obj);

    if (!IS_SET(dangun_apart_list[room_idx].room_flags, APT_IDENTIFY)) {
        send_to_char("Identify를 이용하려면 관리사무소에서 등록해야 합니다.\n\r", ch);
        return TRUE;
    }
    
	if ( narg <= 0 ) {
		send_to_char("어떤 걸 조사하려고요?.\n\r", ch);
		return TRUE ;
    }

	if ( value > 0 ) {	/* search by number */
		if ( value > dangun_apart_list[room_idx].item_num ) {
			send_to_char("그런 물건 번호는 없습니다.\n\r",
                         ch ) ;
			return TRUE ;
        }
		p = &dangun_apart_list[room_idx].item;
		for ( n = 1 ; n < value ; n++ , p = p->next ) ;
        /* is it right ? */
    }
	else {	/* search by name */
		p = &dangun_apart_list[room_idx].item ;
		for ( n = 0 ; p->next != NULL ; p = p->next ) {
			if ( isname(name, p->next->obj->name ))
				break ;
        }
		if ( p->next == NULL ) {
            send_to_char("그런 이름의 물건은 없습니다.\n\r", ch) ;

			return TRUE ;
        }
    }

    act("$p 에 대해 조사합니다.\n\r", 0, ch, p->next->obj, 0, TO_CHAR) ;
	act("$n 님이 $p 에 대해 조사합니다.", 0, ch, p->next->obj, 0, TO_ROOM) ;
    /* identify the object now */
    spell_identify(GET_LEVEL(ch), ch, 0, p->next->obj);

	return TRUE ;
}

int apart_deposit(struct char_data *ch, int room_idx, int narg, char *name)
{
	extern struct index_data *obj_index;
	struct apart_item_data	*p, *newitem ;
	struct obj_data *obj, *tmp_obj, *next_obj ;
	int has_something ;
    char buf[BUFSIZ];

    if (dangun_apart_list[room_idx].item_num >=
        dangun_apart_list[room_idx].max_item) {
		send_to_char("보관함 갯수가 다 찼습니다.\n\r관리 사무실에 문의해 주세요..\n\r", ch ) ;
		return TRUE ;
    }
    
	if ( narg <= 0 ) {
		send_to_char("어떤 걸 넣어두려고요?.\n\r", ch ) ;
		return TRUE ;
    }

	obj = get_obj_in_list_vis(ch, name, ch->carrying) ;
	if ( obj == NULL ) {
        send_to_char("그런 물건은 없습니다.\n\r", ch);
		return TRUE ;
    }

	/* 넣어둘 수 없는 물건들.. corpse, key */
	/* No rent item 은 여기에서 팔도록 허가 함 - 허가 하지 않게 바꿈  */
	if ((GET_ITEM_TYPE(obj) == ITEM_CONTAINER && obj->obj_flags.value[3] == 1)
		|| GET_ITEM_TYPE(obj) == ITEM_KEY
		|| IS_OBJ_STAT(obj, ITEM_NO_RENT) ) {
		send_to_char("이런 물건은 보관할 수 없습니다.\n\r",ch) ;
		return TRUE ;
    }
    
	/* 물건을 저장 */
	obj_from_char(obj) ;

	act("$p를 보관함에 보관합니다.", 0, ch, obj, 0, TO_CHAR) ;
	act("$n님이 $p를 보관합니다.", 0, ch, obj, 0, TO_ROOM) ;

	/* bag 안에 뭐가 있는지 확인 */
	if ( obj->obj_flags.type_flag == ITEM_CONTAINER ) {
		has_something = 0 ;
		for ( tmp_obj = obj->contains ; tmp_obj ; tmp_obj = next_obj ) {
			has_something = 1 ;
			next_obj = tmp_obj->next_content ;
			obj_from_obj(tmp_obj) ;
			obj_to_room(tmp_obj, ch->in_room) ;
        }
		if ( has_something ) {
            send_to_char("안에 들어 있는 물건을 내 놓습니다.\n\r", ch) ;
            act("$p안에 있는 물건을 바닥에 쏟아 놓습니다.",
                0, ch, obj, 0, TO_ROOM) ;
        }
    }

	/* 다음번에 save 하도록 bit 를 set 한다 */
    /* set save bit */
	dangun_apart_list[room_idx].item_save_bit = 1 ;

	/* 같은 물건이 있는지 찾는다.. */
	if ((p = find_item_from_apart(obj, room_idx))) {
		/* same !! Do not increase gom_item_number */
		( p->n )++;
		extract_obj(obj);
	}
    else {
        /* Not found : add as new one */
        newitem = (struct apart_item_data *) malloc_string_type (
            sizeof(struct apart_item_data)) ;
        newitem->n = 1 ;
        newitem->id = obj_index[obj->item_number].virtual ;
        newitem->hash = id_to_hash(obj);
        newitem->obj = obj ;

        newitem->next =  dangun_apart_list[room_idx].item.next;
        dangun_apart_list[room_idx].item.next = newitem ;
        dangun_apart_list[room_idx].item_num++;

    }
    sprintf(buf, "남은 보관함의 자리는 %d개 입니다\n\r",
            dangun_apart_list[room_idx].max_item -
            dangun_apart_list[room_idx].item_num);
    send_to_char(buf, ch);

    // apart_item_save(room_idx);
    
	return TRUE ;
}

int apart_withdraw(struct char_data *ch, int room_idx,
            int narg, int value, char *name)
{
	extern struct str_app_type str_app[];	/* need in weight calculation */
	struct apart_item_data	*p, *tmpp ;
	struct obj_data	*obj ;
	int	n;
    char buf[BUFSIZ];

    if (GET_LEVEL(ch) < IMO && 
		strcasecmp(GET_NAME(ch), dangun_apart_list[room_idx].owner)) {
        send_to_char("아파트 주인만이 물건을 인출할 수 있습니다.\n\r", ch);
        return TRUE;
    }
        
	if ( narg <= 0 ) {
        send_to_char("어떤 걸 찾으시려고요?.\n\r", ch ) ;
		return TRUE ;
    }

	if ( value > 0 ) {	/* search by number */
		if ( value > dangun_apart_list[room_idx].item_num ) {
			send_to_char("그런 물건 번호는 없습니다.\n\r", ch) ;
			return TRUE ;
        }
		p = &dangun_apart_list[room_idx].item;
		for ( n = 1 ; n < value ; n++ , p = p->next ) ;
        /* is it right ? */
    }
	else {	/* search by name */
		p = &dangun_apart_list[room_idx].item;
		for ( n = 0 ; p->next != NULL ; p = p->next ) {
			if ( isname(name, p->next->obj->name ))
				break ;
        }
		if ( p->next == NULL ) {
			send_to_char("그런 이름의 물건은 없습니다.\n\r", ch) ;
			return TRUE ;
        }
    }

	/* set save bit */
	dangun_apart_list[room_idx].item_save_bit = 1 ;

	/* buy now */
	if ( p->next->n > 1 ) {
		/* copy object .. */
		if ((obj = read_object(p->next->obj->item_number, REAL)) == 0 ) {
			log("apart_withdraw_buy: can't clone object") ;
			send_to_char("보관함에 문제가 생겼군요. Implementor에게 문의하세요.\n\r", ch) ;
			return TRUE ;
        }
        (p->next->n) -- ;

		obj->obj_flags.extra_flags = p->next->obj->obj_flags.extra_flags ;

		obj->obj_flags.value[0] = p->next->obj->obj_flags.value[0] ;
		obj->obj_flags.value[1] = p->next->obj->obj_flags.value[1] ;
		obj->obj_flags.value[2] = p->next->obj->obj_flags.value[2] ;
		obj->obj_flags.value[3] = p->next->obj->obj_flags.value[3] ;

		obj->affected[0].location= p->next->obj->affected[0].location ;
		obj->affected[0].modifier= p->next->obj->affected[0].modifier ;
		obj->affected[1].location= p->next->obj->affected[1].location ;
		obj->affected[1].modifier= p->next->obj->affected[1].modifier ;
		/* do not decrease gom_item_number */
    }
	else {	/* remove from list : but do not free obj in tmpp */
		obj = p->next->obj ;
		tmpp = p->next ;
		p->next = p->next->next ;
		dangun_apart_list[room_idx].item_num--;

		free_string_type((char*)tmpp) ;
    }

	act("$p를 보관함에서 찾았습니다.", 0, ch, obj, 0, TO_CHAR) ;
	act("$n님이 $p를 찾았습니다.", 0, ch, obj, 0, TO_ROOM) ;

	if ((1+IS_CARRYING_N(ch)) > CAN_CARRY_N(ch)) {
		act("손에 든게 너무 많아 $n이 $p를 내려 놓습니다.", 0, ch, obj, 0, TO_ROOM) ;
		obj_to_room(obj, ch->in_room) ;
    }
	else if (GET_OBJ_WEIGHT(obj) + IS_CARRYING_W(ch) > CAN_CARRY_W(ch)) {
		act("$p가 너무 무거워 $n께서 내려 놓습니다.", 0, ch, obj, 0, TO_ROOM) ;
		obj_to_room(obj, ch->in_room) ;
    }
	else {	/* give it to player */
		obj_to_char(obj, ch) ;
    }

    // 물건들을 apart 저장함에다가 저장
    // apart_item_save(room_idx);

    sprintf(buf, "%d개의 공간이 보관함에 남아 있습니다.\n\r",
            dangun_apart_list[room_idx].max_item -
            dangun_apart_list[room_idx].item_num);
    send_to_char(buf, ch);

	return TRUE ;
}

int apart_list(struct char_data *ch, int room_idx,
               int narg, int value, char *name)
{
	struct apart_item_data	*p ;
	int	page = 0, n, count ;
	char	buf[BUFSIZ], buf2[BUFSIZ] ;


    if (dangun_apart_list[room_idx].item_num <= 0) {
        send_to_char("보관함에는 아무것도 없습니다.\n\r", ch);
		return TRUE ;
    }

    if ( narg == 0 ) {
        send_to_char("Usage : list < page number | item name >\n\r", ch);
        return TRUE;
    }
        
    if ( value <= 0 ) {   /* list by item name */
        char tmpbuf[BUFSIZ];
        int found =0;
        
        p = &dangun_apart_list[room_idx].item;

        tmpbuf[0] = 0;
		for ( n = 1 ; p->next != NULL ; p = p->next, n++ ) {
			if ( isname(name, p->next->obj->name )) {
                if ( p->next->obj == NULL ) {
                    log("apaart_list: null object !!") ;
                    break ;
                }
                found = 1;
                
                sprintf(buf2, " [%3d] %2d 개의  %s.\n\r", n,
                        p->next->n,
                        OBJS(p->next->obj, ch));
                strcat(tmpbuf, buf2) ;
            }
        }
        if (found) {
            strcpy(buf, "다음과 같은 물건들이 보관함에 있습니다 :\n\r") ;
            strcat(buf, tmpbuf);
        }
        else {
            strcpy(buf, "그런 종류의 item은 없습니다. \n\r");
        }

        page_string(ch->desc, buf, 0);

        return TRUE ;
    }

    /* list by item number */
    if ( value > 0 ) page = value;

	if ( dangun_apart_list[room_idx].item_num <= (page - 1) * ONE_PAGE_LINES ) {
		send_to_char("Page 숫자가 너무 큽니다. 보다 작은 수를 쓰세요.\n\r", ch) ;
		return TRUE ;
    }

	p = dangun_apart_list[room_idx].item.next;
	for ( n = 1 ; n <= (page-1) * ONE_PAGE_LINES && p != NULL ; n++ )
		p = p->next ;

    strcpy(buf, "다음과 같은 물건들이 보관함에 있습니다 :\n\r") ;
	count = 0 ;
	for ( ; count < ONE_PAGE_LINES && p != NULL ; n++, p = p->next, count++ ) {
		if ( p->obj == NULL ) {
			log("apart_list: null object !!") ;
			break ;
        }
		sprintf(buf2, " [%3d] %2d 개의  %s.\n\r", n, p->n, OBJS(p->obj, ch));
		strcat(buf, buf2) ;
    }

	if ( p != NULL ) {
		sprintf(buf2, " *** 다음 page 는 'list %d' 입니다. ***\n\r", page+1) ;
		strcat(buf, buf2) ;
    }

	send_to_char(buf, ch) ;

    act("$n님이 보관함을 조사합니다", 0, ch, 0, 0, TO_ROOM) ;

	return TRUE ;
}

void init_apart_control_file()
{
	FILE	*fp ;
	char	line[BUFSIZ] ;
    int     virtual_nr;
    int     n_room = 0;
    int     flag, i;
    char    *temp;

	log("Load apartment control file") ;
	sprintf(line, "%s/%s", APARTMENT_DIR, CONTROL_FILE) ;

	if ((fp = fopen(line, "r")) == NULL) {
		log("apartment control file open failed") ;
		if ((fp = fopen(line, "w")) == NULL) {
			log("init_apartment: making apartment control file failed!") ;
			return ;
        }
		else {	/* new file is made */
			log("init_apartment: new file is made") ;

			fclose(fp);

            // 파일로 저장하게
            //
            apart_control_save_bit = 1;
            
			// 방 초기화
			for (i=0; i < MAX_APARTMENT; i++) {
				dangun_apart_list[i].room_num = -1;
			}
    
			return;
        }
    }

    // 아파트에 대한 control 정보를 읽어 들임
    do
    {
        fscanf(fp, "#%d ", &virtual_nr);
        if ( n_room > MAX_APARTMENT ) {
            log("init_apart_control_file: number of world over flow.") ;
            exit(1) ;
		}
        temp = fread_string(fp);
        if ((flag = (*temp != '$')))  /* a new record to be read */
        {
            dangun_apart_list[n_room].owner = temp;
            dangun_apart_list[n_room].room_num = virtual_nr;
            dangun_apart_list[n_room].item_save_bit = 0;

            fscanf(fp, "%d %d ", &dangun_apart_list[n_room].max_item,
                   &dangun_apart_list[n_room].room_flags);
            
            dangun_apart_list[n_room].description = fread_string(fp);

            n_room++;
        }
    }
    while (flag);

    /* cleanup the area containing the terminal $  */
    free_string_type(temp);

    // 나머지 방에 대한 정보는 빈 방으로 
    for (i=n_room; i < MAX_APARTMENT; i++) {
        dangun_apart_list[i].room_num = -1;
    }
    
	fclose(fp) ;
}

void apart_control_save()
{
	FILE	*fp ;
	char	line[BUFSIZ] ;
    int i;

    log("Writing apartment control file") ;
    
	sprintf(line, "%s/%s", APARTMENT_DIR, CONTROL_FILE) ;
    if ((fp = fopen(line, "w")) == NULL) {
		log("apart_control_save : file open failed") ;
		return ;
    }
    for (i=0; i < MAX_APARTMENT; i++) {
        // 정보가 존재하고 있는 경우에 저장
        if (dangun_apart_list[i].room_num != -1) {
            sprintf(line, "#%d \n", dangun_apart_list[i].room_num);
            fputs(line, fp);

			// owner 정보 기록
			if (dangun_apart_list[i].owner) {
				fputs(dangun_apart_list[i].owner, fp);
				log(dangun_apart_list[i].owner);
			}
			else {
				log("Error: Invalid apartment owner");
			}
			fputs("~\n", fp);

			sprintf(line, "%d %d \n", dangun_apart_list[i].max_item,
					   dangun_apart_list[i].room_flags);
			fputs(line, fp);

			log(line);

			// descritpion 기록
			if (dangun_apart_list[i].description) {
				fputs(dangun_apart_list[i].description, fp);
				// log(dangun_apart_list[i].description);
			}
			else {
				log("Error: Invalid apartment description");
			}
			fputs("~\n", fp);
		}
    }
    fputs("#20000\n",fp);
    fputs("$~\n", fp);
    
	fclose(fp) ;

    apart_control_save_bit = 0;
}

/* save but do not destroy .. */
void apart_item_save(int room_idx)
{
	struct apart_item_data	*p ;
	FILE	*fp ;
	char	line[BUFSIZ] ;
	int	count ;

    sprintf(line, "%s/room%d", APARTMENT_DIR,
            dangun_apart_list[room_idx].room_num) ;

	if ((fp = fopen(line, "w")) == NULL) {
		log("apart_item_save : file open failed") ;
		return ;
    }
    strcat(line, "... saving");
    log(line);

	p = dangun_apart_list[room_idx].item.next;
	for ( count = 0 ; p != NULL ; p = p->next ) {
		if ( apart_item_to_str(line, p) == 0 ) {	/* success */
			strcat(line, "\n") ;
			fputs(line, fp) ;
			count ++ ;
        }
    }
	fclose(fp) ;

    dangun_apart_list[room_idx].item_save_bit = 0;
}

void apart_save()
{
	int	i ;

    if (apart_control_save_bit) apart_control_save();
    
	for ( i = 0 ;
          i < MAX_APARTMENT && dangun_apart_list[i].room_num != -1; i++ ) {
        if (dangun_apart_list[i].item_save_bit) {
            apart_item_save(i);
        }
    }
}

/* item to string, but do not destroy object */
int apart_item_to_str(char *str, struct apart_item_data *mar)
{
	extern struct index_data *obj_index;

	if ( *str == 0 ) {
		log("apart_item_to_str: no strage type string") ;
		return (-1) ;
    }

	/* save as virtual number */

	if ( mar->id != obj_index[mar->obj->item_number].virtual ) {
		log("market_item_to_str: item number mismatch") ;
		return (-1) ;
    }

	sprintf(str, "%d %d %d %d %d %d %d %d %d %d %d",
            mar->n,	/* number of items */
            mar->id,	/* save as virtual number */
            mar->obj->obj_flags.extra_flags,
            mar->obj->obj_flags.value[0], mar->obj->obj_flags.value[1],
            mar->obj->obj_flags.value[2], mar->obj->obj_flags.value[3],
            mar->obj->affected[0].location, mar->obj->affected[0].modifier,
            mar->obj->affected[1].location, mar->obj->affected[1].modifier);

	return 0 ;
}

int apart_str_to_item(struct apart_item_data *mar, char *str)
{
	struct obj_data	*obj ;
	int	n, id, eflag, value[4], tmp[4] ;
	int ret = 0;
	char	buf[BUFSIZ] ;

	/* wear flag 는 필요 없다.. */
	/* extra flag, restore value[0-3], affected */

	if (str == NULL)	/* failed */
		return (-1) ;

	if ( (ret = sscanf(str, "%d %d %d %d %d %d %d %d %d %d %d", 
				&n, &id, &eflag,
                &value[0], &value[1], &value[2], &value[3], 
                &tmp[0], &tmp[1], &tmp[2], &tmp[3]) < 10) ) {
		/* in sufficient data */
		log("apart_str_to_item: string corrupted") ;
		return (-1) ;
    }

	if ((obj = read_object(id, VIRTUAL)) == NULL) {	/* no such item */
		sprintf(buf, "apart_str_to_item : no such item %d", id) ;
		log(buf) ;
		return (-1) ;
    }

	obj->obj_flags.extra_flags = eflag ;

	obj->obj_flags.value[0] = value[0] ;
	obj->obj_flags.value[1] = value[1] ;
	obj->obj_flags.value[2] = value[2] ;
	obj->obj_flags.value[3] = value[3] ;

	obj->affected[0].location=tmp[0];
	obj->affected[0].modifier=tmp[1];
	obj->affected[1].location=tmp[2];
	obj->affected[1].modifier=tmp[3];

	mar->n = MIN(3, n);
	mar->id = id ; 
	mar->hash = id_to_hash(obj) ;
	mar->obj = obj ;
	mar->next = NULL ;

	return 0 ;
}

void init_room(int room_idx)
{
    struct apart_item_data	*newitem ;
    struct obj_data *obj;
	FILE	*fp ;
	char	line[BUFSIZ];
    int     virtual_nr;

    // 각 방의 description 변경
    //

    virtual_nr = dangun_apart_list[room_idx].room_num;
    
    sprintf(line, "%s의 방", dangun_apart_list[room_idx].owner);
    free_string_type(world[real_room(virtual_nr)].name);
    world[real_room(virtual_nr)].name = string_dup(line);

	if (dangun_apart_list[room_idx].description) {
		free_string_type(world[real_room(virtual_nr)].description);
    	world[real_room(virtual_nr)].description =
       	 	string_dup(dangun_apart_list[room_idx].description);
	}

    // sundew를 샀다면 방에 나타나게 해야
    // 
    if (IS_SET(dangun_apart_list[room_idx].room_flags, APT_SUNDEW)) 
    {
        if (( obj = read_object(1000, VIRTUAL)) == 0 ) {    /* recall scroll */
            log("init_apt_room: fail to load sundew") ;
            return;
        }
        obj_to_room(obj, real_room(dangun_apart_list[room_idx].room_num));
    }
	sprintf(line, "%s/room%d", APARTMENT_DIR,
            dangun_apart_list[room_idx].room_num) ;

	if ((fp = fopen(line, "r")) == NULL) {
		if ((fp = fopen(line, "w")) == NULL) {
			log("init_room: making room stash file failed. give up!") ;
			return ;
        }
		else {	/* new file is made */
			log("init_room: new file is made") ;
			fclose(fp) ;

            dangun_apart_list[room_idx].item.next = NULL;
            dangun_apart_list[room_idx].item_num = 0;
            
			return ;
        }
    }

    dangun_apart_list[room_idx].item.next = NULL;
    dangun_apart_list[room_idx].item_num = 0;

	while( fgets(line, BUFSIZ-1, fp) != NULL ) {
		newitem = (struct apart_item_data *)malloc_string_type (
			sizeof(struct apart_item_data));
		if ( apart_str_to_item(newitem, line) == 0 ) {	/* success */
			newitem->next = dangun_apart_list[room_idx].item.next ;
			dangun_apart_list[room_idx].item.next = newitem ;
			dangun_apart_list[room_idx].item_num++;
        }
		else
			free_string_type((char*)newitem) ;
    }

	sprintf(line, "init_room(%s) : %d item read",
            dangun_apart_list[room_idx].owner,
            dangun_apart_list[room_idx].item_num);
	log(line) ;

	fclose(fp) ;
}

void init_apartment()
{
    int     i;
    
    init_apart_control_file();

    for (i=0; i < MAX_APARTMENT && dangun_apart_list[i].room_num != -1; i++)
    {
        init_room(i);
    }
}

/* returns home : if fails return (-1) */
int check_room_idx(struct char_data *ch)
{
	int	i ;

	for ( i = 0 ;
          i < MAX_APARTMENT && dangun_apart_list[i].room_num != -1; i++ ) {
        if (!strcasecmp(GET_NAME(ch), dangun_apart_list[i].owner)) {
            /* if real room isn't exist, it returns (-1) */
            return (i);
        }
    }
 
    return (-1) ;
}

int check_apart_room(int virtual)
{
	int	i ;

	for ( i = 0 ;
          i < MAX_APARTMENT && dangun_apart_list[i].room_num != -1; i++ ) {

        if (virtual == dangun_apart_list[i].room_num) {
            return (i);
        }
    }
 
    return (-1) ;
}

/* 수위실 */
int check_APT_room(struct char_data *ch, int cmd, char *arg)
{
	extern struct room_data *world ;
	struct char_data	*vict ;
	struct follow_type	*fol ;
	int	old_room, home, ngroup, room_idx;
    int i;

    if (IS_NPC(ch))
        return(FALSE);

	if ( cmd != COM_ENTER && cmd != COM_EXAMINE)
		return FALSE ;

    if (cmd == COM_EXAMINE) {
        // 아파트 입주자 list를 보여줌 

        send_to_char("아파트 입주자 list:\n\r", ch);

        for ( i = 0 ;
              i < MAX_APARTMENT && dangun_apart_list[i].room_num != -1; i++ ) {
            send_to_char(dangun_apart_list[i].owner, ch);
			send_to_char(" ", ch);
        }
		send_to_char("\n\r", ch);
        return TRUE;
    }
    
	if (( room_idx = check_room_idx(ch)) < 0 ) {
		send_to_char_han("You can't enter without permission.\n\r",
                         "주인의 허락없이는 들어갈 수 없습니다.\n\r", ch) ;
		return TRUE ;
    }
    home = (real_room(dangun_apart_list[room_idx].room_num)) ;
    
	old_room = ch->in_room ;

	/* move to home */
	send_to_char("당신의 머리위로 푸른 빛이 내려옵니다.\n\r", ch) ;
	act("위에서 푸른 빛이 내려와 방안을 밝게 비춥니다.", FALSE,
		ch, 0, 0, TO_ROOM);
	char_from_room(ch) ;
	char_to_room(ch, home) ;
	send_to_char("당신의 방에 도착 했습니다.\n\r", ch) ;
	// do_look(ch, "", 0) ;

	ngroup = 1 ;
	if ( IS_AFFECTED(ch, AFF_GROUP) ) {	/* move his/her group members */
		for ( fol = ch->followers ; fol ; fol = fol->next ) {
			if ( fol->follower->in_room == old_room &&
                 !IS_NPC(fol->follower) &&
                 IS_AFFECTED(fol->follower, AFF_GROUP) ) {	/* same group */
				ngroup ++ ;
				char_from_room(fol->follower) ;
				send_to_char("주인의 방으로 이동합니다.\n\r", fol->follower);
				char_to_room(fol->follower, home) ;
				act("$n님이 방가운데 나타났습니다.", FALSE,
					fol->follower, 0, 0, TO_ROOM);
				// do_look(fol->follower, "", 0);
            }
        }
    }

	vict = world[old_room].people ;
	if ( vict ) {	/* someone remains here */
		if ( ngroup == 1 ) {
			send_to_room("옆에 있던 사람이 갑자기 사라집니다.\n\r", old_room) ;
        }
		else {
			send_to_room("옆에 있던 사람들이 갑자기 사라집니다.\n\r", old_room) ;
        }
    }

	return TRUE ;
}

int find_empty_slot()
{
	int	i ;

	for ( i = 0 ;i < MAX_APARTMENT; i++ ) {
        if (dangun_apart_list[i].room_num == -1) {
            return (i);
        }
    }
 
    return (-1) ;
}


/*
  아파트 관리 사무소
 */
#define APT_BUY_EXP         100
#define APT_BUY_GOLD        50

int apart_buy_room(struct char_data *ch, int cmd, char *arg)
{
    int room_idx;
    
    if (IS_NPC(ch)) return FALSE;

    if (cmd != COM_BUY) return FALSE;
    
    if (GET_LEVEL(ch) < IMO-1) {
        send_to_char("단군만이 아파트를 가질 수 있습니다.\n\r", ch);
        return TRUE;
    }
    
    if (check_room_idx(ch) >= 0 ) {
        send_to_char("아파트는 한채 이상 가질 수 없습니다.\n\r",
                     ch) ;
        return TRUE ;
    }

    room_idx = find_empty_slot();
    if (room_idx < 0) {
        send_to_char("아파트 분양이 다 끝났습니다.\n\r다음 기회를 이용하세요.\n\r", ch);
        return TRUE ;
    }

    // 아파트 방 번호(8010-8020)
    //
    dangun_apart_list[room_idx].room_num = 8010 + room_idx;
    dangun_apart_list[room_idx].owner = string_dup(GET_NAME(ch));
    dangun_apart_list[room_idx].description =
        string_dup("방 묘사는 write 명령으로 바꿀 수 있습니다\n\r");
    dangun_apart_list[room_idx].max_item = 1;       // 처음은 하나만
    dangun_apart_list[room_idx].item_num = 0;       // 없는 상태
    dangun_apart_list[room_idx].room_flags = 0;     // 초기화
    dangun_apart_list[room_idx].item_save_bit = 0;

    apart_control_save_bit = 1;     // control정보 save bit

    // 아파트 방 설정 
    init_room(room_idx);
    
    send_to_char("축하합니다.\n\r", ch);
    send_to_char("아파트를 한채 분양 받았습니다.\n\r", ch);
    
    return TRUE;
}

int apart_manage_room(struct char_data *ch, int cmd, char *arg)
{
    int room_idx;
    char buf[MAX_STRING_LENGTH];
    char buf2[MAX_STRING_LENGTH];
    int opt;

    if (IS_NPC(ch))
        return(FALSE);

    if (cmd== COM_LIST ) { /* List */
        if (( room_idx = check_room_idx(ch)) < 0 ) {
            send_to_char("당신은 아직 아파트를 소유하지 않았습니다.\n\r",
                             ch) ;
            return TRUE ;
        }

        strcpy(buf, "1 - 보관함 갯수 늘리기(50개까지)\n\r");
        strcat(buf, "    5M exp당 하나의 보관함을 늘릴 수 있습니다\n\r\n\r");
		strcat(buf, "2 - Sundew 구비\n\r");
        strcat(buf, "    10M gold - 방에 놓아둘 Sundew.\n\r\n\r");
		strcat(buf, "3 - 보관함 아이템 identify\n\r");
        strcat(buf, "    50M exp - 보관함 물건을 identify할 수 있습니다.\n\r");
		send_to_char(buf, ch) ;

        return TRUE;
    }
    else if (cmd == COM_BUY) { /* Buy */
        if (( room_idx = check_room_idx(ch)) < 0 ) {
            send_to_char("당신은 아직 아파트를 소유하지 않았습니다.\n\r",
                             ch) ;
            return TRUE ;
        }

        half_chop(arg,buf,buf2);
        /* arg = one_argument(arg, buf); */
        opt = atoi(buf); 
        if (opt == 1) {
            // 보관함 갯수 늘리기
            if( 5*1000000 > GET_EXP(ch) ) {
                send_to_char("Come back when you are more experienced.\n\r",ch);
                return(TRUE);
            }
            if (dangun_apart_list[room_idx].max_item < 50) 
                dangun_apart_list[room_idx].max_item++;
            ch->points.exp-= 5*1000000;

            save_char(ch, NOWHERE) ;
            apart_control_save_bit = 1;

            sprintf(buf, "이제 당신 방의 보관함의 갯수는 %d개 입니다.\n\r",
                    dangun_apart_list[room_idx].max_item);
            send_to_char(buf, ch);
            return TRUE;
        }
        else if (opt == 2) {
            if( 10*1000000 > GET_GOLD(ch) ) {
                send_to_char("Come back when you have more gold.\n\r",ch);
                return(TRUE);
            }
            SET_BIT(dangun_apart_list[room_idx].room_flags, APT_SUNDEW);
            ch->points.gold-= 10*1000000;

            save_char(ch, NOWHERE) ;
            apart_control_save_bit = 1;

            send_to_char("아파트에 Sundew를 설치하는 작업을 시작하겠습니다.\n\r", ch);
            send_to_char("내일 쯤이면 Sundew가 설치되어 있을겁니다.\n\r", ch);

            return TRUE;
        }
        else if (opt == 3) {
            if( 50*1000000 > GET_EXP(ch) ) {
                send_to_char("Come back when you are more experienced.\n\r",ch);
                return(TRUE);
            }
            SET_BIT(dangun_apart_list[room_idx].room_flags, APT_IDENTIFY);
            ch->points.exp-= 50*1000000;

            save_char(ch, NOWHERE) ;
            apart_control_save_bit = 1;
            
            send_to_char("보관함에 물건을 identify할 수 있는 기계를 설치했습니다.\n\r", ch);

            return TRUE;
        }
    }
    
	return FALSE;
}

