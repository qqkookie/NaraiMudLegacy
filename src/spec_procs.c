
/* ************************************************************************
*  file: spec_procs.c , Special module.                   Part of DIKUMUD *
*  Usage: Procedures handling special procedures for object/room/mobile   *
*  Copyright (C) 1990, 1991 - see 'license.doc' for complete information. *
************************************************************************* */

#define FUDGE (100+dice(6,20))

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
#include "limits.h"
#include "command.h"
#include "quest.h"

/*   external vars  */

extern struct room_data *world;
extern struct char_data *character_list;
extern struct descriptor_data *descriptor_list;
extern struct index_data *obj_index;
extern struct time_info_data time_info;
extern struct title_type titles[4][IMO+4];
extern struct index_data *mob_index;

/* extern procedures */

void hit(struct char_data *ch, struct char_data *victim, int type);
void gain_exp(struct char_data *ch, int gain);
void advance_level(struct char_data *ch);
void stop_fighting(struct char_data *ch);
void set_title(struct char_data *ch);
char *string_dup(char *source);

void cast_cure_light(byte level,struct char_data *ch,char *arg,int type,
                     struct char_data *victim, struct obj_data *tar_obj );
void cast_cure_critic(byte level,struct char_data *ch,char *arg,int type,
                      struct char_data *victim, struct obj_data *tar_obj );
void cast_heal(byte level, struct char_data *ch, char *arg, int type,
               struct char_data *tar_ch, struct obj_data *tar_obj );
void cast_full_heal(byte level, struct char_data *ch, char *arg, int type,
                    struct char_data *tar_ch, struct obj_data *tar_obj );
void cast_sunburst(byte level, struct char_data *ch, char *arg, int type,
                   struct char_data *victim, struct obj_data *tar_obj );
void cast_fireball(byte level, struct char_data *ch, char *arg, int type,
                   struct char_data *victim, struct obj_data *tar_obj );
void cast_colour_spray(byte level, struct char_data *ch, char *arg, int type,
                       struct char_data *victim, struct obj_data *tar_obj );

char *how_good(int percent)
{
    static char buf[64];

    sprintf(buf,"(%d)",percent);
    return (buf);
}

int guild(struct char_data *ch, int cmd, char *arg)
{
    char arg1[MAX_STRING_LENGTH];
    char buf[MAX_STRING_LENGTH], buf2[MAX_STRING_LENGTH];
    char prbuf[80*MAX_SKILLS];
    int number,i,percent,count;
    int lev,cla;
    extern char *spells[];
    extern struct spell_info_type spell_info[MAX_SPL_LIST];
    extern struct int_app_type int_app[26];

    if ((cmd!=COM_ADVANCE)&&(cmd!=COM_PRACTICE)&&(cmd!=COM_PRACTISE))
		return(FALSE);

    if(cmd==COM_ADVANCE){ /* advance */
        if (!IS_NPC(ch)) {
            for (i = 0; titles[GET_CLASS(ch)-1][i].exp <= GET_EXP(ch); i++) {
                if(i >= IMO){
                    send_to_char_han("Immortality cannot be gained here.\n\r",
                                     "여기서는 신이 될 수 없습니다.\n\r",ch);
                    return(TRUE);
                }
                if (i > GET_LEVEL(ch)) {
                    sprintf(buf, "<INFO> : %s level up to %d !!!",
                            ch->player.name, i);
                    log(buf);

                    send_to_char_han("You raise a level\n\r",
                                     "레벨을 올렸습니다\n\r", ch);
                    GET_LEVEL(ch) = i; advance_level(ch);
#ifndef DEATHFIGHT
                    set_title(ch);
#endif 
                    strcat(buf, "\n\r");
                    send_to_except(buf, ch);
                    return(TRUE);
                }
            }
            send_to_char_han("You need more experience.\n\r",
                             "경험이 더 필요합니다.\n\r",ch);
            return(TRUE);
        }
    }

    /* practice */
    lev=GET_LEVEL(ch); cla=GET_CLASS(ch)-1;
    for(; *arg==' '; arg++);
    if (!*arg) {
        if (IS_SET(ch->specials.act, PLR_KOREAN))
            strcpy(prbuf, "당신은 다음과 같은 기술을 익힐 수 있습니다:\n\r");
        else
            strcpy(prbuf, "You can practice any of these skills:\n\r") ;
/*
  send_to_char_han("You can practice any of these skills:\n\r",
  "당신은 다음과 같은 기술을 익힐 수 있습니다:\n\r", ch);
*/
        for(i=0, count=0; *spells[i] != '\n'; i++){
            if(*spells[i] &&
               (spell_info[i+1].min_level[cla] <= lev)) {
/*
  send_to_char(spells[i], ch);
  send_to_char(how_good(ch->skills[i+1].learned), ch);
  send_to_char("\n\r", ch);
*/
                sprintf(buf, "%s%s", spells[i], how_good(ch->skills[i+1].learned)) ;
                if (count%2 == 0)
                    sprintf(buf2, "%-35s", buf) ;
                else
                    sprintf(buf2, "%s\n\r", buf) ;
                strcat(prbuf, buf2);
                count++;
            }
        }
        strcat(prbuf, "\n\r") ;
        sprintf(buf,"You have %d practices left.\n\r",
                ch->specials.spells_to_learn);
        sprintf(buf2,"지금 %d 번 기술을 연마(practice)할 수 있습니다. \n\r",
                ch->specials.spells_to_learn);
/*
  send_to_char_han(buf,buf2,ch);
*/
        if (IS_SET(ch->specials.act, PLR_KOREAN))
            strcat(prbuf, buf2) ;
        else
            strcat(prbuf, buf) ;
        page_string(ch->desc, prbuf, 1);
        return(TRUE);
    }
    number = old_search_block(arg,0,strlen(arg),spells,FALSE);
    if(number == -1) {
        send_to_char_han("You do not know of this spell...\n\r",
                         "그런 기술은 모르는데요 ...\n\r", ch);
        return(TRUE);
    }
    if (lev < spell_info[number].min_level[cla]) {
        send_to_char_han("Your level is too low.\n\r",
                         "아직은 레벨이 낮아 안됩니다...\n\r",ch);
        return(TRUE);
    }
    if (ch->specials.spells_to_learn <= 0) {
        send_to_char_han("You do not seem to be able to practice now.\n\r",
                         "지금은 더이상 배울 수 없습니다.\n\r", ch);
        return(TRUE);
    }
    if(ch->skills[number].learned >= spell_info[number].max_skill[cla]){
        send_to_char_han("You know this area as well as possible.\n\r",
                         "그 분야는 배울 수 있는만큼 배웠습니다.\n\r",ch);
        return(TRUE);
    }
    send_to_char_han("You Practice for a while...\n\r",
                     "기술이 늘고 있습니다...\n\r", ch);
    ch->specials.spells_to_learn--;
    percent=ch->skills[number].learned+1+
        ((int)int_app[GET_INT(ch)].learn
         *(int)spell_info[number].max_skill[cla])/FUDGE;
    ch->skills[number].learned=
        MIN(spell_info[number].max_skill[cla],percent);
    if(ch->skills[number].learned >= spell_info[number].max_skill[cla]){
        send_to_char_han("You're now as proficient as possible.\n\r",
                         "이 분야에는 이미 배울만큼 다 배웠습니다.\n\r",ch);
        return(TRUE);
    }
}


int dump(struct char_data *ch, int cmd, char *arg) 
{
    struct obj_data *k;
    char buf[100];
    struct char_data *tmp_char;
    int value=0, x;
	int	trashquest, nmessages ;

    void do_drop(struct char_data *ch, char *argument, int cmd);
    char *fname(char *namelist);

	/* do not apply message limit at this.. */
    for(k = world[ch->in_room].contents; k ; k = world[ch->in_room].contents) {
        sprintf(buf, "The %s vanish in a puff of smoke.\n\r" ,fname(k->name));
        for(tmp_char = world[ch->in_room].people; tmp_char;
            tmp_char = tmp_char->next_in_room)
            if (CAN_SEE_OBJ(tmp_char, k))
                send_to_char(buf,tmp_char);
        extract_obj(k);
    }
    if(cmd != COM_DROP) return(FALSE);
    do_drop(ch, arg, cmd);
    value = 0;
	trashquest = 0 ;
	nmessages = 12 ;
    for(k=world[ch->in_room].contents; k ; k = world[ch->in_room].contents) {
		if (GET_ITEM_TYPE(k) == ITEM_TRASH) trashquest ++ ;
		if ( nmessages > 0 ) {
			sprintf(buf, "The %s vanish in a puff of smoke.\n\r",fname(k->name));
			nmessages -- ;
			for(tmp_char = world[ch->in_room].people; tmp_char;
				tmp_char = tmp_char->next_in_room)
				if (CAN_SEE_OBJ(tmp_char, k))
					send_to_char(buf,tmp_char);
        }
        value += (k->obj_flags.cost/2 - 10) ;
        extract_obj(k);
    }
	if ( nmessages <= 0 ) {
		sprintf(buf, "  ...\n\rA lot of items vasish in a puff of smoke.\n\r") ;
		send_to_char(buf, ch) ;
		act(buf, FALSE, ch, 0, 0, TO_ROOM) ;
    }

    if (value > 0) 
    {
        acthan("You are awarded for outstanding performance.",
               "당신은 시장으로부터 선행상을 받았습니다.",  FALSE, ch, 0, 0, TO_CHAR);
        acthan("$n has been awarded for being a good citizen.",
               "$n님이 착한 시민상을 받았습니다", TRUE, ch, 0,0, TO_ROOM);

        if ( value > 1000 ) {
            x = value -1000 ;
            value = 1000 ;
            while ( x > 2 ) {
                value += 100 ;
                x = x / 2 ;
            }
        }
        if (GET_LEVEL(ch) < 3 && (GET_EXP(ch) < GET_LEVEL(ch)*2000) )
            gain_exp(ch, value);
        else
            GET_GOLD(ch) += value;
    }

	if ( trashquest && !IS_NPC(ch) && !is_solved_quest(ch, QUEST_TRASH)) {
		sprintf(buf, "%s solved TRASH QUEST", GET_NAME(ch)) ;
		log(buf) ;
		send_to_char_han("You knew that ! You solved TRASH Quest\n\r",
                         "알고 있었군요 ! 당신은 쓰레기 문제를 풀었습니다.\n\r", ch) ;
		set_solved_quest(ch, QUEST_TRASH) ;
		/* Strength bonus */
		if ( ch->abilities.str < 18 ) {
			ch->abilities.str ++ ;
        }
		else if ( ch->abilities.str_add < 90 ) {
			ch->abilities.str_add += 10 ;
        }
		else {
			ch->abilities.str_add = 100 ;
        }
		affect_total(ch) ;
		send_to_char_han("Your strength increased !\n\r",
                         "당신의 힘이 증가 하였습니다.\n\r", ch) ;
		save_char(ch, NOWHERE) ;
    }

	return TRUE ;
}

int mayor(struct char_data *ch, int cmd, char *arg)
{
    static char open_path[] =
        "W3a3003b33000c111d0d111Oe333333Oe22c222112212111a1S.";

    static char close_path[] =
        "W3a3003b33000c111d0d111CE333333CE22c222112212111a1S.";

    static char *path;
    static int index;
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
        } else if (time_info.hours == 20) {
            move = TRUE;
            path = close_path;
            index = 0;
        }
    }

    if (cmd || !move || (GET_POS(ch) < POSITION_SLEEPING) ||
        (GET_POS(ch) == POSITION_FIGHTING))
        return FALSE;

    switch (path[index]) {
    case '0' :
    case '1' :
    case '2' :
    case '3' :
        do_move(ch,"",path[index]-'0'+1);
        break;

    case 'W' :
        GET_POS(ch) = POSITION_STANDING;
        acthan("$n awakens and groans loudly.",
               "$n님이 일어나 기지개를 켜십니다", FALSE,ch,0,0,TO_ROOM);
        break;

    case 'S' :
        GET_POS(ch) = POSITION_SLEEPING;
        acthan("$n lies down and instantly falls asleep.",
               "$n님이 자리에 눕더니 금새 잠이 듭니다.", FALSE,ch,0,0,TO_ROOM);
        break;

    case 'a' :
        acthan("$n says 'Hello Honey!'",
               "$n님이 '안녕 내사랑!' 이라고 말합니다.", FALSE,ch,0,0,TO_ROOM);
        acthan("$n smirks.","$n님이 점잔빼며 웃습니다.", FALSE,ch,0,0,TO_ROOM);
        break;

    case 'b' :
        acthan("$n says 'What a view! I must get something done about dump!'",
               "$n님이 '하! 쓰레기에 대해서 뭔가 조치를 취해야 겠네!' 라고 말합니다",
               FALSE,ch,0,0,TO_ROOM);
        break;

    case 'c' :
        acthan("$n says 'Vandals! Youngsters nowadays have no respect for anything!'",
               "$n님이 '이런 이런! 젊은것들은 요즘 인사를 안한단말야!' 라고 말합니다.",
               FALSE,ch,0,0,TO_ROOM);
        break;

    case 'd' :
        acthan("$n says 'Good day, citizens!'",
               "$n님이 '시민 여러분 안녕들 하십니까!'라고 말합니다",  FALSE, ch, 0,0,TO_ROOM);
        break;

    case 'e' :
        acthan("$n says 'I hereby declare the bazaar open!'",
               "$n님이 '자 이제 시장을 열겠습니다 !' 라고 말합니다", FALSE,ch,0,0,TO_ROOM);
        break;

    case 'E' :
        acthan("$n says 'I hereby declare Midgaard closed!'",
               "$n님이 '자 이제 문을 닫았습니다!' 라고 말합니다", FALSE,ch,0,0,TO_ROOM);
        break;

    case 'O' :
        do_unlock(ch, "gate", 0);
        do_open(ch, "gate", 0);
        break;

    case 'C' :
        do_close(ch, "gate", 0);
        do_lock(ch, "gate", 0);
        break;

    case '.' :
        move = FALSE;
        break;
    }
    index++;
    return FALSE;
}

void npc_steal(struct char_data *ch,struct char_data *victim)
{
    int dir,gold;
	extern long jackpot;

    if(IS_NPC(victim)) return;
    if(GET_LEVEL(victim) >= IMO) return;
    if (AWAKE(victim) && (number(0,GET_LEVEL(ch)) == 0)) {
        acthan("You discover that $n has $s hands in your wallet.",
               "앗! $n님이 당신의 지갑에 손을 넣습니다.",
               FALSE,ch,0,victim,TO_VICT);
        acthan("$n tries to steal gold from $N.",
               "$n님이 $N님으로부터 돈을 훔치려고 합니다.",TRUE,ch,0,victim,TO_NOTVICT);
    } else {
        /* Steal some gold coins */
        acthan("$n suppresses a laugh.","$n님이 키득키득 웃습니다.(왜그럴까?)",
               TRUE,ch,0,0,TO_NOTVICT);
        gold =(int)((GET_GOLD(victim)*number(1,10))/25);
        if (gold > 0) {
            GET_GOLD(ch) += gold/2;
            GET_GOLD(victim) -= gold;

			// jackpot에다가 훔친 돈을 더하도록...
			jackpot += (long)gold/3;

            dir=number(0,5);
            if(CAN_GO(ch,dir) &&
				!IS_SET(world[EXIT(ch, dir)->to_room].room_flags, NO_MOB))
                do_simple_move(ch,dir,FALSE);
        }
    }
}

int snake(struct char_data *ch, int cmd, char *arg)
{
    void cast_poison( byte level, struct char_data *ch, char *arg, int type,
                      struct char_data *tar_ch, struct obj_data *tar_obj );

    if(cmd) return FALSE;
    if(GET_POS(ch)!=POSITION_FIGHTING) return FALSE;
    if(ch->specials.fighting && 
       (ch->specials.fighting->in_room == ch->in_room) &&
       (number(0,32-GET_LEVEL(ch))==0))
    {
        acthan("$n bites $N!", "$n님이 $N님의 다리를 물었습니다!",
               1, ch, 0, ch->specials.fighting, TO_NOTVICT);
        acthan("$n bites you!", "$n님에게 다리를 물렸습니다!",
               1, ch, 0, ch->specials.fighting, TO_VICT);
        cast_poison( GET_LEVEL(ch), ch, "", SPELL_TYPE_SPELL,
                     ch->specials.fighting, 0);
        return TRUE;
    }
    return FALSE;
}

int thief(struct char_data *ch, int cmd, char *arg)
{
    struct char_data *cons;

    if(cmd) return FALSE;
    if(GET_POS(ch)==POSITION_FIGHTING ) {
        if (number(1,4) > 1) {
            acthan("$n says 'Ya haa haa hap'.", "$n님이 '야하햐합' 하고 말합니다",
                   1, ch, 0, 0, TO_ROOM);
            for(cons = world[ch->in_room].people; cons; cons = cons->next_in_room ) {
                if ( number(10,IMO+3) < GET_LEVEL(ch) && GET_LEVEL(cons) < IMO)
                    if (!IS_NPC(cons) && ch!=cons)
                        hit(ch,cons,TYPE_UNDEFINED) ;	/* It is tornado */
            }
        }
        else if ( GET_LEVEL(ch) > 10 &&(GET_HIT(ch) * 100/GET_MAX_HIT(ch))< 47) {
            acthan("$n says 'Quu ha cha cha'.", "$n님이 '크하차차' 하고 외칩니다",
                   1, ch, 0, 0, TO_ROOM);
            do_flash(ch, "", 0);
        }
    }
    else if (GET_POS(ch)!=POSITION_STANDING) return FALSE;
    else {
        for(cons = world[ch->in_room].people; cons; cons = cons->next_in_room ) {
            if((!IS_NPC(cons)) && (GET_LEVEL(cons)<IMO) &&
               (GET_LEVEL(cons)>=GET_LEVEL(ch)) && (number(1,3)==1))
                npc_steal(ch,cons); 
        }
    }
    return TRUE;
}

int cleric(struct char_data *ch, int cmd, char *arg)
{
    struct char_data *vict, *v2;
    int	nfight;

    if(cmd) return FALSE;
    if(GET_POS(ch)!=POSITION_FIGHTING){
        return FALSE;
    }
    if(!ch->specials.fighting) return FALSE;
    for(vict=world[ch->in_room].people; vict; vict = vict->next_in_room )
        if(vict->specials.fighting==ch && number(1,2)==1)
            break;
    switch (GET_LEVEL(ch)) {
    case 1:
    case 2:
    case 3:
    case 4:
    case 5:
    case 6:
    case 7:
    case 8:
    case 9:
    case 10:
    case 11:
        acthan("$n utters the words 'asdghjkasdgi'.",
               "$n님이 '마디해체자' 라고 주문을 욉니다", 1, ch, 0, 0, TO_ROOM);
        cast_cure_light(GET_LEVEL(ch), ch, "", SPELL_TYPE_SPELL, ch, 0);
        break;
    case 12:
    case 13:
    case 14:
    case 15:
    case 16:
    case 17:
    case 18:
    case 19:
        acthan("$n utters the words 'saghkasdlghui'.",
               "$n님이 '마디후지이' 라고 주문을 욉니다",1,ch,0,0,TO_ROOM);
        cast_cure_critic(GET_LEVEL(ch), ch,"",SPELL_TYPE_SPELL,ch,0);
        break;
    case 20:
    case 21:
    case 22:
    case 23:
    case 24:
    case 25:
    case 26:
    case 27:
        acthan("$n utters the words 'heal'.",
               "$n님이 '힐' 이라고 주문을 욉니다",  1, ch, 0, 0, TO_ROOM);
        cast_heal(GET_LEVEL(ch), ch, "", SPELL_TYPE_SPELL, ch, 0);
        break;
    case 28:
    case 29:
    case 30:
    default:
        acthan("$n utters the words 'sdagh'.",
               "$n님이 '푸힐' 이라고 주문을 욉니다", 1, ch, 0, 0, TO_ROOM);
        cast_full_heal(GET_LEVEL(ch), ch, "", SPELL_TYPE_SPELL, ch, 0);
        break;
    }
    if (!vict)
        return FALSE;
    if(number(1,2)==1 && GET_LEVEL(ch)>21){
        acthan("$n utters the words 'oliel ese'.",
               "$n님이 '오디엘 에세' 라고 주문을 욉니다", 1, ch, 0, 0, TO_ROOM);
        cast_sunburst(GET_LEVEL(ch),ch,"",SPELL_TYPE_SPELL,vict,0);
    }
    if(!IS_EVIL(ch) && !IS_GOOD(vict)  && number(1,2)==1 ){
        acthan("$n utters the words 'fidprl rbil'.",
               "$n님이 '피이어 불' 이라고 주문을 욉니다", 1, ch, 0, 0, TO_ROOM);
        if (GET_LEVEL(ch) > 10 )
            spell_dispel_evil(GET_LEVEL(ch),ch,vict,0);
    }
    if ( number(1,3)==1) {
        nfight=0;
        for(v2=world[ch->in_room].people; v2; v2 = v2->next_in_room )
            if(v2->specials.fighting==ch) nfight++;
        if(nfight >=4)
            mob_spell_fire_storm(GET_LEVEL(ch), ch, 0, 0);
    }

    return TRUE;
}


int paladin(struct char_data *ch, int cmd, char *arg)
{
    struct char_data *vict;
    void cast_paralysis(byte level, struct char_data *ch, char *arg, int type,
                        struct char_data *tar_ch, struct obj_data *tar_obj );

    if(cmd) return FALSE;
    if(GET_POS(ch)!=POSITION_FIGHTING){
        return FALSE;
    }
    if(!ch->specials.fighting) return FALSE;
    for(vict=world[ch->in_room].people; vict; vict = vict->next_in_room )
        if(vict->specials.fighting==ch && number(0,2)==0)
            break;
    if(number(1,3)==1)
        switch (GET_LEVEL(ch)) {
        case 1:
        case 2:
        case 3:
        case 4:
        case 5:
        case 6:
        case 7:
        case 8:
        case 9:
        case 10:
        case 11:
        case 12:
        case 13:
            act("$n utters the words 'asdghjkasdgi'.", 1, ch, 0, 0, TO_ROOM);
            cast_cure_light(GET_LEVEL(ch), ch, "", SPELL_TYPE_SPELL, ch, 0);
            break;
        case 14:
        case 15:
        case 16:
        case 17:
        case 18:
        case 19:
        case 20:
            act("$n utters the words 'saghkasdlghui'.",1,ch,0,0,TO_ROOM);
            cast_cure_critic(GET_LEVEL(ch), ch,"",SPELL_TYPE_SPELL,ch,0);
            break;
        case 21:
        case 22:
        case 23:
        case 24:
        case 25:
            act("$n utters the words 'heal'.", 1, ch, 0, 0, TO_ROOM);
            cast_heal(GET_LEVEL(ch), ch, "", SPELL_TYPE_SPELL, ch, 0);
            break;
        case 26:
        case 27:
        case 28:
        case 29:
        case 30:
        case 31:
        case 32:
        case 33:
        case 34:
        default:
            act("$n utters the words 'sdagh'.", 1, ch, 0, 0, TO_ROOM);
            cast_full_heal(GET_LEVEL(ch), ch, "", SPELL_TYPE_SPELL, ch, 0);
            break;
        }
    if (!vict)
        return FALSE;
    if (number(1,3)==1 && number(1,GET_LEVEL(ch)) > 30) {
        do_multi_kick(ch, "", 0) ;
    }
	if ( vict == NULL )
		return TRUE ;
    if(number(1,3)==1){
        if(GET_LEVEL(ch)>20 && GET_LEVEL(ch)<26){
            act("$n utters the words 'dsagjlse'.", 1, ch, 0, 0, TO_ROOM);
            cast_colour_spray(GET_LEVEL(ch),ch,"",SPELL_TYPE_SPELL,vict,0);
        }
        else if(GET_LEVEL(ch)>25 && GET_LEVEL(ch) <= 30){
            act("$n utters the words 'ddsaghjkse'.", 1, ch, 0, 0, TO_ROOM);
            cast_fireball(GET_LEVEL(ch),ch,"",SPELL_TYPE_SPELL,vict,0);
        }
        else if(GET_LEVEL(ch)>30){
            act("$n utters the words 'oliel ese'.", 1, ch, 0, 0, TO_ROOM);
            cast_sunburst(GET_LEVEL(ch),ch,"",SPELL_TYPE_SPELL,vict,0);
        }
		if ( vict && number(1,2) >= 2) {
			act("$n utters the words 'ldt ssiovkets'.", 1, ch, 0, 0, TO_ROOM);
			spell_paralysis(GET_LEVEL(ch),ch,vict,0);
		}
    }
    return TRUE;
}

int magic_user(struct char_data *ch, int cmd, char *arg)
{
    struct char_data *vict;
    void cast_magic_missile(byte level,struct char_data *ch,char *arg,int type,
                            struct char_data *victim, struct obj_data *tar_obj );
    void cast_burning_hands(byte level,struct char_data *ch,char *arg,int type,
                            struct char_data *victim, struct obj_data *tar_obj );
    void cast_sleep(byte level, struct char_data *ch, char *arg, int type,
                    struct char_data *tar_ch, struct obj_data *tar_obj );
    void cast_blindness(byte level, struct char_data *ch, char *arg, int type,
                        struct char_data *tar_ch, struct obj_data *tar_obj );
    void cast_paralysis(byte level, struct char_data *ch, char *arg, int type,
                        struct char_data *tar_ch, struct obj_data *tar_obj );

    if(cmd) return FALSE;
    if(GET_POS(ch)!=POSITION_FIGHTING){
        if(GET_LEVEL(ch) != 27)   /* Hypnos */
            return(FALSE);
        for(vict=world[ch->in_room].people; vict; vict = vict->next_in_room )
            if((!IS_NPC(vict))&&(GET_LEVEL(vict) < IMO)){
                act("$n utters the words 'Hypnos Thanatos'.",1,ch,0,0,TO_ROOM);
                cast_sleep(GET_LEVEL(ch), ch, "", SPELL_TYPE_SPELL, vict, 0);
                return(TRUE);
            }
        return FALSE;
    }
    if(!ch->specials.fighting) return FALSE;
    for(vict=world[ch->in_room].people; vict; vict = vict->next_in_room )
        if(vict->specials.fighting==ch && number(0,1)==0)
            break;
    if (!vict)
        return FALSE;
    if ( vict && number(1,3) == 1) {
        act("$n utters the words 'sulco kow'.", 1, ch, 0, 0, TO_ROOM);
        spell_curse(GET_LEVEL(ch),ch,vict,0);
    }
    if ( vict && number(1,4) == 1) {
        act("$n utters the words 'nkunf bnrd'.", 1, ch, 0, 0, TO_ROOM);
        spell_blindness(GET_LEVEL(ch),ch,vict,0);
    }
    if ( vict && number(1,2) == 1) {
        act("$n utters the words 'ldt ssiovkets'.", 1, ch, 0, 0, TO_ROOM);
        spell_paralysis(GET_LEVEL(ch),ch,vict,0);
    }

    switch (GET_LEVEL(ch)) {
    case 1:
    case 2:
    case 3:
    case 4:
    case 5:
        act("$n utters the words 'hahili duvini'.", 1, ch, 0, 0, TO_ROOM);
        cast_magic_missile(GET_LEVEL(ch), ch, "", SPELL_TYPE_SPELL, vict, 0);
        break;
    case 6:
    case 7:
    case 8:
    case 9:
    case 10:
        act("$n utters the words 'vivani yatima'.",1,ch,0,0,TO_ROOM);
        cast_burning_hands(GET_LEVEL(ch), ch,"",SPELL_TYPE_SPELL,vict,0);
        break;
    case 11:
    case 12:
    case 13:
    case 14:
    case 15:
    case 16:
    case 17:
        act("$n utters the words 'nasson hof'.", 1, ch, 0, 0, TO_ROOM);
        cast_colour_spray(GET_LEVEL(ch), ch, "", SPELL_TYPE_SPELL, vict, 0);
        break;
    case 18:
    case 19:
    case 20:
    case 21:
    case 22:
    case 23:
    case 24:
        act("$n utters the words 'tubu morg'.", 1, ch, 0, 0, TO_ROOM);
        cast_fireball(GET_LEVEL(ch), ch, "", SPELL_TYPE_SPELL, vict, 0);
        break;
    case 25:
    case 26:
    case 27:
    case 28:
    case 29:
    case 30:
    default:
        act("$n utters the words 'oliel ese'.", 1, ch, 0, 0, TO_ROOM);
        cast_sunburst(GET_LEVEL(ch),ch,"",SPELL_TYPE_SPELL,vict,0);
        break;
    }
    return TRUE;
}
int dragon(struct char_data *ch, int cmd, char *arg)
{
    int mh;
    struct char_data *vict;
    void cast_fire_breath( byte level, struct char_data *ch, char *arg,int type,
                           struct char_data *tar_ch, struct obj_data *tar_obj );
    void cast_frost_breath( byte level, struct char_data *ch, char *arg,int type,
                            struct char_data *tar_ch, struct obj_data *tar_obj );
    void cast_gas_breath(byte level, struct char_data *ch,char *arg,int type,
                         struct char_data *tar_ch, struct obj_data *tar_obj );
    void cast_lightning_breath(byte level,struct char_data *ch,char *arg,
                               int type, struct char_data *tar_ch, struct obj_data *tar_obj );

    if(cmd) return FALSE;
    vict=ch->specials.fighting;
    if(!vict) return FALSE;
    mh=GET_MAX_HIT(vict);
    switch(number(0,4)){
    case 0:
    case 1:
        act("$n utters the words 'qassir plaffa'.", 1, ch, 0, 0, TO_ROOM);
        cast_fire_breath(GET_LEVEL(ch), ch, "", SPELL_TYPE_SPELL, vict, 0);
        if(mh < 400) return TRUE;
    case 2:
        act("$n utters the words 'qassir porolo'.", 1, ch, 0, 0, TO_ROOM);
        cast_gas_breath(GET_LEVEL(ch), ch, "", SPELL_TYPE_SPELL, vict, 0);
        if(mh < 400) return TRUE;
    case 3:
        act("$n utters the words 'qassir relata'.", 1, ch, 0, 0, TO_ROOM);
        cast_lightning_breath(GET_LEVEL(ch), ch, "", SPELL_TYPE_SPELL, vict, 0);
        if(mh < 400) return TRUE;
    case 4:
        act("$n utters the words 'qassir moolim'.", 1, ch, 0, 0, TO_ROOM);
        cast_frost_breath(GET_LEVEL(ch), ch, "", SPELL_TYPE_SPELL, vict, 0);
        return TRUE;
    }
    return TRUE;
}

