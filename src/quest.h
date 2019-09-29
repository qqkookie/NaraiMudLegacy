/*
	This file is for Quest definition
*/

#define	QUEST_HIGH	1			/* quest is 2 integer */
#define	QUEST_LOW	0

/* quest number : this number is from 0 to 63 */
/* form 0 to 31 , saves into QUEST_LOW, and   */
/* from 32 to 63, saves into QUEST_HIGH       */
#define	QUEST_SCHOOL	0
#define	QUEST_OKSE	1
#define	QUEST_TRASH	2
#define	QUEST_LOSTCARD	3
#define	QUEST_POLYESTER	4
#define	QUEST_MAKEBOMB	5
#define	QUEST_ALOM		6
#define	QUEST_REMORTAL	7
#define	QUEST_8	8
#define	QUEST_9	9
#define	QUEST_10	10
#define	QUEST_11	11
#define	QUEST_12	12
#define	QUEST_13	13
#define	QUEST_14	14
#define	QUEST_15	15
#define	QUEST_16	16
#define	QUEST_17	17
#define	QUEST_18	18
#define	QUEST_19	19
#define	QUEST_20	20		/* etc.. */

/* functions in quest.c */
int is_solved_quest(struct char_data *ch, int quest) ;
int set_solved_quest(struct char_data *ch, int quest) ;
int clear_solved_quest(struct char_data *ch, int quest) ;
char *name_of_quest(int quest) ;
char *what_is_solved(struct char_data *ch) ;
void clear_all_quest(struct char_data *ch) ;
