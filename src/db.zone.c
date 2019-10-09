#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "char.h"
#include "object.h"
#include "global.h"
#include "comm.h"
#include "gamedb.h"

/* NOTE: Check validity of world/zone file. for zone debugging only. 
         *DON'T* define this for production version. */ 
// #define ZONE_CHECK

/* NOTE: Moved from db.h	*/
/* zone definition structure. for the 'zone-table'   */
struct zone_data {
    char *name;		/* name of this zone                  */
    char *filename;	/* zone file name
			     */
    int lifespan;	/* how long between resets (minutes)  */
    int age;		/* current age of this zone (minutes) */
    int top;		/* upper limit for rooms in this zone */

    int reset_mode;	/* conditions for reset (see below)   */
    struct reset_com *cmd;	/* command table for reset    */

    /* 
     *  Reset mode:
     *  0: Don't reset, and don't update age.
     *  1: Reset if no PC's are located in zone.
     *  2: Just reset.
     */
}; 

/* structure for the reset commands */
struct reset_com {
    char command;	/* current command                      */
    bool if_flag;	/* if TRUE: exe only if preceding exe'd */
    int arg1;		/* */
    int arg2;		/* Arguments to the command             */
    int arg3;		/* */

    /*  Commands:
     *  'M': Read a mobile     
     *  'O': Read an object
     *  'G': Give obj to mob
     *  'P': Put obj in obj
     *  'G': Obj to char
     *  'E': Obj to char equip
     *  'D': Set state of door 
     */
}; 

/* for queueing zones for update   */
struct reset_q_element {
    int zone_to_reset;	/* ref to zone_data */
    struct reset_q_element *next;
}; 

/* structure for the update queue     */
struct reset_q_type {
    struct reset_q_element *head;
    struct reset_q_element *tail;
} reset_q; 


extern struct zone_data *zone_table;	/* table of reset data		*/
extern int top_of_zone_table;

extern int regen_percent;
extern int regen_time_percent;
extern int regen_time;

extern struct obj_data *get_obj_in_list_num(int num, struct obj_data *list);
extern struct obj_data *get_obj_num(int nr);

/* local procedures */
void allocate_room(int new_top);
void setup_dir(FILE * fl, int room, int dir);
void reset_zone(int zone);
int is_empty(int zone_nr); 

#define ALL_WORLD_FILE "world/world_files"

void boot_world(void)
{
    FILE *fl;
    FILE *all_files;
    int room_nr = 0, zone = 0, virtual_nr, flag, tmp;
    char *temp, chk[50];
    struct extra_descr_data *new_descr;
    char file_name[100];
    int len;

    world = 0;
    character_list = 0;
    object_list = 0;

    if (!(all_files = fopen(ALL_WORLD_FILE, "r"))) {
	perror("fopen");
	log("boot_world: could not open world file.");
	exit(2);
    }

    while (1) {
	fgets(file_name, 99, all_files);

	if (file_name[0] == '$')
	    break;	/* end of file */

	len = strlen(file_name) - 1;
	if (file_name[len] == '\n' || file_name[len] == '\r')
	    file_name[len] = 0;
	if (!(fl = fopen(file_name, "r"))) {
	    perror("boot_zones");
	    perror(file_name);
	    exit(2);
	}

	do {
	    fscanf(fl, " #%d\n", &virtual_nr);
	    temp = fread_string(fl);
	    if ((flag = (*temp != '$'))) {	/* a new record to be read */
		allocate_room(room_nr);
		world[room_nr].number = virtual_nr;
		world[room_nr].name = temp;
		world[room_nr].description = fread_string(fl);

		if (top_of_zone_table >= 0) {
		    fscanf(fl, " %*d ");
		    /* OBS: Assumes ordering of input rooms */
		    if (world[room_nr].number <=
			(zone ? zone_table[zone - 1].top : -1)) {
			fprintf(stderr, "Room nr %d is below zone %d.\n",
				room_nr, zone);
			fprintf(stderr, "DEBUG: %d, %s\n",
			    world[room_nr].number, world[room_nr].name);
			exit(2);
		    }
		    while (world[room_nr].number > zone_table[zone].top)
			if (zone > top_of_zone_table) {
			    fprintf(stderr, "Room %d is outside of any zone.\n",
				    virtual_nr);
			    exit(2);
			}
		    world[room_nr].zone = zone;
		}
		fscanf(fl, " %d ", &tmp);
		world[room_nr].room_flags = tmp;
		fscanf(fl, " %d ", &tmp);
		world[room_nr].sector_type = tmp;

		world[room_nr].funct = 0;
		world[room_nr].contents = 0;
		world[room_nr].people = 0;
		world[room_nr].light = 0;	/* Zero light sources */

		for (tmp = 0; tmp <= 5; tmp++)
		    world[room_nr].dir_option[tmp] = 0;

		world[room_nr].ex_description = 0;

		while (1) {
		    fscanf(fl, " %s \n", chk);
		    if (*chk == 'D')	/* direction field */
			setup_dir(fl, room_nr, atoi(chk + 1));
		    else if (*chk == 'E') {	/* extra description field */
			CREATE(new_descr, struct extra_descr_data, 1);

			new_descr->keyword = fread_string(fl);
			new_descr->description = fread_string(fl);
			new_descr->next = world[room_nr].ex_description;
			world[room_nr].ex_description = new_descr;
		    }
		    else if (*chk == 'S')	/* end of current room */
			break;
		}
		room_nr++;
	    }
	} while (flag);

	free(temp);	/* cleanup the area containing the terminal $  */
	fclose(fl);
	zone++;
    }

    fclose(all_files);
    top_of_world = --room_nr;
}

#undef ALL_WORLD_FILE 

/* load the rooms */
/*
void boot_world(void)
{
    FILE *fl;
    int room_nr = 0, zone = 0, dir_nr, virtual_nr, flag, tmp;
    char *temp, chk[50];
    struct extra_descr_data *new_descr;

    world = 0;
    character_list = 0;
    object_list = 0;

    if (!(fl = fopen(WORLD_FILE, "r"))) {
	perror("fopen");
	log("boot_world: could not open world file.");
	exit(2);
    }

    do {
	fscanf(fl, " #%d\n", &virtual_nr);
	temp = fread_string(fl);
	if (flag = (*temp != '$')) {
	    allocate_room(room_nr);
	    world[room_nr].number = virtual_nr;
	    world[room_nr].name = temp;
	    world[room_nr].description = fread_string(fl);

	    if (top_of_zone_table >= 0) {
		fscanf(fl, " %*d ");
		if (world[room_nr].number <= (zone ? zone_table[zone-1].top : -1)) {
		    fprintf(stderr, "Room nr %d is below zone %d.\n",
			room_nr, zone);
		    exit(2);
		}
		while (world[room_nr].number > zone_table[zone].top)
		    if (zone > top_of_zone_table) {
			fprintf(stderr, "Room %d is outside of any zone.\n",
			    virtual_nr);
			exit(2);
		    }
		    world[room_nr].zone = zone;
	    }
	    fscanf(fl, " %d ", &tmp);
	    world[room_nr].room_flags = tmp;
	    fscanf(fl, " %d ", &tmp);
	    world[room_nr].sector_type = tmp;

	    world[room_nr].funct = 0;
	    world[room_nr].contents = 0;
	    world[room_nr].people = 0;
	    world[room_nr].light = 0; 

	    for (tmp = 0; tmp <= 5; tmp++)
		world[room_nr].dir_option[tmp] = 0;

	    world[room_nr].ex_description = 0;

	    for (;;) {
		fscanf(fl, " %s \n", chk);

		if (*chk == 'D') 
		    setup_dir(fl, room_nr, atoi(chk + 1));
		else if (*chk == 'E')  {
		    CREATE(new_descr, struct extra_descr_data, 1);
		    new_descr->keyword = fread_string(fl);
		    new_descr->description = fread_string(fl);
		    new_descr->next = world[room_nr].ex_description;
		    world[room_nr].ex_description = new_descr;
		}
		else if (*chk == 'S') 
		    break;
	    }

	    room_nr++;
	}
	zone++;
    } while (flag);

    free(temp); 

    fclose(fl);
    top_of_world = --room_nr;
}
 */

void allocate_room(int new_top)
{
    struct room_data *new_world;

    if (new_top) {
	if (!(new_world = (struct room_data *)
	      realloc(world, (new_top + 1) * sizeof(struct room_data)))) {
	    perror("alloc_room");
	    exit(2);
	}
    }
    else
	CREATE(new_world, struct room_data, 1);

    world = new_world;
} 

/* read direction data */
void setup_dir(FILE * fl, int room, int dir)
{
    int tmp;

    CREATE(world[room].dir_option[dir], struct room_direction_data, 1);

    world[room].dir_option[dir]->general_description = fread_string(fl);

    world[room].dir_option[dir]->keyword = fread_string(fl);

    fscanf(fl, " %d ", &tmp);
    if (tmp == 1)
	world[room].dir_option[dir]->exit_info = EX_ISDOOR;
    else if (tmp == 2)
	world[room].dir_option[dir]->exit_info = EX_ISDOOR | EX_PICKPROOF;
    else if (tmp == 3)
	world[room].dir_option[dir]->exit_info = EX_ISDOOR | EX_NOPHASE;
    else if (tmp == 4)
	world[room].dir_option[dir]->exit_info = EX_ISDOOR | EX_PICKPROOF
	    | EX_NOPHASE;
    else
	world[room].dir_option[dir]->exit_info = 0;

    fscanf(fl, " %d ", &tmp);
    world[room].dir_option[dir]->key = tmp;

    fscanf(fl, " %d ", &tmp);
    world[room].dir_option[dir]->to_room = tmp;
} 

void renum_world(void)
{
    register int room, door;

    for (room = 0; room <= top_of_world; room++) 
	for (door = 0; door <= 5; door++)
	    if (world[room].dir_option[door])
		if (world[room].dir_option[door]->to_room != NOWHERE) 
		    world[room].dir_option[door]->to_room =
			real_room(world[room].dir_option[door]->to_room);
}

/* NOTE: NEW! check validity of all mobiles and objects */
void check_mobile_and_object(void)
{
    int i;
    struct char_data *mob;
    struct obj_data *obj;

    extern int top_of_mobt;	/* top of mobile index table	*/
    extern int top_of_objt;	/* top of object index table	*/ 

    for( i = 0; i <=top_of_mobt; i++) {
	mob = read_mobile(i, REAL);
	char_to_room(mob, 0);
	if(!mob->player.name )
	    fprintf(stderr, "No mob name! %d\n", mob_index[i].virtual);
	if(!mob->player.short_descr)
	    fprintf(stderr, "No mob short desc! %d\n", mob_index[i].virtual);
	if(!mob->player.long_descr)
	    fprintf(stderr, "No mob long desc! %d\n", mob_index[i].virtual);
	if(!mob->player.description)
	    fprintf(stderr, "No mob description! %d\n", mob_index[i].virtual);
	extract_char(mob);
    }

    for( i = 0; i <=top_of_objt; i++) {
	obj = read_object(i, REAL);
	if ((GET_ITEM_TYPE(obj) == ITEM_WEAPON)
		&& obj->obj_flags.value[0] > 0 ) 
	    fprintf(stderr, "WEAPON #%06d [%-30s] MAGIC %d \n",
		obj_index[i].virtual, obj->name, obj->obj_flags.value[0] );
	if ( GET_ITEM_TYPE(obj)< ITEM_LIGHT || GET_ITEM_TYPE(obj) > ITEM_MONEY
	    || GET_ITEM_TYPE(obj) ==  7 /* ITEM_MISSILE */
	    || GET_ITEM_TYPE(obj) ==  ITEM_OTHER
	    || GET_ITEM_TYPE(obj) ==  13 /* ITEM_TRASH */
	    || GET_ITEM_TYPE(obj) ==  14 /* ITEM_TRAP */
	    )
	    fprintf(stderr, "ITEM TYPE = %2d #%06d [%s]  V0-3: %d,%d,%d,%d.\n",
		GET_ITEM_TYPE(obj), obj_index[i].virtual, obj->name,
		obj->obj_flags.value[0], obj->obj_flags.value[1],
		obj->obj_flags.value[2], obj->obj_flags.value[3]);

	extract_obj(obj);
    }
}

/* NOTE: NEW! check validity of all rooms in the world. */
void check_world(void)
{
    register int room, door;
    char buf[256];
    extern char *exit_bits[];

    for (room = 0; room <= top_of_world; room++) {
	for (door = 0; door <= 5; door++)
	    if (world[room].dir_option[door])
		if ((world[room].dir_option[door]->to_room != NOWHERE)
		    /* NOTE: Check validity of door key object */
		    && world[room].dir_option[door]->key > 0 
		    && real_object(world[room].dir_option[door]->key) < 0 ) {

		    fprintf(stderr,"NO KEY!! %s (%d) %s KEY: %d.\n",
			world[room].name, world[room].number, dirs[door], 
			world[room].dir_option[door]->key );

		    sprintbit( world[room].dir_option[door]->exit_info, 
			exit_bits, buf ) ; 
		    fprintf(stderr, "    EXIT Flags: %s\n", buf ); 
		}
	/* NOTE: Check validity of sector_type */
	if( world[room].sector_type > SECT_SKY 
	    || world[room].sector_type < SECT_INSIDE )
	    fprintf(stderr, "SECT TYPE!! Invalid. room %s (#%d) type = %d\n",
		world[room].name, world[room].number, world[room].sector_type);
    }
} 

/* NOTE: NEW! check validity of all zone reset command list.
        Called by renum_zone_table(). */ 
void check_zcmd(struct reset_com *zcmd, int zone, int comm)
{
    struct char_data *mob;
    struct obj_data *obj;
    struct room_direction_data *door; 
    char buf[MAX_BUFSIZ]; 
    extern char *exit_bits[];

    /* NOTE: Check validity of mobile/object/room */
    if ( zone_table[zone].cmd[comm].arg1 < 0 
	    || zone_table[zone].cmd[comm].arg3 < 0 )
	fprintf(stderr,"Invalid Zcmd: zone %d, comm %d: "
		" cmd %c, %d, %d, %d.\n", zone, comm, 
		zcmd->command, zcmd->arg1, zcmd->arg2, zcmd->arg3 );

    /* NOTE: Check validity of mobile */
    else if( zcmd->command == 'M' ) {
	mob = read_mobile(zone_table[zone].cmd[comm].arg1,REAL);
	char_to_room(mob, zone_table[zone].cmd[comm].arg3);
	extract_char(mob); 
    }
    /* NOTE: Check validity object */
    else if ( zcmd->command == 'O' || zcmd->command == 'G' 
	    || zcmd->command == 'E' || zcmd->command == 'P' ) { 
	obj = read_object(zcmd->arg1,VIRTUAL);
	extract_obj(obj);
    }
    /* NOTE: Check validity of key for locked door with key */
    else if( zcmd->command == 'D' && zcmd->arg2 == 2 ) {
	door = world[zone_table[zone].cmd[comm].arg1].dir_option[zcmd->arg2];  
	if ( door && door->key > 0 && real_object(door->key ) < 0 ) { 
	    fprintf(stderr,"NO KEY!! %s (%d) %s KEY: %d.\n",
		world[zone_table[zone].cmd[comm].arg1].name, 
		zcmd->arg1, dirs[zcmd->arg2], zcmd->arg3 ) ;

	    sprintbit( door->exit_info, exit_bits, buf ) ; 
	    fprintf(stderr, "    EXIT Flags: %s\n", buf ); 
	}
    } 
}

void check_zone_data(void)
{
#ifdef  ZONE_CHECK
    check_world();
    check_mobile_and_object();
#endif		/* ZONE_CHECK */ 
}

void renum_zone_table(void)
{
    int zone, comm;
    struct reset_com ZCMD;


    for (zone = 0; zone <= top_of_zone_table; zone++)
	for (comm = 0; zone_table[zone].cmd[comm].command != 'S'; comm++) {
	    ZCMD = zone_table[zone].cmd[comm];
	    switch (ZCMD.command) {
	    case 'M': 
		zone_table[zone].cmd[comm].arg1 = real_mobile(ZCMD.arg1);
		zone_table[zone].cmd[comm].arg3 = real_room(ZCMD.arg3); 
		break;
	    case 'O':
		zone_table[zone].cmd[comm].arg1 = real_object(ZCMD.arg1);
		if (ZCMD.arg3 != NOWHERE)
		    zone_table[zone].cmd[comm].arg3 = real_room(ZCMD.arg3);
		break;
	    case 'G':
		zone_table[zone].cmd[comm].arg1 = real_object(ZCMD.arg1);
		break;
	    case 'E':
		zone_table[zone].cmd[comm].arg1 = real_object(ZCMD.arg1);
		break;
	    case 'P':
		zone_table[zone].cmd[comm].arg1 = real_object(ZCMD.arg1);
		zone_table[zone].cmd[comm].arg3 = real_object(ZCMD.arg3);
		break;
	    case 'D':
		zone_table[zone].cmd[comm].arg1 = real_room(ZCMD.arg1);
		break;
	    }
#ifdef ZONE_CHECK
	    check_zcmd(&ZCMD, zone, comm);
#endif		/* ZONE_CHECK */ 
	}
}

/* new version of boot_zone : by ares */
/* read lib/zone/.zon files */

#define ALL_ZONE_FILE "zone/zone_files"

void load_zones(int zone)
{
    FILE *fl;
    char buf[255], *check;
    int zon = 0, cmd_no = 0, expand;
    int tmp;

    if (zon > top_of_zone_table)
	return;
    fl = fopen(zone_table[zon].filename, "r");
    if (!fl) {
	sprintf(buf, "Error in reading zone file '%s'", zone_table[zon].filename);
	log(buf);
	return;
    }
    free(zone_table[zon].name);
    free(zone_table[zon].cmd);
    check = fread_string(fl);
    zone_table[zon].name = check;
    fscanf(fl, " %d ", &zone_table[zon].top);
    fscanf(fl, " %d ", &zone_table[zon].lifespan);
    fscanf(fl, " %d ", &zone_table[zon].reset_mode);

    /* read the command table */
    cmd_no = 0;
    for (expand = 1;;) {
	if (expand) {
	    if (!cmd_no)
		CREATE(zone_table[zon].cmd, struct reset_com, 1);

	    else if (!(zone_table[zon].cmd =
		       (struct reset_com *) realloc(zone_table[zon].cmd,
				(cmd_no + 1) * sizeof(struct reset_com)))) {
		perror("reset command load");
		perror(zone_table[zon].filename);
		exit(2);
	    }
	}
	expand = 1;
	fscanf(fl, " ");	/* skip blanks */
	fscanf(fl, "%c", &zone_table[zon].cmd[cmd_no].command);

	/* end of each zone file */
	if (zone_table[zon].cmd[cmd_no].command == 'S') {
	    fclose(fl);
	    break;
	}

	if (zone_table[zon].cmd[cmd_no].command == '*') {
	    expand = 0;
	    fgets(buf, 80, fl);		/* skip command */
	    continue;
	}

	fscanf(fl, " %d %d %d",
	       &tmp,
	       &zone_table[zon].cmd[cmd_no].arg1,
	       &zone_table[zon].cmd[cmd_no].arg2);
	/* NOTE:  if_flag is char type, no %d format */
	zone_table[zon].cmd[cmd_no].if_flag = (char) tmp;

	if (zone_table[zon].cmd[cmd_no].command == 'M' ||
	    zone_table[zon].cmd[cmd_no].command == 'O' ||
	    zone_table[zon].cmd[cmd_no].command == 'E' ||
	    zone_table[zon].cmd[cmd_no].command == 'P' ||
	    zone_table[zon].cmd[cmd_no].command == 'D')
	    fscanf(fl, " %d", &zone_table[zon].cmd[cmd_no].arg3);

	fgets(buf, 80, fl);	/* read comment */

	cmd_no++;
    }
}

void boot_zones(void)
{
    FILE *all_files;
    FILE *fl;
    int zon = 0, cmd_no = 0, expand, tmp;
    char *check, buf[81];
    char file_name[100];
    int len;

    if (!(all_files = fopen(ALL_ZONE_FILE, "r"))) {
	perror("boot_zones (zone_files)");
	exit(2);
    }

    while (1) {
	fgets(file_name, 99, all_files);

	if (file_name[0] == '$')
	    break;	/* end of file */

	len = strlen(file_name) - 1;
	if (file_name[len] == '\n' || file_name[len] == '\r')
	    file_name[len] = 0;
	if (!(fl = fopen(file_name, "r"))) {
	    perror("boot_zones");
	    perror(file_name);
	    exit(2);
	}

	check = fread_string(fl);

	/* alloc a new zone */
	if (!zon)
	    CREATE(zone_table, struct zone_data, 1);

	else if (!(zone_table = (struct zone_data *) realloc(zone_table,
				   (zon + 1) * sizeof(struct zone_data)))) {
	    perror("boot_zones realloc");
	    perror(file_name);
	    exit(2);
	}

	zone_table[zon].name = check;
	/* NOTE: that file_name needs terminating null  */
	CREATE(zone_table[zon].filename, char, strlen(file_name) + 1);

	strcpy(zone_table[zon].filename, file_name);
	fscanf(fl, " %d ", &zone_table[zon].top);
	fscanf(fl, " %d ", &zone_table[zon].lifespan);
	fscanf(fl, " %d ", &zone_table[zon].reset_mode);

	/* read the command table */
	cmd_no = 0;
	for (expand = 1;;) {
	    if (expand) {
		if (!cmd_no)
		    CREATE(zone_table[zon].cmd, struct reset_com, 1);

		else if (!(zone_table[zon].cmd =
			   (struct reset_com *) realloc(zone_table[zon].cmd,
				(cmd_no + 1) * sizeof(struct reset_com)))) {
		    perror("reset command load");
		    perror(file_name);
		    exit(2);
		}
	    }
	    expand = 1;
	    fscanf(fl, " ");	/* skip blanks */
	    fscanf(fl, "%c", &zone_table[zon].cmd[cmd_no].command);

	    /* end of each zone file */
	    if (zone_table[zon].cmd[cmd_no].command == 'S') {
		fclose(fl);
		break;
	    }

	    if (zone_table[zon].cmd[cmd_no].command == '*') {
		expand = 0;
		fgets(buf, 80, fl);	/* skip command */
		continue;
	    }

/* NOTE: that cmd->if_flag is char type , not int .. */
	    fscanf(fl, " %d %d %d", &tmp,
		   &zone_table[zon].cmd[cmd_no].arg1,
		   &zone_table[zon].cmd[cmd_no].arg2);
	    zone_table[zon].cmd[cmd_no].if_flag = tmp;

	    if (zone_table[zon].cmd[cmd_no].command == 'M' ||
		zone_table[zon].cmd[cmd_no].command == 'O' ||
		zone_table[zon].cmd[cmd_no].command == 'E' ||
		zone_table[zon].cmd[cmd_no].command == 'P' ||
		zone_table[zon].cmd[cmd_no].command == 'D')
		fscanf(fl, " %d", &zone_table[zon].cmd[cmd_no].arg3);

	    fgets(buf, 80, fl);		/* read comment */

	    cmd_no++;
	}
	zon++;
    }
    top_of_zone_table = --zon;
    fclose(all_files);
}
#undef ALL_ZONE_FILE

/* original boot_zone : read tinyworld.zon */
/* load the zone table and command tables */
/*
void boot_zones(void) {
    FILE *fl;
    int zon = 0, cmd_no = 0, ch, expand, tmp;
    char *check, buf[81];

    if (!(fl = fopen(ZONE_FILE, "r"))) {
	perror("boot_zones");
	exit(2);
    }

    for (;;) {
	fscanf(fl, " #%*d\n");
	check = fread_string(fl);

	if (*check == '$')
	    break; 

	if (!zon)
	    CREATE(zone_table, struct zone_data, 1);
	else
	    if (!(zone_table = (struct zone_data *) realloc(zone_table,
	    (zon + 1) * sizeof(struct zone_data)))) {
		perror("boot_zones realloc");
		exit(2);
	    }

	zone_table[zon].name = check;
	fscanf(fl, " %d ", &zone_table[zon].top);
	fscanf(fl, " %d ", &zone_table[zon].lifespan);
	fscanf(fl, " %d ", &zone_table[zon].reset_mode);

	cmd_no = 0;

	for (expand = 1;;) {
	    if (expand)
		if (!cmd_no)
		    CREATE(zone_table[zon].cmd, struct reset_com, 1);
		else
		    if (!(zone_table[zon].cmd =
			(struct reset_com *) realloc(zone_table[zon].cmd, 
			(cmd_no + 1) * sizeof(struct reset_com)))) {
			perror("reset command load");
			exit(2);
		    }

	    expand = 1;

	    fscanf(fl, " ");
	    fscanf(fl, "%c", &zone_table[zon].cmd[cmd_no].command);

	    if (zone_table[zon].cmd[cmd_no].command == 'S')
		break;

	    if (zone_table[zon].cmd[cmd_no].command == '*') {
		expand = 0;
		fgets(buf, 80, fl);
		continue;
	    }

	    fscanf(fl, " %d %d %d", &tmp, &zone_table[zon].cmd[cmd_no].arg1,
		&zone_table[zon].cmd[cmd_no].arg2);

	    zone_table[zon].cmd[cmd_no].if_flag = tmp;

	    if (zone_table[zon].cmd[cmd_no].command == 'M' ||
		    zone_table[zon].cmd[cmd_no].command == 'O' ||
		    zone_table[zon].cmd[cmd_no].command == 'E' ||
		    zone_table[zon].cmd[cmd_no].command == 'P' ||
		    zone_table[zon].cmd[cmd_no].command == 'D')
		fscanf(fl, " %d", &zone_table[zon].cmd[cmd_no].arg3);

	    fgets(buf, 80, fl);

	    cmd_no++;
	}
	zon++;
    }
    top_of_zone_table = --zon;
    free(check);
    fclose(fl);
}
 */

/*************************************************************************
*  procedures for resetting, both play-time and boot-time      *
*********************************************************************** */

#define ZO_DEAD  999

/* update zone ages, queue for reset if necessary, and dequeue when possible */
void zone_update(void)
{
    int i;
    struct reset_q_element *update_u, *temp;

    /* enqueue zones */

    for (i = 0; i <= top_of_zone_table; i++) {
	if (zone_table[i].age < (zone_table[i].lifespan * regen_time_percent / 100) &&
	    zone_table[i].reset_mode)
	    (zone_table[i].age)++;
	else if (zone_table[i].age < ZO_DEAD && zone_table[i].reset_mode) {
	    /* enqueue zone */

	    CREATE(update_u, struct reset_q_element, 1);

	    update_u->zone_to_reset = i;
	    update_u->next = 0;

	    if (!reset_q.head)
		reset_q.head = reset_q.tail = update_u;
	    else {
		reset_q.tail->next = update_u;
		reset_q.tail = update_u;
	    }

	    zone_table[i].age = ZO_DEAD;
	}
    }

    /* dequeue zones (if possible) and reset */

    for (update_u = reset_q.head; update_u; update_u = update_u->next)
	if (zone_table[update_u->zone_to_reset].reset_mode == 2 ||
	    is_empty(update_u->zone_to_reset)) {
	    reset_zone(update_u->zone_to_reset);

	    /* dequeue */

	    if (update_u == reset_q.head)
		reset_q.head = reset_q.head->next;
	    else {
		for (temp = reset_q.head; temp->next != update_u;
		     temp = temp->next) ;

		if (!update_u->next)
		    reset_q.tail = temp;

		temp->next = update_u->next; 
	    }

	    free(update_u);
	    break;
	}
} 


#ifdef SPLIT_ZONE_SYSTEM

/* index : real number */
struct char_data *get_mobile_index(int index)
{
    struct char_data *ch;

    /* NOTE: BUG NOT FIXED!!  Some times, infinite loop on index == 0 . */ 
    /* NOTE: DEBUG: This was Temorary FIX */
    /* if( index == 0 ) return(NULL); */

    for (ch = character_list; ch; ch = ch->next) {
	if (IS_NPC(ch) && ch->nr == index) {
	    return ch;
	}
    } 
    /* error */
    return NULL;
}

/* #define ZCMD zone_table[zone].cmd[cmd_no] */

void reset_zone(int zone)
{
    int cmd_no;
    char last_cmd = 1;
    struct char_data *mob = NULL;
    struct obj_data *obj, *obj_to;

    /* item regen !!! */
    /* if (regen == 1) then regen all items. */
    int regen;
    int real_load;
    char buf[MAX_BUFSIZ];
    struct reset_com ZCMD;

    regen = (number(0, regen_time / zone_table[zone].lifespan) == 0);

    real_load = 0;
    for (cmd_no = 0;; cmd_no++) {
	ZCMD = zone_table[zone].cmd[cmd_no];
	if (ZCMD.command == 'S')
	    break;
	if (last_cmd || !ZCMD.if_flag)
	    switch (ZCMD.command) {
	    case 'M':	/* read a mobile */
		if (mob_index[ZCMD.arg1].number < ZCMD.arg2) {
		    mob = read_mobile(ZCMD.arg1, REAL);
		    char_to_room(mob, ZCMD.arg3);
		    last_cmd = 'M';
		    if (regen)
			mob->regened = 1;
		    real_load = 1;
		}
		else {
		    mob = get_mobile_index(ZCMD.arg1);
		    if (mob == NULL)
			last_cmd = 0;
		    else {
			if (mob->regened)
			    last_cmd = 0;
			else {
			    last_cmd = 'M';
			    if (regen)
				/* NOTE: Not newly regened, just refreshed */
				mob->regened = 2;
			    real_load = 0;
			}
		    }
		}
		break;

	    case 'O':	/* read an object */
		/* if (obj_index[ZCMD.arg1].number < ZCMD.arg2) */
		if (ZCMD.arg3 >= 0) {
		    if (!get_obj_in_list_num(ZCMD.arg1, 
		    		world[ZCMD.arg3].contents)) {
			obj = read_object(ZCMD.arg1, REAL);
			if ((obj->obj_flags.type_flag == ITEM_KEY) ||
			(!IS_SET(obj->obj_flags.extra_flags, ITEM_NOLOAD)) ||
			    (regen == 1))
			    /* (number(1,100) <= regen_percent ) ) */
			{
			    obj_to_room(obj, ZCMD.arg3);
			    last_cmd = 'O';
			}
			else
			    extract_obj(obj);
		    }
		}
		else {
		    obj = read_object(ZCMD.arg1, REAL);
		    if ((obj->obj_flags.type_flag == ITEM_KEY) ||
			(!IS_SET(obj->obj_flags.extra_flags, ITEM_NOLOAD)) ||
			(regen == 1))
			/* ( number(1,100) <= regen_percent ) ) */
		    {

			obj->in_room = NOWHERE;
			last_cmd = 'O';
		    }
		    else
			extract_obj(obj);
		}
		break;

	    case 'P':	/* object to object */
		/* if (obj_index[ZCMD.arg1].number < ZCMD.arg2 && */
		if ( (last_cmd == 'O' || last_cmd == 'P' ||
			last_cmd == 'E' || last_cmd == 'G')) {
		    obj = read_object(ZCMD.arg1, REAL);
		    if ((obj->obj_flags.type_flag == ITEM_KEY) ||
			(!IS_SET(obj->obj_flags.extra_flags, ITEM_NOLOAD)) ||
			(regen == 1))
			/* (number(1,100) <= regen_percent ) ) */
		    {
			obj_to = get_obj_num(ZCMD.arg3);
			if (obj_to) {
			    obj_to_obj(obj, obj_to);
			    last_cmd = 'P';
			}
			else
			    extract_obj(obj);
		    }
		    else
			extract_obj(obj);
		}
		break;

	    case 'G':	/* obj_to_char */
		/* if (obj_index[ZCMD.arg1].number < ZCMD.arg2 && */
		if ( (last_cmd == 'M' || last_cmd == 'G' || last_cmd == 'E')) {
		    obj = read_object(ZCMD.arg1, REAL);
		    if ((obj->obj_flags.type_flag == ITEM_KEY) ||
			(!IS_SET(obj->obj_flags.extra_flags, ITEM_NOLOAD)))
			/* (number(1,100) <= regen_percent ) ) */
		    {
			if (mob && real_load) {
			    obj_to_char(obj, mob);
			    last_cmd = 'G';
			}
			else
			    extract_obj(obj);
		    }
		    else
			extract_obj(obj);
		}
		break;

	    case 'E':	/* object to equipment list */
		/* if (obj_index[ZCMD.arg1].number < ZCMD.arg2 && */
		if ( (last_cmd == 'M' || last_cmd == 'G' || last_cmd == 'E')) {
		    obj = read_object(ZCMD.arg1, REAL);
		    if (obj->obj_flags.type_flag == ITEM_KEY)
			/* ( !IS_SET(obj->obj_flags.extra_flags,ITEM_NOLOAD) )
			   ) ( number(1,100) <= regen_percent ) ) */
		    {
			if (mob && real_load) {
			    equip_char(mob, obj, ZCMD.arg3);
			    last_cmd = 'E';
			}
			else
			    extract_obj(obj);
		    }
		    else if (regen == 1) {
			if (mob) {
			    equip_char(mob, obj, ZCMD.arg3);
			    last_cmd = 'E';
			}
			else
			    extract_obj(obj);
		    }
		    else
			extract_obj(obj);
		}
		break;

	    case 'D':	/* set state of door */
		switch (ZCMD.arg3) {
		case 0:
		    REMOVE_BIT(world[ZCMD.arg1].dir_option[ZCMD.arg2]->exit_info,
			       EX_LOCKED);
		    REMOVE_BIT(world[ZCMD.arg1].dir_option[ZCMD.arg2]->exit_info,
			       EX_CLOSED);
		    break;
		case 1:
		    SET_BIT(world[ZCMD.arg1].dir_option[ZCMD.arg2]->exit_info,
			    EX_CLOSED);
		    REMOVE_BIT(world[ZCMD.arg1].dir_option[ZCMD.arg2]->exit_info,
			       EX_LOCKED);
		    break;
		case 2:
		    SET_BIT(world[ZCMD.arg1].dir_option[ZCMD.arg2]->exit_info,
			    EX_LOCKED);
		    SET_BIT(world[ZCMD.arg1].dir_option[ZCMD.arg2]->exit_info,
			    EX_CLOSED); 
		    break;
		}
		last_cmd = 'D';
		break;

	    default:
		sprintf(buf, "Undefd cmd in reset table; zone %d cmd %d.\n\r",
			zone, cmd_no);
		log(buf);
		exit(2);
		break;
	    }
	else
	    last_cmd = 0;

    }

    zone_table[zone].age = 0;
}

#else	// SPLIT_ZONE_SYSTEM


#define ZCMD zone_table[zone].cmd[cmd_no]

/* execute the reset command table of a given zone */
void reset_zone(int zone)
{
    int cmd_no, last_cmd = 1;
    char buf[256];
    struct char_data *f, *mob;
    struct obj_data *obj, *obj_to;

    for (cmd_no = 0;; cmd_no++) {
	if (ZCMD.command == 'S')
	    break;

	if (last_cmd || !ZCMD.if_flag)
	    switch (ZCMD.command) {
	    case 'M':	/* read a mobile */
		if (mob_index[ZCMD.arg1].number < ZCMD.arg2) {
		    mob = read_mobile(ZCMD.arg1, REAL);
		    char_to_room(mob, ZCMD.arg3);
		    last_cmd = 1;
		}
		else
		    last_cmd = 0;
		break;

	    case 'O':	/* read an object */
		if (obj_index[ZCMD.arg1].number <
		    ZCMD.arg2)
		    if (ZCMD.arg3 >= 0) {
			if (!get_obj_in_list_num(
				    ZCMD.arg1, world[ZCMD.arg3].contents)) {
			    obj = read_object(ZCMD.arg1, REAL);
			    obj_to_room(obj, ZCMD.arg3);
			    last_cmd = 1;
			}
			else
			    last_cmd = 0;
		    }
		    else {
			obj = read_object(ZCMD.arg1, REAL);
			obj->in_room = NOWHERE;
			last_cmd = 1;
		    }
		else
		    last_cmd = 0;
		break;

	    case 'P':	/* object to object */
		obj = get_obj_num(ZCMD.arg1);
		obj_to = get_obj_num(ZCMD.arg2);
		obj_to_obj(obj, obj_to);
		last_cmd = 1;
		break;

	    case 'G':	/* obj_to_char */
		obj = get_obj_num(ZCMD.arg1);
		mob = get_char_num(ZCMD.arg2);
		obj_to_char(obj, mob);
		last_cmd = 1;
		break;

	    case 'E':	/* object to equipment list */
		obj = get_obj_num(ZCMD.arg1);
		mob = get_char_num(ZCMD.arg2);
		equip_char(mob, obj, ZCMD.arg3);
		last_cmd = 1;
		break;

	    case 'D':	/* set state of door */
		switch (ZCMD.arg3) {
		case 0:
		    REMOVE_BIT(world[ZCMD.arg1].dir_option[ZCMD.arg2]->exit_info,
			       EX_LOCKED);
		    REMOVE_BIT(world[ZCMD.arg1].dir_option[ZCMD.arg2]->exit_info,
			       EX_CLOSED);
		    break;
		case 1:
		    SET_BIT(world[ZCMD.arg1].dir_option[ZCMD.arg2]->exit_info,
			    EX_CLOSED);
		    REMOVE_BIT(world[ZCMD.arg1].dir_option[ZCMD.arg2]->exit_info,
			       EX_LOCKED);
		    break;
		case 2:
		    SET_BIT(world[ZCMD.arg1].dir_option[ZCMD.arg2]->exit_info,
			    EX_LOCKED);
		    SET_BIT(world[ZCMD.arg1].dir_option[ZCMD.arg2]->exit_info,
			    EX_CLOSED);
		    break;
		}
		break;

	    default:
		sprintf(buf, "Undefd cmd in reset table; zone %d cmd %d.\n\r",
			zone, cmd_no);
		log(buf);
		exit(2);
		break;
	    }
	else
	    last_cmd = 0;

    }

    zone_table[zone].age = 0;
}

#undef ZCMD

#endif	// SPLIT_ZONE_SYSTEM

/* for use in reset_zone; return TRUE if zone 'nr' is free of PC's  */
int is_empty(int zone_nr)
{
    struct descriptor_data *i;

    for (i = descriptor_list; i; i = i->next)
	if (!i->connected)
	    if (world[i->character->in_room].zone == zone_nr)
		return (0);

    return (1);
}


/* NOTE: NEW! Reset all zone at boot time.. It was part of boot_db() in db.c */
void reset_all_zone(void)
{
    int i;
    for (i = 0; i <= top_of_zone_table; i++) {
	fprintf(stderr, "Performing boot-time reset of %s (rooms %d-%d).\n",
		zone_table[i].name,
		(i ? (zone_table[i - 1].top + 1) : 0),
		zone_table[i].top);
	reset_zone(i);
    }
    reset_q.head = reset_q.tail = 0;
}


/* by atre */
#ifdef RESET_ZONE_BY_WIZARD

void do_resetzone(struct char_data *ch, char *argument, int cmd)
{
    char buf[255], buf2[255];
    int i;

    if (!ch)
	return;
    if (NOT_GOD(ch))
	return;
    if (!argument || !*argument) {
	send_to_char("Which zone do you want to reset?\n\r", ch);
	return;
    }
    while (isspace(*argument))
	argument++;
    if (strcmp(argument, "ALL") == 0) {
	for (i = 0; i <= top_of_zone_table; i++) {
	    sprintf(buf2, "Zone %s is now updating..\n\r",
		    zone_table[i].name);
	    send_to_char(buf2, ch);
	    reset_zone(i);
	}
	return;
    }

    if (strcmp(argument, "THIS") == 0) {
	i = world[ch->in_room].zone;
	sprintf(buf2, "Zone %s is now updating..\n\r",
		zone_table[i].name);
	send_to_char(buf2, ch);
	reset_zone(i);
	return;
    }

    for (i = 0; i <= top_of_zone_table; i++)
	if (strncmp(zone_table[i].name, argument, strlen(argument)) == 0) {
	    sprintf(buf2, "Zone %s is now updating..\n\r",
		    zone_table[i].name);
	    send_to_char(buf2, ch);
	    reset_zone(i);
	    break;
	}
}

void do_reloadzone(struct char_data *ch, char *argument, int cmd)
{
    char buf[255], buf2[255];
    int i;

    if (!ch)
	return;
    if (NOT_GOD(ch))
	return;
    if (!argument || !*argument) {
	send_to_char("Which zone do you want to reset?\n\r", ch);
	return;
    }
    while (isspace(*argument))
	argument++;
    if (strcmp(argument, "ALL") == 0) {
	for (i = 0; i <= top_of_zone_table; i++) {
	    sprintf(buf2, "Zone %s is now reloading..\n\r",
		    zone_table[i].name);
	    send_to_char(buf2, ch);
	    log(buf2);
	    load_zones(i);
	}
	return;
    }

    if (strcmp(argument, "THIS") == 0) {
	i = world[ch->in_room].zone;
	sprintf(buf2, "Zone %s is now reloading..\n\r",
		zone_table[i].name);
	send_to_char(buf2, ch);
	log(buf2);
	load_zones(i);
	return;
    }

    for (i = 0; i <= top_of_zone_table; i++)
	if (strncmp(zone_table[i].name, argument, strlen(argument)) == 0) {
	    sprintf(buf2, "Zone %s is now reloading..\n\r",
		    zone_table[i].name);
	    send_to_char(buf2, ch);
	    log(buf2);
	    load_zones(i);
	    break;
	}
}

#endif				/* RESET_ZONE_BY_WIZARD */

