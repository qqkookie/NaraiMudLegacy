/* ************************************************************************
   *   File: house.c                                       Part of CircleMUD *
   *  Usage: Handling of player houses                                       *
   *                                                                         *
   *  All rights reserved.  See license.doc for complete information.        *
   *                                                                         *
   *  Copyright (C) 1993, 94 by the Trustees of the Johns Hopkins University *
   *  CircleMUD is based on DikuMUD, Copyright (C) 1990, 1991.               *
   ************************************************************************ */

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include <errno.h>

#include "char.h"
#include "object.h"
#include "global.h"
#include "comm.h"
#include "gamedb.h"
#include "actions.h"


#define HCONTROL_FILE   	"house/hcontrol"  /* for the house system */

/* NOTE: struct obj_file_elm is same type as struct obj_data */
#define obj_file_elem 		obj_data

#define GET_OBJ_VNUM(obj) 	GET_OBJ_VIRTUAL(obj)
/* NOTE: Rent fee is 1/10 of object cost */
#define GET_OBJ_RENT(obj)	((obj)->obj_flags.cost/10)
/* NOTE: Char's ID number is not used */
/* #define GET_IDNUM(ch) ((ch)->player.time.birth) */
#define debuglog(str, type, level, file) (log(str))

#ifdef NO_DEF
/* NOTE: This part moved to "include/object.h"                          */
#define ROOM_FLAGS(rn)		(world[(rn)].room_flags)

#define ROOM_HOUSE		(1 << 20)	/* (R) Room is a house       */
#define ROOM_HOUSE_CRASH	(1 << 21)	/* (R) House needs saving    */
#define ROOM_ATRIUM          (1 << 22) /* (R) The door to a house   */

#define TOROOM(room, dir) (world[room].dir_option[dir] ? \
			    world[room].dir_option[dir]->to_room : NOWHERE)

#endif				/* NO_DEF */
/* NOTE: ROOM_ATRIUM not used. No atrium room. */

#define ROOM_PRIVATE		(PRIVATE)	/* Can't teleport in         */

#define LVL_GRGOD		(LEV_GOD)
#define LVL_IMMORT		(LEV_IMMO)

/*
   char    buf[MAX_BUFSIZ];
   char    buf1[MAX_BUFSIZ];
   char    buf2[MAX_BUFSIZ];
   char    arg[MAX_BUFSIZ];
 */

/* NOTE: Extern's */
extern void free_obj(struct obj_data *obj);
extern struct char_data *get_char(char *name);
extern int find_name(char *name);

#undef CAP
/* NOTE: CAP() will return the sting as value */
#define CAP(st)  (*(st) = UPPER(*(st)), st)

/* ================================================================== */

/* NOTE: Special function for house rooom */
int House_spec(struct char_data *ch, int cmd, char *arg)
{
    void House_save(int rnum);

    if (!ROOM_FLAGGED(ch->in_room, ROOM_HOUSE))
	return (FALSE);
    switch (cmd) {
	/* NOTE: Room content changed. Mark for save later */
    case CMD_PUT: case CMD_GET: case CMD_DROP: case CMD_JUNK:
	SET_BIT(ROOM_FLAGS(ch->in_room), ROOM_HOUSE_CRASH);
	break;

    case CMD_SAVE: case CMD_RENT:
    case CMD_NORTH: case CMD_EAST: case CMD_SOUTH: case CMD_WEST:
    case CMD_UP: case CMD_DOWN:
	/* NOTE: save the room */
	if (ROOM_FLAGGED(ch->in_room, ROOM_HOUSE_CRASH))
	    House_save(ch->in_room);
	break;

    case CMD_QUIT:
	/* NOTE: Hmm... Is this needed ???????? */
	/* NOTE: If someone is quitting in their house, load them back here */
	save_char(ch);
	break;
    }
    return (FALSE);
}
/* ------------------------------------------------------------------ */
/* NOTE: These were part of CircleMUD "objsave.c".                    */

/* NOTE: NEW! Reverse orders in obj list */
struct obj_data *reverse_obj_list(struct obj_data *obj)
{
    struct obj_data *new_head;
    if(!obj || !obj->next_content)
	return(obj);

    new_head = reverse_obj_list(obj->next_content);
    obj->next_content->next_content = obj;
    obj->next_content = NULL;
    return(new_head);
}
    
struct obj_data *Obj_from_store(FILE * fl)
{
    struct obj_data *obj, object, *tobj, *obj_next;
    int j;

    fread(&object, sizeof(struct obj_file_elem), 1, fl);

    if (feof(fl))
	return (NULL);

    if (real_object(object.item_number) > -1) {
	obj = read_object(object.item_number, VIRTUAL);
	/* 
	GET_OBJ_VAL(obj, 0) = object.value[0];
	GET_OBJ_VAL(obj, 1) = object.value[1];
	GET_OBJ_VAL(obj, 2) = object.value[2];
	GET_OBJ_VAL(obj, 3) = object.value[3];
	GET_OBJ_EXTRA(obj) = object.extra_flags;
	GET_OBJ_WEIGHT(obj) = object.weight;
	GET_OBJ_TIMER(obj) = object.timer;
	obj->obj_flags.bitvector =
	object.bitvector;
	*/ 
    }
    else
	return(NULL); 

    /* NOTE: BUG FIX: Reported by JINNY. Weight restore and NORENT */
    /* NOTE: Don't restore obj_flags of NORENT item. */ 
    if (!IS_SET(object.obj_flags.extra_flags, ITEM_NORENT)) {
	/* NOTE: Restore original weight. */
	object.obj_flags.weight = GET_OBJ_WEIGHT(obj);
	obj->obj_flags = object.obj_flags;
	for (j = 0; j < MAX_OBJ_AFFECT; j++)
	    obj->affected[j] = object.affected[j];
    } 

    if (GET_ITEM_TYPE(obj) == ITEM_CONTAINER && object.contains) {
	tobj = Obj_from_store(fl);
	tobj = reverse_obj_list(tobj);
	for (; tobj; tobj = obj_next) {
	    obj_next = tobj->next_content;
	    obj_to_obj(tobj, obj); 
	}
    }

    if (object.next_content)
	obj->next_content = Obj_from_store(fl);

    return obj;
}

int Obj_to_store(struct obj_data *obj, FILE * fl)
{
    struct obj_file_elem object;

    if (!obj)
	return 0;
    object = *obj;
    object.item_number = GET_OBJ_VNUM(obj);
    /* 
    object.value[0] = GET_OBJ_VAL(obj, 0);
    object.value[1] = GET_OBJ_VAL(obj, 1);
    object.value[2] = GET_OBJ_VAL(obj, 2);
    object.value[3] = GET_OBJ_VAL(obj, 3);
    object.extra_flags = GET_OBJ_EXTRA(obj);
    object.weight = GET_OBJ_WEIGHT(obj);
    object.timer = GET_OBJ_TIMER(obj);
    object.bitvector = obj->obj_flags.bitvector;
    */

    /* NOTE: make NORENT item to useless item: 88 cigarette  */
    if (IS_SET(object.obj_flags.extra_flags, ITEM_NORENT))
	object.item_number = 101;

    /* NOTE: No need to adjust weight of item. */

    if (fwrite(&object, sizeof(struct obj_file_elem), 1, fl) < 1) {
	perror("Error writing object in Obj_to_store");
	return 0;
    }

    if (GET_ITEM_TYPE(obj) == ITEM_CONTAINER && obj->contains)
	Obj_to_store(obj->contains, fl);
    Obj_to_store(obj->next_content, fl);

    return 1;
}
/* ------------------------------------------------------------------ */
/* NOTE: These were part of CircleMUD "house.h".                      */

#define MAX_HOUSES	20
#define MAX_GUESTS	5

#define HOUSE_PRIVATE	1

/* NOTE: owner, guest is string , not player ID number */
struct house_control_rec {
    sh_int vnum;	/* vnum of this house           */
    sh_int exit_num;	/* direction of house's exit    */
    time_t built_on;	/* date this house was built    */
    int mode;		/* mode of ownership            */
    char owner[15];	/* name str of house's owner    */
    int num_of_guests;	/* how many guests for house    */
    char guests[MAX_GUESTS][15];	/* name str of house's guests   */
    time_t last_payment;	/* date of last house payment   */
    long spare0;
    long spare1;
    long spare2;
    long spare3;
    long spare4;
    long spare5;
    long spare6;
    long spare7;
};

void House_listrent(struct char_data *ch, char *arg);
void House_boot(void);
void House_save_all(void);
int House_can_enter(struct char_data *ch, sh_int house);

/* struct obj_data *Obj_from_store(struct obj_file_elem object); */
struct obj_data *Obj_from_store(FILE * fl);
int Obj_to_store(struct obj_data *obj, FILE * fl);

/* ------------------------------------------------------------------ */
/* NOTE: These were part of CircleMUD "house.c".                      */

#ifdef NO_DEF

#include "conf.h"
#include "sysdep.h"


#include "structs.h"
#include "comm.h"
#include "handler.h"
#include "db.h"
#include "interpreter.h"
#include "utils.h"
#include "house.h"

extern char *dirs[];
extern struct room_data *world;
extern int top_of_world;
extern const int rev_dir[];
extern struct index_data *obj_index;

#endif

/* NOTE: house_control[] is array of *ponter* to record, not static array */
/* NOTE: Records are dynamically allocated at house loading time */
struct house_control_rec *house_control[MAX_HOUSES];
int num_of_houses = 0;

/* First, the basics: finding the filename; loading/saving objects */

/* NOTE: Return a filename given a house *real* room num */
int House_get_filename(int rnum, char *filename)
{
    int vnum;

    if (rnum <= 0 || (vnum = world[rnum].number) <= 0)
	return 0;

    sprintf(filename, "house/%d.house", vnum);
    return 1;
}

/* NOTE: Arg is real room number */
/* Load all objects for a house */
int House_load(int rnum)
{
    FILE *fl;
    char fname[MAX_STRING_LENGTH];
    struct obj_data *obj, *obj_next;

    if (!House_get_filename(rnum, fname))
	return 0;
    if (!(fl = fopen(fname, "r+b"))) {
	/* no file found */
	return 0;
    }
    if((obj = Obj_from_store(fl))) {
	if (ferror(fl)) {
	    perror("Reading house file: House_load.");
	    fclose(fl);
	    return 0;
	}
	obj = reverse_obj_list(obj);
	for (; obj ; obj = obj_next ) {
	    obj_next = obj->next_content;
	    obj_to_room(obj, rnum);
	}
    }

    fclose(fl); 
    return 1;
}

/* restore weight of containers after House_save has changed them for saving */
void House_restore_weight(struct obj_data *obj)
{
    if (obj) {
	House_restore_weight(obj->contains);
	House_restore_weight(obj->next_content);
	if (obj->in_obj)
	    GET_OBJ_WEIGHT(obj->in_obj) += GET_OBJ_WEIGHT(obj);
    }
}

/* NOTE: a room number is real room number, not virtual */
/* Save all objects for a house (recursive; initial call must be followed
   by a call to House_restore_weight)  Assumes file is open already. */
void House_save(int rnum)
{
    char buf[MAX_BUFSIZ];
    FILE *fp;

    if (!House_get_filename(rnum, buf))
	return;
    if (!(fp = fopen(buf, "wb"))) {
	perror("SYSERR: Error saving house file");
	return;
    }
    if (!Obj_to_store(world[rnum].contents, fp)) {
	fclose(fp);
	return;
    }
    fclose(fp);

    REMOVE_BIT(ROOM_FLAGS(rnum), ROOM_HOUSE_CRASH);
    House_restore_weight(world[rnum].contents);

    return;
}

/* NOTE: House_crashsave(int vnum) is renamed as House_save() */
/* Save all objects in a house */
/* void House_crashsave(int vnum) */


/* Delete a house save file */
void House_delete_file(int vnum)
{
    char buf[MAX_BUFSIZ], fname[MAX_INPUT_LENGTH];
    FILE *fl;

    if (!House_get_filename(real_room(vnum), fname))
	return;
    if (!(fl = fopen(fname, "rb"))) {
	if (errno != ENOENT) {
	    sprintf(buf, "SYSERR: Error deleting house file #%d. (1)", vnum);
	    perror(buf);
	}
	return;
    }
    fclose(fl);
    if (unlink(fname) < 0) {
	sprintf(buf, "SYSERR: Error deleting house file #%d. (2)", vnum);
	perror(buf);
    }
}


/******************************************************************
 *  Functions for house administration (creation, deletion, etc.  *
 *****************************************************************/

/* NOTE: first arg is real room number */
int find_house(int rnum, struct char_data *ch)
{
    int i, vnum;

    if (rnum == 0 && ch && ch->in_room > NOWHERE)
	rnum = ch->in_room;
    if (rnum < 0 || (vnum = world[rnum].number) < 0)
	return -1;
    for (i = 0; i < num_of_houses; i++)
	if (house_control[i]->vnum == vnum)
	    return i;

    return -1;
} 

/* Save the house control information */
void House_save_control(void)
{
    FILE *fl;
    int i;

    if (!(fl = fopen(HCONTROL_FILE, "wb"))) {
	perror("SYSERR: Unable to open house control file");
	return;
    }
    /* write all the house control recs in one fell swoop.  Pretty nifty, eh? 
     */
    for (i = 0; i < num_of_houses; i++)
	fwrite(house_control[i], sizeof(struct house_control_rec), 1, fl);

    fclose(fl);
}

/* call from boot_db - will load control recs, load objs */
/* should do sanity checks on vnums & remove invalid records */
void House_boot(void)
{
    struct house_control_rec temp_house;
    sh_int real_house;
    FILE *fl;

    memset((char *) house_control, 0, sizeof(struct house_control_rec *) * MAX_HOUSES);

    if (!(fl = fopen(HCONTROL_FILE, "rb"))) {
	log("House control file does not exist.");
	return;
    }
    while (!feof(fl) && num_of_houses < MAX_HOUSES) {
	fread(&temp_house, sizeof(struct house_control_rec), 1, fl);

	if (feof(fl))
	    break;

	if (find_name(temp_house.owner) < 0)
	    continue;	/* owner no longer exists -- skip */

	if ((real_house = real_room(temp_house.vnum)) < 0)
	    continue;	/* this vnum doesn't exist -- skip */

	if ((find_house(real_house, NULL)) >= 0)
	    continue;	/* this vnum is already a hosue -- skip */

	if (temp_house.exit_num < 0 || temp_house.exit_num >= NUM_OF_DIRS)
	    continue;	/* invalid exit num -- skip */

	/* NOTE: Allocate real house_control_rec */
	CREATE(house_control[num_of_houses], struct house_control_rec, 1);

	*house_control[num_of_houses++] = temp_house;

	SET_BIT(ROOM_FLAGS(real_house), ROOM_HOUSE | ROOM_PRIVATE);

	/* NOTE: assign special function to house room */
	world[real_house].funct = House_spec;

	House_load(real_house);
    }

    fclose(fl);
    House_save_control();
}



/* "House Control" functions */

char *HCONTROL_FORMAT =
"Usage: hcontrol build <house vnum> <exit direction> <player name>\r\n"
"       hcontrol destroy <house vnum>\r\n"
"       hcontrol pay <house vnum>\r\n"
"       hcontrol list\r\n"
"       hcontrol contents <house vnum>\r\n";

#define NAME(player) ((find_name(player)) <0  ? "<UNDEF>" : (player))

void hcontrol_list_houses(struct char_data *ch)
{
    int i, j;
    char *timestr;
    char built_on[128], last_pay[128], own_name[128];
    char buf[MAX_OUT_LEN], buf2[MAX_LINE_LEN];

    if (!num_of_houses) {
	send_to_char("No houses have been defined.\r\n", ch);
	return;
    }
    strcpy(buf, "Address  Owner        Guests  Build Date  Last Paymt\r\n");
    strcat(buf, "-------  ------------ ------  ----------  ----------\r\n");

    send_to_char(buf, ch);
    for (i = 0; i < num_of_houses; i++) {
	if (house_control[i]->built_on) {
	    timestr = asctime(localtime(&(house_control[i]->built_on)));
	    *(timestr + 10) = 0;
	    strcpy(built_on, timestr);
	}
	else
	    strcpy(built_on, "Unknown");

	if (house_control[i]->last_payment) {
	    timestr = asctime(localtime(&(house_control[i]->last_payment)));
	    *(timestr + 10) = 0;
	    strcpy(last_pay, timestr);
	}
	else
	    strcpy(last_pay, "None");

	strcpy(own_name, NAME(house_control[i]->owner));

	/* NOTE: New listing format */
	sprintf(buf, "%7d  %-12s    %2d   %-10s  %s\r\n",
		house_control[i]->vnum, CAP(own_name),
		house_control[i]->num_of_guests, built_on, last_pay); 
	send_to_char(buf, ch);

	if (house_control[i]->num_of_guests) {
	    strcpy(buf, "        (Guests): ");
	    for (j = 0; j < house_control[i]->num_of_guests; j++) {
		sprintf(buf2, "%s ", NAME(house_control[i]->guests[j]));
		strcat(buf, CAP(buf2));
	    }
	    strcat(buf, "\r\n");
	    send_to_char(buf, ch);
	}
    }
}

void hcontrol_build_house(struct char_data *ch, char *arg)
{
    char arg1[MAX_LINE_LEN], buf[MAX_OUT_LEN];
    struct house_control_rec temp_house;
    sh_int virt_house, real_house, exit_num;

    extern const int rev_dir[];

    if (num_of_houses >= MAX_HOUSES) {
	send_to_char("Max houses already defined.\r\n", ch);
	return;
    }

    /* first arg: house's vnum */
    arg = one_argument(arg, arg1);
    if (!*arg1) {
	send_to_char(HCONTROL_FORMAT, ch);
	return;
    }
    virt_house = atoi(arg1);
    if ((real_house = real_room(virt_house)) < 0) {
	send_to_char("No such room exists.\r\n", ch);
	return;
    }
    if ((find_house(real_house, ch)) >= 0) {
	send_to_char("House already exists.\r\n", ch);
	return;
    }

    /* second arg: direction of house's exit */
    arg = one_argument(arg, arg1);
    if (!*arg1) {
	send_to_char(HCONTROL_FORMAT, ch);
	return;
    }
    if ((exit_num = search_block(arg1, dirs, FALSE)) < 0) {
	sprintf(buf, "'%s' is not a valid direction.\r\n", arg1);
	send_to_char(buf, ch);
	return;
    }
    if (TOROOM(real_house, exit_num) == NOWHERE) {
	sprintf(buf, "There is no exit %s from room %d.\r\n", dirs[exit_num],
		virt_house);
	send_to_char(buf, ch);
	return;
    }

    if (TOROOM(TOROOM(real_house, exit_num), rev_dir[exit_num]) != real_house) {
	send_to_char("A house's exit must be a two-way door.\r\n", ch);
	return;
    }

    /* third arg: player's name */
    arg = one_argument(arg, arg1);
    if (!*arg1) {
	send_to_char(HCONTROL_FORMAT, ch);
	return;
    }
    if ((find_name(arg1)) < 0) {
	sprintf(buf, "Unknown player '%s'.\r\n", arg1);
	send_to_char(buf, ch);
	return;
    }

    temp_house.mode = HOUSE_PRIVATE;
    temp_house.vnum = virt_house;
    temp_house.exit_num = exit_num;
    temp_house.built_on = time(0);
    temp_house.last_payment = 0;
    strcpy(temp_house.owner, arg1);
    temp_house.num_of_guests = 0;

    /* NOTE: Allocate house_control_rec. */
    CREATE(house_control[num_of_houses], struct house_control_rec, 1);
    *house_control[num_of_houses++] = temp_house;

    /* NOTE: assign special function to house room. */
    world[real_house].funct = House_spec;

    SET_BIT(ROOM_FLAGS(real_house), ROOM_HOUSE | ROOM_PRIVATE);
    House_save(real_house); 

    send_to_char("House built.  Mazel tov!\r\n", ch);
    House_save_control();
}



void hcontrol_destroy_house(struct char_data *ch, char *arg)
{
    int i;
    sh_int real_house;

    if (!*arg) {
	send_to_char(HCONTROL_FORMAT, ch);
	return;
    }
    if ((i = find_house(real_room(atoi(arg)), ch)) < 0) {
	send_to_char("Unknown house.\r\n", ch);
	return;
    }

    if ((real_house = real_room(house_control[i]->vnum)) < 0)
	log("SYSERR: House had invalid vnum!");
    else
	REMOVE_BIT(ROOM_FLAGS(real_house),
		   ROOM_HOUSE | ROOM_PRIVATE | ROOM_HOUSE_CRASH);

    House_delete_file(house_control[i]->vnum);

    /* NOTE: Remove special function from house room. */
    world[real_house].funct = NULL;
    /* NOTE: Free house_control_rec. */
    free(house_control[i]);

    num_of_houses--;
    house_control[i] = house_control[num_of_houses];

    send_to_char("House deleted.\r\n", ch);
    House_save_control();
}


void hcontrol_pay_house(struct char_data *ch, char *arg)
{
    int i;
    char buf[MAX_BUFSIZ];

    if (!*arg)
	send_to_char(HCONTROL_FORMAT, ch);
    else if ((i = find_house(real_room(atoi(arg)), ch)) < 0)
	send_to_char("Unknown house.\r\n", ch);
    else {
	sprintf(buf, "Payment for house %s collected by %s.",
		arg, GET_NAME(ch));
	debuglog(buf, NRM, MAX(LVL_IMMORT, GET_INVIS_LEV(ch)), TRUE);

	house_control[i]->last_payment = time(0);
	House_save_control();
	send_to_char("Payment recorded.\r\n", ch);
    }
}

/* NOTE: This is not so smart. Need to count all in store. */
/* List all objects in a house file */
void House_listrent(struct char_data *ch, char *arg)
{
    FILE *fl;
    char fname[MAX_STRING_LENGTH];
    char buf[MAX_STRING_LENGTH];
    struct obj_data *obj;
    int vnum;


    if (!arg || !*arg) {
	send_to_char(HCONTROL_FORMAT, ch);
	return;
    }
    vnum = atoi(arg);
    if (!House_get_filename(real_room(vnum), fname))
	return;
    if (!(fl = fopen(fname, "rb"))) {
	sprintf(buf, "No objects on file for house #%d.\r\n", vnum);
	send_to_char(buf, ch);
	return;
    }
    while (!feof(fl)) {
	if ((obj = Obj_from_store(fl)) != NULL) {
	    sprintf(buf, " [%5d] (%5dau) %s\r\n", 
	      GET_OBJ_VNUM(obj), GET_OBJ_RENT(obj), obj->short_description);
	    free_obj(obj);
	}
	send_to_char(buf, ch);
    } 
    fclose(fl);
}



/* The hcontrol command itself, used by imms to create/destroy houses */
void do_hcontrol(struct char_data *ch, char *argument, int cmd)
{
    char arg1[MAX_INPUT_LENGTH], arg2[MAX_INPUT_LENGTH];

    half_chop(argument, arg1, arg2);

    if (is_abbrev(arg1, "build"))
	hcontrol_build_house(ch, arg2);
    else if (is_abbrev(arg1, "destroy"))
	hcontrol_destroy_house(ch, arg2);
    else if (is_abbrev(arg1, "pay"))
	hcontrol_pay_house(ch, arg2);
    else if (is_abbrev(arg1, "list"))
	hcontrol_list_houses(ch);
    else if (is_abbrev(arg1, "contents"))
	House_listrent(ch, arg1);
    else
	send_to_char(HCONTROL_FORMAT, ch);
}


/* The house command, used by mortal house owners to assign guests */
void do_house(struct char_data *ch, char *argument, int cmd)
{
    int i, j;
    char arg[MAX_INPUT_LENGTH], buf[MAX_BUFSIZ];

    one_argument(argument, arg);

    if (!IS_SET(ROOM_FLAGS(ch->in_room), ROOM_HOUSE))
	send_to_char("You must be in your house to set guests.\r\n", ch);
    else if ((i = find_house(ch->in_room, ch)) < 0)
	send_to_char("Um.. this house seems to be screwed up.\r\n", ch);
    else if (str_cmp(GET_NAME(ch), house_control[i]->owner) != 0)
	send_to_char("Only the primary owner can set guests.\r\n", ch);
    else if (!*arg) {
	send_to_char("Guests of your house:\r\n", ch);
	if (house_control[i]->num_of_guests == 0)
	    send_to_char("  None.\r\n", ch);
	else
	    for (j = 0; j < house_control[i]->num_of_guests; j++) {
		strcpy(buf, NAME(house_control[i]->guests[j]));
		send_to_char(strcat(CAP(buf), "\r\n"), ch);
	    }
    }
    else if (find_name(arg) < 0)
	send_to_char("No such player.\r\n", ch);
    else {
	for (j = 0; j < house_control[i]->num_of_guests; j++)
	    if (str_cmp(house_control[i]->guests[j], arg) == 0) {
		house_control[i]->num_of_guests--;
		strcpy(house_control[i]->guests[j],
		house_control[i]->guests[(house_control[i]->num_of_guests)]);
		House_save_control();
		send_to_char("Guest deleted.\r\n", ch);
		return;
	    }
	j = house_control[i]->num_of_guests++;
	strcpy(house_control[i]->guests[j], arg);
	House_save_control();
	send_to_char("Guest added.\r\n", ch);
    }
}



/* Misc. administrative functions */


/* crash-save all the houses */
void House_save_all(void)
{
    int i;
    sh_int real_house;

    for (i = 0; i < num_of_houses; i++) {
	real_house = real_room(house_control[i]->vnum);
	if (real_house != NOWHERE
	    && IS_SET(ROOM_FLAGS(real_house), ROOM_HOUSE_CRASH))
	    House_save(real_house);
    }
    House_save_control();
}


/* note: arg passed must be house vnum, so there. */
/* NOTE: house arg is real room number, not virtual */
int House_can_enter(struct char_data *ch, sh_int house)
{
    int i, j;

    if (GET_LEVEL(ch) == LVL_GRGOD || (i = find_house(house, ch)) < 0)
	return 1;

    if ( IS_SET(house_control[i]->mode, HOUSE_PRIVATE)) {
	if (str_cmp(GET_NAME(ch), house_control[i]->owner) == 0)
	    return 1;
	for (j = 0; j < house_control[i]->num_of_guests; j++)
	    if (str_cmp(GET_NAME(ch), house_control[i]->guests[j]) == 0)
		return 1;
	return 0;
    }

    return 0;
}


/* ************************************************************************
   *   File: graph.c                                       Part of CircleMUD *
   *  Usage: various graph algorithms                                        *
   *                                                                         *
   *  All rights reserved.  See license.doc for complete information.        *
   *                                                                         *
   *  Copyright (C) 1993, 94 by the Trustees of the Johns Hopkins University *
   *  CircleMUD is based on DikuMUD, Copyright (C) 1990, 1991.               *
   ************************************************************************ */


#define TRACK_THROUGH_DOORS

/* You can define or not define TRACK_THOUGH_DOORS, above, depending on
   whether or not you want track to find paths which lead through closed
   or hidden doors.
 */

/* breadth-first searching */
#define BFS_ERROR               -1
#define BFS_ALREADY_THERE       -2
#define BFS_NO_PATH             -3

#ifdef NO_DEF

#include "conf.h"
#include "sysdep.h"


#include "structs.h"
#include "utils.h"
#include "comm.h"
#include "interpreter.h"
#include "handler.h"
#include "db.h"
#include "spells.h"


/* Externals */
extern int top_of_world;
extern const char *dirs[];
extern struct room_data *world;

#endif				/* NO_DEF */
#include "spells.h"
#include "play.h"

struct bfs_queue_struct {
    sh_int room;
    char dir;
    struct bfs_queue_struct *next;
};

static struct bfs_queue_struct *queue_head = 0, *queue_tail = 0;

/* Utility macros */
#define MARK(room) (SET_BIT(ROOM_FLAGS(room), ROOM_BFS_MARK))
#define UNMARK(room) (REMOVE_BIT(ROOM_FLAGS(room), ROOM_BFS_MARK))
#define IS_MARKED(room) (IS_SET(ROOM_FLAGS(room), ROOM_BFS_MARK))
/* NOTE: #define TOROOM(x, y) (world[(x)].dir_option[(y)]->to_room) */
#define IS_CLOSED(x, y) (IS_SET(world[(x)].dir_option[(y)]->exit_info, EX_CLOSED))

#ifdef TRACK_THROUGH_DOORS
#define VALID_EDGE(x, y) (world[(x)].dir_option[(y)] && \
			  (TOROOM(x, y) != NOWHERE) &&	\
			  (!ROOM_FLAGGED(TOROOM(x, y), ROOM_NOTRACK)) && \
			  (!IS_MARKED(TOROOM(x, y))))
#else
#define VALID_EDGE(x, y) (world[(x)].dir_option[(y)] && \
			  (TOROOM(x, y) != NOWHERE) &&	\
			  (!IS_CLOSED(x, y)) &&		\
			  (!ROOM_FLAGGED(TOROOM(x, y), ROOM_NOTRACK)) && \
			  (!IS_MARKED(TOROOM(x, y))))
#endif

void bfs_enqueue(sh_int room, int dir)
{
    struct bfs_queue_struct *curr;

    CREATE(curr, struct bfs_queue_struct, 1);

    curr->room = room;
    curr->dir = dir;
    curr->next = 0;

    if (queue_tail) {
	queue_tail->next = curr;
	queue_tail = curr;
    }
    else
	queue_head = queue_tail = curr;
}


void bfs_dequeue(void)
{
    struct bfs_queue_struct *curr;

    curr = queue_head;

    if (!(queue_head = queue_head->next))
	queue_tail = 0;
    free(curr);
}


void bfs_clear_queue(void)
{
    while (queue_head)
	bfs_dequeue();
}


/* find_first_step: given a source room and a target room, find the first
   step on the shortest path from the source to the target.

   Intended usage: in mobile_activity, give a mob a dir to go if they're
   tracking another mob or a PC.  Or, a 'track' skill for PCs.
 */

int find_first_step(sh_int src, sh_int target)
{
    int curr_dir;
    /* sh_int curr_room; */

    if (src < 0 || src > top_of_world || target < 0 || target > top_of_world) {
	log("Illegal value passed to find_first_step (graph.c)");
	return BFS_ERROR;
    }
    if (src == target)
	return BFS_ALREADY_THERE;

#ifdef NO_DEF
    /* NOTE: BFS_MARK room bit is cleared *after* search */
    /* clear marks first */
    for (curr_room = 0; curr_room <= top_of_world; curr_room++)
	UNMARK(curr_room);
#endif		/* NO_DEF */

    MARK(src);

    /* first, enqueue the first steps, saving which direction we're going. */
    for (curr_dir = 0; curr_dir < NUM_OF_DIRS; curr_dir++)
	if (VALID_EDGE(src, curr_dir)) {
	    MARK(TOROOM(src, curr_dir));
	    bfs_enqueue(TOROOM(src, curr_dir), curr_dir);
	}
    /* now, do the classic BFS. */
    while (queue_head) {
	if (queue_head->room == target) {
	    curr_dir = queue_head->dir;
	    bfs_clear_queue();
	    return curr_dir;
	}
	else {
	    for (curr_dir = 0; curr_dir < NUM_OF_DIRS; curr_dir++)
		if (VALID_EDGE(queue_head->room, curr_dir)) {
		    MARK(TOROOM(queue_head->room, curr_dir));
		    bfs_enqueue(TOROOM(queue_head->room, curr_dir), 
				queue_head->dir);
		}
	    bfs_dequeue();
	}
    }

    return BFS_NO_PATH;
}


/************************************************************************
*  Functions and Commands which use the above fns		        *
************************************************************************/

void do_track(struct char_data *ch, char *argument, int cmd)
{
    char arg[MAX_INPUT_LENGTH];
    char buf[MAX_BUFSIZ];
    struct char_data *vict;
    int dir, num, curr_room;

    num = number(0, 150);
    if ( GET_LEARNED(ch, SKILL_TRACK) + GET_SKILLED(ch, SKILL_TRACK) < num) {
	/* send_to_char("You have no idea how.\r\n", ch); */
	send_to_char("The trail is too faint to track.\r\n", ch);
	return;
    }

    one_argument(argument, arg);
    if (!*arg) {
	send_to_char("Whom are you trying to track?\r\n", ch);
	return;
    }
    if (!(vict = get_char_vis(ch, arg))) {
	send_to_char("No-one around by that name.\r\n", ch);
	return;
    }
    if (IS_AFFECTED(vict, AFF_NOTRACK)) {
	send_to_char("You sense no trail.\r\n", ch);
	return;
    }

    dir = find_first_step(ch->in_room, vict->in_room);
    /* NOTE: clear marks after search */
    for (curr_room = 0; curr_room <= top_of_world; curr_room++)
	UNMARK(curr_room); 

    switch (dir) {
    case BFS_ERROR:
	send_to_char("Hmm.. something seems to be wrong.\r\n", ch);
	break;
    case BFS_ALREADY_THERE:
	send_to_char("You're already in the same room!!\r\n", ch);
	break;
    case BFS_NO_PATH:
	sprintf(buf, "You can't sense a trail to %s from here.\r\n",
		HMHR(vict));
	send_to_char(buf, ch);
	break;
    default:
	num = number(0, 101);	/* 101% is a complete failure */
	/* NOTE: Cost: mana comsumption and full delay  */
	GET_MANA(ch) -= 100 - GET_SKILLED(ch, SKILL_TRACK)/2;
	WAIT_STATE(ch, PULSE_VIOLENCE );
	
	if ( num < ( 30 + GET_LEARNED(ch, SKILL_TRACK) 
		+ GET_SKILLED(ch, SKILL_TRACK)) / 2 || IS_DIVINE(ch)) {
	    INCREASE_SKILLED(ch, NULL, SKILL_TRACK);
	}
	else
	    do {
		dir = number(0, NUM_OF_DIRS - 1);
	    } while (!CAN_GO(ch, dir));
	sprintf(buf, "You sense a trail %s from here!\r\n", dirs[dir]);
	send_to_char(buf, ch);
	break;
    }
}

#ifdef NO_DEF
/* NOTE: Not used currently */
void hunt_victim(struct char_data *ch)
{
    extern struct char_data *character_list;

    int dir;
    byte found;
    struct char_data *tmp;
    char buf[MAX_BUFSIZ];

    if (!ch || !GET_HUNTING(ch))
	return;

    /* make sure the char still exists */
    for (found = 0, tmp = character_list; tmp && !found; tmp = tmp->next)
	if (GET_HUNTING(ch) == tmp)
	    found = 1;

    if (!found) {
	do_say(ch, "Damn!  My prey is gone!!", 0);
	GET_HUNTING(ch) = 0;
	return;
    }
    dir = find_first_step(ch->in_room, GET_HUNTING(ch)->in_room);
    if (dir < 0) {
	sprintf(buf, "Damn!  Lost %s!", HMHR(GET_HUNTING(ch)));
	do_say(ch, buf, 0);
	GET_HUNTING(ch) = 0;
    }
    else {
	do_move(ch, "", dir+1);
	if (ch->in_room == GET_HUNTING(ch)->in_room)
	    hit(ch, GET_HUNTING(ch), TYPE_UNDEFINED);
    }
}
#endif		/* NO_DEF */
