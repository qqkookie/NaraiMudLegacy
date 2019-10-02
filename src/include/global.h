/********************************************************************
 *  File: global.h: Global definition and external vars and procs   *
 *								    *	
 ********************************************************************/

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#include "typedef.h" 

#define  sigsetmask(m) __mysigsetmask(m)

extern int sigsetmask(unsigned mask);

#define MAX_STR_LEN		2000
#define MAX_OUT_LEN		512
#define MAX_LINE_LEN		200
#define MAX_NAME_LEN		64

#define MAX_STRING_LENGTH	MAX_STR_LEN
#define MAX_BUFSIZ		MAX_LINE_LEN
#define MAX_INPUT_LENGTH	MAX_LINE_LEN

#ifndef IMO
#define	IMO	41
#endif

#pragma GCC diagnostic ignored "-Wunused-result"
#pragma GCC diagnostic ignored "-Wunused-parameter"

/*  NOTE: also defined in "comm.c"	*/ 
#define DFLT_PORT 4001  /* default port */ 
#define DFLT_DIR "lib"  /* default directory */ 

/* -----------------  Misc. structures about char  ------------------- */

struct str_app_type {
    int tohit;		/* To Hit (THAC0) Bonus/Penalty        */
    int todam;		/* Damage Bonus/Penalty                */
    int carry_w;	/* Maximum weight that can be carrried */
    int wield_w;	/* Maximum weight that can be wielded  */
}; 

struct index_data {
    int virtual;	/* virtual number of this mob/obj           */
    long pos;		/* file position of this field              */
    int number;		/* number of existing units of this mob/obj     */
    int (*func) (struct char_data * ch, int cmd, char *arg);
    /* int (*func) (); */
    /* special procedure for this mob/obj       */
}; 

struct title_type {
    char *title_m;
    char *title_f;
    int exp;
};

/*------------------------    TIME AND WEATHER   ------- -------------*/

#define SUN_DARK		0
#define SUN_RISE		1
#define SUN_LIGHT		2
#define SUN_SET			3

/* And how is the sky ? */

#define SKY_CLOUDLESS		0
#define SKY_CLOUDY		1
#define SKY_RAINING		2
#define SKY_LIGHTNING		3

struct weather_data {
    int pressure;	/* How is the pressure ( Mb ) */
    int change;		/* How fast and what way does it change. */
    int sky;		/* How is the sky. */
    int sunlight;	/* And how much sun. */
};

#define SECS_PER_REAL_MIN  60
#define SECS_PER_REAL_HOUR (60*SECS_PER_REAL_MIN)
#define SECS_PER_REAL_DAY  (24*SECS_PER_REAL_HOUR)
#define SECS_PER_REAL_YEAR (365*SECS_PER_REAL_DAY)

#define SECS_PER_MUD_HOUR  60
#define SECS_PER_MUD_DAY   (24*SECS_PER_MUD_HOUR)
#define SECS_PER_MUD_MONTH (35*SECS_PER_MUD_DAY)
#define SECS_PER_MUD_YEAR  (17*SECS_PER_MUD_MONTH)

struct time_info_data {
    ubyte hours, day, month;
    sh_int year;
}; 

/* **********************************************************************
   *  OLD file: utils.h, Utility module.  Part of DIKUMUD 		*
   *  Usage: Utility macros						*
   ********************************************************************** */

#define TRUE  1 
#define FALSE 0

#define IS_SET(flag,bit)  ((flag) & (bit))

#define SWITCH(a,b) { (a) ^= (b); \
                      (b) ^= (a); \
                      (a) ^= (b); }

#define SET_BIT(var,bit)  ((var) = (var) | (bit)) 
#define REMOVE_BIT(var,bit)  ((var) = (var) & ~(bit) )

/* NOTE: string/character handling macros */
#define LOWER(c) (((c)>='A'  && (c) <= 'Z') ? ((c)+('a'-'A')) : (c)) 
#define UPPER(c) (((c)>='a'  && (c) <= 'z') ? ((c)+('A'-'a')) : (c) )

#define ISDIGIT(c) ((c) >='0' && (c) <= '9') 
#define ISNEWL(ch) ((ch) == '\n' || (ch) == '\r')

/* NOTE: NEW! Prevent NULL pointer to string : it replaces OLD IF_STR()  */
#define STR_NULL(st) ((st) ? (st) : "(NULL)")

#define CAP(st)  (*(st) = UPPER(*(st)))

/* #define ishan(ch) (((ch) & 0xE0) > 0x90) */
/* NOTE: 0xE0 = 11100000B , 0x90 = 10010000B, Is above right for KS-5601? */
#define ishan(ch)	(((ch) & 0377 ) > ( ' ' | 0200 ))
/* #define ishanasc(ch)	(isascii(ch) || ishan(ch))  */
/* #define ishanalp(ch)	(isalpha(ch) || ishan(ch))  */
/* #define isnhdigit(ch)	(!ishan(ch) && isdigit(ch))  */
/* #define isnhspace(ch)	(!ishan(ch) && isspace(ch))  */
#define isnhspace(ch)	(!ishan(ch) && isspace(ch))

/* NOTE: isspace() failed to recognize hangul as non-blank.(Solaris BSD env.) */
#undef isspace
#define isspace(ch)	((ch) == ' ' || (ch) == '\t' || (ch) == '\n' \
		|| (ch) == '\r' || (ch) == '\f' || (ch) == '\v' )

/*  #include <stdlib.h>  */

#define CREATE(result, type, number)  do {\
  if (!((result) = (type *) calloc ((number), sizeof(type))))\
    { perror("malloc failure"); abort(); } } while(0)

#define RECREATE(result,type,number) do {\
  if (!((result) = (type *) realloc ((result), sizeof(type) * (number))))\
    { perror("realloc failure"); abort(); } } while(0) 

/* NOTE: NEW MACRO: Similar to assert(). but don't abort(),
   just print log() where ASSERT() failed.      */
#ifdef NDEBUG
#define ASSERT(test)  (void )( (test) || fprintf(stderr, \
	"ASSERT FAIL in %s at %d \n", __FILE__, __LINE__ ) )
#else
#define ASSERT(test)	(void)(test)
#endif

/* -------------------------------------------------------------- */
/*  global def of frequently used extern proc's			  */

/* NOTE: MIN(), MAX() is predefined on SUN. So, it caused compile error. */
#ifndef MAX
/* Functions in utility.c                     */
extern int MIN(int a, int b);
extern int MAX(int a, int b);

#define MAX(a,b) (((a) > (b)) ? (a) : (b))
#define MIN(a,b) (((a) < (b)) ? (a) : (b))
#endif 

extern int number(int from, int to);
extern int dice(int number, int size);
extern int str_cmp(char *arg1, char *arg2);
extern int strn_cmp(char *arg1, char *arg2, int n);
extern char *skip_spaces(char *string);

#define log(s)	mudlog(s)
extern void log(char *str);


extern void sprintbit(long vektor, char *names[], char *result);
extern void sprinttype(int type, char *names[], char *result);
extern char *monetary(LONGLONG n);
extern char *monetary4(LONGLONG n);

extern int isname(char *str, char *namelist);
extern char *fname(char *namelist); 
extern int is_abbrev(char *arg, char *full);

extern char *one_argument(char *argument, char *first_arg);
extern void half_chop(char *string, char *arg1, char *arg2);
extern void argument_interpreter(char *argument, char *arg1, char *arg2);
extern int search_block(char *arg, char **list, int exact); 

/* One_Word is like one_argument, execpt that words in quotes "" are */
/* regarded as ONE word                                              */ 
extern char *one_word(char *argument, char *first_arg);
/* -------------------------------------------------------------- */
/* some  of public procedures in db.c */

#define REAL 0
#define VIRTUAL 1

extern struct char_data *read_mobile(int nr, int type); 
/* read an object from OBJ_FILE */
extern struct obj_data *read_object(int nr, int type);

extern char *lookup_db(char *keyword);

extern void save_char(struct char_data *ch);

/* returns the real number of the monster with given virtual number */
extern int real_mobile(int virtual); 

/* returns the real number of the room with given virtual number */
extern int real_room(int virtual);

/* returns the real number of the object with given virtual number */
extern int real_object(int virtual);

/* read contents of a text file, and place in buf */
extern char *file_to_string(char *name, char *sbuf);

/* read and allocate space for a '~'-terminated string from a given file */
extern char *fread_string(FILE * fl); 

/* ************************************************************************
   *  file: handler.h , Handler module.                      Part of DIKUMUD *
   *  Usage: Various routines for moving about objects/players               *
   ************************************************************************* */ 

/* handling the affected-structures */
void affect_to_char(struct char_data *ch, struct affected_type *af);
void affect_from_char(struct char_data *ch, byte skill);
void affect_join(struct char_data *ch, struct affected_type *af,
		 bool avg_dur, bool avg_mod); 
bool affected_by_spell(struct char_data *ch, byte skill);
void affect_remove(struct char_data *ch, struct affected_type *af);
/* ******* characters ********* */

void char_from_room(struct char_data *ch);
void char_to_room(struct char_data *ch, int room);
void extract_char(struct char_data *ch);
void equip_char(struct char_data *ch, struct obj_data *obj, int pos);
struct obj_data *unequip_char(struct char_data *ch, int pos);


void obj_to_obj(struct obj_data *obj, struct obj_data *obj_to);
void obj_from_obj(struct obj_data *obj);
void obj_to_char(struct obj_data *object, struct char_data *ch);
void obj_from_char(struct obj_data *object);
void obj_to_room(struct obj_data *object, int room);
void obj_from_room(struct obj_data *object);

void extract_obj(struct obj_data *obj); 

void update_pos(struct char_data *victim);

/* -------------------------------------------------------------- */

/* NOTE: find and get char or object */

extern struct char_data *get_char_vis(struct char_data *ch, char *name);
extern struct char_data *get_char_room_vis(struct char_data *ch, char *name);

extern struct obj_data *get_obj_in_list_vis(struct char_data *ch, char *name,
				     struct obj_data *list);
struct obj_data *get_obj_vis(struct char_data *ch, char *name);

/* NOTE: Function version of OLD macro CAN_SEE()  */
extern int CAN_SEE(struct char_data *ch, struct char_data *vic );

/* Generic Find */ 

#define FIND_CHAR_ROOM     1
#define FIND_CHAR_WORLD    2
#define FIND_OBJ_INV       4
#define FIND_OBJ_ROOM      8
#define FIND_OBJ_WORLD    16
#define FIND_OBJ_EQUIP    32

int generic_find(char *arg, int bitvector, struct char_data *ch,
		 struct char_data **tar_ch, struct obj_data **tar_obj);

/* -------------------------------------------------------------- */
/* NOTE: other frequently used procs */

extern int command_interpreter(struct char_data *ch, char *argument);

extern void clear_object(struct obj_data *obj); 
extern void clear_char(struct char_data *ch); 

extern int do_simple_move(struct char_data *ch, int cmd , int following);
extern void wear(struct char_data *ch, struct obj_data *obj, int where_flag);

extern void do_look(struct char_data *ch, char *arg, int cmd);

extern void gain_exp(struct char_data *ch, long gain);
extern void gain_gold(struct char_data *ch, long money);

extern void stop_follower(struct char_data *ch);

/* -------------------------------------------------------------- */
/* NOTE: global variables */

extern char *dirs[];
extern char *guild_names[];
extern char *spells[];
extern struct time_info_data time_info;
extern struct str_app_type str_app[];
extern struct title_type titles[4][IMO + 4] ;
extern struct weather_data weather_info; /* the infomation about the weather */ 

/* -------------------------------------------------------------- */
