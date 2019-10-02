/* ************************************************************************
   *  file: spec_misc.c , Special module.                  Part of DIKUMUD *
   *  Usage: special procedures for miscellaneous zone/room/mobile   *
   *  Made by Choi Yeon Beom in KAIST                                        *
   ************************************************************************* */
#include <stdio.h>

#include "char.h"
#include "object.h"
#include "global.h"
#include "comm.h"
#include "play.h"
#include "actions.h"
#include "spells.h"
#include "etc.h"

extern struct char_data *choose_victim(struct char_data *mob, int fmode,
					int mode);

/* ************************************************************************
  *  file:  from fight.c: hit() and make_corpse()			  *
  ************************************************************************* */ 

/* NOTE: NEW! Calculate addintional damage dice by special equipment like
	'lss belt sword' or '여의봉'.		*/
/* NOTE: Called from hit() after damage add up and before AC(victim) subtract */
int special_damage(struct char_data *ch, struct char_data *victim,
			int dam)
{
    int dam_add = 0;
    /* 여의봉 */
    /* NOTE: You can wield or hold 여의봉	*/
    /* NOTE:  Sepcial damages of wielded 여의봉(bong) and held bong
                 are added independently.   */ 
    if ( (ch->equipment[WIELD]
	    && ch->equipment[WIELD]->item_number >= 0
	    && GET_OBJ_VIRTUAL(ch->equipment[WIELD]) == 11126))
	if (number(1, 10) >= 8)
	    dam_add += dice(ch->equipment[WIELD]->obj_flags.value[1],
			ch->equipment[WIELD]->obj_flags.value[2]);
     if ((ch->equipment[HOLD]
	    && ch->equipment[HOLD]->item_number >= 0
	    && GET_OBJ_VIRTUAL(ch->equipment[HOLD]) == 11126)) 
	if (number(1, 10) >= 5)
	    dam_add += dice(ch->equipment[HOLD]->obj_flags.value[1],
			ch->equipment[HOLD]->obj_flags.value[2]);

    /* lss belt sword */
    if (ch->equipment[WEAR_WAISTE]
	&& ch->equipment[WEAR_WAISTE]->item_number >= 0
	&& GET_OBJ_VIRTUAL(ch->equipment[WEAR_WAISTE]) == 9508)
	if (number(1, 10) > 6)
	    dam_add += dice(ch->equipment[WEAR_WAISTE]->obj_flags.value[1],
		    ch->equipment[WEAR_WAISTE]->obj_flags.value[2]);
    return(dam_add);
}

/* NOTE: NEW! Special death/corpse proceesing when mob died.	*/
/* NOTE: Called from make_corpse() after creating empty corpse 
	and before unequip char.  */ 
void special_death(struct char_data *ch, struct char_data *who,
	    struct obj_data *corpse)
{ 
    struct obj_data *otmp;

    if (!IS_NPC(ch)) /* NOTE: currently, no spcial for PC daeth */
	return;
    /* NOTE: When Mr. Son died. Remove GOLDEN_RIM from corpse */
    if (GET_MOB_VIRTUAL(ch) == SON_OGONG ) {
	if ( ch->equipment[WEAR_HEAD]) {
	    otmp = unequip_char(ch, WEAR_HEAD);
	    extract_obj(otmp);
	}
    }
    /* 삼장법사(11111) (put 금테(11127), 성수병(11134) Into corpse) */
    else if (GET_MOB_VIRTUAL(ch) == 11111) {
	otmp = read_object( GOLDEN_RIM, VIRTUAL);
	obj_to_obj(otmp, corpse);
	otmp = read_object(11134, VIRTUAL);
	obj_to_obj(otmp, corpse);
    }

    /* GoodBadIsland */
    /* NOTE: GBISLAND Renumbered */
    /* IRON GOLEM(12223) */
    else if (GET_MOB_VIRTUAL(ch) == GBISLAND_GOLEM ) {
	act("거대한 철문이 열리고, 위층으로 올라가는 계단이 보입니다.",
	    FALSE, ch, 0, 0, TO_ROOM);
	REMOVE_BIT(EXIT(ch, 4)->exit_info, EX_LOCKED);
	REMOVE_BIT(EXIT(ch, 4)->exit_info, EX_CLOSED);
    }

    /* KAALM(12201) */
    else if (GET_MOB_VIRTUAL(ch) == GBISLAND_KAALM) { 
	otmp = read_object( GBISLAND_SEED_EVIL_POWER , VIRTUAL);
	obj_to_obj(otmp, corpse);
    } 
}

int neverland(struct char_data *ch, int cmd, char *arg)
{
    int location, loc_nr;

    if (cmd != CMD_DOWN)  /* specific to Room 2707. cmd 6 is move down */
	return (FALSE);
    /* NOTE: to Room 2720-2724. (2724 is no exit room) */
    loc_nr = NEVERLAND_SENDBEACH + number(0, 4);

    location = real_room(loc_nr);
    act("$n씨가 지금 내려 갔나요 ??", FALSE, ch, 0, 0, TO_NOTVICT);
    send_to_char("악.\n\n악..\n\n 악...\n\n\n떨어지고 있습니다..\n\n\r", ch);
    send_to_char("여기가 어딜까 ??\n\r", ch);
    char_from_room(ch);
    char_to_room(ch, location);
    do_look(ch, "", 0);		/* NOTE: look around */
    return (TRUE);
}

int electric_shock(struct char_data *ch, int cmd, char *arg)
{
   /* extern struct weather_data weather_info; */
    int weather, shock;

    shock = 0;
    weather = weather_info.sky;
    /* NOTE: stronger (about 10 times) electric shock */
    /* OLD: if (cmd >= 1 && cmd <= 6 && !IS_NPC(ch)) { */
    if (!IS_NPC(ch) && number(0, 9) < 5 ) {
	switch (weather) {
	case SKY_RAINING:
	    send_to_char("ZZirrrr... 비가 오니까 찌릿 합니다.\n\r", ch);
	    shock = 50;
	    break;
	case SKY_LIGHTNING:
	    send_to_char("ZZirk ZZirk .. 이런 날씨에는 감전되어 죽겠습니다.\n\r", ch);
	    shock = number(200, 400); /* NOTE: was dice(5,7) */
	    break;
	case SKY_CLOUDLESS:
	case SKY_CLOUDY:
	default:
	    return FALSE;
	}
	if (GET_HIT(ch) < 100)
	    shock = 0;
	else if (GET_HIT(ch) < 500)
	    shock = shock / 4;
	else if (GET_HIT(ch) < 1000)
	    shock = shock / 2;
	GET_HIT(ch) -= shock;
    }
    return FALSE;
}

int great_mazinga(struct char_data *ch, int cmd, char *arg)
{
    int tmp;
    struct char_data *vict;
    void mob_punch_drop(struct char_data *mob, struct char_data *player);

    if (cmd)
	return FALSE;

    if (ch->specials.fighting && number(0, 5) < 4) {
	vict = choose_victim(ch, VIC_FIGHTING, MODE_RANDOM);
	if (vict) {
	    act("$n utters the words '로케트 주먹'.", 1, ch, 0, 0, TO_ROOM);
	    mob_punch_drop(ch, vict);
	    if ((tmp = GET_MOVE(vict)) > 0) {
		send_to_char("주먹에 맞아 체력이 떨어집니다.\n\r", vict);
		GET_MOVE(vict) = tmp * 7 / 10;
	    }
	}
    }

    if (number(0, 5) > 3) {
	if (number(0, 1) == 0) {
	    vict = choose_victim(ch, VIC_FIGHTING, MODE_MOVE_MAX);
	    if (vict) {
		act("$n utters the words 'ecuder evom'.", 1, ch, 0, 0, TO_ROOM);
		if ((tmp = GET_MOVE(vict)) > 0) {
		    send_to_char("갑자기 힘이 빠집니다.\n\r", vict);
		    GET_MOVE(vict) = tmp / 3 + dice(3, tmp / 10);
		}
	    }
	}
	else {
	    vict = choose_victim(ch, VIC_ALL, MODE_MANA_MAX);
	    if (vict) {
		act("$n utters the words 'ecuder anam'.", 1, ch, 0, 0, TO_ROOM);
		if ((tmp = GET_MANA(vict)) > 0) {
		    send_to_char("갑자기 힘이 빠집니다.\n\r", vict);
		    GET_MANA(vict) = tmp / 3 + dice(3, tmp / 10);
		}
	    }
	}
    }

    return TRUE;
}

/* ************************************************************************
  *  file: Special module for Daerimsa. 				  *
  ************************************************************************* */ 

#define MOB_STEP(ch)		((ch)->quest.solved)

/* NOTE: FIXED son_ogong quest. */ 
int son_ogong_func(struct char_data *ch, int cmd, char *arg)
{
    struct obj_data *i, *obj;
    struct char_data *victim, *mob;
    int done = 0;
    static int action = 0;
    extern void do_drop(struct char_data *ch, char *argument, int cmd);
    extern void do_wear(struct char_data *ch, char *argument, int cmd);

    /* char buf[256], hbuf[256]; */

    /* fighting */
    if ((victim = ch->specials.fighting)) {
	/* can't */
	/* cast = 84 (X) flee = 151 recite = 207 lightning move = 235 disarm
	   = 268 */
	if (cmd) {
	    if (cmd == CMD_RECITE || cmd == CMD_LIGHT || cmd == CMD_DISARM) {
		acthan( "Ogong makes a golden-barrier, it prevents your action.",
		    "오공이 금빛 막을 만들어서 당신의 움직임을 방해합니다.",
		      FALSE, ch, 0, 0, TO_ROOM);
		return TRUE;
	    }
	    return FALSE ;
	}

	if (MOB_STEP(ch) < 2) {
	    /* make mirror */
	    acthan( "Ogong pulls out a hair, and throws it.",
		      "오공이 머리털 하나를 뽑하서 던집니다.",
		      FALSE, ch, 0, 0, TO_ROOM);
	    mob = read_mobile(SON_OGONG_MIRROR, VIRTUAL);
	    char_to_room(mob, ch->in_room);
	    first_attack(mob, victim);
	    done++;
	}

	if (MOB_STEP(ch) == 2) {
	    /* 여의봉 */
	    acthan( "Ogong gets a bong in his ear.",
		      "오공이 귓속에서 여의봉을 꺼냅니다.",
		      FALSE, ch, 0, 0, TO_ROOM);
	    obj = read_object(SON_OGONG_BONG, VIRTUAL);
	    wear(ch, obj, 12);
	    MOB_STEP(ch) = 3;
	    done++;
	}
    }
    /* no fighting */
    else {
	if (cmd) {
	    action = cmd;	/* NOTE: remember PC action */
	    return FALSE;
	}
	    
	switch (MOB_STEP(ch)) {
	case 0:
	    if (action != CMD_WEAR) {
		for (i = ch->carrying; i; i = i->next_content)
		    if (GET_OBJ_VIRTUAL(i) == GOLDEN_RIM) {
			acthan( "Ogong asks \"What's this?\"",
				"\"이거 머하는 거지?\"라고 묻는다.",
				FALSE, ch, 0, 0, TO_ROOM);
			do_drop(ch, " rim", 60);
			done++;
		    }
	    }
	    else {
		/* wear */
		if (ch->equipment[WEAR_HEAD]
		    && ch->equipment[WEAR_HEAD]->item_number >= 0
		    && GET_OBJ_VIRTUAL(ch->equipment[WEAR_HEAD]) == GOLDEN_RIM){
		    acthan("$n looks at you.", "$n님이 당신을 바라봅니다.",
			   TRUE, ch, 0, ch, TO_VICT);
		    acthan("$n looks at $N.", "$n님이 $N님을 바라봅니다.",
			   TRUE, ch, 0, ch, TO_NOTVICT);
		    acthan( "Ogong says \"Wow~ Beautiful!!!\".",
			    "오공이 \"와~~ 멋있다!!!\" 라고 말한다.",
			    FALSE, ch, 0, 0, TO_ROOM);
		    MOB_STEP(ch) = 1;
		    done++;
		}
	    }
	    break;
	case 1:
	    if (action == CMD_GIVE) {
		/* GIVE */
		for (i = ch->carrying; i; i = i->next_content)
		    if (GET_OBJ_VIRTUAL(i) == GOLDEN_RIM)
			break;
		if (i) {
		    do_wear(ch, " rim", 13);
		    acthan( "Ogong says \"Yaho~ It's beautiful!!!\".",
			    "오공이 \"야호~ 멋있다!!!\"라고 외친다.",
			    FALSE, ch, 0, 0, TO_ROOM);
		    MOB_STEP(ch) = 2;
		    done++;
		}
	    }
	    break;
	}
    } 
/*
   sprintf(buf, "son_special : %s(%d)\n", GET_NAME(ch), cmd);
   log(buf);
 */
    if (done)
	action = 0;
    return done;
}

/* NOTE: FIXED fourth jangro quest. */ 
int fourth_jangro_func(struct char_data *ch, int cmd, char *arg)
{
    int done = 0;
    struct obj_data *i, *obj, *pen, *paper;
    static int action = 0;
    static long gold;

    if ( cmd ) { 
	action = cmd; 	/* NOTE: remember PC action */
	return FALSE;
    }

    switch(MOB_STEP(ch)) {
    case 0:
	 if ( number(1, 2) == 1) {
	/* 고민을 듣는다. */
	    acthan( "4th jangro says \"I need money... some money\".",
	       "4 장로가 \"돈이 필요해... 돈이...\" 라고 중얼거립니다.",
		  FALSE, ch, 0, 0, TO_ROOM);
	    gold = GET_GOLD(ch); 
	    done++ ;
	    MOB_STEP(ch) = 1;
	}
	break;

    case 1:
	if ( action == CMD_GIVE ) {
	    if ((GET_GOLD(ch) < 5000000) && (GET_GOLD(ch) < gold + 2000000)) {
		acthan( "fourth jangro says 'Not enough gold!!! Need more!!!'",
		     "4 장로가 '흠. 아직 부족해!!! 좀 더 많은 돈이 필요한데...'"
		     " 라고 말합니다.", FALSE, ch, 0, 0, TO_ROOM);
		gold = GET_GOLD(ch); 
		done++ ;
	    }
	    /* 돈을 받는다 */
	    else {
		acthan( "4th jangro says 'Ok!!! Enough gold!!! Thanks... "
		    "Bow, bow, bow!!!'",
		    "4 장로가 '충분해요... 충분해... 정말 고마워요. "
		    "감사~ 감사~~' 라고 말합니다.", FALSE, ch, 0, 0, TO_ROOM);
		MOB_STEP(ch) = 2;
		done++ ;
	    }
	}
	else if ( action )
	    MOB_STEP(ch) = 0;

	/*FALLTHRU*/ 
	/* NOTE: FALL THRU*/
    case 2:
	if ( action == CMD_ASK) {	/* ask */
	    acthan( "What? Ah~ You want to know how to enter the tower?",
		    "뭐라고요? 아~~ 대림사 탑에 들어가는 길을?",
		    FALSE, ch, 0, 0, TO_ROOM);
	    acthan( "Hmm. It's TOP Secret in Daerimsa. But...",
		    "음냐. 그거 대림사 최고 비밀인데.... 하지만...",
		    FALSE, ch, 0, 0, TO_ROOM);
	    acthan( "Give me a pen and a paper! I'll make it.",
		    "붓과 종이를 주세요... 할 수 없지.",
		    FALSE, ch, 0, 0, TO_ROOM); 
	    MOB_STEP(ch) = 3 ;
	    done++;
	}
	break;

    case 3:
	/* 붓과 종이 받기 */
	if ( action ) {
	    pen = paper = NULL;
	    for (i = ch->carrying; i; i = i->next_content) {
		if (GET_OBJ_VIRTUAL(i) == DAERIMSA_PEN)
		    pen = i;
		else if (GET_OBJ_VIRTUAL(i) == DAERIMSA_PAPER)
		    paper = i;
	    }

	    if (pen && paper) {
		extract_obj(pen);
		extract_obj(paper);

		MOB_STEP(ch) = 4;
	    }
	    else 
		acthan( "Give me a pen and a paper!", "붓과 종이를 주세요...",
			FALSE, ch, 0, 0, TO_ROOM);
	    done++;
	}
	break;

    case 4:
	switch (number(0, 4)) {
	case 1:
	case 2:
	    acthan( "4th jangro thinks!!!", "4 장로가 고민합니다...",
		      FALSE, ch, 0, 0, TO_ROOM);
	    break;
	case 3:
	case 4:
	    acthan( "4th jangro stares out!!!", "4 장로가 밖을 쳐다봅니다...",
		      FALSE, ch, 0, 0, TO_ROOM);
	    break;
	default:
	    acthan( "4th jangro drops a scroll.",
		    "4 장로가 두루마리를 내려 놓습니다.",
		      FALSE, ch, 0, 0, TO_ROOM);
	    acthan( "4th jangro says \"Recite this scroll in the dark room\".",
		   "\"어두운 방에서 이 두루마리를 읽으세요\"라고 말합니다.",
		    FALSE, ch, 0, 0, TO_ROOM);
	    obj = read_object(DAERIMSA_SCROLL, VIRTUAL);
	    obj_to_room(obj, ch->in_room);
	    MOB_STEP(ch) = 5;
	    break;
	}
	done++;
	break;

    case 5:

	if (number(1, 5) == 1) {
	    acthan( "4th jangro murmurs 'NamuAmitabul!!!'", 
		    "4 장로가 '나무아미타불!!!' 이라고 중얼거립니다.",
		      FALSE, ch, 0, 0, TO_ROOM);
	    done++;
	}
	break;
    }
    if(done) 
	action = 0;

    return done;
}

/* son_ogong mirror */
int son_ogong_mirror_func(struct char_data *ch, int cmd, char *arg)
{
    if( !cmd ) {
	(ch->quest.solved)++;
	if (ch->quest.solved > 50)
	    extract_char(ch);
    }
    return FALSE;
}

int teleport_daerimsa_tower(struct char_data *ch, int cmd, char *arg)
{
    struct follow_type *f;
    struct char_data *victim;
    int i, ch_in_room;
    struct obj_data *scroll;
    char buf[256];

    /* recite */
    if (cmd == CMD_RECITE) {
	arg = one_argument(arg, buf);
	if (!(scroll = get_obj_in_list_vis(ch, buf, ch->carrying))) {
	    act("You do not have that item.", FALSE, ch, 0, 0, TO_CHAR);
	    return 1;
	}

	if (GET_OBJ_VIRTUAL(scroll) != DAERIMSA_SCROLL)
	    return 0;

	if (world[ch->in_room].number == DARK_ROOM_IN_DAERIMSA) {
	    /* transport all memeber to TOWER */
	    for (i = 0; i <= top_of_world; i++)
		if (world[i].number == TOWER_IN_DAERIMSA)
		    break;
	    if (i == top_of_world) {
		log("NO-EXIST ROOM");
		return 0;
	    }

	    ch_in_room = ch->in_room;

	    victim = ch;
	    if (ch->master) {
		victim = ch->master;
	    }
	    f = victim->followers;

	    if (victim->in_room == ch_in_room) {
		act("$n disappears... suddenly", FALSE, victim, 0, 0, TO_ROOM);
		char_from_room(victim);
		char_to_room(victim, i);
		do_look(victim, "", 15);
	    }

	    while (f) {
		if (f->follower->in_room == ch_in_room) {
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

int saint_water(struct char_data *ch, int cmd, char *arg)
{
    char buf[100];
    struct obj_data *obj;

    if (cmd != CMD_QUAFF)
	return FALSE;

    /* quaff */
    one_argument(arg, buf);
    if (!(obj = get_obj_in_list_vis(ch, buf, ch->carrying)))
	return FALSE;

    if (GET_OBJ_VIRTUAL(obj) != SAINT_WATER)
	return FALSE;

    GET_ALIGNMENT(ch) = 999; 
    send_to_room("흠. 조금 찝찝한 맛이군...", ch->in_room);
    send_to_char("갑자기 성인이 된 듯한 기분이 듭니다.\n\r", ch);

    extract_obj(obj);

    return TRUE;
} 

/* ************************************************************************
  *  file: Special module for GoodBad Island. 				  *
  ************************************************************************* */ 

void gbisland_move_seashore(struct char_data *ch)
{
    int need_movement;

/*      int seashore, vnum_seashore; */
    int vnum_seashore;
    struct obj_data *obj, *next_obj;
    extern int movement_loss[];
    extern void save_char_nocon(struct char_data *ch, sh_int load_room);

    need_movement = movement_loss[world[ch->in_room].sector_type];

    if (GET_MOVE(ch) < need_movement && !IS_NPC(ch) && GET_LEVEL(ch) < IMO) {
	send_to_char("You are too exhausted.\n\r", ch);
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
    for (obj = ch->carrying; obj; obj = next_obj) {
	next_obj = obj->next_content;
	extract_obj(obj);
    }

    /* 체력이 떨어짐. */
    GET_HIT(ch) = number(1, GET_PLAYER_MAX_HIT(ch) / 5000 + 1);
    GET_MANA(ch) = number(1, GET_PLAYER_MAX_MANA(ch) / 5000 + 1);
    GET_MOVE(ch) = number(1, GET_PLAYER_MAX_MOVE(ch) / 5000 + 1);

    /* check - remortal */
    if ((ch->player.remortal < 15) && (GET_LEVEL(ch) < IMO)) {
	send_to_char("당신은 소용돌이에 휘말려 정신을 차리지 못합니다.\n\r", ch);
	send_to_char("당신은 죽습니다.\n\r", ch);

	/* NOTE: Use simpler die(), not raw_kill() */
#ifdef	NO_DEF
	wipe_stash(GET_NAME(ch));
	GET_GOLD(ch) = 0;
	save_char(ch);

	if (!IS_NPC(ch)) {
	    save_char_nocon(ch, world[ch->in_room].number);
	}

	raw_kill(ch, 44);
#endif  	/* NO_DEF */

	die(ch, 44, NULL);

	return;
    }

    if (!IS_AFFECTED(ch, AFF_SNEAK)) {
	act("$n 파도에 밀려 옵니다.", TRUE, ch, 0, 0, TO_ROOM);
    }
    do_look(ch, "\0", 15);
}

int gbisland_sea(struct char_data *ch, int cmd, char *arg)
{
    int was_in;
    struct follow_type *k;	/* , *next_dude; */

    /* cmd==2, "east" */
    /* cmd==151, "flee" */
    if (cmd != CMD_EAST && cmd != CMD_FLEE )
	return FALSE;

    was_in = ch->in_room;

    if (!IS_AFFECTED(ch, AFF_SNEAK)) {
	act("$n leaves east.", TRUE, ch, 0, 0, TO_ROOM);
    }

    if (ch->followers) {
	for (k = ch->followers; k; k = k->next) {
	    if ((was_in == k->follower->in_room) &&
		(GET_POS(k->follower) >= POS_STANDING)) {
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

    if (!IS_AFFECTED(ch, AFF_SNEAK)) {
	sprintf(tmp, "$n leaves %s.", dirs[dir - 1]);
	act(tmp, TRUE, ch, 0, 0, TO_ROOM);
    }

    if (ch->followers) {
	for (k = ch->followers; k; k = k->next) {
	    if ((was_in == k->follower->in_room) &&
		(GET_POS(k->follower) >= POS_STANDING)) {
		act("You follow $N.", FALSE, k->follower, 0, ch, TO_CHAR);

		if (!IS_AFFECTED(ch, AFF_SNEAK)) {
		    sprintf(tmp, "$n leaves %s.", dirs[dir - 1]);
		    act(tmp, TRUE, k->follower, 0, 0, TO_ROOM);
		}
	    }
	}
    }

    if (!IS_AFFECTED(ch, AFF_SNEAK)) {
	act("$n has arrived.", TRUE, ch, 0, 0, TO_ROOM);
    }
    if (ch->followers) {
	for (k = ch->followers; k; k = k->next) {
	    if ((was_in == k->follower->in_room) &&
		(GET_POS(k->follower) >= POS_STANDING)) {
		if (!IS_AFFECTED(ch, AFF_SNEAK)) {
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

    if (!IS_AFFECTED(ch, AFF_SNEAK)) {
	sprintf(tmp, "$n leaves %s.", dirs[dir - 1]);
	act(tmp, TRUE, ch, 0, 0, TO_ROOM);
    }

    char_from_room(ch);
    char_to_room(ch, real_room(GBISLAND_MAGIC_BARRIER_OUT));

    if (!IS_AFFECTED(ch, AFF_SNEAK)) {
	act("$n has arrived.", TRUE, ch, 0, 0, TO_ROOM);
    }

    if (ch->followers) {
	for (k = ch->followers; k; k = k->next) {
	    if ((was_in == k->follower->in_room) &&
		(GET_POS(k->follower) >= POS_STANDING)) {
		act("You follow $N.", FALSE, k->follower, 0, ch, TO_CHAR);
		send_to_char("\n\r", k->follower);

		if (!IS_AFFECTED(ch, AFF_SNEAK)) {
		    sprintf(tmp, "$n leaves %s.", dirs[dir - 1]);
		    act(tmp, TRUE, k->follower, 0, 0, TO_ROOM);
		}

		char_from_room(k->follower);
		char_to_room(k->follower, real_room(GBISLAND_MAGIC_BARRIER_OUT));

		if (!IS_AFFECTED(ch, AFF_SNEAK)) {
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

    if (!IS_AFFECTED(ch, AFF_SNEAK)) {
	sprintf(tmp, "$n leaves %s.", dirs[dir - 1]);
	act(tmp, TRUE, ch, 0, 0, TO_ROOM);
    }

    char_from_room(ch);
    char_to_room(ch, real_room(GBISLAND_MIRROR_SAINT));

    if (!IS_AFFECTED(ch, AFF_SNEAK)) {
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
    extern void do_open(struct char_data *ch, char *argument, int cmd);
    extern void do_move(struct char_data *ch, char *argument, int cmd);

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
    if (cmd != 0 && cmd != CMD_ASK && cmd != CMD_GIVE)
	return FALSE;

    if (cmd == 0) {
	if (give) {
	    for (obj = ch->carrying; obj; obj = next_obj) {
		next_obj = obj->next_content;
		if (GET_OBJ_VIRTUAL(obj) == GBISLAND_BOTTLE)
		    bottle = 6;
		extract_obj(obj);
	    }

	    if (bottle > 0) {
		do_say(ch, message[6 - bottle], 0);
		bottle--;
	    }
	    else {
		give = FALSE;
	    }
	}
	else if (ask) {
	    if (!move) {
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
	else if (move) {
	    dir = (char) (path[index + 1]) - '0' + 1;
	    switch (path[index]) {
	    case 'O':
		sprintf(tmp, "door %s", dirs[dir - 1]);
		do_open(ch, tmp, 0);
		index += 2;
		break;
	    case 'M':
		do_move(ch, "", dir);
		index += 2;
		break;
	    case 'R':
		if ((dir = number(1, 5)) < 5) {
		    /* false-move */
		    gbisland_false_move(ch, dir);
		}
		else {
		    /* go out */
		    gbisland_go_out_barrier(ch);
		    index += 2;
		}
		break;
	    case 'B':
		gbisland_go_back(ch);
		index = 0;
		move = FALSE;
		break;
	    }
	}
    }
    else if (cmd == CMD_ASK) {	/* ask */
	send_to_char("Ok.\n\r", ch);
	ask = TRUE;
    }
    else if (cmd == CMD_GIVE) {
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
    if (cmd != 0 && cmd != CMD_GIVE)
	return FALSE;

    if (cmd == 0) {
	if (give) {
	    paper1 = NULL;
	    paper2 = NULL;
	    for (obj = ch->carrying; obj; obj = next_obj) {
		next_obj = obj->next_content;
		if (GET_OBJ_VIRTUAL(obj) == GBISLAND_MAGIC_PAPER1)
		    paper1 = obj;
		else if (GET_OBJ_VIRTUAL(obj) == GBISLAND_MAGIC_PAPER2)
		    paper2 = obj;
		else
		    extract_obj(obj);
	    }

	    if (paper1 && paper2) {
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
	    else if (paper1 || paper2) {
		do_say(ch, "다른 부분도 마저 찾아 주세요...", 0);
	    }
	}
    }
    else if (cmd == CMD_GIVE) {
	do_give(ch, arg, cmd);
	give = TRUE;
    }

    return TRUE;
}

int gbisland_carpie(struct char_data *ch, int cmd, char *arg)
{
    int room;
    struct char_data *victim;
    /* struct affected_type *af, *af_next; */

    if (cmd)
	return FALSE;

    if (GET_POS(ch) == POS_SLEEPING)
	return FALSE;

    room = ch->in_room;
    for (victim = character_list; victim; victim = victim->next) {
	if ((victim->in_room == room) 
		&& GET_POS(victim) == POS_SLEEPING) {
	    /* NOTE: af->next is invalid. See affect_remove()*/
	    /*
	    for (af = victim->affected; af; af = af_next) {
		af_next = af->next;
		if (af->type == SPELL_SLEEP) 
		    affect_remove(victim, af); 
	    }
	    */
	    /* NOTE: Simpler code, Same effect.  */
	    affect_from_char(victim, SPELL_SLEEP);

	    send_to_char("누군가 당신의 허벅지를 꼬집습니다.\n\r", victim);
	    GET_POS(victim) = POS_STANDING;

	    if (IS_NPC(victim)) 
		SET_BIT(victim->specials.act, ACT_AGGRESSIVE);
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
    if (cmd != CMD_RECITE)
	return FALSE;

    one_argument(arg, name);
    obj = get_obj_in_list_vis(ch, name, ch->carrying);
    if (!obj)
	return FALSE;

    if (GET_OBJ_VIRTUAL(obj) != GBISLAND_MAGIC_PAPER)
	return FALSE;

    act("성스러운 기운이 방안에 가득히 넘쳐 흐릅니다.",
	FALSE, ch, 0, 0, TO_ROOM);
    send_to_char("성스러운 기운이 방안에 가득히 넘쳐 흐릅니다.\n\r", ch);

    room = ch->in_room;
    for (victim = character_list; victim; victim = victim->next) {
	if (victim->in_room == room) {
	    duration = GET_ALIGNMENT(victim) / 100;
	    duration = (duration > 0) ? duration : (-duration);
	    duration = (duration > 4) ? duration : 4;

	    /* DR */
	    af.type = SPELL_DAMAGE_UP;
	    af.duration = duration;
	    af.modifier = GET_ALIGNMENT(victim) / 3 + 5;
	    af.location = APPLY_DAMROLL;
	    af.bitvector = 0;
	    affect_to_char(victim, &af);

	    /* AC */
	    af.type = SPELL_ARMOR;
	    af.duration = duration;
	    af.modifier = -GET_ALIGNMENT(victim) / 3 - 5;
	    af.location = APPLY_AC;
	    af.bitvector = 0;
	    affect_to_char(victim, &af);

	    /* HR */
	    af.type = SPELL_BLESS;
	    af.duration = duration;
	    af.modifier = GET_ALIGNMENT(victim) / 4 + 5;
	    af.location = APPLY_HITROLL;
	    af.bitvector = 0;
	    affect_to_char(victim, &af);

	    if (GET_ALIGNMENT(victim) > 0) {
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
    if (cmd != CMD_EAT)
	return FALSE;

    one_argument(arg, name);
    obj = get_obj_in_list_vis(ch, name, ch->carrying);
    if (!obj)
	return FALSE;

    if (GET_OBJ_VIRTUAL(obj) != GBISLAND_SEED_EVIL_POWER)
	return FALSE;

    for (i = 0; i < 5; i++) {
	skno = number(0, MAX_SKILLS - 1);
	skilled = ch->skills[skno].skilled - GET_ALIGNMENT(ch) / 40;
	skilled = MAX(0, skilled);
	ch->skills[skno].skilled = MIN(skilled, 100);
    }

    send_to_char("당신은 갑자기 사악한 기분이 듭니다.\n\r", ch);
    send_to_char("당신은 악마가 됩니다...\n\r", ch);
    GET_ALIGNMENT(ch) = -999;

    extract_obj(obj);
    send_to_char("OK.\n\r", ch);
    return TRUE;
}
