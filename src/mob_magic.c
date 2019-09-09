/* ************************************************************************
*  file: magic.c , Implementation of spells.              Part of DIKUMUD *
*  Usage : The actual effect of magic.                                    *
*  Copyright (C) 1990, 1991 - see 'license.doc' for complete information. *
************************************************************************* */

#include <stdio.h>
#include <string.h>
#include <assert.h>
#include "structs.h"
#include "utils.h"
#include "comm.h"
#include "spells.h"
#include "handler.h"
#include "limits.h"
#include "db.h"
#include "mob_magic.h"		/* by cyb */

/* Extern structures */
extern struct room_data *world;
extern struct obj_data  *object_list;
extern struct char_data *character_list;
extern struct index_data *mob_index;

/* Extern procedures */

void damage(struct char_data *ch, struct char_data *victim,
            int damage, int weapontype);
bool saves_spell(struct char_data *ch, sh_int spell);
// char *strdup(char *source);
int dice(int number, int size);
void hit(struct char_data *ch, struct char_data *victim, int type);
void stop_fighting(struct char_data *ch);
int number(int from, int to);
void log(char *str);

void mob_spell_fire_storm(byte level, struct char_data *ch,
  struct char_data *victim, struct obj_data *obj)
{
  int dam;
  struct char_data *tmp_victim, *temp;

  assert(ch);
  assert((level >=15) && (level <= 55)); 

  dam =  dice(level,4);
  send_to_char("The fire storm is flowing in the air!\n\r", ch);
act("$n makes the fire storm flowing in the air.\n\rYou can't see anything!\n\r"
    ,FALSE, ch, 0, 0, TO_ROOM);
  for(tmp_victim = character_list; tmp_victim; tmp_victim = temp) {
    temp = tmp_victim->next;
    if ( (ch->in_room == tmp_victim->in_room) && (ch != tmp_victim)
      && !IS_NPC(tmp_victim) ) {
      damage(ch, tmp_victim, dam, SPELL_FIRE_STORM);
    } else
      if (world[ch->in_room].zone == world[tmp_victim->in_room].zone)
        send_to_char("The fire storm is flowing in the atmosphere.\n\r",
          tmp_victim);
  }
}

void mob_light_move(struct char_data *ch, char *argument, int cmd)
{
  struct char_data *victim;
  if((victim=ch->specials.fighting)){
    hit(ch,victim,TYPE_UNDEFINED);
    if(ch->specials.fighting) stop_fighting(ch); 
    if(victim->specials.fighting) stop_fighting(victim);
    if(!IS_NPC(ch))
		WAIT_STATE(ch, PULSE_VIOLENCE*2/3);
    }
}

void mob_punch_drop(struct char_data *ch, struct char_data *victim) {
  int i;
  char buffer[MAX_STRING_LENGTH];
  struct obj_data *tmp_object;
  struct obj_data *next_obj;
  // bool test = FALSE;
  
  /* punch out equipments  */
  for (i=0; i< MAX_WEAR; i++) {
     if (((victim->equipment)[i]) && (number(10,100) < GET_LEVEL(ch)) ) {
       /* obj is removed form player */
       tmp_object = unequip_char(victim,i) ;
       if (CAN_SEE_OBJ(victim, tmp_object)) {
         sprintf(buffer, "%s is punched out.\n\r", fname(tmp_object->name));
         send_to_char(buffer, victim);
         }
       else {
         send_to_char("Something is punched out.\n\r", victim);
         }
       act("$n's $p flies in the sky for a while and falls.", 1, victim,
           tmp_object, 0, TO_ROOM);
       obj_to_room(tmp_object,ch->in_room);
       }
     }

      /* punch out carrying items..  */
      for(tmp_object = victim->carrying; tmp_object; tmp_object = next_obj) {
        next_obj = tmp_object->next_content;
        if ( GET_LEVEL(ch) > number(10,160) ) {
          /* item nodrop.. but.. can be punched also.. */
          if (CAN_SEE_OBJ(victim, tmp_object)) {
            sprintf(buffer, "%s is punched out.\n\r", fname(tmp_object->name));
            send_to_char(buffer, victim);
          } else {
            send_to_char("Something is punched out.\n\r", victim);
          }
          act("$n's $p flies in the sky for a while and falls.", 1, victim,
	    tmp_object, 0, TO_ROOM);
          obj_from_char(tmp_object);
          obj_to_room(tmp_object,ch->in_room);
          // test = TRUE;
        }
      }
}

/* mode and fightmode is defined in mob_magic.h  */
#define NUMBER_OF_MAX_VICTIM	8
#define VICT_IS_SAME_ROOM(mob)	(mob->in_room==mob->specials.fighting->in_room)
struct char_data *choose_victim(struct char_data *mob, int fightmode, int mode)
{
    int i, min, max, tmp, count, lev ;
    struct char_data *vict, *next_vict ;
    struct char_data *victims[NUMBER_OF_MAX_VICTIM] ;
    char buf[BUFSIZ] ;

  /* find the partner */
  count = 0 ;
  victims[0] = NULL ;
  for(vict=world[mob->in_room].people; vict && 
         count < NUMBER_OF_MAX_VICTIM ; vict = next_vict) {
    next_vict = vict->next_in_room ;
    if ( !IS_NPC(vict) ) {	/* is player */
       if ( vict -> specials.fighting ) {
          if (fightmode == VIC_FIGHTING || fightmode == VIC_ALL) {
            victims[count] = vict ;
            count ++ ;
            }
          }
       else {	/* not fighting */
          if (fightmode == VIC_WATCHING || fightmode == VIC_ALL) {
            victims[count] = vict ;
            count ++ ;
            }
          }
     }
   }

   if ( count == 0 ) {
     return NULL ;
     }

   /* now choose min or max.. */

   if ( mode == MODE_RANDOM ) {
      tmp = number(0, count-1) ;
      if ( tmp < 0 || tmp >= count ) log("number error in choose victim") ;
      sprintf(buf, "choose mob: count = %d, rnd # %d, (%s)", count, tmp, 
        victims[tmp]->player.name) ;
      log(buf) ;
      return ( victims[tmp] ) ;
      }

   if ( mode == MODE_HIT_MIN || mode == MODE_MANA_MIN || mode == MODE_MOVE_MIN
     || mode == MODE_AC_MIN || mode == MODE_HR_MIN || mode == MODE_DR_MIN ){
     /* find minimum */
     min = 32000 ;
     vict = victims[0] ;
     for ( i = 1 ; i < count ; i ++ ) {
       switch ( mode ) {
         case MODE_HIT_MIN  : tmp = GET_HIT(victims[i]) ; break ;
         case MODE_MANA_MIN : tmp = GET_MANA(victims[i]) ; break ;
         case MODE_MOVE_MIN : tmp = GET_MOVE(victims[i]) ; break ;
         case MODE_AC_MIN   : tmp = GET_AC(victims[i]) ; break ;
         case MODE_HR_MIN   : tmp = GET_DAMROLL(victims[i]) ; break ;
         case MODE_DR_MIN   : tmp = GET_HITROLL(victims[i]) ; break ;
         case MODE_LEVEL_MIN : tmp = GET_LEVEL(victims[i]) ; break ;
         default : tmp = GET_HIT(victims[i]) ;
         }
       if ( min > tmp) {
        min = tmp ;
        vict = victims[i] ;
        }
     }
   }
   else if ( mode == MODE_HIT_MAX || mode == MODE_MANA_MAX 
     || mode == MODE_MOVE_MAX || mode == MODE_AC_MAX
     || mode == MODE_HR_MAX || mode == MODE_DR_MAX ){
     /* find minimum */
     max = -3000 ;
     vict = victims[0] ;
     for ( i = 1 ; i < count ; i ++ ) {
       switch ( mode ) {
         case MODE_HIT_MAX  : tmp = GET_HIT(victims[i]) ; break ;
         case MODE_MANA_MAX : tmp = GET_MANA(victims[i]) ; break ;
         case MODE_MOVE_MAX : tmp = GET_MOVE(victims[i]) ; break ;
         case MODE_AC_MAX   : tmp = GET_AC(victims[i]) ; break ;
         case MODE_HR_MAX   : tmp = GET_DAMROLL(victims[i]) ; break ;
         case MODE_DR_MAX   : tmp = GET_HITROLL(victims[i]) ; break ;
         case MODE_LEVEL_MAX : tmp = GET_LEVEL(victims[i]) ; break ;
         default : tmp = GET_HIT(victims[i]) ;
         }
       if ( max < tmp) {
        max = tmp ;
        vict = victims[i] ;
        }
     }
   }
   else if ( mode == MODE_HIGH_LEVEL || mode == MODE_MID_LEVEL
     || mode == MODE_LOW_LEVEL ) {
     for ( i = 0; i < count; i++ ) {
        lev = GET_LEVEL(victims[i]) ;
        if ( mode == MODE_HIGH_LEVEL && lev >= 31 && lev < IMO )
            return ( victims[i] ) ;
        else if ( mode == MODE_MID_LEVEL && lev >= 13 && lev <= 30 )
            return ( victims[i] ) ;
        else if ( mode == MODE_LOW_LEVEL && lev >= 1 && lev <= 12 )
            return ( victims[i] ) ;
        }
      /* return NULL because thereis no one such that */
      return NULL ;
     }

    return (vict) ;
}
#undef NUMBER_OF_MAX_VICTIM

/*  choose mob routine for rescuer  */
#define NUMBER_OF_MAX_MOB	8
struct char_data *choose_rescue_mob(struct char_data *mob)
{
    int	tmp, count ;
    struct char_data *vict, *next_vict ;
    struct char_data *victims[NUMBER_OF_MAX_MOB] ;

  /* find the partner */
  count = 0 ;
  victims[count] = NULL ;
  for(vict=world[mob->in_room].people; vict && 
         count < NUMBER_OF_MAX_MOB ; vict = next_vict) {
    next_vict = vict->next_in_room ;
    if ( IS_NPC(vict) ) {	/* is mob */
       if ( vict -> specials.fighting && (mob != vict) ) {
             victims[count] = vict ;
             count ++ ;
             }
       }
    }

   if ( count == 0 ) {	/* there is no mob fighting except me.. */
      if ( mob->specials.fighting )  return mob ;
      else return NULL ;
      }

   /* random select  */
   tmp = number(0, count-1) ;
   if ( tmp >= 0 && tmp < count )
      return ( victims[tmp] ) ;
   else
      return NULL ;
}
#undef NUMBER_OF_MAX_MOB
