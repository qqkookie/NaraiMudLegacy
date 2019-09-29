/*
	market.c
*/
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <assert.h>

#include "structs.h"
#include "comm.h"
#include "handler.h"
#include "db.h"
#include "interpreter.h"
#include "utils.h"
#include "command.h"

/* sell/buy ratio */
#define	FACTOR_GOMULSANG	5
/* gomulsang price modifier */
#define GOMULSANG_PRICE     8

#define	MARKET_DIR	"market"
#define	ROOM_GOMULSANG	3075

void gomulsang_save();

struct market_item_data {
	int	n ;
	int	id ;
	int	hash ;
	int	cost ;
    time_t touched_time;        // add by jmjeong
	struct obj_data	*obj ;
	struct market_item_data	*next ;
} ;

struct market_item_data	gom_item_head ;
int	gom_item_number = 0 ;
int	gomulsang_save_bit = 0 ;


extern struct room_data *world;
int item_cost(struct obj_data *o) ;
int is_all_digit(char *str) ;
int id_to_hash(struct obj_data *obj) ;
int id_to_hash(struct obj_data *obj);

int gomulsang(struct char_data *ch, int cmd, char *arg)
{
	struct char_data	*keeper, *next ;
    char  name[BUFSIZ] ;
	int	narg, nvalue, status ;

    if ( cmd != COM_LIST && cmd != COM_VALUE && cmd != COM_BUY
         && cmd != COM_SELL && cmd != COM_IDENTIFY )
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

	/* find the keeper */
	for ( keeper = world[ch->in_room].people ; keeper ; keeper = next ) {
		next = keeper->next_in_room ;
		if ( IS_NPC(keeper) && keeper->nr == real_mobile(3095))
			break ;
    }

	if ( keeper == NULL ) {
		send_to_char("고물상 할아버지가 지금 안계십니다.\n\r", ch) ;
		return TRUE ;
    }

	switch(cmd) {
    case COM_LIST :
        status = gom_list(ch, keeper, narg, nvalue, name) ;
        break ;
    case COM_VALUE :
        status = gom_value(ch, keeper, narg, nvalue, name) ;
        break ;
    case COM_BUY :
        status = gom_buy(ch, keeper, narg, nvalue, name) ;
        break ;
    case COM_SELL :
        status = gom_sell(ch, keeper, narg, nvalue, name) ;
        break ;
    case COM_IDENTIFY :
        status = gom_identify(ch, keeper, narg, nvalue, name);
        break;
    }

	return status ;
}

/* added by jmjeong */
int gom_identify(struct char_data *ch, struct char_data *keeper,
                 int narg, int value, char *name)
{
	struct market_item_data	*p, *tmpp ;
	struct obj_data	*obj ;
	int	n, cost, bargain ;
	char	buf[BUFSIZ] ;

	if ( narg <= 0 ) {
		send_to_char("할아버지께서 '뭘 알려고?' 하고 당신에게 묻습니다.\n\r",
                     ch ) ;
		return TRUE ;
    }

	if ( value > 0 ) {	/* search by number */
		if ( value > gom_item_number ) {
			send_to_char("할아버지께서 '그런 물건 번호는 없다.' 라고 말씀하십니다.\n\r",
                         ch ) ;
			return TRUE ;
        }
		p = &gom_item_head ;
		for ( n = 1 ; n < value ; n++ , p = p->next ) ;
        /* is it right ? */
    }
	else {	/* search by name */
		p = &gom_item_head ;
		for ( n = 0 ; p->next != NULL ; p = p->next ) {
			if ( isname(name, p->next->obj->name ))
				break ;
        }
		if ( p->next == NULL ) {
			if ( number(0,1) == 0 )
				send_to_char("할아버지께서 '그런 이름의 물건은 없는데...' 라고 말씀하십니다.\n\r",
                             ch ) ;
			else
				send_to_char("할아버지께서 '가만있자.. 그걸 광에 넣어 두었나 ?' 라고 말씀하십니다.\n\r",
                             ch ) ;
			return TRUE ;
        }
    }

	/* check player gold */

    assert(p->next);
	cost = MIN((p->next->cost * FACTOR_GOMULSANG / 20), 8000);
	p->next->touched_time = time(0);        		// touched time
	if ( GET_GOLD(ch) < cost ) {
		sprintf(buf, "알아보려면 %d원이 있어야 합니다.", cost);
		send_to_char(buf, ch);
		return TRUE ;
    }
    GET_GOLD(ch) -= cost ;
    sprintf(buf, "%d원을 주고 물건에 대해 조사합니다.\n\r\n\r", cost) ;
	send_to_char(buf, ch) ;
	act("$n 님이 $p 에 대해 조사합니다.\n\r", 0, ch, p->next->obj, 0, TO_ROOM) ;

    /* identify the object now */
    spell_identify(GET_LEVEL(ch), ch, 0, p->next->obj);

	return TRUE ;
}

#define	ONE_PAGE_LINES	18

int gom_list(struct char_data *ch, struct char_data *keeper,
             int narg, int value, char *name)
{
	struct market_item_data	*p ;
	int	page, n, count ;
	char	buf[BUFSIZ], buf2[BUFSIZ] ;

    if ( gom_item_number <= 0 ) {
		do_sayh(keeper, "흐흠.. 팔것이 아무것도 없네..", 0) ;
		return TRUE ;
    }

    if ( narg == 0 ) {
        send_to_char("Usage : list < page number | item name >\n\r", ch);
        return TRUE;
    }
        
    if ( value <= 0 ) {   /* list by item name */
        char tmpbuf[BUFSIZ];
        int found =0;
        
        p = &gom_item_head ;

        tmpbuf[0] = 0;
		for ( n = 1 ; p->next != NULL ; p = p->next, n++ ) {
			if ( isname(name, p->next->obj->name )) {
                if ( p->next->obj == NULL ) {
                    log("gom_list: null object !!") ;
                    break ;
                }
                found = 1;
                
                sprintf(buf2, " [%3d] %2d 개의  %s  -  %d 원.\n\r", n,
                        p->next->n,
                        OBJS(p->next->obj, ch),
                        p->next->cost * FACTOR_GOMULSANG ) ;
                strcat(tmpbuf, buf2) ;
            }
        }
        if (found) {
            strcpy(buf, "다음과 같은 물건들이 있습니다 :\n\r") ;
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

	if ( gom_item_number <= (page - 1) * ONE_PAGE_LINES ) {
		send_to_char("Page 숫자가 너무 큽니다. 보다 작은 수를 쓰세요.\n\r", ch) ;
		return TRUE ;
    }

	p = gom_item_head.next ;
	for ( n = 1 ; n <= (page-1) * ONE_PAGE_LINES && p != NULL ; n++ )
		p = p->next ;

	strcpy(buf, "다음과 같은 물건들이 있습니다 :\n\r") ;
	count = 0 ;
	for ( ; count < ONE_PAGE_LINES && p != NULL ; n++, p = p->next, count++ ) {
		if ( p->obj == NULL ) {
			log("gom_list: null object !!") ;
			break ;
        }
		sprintf(buf2, " [%3d] %2d 개의  %s  -  %d 원.\n\r", n, p->n,
                OBJS(p->obj, ch), p->cost * FACTOR_GOMULSANG ) ;
		strcat(buf, buf2) ;
    }

	if ( p != NULL ) {
		sprintf(buf2, " *** 다음 page 는 'list %d' 입니다. ***\n\r", page+1) ;
		strcat(buf, buf2) ;
    }

	send_to_char(buf, ch) ;
	return TRUE ;
}

int gom_value(struct char_data *ch, struct char_data *keeper,
              int narg, int value, char *name)
{
	struct obj_data *obj ;
	char	buf[BUFSIZ] ;
	int	cost ;

	if (narg <= 0) {
		send_to_char("usage: value <item name>\n\r", ch) ;
		return TRUE ;
    }

	obj = get_obj_in_list_vis(ch, name, ch->carrying) ;
	if ( obj == NULL ) {
		if ( number(0,1) == 0 )
			send_to_char("할아버지께서 '그런 물건이 어디 있지?' 라고 묻습니다.\n\r",
                         ch) ;
		else
			do_sayh(keeper, "주머니를 다시 한번 잘 찾아 보거라.", COM_SAY) ;
		return TRUE ;
    }

	if ((cost = GET_OBJ_RENTCOST(obj)) <= 0) {
		cost = item_cost(obj);
		GET_OBJ_RENTCOST(obj) = cost ;
    }
    cost *= GOMULSANG_PRICE;
    
	sprintf(buf, "할아버지께서 '음 그거라면 %d 원을 주마.' 라고 말씀하십니다.\n\r",
            cost) ;
	send_to_char(buf, ch) ;
	return TRUE ;
}

int gom_buy(struct char_data *ch, struct char_data *keeper,
            int narg, int value, char *name)
{
	extern struct str_app_type str_app[];	/* need in weight calculation */
	struct market_item_data	*p, *tmpp ;
	struct obj_data	*obj ;
	int	n, cost, bargain ;
	char	buf[BUFSIZ] ;

	if ( narg <= 0 ) {
		send_to_char("할아버지께서 '뭘 사려고?' 하고 당신에게 묻습니다.\n\r",
                     ch ) ;
		return TRUE ;
    }

	if ( value > 0 ) {	/* search by number */
		if ( value > gom_item_number ) {
			send_to_char("할아버지께서 '그런 물건 번호는 없다.' 라고 말씀하십니다.\n\r",
                         ch ) ;
			return TRUE ;
        }
		p = &gom_item_head ;
		for ( n = 1 ; n < value ; n++ , p = p->next ) ;
        /* is it right ? */

    }
	else {	/* search by name */
		p = &gom_item_head ;
		for ( n = 0 ; p->next != NULL ; p = p->next ) {
			if ( isname(name, p->next->obj->name ))
				break ;
        }
		if ( p->next == NULL ) {
			if ( number(0,1) == 0 )
				send_to_char("할아버지께서 '그런 이름의 물건은 없는데...' 라고 말씀하십니다.\n\r",
                             ch ) ;
			else
				send_to_char("할아버지께서 '가만있자.. 그걸 광에 넣어 두었나 ?' 라고 말씀하십니다.\n\r",
                             ch ) ;
			return TRUE ;
        }
    }

	/* check player gold */
	cost = p->next->cost * FACTOR_GOMULSANG ;
	if ( GET_GOLD(ch) < cost ) {
		do_sayh(keeper, "돈이 모자라면 다음에 오거라.", COM_SAY) ;
		return TRUE ;
    }

	/* set save bit */
	gomulsang_save_bit = 1 ;

	/* buy now */
	if ( p->next->n > 1 ) {
		(p->next->n) -- ;
        p->next->touched_time = time(0);        // touched time
		/* copy object .. */
		if ((obj = read_object(p->next->obj->item_number, REAL)) == 0 ) {
			log("gom_buy: can't clone object") ;
			do_sayh(keeper, "오늘은 허리가 아파서 장사를 못하겠다.. ", COM_SAY) ;
			return TRUE ;
        }
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
		gom_item_number -- ;

		free_string_type(tmpp) ;
    }

	/* give it to player : 조금 깍아주기도 한다.. 엿도 주고.. */
	/* player 가 무거워서 못들면, 땅에 내려 놓는다. */
	bargain = number(3, 25) ;
	if ( bargain > 19 ) {
		do_sayh(keeper, "허허허.. 오늘은 기분이 좋으니 왕창 깎아주마.", COM_SAY);
    }
	else if ( bargain > 10 ) {
		do_sayh(keeper, "네가 맘에들어 많이 깎아주었다.", COM_SAY);
    }
	else {
		do_sayh(keeper, "그래도 조금은 깎아줘야 맛이지...", COM_SAY);
    }
		
	cost = cost - cost * number(bargain/2, bargain) / 100 ;
	GET_GOLD(ch) -= cost ;
	sprintf(buf, "돈을 %d원을 주고 삽니다.\n\r", cost) ;
	send_to_char(buf, ch) ;
	act("$n 님이 $p 를 삽니다.\n\r", 0, ch, obj, 0, TO_ROOM) ;

	if ((1+IS_CARRYING_N(ch)) > CAN_CARRY_N(ch)) {
		do_sayh(keeper, "손에 든게 너무 많으니 땅에 내려 놓겠다.", COM_SAY) ;
		act("$n께서 $p 를 내려 놓습니다.", 0, keeper, obj, ch, TO_ROOM) ;
		obj_to_room(obj, ch->in_room) ;
    }
	else if (GET_OBJ_WEIGHT(obj) + IS_CARRYING_W(ch) > CAN_CARRY_W(ch)) {
		do_sayh(keeper, "이건 좀 무거우니 땅에 내려 놓겠다.", COM_SAY) ;
		act("$n께서 $p 를 내려 놓습니다.", 0, keeper, obj, ch, TO_ROOM) ;
		obj_to_room(obj, ch->in_room) ;
    }
	else {	/* give it to player */
		obj_to_char(obj, ch) ;
    }

	return TRUE ;
}

/*
	market에 똑 같은 물품이 있는지 찾아본다
	있으면 해당되는 item pointer를 반환한다.
*/
struct market_item_data *find_item_from_market(struct obj_data *obj)
{
	extern struct index_data *obj_index;
	struct market_item_data *p;
	int hash, cost;

	/* 여기서는 물건 값을 다시 확인 해야 함.. enchant 되어있는지 등등. */
	cost = item_cost(obj);
	GET_OBJ_RENTCOST(obj) = cost ;
    cost *= GOMULSANG_PRICE;
    
	/* 같은 물건이 있는지 찾는다.. */
	p = gom_item_head.next ;
	hash = id_to_hash(obj) ;

	for ( ; p != NULL ; p = p->next ) {
		if ( p->id == obj_index[obj->item_number].virtual ) {
			if ( hash == p->hash && cost == p->cost ) {
				/* found */
				return p;
            }
        }
    }
	return NULL;
}

int gom_sell(struct char_data *ch, struct char_data *keeper,
             int narg, int value, char *name)
{
	extern struct index_data *obj_index;
	struct market_item_data	*p, *newitem ;
	struct obj_data *obj, *tmp_obj, *next_obj ;
	char	buf[BUFSIZ] ;
	int	cost, hash, has_something ;

	if ( narg <= 0 ) {
		send_to_char("할아버지께서 '무엇을 팔려고 ?' 하고 묻습니다.\n\r", ch ) ;
		return TRUE ;
    }

	obj = get_obj_in_list_vis(ch, name, ch->carrying) ;
	if ( obj == NULL ) {
		if ( number(0,1) == 0 )
			send_to_char("할아버지께서 '그 물건을 먼저 가져오너라..' 하고 말씀하십니다.\n\r",
                         ch) ;
		else
			do_sayh(keeper, "주머니에 구멍이 났나 보구나. 쯧쯧...", COM_SAY) ;
		return TRUE ;
    }

	/* 팔수 없는 물건들.. corpse, key */
	/* No rent item 은 여기에서 팔도록 허가 함 - 허가 하지 않게 바꿈  */
	if ((GET_ITEM_TYPE(obj) == ITEM_CONTAINER && obj->obj_flags.value[3] == 1)
		|| GET_ITEM_TYPE(obj) == ITEM_KEY
		|| IS_OBJ_STAT(obj, ITEM_NO_RENT) ) {
		send_to_char("할아버지께서 '그런 물건은 안받는다' 라고 말씀하십니다.\n\r",
                     ch ) ;
		return TRUE ;
    }
    
	if ((p = find_item_from_market(obj)) && p->n >= number(2,3)) {
        switch (number(1,6))
        {
        case 1:
        case 2:
            send_to_char("할아버지께서 '지금은 창고정리중이란다.' 라고 말씀하십니다.\n\r", ch);
            break;
        case 3:
            send_to_char("할아버지께서 '음.. 그 물건은 안 산다.' 라고 말씀하십니다.\n\r", ch);
            break;
        default:
            send_to_char("할아버지께서 '그 물건은 재고가 너무 많단다.' 라고 말씀하십니다.\n\r", ch);
            break;

        }
        p->touched_time = time(0);  // reset touched time
      
		return TRUE;
	}

	/* 물건 판다는 message.. */
	obj_from_char(obj) ;

	/* 여기서는 물건 값을 다시 확인 해야 함.. enchant 되어있는지 등등. */
	cost = item_cost(obj);
	GET_OBJ_RENTCOST(obj) = cost ;
    cost *= GOMULSANG_PRICE;
    
	sprintf(buf, "당신은 %d 원을 받고 그 물건을 팔았습니다.\n\r", cost) ;
	send_to_char(buf, ch) ;
	GET_GOLD(ch) += cost ;
	act("$n 님이 $p 를 팝니다.", 0, ch, obj, keeper, TO_ROOM) ;

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
			do_sayh(keeper, "으잉? 근데 이게 뭐야 ?", COM_SAY) ;
			act("$n께서 속안에 든 물건을 바닥에 쏟아 놓습니다.", 0, keeper,
				0, 0, TO_ROOM) ;
        }
    }

	/* 다음번에 save 하도록 bit 를 set 한다 */
	gomulsang_save_bit = 1;

	/* 같은 물건이 있는지 찾는다.. */
	if (p = find_item_from_market(obj)) {
		/* same !! Do not increase gom_item_number */
		( p->n )++;
        p->touched_time = time(0);  // reset touched time
		extract_obj(obj);
		return TRUE;
	}

	/* Not found : add as new one */
	newitem = (struct market_item_data *) malloc_string_type (
		sizeof(struct market_item_data)) ;
	newitem->n = 1 ;
	newitem->id = obj_index[obj->item_number].virtual ;
	newitem->hash = id_to_hash(obj);
	newitem->cost = cost ;
	newitem->obj = obj ;
    newitem->touched_time = time(0);
	newitem->next = gom_item_head.next ;
	gom_item_head.next = newitem ;
	gom_item_number ++ ;

	return TRUE ;

}

int id_to_hash(struct obj_data *obj)
{
	int	i, sum ;

	/*  Hash by object flag (extra flag), value[0-4], affected */
	/*  Just add them all - easy way */
	/*  Bitvector - do not need wear flag */

	sum = 0 ;
	sum += obj->obj_flags.value[0] ;
	sum += obj->obj_flags.value[1] ;
	sum += obj->obj_flags.value[2] ;
	sum += obj->obj_flags.value[3] ;
	sum += obj->obj_flags.extra_flags ;


	for ( i = 0 ; i < MAX_OBJ_AFFECT ; i ++) {
		sum += obj->affected[i].location ;
		sum += obj->affected[i].modifier ;
    }

	return sum ;
}

int is_all_digit(char *str)
{
	int	i, leng ;

	leng = strlen(str) ;
	for ( i = 0 ; i < leng ; i ++ ) {
		if ( !isdigit(str[i]) )
			return FALSE ;
    }

	return TRUE ;
}

void init_market()
{
	struct market_item_data	*newitem ;
	FILE	*fp ;
	char	line[BUFSIZ] ;

	log("Load saved items to Market") ;
	sprintf(line, "%s/room%d", MARKET_DIR, ROOM_GOMULSANG) ;

	if ((fp = fopen(line, "r")) == NULL) {
		log("init_market: gomulsang file open failed") ;
		if ((fp = fopen(line, "w")) == NULL) {
			log("init_market: making gomulsang file failed. give up!") ;
			return ;
        }
		else {	/* new file is made */
			log("init_market: new file is made") ;
			fclose(fp) ;

			gom_item_head.next = NULL ;
			gom_item_number = 0 ;
			return ;
        }
    }

	gom_item_head.next = NULL ;
	gom_item_number = 0 ;
	while( fgets(line, BUFSIZ-1, fp) != NULL ) {
		newitem = (struct market_item_data *) malloc_string_type (
			sizeof(struct market_item_data)) ;
		if ( market_str_to_item(newitem, line) == 0 ) {	/* success */
			newitem->next = gom_item_head.next ;
			gom_item_head.next = newitem ;
			gom_item_number ++ ;
        }
		else
			free_string_type(newitem) ;
    }

	sprintf(line, "init_market : %d item read", gom_item_number) ;
	log(line) ;

	fclose(fp) ;
}

/* save but do not destroy .. */
void gomulsang_save()
{
	struct market_item_data	*p ;
	FILE	*fp ;
	char	line[BUFSIZ] ;
	int	count ;

	if ( gomulsang_save_bit == 0 )	/* Need not save .. not changed */
		return ;

	log("saving gomulsang.") ;
	sprintf(line, "%s/room%d", MARKET_DIR, ROOM_GOMULSANG) ;
	if ((fp = fopen(line, "w")) == NULL) {
		log("gomulsang_save : file open failed") ;
		return ;
    }

	p = gom_item_head.next ;
	for ( count = 0 ; p != NULL ; p = p->next ) {
		if ( market_item_to_str(line, p) == 0 ) {	/* success */
			strcat(line, "\n") ;
			fputs(line, fp) ;
			count ++ ;
        }
    }

	fclose(fp) ;

	gomulsang_save_bit = 0 ;
	/*
      sprintf(line, "gomulsang_save: (%d/%d) line saved.", count, gom_item_number);
      log(line) ;
	*/
}

// 4 days
#define MARKET_DECAY_TIME           3

int market_str_to_item(struct market_item_data *mar, char *str)
{
	struct obj_data	*obj ;
	int	n, id, eflag, value[4], tmp[4] ;
	int ret = 0;
    long touched_time;
	char	buf[BUFSIZ] ;

	/* wear flag 는 필요 없다.. */
	/* extra flag, restore value[0-3], affected */

	if (str == NULL)	/* failed */
		return (-1) ;

	if ( (ret = sscanf(str, "%d %d %d %d %d %d %d %d %d %d %d %ld", 
				&n, &id, &eflag,
                &value[0], &value[1], &value[2], &value[3], 
                &tmp[0], &tmp[1], &tmp[2], &tmp[3], &touched_time)) < 11) {
		/* in sufficient data */
		log("market_str_to_item: string corrupted") ;
		return (-1) ;
    }
	else if ( ret == 11 ) {
		touched_time = time(0);
	}

    if ( (time(0) - touched_time)/3600 > 24*MARKET_DECAY_TIME) {
        sprintf(buf, "market_str_to_item : item %d is decayed", id);
        log(buf);
        return(-1);
    }
	if ((obj = read_object(id, VIRTUAL)) == NULL) {	/* no such item */
		sprintf(buf, "market_str_to_item : no such item %d", id) ;
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

	mar->n = MIN(n, 2) ;
	mar->id = id ; 
	mar->hash = id_to_hash(obj) ;
	mar->cost = item_cost(obj) * GOMULSANG_PRICE;
    mar->touched_time = touched_time;
                            /* here, calculate cost directly */
	mar->obj = obj ;
	mar->next = NULL ;

	return 0 ;
}

/* item to string, but do not destroy object */
int market_item_to_str(char *str, struct market_item_data *mar)
{
	extern struct index_data *obj_index;

	if ( *str == 0 ) {
		log("market_item_to_str: no strage type string") ;
		return (-1) ;
    }

	/* save as virtual number */

	if ( mar->id != obj_index[mar->obj->item_number].virtual ) {
		log("market_item_to_str: item number mismatch") ;
		return (-1) ;
    }

	sprintf(str, "%d %d %d %d %d %d %d %d %d %d %d %ld",
            mar->n,	/* number of items */
            mar->id,	/* save as virtual number */
            mar->obj->obj_flags.extra_flags,
            mar->obj->obj_flags.value[0], mar->obj->obj_flags.value[1],
            mar->obj->obj_flags.value[2], mar->obj->obj_flags.value[3],
            mar->obj->affected[0].location, mar->obj->affected[0].modifier,
            mar->obj->affected[1].location, mar->obj->affected[1].modifier,
            mar->touched_time) ;

	return 0 ;
}

