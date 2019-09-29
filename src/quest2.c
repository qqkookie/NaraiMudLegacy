/*
	quest2.c - contains some quests
*/
#include <stdio.h>

#include "structs.h"
#include "utils.h"
#include "comm.h"
/* #include "interpreter.h" */
#include "handler.h"
#include "db.h"
/* #include "spells.h" */
/* #include "limits.h" */
/* #include "mob_magic.h"*/		/* by cyb */
#include "quest.h"	/* by cyb */
#include "command.h"

extern struct index_data *obj_index;
extern struct index_data *mob_index;

/********************************* 
	Quest Okse - by Big and Cold
 *********************************/

#define	SUNIN_KANGYU	0
#define	SUNIN_WOOGIL	1
#define	SUNIN_JWAJA	2

#define	KEYWORD1		2703	/* seal keyword I */
#define	KEYWORD2		2704
#define	KEYWORD3		2705
#define	PASTRY	3011	/* pastry */

int myseal[3] ;
int component_stone, component_stick, component_pastry ;
char	component_stone_name[80] ;
char	component_stick_name[80] ;

/* initializing */
int init_quest_okse()
{
	extern int component_stone, component_stick, component_pastry ;
	extern char component_stone_name[], component_stick_name[] ;
	struct obj_data	*obj ;
	int	i, n1, n2, r_num, tmp ;

	/* select seals : shuffling */
	myseal[0] = KEYWORD1 ;
	myseal[1] = KEYWORD2 ;
	myseal[2] = KEYWORD3 ;
	for ( i = 0 ; i < 10 ; i ++ ) {
		switch ( number(0,2) ) {
        case 0 : n1 = 0 ; n2 = 1 ; break ;
        case 1 : n1 = 1 ; n2 = 2 ; break ;
        case 2 : n1 = 0 ; n2 = 2 ; break ;
        }
		tmp = myseal[n1] ;
		myseal[n1] = myseal[n2] ;
		myseal[n2] = tmp ;
    }

	/* select stone : object from 5230 to 5244 is stone */
	component_stone = number(5230, 5244) ;
	if (( obj = read_object(component_stone, VIRTUAL)) == 0 ) {
		log ("init_quest_okse: can't load stone") ;
		exit(1) ;
    }
	strcpy(component_stone_name, obj->short_description) ;
	extract_obj(obj) ;	/* destroy */

	/* select stick : object from 2100 to 2112 is stone */
	component_stick = number(2100, 2112) ;
	if (( obj = read_object(component_stick, VIRTUAL)) == 0 ) {
		log ("init_quest_okse: can't load stick") ;
		exit(1) ;
    }
	strcpy(component_stick_name, obj->short_description) ;
	extract_obj(obj) ;	/* destroy */

	/* select the number of pastry */
	component_pastry = number(1,4) ;

}

/* sun-in : kangyu, woogil, jwaja 강유 2836, 우길 17010, 좌자 16012 */
int okse_sunin(struct char_data *ch, int cmd, char *arg)
{
	extern int myseal[] ;
	struct obj_data	*seal, *seal_other, *list ;
	int	vnum, whoami ;
	char	buf[BUFSIZ] ;

	if (cmd)
		return FALSE ;

	if ( ch->specials.fighting )	/* He is fighting !!! */
		return FALSE ;

	if (mob_index[ch->nr].virtual == 2836 )
		whoami = SUNIN_KANGYU ;
	else if (mob_index[ch->nr].virtual == 17010 )
		whoami = SUNIN_WOOGIL ;
	else if (mob_index[ch->nr].virtual == 16012 )
		whoami = SUNIN_JWAJA ;
	else
		return FALSE ;

	seal = seal_other = NULL ;
	for (list = ch->carrying ; list ; list = list->next_content ) {
		vnum = obj_index[list->item_number].virtual ;
		if ( vnum == KEYWORD1 || vnum == KEYWORD2 || vnum == KEYWORD3 ) {
			if ( vnum == myseal[whoami] )
				seal = list ;
			else
				seal_other = list ;
        }
    }

	if ( seal ) {	/* has correct seal keyword */
		if ( number(1,4) < 4 ) {
			switch ( number(1,3) ) {
            case 1 : act("$n님이 $p 을 이리저리 살펴 봅니다.", TRUE, ch,
                         seal, 0, TO_ROOM) ;
            break ;
            case 2 :
                do_sayh(ch, "하, 이건.. 잠시만 기다리시오.", COM_HAN_SAY) ;
                break ;
            case 3 : act("$n님이 $p 을 보며 뭔가를 끄적거립니다.", TRUE, ch,
                         seal, 0, TO_ROOM) ;
            break ;
            }
			return TRUE ;
        }
		switch(whoami) {
        case SUNIN_KANGYU : sprintf(buf,
                                    "흠.. 이 암호의 의미는 옥새 만들때 %s 이 필요하다는 것이오",
                                    component_stone_name) ;
        break ;
        case SUNIN_WOOGIL : sprintf(buf,
                                    "흠.. 이것은 옥새를 만들때 %s 이 필요하다는 뜻이요",
                                    component_stick_name) ;
        break ;
        case SUNIN_JWAJA : sprintf(buf,
                                   "흠.. 이것은 옥새에 Keyword외에 %d 개의 pastry가 필요하다는 뜻이오",
                                   component_pastry ) ;
        }
		do_sayh(ch, buf, COM_HAN_SAY) ;
		do_sayh(ch, "Keyword 를 같이 넣는 것을 잊지 마시오.", COM_HAN_SAY) ;
		act("$n님이 $p 을 바닥에 가만히 내려 놓습니다.", TRUE, ch,
			seal, 0, TO_ROOM) ;
		obj_from_char(seal) ;
		obj_to_room(seal, ch->in_room) ;
    }
	else if ( seal_other ) {	/* has incorrect seal keyword */
		switch(number(0,6)) {
        case 0 :	do_sayh(ch, "흠..", COM_HAN_SAY) ; break ;
        case 1 :
            do_sayh(ch,"이 귀한 물건을 어디에서 얻었습니까?",COM_HAN_SAY);
            break ;
        case 2 :
            do_sayh(ch, "하.. 해석하기가 아주 어려운데요 ?",COM_HAN_SAY);
            break ;
        case 3 :
            do_sayh(ch, "이것은 옥새에 관한게 틀림 없는데..",COM_HAN_SAY);
            break ;
        case 4 :
            do_sayh(ch, "다른 나라에 가보시는게 좋겠습니다.", COM_HAN_SAY) ;
            break ;
        default :
            if ( whoami != SUNIN_KANGYU ) {
                act("$n님이 $p 를 바닥에 내려 놓습니다.", TRUE, ch,
                    seal_other, 0, TO_ROOM) ;
                obj_from_char(seal_other) ;
                obj_to_room(seal_other, ch->in_room) ;
            }
        }
    }
	else
		return FALSE ;

	return TRUE ;
}

/* composition check in retreatment machine */
int check_okse_composition(struct obj_data *list)
{
	extern int component_stone, component_stick, component_pastry ;
	struct obj_data	*obj, *next_obj ;
	int	stone, stick, pastry, seal[3], panalty, vnum ;

	if ( list->obj_flags.type_flag != ITEM_CONTAINER )
		return FALSE ;

	panalty = 0 ;
	stone = stick = pastry = 0 ;
	seal[0] = seal[1] = seal[2] = 0 ;

	/* count content */
	for ( obj = list->contains ; obj ; obj = next_obj ) {
		next_obj = obj->next_content ;
		vnum = obj_index[obj->item_number].virtual ;
		if ( vnum == component_stone )
			stone ++ ;
		else if ( vnum == component_stick )
			stick ++ ;
		else {
			switch( vnum ) {
            case KEYWORD1 : seal[0] ++ ; break ;
            case KEYWORD2 : seal[1] ++ ; break ;
            case KEYWORD3 : seal[2] ++ ; break ;
            case PASTRY : pastry ++ ; break ;
            default : panalty ++ ;
            }
        }
    }

	/* check composition */
	if ( panalty )
		return FALSE ;

	if ( seal[0] == 1 && seal[1] == 1 && seal[2] == 1 &&
         stone == 1 && stick == 1 && (component_pastry == pastry))
		return TRUE ;

	return FALSE ;
}

/*****************************
  Quest lost card -  by Cold
 *****************************/

struct char_data *donjonkeeper_lastperson ;
int donjonkeeper(struct char_data *ch, int cmd, char *arg)
{
	struct obj_data	*obj ;
	char	give_obj[80], give_person[80], arg2[BUFSIZ] ;
	char	buf[BUFSIZ] ;
	int	found ;

	if ( cmd )
		if ( cmd != COM_GIVE )
			return FALSE ;

	if ( cmd == COM_GIVE ) {
		argument_split_2(arg, give_obj, arg2) ;
		if ( *give_obj == 0 ) return FALSE ;
		if ( !isname(give_obj, "현금카드 cashcard")) return FALSE ;

		one_argument(arg2, give_person) ;
		if ( *give_person == 0 || !isname(give_person, "donjonkeeper") )
			return FALSE ;

		if ( !IS_NPC(ch) )
			donjonkeeper_lastperson = ch ;
		return FALSE ;	/* It must be FALSE !!! */
    }

	if ( ch->specials.fighting )
		return FALSE ;

	if (donjonkeeper_lastperson && donjonkeeper_lastperson->desc) {
		found = 0 ;
		for (obj = ch->carrying ; obj ; obj = obj->next_content ) {
			if ( obj_index[obj->item_number].virtual == 9006) {	/* 현금카드 */
				extract_obj(obj) ;
				found = 1 ;
            }
        }
		if ( found ) {
			if ( !is_solved_quest(donjonkeeper_lastperson, QUEST_LOSTCARD)) {
				send_to_char_han("Congratualtion ! You solved Lost object Quest\n\r",
                                 "축하합니다 ! 당신은 잃어버린 물건 문제를 풀었습니다.\n\r",
                                 donjonkeeper_lastperson) ;
				set_solved_quest(donjonkeeper_lastperson, QUEST_LOSTCARD) ;
				strcpy(buf,"Donjonkeeper님이 잃어 버린 현금카드를 찾아준 것에\n\r");
				strcat(buf, " 감사하여 은행 계좌에 5000원을 보내주었습니다.\n\r") ;
				send_to_char(buf, donjonkeeper_lastperson) ;
				donjonkeeper_lastperson->bank += 5000 ;
            }
			else {
				strcpy(buf,"Donjonkeeper님이 잃어 버린 현금카드를 찾아준 것에\n\r");
				strcat(buf, " 감사하여 은행 계좌에 500원을 보내주었습니다.\n\r") ;
				send_to_char(buf, donjonkeeper_lastperson) ;
				donjonkeeper_lastperson->bank += 500 ;
            }
        }
		donjonkeeper_lastperson = NULL ;
    }

}

/*****************************
  polyester Quest - by Cold
 *****************************/

/* composition check in retreatment machine */
int check_polyester_composition(struct obj_data *list)
{
	struct obj_data	*obj, *next_obj ;
	int	building, vnum ;

	if ( list->obj_flags.type_flag != ITEM_CONTAINER )
		return FALSE ;

	/* count content */
	building = 0 ;
	for ( obj = list->contains ; obj ; obj = next_obj ) {
		next_obj = obj->next_content ;
		vnum = obj_index[obj->item_number].virtual ;
		if ( vnum >= 2500 && vnum <= 2524 )	{ /* it's tall building's item */
			building ++ ;
        }
    }

	if ( building > 11 ) 
		return TRUE ;

	return FALSE ;
}

void polyester_reward(struct char_data *ch)
{
	switch(GET_CLASS(ch)) {
    case CLASS_MAGIC_USER :
    case CLASS_CLERIC :
        if (ch->abilities.con < 18 ) {
            ch->abilities.con ++ ;
            send_to_char_han("Your constitution point increased !\n\r",
                             "당신의 몸이 좋아짐을 느낍니다.\n\r", ch) ;
        }
        break ;
    case CLASS_THIEF :
    case CLASS_WARRIOR :
        if (ch->abilities.intel < 18 ) {
            ch->abilities.intel ++ ;
            send_to_char_han("You notice you were fool.\n\r",
                             "조금 똑똑해진 기분이 듭니다.\n\r", ch) ;
        }
        break ;
    }
	affect_total(ch) ; 
	save_char(ch, NOWHERE) ;
}

/*****************************
  Make Bomb Quest - by Cold
 *****************************/

/* composition check in retreatment machine */
int check_bomb_composition(struct obj_data *list)
{
	struct obj_data	*obj, *next_obj ;
	int	ammo, vnum ;

	if ( list->obj_flags.type_flag != ITEM_CONTAINER )
		return FALSE ;

	/* count content */
	ammo = 0 ;
	for ( obj = list->contains ; obj ; obj = next_obj ) {
		next_obj = obj->next_content ;
		vnum = obj_index[obj->item_number].virtual ;
		if ( vnum == 12105 )	{ /* it's 화약 */
			ammo ++ ;
        }
		else {	/* incorrect composition */
			return 0 ;
        }
    }

	return ammo ;
}

/******************************************I*
  ALOM Quest - Idea by 'Mola'   code by Cold
 ********************************************/

/* composition check in retreatment machine */
int check_alom_composition(struct obj_data *list)
{
	struct obj_data	*obj, *next_obj ;
	int	i, alom[5], vnum ;

	if ( list->obj_flags.type_flag != ITEM_CONTAINER )
		return FALSE ;

	/* count content */
	for ( i = 0 ; i < 5 ; i ++ )
		alom[i] = 0 ;

	for ( obj = list->contains ; obj ; obj = next_obj ) {
		next_obj = obj->next_content ;
		vnum = obj_index[obj->item_number].virtual ;
		switch (vnum) {
        case 12106 :	alom[0] ++ ; break ;	/* paper 1 */
        case 12107 :	alom[1] ++ ; break ;	/* paper 2 */
        case 12108 :	alom[2] ++ ; break ;	/* paper 3 */
        case 12109 :	alom[3] ++ ; break ;	/* paper 4 */
        case 12110 :	alom[4] ++ ; break ;	/* Glue */
        default : 
            return 0 ;	/* failed : incorrect composition */
        }
    }

	if ( alom[0] && alom[1] && alom[2] && alom[3] && alom[4] )
		return (1) ;	/* correct composition */
	else
		return 0 ;
}

void alom_quest_reward(struct char_data *ch)
{
	ch->points.max_hit += 20 ;
	send_to_char_han("Your HIT POINT increased !!!\n\r",
                     "당신의 HIT POINT 가 늘었습니다 !!!\n\r", ch) ;
	affect_total(ch) ; 
	save_char(ch, NOWHERE) ;
}

