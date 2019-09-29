/*
	chamber.c - 재활용 기계를 위한 file
*/
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <time.h>

#include "structs.h"
#include "utils.h"
#include "handler.h"
#include "comm.h"
#include "db.h"
#include "quest.h"	/* by cyb */
#include "command.h"

/* 재활용 기계 : 재활용 센터 (market) */
int chamber(struct char_data *ch, int cmd, char *arg)
{
	int	retreat_chamber(struct char_data *ch, struct obj_data *obj) ;
	extern struct index_data *obj_index ;
	struct obj_data	*obj ;
	char	name[256] ;
	char	buf[BUFSIZ], buf2[BUFSIZ], buf3[BUFSIZ], buf4[BUFSIZ] ;
	int	bits ;
	struct char_data *tmp_char ;

	/* pull lever */
	if ( cmd != COM_PULL )
		return FALSE ;

	one_argument(arg, name) ;
	if ( !isname(name, "손잡이 lever")) {
		send_to_char_han("Pull what ?\n\r", "무엇을 당길까요 ?\n\r", ch) ;
		return TRUE ;
    }

	/* find chamber */
	bits = generic_find("chamber", FIND_OBJ_ROOM, ch, &tmp_char, &obj) ;

	if ( obj && obj_index[obj->item_number].virtual == 12100 ) {
		if (IS_SET(obj->obj_flags.value[1],CONT_CLOSED )) {
			send_to_char_han("You pulled the lever and machine works.\n\r",
                             "손잡이를 당기자 기계가 움직입니다.\n\r", ch) ;
			acthan("$n pulls the lever and machine start dancing.", 
                   "$n님이 손잡이를 당기자 기계가 덜컹거리며 움직입니다.",
                   FALSE, ch, 0, 0, TO_ROOM) ;
			if ( retreat_chamber(ch, obj) < 0 ) {
				strcpy(buf, "   clink\n\r   clink\n\r   ...\n\rKABOOM !!!\n\r") ;
				strcat(buf, "Ops ! Machine is out of order, Item crushed\n\r" ) ;
				strcpy(buf2, "   덜컹\n\r   덜커덩\n\r   ....\n\r콰과광 !!!\n\r") ;
				strcat(buf2, "저런 ! 기계가 고장나면서 물건들을 깨뜨렸습니다.\n\r") ;
				send_to_char_han(buf, buf2, ch) ;
				acthan("clink.. clink.. KABOOM !! Machine stoped",
                       "덜컹..덜컹..  쾅 !! 기계가 부서졌습니다.",
                       FALSE, ch, 0, 0, TO_ROOM) ;
            }
			else {	/* It worked successfully !! */
				strcpy(buf, "  chi-kok\n\r  chi-kok\n\r  ...\n\r  *tic-tok*\n\r") ;
				strcat(buf, "Machine stoped.\n\r" ) ;
				strcpy(buf2, "  찰칵\n\r  찰칵\n\r   ...\n\r*철커덕*\n\r") ;
				strcat(buf2, "기계가 멈췄습니다.\n\r") ;
				send_to_char_han(buf, buf2, ch) ;
				acthan("Machine stoped.", "기계가 멈췄습니다.",
                       FALSE, ch, 0, 0, TO_ROOM) ;
            }
        }
		else {
			send_to_char_han("Close chamber first !\n\r",
                             "먼저 chamber 를 닫으세요 !\n\r", ch ) ;
        }
    }
	else {	/* Can't find chamber !!! */
		send_to_char_han("Can't find main chamber and lever\n\r",
                         "기계 본체를 찾을수 없습니다.\n\r", ch) ;
    }
}

int retreat_chamber(struct char_data *ch, struct obj_data *obj)
{
	int	empty_chamber(struct obj_data *list) ;
	int	count_cost(struct obj_data *list) ;
	int	check_okse_composition(struct obj_data *obj) ;
	int	check_polyester_composition(struct obj_data *obj) ;
	int	check_bomb_composition(struct obj_data *obj) ;
	void	polyester_reward(struct char_data *ch) ;
	struct obj_data	*gold ;
	struct obj_data	*bonus_obj ;
	int	money, r_num, nammo, vnum ;
	char	buf[BUFSIZ] ;

	if ( obj->obj_flags.type_flag != ITEM_CONTAINER )
		return (-1) ;

	/* check composition */
	nammo = 0 ;
	if ( check_okse_composition(obj) ) {
		if ( empty_chamber(obj) < 0 ) {	/* machine 을 비움 */
			log("retreat_chamber: can't empty chamber") ;
			return (-1) ;
        }
		if (( bonus_obj = read_object(2706, VIRTUAL)) == 0 ) {	/* okse */
			log("retreat_chamber: Fatal error making okse") ;
			return(-1) ;
        }
		obj_to_obj(bonus_obj, obj) ;
		gold = create_money(180000) ;	/* give some gold */
		obj_to_obj(gold, obj) ;

		/* Quest solved !! */
		log("<<<<  OKSE composition completed  >>>>") ;
		if ( !is_solved_quest(ch, QUEST_OKSE) ) {
			send_to_char_han("Cool ! You solved Okse Quest.\n\r",
                             "잘 하셨습니다. 당신은 옥새 문제를 풀었습니다.\n\r", ch) ;
			set_solved_quest(ch, QUEST_OKSE) ;
			sprintf(buf, "%s solved OKSE QUEST", GET_NAME(ch)) ;
			log(buf) ;
        }
    }
	else if ( check_polyester_composition(obj) ) {
		/* Quest solved !! */
		money = 0 ;
		if ( !is_solved_quest(ch, QUEST_POLYESTER) ) {
			send_to_char_han("Good ! You solved Polyester Quest.\n\r",
                             "잘 하셨습니다. 당신은 Polyester 문제를 풀었습니다.\n\r", ch) ;
			set_solved_quest(ch, QUEST_POLYESTER) ;
			polyester_reward(ch) ;
			sprintf(buf, "%s solved POLYESTER QUEST", GET_NAME(ch)) ;
			log(buf) ;
			money += 10000 ;
        }
		money += count_cost(obj) ;
		if ( empty_chamber(obj) < 0 ) {	/* machine 을 비움 */
			log("retreat_chamber: can't empty chamber") ;
			return (-1) ;
        }
		if ( money > 0 ) {
			gold = create_money(money) ;
			if ( gold ) obj_to_obj(gold, obj) ;
        }
    }
	else if (( nammo = check_bomb_composition(obj)) > 0 ) {
		money = 0 ;
		if ( !is_solved_quest(ch, QUEST_MAKEBOMB) ) {
			send_to_char_han("Good ! You solved Bomb Quest.\n\r",
                             "잘 하셨습니다. 당신은 폭탄 만들기 문제를 풀었습니다.\n\r", ch) ;
			set_solved_quest(ch, QUEST_MAKEBOMB) ;
			sprintf(buf, "%s solved MAKEBOMB QUEST", GET_NAME(ch)) ;
			log(buf) ;
			money += 10000 ;
        }
		if ( empty_chamber(obj) < 0 ) {	/* machine 을 비움 */
			log("retreat_chamber: can't empty chamber") ;
			return (-1) ;
        }
		if ( money > 0 ) {
			gold = create_money(money) ;
			if (gold)	obj_to_obj(gold, obj) ;
        }
		if ( nammo < 3 ) {
			vnum = 12102 ;	/* small bomb */
        }
		else if ( nammo < 5 ) {
			vnum = 12103 ;	/* medium bomb */
        }
		else {
			vnum = 12104 ;	/* large bomb */
        }
		if (( bonus_obj = read_object(vnum, VIRTUAL)) == 0 ) {	/* time bombs */
			log("retreat_chamber: Fatal error making time bomb") ;
			return(-1) ;
        }
		obj_to_obj(bonus_obj, obj) ;
    }
	else if ( check_alom_composition(obj) ) {
		/* Quest solved !! */
		money = 0 ;
		if ( !is_solved_quest(ch, QUEST_ALOM) ) {
			send_to_char_han("Good ! You solved ALOM Quest.\n\r",
                             "잘 하셨습니다. 당신은 ALOM 문제를 풀었습니다.\n\r", ch) ;
			set_solved_quest(ch, QUEST_ALOM) ;
			alom_quest_reward(ch) ;
			sprintf(buf, "%s solved ALOM QUEST", GET_NAME(ch)) ;
			log(buf) ;
			money += 1000 ;
        }
		money += count_cost(obj) ;
		if ( empty_chamber(obj) < 0 ) {	/* machine 을 비움 */
			log("retreat_chamber: can't empty chamber") ;
			return (-1) ;
        }
		if ( money > 0 ) {
			gold = create_money(money) ;
			if (gold)	obj_to_obj(gold, obj) ;
        }
    }
	else {
		money = count_cost(obj) ;
		money = MIN(money, 700000);
		if ( empty_chamber(obj) < 0 ) {	/* machine 을 비움 */
			log("retreat_chamber: can't empty chamber") ;
			return (-1) ;
        }
		if ( money > 0 ) {
			gold = create_money(money) ;
			if (gold)	obj_to_obj(gold, obj) ;
        }
	}

    return 0 ;
}

int count_cost(struct obj_data *list)
{
	struct obj_data	*obj, *next_obj ;
	int	money, count, factor ;

	money = count = 0 ;
	factor = 1 ;
	for ( obj = list->contains ; obj ; obj = next_obj ) {
		next_obj = obj->next_content ;
		money += (GET_OBJ_WEIGHT(obj)+1) ;
		money += (obj->obj_flags.cost)/10 ;
		if ( ++count > 50) {
			factor ++ ;
			count = 0 ;
        }
    }
	money = factor * money ;

	return money ;
}

int empty_chamber(struct obj_data *list)
{
	struct obj_data	*obj, *next_obj ;
	
	for ( obj = list->contains ; obj ; obj = next_obj ) {
		next_obj = obj->next_content ;
		extract_obj(obj) ;
    }

	return 0 ;
}
