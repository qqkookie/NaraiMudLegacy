/*  ************************************************************************
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

extern struct char_data *character_list;
extern struct index_data *obj_index;
extern struct room_data *world;
extern int top_of_world;
extern int movement_loss[];
extern char *dirs[];

#define GBISLAND_SEASHORE			23302
#define GBISLAND_MIRROR_SAINT		23321
#define GBISLAND_MAGIC_BARRIER_OUT	23331
#define GBISLAND_BOTTLE				23312

#define GBISLAND_MAGIC_PAPER1		23306
#define GBISLAND_MAGIC_PAPER2		23307
#define GBISLAND_MAGIC_PAPER		23308

#define GBISLAND_SEED_EVIL_POWER	23309

int number(int from, int to);
void wipe_stash(char *filename);
void save_char_nocon(struct char_data *ch, sh_int load_room );
void raw_kill(struct char_data *ch, int level);
void do_look(struct char_data *ch, char *argument, int cmd);
void do_say(struct char_data *ch, char *str, int cmd);
void do_open(struct char_data *ch, char *argument, int cmd);
void do_move(struct char_data *ch, char *argument, int cmd);
void do_give(struct char_data *ch, char *argument, int cmd);
int MAX(int a, int b);
int MIN(int a, int b);

void gbisland_move_seashore(struct char_data *ch)
{
	int need_movement;
	int vnum_seashore;
	struct obj_data *obj, *next_obj;

	need_movement = movement_loss[world[ch->in_room].sector_type];

	if(GET_MOVE(ch) < need_movement && !IS_NPC(ch) && GET_LEVEL(ch) < IMO){
		send_to_char("You are too exhausted.\n\r",ch);
		return;
	}

	send_to_char("\n\r", ch);
	send_to_char("갑자기 하늘이 먹구름으로 가득찹니다.\n\r", ch);
	send_to_char(".........\n\r", ch);
	send_to_char(".........\n\r", ch);
	send_to_char(".........\n\r", ch);
	send_to_char("빗방울이 굵어 지더니 배가 심하게 흔들립니다.\n\r", ch);
	send_to_char("당신은 배의 난간을 잡고 힘껏 버티어 봅니다.\n\r", ch);
	send_to_char("난간이 부서져 나가고 당신은 배의 반대쪽으로 쓰러져 갑니다.\n\r", ch);
	send_to_char(".........\n\r", ch);
	send_to_char(".........\n\r", ch);
	send_to_char("설상가상으로 멀리 소용돌이가 보입니다.\n\r", ch);
	send_to_char("당신은 당신의 배와 함께 소용돌이로 빨려들어갑니다.\n\r", ch);
	send_to_char(".........\n\r", ch);
	send_to_char(".........\n\r", ch);
	send_to_char(".........\n\r", ch);
	send_to_char("소용돌이의 힘에 배가 부서지고, 당신은 바다로 내동댕이 쳐 집니다.\n\r", ch);
	send_to_char(".........\n\r", ch);
	send_to_char("헤엄을 잘치는 당신이지만 소용돌이의 힘에서는 어쩔수 없습니다.\n\r", ch);
	send_to_char("당신은 점차로 의식을 일어갑니다.\n\r", ch);
	send_to_char(".........\n\r", ch);
	send_to_char("으~~~\n\r", ch);
	send_to_char("으~~\n\r", ch);
	send_to_char("으~\n\r", ch);
	send_to_char("~\n\r", ch);
	send_to_char("\n\r", ch);
	send_to_char("\n\r", ch);
	send_to_char("\n\r", ch);
	send_to_char("\n\r", ch);
	send_to_char("\n\r", ch);
	send_to_char("\n\r", ch);
	send_to_char("\n\r", ch);

	char_from_room(ch);

	/* goto seashore */
	vnum_seashore = GBISLAND_SEASHORE + number(0, 2);
	char_to_room(ch, real_room(vnum_seashore));

	/* break all items in ch's inventory */
	for(obj = ch->carrying; obj; obj = next_obj){
		next_obj = obj->next_content;
		extract_obj(obj);
	}

	/* 체력이 떨어짐. */
	GET_HIT(ch) = number(1, GET_PLAYER_MAX_HIT(ch) / 5000 + 1);
	GET_MANA(ch) = number(1, GET_PLAYER_MAX_MANA(ch) / 5000 + 1);
	GET_MOVE(ch) = number(1, GET_PLAYER_MAX_MOVE(ch) / 5000 + 1);

	/* check - remortal */
	if((ch->player.remortal < 15) && (GET_LEVEL(ch) < IMO)){
		send_to_char("당신은 소용돌이에 휘말려 정신을 차리지 못합니다.\n\r", ch);
		send_to_char("당신은 죽습니다.\n\r", ch);

		wipe_stash(GET_NAME(ch));
		GET_GOLD(ch) = 0;
		save_char(ch, ch->in_room);

		if(!IS_NPC(ch)){
			save_char_nocon(ch, world[ch->in_room].number);
		}

		raw_kill(ch, 44);

		return;
	}

	if(!IS_AFFECTED(ch, AFF_SNEAK)){
		act("$n 파도에 밀려 옵니다.", TRUE, ch, 0, 0, TO_ROOM);
	}
	do_look(ch, "\0", 15);
}

int gbisland_sea(struct char_data *ch, int cmd, char *arg)
{
	int was_in;
	struct follow_type *k;

	/* cmd==2, "east" */
	/* cmd==151, "flee" */
	/* if(cmd != 2 && cmd != 151) return FALSE; */
	/* cmd==112, "flee" by Moon */
	if(cmd != 2 && cmd != 112) return FALSE;

	was_in = ch->in_room;

	if(!IS_AFFECTED(ch, AFF_SNEAK)){
		act("$n leaves east.", TRUE, ch, 0, 0, TO_ROOM);
	}

	if(ch->followers){
		for(k = ch->followers; k; k = k->next){
			if((was_in == k->follower->in_room) &&
			   (GET_POS(k->follower) >= POSITION_STANDING)){
				act("You follow $N.", FALSE, k->follower, 0, ch, TO_CHAR);
				send_to_char("\n\r", k->follower);

				gbisland_move_seashore(k->follower);
			}
		}
	}
	gbisland_move_seashore(ch);

	return TRUE;
}

void gbisland_false_move(struct char_data *ch, int dir)
{
	int was_in;
	char tmp[256];
	struct follow_type *k;

	was_in = ch->in_room;

	if(!IS_AFFECTED(ch, AFF_SNEAK)){
		sprintf(tmp, "$n leaves %s.", dirs[dir - 1]);
		act(tmp, TRUE, ch, 0, 0, TO_ROOM);
	}

	if(ch->followers){
		for(k = ch->followers; k; k = k->next){
			if((was_in == k->follower->in_room) &&
			   (GET_POS(k->follower) >= POSITION_STANDING)){
				act("You follow $N.", FALSE, k->follower, 0, ch, TO_CHAR);

				if(!IS_AFFECTED(ch, AFF_SNEAK)){
					sprintf(tmp, "$n leaves %s.", dirs[dir - 1]);
					act(tmp, TRUE, k->follower, 0, 0, TO_ROOM);
				}
			}
		}
	}

	if(!IS_AFFECTED(ch, AFF_SNEAK)){
		act("$n has arrived.", TRUE, ch, 0, 0, TO_ROOM);
	}
	if(ch->followers){
		for(k = ch->followers; k; k = k->next){
			if((was_in == k->follower->in_room) &&
			   (GET_POS(k->follower) >= POSITION_STANDING)){
				if(!IS_AFFECTED(ch, AFF_SNEAK)){
					act("$n has arrived.", TRUE, k->follower, 0, 0, TO_ROOM);
				}

				do_look(k->follower, "\0", 15);
			}
		}
	}
}

void gbisland_go_out_barrier(struct char_data *ch)
{
	int was_in;
	char tmp[256];
	struct follow_type *k;
	int dir;

	was_in = ch->in_room;
	dir = number(1, 4);

	if(!IS_AFFECTED(ch, AFF_SNEAK)){
		sprintf(tmp, "$n leaves %s.", dirs[dir - 1]);
		act(tmp, TRUE, ch, 0, 0, TO_ROOM);
	}

	char_from_room(ch);
	char_to_room(ch, real_room(GBISLAND_MAGIC_BARRIER_OUT));

	if(!IS_AFFECTED(ch, AFF_SNEAK)){
		act("$n has arrived.", TRUE, ch, 0, 0, TO_ROOM);
	}

	if(ch->followers){
		for(k = ch->followers; k; k = k->next){
			if((was_in == k->follower->in_room) &&
			   (GET_POS(k->follower) >= POSITION_STANDING)){
				act("You follow $N.", FALSE, k->follower, 0, ch, TO_CHAR);
				send_to_char("\n\r", k->follower);

				if(!IS_AFFECTED(ch, AFF_SNEAK)){
					sprintf(tmp, "$n leaves %s.", dirs[dir - 1]);
					act(tmp, TRUE, k->follower, 0, 0, TO_ROOM);
				}

				char_from_room(k->follower);
				char_to_room(k->follower, real_room(GBISLAND_MAGIC_BARRIER_OUT));

				if(!IS_AFFECTED(ch, AFF_SNEAK)){
					act("$n has arrived.", TRUE, k->follower, 0, 0, TO_ROOM);
				}

				do_look(k->follower, "\0", 15);
			}
		}
	}
}

void gbisland_go_back(struct char_data *ch)
{
	// int was_in;
	char tmp[256];
	int dir;

	// was_in = ch->in_room;
	dir = number(1, 4);

	do_say(ch, "신의 은총이 함께하길...", 0);

	if(!IS_AFFECTED(ch, AFF_SNEAK)){
		sprintf(tmp, "$n leaves %s.", dirs[dir - 1]);
		act(tmp, TRUE, ch, 0, 0, TO_ROOM);
	}

	char_from_room(ch);
	char_to_room(ch, real_room(GBISLAND_MIRROR_SAINT));

	if(!IS_AFFECTED(ch, AFF_SNEAK)){
		act("$n has arrived.", TRUE, ch, 0, 0, TO_ROOM);
	}
}

int gbisland_saint_mirror(struct char_data *ch, int cmd, char *arg)
{
	static bool move = FALSE;
	static bool ask = FALSE;
	static bool give = FALSE;

	static char path[] = "O1M1M2M1M1M1R0B0";
	static int index = 0;

	static char *message[] = {
		"저는 성녀 라네사입니다.",
		"저는 지금 악마성주 카암에게 소환되어 지하감옥에 있습니다.",
		"여러분 앞의 저의 모습은 저의 분신입니다.",
		"악마성주 카암을 물리치고 마을을 구해주세요.",
		"옛날부터 내려오던 부적을 찾아서 이용하시면 좋을 것입니다.",
		"악마군이 쳐들어 오지 못하도록 마을의 주위에 마법의 진을 설치했습니다."
	};
	static int bottle;

	int dir;
	char tmp[256];
	struct obj_data *obj, *next_obj;

	/* cmd==0, no-command */
	/* cmd == 86, "ask" */
	/* cmd == 72, "give" */
	if(cmd != 0 && cmd != 86 && cmd != 72)
		return FALSE;

	if(cmd == 0){
		if(give){
			for(obj = ch->carrying; obj; obj = next_obj){
				next_obj = obj->next_content;
				if(obj_index[obj->item_number].virtual == GBISLAND_BOTTLE)
					bottle = 6;
				extract_obj(obj);
			}

			if(bottle > 0){
				do_say(ch, message[6 - bottle], 0);
				bottle --;
			}
			else {
				give = FALSE;
			}
		}
		else if(ask){
			if(!move){
				do_say(ch, "저를 따라오세요...", 0);
				do_say(ch, "절대로 마법의 진 안에서 다른 곳을 밟으면 안됩니다.", 0);

				move = TRUE;
			}
			else {
				do_say(ch, "지금 가고 있습니다...", 0);
				do_say(ch, "저를 따라오세요...", 0);
			}

			ask = FALSE;
		}
		else if(move){
			dir = (char)(path[index + 1]) - '0' + 1;
			switch(path[index]){
				case 'O' :	sprintf(tmp, "door %s", dirs[dir - 1]);
							do_open(ch, tmp, 0);
							index += 2;
							break;
				case 'M' :	do_move(ch, "", dir);
							index += 2;
							break;
				case 'R' :	if((dir = number(1, 5)) < 5){
								/* false-move */
								gbisland_false_move(ch, dir);
							}
							else {
								/* go out */
								gbisland_go_out_barrier(ch);
								index += 2;
							}
							break;
				case 'B' :	gbisland_go_back(ch);
							index = 0;
							move = FALSE;
							break;
			}
		}
	}
	else if(cmd == 86){		/* ask */
		send_to_char("Ok.\n\r", ch);
		ask = TRUE;
	}
	else if(cmd == 72){
		do_give(ch, arg, cmd);
		give = TRUE;
	}

	return TRUE;
}

int gbisland_lanessa(struct char_data *ch, int cmd, char *arg)
{
	static bool give;

	struct obj_data *obj, *next_obj, *paper1, *paper2;
	int rnum;

	/* cmd==0, no-command */
	/* cmd == 72, "give" */
	if(cmd != 0 && cmd != 72)
		return FALSE;

	if(cmd == 0){
		if(give){
			paper1 = NULL;
			paper2 = NULL;
			for(obj = ch->carrying; obj; obj = next_obj){
				next_obj = obj->next_content;
				if(obj_index[obj->item_number].virtual == GBISLAND_MAGIC_PAPER1)
					paper1 = obj;
				else if(obj_index[obj->item_number].virtual == GBISLAND_MAGIC_PAPER2)
					paper2 = obj;
				else extract_obj(obj);
			}

			if(paper1 && paper2){
				extract_obj(paper1);
				extract_obj(paper2);

				rnum = real_object(GBISLAND_MAGIC_PAPER);
				obj = read_object(rnum, REAL);

				do_say(ch, "제 영혼으로 부적을 붙여야해요...", 0);
				do_say(ch, "...", 0);
				do_say(ch, "그럼, 부디, 카암을 물리치시길...", 0);
				do_say(ch, "그리고, 마을을 지켜주세요...", 0);
				do_say(ch, "그럼... 신의 은총이 함께 하길...", 0);

				act("성녀 라네사의 몸이 성스러운 빛에 가려 사라집니다.",
						TRUE, ch, 0, 0, TO_ROOM);
				act("성스러운 빛이 사라지면서 부적이 완성됩니다.",
						TRUE, ch, 0, 0, TO_ROOM);
				act("성녀 라네사의 아름다운 모습이 부적에 살아 있는 듯 합니다.",
						TRUE, ch, 0, 0, TO_ROOM);

				obj_to_room(obj, ch->in_room);

				extract_char(ch);
			}
			else if(paper1 || paper2){
				do_say(ch, "다른 부분도 마저 찾아 주세요...", 0);
			}
		}
	}
	else if(cmd == 72){
		do_give(ch, arg, cmd);
		give = TRUE;
	}

	return TRUE;
}

int gbisland_carpie(struct char_data *ch, int cmd, char *arg)
{
	int room;
	struct char_data *victim;
	struct affected_type *af;

	if(cmd) return FALSE;

	if(GET_POS(ch) == POSITION_SLEEPING) return FALSE;

	room = ch->in_room;
	for(victim = character_list; victim; victim = victim->next){
		if(victim->in_room == room){
			if(GET_POS(victim) == POSITION_SLEEPING){
				for(af = victim->affected; af; af = af->next){
					if(af->type == SPELL_SLEEP){
						affect_remove(victim, af);
					}
				}

				send_to_char("누군가 당신의 허벅지를 꼬집습니다.\n\r", victim);
				GET_POS(victim) = POSITION_STANDING;

				if(IS_NPC(victim)){
					SET_BIT(victim->specials.act, ACT_AGGRESSIVE);
				}
			}
		}
	}

	return TRUE;
}

int gbisland_magic_paper(struct char_data *ch, int cmd, char *arg)
{
	char name[256];
	struct obj_data *obj;
	struct char_data *victim;
	int room;
	struct affected_type af;
	int duration;

	/* cmd == 207, "recite" */
	if(cmd != 207) return FALSE;

	one_argument(arg, name);
	obj = get_obj_in_list_vis(ch, name, ch->carrying);
	if(!obj) return FALSE;

	if(obj_index[obj->item_number].virtual != GBISLAND_MAGIC_PAPER)
		return FALSE;

	act("성스러운 기운이 방안에 가득히 넘쳐 흐릅니다.",
			FALSE, ch, 0, 0, TO_ROOM);
	send_to_char("성스러운 기운이 방안에 가득히 넘쳐 흐릅니다.\n\r", ch);

	room = ch->in_room;
	for(victim = character_list; victim; victim = victim->next){
		if(victim->in_room == room){
			duration = GET_ALIGNMENT(victim) / 100;
			duration = (duration > 0) ? duration : (-duration);
			duration = (duration > 4) ? duration : 4;

			/* DR */
			af.type      = SPELL_DAMAGE_UP;
			af.duration  = duration;
			af.modifier  = GET_ALIGNMENT(victim) / 3 + 5;
			af.location  = APPLY_DAMROLL;
			af.bitvector = 0;
			affect_to_char(victim, &af);

			/* AC */
			af.type      = SPELL_ARMOR;
			af.duration  = duration;
			af.modifier  = -GET_ALIGNMENT(victim) / 3 - 5;
			af.location  = APPLY_AC;
			af.bitvector = 0;
			affect_to_char(victim, &af);

			/* HR */
			af.type      = SPELL_BLESS;
			af.duration  = duration;
			af.modifier  = GET_ALIGNMENT(victim) / 4 + 5;
			af.location  = APPLY_HITROLL;
			af.bitvector = 0;
			affect_to_char(victim, &af);

			if(GET_ALIGNMENT(victim) > 0){
				/* good */
				send_to_char("성스러운 기운이 당신에게 힘을 줍니다.\n\r", victim);
			}
			else {
				/* evil */
				send_to_char("성스러운 기운에 당신은 힘이 빠집니다.\n\r", victim);
			}
		}
	}

	extract_obj(obj);
	return TRUE;
}

int gbisland_seed_evil_power(struct char_data *ch, int cmd, char *arg)
{
	char name[256];
	struct obj_data *obj;
	int skno, skilled;
	int i;

	/* cmd == 12, "eat" */
	if(cmd != 12) return FALSE;

	one_argument(arg, name);
	obj = get_obj_in_list_vis(ch,name,ch->carrying);
	if(!obj) return FALSE;

	if(obj_index[obj->item_number].virtual != GBISLAND_SEED_EVIL_POWER)
		return FALSE;

	for(i = 0; i < 5; i ++){
		skno = number(0, MAX_SKILLS - 1);
		skilled = ch->skills[skno].skilled - GET_ALIGNMENT(ch) / 40;
		skilled = MAX(0, skilled);
		ch->skills[skno].skilled = MIN(skilled , 100);
	}

	send_to_char("당신은 갑자기 사악한 기분이 듭니다.\n\r", ch);
	send_to_char("당신은 악마가 됩니다...\n\r", ch);
	GET_ALIGNMENT(ch) = -999;

	extract_obj(obj);
	send_to_char("OK.\n\r", ch);
	return TRUE;
}
