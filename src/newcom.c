/* ************************************************************************
*  file: act.offensive.c , Implementation of commands.    Part of DIKUMUD *
*  Usage : Offensive commands.                                            *
*  Copyright (C) 1990, 1991 - see 'license.doc' for complete information. *
************************************************************************* */

#include <stdio.h>
#include <string.h>
#include <time.h>

#include "structs.h"
#include "utils.h"
#include "comm.h"
#include "interpreter.h"
#include "handler.h"
#include "db.h"
#include "spells.h"
#include "limits.h"
#include "command.h"
#include "memory.h"
#include "quest.h"

/* extern variables */

extern struct room_data *world;
extern struct descriptor_data *descriptor_list;
extern struct char_data *character_list ;

void do_assist(struct char_data *ch, char *argument, int cmd)
{
    char arg[MAX_STRING_LENGTH];
    char buffer[MAX_STRING_LENGTH];
    struct char_data *victim;
    extern int nokillflag;
    char cyb[80];

    one_argument(argument, arg);

    if (*arg) {
        victim = get_char_room_vis(ch, arg);
        if (victim) {
            if (victim == ch) {
                send_to_char("Can you assist your self?\n\r", ch);
                act("$n tries to help $mself, and says YAHA!", FALSE, ch, 0, victim, TO_ROOM);
            } else {

                if(nokillflag) {
                    if((!IS_NPC(ch)) && (IS_NPC(victim))) {
                        send_to_char("You can't help MOB.\n\r", ch);
                        return;
                    }
                }

                /* block indirect player killing */
                if (IS_AFFECTED(ch, AFF_CHARM) && (IS_NPC(victim))) return;
                if ((GET_POS(ch)==POSITION_STANDING) &&
                    (victim != ch->specials.fighting)) {
                    if (!(victim->specials.fighting)) {
                        send_to_char("Who do you think fight ?\n\r", ch);
                        return ;
                    }
                    sprintf(cyb, "%s assist you !!\n\r", GET_NAME(ch));
                    send_to_char(cyb, victim);
                    sprintf(cyb, "You assist %s !!\n\r", GET_NAME(victim));
                    send_to_char(cyb, ch);
                    act("$n ASSIST $N !", FALSE, ch, 0, victim, TO_NOTVICT);
                    hit(ch, victim->specials.fighting, TYPE_UNDEFINED);
                    WAIT_STATE(ch, PULSE_VIOLENCE+2); /* HVORFOR DET?? */
                } else {
                    send_to_char("You do the best you can!\n\r",ch);
                }
            }
        } else {
            send_to_char("They aren't here.\n\r", ch);
        }
    } else {
        send_to_char("Assist who?\n\r", ch);
    }
}

void stop_group(struct char_data *ch)
{
    struct follow_type *j, *k;

    if(!ch->master) {
        log("No master but called stop_group");
        return ;
    }

    act("You stop following $N.", FALSE, ch, 0, ch->master, TO_CHAR);
    act("$n stops following $N.", FALSE, ch, 0, ch->master, TO_NOTVICT);
    act("$n stops following you.", FALSE, ch, 0, ch->master, TO_VICT);

    if (ch->master->followers->follower == ch) { /* Head of follower-list? */
        k = ch->master->followers;
        ch->master->followers = k->next;
        free_general_type((char *)k, MEMORY_FOLLOW_TYPE);
    } else { /* locate follower who is not head of list */
        for(k = ch->master->followers; k->next->follower!=ch; k=k->next)  ;

        j = k->next;
        k->next = j->next;
        free_general_type((char *)j, MEMORY_FOLLOW_TYPE);
    }

    ch->master = 0;
    REMOVE_BIT(ch->specials.affected_by, AFF_GROUP);
}



/* Called when a character that follows/is followed dies */
void do_ungroup(struct char_data *ch, char *argument, int cmd)
{
    struct follow_type *j, *k;

    if (ch->master)
        stop_group(ch);

    for (k=ch->followers; k; k=j) {
        j = k->next;
        stop_group(k->follower);
    }
    REMOVE_BIT(ch->specials.affected_by, AFF_GROUP);
    send_to_char("Your group is dismissed.\n\r", ch);
}

void do_version(struct char_data *ch, char *argument, int cmd)
{
    char *get_version() ;
    char buf[256];

    sprintf(buf, "%s\n\r", get_version()) ;
    send_to_char(buf, ch);
}

#define MAX_LASTCHAT	20
static char lastchatbuf[MAX_LASTCHAT][BUFSIZ] ;
int lastchat_pointer = 0 ;

void do_lastchat(struct char_data *ch, char *argument, int cmd)
{
    int  i, point ;

    send_to_char_han("There were some chat(shout) messages..\n\r\n\r",
                     "There were some chat(shout) messages..\n\r\n\r",  ch);
    point = lastchat_pointer ;
    for ( i = 0; i < MAX_LASTCHAT ; i++) {
        if ( lastchatbuf[point][0] == 0 ) {
            point = (point +1) % MAX_LASTCHAT ;
            continue ;
        }
        send_to_char(lastchatbuf[point], ch);
        point = (point +1) % MAX_LASTCHAT ;
    }
}

void lastchat_add(char *str)
{
	char buf[BUFSIZ];
	long ct;
	struct tm *tmstr;

	ct = time(0);
	tmstr = localtime(&ct);

	sprintf(buf, "%02d:%02d] ", tmstr->tm_hour, tmstr->tm_min);

	strcpy(lastchatbuf[lastchat_pointer], buf);
    strncat(lastchatbuf[lastchat_pointer], str, BUFSIZ);
    strncat(lastchatbuf[lastchat_pointer], "\n\r", BUFSIZ);

    lastchat_pointer = (lastchat_pointer +1) % MAX_LASTCHAT ;
//	str[strlen(str)-2] = 0; 	// eliminate '^M'
	log(str);

//	str[strlen(str)-2] = ch;
}
#undef MAX_LASTCHAT

void do_date(struct char_data *ch, char *argument, int cmd)
{
	long	ct;
	char	buf[BUFSIZ] ;

	ct = time(0);

	sprintf(buf, "Real life time : %s\r", asctime(localtime(&ct))) ;
	send_to_char(buf, ch) ;
}

/*  Throw object to victim  */
void do_throw(struct char_data *ch, char *argument, int cmd)
{
	void throw_object(struct obj_data *obj, struct char_data *ch,
                      struct char_data *vict) ;
	extern int nokillflag ;
	extern struct str_app_type str_app[];
	struct obj_data *obj ;
	struct char_data *vict ;
	char	obj_name[80], vict_name[80] ;
	int	catch_rate, hitting_rate ;

/*
  send_to_char("아직 만들어지지 않은 명령어 입니다.\n\r", ch) ;
  return ;
*/

	argument = one_argument(argument, obj_name);
	if ( *obj_name == 0 ) {
		send_to_char("무엇을 던지고 싶으세요 ?\n\r", ch) ;
		return ;
    }

	argument = one_argument(argument, vict_name) ;
	if ( *vict_name == 0 ) {
		send_to_char("무엇을 누구에게 던질까요?\n\r", ch) ;
		return ;
    }

	if (!(obj = get_obj_in_list_vis(ch, obj_name, ch->carrying))) {
		send_to_char("당신에게는 그런 물건이 없습니다.\n\r", ch);
		return;
    }

	if (IS_OBJ_STAT(obj, ITEM_NODROP)) {
		send_to_char("이크!! 저주받은 물건이라 던질 수가 없습니다 !!\n\r", ch);
		return;
    }

	if (!(vict = get_char_room_vis(ch, vict_name))) {
		send_to_char("여기에 그런 사람은 없는데요 ?\n\r", ch);
		return;
    }

	if (GET_OBJ_WEIGHT(obj) > str_app[STRENGTH_APPLY_INDEX(ch)].wield_w) {
		send_to_char("물건이 너무 무거워 금방 땅에 떨어집니다.\n\r", ch) ;
		act("$n님이 $p을 던지려 했으나 발밑에 떨어집니다.", 1, ch, obj,
			0, TO_ROOM);
		obj_from_char(obj);
		obj_to_room( obj, ch->in_room) ;
		return ;
    }

	if ( ch == vict ) {
		act("당신은 $p에 머리를 받았습니다.", 0, ch, obj, ch, TO_CHAR);
		act("$n님이 주머니에서 꺼낸 $p에 얼굴을 갖다 댑니다.", 1, ch, obj,
			ch, TO_ROOM);
		return ;
    }

	/* avoid players fight : also block 'order' */
	if (nokillflag && !IS_NPC(vict) ) {
		send_to_char("친구와는 싸우지 마세요 !\n\r", ch) ;
		return ;
    }

	/* Now throw item to victim */
	obj_from_char(obj);	/* get object from ch */
		
	/* calculate hitting probability */
	hitting_rate = (500 - IS_CARRYING_W(ch) - GET_OBJ_WEIGHT(obj))/10 ;

	if ( hitting_rate > number(1,50)) {	/* hit */
		catch_rate = GET_LEVEL(vict) + 5 ;
		if ( catch_rate > number(1,50) ||
            (IS_NPC(vict) && vict->nr == real_mobile(1550))) {	/* catch */
			act("당신이 던진 $p을 $N님이 잡았습니다.", 0, ch, obj, vict, TO_CHAR);
			act("$n님이 던진 $p을 $N님이 잡았습니다.",
				FALSE, ch, obj, vict, TO_NOTVICT);
			act("$n님이 당신에게 던진 $p를 잡는데 성공했습니다.", FALSE, ch,
				obj, vict, TO_VICT);
			obj_to_char(obj, vict);
        }
		else {	/* hit realy */
			send_to_char("맞았습니다!\n\r", ch);
			act("$n님이 던진 $p이 $N님의 이마에 정통으로 맞았습니다.",
				FALSE, ch, obj, vict, TO_NOTVICT);
			act("당신은 $n님이 던진 $p에 맞았습니다.", FALSE, ch,
				obj, vict, TO_VICT);
			throw_object(obj, ch, vict) ;
        }
    }
	else {	/* missed */
		if ( number(1,2) == 1 ) {
			act("던진 $p이 빗나가서 땅에 떨어집니다.", 0, ch, obj, 0, TO_CHAR);
			act("$n님이 $p을 $N님에게 던졌습니다만 아슬아슬하게 빗나갑니다.",
				FALSE, ch, obj, vict, TO_NOTVICT);
			act("$n님이 당신에게 던진 $p를 아슬아슬하게 피했습니다.", FALSE, ch,
				obj, vict, TO_VICT);
        }
		else {
			act("당신이 던진 $p을 $N님이 쳐냈습니다.", 0, ch, obj, vict, TO_CHAR);
			act("$n님이 던진 $p을 $N님이 가볍게 쳐냅니다.",
				FALSE, ch, obj, vict, TO_NOTVICT);
			act("$n님이 당신에게 던진 $p를 받아 쳤습니다.", FALSE, ch,
				obj, vict, TO_VICT);
        }
		obj_to_room( obj, ch->in_room) ;
    }
}

void throw_object(struct obj_data *obj, struct char_data *ch,
                  struct char_data *vict)
{
	int	i, dam, d1, d2 ;
	extern struct spell_info_type spell_info[];

	/* special actions */
	switch ( GET_ITEM_TYPE(obj) ) {
    case ITEM_POTION :
        act("$p의 내용물이 $N님 몸에 퍼집니다.", 0, ch, obj, vict, TO_CHAR);
        act("$p의 병이 깨지며 조각이 사방에 흩어 집니다.",
            FALSE, ch, obj, vict, TO_NOTVICT);
        act("$p이 깨지면서 약이 당신의 몸에 퍼지기 시작 합니다.", FALSE, ch,
            obj, vict, TO_VICT);
        for ( i = 1 ; i < 4 ; i ++ )
            if (obj->obj_flags.value[i] >= 1)
                ((*spell_info[obj->obj_flags.value[i]].spell_pointer)
                 ((byte) obj->obj_flags.value[0], vict, "",
                  SPELL_TYPE_POTION, ch, 0));
        extract_obj(obj) ;
        damage(ch, vict, 0, TYPE_UNDEFINED) ;
        break ;
    case ITEM_WEAPON :
        d1 = obj->obj_flags.value[1] ;
        d2 = obj->obj_flags.value[2] ;
        dam = GET_OBJ_WEIGHT(obj) + d1*d2/4 ;
        damage(ch, vict, dam, TYPE_UNDEFINED) ;
        obj_to_room( obj, ch->in_room) ;
        break ;
    case ITEM_FOOD :
    case ITEM_DRINKCON :
    default :
        dam = GET_OBJ_WEIGHT(obj) ;
        damage(ch, vict, dam, TYPE_UNDEFINED) ;
        obj_to_room( obj, ch->in_room) ;
        break ;
    }
}

/* list available commands */
void do_commands(struct char_data *ch, char *argument, int cmd)
{
	/* The first command is command[0] */
	/* cmd_info[1] ~~ commando[0]  */
	extern char	*command[] ;
	extern struct command_info cmd_info[] ;
	int	count, level, n ;
	char	*buf, tmp[32] ;

	/* assume max command length is 16 */
	buf = (char *) malloc ( MAX_CMD_LIST * 16 ) ;

	level = GET_LEVEL(ch) ;
	buf[0] = 0 ;
	count = 0 ;
	/* new line is end of list */
	while ( *command[count] != '\n' && count < MAX_CMD_LIST ) {
		if ( level >= cmd_info[count+1].minimum_level ) {
			n ++ ;
			sprintf(tmp, "%-13.13s", command[count]) ;
			if ( n % 6 == 0 )
				strcat(tmp, "\n\r") ;
			strcat(buf, tmp) ;
        }
		count ++ ;
    }
	strcat(buf, "\n\r") ;
	page_string(ch->desc, buf, 1) ;
	free_string_type(buf) ;
}

void do_nothing(struct char_data *ch, char *argument, int cmd)
{
	Huh(ch) ;
}

void do_quest(struct char_data *ch, char *argument, int cmd)
{
	send_to_char_han("You solved Quest :\n\r",
                     "다음과 같은 문제를 해결 하였습니다.\n\r", ch) ;
	send_to_char(what_is_solved(ch), ch) ;
}


#define	RENT_COST_FACTOR	1.0

void do_offer(struct char_data *ch, char *argument, int cmd)
{
	int item_cost_lists(struct obj_data *p, char *str, int size) ;
	int item_cost_equip(struct char_data *ch, char *str) ;
	struct obj_data *obj, *tmp_obj, *next_obj ;
	char	obj_name[BUFSIZ], buf[MAX_STRING_LENGTH], buf2[BUFSIZ] ;
	int	cost ,count, sum ;

	if (GET_LEVEL(ch) < IMO && ch->in_room != real_room(3008)) {
		send_to_char("You must go to Reception to check the cost.\n\r", ch) ;
		return ;
    }

	argument = one_argument(argument, obj_name);
	if ( *obj_name == 0 ) {
		send_to_char("usage: offer < item | all | inv | equip | rent >\n\r", ch) ;
		return ;
    }

	if ( strcmp(obj_name, "all") == 0 ) {
		sum = cost = item_cost_equip(ch, 0) ;
		sprintf(buf2, "입고 있는 것 (wearing) : %d 원.\n\r", cost ) ;
		strcpy(buf, buf2) ;
		if ( ch->carrying ) {
			cost = item_cost_lists(ch->carrying, 0, 0) ;
			sum += cost ;
			sprintf(buf2, "들고 있는 것 (carring) : %d 원.\n\r", cost) ;
			strcat(buf, buf2) ;
        }
		sprintf(buf2, "총 합 (total) : %d 원.\n\r", sum ) ;
		strcat(buf, buf2) ;
		send_to_char(buf, ch) ;
		return ;
    }
	if ( strcmp(obj_name, "equip") == 0 ) {
		sprintf(buf, "입고 있는 물건들 (wearing) :\n\r") ;
		cost = item_cost_equip(ch, buf) ;
		sprintf(buf2, "총 합 (total) : %d 원.\n\r", cost ) ;
		strcat(buf, buf2) ;
		send_to_char(buf, ch) ;
		return ;
    }
	if ( strcmp(obj_name, "inv") == 0 ) {
		if ( ch->carrying ) {
			sprintf(buf, "들고 있는 물건들 (inventory) :\n\r") ;
			cost = item_cost_lists(ch->carrying, buf, MAX_STRING_LENGTH) ;
			sprintf(buf2, "총 합 (total) : %d 원.\n\r", cost ) ;
			strcat(buf, buf2) ;
        }
		else {
			strcpy(buf, "You are carryng nothing.\n\r") ;
        }
		send_to_char(buf, ch) ;
		return ;
    }
	if ( strcmp(obj_name, "rent") == 0 ) {
		if ( ch->rent_items ) {
			sprintf(buf, "Rent 하고 있는 물건들 :\n\r") ;
			cost = item_cost_lists(ch->rent_items, buf, MAX_STRING_LENGTH-30) ;
			sprintf(buf2, "총 합 (total) : %d 원.\n\r", cost ) ;
			strcat(buf, buf2) ;
        }
		else {
			strcpy(buf, "Rent 한 물건이 없습니다.\n\r") ;
        }
		send_to_char(buf, ch) ;
		return ;
    }

	obj = get_obj_in_list_vis(ch, obj_name, ch->carrying) ;

	if ( obj == NULL ) {
		send_to_char_han("You don't have such thing.\n\r",
                         "당신에게는 그런 물건이 없습니다.\n\r", ch) ;
		return ;
    }

	if (( cost =  GET_OBJ_RENTCOST(obj))<= 0 ) {
		cost = item_cost(obj) ;
		GET_OBJ_RENTCOST(obj) = cost ;
    }
	cost = RENT_COST_FACTOR * cost ;
	sprintf(buf, "%s  -  %d원.\n\r", obj->short_description, cost) ;

	if (obj->contains) {
		strcat(buf, "속안의 물건들 (inside) :\n\r") ;
		sum = item_cost_lists(obj->contains, buf, MAX_STRING_LENGTH-35) ;
		sprintf(buf2, " 안의 물건 합 : %d 원.\n\r", sum) ;
		strcat(buf, buf2) ;
    }

	send_to_char(buf, ch) ;
}

int item_cost_all(struct char_data *ch)
{
	int	sum ;

	if ( ch == 0 )
		return 0 ;

	sum = item_cost_equip(ch, 0) ;
	if ( ch->carrying )
		sum += item_cost_lists(ch->carrying, 0, 0) ;

	return sum ;
}

int item_cost_equip(struct char_data *ch, char *str)
{
	struct obj_data	*obj ;
	int	i, cost, sum ;
	char	buf[BUFSIZ] ;

	for ( i = 0, sum = 0 ; i < MAX_WEAR ; i++ ) {
		if ( obj = ch->equipment[i] ) {
			if ((cost = GET_OBJ_RENTCOST(obj)) <= 0 ) {
				cost = item_cost(obj) ;
				GET_OBJ_RENTCOST(obj) = cost ;
            }
			cost = RENT_COST_FACTOR * cost ;
			if ( str ) {
				sprintf(buf, "%s  -  %d원.\n\r", obj->short_description, cost) ;
				strcat(str, buf) ;
            }
			sum += cost ;
			if ( obj->contains ) {
				cost = item_cost_lists(obj->contains, 0, 0) ;
				sum += cost ;
				if ( str ) {
					sprintf(buf, "  그 안에 있는 물건들  -  %d 원.\n\r", cost ) ;
					strcat(str, buf) ;
                }
            }
        }
    }

	return sum ;
}

int item_cost_lists(struct obj_data *p, char *str, int size)
{
	struct obj_data	*pp;
	int	cost, sum, toolong ;
	char	buf[BUFSIZ] ;

	if ( p == NULL )
		return 0 ;

	sum = 0 ;
	toolong = 0 ;
	for ( pp = p ; pp != 0 ; pp = pp->next_content) {
		if ((cost = GET_OBJ_RENTCOST(pp)) <= 0 ) {
			cost = item_cost(pp) ;
			GET_OBJ_RENTCOST(pp) = cost ;
        }
		cost = RENT_COST_FACTOR * cost ;
		sum += cost ;
		if ( str ) {
			sprintf(buf, "%s  -  %d원.\n\r", pp->short_description, cost) ;
			if ( strlen(str) + strlen(buf) < size - 24 )
				strcat(str, buf) ;
			else
				toolong = 1 ;
        }
		if(pp->contains) {
			cost = item_cost_lists(pp->contains, 0, 0);
			sum += cost ;
			if ( str ) {
				sprintf(buf, "  그 안에 있는 물건들  -  %d원.\n\r", cost) ;
				if ( strlen(str) + strlen(buf) < size - 24 )
					strcat(str, buf) ;
				else
					toolong = 1 ;
            }
        }
    }

	if ( str && toolong )
		strcat(str, "\n\rToo many items.\n\r") ;

	return sum ;
}

void do_sacrifice(struct char_data *ch, char *argument, int cmd)
{
	char	buf[BUFSIZ] ;
	struct obj_data	*temp ;
	struct affected_type	af ;
	int	gold ;

	one_argument(argument,buf);
	if ( *buf == 0 ) {
		send_to_char("Sacrifice what ?", ch) ;
		return ;
    }

	if(!(temp = get_obj_in_list_vis(ch, buf, ch->carrying))) {
		send_to_char("You can't find it!\n\r", ch);
		return;
    }

	if (IS_OBJ_STAT(temp, ITEM_NODROP)) {
		send_to_char("It was cursed object! You punished.\n\r", ch) ;
		if ( affected_by_spell(ch, SPELL_CURSE) ) {
			send_to_char("Don't sacrifice cursed object!\n\r", ch) ;
			return ;
        }

		af.type      = SPELL_CURSE;
		af.duration  = 2;	/* 2 hours */
		af.modifier  = -1;
		af.location  = APPLY_HITROLL;
		af.bitvector = AFF_CURSE;
		affect_to_char(ch, &af);

		af.location = APPLY_SAVING_PARA;
		af.modifier = 1; /* Make worse */
		affect_to_char(ch, &af);

		act("$n briefly reveal a red aura!", FALSE, ch, 0, 0, TO_ROOM) ;
		send_to_char("You feel very uncomfortable.\n\r", ch) ;
		extract_obj(temp) ;
    }
	else {
		if (IS_OBJ_STAT(temp, ITEM_BLESS) && GET_ALIGNMENT(ch) < 400) {
			GET_ALIGNMENT(ch) = MIN(400, GET_ALIGNMENT(ch) + 4 + number(1,4)) ;
        }
		act("$n님이 $p 를 신에게 바칩니다.", TRUE, ch, temp, 0, TO_ROOM) ;
		act("당신은 $o 를 신에게 바쳤습니다.",FALSE, ch, temp, 0, TO_CHAR) ;
		gold = GET_OBJ_WEIGHT(temp)/7 + GET_OBJ_WEIGHT(temp) % 8 + number(1,4) ;
		GET_GOLD(ch) += gold ;
		sprintf(buf,"신이 당신의 선행에 기뻐하며 %d 원을 하사합니다.\n\r", gold) ;
		send_to_char(buf, ch) ;
		extract_obj(temp);
    }
}

void do_display(struct char_data *ch, char *argument, int cmd)
{
	char	buf[BUFSIZ] ;

	one_argument(argument,buf);
	if ( *buf == 0 ) {	/* display current stat */
		strcat(buf, "  Brief message : ") ;
		if (IS_SET(ch->specials.act, PLR_BRIEF))
			strcat(buf, "Yes\n\r") ;
		else
			strcat(buf, "No\n\r") ;
		strcat(buf, "  Compact mode : ") ;
		if (IS_SET(ch->specials.act, PLR_COMPACT))
			strcat(buf, "Yes\n\r") ;
		else
			strcat(buf, "No\n\r") ;
		strcat(buf, "  Commands : ") ;
		if (IS_SET(ch->specials.act, PLR_KOREAN))
			strcat(buf, "Korean\n\r") ;
		else
			strcat(buf, "English\n\r") ;
        /* PLR_NOSHOUT is wizard command.. forced command.  */
		strcat(buf, "  Hear other's tell : ") ;
		if (IS_SET(ch->specials.act, PLR_NOTELL))
			strcat(buf, "No\n\r") ;
		else
			strcat(buf, "Yes\n\r") ;
		strcat(buf, "  Hear other's shout : ") ;
		if (IS_SET(ch->specials.act, PLR_EARMUFFS))
			strcat(buf, "NO\n\r") ;
		else
			strcat(buf, "Yes\n\r") ;
		strcat(buf, "  Hear other's chat : ") ;
		if (IS_SET(ch->specials.act, PLR_NOCHAT))
			strcat(buf, "No\n\r") ;
		else
			strcat(buf, "Yes\n\r") ;
		strcat(buf, "  Wimpy mode : ") ;
		if (IS_SET(ch->specials.act, PLR_WIMPY))
			strcat(buf, "Yes\n\r") ;
		else
			strcat(buf, "No\n\r") ;
		strcat(buf, "  Display exits : ") ;
		if (IS_SET(ch->specials.act, PLR_AUTOEXIT))
			strcat(buf, "Yes\n\r") ;
		else
			strcat(buf, "No\n\r") ;
		if (GET_LEVEL(ch) >= IMO) {
			strcat(buf, "  Wizard invisibility : ") ;
			if (IS_SET(ch->specials.act, PLR_WIZINVIS))
				strcat(buf, "Yes\n\r") ;
			else
				strcat(buf, "No\n\r") ;
        }

		send_to_char(buf, ch) ;
    }
}
