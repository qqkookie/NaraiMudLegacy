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
extern struct title_type titles[4][IMO+4];
extern struct index_data *mob_index;

/* extern procedures */
void hit(struct char_data *ch, struct char_data *victim, int type);
void gain_exp(struct char_data *ch, int gain);
void advance_level(struct char_data *ch);
void stop_fighting(struct char_data *ch);
void set_title(struct char_data *ch);
char *string_dup(char *source);

int level_gate(struct char_data *ch, int cmd, char *arg)
{
    char buf[100];
    int f,r;

    if ( cmd== COM_CAST ) {
        send_to_char("You cannot do that here.\n\r",ch);
        sprintf(buf,"%s attempts to misbehave here.\n\r",ch->player.name);
        send_to_room_except(buf,ch->in_room,ch);
        return TRUE;
    }

    f = 0;        r=world[ch->in_room].number;
    switch ( cmd ) {
    case COM_NORTH :	/* north */
    case COM_EAST :	/* east */
        break;
    case COM_SOUTH :	/* south */
        if(r==1453) f=(GET_LEVEL(ch) < 12);
        else if(r==3041) f=(GET_LEVEL(ch) < 25);
        else if(r==2535) f=(GET_LEVEL(ch) > 15);
        else if(r==6001) f=(GET_LEVEL(ch) > 12);
        else if(r==9400) f=(GET_LEVEL(ch) > 3);
        break;
    case COM_WEST :	/* west */
        if(r==3500) f=(GET_LEVEL(ch) > 20);
        else if(r==5200) f=(GET_LEVEL(ch) > 24 || GET_LEVEL(ch) < 10);
		/* else if(r==5027) f=(GET_LEVEL(ch) > 38); */
        break;
    case COM_UP :	/* up */
    case COM_DOWN :	/* down */
    default :
        break;
    }

    if((f)&&(GET_LEVEL(ch) < IMO)){
        act("$n attempts go to where $e is not welcome.",FALSE,ch,0,0,TO_ROOM)
            ;
        send_to_char("People of your level may not enter.\n\r",ch);
        return TRUE;
    }
    return FALSE;
}

int neverland(struct char_data *ch, int cmd, char *arg)
{
    int location,loc_nr;
    extern int top_of_world;

    if(cmd != COM_DOWN )   /* specific to Room 2707. cmd 6 is move down */
        return(FALSE);
    switch(number(1,4)) {
    case 1:  loc_nr = 2713;
        break ;
    case 2:  loc_nr = 2711;
        break ;
    case 3:  loc_nr = 2722;
        break ;
    case 4:  loc_nr = 2716;
        break ;
    default: loc_nr = 2724;
    }
    
    location = real_room(loc_nr) ;
    act("$n씨가 지금 내려 갔나요 ??",FALSE,ch,0,0,TO_NOTVICT);
    send_to_char("악.\n\n악..\n\n 악...\n\n\n떨어지고 있습니다..\n\n\r",ch);
    send_to_char("여기가 어딜까 ??\n\r", ch) ;
    char_from_room(ch);
    char_to_room(ch,location);
    return(TRUE);
}


int helper(struct char_data *ch, int cmd, char *arg)
{
    struct char_data *vict, *next_victim;
    int lev ;

    if(cmd) return FALSE ;
    for(vict=world[ch->in_room].people; vict; vict = next_victim ) {
        next_victim = vict->next_in_room ;
        if(IS_NPC(vict)) {
            if ((100*GET_HIT(vict)/GET_MAX_HIT(vict)) > 85)
                continue ;
            else
                break;
        }
    }
    if (!vict || number(0,4) > 3)
        return FALSE;
    lev = GET_LEVEL(ch) ;
    if ( lev <= 11 ) {
        act("$n utters the words 'asd iasp'.", 1, ch, 0, 0, TO_ROOM);
        cast_cure_light(GET_LEVEL(ch), ch, "", SPELL_TYPE_SPELL, vict, 0);
    }
    else if ( lev <= 19 ) {
        act("$n utters the words 'sagok sghui'.",1,ch,0,0,TO_ROOM);
        cast_cure_critic(GET_LEVEL(ch), ch,"",SPELL_TYPE_SPELL,vict,0);
    }
    else if ( lev <= 27 ) {
        act("$n utters the words 'laeh'.", 1, ch, 0, 0, TO_ROOM);
        cast_heal(GET_LEVEL(ch), ch, "", SPELL_TYPE_SPELL, vict, 0);
    }
    else {
        act("$n utters the words 'sjagh'.", 1, ch, 0, 0, TO_ROOM);
        cast_full_heal(GET_LEVEL(ch), ch, "", SPELL_TYPE_SPELL, vict, 0);
    }
    return TRUE;
}
int dark_sand(struct char_data *ch, int cmd, char *arg)
{
    int drain;

    if (!IS_NPC(ch) && ( cmd == COM_NORTH || cmd == COM_EAST || cmd == COM_SOUTH
                         || cmd == COM_WEST || cmd == COM_UP || cmd == COM_DOWN)) {
		send_to_char(".... 검은 모래에서 풍기는 기운이 힘을 빠지게합니다.\n\r", ch);
		drain = dice(4,4);
        if ( GET_HIT(ch) < 150 ) drain = drain / 2 ;
        if ( GET_HIT(ch) < 60 ) drain = drain / 2 ;
        if ( GET_HIT(ch) < 30 ) drain = 1 ;
        if ( GET_HIT(ch) < 3 ) drain = 0 ;
        GET_HIT(ch) -= drain ;

		if (GET_MANA(ch) > 0) GET_MANA(ch) -= dice(3,3);
		if (GET_MOVE(ch) > 0) GET_MOVE(ch) -= dice(3,4);
    }
	else if (!IS_NPC(ch) && (cmd==COM_CAST)) {
		GET_MANA(ch) -= dice(3,3);
	}
    return FALSE ;
}

int electric_shock(struct char_data *ch, int cmd, char *arg)
{
    extern struct weather_data weather_info;
    char *msg;
    int weather, shock ;

    shock = 0 ;
    weather = weather_info.sky ;
    if (!IS_NPC(ch) && ( cmd == COM_NORTH || cmd == COM_EAST || cmd == COM_SOUTH
                         || cmd == COM_WEST || cmd == COM_UP || cmd == COM_DOWN)) {
        switch (weather) {
        case SKY_RAINING :
            msg = "ZZirrrr... 비가 오니까 찌릿 합니다.\n\r" ;
            shock = 50;
            break ;
        case SKY_LIGHTNING :
            msg = "ZZirk ZZirk .. 이런 날씨에는 감전되어 죽겠습니다.\n\r" ;
            shock = dice(50,7) ;
            break ;
        case SKY_CLOUDLESS :
        case SKY_CLOUDY :
            return FALSE ;
        default :
            return FALSE ;
        }
        send_to_char(msg,ch);
        if ( GET_HIT(ch) < 150 ) shock = shock / 2 ;
        if ( GET_HIT(ch) < 60 ) shock = shock / 2 ;
        if ( GET_HIT(ch) < 30 ) shock = 1 ;
        if ( GET_HIT(ch) < 3 ) shock = 0 ;
        GET_HIT(ch) -= shock ;
    }
    return FALSE ;
}

int great_mazinga(struct char_data *ch, int cmd, char *arg)
{
    int	tmp ;
    struct char_data *vict;
    struct char_data *choose_victim(struct char_data *mob, int fmode, int mode) ;
    void mob_punch_drop(struct char_data *mob, struct char_data *player) ;

    if(cmd) return FALSE;

    if ( ch->specials.fighting && number(0,5) < 4 ) {
        vict = choose_victim(ch, VIC_FIGHTING, MODE_RANDOM) ;
        if ( vict ) {
            act("$n utters the words '로케트 주먹'.", 1, ch, 0, 0, TO_ROOM);
            mob_punch_drop(ch, vict) ;
            if ((tmp=GET_MOVE(vict)) > 0 ) {
                send_to_char("주먹에 맞아 체력이 떨어집니다.\n\r", vict) ;
                GET_MOVE(vict) = tmp*7/10 ;
            }
        }
    }

    if ( number(0,5) > 3 ){
        if ( number(0,1) == 0 ) {
            vict = choose_victim(ch, VIC_FIGHTING, MODE_MOVE_MAX) ;
            if ( vict ) {
                act("$n utters the words 'esuder evom evo'.", 1, ch, 0, 0, TO_ROOM);
                if ((tmp=GET_MOVE(vict)) > 0 ) {
                    send_to_char("갑자기 힘이 빠집니다.\n\r", vict) ;
                    GET_MOVE(vict) = tmp/3 + dice(3, tmp/10) ;
                }
            }
        }
        else {
            vict = choose_victim(ch, VIC_ALL, MODE_MANA_MAX) ;
            if ( vict ) {
                act("$n utters the words 'esuder evom ana'.", 1, ch, 0, 0, TO_ROOM);
                if ((tmp=GET_MOVE(vict)) > 0 ) {
                    send_to_char("갑자기 힘이 빠집니다.\n\r", vict) ;
                    GET_MOVE(vict) = tmp/3 + dice(3, tmp/10) ;
                }
            }
        }
    }

    return TRUE;
}

/* by jmjeong (2001/11/3) */
static char *sumiae_song[]={
    "$n sings, '...다시 또 누군가를 만나서 사랑을 하게 될 수 있을까?'",
    "$n sings, '그럴 수는 없을 것 같아.'",
    "$n sings, '도무지 알 수 없는 한가지'",
    "$n sings, '사람을 사랑하게 되는 일. 참 쓸쓸한 일인 것 같아'",
    "$n sings, '사랑이 끝나고 난 뒤에는 이 세상도 끝나고'",
    "$n sings, '날 위해 빛나던 모든 것도 그 빛을 잃어 버려'",
    "$n sings, '누구나 사는 동안에 한번, 잊지 못할 사람을 만나고'",
    "$n sings, '잊지 못할 이별도 하지. ",
    "$n sings, '도무지 알 수 없는 한가지'",
    "$n sings, '사람을 사랑한다는 그 일. 참 쓸쓸한 일인 것 같아...'"
};
int sumiae_healer(struct char_data *ch, int cmd, char *arg)
{
	static int n = 0;
	int healed = 0;
	int num_of_pcs = 0;
	char buf[255];
	static char memory_name[255];
	static long count = 0;

	void mob_teleport(struct char_data *ch, int level);

	struct char_data *vict, *next_victim;
	int lev, alive, message;

	if (cmd == COM_RECITE || cmd == COM_USE) {
        send_to_char("You cannot do that here.\n\r",ch);
		sprintf(buf,"%s attempts to misbehave here.\n\r",ch->player.name);
		send_to_room_except(buf,ch->in_room,ch);
		return TRUE;
	}
	if (cmd) return FALSE ;					            

	count++;

	if(GET_HIT(ch) < GET_MAX_HIT(ch) * 0.5) {
        act("$n utters the words 'tloiv'.", 1, ch, 0, 0, TO_ROOM);
		cast_full_heal(GET_LEVEL(ch), ch, "", SPELL_TYPE_SPELL, ch, 0);
	}
	if(GET_HIT(ch) < GET_MAX_HIT(ch) * 0.4) {
        act("$n utters the words 'teloiv'.", 1, ch, 0, 0, TO_ROOM);
		cast_full_heal(GET_LEVEL(ch), ch, "", SPELL_TYPE_SPELL, ch, 0);
	}
	if(GET_HIT(ch) < GET_MAX_HIT(ch) * 0.2) {
        act("$n utters the words 'rebmit'.", 1, ch, 0, 0, TO_ROOM);
		cast_full_heal(GET_LEVEL(ch), ch, "", SPELL_TYPE_SPELL, ch, 0);
		cast_full_heal(GET_LEVEL(ch), ch, "", SPELL_TYPE_SPELL, ch, 0);
	}
	if (IS_AFFECTED(ch, AFF_POISON) && number(1,3) == 1) {
		cast_remove_poison(GET_LEVEL(ch), ch, "", SPELL_TYPE_SPELL, ch, 0);
	}

	if ( ch->specials.fighting ) {
		if (!IS_NPC(ch->specials.fighting)) {
			strncpy(memory_name, ch->specials.fighting->player.name, 254);
			count = 0;
		}

		if ( ch->in_room != ch->specials.fighting->in_room) {
			stop_fighting(ch) ; /* he flee already */
            return TRUE;
		}
		else {
			/* set attacker bit is criminal */
			if (GET_LEVEL(ch->specials.fighting) > 30) {
				SET_BIT(ch->specials.fighting->specials.act, PLR_CRIMINAL);
			}

			if (number(1,10) >= 4) {
				act("$n가 보라빛 꽃잎을 흩날립니다.",TRUE,ch,0,0,TO_ROOM);
				send_to_char("\n\r몸이 갑자기 가벼워지더니 바람에 날리네요\n\r", 
					ch->specials.fighting);
				send_to_char("악.\n\n악..\n\n 악...\n\n\n떨어지고 있습니다..\n\n\r",
					ch->specials.fighting);
				send_to_char("여기가 어딜까 ??\n\r", ch->specials.fighting);
				send_to_char("떨어진 충격에 정신을 차릴 수가 없습니다.\n\r", 
					ch->specials.fighting);

				mob_teleport(ch->specials.fighting, 
							GET_LEVEL(ch->specials.fighting));

				WAIT_STATE(ch->specials.fighting, PULSE_VIOLENCE*4);
			}
			else {
				if(number(1,10) > 4)
				{     
					act("$n	get angry .",1,ch,0,0,TO_ROOM);
					act("$n	POWER UP !.",1,ch,0,0,TO_ROOM);
					if (ch->points.hitroll < 80 && number(1,3) > 1)  {
						ch->points.hitroll += 2;
					}
					if (ch->points.damroll < 90 && number(1,3) > 2){
						ch->points.damroll += 3;
					}
				}
				else if (ch->points.armor > -195) ch->points.armor -= 3;
			}
		}
		return TRUE;
	}

	// check the man who attacked me
	for(vict=world[ch->in_room].people; vict; vict = next_victim ) {
		next_victim = vict->next_in_room ;
		if(!IS_NPC(vict)&&!strncmp(vict->player.name, memory_name,254)) {
			if (count < 220) {
				if (number(1,10) >= 3) {
					act("$n이 소리쳤습니다, '당신은 절 공격한 사람이군요!!!'",
						FALSE, ch, 0, 0, TO_ROOM);
					hit(ch, vict, TYPE_UNDEFINED);
					return TRUE;
				}
				else {
					act("$n utters the words 'Deep Thanatos'.",
								1,ch,0,0,TO_ROOM);
					cast_sleep(GET_LEVEL(ch), ch, "", 
								SPELL_TYPE_SPELL, vict, 0);
					cast_paralysis(GET_LEVEL(ch), ch, "", 
								SPELL_TYPE_SPELL, vict, 0);
					return(TRUE);
				}
			}
			else {
				// forget the victim
				memory_name[0] = 0;
			}
        }
	}

	for(vict=world[ch->in_room].people; vict; vict = next_victim ) {
		next_victim = vict->next_in_room ;
		if(!IS_NPC(vict)) {					/* helping player */
			if (GET_LEVEL(vict) < IMO+1) num_of_pcs++;

			lev = GET_LEVEL(vict) ;
			if ( lev > 25 ) continue ;		/* only help low level */

			if (number(1,15)>14) continue;

			message = 0;
			alive = 100*GET_HIT(vict)/GET_MAX_HIT(vict) ;/* are they hurt ? */
			if ( alive < 60 && lev < 25 ) {
				message = 1;
				healed = 1;
				act("$n님이 '저런, 많이 다치셨군요' 라고 말합니다.",
					1,ch,0,0,TO_ROOM);
				act("$n님이 손을 저어 따스한 기운을 만들어 냅니다.",
					1,ch,0,0,TO_ROOM);
				cast_cure_critic(GET_LEVEL(ch), ch, "", SPELL_TYPE_SPELL, vict, 0);
            }
			else if ( alive < 90 ) {
				healed = 1;
				if (!message)
					act("$n님이 손을 저어 따스한 기운을 만들어 냅니다.",
						1,ch,0,0,TO_ROOM);
				cast_cure_light(GET_LEVEL(ch), ch, "", SPELL_TYPE_SPELL, vict, 0);
            }
        }
    }		/* end of for */

	if(GET_POS(ch)!=POSITION_STANDING) return FALSE;

	// 저레벨을 치료하지 않았고, 방에 혼자만 있으면..
	// 
	if (!healed && num_of_pcs <= 1 ) {
		if (number(1,26) == 20) n = 11;
		if(n < 10)
			act(sumiae_song[n],TRUE,ch,0,0,TO_ROOM);
		else if (n == 10)  act("$n bows deeply.", TRUE, ch, 0,0, TO_ROOM);
		if((++n)==32) n=0;
		return TRUE;
	}

	return TRUE;
}

int school_cold(struct char_data *ch, int cmd, char *arg)
{
	struct char_data *vict, *next_victim;
	int lev, alive, healed, blessed, armored ;

	if(cmd) return FALSE ;

	healed = blessed = armored = 0 ;
	for(vict=world[ch->in_room].people; vict; vict = next_victim ) {
		next_victim = vict->next_in_room ;
		if(!IS_NPC(vict)) {		/* helping player */
			lev = GET_LEVEL(vict) ;
			if ( lev > 9 ) continue ;		/* only help low level */
			alive = 100*GET_HIT(vict)/GET_MAX_HIT(vict) ;	/* are they hurt ? */
			if ( alive < 50 && lev < 6 ) {
				if ( !healed ) {
					act("$n님이 '저런, 많이 다치셨군요' 라고 말합니다.",
						1,ch,0,0,TO_ROOM);
					act("$n님이 손을 저어 따스한 기운을 만들어 냅니다.",
						1,ch,0,0,TO_ROOM);
                }
				cast_cure_light(GET_LEVEL(ch), ch, "", SPELL_TYPE_SPELL, vict, 0);
				healed ++ ;
            }
			if ( alive < 90 ) {
				if ( !healed ) {
					act("$n님이 손을 저어 따스한 기운을 만들어 냅니다.",
						1,ch,0,0,TO_ROOM);
                }
				cast_cure_light(GET_LEVEL(ch), ch, "", SPELL_TYPE_SPELL, vict, 0);
				healed ++ ;
            }
			if ( lev > 6 ) continue ;
			if ( !affected_by_spell(vict, SPELL_ARMOR) ) {
				if ( !armored ) {
					act("$n님의 몸에서 바람이 일더니 꽃향기가 되어 방안에 가득 퍼집니다.",
						1, ch, 0, 0, TO_ROOM);
					cast_armor(20, ch, "", SPELL_TYPE_SPELL, vict, 0);
					armored ++ ;
                }
            }
			if ( lev > 3 ) continue ;
			if ( !affected_by_spell(vict, SPELL_BLESS) ) {
				if ( !blessed ) {
					act("$n님이 그의 반지에서 눈부신 빛살을 일으킵니다.",
						1,ch,0,0,TO_ROOM);
					cast_bless(20, ch, "", SPELL_TYPE_SPELL, vict, 0);
					blessed ++ ;
                }
            }
        }
    }		/* end of for */

	return TRUE;
}

#define G_NONE		0
#define G_SAY		1
#define G_WHISPER	2
#define G_SMILE		3

int gracia_response ;	/* value is G_NONE to G_SMILE */
struct char_data *gracia_lastperson ;

int school_gracia(struct char_data *ch, int cmd, char *arg)
{
	static char *say_to[] = {
		"만나서 참 반가와요, $N님.",
		"자 조금만 기운내세요, $N님.", 
		"$N님은 어디서 오셨나요 ?",
		"$N님은 여기서 다른 친구들은 많이 사귀었나요 ?"
    } ;

	static char *say_all[] = {
		"라라라라 ~~ 사람이 많이 오면 기분이 좋아진단 말야.",
		"여기 까지 오는데 힘드 셨죠 ?",
		"힘들면 좀 앉아서 쉬었다 가세요 :)",
		"여러분들을 보니까 기분이 참 좋아요.",
		"여러분들 얼굴을 직접 볼 수 있으면 더 좋을텐데... :)"
    } ;

	static char *whisper_to[] = {
		"오늘은 기분이 어때요 ?", 
		"이런 좋은 날 어디 안가세요 ?",
		"머드 학교 졸업하면 가장 먼저 뭘 하고 싶으세요 ?"
    } ;
	extern int gracia_response ;
	extern struct char_data *gracia_lastperson ;
	struct char_data *vict, *next_victim, *response_to ;
	int n_person ;
	char buf[200] ;

	if(cmd) {
		if ( cmd == COM_SAY || cmd == COM_LITTLE_QUOAT || cmd == COM_HAN_SAY )
			gracia_response = G_SAY ; /* say */
		if ( cmd == COM_WHISPER )	/* whisper */
			gracia_response = G_WHISPER ;
		if ( cmd == COM_SMILE || cmd ==COM_HAN_SMILE || cmd == COM_HAN_SMILE2 ) {
			/* smile */
			gracia_response = G_SMILE ;
			gracia_lastperson  = ch ;
        }

		return FALSE ;	/* anyway return False */
    }

	n_person = 0 ;
	for(vict=world[ch->in_room].people; vict; vict = next_victim ) {
		next_victim = vict->next_in_room ;
		if(!IS_NPC(vict)) {		/* There is a player */
			if (n_person == 0) {
				response_to = vict ;
            }
			n_person ++ ;
        }
    }

	if ( n_person == 0 )
		return TRUE ;

	if ( gracia_response != 0 ) {
		switch (gracia_response) {
        case G_SAY :
        case G_WHISPER :
            if ( number(0,1) == 0)
                act("Gracia님이 '흠..' 하고 말합니다", 1, ch, 0, 0, TO_ROOM);
            else
                act("$n님이 곰곰이 생각 합니다.", 1, ch, 0, 0, TO_ROOM);
            break ;
        case G_SMILE :
            if ( gracia_lastperson != NULL &&
                 gracia_lastperson->in_room == ch->in_room) {
                act("$n님이 $N님을 보고 미소를 짓습니다.", FALSE, ch, 0,
                    gracia_lastperson, TO_NOTVICT) ;
                send_to_char("Gracia님이 당신을 보고 방긋 웃습니다.\n\r",
                             gracia_lastperson) ;
                act("$n님의 눈가에 웃음이 번집니다.", 1, ch, 0, 0, TO_ROOM);
            }
            break ;
        }
		gracia_response = G_NONE ;
		gracia_lastperson = NULL ;
    }

	if ( n_person + number(0,4) > 4 ) {
		sprintf(buf, "Gracia님이 '%s' 라고 말합니다.", say_all[number(0,3)]) ;
		act(buf, 1, ch, 0, 0, TO_ROOM);
    }
	else if ( number(0,4) < 3 ) {	/* say to */
		if (response_to != NULL) {
			sprintf(buf, "Gracia님이 '%s' 라고 말합니다.", say_to[number(0,3)]) ;
			act(buf, 1, ch, 0, response_to, TO_ROOM);
        }
    }
	else {		/* whisper to */
		if ( response_to != NULL &&
             response_to->in_room == ch->in_room) {
			sprintf(buf, "Gracia님이 당신에게 '%s' 라고 속삭입니다.\n\r",
                    whisper_to[number(0,2)]) ;
			act("$n님이 $N님의 귀에 작은 소리로 무엇인가 말합니다.",
				FALSE, ch, 0, response_to, TO_NOTVICT) ;
			send_to_char(buf, response_to) ;
        }
    }

	return TRUE ;
}

#undef G_NONE
#undef G_SAY
#undef G_WHISPER
#undef G_SMILE

struct char_data *nara_lastperson ;
int school_nara(struct char_data *ch, int cmd, char *arg)
{
	extern struct char_data *nara_lastperson ;
	struct obj_data	*obj ;
	int	r_num ;
	char name[256], buf[256] ;

	if(cmd) {
		if ( IS_NPC(ch) )
			return FALSE ;
		if ( cmd == COM_BOW ) {		/* bow */
			one_argument(arg, name) ;
			if (strcmp(name, "nara") == 0 || strcmp(name, "Nara") ) {
				if ( nara_lastperson == NULL ) {
					nara_lastperson = ch ;
					send_to_char("앉아서 기다리세요.\n\r", nara_lastperson) ;
                }
            }
        }
		return FALSE ;
    }

	if ( nara_lastperson == NULL )
		return FALSE ;

	if ( IS_NPC(nara_lastperson) ) {
		nara_lastperson = NULL ;
		return FALSE ;
    }

	if ( ch->in_room == nara_lastperson->in_room ) {
		if ( !is_solved_quest(nara_lastperson, QUEST_SCHOOL)) {
			sprintf(buf, "%s %s\n\r", "잘 하셨습니다.",
                    "당신은 이 학교에서 해야 할 일을 잘 마치셨습니다.") ;
			send_to_char(buf, nara_lastperson) ;
			set_solved_quest(nara_lastperson, QUEST_SCHOOL) ;
			act("$n님이 $N님에게 졸업장을 수여 합니다.",
				FALSE, ch, 0, nara_lastperson, TO_NOTVICT) ;
			send_to_char("Nara님이 당신에게 졸업장을 줍니다.\n\r",
                         nara_lastperson) ;

			/* remove nochat, noshout bit */
			if (IS_SET(nara_lastperson->specials.act, PLR_NOCHAT)) {
				REMOVE_BIT(nara_lastperson->specials.act, PLR_NOCHAT);
            }
			if (IS_SET(nara_lastperson->specials.act, PLR_NOTELL)) {
				REMOVE_BIT(nara_lastperson->specials.act, PLR_NOTELL);
            }
			if (IS_SET(nara_lastperson->specials.act, PLR_EARMUFFS)) {
				REMOVE_BIT(nara_lastperson->specials.act, PLR_EARMUFFS);
            }

			if (GET_LEVEL(nara_lastperson) < 2) {
				GET_LEVEL(nara_lastperson) = 2 ;
				advance_level(nara_lastperson) ;
				set_title(nara_lastperson) ;
				send_to_char("Nara님이 당신의 레벨을 올려 주었습니다.\n\r",
                             nara_lastperson) ;
            }
			if (( obj = read_object(3052, VIRTUAL)) == 0 ) {	/* recall scroll */
				log("graduation_hall: fail to find scroll number") ;
				return FALSE ;
            }
			/* 0 is direction north 
               if (EXIT(nara_lastperson, 0)) {	
               obj_to_room(obj, EXIT(nara_lastperson, 0)->to_room) ;
               }
			*/
			obj_to_char(obj, nara_lastperson) ;
        }
		else {	/* he/she solved already */
			act("$n님이 $N님에게 '예의가 바른 학생이군' 이라 말합니다.",
				FALSE, ch, 0, nara_lastperson, TO_NOTVICT) ;
			send_to_char("Nara님이 당신을 칭찬 합니다.\n\r",
                         nara_lastperson) ;
        }
    }
	else {	/* he/she isn't here */
		act("Nara님이 '아니 아까 그 학생이 어디에 갔지 ?' 하고 말합니다.",
			1, ch, 0, nara_lastperson, TO_ROOM);
    }

	nara_lastperson = NULL ;
	return TRUE ;
}

int teleport_machine(struct char_data *ch, int cmd, char *arg)
{
	extern struct room_data *world ;
	extern struct time_info_data time_info ;
	int is_wearing_teleport_ring(struct char_data *ch) ;
	struct char_data	*tch, *next_tch ;
	int	old_room, new_room ;
	char	name[256] ;

	/* teleport machine in robot city */
	if ( cmd != COM_PRESS )
		return FALSE ;

	one_argument(arg, name) ;
	if ( *name == 0 ) {
		send_to_char("Press what ?\n\r", ch) ;
		return TRUE ;
    }

	if ( !isname(name, "button 단추")) {
		send_to_char("I can't find that.\n\r", ch) ;
		return TRUE ;
    }
	else {	/* press button */
		if (( time_info.hours <= 12 && ch->in_room == real_room(15076) ) || (
			time_info.hours > 12 && ch->in_room == real_room(15023))) {
			send_to_char("기계가 꺼져 있습니다. 다른 곳에 가보세요.\n\r", ch);
			return TRUE ;
        }
		send_to_char("단추를 누르자 꽝 하는 소리가 납니다.\n\r", ch) ;
		act("꽝 하는 소리와 함께 방안에 연기가 가득 찹니다.",
			FALSE, ch, 0, 0, TO_ROOM);
		if ( is_wearing_teleport_ring(ch)) {
			new_room = choose_teleport_room(ch) ;
			for ( tch = world[ch->in_room].people ; tch ; tch = next_tch ) {
				next_tch = tch->next_in_room ;
				if (IS_NPC(tch)) continue ;
				if (tch->specials.fighting)
					stop_fighting(tch);
				char_from_room(tch);
				char_to_room(tch, new_room);
				send_to_char("야릇한 기분이 듭니다.\n\r", tch) ;
				act("$n님이 작은 연기와 함께 방 가운데 나타 납니다.", FALSE,
					tch, 0, 0, TO_ROOM);
				do_look(tch, "", 0);
            }
        }	/* wearing teleport control ring */
		else {
			for ( tch = world[ch->in_room].people ; tch ; tch = next_tch ) {
				next_tch = tch->next_in_room ;
				if (IS_NPC(tch)) continue ;
				if (tch->specials.fighting)
					stop_fighting(tch);
				new_room = choose_teleport_room(tch) ;
				char_from_room(tch);
				char_to_room(tch, new_room);
				send_to_char("야릇한 기분이 듭니다.\n\r", tch) ;
				act("$n님이 작은 연기와 함께 방 가운데 나타 납니다.", FALSE,
					tch, 0, 0, TO_ROOM);
				do_look(tch, "", 0);
            }
        }	/* not wearing teleport control ring */
    }	/* press button */

	return TRUE ;
}

int is_wearing_teleport_ring(struct char_data *ch)
{
	int teleport_ring ;

	if ( !ch ) return 0 ;

	teleport_ring = 0 ;
	if ( ch->equipment[WEAR_FINGER_R] ) {
		if (obj_index[ ch->equipment[WEAR_FINGER_R]->item_number ].virtual
			== 2711 )
			teleport_ring = 1;
    }
	if ( ch->equipment[WEAR_FINGER_L] ) {
		if (obj_index[ ch->equipment[WEAR_FINGER_L]->item_number ].virtual
			== 2711 )
			teleport_ring = 1;
    }
	return teleport_ring ;
}

int choose_teleport_room(struct char_data *ch)
{
	extern int	top_of_world ;
	int is_wearing_teleport_ring(struct char_data *ch) ;
	int	destiny, room_from, room_to, tmp ;

	destiny = -1 ;
	if ( ch == NULL ) {
		do {
			destiny = number(0, top_of_world);
        } while (IS_SET(world[destiny].room_flags, PRIVATE));
		return destiny ;
    }

	if (!is_wearing_teleport_ring(ch)) {
		room_from = 0 ;
		room_to = top_of_world ;
    }
	else {
		if ( ch->equipment[HOLD] ) {
			switch (obj_index[ ch->equipment[HOLD]->item_number ].virtual) {
            case 5230 : /* stone pale blue */
                destiny = real_room(15193) ;
                break ;
            case 5231 : /* stone scarlet blue */
            case 5232 : /* stone blue incandescent */
            case 5233 : /* stone red deep */
            case 5234 : /* stone pink */
                room_from = real_room(201) ;	/* castle gate */
                room_to = real_room(2900) - 1 ;	/* chok */
                break ;
            case 5235 : /* stone pearly white */
            case 5236 : /* stone lavander pale */
                /* moria, aerie, desert, drowcity */
                room_from = real_room(4000) ;
                room_to = real_room(5200) - 1 ;
                break ;
            case 5237 : /* stone lavander green */
            case 5238 : /* stone rose dusty */
                /* forest, dwarven, sewer, redferne, arachnos, arena */
                room_from = real_room(6000) ;
                room_to = real_room(9500) - 1 ;
                break ;
            case 5239 : /* stone grey dull */
            case 5240 : /* stone purple vibrant */
                /* galaxy, deathstar, shogun, easy, market, olympus */
                room_from = real_room(9701) ;
                room_to = real_room(15000)  - 1;
                break ;
            case 5241 : /* stone pink green */
            case 5242 : /* stone green pale : wee and onara */
                room_from = real_room(16000) ;
                room_to = real_room(17094) ;	/* onara */
                break ;
            case 5243 : /* stone clear : robo city */
                room_from = real_room(15000) ;
                room_to = real_room(16000)  - 1;
                break ;
            case 5244 : /* stone iridescent */
                room_from = top_of_world/2 ;
                room_to = top_of_world ;
                break ;
            case 1325 :	/* mauve stone */
                room_from = 0 ;
                room_to = top_of_world/2 ;
                break ;
            case 1311 :	/* totem : midgaard */
                room_from = real_room(3001) ;
                room_to = real_room(4000) - 1 ;
                break ;
            case 2838 :	/* sharon stone : death kingdom */
                room_from = real_room(9500) ;
                room_to = real_room(9701) -1 ;
                break ;
            default :
                room_from = 0 ;
                room_to = top_of_world ;
            }
        }
		else {	/* wear ring, but has no special stone */
			room_from = top_of_world / 4 ;
			room_to = top_of_world * 3 / 4 ;
        }
    }	/* end of teleport ring */
	if ( destiny < 0 || destiny > top_of_world ) {	/* not decided yet */
		if ( room_from > room_to ) {
			log("debug: from to swap in select teleport") ;
			tmp = room_from ;
			room_from = room_to ;
			room_to = tmp ;
        }
		do {
			destiny = number(room_from, room_to);
        } while (IS_SET(world[destiny].room_flags, PRIVATE));
    }

	return destiny ;
}

int control_center(struct char_data *ch, int cmd, char *arg)
{
	char	name[256], buf[BUFSIZ] ;
	static int	power_button = 0 ;
	static int	shutdown_button = 0 ;
	extern int	slow_death ;
	extern int	nologin ;
	extern int	boottime ;
	int	running_time ;

	/* reboot button */
	if ( cmd != COM_LOOK && cmd != COM_PRESS )
		return FALSE ;

	one_argument(arg, name) ;
	if ( *name == 0 ) {	/* no argument */
		if ( cmd == COM_LOOK ) return FALSE ;
		if ( cmd == COM_PRESS ) {
			send_to_char("Press what ?\n\r", ch) ;
			return TRUE ;
        }
    }
	else {	/* has argument */
		if ( cmd == COM_LOOK ) {
			if ( !isname(name, "machine button switch"))
				return FALSE ;
			/* look machine now */
			sprintf(buf, "This is Mud control machine\n\r") ;
			strcat(buf, "You can see power button and shutdownbutton.\n\r\n\r") ;
			if ( power_button ) {	/* is on */
				strcat(buf, "  Power button - ON\n\r") ;
				if ( shutdown_button )
					strcat(buf, "  Shutdownbutton - Shutdown Started\n\r") ;
				else
					strcat(buf, "  Shutdownbutton - Ready\n\r") ;
            }
			else {	/* power button is off */
				strcat(buf, "  Power button - OFF\n\r") ;
				strcat(buf, "  Shutdownbutton - Standby\n\r") ;
            }
			strcat(buf,
                   "\n\rNOTE: If shutdown is started no one is able to stop it\n\r" ) ;
			send_to_char(buf, ch) ;
			act("$n looks the machine", FALSE, ch, 0, 0, TO_ROOM) ;
        }	/* end of command 'look button' */
		else {	/* command 'press' */
			if ( !isname(GET_NAME(ch), "indra juniper poseidon bluebard")) {
				send_to_char("You didn't register to do that.\n\r", ch) ;
				act("$n tried to touch machine, but failed", FALSE, ch,
					0, 0, TO_ROOM) ;
            }
			else if ( isname(name, "power switch powerbutton")) {
				if ( power_button ) {
					send_to_char("Machine is stoped now.\n\r", ch) ;
					act("$n toggled power switch and machine is stopped.",
						FALSE, ch, 0, 0, TO_ROOM) ;
					if ( shutdown_button ) {
						sprintf(buf, "SHUTDOWN is ABORTED by %s.", GET_NAME(ch));
						log(buf) ;
						strcat(buf, "\n\r") ;
						send_to_all(buf);
						power_button = 0 ;
						shutdown_button = 0 ;
						slow_death = -1 ;
						nologin = 0 ;
                    }
                }
				else {
					running_time = 30 + time(0) - boottime ;	/* second */
					if ( (running_time / 3600) < 10 ) {
						send_to_char("Too frequent shutdown, wait! \n\r", ch) ;
						return TRUE ;
                    }
					send_to_char("Machine is ready to shutdown now.\n\r", ch) ;
					act("$n turn on the power switch and it emits bright lights.",
						FALSE, ch, 0, 0, TO_ROOM) ;
					power_button = 1 ;
                }
            }
			else if ( strcmp(name, "shutdownbutton") == 0 ) {
				if ( power_button ) {
					if ( shutdown_button ) {
						send_to_char("Shut down started already.\n\r", ch) ;
                    }
					else {
						sprintf(buf,"-- SHUTDOWN STARTED by %s --",GET_NAME(ch));
						log(buf) ;
						strcat(buf, "\n\r") ;
						send_to_all(buf) ;
						send_to_char("Ok.\n\r", ch) ;
						nologin = 1 ;
						shutdown_button = 1 ;
						slow_death = 185 ;	/* 185 second */
                    }
                }
				else {
					send_to_char("Machine is offed. Turn on first\n\r", ch) ;
                }
            }
			else {
				send_to_char("You can't press such thing.\n\r", ch) ;
            }
        }	/* end of command 'press' */
    }	/* end of argument */

	return TRUE ;
}

int teleport_demon(struct char_data *ch, int cmd, char *arg)
{
	void mob_teleport(struct char_data *ch, int level) ;
	extern struct char_data *nara_lastperson ;
	char name[256], buf[256] ;
	static int	count ;

	if (cmd) return FALSE ;
	if (ch == NULL) return FALSE ;
	if (!IS_NPC(ch)) return FALSE ;

	if (ch->specials.fighting) {
		if ( GET_HIT(ch)*100/GET_MAX_HIT(ch) < 80 ) {
			/* do teleport */
			mob_teleport(ch, GET_LEVEL(ch)) ;
			if ( GET_LEVEL(ch) < IMO )
                GET_LEVEL(ch) = GET_LEVEL(ch) + 8 ;
        }
    }
	else if ( dice(3,4) > 10 || ++count > 100)	{	/* not fighting , teleport */
		mob_teleport(ch, GET_LEVEL(ch)) ;
		count = 0 ;
    }
	else {	/* Do nothing or kidding player */
		return FALSE ;
    }

	return FALSE ;
}

int terminator_shops(struct char_data *ch, int cmd, char *arg)
{
	struct affected_type	aff ;
	char	buf[MAX_STRING_LENGTH], tmp[BUFSIZ];
	char	buf2[BUFSIZ] ;
	int	termi_room, k;
	struct char_data	*termi;
	struct follow_type	*j;
	int	cost, hours ;

	if(IS_NPC(ch))
		return(FALSE);

	termi_room = real_room(12188) ;
	if (cmd== COM_LIST) { /* List */
		sprintf(buf, "다음과 같은 로보트가 있습니다:\n\r") ;
		for(termi = world[termi_room].people;termi; termi= termi->next_in_room) {
			if ( !IS_NPC(termi) )
				continue ;
			cost = 5 * GET_LEVEL(termi) * GET_LEVEL(termi) * GET_LEVEL(termi) ;
			sprintf(buf2, "%14s - %5d per hour\n\r", GET_NAME(termi), cost) ;
			strcat(buf, buf2) ;
        }
		sprintf(buf2, " 마음에 들면 'buy 이름 시간' 이라고 치십시오.\n\r") ;
		strcat(buf, buf2) ;
		send_to_char(buf, ch);
		return(TRUE);
    }
	else if (cmd== COM_BUY ) { /* Buy */
		arg = one_argument(arg, buf);
		arg = one_argument(arg, tmp);
		if ( *tmp ) hours = atoi(tmp) ;
		if ( *buf == 0 || *tmp == 0 || hours <= 0 ) {
			send_to_char("Usage: buy <이름> <시간>\n\r", ch) ;
			return (TRUE) ;
        }

		if (!(termi = get_char_room(buf, termi_room))) {
			send_to_char("그런 로보트는 없는데요 ?\n\r", ch);
			return(TRUE);
        }
		for (k=0,j=ch->followers ;(j) && (k<5) ; ++k ) {
			j=j->next;
        }
		if(k>=3){
			send_to_char("당신은 지금 데리고 있는 동물로 충분할 것 같은데요.\n\r",ch);
			return(TRUE);
        }
		cost = 5 * GET_LEVEL(termi) * GET_LEVEL(termi) * GET_LEVEL(termi) ;
		if (GET_GOLD(ch) < (cost * hours)) {
			send_to_char("돈이 모자랍니다 !!\n\r", ch);
			return(TRUE);
        }
		GET_GOLD(ch) -= cost * hours ;
		termi = read_mobile(termi->nr, REAL);
		GET_EXP(termi) = 0;

		char_to_room(termi, ch->in_room);
		add_follower(termi, ch);
		aff.type = SPELL_CHARM_PERSON ;
		aff.duration = hours ;
		aff.modifier = 0 ;
		aff.location = 0 ;
		aff.bitvector = AFF_CHARM ;
		affect_to_char(termi, &aff) ;

		sprintf(buf, "이제 %d 시간동안 당신을 위해 일을 할 것입니다.\n\r", hours);
		send_to_char(buf, ch);
		sprintf(buf, "$n님이 $N 을 %d 시간동안 고용했습니다.\n\r", hours);
		act(buf, FALSE, ch, 0, termi, TO_ROOM);

		return(TRUE);
    }

	/* All commands except list and buy */
	return(FALSE);
}

int bogan(struct char_data *ch, int cmd, char *arg)
{
	void do_shout(struct char_data *ch, char *argument, int cmd);
	int	item_cost(struct obj_data *o) ;
	struct char_data *vict ;
	struct char_data *choose_victim(struct char_data *ch, int fightmode, int mode);
	struct obj_data	*obj, *sobj ;
	int	cost, maxcost, dir ;
	char	buf[BUFSIZ] ;

	if (cmd) return FALSE ;
	if (ch == NULL) return FALSE ;
	if ( ch->specials.fighting )
		return FALSE ;

	vict = choose_victim(ch, VIC_ALL, MODE_HIGH_LEVEL) ;
	if ( vict && vict->desc ) {	/* steal inventory ! */
		maxcost = 0 ;
		sobj = NULL ;
		for ( obj = vict->carrying ; obj ; obj = obj->next_content ) {
			if ((cost = GET_OBJ_RENTCOST(obj)) <= 0) {
				cost = item_cost(obj) ;
				GET_OBJ_RENTCOST(obj) = cost ;
            }
			if ( cost > maxcost ) {
				maxcost = cost ;
				sobj = obj ;
            }
        }
		if ( sobj && number(0,5) > 1 ) {	/* found something - steal */
			obj_from_char(sobj) ;
			obj_to_char(sobj, ch) ;
			acthan("$n grins evilly.", "$n 이 음흉한 미소를 짓습니다.",
                   TRUE, ch, 0, vict, TO_ROOM) ;
			sprintf(buf, "How fool! I got %s from %s!", 
					(sobj)->short_description, vict->player.name);
			do_shout(ch,buf,0);
			act("$n ROBBED YOU!",TRUE,ch,0,vict, TO_VICT) ;
			stash_char(vict, 0) ;
			dir = number(0, 5) ;
			if (CAN_GO(ch, dir) && 
				!IS_SET(world[EXIT(ch, dir)->to_room].room_flags, NO_MOB))
				do_simple_move(ch, dir, FALSE) ;
			return TRUE ;
        }
    }
	return FALSE ;
}

/* aggresive, smart_thief */
int smart_thief(struct char_data *ch, int cmd, char *arg)
{
	struct char_data *choose_victim(struct char_data *mob, int fmode, int mode) ;
	struct char_data *vict;
	int	aggresive, back_ok ;

    if(cmd) return FALSE;

	aggresive = back_ok = 0 ;
	if ( IS_SET(ch->specials.act, ACT_AGGRESSIVE))
		aggresive = 1 ;
	
	if ( ch->equipment[WIELD] )
		if ( ch->equipment[WIELD]->obj_flags.value[3] == 11 )
			back_ok = 1 ;

	if ( ch->specials.fighting ) {
		if (GET_MOVE(ch) > 99) {
			acthan("$n says 'Quu ha cha cha'.",
                   "$n님이 '크하차차' 하고 외칩니다", 1, ch, 0, 0, TO_ROOM);
			do_flash(ch, "", 0) ;
        }
		/* vict can be die.. light_move */
		acthan("$n says 'Gnint Hgil'.",
               "$n님이 '니트 길' 하고 외칩니다", 1, ch, 0, 0, TO_ROOM);
		mob_light_move(ch, "", 0) ;
		if ( !(ch->specials.fighting) ) {
			switch ( number(0,2) ) {
            case 0 :
            case 1 : vict = choose_victim(ch, VIC_ALL, MODE_HIT_MIN) ;
                break ;
            case 2 : vict = choose_victim(ch, VIC_ALL, MODE_AC_MAX) ;
                break ;
            }
			if ( vict ) {
				if ( back_ok )
					hit(ch, vict, SKILL_BACKSTAB) ;
				else
					hit(ch, vict, TYPE_UNDEFINED) ;
            }
        }
    }
	else {	/* not fighting */
		if ( aggresive ) {	/* look for target */
			switch ( number(0,2) ) {
            case 0 :
            case 1 : vict = choose_victim(ch, VIC_ALL, MODE_HIT_MIN) ;
                break ;
            case 2 : vict = choose_victim(ch, VIC_ALL, MODE_AC_MAX) ;
                break ;
            }
			if (vict) {
				if ( back_ok )
					hit(ch, vict, SKILL_BACKSTAB) ;
				else
					hit(ch, vict, TYPE_UNDEFINED) ;
            }
        }
    }

    return TRUE;
}

