/* ************************************************************************
*  file: spec_procs.c , Special module.                   Part of DIKUMUD *
*  Usage: Procedures handling special procedures for object/room/mobile   *
*  Copyright (C) 1990, 1991 - see 'license.doc' for complete information. *
************************************************************************* */

#include <stdio.h>
#include <string.h>
#include <ctype.h>

#include "structs.h"
#include "utils.h"
#include "comm.h"
#include "interpreter.h"
#include "handler.h"
#include "db.h"
#include "spells.h"
#include "limit.h"

extern struct index_data *obj_index;
extern struct room_data *world;
extern int top_of_world;

/* it should be initialized in read_mobile */
struct char_data *son_ogong;
struct char_data *fourth_jangro;

#define SON_OGONG			11101
#define SON_OGONG_MIRROR	11141
#define FOURTH_JANGRO		11132

#define SON_OGONG_STEP		(son_ogong->quest.solved)
#define FOURTH_JANGRO_STEP	(fourth_jangro->quest.solved)

#define DAERIMSA_PEN		11123
#define DAERIMSA_PAPER		11124
#define DAERIMSA_SCROLL		11132

#define GOLDEN_RIM			11127
#define SON_OGONG_BONG		11126

void first_attack(struct char_data *ch, struct char_data *victim);
void wear(struct char_data *ch, struct obj_data *obj, int where_flag);
void do_drop(struct char_data *ch, char *argument, int cmd);
void do_wear(struct char_data *ch, char *argument, int cmd);
void do_give(struct char_data *ch, char *argument, int cmd);
int number(int from, int to);
void log(char *str);
void do_look(struct char_data *ch, char *argument, int cmd);


int son_ogong_func(struct char_data *ch, int cmd, char *arg)
{
	struct obj_data *i, *obj;
	struct char_data *victim, *mob;

	if(cmd){
		if(cmd != 207 && cmd !=  235 && cmd != 268 && cmd != 72 && cmd != 13)
			return 0;
	}

	/* fighting */
	if((victim = son_ogong->specials.fighting)){
		/* can't */
		/*
			cast = 84 (X)
			flee = 151
			recite = 207
			lightning move = 235
			disarm = 268
		*/
		if(cmd == 207 || cmd == 235 || cmd == 268){
			acthan(
				"Ogong makes a golden-barrier, it prevents your activity.",
				"오공이 금빛 막을 만들어서 당신의 움직임을 방해합니다.",
				FALSE, son_ogong, 0, 0, TO_ROOM);
			return 1;
		}
		else if(cmd) return 0;

		if(SON_OGONG_STEP < 2){
			/* make mirror */
			acthan(
				"Ogong pulls out a hair, and throws it.",
				"오공이 머리털 하나를 뽑하서 던집니다.",
				FALSE, son_ogong, 0, 0, TO_ROOM);
			mob = read_mobile(SON_OGONG_MIRROR, VIRTUAL);
			char_to_room(mob, son_ogong->in_room);
			first_attack(mob, victim);
		}

		if(SON_OGONG_STEP == 2){
			/* 여의봉 */
			acthan(
				"Ogong gets a bong in his ear.",
				"오공이 귓속에서 여의봉을 꺼냅니다.",
				FALSE, son_ogong, 0, 0, TO_ROOM);
			obj = read_object(SON_OGONG_BONG, VIRTUAL);
			wear(son_ogong, obj, 12);
			SON_OGONG_STEP = 3;
		}
	}
	/* no fighting */
	else {
		switch(SON_OGONG_STEP){
			case 0 :
				if(!cmd){
					for(i = son_ogong->carrying; i ; i = i->next_content)
						if(obj_index[i->item_number].virtual == GOLDEN_RIM){
							acthan(
								"Ogong asks \"What's this?\"",
								"\"이거 머하는 거지?\"라고 묻는다.",
								FALSE, son_ogong, 0, 0, TO_ROOM);
							do_drop(son_ogong, " rim", 60);
							break;
						}
					return 1;
				}
				else if(cmd == 13){
					/* wear */
					do_wear(ch, arg, cmd);
					if(ch->equipment[WEAR_HEAD]
						&& ch->equipment[WEAR_HEAD]->item_number >= 0
						&& obj_index[ch->equipment[WEAR_HEAD]->item_number].virtual == GOLDEN_RIM){
						acthan("$n looks at you.", "$n님이 당신을 바라봅니다.",
							TRUE, son_ogong, 0, ch, TO_VICT);
						acthan("$n looks at $N.", "$n님이 $N님을 바라봅니다.",
							TRUE, son_ogong, 0, ch, TO_NOTVICT);
						acthan(
							"Ogong says \"Wow~ Beautiful!!!\".",
							"오공이 \"와~~ 멋있다!!!\" 라고 말한다.",
							FALSE, son_ogong, 0, 0, TO_ROOM);
						SON_OGONG_STEP = 1;
					}
					return 1;
				}
				break;
			case 1 :
				if(cmd == 72){
					/* GIVE */
					do_give(ch, arg, cmd);
					for(i = son_ogong->carrying; i ; i = i->next_content)
						if(obj_index[i->item_number].virtual == GOLDEN_RIM){
							break;
						}
					if(i){
						do_wear(son_ogong, " rim", 13);
						acthan(
							"Ogong says \"Yaho~ It's beautiful!!!\".",
							"오공이 \"야호~ 멋있다!!!\"라고 외친다.",
							FALSE, son_ogong, 0, 0, TO_ROOM);
						SON_OGONG_STEP = 2;
					}
					return 1;
				}
				break;
		}
	}

/*
sprintf(buf, "son_special : %s(%d)\n", GET_NAME(ch), cmd);
log(buf);
*/
	return 0;
}

int fourth_jangro_func(struct char_data *ch, int cmd, char *arg)
{
	long gold;
	struct obj_data *i, *obj, *pen, *paper;

	switch(FOURTH_JANGRO_STEP){
		case 0 :
		case 1 :
			/* 돈을 받는다 */
			if(cmd == 72){
				/* give */
				gold = GET_GOLD(fourth_jangro);
				do_give(ch, arg, cmd);

				if((GET_GOLD(fourth_jangro) > 5000000) ||
				   (GET_GOLD(fourth_jangro) > gold + 2000000)){
					acthan(
						"Ok!!! Enough gold!!! Thanks... Bow, bow, bow!!!",
						"충분해요... 충분해... 정말 고마워요. 감사~ 감사~~",
						FALSE, fourth_jangro, 0, 0, TO_ROOM);
					FOURTH_JANGRO_STEP = 2;
				}
				else {
					acthan(
						"Not enough gold!!! Need more!!!",
						"흠. 아직 부족해!!! 좀 더 많은 돈이 필요한데...",
						FALSE, fourth_jangro, 0, 0, TO_ROOM);
				}
				return 1;
			}
			else if(!cmd){
				/* 고민을 듣는다. */
				if(number(1, 2) ==  1){
					acthan(
						"4th jangro says \"I need money... some money\".",
						"4 장로가 \"돈이 필요해... 돈이...\" 라고 중얼거립니다.",
						FALSE, ch, 0, 0, TO_ROOM);
					return 1;
				}
				return 0;
			}
			else return 0;
			break;
		case 2 :
			/* ask */
			if(cmd == 86){	/* ask */
				acthan(
					"What? Ah~ You want to know how to enter the tower?",
					"뭐라고요? 아~~ 대림사 탑에 들어가는 길을?",
					FALSE, fourth_jangro, 0, 0, TO_ROOM);
				acthan(
					"Hmm. It's TOP Secret in Daerimsa. But...",
					"음냐. 그거 대림사 최고 비밀인데.... 하지만...",
					FALSE, fourth_jangro, 0, 0, TO_ROOM);
				acthan(
					"Give me a pen and a paper! I'll make it.",
					"붓과 종이를 주세요... 할 수 없지.",
					FALSE, fourth_jangro, 0, 0, TO_ROOM);

				return 1;
			}
			/* 붓과 종이 받기 */
			else if(cmd == 72){
				do_give(ch, arg, cmd);

				pen = paper = NULL;
				for(i = fourth_jangro->carrying; i ; i = i->next_content){
					if(obj_index[i->item_number].virtual == DAERIMSA_PEN)
						pen = i;
					else if(obj_index[i->item_number].virtual == DAERIMSA_PAPER)
						paper = i;
				}

				if(pen && paper){
					extract_obj(pen);
					extract_obj(paper);

					FOURTH_JANGRO_STEP = 4;
				}
				else {
					acthan(
						"Give me a pen and a paper!",
						"붓과 종이를 주세요...",
						FALSE, fourth_jangro, 0, 0, TO_ROOM);
				}
				return 1;
			}
			else return 0;
			break;
		case 4 :
			if(cmd) return 0;

			switch(number(0, 4)){
				case 0 :
				case 1 :
					acthan(
						"4th jangro thinks!!!",
						"4 장로가 고민합니다...",
						FALSE, ch, 0, 0, TO_ROOM);
					break;
				case 2 :
					acthan(
						"4th jangro stares out!!!",
						"4 장로가 밖을 쳐다봅니다...",
						FALSE, ch, 0, 0, TO_ROOM);
					break;
				default :
					acthan(
						"4th jangro drops a scroll.",
						"4 장로가 두루마리를 내려 놓습니다.",
						FALSE, ch, 0, 0, TO_ROOM);
					acthan(
						"4th jangro says \"Recite this scroll in the dark room\".",
						"\"어두운 방에서 이 두루마리를 읽으세요\"라고 말합니다.",
						FALSE, ch, 0, 0, TO_ROOM);
					obj = read_object(DAERIMSA_SCROLL, VIRTUAL);
					obj_to_room(obj, fourth_jangro->in_room);
					FOURTH_JANGRO_STEP = 5;
					break;
			}
			break;
		case 5 :
			if(cmd) return 0;

			if(number(1, 5) == 1){
				acthan(
					"NamuAmitabul!!!",
					"나무아미타불!!!",
					FALSE, ch, 0, 0, TO_ROOM);
				return 1;
			}
			break;
	}

	return 0;
}

#define DARK_ROOM_IN_DAERIMSA		11134
#define TOWER_IN_DAERIMSA			11161

int teleport_daerimsa_tower(struct char_data *ch, int cmd, char *arg)
{
	struct follow_type *f;
	struct char_data *victim;
	int i, ch_in_room;
	struct obj_data *scroll;
	char buf[256];

	/* recite */
	if(cmd == 207){
		arg = one_argument(arg, buf);
		if(!(scroll = get_obj_in_list_vis(ch, buf, ch->carrying))){
			act("You do not have that item.", FALSE, ch, 0, 0, TO_CHAR);
			return 1;
		}

		if(obj_index[scroll->item_number].virtual != DAERIMSA_SCROLL) return 0;

		if(world[ch->in_room].number == DARK_ROOM_IN_DAERIMSA){
			/* transport all memeber to TOWER */
			for(i = 0; i <= top_of_world; i ++)
				if(world[i].number == TOWER_IN_DAERIMSA) break;
			if(i == top_of_world){
				log("NO-EXIST ROOM");
				return 0;
			}

			ch_in_room = ch->in_room;

			victim = ch;
			if(ch->master){
				victim = ch->master;
			}
			f = victim->followers;

			if(victim->in_room == ch_in_room){
				act("$n disappears... suddenly", FALSE, victim, 0, 0, TO_ROOM);
				char_from_room(victim);
				char_to_room(victim, i);
				do_look(victim, "", 15);
			}

			while(f){
				if(f->follower->in_room == ch_in_room){
					act("$n disappears... suddenly",
						FALSE, f->follower, 0, 0, TO_ROOM);
					char_from_room(f->follower);
					char_to_room(f->follower, i);
					do_look(f->follower, "", 15);
				}

				f = f->next;
			}

			extract_obj(scroll);
		}
		else {
			act("You can't see anything.", FALSE, ch, 0, 0, TO_CHAR);
		}
		return 1;
	}

	return 0;
}

#define SAINT_WATER	11134

int saint_water(struct char_data *ch, int cmd, char *arg)
{
	char buf[100];
	struct obj_data *obj;

	if(cmd != 206) return FALSE;

	/* quaff */
	one_argument(arg, buf);
	if(!(obj = get_obj_in_list_vis(ch, buf, ch->carrying)))
		return FALSE;

	if(obj_index[obj->item_number].virtual != SAINT_WATER)
		return FALSE;

	GET_ALIGNMENT(ch) = 999;
	act("흠. 조금 찝찝한 맛이군...", TRUE, ch, 0, 0, TO_ROOM);
	send_to_char("갑자기 성인이 된 듯한 기분이 듭니다.\n\r", ch);

	extract_obj(obj);

	return TRUE;
}
