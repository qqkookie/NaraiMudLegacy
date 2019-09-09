/* ************************************************************************
*  file: spec_procs2.c , Special module.                  Part of DIKUMUD *
*  Usage: Procedures handling special procedures for object/room/mobile   *
*  Made by Choi Yeon Beom in KAIST                                        *
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
#include "mob_magic.h"		/* by cyb */

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
void stop_fighting(struct char_data *ch);
void set_title(struct char_data *ch);
int number(int from, int to);
int dice(int num, int size);


void cast_cure_light(byte level,struct char_data *ch,char *arg,int type,
    struct char_data *victim, struct obj_data *tar_obj );
void cast_cure_critic(byte level,struct char_data *ch,char *arg,int type,
    struct char_data *victim, struct obj_data *tar_obj );
void cast_heal(byte level,struct char_data *ch,char *arg,int type,
    struct char_data *victim, struct obj_data *tar_obj );
void cast_full_heal(byte level,struct char_data *ch,char *arg,int type,
    struct char_data *victim, struct obj_data *tar_obj );

int level_gate(struct char_data *ch, int cmd, char *arg)
{
  char buf[100];
  int f,r;

  if ( cmd==84) {
        send_to_char("You cannot do that here.\n\r",ch);
        sprintf(buf,"%s attempts to misbehave here.\n\r",ch->player.name);
        send_to_room_except(buf,ch->in_room,ch);
        return TRUE;
  }

  f = 0;        r=world[ch->in_room].number;
  switch ( cmd ) {
        case 1 :	/* north */
        case 2 :	/* east */
               break;
        case 3 :	/* south */
                if(r==1453) f=(GET_LEVEL(ch) < 12);
                else if(r==3041) f=(GET_LEVEL(ch) < 25);
                else if(r==2535) f=(GET_LEVEL(ch) > 15);
                else if(r==6001) f=(GET_LEVEL(ch) > 12);
                else if(r==9400) f=(GET_LEVEL(ch) > 3);
                break;
        case 4 :	/* west */
                if(r==3500) f=(GET_LEVEL(ch) > 20);
                else if(r==5200) f=(GET_LEVEL(ch) > 24 || GET_LEVEL(ch) < 10);
                break;
        case 5 :	/* up */
        case 6 :	/* down */
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

  if(cmd != 6)   /* specific to Room 2707. cmd 6 is move down */
    return(FALSE);
  switch(number(1,4)) {
    case 1:  loc_nr = 2720;
             break ;
    case 2:  loc_nr = 2721;
             break ;
    case 3:  loc_nr = 2722;
             break ;
    case 4:  loc_nr = 2723;
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
      if ((100 * GET_HIT(vict)/GET_PLAYER_MAX_HIT(vict)) > 85)
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

int electric_shock(struct char_data *ch, int cmd, char *arg)
{
  extern struct weather_data weather_info;
  char *msg;
  int weather, shock ;

  shock = 0 ;
  weather = weather_info.sky ;
  if ( cmd >= 1 && cmd <= 6 && !IS_NPC(ch)) {
    switch (weather) {
      case SKY_RAINING :
             msg = "ZZirrrr... 비가 오니까 찌릿 합니다.\n\r" ;
             shock = 5 ;
             break ;
      case SKY_LIGHTNING :
             msg = "ZZirk ZZirk .. 이런 날씨에는 감전되어 죽겠습니다.\n\r" ;
             shock = dice(5,7) ;
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
        act("$n utters the words 'ecuder evom'.", 1, ch, 0, 0, TO_ROOM);
        if ((tmp=GET_MOVE(vict)) > 0 ) {
           send_to_char("갑자기 힘이 빠집니다.\n\r", vict) ;
           GET_MOVE(vict) = tmp/3 + dice(3, tmp/10) ;
           }
        }
      }
    else {
      vict = choose_victim(ch, VIC_ALL, MODE_MANA_MAX) ;
      if ( vict ) {
        act("$n utters the words 'ecuder anam'.", 1, ch, 0, 0, TO_ROOM);
        if ((tmp=GET_MANA(vict)) > 0 ) {
           send_to_char("갑자기 힘이 빠집니다.\n\r", vict) ;
           GET_MANA(vict) = tmp/3 + dice(3, tmp/10) ;
           }
        }
      }
   }

  return TRUE;
}

int string_machine(struct char_data *ch, int cmd, char *arg)
{
	struct obj_data *obj;
	char buf[256];

	/* change */
	if(cmd != 306) return 0;

	if(GET_GOLD(ch) < 10000000){
		act("You should earn money for this service.",
			FALSE, ch, 0, 0, TO_CHAR);
		return 1;
	}

	arg = one_argument(arg, buf);
	obj = get_obj_in_list_vis(ch, buf, ch->carrying);
	if(!obj){
		act("You do not have that item.", FALSE, ch, 0, 0, TO_CHAR);
		return 1;
	}

	/* remove space */
	while(*arg == ' ') arg ++;
	if(!*arg){	/* null string */
		act("You should propose the name.", FALSE, ch, 0, 0, TO_CHAR);
		return 1;
	}

	free(obj->short_description);
	CREATE(obj->short_description, char, strlen(arg) + 1);
	strcpy(obj->short_description, arg);

	GET_GOLD(ch) -= 10000000;

	act("STRING Service DONE.\n\r", FALSE, ch, 0, 0, TO_CHAR);

	return 1;
}

int slot_machine(struct char_data *ch, int cmd, char *arg)
{
	int num[3];
	int same;
	char buf[256];

	if(cmd != 305) return 0;

	/* no mob */
	if(IS_NPC(ch)) return 1;

	/* no gold */
	if(GET_GOLD(ch) < 1000000){
		act("You should have money!!!", FALSE, ch, 0, 0, TO_CHAR);
		return 1;
	}

	num[0] = number(0,9);
	num[1] = number(0,9);
	num[2] = number(0,9);

	same = 0;
	same += (num[0] == num[1]);
	same += (num[1] == num[2]);
	same += (num[2] == num[0]);

	sprintf(buf, "Your dices are %d, %d, %d.", num[0], num[1], num[2]);
	act(buf, FALSE, ch, 0, 0, TO_CHAR);

	if(same == 0){
		act("You lost your coins.", FALSE, ch, 0, 0, TO_CHAR);
		GET_GOLD(ch) -= 1000000;
	}
	else if(same == 1){
		act("Good!!! You win this turn!!!", FALSE, ch, 0, 0, TO_CHAR);
		GET_GOLD(ch) += 2000000;
	}
	else {
		sprintf(buf, "Excellent!!! %s made JACKPOT!!!\n\r", GET_NAME(ch));
		act(buf, FALSE, ch, 0, 0, TO_ROOM);
		act(buf, FALSE, ch, 0, 0, TO_CHAR);
		/*
		send_to_all(buf);
		*/
		GET_GOLD(ch) += 20000000;
	}

	return 1;
}
