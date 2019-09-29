/*
	This file is for Quest
*/
#include <stdio.h>
#include "structs.h"
#include "quest.h"

char *quest_name[64] = {
	"SCHOOL", "OKSE", "TRASH", "LOSTCARD", "POLYESTER", 	/* 0 - 4 */
	"MAKEBOMB", "ALOM", "REMORTAL", "NOT_YET", "NOT_YET",	/* 5 - 9 */
	"NOT_YET", "NOT_YET", "NOT_YET", "NOT_YET", "NOT_YET",
	"NOT_YET", "NOT_YET", "NOT_YET", "NOT_YET", "NOT_YET",	/*  - 19 */
	"NOT_YET", "NOT_YET", "NOT_YET", "NOT_YET", "NOT_YET",
	"NOT_YET", "NOT_YET", "NOT_YET", "NOT_YET", "NOT_YET",	/*  - 29 */
	"NOT_YET", "NOT_YET", "NOT_YET", "NOT_YET", "NOT_YET",
	"NOT_YET", "NOT_YET", "NOT_YET", "NOT_YET", "NOT_YET",	/*  - 39 */
	"NOT_YET", "NOT_YET", "NOT_YET", "NOT_YET", "NOT_YET",
	"NOT_YET", "NOT_YET", "NOT_YET", "NOT_YET", "NOT_YET",	/*  - 49 */
	"NOT_YET", "NOT_YET", "NOT_YET", "NOT_YET", "NOT_YET",
	"NOT_YET", "NOT_YET", "NOT_YET", "NOT_YET", "NOT_YET",	/*  - 59 */
	"NOT_YET", "NOT_YET", "NOT_YET", "NOT_YET"	/*  - 63 */
} ;

int is_solved_quest(struct char_data *ch, int quest)
{
	int mask ;
	/*  it assume, integer size is 4 byte(32 bit)  */
    if ( quest >= 64 || quest < 0 )
		return 0 ;

	if (quest >= 32) {
		mask = 0x1 << (quest - 32) ;
		return ( ch->quest[QUEST_HIGH] & mask ) ;
    }
	else {
		mask = 0x1 << quest ;
		return ( ch->quest[QUEST_LOW] & mask ) ;
    }
}

int set_solved_quest(struct char_data *ch, int quest)
{
	int mask ;
	/*  it assume, integer size is 4 byte(32 bit)  */
    if ( quest >= 64 || quest < 0 )
		return 0 ;

	if (quest >= 32) {
		mask = 0x1 << (quest - 32) ;
		ch->quest[QUEST_HIGH] = ch->quest[QUEST_HIGH] | mask  ;
    }
	else {
		mask = 0x1 << quest ;
		ch->quest[QUEST_LOW] = ch->quest[QUEST_LOW] | mask  ;
    }
	return 1 ;
}

int clear_solved_quest(struct char_data *ch, int quest)
{
	int mask ;
	/*  it assume, integer size is 4 byte(32 bit)  */
    if ( quest >= 64 || quest < 0 )
		return 0 ;

	if (quest >= 32) {
		mask = 0x1 << (quest - 32) ;
		ch->quest[QUEST_HIGH] = ch->quest[QUEST_HIGH] & ~mask  ;
    }
	else {
		mask = 0x1 << quest ;
		ch->quest[QUEST_LOW] = ch->quest[QUEST_LOW] & ~mask  ;
    }
	return 1 ;
}

char *name_of_quest(int quest)
{
	if ( quest >= 64 || quest < 0 )
		return ("INVALID") ;
	else
		return ( quest_name[quest] ) ;
}

char *what_is_solved(struct char_data *ch)
{
	static char buf[20*64] ;
	int is_solved_quest(struct char_data *ch, int quest) ;
	char *name_of_quest(int quest) ;
	int i, count ;

	buf[0] = 0 ;
	for ( i = 0, count = 0 ; i < 64 ;i ++ ) {
		if ( is_solved_quest(ch, i)) {
			strcat(buf, name_of_quest(i)) ;
			if ( count % 5 == 4 )
				strcat(buf, "\n\r") ;
			else
				strcat(buf, "  ") ;
			count ++ ;
        }
    }
	strcat(buf, "\n\r") ;

	return(buf) ;
}

void clear_all_quest(struct char_data *ch)
{
	int i ;

	for ( i = 0 ; i < 64 ; i ++ ) {
		clear_solved_quest(ch, i) ;
    }
}

int init_quest()
{
	int	init_quest_okse() ;

	/* this subroutine is for initializing quests */
	init_quest_okse() ;

	return 0 ;
}
