
/* ************************************************************************
   *  file: handler.c , Handler module.                      Part of DIKUMUD *
   *  Usage: Various routines for moving about objects/players               *
   *  Copyright (C) 1990, 1991 - see 'license.doc' for complete information. *
   ************************************************************************* */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <time.h>


#include "char.h"
#include "object.h"
#include "global.h"
#include "comm.h"


/* NOTE: These get_char_*(), get_obj_*() function was moved from handler.c */
int get_number(char **name)
{

    int i;
    char *ppos;
    char number[MAX_INPUT_LENGTH];

    number[0] = '\0';
    if ((ppos = (char *) index((char *) (*name), '.'))) {
	*(ppos++) = '\0';
	strcpy(number, *name);
	strcpy(*name, ppos);

	for (i = 0; *(number + i); i++)
	    if (!ISDIGIT(*(number + i)))
		return (0);

	return (atoi(number));
    }

    return (1);
}

/* Search a given list for an object, and return a pointer to that object */
struct obj_data *get_obj_in_list(char *name, struct obj_data *list)
{
    struct obj_data *i;
    int j, number;
    char tmpname[MAX_INPUT_LENGTH];
    char *tmp;

    strcpy(tmpname, name);
    tmp = tmpname;
    if (!(number = get_number(&tmp)))
	return (0);

    for (i = list, j = 1; i && (j <= number); i = i->next_content)
	if (isname(tmp, i->name)) {
	    if (j == number)
		return (i);
	    j++;
	}

    return (0);
}

/* Search a given list for an object number, and return a ptr to that obj */
struct obj_data *get_obj_in_list_num(int num, struct obj_data *list)
{
    struct obj_data *i;

    for (i = list; i; i = i->next_content)
	if (i->item_number == num)
	    return (i);

    return (0);
}

/*search the entire world for an object, and return a pointer  */
struct obj_data *get_obj(char *name)
{
    struct obj_data *i;
    int j, number;
    char tmpname[MAX_INPUT_LENGTH];
    char *tmp;

    strcpy(tmpname, name);
    tmp = tmpname;
    if (!(number = get_number(&tmp)))
	return (0);

    for (i = object_list, j = 1; i && (j <= number); i = i->next)
	if (isname(tmp, i->name)) {
	    if (j == number)
		return (i);
	    j++;
	}

    return (0);
}


/*search the entire world for an object number, and return a pointer  */
struct obj_data *get_obj_num(int nr)
{
    struct obj_data *i;

    for (i = object_list; i; i = i->next)
	if (i->item_number == nr)
	    return (i);

    return (0);
}


/* search a room for a char, and return a pointer if found..  */
/*
struct char_data *get_char_room(char *name, int room)
{
    struct char_data *i;
    int j, number;
    char tmpname[MAX_INPUT_LENGTH];
    char *tmp;

    strcpy(tmpname, name);
    tmp = tmpname;
    if (!(number = get_number(&tmp)))
	return (0);

    for (i = world[room].people, j = 1; i && (j <= number); i = i->next_in_room)
	if (isname(tmp, GET_NAME(i))) {
	    if (j == number)
		return (i);
	    j++;
	}

    return (0);
}
*/

/* search all over the world for a char, and return a pointer if found */
struct char_data *get_char(char *name)
{
    struct char_data *i;
    int j, number;
    char tmpname[MAX_INPUT_LENGTH];
    char *tmp;

    strcpy(tmpname, name);
    tmp = tmpname;
    if (!(number = get_number(&tmp)))
	return (0);
    for (i = character_list, j = 1; i && (j <= number); i = i->next)
	if (isname(tmp, GET_NAME(i))) {
	    if (j == number)
		return (i);
	    j++;
	}
    return (0);
}

/* search all over the world for a char num, and return a pointer if found */
struct char_data *get_char_num(int nr)
{
    struct char_data *i;

    for (i = character_list; i; i = i->next)
	if (i->nr == nr)
	    return (i);

    return (0);
} 


/* ***********************************************************************
   Here follows high-level versions of some earlier routines, ie functions
   which incorporate the actual player-data.
   *********************************************************************** */ 

struct char_data *get_char_room_vis(struct char_data *ch, char *name)
{
    struct char_data *i;
    int j, number;
    char tmpname[MAX_INPUT_LENGTH];
    char *tmp;

    strcpy(tmpname, "ALL");
    if (name)
	strcpy(tmpname, name);
    tmp = tmpname;
    if (!(number = get_number(&tmp)))
	return (0);

    for (i = world[ch->in_room].people, j = 1; i && (j <= number); i = i->next_in_room)
	if (isname(tmp, GET_NAME(i)))
	    if (CAN_SEE(ch, i)) {
		if (j == number)
		    return (i);
		j++;
	    }

/* NOTE:  'self' is generic name refering yourself. ( cast 'heal' self )
   This works even when you are blind or invisible. (Goooood!)  */
    if ( ch && !strcmp(name, "self") )
	return (ch);

/* NOTE: 'leader' is generic name refering player you follow. ( assist leader ) 
   This applies only when you are following PC player leader,
   can see him/her, and in same room. ( grouping not needed )    */
    if (ch->master && !strcmp(name, "leader") && !IS_NPC(ch->master)
	&& (ch->in_room == ch->master->in_room) && CAN_SEE(ch, ch->master))
	return (ch->master); 

/* NOTE: 'target' is geneic name PC/NPC char you are fighting now. 
	Or victim fleed while fighting you or you did 'lightning move'. 
	If you are not fighting, 'target' is char you or your leader 
	'pointed' last time. target is unset when target died. */
    if (!strcmp(name, "target")) {
	struct char_data *target ;

	target = ( GET_POS(ch) == POS_FIGHTING ) ? 
	    ch->specials.fighting : ch->specials.hunting ; 
	if (target && (ch->in_room == target->in_room) && CAN_SEE(ch, target))
	    return (target); 
    }

    return (NULL);
}

struct char_data *get_char_vis_zone(struct char_data *ch, char *name)
{
    struct char_data *i;
    int j, number;
    char tmpname[MAX_INPUT_LENGTH];
    char *tmp;

    /* check location */
    if ((i = get_char_room_vis(ch, name)))
	return (i);

    strcpy(tmpname, "ALL");
    if (name)
	strcpy(tmpname, name);
    tmp = tmpname;
    if (!(number = get_number(&tmp)))
	return (0);

    for (i = character_list, j = 1; i && (j <= number); i = i->next)
	if (isname(tmp, GET_NAME(i)))
	    if (CAN_SEE(ch, i) && world[ch->in_room].zone
		    == world[i->in_room].zone) {
		if (j == number)
		    return (i);
		j++;
	    }

    return (0);
}

struct char_data *get_char_vis(struct char_data *ch, char *name)
{
    struct char_data *i;
    int j, number;
    char tmpname[MAX_INPUT_LENGTH];
    char *tmp;

    /* check location */
    if ((i = get_char_room_vis(ch, name)))
	return (i);

    /* NOTE: Search char in zone before search all over the world */
    if ((i = get_char_vis_zone(ch, name)))
	return (i);

    strcpy(tmpname, "ALL");
    if (name)
	strcpy(tmpname, name);
    tmp = tmpname;
    if (!(number = get_number(&tmp)))
	return (0);

    for (i = character_list, j = 1; i && (j <= number); i = i->next)
	if (isname(tmp, GET_NAME(i)))
	    if (CAN_SEE(ch, i)) {
		if (j == number)
		    return (i);
		j++;
	    }

    return (0);
} 

/* NOTE: char_specific()  is rewritten as get_char_wiz()                 */
/* NOTE: 3rd arg (type) is bit vector, not bool        */
/* NOTE: This func is called solely by do_stat() in act.wizard.c  */
struct char_data *get_char_wiz(struct char_data *ch, char *name, int type)
{
    struct char_data *i;

/* NOTE: Added check for mob or player */
#define TYPEOK(x, type) (( IS_NPC(x) ? 02 : 01 ) & (type))

    if ((i = get_char_room_vis(ch, name)) && TYPEOK(i, type))
	return (i);
    /* NOTE: search for mob/char in this zone before seaching whole world */
    if ((i = get_char_vis_zone(ch, name)) && TYPEOK(i, type))
	return (i);
    if ((i = get_char(name)) && TYPEOK(i, type))
	return (i);

    for (i = character_list; i; i = i->next)
	if (TYPEOK(i, type) && isname(name, GET_NAME(i)))
	    return (i);

    return (0);
} 

struct obj_data *get_obj_in_list_vis(struct char_data *ch, char *name,
				     struct obj_data *list)
{
    struct obj_data *i;
    int j, number;
    char tmpname[MAX_INPUT_LENGTH];
    char *tmp;

    strcpy(tmpname, "ALL");
    if (name)
	strcpy(tmpname, name);
    tmp = tmpname;
    if (!(number = get_number(&tmp)))
	return (0);

    for (i = list, j = 1; i && (j <= number); i = i->next_content)
	if (isname(tmp, i->name))
	    if (CAN_SEE_OBJ(ch, i)) {
		if (j == number)
		    return (i);
		j++;
	    }
    return (0);
}


/*search the entire world for an object, and return a pointer  */
struct obj_data *get_obj_vis(struct char_data *ch, char *name)
{
    struct obj_data *i;
    int j, number;
    char tmpname[MAX_INPUT_LENGTH];
    char *tmp;

    /* scan items carried */
    if ((i = get_obj_in_list_vis(ch, name, ch->carrying)))
	return (i);

    /* scan room */
    if ((i = get_obj_in_list_vis(ch, name, world[ch->in_room].contents)))
	return (i);

    strcpy(tmpname, "ALL");
    if (name)
	strcpy(tmpname, name);
    tmp = tmpname;
    if (!(number = get_number(&tmp)))
	return (0);

    /* ok.. no luck yet. scan the entire obj list   */
    for (i = object_list, j = 1; i && (j <= number); i = i->next)
	if (isname(tmp, i->name))
	    if (CAN_SEE_OBJ(ch, i)) {
		if (j == number)
		    return (i);
		j++;
	    }
    return (0);
}

struct obj_data *get_object_in_equip_vis(struct char_data *ch,
			    char *arg, struct obj_data *equipment[], int *j)
{

    for ((*j) = 0; (*j) < MAX_WEAR; (*j)++)
	if (equipment[(*j)])
	    if (CAN_SEE_OBJ(ch, equipment[(*j)]))
		if (isname(arg, equipment[(*j)]->name))
		    return (equipment[(*j)]);

    return (0);
}


/* Generic Find, designed to find any object/character                    */
/* Calling :                                                              */
/*  *arg     is the sting containing the string to be searched for.       */
/*           This string doesn't have to be a single word, the routine    */
/*           extracts the next word itself.                               */
/*  bitv..   All those bits that you want to "search through".            */
/*           Bit found will be result of the function                     */
/*  *ch      This is the person that is trying to "find"                  */
/*  **tar_ch Will be NULL if no character was found, otherwise points     */
/* **tar_obj Will be NULL if no object was found, otherwise points        */
/*                                                                        */
/* The routine returns a pointer to the next word in *arg (just like the  */
/* one_argument routine).                                                 */

int generic_find(char *arg, int bitvector, struct char_data *ch,
		 struct char_data **tar_ch, struct obj_data **tar_obj)
{
    static char *ignore[] =
    { "in", "on", "at", "\n"};

    int i;
    char name[256];
    bool found;

    found = FALSE; 

    /* Eliminate spaces and "ignore" words */
    while (*arg && !found) { 
	for (; *arg == ' '; arg++) ;

	for (i = 0; (name[i] = *(arg + i)) && (name[i] != ' '); i++) ;
	name[i] = 0;
	arg += i;
	if (search_block(name, ignore, TRUE) > -1)
	    found = TRUE; 
    }

    if (!name[0])
	return (0);

    *tar_ch = 0;
    *tar_obj = 0;

    /* NOTE: Little code clean up */
    if (IS_SET(bitvector, FIND_CHAR_ROOM)	/* Find person in room */
	&& (*tar_ch = get_char_room_vis(ch, name)))
	    return (FIND_CHAR_ROOM);

    if (IS_SET(bitvector, FIND_CHAR_WORLD)
	&& (*tar_ch = get_char_vis(ch, name))) 
	    return (FIND_CHAR_WORLD);

    if (IS_SET(bitvector, FIND_OBJ_EQUIP)) {
	for (found = FALSE, i = 0; i < MAX_WEAR && !found; i++)
	    if (ch->equipment[i] && !str_cmp(name, ch->equipment[i]->name)) {
		*tar_obj = ch->equipment[i];
		found = TRUE;
	    }
	if (found) 
	    return (FIND_OBJ_EQUIP);
    }

    if (IS_SET(bitvector, FIND_OBJ_INV)
	    && (*tar_obj = get_obj_in_list_vis(ch, name, ch->carrying)))
	return (FIND_OBJ_INV);

    if (IS_SET(bitvector, FIND_OBJ_ROOM)
	    && (*tar_obj = get_obj_in_list_vis(ch, name, world[ch->in_room].contents))) 
	return (FIND_OBJ_ROOM);

    if (IS_SET(bitvector, FIND_OBJ_WORLD) && (*tar_obj = get_obj_vis(ch, name)))
	    return (FIND_OBJ_WORLD);

    return (0);
}

/* NOTE: Function version of OLD CAN_SEE().
	It was too complex macros to use time- and space-efficently */
int CAN_SEE(struct char_data *ch, struct char_data *vic ) 
{
#define OMNI(sub) (!IS_NPC(sub) && (GET_LEVEL(sub) >= IMO))
#define X99(sub, obj)   ( ((!IS_AFFECTED((obj),AFF_INVISIBLE) || \
				IS_AFFECTED((sub),AFF_DETECT_INVISIBLE)) && \
				!IS_AFFECTED((sub),AFF_BLIND) ) && \
		(IS_NPC(obj) || !IS_SET((obj)->specials.act,PLR_WIZINVIS)) && \
    (OMNI(sub) || IS_LIGHT(sub->in_room) || IS_AFFECTED(sub,AFF_INFRAVISION)))

#define CAN_SEE_MACRO(sub,obj) (X99(sub,obj)||(OMNI(sub) && (IS_NPC(obj) || \
   (GET_LEVEL(sub) >= GET_LEVEL(obj)))))

    return(CAN_SEE_MACRO(ch , vic));
}

#define CAN_CARRY_OBJ(ch,obj)  \
   (((GET_CARRYING_W(ch) + GET_OBJ_WEIGHT(obj)) <= CAN_CARRY_W(ch)) &&   \
    ((GET_CARRYING_N(ch) + 1) <= CAN_CARRY_N(ch)))

#define CAN_GET_OBJ_MACRO(ch, obj)   \
   (CAN_WEAR((obj), ITEM_TAKE) && CAN_CARRY_OBJ((ch),(obj)) &&  \
    CAN_SEE_OBJ((ch),(obj)))

int CAN_GET_OBJ(struct char_data *ch, struct obj_data *obj) 
{ 
	return(CAN_GET_OBJ_MACRO(ch, obj));
}

int ac_applicable(struct obj_data *obj_object)
{
    return (CAN_WEAR(obj_object, ITEM_WEAR_KNEE) ||
	    CAN_WEAR(obj_object, ITEM_WEAR_ABOUTLEGS) ||
	    CAN_WEAR(obj_object, ITEM_WEAR_SHIELD) ||
	    CAN_WEAR(obj_object, ITEM_WEAR_WRIST) ||
	    CAN_WEAR(obj_object, ITEM_WEAR_WAISTE) ||
	    CAN_WEAR(obj_object, ITEM_WEAR_ARMS) ||
	    CAN_WEAR(obj_object, ITEM_WEAR_HANDS) ||
	    CAN_WEAR(obj_object, ITEM_WEAR_FEET) ||
	    CAN_WEAR(obj_object, ITEM_WEAR_LEGS) ||
	    CAN_WEAR(obj_object, ITEM_WEAR_ABOUT) ||
	    CAN_WEAR(obj_object, ITEM_WEAR_HEAD) ||
	    CAN_WEAR(obj_object, ITEM_WEAR_BODY));
}

/* NOTE: New isname() matches multiple word string       */
/*      i.e. "xxx yyy" will match to "abc xxx yyy zzz "  */
/* NOTE: number, '-', '_' is valid char for name     */
/* NOTE: confusing code cleanup of isname()     */
int isname(char *str, char *namelist)
{
#define ISNAMECHAR(c) 	( isalnum(c) || (c) == '-' || (c) == '_' )
    register char *curstr, *look, *curname;

    if (!namelist || !str)
	return (0);
    while (*str && (*str == ' ' || !ISNAMECHAR(*str)))
	str++;
    if (!*str)
	return 0;
    if (strcmp(str, "ALL") == 0)
	return (1);

    curname = namelist;
    for (;;) {
	curstr = str;
	look = curname;
	for (;;) {
	    if (!*curstr)
		return (1);	/* SUCCESS */
	    if (!*look)
		return (0);

	    if (LOWER(*curstr) != LOWER(*look))
		break;
	    curstr++;
	    look++;
	}
	/* find start of next word in namelist */
	while (ISNAMECHAR(*curname))
	    curname++;
	while (*curname && !ISNAMECHAR(*curname))
	    curname++;
	if (!*curname)
	    return (0);
    }
}

#ifdef NO_DEF
/* NOTE: Old isname() function  */
int old_isname(char *str, char *namelist)
{
    register char *curname, *curstr;

    while (*str == ' ')
	str++;

    curname = namelist;
    if (strcmp(str, "ALL") == 0)
	return (1);
    if (!curname)
	return (0);
    if (!str || !*str)
	return 0;
    for (;;) {
	for (curstr = str;; curstr++, curname++) {
	    /* 
	       if (!*curstr && !isalpha(*curname)) */
	    if (!*curstr)	/* SUCCESS */
		return (1);
	    if (!*curname)
		return (0);
	    if (!*curstr || *curname == ' ')
		break;
	    if (LOWER(*curstr) != LOWER(*curname))
		break;
	}
	/* skip to next name */
	for (; isalpha(*curname); curname++) ;
	if (!*curname)
	    return (0);
	curname++;	/* first char of new name */
    }
}

/* is exact name */
int isexactname(char *str, char *namelist)
{
    register char *curname, *curstr;

    curname = namelist;
    if (strcmp(str, "ALL") == 0)
	return (1);
    if (!curname)
	return (0);
    if (!str || !*str)
	return 0;
    for (;;) {
	for (curstr = str;; curstr++, curname++) {
	    if (!*curstr && !isalpha(*curname))
		return (1);
	    if (!*curname)
		return (0);
	    if (!*curstr || *curname == ' ')
		break;
	    if (LOWER(*curstr) != LOWER(*curname))
		break;
	}
	/* skip to next name */
	for (; isalpha(*curname); curname++) ;
	if (!*curname)
	    return (0);
	curname++;	/* first char of new name */
    }
} 

#endif				/* NO_DEF */

/**************************************************************************
*  file: utility.c, Utility module.                       Part of DIKUMUD *
*  Usage: Utility procedures                                              *
*  Copyright (C) 1990, 1991 - see 'license.doc' for complete information. *
************************************************************************* */

/*
extern struct time_data time_info;
extern struct index_data *mob_index;
*/

/* defined in spell_parser.c */
/*
extern char *spells[];
*/

#undef MIN
#undef MAX

int MIN(int a, int b)
{
    return (a < b ? a : b);
}


int MAX(int a, int b)
{
    return (a > b ? a : b);
}

/* creates a random number in interval [from;to] */
int number(int from, int to)
{
    register int d, tmp;

    if (from == to)
	return from;
    if (from > to) {
	tmp = from;
	from = to;
	to = tmp;
    }

    d = to - from + 1;

    if (d < 2)
	d = 2;
    return ((random() % d) + from);
}

/* simulates dice roll */
int dice(int number, int size)
{
    int r;
    int sum = 0;

    if (size < 1)
	return 1;

    for (r = 1 ; r <= number ; r++)
	sum += ((random() % size) + 1);

    return sum;
}

/* returns: 0 if equal, 1 if arg1 > arg2, -1 if arg1 < arg2  */
/* scan 'till found different or end of both                 */
int str_cmp(char *arg1, char *arg2)
{
    int chk ;

    if(!arg1 && !arg2) return(0);
    if(!arg1) return(-1);
    if(!arg2) return(1);

    for ( ; *arg1 || *arg2 ; arg1++, arg2++)
	if (( chk =  LOWER(*arg1) - LOWER(*arg2)))
	    return((chk > 0) ? 1 : -1);

    return(0);
}

/* returns: 0 if equal, 1 if arg1 > arg2, -1 if arg1 < arg2  */
/* scan 'till found different, end of both, or n reached     */
int strn_cmp(char *arg1, char *arg2, int n)
{
    int chk ;

    if(!arg1 && !arg2) return(0);
    if(!arg1) return(-1);
    if(!arg2) return(1);

    for (; (*arg1 || *arg2) && (n > 0); arg1++, arg2++, n--)
	if (( chk = LOWER(*arg1) - LOWER(*arg2 )))
	    return((chk > 0) ? 1 : -1 );

    return (0);
} 

char *skip_spaces(char *string)
{
    while( *string && isspace(*string)) string++ ;
    return (string);
}

/* writes a string to the log */
void log(char *str)
{
    time_t ct;
    char *tmstr;
    static int count = 0;

    ct = time(0);
    tmstr = asctime(localtime(&ct));
    /* NOTE: do less frequent full time log */
    if (count++ % 100 == 0) {
	*(tmstr + strlen(tmstr) - 1) = '\0';
	fprintf(stderr, "Current time : %s\n", tmstr);
	*(tmstr + strlen(tmstr) - 5) = '\0';
	fprintf(stderr, "%s : %s\n", &tmstr[11], str);
    }
    else {
	*(tmstr + strlen(tmstr) - 6) = '\0';
	fprintf(stderr, "%s : %s\n", &tmstr[11], str);
    }
    fflush(stderr);
} 

void sprintbit(long vektor, char *names[], char *result)
{
    long nr;

    *result = '\0';

    for (nr = 0; vektor; vektor >>= 1) {
	if (IS_SET(1, vektor)) {
	    if (*names[nr] != '\n') {
		strcat(result, names[nr]);
		strcat(result, " ");
	    }
	    else {
		strcat(result, "UNDEFINED");
		strcat(result, " ");
	    }
	}
	if (*names[nr] != '\n')
	    nr++;
    }

    if (!*result)
	strcat(result, "NOBITS");
} 


void sprinttype(int type, char *names[], char *result)
{
    int nr;

    for (nr = 0; (*names[nr] != '\n'); nr++) ;
    if (type < nr)
	strcpy(result, names[type]);
    else
	strcpy(result, "UNDEFINED");
}


/* Calculate the REAL time passed over the last t2-t1 centuries (secs) */
struct time_info_data real_time_passed(time_t t2, time_t t1)
{
    long secs;
    struct time_info_data now;

    secs = (long) (t2 - t1);

    now.hours = (secs / SECS_PER_REAL_HOUR) % 24;	/* 0..23 hours */
    secs -= SECS_PER_REAL_HOUR * now.hours;

    now.day = (secs / SECS_PER_REAL_DAY);	/* 0..34 days  */
    secs -= SECS_PER_REAL_DAY * now.day;

    now.month = -1;
    now.year = -1;

    return now;
} 


/* Calculate the MUD time passed over the last t2-t1 centuries (secs) */
struct time_info_data mud_time_passed(time_t t2, time_t t1)
{
    long secs;
    struct time_info_data now;

    secs = (long) (t2 - t1);

    now.hours = (secs / SECS_PER_MUD_HOUR) % 24;	/* 0..23 hours */
    secs -= SECS_PER_MUD_HOUR * now.hours;

    now.day = (secs / SECS_PER_MUD_DAY) % 35;	/* 0..34 days  */
    secs -= SECS_PER_MUD_DAY * now.day;

    now.month = (secs / SECS_PER_MUD_MONTH) % 17;	/* 0..16 months */
    secs -= SECS_PER_MUD_MONTH * now.month;

    now.year = (secs / SECS_PER_MUD_YEAR);	/* 0..XX? years */

    return now;
}

struct time_info_data age(struct char_data *ch)
{
    struct time_info_data player_age;

    player_age = mud_time_passed(time(0), ch->player.time.birth);
    player_age.year += 17;	/* All players start at 17 */
    return player_age;
}


/* NOTE: Show number in financial format like "-98,765,432".    */
/* NOTE: Returns pointer to recycled static internal char buffer.  */
char *monetary(LONGLONG n)
{
    static char outbuf[5][20];
    static char ring = 0;
    char digits[20], *po, *pd;
    int pos;

    pd = digits;
    po = outbuf[ring % 5];

    sprintf(digits, "%lld", n < 0 ? -n : n);
    pos = strlen(digits);
    if (n < 0)
	*po++ = '-';
    while (pos) {
	*po++ = *pd++;
	if (--pos % 3 == 0 && pos > 0)
	    *po++ = ',';
    }
    *po = '\0';
    return (outbuf[(ring++) % 5]);
}

/* NOTE: Similar to monetary(). but show only most significant 4 digit
   without rounding. like "-98,760,000".         */
char *monetary4(LONGLONG n)
{
    int nsig = 0;
    char *out, *po;

    out = po = monetary(n);
    while (*po) {
	if (isdigit(*po) && ++nsig > 4)
	    *po = '0';
	po++;
    }
    return (out);
}

/* NOTE: Test driver for monetary(), monetary4() */
/*
   main()
   {
   int n = 136273879 ;
   printf(" %d, [%s] [%s] [%s]\n [%s] [%s] [%s]  \n",
   n, monetary(n), monetary(10*n), monetary(-100*n), 
   monetary(-4* n) ,monetary(10*n) , monetary(n*8) );

   }
 */

/* NOTE: NEW! Remove last CR/LF or LF/CR or LF or CR seq  in string */
/* NOTE: Used to append other string as a single line.   */
char *remove_eol(char *str)
{
    char *scan, *look;

    if (!str || !*str)
	return (str);
    look = scan = str + strlen(str);
    while (--scan >= str) {
	if ((*look == '\n' && *scan == '\r') 
	    || (*look == '\r' && *scan == '\n')) {
		look = scan;
		break;
	    }
	if ((*look == '\n') || (*look == '\r') 
		|| (*scan != '\n' && *scan != '\r'))
	    break;
	look = scan;
    }
    *look = '\0';  /* remove eol! */
    return (look);
}

/************************************************************************
*  procs of a (more or less) general utility nature			*
********************************************************************** */

char *fname(char *namelist)
{
    static char holder[30];
    register char *point;

    for (point = holder; isalpha(*namelist); namelist++, point++)
	*point = *namelist;

    *point = '\0';

    return (holder);
}

int search_block(char *arg, char **list, int exact)
{
    register int i, l;

    /* Make into lower case, and get length of string */
    for (l = 0; *(arg + l); l++)
	/* cyb   *(arg+l)=LOWER(*(arg+l)); */
	/* cyb just iterate */ ;

    /* NOTE: New! keyword-data pair list search for alias expansion. */  
    /* Find exact match for keyword part and return its index. */  
    if (exact == 2) {
	for (i = 0; **(list + i) != '\n'; i += 2 )
	    if (!strcmp(arg, *(list + i)))
		return (i);
    }
    else if (exact) {
	for (i = 0; **(list + i) != '\n'; i++)
	    if (!strcmp(arg, *(list + i)))
		return (i);
    }
    else {
	if (!l)
	    l = 1;	/* Avoid "" to match the first available string */
	for (i = 0; **(list + i) != '\n'; i++)
	    if (!strncmp(arg, *(list + i), l))
		return (i);
    }

    return (-1);
}

#ifdef	NO_DEF
/* NOTE: old_search_block() no longer used. Commented out  */
int old_search_block(char *argument, int begin, unsigned length, char **list, int mode)
{
    int guess, found, search;

    /* If the word contain 0 letters, then a match is already found */
    found = (length < 1);
    guess = 0;
    /* Search for a match */
    if (mode)
	while (NOT found AND * (list[guess]) != '\n') {
	    found = (length == strlen(list[guess]));
	    for (search = 0; (search < length AND found); search++)
		found = (*(argument + begin + search) 
		    == *(list[guess] + search));
	    guess++;
	}
    else {
	while (NOT found AND * (list[guess]) != '\n') {
	    found = 1;
	    for (search = 0; (search < length AND found); search++)
		found = (*(argument + begin + search) 
		    == *(list[guess] + search));
	    guess++;
	}
    }
    return (found ? guess : -1);
}
#endif 		/*  NO_DEF  */

/* #define ISLETTER(c)	( c  > ' ' || c < 0 ) */
/* NOTE: Above may not work for Hangul char. on unsigned char  */
/* #define ISLETTER(c)	(((c) & 0177) > ' ' ) */
/* NOTE: Check c is ASCII non-blank char, excluding hangul  */ 
#define ISLETTER(c)	(((c)  > ' ' ) && ((c) < 0177 ))

/* NOTE: New pointer version. Similar function but more simple code     */
/* Old argument_interpreter() will hang on input like "kill at" without arg. */
void argument_interpreter(char *argument, char *arg1, char *arg2)
{
    char *next;

    arg1[0] = arg2[0] = '\0';
    next = one_argument(argument, arg1);
    one_argument(next, arg2);
    /* CAVEATS: If second word is fill word, it will be discarded */
    /* Ex. "kill at mob1 with knife -> arg1 = "mob", arg2 = "knife"     */
    /* If no word after fill word,("kill at") arg1 = "at" arg2 = ""     */
}

int fill_word(char *argument)
{
    static char *fill[] = {
	 "in", "from", "with", "the", "on", "at", "to", "\n",
    };
    return (search_block(argument, fill, TRUE) >= 0);
}

#ifdef NO_DEF
void argument_interpreter(char *argument, char *first_arg, char *second_arg)
{
    int look_at, found, begin;

    found = begin = 0;

    do {
	/* Find first non blank */
	for (; *(argument + begin) == ' '; begin++) ;

	/* Find length of first word */
	for (look_at = 0; ISLETTER(*(argument + begin + look_at)); look_at++)
	    /* Make all letters lower case, AND copy them to first_arg */
	    *(first_arg + look_at) =
	    /* cyb   LOWER(*(argument + begin + look_at));  */
		*(argument + begin + look_at);

	*(first_arg + look_at) = '\0';
	begin += look_at; 
    }
    while (fill_word(first_arg));

    do {
	/* Find first non blank */
	for (; *(argument + begin) == ' '; begin++) ;

	/* Find length of first word */
	for (look_at = 0; ISLETTER(*(argument + begin + look_at)); look_at++)
	    /* Make all letters lower case, AND copy them to second_arg */
	    *(second_arg + look_at) =
	    /* cyb   LOWER(*(argument + begin + look_at)); */
		*(argument + begin + look_at);

	*(second_arg + look_at) = '\0';
	begin += look_at;

    }
    while (fill_word(second_arg));
}

#endif				/* NO_DEF  */

/* NOTE: New one_argument() will take quoted args as single arg.        */
/* NOTE: Surround multiple words with quotation marks. ( 'like this' )
   Now, you can resolve confusing name like president as
   "MSTAT 'president robot'" (single quotation included )               */

/* NOTE: rewrote indexed version to pointer version for clarity and efficency.*/
/* NOTE: Find the first word of string excluding fill word ( "at", "in" ),
   Copy it to first_arg. If non-fill word don't exist. fill word will.
   Returns pointer to next unlooked char ( blank + following words )  */
char *one_argument(char *argument, char *first_arg)
{
    register char *look_at, *to;
    int quot = 0, fillok = 0;

    look_at = argument;
    do {
	first_arg[0] = '\0';
	to = first_arg;
	/* Find first non blank */
	while (*look_at && isspace(*look_at))
	    look_at++;

	if (*look_at == '\'') {
	    quot++;
	    look_at++;
	}
	/* Find length of first word */
	while ((*to = *look_at)) {
	    if (quot && *to == '\'')
		quot = 0;
	    else if ( ISLETTER((*to) & 0177) || (quot && (*to == ' ')))
		to++;
	    else
		break;
	    look_at++;
	}
	*to = '\0';
	/* NOTE: Fill word will be treated only once */
	/* NOTE: If no more char after fill word, first_arg is fill word. */
    } while (!fillok && (fillok = fill_word(first_arg)) && *look_at);

    return (look_at);
}

/* One_Word is like one_argument, execpt that words in quotes "" are */
/* regarded as ONE word                                              */

char *one_word(char *argument, char *first_arg)
{
    int /*found,*/ begin, look_at;

    /* found = */ begin = 0;

    do {
	for (; isspace(*(argument + begin)); begin++) ;

	if (*(argument + begin) == '\"') {	/* is it a quote */

	    begin++;

	    for (look_at = 0; (*(argument + begin + look_at) >= ' ') &&
		 (*(argument + begin + look_at) != '\"'); look_at++)
		*(first_arg + look_at) = LOWER(*(argument + begin + look_at));

	    if (*(argument + begin + look_at) == '\"')
		begin++;

	}
	else {

	    for (look_at = 0; *(argument + begin + look_at) > ' '; look_at++)
		*(first_arg + look_at) = LOWER(*(argument + begin + look_at));

	}

	*(first_arg + look_at) = '\0';
	begin += look_at;
    }
    while (fill_word(first_arg));

    return (argument + begin);
}

/* return first 'word' plus trailing substring of input string */
/* NOTE: Move this defintion to global.h */
/* #define isnhspace(ch) (!(((ch) & 0xE0) > 0x90) && isspace(ch)) */

void half_chop(char *string, char *arg1, char *arg2)
{
    for (; isnhspace(*string); string++) ;

    for (; !isnhspace(*arg1 = *string) && *string; string++, arg1++) ;

    *arg1 = '\0';

    for (; isnhspace(*string); string++) ;

    for (;( *arg2 = *string); string++, arg2++) ;
}

/* determine if a given string arg1 is an abbreviation of arg2 */
int is_abbrev(char *arg1, char *arg2)
{
    /* NOTE: check argument more thoroughly */
    if (!arg1 || !arg2 || !*arg1 || !*arg2)
	return (0);

    for (; *arg1; arg1++, arg2++)
	if (LOWER(*arg1) != LOWER(*arg2))
	    return (0);

    return (1);
}

int is_number(char *str)
{
    int look_at;

    if (*str == '\0')
	return (0);

    for (look_at = 0; *(str + look_at) != '\0'; look_at++)
	if ((*(str + look_at) < '0') || (*(str + look_at) > '9'))
	    return (0);
    return (1);
} 

