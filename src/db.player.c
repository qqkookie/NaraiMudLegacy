/**************************************************************************
*  file: db.player.c , Database module. 		Part of DIKUMUD *
*  Usage: Loading/Saving chars, booting world, resetting etc.             *
*  Copyright (C) 1990, 1991 - see 'license.doc' for complete information. *
***************************************************************************/

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <signal.h>

#include "char.h"
#include "object.h"
#include "global.h"
#include "comm.h"
#include "gamedb.h"

// #define SAVE_DESC      /* NOTE: was OLD $(CC) -DSYPARK */ 
#define RETURN_TO_QUIT /* NOTE: was OLD $(CC) -DRETURN_TO_QUIT */ 

#define KJHRENT      66666	/* kjh number to tell new rent format */

extern void affect_total(struct char_data *ch);
extern void affect_modify(struct char_data *ch, byte loc, short mod,
			long bitv, bool add); 

/**************************************************************************
*  declarations of most of the 'global' variables                         *
************************************************************************ */

extern struct player_index_element *player_table ;/* index to player file */
extern int top_of_p_table;		/* ref to top of table		*/
extern int loglevel; 

/* generate index table for the player file */
void build_player_index(void)
{
    int index = -1, i;
    struct char_file_u dummy;
    FILE *fl;

    if (!(fl = fopen(PLAYER_FILE, "r+"))) {
	perror("build player index");
	exit(2);
    }
    for (; !feof(fl);) {
	fread(&dummy, sizeof(struct char_file_u), 1, fl);

	if (!feof(fl)) {	/* new record */
	    /* Create new entry in the list */
	    if (index == -1) {
		CREATE(player_table, struct player_index_element, 1); 
		index = 0;
	    }
	    else {
		if (!(player_table = (struct player_index_element *)
		      realloc(player_table, (++index + 1) *
			      sizeof(struct player_index_element)))) {
		    perror("generate index");
		    exit(2);
		}
	    }

	    player_table[index].index = index; 

	    player_table[index].name = strdup(dummy.name); 
	    for (i = 0; (*(player_table[index].name + i) =
		 LOWER(*(dummy.name + i))); i++) ;
	}
    }

    fclose(fl); 
    top_of_p_table = index; 
}

/*************************************************************************
*  stuff related to the save/load player system                  *
*********************************************************************** */

int find_name(char *name)
{
    int i;

    for (i = 0; i <= top_of_p_table; i++) {
	if (!str_cmp((player_table + i)->name, name))
	    return (i);
    }
    return (-1);
} 


/* Load a char, TRUE if loaded, FALSE if not */
int load_char(char *name, struct char_file_u *char_element)
{
    FILE *fl;
    int player_i;

    if ((player_i = find_name(name)) >= 0) {
	if (!(fl = fopen(PLAYER_FILE, "r"))) {
	    perror("Opening player file for reading. (db.c, load_char)");
	    exit(2);
	}
	fseek(fl, (long) (player_table[player_i].index *
			  sizeof(struct char_file_u)), 0);
	fread(char_element, sizeof(struct char_file_u), 1, fl);

	fclose(fl);
	return (player_i);
    }
    else
	return (-1);
}


/* copy data from the file structure to a char struct */
/* COPY FILE to DATA */
void store_to_char(struct char_file_u *st, struct char_data *ch)
{
    int i;

    GET_SEX(ch) = st->sex;
    GET_CLASS(ch) = st->class;
    GET_LEVEL(ch) = st->level;
    ch->player.short_descr = 0;
    ch->player.long_descr = 0;
    if (*st->title) {
	CREATE(GET_TITLE(ch), char, strlen(st->title) + 1); 
	strcpy(GET_TITLE(ch), st->title);
    }
    else /* NOTE: Prevent nil title */ 
	GET_TITLE(ch) = strdup("");
    if (*st->description) {
	CREATE(ch->player.description, char, strlen(st->description) + 1); 
	strcpy(ch->player.description, st->description);
    }
    else
	ch->player.description = 0;

    ch->player.pk_num = st->pk_num;	/* by process */
    ch->player.pked_num = st->pked_num;		/* by process */
    ch->player.guild = st->guild;	/* by process */
    for (i = 0; i < MAX_GUILD_SKILLS; i++)	/* by process */
	ch->player.guild_skills[i] = st->guild_skills[i];

    ch->player.time.birth = st->birth;
    ch->player.time.played = st->played;
    ch->player.time.logon = time(0);
    ch->player.weight = st->weight;
    ch->player.height = st->height;
    ch->abilities = st->abilities;
    ch->tmpabilities = st->abilities;
    ch->points = st->points;
    for (i = 0; i <= MAX_SKILLS - 1; i++) {
	ch->skills[i].learned = st->skills[i].learned;
	ch->skills[i].skilled = st->skills[i].skilled;
	ch->skills[i].recognise = st->skills[i].recognise;
    }
    ch->specials.spells_to_learn = st->spells_to_learn;
    ch->specials.alignment = st->alignment;
    ch->specials.act = st->act;
    ch->specials.carry_weight = 0;
    ch->specials.carry_items = 0;

    ch->points.armor = st->points.armor;
    ch->points.hitroll = st->points.hitroll;
    ch->points.damroll = st->points.damroll;

    /* quest */
    ch->quest.type = st->quest.type;
    ch->quest.data = st->quest.data;
    ch->quest.solved = st->quest.solved;

    /* hand dice */
#ifdef INIT_BARE_HAND
    ch->specials.damnodice = 1;
    ch->specials.damsizedice = 2;
#else
    ch->specials.damnodice = st->damnodice;
    ch->specials.damsizedice = st->damsizedice;
#endif

    /* wimpyness */
    ch->specials.wimpyness = st->wimpyness;
    /* NOTE: New. Time to be released from jail   */
    ch->specials.jail_time = st->jail_time;

    /* remortal */
    ch->player.remortal = st->remortal;

    ch->regeneration = 0;
    CREATE(GET_NAME(ch), char, strlen(st->name) + 1);

    strcpy(GET_NAME(ch), st->name);
    for (i = 0; i <= 4; i++)
	ch->specials.apply_saving_throw[i] = 0;
    for (i = 0; i <= 2; i++)
	GET_COND(ch, i) = st->conditions[i];
    /* Add all spell effects */
    for (i = 0; i < MAX_AFFECT; i++) {
	if (st->affected[i].type)
	    affect_to_char(ch, &st->affected[i]);
    }
    /* NOTE: load room number is virtual  */
#ifdef RETURN_TO_QUIT
    ch->in_room = ( st->load_room >= 0 ) ?
	    (real_room(st->load_room)) : NOWHERE;
#else
    ch->in_room = st->load_room;
#endif
    affect_total(ch);
    ch->bank = st->bank;
    GET_HIT(ch) = MIN(GET_HIT(ch) + (time(0) - st->last_logon) / SECS_PER_MUD_HOUR * 5, GET_MAX_HIT(ch));
    GET_MANA(ch) = MIN(GET_MANA(ch) + (time(0) - st->last_logon) / SECS_PER_MUD_HOUR * 3, GET_MAX_MANA(ch));
    GET_MOVE(ch) = MIN(GET_MOVE(ch) + (time(0) - st->last_logon) / SECS_PER_MUD_HOUR * 10, GET_MAX_MOVE(ch));
}			/* store_to_char */

void store_to_char_for_transform(struct char_file_u *st, struct char_data *ch)
{
    int i;

    GET_SEX(ch) = st->sex;
    GET_CLASS(ch) = st->class;
    GET_LEVEL(ch) = st->level;
    ch->player.short_descr = 0;
    ch->player.long_descr = 0;
    if (*st->title) {
	RECREATE(GET_TITLE(ch), char, strlen(st->title) + 1);

	strcpy(GET_TITLE(ch), st->title);
    }
    else
	GET_TITLE(ch) = 0;
    if (*st->description) {
	RECREATE(ch->player.description, char,
		 strlen(st->description) + 1);

	strcpy(ch->player.description, st->description);
    }
    else
	ch->player.description = 0;
    ch->player.time.birth = st->birth;
    ch->player.time.played = st->played;
    ch->player.time.logon = time(0);
    ch->player.weight = st->weight;
    ch->player.height = st->height;
    ch->abilities = st->abilities;
    ch->tmpabilities = st->abilities;
    ch->points = st->points;
    for (i = 0; i <= MAX_SKILLS - 1; i++)
	ch->skills[i] = st->skills[i];
    ch->specials.spells_to_learn = st->spells_to_learn;
    ch->specials.alignment = st->alignment;
    ch->specials.act = st->act;
    ch->specials.carry_weight = 0;
    ch->specials.carry_items = 0;
    ch->points.armor = 101 - st->points.armor;
    ch->points.hitroll = 0;
    ch->points.damroll = 0;

    RECREATE(GET_NAME(ch), char, strlen(st->name) + 1);

    strcpy(GET_NAME(ch), st->name);
    for (i = 0; i <= 4; i++)
	ch->specials.apply_saving_throw[i] = 0;
    for (i = 0; i <= 2; i++)
	GET_COND(ch, i) = st->conditions[i];
    /* Add all spell effects */
    for (i = 0; i < MAX_AFFECT; i++) {
	if (st->affected[i].type)
	    affect_to_char(ch, &st->affected[i]);
    }
    affect_total(ch);
    ch->bank = st->bank;
}			/* store_to_char_for_transform */


/* copy vital data from a players char-structure to the file structure */
void char_to_store(struct char_data *ch, struct char_file_u *st)
{
    int i;
    struct affected_type *af;
    struct obj_data *char_eq[MAX_WEAR];

    /* Unaffect everything a character can be affected by */

    for (i = 0; i < MAX_WEAR; i++) {
	if (ch->equipment[i])
	    char_eq[i] = unequip_char(ch, i);
	else
	    char_eq[i] = 0;
    }

    for (af = ch->affected, i = 0; i < MAX_AFFECT; i++) {
	if (af) {
	    st->affected[i] = *af;
	    st->affected[i].next = 0;
	    /* subtract effect of the spell or the effect will be doubled */
	    affect_modify(ch, st->affected[i].location,
			  st->affected[i].modifier,
			  st->affected[i].bitvector, FALSE);
	    af = af->next;
	}
	else {
	    st->affected[i].type = 0;	/* Zero signifies not used */
	    st->affected[i].duration = 0;
	    st->affected[i].modifier = 0;
	    st->affected[i].location = 0;
	    st->affected[i].bitvector = 0;
	    st->affected[i].next = 0;
	}
    }
    if ((i >= MAX_AFFECT) && af && af->next)
	log("WARNING: OUT OF STORE ROOM FOR AFFECTED TYPES!!!");
    ch->tmpabilities = ch->abilities;
    st->birth = ch->player.time.birth;
    st->played = ch->player.time.played;
    st->played += (long) (time(0) - ch->player.time.logon);
    st->last_logon = time(0);
    ch->player.time.played = st->played;
    ch->player.time.logon = time(0);
    st->weight = ch->player.weight;
    st->height = ch->player.height;
    st->sex = GET_SEX(ch);
    st->class = GET_CLASS(ch);
    st->level = GET_LEVEL(ch);
    st->abilities = ch->abilities;
    st->points = ch->points;
    st->alignment = ch->specials.alignment;
    st->spells_to_learn = ch->specials.spells_to_learn;
    st->act = ch->specials.act;
    st->points.armor = ch->points.armor;
    st->points.hitroll = ch->points.hitroll;
    st->points.damroll = ch->points.damroll;
    if (GET_TITLE(ch))
	strcpy(st->title, GET_TITLE(ch));
    else
	*st->title = '\0';
    if (ch->player.description)
	strcpy(st->description, ch->player.description);
    else
	*st->description = '\0';
    st->pk_num = ch->player.pk_num;	/* by process */
    st->pked_num = ch->player.pked_num;		/* by process */
    st->guild = ch->player.guild;	/* by process */
    for (i = 0; i < MAX_GUILD_SKILLS; i++)
	st->guild_skills[i] = ch->player.guild_skills[i];	/* by process 
								 */

    for (i = 0; i <= MAX_SKILLS - 1; i++) {
	st->skills[i].learned = ch->skills[i].learned;
	st->skills[i].skilled = ch->skills[i].skilled;
	st->skills[i].recognise = ch->skills[i].recognise;
    }

    /* quest */
    st->quest.type = ch->quest.type;
    st->quest.data = ch->quest.data;
    st->quest.solved = ch->quest.solved;

    /* bare hand dice */
    st->damnodice = ch->specials.damnodice;
    st->damsizedice = ch->specials.damsizedice;

    st->wimpyness = ch->specials.wimpyness;
    /* NOTE: New. Time to be released from jail   */
    st->jail_time = ch->specials.jail_time;

    /* remortal */
    st->remortal = ch->player.remortal;

    if (GET_NAME(ch))
	strcpy(st->name, GET_NAME(ch));
    for (i = 0; i <= 2; i++)
	st->conditions[i] = GET_COND(ch, i);
    for (af = ch->affected, i = 0; i < MAX_AFFECT; i++) {
	if (af) {
	    /* Add effect of the spell or it will be lost */
	    /* When saving without quitting               */
	    affect_modify(ch, st->affected[i].location,
			  st->affected[i].modifier,
			  st->affected[i].bitvector, TRUE);
	    af = af->next;
	}
    }
    for (i = 0; i < MAX_WEAR; i++) {
	if (char_eq[i])
	    equip_char(ch, char_eq[i], i);
    }
    affect_total(ch);
    st->bank = ch->bank;
    /* NOTE: load_room number is virtual */
#ifdef RETURN_TO_QUIT
    st->load_room = ( ch->in_room > 0 ) ? (world[ch->in_room].number) : NOWHERE;
#else
    st->load_room = ch->in_room;
#endif
}			/* Char to store */

/* create a new entry in the in-memory index table for the player file */
int create_entry(char *name)
{
    int i;

    if (top_of_p_table == -1) {
	CREATE(player_table, struct player_index_element, 1);

	top_of_p_table = 0;
    }
    else if (!(player_table = (struct player_index_element *)
	       realloc(player_table, sizeof(struct player_index_element) *
		           (++top_of_p_table + 1)))) {
	perror("create entry");
	exit(2);
    }
    CREATE(player_table[top_of_p_table].name, char, strlen(name) + 1);

    /* copy lowercase equivalent of name to table field */
    for (i = 0; (*(player_table[top_of_p_table].name + i) =
	 LOWER(*(name + i))); i++) ;
    player_table[top_of_p_table].index = top_of_p_table;
    return (top_of_p_table);
}

int remove_entry(struct char_data *ch)
{
    int i;
    int need_copy;
    struct player_index_element *tmp_player_table;
    /* struct char_data *tmp_ch; */
    struct descriptor_data *desc;

    /* only exist imple */
    if (top_of_p_table == 0) {
	free(player_table);
	player_table = 0;
    }
    else {
	/* create new entry */
	CREATE(tmp_player_table, struct player_index_element, top_of_p_table);

	if (!tmp_player_table)
	    return 0;
	/* now copy entry */
	need_copy = ch->desc->pos;
	for (i = 0; i < need_copy; i++)
	    tmp_player_table[i] = player_table[i];
	for (; i < top_of_p_table; i++) {
	    tmp_player_table[i] = player_table[i + 1];
	    tmp_player_table[i].index = i;
	}
	/* update already playing player's pos */
	/*
	for (tmp_ch = character_list; tmp_ch; tmp_ch = tmp_ch->next) {
	    if (tmp_ch->desc && tmp_ch->desc->pos > need_copy)
		tmp_ch->desc->pos--;
	}
	*/
	/* NOTE: Update d->pos of all active player */
	for (desc = descriptor_list; desc; desc = desc->next)
	    if (desc->pos > need_copy)
		(desc->pos)--;
	/* remove old entry */
	free(player_table);

	/* assign new entry */
	player_table = tmp_player_table;
    }
    top_of_p_table--;
    return 1;
}

/* write the vital data of a player to the player file */ 
/* NOTE: save_char() can works correctely for link-dead char. */
/*	save_char_nocon is made for coin copy bug fixing by dsshin */ 
/* NOTE: 2nd argument load_room is removed. 
    world[ch->in_room].number (virtual room number) is default load room. */
/*	void save_char(struct char_data *ch, sh_int load_room) */
void save_char(struct char_data *ch)
{
    FILE *fl;
    int player_i, pos;
    struct char_file_u st;

    if( !ch || IS_NPC(ch))
	return;
    if( ch->desc )
	pos = ch->desc->pos;
    /* NOTE: char has no descriptor (link-dead) */
    else if ((player_i = find_name(GET_NAME(ch))) >= 0)
        pos = player_table[player_i].index;
    else
	return;

    if (!(fl = fopen(PLAYER_FILE, "r+"))) {
	perror("Error open player file(db.player.c, save_char)");
	exit(2);
    } 
    if ( ch->desc )
	bzero(&st, sizeof(struct char_file_u));	 /* NOTE: clear store */
    else {
	fseek(fl, (long) ( pos * sizeof(struct char_file_u)), SEEK_SET); 
	fread(&st, sizeof(struct char_file_u), 1, fl);
    }

    char_to_store(ch, &st);
#ifdef RETURN_TO_QUIT
    st.load_room = ( ch->in_room > 0 ) ? (world[ch->in_room].number) : NOWHERE;
#else
    st.load_room = ch->in_room;
#endif
    if( ch->desc ) {
	if (ch->desc->pwd[0])
	    strcpy(st.pwd, ch->desc->pwd);
	/* NOTE: save page length */
	st.page_len = ch->desc->page_len;
    }
    /* NOTE: Reposition and update */
    fseek(fl, (long) ( pos * sizeof(struct char_file_u)), SEEK_SET); 
    fwrite(&st, sizeof(struct char_file_u), 1, fl);

    fclose(fl);
}

/* rewrite player file to delete one character */
/* NOTE: Now, PLAYER_FILE may be symbolic link. */
void delete_char(struct char_data *ch)
{
    struct char_file_u element;
    FILE *old, *new;
    int i, skip;
    char fname[256], oldname[256];

    if (IS_NPC(ch) || !ch->desc)
	return;

    /* NOTE: Resolve symbolic link */
    if(( i = readlink(PLAYER_FILE, oldname, sizeof(oldname))) > 0 )
	oldname[i] = '\0' ;
    else
	strcpy( oldname, PLAYER_FILE );

    if (!( old = fopen(oldname, "r")))
	return; 
    strcpy(fname, oldname);
    strcat(fname, ".tmp");
    if (!(new = fopen(fname, "w")))
	return;

    skip = ch->desc->pos;

    /* NOTE: top_of_p_table in already decremented in remove_entry() */
    for (i = 0; i <= (top_of_p_table+1); i++) {
	fread(&element, sizeof(struct char_file_u), 1, old ); 
	if (i != skip)
	    fwrite(&element, sizeof(struct char_file_u), 1, new);
    }

    fclose(old);
    fclose(new); 
    remove(oldname);
    rename(fname, oldname); 
}

void stash_char(struct char_data *ch)
{
    struct obj_data *p;
    char stashfile[MAX_LINE_LEN], name[100];
    FILE *fl;
    int i;
    unsigned int mask;
    void stash_contents(FILE * fl, struct obj_data *p, int wear_flag);
    char buf[256];

    mask = sigmask(SIGUSR1) | sigmask(SIGUSR2) | sigmask(SIGINT) |
	sigmask(SIGBUS) | sigmask(SIGSEGV) |
	sigmask(SIGPIPE) | sigmask(SIGALRM) | sigmask(SIGTERM) |
	sigmask(SIGURG) | sigmask(SIGXCPU) | sigmask(SIGHUP);

    if (!ch)
	return;
    if (IS_NPC(ch) || !ch->desc)
	return;
    if (GET_NAME(ch))
	strcpy(name, GET_NAME(ch));
    else
	return;

    for (i = 0; name[i]; ++i)
	if (isupper(name[i]))
	    name[i] = tolower(name[i]);
    /* NOTE: stash file name changed from "name.x.y" to "name.sta" */
    sprintf(stashfile, "%s/%c/%s.%s", STASH, name[0], name, STASH_EXT);

    /*   NOTE: Log "Stash: ..." selectively    */
    if ( loglevel == 4 ) {
	sprintf(buf, "Stash : %s", stashfile);
	log(buf);
    }

    sigsetmask(mask);
    if (!(fl = fopen(stashfile, "w"))) {	/* remove all data if exist */
	perror("saving PC's stash");
	sigsetmask(0);
	return;
    }

    fprintf(fl, "%d\n", KJHRENT);
    for (i = 0; i < MAX_WEAR; ++i)
	if ((p = ch->equipment[i])) {
	    if (p != NULL)
		stash_contents(fl, p, i);
	}
    if (ch->carrying)
	stash_contents(fl, ch->carrying, -1);

    fflush(fl);
    fclose(fl);
    sigsetmask(0);
}

/* Read stash file and load objects to player. and remove stash files */
/* save one item into stashfile. if item has contents, save it also */
/* current version */
void stash_contents(FILE * fl, struct obj_data *p, int wear_flag)
{
    struct obj_data *pc;
    int j;

    if (!fl)
	return;
    if (p == 0)
	return;

    if (p->obj_flags.type_flag != ITEM_KEY && !IS_OBJ_STAT(p, ITEM_NORENT)) {
	if ((pc = p->contains))
	    stash_contents(fl, pc, wear_flag >= 0 ? -2 : wear_flag - 1);

	fprintf(fl, "%d", GET_OBJ_VIRTUAL(p));
	fprintf(fl, " %d", wear_flag);
	for (j = 0; j < 4; ++j)
	    fprintf(fl, " %d", p->obj_flags.value[j]);
	for (j = 0; j < 2; j++)
	    fprintf(fl, " %d %d", p->affected[j].location, p->affected[j].modifier);
	// #ifdef SAVE_DESC
	fprintf(fl, " %d %d", p->obj_flags.extra_flags, p->obj_flags.gpd);
	fprintf(fl, "\n");
	fprintf(fl, "%s\n", p->name);
	fprintf(fl, "%s\n", p->short_description);
	fprintf(fl, "%s\n", p->description);
	// #endif
    }

    if ((pc = p->next_content))
	stash_contents(fl, pc, wear_flag);
}

/* Read stash file and load objects to player. and remove stash files */
/* current version */
void unstash_char(struct char_data *ch, char *stashname)
{
    struct obj_data *obj;
    char stashfile[MAX_LINE_LEN], name[100];	/* ,sf2[100]; */
    FILE *fl;
    int i, n, tmp[4];	/* , newflag ; */
    char tmp_str[255], *str;
    char buf[256];

    /* for Knife rent */
#define MAX_RENT_ITEM 1000
    struct obj_data *item_stackp[MAX_RENT_ITEM];
    int item_stack[MAX_RENT_ITEM];
    int stack_count = 0;
    int where;
    unsigned int mask;
    static int loc_to_where[22] =
    {
	0, 1, 1, 2, 2, 3, 4, 5, 6, 7,
	8, 14, 9, 10, 11, 11, 12, 13, 15, 15,
	16, 17};

#undef MAX_RENT_ITEM

    mask = sigmask(SIGUSR1) | sigmask(SIGUSR2) | sigmask(SIGINT) |
	sigmask(SIGBUS) | sigmask(SIGSEGV) |
	sigmask(SIGPIPE) | sigmask(SIGALRM) | sigmask(SIGTERM) |
	sigmask(SIGURG) | sigmask(SIGXCPU) | sigmask(SIGHUP);
    if (!ch)
	return;
    if (IS_NPC(ch) || !ch->desc)
	return;
    if (GET_NAME(ch))
	strcpy(name, stashname ? stashname : GET_NAME(ch));
    else
	return;
    for (i = 0; name[i]; ++i)
	if (isupper(name[i]))
	    name[i] = tolower(name[i]);
    sigsetmask(mask);
    sprintf(stashfile, "%s/%c/%s.%s", STASH, name[0], name, STASH_EXT);
    if (!(fl = fopen(stashfile, "r"))) {
	sprintf(stashfile, "%s/%c/%s.x.y", STASH, name[0], name);
	if (!(fl = fopen(stashfile, "r"))) {
	    sigsetmask(0);
	    return;
	}
    }

    /* NOTE: More selectively log "unstash...." */
    if( loglevel == 4 ) {
	sprintf(buf, "Unstash : %s", stashfile);
	log(buf);
    }

    fscanf(fl, "%d", &n);
    if (n != KJHRENT) {
	log("File format error in unstash_char. (db.c)");
	sigsetmask(0);
	fclose(fl);
	return;
    }

    for (;;) {
	if (fscanf(fl, "%d", &n) <= 0)
	    break; 

	fscanf(fl, "%d", &where);
	for (i = 0; i < 4; ++i)
	    fscanf(fl, "%d", &tmp[i]);
	obj = read_object(n, VIRTUAL);
	if (obj == 0)
	    continue;
	for (i = 0; i < 4; ++i)
	    obj->obj_flags.value[i] = tmp[i];
	for (i = 0; i < 4; i++)
	    fscanf(fl, "%d", &tmp[i]);
	for (i = 0; i < 2; i++) {
	    obj->affected[i].location = tmp[i * 2];
	    obj->affected[i].modifier = tmp[i * 2 + 1];
	}
	// #ifdef	SAVE_DESC
	fscanf(fl, "%d", &tmp[0]);
	if (tmp[0] != -1)
	    obj->obj_flags.extra_flags = tmp[0];
	fscanf(fl, "%d", &tmp[0]);
	if (tmp[0] != -1)
	    obj->obj_flags.gpd = tmp[0];
	fgets(tmp_str, 255, fl);
	fgets(tmp_str, 255, fl);
	tmp_str[strlen(tmp_str) - 1] = 0;
	if (strlen(tmp_str) != 0) {
	    str = malloc(strlen(tmp_str) + 1);
	    strcpy(str, tmp_str);
	    free(obj->name);
	    obj->name = str;
	}
	fgets(tmp_str, 255, fl);
	tmp_str[strlen(tmp_str) - 1] = 0;
	if (strlen(tmp_str) != 0) {
	    str = malloc(strlen(tmp_str) + 1);
	    strcpy(str, tmp_str);
	    free(obj->short_description);
	    obj->short_description = str;
	}
	fgets(tmp_str, 255, fl);
	tmp_str[strlen(tmp_str) - 1] = 0;
	if (strlen(tmp_str) != 0) {
	    str = malloc(strlen(tmp_str) + 1);
	    strcpy(str, tmp_str);
	    free(obj->description);
	    obj->description = str;
	}
	// #endif
	while (stack_count && item_stack[stack_count - 1] < -1 &&
	       item_stack[stack_count - 1] < where) {
	    stack_count--;
	    obj_to_obj(item_stackp[stack_count], obj);
	}
	item_stackp[stack_count] = obj;
	item_stack[stack_count] = where;
	stack_count++;
    }
    while (stack_count > 0) {
	stack_count--;
	obj_to_char(item_stackp[stack_count], ch);
	if (item_stack[stack_count] >= 0) {
	    where = loc_to_where[item_stack[stack_count]];
	    wear(ch, item_stackp[stack_count], where);
	}
    }

    fclose(fl);
    sigsetmask(0);

/*
   sprintf(sf2,"%s/%c/%s.x.tmp",STASH,name[0],name);
   rename(stashfile,sf2);
   wipe_stash(name);
 */
    /* delete file.x and file.x.y */
}

void wipe_stash(char *name)
{			/* delete id.x and id.x.y */
    char stashfile[100], stname[50];
    int i;

    for (i = 0; name[i]; ++i)
	stname[i] = tolower(name[i]);
    stname[i] = 0;

    sprintf(stashfile, "%s/%c/%s.%s", STASH, stname[0], stname, STASH_EXT);
    /* NOTE: use remove() instead of unlink() for portability */
    remove(stashfile);
}

#ifdef 	NO_DEF
/* NOTE: for adding new player only.. */
void save_char_new( struct char_data *ch )
{
    struct char_file_u st;
    FILE *fl;

    if ( !ch || IS_NPC(ch) || !ch->desc)
	return; 

    /* top_of_p_file++; */
    char_to_store(ch, &st);
    if (ch->desc->pwd)
	strcpy(st.pwd, ch->desc->pwd);
    st.load_room = NOWHERE;
    if (!(fl = fopen(PLAYER_FILE, "r+"))) {
	perror("save char expand");
	exit(2);
    }
    fseek(fl, (long) 0 , SEEK_END);
    fwrite(&st, sizeof(struct char_file_u), 1, fl);

    fclose(fl);
} 

/* NOTE: Move stash is not used any more. Comment out */
void move_stashfile(char *victim)
{			/* move file.x to file.x.y */
    char sf1[100], sf2[100], name[100];
    int i;		/* ,j;  */

    strcpy(name, victim);
    for (i = 0; name[i]; ++i)
	if (isupper(name[i]))
	    name[i] = tolower(name[i]);
    sprintf(sf1, "%s/%c/%s.x", STASH, name[0], name);
    sprintf(sf2, "%s/%c/%s.x.y", STASH, name[0], name);
    rename(sf1, sf2);
}

void stash_contentsII(FILE * fp, struct obj_data *o, int wear_flag)
{
    struct obj_data *oc;
    int i;

    if (!o)
	return;
    if (o->obj_flags.type_flag != ITEM_KEY && !IS_OBJ_STAT(o, ITEM_NORENT)) {
	if (oc = o->contains)
	    stash_contentsII(fp, oc, ((wear_flag >= 0) ? -2 : (wear_flag - 1)));
	fprintf(fp, "%d", GET_OBJ_VIRTUAL(o));
	fprintf(fp, " %d", wear_flag);
	for (i = 0; i < 4; i++)
	    fprintf(fp, " %d", o->obj_flags.value[i]);
	for (i = 0; i < 2; i++)
	    fprintf(fp, " %d %d", o->affected[i].location,
		    o->affected[i].modifier);
	// #ifdef SAVE_DESC
	fprintf(fp, " %d %d", o->obj_flags.extra_flags, o->obj_flags.gpd);
	fprintf(fp, "\n");
	fprintf(fp, "%s\n", o->name);
	fprintf(fp, "%s\n", o->short_description);
	fprintf(fp, "%s\n", o->description);
	// #endif
	fprintf(fp, "\n");
    }
    if (oc = o->next_content)
	stash_contentsII(fp, oc, wear_flag);
}

#ifdef NEW_STASH	/* fail */
void stash_contents(FILE * fp, struct obj_data *o, int wear_flag)
{
    struct obj_data *oc;
    int i;

    if (!o)
	return;
    if (o->obj_flags.type_flag != ITEM_KEY && !IS_OBJ_STAT(o, ITEM_NORENT)) {
	if (oc = o->contains)
	    stash_contents(fp, oc, wear_flag >= 0 ? -2 : wear_flag - 1);
	if (oc = o->next_content)
	    stash_contents(fp, oc, wear_flag);
	fprintf(fp, "%d", obj_index[o->item_number].virtual);
	fprintf(fp, " %d", wear_flag);
	for (i = 0; i < 4; i++)
	    fprintf(fp, " %d", o->obj_flags.value[i]);
	for (i = 0; i < 2; i++)
	    fprintf(fp, " %d %d", o->affected[i].location,
		    o->affected[i].modifier);
	fprintf(fp, "\n");
    }
}

/* Read stash file and load objects to player. and remove stash files */
void unstash_char(struct char_data *ch, char *stashname)
{
#define MAX_RENT_ITEM 1000
    struct obj_data *item_stackp[MAX_RENT_ITEM];
    int item_stack[MAX_RENT_ITEM];

#undef MAX_RENT_ITEM
    FILE *fp;
    struct obj_data *obj;
    int i;
    int value[4];
    int virtual_n;
    int magic_num;
    char fname[256];
    char name[256];

    int stack_count = 0;
    int where;
    static int loc_to_where[22] =
    {
	0, 1, 1, 2, 2, 3, 4, 5, 6, 7,
	8, 14, 9, 10, 11, 11, 12, 13, 15, 15,
	16, 17};

    if (GET_NAME(ch))
	strcpy(name, stashname ? stashname : GET_NAME(ch));
    else
	return;

    for (i = 0; name[i]; i++)
	name[i] = tolower(name[i]);
    sprintf(fname, "%s/%c/%s.x.y", STASH, name[0], name);

    if (!(fp = fopen(fname, "r"))) {
	log("Can't open file in unstash_char. (db.c)");
	return;
    }

    fscanf(fp, "%d\n", &magic_num);
    if (magic_num != KJHRENT) {
	log("File format error in unstash_char. (db.c)");
	fclose(fp);
	return;
    }

    while (1) {
	if (fscanf(fp, "%d", &virtual_n) <= 0)
	    break;

	fscanf(fp, "%d", &where);
	for (i = 0; i < 4; i++)
	    fscanf(fp, "%d", &value[i]);
	obj = read_object(virtual_n, VIRTUAL);
	if (!obj) {
	    log("Obj's virtual number doesn't exist. (db.c)");
	    fclose(fp);
	    return;
	}
	for (i = 0; i < 4; i++)
	    obj->obj_flags.value[i] = value[i];
	for (i = 0; i < 4; i++)
	    fscanf(fp, "%d", &value[i]);
	for (i = 0; i < 2; i++) {
	    obj->affected[i].location = value[i << 1];
	    obj->affected[i].modifier = value[(i << 1) + 1];
	}
	while (stack_count && item_stack[stack_count - 1] < -1 &&
	       item_stack[stack_count - 1] < where) {
	    stack_count--;
	    obj_to_obj(item_stackp[stack_count], obj);
	}
	item_stackp[stack_count] = obj;
	item_stack[stack_count] = where;
	stack_count++;
    }
    while (stack_count > 0) {
	stack_count--;
	obj_to_char(item_stackp[stack_count], ch);
	if (item_stack[stack_count] >= 0) {
	    where = loc_to_where[item_stack[stack_count]];
	    wear(ch, item_stackp[stack_count], where);
	}
    }
    fclose(fp);
}

#endif		/* NEW_STASH */ 
#endif		/* NO_DEF */

void do_checkrent(struct char_data *ch, char *argument, int cmd)
{
    char stashfile[MAX_LINE_LEN], name[MAX_NAME_LEN], buf[MAX_LINE_LEN];
    char str[255];
    FILE *fl;
    int i, j, n;

    one_argument(argument, name);
    if (!*name)
	return;
    for (i = 0; name[i]; ++i)
	if (isupper(name[i]))
	    name[i] = tolower(name[i]);
    sprintf(stashfile, "%s/%c/%s.%s", STASH, name[0], name, STASH_EXT);
    if (!(fl = fopen(stashfile, "r"))) {
	sprintf(buf, "%s has nothing in rent.\n\r", name);
	send_to_char(buf, ch);
	return;
    }
    buf[0] = 0;
    fscanf(fl, "%d", &n);
    for (i = j = 0;;) {
	if (fscanf(fl, "%d", &n) <= 0)
	    break;
	if (n < 1000)
	    continue;
	if (n > 99999)
	    continue;
	++j;
	sprintf(buf + i, "%5d%c", n, (j == 10) ? '\n' : ' ');
	if (j == 10)
	    j = 0;
	i += 5;
	fgets(str, 255, fl);
	fgets(str, 255, fl);
	fgets(str, 255, fl);
	fgets(str, 255, fl);
    }
    fclose(fl);
    strcat(buf, "\n\r");
    send_to_char(buf, ch);
    return;
}

void do_extractrent(struct char_data *ch, char *argument, int cmd)
{
    char name[MAX_NAME_LEN], buf[MAX_LINE_LEN];

    one_argument(argument, name);
    if (!*name)
	return;
    unstash_char(ch, name);
    send_to_char("OK.\n\r", ch);
    sprintf(buf, "%s grabbed rent for %s", GET_NAME(ch), name);
    log(buf);
}

void do_replacerent(struct char_data *ch, char *argument, int cmd)
{
    char name[MAX_NAME_LEN], buf[MAX_LINE_LEN];

    one_argument(argument, name);
    if (!*name)
	return;
    stash_char(ch);
    send_to_char("OK.\n\r", ch);
    sprintf(buf, "%s replaced rent for %s", GET_NAME(ch), name);
    log(buf);
}

/* NOTE: BUG FIX: Humm.. incorrect args */
void do_rent(struct char_data *ch, char *arg, int cmd)
{ 
    if (IS_NPC(ch))
	return;
    if (cmd) {
	if (!IS_SET(world[ch->in_room].room_flags, RENT)) {
	    send_to_char("You cannot rent here.\n\r", ch);
	    return;
	}
	send_to_char("\r\nYou retire for the night.\r\n", ch);
	act("$n retires for the night.", FALSE, ch, 0, 0, TO_NOTVICT);
    }
    save_char(ch);
    stash_char(ch);	/* clear file.x and save into file.x */
    extract_char(ch); 

    /* NOTE: If idle char is forced to rent by check_idling() ,
    *       cmd and descriptor may be NULL  - check cmd by jhpark */
    if (ch->desc && cmd) {
	/* NOTE: leaving game code moved here from extract_char() (handler.c) */
	ch->desc->connected = CON_RMOTD;
	SEND_TO_Q("\r\n*** PRESS RETURN : ", ch->desc);
    } 
    return;
}

