/* ************************************************************************
   *  file: spec_procs.c , Special module.                   Part of DIKUMUD *
   *  Usage: Procedures handling special procedures for Midgaard mobile   *
   *  Copyright (C) 1990, 1991 - see 'license.doc' for complete information. *
   ************************************************************************* */

#include <stdio.h>
#include <string.h>

#include "char.h"
#include "object.h"
#include "global.h"
#include "comm.h"
#include "play.h"
#include "actions.h"
#include "spells.h"
#include "etc.h"


/* ********************************************************************
   *  Special procedures for mobiles                                      *
   ******************************************************************** */

int puff(struct char_data *ch, int cmd, char *arg)
{
    if (cmd)
	return (0);

    switch (number(0, 60)) {
    case 0:
	do_say(ch, "My god! It's full of stars!", 0);
	return (1);
    case 1:
	do_say(ch, "How'd all those fish get up here?", 0);
	return (1);
    case 2:
	do_say(ch, "I'm a very female dragon.", 0);
	return (1);
    case 3:
	do_say(ch, "I've got a peaceful, easy feeling.", 0);
	return (1);
    default:
	return (0);
    }
}

int perhaps(struct char_data *ch, int cmd, char *arg)
{
    struct char_data *vict, *next;
    static int perhaps_words_size = 0;
    int i;

    /* NOTE: not change of functionality, just to make it easy to add tips */
    /* NOTE: Moved perhaps_words[] definition to "constants.c"   */
    extern char *perhaps_words[];

#ifdef 	MID_HELPER
    static struct char_data *perhaps = NULL;
    char buf[MAX_LINE_LEN];
    char *blessings[3] = {
	    "%s님에게 신의 가호가 있기를...",
	    "%s님에게 축복을 드립니다...",
	    "%s님에게 신의 은총이 내리기를..." ,
    };
    while(*arg==' ') arg++;
    if (cmd == CMD_BOW && strcasecmp(arg, MID_HELPER)==0 	// bow narai
	    && strcasecmp(GET_NAME(ch), MID_HELPER) != 0) {
	cast_sanctuary(GET_LEVEL(perhaps), perhaps, NULL,
	    SPELL_TYPE_SPELL, ch, NULL);
	cast_haste(GET_LEVEL(perhaps), perhaps, NULL,
	    SPELL_TYPE_SPELL, ch, NULL);

	char *msg =  blessings[number(0,2)];
	sprintf(buf, msg , GET_NAME(ch));
	do_say(perhaps, buf, 0);
	return 0;
    }
    if (perhaps == NULL && cmd == 0 && ch != NULL
	&& mob_index[ch->nr].virtual == MOB_MID_HELPER ) {
	perhaps  = ch;
	REMOVE_BIT( ch->specials.affected_by, AFF_SANCTUARY|AFF_LOVE );
    }
#endif

    if (cmd) return (0);	/* If void return */
    if (!ch) return (0);
    /* NOTE: initialize tips array size */
    if( perhaps_words_size  == 0 )
	perhaps_words_size = search_block("END_OF_TIPS", perhaps_words, 1 );

    for (vict = world[ch->in_room].people; vict; vict = next) {
	next = vict->next_in_room;
	if (vict && IS_NPC(vict) && ch != vict) {
	    /* NOTE: Added message killing unwelcomed mob */
	    act("$n embraces $N and blows $S head off.",
		0, ch, 0, vict, TO_ROOM);
	    die(vict, GET_LEVEL(ch), ch);
	    continue;
	}
	if (vict && ((vict->points.max_hit / 9) > vict->points.hit)) {
	    /* act("$n kisses $N on $s cheek.",1,ch,0,vict,TO_ROOM); */
	    /* NOTE: Don't hide kiss to who can not see her */
	    act("$n kisses you on your cheek.", 0, ch, 0, vict, TO_VICT);
	    act("$n kisses $N on $S cheek.", 1, ch, 0, vict, TO_NOTVICT);
	    vict->points.hit = vict->points.max_hit;
	}
#ifdef UNUSED_CODE
	if( vict&&((vict->points.armor)>1) ) {
	    act("$n hugs $N.", 1, ch, 0, vict, TO_ROOM);
	    vict->points.armor=1;
	}
	if( vict&&((vict->points.hitroll) < GET_LEVEL(vict))
		&&IS_MORTAL(vict)) {
	    act("$n pats $N.", 1, ch, 0, vict, TO_ROOM);
	    vict->points.hitroll=GET_LEVEL(vict);
	}
	if( vict&&((vict->points.damroll) < GET_LEVEL(vict))
		&&IS_MORTAL(vict)) {
	    act("$n raises $s POWER!!!", 1, ch, 0, vict, TO_ROOM);
	    vict->points.damroll=GET_LEVEL(vict);
	}
#endif		// UNUSED_CODE
    }
    /* NOTE: To sustain similar frequency of perhaps talk */
    i = number(0, 1000);
    if (i < perhaps_words_size )
	    do_say(ch, perhaps_words[i], 0);

    return 1;
}

int mayor(struct char_data *ch, int cmd, char *arg)
{
    static char open_path[] =
    "W3a3003b33000c111d0d111Oe333333Oe22c222112212111a1S.";

    static char close_path[] =
    "W3a3003b33000c111d0d111CE333333CE22c222112212111a1S.";

    /* NOTE: Init path, index for safety. They should not need to be inited. */
    static char *path = open_path;
    static int index = 0;
    static bool move = FALSE;

    void do_move(struct char_data *ch, char *argument, int cmd);
    void do_open(struct char_data *ch, char *argument, int cmd);
    void do_lock(struct char_data *ch, char *argument, int cmd);
    void do_unlock(struct char_data *ch, char *argument, int cmd);
    void do_close(struct char_data *ch, char *argument, int cmd);

    if (!move) {
	if (time_info.hours == 6) {
	    move = TRUE;
	    path = open_path;
	    index = 0;
	}
	else if (time_info.hours == 20) {
	    move = TRUE;
	    path = close_path;
	    index = 0;
	}
    }

    if (cmd || !move || (GET_POS(ch) < POS_SLEEPING) ||
	(GET_POS(ch) == POS_FIGHTING))
	return FALSE;

/* NOTE: BUG SEGV Error at switch(path[index])  */
/*   index or path[] were randomly and intermittentely overwritten by someone.
     I suspect some stray pointer refernce or spurious buffer overun.
     But I can't zero in the spoiler and kill it. Need SEARCH and DESTROY IT!!
     Plz watch closely affect_remove() and it's callers. Esp. affect_update().
     BAND-AID FIX: static path, index initalized. Shouldn't need it.
*/
    switch (path[index]) {
    case '0':
    case '1':
    case '2':
    case '3':
	do_move(ch, "", path[index] - '0' + 1);
	break;

    case 'W':
	GET_POS(ch) = POS_STANDING;
	acthan("$n awakens and groans loudly.",
	       "$n님이 일어나 기지개를 켜십니다", FALSE, ch, 0, 0, TO_ROOM);
	break;

    case 'S':
	GET_POS(ch) = POS_SLEEPING;
	acthan("$n lies down and instantly falls asleep.",
	"$n님이 자리에 눕더니 금새 잠이 듭니다.", FALSE, ch, 0, 0, TO_ROOM);
	break;

    case 'a':
	acthan("$n says 'Hello Honey!'",
	"$n님이 '안녕 내사랑!' 이라고 말합니다.", FALSE, ch, 0, 0, TO_ROOM);
	acthan("$n smirks.", "$n님이 점잔빼며 웃습니다.",
		FALSE, ch, 0, 0, TO_ROOM);
	break;

    case 'b':
	acthan("$n says 'What a view! I must get something done about dump!'",
       "$n님이 '하! 쓰레기에 대해서 뭔가 조치를 취해야 겠네!' 라고 말합니다",
	       FALSE, ch, 0, 0, TO_ROOM);
	break;

    case 'c':
	acthan("$n says 'Vandals! Youngsters nowadays have no respect for anything!'",
       "$n님이 '이런 이런! 젊은것들은 요즘 인사를 안한단말야!' 라고 말합니다.",
	       FALSE, ch, 0, 0, TO_ROOM);
	break;

    case 'd':
	acthan("$n says 'Good day, citizens!'",
	       "$n님이 '시민 여러분 안녕들 하십니까!'라고 말합니다",
	       FALSE, ch, 0, 0, TO_ROOM);
	break;

    case 'e':
	acthan("$n says 'I hereby declare the bazaar open!'",
	       "$n님이 '자 이제 시장을 열겠습니다 !' 라고 말합니다",
	       FALSE, ch, 0, 0, TO_ROOM);
	break;

    case 'E':
	acthan("$n says 'I hereby declare Midgaard closed!'",
	       "$n님이 '자 이제 문을 닫았습니다!' 라고 말합니다",
	       FALSE, ch, 0, 0, TO_ROOM);
	break;

    case 'O':
	do_unlock(ch, "gate", 0);
	do_open(ch, "gate", 0);
	break;

    case 'C':
	do_close(ch, "gate", 0);
	do_lock(ch, "gate", 0);
	break;

    case '.':
	move = FALSE;
	break;
    }
    ++index ;

    return FALSE;
}

int Quest_bombard(struct char_data *ch, int cmd, char *arg)
{			/* question function and level up monster */
    struct char_data *vict;
    struct obj_data *list;
    int maxnum = 0, newnum = 0;
    int dam;

/* bombard like sword 2702 */
/* bombard room is 3035 and other room is 3094 */
/* if ch->in_room == real_room(3035)  .... */

    if (ch->in_room == real_room(ROOM_MID_BOMBARD))
	newnum = OBJ_SWORD_BOMBARD;
    else if (ch->in_room == real_room(ROOM_MID_STORAGE))
	newnum = OBJ_BOMBARD_MINOR;
    else
	newnum = OBJ_SWORD_BOMBARD;

    if (cmd)
	return (FALSE);
    for (list = ch->carrying; list; list = list->next_content) {
	if (GET_OBJ_VIRTUAL(list) == OBJ_SWORD_PARTICLE)
	    maxnum++;
    }
    if (maxnum >= 7) {
	while ((list = ch->carrying))
	    extract_obj(list);
	if ((newnum = real_object(newnum)) >= 0) {
	    list = read_object(newnum, REAL);
	    obj_to_char(list, ch);
	    wear(ch, list, 12);		/* NOTE: Wield it */
	}
	return TRUE;
    }
    else if (maxnum < 7 && maxnum > 0)
	do_say(ch, "I need more particle.", 0);

    if (ch->master && ch->in_room == ch->master->in_room) {
	/* NOTE: master of bombard? who? Let's see... Log it for DEBUGGING. */
	log("DEBUG: BOMBARD MASTER HIT!!!");
	hit(ch, ch->master, TYPE_UNDEFINED);
    }
    if (! ch->specials.fighting)
	return (FALSE);

    if (!IS_AFFECTED(ch, SPELL_SANCTUARY)) {
	spell_sanctuary(GET_LEVEL(ch), ch, ch, 0 );
	return(TRUE);
    }
    vict = ch->specials.fighting;
    /* NOTE: make sure that victim is in same room */
    if ( ch->in_room != vict->in_room)
	return(FALSE);
    switch(number(1,11)) {
    case 1:
	if ( GET_LEVEL(ch) < 40) {
	    act("$n	get experience .", 1, ch, 0, 0, TO_ROOM);
	    act("$n	LEVEL UP !.", 1, ch, 0, 0, TO_ROOM);
	    GET_LEVEL(ch) += 1;
	}
	break;
    case 2:
    case 3:
	do_say(ch, "Shou Ryu Ken..", 0);
	dam = GET_HIT(vict)/3 - number(1, GET_HIT(vict)/8 + GET_LEVEL(vict)/2);
	/* NOTE: Use damage() of BOMBARD type */
	damage(ch, vict, dam, WEAPON_BOMBARD);
	send_to_char("당신은 꽈당 넘어집니다.\r\n", vict);
	send_to_char("크으으아아아아 . .  .  . \r\n", vict);
	break;
    case 4:
	act("$n utters the words 'frost'.", 1, ch, 0, 0, TO_ROOM);
	spell_cone_of_ice(GET_LEVEL(ch), ch, vict, 0);
	break;
    case 5:
	act("$n utters the words 'sunburst'.", 1, ch, 0, 0, TO_ROOM);
	spell_sunburst(GET_LEVEL(ch), ch, vict, 0);
	break;
    case 6:
	act("$n utters the words 'fireball'.", 1, ch, 0, 0, TO_ROOM);
	spell_fireball(GET_LEVEL(ch), ch, vict, 0);
	break;
    case 7:
	do_bash(ch, GET_NAME(vict), 0);
	break;
    case 8:
	do_multi_kick(ch, GET_NAME(vict), 0);
	break;
    case 9:
	do_punch(ch, GET_NAME(vict), 0);
	break;
    case 10:
    case 11:
	hit(ch, vict, TYPE_UNDEFINED);
	break;
    }
    return (1);
}

int mud_message(struct char_data *ch, int cmd, char *arg)
{
    struct char_data *vict;

    if (cmd)
	return (0);
    vict = ch->specials.fighting;
    if (vict && !IS_NPC(vict) && (vict->in_room == ch->in_room)) {
	act("$n annihilate you with his full power.", 1, ch, 0, 0, TO_ROOM);
	/* NOTE: Damage type changed to check victim's location.
		TYPE_UNDEFINED -> TYPE_HIT   */
	if (vict->points.hit >= 500 && vict->points.hit < 1000)
	    damage(ch, vict, dice(20, GET_LEVEL(ch)), TYPE_HIT);
	else if (vict->points.hit >= 1000 && vict->points.hit < 2000)
	    damage(ch, vict, dice(30, GET_LEVEL(ch)), TYPE_HIT);
	else if (vict->points.hit >= 2000 && vict->points.hit < 4000)
	    damage(ch, vict, dice(35, GET_LEVEL(ch)), TYPE_HIT);
	else if (vict->points.hit >= 4000 && vict->points.hit < 8000)
	    damage(ch, vict, dice(45, GET_LEVEL(ch)), TYPE_HIT);
	else if (vict->points.hit >= 8000 && vict->points.hit < 16000)
	    damage(ch, vict, dice(60, GET_LEVEL(ch)), TYPE_HIT);
	/* NOTE: More damage to high hit point victim */
	else if (vict->points.hit >= 16000 )
	    damage(ch, vict, dice(80, GET_LEVEL(ch)), TYPE_HIT);
	return (TRUE);
    }
    return (FALSE);
}

int deathcure(struct char_data *ch, int cmd, char *arg)
{
    struct char_data *vict;
    int h;

    if (cmd)
	return (0);
    vict = world[ch->in_room].people;
    h = GET_HIT(ch);
    if (h < 10000)
	cast_full_heal(GET_LEVEL(ch), ch, "", SPELL_TYPE_SPELL, ch, 0);
    if (GET_POS(ch) == POS_FIGHTING)
	return (TRUE);
    if (vict->equipment[WEAR_ABOUTLEGS]) {
	if (GET_OBJ_VIRTUAL(vict->equipment[WEAR_ABOUTLEGS])== OBJ_ANTI_DEATHCURE) {
	    /* ANTI deathcure */
	    act("$n tries to do something to you, but failed miserably by ANTI deathcure."
		,1, ch, 0, 0, TO_ROOM);
	    return (TRUE);
	}
    }
    if (vict && !IS_NPC(vict)) {
	if (number(1, 10) > 7) {
	    act("$n control your body ZERO.", 1, ch, 0, 0, TO_ROOM);
	    vict->points.hit = 1;
	    vict->points.move = 0;
	    act("$n Wuhhhhhh...... thank you very much ? .", 1, ch, 0, 0, TO_ROOM);
	    return TRUE;
	}
	else if (number(1, 10) > 8) {
	    vict->points.hit += 60 * (1 + GET_LEVEL(vict) / 7);
	    act("$n Wuhhhhhh...... that's too bad ! ? .", 1, ch, 0, 0, TO_ROOM);
	    return TRUE;
	}
	else if (number(1, 10) > 5) {
	    act("$n control your mana ZERO.", 1, ch, 0, 0, TO_ROOM);
	    vict->points.mana = 0;
	    act("$n hmmmmm.... great miracle !!! ", 1, ch, 0, 0, TO_ROOM);
	    return TRUE;
	}
	else if (number(1, 10) > 9) {
	    act("$n hmmmmm.... miracle !!!", 1, ch, 0, 0, TO_ROOM);
	    vict->points.mana += 40 * (1 + GET_LEVEL(vict) / 9);
	    return TRUE;
	}
    }
    switch (number(0, 30)) {
    case 0:
	do_say(ch, "Chouchouchouchouzzzzzaaaa. .....", 0);
	return (1);
    case 1:
	do_say(ch, "HaHaHa ......", 0);
	return (1);
    case 2:
	do_say(ch, "You are insect me !! ......", 0);
	return (1);
    case 3:
	do_say(ch, "if you give me 50000 coins , i forgive me your crime ...", 0);
	return (1);
    case 4:
	do_say(ch, "I can make your body Zero ! ", 0);
	return (1);
    case 5:
	do_say(ch, "I can heal your body until 100 ! ", 0);
	return (1);
    case 6:
	do_say(ch, "Please, allow me to make your body ZERO.", 0);
	return (1);
    case 7:
	do_say(ch, "I admire Dangun the greatgod !", 0);
	return (1);
    case 8:
	act("$n try to steal your inventory .", 1, ch, 0, 0, TO_ROOM);
	return (1);
    case 9:
	act("$n hashes you to bloody sushi pieces with his slash.",
	    1, ch, 0, 0, TO_ROOM);
	/*FALLTHRU*/
    case 10:
	act("$n decimates you to micro mesh powder with his pound.",
	    1, ch, 0, 0, TO_ROOM);
	/*FALLTHRU*/
    case 11:
	act("$n disintegrates you to thounsand particles with his pierce.",
	    1, ch, 0, 0, TO_ROOM);
	/*FALLTHRU*/
    case 12:
    case 13:
	act("$n massacres you to small fragment with his slash.",
	    1, ch, 0, 0, TO_ROOM);
	return (1);
    default:
	return (0);
    }
}

int super_deathcure(struct char_data *ch, int cmd, char *arg)
{
    struct char_data *vict, *mob;
    struct char_data *choose_victim(struct char_data *mob, int fightmode,
				    int mode);

    int h, real_number, ran_num;

    if (cmd)
	return (0);
    /* cyb vict=world[ch->in_room].people; */
    vict = choose_victim(ch, VIC_ALL, MODE_MOVE_MAX);

    h = GET_HIT(ch);
    if (h < 5000)
	cast_full_heal(GET_LEVEL(ch), ch, "", SPELL_TYPE_SPELL, ch, 0);
    if (vict && !IS_NPC(vict)) {
	if (number(1, 10) > 7) {
	    act("$n control your move -100.", 1, ch, 0, 0, TO_ROOM);
	    vict->points.move = 0;
	    vict->points.move -= 200;
	    act("$n Wuhhhhhh...... thank you very much ? .", 1, ch, 0, 0, TO_ROOM);
	    return TRUE;
	}
	else if (number(1, 10) > 9) {
	    act("$n hmmmmm.... miracle !!!", 1, ch, 0, 0, TO_ROOM);
	    vict->points.mana += 40 * (1 + GET_LEVEL(vict) / 9);
	    return TRUE;
	}
    }
    vict = ch->specials.fighting;
    if (vict) {
	h = number(0, 12);
	if (h > 7)
	    return (0);
	switch (h) {
	case 0:
	    ran_num = MOB_SDC_HELPER1;
	    break;	/* Drow 2nd mother */
	case 1:
	    ran_num = MOB_SDC_HELPER2;
	    break;	/* dracolich */
	case 2:
	    ran_num = MOB_SDC_HELPER3;
	    break;	/* giant purple worm */
	case 3:
	    ran_num = MOB_SDC_HELPER4;
	    break;	/* medusa third gorgon */
	case 4:
	    ran_num = MOB_SDC_HELPER5;
	    break;	/* hecate */
	case 5:
	case 6:
	    ran_num = MOB_SDC_HELPER6;
	    break;	/* semi robot : cyb */
	case 7:
	case 8:
	    ran_num = MOB_SDC_HELPER7;
	    break;	/* head soldier wee : cyb */
	default:
	    return (1);
	}
	if ((real_number = real_mobile(ran_num))) {
	    act("$n utters 'Em pleh! Em pleh!! Em pleh!!!", 1, ch, 0, 0, TO_ROOM);
	    mob = read_mobile(real_number, REAL);
	    char_to_room(mob, ch->in_room);
	}
	return (1);
    }
    return (0);
}

int musashi(struct char_data *ch, int cmd, char *arg)
{
    struct char_data *choose_victim(struct char_data *mob, int fightmode,
				    int mode);
    struct char_data *vict, *next_vict;
    int i, dam;

    if (cmd)
	return (0);

    if( GET_HIT(ch) < GET_MAX_HIT(ch)/2)
	cast_full_heal(GET_LEVEL(ch), ch, "", SPELL_TYPE_SPELL,  ch, 0);
    else if (! ch->specials.fighting ) {
	/* cyb  vict=world[ch->in_room].people; */
	vict = choose_victim(ch, VIC_ALL, MODE_HIGH_LEVEL);

	/* high level victim is already selected */
	if (vict && number(0, LEVEL_LIMIT - GET_LEVEL(vict)) < 4) {
	    send_to_char("I CHALLENGE YOU.\r\n", vict);
	    hit(ch, vict, TYPE_UNDEFINED);
	    return (TRUE);
	}
	return (FALSE);
    }
    else if (ch->specials.fighting && !affected_by_spell(ch, SPELL_SANCTUARY)) {
	act("$n 주위로 휘황한 광채가  감쌉니다.", TRUE, ch, 0, 0, TO_ROOM);
	spell_sanctuary(GET_LEVEL(ch), ch, ch, 0 );
	return (TRUE);
    }

    for ( vict = ch->specials.fighting; vict; vict = next_vict) {
	next_vict = vict->next_in_room;
	if (vict->specials.fighting == ch) {
	    hit(ch, vict, TYPE_UNDEFINED);
	    hit(ch, vict, TYPE_UNDEFINED);
	}
    }

    vict = ch->specials.fighting;
    if (!vict || ch->in_room != vict->in_room)
	return (FALSE);
    switch (number(0, 25)) {
    case 1:
    case 2:
	do_kick(ch, GET_NAME(vict), 0 );
	break;
    case 3:
    case 4:
	do_bash(ch, GET_NAME(vict), 0 );
	break;
    case 5:
	act("$n utters the words 'fire'.", 1, ch, 0, 0, TO_ROOM);
	spell_sunburst(GET_LEVEL(ch), ch, vict, 0);
	break;
    case 6:
	act("$n utters the words 'frost'.", 1, ch, 0, 0, TO_ROOM);
	spell_cone_of_ice(GET_LEVEL(ch), ch, vict, 0);
	break;
    case 7:
	do_say(ch, "A cha cha cha cha ..", 0);
	act("$n tornado fire with miracle speed .", 1, ch, 0, 0, TO_ROOM);
	for (i = 0; ch->specials.fighting && i < number(5, 8); i++)
	    spell_sunburst(GET_LEVEL(ch), ch, ch->specials.fighting, 0);
	return(1);
    case 8:
	act("$n double attack by double sword method .", 1, ch, 0, 0, TO_ROOM);
	hit(ch, vict, TYPE_UNDEFINED);
	hit(ch, vict, TYPE_UNDEFINED);
	break;
    case 9:
	act("$n double kick by double circle kick .", 1, ch, 0, 0, TO_ROOM);
	    do_kick(ch, GET_NAME(vict), TYPE_UNDEFINED);
	if(ch->specials.fighting)
	    do_kick(ch, GET_NAME(vict), TYPE_UNDEFINED);
	break;
    case 10:
	do_say(ch, "Heau Heau Heau Heau Heau..", 0);
	act("$n use hundreds bash .", 1, ch, 0, 0, TO_ROOM);
	for (i = 0; i < number(3, 6); i++)
	    do_bash(ch, GET_NAME(vict), 0 );
	WAIT_STATE(vict, i * PULSE_VIOLENCE/2);
	break;
    case 11:
	do_say(ch, "Ya uuuuu aaaaa    ..", 0);
	act("$n throw powerfull punch ! .", 1, ch, 0, 0, TO_ROOM);
	/* damage(ch, vict, 80, SKILL_KICK); */
	do_punch(ch, GET_NAME(vict), 0);
	break;
    case 12:
    case 13:
	do_say(ch, "Yak Yak Yak Yak Ya..", 0);
	act("$n use thousands  kick .", 1, ch, 0, 0, TO_ROOM);
	    do_multi_kick(ch, GET_NAME(ch->specials.fighting), 0);
	    if (ch->specials.fighting && number(0,9) < 7 )
		do_multi_kick(ch, GET_NAME(ch->specials.fighting), 0);
	    if (ch->specials.fighting && number(0,9) < 3 )
		do_multi_kick(ch, GET_NAME(ch->specials.fighting), 0);
	break;
    case 14:
    case 15:
	if (!affected_by_spell(vict, SPELL_CRUSH_ARMOR)) {
	    do_say(ch, "Crush Armor    ..", 0);
	    spell_crush_armor(GET_LEVEL(ch)+10, ch, vict, 0 );
	    send_to_char("You feel shrink.\r\n", vict);
	}
	else
	    spell_chill_touch(GET_LEVEL(ch), ch, vict, 0 );
	break;
    case 16:
    case 17:
	do_shouryuken(ch, GET_NAME(vict), 0);
	break;
    case 18:
    case 19:
    case 20:
    case 21:
	do_say(ch, "Shou Ryu Ken..", 0);
	dam =  vict->points.hit / 3
		- number(1, vict->points.hit / 8 + GET_LEVEL(vict) / 2);
	damage(ch, vict, dam , TYPE_HIT);
	send_to_char("You are falling down.\r\n", vict);
	send_to_char("Quuu aaaa rrrrrrrr . .  .  . \r\n", vict);
	break;
    default:
	return (FALSE);
    }
    return (TRUE);
}

int super_musashi(struct char_data *ch, int cmd, char *arg)
{
    struct char_data *choose_victim(struct char_data *mob, int fightmode,
				    int mode);
    struct char_data *vict, *next_vict;
    struct affected_type af;
    int i, musash_mod = 0;

    if (cmd)
	return (0);

    /* cyb  vict=world[ch->in_room].people; */
    vict = choose_victim(ch, VIC_ALL, MODE_HIGH_LEVEL);

    if (GET_HIT(ch) < GET_MAX_HIT(ch)/2) {
	cast_full_heal(GET_LEVEL(ch), ch, "", SPELL_TYPE_SPELL, ch, 0);
	cast_full_heal(GET_LEVEL(ch), ch, "", SPELL_TYPE_SPELL, ch, 0);
	musash_mod = 1;
    }
    if (GET_HIT(ch) < 100000) {
	cast_full_heal(GET_LEVEL(ch), ch, "", SPELL_TYPE_SPELL, ch, 0);
	musash_mod = 1;
    }

    if (ch->specials.fighting) {
	for (; vict; vict = next_vict) {
	    next_vict = vict->next_in_room;
	    if (vict->specials.fighting == ch) {
		hit(ch, vict, TYPE_UNDEFINED);
		hit(ch, vict, TYPE_UNDEFINED);
		musash_mod = 1;
	    }
	}
	vict = ch->specials.fighting;
	if (!vict)
	    return (1);
	if (number(1, 2) == 1) {
	    damage(ch, vict, 10 * GET_LEVEL(ch), SKILL_KICK);
	    musash_mod = 1;
	}
	else {
	    damage(ch, vict, 10 * GET_LEVEL(ch), SKILL_BASH);
	    musash_mod = 1;
	}
	if (ch->specials.fighting && !affected_by_spell(ch, SPELL_SANCTUARY)) {
	    act("$n 주위로 휘황한 광채가  감쌉니다.", TRUE, ch, 0, 0, TO_ROOM);
	    af.type = SPELL_SANCTUARY;
	    af.duration = 30;
	    af.modifier = 0;
	    af.location = APPLY_NONE;
	    af.bitvector = AFF_SANCTUARY;
	    affect_to_char(ch, &af);
	    return (1);
	}
	vict = ch->specials.fighting;
	if (!vict)
	    return (1);
	switch (number(0, 18)) {
	case 0:
	    act("$n utters the words 'fire'.", 1, ch, 0, 0, TO_ROOM);
	    for (vict = ch->specials.fighting; vict; vict = next_vict) {
		if (vict->in_room != ch->in_room)
		    break;
		next_vict = vict->next_in_room;
		if (vict->specials.fighting == ch) {
		    cast_sunburst(GET_LEVEL(ch), ch, "", SPELL_TYPE_SPELL, vict, 0);
		}
	    }
	    return (1);
	case 1:
	    act("$n utters the words 'frost'.", 1, ch, 0, 0, TO_ROOM);
	    for (vict = ch->specials.fighting; vict; vict = next_vict) {
		if (vict->in_room != ch->in_room)
		    break;
		next_vict = vict->next_in_room;
		if (vict->specials.fighting == ch) {
		    cast_cone_of_ice(GET_LEVEL(ch), ch, "", SPELL_TYPE_SPELL, vict, 0);
		}
	    }
	    return (1);
	case 2:
	case 3:
	    act("$n double kick by double circle kick .", 1, ch, 0, 0, TO_ROOM);
	    damage(ch, vict, 20 * GET_LEVEL(ch), SKILL_KICK);
	    damage(ch, vict, 20 * GET_LEVEL(ch), SKILL_KICK);
	    return (1);
	case 4:
	case 5:
	    do_say(ch, "A cha cha cha cha ..", 0);
	    act("$n tornado fire with miracle speed .", 1, ch, 0, 0, TO_ROOM);
	    for (i = 0; i < number(10, 20); i++)
		cast_sunburst(GET_LEVEL(ch), ch, "", SPELL_TYPE_SPELL, vict, 0);
	    return (1);
	case 6:
	    do_say(ch, "Yak Yak Yak Yak Ya..", 0);
	    act("$n use thousands  kick .", 1, ch, 0, 0, TO_ROOM);
	    for (i = 0; i < number(10, 20); i++)
		damage(ch, vict, 10 * GET_LEVEL(ch), SKILL_KICK);
	    return (1);
	case 7:
	    do_say(ch, "Heau Heau Heau Heau Heau..", 0);
	    act("$n use hundreds bash .", 1, ch, 0, 0, TO_ROOM);
	    for (i = 0; i < number(10, 20); i++)
		damage(ch, vict, 10 * GET_LEVEL(ch), SKILL_BASH);
	    return (1);
	case 8:
	    do_say(ch, "Ya uuuuu aaaaa    ..", 0);
	    act("$n throw powerfull punch ! .", 1, ch, 0, 0, TO_ROOM);
	    damage(ch, vict, GET_HIT(vict) / 3, SKILL_BASH);
	    return (1);
	case 9:
	case 10:
	case 11:
	case 12:
	    if (!vict)
		break;
	    if (!affected_by_spell(vict, SPELL_CRUSH_ARMOR)) {
		do_say(ch, "Crush Armor    ..", 0);
		send_to_char("You feel shrink.\r\n", vict);

		af.type = SPELL_CRUSH_ARMOR;
		af.duration = 20;
		af.modifier = GET_LEVEL(ch) / 4 * (-1);
		af.location = APPLY_DAMROLL;
		af.bitvector = 0;
		affect_to_char(vict, &af);
		af.location = APPLY_AC;
		af.modifier = GET_LEVEL(ch) + 20;
		affect_to_char(vict, &af);
		return (1);
	    }
	    /*FALLTHRU*/
	case 13:
	case 14:
	case 15:
	case 16:
	case 17:
	case 18:
	    do_say(ch, "Shou Ryu Ken..", 0);
	    damage(ch, vict, GET_HIT(vict) / 3 - number(1, GET_HIT(vict) / 15
					+ GET_LEVEL(vict) / 2), SKILL_BASH);
	    send_to_char("You are falling down.\r\n", vict);
	    send_to_char("Quuu aaaa rrrrrrrr . .  .  . \r\n", vict);
	    return (1);
	}
	return (1);
    }
    if (musash_mod == 1)
	return (1);

    /* high level victim is already selected */
    if (vict && number(0, LEVEL_LIMIT - GET_LEVEL(vict)) < 4) {
	send_to_char("I CHALLENGE YOU.\r\n", vict);
	hit(ch, vict, TYPE_UNDEFINED);
	return (1);
    }
    return (FALSE);
}

int mom(struct char_data *ch, int cmd, char *arg)
{
    struct char_data *vict;
    int h;
    extern void shoot(struct char_data *ch, struct char_data *victim, int type);

    if (cmd)
	return (0);
    vict = ch->specials.fighting;
    if (vict) {
	if ((GET_MOVE(vict) > 50)) {	/* cyb : reduce move point  */
	    send_to_char("으라차아  !\r\n", vict);
	    GET_MOVE(vict) -= dice(10, 10);
	}
	h = GET_HIT(ch);
	if ((h > 100) && (h < 999)) {
	    act("$n 이 크고 푸른 알약을 삼킵니다. (윙크)", 1, ch, 0, 0, TO_ROOM);
	    cast_heal(GET_LEVEL(ch), ch, "", SPELL_TYPE_SPELL, ch, 0);
	    return TRUE;
	}
	else {
	    if (ch->in_room == vict->in_room) {
		shoot(ch, vict, TYPE_SHOOT);
	    }
	}
    }
    switch (number(0, 50)) {
    case 0:
	do_say(ch, "오늘 할 일을 내일로 미루지 마라.", 0);
	return (1);
    case 1:
	do_say(ch, "집에서 너무 멀리 가지 말아라.", 0);
	return (1);
    case 2:
	do_say(ch, "일찍 자고 일찍 일어나라.", 0);
	return (1);
    case 3:
	do_say(ch, "얘들아 잘시간이다.", 0);
	return (1);
    case 4:
	do_say(ch, "누가 여기 쓰레기 치우지 않았지 ?", 0);
	return (1);
    case 5:
	do_say(ch, "얘 ! 방좀 가서 치워라.", 0);
	return (1);
    case 6:
	do_say(ch, "밥먹기 전에는 손을 씻어야지.", 0);
	return (1);
    case 7:
	do_say(ch, "밤늦게 까지 오락 하지 말랬지 !", 0);
	return (1);
    case 8:
	do_say(ch, "넌 집에서 가정교육을 좀 더 받아야해", 0);
	return (1);
    case 9:
	do_say(ch, "보고서 다 썼니 ?", 0);
	return (1);
    case 10:
	do_say(ch, "숙제 다 했어 ?", 0);
	return (1);
    default:
	return (0);
    }
}

static char *song[] =
{
    "$n sings, '...lights flicker from the opposite loft'",
    "$n sings, 'in this room the heat pipes just coughed'",
    "$n sings, 'the country music station plays soft'",
    "$n sings, 'but there's nothing, really nothing, to turn off...'"
};

int singer(struct char_data *ch, int cmd, char *arg)
{
    static int n = 0;

    if (cmd)
	return FALSE;
    if (GET_POS(ch) != POS_STANDING)
	return FALSE;
    if (n < 4)
	act(song[n], TRUE, ch, 0, 0, TO_ROOM);
    if ((++n) == 16)
	n = 0;
    return TRUE;
}

int fido(struct char_data *ch, int cmd, char *arg)
{
    struct obj_data *i, *temp, *next_obj;

    if (cmd || !AWAKE(ch))
	return (FALSE);

    for (i = world[ch->in_room].contents; i; i = i->next_content) {
	if (GET_ITEM_TYPE(i) == ITEM_CONTAINER && i->obj_flags.value[3]) {
	    act("$n savagely devour a corpse.", FALSE, ch, 0, 0, TO_ROOM);
	    for (temp = i->contains; temp; temp = next_obj) {
		next_obj = temp->next_content;
		obj_from_obj(temp);
		obj_to_room(temp, ch->in_room);
	    }
	    extract_obj(i);
	    return (TRUE);
	}
    }
    return (FALSE);
}

int janitor(struct char_data *ch, int cmd, char *arg)
{
    struct obj_data *i;

    if (cmd || !AWAKE(ch))
	return (FALSE);

    for (i = world[ch->in_room].contents; i; i = i->next_content) {
	if (IS_SET(i->obj_flags.wear_flags, ITEM_TAKE) &&
	    ((i->obj_flags.type_flag == ITEM_DRINKCON) ||
	     (i->obj_flags.cost <= 10))) {
	    act("$n picks up some trash.", FALSE, ch, 0, 0, TO_ROOM);
	    obj_from_room(i);
	    obj_to_char(i, ch);
	    return (TRUE);
	}
    }
    return (FALSE);
}

int snake(struct char_data *ch, int cmd, char *arg)
{
    if (cmd)
	return FALSE;
    if (GET_POS(ch) != POS_FIGHTING)
	return FALSE;
    if (ch->specials.fighting &&
	(ch->specials.fighting->in_room == ch->in_room) &&
	(number(0, 32 - GET_LEVEL(ch)) == 0)) {
	acthan("$n bites $N!", "$n님이 $N님의 다리를 물었습니다!",
	       1, ch, 0, ch->specials.fighting, TO_NOTVICT);
	acthan("$n bites you!", "$n님에게 다리를 물렸습니다!",
	       1, ch, 0, ch->specials.fighting, TO_VICT);
	cast_poison(GET_LEVEL(ch), ch, "", SPELL_TYPE_SPELL,
		    ch->specials.fighting, 0);
	return TRUE;
    }
    return FALSE;
}

int archmage(struct char_data *ch, int cmd, char *arg)
{
    struct char_data *vict;
    void do_shout(struct char_data *ch, char *argument, int cmd);

    if (cmd)
	return FALSE;
    if (!ch->specials.fighting)
	return FALSE;
    do_shout(ch, "SUNFIRE", 0);
    act("The outside sun flares brightly, flooding the room with searing rays.",
	TRUE, ch, 0, 0, TO_ROOM);
    for (vict = world[ch->in_room].people; vict; vict = vict->next_in_room) {
	if (vict->specials.fighting == ch) {
	    if (IS_NPC(vict))
		damage(ch, vict, dice(100, 2), TYPE_MISC);
	}
    }
    return TRUE;
}

