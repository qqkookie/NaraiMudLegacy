/* ************************************************************************
   *  file: spec_mid.c , Special module.                   Part of DIKUMUD *
   *  Usage: special procedures for Midgaard object/room   		*
   ************************************************************************* */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <time.h>

#include "char.h"
#include "object.h"
#include "global.h"
#include "comm.h"
#include "play.h"
#include "actions.h"
#include "gamedb.h"
#include "etc.h"


#define FUDGE (100+dice(6,20))

int guild(struct char_data *ch, int cmd, char *arg)
{
    char buf[MAX_STRING_LENGTH], buf2[MAX_STRING_LENGTH];
    char buf3[MAX_STRING_LENGTH * MAX_SKILLS];
    char tmp[MAX_STRING_LENGTH];
    int number, i, percent, nlist;
    int lev, cla;
    extern int level_quest[];
    extern void advance_level(struct char_data *ch, int level_up);
    extern char *how_good(int p1, int p2);
    extern void set_title(struct char_data *ch);

    /* for(i=0;i<MAX_STRING_LENGTH*MAX_SKILLS;i++) buf3[i]=NULL; */
    strcpy(buf3, "");
    if ((cmd != CMD_ADVANCE) && (cmd != CMD_PRACTICE) && (cmd != 170))
	return (FALSE);

    if (cmd == CMD_ADVANCE) {	/* advance */
	if (!IS_NPC(ch)) {
	    for (i = 0; titles[GET_CLASS(ch) - 1][i].exp <= GET_EXP(ch); i++) {
		if (i > LEVEL_LIMIT) {
		    send_to_char_han("Immortality cannot be gained here.\r\n",
				   "여기서는 신이 될 수 없습니다.\r\n", ch);
		    return (TRUE);
		}
		if ((i > GET_LEVEL(ch)) &&
			(ch->quest.solved >= level_quest[GET_LEVEL(ch)])) {
		    send_to_char_han("You raise a level\r\n",
				     "레벨을 올렸습니다\r\n", ch);
		    GET_LEVEL(ch) = i;
		    advance_level(ch, 1);
		    set_title(ch);
		    return (TRUE);
		}
	    }
	    send_to_char_han("You need more experience.\r\n",
			     "경험이 더 필요합니다.\r\n", ch);
	    return (TRUE);
	}
    }
    lev = GET_LEVEL(ch);
    cla = GET_CLASS(ch) - 1;
    for (; *arg == ' '; arg++) ;
    if (!*arg) {
	send_to_char_han("You can practice any of these skills:\r\n",
		     "당신은 다음과 같은 기술을 익힐 수 있습니다:\r\n", ch);
	/* NOTE: spells index starts from 1, not zero.   */
	/* Perhaps modified  for(i=1; *spells[i] != '\n'; i++){ if(*spells[i]
	   && (spell_info[i+1].min_level[cla] <= lev)) {
	   send_to_char(spells[i], ch);
	   send_to_char(how_good(ch->skills[i+1].learned,
	   ch->skills[i+1].skilled), ch); send_to_char("\r\n", ch); } } */
	/* NOTE: Show practice number also with pager not to be scrolled out */
	sprintf(buf, "You have %d practices left.\r\n",
		ch->player.spells_to_learn);
	sprintf(buf2, "지금 %d 번 기술을 연마(practice)할 수 있습니다. \r\n",
		ch->player.spells_to_learn);
	/* send_to_char_han(buf,buf2,ch); */
	strcpy(buf3, STRHAN(buf, buf2, ch));

	nlist = 0;
	/* NOTE: spell index starts from 1, not zero */
	for (i = 1; *spells[i] != '\n'; i++) {
	    if (*spells[i] && (spell_info[i].min_level[cla] <= lev)) {
		/* NOTE: Added spell number for cross ref. */
		/* sprintf(tmp,"%-20s %-4s ", */
		sprintf(tmp, "%3d %-20s %-9s    ", i, spells[i],
			how_good(ch->skills[i].learned, ch->skills[i].skilled));
		strcat(buf3, tmp);
		/* NOTE: show two skill/spells per line */
		if (!(++nlist % 2))
		    strcat(buf3, "\r\n");
	    }
	}
	if (!(++nlist % 2))
	    strcat(buf3, "\r\n");
/* NOTE: Keep internal copy. Otherwise, two players in practice room mixed up */
	page_string(ch->desc, buf3, 1);
	return (TRUE);
    }
    /* NOTE: use serch_block() instead of old_search_block()    */
    number = search_block(arg, spells, FALSE);
    if (number == -1) {
	send_to_char_han("You do not know of this spell...\r\n",
			 "그런 기술은 모르는데요 ...\r\n", ch);
	return (TRUE);
    }
    if (lev < spell_info[number].min_level[cla]) {
	send_to_char_han("Your level is too low.\r\n",
			 "아직은 레벨이 낮아 안됩니다...\r\n", ch);
	return (TRUE);
    }
    if (ch->player.spells_to_learn <= 0) {
	send_to_char_han("You do not seem to be able to practice now.\r\n",
			 "지금은 더이상 배울 수 없습니다.\r\n", ch);
	return (TRUE);
    }
    if (ch->skills[number].learned >= spell_info[number].max_skill[cla]) {
	send_to_char_han("You know this area as well as possible.\r\n",
			 "그 분야는 배울 수 있는만큼 배웠습니다.\r\n", ch);
	return (TRUE);
    }
    send_to_char_han("You Practice for a while...\r\n",
		     "기술이 늘고 있습니다...\r\n", ch);
    ch->player.spells_to_learn--;
    percent = ch->skills[number].learned + 1 +
	((int) int_app[GET_INT(ch)].learn
	 * (int) spell_info[number].max_skill[cla]) / FUDGE;
    ch->skills[number].learned =
	MIN(spell_info[number].max_skill[cla], percent);
    if (ch->skills[number].learned >= spell_info[number].max_skill[cla]) {
	send_to_char_han("You're now as proficient as possible.\r\n",
			 "이 분야에는 이미 배울만큼 다 배웠습니다.\r\n", ch);
	return (TRUE);
    }
    return TRUE;
}

int remortal(struct char_data *ch, int cmd, char *arg)
{
    char buf1[100], buf2[100];
    char class;
    char buf[255];
    char classes[4][20] =
    {
	"지존 마법사",
	"지존 힐러",
	"지존 도둑으",
	"지존 워리어"
    };
    int all_done;
    extern void do_start(struct char_data *ch);

    /* not say */
    if (cmd != CMD_SAY)
	return 0;

/* furfuri modify ... under if .. && >>> || */
/*
   if(ch->points.exp<500000000 || ch->player.level!=40)
 */
    if (GET_LEVEL(ch) != 40) {
	send_to_char("You are not enough to remortal.!!!\r\n", ch);
	return 0;
    }

/*
   defined in structs.h
   #define REMORTAL_MAGIC_USER     1
   #define REMORTAL_CLERIC         2
   #define REMORTAL_THIEF          4
   #define REMORTAL_WARRIOR        8
 */
    all_done = (ch->player.remortal == (1 + 2 + 4 + 8));

    if (all_done) {
	if (ch->points.exp < 500000000) {
	    send_to_char("You are not enough to remortal.!!!\r\n", ch);
	    return 0;
	}
    }

    half_chop(arg, buf1, buf2);
    class = buf1[0];
    switch (class) {
    case '?':
	/* show remortal */
	if (ch->player.remortal & REMORTAL_MAGIC_USER)
	    send_to_char("You did a magic user.!!!\r\n", ch);
	if (ch->player.remortal & REMORTAL_CLERIC)
	    send_to_char("You did a cleric.!!!\r\n", ch);
	if (ch->player.remortal & REMORTAL_THIEF)
	    send_to_char("You did a thief.!!!\r\n", ch);
	if (ch->player.remortal & REMORTAL_WARRIOR)
	    send_to_char("You did a warrior.!!!\r\n", ch);
	return TRUE;
    case 'W':
    case 'w':
	class = 4;
	if (ch->abilities.str == 18) {
	    if (ch->abilities.str_add < 100) {
		ch->abilities.str_add += 10;
		ch->abilities.str_add = MIN(ch->abilities.str_add, 100);
	    }
	    else {
		break;
	    }
	}
	else {
	    ch->abilities.str++;
	}
	ch->player.remortal |= REMORTAL_WARRIOR;
	break;
    case 'C':
    case 'c':
	class = 2;
	if (ch->abilities.wis < 18)
	    ch->abilities.wis++;
	ch->player.remortal |= REMORTAL_CLERIC;
	break;
    case 'M':
    case 'm':
	class = 1;
	if (ch->abilities.intel < 18)
	    ch->abilities.intel++;
	ch->player.remortal |= REMORTAL_MAGIC_USER;
	break;
    case 'T':
    case 't':
	class = 3;
	if (ch->abilities.dex < 18)
	    ch->abilities.dex++;
	ch->player.remortal |= REMORTAL_THIEF;
	break;
    default:
	return TRUE;
    }
    /* NOTE: Add CR+LF */
    sprintf(buf, "%s님이 %s로 직업을 바꾸셨습니다.\r\n", GET_NAME(ch),
	    classes[class - 1]);
    send_to_all(buf);

    GET_CLASS(ch) = class;

    /* initialize other data */
    if (!all_done) {
	GET_EXP(ch) = 0;
	GET_LEVEL(ch) = 0;

	GET_HIT(ch) = GET_PLAYER_MAX_HIT(ch) = 0;
	GET_MANA(ch) = GET_PLAYER_MAX_MANA(ch) = 0;
	GET_MOVE(ch) = GET_PLAYER_MAX_MOVE(ch) = 0;

	GET_AC(ch) += 40;
	GET_HITROLL(ch) -= 40;
	GET_DAMROLL(ch) -= 40;

	/* init-quest */
	ch->quest.type = 0;
	ch->quest.data = 0;
	ch->quest.solved = 0;

	do_start(ch);
    }
    else {
	GET_EXP(ch) -= 500000000;
    }

    return TRUE;
}


int bank(struct char_data *ch, int cmd, char *arg)
{
    char buf[MAX_STRING_LENGTH];
    LONGLONG amt;

    if (IS_NPC(ch))
	return (FALSE);
    if (cmd == CMD_LIST) {	/* List */
	send_to_char("At the bank you may:\r\n\r\n", ch);
	send_to_char("list - get this list\r\n", ch);
	send_to_char("balance - check your balance\r\n", ch);
	send_to_char("deposit <amount>, or\r\n", ch);
	send_to_char("withdraw <amount>\r\n\r\n", ch);
	return (TRUE);
    }
    else if (cmd == CMD_BALANCE) {	/* Balance */
	sprintf(buf, "You have %s coins in the bank.\r\n", monetary(ch->bank));
	send_to_char(buf, ch);
	return (TRUE);
    }
    else if ((cmd == CMD_DEPOSIT) || (cmd == CMD_WITHDRAW)) {	/* deposit or withdraw */
	if (!*arg) {
	    send_to_char("The banker says 'Specify an amount.'\r\n", ch);
	    return (TRUE);
	}
	arg = one_argument(arg, buf);
	amt = atoll(buf);
	if (amt <= 0) {
	    send_to_char("The banker says 'Amount must be positive.'\r\n", ch);
	    return (TRUE);
	}
	if (cmd == CMD_DEPOSIT) {
	    if (amt > GET_GOLD(ch)) {
		send_to_char("The banker says 'You don't have that much.'\r\n", ch);
		return (TRUE);
	    }
	    if (ch->bank + amt < ch->bank) {
		send_to_char("Too much money! I can't afford so much money.\r\n", ch);
		return (TRUE);
	    }
	    GET_GOLD(ch) -= amt;
	    ch->bank += amt;
	}
	else {
	    if (amt > ch->bank) {
		send_to_char("The banker says 'We don't make loans.'\r\n", ch);
		return (TRUE);
	    }
	    /* NOTE: prevent purse (coin you carry) overflow */
	    if (GET_GOLD(ch) + amt < GET_GOLD(ch)) {
		send_to_char("The banker says 'Your wallet will burst.'\r\n", ch);
		return (TRUE);
	    }
	    GET_GOLD(ch) += amt;
	    ch->bank -= amt;
	}
	send_to_char("The banker says 'Have a nice day.'\r\n", ch);
	return (TRUE);
    }
    return (FALSE);
}

int dump(struct char_data *ch, int cmd, char *arg)
{
    struct obj_data *k;
    char buf[100];
    struct char_data *tmp_char;
    int value = 0;

    void do_drop(struct char_data *ch, char *argument, int cmd);

    for (k = world[ch->in_room].contents; k; k = world[ch->in_room].contents) {
	sprintf(buf, "The %s vanish in a puff of smoke.\r\n", fname(k->name));
	for (tmp_char = world[ch->in_room].people; tmp_char;
	     tmp_char = tmp_char->next_in_room)
	    if (CAN_SEE_OBJ(tmp_char, k))
		send_to_char(buf, tmp_char);
	extract_obj(k);
    }
    if (cmd != CMD_DROP)
	return (FALSE);
    do_drop(ch, arg, cmd);
    value = 0;
    for (k = world[ch->in_room].contents; k; k = world[ch->in_room].contents) {
	sprintf(buf, "The %s vanish in a puff of smoke.\r\n", fname(k->name));
	for (tmp_char = world[ch->in_room].people; tmp_char;
	     tmp_char = tmp_char->next_in_room) {
	    if (CAN_SEE_OBJ(tmp_char, k))
		send_to_char(buf, tmp_char);
	}

	/* NOTE: Hard currency is more valuable than item  */
	if( k->obj_flags.type_flag == ITEM_MONEY )
	    value += k->obj_flags.cost*2 ;
	else
	    value += k->obj_flags.cost;
	extract_obj(k);
    }
    if (value > 0) {
	acthan("You are awarded for outstanding performance.",
	       "당신은 시장으로부터 선행상을 받았습니다.", FALSE, ch, 0, 0, TO_CHAR);
	acthan("$n has been awarded for being a good citizen.",
	       "$n님이 착한 시민상을 받았습니다", TRUE, ch, 0, 0, TO_ROOM);

	ch->points.exp += value/3;
    }
    return TRUE;
}

int safe_house(struct char_data *ch, int cmd, char *arg)
{
    if (GET_LEVEL(ch) >= LEV_DEMI)
	return FALSE;
    switch (cmd) {
    case CMD_KILL:		/* kill */
    case CMD_HIT:		/* hit */
    case CMD_CAST:		/* cast Perhaps modified */
    case CMD_ORDER:		/* order */
    case CMD_BACKSTAB:		/* backstab */
    case CMD_STEAL:		/* steal */
    case CMD_BASH:		/* bash */
    case CMD_KICK:		/* kick */
    case CMD_USE:		/* use */
    case CMD_SHOOT:		/* shoot */
    case CMD_RECITE:		/* recite */
    case CMD_TORNADO:		/* tornado */
    case CMD_FLASH:		/* flash */
    case CMD_DISARM:		/* disarm by chase */
    case CMD_SHOURYUKEN:	/* shouryuken by chase */
    case CMD_THROW:		/* throw object */
    case CMD_PUNCH:		/* punch */
    case CMD_ASSAULT:		/* assault by process */
    case CMD_BROADCAST:
    case CMD_SIMULTANEOUS:
    case CMD_ARREST:
    case CMD_ANGRY_YELL:
    case CMD_SOLACE:
    case CMD_TEMPTATION:
    case CMD_SHADOW_FIGURE:
    case CMD_SMOKE:
    case CMD_INJECT:
    case CMD_POWER_BASH:
    case CMD_EVIL_STRIKE:
    case CMD_SPIN_BIRD_KICK:		/* spin bird kick */
	send_to_char("You cannot do that here.\r\n", ch);
	/*
	sprintf(buf, "%s attempts to misbehave here.\r\n", ch->player.name);
	send_to_room_except(buf, ch->in_room, ch);
	*/
	/* NOTE: Hide misbehavior to whom cannot see it. */
	act("$n attempts to misbehave here.", TRUE, ch, 0, 0, TO_ROOM);
	return TRUE;

    default:
	return FALSE;
    }
}

int jail_room(struct char_data *ch, int cmd, char *arg)
{
    static int dismiss = 0;

    if (IS_WIZARD(ch))
	return FALSE;

    if (IS_SET(ch->specials.act, PLR_BANISHED)
	&& (time(0) > ch->player.jail_time)) {
	char buf[256];

	sprintf(buf, "A pierce-looking prison warden open small window"
		" and yells:\r\nPrisoner number 6999. %s!!! Listen!\r\n",
		GET_NAME(ch));
	send_to_room(buf, ch->in_room);
	send_to_char("\r\nYour jail term expired... Now, You are free.\r\n"
		 "Please, be a nice boy. 똑바루 살어! Leave here.\r\n", ch);
	REMOVE_BIT(ch->specials.act, PLR_BANISHED);
	dismiss++;
	return (TRUE);
    }
    /* NOTE: Don't add code for freeing non-banished player */
    /*	Non-banished player can fall here by (mis-)teleporting. He deseves it.
	Secret: Such player can escape jail by quaffing teleport potion */

    /* if(IS_AFFECTED(ch, AFF_ARREST)) { ; } */
    switch (cmd) {
    case CMD_LOOK:		/* look */
    case CMD_SCORE:		/* score */
    case CMD_SAY:		/* say */    /* case 18 : *//* shout */
    case CMD_STAND:		/* stand */
    case CMD_SIT:		/* sit */
    case CMD_REST:		/* rest */
    case CMD_SLEEP:		/* sleep */
    case CMD_WAKE:		/* wake */
	return FALSE;

    case CMD_SHOUT:		/* shout  */
    case CMD_CHAT:		/* chat  */
	send_to_char("Shut up!!! You are a PRISONER.\r\n", ch);
	return (TRUE);
    case CMD_NORTH:
    case CMD_EAST:
    case CMD_WEST:
    case CMD_SOUTH:
	/* NOTE: Freed player can simply walk away from jail */
	/* simple movement (north, south, ...)  */
	if (dismiss && !IS_SET(ch->specials.act, PLR_BANISHED)) {
	    act("The heavy door opened slowly and $n walks way.. smiling..",
		0, ch, 0, 0, TO_ROOM);
	    char_from_room(ch);
	    char_to_room(ch, real_room(MID_PORTAL));	/* MID */
	    act("$n appears with ear-splitting bang.", 0, ch, 0, 0, TO_ROOM);
	    do_look(ch, "", 15);
	    dismiss = 0;
	    return (TRUE);
	}
	else
	    send_to_char("Freeeeeze!!! You are under ARREST.\r\n", ch);
	return (TRUE);
    case CMD_QUAFF:		/* quaff  */
	/* NOTE: Prohibit quaffing murky-grey vail (teleport potion)
	    for banished prosoner. But not for ordinary player.  */
	if(IS_SET(ch->specials.act, PLR_BANISHED)) {
	    send_to_char("He he he. Drug is illegal substances here..\r\n", ch);
	    return(TRUE);
	}
	break;
    default:
	if (dismiss) {
	    send_to_char("Get way!! I don't want see your ugly face"
			 " here, AGAIN!!\r\n", ch);
	    return (TRUE);
	}
    }
    return (safe_house(ch, cmd, arg));
}

/* NOTE: Separated code from hospital() */
int change_player_name(struct char_data *ch, char *arg)
{
/* by ares */
    int i;
    char *newname;
    extern int find_name(char *name);

    /* skip white space */
    newname = skip_spaces(arg);
    /* check illegal name */
    for (i = 0; newname[i]; i++) {
	if ((newname[i] < 0) || !isalpha(newname[i]) || i > 15) {
	    send_to_char("That's Illegal name!\r\n", ch);
	    return FALSE;
	}
    }
    if (i < 2) {
	send_to_char("That's too short name!\r\n", ch);
	return FALSE;
    }

#ifdef  UNUSED_CODE
    /* NOTE: find_name(newname) will check all player name, playing or not. */

    /* check if already playing */
    /* stolen from interpreter.c */
    for (k = descriptor_list; k; k = k->next) {
	if ((k->character != ch) && k->character) {
	    if (k->original) {
		if (GET_NAME(k->original) &&
		    (str_cmp(GET_NAME(k->original), newname) == 0)) {
		    send_to_char("That name already exist!\r\n", ch);
		    return FALSE;	/* jhpark 설명 필요없죠? */
		}
	    }
	    else {	/* No switch has been made */
		if (GET_NAME(k->character) &&
		    (str_cmp(GET_NAME(k->character), newname) == 0)) {
		    send_to_char("That name already exist!\r\n", ch);
		    return FALSE;	/* jhpark */
		}
	    }
	}
    }
#endif 		/* UNUSED_CODE */

    /* check player index table. You don't need to check player file */
    /* for detail, see load_char() and find_name() */
    if ((i = find_name(newname)) != -1) {
	send_to_char("That name already used by another player!\r\n", ch);
	return FALSE;
    }
    i = find_name(ch->player.name);
    if (i == -1) {
	log("이럴 수가!!!");
	send_to_char("ING? Then, how can i be here?\r\n", ch);
	return FALSE;
    }
    /* NOTE: Remove stash instead of rename it */
    wipe_stash(GET_NAME(ch));

    /* now, name can be changed! */
    /* but u must update not only ch's name but also player_table */
    /* and rename item file, and force save */
    CAP(newname);
    free(player_table[i].name);
    player_table[i].name = strdup(newname);

    free(ch->player.name);
    ch->player.name  = strdup(newname);

    save_char(ch);
    /* now save STASH file in new name */
    /* NOTE: BEWARE! This can be source of stash copy BUG!! Reported by Yun */
    stash_char(ch);

    send_to_char("\r\n", ch);
    send_to_char("이름을 바꾸기 위해 당신은 ARES님에게 갑니다.\r\n", ch);
    send_to_char("흠...그래, 이름을 바꾸시겠다고?\r\n", ch);
    send_to_char("잘 생각했네! 자네가 지금 쓰는 이름은 딱 ", ch);
    send_to_char("비명횡사하기 좋지...\r\n", ch);
    send_to_char("자, 그럼 뭘로 할텐가?\r\n\r\n", ch);
    do_say(ch, newname, 0);
    send_to_char("\r\n", ch);
    send_to_char(newname, ch);
    send_to_char("? 그것도 제 명에 살기는 틀린 이름인데?\r\n", ch);
    send_to_char("허허..거참..허나 자네가 좋다니 별수 없지 뭐...\r\n", ch);
    send_to_char("당신이 가고 난 후 ARES님이 이렇게 중얼거립니다.\r\n", ch);
    send_to_char("하! 이름 바꾼다고 잘 살수 있을거 같지?\r\n", ch);
    send_to_char("어림 반 푼어치도 없다!!!!!!!!!!!!!!\r\n", ch);

    return TRUE;
}

int hospital(struct char_data *ch, int cmd, char *arg)
{
    char buf[MAX_STRING_LENGTH];
    /* NOTE: BUG FIX cost[6] is too short. */
    int opt, cost[8];

    /* NOTE: Hospital fee lowered for level < 20, raised for level > 20 */
    /* NOTE : OLD cost[0] = 60 + level */
    cost[0] = 60 + (GET_LEVEL(ch)*GET_LEVEL(ch))/20  ;
    cost[1] = cost[0] * (GET_MAX_HIT(ch) - GET_HIT(ch));
    cost[2] = cost[0] * (GET_MAX_MANA(ch) - GET_MANA(ch));
    cost[3] = cost[0] * (GET_MAX_MOVE(ch) - GET_MOVE(ch));
    cost[4] = 50000 + cost[0] * 20;
    /* NOTE: Trans-sexual operation fee should be at least that much. */
    cost[5] = 40000000 + GET_SEX(ch) * 10000000;
    cost[6] = 500000000;

    if (cmd == CMD_LIST) {	/* List */
	sprintf(buf, "1 - Hit points restoration (%d coins)\r\n", cost[1]);
	send_to_char(buf, ch);
	sprintf(buf, "2 - Mana restoration (%d coins)\r\n", cost[2]);
	send_to_char(buf, ch);
	sprintf(buf, "3 - Move restoration (%d coins)\r\n", cost[3]);
	send_to_char(buf, ch);
	sprintf(buf, "4 - Poison cured (%d coins)\r\n", cost[4]);
	send_to_char(buf, ch);
	sprintf(buf, "5 - Change Your Sex(toggle) (%d coins)\r\n", cost[5]);
	send_to_char(buf, ch);
	sprintf(buf, "6 - Change Your name (%d coins)\r\n", cost[6]);
	send_to_char(buf, ch);
	sprintf(buf, "To change name, TYPE \"buy 5 <NAME>\"\r\n");
	send_to_char(buf, ch);
	return (TRUE);
    }
    else if (cmd != CMD_BUY) 	/* Buy */
	return (FALSE);

    if (!arg || !*arg) {
	send_to_char("Buy what?\r\n", ch);
	return (TRUE);
    }
    arg = one_argument(arg, buf);
    if ((opt = atoi(buf)) > 6) {
	send_to_char("Huh?\r\n", ch);
	return (TRUE);
    }
    /* opt의 value가 1보다 작을수도 있으므로 아래의 routine을 넣었습니다. */
    if (opt < 1) {
	send_to_char("Huh?\r\n", ch);
	return (TRUE);
    }
    if (cost[opt] > GET_GOLD(ch)) {
	if (IS_WIZARD(ch)) {
	    send_to_char("쩝...당신한테만 외상으로 해드리는 거에요...\r\n", ch);
	    cost[opt] = 0;
	}
	else {
	    send_to_char("외상은 안됩니다.\r\n", ch);
	    return (TRUE);
	}
    }
    switch (opt) {
    case 1:
	GET_HIT(ch) = GET_MAX_HIT(ch);
	GET_GOLD(ch) -= cost[1];
	send_to_char("You feel magnificent!\r\n", ch);
	break;
    case 2:
	GET_MANA(ch) = GET_MAX_MANA(ch);
	GET_GOLD(ch) -= cost[2];
	send_to_char("You feel marvelous!\r\n", ch);
	break;
    case 3:
	GET_MOVE(ch) = GET_MAX_MOVE(ch);
	GET_GOLD(ch) -= cost[3];
	send_to_char("You feel The GOD!\r\n", ch);
	break;
    case 4:
	if (affected_by_spell(ch, SPELL_POISON)) {
	    affect_from_char(ch, SPELL_POISON);
	    GET_GOLD(ch) -= cost[4];
	    send_to_char("You feel stupendous!\r\n", ch);
	}
	else
	    send_to_char("Nothing wrong with you.\r\n", ch);
	break;

    case 5:
	send_to_char("프로세스 라는 의사가 당신에게 물어봅니다.\r\n", ch);
	send_to_char("\"왜 성전환을 하려고 합니까?\"\r\n", ch);
	switch (GET_SEX(ch)) {
	case SEX_NEUTRAL:
	    do_say(ch, "그냥요,왜 떫으요?\r\n", 0);
	    break;
	case SEX_MALE:
	    do_say(ch, "전요, 여자랑 그짓만 할 생각하는 남자가 싫어요!!!\r\n", 0);
	    send_to_char("프로세스는 멍텅해서 거시기를 잘라버린다!\r\n", ch);
	    GET_SEX(ch) = SEX_FEMALE;
	    break;
	case SEX_FEMALE:
	    do_say(ch, "전 당신같은 멋진 남자가 되고 싶었어요....\r\n", 0);
	    send_to_char("프로세스는 그녀를 보고 안타까워 하면서 오이를 심어줍니다.\r\n", ch);
	    GET_SEX(ch) = SEX_MALE;
	    break;
	}
	GET_GOLD(ch) -= cost[5];
	break;

    case 6:
	if(change_player_name(ch, arg))
	    GET_GOLD(ch) -= cost[6];
	break;

    default:
	send_to_char("뭐요?\r\n", ch);
	return TRUE;
    }
    save_char(ch);
    return TRUE;
}

/* modified by atre */
/*
   remove
   5. exp --> gold
   6. freedom of hunger, thirst
   add
   use ticket
 */
/* modified by epochal
   use ticket is changed. */

int metahospital(struct char_data *ch, int cmd, char *arg)
{
    char buf[MAX_STRING_LENGTH];
    char buf2[MAX_STRING_LENGTH];
    int k, opt;
    // int mult;
    long int cost = 0;
    struct obj_data *tmp_obj;

    if (IS_NPC(ch))
	return (FALSE);

    if (cmd == CMD_LIST) {	/* List */
	send_to_char("1 - Hit points inflator(NO LIMIT)\r\n", ch);
	send_to_char("2 - Mana increase(NO LIMIT)\r\n", ch);
	send_to_char("3 - Movement increase(NO LIMIT)\r\n", ch);
	send_to_char("4 - Increase your practice number (+2)(small exp depend on level)\r\n", ch);
/*
   send_to_char("5 - Change exp to gold(3:2)\r\n",ch);
   send_to_char("6 - Freedom from hunger and thirsty(100M exp)\r\n",ch);
 */
	send_to_char("1,2,3,4 of these for some experience points\r\n", ch);
	send_to_char("use tickets for meta\r\n", ch);
	return (TRUE);
    }
    else if (cmd == CMD_BUY) {	/* Buy */
	half_chop(arg, buf, buf2);
	opt = atoi(buf);
	// mult = 1;

	/*
	   if (1 <= opt && opt <= 6) { */
	/* remove 5, 6 */
	if (1 <= opt && opt <= 4) {
	    switch (opt) {
	    case 1:
		k = GET_PLAYER_MAX_HIT(ch);
		cost = k * 200;
		cost = number(cost, cost << 1);
		cost = MIN(50000000, cost);
		if (k >= 15000 && 40 > GET_LEVEL(ch)) {
		    send_to_char("Come back when you attain to Dangun(40 lvl).\r\n", ch);
		    return (TRUE);
		}
		if (cost <= 0 || cost > GET_EXP(ch)) {
		    send_to_char("Come back when you are ", ch);
		    send_to_char("more experienced.\r\n", ch);
		    return (TRUE);
		}
		if (k > 3000000) {
		    send_to_char("you are full hit point!!!", ch);
		    cost = 0;
		    break;
		}
		else {
		    ch->points.max_hit += dice(3, 5) + ch->abilities.con / 6;
		    send_to_char("@>->->--\r\n", ch);
		    break;
		}
	    case 2:
		k = ch->points.max_mana;
		cost = k * 200;
		cost = number(cost, cost << 1);
		cost = MIN(50000000, cost);
		if (k >= 15000 && 40 > GET_LEVEL(ch)) {
		    send_to_char("Come back when you attain to Dangun(40 lvl).\r\n", ch);
		    return (TRUE);
		}
		if (cost <= 0 || cost > GET_EXP(ch)) {
		    send_to_char("Come back when you are ", ch);
		    send_to_char("more experienced.\r\n", ch);
		    return (TRUE);
		}
		if (k > 3000000) {
		    send_to_char("you are full mana point!!!", ch);
		    cost = 0;
		    break;
		}
		else {
		    ch->points.max_mana += dice(2, 4)
			+ (GET_INT(ch) + GET_WIS(ch)) / 6;
		    send_to_char("<,,,,,,,>\r\n", ch);
		    break;
		}
	    case 3:
		k = ch->points.max_move;
		cost = k * 200;
		cost = number(cost, cost << 1);
		cost = MIN(50000000, cost);
		if (k >= 15000 && 40 > GET_LEVEL(ch)) {
		    send_to_char("Come back when you attain to Dangun(40 lvl).\r\n", ch);
		    return (TRUE);
		}
		if (cost <= 0 || cost > GET_EXP(ch)) {
		    send_to_char("Come back when you are ", ch);
		    send_to_char("more experienced.\r\n", ch);
		    return (TRUE);
		}
		if (k > 3000000) {
		    send_to_char("you are full move point!!!", ch);
		    cost = 0;
		    break;
		}
		else {
		    ch->points.max_move += dice(3, 5) + ch->abilities.dex / 6;
		    send_to_char("<xxxxx>\r\n", ch);
		    break;
		}
	    case 4:
		cost = 3000;
		cost *= GET_LEVEL(ch);
		cost *= GET_LEVEL(ch);
		if (cost > GET_EXP(ch)) {
		    send_to_char("Come back when you are more experienced.\r\n", ch);
		    return (TRUE);
		}
		ch->player.spells_to_learn += (int) (ch->abilities.wis / 3);
		send_to_char("Aaaaaaarrrrrrrrggggg\r\n", ch);
		break;
	    case 5:
		cost = 30000;
		cost *= GET_LEVEL(ch);
		cost = number(cost, cost << 1);
		if (cost > GET_EXP(ch)) {
		    send_to_char("Come back when you are more experienced.\r\n", ch);
		    return (TRUE);
		}
		ch->points.gold += cost * 2 / 3;
		break;
	    case 6:
		if (GET_EXP(ch) < 100000000) {
		    send_to_char("Come back when you are more experienced.\r\n", ch);
		    return (TRUE);
		}
		GET_COND(ch, DRUNK) = -1;
		GET_COND(ch, FULL) = -1;
		GET_COND(ch, THIRST) = -1;
		cost = 100000000;
		send_to_char_han( "You are free from hunger and thirsty "
			"from now on!!!\r\n     Worship the God!\r\n",
			"당신은 이제 배고픔과 목마름으로부터 해방입니다.\r\n"
			"신께 경배드리십시요.\r\n", ch);
		break;
	    default:
		break;
	    }
	    ch->points.exp -= cost;
	}
	else {
	    send_to_char("예?\r\n", ch);
	}
	return (TRUE);
    }

    else if (cmd == CMD_USE) {	/* use */
	half_chop(arg, buf, buf2);
	/* buf == tickets */
	if (*buf) {
	    if (strncmp("tickets", buf, strlen(buf))) {
		/* not ticket */
		return FALSE;
	    }
	    tmp_obj = get_obj_in_list_vis(ch, buf, ch->carrying);
	    if (tmp_obj) {
		if (GET_EXP(ch) < 250000000) {
		    send_to_char("Come back when you are more experienced.", ch);
		    return TRUE;
		}
		if ( ch->quest.solved < (GET_LEVEL(ch) < 40 ? 70 : 30 )) {
		/* NOTE: Dangun can not use ticket if quest is less than 30,
			   This is to prevent negative quest.  */
		    send_to_char("Come back when you solve more quest.", ch);
		    return TRUE;
		}

 		/* NOTE:  Reduced Quest to use ticket:
		    AC: 30 -> 20  DR: 30 -> 20  HR: 20 -> 10. */
		/* use meta ticket */
		switch (GET_OBJ_VIRTUAL(tmp_obj)) {
		case OBJ_TICKET_AC:	/* ticket for AC */
		    GET_AC(ch) -= number(2, 3);
		    GET_EXP(ch) -= 200000000;
		    if (GET_AC(ch) < -120)
			ch->quest.solved  -= 20;
		    else
			ch->quest.solved -= 7;
		    break;
		case OBJ_TICKET_HR:	/* ticket for HR */
		    GET_HITROLL(ch) += number(1, 2);
		    GET_EXP(ch) -= 150000000;
		    if (GET_HITROLL(ch) > 100)
			ch->quest.solved  -= 10;
		    else
			ch->quest.solved -= 6;
		    break;
		case OBJ_TICKET_DR:	/* ticket for DR */
		    /*
		       GET_DAMROLL(ch) += number(1, 2); */
		    GET_DAMROLL(ch)++;
		    GET_EXP(ch) -= 250000000;
		    if (GET_DAMROLL(ch) > 100)
			ch->quest.solved  -= 20;
		    else
			ch->quest.solved -= 8;
		    break;
		default:
		    send_to_char("I cannot understand your ticket.\r\n", ch);
		    return TRUE;
		}

		extract_obj(tmp_obj);
	    }
	    else {
		send_to_char("You do not have that item.\r\n", ch);
	    }
	    return TRUE;
	}
    }
    return (FALSE);
}

/* ******************************************************/
/*  Origin: This taxi() code was in guild_command.c 	*/
/* ******************************************************/
/* shuttle bus to KAIST */

/*
char *msg_for_taxi[] =
{
    "NOT-DEFINED",
    "Welcome to KAIST!!!\r\n",
    "Welcome to Process' house\r\n"
};

char *taxi_name[] =
{
    "NOT-DEFINED",
    "kaist",
    "process",
};
*/

char *where_to_taxi[] =
{
    "NOT-DEFINED",
    "KAIST",
    "Process' House",
};

/* NOTE: taxi fare should be more realistic :P  */
int charge_taxi[] =
{
    0,
    20000,
    30000,
};

int room_taxi[] =
{
    0,
    TAXI_STOP_KAIST,	// NOTE: KAIST zone renumber 31001->17411
    TAXI_STOP_PROCESS,
};

int level_taxi[] =
{
    0,
    10,
    10
};

int taxi(struct char_data *ch, int cmd, char *arg)
{
    char buf[MAX_LINE_LEN], *where_to;
    int charge;
    int taxi_num;
    int to_room;

    if (IS_NPC(ch))
	return FALSE;
    if (cmd != CMD_TAXI) 	/* taxi  or call */
	return FALSE;

    // room_number = world[ch->in_room].number;
    // case ROOM_TAXI_STOP:

    arg = one_argument(arg, buf);

    if ( strcasecmp(buf, "kaist") ==0 )
	taxi_num = 1;
    else if ( strcasecmp(buf, "process") == 0)
	taxi_num = 2;
    else  {
	send_to_char("To where? 'taxi kaist' or 'taxi process'.\r\n", ch);
	return TRUE;
    }
    where_to =  where_to_taxi[taxi_num];

    to_room = real_room(room_taxi[taxi_num]);
    charge =  charge_taxi[taxi_num];

    if (GET_LEVEL(ch) < level_taxi[taxi_num]) {
	send_to_char("You cannot ride this taxi. Sorry", ch);
	return TRUE;
    }
    sprintf(buf, "Welcome to %s!!!\r\n", where_to);
    send_to_char(buf, ch);
    send_to_char("Sit down on your seat, and wait for a while\r\n", ch);
    sprintf(buf, "The taxi with %s starts to leave for %s.", GET_NAME(ch), where_to);
    act(buf, TRUE, ch, 0, 0, TO_ROOM);
    char_from_room(ch);
    sprintf(buf, "OK. Here is %s.\r\n", where_to);
    send_to_char(buf, ch);
    sprintf(buf, "The charge is %d.\r\n", charge);
    send_to_char(buf, ch);
    if (GET_GOLD(ch) < charge_taxi[taxi_num]) {
	send_to_char("The taxi driver slaps you!!!\r\n", ch);
	send_to_char("Get the fuck out!!\r\n", ch);
	GET_HIT(ch) = 1;
	GET_MOVE(ch) = 1;
	/* NOTE: taxi driver takes away all the money you have */
	GET_GOLD(ch) = 0;
	char_to_room(ch, real_room(MID_PORTAL));
	do_look(ch, "", 15);
	return TRUE;
    }
    GET_GOLD(ch) -= charge;
    char_to_room(ch, to_room);
    do_look(ch, "", 15);
    return TRUE;
}

int pet_shops(struct char_data *ch, int cmd, char *arg)
{
    char buf[MAX_STRING_LENGTH], pet_name[256];
    int pet_room, k;
    struct char_data *pet;
    struct follow_type *j;
    extern void add_follower(struct char_data *ch, struct char_data *leader);

    if (IS_NPC(ch))
	return (FALSE);
    pet_room = ch->in_room + 1;
    if (cmd == 59) {	/* List */
	send_to_char("애완동물 이 이런 것이 있습니다:\r\n", ch);
	for (pet = world[pet_room].people; pet; pet = pet->next_in_room) {
	    /* can't buy PC */
	    if (!IS_NPC(pet))
		continue;
	    sprintf(buf, "%8lld - %s\r\n", 10 * GET_EXP(pet), pet->player.short_descr);
	    send_to_char(buf, ch);
	}
	return (TRUE);
    }
    else if (cmd == CMD_BUY) {	/* Buy */
	arg = one_argument(arg, buf);
	arg = one_argument(arg, pet_name);
	/* Pet_Name is for later use when I feel like it */
	/* NOTE: Don't use obsoleted get_char_room() */
	/* if (!(pet = get_char_room(buf, pet_room))) */
	if (!(pet = get_char_room_vis(world[pet_room].people , buf))) {
	    send_to_char("아 그런 동물은 없는데요 ?\r\n", ch);
	    return (TRUE);
	}
	if (!IS_MOB(pet)) {
	    send_to_char("사람은 안파는데요 ?\r\n", ch);
	    return (TRUE);
	}
	for (k = 0, j = ch->followers; (j) && (k < 5); ++k) {
	    j = j->next;
	}
	if (k >= 3) {
	    send_to_char("당신은 지금 데리고 있는 동물로 충분할 것 같은데요.\r\n", ch);
	    return (TRUE);
	}
	if (GET_GOLD(ch) < (GET_EXP(pet) * 10)) {
	    send_to_char("돈 더 가져오세요!\r\n", ch);
	    return (TRUE);
	}
	GET_GOLD(ch) -= GET_EXP(pet) * 10;
	pet = read_mobile(pet->nr, REAL);
	GET_EXP(pet) = 0;
	SET_BIT(pet->specials.affected_by, AFF_CHARM);
	if (*pet_name) {
	    sprintf(buf, "%s %s", pet->player.name, pet_name);
	    free(pet->player.name);
	    pet->player.name = strdup(buf);

	    sprintf(buf, "%sA small sign on a chain around the neck says 'My Name is %s'\r\n",
		    pet->player.description, pet_name);
	    free(pet->player.description);
	    pet->player.description = strdup(buf);
	}

	char_to_room(pet, ch->in_room);
	add_follower(pet, ch);

	/* Be certain that pet's can't get/carry/use/wield/wear items */
/*
   And why not?

   GET_CARRYING_W(pet) = 1000;
   GET_CARRYING_N(pet) = 100;
 */
	send_to_char("애완동물이 마음에 드실껍니다.\r\n", ch);
	act("$n님이 $N 을 데리고 갑니다.", FALSE, ch, 0, pet, TO_ROOM);

	return (TRUE);
    }

    /* All commands except list and buy */
    return (FALSE);
}

int portal(struct char_data *ch, int cmd, char *arg)
{
    int location, ok;

    if (cmd != CMD_SOUTH)	/* specific to Room 2176 */
	return (FALSE);
    location = number(1, top_of_world);
    ok = TRUE;
    if (IS_SET(world[location].room_flags, OFF_LIMITS))
	ok = FALSE;
    else if (IS_SET(world[location].room_flags, PRIVATE))
	ok = FALSE;
    if (!ok) {
	send_to_char("You bump into something, and go nowhere.\r\n", ch);
	act("$n seems to bump into nothing??", FALSE, ch, 0, 0, TO_NOTVICT);
    }
    else {
	act("$n님이 없어진것 같네요 ??", FALSE, ch, 0, 0, TO_NOTVICT);
	send_to_char("You are momentarily disoriented.\r\n", ch);
	char_from_room(ch);
	char_to_room(ch, location);
	do_look(ch, "", 15);
    }
    return (TRUE);
}

int level_gate(struct char_data *ch, int cmd, char *arg)
{
    char buf[100];
    int f, r;

    if (cmd == CMD_CAST) {
	send_to_char("You cannot do that here.\r\n", ch);
	sprintf(buf, "%s attempts to misbehave here.\r\n", ch->player.name);
	send_to_room_except(buf, ch->in_room, ch);
	return TRUE;
    }

    f = 0;
    r = world[ch->in_room].number;
    switch (cmd) {
    case CMD_NORTH:		/* north */
    case CMD_EAST:		/* east */
	break;
    case CMD_SOUTH:		/* south */
	if (r == LEVGATE_CLUBSAFARI)		    // Club Safari
	    f = (GET_LEVEL(ch) < 12);
	else if (r == LEVGATE_MAGETOWER)	    // To Mage Tower
	    f = (GET_LEVEL(ch) < 25);
	else if (r == LEVGATE_TB8TH)	    // TB floor 8
	    f = (GET_LEVEL(ch) > 15);
	else if (r == LEVGATE_SHIRE)	    // To SHIRE
	    f = (GET_LEVEL(ch) > 12);
	else if (r == LEVGATE_ARENA)	    // To Arena
	    f = (GET_LEVEL(ch) > 3);
	break;
    case CMD_WEST:		/* west */
	if (r == LEVGATE_EASTCASTLE)		    // Eest Castle
	    f = (GET_LEVEL(ch) > 20);
	else if (r == LEVGATE_THALOS)	    // to Thalos
	    f = (GET_LEVEL(ch) > 24 || GET_LEVEL(ch) < 10);
	break;
    case CMD_UP:		/* up */
    case CMD_DOWN:		/* down */
    default:
	break;
    }

    if ((f) && IS_MORTAL(ch)) {
	act("$n attempts go to where $e is not welcome.", FALSE, ch, 0, 0, TO_ROOM);
	send_to_char("People of your level may not enter.\r\n", ch);
	return TRUE;
    }
    return FALSE;
}

int string_machine(struct char_data *ch, int cmd, char *arg)
{
    struct obj_data *obj;
    char buf[256];

    /* change */
    if (cmd != CMD_CHANGE)
	return 0;

    if (GET_GOLD(ch) < 10000000) {
	act("You should earn money for this service.",
	    FALSE, ch, 0, 0, TO_CHAR);
	return 1;
    }

    arg = one_argument(arg, buf);
    obj = get_obj_in_list_vis(ch, buf, ch->carrying);
    if (!obj) {
	act("You do not have that item.", FALSE, ch, 0, 0, TO_CHAR);
	return 1;
    }

    /* remove space */
    while (*arg == ' ')
	arg++;
    if (!*arg) {	/* null string */
	act("You should propose the name.", FALSE, ch, 0, 0, TO_CHAR);
	return 1;
    }

    free(obj->short_description);
    CREATE(obj->short_description, char, strlen(arg) + 1);
    strcpy(obj->short_description, arg);

    GET_GOLD(ch) -= 10000000;

    act("STRING Service DONE.\r\n", FALSE, ch, 0, 0, TO_CHAR);

    return 1;
}

int slot_machine(struct char_data *ch, int cmd, char *arg)
{
    int num[3];
    int same;
    char buf[256];

    if (cmd != CMD_PULL)
	return 0;

    /* no mob */
    if (IS_NPC(ch))
	return 1;

    /* no gold */
    if (GET_GOLD(ch) < 1000000) {
	act("You should have money!!!", FALSE, ch, 0, 0, TO_CHAR);
	return 1;
    }

    num[0] = number(0, 9);
    num[1] = number(0, 9);
    num[2] = number(0, 9);

    same = 0;
    same += (num[0] == num[1]);
    same += (num[1] == num[2]);
    same += (num[2] == num[0]);

    sprintf(buf, "Your dices are %d, %d, %d.", num[0], num[1], num[2]);
    act(buf, FALSE, ch, 0, 0, TO_CHAR);

    if (same == 0) {
	act("You lost your coins.", FALSE, ch, 0, 0, TO_CHAR);
	GET_GOLD(ch) -= 1000000;
    }
    else if (same == 1) {
	act("Good!!! You win this turn!!!", FALSE, ch, 0, 0, TO_CHAR);
	/* GET_GOLD(ch) += 2000000; */
	gain_gold(ch, 2000000);		/* NOTE: check overflow */
    }
    else {
	sprintf(buf, "Excellent!!! %s made JACKPOT!!!\r\n", GET_NAME(ch));
	act(buf, FALSE, ch, 0, 0, TO_ROOM);
	act(buf, FALSE, ch, 0, 0, TO_CHAR);
	/*
	   send_to_all(buf); */
	/* GET_GOLD(ch) += 20000000; */
	gain_gold(ch, 20000000);	/* NOTE: check overflow */
    }

    return 1;
}

int totem(struct char_data *ch, int cmd, char *arg)
{
    int rm;
    struct obj_data *obj;
    extern void list_all_in_room(int room, struct char_data *ch);

    if (cmd != CMD_LOOK)
	return (FALSE);
    obj = ch->equipment[HOLD];
    if (!obj)
	return (FALSE);
    if (GET_OBJ_VIRTUAL(obj) != 1311)
	return (FALSE);
    if (!ch->followers)
	return (FALSE);
    rm = ch->followers->follower->in_room;
    send_to_char(world[rm].name, ch);
    send_to_char("\r\n", ch);
    if (!IS_SET(ch->specials.act, PLR_BRIEF))
	send_to_char(world[rm].description, ch);
    /* NOTE:   same as list_obj_to_char() + list_char_to_char()    */
    list_all_in_room(rm, ch);
/*
   list_obj_to_char(world[rm].contents, ch, 0,FALSE);
   list_char_to_char(world[rm].people, ch, 0);
 */
    return (TRUE);
}

int magicseed(struct char_data *ch, int cmd, char *arg)
{
    int skno;
    char name[256];
    struct obj_data *obj;

    if (cmd != CMD_EAT)
	return (FALSE);
    one_argument(arg, name);
    obj = get_obj_in_list_vis(ch, name, ch->carrying);
    if (!obj)
	return (FALSE);
    if (GET_COND(ch, FULL) > 20 && GET_COND(ch, FULL) != -1)
	return (FALSE);
    if (GET_OBJ_VIRTUAL(obj) != 2157)
	return (FALSE);

    do {
	skno = number(0, MAX_SKILLS - 1);
	if (ch->skills[skno].learned > 50 && ch->skills[skno].learned < 99) {
	    ch->skills[skno].learned = 99;
	    send_to_char("자욱한 연기가 눈앞을 가립니다.\r\n", ch);
	    if (GET_COND(ch, FULL) >= 0)
		GET_COND(ch, FULL)++;
	    break;
	}
    } while (number(0, 10));

    extract_obj(obj);
    send_to_char("OK.\r\n", ch);
    return TRUE;
}
