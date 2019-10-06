/* ************************************************************************
   *  file: actwiz.c , Implementation of commands.           Part of DIKUMUD *
   *  Usage : Wizard Commands.                                               *
   *  Copyright (C) 1990, 1991 - see 'license.doc' for complete information. *
   ************************************************************************* */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <time.h>
#include <sys/resource.h>

#include "char.h"
#include "object.h"
#include "global.h"
#include "comm.h"
#include "gamedb.h"
#include "actions.h"
#include "etc.h"


/* NOTE: moved from comm.c */
#define BADDOMS 	16
int baddoms;
char baddomain[BADDOMS][32];

/* external functs */
extern struct char_data *get_char(char *name);
extern void init_player(struct char_data *ch);
extern void do_start(struct char_data *ch);
extern void purge_player(struct char_data *ch); 

/* NOTE: do_shutdown() will check complete command line.  */
/*
void do_shutdow(struct char_data *ch, char *argument, int cmd)
{
    send_to_char("If you want to shut something down - say so!\n\r", ch);
}
*/

void do_shutdown(struct char_data *ch, char *argument, int cmd)
{
    extern int shutdowngame;
    char arg[MAX_INPUT_LENGTH]; 
    extern char *CMD_LINE;

    /* NOTE: make sure that 'shutdown' is not abbreviated */
    if ( strncmp(CMD_LINE, "shutdown", 8 ) != 0 ){
	send_to_char("If you want to shut down, enter 'shutdown'.\r\n", ch );
	return;
    }

    if (IS_NPC(ch) || GET_LEVEL(ch) > (IMO + 3))
	return;
    one_argument(argument, arg);
    if (!*arg) {
	send_to_all("Shutting down immediately.\n\r");
	/* NOTE: Leave log about who did shutdown */
        sprintf(arg, "***** Shutdown by %s ******", GET_NAME(ch));
	log(arg);
	shutdowngame = 1;
    }
    else
	send_to_char("Go shut down someone your own size.\n\r", ch);
}

#define STR_NONE(s) ((s) ? (s) : "None.")

/* NOTE: do_stat() is divided into 3 piece: 
	stat_room(), stat_object(), stat_char() */
/* NOTE: MSTAT/PSTAT/STAT wiz command will search mob/player in 
   current zone before seaching whole world for mob/player. */ 
/* NOTE: 'stat' command takes keyword to specify type of target
	like this 'stat <room|obj|player|mob|char> target'. */
void do_stat(struct char_data *ch, char *argument, int cmd)
{
    char arg1[MAX_INPUT_LENGTH], arg2[MAX_INPUT_LENGTH];
    struct char_data *k = 0;
    struct obj_data *j = 0;
    static char *keywords[] = { "",
	"player", "mobile", "character", "object", "room", "\n" };

    void stat_room(struct char_data *ch, int rm_nr);
    void stat_object(struct char_data *ch, struct obj_data *obj);
    void stat_char(struct char_data *ch, struct char_data *kch); 
    extern struct char_data *get_char_wiz(struct char_data *ch,
				   char *name, int type);

    if (IS_NPC(ch))
	return; 
    half_chop(argument, arg1, arg2);

    if ( cmd == CMD_PSTAT ) {
	strcpy(arg2, arg1);
	strcpy(arg1, "player");
    }
    else if ( cmd == CMD_MSTAT) {
	strcpy(arg2, arg1);
	strcpy(arg1, "mobile");
    } 
    else if ( !*arg1 )    /* NOTE: If No arg, default is 'stat room'  */ 
	strcpy(arg1, "room" );
    
    switch( search_block( arg1, keywords, 0 )) { 
    case 1 :   /* pstat PC */
	/* NOTE: get_specific_vis() is chnaged to get_char_wiz() */
	if(( k = get_char_wiz(ch, arg2, 01)))
	     stat_char(ch, k ); 
	break;
    case 2 :   /* mstat mob */
	if (( k = get_char_wiz(ch, arg2, 02)))
	     stat_char(ch, k ); 
	break;
    case 3 :   /* stat PC or mob */
	if((k = get_char_wiz(ch, arg2, 03)))
	    stat_char(ch, k ); 
	break;
    case 4 :   /* stat obj */ 
	if((j = get_obj_vis(ch, arg2)))
	    stat_object(ch, j );
	break;
    case 5 :   /* stats on room */ 
    /* NOTE: 'stat room' can take room number arg   */
        if ( *arg2 ) 
	    stat_room(ch, real_room(atoi(arg2)));
	else
	    stat_room(ch, ch->in_room);
	return;
    default :
	send_to_char("stat <room|obj|player|mob|char> target.\n\r", ch);
	return;
    }
    if (!k && !j ) 
	send_to_char("No mobile or object by that name in the world\n\r", ch);
}

void stat_room(struct char_data *ch, int rm_nr)
{
    struct room_data *rm;
    struct char_data *k;
    struct obj_data *j;
    struct extra_descr_data *desc;
    char page_buffer[MAX_STR_LEN];
    char buf[MAX_LINE_LEN], buf2[MAX_NAME_LEN];
    int i;

    static char *Dirs[] = /* NOTE: Capitalized  'dirs[]' */
	{ "North.", "East.", "South.", "West.", "Up.", "Down.", };

    /* for rooms */
    extern char *sector_types[];
    extern char *room_bits[];
    extern char *exit_bits[];
    extern char *item_types[];

    rm = &world[rm_nr]; 
    sprintf(page_buffer, "Room: %s, Zone: %d. V-Num: [%d], R-num: %d\n\r",
	    rm->name, rm->zone, rm->number, rm_nr );

    sprinttype(rm->sector_type, sector_types, buf2);
    sprintf(buf, "Sector type : %-15s Light: %d     Special-proc : %s\r\n",
	    buf2, rm->light, (rm->funct ? "Exists" : "No"));
    strcat(page_buffer, buf);

    sprintbit((long) rm->room_flags, room_bits, buf2);
    sprintf(buf, "Room flags: %s\r\n", buf2);
    strcat(page_buffer, buf);

    strcat(page_buffer, "Description:\n\r");
    strcat(page_buffer, rm->description);
    if (rm->ex_description) {
	strcpy(buf, "Extra descr. keywords: ");
	for (desc = rm->ex_description; desc; desc = desc->next) {
	    sprintf(buf2, "  %s", desc->keyword);
	    strcat(buf, buf2 );
	}
	strcat(buf, "\n\r");
	strcat(page_buffer, buf);
    }

    strcat(page_buffer, "\r\nChars present:\n\r");
    for (k = rm->people; k; k = k->next_in_room) {
	if (!CAN_SEE(ch, k))
	    continue;
	/* NOTE: Show class and level, too */
	sprintf(buf, "    < %c %d > %s %s\r\n",  
		"UMCTWU"[GET_CLASS(k)], GET_LEVEL(k), 
		(!IS_NPC(k) ? "(PC)" : (!IS_MOB(k) ? "(NPC)" : "(MOB)")),
		GET_NAME(k) );
	strcat(page_buffer, buf);
    }

    strcat(page_buffer, "Contents:\n\r");
    for (j = rm->contents; j; j = j->next_content) {
	sprinttype(GET_ITEM_TYPE(j), item_types, buf2);
	sprintf(buf, "    %s (%s)\r\n",  j->name, buf2 );
	strcat(page_buffer, buf);
    }

    strcat(page_buffer, "Exits:\n\r");
    for (i = 0; i <= 5; i++) {
	if (rm->dir_option[i]) {
	    sprintf(buf, "    Direction %-6s   To room : %5d  Keywords : %s\n\r",
		/* NOTE: Show virtual room number. */	
		Dirs[i], world[rm->dir_option[i]->to_room].number,
		/* NOTE: Some door has no keyword */
		STR_NONE(rm->dir_option[i]->keyword));
	    strcat(page_buffer, buf); 

	    if( rm->dir_option[i]->general_description ) {
		sprintf(buf, "      Descr.: %s\r\n",
		    rm->dir_option[i]->general_description); 
		strcat(page_buffer, buf); 
	    }

	    sprintbit(rm->dir_option[i]->exit_info, exit_bits, buf2);
	    if( rm->dir_option[i]->key > 0 ) 
		sprintf(buf, "      Exit flag: %s  KEY #: %d\r\n", 
		    buf2, rm->dir_option[i]->key ); 
	    else
		sprintf(buf, "      Exit flag: %s \r\n", buf2 );
	    strcat(page_buffer, buf);
	}
    }

    page_string(ch->desc, page_buffer, 1);
    return;
}		/* end of stat room */ 

/* NOTE: Show action desc, obj_flags.gpd, obj_flags.bitvector, too */
void stat_object(struct char_data *ch, struct obj_data *obj)
{
    struct extra_descr_data *desc;
    char buf[MAX_LINE_LEN], buf2[MAX_NAME_LEN];
    char page_buffer[MAX_STR_LEN];
    int virtual, i; 

    /* for objects */
    extern char *item_types[];
    extern char *extra_bits[];
    extern char *wear_bits[]; 
    extern char *apply_types[];
    extern char *affected_bits[];

    virtual = (obj->item_number >= 0) ?  GET_OBJ_VIRTUAL(obj) : 0;
    sprintf(page_buffer, "Object name: [%s]    Short desc: %s\n\r",
	    obj->name, STR_NONE(obj->short_description)) ;

    sprintf(buf, "Long desc: %s\n\r", STR_NONE(obj->description) );
    strcat(page_buffer, buf);

    sprinttype(GET_ITEM_TYPE(obj), item_types, buf2);
    sprintf( buf, "Item type: %s     V-number: [%d]    R-num %d\r\n",
	    buf2, obj->item_number, virtual); 
    strcat(page_buffer, buf);

    sprintbit(obj->obj_flags.wear_flags, wear_bits, buf2);
    sprintf(buf, "Can be worn on : %s\r\n", buf2 );
    strcat(page_buffer, buf ); 

    if (obj->action_description) {
	sprintf(buf, "Action desc: %s\r\n", obj->action_description);
	strcat(page_buffer, buf);
    }

    if (obj->ex_description) {
	strcpy(buf, "Extra desc keyword(s): ");
	for (desc = obj->ex_description; desc; desc = desc->next) {
	    sprintf(buf2, "  %s", desc->keyword);
	    strcat(buf, buf2);
	}
	strcpy(buf, "\n\r");
	strcat(page_buffer, buf);
    }

    sprintbit(obj->obj_flags.extra_flags, extra_bits, buf2 );
    sprintf(buf, "Extra flags: %s\r\n", buf2);
    strcat(page_buffer, buf);

    sprintf(buf, "Values 0-3 : [%d] [%d] [%d] [%d] \r\n",
	    obj->obj_flags.value[0], obj->obj_flags.value[1],
	    obj->obj_flags.value[2], obj->obj_flags.value[3] );
    strcat(page_buffer, buf); 

    sprintf(buf, "Weight: %d, Value: %d, Timer: %d,  GPD: %d\n\r",
	    obj->obj_flags.weight, obj->obj_flags.cost, obj->obj_flags.timer,
	    obj->obj_flags.gpd );
    strcat(page_buffer, buf);

    sprintbit(obj->obj_flags.bitvector, affected_bits, buf2);
    sprintf(buf, "Can affect char : %s\n\r", buf2);
    strcat(page_buffer, buf);

    for (i = 0; i < MAX_OBJ_AFFECT; ++i) {
	sprinttype(obj->affected[i].location, apply_types, buf2);
	sprintf(buf, "   Affects: %-10s by %d\n\r",
		buf2, obj->affected[i].modifier);
	strcat(page_buffer, buf);
    }

    page_string(ch->desc, page_buffer, 1);
} 	/* end of stat obj */

void stat_char(struct char_data *ch, struct char_data *kch)
{
    struct follow_type *fol;
    struct affected_type *aff;
    struct obj_data *j;
    char buf[MAX_LINE_LEN], buf2[MAX_NAME_LEN];
    char page_buffer[MAX_STR_LEN];
    int i,i1, i2, t;

    /* for chars */
    extern char *spells[];
    extern char *affected_bits[];
    extern char *apply_types[];
    extern char *pc_class_types[];
    extern char *connected_types[];
    extern char *action_bits[];
    extern char *player_bits[];
    extern int hit_gain(struct char_data *ch);
    extern int mana_gain(struct char_data *ch);
    extern int move_gain(struct char_data *ch);

    sprintf(page_buffer, "Name: %s     ", GET_NAME(kch)); 
    if (IS_NPC(kch)) {
	sprintf(buf, "Short desc: %s\r\n", STR_NONE(kch->player.short_descr)); 
	strcat(page_buffer, buf );

	strcat(page_buffer, "Long desc: ");
	strcat(page_buffer, STR_NONE(kch->player.long_descr));

	strcpy(buf2, (kch->nr >=0 && (mob_index[kch->nr].func )) ?
			"    Special: Exists" : "" ); 
	sprintf(buf, "V-Number [%d] R-Number %d %s\r\n",
	    GET_MOB_VIRTUAL(kch), kch->nr, buf2 ); 
    }
    else 
	sprintf( buf, "Title: %s\r\n", STR_NONE(GET_TITLE(kch))); 
    strcat(page_buffer, buf);

    sprinttype(kch->player.class, pc_class_types, buf2);
    sprintf(buf, "(%s)  Class: %s   Level: %d   Sex: %c \r\n",
	(!IS_NPC(kch) ? "PC" : (!IS_MOB(kch) ? "NPC" : "MOB")),
	buf2, GET_LEVEL(kch), "NMFU"[kch->player.sex]);
    if (!IS_NPC(kch)) {
	sprintf(buf2, "    Remortal : [%c%c%c%c]\r\n",
	    ((kch->player.remortal & REMORTAL_MAGIC_USER) ? 'M' : ' ' ),
	    ((kch->player.remortal & REMORTAL_CLERIC) ? 'C' : ' ' ), 
	    ((kch->player.remortal & REMORTAL_THIEF) ? 'T' : ' ' ),
	    ((kch->player.remortal & REMORTAL_WARRIOR) ? 'W' : ' ' ));
	strcpy(buf+strlen(buf)-2, buf2);   /* NOTE: Append */
    }
    strcat(page_buffer, buf); 

    sprintf(buf, "Str:[%d/%d]  Int:[%d]  Wis:[%d]  Dex:[%d]  Con:[%d]\n\r",
	    GET_STR(kch), GET_ADD(kch), GET_INT(kch),
	    GET_WIS(kch), GET_DEX(kch), GET_CON(kch));
    strcat(page_buffer, buf);

    sprintf(buf, "Hits: %d/%d+%d, Mana: %d/%d+%d, Moves: %d/%d+%d\r\n",
	    GET_HIT(kch), hit_limit(kch), hit_gain(kch),
	    GET_MANA(kch), mana_limit(kch), mana_gain(kch),
	    GET_MOVE(kch), move_limit(kch), move_gain(kch));
    strcat(page_buffer, buf);

    sprintf(buf, "AC: %d, Hitroll: %d, Damroll: %d,   Hand Dice: %dd%d\r\n",
	    GET_AC(kch), kch->points.hitroll, kch->points.damroll,
	    kch->specials.damnodice, kch->specials.damsizedice );
    strcat(page_buffer, buf);

    sprintf(buf, "Gold: %lld, Bank: %lld, Exp: %lld  \r\n",
	    GET_GOLD(kch), kch->bank, GET_EXP(kch)); 

    sprintf(buf, "Alignment[%d]   Regen : %d \r\n",
	    GET_ALIGNMENT(kch), kch->regeneration );
    /* NOTE: Show guild, quest, too */
    if (!IS_NPC(kch)) {
	sprintf(buf2, "    Practice: %d    Guild %s   quest #: %d\r\n",
	kch->specials.spells_to_learn, guild_names[GET_GUILD(kch)],
	kch->quest.solved );
	strcpy(buf+strlen(buf)-2, buf2); /* NOTE: Append */
    } 
    strcat(page_buffer, buf);

    for (i1 = 0, j = kch->carrying; j; j = j->next_content, i1++) ;
    for (i = 0, i2 = 0; i < MAX_WEAR; i++)
	if (kch->equipment[i]) i2++;

    sprintf(buf, "Carried weight: %d,  Carried items: %d " 
		"/ Items in inv: %d, Items in equ: %d\n\r", 
	    GET_CARRYING_W(kch), GET_CARRYING_N(kch), i1, i2 );
    strcat(page_buffer, buf);

    if (IS_NPC(kch)) {
	sprintbit(kch->specials.act, action_bits, buf2);
	sprintf(buf, "NPC flags: %s\r\n", buf2);
    }
    else {
	sprintbit(kch->specials.act, player_bits, buf2);
	sprintf(buf, "PC flags: %s\r\n", buf2);
    }
    strcat(page_buffer, buf); 

    sprintf(buf, "\r\nAge: %d Y, %d M, %d D, %d H.  Ht: %d cm, Wt: %d lbs\n\r",
	    age(kch).year, age(kch).month, age(kch).day, age(kch).hours,
	    kch->player.height, kch->player.weight);
    strcat(page_buffer, buf); 

    t = kch->player.time.birth;
    strcpy(buf2, (char *) ctime((time_t *) & t));
    buf2[24] = '\0';

    t = kch->player.time.played;
    sprintf(buf, "Birth: %s   Played: %d days, %d:%02d\r\n", buf2,
		t / 86400, ( t % 86400 ) /3600 , (( t + 30) % 3600)/ 60); 
    strcat(page_buffer, buf);

    if (kch->desc) {
	sprinttype(kch->desc->connected, connected_types, buf2);
	sprintf(buf, "Connected: %s from %s (%d)    ",
	    buf2, kch->desc->host, kch->desc->descriptor);
	strcat(page_buffer, buf);
    } 
    if (!IS_NPC(kch)) { 
	t = kch->player.time.logon;
	strcpy( buf2, ctime((time_t *) & t));
	buf2[24] = '\0';
	sprintf( buf, "Logon: %s\r\n", buf2 );
	strcat(page_buffer, buf);

	sprintf(buf, "Thirst: %d, Hunger: %d, Drunk: %d\r\n",
		kch->specials.conditions[THIRST],
		kch->specials.conditions[FULL],
		kch->specials.conditions[DRUNK]);
	strcat(page_buffer, buf); 
    } 

    if ( kch->master) {
	sprintf(buf, "Master is '%s'     ", GET_NAME(kch->master) );
	strcat(page_buffer, buf );
    } 
    if( kch->followers) { 
	strcpy( buf , "Followers are:");
	for (fol = kch->followers; fol; fol = fol->next) {
	    sprintf(buf2, "  %s", GET_NAME(fol->follower));
	    strcat(buf, buf2);
	}
	strcat(buf, "\r\n");
    }
    
    /* Showing the bitvector */
    sprintbit(kch->specials.affected_by, affected_bits, buf2);
    sprintf( buf, "Affected by: %s\r\n", buf2);
    strcat(page_buffer, buf);

    if (kch->affected) {
	strcat(page_buffer, "Affecting Spells:\n\r");
	/* NOTE: spells index starts from 1, not zero.   */
	for (aff = kch->affected; aff; aff = aff->next) {
	    sprintbit(aff->bitvector, affected_bits, buf2);
	    sprintf(buf, "    %s: %-10s by %d, %d hrs, bits: %s\r\n",
		    spells[aff->type], apply_types[aff->location],
		    aff->modifier, aff->duration, buf2);
	    strcat(page_buffer, buf);
	}
    }
    page_string(ch->desc, page_buffer, 1);
}

/* NOTE: OLD command 'set' and do_set() are renamed as 'wizset' and 
	do_wizset(), respectively. */
void do_wizset(struct char_data *ch, char *argument, int cmd)
{
    struct char_data *victim;
    /* NOTE: mess length extended to accomodate longer list */
    char mess[MAX_STRING_LENGTH] ; 
    char buf[64], buf2[32], buf3[32], buf4[MAX_INPUT_LENGTH];
    int i =0, j=0, k=0;
    struct obj_data *tar_obj;
    /* NOTE: logging level lable */
    static char *loglable[] = { 
    		"no(0)", "chat/shout cmd(1)", "chat/say cmd(2)", "all cmd(3)",
		"all+extr/stash(4)", "chat+debug(5)", "?(6)" };

    extern int nokillflag, nostealflag, nonewplayers, noshoutflag, nochatflag;
    extern int nodisarmflag;	/* chase written */
    extern int noplayingflag, noenchantflag, loglevel ; /* NOTE: NEW! */
    extern u_long reboot_time;
    extern int regen_time_percent;
    extern int regen_time;
    extern int regen_percent;

    if (IS_NPC(ch))
	return;
    if (GET_LEVEL(ch) < (IMO + 3))
	return;
    half_chop(argument, buf, buf2);
    if (!*buf) {
	/* NOTE: Added noenchant, noplaying, loglevel flag.	*/ 
	/* NOTE: Removed baddomain, use 'wizlock' instead. 	*/
	/* NOTE: Little prettier printing */
	sprintf(mess,
	  "nokill    = %d   nosteal = %d   nodisarm = %d   noenchant = %d\r\n"
	  "noplaying = %d   freeze  = %d   noshout  = %d  nochat  = %d\r\n"
	  "loglevel = %s     reboot_time = %ld (%.1f hr).\r\n"
	  "regen_percent = %d   regen_time_percent = %d   regen_time = %d\r\n",
	    nokillflag, nostealflag, nodisarmflag, noenchantflag,
	    noplayingflag, nonewplayers, noshoutflag, nochatflag,
	    loglable[loglevel], reboot_time, ((float) reboot_time / 3600.0 ),
	    regen_percent, regen_time_percent, regen_time );
	/* chase added nodisarm */
	send_to_char(mess, ch);
	return;
    }

    if (strcmp(buf, "nokill") == 0) {
	one_argument(buf2, buf3);
	if (*buf3)
	    nokillflag = atoi(buf3);
	sprintf(mess, "No-kill flag is %d.\n\r", nokillflag);
	send_to_char(mess, ch);
	return;
    }
    /* NOTE: Removed baddomain, use 'wizlock' instead. 	*/
    /*
    if (strcmp(buf, "baddomain") == 0) {
	one_argument(buf2, buf3);
	if (*buf3)
	    strcpy(baddomain[0], buf3);
	sprintf(mess, "Bad Domain is %s.\n\n", baddomain[0]);
	send_to_char(mess, ch);
	return;
    }
    */
    if (strcmp(buf, "nosteal") == 0) {
	one_argument(buf2, buf3);
	if (*buf3)
	    nostealflag = atoi(buf3);
	sprintf(mess, "No-steal flag is %d.\n\r", nostealflag);
	send_to_char(mess, ch);
	return;
    }
    if (strcmp(buf, "nodisarm") == 0) {		/* Chase written */
	one_argument(buf2, buf3);
	if (*buf3)
	    nodisarmflag = atoi(buf3);
	sprintf(mess, "No-disarm flag is %d.\n\r", nodisarmflag);
	send_to_char(mess, ch);
	return;
    }
    if (strcmp(buf, "freeze") == 0) {
	one_argument(buf2, buf3);
	if (*buf3)
	    nonewplayers = atoi(buf3);
	sprintf(mess, "Freeze flag is %d.\n\r", nonewplayers);
	send_to_char(mess, ch);
	return;
    }
    if (strcmp(buf, "noshout") == 0) {
	one_argument(buf2, buf3);
	if (*buf3)
	    noshoutflag = atoi(buf3);
	sprintf(mess, "NoShout flag is %d.\n\r", noshoutflag);
	send_to_char(mess, ch);
	return;
    }
    if (strcmp(buf, "nochat") == 0) {
	one_argument(buf2, buf3);
	if (*buf3)
	    nochatflag = atoi(buf3);
	sprintf(mess, "Nochat flag is %d.\n\r", nochatflag);
	send_to_char(mess, ch);
	return;
    }
    if (strcmp(buf, "regen_percent") == 0) {
	one_argument(buf2, buf3);
	if (*buf3)
	    regen_percent = atoi(buf3);
	sprintf(mess, "Regeneration Percentage is %d.\n\n", regen_percent);
	send_to_char(mess, ch);
	return;
    }
    if (strcmp(buf, "regen_time") == 0) {
	one_argument(buf2, buf3);
	if (*buf3)
	    regen_time = atoi(buf3);
	sprintf(mess, "Equipment-Regeneration-Time is %d.\r\n", regen_time);
	send_to_char(mess, ch);
	return;
    }
    if (strcmp(buf, "regen_time_percent") == 0) {
	one_argument(buf2, buf3);
	if (*buf3)
	    regen_time_percent = atoi(buf3);
	sprintf(mess, "Regeneration Time is %d / 100 of original.\r\n",
		regen_time_percent);
	send_to_char(mess, ch);
	return;
    }
    if (strcmp(buf, "reboot_time") == 0) {
	one_argument(buf2, buf3);
	if (*buf3)
	    reboot_time = atoi(buf3);

	sprintf(mess, "Reboot Time is %ld seconds (%.2f hours) after current"
	    " boot time.\r\n", reboot_time, (((float) reboot_time) / 3600.0));
	send_to_char(mess, ch);
	return;
    }
    /* NOTE: Add setting command log level */
    /*	    "no(0)", "chat/shout(1)", "chat/say(2)", "all(3)" */
    if (strcmp(buf, "loglevel") == 0) { 
	one_argument(buf2, buf3);
	if (*buf3)
	    loglevel = MIN(MAX( atoi(buf3), 0 ), 5) ;
	sprintf(mess, "Logging %s.\r\n", loglable[loglevel] );
	send_to_char(mess, ch);
	return;
    }
    /* NOTE: Prohibiting/permitting enchanting weapon/armor/person  */
    if (strcmp(buf, "noenchant") == 0) {
	one_argument(buf2, buf3);
	if (*buf3)
	    noenchantflag = atoi(buf3);
	sprintf(mess, "No-Enchant w/a/p flag is %d.\r\n", noenchantflag);
	send_to_char(mess, ch);
	return;
    }
    /* NOTE: Prohibiting/permitting entering game to play  */
    if (strcmp(buf, "noplaying") == 0) {
	one_argument(buf2, buf3);
	if (*buf3)
	    noenchantflag = atoi(buf3);
	sprintf(mess, "No-Playing flag is %d.\r\n", noplayingflag);
	send_to_char(mess, ch);
	return;
    }

    /* NOTE: just exchange of source position: no effect */
    if ((victim = get_char(buf))) {	/* NOTE: if arg is char name */

	half_chop(buf2, buf3, buf4);
	k = atoi(buf4); 
	LONGLONG kk = atoll(buf4);

	if (((GET_LEVEL(ch) < (IMO + 2)) && (strcmp(buf3, "gold")))
	    || (k == 0 && strcmp("class", buf3) && strcmp("remortal", buf3)))
	    return;
	
	sprintf(mess, "Setting %s's %s to %s Ok.\n\r", buf, buf3, buf4);

	if (strcmp(buf3, "exp") == 0 && (GET_LEVEL(ch) > (IMO + 2)))
	    victim->points.exp = kk;
	else if (strcmp("skill", buf3) == 0) {
	    /* NOTE: keyword 'skill' takes 3 more arguments.
		Set both learned value and skilled value of specifed skill.
		wizset <char> skill <learned> <skilled> <skill#>  */
	    i=j=k=-1;
	    sscanf(buf4, " %d %d %d ", &k, &j, &i );

	    if ( i < 0 ) {	// 4th arg, set all skills
		if ( k > 0 )	// 2nd
		    for (i = 0; i < MAX_SKILLS; i++)
			victim->skills[i].learned = k;
		if ( j > 0 )	// 3rd
		    for (i = 0; i < MAX_SKILLS; i++)
			victim->skills[i].skilled = j;
	    } 
	    else if ( i > 0 && i <= MAX_SKILLS ) {
		victim->skills[i].learned = k;
		victim->skills[i].skilled = j;
	    }
	    else
		return;
	}
	else if (strcmp("recognise", buf3) == 0) {
	    for (i = 0; i < MAX_SKILLS; i++)
		victim->skills[i].recognise = k;
	}
	else if (strcmp("lev", buf3) == 0)
	    GET_LEVEL(victim) = k;
	else if (strcmp("hit", buf3) == 0)
	    victim->points.hit = victim->points.max_hit = k;
	/* NOTE: Don't ever reset birth. It is player unique ID. */
	/*
	else if (strcmp("age", buf3) == 0) {
	    victim->player.time.birth = time(0);
	    victim->player.time.played = 0;
	    victim->player.time.logon = time(0);
	}
	*/
	else if (strcmp("mana", buf3) == 0)
	    victim->points.mana = victim->points.max_mana = k;
	else if (strcmp("move", buf3) == 0)
	    victim->points.move = victim->points.max_move = k;
	else if (strcmp("bank", buf3) == 0)
	    victim->bank = kk;
	else if (strcmp("gold", buf3) == 0)
	    victim->points.gold = kk;
	else if (strcmp("align", buf3) == 0)
	    victim->specials.alignment = k;
	/* NOTE: keyword 'str' and 'str_add' merged to single 'str' which
	    will take 2 more args str value and str_add value.
	    EX) wizset <char> str <str> <str_add>  */ 
	else if (strcmp("str", buf3) == 0) { 
	    k = 0;
	    sscanf(buf4, " %d %d ", &j, &k );
	    victim->abilities.str = j;
	    if ( j < 18) 
		k = 0;
	    victim->abilities.str_add = k;
	}
	else if (strcmp("dex", buf3) == 0)
	    victim->abilities.dex = k;
	else if (strcmp("wis", buf3) == 0)
	    victim->abilities.wis = k;
	else if (strcmp("con", buf3) == 0)
	    victim->abilities.con = k;
	else if (strcmp("int", buf3) == 0)
	    victim->abilities.intel = k;
	else if (strcmp("prac", buf3) == 0)
	    victim->specials.spells_to_learn = k;
	/* NOTE: 'hungry', 'thirst' and 'drunk' keywords are merged to 
	    single 'cond' which will take 3 more args for 3 conditions.
	    Drunk is first.
	    EX) wizset <char> cond <drunk> <hungry> <thirst>   */ 
	else if (strcmp("cond", buf3) == 0) {
	    sscanf(buf4, " %d %d %d ", &i, &j, &k );
	    victim->specials.conditions[DRUNK] = i;
	    victim->specials.conditions[FULL] = j;
	    victim->specials.conditions[THIRST] = k;
	}
	else if (strcmp("dr", buf3) == 0)
	    victim->points.damroll = k;
	else if (strcmp("hr", buf3) == 0)
	    victim->points.hitroll = k;
	/* NOTE: set player's regen percent. */
	else if (strcmp("regen", buf3) == 0)
	    victim->regeneration = k;
	/* NOTE: 'handno' and 'handsize' keywords are merged as 'hand' */
	/*	It will take 2 more args: handno and handsize   */
	else if (strcmp("hand", buf3) == 0) {
	    sscanf(buf4, " %d %d ", &j, &k );
	    victim->specials.damnodice = j; 
	    victim->specials.damsizedice = k;
	}
	else if (strcmp("quest", buf3) == 0)
	    victim->quest.solved = k;
	else if (strcmp("sex", buf3) == 0) {
	    if (strncmp("m", buf4, 1) == 0 || strncmp("M", buf4, 1) == 0)
		victim->player.sex = 1;
	    else if (strncmp("f", buf4, 1) == 0 || strncmp("F", buf4, 1) == 0)
		victim->player.sex = 2;
	    else
		send_to_char("Invalid sex\n\r", ch);
	}
	else if (strcmp("ac", buf3) == 0)
	    victim->points.armor = k;
	else if (strcmp("remortal", buf3) == 0) {
	    if (strncmp("w", buf4, 1) == 0 || strncmp("W", buf4, 1) == 0) {
		victim->player.remortal |= REMORTAL_WARRIOR;
		strcpy(mess, "WARRIOR. Done.\n\r");
	    }
	    else if (strncmp("c", buf4, 1) == 0 || strncmp("C", buf4, 1) == 0) {
		victim->player.remortal |= REMORTAL_CLERIC;
		strcpy(mess, "CLERIC. Done.\n\r");
	    }
	    else if (strncmp("m", buf4, 1) == 0 || strncmp("M", buf4, 1) == 0) {
		victim->player.remortal |= REMORTAL_MAGIC_USER;
		strcpy(mess, "MAGIC USER. Done.\n\r");
	    }
	    else if (strncmp("t", buf4, 1) == 0 || strncmp("T", buf4, 1) == 0) {
		victim->player.remortal |= REMORTAL_THIEF;
		strcpy(mess, "THIEF. Done.\n\r");
	    }
	    else {
		victim->player.remortal = 0;
		strcpy(mess, "Invalid class. remortal is resetted.\n\r");
	    }
	}
	else if (strcmp("class", buf3) == 0) {
	    if (strncmp("w", buf4, 1) == 0 || strncmp("W", buf4, 1) == 0)
		victim->player.class = 4;
	    else if (strncmp("c", buf4, 1) == 0 || strncmp("C", buf4, 1) == 0)
		victim->player.class = 2;
	    else if (strncmp("m", buf4, 1) == 0 || strncmp("M", buf4, 1) == 0)
		victim->player.class = 1;
	    else if (strncmp("t", buf4, 1) == 0 || strncmp("T", buf4, 1) == 0)
		victim->player.class = 3;
	    else
		strcpy(mess, "Invalid class\n\r");
	}

/* NOTE: It's meaningless to change passworod of active player */
#ifdef NO_DEF
	else if (strcmp("passwd", buf3) == 0) {
	    /* NOTE: password should be encryted   */
	    if (strlen(buf4) > 0) {
		strncpy(victim->desc->pwd,
			(char *) crypt(buf4, victim->player.name), 10);
		victim->desc->pwd[10] = '\0';
		save_char(victim);
/* NOTE: should save victim, not ch */
	    }
	}
#endif				/* NO_DEF */

	else {
	    /* NOTE: show help when failed */
	    strcpy(mess, "set CHAR keywords:\r\n"
		"\tclass remortal str wis int dex con sex\r\n"
		"\tlev hit mana move ac dr hr hand regen align\r\n"
		"\texp gold bank prac skill recognise quest cond.\r\n");

	}
	victim->tmpabilities = victim->abilities;
	send_to_char(mess, ch);
    }
    else {		/* NOTE: if arg is obj name? */ 
	
	tar_obj = get_obj_in_list_vis(ch, buf, ch->carrying);
	if(!tar_obj) 
	    tar_obj = get_obj_in_list_vis(
			ch, buf, world[ch->in_room].contents);
	if(!tar_obj)
	    tar_obj = get_obj_vis(ch, buf);
	if(!tar_obj)
	    for (i = 0; i < MAX_WEAR && !tar_obj; i++)
		if (ch->equipment[i] 
		&& str_cmp(buf, ch->equipment[i]->name) == 0) 
		    tar_obj = ch->equipment[i];
	if (!tar_obj) {
	    send_to_char("No-object by that name in the world.\n\r", ch);
	    return;
	}

	half_chop(buf2, buf3, buf4);
	k = atoi(buf4);
	if (GET_LEVEL(ch) < (IMO + 3))
	    return;
	if (strcmp(buf3, "value1") == 0)
	    tar_obj->obj_flags.value[0] = k;
	else if (strcmp(buf3, "value2") == 0)
	    tar_obj->obj_flags.value[1] = k;
	else if (strcmp(buf3, "value3") == 0)
	    tar_obj->obj_flags.value[2] = k;
	else if (strcmp(buf3, "value4") == 0)
	    tar_obj->obj_flags.value[3] = k;
	else if (strcmp(buf3, "weight") == 0)
	    tar_obj->obj_flags.weight = k;
	else if (strcmp(buf3, "type") == 0)
	    tar_obj->obj_flags.type_flag = k;
	else if (strcmp(buf3, "extra") == 0)
	    tar_obj->obj_flags.extra_flags = k;
	else if (strcmp(buf3, "wear") == 0)
	    tar_obj->obj_flags.wear_flags = k;
	else if (strcmp(buf3, "cost") == 0)
	    tar_obj->obj_flags.cost = k;
	else if (strcmp(buf3, "magic") == 0)
	    tar_obj->obj_flags.gpd = k;
	else if (strcmp(buf3, "affect1") == 0) {
	    /*
	    half_chop(buf4, buf5, buf6); 
	    k = atoi(buf5);
	    j = atoi(buf6);
	    */
	    sscanf(buf4, " %d %d ", &k, &j );
	    tar_obj->affected[0].location = k;
	    tar_obj->affected[0].modifier = j;
	}
	else if (strcmp(buf3, "affect2") == 0) {
	    /*
	    half_chop(buf4, buf5, buf6);
	    k = atoi(buf5);
	    j = atoi(buf6);
	    */
	    sscanf(buf4, " %d %d ", &k, &j );
	    tar_obj->affected[1].location = k;
	    tar_obj->affected[1].modifier = j;
	}
	else {
	    send_to_char("Huh?\n\r", ch);
	}
    }
} 

void do_force(struct char_data *ch, char *argument, int cmd)
{
    struct descriptor_data *i;
    struct char_data *vict;
    char name[100], to_force[100], buf[MAX_LINE_LEN];
    int diff;

    if (IS_NPC(ch) || GET_LEVEL(ch) > (IMO + 3))
	return;

    half_chop(argument, name, to_force);

    if (!*name || !*to_force)
	send_to_char("Who do you wish to force to do what?\n\r", ch);
    else if (str_cmp("all", name)) {
	if (!(vict = get_char_vis(ch, name)))
	    send_to_char("No-one by that name here..\n\r", ch);
	else {
	    diff = GET_LEVEL(ch) - GET_LEVEL(vict);
	    if ((!IS_NPC(vict)) && (diff <= 0)) {
		send_to_char("Oh no you don't!!\n\r", ch);
	    }
	    else {
		if (GET_LEVEL(ch) < (IMO + 3))
		    sprintf(buf, "$n has forced you to '%s'.", to_force);
		else
		    buf[0] = 0;
		act(buf, FALSE, ch, 0, vict, TO_VICT);
		send_to_char("Ok.\n\r", ch);
		command_interpreter(vict, to_force);
	    }
	}
    }
    else {		/* force all */
	if (GET_LEVEL(ch) < (IMO + 3)) {
	    send_to_char("Force all's are a bad idea these days.\n\r", ch);
	    return;
	}
	for (i = descriptor_list; i; i = i->next)
	    if (i->character != ch && !i->connected) {
		vict = i->character;
		command_interpreter(vict, to_force);
	    }
	send_to_char("Ok.\n\r", ch);
    }
}

void do_trans(struct char_data *ch, char *argument, int cmd)
{
    struct descriptor_data *i;
    struct char_data *victim;
    char buf[100];
    int target;

    if (IS_NPC(ch))
	return;

    one_argument(argument, buf);
    if (!*buf)
	send_to_char("Who do you wish to transfer?\n\r", ch);
    else if (str_cmp("all", buf)) {
	if (!(victim = get_char_vis(ch, buf)))
	    send_to_char("No-one by that name around.\n\r", ch);
	else {
	    if (GET_LEVEL(ch) < IMO + 3 && GET_LEVEL(victim) > GET_LEVEL(ch)) {
		send_to_char("That might not be appreciated.\n\r", ch);
		return;
	    }
	    act("$n disappears in a mushroom cloud.", 
		FALSE, victim, 0, 0, TO_ROOM);
	    target = ch->in_room;
	    char_from_room(victim);
	    char_to_room(victim, target);
	    act("$n arrives from a puff of smoke.",
		FALSE, victim, 0, 0, TO_ROOM);
	    act("$n has transferred you!", FALSE, ch, 0, victim, TO_VICT);
	    do_look(victim, "", 15);
	    send_to_char("Ok.\n\r", ch);
	}
    }
    else {		/* Trans All */
	if (ch->player.level >= (IMO + 3))
	    for (i = descriptor_list; i; i = i->next)
		if (i->character != ch && !i->connected) {
		    target = ch->in_room;
		    victim = i->character;
		    char_from_room(victim);
		    char_to_room(victim, target);
		    act("$n arrives from a puff of smoke.",
			FALSE, victim, 0, 0, TO_ROOM);
		    act("$n has transferred you!",
			FALSE, ch, 0, victim, TO_VICT);
		    do_look(victim, "", 15);
		}
	send_to_char("Ok.\n\r", ch);
    }
}

/* NOTE: NEW! find target room for 'goto' or 'at' command. */
int find_target_room(struct char_data *ch, char *loc_str )
{
    int location;
    struct obj_data *target_obj;
    struct char_data *target_mob; 
    extern int House_can_enter(struct char_data * ch, sh_int house);

    location = NOWHERE;
    if (!*loc_str) 
	send_to_char("You must supply a room number or a name.\n\r", ch);
    else if ( ISDIGIT(*loc_str)) {
	if ((location = real_room(atoi(loc_str))) < 0 )
	    send_to_char("No room exists with that number.\n\r", ch);
    }
    else if ((target_mob = get_char_vis(ch, loc_str)))
	location = target_mob->in_room;
    else if ((target_obj = get_obj_vis(ch, loc_str)))
	if (target_obj->in_room != NOWHERE)
	    location = target_obj->in_room;
	else 
	    send_to_char("The object is not available.\n\r", ch);
    else 
	send_to_char("No such creature or object around.\n\r", ch); 

    /* NOTE: OFF_LIMIT/HOUSE room is permited only to level 43 or up */
    if ( GET_LEVEL(ch) < IMO + 2 && location != NOWHERE ) {
	if ( IS_SET(world[location].room_flags, OFF_LIMITS)) {
	    send_to_char("Sorry, off limits.\n", ch);
	    return NOWHERE;
	}
	/* NOTE: for "house.c" */
	else if ( ROOM_FLAGGED(location, ROOM_HOUSE) 
		    &&  !House_can_enter(ch, location)) {
	    send_to_char("That's private property -- no trespassing!\r\n", ch);
	    return NOWHERE;
	}
    }
    return(location);
}


void do_at(struct char_data *ch, char *argument, int cmd)
{
    char command[MAX_INPUT_LENGTH], loc_str[MAX_INPUT_LENGTH];
    int location, original_loc;
    struct char_data *target_mob;

    if (IS_NPC(ch))
	return;

    half_chop(argument, loc_str, command);

#ifdef NO_DEF
    if (!*loc_str) {
	send_to_char("You must supply a room number or a name.\n\r", ch);
	return;
    }
    if (ISDIGIT(*loc_str)) {
	loc_nr = atoi(loc_str);
	for (location = 0; location <= top_of_world; location++)
	    if (world[location].number == loc_nr)
		break;
	    else if (location == top_of_world) {
		send_to_char("No room exists with that number.\n\r", ch);
		return;
	    }
    }
    else if (target_mob = get_char_vis(ch, loc_str))
	location = target_mob->in_room;
    else if (target_obj = get_obj_vis(ch, loc_str))
	if (target_obj->in_room != NOWHERE)
	    location = target_obj->in_room;
	else {
	    send_to_char("The object is not available.\n\r", ch);
	    return;
	}
    else {
	send_to_char("No such creature or object around.\n\r", ch);
	return;
    } 

    /* a location has been found. */

    if ((GET_LEVEL(ch) < (IMO + 3)) 
	    && (IS_SET(world[location].room_flags, OFF_LIMITS))) {
	send_to_char("That room is off-limits.\n", ch);
	return;
    }
#endif 		/* NO_DEF */
    if( (location = find_target_room(ch, loc_str)) < 0 )
	return;

    original_loc = ch->in_room;
    char_from_room(ch);
    char_to_room(ch, location);
    command_interpreter(ch, command);

    /* check if the guy's still there */
    for (target_mob = world[location].people; target_mob; target_mob =
	 target_mob->next_in_room)
	if (ch == target_mob) {
	    char_from_room(ch);
	    char_to_room(ch, original_loc);
	}
}

void do_goto(struct char_data *ch, char *argument, int cmd)
{
    char buf[MAX_INPUT_LENGTH];
    int location, i, flag; 
    struct char_data *pers;

    if (IS_NPC(ch) || GET_LEVEL(ch) > (IMO + 4))
	return;
    one_argument(argument, buf);

#ifdef	NO_DEF
    if (!*buf) {
	send_to_char("You must supply a room number or a name.\n\r", ch);
	return;
    }

    if (ISDIGIT(*buf)) {
	loc_nr = atoi(buf);
	for (location = 0; location <= top_of_world; location++)
	    if (world[location].number == loc_nr)
		break;
	    else if (location == top_of_world) {
		send_to_char("No room exists with that number.\n\r", ch);
		return;
	    }
    }
    else if (target_mob = get_char_vis(ch, buf))
	location = target_mob->in_room;
    else if (target_obj = get_obj_vis(ch, buf))
	if (target_obj->in_room != NOWHERE)
	    location = target_obj->in_room;
	else {
	    send_to_char("The object is not available.\n\r", ch);
	    return;
	}
    else {
	send_to_char("No such creature or object around.\n\r", ch);
	return;
    }
#endif		/* NO_DEF */
    /* a location has been found. */

    if( (location = find_target_room(ch, buf)) < 0 )
	return;

    if (GET_LEVEL(ch) < (IMO + 2)) {
	if (IS_SET(world[location].room_flags, PRIVATE)) {
	    for (i = 0, pers = world[location].people; pers; pers =
		 pers->next_in_room, i++) ;
	    if (i > 1) {
		send_to_char(
	    "There's a private conversation going on in that room.\n\r", ch);
		return;
	    }
	}
    }
    flag = ((GET_LEVEL(ch) >= (IMO + 2)) &&
	    IS_SET(ch->specials.act, PLR_WIZINVIS));
    if (!flag)
	act("$n disappears in a puff of smoke.", FALSE, ch, 0, 0, TO_ROOM);
    char_from_room(ch);
    char_to_room(ch, location);
    if (!flag)
	act("$n appears with an ear-splitting bang.", FALSE, ch, 0, 0, TO_ROOM);
    do_look(ch, "", 15);
}

void do_load(struct char_data *ch, char *argument, int cmd)
{
    struct char_data *mob;
    struct obj_data *obj;
    char type[100], num[100], buf[100];
    int number, r_num;

    if (IS_NPC(ch) || GET_LEVEL(ch) > (IMO + 4))
	return;

    half_chop(argument, type, num);

    if (!*type || !*num || !ISDIGIT(*num)) {
	send_to_char("Syntax:\n\rload <'char' | 'obj'> <number>.\n\r", ch);
	return;
    }

    if ((number = atoi(num)) < 0) {
	send_to_char("A NEGATIVE number??\n\r", ch);
	return;
    }
    if (is_abbrev(type, "char")) {
	if ((r_num = real_mobile(number)) < 0) {
	    send_to_char("There is no monster with that number.\n\r", ch);
	    return;
	}
	mob = read_mobile(r_num, REAL);
	char_to_room(mob, ch->in_room);
	act("$n makes a quaint, magical gesture with one hand.", TRUE, ch,
	    0, 0, TO_ROOM);
	act("$n has created $N!", FALSE, ch, 0, mob, TO_ROOM);
	send_to_char("Done.\n\r", ch);
	/* NOTE: log loaded object/character name */
	sprintf(buf, "%s loaded char %s (%d)",
	    GET_NAME(ch), mob->player.short_descr, number);
	log(buf);
    }
    else if (is_abbrev(type, "obj")) {
	if ((r_num = real_object(number)) < 0) {
	    send_to_char("There is no object with that number.\n\r", ch);
	    return;
	}
	obj = read_object(r_num, REAL);
	if (GET_LEVEL(ch) < (IMO + 2))
	    if (IS_SET(obj->obj_flags.extra_flags, ITEM_NOLOAD)) {
		send_to_char("That item is not loadable.\n\r", ch);
		extract_obj(obj);
		sprintf(buf, "%s tried to load %d", ch->player.name, number);
		log(buf);
		return;
	    }
	act("$n makes a strange magical gesture.", TRUE, ch, 0, 0, TO_ROOM);
	act("$n has created $p!", FALSE, ch, obj, 0, TO_ROOM);
	if (IS_SET(obj->obj_flags.wear_flags, ITEM_TAKE)) {
	    obj_to_char(obj, ch);
	    act("$n snares $p!\n\r", FALSE, ch, obj, 0, TO_ROOM);
	}
	else
	    obj_to_room(obj, ch->in_room);
	send_to_char("Ok.\n\r", ch);
	/* NOTE: log loaded object/character name */
	sprintf(buf, "%s loaded object %s (%d)",
		GET_NAME(ch), obj->name, number);
	log(buf);
    }
    else
	send_to_char("That'll have to be either 'char' or 'obj'.\n\r", ch);
}

/* clean a room of all mobiles and objects */
void do_purge(struct char_data *ch, char *argument, int cmd)
{
    struct char_data *vict, *next_v;
    struct obj_data *obj, *next_o;
    char name[100], buf[100];

    if (IS_NPC(ch) || GET_LEVEL(ch) > (IMO + 3))
	return;
    one_argument(argument, name);
    if (*name) {
	if ((vict = get_char_room_vis(ch, name))) {
	    if (!IS_NPC(vict)) {
		if (GET_LEVEL(ch) < GET_LEVEL(vict)) {
		    sprintf(buf, "%s tried to purge you.\n\r", ch->player.name);
		    send_to_char(buf, vict);
		    return;
		}
		/* NOTE: same effect, simpler code */
		save_char(vict);
		stash_char(vict);
		purge_player(vict);
		/* 
		for (i = 0; i < MAX_WEAR; i++)
		    if (vict->equipment[i]) {
			extract_obj(unequip_char(vict, i));
			vict->equipment[i] = 0;
		    }
		wipe_obj(vict->carrying);
		vict->carrying = 0;
		if (vict->desc)
		    close_socket(vict->desc);
		extract_char(vict);
		*/
		return;
	    }
	    act("$n disintegrates $N.", FALSE, ch, 0, vict, TO_NOTVICT);
	    if (IS_NPC(vict))
		extract_char(vict);
	}
	else if ((obj = get_obj_in_list_vis(
			    ch, name, world[ch->in_room].contents))) {
	    act("$n destroys $p.", FALSE, ch, obj, 0, TO_ROOM);
	    extract_obj(obj);
	}
	else {
	    send_to_char(
		"I don't know anyone or anything by that name.\n\r", ch);
	    return;
	}
	send_to_char("Ok.\n\r", ch);
    }
    else {		/* no argument. clean out the room */
	if (IS_NPC(ch)) {
	    send_to_char("Don't... You would only kill yourself..\n\r", ch);
	    return;
	}
	act("$n gestures... You are surrounded by scorching flames!",
	    FALSE, ch, 0, 0, TO_ROOM);
	send_to_room("The world seems a little cleaner.\n\r", ch->in_room);
	for (vict = world[ch->in_room].people; vict; vict = next_v) {
	    next_v = vict->next_in_room;
	    if (IS_NPC(vict))
		extract_char(vict);
	}
	for (obj = world[ch->in_room].contents; obj; obj = next_o) {
	    next_o = obj->next_content;
	    extract_obj(obj);
	}
    }
}

void do_flag(struct char_data *ch, char *argument, int cmd)
{
    struct char_data *vict;
    struct obj_data *dummy;
    char arg1[MAX_INPUT_LENGTH], arg2[MAX_INPUT_LENGTH];
    char log_buf[50];
    int key, st, flag;
    static char *msgs[][2] = { { "", "", },
	{ "SHUTUP removed. $N can shout or chat, again.",
	  "SHUTUP set. $N can not shout or chat." },
	{ "WIZARD_DUMB removed. You put back $S tongue.",
	  "WIZARD_DUMB set. You pull out and take away $S tongue." },
	{ "Logging removed on $N." , "Logging set on $N." },
	{ "You made $M visble." , "You made $M invisible." },
	{ "Criminal flag removed." , "Criminal flag set." },
    };
    static char *keywords[] = { "",
	"shutup", "dumb", "logging", "invisible", "criminal", "\n" };
    static unsigned long attrs[] = { 0, 
	PLR_SHUTUP, PLR_DUMB_BY_WIZ, PLR_XYZZY, PLR_WIZINVIS, PLR_CRIMINAL };

    if (IS_NPC(ch))
	return;
    half_chop(argument, arg1, arg2);

    if (!*arg1)
	send_to_char("Flag whom?\n\r", ch);
    else if (!*arg2 || ( key = search_block(arg2, keywords, FALSE)) <= 0 )
	send_to_char("Flag keywords: shutup dumb log invis criminal.\n\r", ch);
    else if (!generic_find(arg1, FIND_CHAR_WORLD, ch, &vict, &dummy))
	send_to_char("Couldn't find any such creature.\n\r", ch);
    else if (IS_NPC(vict))
	send_to_char("Can't do that to a beast.\n\r", ch);
    else if ((GET_LEVEL(vict) >= IMO) && 
	( attrs[key] == PLR_DUMB_BY_WIZ || attrs[key] == PLR_CRIMINAL ))
	send_to_char("It's pointless to flag an immortal.\n\r", ch);
    else if ((GET_LEVEL(vict) >= IMO) && GET_LEVEL(ch) < GET_LEVEL(vict))
	act("$E might object to that.. better not.", 0, ch, 0, vict, TO_CHAR);
    else {
	flag = attrs[key]; 
	st = IS_ACTPLR(vict, flag);
	if (st) {
	    REMOVE_BIT( GET_ACT(vict), flag);
	    if ( flag == PLR_XYZZY) {
		sprintf(log_buf, "Log for %s removed by %s", 
		    GET_NAME(vict), GET_NAME(ch));
		log(log_buf);
	    }
	    else if ( flag == PLR_SHUTUP )
		send_to_char("You can shout or chat again.\n\r", vict);
	}
	else {
	    SET_BIT(GET_ACT(vict), flag);
	    if ( flag == PLR_XYZZY) {
		sprintf(log_buf, "Log for %s set by %s",
		    GET_NAME(vict), GET_NAME(ch));
		log(log_buf);
	    }
	    else if ( flag == PLR_SHUTUP )
		send_to_char("The gods take away your ability to "
			     "shout or chat!\n\r", vict);
	}
	act( msgs[key][ IS_ACTPLR(vict, flag) ? 1:0 ], 0, ch, 0, vict, TO_CHAR);
    }
}

#ifdef   NO_DEF
/* NOTE: OLD 'noshout' command absorbed by 'set shout' command 
	and 'flag shutup' wizard command. */ 
void do_noshout(struct char_data *ch, char *argument, int cmd)
{
    struct char_data *vict;
    struct obj_data *dummy;
    char buf[MAX_INPUT_LENGTH];

    if (IS_NPC(ch))
	return;
    one_argument(argument, buf);
    if (!*buf) {
	if (IS_SET(ch->specials.act, PLR_EARMUFFS)) {
	    send_to_char("You can now hear shouts again.\n\r", ch);
	    REMOVE_BIT(ch->specials.act, PLR_EARMUFFS);
	}
	else {
	    send_to_char("From now on, you won't hear shouts.\n\r", ch);
	    SET_BIT(ch->specials.act, PLR_EARMUFFS);
	}
	return;
    }
    if (GET_LEVEL(ch) < IMO)
	return;
    if (!generic_find(argument, FIND_CHAR_WORLD, ch, &vict, &dummy))
	send_to_char("Couldn't find any such creature.\n\r", ch);
    else if (IS_NPC(vict))
	send_to_char("Can't do that to a beast.\n\r", ch);
    else if (GET_LEVEL(vict) > GET_LEVEL(ch))
	act("$E might object to that.. better not.", 0, ch, 0, vict, TO_CHAR);
    else if (IS_SET(vict->specials.act, PLR_NOSHOUT)) {
	send_to_char("You can shout again.\n\r", vict);
	send_to_char("NOSHOUT removed.\n\r", ch);
	REMOVE_BIT(vict->specials.act, PLR_NOSHOUT);
    }
    else {
	send_to_char("The gods take away your ability to shout!\n\r", vict);
	send_to_char("NOSHOUT set.\n\r", ch);
	SET_BIT(vict->specials.act, PLR_NOSHOUT);
    }
}
#endif		/* NO_DEF */

void do_invis(struct char_data *ch, char *argument, int cmd)
{
    if (IS_NPC(ch) || GET_LEVEL(ch) > (IMO + 4))
	return;
    if (IS_SET(ch->specials.act, PLR_WIZINVIS)) {
	REMOVE_BIT(ch->specials.act, PLR_WIZINVIS);
	send_to_char("You are visible again.\n\r", ch);
    }
    else {
	SET_BIT(ch->specials.act, PLR_WIZINVIS);
	send_to_char("You vanish.\n\r", ch);
    }
    send_to_char("Ok.\n\r", ch);
} 

void do_noaffect(struct char_data *ch, char *argument, int cmd)
{
    struct char_data *vict;
    struct obj_data *dummy;
    /* struct affected_type *hjp, *hjp_next; */
    char buf[MAX_INPUT_LENGTH];
    extern void affect_remove_all(struct char_data *ch);

    if (IS_NPC(ch))
	return;
    one_argument(argument, buf);
    if (!*buf) {
	send_to_char("Remove affects from whom?\n\r", ch);
	return;
    }
    else {
	if (!generic_find(argument, FIND_CHAR_WORLD, ch, &vict, &dummy))
	    send_to_char("Couldn't find any such creature.\n\r", ch);
	else if (IS_NPC(vict) && vict->in_room != ch->in_room)
	    send_to_char("Only can do that to a mob in same room.\n\r", ch);
	else if (!IS_NPC(vict) && GET_LEVEL(vict) > GET_LEVEL(ch))
	    act("$E might object to that.. better not.",
		0, ch, 0, vict, TO_CHAR);
	else {
	    send_to_char("You are normal again.\n\r", vict);
#ifdef 	NO_DEF
	    /* NOTE: hjp->next is invalid. See affect_remove() in handler.c */
	    for (hjp = vict->affected; hjp; hjp = hjp_next) {
		hjp_next = hjp->next;
		affect_remove(vict, hjp);
	    }
#endif	/* NO_DEF */
	    affect_remove_all(vict);
	}
    }
    send_to_char("Ok.\n\r", ch);
}

void do_restore(struct char_data *ch, char *argument, int cmd)
{
    struct char_data *victim;
    char buf[100];
    int i;


    if (IS_NPC(ch) || GET_LEVEL(ch) > (IMO + 3))
	return;
    one_argument(argument, buf);
    if (!*buf)
	send_to_char("Who do you wish to restore?\n\r", ch);
    else if (!(victim = get_char_vis(ch, buf)))
	send_to_char("No-one by that name in the world.\n\r", ch);
    else {
	GET_MANA(victim) = GET_PLAYER_MAX_MANA(victim);
	GET_HIT(victim) = GET_PLAYER_MAX_HIT(victim);
	GET_MOVE(victim) = GET_PLAYER_MAX_MOVE(victim);

	if (GET_LEVEL(victim) >= IMO) {
	    for (i = 0; i < MAX_SKILLS; i++) {
		victim->skills[i].learned = 100;
		victim->skills[i].recognise = 0;
	    }

	    if (GET_LEVEL(victim) >= (IMO + 3)) {
		victim->abilities.str_add = 100;
		victim->abilities.intel = 25;
		victim->abilities.wis = 25;
		victim->abilities.dex = 25;
		victim->abilities.str = 25;
		victim->abilities.con = 25;
	    }
	    victim->tmpabilities = victim->abilities;

	}
	update_pos(victim);
	send_to_char("Done.\n\r", ch);
	act("You have been fully healed by $N!", FALSE, victim, 0, ch, TO_CHAR);
    }
}

void do_reroll(struct char_data *ch, char *argument, int cmd)
{
    struct char_data *victim;
    char buf[100]; 
    extern void roll_abilities(struct char_data *ch);

    if (IS_NPC(ch) || GET_LEVEL(ch) > (IMO + 3))
	return;

    one_argument(argument, buf);
    if (!*buf)
	send_to_char("Who do you wish to reroll?\n\r", ch);
    else if (!(victim = get_char(buf)))
	send_to_char("No-one by that name in the world.\n\r", ch);
    else {
	roll_abilities(victim);
	sprintf(buf, "Rerolled: %d/%d %d %d %d %d\n", victim->abilities.str,
		victim->abilities.str_add,
		victim->abilities.intel,
		victim->abilities.wis,
		victim->abilities.dex,
		victim->abilities.con);
	send_to_char(buf, ch);
    }
}

void do_advance(struct char_data *ch, char *argument, int cmd)
{
    struct char_data *victim;
    char name[100], level[100];
    int i, newlevel; 
    extern void gain_exp_regardless(struct char_data *ch, int gain);

    if (IS_NPC(ch))
	return;

    if (GET_LEVEL(ch) < (IMO + 3)) {
	send_to_char("You can only do that in a guild.\n\r", ch);
	return;
    }

    half_chop(argument, name, level);
    if (*name) {
	if (!(victim = get_char_room_vis(ch, name))) {
	    send_to_char("That player is not here.\n\r", ch);
	    return;
	}
    }
    else {
	send_to_char("Advance who?\n\r", ch);
	return;
    }

    if (IS_NPC(victim)) {
	send_to_char("NO! Not on NPC's.\n\r", ch);
	return;
    }

    if (!*level) {
	send_to_char("You must supply a level number.\n\r", ch);
	return;
    }
    else {
	if (!ISDIGIT(*level)) {
	    send_to_char("Second argument must be a positive integer.\n\r", ch);
	    return;
	}
	if ((newlevel = atoi(level)) < GET_LEVEL(victim)) {
	    if (GET_LEVEL(ch) <= GET_LEVEL(victim)) {
		send_to_char("Very amusing.\n\r", ch);
		return;
	    }
	    victim->player.level = newlevel;
	    if (newlevel < IMO) {
		for (i = 0; i < 3; ++i)
		    victim->specials.conditions[i] = 0;
	    }
	    victim->tmpabilities = victim->abilities;
	    send_to_char("The poor soul...\n\r", ch);
	    send_to_char("You have been punished.\n\r", victim);
	    return;
	}
    }

    if (newlevel > IMO) {
	send_to_char("Too high a level.\n\r", ch);
	return;
    }
    send_to_char("You feel generous.\n\r", ch);

    if (GET_LEVEL(victim) == 0) {
	init_player(victim);
	do_start(victim);
    }
    else {
	victim->points.exp = 1;
	LONGLONG le = titles[GET_CLASS(victim) - 1][newlevel].exp;
	gain_exp_regardless(victim, le - GET_EXP(victim));
    }
}

void do_demote(struct char_data *ch, char *argument, int cmd)
{
    struct char_data *victim;
    struct char_ability_data tmp_abilities;
    struct time_data tmp_time;
    char name[100], buf[BUFSIZ];

    if (IS_NPC(ch))
	return;

    one_argument(argument, name);
    if (*name) {
	if (!(victim = get_char_room_vis(ch, name))) {
	    send_to_char("That player is not here.\n\r", ch);
	    return;
	}
    }
    else {
	send_to_char("Demote who?\n\r", ch);
	return;
    }
    if (IS_NPC(victim)) {
	send_to_char("NO! Not on NPC's.\n\r", ch);
	return;
    }
    if (GET_LEVEL(victim) > GET_LEVEL(ch)) {
	send_to_char("Impossible!\n\r", ch);
	return;
    }

    /* NOTE: log demote after checking all is O.K.  */
    sprintf(buf, "%s demoted %s", GET_NAME(ch), argument);
    log(buf);

    GET_AC(ch) += GET_LEVEL(ch);
    GET_HITROLL(ch) -= GET_LEVEL(ch);
    GET_DAMROLL(ch) -= GET_LEVEL(ch);

    /* NOTE: preserve str, wis, int, dex, con even on demote */
    /* NOTE: Also preserve birth time.	 */
    tmp_abilities = victim->abilities;
    tmp_time = victim->player.time;
    init_player(victim);
    victim->abilities = tmp_abilities;
    victim->player.time = tmp_time;

    do_start(victim);
}

/* NOTE: banish command take optional 2nd arg. to specify jail term hours  */
/* NOTE: term hours can be specified in factional number like 3.5   */
void do_banish(struct char_data *ch, char *argument, int cmd)
{
    struct char_data *vict;
    struct obj_data *dummy;
    char buf[MAX_INPUT_LENGTH], *next;
    float term;

    if (IS_NPC(ch))
	return;
    /* NOTE: new banish may have second args */
    next = one_argument(argument, buf);
    if (!*buf)
	send_to_char("Banish whom?\n\r", ch);
    else if (!generic_find(buf, FIND_CHAR_WORLD, ch, &vict, &dummy))
	send_to_char("Couldn't find any such creature.\n\r", ch);
    else if (IS_NPC(vict))
	send_to_char("Can't do that to a beast.\n\r", ch);
    else if (GET_LEVEL(vict) >= IMO)
	send_to_char("It's pointless to banish an immortal.\n\r", ch);
    else 
	goto OK;
    return;
    
OK: 
    if (IS_SET(vict->specials.act, PLR_BANISHED)) {
	REMOVE_BIT(vict->specials.act, PLR_BANISHED);
	send_to_char("You feel forgiven?\n\r", vict);
	act("$N is forgiven.", FALSE, ch, 0, vict, TO_CHAR);
	sprintf(buf, "%s forgives %s.", GET_NAME(ch), GET_NAME(vict));
	/*  NOTE: Don't add code for trans to MID  */
    }
    else {
	SET_BIT(vict->specials.act, PLR_BANISHED);

	/* NOTE: Use real_room() to find JAIL_ROOM */
	if (real_room(JAIL_ROOM) < 0 ) {
	    send_to_char("Death Room is gone?\n\r", ch);
	    return;
	}

	/* NOTE: 2nd arg is jail term in hours on real world clock */
	one_argument(next, buf);
	if (!*buf || (term = atof(buf)) < 0.02)
	    term = 24.0;	/* default 24 hours */
	vict->specials.jail_time 
	    = time(0) + (time_t) ((long) (term * 60.0) * 60);

	/* NOTE: Notify to me, victim, players in the room */
	sprintf(buf, "%s will be held in jail for %3.1f hours.\r\n",
		GET_NAME(vict), term);
	send_to_room_except(buf, vict->in_room, ch);
	sprintf(buf, "%s will be held in jail for %3.1f hours by %s.",
		GET_NAME(vict), term, GET_NAME(ch));
	send_to_char(buf, ch);
	send_to_char("\r\n", ch);

	act("$n disappears in a puff of smoke.", FALSE, vict, 0, 0, TO_ROOM);
	char_from_room(vict);
	char_to_room(vict, real_room(JAIL_ROOM));
	act("$n appears with an ear-splitting bang.",
	    FALSE, vict, 0, 0, TO_ROOM);
	/* NOTE: See desolate scene of jail cell you are in. :)   */
	do_look(vict, "", 15);
	send_to_char("You smell fire and brimstone?\n\r", vict);
	/* act("$N is banished.",FALSE,ch,0,vict,TO_CHAR); */
    }
    log(buf);	/* leave record */
    send_to_char("OK.\n\r", ch);
}

void do_transform(struct char_data *ch, char *argument, int cmd)
{
    struct char_data *tmp_ch;
    struct char_file_u tmp;
    int i_player;
    char buf[200];
    extern int load_char(char *name, struct char_file_u *char_element);
    extern void store_to_char_for_transform(struct char_file_u *st,
	    struct char_data *ch);

    return;
    if (IS_NPC(ch))
	return;
    one_argument(argument, buf);
    if (*buf)
	return;
    for (tmp_ch = character_list; tmp_ch; tmp_ch = tmp_ch->next) {
	if (!strcmp(buf, GET_NAME(tmp_ch)) && !IS_NPC(tmp_ch)) {
	    send_to_char("Body already in game.\n\r", ch);
	    return;
	}
    }
    if ((i_player = load_char(buf, &tmp)) < 0) {
	send_to_char("No such player\n\r", ch);
	return;
    }
    store_to_char_for_transform(&tmp, ch->desc->original);
    store_to_char_for_transform(&tmp, ch);
    ch->desc->pos = player_table[i_player].index;
    unstash_char(ch, 0);
    stash_char(ch);
}

void do_snoop(struct char_data *ch, char *argument, int cmd)
{
    static char arg[MAX_STRING_LENGTH];
    struct char_data *victim;
    int diff;

    if (!ch->desc)
	return;
    if (IS_NPC(ch))
	return;
    one_argument(argument, arg);
    if (!*arg) {
	victim = ch;
    }
    else if (!(victim = get_char_vis(ch, arg))) {
	send_to_char("No such person around.\n\r", ch);
	return;
    }
    if (!victim->desc) {
	send_to_char("There's no link.. nothing to snoop.\n\r", ch);
	return;
    }
    if (victim == ch) {
	send_to_char("Ok, you just snoop yourself.\n\r", ch);
	if (ch->desc->snoop.snooping) {
	    ch->desc->snoop.snooping->desc->snoop.snoop_by = 0;
	    ch->desc->snoop.snooping = 0;
	}
	return;
    }
    if (victim->desc->snoop.snoop_by) {
	send_to_char("Busy already. \n\r", ch);
	return;
    }
    diff = GET_LEVEL(victim) - GET_LEVEL(ch);
    if (diff >= 0) {
	send_to_char("You failed.\n\r", ch);
	return;
    }
    send_to_char("Ok. \n\r", ch);
    if (ch->desc->snoop.snooping)
	ch->desc->snoop.snooping->desc->snoop.snoop_by = 0;
    ch->desc->snoop.snooping = victim;
    victim->desc->snoop.snoop_by = ch;
    return;
}

void do_switch(struct char_data *ch, char *argument, int cmd)
{
    static char arg[MAX_STRING_LENGTH];
    struct char_data *victim;

    if (IS_NPC(ch))
	return;
    one_argument(argument, arg);
    if (!*arg) {
	send_to_char("Switch with who?\n\r", ch);
    }
    else {
	if (!(victim = get_char(arg)))
	    send_to_char("They aren't here.\n\r", ch);
	else {
	    if (ch == victim) {
		send_to_char( "He he he... We are jolly funny today, eh?\n\r", ch);
		return;
	    }
	    if (!ch->desc || ch->desc->snoop.snoop_by 
		    || ch->desc->snoop.snooping) {
		send_to_char(
		  "You can't do that, the body is already in use.\n\r", ch);
		return;
	    }
	    if (victim->desc || (!IS_NPC(victim))) {
		if (GET_LEVEL(victim) > GET_LEVEL(ch))
		    send_to_char("They aren't here.\n\r", ch);
		else
		    send_to_char(
		    "You can't do that, the body is already in use!\n\r", ch);
	    }
	    else {
		send_to_char("Ok.\n\r", ch);
		ch->desc->character = victim;
		ch->desc->original = ch;
		victim->desc = ch->desc;
		ch->desc = 0;
		/* NOTE: look room when switched. */
		do_look(victim, "", 0 );
	    }
	}
    }
}

void do_return(struct char_data *ch, char *argument, int cmd)
{
    if (!ch->desc)
	return;

    if (!ch->desc->original) {
	send_to_char("Eh?\n\r", ch);
	return;
    }
    else {
	send_to_char("\r\nYou return to your originaly body.\r\n\r\n", ch);

	ch->desc->character = ch->desc->original; 
	ch->desc->original = 0;

	ch->desc->character->desc = ch->desc;
	/* NOTE: show room when return to original */
	do_look(ch->desc->character, "", 0 );
	ch->desc = 0;
    }
} 

void do_sys(struct char_data *ch, char *argument, int cmd)
{
    struct rusage xru;
    char buffer[MAX_BUFSIZ];
    extern int boottime;
    struct char_data *i;
    struct obj_data *k;
    struct descriptor_data *d;
    static int nits, nics, nids;

    getrusage(RUSAGE_SELF, &xru);
    sprintf(buffer,
	 "sys time: %d secs \tusr time: %d secs \trun time: %d secs\n\r",
	    (int) xru.ru_stime.tv_sec, (int) xru.ru_utime.tv_sec, (int)(time(0) - boottime));
    send_to_char(buffer, ch);
    /* NOTE: Show memory usage */
    /*
    sprintf(buffer, 
	"Max core: %ldK \tshared: %ldK \tdata: %ldK \tstack: %ldK\r\n",
	xru.ru_maxrss, xru.ru_ixrss, xru.ru_idrss, xru.ru_isrss );
    send_to_char(buffer, ch);
    */

    if (GET_LEVEL(ch) >= (IMO + 2)) {
	nits = 0;
	for (k = object_list; k; k = k->next)
	    ++nits;
	nics = 0;
	for (i = character_list; i; i = i->next)
	    ++nics;
	nids = 0;
	for (d = descriptor_list; d; d = d->next)
	    ++nids;
    }
    sprintf(buffer, "\r\nobjects: %d    chars: %d    players: %d\n\r",
	    nits, nics, nids);
    send_to_char(buffer, ch);
}

void do_police(struct char_data *ch, char *argument, int cmd)
{
    char name[MAX_INPUT_LENGTH];
    struct descriptor_data *d;
    int target;

    one_argument(argument, name);
    if (!*argument)
	return;
    target = atoi(name);
    for (d = descriptor_list; d; d = d->next) {
	if (target == d->descriptor) {
	    /* NOTE: Log more info for descriptor */
	    sprintf(name, "Policed: desc: %d, host: %s",
		    d->descriptor, d->host);
	    log(name);
	    if ((d->connected == CON_PLYNG) && (d->character)) {
		if (d->character->player.level < ch->player.level) { 
		    /* NOTE: Same effect with simpler code. */
		    save_char(d->character);
		    stash_char(d->character);
		    purge_player(d->character);
		    /*
		    stash_char(d->character);
		    move_stashfile(d->character->player.name);
		    for (i = 0; i < MAX_WEAR; i++)
			if (d->character->equipment[i]) {
			    extract_obj(unequip_char(d->character, i));
			    d->character->equipment[i] = 0;
			}
		    if (d->character->carrying)
			wipe_obj(d->character->carrying);
		    d->character->carrying = 0;
		    close_socket(d);
		    extract_char(d->character);
		    */
		}
	    }
	    else if (d->character)
		purge_player(d->character);
	    else
		d->connected = CON_CLOSE; 
	}
    }
}

void do_wizlock(struct char_data *ch, char *argument, int cmd)
{
    char buf[MAX_INPUT_LENGTH];
    int i, j;

    buf[0] = 0;
    one_argument(argument, buf);
    if (*argument) {
	j = (-1);
	for (i = 0; i < baddoms; ++i)
	    if (strcmp(baddomain[i], buf) == 0) {
		j = i;
		break;
	    }
	if (j >= 0) {
	    // strcpy(baddomain[j], baddomain[--baddoms]);
	    strcpy(buf, baddomain[--baddoms]);
	    strcpy(baddomain[j], buf);
	}
	else {
	    if (baddoms < BADDOMS)
		strcpy(baddomain[baddoms++], buf);
	}
    }
    else {
	for (i = 0; i < baddoms; ++i) {
	    sprintf(buf, "%s\n", baddomain[i]);
	    send_to_char(buf, ch);
	}
    }
}
