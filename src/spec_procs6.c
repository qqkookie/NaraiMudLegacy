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
#include "mob_magic.h"	
#include "quest.h"	
#include "command.h"

extern struct room_data *world;

//
// zone coding : by sam(gayun)
// special action coding: by jmjeong
//
int zorro(struct char_data *ch, int cmd, char *arg)
{
    struct char_data *tmp_victim, *temp, *vict, *mob;
	extern struct char_data *character_list;
    extern struct index_data *obj_index;
    struct affected_type af;
    int real_number, h;

    if (cmd) return 0;
    
    vict=ch->specials.fighting;
    if(vict){
        if ( ch->in_room != ch->specials.fighting->in_room) {
			stop_fighting(ch) ; /* he flee already */
            return FALSE;
		}

        // 체력이 절반이 안 남은 경우
        //
        if (GET_HIT(ch) < GET_MAX_HIT(ch) /2) {
            h=number(0,12);
            if(h>8) return(0);

            for(tmp_victim = character_list; tmp_victim; tmp_victim = temp) {
                temp = tmp_victim->next;
                if ( ch->in_room != tmp_victim->in_room  &&
                     world[ch->in_room].zone == world[tmp_victim->in_room].zone){
                    send_to_char("Zorro shouts, '시민의 엄단!!!'", 
                                 tmp_victim);
                }
            }

            act("어디선가 성난 시민들의 목소리가 들려옵니다!",1,ch,0,0,TO_ROOM);
            act("'이런 못된 놈들!!!'",1,ch,0,0,TO_ROOM);

            if(real_number=real_mobile(18008)){     // 분노한 시민 소환
                mob=read_mobile(real_number,REAL);
                char_to_room(mob,ch->in_room);
            }
            return(1);
        }

		h=number(0,20);
		if (h > 11) return(0);

		for(tmp_victim = character_list; tmp_victim; tmp_victim = temp) {
			temp = tmp_victim->next;
			if ( ch->in_room != tmp_victim->in_room  &&
				 world[ch->in_room].zone == world[tmp_victim->in_room].zone){
				send_to_char("Zorro shouts, '민족의 특단!!!'", 
							 tmp_victim);
			}
		}
		act("$n에게 날벼락이 떨어집니다. !!!", FALSE,
						  vict, 0, 0, TO_ROOM);
		act("마른 하늘에서 날벼락이 떨어집니다!!!",FALSE,vict,0,0,TO_CHAR);
		send_to_char("\n\r  '번쩍'   \n\r", vict);
		send_to_char("\n\r으아악!!!\n\r\n\r", vict);
		send_to_char("*******\n\r", vict);
		send_to_char("******\n\r", vict);
		send_to_char("*****\n\r", vict);
		send_to_char("****\n\r", vict);
		send_to_char("***\n\r", vict);
		send_to_char("**\n\r", vict);
		send_to_char("*\n\r", vict);

		if(!affected_by_spell(vict,SPELL_CRUSH_ARMOR)){
			af.type      = SPELL_CRUSH_ARMOR;
			af.duration  = 4;
			af.modifier  = GET_LEVEL(ch)/10*(-1) ;
			af.location  = APPLY_DAMROLL;
			af.bitvector = 0;
			affect_to_char(vict, &af);
			af.location  = APPLY_AC;
			af.modifier  = GET_LEVEL(ch)+8;
			affect_to_char(vict, &af);
		}
		// 체력을 1/4로 줄이도록
		vict->points.hit-=
			vict->points.hit/3-number(1,vict->points.hit/8+GET_LEVEL(vict)/2);
		WAIT_STATE(vict, PULSE_VIOLENCE*2);
					
		return(TRUE);
    }
    else {      // 평상시에(안싸울때)
        struct obj_data *list;
        int briefcase = 0, key = 0;
        int newnum = 0;

        for(list=ch->carrying;list;list=list->next_content){
            if(obj_index[list->item_number].virtual==18014) briefcase = 1;
            if(obj_index[list->item_number].virtual==18015) key = 1;
        }

        if (briefcase && key) {  // 조로가 변신 아이템을 가지고 있는 경우
            // 가지고 있는 변신 item을 일단 없애고
            //  (일단 가지고 있는 걸 전부 없앰)
            while(list=ch->carrying){
                extract_obj(list);
            }

            // for(list=ch->carrying;list;list=list->next_content){
            //    if(obj_index[list->item_number].virtual==18014) extract_obj(list);
            //    if(obj_index[list->item_number].virtual==18015) extract_obj(list);
            //}

            // 이미 가지고 있던 item을 없앰
            //
            extract_obj(unequip_char(ch, HOLD));
            extract_obj(unequip_char(ch, WEAR_HEAD));


            // 새로운 아이템으로 갈아입음
            //
            
            // mask of zorro([18005]) - head
            if((newnum=real_object(18005))>=0) { 
                list=read_object(newnum,REAL);
                obj_to_char(list,ch);
                wear(ch,list,4);
            }
            // cloak of zorro([18006]) - about body
            if((newnum=real_object(18006))>=0) { 
                list=read_object(newnum,REAL);
                obj_to_char(list,ch);
                wear(ch,list,9);
            }
            // whip of zorro([18007]) - wield
            if((newnum=real_object(18007))>=0) { 
                list=read_object(newnum,REAL);
                obj_to_char(list,ch);
                wear(ch,list,12);
            }
            // handphone([18004]) - held
            if((newnum=real_object(18004))>=0) { 
                list=read_object(newnum,REAL);
                obj_to_char(list,ch);
                wear(ch,list,13);
            }
        }
        else if ( (key&&!briefcase) || (!key&&briefcase) ) {
            // 모자라는 경우
            switch (number(0,4)) {
            case 0:
                do_say(ch, "왜 이리 늦는거야....", 0);
                break;
            case 1:
                act("Zorro가 궁시렁댑니다.",1,ch,0,0,TO_ROOM);
                break;
            default:
            }
        }
        return TRUE;
    }
    
	return(0);
}

// 깨진 Item 수리
//
int smith(struct char_data *ch, int cmd, char *arg)
{
    struct char_data *tmp_victim, *temp, *vict, *mob;
	extern struct char_data *character_list;
    extern struct index_data *obj_index;
    struct affected_type af;
 	int real_number, h;
	char    buf[BUFSIZ];

    if (cmd) return 0;
    
    vict=ch->specials.fighting;
    if(vict){
        if ( ch->in_room != ch->specials.fighting->in_room) {
			stop_fighting(ch) ; /* he flee already */
            return FALSE;
		}
		return(TRUE);
    }
    else {      // 평상시에(안싸울때)
        struct obj_data *list,*particle;
        int stickum = 0;
        int num = 0, newnum, fix_num = 0;

        for(list=ch->carrying;list;list=list->next_content){
            if(obj_index[list->item_number].virtual==12015) stickum = 1;;
            if(list->obj_flags.type_flag ==ITEM_BROKEN) {
				num++;
			}
        }
		if (!stickum && num == 0) {
			return FALSE;
		}
		if (!stickum && num >= 1) {
			do_say(ch, "접착제가 필요합니다.....", 0);
			return FALSE;
		}
		else if (stickum && num == 0) {
			do_say(ch, "깨진 물품을 주세요 ....", 0);
			return FALSE;
		}

		/*
		if (num > 1) {
			do_say(ch, "수리할게 너무 많습니다. 하나만 주세요....", 0);
			for(list=ch->carrying;list;list=list->next_content){
				if(list->obj_flags.type_flag ==ITEM_BROKEN) {
					act("$n님이 $p 을 바닥에 가만히 내려 놓습니다.", TRUE, ch,
					            list, 0, TO_ROOM) ;
					obj_from_char(list);
					obj_to_room(list, ch->in_room);
				}
			}
			return FALSE;
		}
		*/
		if (GET_GOLD(ch) < 800000*num) {
			do_say(ch, "수리비가 부족합니다. 돈을 더 주세요....", 0);
			return FALSE;
		}

		// item 수리
		GET_GOLD(ch) -= 750000*num;

		for(list=ch->carrying;list;list=list->next_content){
			if(list->obj_flags.type_flag ==ITEM_BROKEN) {
				if (number(1,3) != 1) continue;
				if((newnum=real_object(list->obj_flags.value[1]))>=0) { 
					list=read_object(newnum,REAL);
					act("$n님이 $p 을 바닥에 가만히 내려 놓습니다.", TRUE, ch,
							list, 0, TO_ROOM) ;
					obj_to_room(list, ch->in_room);
					fix_num++;
				}
				else {
					log("Error: item 복구 code");
				}
			}
		}
		sprintf(buf, "%d개의 깨진 item중에서 %d개가 수리 가능했습니다..",
				num, fix_num);
		do_say(ch, buf, 0);

		// 나머지 가진 Item을 다 없앰
		while(list=ch->carrying){
			extract_obj(list);
		}

        return TRUE;
    }
    
	return(0);
}
