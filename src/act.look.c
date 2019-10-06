/* ************************************************************************
*  file: act.look.c 							  *
*  Usage : Informative commands.  looking char , object or status.        *
* 	OLD File: Part of OLD act.informative.c 			  *
************************************************************************* */ 

#include <stdio.h>
#include <string.h>
#include <time.h>

#include "char.h"
#include "object.h"
#include "global.h"
#include "comm.h"
#include "play.h" 
#include "actions.h" 

/* internal functions */

/* NOTE: Removed : list_obj_char(); Added list_equips_buf() */
/* void list_obj_to_char(struct obj_data *list, struct char_data *ch, int mode,
   		bool show ) */
int list_equips_buf(struct char_data *ch, struct char_data *viewer,
		    char *buffer);

/* extern's */ 
extern char *where[];

/* NOTE: Symbolic defintion of mode arg of list_obj_buf(), show_obj_buf(). */
/* NOTE: Currently, LOOK_INV is same as LOOK_CONT.      */
#define LOOK_ROOM	0
#define LOOK_INV	1
#define LOOK_CONT	1
#define LOOK_BAG	2
#define LOOK_NOAFF	3
#define LOOK_EQUIP	4
#define LOOK_AT		5
#define LOOK_EXTRA	6

/* Procedures related to 'look' */

/* NOTE: NEW!  argument_split_2() + keyword search and args parsing     */
/* NOTE: Parse argument for look command as argument_split_2() does.
   Search for direction word ( north, ..., up, in, at )
   Copy first non diection argument in arg1.
   Returns direction keyword index or -1 if none. */
int look_argument(char *argument, char *arg1)
{
    char *word;
    int dir;

    static char *keywords[] = {
	"north", "east", "south", "west",
	"up", 	 "down",  "in",    "at",
	"",			/* Look at '' case */
	"\n"}; 

    arg1[0] = '\0';
    word = strtok(argument, " \t\r\n");		/* find first word */
    if (!word)					/* 'look' with no arg */
	return (8);

    dir = search_block(word, keywords, FALSE);	/* partial match */
    if (dir == -1) {		/* first word is not direction  */
	strcpy(arg1, word);
	dir = 7;
    }
    else if ((word = strtok(NULL, " \t\r\n")))	/* second word */
	strcpy(arg1, word);
    return (dir);
}

#ifdef UNUSED_CODE

void argument_split_2(char *argument, char *first_arg, char *second_arg)
{
    int look_at, found, begin;

    found = begin = 0;

    /* Find first non blank */
    for (; *(argument + begin) == ' '; begin++) ;

    /* Find length of first word */
    for (look_at = 0; *(argument + begin + look_at) > ' '; look_at++)
	/* Make all letters lower case, AND copy them to first_arg */
	*(first_arg + look_at) = LOWER(*(argument + begin + look_at));
    *(first_arg + look_at) = '\0';
    begin += look_at;

    /* Find first non blank */
    for (; *(argument + begin) == ' '; begin++) ;

    /* Find length of second word */
    for (look_at = 0; *(argument + begin + look_at) > ' '; look_at++)
	/* Make all letters lower case, AND copy them to second_arg */
	*(second_arg + look_at) = LOWER(*(argument + begin + look_at));
    *(second_arg + look_at) = '\0';
    begin += look_at;
}

#endif	// UNUSED_CODE

char *find_ex_description(char *word, struct extra_descr_data *list)
{
    struct extra_descr_data *i;

    for (i = list; i; i = i->next)
	if (isname(word, i->keyword))
	    return (i->description);
    return (0);
}

/* NOTE: OLD show_obj_char() is modified to NEW show_obj_to_buf().  */
/* NOTE: It sees one and only one object, not objects in container.
   It store output to buffer, not sending to char       
   It checks visibility of obj to char and returns it.    */
int show_obj_to_buf(struct obj_data *object, struct char_data *ch, int mode,
		    char *buffer)
{
    char *mp;
    extern char *item_type_desc[]; 

    buffer[0] = '\0';
    /* NOTE: Check visibility here. If char can't see obj, return 0   */
    if (!CAN_SEE_OBJ(ch, object))
	return (0);

    ASSERT(mode >= LOOK_ROOM && mode <= LOOK_EXTRA);

    if ((mode == LOOK_ROOM) && object->description)
	strcpy(buffer, object->description);
    else if (object->short_description &&
	     (mode == LOOK_CONT || mode == LOOK_EQUIP
		|| mode == LOOK_BAG || mode == LOOK_INV))
	strcpy(buffer, object->short_description); 
    else if (mode == LOOK_AT) 
    /* NOTE: Move item type description to string array item_type_desc[]
	    defined in "constants.c" */
    /* NOTE: Move weapon type description to do_examine() */
	sprinttype( GET_ITEM_TYPE(object), item_type_desc, buffer);
    /* NOTE: no "You see nothing special.."; */

    if (mode != LOOK_EXTRA) {
	int len; 
	
	if (IS_OBJ_STAT(object, ITEM_INVISIBLE))
	    strcat(buffer, "(invisible)");
	if (IS_OBJ_STAT(object, ITEM_EVIL) && ch 
		&& IS_AFFECTED(ch, AFF_DETECT_EVIL))
	    strcat(buffer, "  ..It glows red!");
	/* NOTE: Shorten ITEM_GLOW/ITEM_HUM descr. to single line, if possible*/
	len = strlen(buffer);
	if (mode == LOOK_EQUIP)
	    /* NOTE: Compensate equiped location desc. length */
	    len += strlen(where[0]);
	if (IS_OBJ_STAT(object, ITEM_GLOW && IS_OBJ_STAT(object, ITEM_HUM))) {
	    if (len < 17)
		mp = "  ..It has a soft glowing aura!"
		    " and emits a faint humming sound!";
	    else if (len < 34)
		mp = "  ..soft glowing aura! and faint humming sound!";
	    else
		mp = "  ..glowing aura! and humming sound!";
	}
	else if (IS_OBJ_STAT(object, ITEM_GLOW)) {
	    if (len < 50)
		mp = "  ..It has a soft glowing aura!";
	    else if (len < 59)
		mp = "  ..soft glowing aura!";
	    else
		mp = "  ..glowing aura!";
	}
	else if (IS_OBJ_STAT(object, ITEM_HUM)) {
	    if (len < 46)
		mp = "  ..It emits a faint humming sound!";
	    else if (len < 57)
		mp = "  ..faint humming sound!";
	    else
		mp = "  ..humming sound! ";
	}
	else
	    mp = "";

	strcat(buffer, mp);
    }
    strcat(buffer, "\n\r");
    return (1);
}

/* NOTE: Now, it send output to char through page_string() for long output.
   It will mitigate problem that disconnecting player due to 
   long and rapid output when too many object is in single room.
   Can list contents of bag in bag. (for one level only, not recursively)
 */
void list_obj_to_char(struct obj_data *list, struct char_data *ch, int mode)
{
    struct obj_data *i, *j;
    char buffer[MAX_STR_LEN * 2], *buf;
    int found = 0;

    buf = buffer;
    *buf = '\0';
    for (i = list; i; i = i->next_content) {
	if (!CAN_SEE_OBJ(ch, i))
	    continue;
	show_obj_to_buf(i, ch, mode, buf);
	buf += strlen(buf);
	found++;
	/* NOTE: List contents of bag in bag when LOOK_BAG mode */
	if ((mode == LOOK_BAG) && (GET_ITEM_TYPE(i) == ITEM_CONTAINER)) {
	    sprintf(buf, "The %s contains:\n\r", fname(i->name));
	    buf += strlen(buf);
	    for (j = i->contains; j; j = j->next_content) {
		if (!CAN_SEE_OBJ(ch, j))
		    continue;
		strcat(buf, " > ");
		/* NOTE: BUG FIX: obj arg incorrect: BUG reported by Ljw. */
		show_obj_to_buf( j, ch, LOOK_CONT, buf + 3);
		buf += strlen(buf);
		found++;
	    }
	}
	/* NOTE: check buffer overflow */
	if ( buf > buffer + sizeof(buffer) -100) {
	    strcat(buf, "AND many other things too numerous to list....\r\n"); 
	    break;
	}
    }

    if (found > 0)
	/* NOTE: was no keep internal */
	page_string(ch->desc, buffer, TRUE);
    else if (mode == LOOK_CONT || mode == LOOK_BAG || mode == LOOK_INV)
	send_to_char_han("Nothing.\r\n", "없음.\r\n", ch);

}

/* NOTE:  OLD show_char_to_char() in mode == 0 */
/*      Use when look room and show a char, one char by one */
/*      ex)  Musashi is here, dancing with his samurai sword.
   	     He glows with a bright light!                */
void show_char_to_char(struct char_data *i, struct char_data *ch)
{
    char buffer[MAX_BUFSIZ];
    extern char *remove_eol(char *line); 

    static char *position[] = {
	" is lying here, dead.",
	" is lying here, mortally wounded.",
	" is lying here, incapacitated.",
	" is lying here, stunned.",
	" is sleeping here.",
	" is resting here.",
	" is sitting here.",
	"!FIGHTING!",
	" is standing here.",
	"\n"
    };

    /* NOTE: Now Check visibility of i to ch */
    /* NOTE: Original check for visible char
	if ( (ch!=i) && (ch&&IS_AFFECTED(ch, AFF_SENSE_LIFE) ||
		(CAN_SEE(ch,i) && i && !IS_AFFECTED(i, AFF_HIDE))) ) */
    ASSERT(ch || i);
    if (!ch || !i || (ch == i) || (!IS_AFFECTED(ch, AFF_SENSE_LIFE)
			  && (!CAN_SEE(ch, i) || IS_AFFECTED(i, AFF_HIDE))))
	return;

    /* Code cleanup */
    if (IS_AFFECTED(i, AFF_SHADOW_FIGURE)) {
	send_to_char("A shadow figure is here.\n\r", ch);
	return;
    }
    if (IS_AFFECTED(i, AFF_HIDE) || !CAN_SEE(ch, i)) {
	if (IS_AFFECTED(ch, AFF_SENSE_LIFE) && (GET_LEVEL(i) < (IMO + 2)))
	    send_to_char_han("You sense a hidden life in the room.\n\r",
			     "방에 숨어있는 생명을 감지합니다.\n\r", ch);
	return;
    }
    if (!(i->player.long_descr) || (GET_POS(i) != i->specials.default_pos)) {
	if (!IS_NPC(i)) {
	    strcpy(buffer, GET_NAME(i));
	    strcat(buffer, " ");
	    if (GET_TITLE(i))
		strcat(buffer, GET_TITLE(i));
	}
	else {
	    strcpy(buffer, i->player.short_descr);
	    CAP(buffer);
	}
	/* NOTE: add linkless PC description. */
	if (!IS_NPC(i) && !i->desc )
	    strcat(buffer, " (linkless)");
	else if (IS_SET(i->specials.act, PLR_WIZINVIS) && !IS_NPC(i))
	    strcat(buffer, " (wizinvis)");
	else if (IS_AFFECTED(i, AFF_INVISIBLE))
	    strcat(buffer, " (invisible)");
	if (GET_POS(i) == POS_FIGHTING ) {
	    if (i->specials.fighting) {
		strcat(buffer, " is here, fighting ");
		if (i->specials.fighting == ch)
		    strcat(buffer, " YOU!");
		else {
		    if (i->in_room == i->specials.fighting->in_room)
			if (IS_NPC(i->specials.fighting))
			    strcat(buffer,
				   i->specials.fighting->player.short_descr);
			else
			    strcat(buffer, GET_NAME(i->specials.fighting));
		    else
			strcat(buffer, "someone who has already left.");
		}
	    }
	    else	/* NIL fighting pointer */
		strcat(buffer, " is here struggling with thin air.");
	}
	/* NOTE: Use position[] string array instead of switch statment. */ 
	else if (GET_POS(i) <= POS_STANDING) 
	    strcat(buffer, position[GET_POS(i)]);
	else 
	    strcat(buffer, " is floating here.");
    }
    else {		/* npc with long */
	if (IS_AFFECTED(i, AFF_INVISIBLE))
	    strcpy(buffer, "*");
	else
	    *buffer = '\0';

	strcat(buffer, i->player.long_descr);

	/* NOTE: remove last CR-LF of long description. */
	remove_eol(buffer);
    }
    /* NOTE: (Red Aura) should be behind long description   */
    if (IS_AFFECTED(ch, AFF_DETECT_EVIL) && IS_EVIL(i))
	strcat(buffer, " (Red Aura)");
    strcat(buffer, "\r\n");
    send_to_char(buffer, ch);

    /* NOTE: Merge SANC & LOVE affection look in single line    */
    /*		to prevent affection description stacked on and on */
    /* NOTE: I don't like to see character's name twice  */
    /* act("$n ....") is changed to act("$e ....") */
    if (IS_AFFECTED(i, AFF_LOVE) && IS_AFFECTED(i, AFF_SANCTUARY))
	act("$e glows with a bright light! and EXCITED with glorious feeling!!",
	    FALSE, i, 0, ch, TO_VICT);
    else {
	if (IS_AFFECTED(i, AFF_SANCTUARY))
	    act("$e glows with a bright light!", FALSE, i, 0, ch, TO_VICT);
	if (IS_AFFECTED(i, AFF_LOVE))
	    act("$e is EXCITED with glorious feeling!!!!",
		FALSE, i, 0, ch, TO_VICT);
    }
    if (IS_AFFECTED(i, AFF_RERAISE))	/* by chase */
	act("$e is calmed down with angelic comfortablity!!!",
	    FALSE, i, 0, ch, TO_VICT);
    if (IS_AFFECTED(i, AFF_DEATH))	/* by process */
	act("$e is deadly UNEASE!!!!!", FALSE, i, 0, ch, TO_VICT);

}

/* NOTE:  OLD show_char_to_char() in mode == 1 */
/* NOTE: Used when char looks 'at' some specific char   */
void look_char_to_char(struct char_data *i, struct char_data *ch)
{
    char buffer[MAX_STRING_LENGTH], *buf;
    int j, found, percent;
    struct obj_data *tmp_obj;

    /* NOTE: check visibility of i to ch */
    if (!ch || !i || !CAN_SEE(ch, i))
	return;

    if (i->player.description)
	strcpy(buffer, i->player.description );
    else
	sprintf(buffer, "You see nothing special about %s.\r\n", HMHR(i));

    /* Show a character to another */

    if (GET_PLAYER_MAX_HIT(i) > 0)
	percent = (100 * GET_HIT(i)) / GET_PLAYER_MAX_HIT(i);
    else
	percent = -1;	/* How could MAX_HIT be < 1?? */

    if (IS_NPC(i))
	strcat(buffer, i->player.short_descr);
    else
	strcat(buffer, GET_NAME(i));

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

    /* send_to_char(buffer, ch); */

    /* NOTE: Init buffer for paging */
    buf = buffer + strlen(buffer);

    found = FALSE;
    for (j = 0; !found && j < MAX_WEAR; j++)
	if (i->equipment[j])
	    if (CAN_SEE_OBJ(ch, i->equipment[j]))
		found = TRUE;

    if (found) {
	/* NOTE: act("\n\r$n is using:", FALSE, i, 0, ch, TO_VICT); */
	sprintf(buf, "\n\r%s is using: \r\n", GET_NAME(i));
	buf += strlen(buf);
	list_equips_buf(i, ch, buf);
	buf += strlen(buf);
    }

    if (((GET_CLASS(ch) == CLASS_THIEF) && (ch != i)) 
    	|| (GET_LEVEL(ch) >= IMO)) {
	found = FALSE;
	strcat(buf, STRHAN("\n\rYou attempt to peek at the inventory:\n\r",
			   "\n\r장비와 물건을 엿보려고 합니다.\n\r", ch));
	buf += strlen(buf);
	for (tmp_obj = i->carrying; tmp_obj;
	     tmp_obj = tmp_obj->next_content) {
	    /* NOTE : Reduced peek success rate for low level thief  */
	    /* number(0,20) -> number(0,40)     */
	    if (CAN_SEE_OBJ(ch, tmp_obj) &&
		(number(0, IMO-1) <= GET_LEVEL(ch))) {
		show_obj_to_buf(tmp_obj, ch, LOOK_INV, buf);
		buf += strlen(buf);
		found = TRUE;
	    }
	}
	if (!found)
	    strcat(buf, STRHAN("You can't see anything.\n\r",
			       "아무 것도 없군요.\n\r", ch));
    } 
    page_string(ch->desc, buffer, 1); 
}

/* NOTE: NEW! list equiped items on char to buffer */
int list_equips_buf(struct char_data *ch, struct char_data *viewer,
		    char *buf)
{
    int j, found = 0;

    *buf = '\0';
    for (j = 0; j < MAX_WEAR; j++) {
	if (ch->equipment[j]) {
	    strcat(buf, where[j]);
	    buf += strlen(buf);
	    if (CAN_SEE_OBJ(viewer, ch->equipment[j]))
		show_obj_to_buf(ch->equipment[j], viewer, LOOK_EQUIP, buf);
	    else
		strcat(buf, "Something.\r\n");
	    buf += strlen(buf);
	    found++;
	}
    }
    return (found);
}

/* NOTE: New! list all objects and all character in the room.
 *    list_obj_to_char() + OLD list_char_to_char()  */ 
void list_all_in_room(int room, struct char_data *ch)
{
    struct char_data *i;

    list_obj_to_char(world[room].contents, ch, LOOK_ROOM);

    for (i = world[room].people; i; i = i->next_in_room)
	show_char_to_char(i, ch);
}

/* NOTE: look_dir() was part of do_look(). */
void look_dir(struct char_data *ch, int dir )
{
    char buffer[MAX_STRING_LENGTH];

    if (EXIT(ch, dir)) { 
	if (EXIT(ch, dir)->general_description) 
	    send_to_char(EXIT(ch, dir)->general_description, ch);
	else
	    send_to_char_han("You see nothing special.\n\r",
			     "별다른 것이 없습니다.\n\r", ch);

	if (IS_SET(EXIT(ch, dir)->exit_info, EX_CLOSED) &&
	    (EXIT(ch, dir)->keyword)) {
	    sprintf(buffer, "The %s is closed.\n\r",
		    fname(EXIT(ch, dir)->keyword));
	    send_to_char(buffer, ch);
	}
	else {
	    if (IS_SET(EXIT(ch, dir)->exit_info, EX_ISDOOR) &&
		EXIT(ch, dir)->keyword) {
		sprintf(buffer, "The %s is open.\n\r",
			fname(EXIT(ch, dir)->keyword));
		send_to_char(buffer, ch);
	    }
	}
    }
    else 
	send_to_char_han("Nothing special there...\n\r",
			 "거기엔 특별한 것이 없습니다 ... \n\r", ch);
}

/* NOTE: look_in() was part of do_look(). */
void look_in(struct char_data *ch, char *arg, int recurse)
{
    struct char_data *tmp_char;
    struct obj_data *tmp_object;
    int bits, temp;
    char buffer[MAX_STRING_LENGTH];

    extern char *fullness[];
    extern char *color_liquid[];

    if (!*arg) {	/* no argument */
	send_to_char_han("Look in what?!\n\r",
			 "무엇의 안을 들여다 본다구요?\n\r", ch);
	return;
    } 
    /* NOTE: Item carried or item in room */
    bits = generic_find(arg, FIND_OBJ_INV | FIND_OBJ_ROOM |
			FIND_OBJ_EQUIP, ch, &tmp_char, &tmp_object); 
    if (!bits) {	/* wrong argument */
	send_to_char_han("You do not see that item here.\n\r",
			 "그런 아이템은 없어요.", ch);
	return;
    }

    /* Found something */
    if (GET_ITEM_TYPE(tmp_object) == ITEM_DRINKCON) {
	if (tmp_object->obj_flags.value[1] <= 0) {
	    acthan("It is empty.", "비어 있습니다.",
		    FALSE, ch, 0, 0, TO_CHAR);
	}
	else {
	    temp = ((tmp_object->obj_flags.value[1] * 3)
		    / tmp_object->obj_flags.value[0]);
	    sprintf(buffer, "It's %sfull of a %s liquid.\n\r",
		    fullness[temp], 
		    color_liquid[tmp_object->obj_flags.value[2]]);
	    send_to_char(buffer, ch);
	}
    }
    else if (GET_ITEM_TYPE(tmp_object) != ITEM_CONTAINER)
	send_to_char_han("That is not a container.\n\r",
		     "이것은 물건을 담을 용기가 아니군요.\n\r", ch);
    else if (IS_SET(tmp_object->obj_flags.value[1], CONT_CLOSED))
	    send_to_char_han("It is closed.\n\r", "닫혔군요", ch);
    else { 
	send_to_char(fname(tmp_object->name), ch);
	switch (bits) {
	case FIND_OBJ_INV:
	    send_to_char(" (carried) : \n\r", ch); break;
	case FIND_OBJ_ROOM:
	    send_to_char(" (here) : \n\r", ch); break;
	case FIND_OBJ_EQUIP:
	    send_to_char(" (used) : \n\r", ch); break;
	}
	if ( recurse )
	    list_obj_to_char(tmp_object->contains, ch, LOOK_BAG);
	else
	    list_obj_to_char(tmp_object->contains, ch, LOOK_CONT);
    }
}

/* NOTE: look_at_obj() was part of do_look(). */
struct obj_data *look_at_obj(struct char_data *ch, char *arg)
{
    struct obj_data *object;
    struct char_data *tmp_char;
    char *tmp_desc;
    int bits, j;
    char buffer[MAX_STRING_LENGTH];

    bits = generic_find(arg, FIND_OBJ_INV | FIND_OBJ_ROOM | FIND_OBJ_EQUIP,
			ch, &tmp_char, &object);

    /* NOTE: Code clean up  */
    if (bits) {
	/* NOTE: Will test visibility or not? */
	if ((tmp_desc = find_ex_description(arg, object->ex_description)))
	    send_to_char(tmp_desc, ch);
	show_obj_to_buf(object, ch, LOOK_AT, buffer);
	send_to_char(buffer, ch);
	return object;
    }

    /* Search for Extra Descriptions in room and items */
    /* Extra description in room?? */
    tmp_desc = find_ex_description(arg, world[ch->in_room].ex_description);
    if (tmp_desc) {
	page_string(ch->desc, tmp_desc, 0);
	return NULL;		/* RETURN SINCE IT WAS A ROOM DESCRIPTION */
	/* Old system was: found = TRUE; */
    }

    /* Search for extra descriptions in items */
    /* Equipment Used */
    for (j = 0; j < MAX_WEAR && !tmp_desc; j++)
	if (ch->equipment[j] && (CAN_SEE_OBJ(ch, ch->equipment[j])))
	    if ((tmp_desc = find_ex_description(arg,
				 ch->equipment[j]->ex_description)))
		object = ch->equipment[j];

    /* In inventory */
    for (object = ch->carrying; object && !tmp_desc;
	 object = object->next_content)
	if (CAN_SEE_OBJ(ch, object))
	    tmp_desc = find_ex_description(arg, object->ex_description);

    /* Object In room */
    for (object = world[ch->in_room].contents;
     object && !tmp_desc; object = object->next_content)
	if (CAN_SEE_OBJ(ch, object))
	    tmp_desc = find_ex_description(arg, object->ex_description);

    /* NOTE: print first found extra description, if any  */
    if (tmp_desc && object) {
	/* NOTE: page_string(ch->desc, tmp_desc, 0);  */
	send_to_char(tmp_desc, ch);
	/* NOTE: a blanks line between extra desc and show_obj */
	send_to_char("\r\n", ch);
	/* NOTE: show obj, not found_obj!!! */
	show_obj_to_buf(object, ch, LOOK_EXTRA, buffer);
	send_to_char(buffer, ch);
	return object;
    }
    else /* wrong argument */
	send_to_char_han("You do not see that here.\n\r",
			 "그런 것은 여기에 없습니다.\n\r", ch);
    return NULL;
}

/* NOTE:  do_look() is divided into do_look(), look_dir(), 
	look_in(), look_at_obj(). */
void do_look(struct char_data *ch, char *argument, int cmd)
{
    char buffer[MAX_STRING_LENGTH];
    char arg1[MAX_INPUT_LENGTH];
    int keyword_no;
    struct char_data *tmp_char;

    extern char *room_bits[];

    /* Wizard switched to mob can 'look' room */
    if (!ch || !ch->desc || IS_MOB(ch))
	return;

    if (GET_POS(ch) < POS_SLEEPING)
	send_to_char_han("You can't see anything but stars!\n\r",
			 "별들이 아름답게 빛나고 있습니다.\n\r", ch);
    else if (GET_POS(ch) == POS_SLEEPING)
	send_to_char_han("You can't see anything, you're sleeping!\n\r",
			 "아마도 지금 꿈을 꾸는게 아닌지요 ??\n\r", ch);
    else if (ch && IS_AFFECTED(ch, AFF_BLIND))
	send_to_char_han("You can't see a damn thing, you're blinded!\n\r",
		     "앞을 볼 수가 없습니다! 눈이 멀었습니다!\n\r", ch);
    else if ( ch && GET_LEVEL(ch) < IMO && IS_DARK(ch->in_room)
	    && !IS_AFFECTED(ch, AFF_INFRAVISION))
	send_to_char_han("It is pitch black...\n\r",
			 "너무 깜깜합니다..\n\r", ch);
    else
	goto ok;
    return;
ok:
    keyword_no = look_argument(argument, arg1);

    switch (keyword_no) {
    case 0: case 1: case 2: case 3: case 4: case 5:
	/* look <dir> */
	look_dir(ch, keyword_no); 
	break;

    case 6: /* look 'in'  */
	look_in(ch, arg1, TRUE);
	break;

    case 7: /* look 'at'  */
	if (!*arg1) {	/* no argument */
	    send_to_char_han("Look at what?\n\r",
			     "무엇을 바라보라고요 ?\n\r", ch);
	    break;
	}

	tmp_char = get_char_room_vis(ch, arg1);
	if (tmp_char) {
	    look_char_to_char(tmp_char, ch);
	    if (ch != tmp_char) {
		acthan("$n looks at you.", "$n님이 당신을 바라봅니다.",
		       TRUE, ch, 0, tmp_char, TO_VICT);
		acthan("$n looks at $N.", "$n님이 $N님을 바라봅니다.",
		       TRUE, ch, 0, tmp_char, TO_NOTVICT);
	    }
	}
	else 
	    look_at_obj(ch, arg1);
	break;

	/* look ''    */
    case 8:{
	    if (GET_LEVEL(ch) >= IMO) {

		sprintf(buffer, "%s  [%5d] [ ", world[ch->in_room].name,
			world[ch->in_room].number);
		sprintbit((long) world[ch->in_room].room_flags, room_bits,
			  buffer + strlen(buffer));
		strcat(buffer, "]\n\r");
	    }
	    else
		sprintf(buffer, "%s\n\r", world[ch->in_room].name);
	    send_to_char(buffer, ch);
	    if (!IS_SET(ch->specials.act, PLR_BRIEF))
		send_to_char(world[ch->in_room].description, ch);
	    sprintf(buffer, "[ EXITS : ");
	    if (EXIT(ch, 0)) strcat(buffer, "N ");
	    if (EXIT(ch, 1)) strcat(buffer, "E ");
	    if (EXIT(ch, 2)) strcat(buffer, "S ");
	    if (EXIT(ch, 3)) strcat(buffer, "W ");
	    if (EXIT(ch, 4)) strcat(buffer, "U ");
	    if (EXIT(ch, 5)) strcat(buffer, "D ");
	    strcat(buffer, " ]\n\r");
	    send_to_char(buffer, ch);

	    list_all_in_room(ch->in_room, ch);
/* NOTE: OLD look room
   list_obj_to_char(world[ch->in_room].contents, ch, 0,FALSE);
   list_char_to_char(world[ch->in_room].people, ch, 0);
 */
	}
	break;

	/* wrong arg  */
    case -1:
	send_to_char_han("Sorry, I didn't understand that!\n\r",
			 "미안합니다. 이해를 못하겠네요.\n\r", ch);
	break;
    }
}
/* end of look */ 

void do_examine(struct char_data *ch, char *argument, int cmd)
{
    char arg[MAX_INPUT_LENGTH], buf[MAX_BUFSIZ];
    struct obj_data *object;

    extern int weapon_type(struct obj_data *weapon); 
    extern char *weapon_type_desc[];

    /* NOTE: Redundant errer msg on null arg. Check arg number first. */
    one_argument(argument, arg);

    if (!*arg) {
	send_to_char_han("Examine what?\n\r", "무엇을 조사하시려구요?\n\r", ch);
	return;
    }

    /*	NOTE: Use look_at_obj() and look_in() directly, not via do_look(). */ 
    object = look_at_obj(ch, arg); 
    if (!object) 
	return;

    if ((GET_ITEM_TYPE(object) == ITEM_CONTAINER )
	|| (GET_ITEM_TYPE(object) == ITEM_DRINKCON)) {
	send_to_char_han("When you look inside, you see:\r\n",
			 "안을 들여다 보니 이런 것이 있군요.\r\n", ch);
	look_in(ch, arg, FALSE );
    }
    /* NOTE: Move looking wrtings on 'note' from do_look() to here. */ 
    else if (GET_ITEM_TYPE(object) == ITEM_NOTE ) {
	if (object->action_description) {
	    strcpy(buf, "There is something written upon it:\n\r\n\r");
	    strcat(buf, object->action_description);
	}
	else
	    strcpy(buf, STRHAN("It's blank.", "비어 있습니다.", ch));
	strcat(buf, "\r\n");
    }
    /* NOTE: Move looking weapon type description from do_look() to here. */
    else if (GET_ITEM_TYPE(object) == ITEM_WEAPON ) {
	/* NOTE: gives more specific info about WEAPON TYPE */
	sprinttype(weapon_type(object) - TYPE_HIT, weapon_type_desc, buf);
	strcat(buf, "\r\n"); 
    }
    else if (GET_ITEM_TYPE(object) == ITEM_FIREWEAPON) {
	sprintf(buf, "There are %d shots left.\n\r",
		object->obj_flags.value[0]);
	send_to_char(buf, ch);
    }
}

void do_read(struct char_data *ch, char *argument, int cmd)
{
    /* This is just for now - To be changed later.! */
    /* NOTE: Use look_at_obj() directly, not via do_look(). */

    look_at_obj(ch, argument);
} 

void do_inventory(struct char_data *ch, char *argument, int cmd)
{
    send_to_char_han("You are carrying:\n\r", "가지고 있는 물건들:\n\r", ch);
    list_obj_to_char(ch->carrying, ch, LOOK_INV);
}

void do_equipment(struct char_data *ch, char *argument, int cmd)
{
    int found;
    char buffer[MAX_STRING_LENGTH];

    send_to_char_han("You are using:\n\r", "쓰고 있는 물건들:\n\r", ch);
    found = list_equips_buf(ch, ch, buffer);

    if (found)
	send_to_char(buffer, ch);
    else
	send_to_char_han(" Nothing.\n\r", " 아무것도 없네요.\n\r", ch);
} 

/*
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
 */

static char *align_msg[13] = {
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

static char *align_msg_han[13] = {
    "당신은 성인 군자십니다.\n\r",
    "당신은 성이 군자가 되어가고 있습니다.\n\r",
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

/* NOTE: NEW! position description string array */
static char *position[] = {
    "You are DEAD!", 
    "You are mortally wounded!, you should seek help!",
    "You are incapacitated, slowly fading away.",
    "You are stunned! You can't move.",
    "You are sleeping.", 
    "You are resting.", 
    "You are sitting.",
    "!FIGHTING!",
    "You are standing.",
    "You are floating.",
};

static char *position_han[] = {
    "당신은 죽으셨습니다.", 
    "당신은 치명적으로 다쳤습니다. 죽음이 다가오고 있습니다.", 
    "당신은 심하게 다쳐서 정신이 흐려지고 있습니다.", 
    "당신은 기절해서 움직일 수 없습니다.", 
    "당신은 자고 있습니다.",
    "당신은 쉬고 있습니다.",
    "당신은 앉아 있습니다.",
    "!FIGHTING!",
    "당신은 서있습니다.",
    "당신은 공중에 떠 있습니다.",
};

void do_score(struct char_data *ch, char *argument, int cmd)
{
    struct affected_type *aff;
    struct time_info_data playing_time;
    char buf[MAX_BUFSIZ];
    struct time_info_data real_time_passed(time_t t2, time_t t1);
    int tmp, align, brief;
    struct char_data *to, *tmpch ;

    extern byte saving_throws[4][5][IMO + 10];

    one_argument(argument, buf);

    brief = 0 ; /* NOTE: Default is "full" score mode */
    to = ch ;   /* NOTE: Default is see my own 'score' */
    if (*buf) {
	/* NOTE: breif mode: skip uninteresting score */
	if (!strcmp(buf,"-b") || !strcmp(buf,"/brief"))
	    brief++;
	/* NOTE: Wizard can see 'score' of other player/ mobile */
	else if( GET_LEVEL(ch) >= IMO ) {
	    if ((tmpch = get_char_vis(ch, buf))) {
		ch = tmpch ; 	/* NOTE: See player/mob's score */
		sprintf(buf, "Looking for score of %s.\r\n", GET_NAME(ch));
		send_to_char(buf, to); 
	    }
	    else {
		send_to_char("score who?\r\n", to );
		return;
	    }
	}
    }
	
    /* alignment message */
    tmp = GET_ALIGNMENT(ch) / 50;
    if (tmp > 17) align = 0;
    else if (tmp > 12) align = 1;
    else if (tmp > 7) align = 2;
    else if (tmp > 6) align = 3;
    else if (tmp > 5) align = 4;
    else if (tmp > 2) align = 5;
    else if (tmp > -3) align = 6;
    else if (tmp > -6) align = 7;
    else if (tmp > -7) align = 8;
    else if (tmp > -8) align = 9;
    else if (tmp > -13) align = 10;
    else if (tmp > -18) align = 11;
    else align = 12;

    sprintf(buf, STRHAN("Your AC is  %d.    %s",
			"당신의  방어력은  %d 입니다.    %s", ch),
      ch->points.armor, STRHAN(align_msg[align], align_msg_han[align], ch));
    send_to_char(buf, to);

    sprintf(buf,
     STRHAN("You have  %d(%d) hit, %d(%d) mana and %d(%d) move points.\n\r",
       "당신은  %d(%d) hit, %d(%d) mana와 %d(%d) move가 있습니다.\n\r", ch),
	    GET_HIT(ch), GET_PLAYER_MAX_HIT(ch),
	    GET_MANA(ch), GET_PLAYER_MAX_MANA(ch),
	    GET_MOVE(ch), GET_PLAYER_MAX_MOVE(ch));
    send_to_char(buf, to);

/*  
   tmp = ch->points.armor/10;
   switch( tmp )
   { case 10: send_to_char( ac_msg[0], ch ); break;
   case 9: case 8:
	send_to_char( ac_msg[1], ch ); break;
   case 7: case 6:
	send_to_char( ac_msg[2], ch ); break;
   case 5: case 4:
	send_to_char( ac_msg[3], ch ); break;
   case 3: case 2:
	send_to_char( ac_msg[4], ch ); break;
   case 1: case 0:
	send_to_char( ac_msg[5], ch ); break;
   case -1: case -2: case -3:
	send_to_char( ac_msg[6], ch ); break;
   case -4: case -5: case -6:
	send_to_char( ac_msg[7], ch ); break;
   case -7: case -8: case -9:
	send_to_char( ac_msg[8], ch ); break;
   case -10: case -11: case -12:
	send_to_char( ac_msg[9], ch ); break;
   case -13: case -14: case -15:
	send_to_char( ac_msg[10], ch ); break;
   case -16: case -17: case -18:
	send_to_char( ac_msg[11], ch ); break;
   default:
	send_to_char( ac_msg[12], ch ); break;
   } 
 */

    if (GET_LEVEL(ch) > 12) {
	if( !brief ) {
	    send_to_char_han("Your stat:", "당신의  체질:", to);
	    sprintf(buf, "  str %d/%d wis %d int %d dex %d con %d.\n\r",
		    GET_STR(ch), GET_ADD(ch), GET_WIS(ch), GET_INT(ch),
		    GET_DEX(ch), GET_CON(ch));
	    send_to_char(buf, to);
	}

	sprintf(buf, STRHAN( "Your hitroll is %d , and damroll is %d."
			     "   Bare Hand Dice is  %dd%d.\n\r",
			     "당신의  hitroll은 %d , damroll은 %d."
			     "  맨손 위력은  %dd%d 입니다.\n\r", ch),
			     GET_HITROLL(ch), GET_DAMROLL(ch),
		ch->specials.damnodice, ch->specials.damsizedice);
	send_to_char(buf, to);	/* changed by shin won dong */

	sprintf(buf, STRHAN(
	      "You save para: %d, hit skill: %d, breath: %d, spell: %d."
			       "  Regeneration: %d.\n\r",
	  "마법면역도는 para: %d, hit skill: %d, breath: %d, spell: %d."
			       "  회복도: %d.\n\r", ch),
		IS_NPC(ch) ? ch->specials.apply_saving_throw[SAVING_PARA] :
		ch->specials.apply_saving_throw[SAVING_PARA] +
	   saving_throws[GET_CLASS(ch) - 1][SAVING_PARA][GET_LEVEL(ch) - 1],
	    IS_NPC(ch) ? ch->specials.apply_saving_throw[SAVING_HIT_SKILL] :
		ch->specials.apply_saving_throw[SAVING_HIT_SKILL] +
		saving_throws[GET_CLASS(ch) - 1][SAVING_HIT_SKILL][GET_LEVEL(ch) - 1],
		IS_NPC(ch) ? ch->specials.apply_saving_throw[SAVING_BREATH] :
		ch->specials.apply_saving_throw[SAVING_BREATH] +
	 saving_throws[GET_CLASS(ch) - 1][SAVING_BREATH][GET_LEVEL(ch) - 1],
		IS_NPC(ch) ? ch->specials.apply_saving_throw[SAVING_SPELL] :
		ch->specials.apply_saving_throw[SAVING_SPELL] +
	  saving_throws[GET_CLASS(ch) - 1][SAVING_SPELL][GET_LEVEL(ch) - 1],
		ch->regeneration);
	if ( !brief )
	    send_to_char(buf, to);
    }

    sprintf(buf,STRHAN("You have scored  %s exp,  and have  %s gold coins.\n\r",
	   "당신은  %s 의 경험치와  %s 원의 돈을 가지고 있습니다.\n\r", ch),
	    monetary(GET_EXP(ch)), monetary(GET_GOLD(ch)));
    send_to_char(buf, to);

    playing_time = real_time_passed((time(0) - ch->player.time.logon) +
				    ch->player.time.played, 0);
    sprintf(buf, STRHAN(
   "You are %d years old, and have been playing for %d days and %d hours.\n\r",
      "당신은  %d 살이고, 이 안에서 %d 일 %d 시간동안 있었습니다.\n\r", ch),
	    age(ch).year, playing_time.day, playing_time.hours);
    if ( !brief )
	send_to_char(buf, to);

    /* NOTE: Add class, too */
    sprintf(buf, STRHAN("This ranks you as  %s  %s (level %c %2d).\n\r",
			"당신은  %s  %s (레벨 %c %2d) 입니다.\n\r", ch),
	    GET_NAME(ch), (GET_TITLE(ch) ? GET_TITLE(ch) : "No Title"), 
	    "UMCTWU"[GET_CLASS(ch)], GET_LEVEL(ch));
    if ( !brief )
	send_to_char(buf, to);
    if (GET_LEVEL(ch) < IMO) {
	if (titles[GET_CLASS(ch) - 1][GET_LEVEL(ch) + 1].exp < GET_EXP(ch))
	    strcpy(buf, STRHAN("You have enough experience to advance.\n\r",
		    "레벨을 올릴만큼 충분한 경험치가 쌓였습니다.\n\r", ch));
	else
	    sprintf(buf, STRHAN("You need  %s experience to advance.\n\r",
		   "다음 레벨까지  %s 만큼의 경험치가 필요합니다.\n\r", ch),
		    monetary(titles[GET_CLASS(ch) - 1][GET_LEVEL(ch) + 1].exp - GET_EXP(ch)));
	send_to_char(buf, to);
    }

    if (GET_COND(ch, DRUNK) > 10)
	send_to_char_han("You are intoxicated.\n\r",
			 "당신은 취해 있습니다.\n\r", to);

    if (GET_POS(ch) == POS_FIGHTING ) {
	if (ch->specials.fighting) {
	    sprintf(buf, STRHAN("You are fighting %s.", 
	"당신은 %s님과 싸우고 있습니다.", ch), GET_NAME(ch->specials.fighting));
	   send_to_char(buf, to);
	}
	else
	   send_to_char("You are fighting thin air." , to);
    }
    else if ( GET_POS(ch) <= POS_STANDING ) {
	/* NOTE: Use string array position[] and position_han[] 
		to print my position instead of switch statement. */ 
	send_to_char_han(position[GET_POS(ch)], position_han[GET_POS(ch)], to);
    }
    else
	send_to_char("Hmmm.. You are in odd position", to);

    /* NOTE: Show whom you are following *after* position printing. */ 
    if(!brief && ch->master  && GET_NAME(ch->master)) {
	sprintf(buf, STRHAN( "    You are following %s.", 
	    "    당신은 %s 님을 따르고 있습니다.", to), GET_NAME(ch->master));
	send_to_char(buf, to);
    }
    send_to_char("\r\n", to);
	
    if (ch->affected) {
	send_to_char(STRHAN("Affecting Spells:\n\r",
		"걸려있는 마법들:\n\r", ch), to );
	for (aff = ch->affected; aff; aff = aff->next) {
	    /* NOTE:  spells index starts from 1, not zero.   */
	    sprintf(buf, STRHAN("%s: %d hrs\n\r", "%s: %d 시간\n\r", ch),
		    spells[aff->type], aff->duration);
	    send_to_char(buf, to);
	}
    }

    sprintf(buf, STRHAN(
   "\r\nYou have been killed  #%d time(s) and have killed  #%d player(s).\r\n",
   "\r\n당신은  지금까지  #%d 번 죽었고  #%d 명을 죽였습니다.\r\n", ch),
	    ch->player.pked_num, ch->player.pk_num);
    if ( !brief )
	send_to_char(buf, to);
    if (!brief && ch->player.guild >= 1 && ch->player.guild <= MAX_GUILD_LIST) {
	sprintf(buf, STRHAN("You are a member of %s guild.    ",
			    "당신은  %s 길드의 회원입니다.    ", ch),
		guild_names[ch->player.guild]);
	send_to_char(buf, to);
    }
    sprintf(buf, STRHAN("You have made  #%d QUEST(s).\r\n",
	 "당신은 현재 %d번 QUEST를 풀었습니다.\r\n", ch), ch->quest.solved);
    send_to_char(buf, to);

}			/* end of do_score() */

void do_report(struct char_data *ch, char *argument, int cmd)
{
    char buf[100];

    sprintf(buf, "%s %d/%d hp, %d/%d mn, %d/%d mv", GET_NAME(ch),
	    GET_HIT(ch), GET_PLAYER_MAX_HIT(ch), GET_MANA(ch),
	    GET_PLAYER_MAX_MANA(ch), GET_MOVE(ch), GET_PLAYER_MAX_MOVE(ch));
    /* NOTE: Don't report to whom cannot see me */
    act(buf, TRUE, ch, 0, 0, TO_ROOM);
    /* NOTE: Report to myself, too */
    act(buf, FALSE, ch, 0, 0, TO_CHAR);
    /* send_to_char("ok.\n\r",ch); */
}

#ifdef NO_DEF
/* From msd mud : cyb */
void do_attribute(struct char_data *ch, char *argument, int cmd)
{
    char buf[MAX_STRING_LENGTH];
    struct affected_type *aff;

    sprintf(buf,
	"You are %d years and %d months, %d cms, and you weigh %d lbs.\n\r",
	    age(ch).year, age(ch).month, ch->player.height, ch->player.weight);

    send_to_char(buf, ch);

    sprintf(buf, "You are carrying %d lbs of equipment.\n\r",
	    GET_CARRYING_W(ch));
    send_to_char(buf, ch);

    /*  sprintf(buf,"Your armor is %d (-100 to 100).\n\r",ch->points.armor);
	send_to_char(buf,ch); */

    /* if ((GET_LEVEL(ch) > 15) || (GET_CLASS(ch))) {    cyb */
    if ((GET_LEVEL(ch) > 15)) {
	if ((GET_STR(ch) == 18) && (GET_CLASS(ch))) {
	    sprintf(buf, "You have %d/%d STR, %d INT, %d WIS, %d DEX, %d CON\n\r",
		    GET_STR(ch), GET_ADD(ch), GET_INT(ch), GET_WIS(ch),
		    GET_DEX(ch), GET_CON(ch));
	    send_to_char(buf, ch);
	}
	else {
	    sprintf(buf, "You have %d STR, %d INT, %d WIS, %d DEX, %d CON\n\r",
	    GET_STR(ch), GET_INT(ch), GET_WIS(ch), GET_DEX(ch), GET_CON(ch));
	    send_to_char(buf, ch);
	}
    }

    sprintf(buf, "Your hitroll and damroll are %d and %d respectively.\n\r",
	    GET_HITROLL(ch), GET_DAMROLL(ch));
    send_to_char(buf, ch);

    /* **   by popular demand -- affected stuff */
    if (ch->affected) {
	send_to_char("\n\rAffecting Spells:\n\r--------------\n\r", ch);
	for (aff = ch->affected; aff; aff = aff->next) {
	    switch (aff->type) {
	    /* NOTE: show affection by sneak, poision or curse. */
	    /*
	    case SKILL_SNEAK:
	    case SPELL_POISON:
	    case SPELL_CURSE:
	    */
		break;
	    default:
		/* NOTE:  spells index starts from 1, not zero.   */
		sprintf(buf, "Spell : '%s'\n\r", spells[aff->type]);
		send_to_char(buf, ch);
		break;
	    }
	}
    }
}

/* end of Attribute Module... */ 
#endif		/* NO_DEF */

void do_data(struct char_data *ch, char *argument, int cmd)
{
    struct descriptor_data *d;
    struct char_data *victim;
    char buf[256], name[256], fmt[16];
    int i = 0, k, nc;
    LONGLONG n;
    static char *keywords[] = { "",
	"exp", "hit", "gold", "armor", "age",
	"time", "flags", "bank", "des", "level",
    };
#define KEYS 	"exp hit gold armor age time flags bank des level" 

    one_argument(argument, name);
    /* NOTE: Use search_block() instead of nested if strcmp() */
    k = search_block( name, keywords, 0 );
    if( k <= 0 ) {
	send_to_char("Keywords: " KEYS "\r\n", ch);
	return;
    }
    if ((k == 1) || (k == 3) || (k == 7) || (k == 8)) {
	nc = 3;
	strcpy(fmt, "%-12s%13lld%s");
    }
    else {
	nc = 4;
	strcpy(fmt, "%-12s%6lld%s");
    }
    for (d = descriptor_list; d; d = d->next) {
	if (!d->connected && CAN_SEE(ch, d->character)) {
	    ++i;
	    if (d->original)	/* If switched */
		victim = d->original;
	    else
		victim = d->character;
	    switch (k) {
	    case 1: n = victim->points.exp; break;
	    case 2: n = victim->points.max_hit; break;
	    case 3: n = victim->points.gold; break;
	    case 4: n = victim->points.armor; break;
	    case 5: n = (time(0) - victim->player.time.birth) / 86400; break;
	    case 6: n = (60 * victim->desc->ncmds) 
		    / (1 + time(0) - victim->desc->contime); break;
	    case 7: n = victim->specials.act; break;
	    case 8: n = victim->bank; break;
	    case 9: n = d->descriptor; break;
	    case 10: n = victim->player.level; break;
	    default:
		n = -1;
	    }
	    sprintf(buf, fmt, GET_NAME(victim), n, (i % nc) ? "|" : "\n\r");
	    send_to_char(buf, ch);
	}
    }
    if (i % nc)
	send_to_char("\n\r", ch);
}
