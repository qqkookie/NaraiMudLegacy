/* ************************************************************************
*  file: act.informative.c , Implementation of commands.  Part of DIKUMUD *
*  Usage : Informative commands.                                          *
*  Copyright (C) 1990, 1991 - see 'license.doc' for complete information. *
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
#include "memory.h"
#include "command.h"
#include "quest.h"

/* extern variables */

extern struct room_data *world;
extern struct descriptor_data *descriptor_list;
extern struct char_data *character_list;
extern struct obj_data *object_list;
extern char credits[MAX_STRING_LENGTH];
extern char news[MAX_STRING_LENGTH];
extern char info[MAX_STRING_LENGTH];
extern char wizards[MAX_STRING_LENGTH];	/* cyb */
extern char *dirs[]; 
extern char *where[];
extern char *color_liquid[];
extern char *fullness[];
extern char *spells[];

/* extern functions */

struct time_info_data age(struct char_data *ch);
void page_string(struct descriptor_data *d, char *str, int keep_internal);

/* intern functions */

void list_obj_to_char(struct obj_data *list,struct char_data *ch, int mode,
                      bool show);


/* Procedures related to 'look' */

void argument_split_2(char *argument, char *first_arg, char *second_arg) {
    int look_at, found, begin;
    found = begin = 0;

    /* Find first non blank */
    for ( ;*(argument + begin ) == ' ' ; begin++);

    /* Find length of first word */
    for (look_at=0; !isspace(*(argument+begin+look_at)) ||
             is_hangul(*(argument+begin+look_at)) ; look_at++) {
        /* Make all letters lower case, AND copy them to first_arg */
        if (is_hangul(*(argument + begin + look_at))) /* hangul */
            *(first_arg + look_at) = *(argument + begin + look_at);
        else
            *(first_arg + look_at) = LOWER(*(argument + begin + look_at));
    }
    *(first_arg + look_at) = '\0';
    begin += look_at;

    /* Find first non blank */
    for ( ;*(argument + begin ) == ' ' ; begin++);

    /* Find length of second word */
    for ( look_at=0; !isspace(*(argument+begin+look_at)) ||
              is_hangul(*(argument+begin+look_at)); look_at++) {
        /* Make all letters lower case, AND copy them to second_arg */
        if (is_hangul(*(argument + begin + look_at)))   /* hangul */
            *(second_arg + look_at) = *(argument + begin + look_at);
        else
            *(second_arg + look_at) = LOWER(*(argument + begin + look_at));
    }
    *(second_arg + look_at)='\0';
    begin += look_at;
}

struct obj_data *get_object_in_equip_vis(struct char_data *ch,
                                         char *arg, struct obj_data *equipment[], int *j) {

    for ((*j) = 0; (*j) < MAX_WEAR ; (*j)++)
        if (equipment[(*j)])
            if (CAN_SEE_OBJ(ch,equipment[(*j)]))
                if (isname(arg, equipment[(*j)]->name))
                    return(equipment[(*j)]);

    return (0);
}

char *find_ex_description(char *word, struct extra_descr_data *list)
{
    struct extra_descr_data *i;

    for (i = list; i; i = i->next)
        if (isname(word,i->keyword))
            return(i->description);
    return(0);
}

void show_obj_to_char(struct obj_data *object, struct char_data *ch, int mode)
{
    char buffer[MAX_STRING_LENGTH+128] ;
    char *temp_desc;
    struct obj_data *i;
    int temp;
    bool found;

    buffer[0] = '\0';
    if ((mode == 0) && object->description)
        strcpy(buffer,object->description);
    else   if (object->short_description && ((mode == 1) ||
                                             (mode == 2) || (mode==3) || (mode == 4))) 
        strcpy(buffer,object->short_description);
    else if (mode == 5) {
        if (object->obj_flags.type_flag == ITEM_NOTE)
        {
            if (object->action_description)
            {
                strcpy(buffer, "There is something written upon it:\n\r\n\r");
                strcat(buffer, object->action_description);
                page_string(ch->desc, buffer, 1);
            }
            else
                acthan("It's blank.", "비어 있습니다.", FALSE, ch,0,0,TO_CHAR);
            return;
        }
        else if((object->obj_flags.type_flag != ITEM_DRINKCON))
        {
            strcpy(buffer,"You see nothing special..");
        }
        else /* ITEM_TYPE == ITEM_DRINKCON */
        {
            strcpy(buffer, "It looks like a drink container.");
        }
    }

    if (mode != 3) { 
        found = FALSE;
        if (IS_OBJ_STAT(object,ITEM_INVISIBLE)) {
            strcat(buffer,"(invisible)");
            found = TRUE;
        }
        if (IS_OBJ_STAT(object,ITEM_EVIL) && IS_AFFECTED(ch,AFF_DETECT_ALIGN)){
            strcat(buffer,"..It glows red!");
            found = TRUE;
        }
        if (IS_OBJ_STAT(object,ITEM_GLOW)) {
            strcat(buffer,"..It has a soft glowing aura!");
            found = TRUE;
        }
        if (IS_OBJ_STAT(object,ITEM_HUM)) {
            strcat(buffer,"..It emits a faint humming sound!");
            found = TRUE;
        }
    }

    strcat(buffer, "\n\r");
    page_string(ch->desc, buffer, 1);

/*
  if (((mode == 2) || (mode == 4)) && (GET_ITEM_TYPE(object) == 
  ITEM_CONTAINER)) {
  strcpy(buffer,"The ");
  strcat(buffer,fname(object->name));
  strcat(buffer," contains:\n\r");
  send_to_char(buffer, ch);
  if (mode == 2) list_obj_to_char(object->contains, ch, 1,TRUE);
  if (mode == 4) list_obj_to_char(object->contains, ch, 3,TRUE);
  }
*/
}

void list_obj_to_char(struct obj_data *list,struct char_data *ch, int mode, 
                      bool show) {
    struct obj_data *i;
    bool found;

    found = FALSE;
    for ( i = list ; i ; i = i->next_content ) { 
        if (CAN_SEE_OBJ(ch,i)) {
            show_obj_to_char(i, ch, mode);
            found = TRUE;
        }    
    }  
    if ((! found) && (show)) send_to_char("Nothing\n\r", ch);
}

void show_char_to_char(struct char_data *i, struct char_data *ch, int mode)
{
    char buffer[MAX_STRING_LENGTH];
    int j, found, percent;
    struct obj_data *tmp_obj;
	int class;

    if (mode == 0) {
        if (IS_AFFECTED(i, AFF_HIDE) || !CAN_SEE(ch,i)) {
            if (IS_AFFECTED(ch, AFF_SENSE_LIFE))
                if(GET_LEVEL(i) < (IMO+2))
                    send_to_char("You sense a hidden life form in the room.\n\r", ch);
            return;
        }
        if (!(i->player.long_descr)||(GET_POS(i) != i->specials.default_pos)){
            if (IS_AFFECTED(i,AFF_INVISIBLE))
                strcpy(buffer,"*");
            else
                *buffer = '\0';

            if (IS_AFFECTED(ch, AFF_DETECT_ALIGN)) {
                if (IS_EVIL(i))
                    strcat(buffer, "(E)");
                if (IS_GOOD(i))
                    strcat(buffer, "(G)");
            }

            if (!IS_NPC(i)) {  
                strcpy(buffer,GET_NAME(i));
                strcat(buffer," ");
                if(GET_TITLE(i))
                    strcat(buffer,GET_TITLE(i));
            } else {
                strcpy(buffer, i->player.short_descr);
                CAP(buffer);
            }
			/*
            if(IS_AFFECTED(i,AFF_INVISIBLE))
                strcat(buffer," (invisible)");
			*/
            switch(GET_POS(i)) {
            case POSITION_STUNNED  : 
                strcat(buffer," is lying here, stunned."); break;
            case POSITION_INCAP    : 
                strcat(buffer," is lying here, incapacitated."); break;
            case POSITION_MORTALLYW: 
                strcat(buffer," is lying here, mortally wounded."); break;
            case POSITION_DEAD     : 
                strcat(buffer," is lying here, dead."); break;
            case POSITION_STANDING : 
                strcat(buffer," is standing here."); break;
            case POSITION_SITTING  : 
                strcat(buffer," is sitting here.");  break;
            case POSITION_RESTING  : 
                strcat(buffer," is resting here.");  break;
            case POSITION_SLEEPING : 
                strcat(buffer," is sleeping here."); break;
            case POSITION_FIGHTING :
                if (i->specials.fighting) {

                    strcat(buffer," is here, fighting ");
                    if (i->specials.fighting == ch)
                        strcat(buffer," YOU!");
                    else {
                        if (i->in_room == i->specials.fighting->in_room)
                            if (IS_NPC(i->specials.fighting))
                                strcat(buffer, i->specials.fighting->player.short_descr);
                            else
                                strcat(buffer, GET_NAME(i->specials.fighting));
                        else
                            strcat(buffer, "someone who has already left.");
                    }
                } else /* NIL fighting pointer */
                    strcat(buffer," is here struggling with thin air.");
                break;
            default : strcat(buffer," is floating here."); break;
            }
			/*
            if (IS_AFFECTED(ch, AFF_DETECT_ALIGN)) {
                if (IS_EVIL(i))
                    strcat(buffer, " (Red)");
                if (IS_GOOD(i))
                    strcat(buffer, "(Blue)");
            }
			*/

            strcat(buffer,"\n\r");
            send_to_char(buffer, ch);
        }
        else  /* npc with long */
        {
            if (IS_AFFECTED(i,AFF_INVISIBLE))
                strcpy(buffer,"*");
            else
                *buffer = '\0';

            if (IS_AFFECTED(ch, AFF_DETECT_ALIGN)) {
                if (IS_EVIL(i))
                    strcat(buffer, "(E)");
                if (IS_GOOD(i))
                    strcat(buffer, "(G)");
            }

            strcat(buffer, i->player.long_descr);

            send_to_char(buffer, ch);
        }
              
        if (IS_AFFECTED(i,AFF_SANCTUARY))
            act("$n glows with a bright light!", FALSE, i, 0, ch, TO_VICT);

    } else if (mode == 1) {

        if (i->player.description)
            send_to_char(i->player.description, ch);
        else {
            act("You see nothing special about $m.", FALSE, i, 0, ch, TO_VICT);
        }

        /* Show a character to another */

        if (GET_MAX_HIT(i) > 0)
            percent = (100*GET_HIT(i))/GET_MAX_HIT(i);
        else
            percent = -1; /* How could MAX_HIT be < 1?? */

        if (IS_NPC(i))
            strcpy(buffer, i->player.short_descr);
        else
            strcpy(buffer, GET_NAME(i));

        if (percent >= 100)
            strcat(buffer, " is in an excellent condition.\n\r");
        else if (percent >= 90)
            strcat(buffer, " has a few scratches.\n\r");
        else if (percent >= 75)
            strcat(buffer, " has some small wounds and bruises.\n\r");
        else if (percent >= 50)
            strcat(buffer, " has quite a few wounds.\n\r");
        else if (percent >= 30)
            strcat(buffer, " has some big nasty wounds and scratches.\n\r");
        else if (percent >= 15)
            strcat(buffer, " looks pretty hurt.\n\r");
        else if (percent >= 0)
            strcat(buffer, " is in an awful condition.\n\r");
        else
            strcat(buffer, " is bleeding awfully from big wounds.\n\r");

        send_to_char(buffer, ch);

        found = FALSE;
        for (j=0; j< MAX_WEAR; j++) {
            if (i->equipment[j]) {
                if (CAN_SEE_OBJ(ch,i->equipment[j])) {
                    found = TRUE;
                }
            }
        }
        if (found) {
            act("\n\r$n is using:", FALSE, i, 0, ch, TO_VICT);
            for (j=0; j< MAX_WEAR; j++) {
                if (i->equipment[j]) {
                    if (CAN_SEE_OBJ(ch,i->equipment[j])) {
                        send_to_char(where[j],ch);
                        show_obj_to_char(i->equipment[j],ch,1);
                    }
                }
            }
        }
		if (is_solved_quest(ch, QUEST_REMORTAL)) class = GET_CLASS_O(ch);
		else class = GET_CLASS(ch);

        if (((class==CLASS_THIEF)&&(ch != i))||(GET_LEVEL(ch)>=IMO)) {
            found = FALSE;
            send_to_char("\n\rYou attempt to peek at the inventory:\n\r", ch);
            for(tmp_obj = i->carrying; tmp_obj; tmp_obj = tmp_obj->next_content) {
                if (CAN_SEE_OBJ(ch, tmp_obj) && (number(0,20) < GET_LEVEL(ch))) {
                    show_obj_to_char(tmp_obj, ch, 1);
                    found = TRUE;
                }
            }
            if (!found)
                send_to_char("You can't see anything.\n\r", ch);
        }

    } else if (mode == 2) {

        /* Lists inventory */
        act("$n is carrying:", FALSE, i, 0, ch, TO_VICT);
        list_obj_to_char(i->carrying,ch,1,TRUE);
    }
}



void list_char_to_char(struct char_data *list, struct char_data *ch, 
                       int mode) {
    struct char_data *i;

    for (i = list; i ; i = i->next_in_room) {
        if ( (ch!=i) && (IS_AFFECTED(ch, AFF_SENSE_LIFE) ||
                         (CAN_SEE(ch,i) && !IS_AFFECTED(i, AFF_HIDE))) )
            show_char_to_char(i,ch,0); 
    } 
}

void do_look(struct char_data *ch, char *argument, int cmd)
{
	void show_autoexit(struct char_data *ch) ;
    char buffer[MAX_STRING_LENGTH];
    char arg1[MAX_STRING_LENGTH];
    char arg2[MAX_STRING_LENGTH];
    int keyword_no;
    int j, bits, temp;
    bool found;
    struct obj_data *tmp_object, *found_object;
    struct char_data *tmp_char;
    char *tmp_desc;
    static char *keywords[]= { 
        "north",
        "east",
        "south",
        "west",
        "up",
        "down",
        "in",
        "at",
        "",  /* Look at '' case */
        "\n" };

    if (!ch->desc)
        return;

    if (GET_POS(ch) < POSITION_SLEEPING)
        send_to_char_han("You can't see anything but stars!\n\r",
                         "별들이 아름답게 빛나고 있습니다.\n\r",  ch);
    else if (GET_POS(ch) == POSITION_SLEEPING)
        send_to_char_han("You can't see anything, you're sleeping!\n\r",
                         "아마도 지금 꿈을 꾸는게 아닌지요 ??\n\r", ch);
    else if ( IS_AFFECTED(ch, AFF_BLIND) )
        send_to_char_han("You can't see a damn thing, you're blinded!\n\r",
                         "앞을 볼 수가 없습니다! 눈이 멀었습니다!\n\r",  ch);
    else if ( IS_DARK(ch->in_room) && !OMNI(ch) && !IS_AFFECTED(ch, AFF_INFRAVISION) )
        send_to_char_han("It is pitch black...\n\r", "너무 깜깜합니다..\n\r",ch);
    else {
        argument_split_2(argument,arg1,arg2);
        keyword_no = search_block(arg1, keywords, FALSE); /* Partial Match */

        if ((keyword_no == -1) && *arg1) {
            keyword_no = 7;
            strcpy(arg2, arg1); /* Let arg2 become the target object (arg1) */
        }

        found = FALSE;
        tmp_object = 0;
        tmp_char   = 0;
        tmp_desc   = 0;

        switch(keyword_no) {
            /* look <dir> */
        case 0 :
        case 1 :
        case 2 : 
        case 3 : 
        case 4 :
        case 5 : {   

            if (EXIT(ch, keyword_no)) {

                if (EXIT(ch, keyword_no)->general_description) {
                    send_to_char(EXIT(ch, keyword_no)->
                                 general_description, ch);
                } else {
                    send_to_char_han("You see nothing special.\n\r",
                                     "별다른 것이 없습니다.\n\r", ch);
                }

                if (IS_SET(EXIT(ch, keyword_no)->exit_info, EX_CLOSED) && 
                    (EXIT(ch, keyword_no)->keyword)) {
                    sprintf(buffer, "The %s is closed.\n\r",
                            fname(EXIT(ch, keyword_no)->keyword));
                    send_to_char(buffer, ch);
                }  else {
                    if (IS_SET(EXIT(ch, keyword_no)->exit_info, EX_ISDOOR) &&
                        EXIT(ch, keyword_no)->keyword) {
                        sprintf(buffer, "The %s is open.\n\r",
                                fname(EXIT(ch, keyword_no)->keyword));
                        send_to_char(buffer, ch);
                    }
                }
            } else {
                send_to_char_han("Nothing special there...\n\r",
                                 "거기엔 특별한 것이 없습니다 ... \n\r", ch);
            }
        }
        break;

        /* look 'in'  */
        case 6: {
            if (*arg2) {
                /* Item carried */

                bits = generic_find(arg2, FIND_OBJ_INV | FIND_OBJ_ROOM |
                                    FIND_OBJ_EQUIP, ch, &tmp_char, &tmp_object);

                if (bits) { /* Found something */
                    if (GET_ITEM_TYPE(tmp_object)== ITEM_DRINKCON)
                    {
                        if (tmp_object->obj_flags.value[1] <= 0) {
                            acthan("It is empty.", "비어 있습니다.", FALSE, ch,0,0,TO_CHAR);
                        } else {
                            temp=((tmp_object->obj_flags.value[1]*3)/tmp_object->obj_flags.value[0]);
                            sprintf(buffer,"It's %sfull of a %s liquid.\n\r",
                                    fullness[temp],color_liquid[tmp_object->obj_flags.value[2]]);
                            send_to_char(buffer, ch);
                        }
                    } else if (GET_ITEM_TYPE(tmp_object) == ITEM_CONTAINER) {
                        if (!IS_SET(tmp_object->obj_flags.value[1],CONT_CLOSED)) {
                            send_to_char(fname(tmp_object->name), ch);
                            switch (bits) {
                            case FIND_OBJ_INV :
                                send_to_char(" (carried) : \n\r", ch);
                                break;
                            case FIND_OBJ_ROOM :
                                send_to_char(" (here) : \n\r", ch);
                                break;
                            case FIND_OBJ_EQUIP :
                                send_to_char(" (used) : \n\r", ch);
                                break;
                            }
                            list_obj_to_char(tmp_object->contains, ch, 2, TRUE);
                        }
                        else
                            send_to_char("It is closed.\n\r", ch);
                    } else {
                        send_to_char("That is not a container.\n\r", ch);
                    }
                } else { /* wrong argument */
                    send_to_char("You do not see that item here.\n\r", ch);
                }
            } else { /* no argument */
                send_to_char("Look in what?!\n\r", ch);
            }
        }
        break;

        /* look 'at'  */
        case 7 : {


            if (*arg2) {

                bits = generic_find(arg2, FIND_OBJ_INV | FIND_OBJ_ROOM |
                                    FIND_OBJ_EQUIP | FIND_CHAR_ROOM, ch, &tmp_char, &found_object);

                if (tmp_char) {
                    show_char_to_char(tmp_char, ch, 1);
                    if (ch != tmp_char) {
                        acthan("$n looks at you.", "$n님이 당신을 바라봅니다.",
                               TRUE, ch, 0, tmp_char, TO_VICT);
                        acthan("$n looks at $N.", "$n님이 $N님을 바라봅니다.",
                               TRUE, ch, 0, tmp_char, TO_NOTVICT);
                    }
                    return;
                }


                /* Search for Extra Descriptions in room and items */

                /* Extra description in room?? */
                if (!found) {
                    tmp_desc = find_ex_description(arg2, 
                                                   world[ch->in_room].ex_description);
                    if (tmp_desc) {
                        page_string(ch->desc, tmp_desc, 0);
                        return; /* RETURN SINCE IT WAS A ROOM DESCRIPTION */
                        /* Old system was: found = TRUE; */
                    }
                }
                /* Search for extra descriptions in items */
                /* Equipment Used */
                if (!found) {
                    for (j = 0; j< MAX_WEAR && !found; j++) {
                        if (ch->equipment[j]) {
                            if (CAN_SEE_OBJ(ch,ch->equipment[j])) {
                                tmp_desc = find_ex_description(arg2, 
                                                               ch->equipment[j]->ex_description);
                                if (tmp_desc) {
                                    page_string(ch->desc, tmp_desc, 1);
                                    found = TRUE;
                                }
                            }
                        }
                    }
                }
                /* In inventory */
                if (!found) {
                    for(tmp_object = ch->carrying; 
                        tmp_object && !found; 
                        tmp_object = tmp_object->next_content) {
                        if CAN_SEE_OBJ(ch, tmp_object) {
                            tmp_desc = find_ex_description(arg2, 
                                                           tmp_object->ex_description);
                            if (tmp_desc) {
                                page_string(ch->desc, tmp_desc, 1);
                                found = TRUE;
                            }
                        }
                    }
                }

                /* Object In room */

                if (!found) {
                    for(tmp_object = world[ch->in_room].contents; 
                        tmp_object && !found; 
                        tmp_object = tmp_object->next_content) {
                        if CAN_SEE_OBJ(ch, tmp_object) {
                            tmp_desc = find_ex_description(arg2, 
                                                           tmp_object->ex_description);
                            if (tmp_desc) {
                                page_string(ch->desc, tmp_desc, 1);
                                found = TRUE;
                            }
                        }
                    }
                }
                /* wrong argument */

                if (bits) { /* If an object was found */
                    if (!found)
                        show_obj_to_char(found_object, ch, 5); /* Show no-description */
                    else
                        show_obj_to_char(found_object, ch, 6); /* Find hum, glow etc */
                } else if (!found) {
                    send_to_char_han("You do not see that here.\n\r",
                                     "그런 것은 여기에 없습니다.\n\r", ch);
                }
            } else {
                /* no argument */

                send_to_char_han("Look at what?\n\r",
                                 "무엇을 바라보라고요 ?\n\r", ch);
            }
        }
        break;


        /* look ''    */ 
        case 8 : {
            send_to_char(world[ch->in_room].name, ch);
            send_to_char("\n\r", ch);
            if (!IS_SET(ch->specials.act, PLR_BRIEF))
                send_to_char(world[ch->in_room].description, ch);
			if (IS_SET(ch->specials.act, PLR_AUTOEXIT))
				show_autoexit(ch) ;
            list_obj_to_char(world[ch->in_room].contents, ch, 0,FALSE);
            list_char_to_char(world[ch->in_room].people, ch, 0);
        }
        break;

        /* wrong arg  */
        case -1 : 
            send_to_char("Sorry, I didn't understand that!\n\r", ch);
            break;
        }
    }
}

/* end of look */

void show_autoexit(struct char_data *ch)
{
	int	door;
	char	buf[BUFSIZ], buf2[BUFSIZ] ;
	static char	*exits[] = {  
		"n ", "e ", "s ",
		"w ", "u ", "d "
    };
	static char	*closedexits[] = {  
		"(n) ", "(e) ", "(s) ",
		"(w) ", "(u) ", "(d) "
    };

	buf2[0] = 0 ;
	for (door = 0; door < 6; door++)
		if (EXIT(ch, door))
			if (EXIT(ch, door)->to_room != NOWHERE &&
				!IS_SET(EXIT(ch, door)->exit_info, EX_CLOSED))
				strcat(buf2, exits[door]) ;
			else if (EXIT(ch, door)->to_room != NOWHERE &&
                     IS_SET(EXIT(ch, door)->exit_info, EX_CLOSED))
				strcat(buf2, closedexits[door]) ;

	if (*buf2) {
		sprintf(buf, "< Exits: %s>\n\r", buf2) ;
		send_to_char(buf, ch);
    }
	else
		send_to_char("\n\r< Exits: None >\n\r", ch);
}

void do_bank(struct char_data *ch, char *argument, int cmd)
{
    send_to_char("You can only do that at the bank.\n\r",ch);
}


void do_read(struct char_data *ch, char *argument, int cmd)
{
    char buf[100];

    /* This is just for now - To be changed later.! */
    sprintf(buf,"at %s",argument);
    do_look(ch,buf,15);
}



void do_examine(struct char_data *ch, char *argument, int cmd)
{
    char name[100], buf[100];
    int bits;
    struct char_data *tmp_char;
    struct obj_data *tmp_object;

    one_argument(argument, name);

    if (!*name)
    {
        send_to_char("Examine what?\n\r", ch);
        return;
    }

    sprintf(buf,"at %s",argument);
    do_look(ch,buf,COM_LOOK);

    bits = generic_find(name, FIND_OBJ_INV | FIND_OBJ_ROOM |
                        FIND_OBJ_EQUIP, ch, &tmp_char, &tmp_object);

    if (tmp_object) {
        if ((GET_ITEM_TYPE(tmp_object)==ITEM_DRINKCON) ||
            (GET_ITEM_TYPE(tmp_object)==ITEM_CONTAINER)) {
            send_to_char("When you look inside, you see:\n\r", ch);
            sprintf(buf,"in %s",argument);
            do_look(ch,buf,COM_LOOK);
        } else {
            if(GET_ITEM_TYPE(tmp_object)==ITEM_FIREWEAPON){
                sprintf(buf,"There are %d shots left.\n\r",
                        tmp_object->obj_flags.value[0]);
                send_to_char(buf,ch);
            }
        }
    }
}

void do_report(struct char_data *ch, char *argument, int cmd)
{
    char buf[80];
    sprintf(buf,"%s %d/%d hp, %d/%d mn, %d/%d mv", GET_NAME(ch),
            GET_HIT(ch),GET_MAX_HIT(ch),GET_MANA(ch),GET_MAX_MANA(ch),
            GET_MOVE(ch),GET_MAX_MOVE(ch) );
    act(buf,FALSE,ch,0,0,TO_ROOM);
    send_to_char("ok.\n\r",ch);
}

void do_title(struct char_data *ch, char *argument, int cmd)
{
    char buf[100];
    if( *argument == 0 ) {
        sprintf(buf, "You are %s %s\n\r", GET_NAME(ch), GET_TITLE(ch));
        send_to_char(buf, ch);
        return;
    }
    if (GET_TITLE(ch)) {
        /*
          recreate(GET_TITLE(ch),char,strlen(argument));
        */
		free_string_type(GET_TITLE(ch)) ;
		GET_TITLE(ch) = malloc_string_type(strlen(argument)) ;
	}
    else {
        /*
          create(GET_TITLE(ch),char,strlen(argument));
        */
		GET_TITLE(ch) = malloc_string_type(strlen(argument)) ;
    }

	if (GET_TITLE(ch))
		strcpy(GET_TITLE(ch), argument+1);
}

void do_exits(struct char_data *ch, char *argument, int cmd)
{
    int door;
    char buf[MAX_STRING_LENGTH];
    static char *exits[] = {  
	    "North",
	    "East ",
	    "South",
	    "West ",
	    "Up   ",
	    "Down "
    };

    *buf = '\0';

    for (door = 0; door <= 5; door++)
        if (EXIT(ch, door))
            if (EXIT(ch, door)->to_room != NOWHERE &&
                !IS_SET(EXIT(ch, door)->exit_info, EX_CLOSED))
                if (IS_DARK(EXIT(ch, door)->to_room) && !OMNI(ch))
                    sprintf(buf + strlen(buf), "%s - Too dark to tell\n\r", exits[door]);
                else
                    sprintf(buf + strlen(buf), "%s - %s\n\r", exits[door],
                            world[EXIT(ch, door)->to_room].name);

    send_to_char("Obvious exits:\n\r", ch);

    if (*buf)
        send_to_char(buf, ch);
    else
        send_to_char("None.\n\r", ch);
}

static char ac_msg[13][44] = {
    "You are naked.\n\r",
    "You are almost naked.\n\r",
    "You are barely covered.\n\r",
    "You are somewhat covered.\n\r",
    "You are pretty covered.\n\r",
    "You are pretty well covered.\n\r",
    "You are pretty well armored.\n\r",
    "You are heavily armored.\n\r",
    "You are very heavily armored.\n\r",
    "You are extremely heavily armored.\n\r",
    "You are almost an armored tank.\n\r",
    "You are an armored tank.\n\r",
    "You are a walking fortress.\n\r"
};

static char align_msg[13][44] = {
    "You are a saint.\n\r",
    "You feel like being a saint.\n\r",
    "You are good.\n\r",
    "You are slightly good.\n\r",
    "You are almost good.\n\r",
    "You are going to be good.\n\r",
    "You are neutral.\n\r",
    "You are going to be evil.\n\r",
    "You are almost evil.\n\r",
    "You are slightly evil.\n\r",
    "You are evil.\n\r",
    "You feel like being a devil.\n\r",
    "You are a devil.\n\r"
};

static char align_msg_han[13][44] = {
    "당신은 성인 군자십니다.\n\r",
    "당신은 성인 군자가 되어가고 있습니다.\n\r",
    "당신은 선하십니다.\n\r",
    "당신은 약간 선합니다.\n\r",
    "당신은 거의 선한 경지에 이르렀습니다.\n\r",
    "당신은 선한쪽으로 변하고 있습니다.\n\r",
    "당신은 아주 평범한 성향입니다.\n\r",
    "당신은 악한 쪽으로 변해가고 있습니다.\n\r",
    "당신은 거의 악해졌습니다.\n\r",
    "당신은 약간 악한 성향을 띄고 있습니다.\n\r",
    "당신은 악하십니다.\n\r",
    "당신은 거의 악마가 되어갑니다.\n\r",
    "당신은 악마가 되셨습니다.\n\r"
};

void do_score(struct char_data *ch, char *argument, int cmd)
{
    struct affected_type *aff;
    struct time_info_data playing_time;
    static char buf[200], buf2[200];
    struct time_info_data real_time_passed(time_t t2, time_t t1);
    extern struct title_type titles[4][IMO+4];
    int tmp;

    sprintf(buf, "You are %d years old.\n\r", GET_AGE(ch) );
    sprintf(buf2, "당신은 %d 살 입니다.\n\r", GET_AGE(ch) );
    send_to_char_han(buf,buf2,ch);

    /* alignment message */
    tmp = GET_ALIGNMENT(ch)/50;
    if( tmp > 17 ) send_to_char_han( align_msg[0], align_msg_han[0], ch );
    else if( tmp > 12 ) send_to_char_han( align_msg[1], align_msg_han[1], ch );
    else if( tmp > 7  ) send_to_char_han( align_msg[2], align_msg_han[2], ch );
    else if( tmp > 6  ) send_to_char_han( align_msg[3], align_msg_han[3], ch );
    else if( tmp > 5  ) send_to_char_han( align_msg[4], align_msg_han[4], ch );
    else if( tmp > 2  ) send_to_char_han( align_msg[5], align_msg_han[5], ch );
    else if( tmp > -3  ) send_to_char_han( align_msg[6], align_msg_han[6], ch );
    else if( tmp > -6  ) send_to_char_han( align_msg[7], align_msg_han[7], ch );
    else if( tmp > -7  ) send_to_char_han( align_msg[8], align_msg_han[8], ch );
    else if( tmp > -8  ) send_to_char_han( align_msg[9], align_msg_han[9], ch );
    else if( tmp > -13 ) send_to_char_han( align_msg[10], align_msg_han[10], ch );
    else if( tmp > -18 ) send_to_char_han( align_msg[11], align_msg_han[11], ch );
    else send_to_char_han( align_msg[12], align_msg_han[12], ch );

    sprintf(buf,"Your ac is %d.\n\r",ch->points.armor);
    sprintf(buf2,"당신의 무장정도는 %d 입니다.\n\r",ch->points.armor);
    send_to_char_han(buf,buf2,ch);

/*  tmp = ch->points.armor/10;
    switch( tmp )
    { case 10: send_to_char( ac_msg[0], ch ); break;
    case 9:
    case 8:  send_to_char( ac_msg[1], ch ); break;
    case 7:
    case 6:  send_to_char( ac_msg[2], ch ); break;
    case 5:
    case 4:  send_to_char( ac_msg[3], ch ); break;
    case 3:
    case 2:  send_to_char( ac_msg[4], ch ); break;
    case 1:
    case 0:  send_to_char( ac_msg[5], ch ); break;
    case -1:
    case -2:
    case -3:  send_to_char( ac_msg[6], ch ); break;
    case -4:
    case -5:
    case -6:  send_to_char( ac_msg[7], ch ); break;
    case -7:
    case -8:
    case -9:  send_to_char( ac_msg[8], ch ); break;
    case -10:
    case -11:
    case -12:  send_to_char( ac_msg[9], ch ); break;
    case -13:
    case -14:
    case -15:  send_to_char( ac_msg[10], ch ); break;
    case -16:
    case -17:
    case -18:  send_to_char( ac_msg[11], ch ); break;
    default:  send_to_char( ac_msg[12], ch ); break;
    } */

    /* AC message
       sprintf(buf, "Your AC is %d.(max:-300,min:100)\n\r", ch->points.armor);
       send_to_char(buf,ch);
    */

    if (GET_COND(ch,DRUNK)>10)
        send_to_char_han("You are intoxicated.\n\r","당신은 취해 있습니다.\n\r",ch);
    sprintf(buf, 
            "You have %d(%d) hit, %d(%d) mana and %d(%d) movement points.\n\r",
            GET_HIT(ch),GET_MAX_HIT(ch),
            GET_MANA(ch),GET_MAX_MANA(ch),
            GET_MOVE(ch),GET_MAX_MOVE(ch));
    sprintf(buf2, 
            "당신은 %d(%d) hit, %d(%d) mana 와 %d(%d) move 를 가지고 있습니다.\n\r",
            GET_HIT(ch),GET_MAX_HIT(ch),
            GET_MANA(ch),GET_MAX_MANA(ch),
            GET_MOVE(ch),GET_MAX_MOVE(ch));
    send_to_char_han(buf,buf2,ch);

    if (GET_LEVEL(ch) > 12) {
        sprintf(buf,"Your stat: str %d/%d wis %d int %d dex %d con %d.\n\r",
                GET_STR(ch),GET_ADD(ch),GET_WIS(ch),GET_INT(ch),GET_DEX(ch),GET_CON(ch));
        sprintf(buf2,"당신의 체질: str %d/%d wis %d int %d dex %d con %d.\n\r",
                GET_STR(ch),GET_ADD(ch),GET_WIS(ch),GET_INT(ch),GET_DEX(ch),GET_CON(ch));
        send_to_char_han(buf, buf2, ch);

        sprintf(buf,"Your hitroll is %d, damroll is %d, and regen is %d.\n\r",
                GET_HITROLL(ch),GET_DAMROLL(ch), GET_REGENERATION(ch));
        sprintf(buf2,"당신의 hitroll 은 %d, damroll 은 %d, 회복률은 %d.\n\r",
                GET_HITROLL(ch),GET_DAMROLL(ch), GET_REGENERATION(ch));
        send_to_char_han(buf, buf2, ch); /* changed by shin won dong */
    }

    sprintf(buf,"You have scored %d exp, and have %d gold coins.\n\r",
            GET_EXP(ch),GET_GOLD(ch));
    sprintf(buf2,"당신은 %d 의 경험치와 %d 원의 돈을 가지고 있습니다.\n\r",
            GET_EXP(ch),GET_GOLD(ch));
    send_to_char_han(buf, buf2, ch);

    playing_time = real_time_passed((time(0)-ch->player.time.logon) +
                                    ch->player.time.played, 0);
    sprintf(buf,"You have been playing for %d days and %d hours.\n\r",
            playing_time.day, playing_time.hours);    
    sprintf(buf2,"당신은 이 안에서 %d 일 %d 시간동안 있었습니다.\n\r",
            playing_time.day, playing_time.hours);    
    send_to_char_han(buf, buf2, ch);    
    sprintf(buf,"This ranks you as %s %s (level %d).\n\r",
            GET_NAME(ch), GET_TITLE(ch), GET_LEVEL(ch) );
    sprintf(buf2, "당신은 %s %s (레벨 %d) 입니다.\n\r",
            GET_NAME(ch), GET_TITLE(ch), GET_LEVEL(ch) );
    send_to_char_han(buf, buf2, ch);
    if(GET_LEVEL(ch)<IMO-1){
        if(titles[GET_CLASS(ch)-1][GET_LEVEL(ch)+1].exp<GET_EXP(ch)) {
            strcpy(buf,"You have enough experience to advance.\n\r");
            strcpy(buf2,"레벨을 올릴만큼 충분한 경험치가 쌓였습니다.\n\r");
        }
        else {
            sprintf(buf,"You need %d experience to advance\n\r",
                    titles[GET_CLASS(ch)-1][GET_LEVEL(ch)+1].exp-GET_EXP(ch));
            sprintf(buf2,"다음 레벨까지 %d 만큼의 경험치가 필요합니다.\n\r",
                    titles[GET_CLASS(ch)-1][GET_LEVEL(ch)+1].exp-GET_EXP(ch));
        }
        send_to_char_han(buf, buf2, ch);
    }

    switch(GET_POS(ch)) {
    case POSITION_DEAD : 
        send_to_char_han("You are DEAD!\n\r", "당신은 죽으셨습니다\n\r", ch);
        break;
    case POSITION_MORTALLYW :
        send_to_char("You are mortally wounded!, you should seek help!\n\r", ch); break;
    case POSITION_INCAP : 
        send_to_char("You are incapacitated, slowly fading away\n\r", ch); break;
    case POSITION_STUNNED : 
        send_to_char("You are stunned! You can't move\n\r", ch); break;
    case POSITION_SLEEPING : 
        send_to_char_han("You are sleeping.\n\r", "당신은 자고 있습니다.\n\r",
                         ch);
        break;
    case POSITION_RESTING  : 
        send_to_char_han("You are resting.\n\r", "당신은 쉬고 있습니다.\n\r",ch);
        break;
    case POSITION_SITTING  : 
        send_to_char_han("You are sitting.\n\r","당신은 앉아 있습니다.\n\r",ch);
        break;
    case POSITION_FIGHTING :
        if (ch->specials.fighting)
            acthan("You are fighting $N.\n\r", "당신은 $N님과 싸우고 있습니다.\n\r",
                   FALSE, ch, 0, ch->specials.fighting, TO_CHAR);
        else
            send_to_char("You are fighting thin air.\n\r", ch);
        break;
    case POSITION_STANDING : 
        send_to_char_han("You are standing.\n\r","당신은 서있습니다.\n\r", ch);
        break;
    default :
        send_to_char("You are floating.\n\r",ch); break;
    }

    if (ch->affected) {
        send_to_char_han("Affecting Spells:\n\r", "걸려있는 마법들:\n\r", ch);
        for(aff = ch->affected; aff; aff = aff->next) {
            sprintf(buf,"%s: %d hrs\n\r",spells[aff->type-1],aff->duration);
            sprintf(buf2,"%s: %d 시간\n\r",spells[aff->type-1],aff->duration);
            send_to_char_han(buf, buf2, ch);
        }
    }
}

/* From msd mud : cyb */
void do_attribute(struct char_data *ch, char *argument, int cmd)
{
    char buf[MAX_STRING_LENGTH];
    struct affected_type *aff;
    extern char *spells[];
  
    sprintf(buf,
            "You are %d years and %d months, %d cms, and you weigh %d lbs.\n\r",
            GET_AGE(ch), age(ch).month,
            ch->player.height,
            ch->player.weight);

    send_to_char(buf, ch);
  
    sprintf(buf, "You are carrying %d lbs of equipment.\n\r",
            IS_CARRYING_W(ch));
    send_to_char(buf, ch); 
  
/*sprintf(buf,"Your armor is %d (-100 to 100).\n\r",ch->points.armor);
  send_to_char(buf,ch); */
  
    /* if ((GET_LEVEL(ch) > 15) || (GET_CLASS(ch))) {    cyb */
    if ((GET_LEVEL(ch) > 15) ) {
        if ((GET_STR(ch)==18) && (GET_CLASS(ch))) {
            sprintf(buf,"You have %d/%d STR, %d INT, %d WIS, %d DEX, %d CON\n\r",
                    GET_STR(ch), GET_ADD(ch), GET_INT(ch), GET_WIS(ch), GET_DEX(ch), GET_CON(ch));
            send_to_char(buf,ch);
        } else {
            sprintf(buf,"You have %d STR, %d INT, %d WIS, %d DEX, %d CON\n\r",
                    GET_STR(ch), GET_INT(ch), GET_WIS(ch), GET_DEX(ch), GET_CON(ch));
            send_to_char(buf,ch);
        }
    }  

    sprintf(buf, "Your hitroll and damroll are %d and %d respectively.\n\r",
            GET_HITROLL(ch), GET_DAMROLL(ch));
    send_to_char(buf, ch);

    /*
    **   by popular demand -- affected stuff
    */
    if (ch->affected) {
        send_to_char("\n\rAffecting Spells:\n\r--------------\n\r", ch);
        for(aff = ch->affected; aff; aff = aff->next) {
            switch(aff->type) {
            case SKILL_SNEAK:
            case SPELL_POISON:
            case SPELL_CURSE:
                break;
            default:
                sprintf(buf, "Spell : '%s'\n\r",spells[aff->type-1]);
                send_to_char(buf, ch);
                break;
            }
        }
    }
}

/* end of Attribute Module... */


void do_time(struct char_data *ch, char *argument, int cmd)
{
    char buf[100], *suf;
    int weekday, day;
    extern struct time_info_data time_info;
    extern char *weekdays[];
    extern char *month_name[];

    sprintf(buf, "It is %d o'clock %s, on ",
            ((time_info.hours % 12 == 0) ? 12 : ((time_info.hours) % 12)),
            ((time_info.hours >= 12) ? "pm" : "am") );

    weekday = ((35*time_info.month)+time_info.day+1) % 7;/* 35 days in a month */

    strcat(buf,weekdays[weekday]);
    strcat(buf,"\n\r");
    send_to_char(buf,ch);

    day = time_info.day + 1;   /* day in [1..35] */

    if (day == 1)
        suf = "st";
    else if (day == 2)
        suf = "nd";
    else if (day == 3)
        suf = "rd";
    else if (day < 20)
        suf = "th";
    else if ((day % 10) == 1)
        suf = "st";
    else if ((day % 10) == 2)
        suf = "nd";
    else if ((day % 10) == 3)
        suf = "rd";
    else
        suf = "th";

    sprintf(buf, "The %d%s Day of the %s, Year %d.\n\r",
            day,
            suf,
            month_name[time_info.month],
            time_info.year);

    send_to_char(buf,ch);
}


void do_weather(struct char_data *ch, char *argument, int cmd)
{
    extern struct weather_data weather_info;
    static char buf[100], buf2[100];
    char static *sky_look[4]= {
        "cloudless",
        "cloudy",
        "rainy",
        "lit by flashes of lightning"
    };
    char static *sky_look_han[4]= {
        "하늘이 구름 한점 없이 맑습니다.",
        "하늘에 구름이 조금 있습니다.",
        "비가 오고 있습니다.",
        "천둥 번개가 칩니다."
    };

    if (OUTSIDE(ch)) {
        sprintf(buf, "The sky is %s and %s.\n\r", sky_look[weather_info.sky],
                (weather_info.change >=0 ? "you feel a warm wind from south" :
                 "your foot tells you bad weather is due"));
        sprintf(buf2, "%s %s.\n\r", sky_look_han[weather_info.sky],
                (weather_info.change >=0 ? "서편하늘부터 구름이 개이고 있습니다" :
                 "더 궂은 날이 되것 같습니다"));
        send_to_char_han(buf, buf2, ch);
    } else
        send_to_char_han("You have no feeling about the weather at all.\n\r",
                         "여기선 날씨가 어떤지 알 수 없습니다.\n\r", ch);
}


void do_help(struct char_data *ch, char *argument, int cmd)
{
    extern char *spells[];   /* The list of spells (spells.c)         */
    extern int top_of_helpt;
    extern struct help_index_element *help_index;
    extern FILE *help_fl;
    extern char help[MAX_STRING_LENGTH];
    int	i, no, chk, bot, top, mid, minlen;
    char	buf[MAX_STRING_LENGTH], buffer[MAX_STRING_LENGTH];

    if (!ch->desc)
        return;

    for(; !is_hangul(*argument) && isspace(*argument); argument++) ;

    if (*argument != 0)
    {
        if (!help_index)
        {
            send_to_char("No help available.\n\r", ch);
            return;
        }
        bot = 0;
        top = top_of_helpt;

        /* clear tail space */
        for ( i = strlen(argument)-1 ; i >= 0 && argument[i] == ' ' ; i-- ) {
            argument[i] = 0 ;
		} 

        for (;;)
        {
            mid = (bot + top) / 2;
            minlen = strlen(argument);

            if (!(chk = strn_cmp(argument, help_index[mid].keyword, minlen)))
            {
                fseek(help_fl, help_index[mid].pos, 0);
                *buffer = '\0';
                for (;;)
                {
                    fgets(buf, 80, help_fl);
                    if (*buf == '#')
                        break;
                    strcat(buffer, buf);
                    strcat(buffer, "\r");
                }
                page_string(ch->desc, buffer, 1);
                return;
            }
            else if (bot >= top)
            {
                send_to_char("There is no help on that word.\n\r", ch);
                sprintf(buf, "HELP: tried to seek '%s'", argument) ;
                log(buf) ;
                return;
            }
            else if (chk > 0)
                bot = ++mid;
            else
                top = --mid;
        }
        return;
    }
    send_to_char(help, ch);
}

void do_spells(struct char_data *ch, char *argument, int cmd)
{
    char buf[80*MAX_SKILLS], tmp[MAX_STRING_LENGTH];
    char name[100];
    int class;
    int no, i, count;
    extern char *spells[];
    extern struct spell_info_type spell_info[];

    if (IS_NPC(ch))
        return;

    one_argument(argument, name);
    if (!*name) {
        class = GET_CLASS(ch);
    }
    else {
        switch(*name) {
        case 'm':
        case 'M': class = CLASS_MAGIC_USER ; break ;      /* 1 */
        case 'c':
        case 'C': class = CLASS_CLERIC ; break ;          /* 2 */
        case 't':
        case 'T': class = CLASS_THIEF ; break ;           /* 3 */
        case 'w':
        case 'W': class = CLASS_WARRIOR ; break ;           /* 4 */
        default:
            send_to_char("Usage: spells {CMTW}\n\r", ch) ;
            return ;
        }
    }

    strcpy(buf,"    SPELL NAME       LEVEL/PRACTICE/MANA-MINIMUM\n\r") ;
    for (i = 0, count=0; *spells[i] != '\n'; i++){
        if(*spells[i]==0) continue;
        if(spell_info[i+1].min_level[class-1] > 40) continue;
        sprintf(tmp,"%-20s%4d%8d%6d%s",
                spells[i], spell_info[i+1].min_level[class-1],
                spell_info[i+1].max_skill[class-1],
                spell_info[i+1].min_usesmana, (count%2==0)? " | " : "\n\r");
        strcat(buf,tmp);
        count++ ;
    }
    strcat(buf, "\n\r");
    page_string(ch->desc, buf, 1);
}

do_wizhelp(struct char_data *ch, char *argument, int cmd)
{
    char buf[MAX_STRING_LENGTH];
    int no, i;
    extern char *command[];   /* The list of commands (interpreter.c)  */
    /* First command is command[0]           */
    extern struct command_info cmd_info[];
    /* cmd_info[1] ~~ commando[0]            */

    if (IS_NPC(ch))
        return;
    send_to_char("The following privileged commands are available:\n\r\n\r", ch);
    *buf = '\0';
    for (no = 1, i = 0; *command[i] != '\n'; i++)
        if ((GET_LEVEL(ch) >= cmd_info[i+1].minimum_level) &&
            (cmd_info[i+1].minimum_level >= IMO)) {
            sprintf(buf + strlen(buf), "%-15s", command[i]);
            if (!(no % 5))
                strcat(buf, "\n\r");
            no++;
        }
    strcat(buf, "\n\r");
    page_string(ch->desc, buf, 1);
}

void do_who(struct char_data *ch, char *argument, int cmd)
{
    struct descriptor_data *d;
    char buf[BUFSIZ], buf2[BUFSIZ], class, class_o;
    int num_player=0,tlev,lev,tz,z,flag,condtype;
	int	itsclass ;

    if(argument) {
        while(argument[0]==' ') ++argument;
        switch(argument[0]) {
        case '+': condtype=1; ++argument; break;
        case '-': condtype=2; ++argument; break;
        case '=': condtype=3; ++argument; break;
        case '*': condtype=4; ++argument; z=world[ch->in_room].zone; break;
		case 'M' :
		case 'm' : condtype = 5 ; ++argument ; break ;
		case 'C' :
		case 'c' : condtype = 6 ; ++argument ; break ;
		case 'T' :
		case 't' : condtype = 7 ; ++argument ; break ;
		case 'W' :
		case 'w' : condtype = 8 ; ++argument ; break ;
        default: condtype=1;
        }
        if(condtype < 4)
            lev=atoi(argument);
    } else {
        condtype=0;
    }
    send_to_char_han("Players\n\r-------\n\r", "사람들\n\r-------\n\r", ch);
    for (d = descriptor_list; d; d = d->next) {
        if (!d->connected && CAN_SEE(ch, d->character)) {
            if(condtype){
                if(condtype < 4) {
                    tlev=d->original ? GET_LEVEL(d->original) : GET_LEVEL(d->character);
				}
                else if ( condtype == 4 ) {
                    tz=d->original ? world[d->original->in_room].zone :
                        world[d->character->in_room].zone;
				}
                else {
                    itsclass = ( d->original ? (GET_CLASS(d->original)) :
                                 (GET_CLASS(d->character)) ) ;
				}
                switch(condtype) {
                case 1: flag=(tlev >= lev); break;
                case 2: flag=(tlev <= lev); break;
                case 3: flag=(tlev == lev); break;
                case 4: flag=(tz == z); break;
                case 5: flag = ( itsclass == CLASS_MAGIC_USER ) ; break ;
                case 6: flag = ( itsclass == CLASS_CLERIC ) ; break ;
                case 7: flag = ( itsclass == CLASS_THIEF ) ; break ;
                case 8: flag = ( itsclass == CLASS_WARRIOR ) ; break ;
                }
                if(!flag) continue;
            }
            if(d->original){ /* If switched */ 
                switch(GET_CLASS(d->original)){
                case CLASS_MAGIC_USER: class='M'; break;
                case CLASS_CLERIC    : class='C'; break;
                case CLASS_THIEF     : class='T'; break;
                case CLASS_WARRIOR   : class='W'; break;
                default        : class='U';
                }
                switch (GET_CLASS_O(d->original)) {
                case CLASS_MAGIC_USER: class_o='M'; break;
                case CLASS_CLERIC    : class_o='C'; break;
                case CLASS_THIEF     : class_o='T'; break;
                case CLASS_WARRIOR   : class_o='W'; break;
                default        : class_o=' ';
                }

				if (class_o != ' ') {
					char tmp = class;
					class = class_o;	
					class_o = tmp - 'A' + 'a';
				}

                if(IS_SET(d->character->specials.act,PLR_KOREAN))
                    sprintf(buf, "( %c%c %2d ) %s %s", class, class_o,
                            GET_LEVEL(d->original), 
                            GET_NAME(d->original), d->original->player.title);
                else
                    sprintf(buf, "< %c%c %2d > %s %s", class, class_o,
                            GET_LEVEL(d->original), 
                            GET_NAME(d->original), d->original->player.title);
            }
            else{
                switch(GET_CLASS(d->character)){
                case CLASS_MAGIC_USER: class='M'; break;
                case CLASS_CLERIC    : class='C'; break;
                case CLASS_THIEF     : class='T'; break;
                case CLASS_WARRIOR   : class='W'; break;
                default        : class='U';
                }
                switch (GET_CLASS_O(d->character)) {
                case CLASS_MAGIC_USER: class_o='M'; break;
                case CLASS_CLERIC    : class_o='C'; break;
                case CLASS_THIEF     : class_o='T'; break;
                case CLASS_WARRIOR   : class_o='W'; break;
                default        : class_o=' ';
                }

				if (class_o != ' ') {
					char tmp = class;
					class = class_o;	
					class_o = tmp - 'A' + 'a';
				}

                if(IS_SET(d->character->specials.act,PLR_KOREAN))
                    sprintf(buf, "( %c%c %2d ) %s %s", class, class_o,
                            GET_LEVEL(d->character), 
                            GET_NAME(d->character), d->character->player.title);
                else
                    sprintf(buf, "< %c%c %2d > %s %s", class, class_o,
                            GET_LEVEL(d->character), 
                            GET_NAME(d->character), d->character->player.title);
                if(IS_SET(d->character->specials.act,PLR_CRIMINAL))
                    strcat(buf," (CRIMINAL)\n\r");
                else
                    strcat(buf,"\n\r");
            }
            send_to_char(buf, ch);
            num_player++;
        }
    }
    send_to_char("\n\r", ch);
    sprintf(buf,"You can see %d players.\n\r",num_player);
    sprintf(buf2,"%d 명이 있습니다.\n\r",num_player);
    send_to_char_han(buf, buf2, ch);
}

extern char *connected_types[];

void do_users(struct char_data *ch, char *argument, int cmd)
{
    char line[256], line2[256];
    struct descriptor_data *d;
    int m=0,n=0,flag,t;
    static most=0;
    extern int boottime;

    one_argument(argument,line);
    flag=((GET_LEVEL(ch) < IMO) || (strcmp("-t",line)==0));
    line[0]=0;
    for (d=descriptor_list;d;d=d->next) {
        ++m;
        if(flag) continue;
        if (d->original){
            if(!CAN_SEE(ch,d->original)) continue;
            sprintf(line+strlen(line),"%3d%2d:",d->descriptor,
                    d->original->specials.timer);
            sprintf(line+strlen(line), "%-14s%2d ",
                    GET_NAME(d->original),GET_LEVEL(d->original));
        } else if (d->character){
            if(!CAN_SEE(ch,d->character)) continue;
            sprintf(line+strlen(line),"%3d%2d:",d->descriptor,
                    d->character->specials.timer);
            sprintf(line+strlen(line), "%-14s%2d ",
                    (d->connected==CON_PLYNG) ? GET_NAME(d->character) : "Not in game",
                    GET_LEVEL(d->character));
        } else
            sprintf(line+strlen(line), "%3d%9s%10s ",
                    d->descriptor,"  UNDEF  ",connected_types[d->connected]);
        sprintf(line+strlen(line),"%-15s",d->host);
        if(!(n%2)){
            strcat(line,"|");
        } else {
            strcat(line,"\n\r");
            send_to_char(line, ch);
            line[0]=0;
        }
        ++n;
    }
    if((!flag)&&(n%2)){
        strcat(line,"\n\r");
        send_to_char(line, ch);
    }
    if(m > most) most=m;
    sprintf(line,"%s%d/%d active connections\n\r",
            (n%2) ? "\n\r" : "",m,most);
    if (GET_LEVEL(ch) > IMO+2) send_to_char(line,ch);
    t=30+time(0)-boottime;
    sprintf(line,"Running time %d:%02d\n\r",t/3600,(t%3600)/60);
    sprintf(line2,"현재 %d시간 %02d분 지났습니다.\n\r",t/3600,(t%3600)/60);
    send_to_char_han(line, line2, ch);
}
void do_inventory(struct char_data *ch, char *argument, int cmd) {
    send_to_char_han("You are carrying:\n\r", "가지고 있는 물건들:\n\r", ch);
    list_obj_to_char(ch->carrying, ch, 1, TRUE);
}
void do_equipment(struct char_data *ch, char *argument, int cmd) {
    int j;
    bool found;

    send_to_char_han("You are using:\n\r", "쓰고 있는 물건들:\n\r", ch);
    found = FALSE;
    for (j=0; j< MAX_WEAR; j++) {
        if (ch->equipment[j]) {
            if (CAN_SEE_OBJ(ch,ch->equipment[j])) {
                send_to_char(where[j],ch);
                show_obj_to_char(ch->equipment[j],ch,1);
                found = TRUE;
            } else {
                send_to_char(where[j],ch);
                send_to_char("Something.\n\r",ch);
                found = TRUE;
            }
        }
    }
    if(!found) {
        send_to_char_han(" Nothing.\n\r", " 아무것도 없네요.\n\r", ch);
    }
}


void do_credits(struct char_data *ch, char *argument, int cmd) {

    page_string(ch->desc, credits, 0);
}
void do_news(struct char_data *ch, char *argument, int cmd) {
    page_string(ch->desc, news, 0);
}
void do_info(struct char_data *ch, char *argument, int cmd) {
    page_string(ch->desc, info, 0);
}
void do_wizards(struct char_data *ch, char *argument, int cmd) {
    page_string(ch->desc, wizards, 0);
}

void do_where(struct char_data *ch, char *argument, int cmd)
{
#ifdef DEATHFIGHT
	extern int	deathfight ;
#endif 

    char name[MAX_INPUT_LENGTH], buf[MAX_STRING_LENGTH], buf2[256];
    struct char_data *i;
    int j,n;
    struct obj_data *k;
    struct descriptor_data *d;

    one_argument(argument, name);
    n=0;
    if (!*name) {
        if (GET_LEVEL(ch) < IMO) {
            for (d = descriptor_list; d; d = d->next) {
                if (d->character && (d->connected == CON_PLYNG) &&
                    (d->character->in_room != NOWHERE)) {
                    if(CAN_SEE(ch,d->character)) {
#ifdef DEATHFIGHT
                        if ( deathfight != 0 ) {	/* second round of DF */
                            sprintf(buf, "%-20s - %s [%d]\n\r",
                                    d->character->player.name,
                                    world[d->character->in_room].name,
                                    world[d->character->in_room].number);
                            send_to_char(buf, ch);
						}
                        else if ( world[d->character->in_room].zone ==
                                  world[ch->in_room].zone) {
                            sprintf(buf, "%-20s - %s [%d]\n\r",
                                    d->character->player.name,
                                    world[d->character->in_room].name,
                                    world[d->character->in_room].number);
                            send_to_char(buf, ch);
						}
#else
                        if ( world[d->character->in_room].zone ==
                             world[ch->in_room].zone) {
                            sprintf(buf, "%-20s - %s [%d]\n\r",
                                    d->character->player.name,
                                    world[d->character->in_room].name,
                                    world[d->character->in_room].number);
                            send_to_char(buf, ch);
						}
#endif 
					}
				}
			}	/* end of for */
            return;
        } else {
            for (d = descriptor_list; d; d = d->next) {
                if (d->character && (d->connected == CON_PLYNG) &&
                    (d->character->in_room != NOWHERE))
                    if(CAN_SEE(ch,d->character)) {
                        if (d->original)   /* If switched */
                            sprintf(buf, "%-20s - %s [%d] In body of %s\n\r",
                                    d->original->player.name,
                                    world[d->character->in_room].name,
                                    world[d->character->in_room].number,
                                    fname(d->character->player.name));
                        else
                            sprintf(buf, "%-20s - %s [%d]\n\r",
                                    d->character->player.name,
                                    world[d->character->in_room].name,
                                    world[d->character->in_room].number);
                        send_to_char(buf, ch);
                    }
            }
            return;
        }
    }
    *buf = '\0';
    for (i = character_list; i; i = i->next)
        if(isname(name, i->player.name) && CAN_SEE(ch, i) ) {
            if((i->in_room != NOWHERE) && ((GET_LEVEL(ch)>=IMO) ||
                                           (world[i->in_room].zone == world[ch->in_room].zone))) {
                if (IS_NPC(i))
                    sprintf(buf, "%2d: %-30s- %s ",++n,i->player.short_descr,
                            world[i->in_room].name);
                else
                    sprintf(buf, "%2d: %-30s- %s ",++n,i->player.name,
                            world[i->in_room].name);
                if (GET_LEVEL(ch) >= IMO)
                    sprintf(buf2,"[%d]\n\r", world[i->in_room].number);
                else
                    strcpy(buf2, "\n\r");
                strcat(buf, buf2);
                send_to_char(buf, ch);
                if (GET_LEVEL(ch) < IMO)
                    break;
            }
        }
    if (GET_LEVEL(ch) >= IMO) {
        for (i=character_list;i;i=i->next)
            for(j=0;j<MAX_WEAR;++j) if(i->equipment[j])
                if(isname(name, i->equipment[j]->name)){
                    sprintf(buf, "%2d: %s worn by %s\n\r",
                            ++n,i->equipment[j]->short_description, GET_NAME(i));
                    send_to_char(buf, ch);
                }
        for (k=object_list;k;k=k->next) if(isname(name, k->name)){
            if (k->rented_by) {
                sprintf(buf,"%2d: %s rented by %s.\n\r",
                        ++n,k->short_description,PERS(k->rented_by,ch));
                send_to_char(buf,ch);
			}
            else if(k->carried_by) {
                sprintf(buf,"%2d: %s carried by %s.\n\r",
                        ++n,k->short_description,PERS(k->carried_by,ch));
                send_to_char(buf,ch);
            } else if(k->in_obj) {
                sprintf(buf,"%2d: %s in %s",++n,k->short_description,
                        k->in_obj->short_description);
                if(k->in_obj->rented_by){
                    sprintf(buf2," rented by %s\n\r",PERS(k->in_obj->rented_by,ch));
                    strcat(buf,buf2);
				}
                else if(k->in_obj->carried_by){
                    sprintf(buf2," carried by %s\n\r",PERS(k->in_obj->carried_by,ch));
                    strcat(buf,buf2);
                } else
                    strcat(buf,"\n\r");
                send_to_char(buf,ch);
            } else if(k->in_room != NOWHERE) {
                sprintf(buf, "%2d: %s in %s [%d]\n\r",++n, k->short_description,
                        world[k->in_room].name, world[k->in_room].number);
                send_to_char(buf, ch);
            }
        }
    }
    if (!*buf)
        send_to_char("Couldn't find any such thing.\n\r", ch);
}
void do_levels(struct char_data *ch, char *argument, int cmd)
{
    int i;
    char buf[MAX_STRING_LENGTH];
    extern struct title_type titles[4][IMO+4];

    if (IS_NPC(ch)) {
        send_to_char("You ain't nothin' but a hound-dog.\n\r", ch);
        return;
    }
    *buf = '\0';
    for (i = 1; i < IMO; i++) {
        sprintf(buf + strlen(buf), "%2d: %9d to %9d: ",i,
                titles[GET_CLASS(ch) - 1][i].exp,
                titles[GET_CLASS(ch) - 1][i + 1].exp);
        switch(GET_SEX(ch)) {
        case SEX_MALE:
            strcat(buf, titles[GET_CLASS(ch) - 1][i].title_m); break;
        case SEX_FEMALE:
            strcat(buf, titles[GET_CLASS(ch) - 1][i].title_f); break;
        default:
            send_to_char("Oh dear.\n\r", ch); break;
        }
        strcat(buf, "\n\r");
    }
    send_to_char(buf, ch);
}



void do_consider(struct char_data *ch, char *argument, int cmd)
{
    struct char_data *victim;
    char name[256], buf[256];
    int diff;

    one_argument(argument, name);

    if (!(victim = get_char_room_vis(ch, name))) {
        send_to_char("Consider killing who?\n\r", ch);
        return;
    }

    if (victim == ch) {
        send_to_char("Easy! Very easy indeed!\n\r", ch);
        return;
    }

    if (!IS_NPC(victim)) {
        send_to_char("Would you like to borrow a cross and a shovel?\n\r", ch);
        return;
    }

    diff = (GET_LEVEL(victim)-GET_LEVEL(ch));

    if (diff <= -10)
        send_to_char("Now where did that chicken go?\n\r", ch);
    else if (diff <= -5)
        send_to_char("You could do it with a needle!\n\r", ch);
    else if (diff <= -2)
        send_to_char("Easy.\n\r", ch);
    else if (diff <= -1)
        send_to_char("Fairly easy.\n\r", ch);
    else if (diff == 0)
        send_to_char("The perfect match!\n\r", ch);
    else if (diff <= 1)
        send_to_char("You would need some luck!\n\r", ch);
    else if (diff <= 2)
        send_to_char("You would need a lot of luck!\n\r", ch);
    else if (diff <= 3)
        send_to_char("You would need a lot of luck and great equipment!\n\r", ch);
    else if (diff <= 5)
        send_to_char("Do you feel lucky, punk?\n\r", ch);
    else if (diff <= 10)
        send_to_char("Are you mad!?\n\r", ch);
    else if (diff <= 100)
        send_to_char("You ARE mad!\n\r", ch);
}

void do_police(struct char_data *ch, char *argument, int cmd)
{
    char name[200], buf[BUFSIZ] ;
    struct descriptor_data *d;
    int i,target,type;
    int stash_char(struct char_data *ch, char *filename);
    void wipe_obj(struct obj_data *obj);
    struct obj_data *unequip_char(struct char_data *ch, int pos);

    one_argument(argument, name);
    if(! *argument)
        return;
	if (( target = atoi(name)) <= 0 ) {
		send_to_char("police: irregal number", ch) ;
		return ;
    }
    for (d=descriptor_list;d;d=d->next){
        if(target==d->descriptor){
            sprintf(buf,"police: %s",GET_NAME(d->character));
            log(buf);
            if((d->connected == CON_PLYNG)&&(d->character)){
                if(d->character->player.level < ch->player.level){
                    if ( stash_char(d->character,0) < 0) {
                        send_to_char("Police : Saving failed.\n\r", ch) ;
                    }
                    move_stashfile(d->character->player.name);
                    for(i=0; i<MAX_WEAR; i++)
                        if(d->character->equipment[i]){
                            extract_obj(unequip_char(d->character,i));
                            d->character->equipment[i]=0;
                        }
                    if(d->character->carrying)
                        wipe_obj(d->character->carrying);
                    d->character->carrying=0;
                    close_socket(d);
                    extract_char(d->character);
                }
            } else {
                close_socket(d);
            }
        }
    }
}
void do_wizlock(struct char_data *ch, char *argument, int cmd)
{
    char buf[200];
    int i,j;

    buf[0]=0;
    one_argument(argument, buf);
    if(*argument){
        j=(-1);
        for(i=0;i<baddoms;++i)
            if(strcmp(baddomain[i],buf)==0){
                j=i; break;
            }
        if(j>=0){
            strcpy(baddomain[j],baddomain[--baddoms]);
        } else {
            if(baddoms < BADDOMS)
                strcpy(baddomain[baddoms++],buf);
        }
    } else {
        for(i=0;i<baddoms;++i){
            sprintf(buf,"%s\n",baddomain[i]);
            send_to_char(buf,ch);
        }
    }
}
do_nochat(struct char_data *ch, char *argument, int cmd)
{
    if (IS_NPC(ch))
        return;

    if (IS_SET(ch->specials.act, PLR_NOCHAT)) {
        send_to_char("You can now hear chats again.\n\r", ch);
        REMOVE_BIT(ch->specials.act, PLR_NOCHAT);
    } else {
        send_to_char("From now on, you won't hear chats.\n\r", ch);
        SET_BIT(ch->specials.act, PLR_NOCHAT);
    }
}
do_notell(struct char_data *ch, char *argument, int cmd)
{
    if (IS_NPC(ch))
        return;

    if (IS_SET(ch->specials.act, PLR_NOTELL)) {
        send_to_char("You can now hear tells again.\n\r", ch);
        REMOVE_BIT(ch->specials.act, PLR_NOTELL);
    } else {
        send_to_char("From now on, you won't hear tells.\n\r", ch);
        SET_BIT(ch->specials.act, PLR_NOTELL);
    }
}
void do_data(struct char_data *ch, char *argument, int cmd)
{
    struct descriptor_data *d;
    struct char_data *victim;
    char buf[256],tbuf[256],name[256],fmt[16];
    int lev,i=0,k,n,t,nc;

    one_argument(argument, name);
    if(strcmp(name,"exp")==0)
        k=1;
    else if(strcmp("hit",name)==0)
        k=2;
    else if(strcmp("gold",name)==0)
        k=3;
    else if(strcmp("armor",name)==0)
        k=4;
    else if(strcmp("age",name)==0){
        k=5; t=time(0);
    } else if(strcmp("com",name)==0){
        k=6; t=time(0);
    } else if(strcmp("flags",name)==0)
        k=7;
    else if(strcmp("bank",name)==0)
        k=8;
    else if(strcmp("des",name)==0)
        k=9;
    else if(strcmp("level", name)==0)
        k=10;
    else if(strcmp("align", name)==0)
        k=11;
    else {
        send_to_char("Usage: data mode\n\r", ch) ;
        send_to_char("mode : exp hit gold armor age com flags bank des level align\n\r",
                     ch) ;
        return ;
	}

    if((k==1)||(k==3)||(k==8)){
        nc=3;
        strcpy(fmt,"%-15s%10d%s");
    } else {
        nc=4;
        strcpy(fmt,"%-15s%4d%s");
    }
    for (d = descriptor_list; d; d = d->next) {
        if (!d->connected && CAN_SEE(ch, d->character)) {
            ++i;
            if(d->original) /* If switched */      
                victim=d->original;
            else
                victim=d->character;
            switch(k){
            case  1: n=victim->points.exp; break;
            case  2: n=victim->points.max_hit; break;
            case  3: n=victim->points.gold; break;
            case  4: n=victim->points.armor; break;
            case  5: n=(t-victim->player.time.birth)/86400; break;
            case  6: n=(60*victim->desc->ncmds)/(1+t-victim->desc->contime); break;
            case  7: n=victim->specials.act; break;
            case  8: n=victim->bank; break;
            case  9: n=d->descriptor; break;
            case 10: n=victim->player.level; break ;
            case 11: n= GET_ALIGNMENT(victim) ; break;
            }
            sprintf(buf,fmt,GET_NAME(victim),n,(i%nc) ? "|" : "\n\r");
            send_to_char(buf,ch);
        }
    }
    if(i%nc)
        send_to_char("\n\r",ch);
}

do_hangul(struct char_data *ch, char *argument, int cmd)
{
    if (IS_NPC(ch))
        return;

    if (IS_SET(ch->specials.act, PLR_KOREAN)) {
        send_to_char("English command message mode.\n\r", ch);
        REMOVE_BIT(ch->specials.act, PLR_KOREAN);
    } else {
        send_to_char("You can now see hangul command messages.\n\r", ch);
        SET_BIT(ch->specials.act, PLR_KOREAN);
    }
}

do_autoexit(struct char_data *ch, char *argument, int cmd)
{
    if (IS_NPC(ch))
        return;

    if (IS_SET(ch->specials.act, PLR_AUTOEXIT)) {
        send_to_char("Auto exit off.\n\r", ch);
        REMOVE_BIT(ch->specials.act, PLR_AUTOEXIT);
    } else {
        send_to_char("Display exits automatically.\n\r", ch);
        SET_BIT(ch->specials.act, PLR_AUTOEXIT);
    }
}
