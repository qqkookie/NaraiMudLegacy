/* ************************************************************************
*  file: spec_procs3.c , Special module.                  Part of DIKUMUD *
*  Usage: Procedures handling special procedures for object/room/mobile   *
*  Made by Choi Yeon Beom in KAIST                                        *
************************************************************************* */
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <time.h>

#include "structs.h"
#include "utils.h"
#include "comm.h"
#include "interpreter.h"
#include "handler.h"
#include "db.h"
#include "spells.h"
#include "limits.h"
#include "mob_magic.h"		/* by cyb */
#include "quest.h"	/* by cyb */
#include "command.h"

/*   external vars  */
extern struct room_data *world;
extern struct char_data *character_list;
extern struct descriptor_data *descriptor_list;
extern struct index_data *obj_index;
extern struct time_info_data time_info;
extern struct index_data *mob_index;

/* extern procedures */
void hit(struct char_data *ch, struct char_data *victim, int type);
void stop_fighting(struct char_data *ch);

int sonogong(struct char_data *ch, int cmd, char *arg)
{
	struct char_data	*tmp_ch, *mob1, *mob2 ;
	struct obj_data	*ring ;
	struct follow_type	*fol ;
	int	r_num, count ;

	if (cmd)
		return FALSE ;

	if ( ch->specials.fighting ) {	/* son ogong is fighting now */
		if ((ring = ch->equipment[WEAR_HEAD] )) {
			/* check 금태 (golden ring) */
			if (obj_index[ring->item_number].virtual == 18033) { 
				act("$n이 괴로운 듯이 머리를 두 손으로 감싸쥡니다.",
					TRUE, ch, 0, 0, TO_ROOM);
				return TRUE ;
            }
        }

		if (( 100*GET_HIT(ch)/GET_MAX_HIT(ch)) < 90 ) {
			act("$n utters the words 'lup sjagh'.", TRUE, ch, 0, 0, TO_ROOM);
			GET_HIT(ch) += 5000 ;
			if (GET_HIT(ch) >= hit_limit(ch))
				GET_HIT(ch) = hit_limit(ch)-number(1,4);
        }

		/* 분신의 수를 셈.. */
		for (count = 0, fol = ch->followers ; fol && count < 7 ; count ++ )
			fol = fol->next ;

		/* 분신 - 18000 으로.. */
		if ( count < 7 && (r_num = real_mobile(18000)) ) {
			act("$n utters 'QnsTls QnsTls!'.", TRUE, ch, 0, 0, TO_ROOM) ;
			mob1 = read_mobile(r_num, REAL) ;
			char_to_room(mob1, ch->in_room) ;
			add_follower(mob1, ch) ;
			mob2 = read_mobile(r_num, REAL) ;
			char_to_room(mob2, ch->in_room) ;
			add_follower(mob2, ch) ;

			/* mob1 rescues sonogong */
			tmp_ch = world[ch->in_room].people ;
			for ( ; tmp_ch && (tmp_ch->specials.fighting != ch) ;
                  tmp_ch = tmp_ch->next_in_room)  ;

			if ( tmp_ch ) {	/* tmp_ch is fighting against sonogong */
				stop_fighting(tmp_ch) ;
				set_fighting(mob1, tmp_ch) ;
				set_fighting(tmp_ch, mob1) ;
            }

			/* mob2 rescues sonogong */
			tmp_ch = world[ch->in_room].people ;
			for ( ; tmp_ch && (tmp_ch->specials.fighting != ch) ;
                  tmp_ch = tmp_ch->next_in_room)  ;

			if ( tmp_ch ) {	/* tmp_ch is fighting against sonogong */
				stop_fighting(tmp_ch) ;
				set_fighting(mob2, tmp_ch) ;
				set_fighting(tmp_ch, mob2) ;
            }
        }
    }
	else {
		/*
          "머리를 긁적입니다."
          "이거 어디다 쓰는 물건이지 ?",
          "금으로 두른 거 보면 아주 멋있어 보이는 데~~~",
          "아하, 머리에 쓰는 것이군, 이야 멋있네요..."
		*/
    }

	return FALSE ;
}

int son_boonsin(struct char_data *ch, int cmd, char *arg)
{
	struct char_data	*master, *vict ;
	struct obj_data	*obj ;
	int	was_in ;

	if (cmd)
		return FALSE ;

	if ( !(master = ch->master) )	{	/* master son ogong is dead ! */
		/* turn into hair */
		was_in = ch->in_room ;
		act("$n 주위로 흰 연기구름이 일더니 서서히 사라집니다.",
			TRUE, ch, 0, 0, TO_ROOM) ;
		extract_char(ch) ;
		if (( obj = read_object(18000, VIRTUAL)) == 0) {
			log("son_boonsin: load hair failed") ;
			send_to_room("벌레가 한마리 나타나더니 땅속으로 기어들어갑니다.\n\r",
                         was_in) ;
        }
		else {
			obj_to_room(obj, was_in) ;
        }
    }
	else if ( ch->specials.fighting && (100*GET_HIT(ch)/GET_MAX_HIT(ch) < 50)) {
		vict = ch->specials.fighting ;
		act("$n이 '최후의 일격!' 하고 소리칩니다.", TRUE, ch, 0, 0, TO_ROOM) ;

		if ( GET_HIT(vict) < 200 )
			damage(ch, ch->specials.fighting, dice(10,36), TYPE_UNDEFINED);
		else if (GET_HIT(vict) > 1000)
			damage(ch, ch->specials.fighting, dice(10,90), TYPE_UNDEFINED);
		else
			damage(ch, ch->specials.fighting, dice(10,55), TYPE_UNDEFINED);

		GET_HIT(ch) = dice(5,10) ;
    }
	else if ( ch->in_room != master->in_room ) {
		act("작은 연기구름이 주위를 감쌉니다.", TRUE, ch, 0, 0, TO_ROOM) ;
		char_from_room(ch) ;
		char_to_room(ch, master->in_room) ;
		act("작은 연기구름이 땅위에서 피어오릅니다.", TRUE, ch, 0, 0, TO_ROOM) ;
    }
	else {
		return FALSE ;
    }

	return TRUE ;
}

int fourth_jangro(struct char_data *ch, int cmd, char *arg)
{
    static int gold_mode, amulet_mode ;

	if (cmd)
		return FALSE ;

	/* gold mode on/off */
	/* amulet_mode on/off */

	if ( GET_GOLD(ch) < 2000000 ) {
		act("$n님이 '누구 돈주는 사람 없나..' 하고 중얼 거립니다.",
			TRUE, ch, 0, 0, TO_ROOM) ;
		/*
          "당신이 돈을 조금 주었으니깐, 뭐라고 중얼거리며"
          "이 주문은 아주 어두운 방에서 읽어야 효력이 있을 것이요."
		*/
    }
}
