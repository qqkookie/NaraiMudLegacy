/**************************************************************************
*  file: utility.c, Utility module.                       Part of DIKUMUD *
*  Usage: Utility procedures                                              *
*  Copyright (C) 1990, 1991 - see 'license.doc' for complete information. *
************************************************************************* */

#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <time.h>

#include "structs.h"
#include "utils.h"
#include "memory.h"
#include "db.h"

extern struct time_data time_info;


int MIN(int a, int b)
{
    return (a < b ? a:b);
}


int MAX(int a, int b)
{
    return (a > b ? a:b);
}

/* creates a random number in interval [from;to] */
int number(int from, int to) 
{
    register int d, tmp;

    if ( from == to ) return from ;
    if ( from > to ) {
        tmp = from ;
        from = to ;
        to = tmp ;
    }

    d=to-from+1;

    if(d < 2) d=2;
    return((random() % d) + from);
}



/* simulates dice roll */
int dice(int number, int size) 
{
    int r;
    int sum = 0;
/*
  assert(size >= 1);
*/if(size < 1) return(1);
    for (r = 0; r < number; r++) sum += ((random() % size)+1);
    return(sum);
}

int num_pc_in_room(struct room_data *room)
{
  	int i = 0;
    struct char_data *ch;

	for (ch = room->people; ch != NULL; ch = ch->next_in_room)
		if (!IS_NPC(ch))
			i++;

	return (i);
}

/* Create a duplicate of a string */
char *string_dup(char *source)
{
    char *new;

	/*
      create(new, char, strlen(source)+1);
	*/
	new = malloc_string_type(strlen(source)+1) ;
    return(strcpy(new, source));
}

/* returns: 0 if equal, 1 if arg1 > arg2, -1 if arg1 < arg2  */
/* scan 'till found different or end of both                 */
int str_cmp(char *arg1, char *arg2)
{
    int chk, i;

    if(!arg1)
        return(!arg2 ? 0 : -1);
    else if(! arg2)
        return(1);
    for (i = 0; *(arg1 + i) || *(arg2 + i); i++)
        if (chk = LOWER(*(arg1 + i)) - LOWER(*(arg2 + i)))
            if (chk < 0)
                return (-1);
            else 
                return (1);
    return(0);
}

char LOWER(char ch)
{
	if ( ch >= 'A' && ch <= 'Z' &&  !is_hangul(ch) )
		return ( ch - 'A' + 'a' ) ;
	else
		return ( ch ) ;
}

int is_hangul(char ch)
{
	return ( ch & 0x80 ) ;
}
/* returns: 0 if equal, 1 if arg1 > arg2, -1 if arg1 < arg2  */
/* scan 'till found different, end of both, or n reached     */
int strn_cmp(char *arg1, char *arg2, int n)
{
    int chk, i;

    for (i = 0; (*(arg1 + i) != 0 || *(arg2 + i) != 0 ) && (n>0); i++, n--)
        if ((chk = LOWER(*(arg1 + i)) - LOWER(*(arg2 + i))))
            if (chk < 0)
                return (-1);
            else 
                return (1);

    return(0);
}



/* writes a string to the log */
void log(char *str)
{
	long	ct;
	char	*tmstr ;
	char	buf[BUFSIZ] ;
	static int	count = 0;

	ct = time(0);
	tmstr = asctime(localtime(&ct));

	/* It must be used by copy and need not free tmstr */
	strcpy(buf, tmstr) ;

	if ( count++ % 10 == 0 ) {
		buf[strlen(buf) - 1] = '\0';
		fprintf(stderr, "Current time : %s\n", buf);
		buf[strlen(buf) - 5] = '\0';
		fprintf(stderr, "%s :: %s\n", &buf[11], str);
    }
	else {
		buf[strlen(buf) - 6] = '\0';
		fprintf(stderr, "%s :: %s\n", &buf[11], str);
    }
}



void sprintbit(long vektor, char *names[], char *result)
{
    int	size, count ;
    long nr;

    if ( result == NULL )
		return ;
	*result = 0 ;

	if ( names == NULL || *names == NULL )
		return ;

	size = 8 * sizeof(long) ;
	for(nr=0, count = 0 ; count < size ; count ++, vektor>>=1) {
		if (IS_SET(1, vektor))
			if (*names[nr] != '\n') {
				strcat(result,names[nr]);
				strcat(result," ");
            }
			else {
				strcat(result,"UNDEFINED");
				strcat(result," ");
			}
		if (*names[nr] != '\n')
			nr++;
		else
			break ;
    }

    if (!*result)
        strcat(result, "NOBITS");
}



void sprinttype(int type, char *names[], char *result)
{
    int nr;

    for(nr=0;(*names[nr]!='\n');nr++);
    if(type < nr)
        strcpy(result,names[type]);
    else
        strcpy(result,"UNDEFINED");
}


/* Calculate the REAL time passed over the last t2-t1 centuries (secs) */
struct time_info_data real_time_passed(time_t t2, time_t t1)
{
    long secs;
    struct time_info_data now;

    secs = (long) (t2 - t1);

    now.hours = (secs/SECS_PER_REAL_HOUR) % 24;  /* 0..23 hours */
    secs -= SECS_PER_REAL_HOUR*now.hours;

    now.day = (secs/SECS_PER_REAL_DAY);          /* 0..34 days  */
    secs -= SECS_PER_REAL_DAY*now.day;

    now.month = -1;
    now.year  = -1;

    return now;
}



/* Calculate the MUD time passed over the last t2-t1 centuries (secs) */
struct time_info_data mud_time_passed(time_t t2, time_t t1)
{
    long secs;
    struct time_info_data now;

    secs = (long) (t2 - t1);

    now.hours = (secs/SECS_PER_MUD_HOUR) % 24;  /* 0..23 hours */
    secs -= SECS_PER_MUD_HOUR*now.hours;

    now.day = (secs/SECS_PER_MUD_DAY) % 35;     /* 0..34 days  */
    secs -= SECS_PER_MUD_DAY*now.day;

    now.month = (secs/SECS_PER_MUD_MONTH) % 17; /* 0..16 months */
    secs -= SECS_PER_MUD_MONTH*now.month;

    now.year = (secs/SECS_PER_MUD_YEAR);        /* 0..XX? years */

    return now;
}

struct time_info_data age(struct char_data *ch)
{
    long secs;
    struct time_info_data player_age;

    player_age = mud_time_passed(time(0),ch->player.time.birth);
    player_age.year += 17;   /* All players start at 17 */
    return player_age;
}

/***************************************************
	From utils.h :  PERS and CAN_SEE
 ***************************************************/

/*  Return object's name : it can be 'someone'  */
/*  'person' want to see 'objcet' character  */
char *PERS(struct char_data *object, struct char_data *person)
{
	int CAN_SEE(struct char_data *subject, struct char_data *object) ;

	if ( object == NULL || person == NULL )
		return ( "FATALBUG" ) ;
	else if ( CAN_SEE(person, object) ) {
		if ( IS_NPC(object) )	/* mob */
			return (object->player.short_descr) ;
		else	/* player */
			return (GET_NAME(object)) ;
    }
	else
		return ( "someone" ) ;
}

/*  Can 'subject' see character 'object' ?  */
int CAN_SEE(struct char_data *subject, struct char_data *object)
{
	int CAN_SEE_surely(struct char_data *subject, struct char_data *object) ;

	/* There is no one to see at all */
	if ( subject == NULL || object == NULL )
		return FALSE ;

	/* check invisible, blind, wiz-invis, dark room etc  */
	if ( CAN_SEE_surely(subject, object) )
		return TRUE ;

	/*  If someone is wiz-invis, only level is important  */
	else if ( OMNI(subject) &&
              (IS_NPC(object) || (GET_LEVEL(subject) >= GET_LEVEL(object)) ))
		return TRUE ;
	else
		return FALSE ;
}

int CAN_SEE_surely(struct char_data *subject, struct char_data *object)
{
	extern struct room_data	*world ;	/* used in IS_LIGHT */
	int	cond1, cond2, cond3, cond4 ;

	/* condition for invisible */
	cond1 = !IS_AFFECTED((object),AFF_INVISIBLE) ||
		IS_AFFECTED((subject),AFF_DETECT_INVISIBLE) ;

	/* condition for blind */
	cond2 = !IS_AFFECTED((subject),AFF_BLIND) ;

	/* condition for wizard invisible : anyone can see mobs. */
	cond3 = IS_NPC(object) || !IS_SET((object)->specials.act, PLR_WIZINVIS) ;

	/* condition for dark room : wizard or light room or infra vistion */
	cond4 = OMNI(subject) || IS_LIGHT(subject->in_room) ||
		IS_AFFECTED(subject,AFF_INFRAVISION) ;

	return ( cond1 && cond2 && cond3 && cond4 ) ;
}

/******************
  from fight.c
******************/

struct obj_data *make_particle(struct obj_data *obj)
{
	extern struct index_data *obj_index;
	extern struct obj_data  *object_list;
	struct obj_data *particle ;
	char buf[MAX_STRING_LENGTH] ;
	char *string_dup(char *source);

	if (obj == NULL)
		return NULL ;

	particle = (struct obj_data *) malloc_general_type(MEMORY_OBJ_DATA) ;
	clear_object(particle);

	particle->item_number = NOWHERE;
	particle->in_room = NOWHERE;

	sprintf(buf,"particle %s", obj->name) ;
	particle->name = string_dup(buf);

	sprintf(buf, "A particle of %s is abandoned here.", fname(obj->name)) ;
	particle->description = string_dup(buf);

	sprintf(buf, "a particle of %s", obj->short_description ) ;
	particle->short_description = string_dup(buf);

	/*  store item number in value : [0] real, [1] virtual */
	particle->obj_flags.value[0] = obj->item_number ;	/* Real number */
	particle->obj_flags.value[1] = obj_index[obj->item_number].virtual ;

	/* set flags */
	particle->obj_flags.type_flag = ITEM_BROKEN;
	particle->obj_flags.wear_flags = ITEM_TAKE;
	particle->obj_flags.weight = GET_OBJ_WEIGHT(obj)/2 + 1 ;
	SET_BIT(GET_OBJ_STAT(particle), ITEM_NO_RENT) ;

	/* insert to object list */
	particle->next = object_list;
	object_list = particle ;

	return particle ;
}

