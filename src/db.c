/**************************************************************************
*  file: db.c , Database module.                          Part of DIKUMUD *
*  Usage: Loading/Saving chars, booting world, resetting etc.             *
*  Copyright (C) 1990, 1991 - see 'license.doc' for complete information. *
***************************************************************************/

// $log$
// 

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <assert.h>
#include <time.h>

#include "structs.h"
#include "utils.h"
#include "db.h"
#include "comm.h"
#include "handler.h"
#include "limits.h"
#include "spells.h"
#include "memory.h"	/* memory managing - by cold */

#define	WORLD_ROOM_SIZE	4000	/* maximum number of room */
#define NEWRENTMAGIC 29922 /* magic number to tell new rent format from old */
#define KJHRENT      66666 /* kjh number to tell new rent format */
#define NEW_ZONE_SYSTEM
#define	NEW_RENT_SYSTEM	7777
#define	NEW_SYSTEM_BAG_START	"BAG"
#define	NEW_SYSTEM_BAG_END	"END"
#define STASH        "stash"


/**************************************************************************
*  declarations of most of the 'global' variables                         *
************************************************************************ */

struct room_data *world;              /* dyn alloc'ed array of rooms     */
int top_of_world = 0;                 /* ref to the top element of world */
struct obj_data  *object_list = 0;    /* the global linked list of obj's */
struct char_data *character_list = 0; /* global l-list of chars          */

struct zone_data *zone_table;         /* table of reset data             */
int top_of_zone_table = 0;
struct message_list fight_messages[MAX_MESSAGES]; /* fighting messages   */
struct player_index_element *player_table = 0; /* index to player file   */
int top_of_p_table = 0;               /* ref to top of table             */
int top_of_p_file = 0;

char credits[MAX_STRING_LENGTH];      /* the Credits List                */
char news[MAX_STRING_LENGTH];         /* the news                        */
char motd[MAX_STRING_LENGTH];         /* the messages of today           */
char help[MAX_STRING_LENGTH];         /* the main help page              */
char info[MAX_STRING_LENGTH];         /* the info text                   */
char *announce[MAX_ANNOUNCE] = {0, }; /* the announce text               */
char wizards[MAX_STRING_LENGTH];      /* the wizards text                */
char wizhelp[MAX_STRING_LENGTH];      /* the wizards help text           */
char *cookie[MAX_FORTUNE_COOKIE];     /* the fortuen cookie text         */
int  number_of_cookie = 0;            /* number of cookie */
int number_of_announce = 0;           /* number of announce */

FILE *mob_f,                          /* file containing mob prototypes  */
    *obj_f,                          /* obj prototypes                  */
    *help_fl;                        /* file for help texts (HELP <kwd>)*/

struct index_data *mob_index;         /* index table for mobile file     */
struct index_data *obj_index;         /* index table for object file     */
struct help_index_element *help_index = 0;

int top_of_mobt = 0;                  /* top of mobile index table       */
int top_of_objt = 0;                  /* top of object index table       */
int top_of_helpt;                     /* top of help index table         */

struct time_info_data time_info;  /* the infomation about the time   */
struct weather_data weather_info;  /* the infomation about the weather */

struct reset_q_type	reset_q ;	/* for zone reset queue : cold */

/* local procedures */
void boot_zones(void);
void setup_dir(FILE *fl, int room, int dir);
void allocate_room(int new_top);
void boot_world(void);
struct index_data *generate_indices(FILE *fl, int *top);
void build_player_index(void);
void char_to_store(struct char_data *ch, struct char_file_u *st);
void store_to_char(struct char_file_u *st, struct char_data *ch);
int is_empty(int zone_nr);
void reset_zone(int zone);
int file_to_string(char *name, char *buf);
void renum_world(void);
void renum_zone_table(void);
void reset_time(void);
void clear_char(struct char_data *ch);

/* external refs */
extern struct descriptor_data *descriptor_list;
void load_messages(void);
void weather_and_time ( int mode );
void assign_command_pointers ( void );
void assign_spell_pointers ( void );
void log(char *str);
int dice(int number, int size);
int number(int from, int to);
void boot_social_messages(void);
struct help_index_element *build_help_index(FILE *fl, int *num);
int implementor(char *name);
void init_mbox() ;
void init_market() ;
void init_command_block() ;	/* in new_interp.c */


/*************************************************************************
*  routines for booting the system                                       *
*********************************************************************** */


/* body of the booting system */
void boot_db(void)
{
    int i;
    extern int no_specials;
	extern void init_exp_table();

    log("Boot db -- BEGIN.");

	log("Initializing memory manager") ;
	init_memory_manager() ;

    log("Resetting the game time:");
    reset_time();

    log("Reading news, credits, help-page, info, wizards, cookie and motd.");
    file_to_string(NEWS_FILE, news);
    file_to_string(CREDITS_FILE, credits);
    file_to_string(MOTD_FILE, motd);
    file_to_string(HELP_PAGE_FILE, help);
    file_to_string(INFO_FILE, info);
    file_to_string(WIZARDS_FILE, wizards);
    file_to_announce(ANNOUNCE_FILE, announce);
    file_to_cookie(FORTUNE_COOKIE_FILE, cookie);

    log("Opening mobile, object and help files.");
    if (!(mob_f = fopen(MOB_FILE, "r")))
    {
        perror("boot:mob file");
        exit(1);
    }

    if (!(obj_f = fopen(OBJ_FILE, "r")))
    {
        perror("boot:object file");
        exit(1);
    }
    if (!(help_fl = fopen(HELP_KWRD_FILE, "r")))
        log("   Could not open help file.");
    else 
        help_index = build_help_index(help_fl, &top_of_helpt);


    log("Loading zone table.");
    boot_zones();

    log("Loading rooms.");
    boot_world();
    log("Renumbering rooms.");
    renum_world();

    log("Generating index tables for mobile files.");
    mob_index = generate_indices(mob_f, &top_of_mobt);
    log("Generating index tables for object files.");
    obj_index = generate_indices(obj_f, &top_of_objt);
      
    log("Renumbering zone table.");
    renum_zone_table();

    log("Generating player index.");
    build_player_index();

    log("Loading fight messages.");
    load_messages();

    log("Loading social messages.");
    boot_social_messages();

    log("Assigning function pointers:");
    if (!no_specials)
    {
        log("   Mobiles.");
        assign_mobiles();
        log("   Objects.");
        assign_objects();
        log("   Room.");
        assign_rooms();
    }

    log("   Commands.");  
    assign_command_pointers();
    log("   Spells.");
    assign_spell_pointers();

    for (i = 0; i <= top_of_zone_table; i++) {
        fprintf(stderr, "Performing boot-time reset of %s (rooms %d-%d).\n",
                zone_table[i].name,
                (i ? (zone_table[i - 1].top + 1) : 0),
                zone_table[i].top);
        reset_zone(i);
    }
    reset_q.head = reset_q.tail = 0;

    log("Mail system initialing") ;
    init_mbox() ;	/* init mail box system */

	log("Quest initialization") ;
	if ( init_quest() < 0 ) {
		log("Quest init: failed") ;
		abort() ;
    }

	log("Init market : gomulsang") ;
	init_market() ;

	log("Init apartment : apartment") ;
	init_apartment() ;

	log("Init new command search block") ;
	init_command_block() ;

	log("Rebalancing exp table") ;
	init_exp_table();

    log("Boot db -- DONE.");
}

/* reset the time in the game from file */
void reset_time(void)
{
    char buf[MAX_STRING_LENGTH];
    struct time_info_data mud_time;

    long beginning_of_time = 650336715;
    struct time_info_data mud_time_passed(time_t t2, time_t t1);
    time_info = mud_time_passed(time(0), beginning_of_time);

    switch(time_info.hours){
    case 0 :
    case 1 :
    case 2 :
    case 3 :
    case 4 : 
    {
        weather_info.sunlight = SUN_DARK;
        break;
    }
    case 5 :
    {
        weather_info.sunlight = SUN_RISE;
        break;
    }
    case 6 :
    case 7 :
    case 8 :
    case 9 :
    case 10 :
    case 11 :
    case 12 :
    case 13 :
    case 14 :
    case 15 :
    case 16 :
    case 17 :
    case 18 :
    case 19 :
    case 20 :
    {
        weather_info.sunlight = SUN_LIGHT;
        break;
    }
    case 21 :
    {
        weather_info.sunlight = SUN_SET;
        break;
    }
    case 22 :
    case 23 :
    default :
    {
        weather_info.sunlight = SUN_DARK;
        break;
    }
    }

    sprintf(buf,"   Current Gametime: %dH %dD %dM %dY.",
            time_info.hours, time_info.day,
            time_info.month, time_info.year);
    log(buf);

    weather_info.pressure = 960;
    if ((time_info.month>=7)&&(time_info.month<=12))
        weather_info.pressure += dice(1,50);
    else
        weather_info.pressure += dice(1,80);

    weather_info.change = 0;

    if (weather_info.pressure<=980)
        weather_info.sky = SKY_LIGHTNING;
    else if (weather_info.pressure<=1000)
        weather_info.sky = SKY_RAINING;
    else if (weather_info.pressure<=1020)
        weather_info.sky = SKY_CLOUDY;
    else weather_info.sky = SKY_CLOUDLESS;
}

/* generate index table for the player file */
void build_player_index(void)
{
    int nr = -1, i;
    struct char_file_u dummy;
    FILE *fl;

    if (!(fl = fopen(PLAYER_FILE, "rb+")))
    {
        perror("build player index");
        exit(1);
    }
    for (; !feof(fl);) {
        fread(&dummy, sizeof(struct char_file_u), 1, fl);
        if (!feof(fl))   /* new record */
        {
            /* Create new entry in the list */
            if (nr == -1) {
                CREATE(player_table, 
                       struct player_index_element, 1);
                nr = 0;
            }  else {
                if (!(player_table = (struct player_index_element *)
                      realloc(player_table, (++nr + 1) *
                              sizeof(struct player_index_element))))
                {
                    perror("generate index");
                    exit(1);
                }
            }
    
            player_table[nr].nr = nr;

            /*
              create(player_table[nr].name, char,
              strlen(dummy.name) + 1);
            */
            player_table[nr].name = malloc_string_type(strlen(dummy.name)+1) ;

            for (i = 0; *(player_table[nr].name + i) = 
                     LOWER(*(dummy.name + i)); i++);
        }
    }

    fclose(fl);

    top_of_p_table = nr;

    top_of_p_file = top_of_p_table;
}
  

/* generate index table for object or monster file */
struct index_data *generate_indices(FILE *fl, int *top)
{
    int i = 0;
    struct index_data *index;
    long pos;
    char buf[82];

    rewind(fl);

    for (;;)
    {
        if (fgets(buf, 81, fl))
        {
            if (*buf == '#')
            {
                /* allocate new cell */
        
                if (!i)             /* first cell */
                    CREATE(index, struct index_data, 1);
                else
                    if (!(index = 
                          (struct index_data*) realloc(index, 
                                                       (i + 1) * sizeof(struct index_data))))
                    {
                        perror("load indices");
                        exit(1);
                    }
                sscanf(buf, "#%d", &index[i].virtual);
                index[i].pos = ftell(fl);
                index[i].number = 0;
                index[i].func = 0;
                i++;
            }
            else 
                if (*buf == '$')  /* EOF */
                    break;
        }
        else
        {
            perror("generate indices");
            exit(1);
        }
    }
    *top = i - 2;
    return(index);
}




/* load the rooms */
void boot_world(void)
{
    struct extra_descr_data *new_descr;
    FILE *fl;
    int room_nr = 0, zone = 0, dir_nr, virtual_nr, flag, tmp;
	int	n_room ;
    char *temp, chk[50], buf[BUFSIZ] ;

    world = 0;
    character_list = 0;
    object_list = 0;
	n_room = 0 ;
  
    if (!(fl = fopen(WORLD_FILE, "r")))
    {
        perror("fopen");
        log("boot_world: could not open world file.");
        exit(1);
    }

    do
    {
        fscanf(fl, " #%d\n", &virtual_nr);
        if ( ++ n_room >= WORLD_ROOM_SIZE ) {
            log("boot_world: number of world over flow. WORLD_ROOM_SIZE") ;
            exit(1) ;
		}
        temp = fread_string(fl);
        if (flag = (*temp != '$'))  /* a new record to be read */
        {
            allocate_room(room_nr);
            world[room_nr].number = virtual_nr;
            world[room_nr].name = temp;
            world[room_nr].description = fread_string(fl);

            if (top_of_zone_table >= 0)
            {
                fscanf(fl, " %*d ");
                /* OBS: Assumes ordering of input rooms */
                if (world[room_nr].number <= (zone ? zone_table[zone-1].top : -1))
                {
                    fprintf(stderr, "Room nr %d is below zone %d.\n",
                            room_nr, zone);
                    exit(1);
                }
                while (world[room_nr].number > zone_table[zone].top)
                    if (++zone > top_of_zone_table)
                    {
                        fprintf(stderr, "Room %d is outside of any zone.\n",
                                virtual_nr);
                        exit(1);
                    }
                world[room_nr].zone = zone;
            }
            fscanf(fl, " %d ", &tmp);
            world[room_nr].room_flags = tmp;
            fscanf(fl, " %d ", &tmp);

            if ( tmp < 0 || tmp > 9 ) {
                /* sector !!! */
                sprintf(buf, "Room #%d irregal sector type %d.", 
					world[room_nr].number, tmp) ;
                log(buf) ;
                tmp = 1 ;
			}
            world[room_nr].sector_type = tmp;

            world[room_nr].funct = 0;
            world[room_nr].contents = 0;
            world[room_nr].people = 0;
            world[room_nr].light = 0; /* Zero light sources */

            for (tmp = 0; tmp <= 5; tmp++)
                world[room_nr].dir_option[tmp] = 0;

            world[room_nr].ex_description = 0;

            for (;;)
            {
                fscanf(fl, " %s \n", chk);

                if (*chk == 'D')  /* direction field */
                    setup_dir(fl, room_nr, atoi(chk + 1));
                else if (*chk == 'E')  /* extra description field */
                {
                    /*
                      create(new_descr, struct extra_descr_data, 1);
                    */
                    new_descr = (struct extra_descr_data *) malloc_general_type (
                        MEMORY_EXTRA_DESCR_DATA) ;
                    new_descr->keyword = fread_string(fl);
                    new_descr->description = fread_string(fl);
                    new_descr->next = world[room_nr].ex_description;
                    world[room_nr].ex_description = new_descr;
                }
                else if (*chk == 'S')  /* end of current room */
                    break;
            }
            
            room_nr++;
        }
    }
    while (flag);

    free_string_type(temp);  /* cleanup the area containing the terminal $  */

    fclose(fl);
    top_of_world = --room_nr;
}

struct room_data world_large[WORLD_ROOM_SIZE] ;
void allocate_room(int new_top)
{
    struct room_data *new_world;
/*

  if (new_top)
  { 
*/
	/*
      free_general_type((char *) new_world, MEMORY_ROOM_DATA) ;
      new_world = (struct room_data *) malloc((new_top + 1) *
      sizeof(struct room_data)) ;
	*/
/*
  free(new_world) ;
  new_world = (struct room_data *) malloc((new_top + 1) *
  sizeof(struct room_data)) ;
  if (new_world == NULL) 
  {
  perror("alloc_room");
  exit(1);
  } 
  }
  else {
*/
	/*
      new_world = (struct room_data *) malloc_general_type(MEMORY_ROOM_DATA) ;
      bzero(new_world, sizeof(struct room_data)) ;
	*/
	/*
      create
	*/
/*
  }
*/

	/*
      world = new_world;
	*/
    world = world_large ;
}






/* read direction data */
void setup_dir(FILE *fl, int room, int dir)
{
    int tmp;

    CREATE(world[room].dir_option[dir], 
           struct room_direction_data, 1);

    world[room].dir_option[dir]->general_description =
        fread_string(fl);
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

void renum_zone_table(void)
{
    int zone, comm;

    for (zone = 0; zone <= top_of_zone_table; zone++)
        for (comm = 0; zone_table[zone].cmd[comm].command != 'S'; comm++)
            switch(zone_table[zone].cmd[comm].command) {
            case 'M':
                zone_table[zone].cmd[comm].arg1 =
                    real_mobile(zone_table[zone].cmd[comm].arg1);
                zone_table[zone].cmd[comm].arg3 = 
                    real_room(zone_table[zone].cmd[comm].arg3);
                break;
            case 'O':
                zone_table[zone].cmd[comm].arg1 = 
                    real_object(zone_table[zone].cmd[comm].arg1);
                if (zone_table[zone].cmd[comm].arg3 != NOWHERE)
                    zone_table[zone].cmd[comm].arg3 =
                        real_room(zone_table[zone].cmd[comm].arg3);
                break;
            case 'G':
                zone_table[zone].cmd[comm].arg1 =
                    real_object(zone_table[zone].cmd[comm].arg1);
                break;
            case 'E':
                zone_table[zone].cmd[comm].arg1 =
                    real_object(zone_table[zone].cmd[comm].arg1);
                break;
            case 'P':
                zone_table[zone].cmd[comm].arg1 =
                    real_object(zone_table[zone].cmd[comm].arg1);
                zone_table[zone].cmd[comm].arg3 =
                    real_object(zone_table[zone].cmd[comm].arg3);
                break;          
            case 'D':
                zone_table[zone].cmd[comm].arg1 =
                    real_room(zone_table[zone].cmd[comm].arg1);
                break;
            }
}


/* load the zone table and command tables */
void boot_zones(void)
{
    FILE *fl;
    int zon = 0, cmd_no = 0, ch, expand, tmp;
    char *check, buf[81];

    if (!(fl = fopen(ZONE_FILE, "r")))
    {
        perror("boot_zones");
        exit(1);
    }

    for (;;)
    {
        fscanf(fl, " #%*d\n");
        check = fread_string(fl);

        if (*check == '$')
            break;    /* end of file */

        /* alloc a new zone */

        if (!zon)
            CREATE(zone_table, struct zone_data, 1);
        else
            if (!(zone_table = (struct zone_data *) realloc(zone_table,
                                                            (zon + 1) * sizeof(struct zone_data))))
            {
                perror("boot_zones realloc");
                exit(1);
            }

        zone_table[zon].name = check;
        fscanf(fl, " %d ", &zone_table[zon].top);
        fscanf(fl, " %d ", &zone_table[zon].lifespan);
        fscanf(fl, " %d ", &zone_table[zon].reset_mode);

        /* read the command table */

        cmd_no = 0;

        for (expand = 1;;)
        {
            if (expand)
                if (!cmd_no)
                    CREATE(zone_table[zon].cmd, struct reset_com, 1);
                else
                    if (!(zone_table[zon].cmd =
                          (struct reset_com *) realloc(zone_table[zon].cmd, 
                                                       (cmd_no + 1) * sizeof(struct reset_com))))
                    {
                        perror("reset command load");
                        exit(1);
                    }

            expand = 1;

            fscanf(fl, " "); /* skip blanks */
            fscanf(fl, "%c", 
                   &zone_table[zon].cmd[cmd_no].command);
      
            if (zone_table[zon].cmd[cmd_no].command == 'S')
                break;

            if (zone_table[zon].cmd[cmd_no].command == '*')
            {
                expand = 0;
                fgets(buf, 80, fl); /* skip command */
                continue;
            }

            fscanf(fl, " %d %d %d", 
                   &tmp,
                   &zone_table[zon].cmd[cmd_no].arg1,
                   &zone_table[zon].cmd[cmd_no].arg2);

            zone_table[zon].cmd[cmd_no].if_flag = tmp;

            if (zone_table[zon].cmd[cmd_no].command == 'M' ||
                zone_table[zon].cmd[cmd_no].command == 'O' ||
                zone_table[zon].cmd[cmd_no].command == 'E' ||
                zone_table[zon].cmd[cmd_no].command == 'P' ||
                zone_table[zon].cmd[cmd_no].command == 'D')
                fscanf(fl, " %d", &zone_table[zon].cmd[cmd_no].arg3);

            fgets(buf, 80, fl);  /* read comment */

            cmd_no++;
        }
        zon++;
    }
    top_of_zone_table = --zon;
    free_string_type(check);
    fclose(fl);
}

#define BITSHIFT(c) (c - 'A')

#define FLAG(cr)  (BITSHIFT(cr) <= 25 ?  (1 << BITSHIFT(cr)) : \
		((BITSHIFT(cr)-6) < 31 ? (1 << (BITSHIFT(cr)-6)): 0))

long flag_convert(char *argument) {

	long flagval = 0;
	int i;
	char buf[52];

	strcpy(buf, argument);

	for (i = 0; i < strlen(argument); i++)
		if (buf[i] != '0')
			flagval += FLAG(buf[i]);
		else return(flagval);

	// sprintf(buf, "convert flag: %ld",flagval);
	// log(buf);
	return(flagval);
}

/*************************************************************************
*  procedures for resetting, both play-time and boot-time      *
*********************************************************************** */

/* read a mobile from MOB_FILE */
struct char_data *read_mobile(int nr, int type)
{
    int i, skill_nr;
    long tmp, tmp2, tmp3;
    struct char_data *mob;
    char chk[10], buf[100];
    char letter;
	char bitflag[255];
	extern byte saving_throws[4][5][IMO+10]; /* spell_parser.c */

    i = nr;
	if (nr <0 ) {
		sprintf(buf, "Mobile %d does not exist in database.", nr);
		log(buf);
		return(0);
	}
    if (type == VIRTUAL)
        if ((nr = real_mobile(nr)) < 0)
        {
            sprintf(buf, "Mobile (V) %d does not exist in database.", i);
			log(buf);
            return(0);
        }

    fseek(mob_f, mob_index[nr].pos, 0);

	/* create */
	mob = (struct char_data *) malloc_general_type(MEMORY_CHAR_DATA) ;
    clear_char(mob);

    /***** String data *** */
    
    mob->player.name = fread_string(mob_f);
    mob->player.short_descr = fread_string(mob_f);
    mob->player.long_descr = fread_string(mob_f);
    mob->player.description = fread_string(mob_f);
    mob->player.title = 0;

    /* *** Numeric data *** */

	fscanf(mob_f, "%s ", bitflag);
	if (isdigit(bitflag[0])) tmp = atol(bitflag);
	else {
		// log(mob->player.name);
		// log(bitflag);
		tmp = flag_convert(bitflag);
	}

/*  fscanf(mob_f, "%d ", &tmp); */
    mob->specials.act = tmp;
    SET_BIT(mob->specials.act, ACT_ISNPC);

	fscanf(mob_f, "%s ", bitflag);
	if (isdigit(bitflag[0])) tmp = atol(bitflag);
	else tmp = flag_convert(bitflag);

/*  fscanf(mob_f, " %d ", &tmp); */
    mob->specials.affected_by = tmp;

    fscanf(mob_f, " %d ", &tmp);
    mob->specials.alignment = tmp;

    fscanf(mob_f, " %c \n", &letter);

    if (letter == 'S') {
        /* The new easy monsters */
        mob->abilities.str   = 16;
        mob->abilities.intel = 12; 
        mob->abilities.wis   = 12;
        mob->abilities.dex   = 12;
        mob->abilities.con   = 12;

        fscanf(mob_f, " %ld ", &tmp);
        GET_LEVEL(mob) = tmp;
    
        fscanf(mob_f, " %ld ", &tmp);
        mob->points.hitroll = 20-tmp;
    
        fscanf(mob_f, " %ld ", &tmp);
        mob->points.armor = 10*tmp;

        fscanf(mob_f, " %ldd%ld+%ld ", &tmp, &tmp2, &tmp3);
        mob->points.max_hit = dice(tmp, tmp2)+tmp3;
        mob->points.hit = mob->points.max_hit;

        fscanf(mob_f, " %ldd%ld+%ld \n", &tmp, &tmp2, &tmp3);
        mob->points.damroll = tmp3;
        mob->specials.damnodice = tmp;
        mob->specials.damsizedice = tmp2;
        mob->points.mana = 10 + GET_LEVEL(mob)*3;
        mob->points.max_mana = 10 + GET_LEVEL(mob)*3;
        mob->points.move = 100 + GET_LEVEL(mob)*4 ;
        mob->points.max_move = 100 + GET_LEVEL(mob)*4 ;
        fscanf(mob_f, " %ld ", &tmp);
        mob->points.gold = tmp;
        fscanf(mob_f, " %ld \n", &tmp);
        GET_EXP(mob) = tmp;
        fscanf(mob_f, " %ld ", &tmp);
        mob->specials.position = tmp;
        fscanf(mob_f, " %ld ", &tmp);
        mob->specials.default_pos = tmp;
        fscanf(mob_f, " %ld \n", &tmp);
        mob->player.sex = tmp;
        mob->player.class = 0;
        mob->player.time.birth = time(0);
        mob->player.time.played  = 0;
        mob->player.time.logon  = time(0);
        mob->player.weight = 200;
        mob->player.height = 198;
        for (i = 0; i < 3; i++) {
            GET_COND(mob, i) = -1;
		}


		/* saving throw is set by char's saving throw 
			by jmjeong@oopsla.snu.ac.kr
		*/
        for (i = 0; i < 5; i++) {
			if (GET_LEVEL(mob) >= IMO+10) {
				mob->specials.apply_saving_throw[i] = 0;
			}
    		else if (IS_SET(mob->specials.act,ACT_NICE_THIEF) || 
       			IS_SET(mob->specials.act,ACT_THIEF)){
				/* thief */
				mob->specials.apply_saving_throw[i] = 
					saving_throws[2][i][GET_LEVEL(mob)];
			}
			else if (IS_SET(mob->specials.act,ACT_CLERIC) || 
						IS_SET(mob->specials.act,ACT_PALADIN) ||
						IS_SET(mob->specials.act,ACT_HELPER)){
				/* cleric */
				mob->specials.apply_saving_throw[i] = 
					saving_throws[1][i][GET_LEVEL(mob)];
			}
			else if (IS_SET(mob->specials.act,ACT_MAGE) ||
						IS_SET(mob->specials.act,ACT_DRAGON)) {
				/* mage */
				mob->specials.apply_saving_throw[i] = 
					saving_throws[0][i][GET_LEVEL(mob)];
			}
			else {
				/* warrior */
				mob->specials.apply_saving_throw[i] = 
					saving_throws[3][i][GET_LEVEL(mob)];
			}
		}
    } else {  /* The old monsters are down below here */
        fscanf(mob_f, " %ld ", &tmp);
        mob->abilities.str = tmp;
        fscanf(mob_f, " %ld ", &tmp);
        mob->abilities.intel = tmp; 
        fscanf(mob_f, " %ld ", &tmp);
        mob->abilities.wis = tmp;
        fscanf(mob_f, " %ld ", &tmp);
        mob->abilities.dex = tmp;
        fscanf(mob_f, " %ld \n", &tmp);
        mob->abilities.con = tmp;
        fscanf(mob_f, " %ld ", &tmp);
        fscanf(mob_f, " %ld ", &tmp2);
        mob->points.max_hit = number(tmp, tmp2);
        mob->points.hit = mob->points.max_hit;
        fscanf(mob_f, " %ld ", &tmp);
        mob->points.armor = 10*tmp;
        fscanf(mob_f, " %ld ", &tmp);
        mob->points.mana = tmp;
        mob->points.max_mana = tmp;
        fscanf(mob_f, " %ld ", &tmp);
        mob->points.move = tmp;    
        mob->points.max_move = tmp;
        fscanf(mob_f, " %ld ", &tmp);
        mob->points.gold = tmp;
        fscanf(mob_f, " %ld \n", &tmp);
        GET_EXP(mob) = tmp;
        fscanf(mob_f, " %ld ", &tmp);
        mob->specials.position = tmp;
        fscanf(mob_f, " %ld ", &tmp);
        mob->specials.default_pos = tmp;
        fscanf(mob_f, " %ld ", &tmp);
        mob->player.sex = tmp;
        fscanf(mob_f, " %ld ", &tmp);
        mob->player.class = tmp;

        fscanf(mob_f, " %ld ", &tmp);
        GET_LEVEL(mob) = tmp;

        fscanf(mob_f, " %ld ", &tmp);
        mob->player.time.birth = time(0);
        mob->player.time.played  = 0;
        mob->player.time.logon  = time(0);

        fscanf(mob_f, " %ld ", &tmp);
        mob->player.weight = tmp;

        fscanf(mob_f, " %ld \n", &tmp);
        mob->player.height = tmp;

        for (i = 0; i < 3; i++)
        {
            fscanf(mob_f, " %ld ", &tmp);
            GET_COND(mob, i) = tmp;
        }
        fscanf(mob_f, " \n ");

        for (i = 0; i < 5; i++)
        {
            fscanf(mob_f, " %ld ", &tmp);
            mob->specials.apply_saving_throw[i] = tmp;
        }

        fscanf(mob_f, " \n ");

        /* Set the damage as some standard 1d4 */
        mob->points.damroll = 0;
        mob->specials.damnodice = 1;
        mob->specials.damsizedice = 6;

        /* Calculate THAC0 as a formular of Level */
        mob->points.hitroll = MAX(1, GET_LEVEL(mob)-3);
    }

    mob->tmpabilities = mob->abilities;

    for (i = 0; i < MAX_WEAR; i++) /* Initialisering Ok */
        mob->equipment[i] = 0;

    mob->nr = nr;

    mob->desc = 0;

    if(IS_SET(mob->specials.act,ACT_FIGHTER) || 
       IS_SET(mob->specials.act,ACT_SPITTER) ||
       IS_SET(mob->specials.act,ACT_PALADIN)){
        mob->skills[SKILL_KICK].learned = 95;
        mob->skills[SKILL_BASH].learned = 95;
        mob->skills[SKILL_MULTI_KICK].learned = 95;
    }
    if(IS_SET(mob->specials.act, ACT_PALADIN)){
        mob->skills[SKILL_PARRY].learned = MIN(70,GET_LEVEL(mob)+dice(10,10));
    }
    if(IS_SET(mob->specials.act, ACT_THIEF)) {
        mob->skills[SKILL_PARRY].learned = MIN(95,GET_LEVEL(mob)+dice(10,10));
        mob->skills[SKILL_FLASH].learned = 95;
        mob->skills[SKILL_BACKSTAB].learned = 95;
    }
	if (GET_LEVEL(mob) > 49)
		mob->skills[SKILL_PARRY].learned = 95;

    mob->regeneration = 0;

	/*
	if (mob->points.exp > GET_LEVEL(mob)*1500) {
		char buf[200];
		sprintf(buf, "%s(%d)(%d,%d)(%d)(%dd%d)(%d,%d)", 
				mob->player.short_descr, GET_LEVEL(mob), 
        		mob->points.hitroll,
        		mob->points.damroll,
        		mob->points.armor,
        		mob->specials.damnodice,
        		mob->specials.damsizedice,
				mob->points.gold/1000,
				mob->points.exp/1000);
		log(buf);
	}
	*/

    /* insert in list */

    mob->next = character_list;
    character_list = mob;

    mob_index[nr].number++;

    return(mob);
}


/* read an object from OBJ_FILE */
struct obj_data *read_object(int nr, int type)
{
    struct obj_data *obj;
    int tmp, i;
    char chk[50], buf[100];
	char bitflag[255];
    struct extra_descr_data *new_descr;

    i = nr;
	if (nr <0 ) {
		sprintf(buf, "Mobile %d does not exist in database.", nr);
		log(buf);
		return(0);
	}
    if (type == VIRTUAL)
        if ((nr = real_object(nr)) < 0)
        {
            sprintf(buf, "Object (V) %d does not exist in database.", i);
			log(buf);
            return(0);
        }

    fseek(obj_f, obj_index[nr].pos, 0);

    /* create(obj, struct obj_data, 1); */
	obj = (struct obj_data *) malloc_general_type(MEMORY_OBJ_DATA) ;

    clear_object(obj);

    /* *** string data *** */

    obj->name = fread_string(obj_f);
    obj->short_description = fread_string(obj_f);
    obj->description = fread_string(obj_f);
    obj->action_description = fread_string(obj_f);

    /* *** numeric data *** */

    fscanf(obj_f, " %d ", &tmp);
    obj->obj_flags.type_flag = tmp;

	fscanf(obj_f, "%s ", bitflag);
	if (isdigit(bitflag[0])) tmp = atol(bitflag);
	else tmp = flag_convert(bitflag);
/*    fscanf(obj_f, " %d ", &tmp);  */
    obj->obj_flags.extra_flags = tmp;	/* == GET_OBJ_STAT(obj) */

	fscanf(obj_f, "%s \n", bitflag);
	if (isdigit(bitflag[0])) tmp = atol(bitflag);
	else tmp = flag_convert(bitflag);
/*    fscanf(obj_f, " %d ", &tmp);  */
    obj->obj_flags.wear_flags = tmp;

    fscanf(obj_f, " %d ", &tmp);
    obj->obj_flags.value[0] = tmp;
    fscanf(obj_f, " %d ", &tmp);
    obj->obj_flags.value[1] = tmp;
    fscanf(obj_f, " %d ", &tmp);
    obj->obj_flags.value[2] = tmp;
    fscanf(obj_f, " %d ", &tmp);
    obj->obj_flags.value[3] = tmp;
    fscanf(obj_f, " %d ", &tmp);
    obj->obj_flags.weight = tmp;
    fscanf(obj_f, " %d \n", &tmp);
    obj->obj_flags.cost = tmp;
    fscanf(obj_f, " %d \n", &tmp);
    obj->obj_flags.gpd = tmp;

    /* *** extra descriptions *** */

    obj->ex_description = 0;

    while (fscanf(obj_f, " %s \n", chk), *chk == 'E')
    {
		/*
          create(new_descr, struct extra_descr_data, 1);
		*/
        new_descr = (struct extra_descr_data *) malloc_general_type(
            MEMORY_EXTRA_DESCR_DATA) ;

        new_descr->keyword = fread_string(obj_f);
        new_descr->description = fread_string(obj_f);

        new_descr->next = obj->ex_description;
        obj->ex_description = new_descr;
    }

	/* first.. clear */
    for (i = 0 ; i < MAX_OBJ_AFFECT ; i++)
    {
        obj->affected[i].location = APPLY_NONE;
        obj->affected[i].modifier = 0;
    }

    for( i = 0 ; (i < MAX_OBJ_AFFECT) && (*chk == 'A') ; i++)
    {
        fscanf(obj_f, " %d ", &tmp);
        obj->affected[i].location = tmp;
        fscanf(obj_f, " %d \n", &tmp);
        obj->affected[i].modifier = tmp;
        fscanf(obj_f, " %s \n", chk);
    }

    obj->in_room = NOWHERE;
    obj->next_content = 0;
    obj->carried_by = 0;
	obj->rented_by = 0;
    obj->in_obj = 0;
    obj->contains = 0;
    obj->item_number = nr;  

    obj->next = object_list;
    object_list = obj;

    obj_index[nr].number++;


    return (obj);  
}




#define ZO_DEAD  999

/* update zone ages, queue for reset if necessary, and dequeue when possible */
void zone_update(void)
{
	int i ;
	struct reset_q_element *update_u, *temp;
    static int timer = 0;
    

	/* enqueue zones */

    if (((++timer * PULSE_ZONE) / PASSES_PER_SEC) >= 60) {
        timer = 0;
        /* one minute has passed */
        /* Not accurate unless PULSE_ZONE is a a multiple of PASSES_PER_SEC
           or a factor of 60
        */
           
        /* since one minute has passed, increment zone ages */
        for (i = 0; i <= top_of_zone_table; i++) {
            if ( zone_table[i].reset_mode == 0)	/* do not reset this zone */
                continue ;
            if (zone_table[i].age < zone_table[i].lifespan )
                (zone_table[i].age)++;
            else if (zone_table[i].age < ZO_DEAD ) {	/* not enqueued yet */
                /* enqueue zone */

                /* create(update_u, struct reset_q_element, 1); */
                /*
                  update_u = (struct reset_q_element *) malloc_string_type(
                  sizeof(struct reset_q_element)) ;
                */
                update_u = (struct reset_q_element *) malloc_general_type(
                    MEMORY_RESET_Q_ELEMENT) ;

                update_u->zone_to_reset = i;
                update_u->next = NULL ;

                if (!reset_q.head)	/* create */
                    reset_q.head = reset_q.tail = update_u;
                else {	/* insert to tail of reset_q */
                    reset_q.tail->next = update_u;
                    reset_q.tail = update_u;
                }

                zone_table[i].age = ZO_DEAD;	/* it's enqueued mark */
            }
        } /* end of for */
    }
    
	/* dequeue zones (if possible) and reset */

	for (update_u = reset_q.head; update_u; update_u = update_u->next) {
		if ( update_u->zone_to_reset > top_of_zone_table ) {
			log("zone_update: zone number is out of range. FATAL") ;
			update_u->zone_to_reset = 0 ;	/* it's limbo */
			update_u->next = NULL ;
			for (i = 0; i <= top_of_zone_table; i++) {
				if (zone_table[i].age >= ZO_DEAD) {
					zone_table[i].age = ZO_DEAD - 1 ;
                }
            }
			break ;
        }
		/* reset_mode 2 is reset unconditionally */
		/* if reset_mode is 1, then reset if that zone is empty */
		if (zone_table[update_u->zone_to_reset].reset_mode == 2 ||
			is_empty(update_u->zone_to_reset)) {
			reset_zone(update_u->zone_to_reset);

			/* dequeue */

			if (update_u == reset_q.head)
				reset_q.head = update_u->next;
			else {
				for (temp = reset_q.head; temp->next != update_u;
                     temp = temp->next);

				if (!update_u->next)	/* if the last element.. */
					reset_q.tail = temp;

				temp->next = update_u->next;
            }

			/*
              free_string_type((char *)update_u);
			*/
			free_general_type((char *)update_u, MEMORY_RESET_Q_ELEMENT) ;
			break;	/* only reset one zone -> it costs a lot of load */
        } 
    } /* end of for */
}


#ifdef NEW_ZONE_SYSTEM

#define ZCMD zone_table[zone].cmd[cmd_no]

/* execute the reset command table of a given zone */
void reset_zone(int zone)
{
    int cmd_no, last_cmd = 1;
    char buf[256];
    struct char_data *f, *mob;
    struct obj_data *obj, *obj_to;

    for (cmd_no = 0;;cmd_no++) {
        if (ZCMD.command == 'S')
            break;
        if (last_cmd || !ZCMD.if_flag)
            switch(ZCMD.command) {
            case 'M': /* read a mobile */
                if (mob_index[ZCMD.arg1].number < ZCMD.arg2) {
                    mob = read_mobile(ZCMD.arg1, REAL);
                    char_to_room(mob, ZCMD.arg3);
                    last_cmd = 1;
                } else
                    last_cmd = 0;
                break;

            case 'O': /* read an object */
                if (obj_index[ZCMD.arg1].number < ZCMD.arg2)
                    if (ZCMD.arg3 >= 0) {
                        if (!get_obj_in_list_num(ZCMD.arg1,world[ZCMD.arg3].contents)) {
                            obj = read_object(ZCMD.arg1, REAL);
                            obj_to_room(obj, ZCMD.arg3);
                            last_cmd = 1;
                        } else
                            last_cmd = 0;
                    } else {
                        obj = read_object(ZCMD.arg1, REAL);
                        obj->in_room = NOWHERE;
                        last_cmd = 1;
                    } else
                        last_cmd = 0;
                break;

            case 'P': /* object to object */
                if (obj_index[ZCMD.arg1].number < ZCMD.arg2) {
                    obj = read_object(ZCMD.arg1, REAL);
                    obj_to = get_obj_num(ZCMD.arg3);
                    obj_to_obj(obj, obj_to);
                    last_cmd = 1;
                }
                else
                    last_cmd = 0;
                break;

            case 'G': /* obj_to_char */
                if (obj_index[ZCMD.arg1].number < ZCMD.arg2)
                {    
                    obj = read_object(ZCMD.arg1, REAL);
                    obj_to_char(obj, mob);
                    last_cmd = 1;
                }
                else
                    last_cmd = 0;
                break;

            case 'E': /* object to equipment list */
                if (obj_index[ZCMD.arg1].number < ZCMD.arg2)
                {    
                    obj = read_object(ZCMD.arg1, REAL);
                    equip_char(mob, obj, ZCMD.arg3);
                    last_cmd = 1;
                }
                else
                    last_cmd = 0;
                break;

            case 'D': /* set state of door */
                switch (ZCMD.arg3)
                {
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
                last_cmd = 1;
                break;

            default:
                sprintf(buf, "Undefd cmd in reset table; zone %d cmd %d.\n\r",
                        zone, cmd_no);
                log(buf);
                exit(1);
                break;
            }
        else
            last_cmd = 0;

    }

    zone_table[zone].age = 0;
}

#undef ZCMD

#else


#define ZCMD zone_table[zone].cmd[cmd_no]

/* execute the reset command table of a given zone */
void reset_zone(int zone)
{
    int cmd_no, last_cmd = 1;
    char buf[256];
    struct char_data *f,*mob;
    struct obj_data *obj, *obj_to;

    for (cmd_no = 0;;cmd_no++)
    {
        if (ZCMD.command == 'S')
            break;

        if (last_cmd || !ZCMD.if_flag)
            switch(ZCMD.command)
            {
            case 'M': /* read a mobile */
                if (mob_index[ZCMD.arg1].number < ZCMD.arg2) {
                    mob = read_mobile(ZCMD.arg1, REAL);
                    char_to_room(mob, ZCMD.arg3);
                    last_cmd = 1;
                } else
                    last_cmd = 0;
                break;

            case 'O': /* read an object */
                if (obj_index[ZCMD.arg1].number <
                    ZCMD.arg2)
                    if (ZCMD.arg3 >= 0)
                    {
                        if (!get_obj_in_list_num(
                            ZCMD.arg1,world[ZCMD.arg3].contents))
                        {
                            obj = read_object(ZCMD.arg1, REAL);
                            obj_to_room(obj, ZCMD.arg3);
                            last_cmd = 1;
                        }
                        else
                            last_cmd = 0;
                    }
                    else
                    {
                        obj = read_object(ZCMD.arg1, REAL);
                        obj->in_room = NOWHERE;
                        last_cmd = 1;
                    }
                else
                    last_cmd = 0;
                break;

            case 'P': /* object to object */
                obj = get_obj_num(ZCMD.arg1);
                obj_to = get_obj_num(ZCMD.arg2);
                obj_to_obj(obj, obj_to);
                last_cmd = 1;
                break;

            case 'G': /* obj_to_char */
                obj = get_obj_num(ZCMD.arg1);
                mob = get_char_num(ZCMD.arg2);
                obj_to_char(obj, mob);
                last_cmd = 1;
                break;

            case 'E': /* object to equipment list */
                obj = get_obj_num(ZCMD.arg1);
                mob = get_char_num(ZCMD.arg2);
                equip_char(mob, obj, ZCMD.arg3);
                last_cmd = 1;
                break;

            case 'D': /* set state of door */
                switch (ZCMD.arg3)
                {
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
                exit(1);
                break;
            }
        else
            last_cmd = 0;

    }

    zone_table[zone].age = 0;
}

#undef ZCMD

#endif

/* for use in reset_zone; return TRUE if zone 'nr' is free of PC's  */
int is_empty(int zone_nr)
{
    struct descriptor_data *i;

    for (i = descriptor_list; i; i = i->next)
        if (i->connected == CON_PLYNG)
            if (world[i->character->in_room].zone == zone_nr)
                return(0);

    return(1);
}





/*************************************************************************
*  stuff related to the save/load player system                  *
*********************************************************************** */

/* Load a char, TRUE if loaded, FALSE if not */
int load_char(char *name, struct char_file_u *char_element)
{
    FILE *fl;
    int player_i;

    int find_name(char *name);

    if ((player_i = find_name(name)) >= 0) {
        if (!(fl = fopen(PLAYER_FILE, "r"))) {
            perror("Opening player file for reading. (db.c, load_char)");
            exit(1);
        }
        fseek(fl, (long) (player_table[player_i].nr *
                          sizeof(struct char_file_u)), 0);
        fread(char_element, sizeof(struct char_file_u), 1, fl);
        fclose(fl);
        return(player_i);
    } else
        return(-1);
}


/* copy data from the file structure to a char struct */  
void store_to_char(struct char_file_u *st, struct char_data *ch)
{
    int i;

    GET_SEX(ch) = st->sex;
    // GET_CLASS(ch) = st->class;
    ch->player.class = st->class; /* origial set */
    if ( st->level >= IMO+3 && !implementor(st->name))
        GET_LEVEL(ch) = 1 ;
    else
        GET_LEVEL(ch) = st->level;
    ch->player.short_descr = 0;
    ch->player.long_descr = 0;
    if (*st->title) {
        /*
          create(ch->player.title, char, strlen(st->title) + 1);
        */
		ch->player.title = malloc_string_type(strlen(st->title) + 1) ;
        strcpy(ch->player.title, st->title);
    }
    else
        GET_TITLE(ch) = 0;
    if (*st->description) {
        /*
          create(ch->player.description, char, strlen(st->description) + 1);
        */
        ch->player.description = malloc_string_type(strlen(st->description) + 1);
        strcpy(ch->player.description, st->description);
    }
    else
        ch->player.description = 0;
    ch->player.time.birth = st->birth;
    ch->player.time.played = st->played;
    ch->player.time.logon  = time(0);
    ch->player.weight = st->weight;
    ch->player.height = st->height;
    ch->abilities = st->abilities;
    ch->tmpabilities = st->abilities;
    ch->points = st->points;
    for (i = 0; i <= MAX_SKILLS - 1; i++)
        ch->skills[i] = st->skills[i];
    ch->specials.spells_to_learn = st->spells_to_learn;
    ch->specials.alignment    = st->alignment;
    ch->specials.act          = st->act;
    ch->specials.carry_weight = 0;
    ch->specials.carry_items  = 0;
    ch->specials.damnodice = 1;
    ch->specials.damsizedice = 2;

#ifdef DEATHFIGHT
    ch->points.armor          = 51-st->level;
#else
    ch->points.armor          = 101-st->level;
#endif 
    ch->points.hitroll        = 0;
    ch->points.damroll        = 0;

    ch->regeneration          = (int)st->level/10;
	/*
      create(GET_NAME(ch), char, strlen(st->name) +1);
	*/
	GET_NAME(ch) = malloc_string_type(strlen(st->name) + 1) ;
    strcpy(GET_NAME(ch), st->name);
    for(i = 0; i <= 4; i++)
        ch->specials.apply_saving_throw[i] = 0;
    for(i = 0; i <= 2; i++)
        GET_COND(ch, i) = st->conditions[i];
    /* Add all spell effects */
    for(i=0;i<MAX_AFFECT;i++){
        if (st->affected[i].type)
            affect_to_char(ch, &st->affected[i]);
    }
    ch->in_room = st->load_room;
    affect_total(ch);
    ch->bank = st->bank;
    GET_HIT(ch)=MIN(GET_HIT(ch)+(time(0)-st->last_logon)/SECS_PER_MUD_HOUR*5,GET_MAX_HIT(ch));
    GET_MANA(ch)=MIN(GET_MANA(ch)+(time(0)-st->last_logon)/SECS_PER_MUD_HOUR*3,GET_MAX_MANA(ch));
    GET_MOVE(ch)=MIN(GET_MOVE(ch)+(time(0)-st->last_logon)/SECS_PER_MUD_HOUR*10,GET_MAX_MOVE(ch));

    ch->quest[0] = st->quest[0] ;		/* restore quest bits */
    ch->quest[1] = st->quest[1] ;
} /* store_to_char */

void store_to_char_for_transform(struct char_file_u *st, struct char_data *ch)
{
    int i;

    GET_SEX(ch) = st->sex;
    // GET_CLASS(ch) = st->class;
    ch->player.class = st->class;

    if (st->level >= IMO+3 && !implementor(st->name))
        GET_LEVEL(ch) = 1 ;
    else
        GET_LEVEL(ch) = st->level;
    ch->player.short_descr = 0;
    ch->player.long_descr = 0;
    if (*st->title) {
        /*
          recreate(ch->player.title, char, strlen(st->title) + 1);
        */
        free_string_type(ch->player.title) ;
        ch->player.title = malloc_string_type(strlen(st->title) + 1) ;
        strcpy(ch->player.title, st->title);
    }
    else
        GET_TITLE(ch) = 0;
    if (*st->description) {
        /*
          recreate(ch->player.description, char, strlen(st->description) + 1);
        */
        free_string_type(ch->player.description) ;
        ch->player.description = malloc_string_type(strlen(st->description) + 1) ;
        strcpy(ch->player.description, st->description);
    }
    else
        ch->player.description = 0;
    ch->player.time.birth = st->birth;
    ch->player.time.played = st->played;
    ch->player.time.logon  = time(0);
    ch->player.weight = st->weight;
    ch->player.height = st->height;
    ch->abilities = st->abilities;
    ch->tmpabilities = st->abilities;
    ch->points = st->points;
    for (i = 0; i <= MAX_SKILLS - 1; i++)
        ch->skills[i] = st->skills[i];
    ch->specials.spells_to_learn = st->spells_to_learn;
    ch->specials.alignment    = st->alignment;
    ch->specials.act          = st->act;
    ch->specials.carry_weight = 0;
    ch->specials.carry_items  = 0;
#ifdef DEATHFIGHT
    ch->points.armor          = 51-st->level;
#else
    ch->points.armor          = 101-st->level;
#endif 
    ch->points.hitroll        = 0;
    ch->points.damroll        = 0;
	/*
      recreate(GET_NAME(ch), char, strlen(st->name) +1);
	*/
	free_string_type(GET_NAME(ch)) ;
	GET_NAME(ch) = malloc_string_type(strlen(st->name) + 1) ;
    strcpy(GET_NAME(ch), st->name);
    for(i = 0; i <= 4; i++)
        ch->specials.apply_saving_throw[i] = 0;
    for(i = 0; i <= 2; i++)
        GET_COND(ch, i) = st->conditions[i];
    /* Add all spell effects */
    for(i=0;i<MAX_AFFECT;i++){
        if (st->affected[i].type)
            affect_to_char(ch, &st->affected[i]);
    }
    affect_total(ch);
    ch->bank = st->bank;

    ch->quest[0] = st->quest[0] ;		/* restore quest bit */
    ch->quest[1] = st->quest[1] ;
} /* store_to_char_for_transform */


/* copy vital data from a players char-structure to the file structure */
void char_to_store(struct char_data *ch, struct char_file_u *st)
{
    int i;
    struct affected_type *af;
    struct obj_data *char_eq[MAX_WEAR];

    /* Unaffect everything a character can be affected by */

    for(i=0; i<MAX_WEAR; i++) {
        if (ch->equipment[i])
            char_eq[i] = unequip_char(ch, i);
        else
            char_eq[i] = 0;
    }

    for(af = ch->affected, i = 0; i<MAX_AFFECT; i++) {
        if (af) {
            st->affected[i] = *af;
            st->affected[i].next = 0;
            /* subtract effect of the spell or the effect will be doubled */
            affect_modify( ch, st->affected[i].location,
                           st->affected[i].modifier,
                           st->affected[i].bitvector, FALSE);
            af = af->next;
        } else {
            st->affected[i].type = 0;  /* Zero signifies not used */
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
    st->birth      = ch->player.time.birth;
    st->played     = ch->player.time.played;
    st->played    += (long) (time(0) - ch->player.time.logon);
    st->last_logon = time(0);
    ch->player.time.played = st->played;
    ch->player.time.logon = time(0);
    st->weight   = GET_WEIGHT(ch);
    st->height   = GET_HEIGHT(ch);
    st->sex      = GET_SEX(ch);
    st->class    = (ch)->player.class;
    st->level    = GET_LEVEL(ch);
    st->abilities = ch->abilities;
    st->points    = ch->points;
    st->alignment       = ch->specials.alignment;
    st->spells_to_learn = ch->specials.spells_to_learn;
    st->act             = ch->specials.act;
#ifdef DEATHFIGHT
    st->points.armor   = 50;
#else
    st->points.armor   = 100;
#endif
    st->points.hitroll =  0;
    st->points.damroll =  0;
    if (GET_TITLE(ch))
        strcpy(st->title, GET_TITLE(ch));
    else
        *st->title = '\0';
    if (ch->player.description)
        strcpy(st->description, ch->player.description);
    else
        *st->description = '\0';
    for (i = 0; i <= MAX_SKILLS - 1; i++)
        st->skills[i] = ch->skills[i];
    strcpy(st->name, GET_NAME(ch) );
    for(i = 0; i <= 2; i++)
        st->conditions[i] = GET_COND(ch, i);
    for(af = ch->affected, i = 0; i<MAX_AFFECT; i++) {
        if (af) {
            /* Add effect of the spell or it will be lost */
            /* When saving without quitting               */
            affect_modify( ch, st->affected[i].location,
                           st->affected[i].modifier,
                           st->affected[i].bitvector, TRUE);
            af = af->next;
        }
    }
    for(i=0; i<MAX_WEAR; i++) {
        if (char_eq[i])
            equip_char(ch, char_eq[i], i);
    }
    affect_total(ch);
    st->bank = ch->bank;

    st->quest[0] = ch->quest[0] ;		/* save quest bits */
    st->quest[1] = ch->quest[1] ;

} /* Char to store */

/* create a new entry in the in-memory index table for the player file */
int create_entry(char *name)
{
    int i, pos;
    struct player_index_element tmp;

    if (top_of_p_table == -1) {
        CREATE(player_table, struct player_index_element, 1);
        top_of_p_table = 0;
    } else if (!(player_table = (struct player_index_element *) 
                 realloc(player_table, sizeof(struct player_index_element) * 
                         (++top_of_p_table + 1)))) {
        perror("create entry");
        exit(1);
    }
	/*
      create(player_table[top_of_p_table].name, char , strlen(name) + 1);
	*/
    player_table[top_of_p_table].name = malloc_string_type(strlen(name) + 1) ;

    /* copy lowercase equivalent of name to table field */
    for (i = 0; *(player_table[top_of_p_table].name + i) = 
             LOWER(*(name + i)); i++);
    player_table[top_of_p_table].nr = top_of_p_table;
    return (top_of_p_table);
}
    
/* write the vital data of a player to the player file */
void save_char(struct char_data *ch, sh_int load_room)
{
    struct char_file_u st;
    FILE *fl;
    char mode[4];
    int expand;

    if (IS_NPC(ch) || !ch->desc)
        return;
    if (expand = (ch->desc->pos > top_of_p_file)) {
        strcpy(mode, "a");
        top_of_p_file++;
    } else
        strcpy(mode, "r+");
    char_to_store(ch, &st);
    st.load_room = load_room;
    strcpy(st.pwd, ch->desc->pwd);
    if (!(fl = fopen(PLAYER_FILE, mode))) {
        perror("save char");
        exit(1);
    }
    if (!expand)
        fseek(fl, ch->desc->pos * sizeof(struct char_file_u), 0);
    fwrite(&st, sizeof(struct char_file_u), 1, fl);
    fclose(fl);
}

/* for possible later use with qsort */
int compare(struct player_index_element *arg1, struct player_index_element 
            *arg2)
{
    return (str_cmp(arg1->name, arg2->name));
}

/************************************************************************
*  procs of a (more or less) general utility nature      *
********************************************************************** */


/* read and allocate space for a '~'-terminated string from a given file */
char *fread_string(FILE *fl)
{
    char buf[MAX_STRING_LENGTH], tmp[MAX_STRING_LENGTH];
    char *rslt;
    register char *point;
    int flag;

    bzero(buf, MAX_STRING_LENGTH);
    do
    {
        if (!fgets(tmp, MAX_STRING_LENGTH, fl))
        {
            perror("fread_str");
            exit(1);
        }
        if (strlen(tmp) + strlen(buf) > MAX_STRING_LENGTH)
        {
            log("fread_string: string too large (db.c)");
            buf[70]=0;
            fprintf(stderr,"%s\n",buf);
            exit(1);
        }
        else
            strcat(buf, tmp);

        for (point = buf + strlen(buf) - 2; point >= buf && isspace(*point);
             point--);    
        if (flag = (*point == '~'))
            if (*(buf + strlen(buf) - 3) == '\n')
            {
                *(buf + strlen(buf) - 2) = '\r';
                *(buf + strlen(buf) - 1) = '\0';
            }
            else
                *(buf + strlen(buf) -2) = '\0';
        else
        {
            *(buf + strlen(buf) + 1) = '\0';
            *(buf + strlen(buf)) = '\r';
        }
    }
    while (!flag);

    /* do the allocate boogie  */

    if (strlen(buf) > 0)
    {
        /*
          create(rslt, char, strlen(buf) + 1);
        */
        rslt = malloc_string_type(strlen(buf) + 1) ;
        strcpy(rslt, buf);
    }
    else
        rslt = 0;
    return(rslt);
}


/* release memory allocated for a char struct */
void free_char(struct char_data *ch)
{
    struct affected_type *af;

	if (GET_NAME(ch))
		free_string_type(GET_NAME(ch));

	if (ch->player.title)
		free_string_type(ch->player.title);
	if (ch->player.short_descr)
		free_string_type(ch->player.short_descr);
	if (ch->player.long_descr)
		free_string_type(ch->player.long_descr);
	if(ch->player.description)
		free_string_type(ch->player.description);

    for (af = ch->affected; af; af = af->next) 
        affect_remove(ch, af);

	if ( free_general_type((char *)ch, MEMORY_CHAR_DATA) < 0 ) {
		log("free_char: free failed") ;
    }
}


/* release memory allocated for an obj struct */
void free_obj(struct obj_data *obj)
{
    struct extra_descr_data *this, *next_one;

	if ( obj->name)
		free_string_type(obj->name);
	if(obj->description)
		free_string_type(obj->description);
	if(obj->short_description)
		free_string_type(obj->short_description);
	if(obj->action_description)
		free_string_type(obj->action_description);

    for( this = obj->ex_description ; (this != 0);this = next_one ) {
        next_one = this->next;
        if(this->keyword)
            free_string_type(this->keyword);
        if(this->description)
            free_string_type(this->description);
        free_general_type((char *)this, MEMORY_EXTRA_DESCR_DATA);
    }

	if ( free_general_type((char *)obj, MEMORY_OBJ_DATA) < 0 ) {
		log("free_obj: free failed") ;
    }
}


/* read contents of a text file, and place in buf */
int file_to_string(char *name, char *buf)
{
    FILE *fl;
    char tmp[200];

    *buf = '\0';

    if (!(fl = fopen(name, "r")))
    {
        perror("file_to_string");
        *buf = 0 ;
        return(-1);
    }

    do
    {
        fgets(tmp, 199, fl);

        if (!feof(fl))
        {
            if (strlen(buf) + strlen(tmp) + 2 > MAX_STRING_LENGTH)
            {
                log("fl->strng: string too big (db.c, file_to_string)");
                buf[25]='\0';
                log(buf);
                *buf = '\0';
                return(-1);
            }

            strcat(buf, tmp);
            *(buf + strlen(buf) + 1) = '\0';
            *(buf + strlen(buf)) = '\r';
        }
    }
    while (!feof(fl));

    fclose(fl);

    return(0);
}



/* clear some of the the working variables of a char */
void reset_char(struct char_data *ch)
{
    int i;

    for (i = 0; i < MAX_WEAR; i++) /* Initialisering */
        ch->equipment[i] = 0;
    ch->followers = 0;
    ch->master = 0;
    ch->carrying = 0;
    ch->next = 0;
    ch->next_fighting = 0;
    ch->next_in_room = 0;
    ch->specials.fighting = 0;
    ch->specials.position = POSITION_STANDING;
    ch->specials.default_pos = POSITION_STANDING;
    ch->specials.carry_weight = 0;
    ch->specials.carry_items = 0;
    if (GET_HIT(ch) <= 0)
        GET_HIT(ch) = 1;
    if (GET_MOVE(ch) <= 0)
        GET_MOVE(ch) = 1;
    if (GET_MANA(ch) <= 0)
        GET_MANA(ch) = 1;
	
}



/* clear ALL the working variables of a char and do NOT free any space alloc'ed*/
void clear_char(struct char_data *ch)
{
    bzero(ch, sizeof(struct char_data));

    ch->in_room = NOWHERE;
    ch->specials.was_in_room = NOWHERE;
    ch->specials.position = POSITION_STANDING;
    ch->specials.default_pos = POSITION_STANDING;
#ifdef DEATHFIGHT
    GET_AC(ch) = 50; /* Basic Armor */
#else
    GET_AC(ch) = 100; /* Basic Armor */
#endif 
}


void clear_object(struct obj_data *obj)
{
    bzero(obj, sizeof(struct obj_data));

    obj->item_number = -1;
    obj->in_room    = NOWHERE;
}




/* initialize a new character only if class is set */
void init_char(struct char_data *ch)
{
    int i;

    /* *** if this is our first player --- he be God *** */

    if(top_of_p_table < 0)
    {
        GET_EXP(ch) = 7000000;
        GET_LEVEL(ch) = (IMO+3);
    }
    set_title(ch);

    ch->player.short_descr = 0;
    ch->player.long_descr = 0;
    ch->player.description = 0;

    ch->player.time.birth = time(0);
    ch->player.time.played = 0;
    ch->player.time.logon = time(0);

    GET_STR(ch) = 9;
    GET_INT(ch) = 9;
    GET_WIS(ch) = 9;
    GET_DEX(ch) = 9;
    GET_CON(ch) = 9;

    /* make favors for sex */
    if (ch->player.sex == SEX_MALE) {
        ch->player.weight = number(120,180);
        ch->player.height = number(160,200);
    } else {
        ch->player.weight = number(100,160);
        ch->player.height = number(150,180);
    }

    ch->points.mana = GET_MAX_MANA(ch);
    ch->points.hit = GET_MAX_HIT(ch);
    ch->points.move = GET_MAX_MOVE(ch);
#ifdef DEATHFIGHT
    ch->points.armor = 50;
#else
    ch->points.armor = 100;
#endif 

    for (i = 0; i <= MAX_SKILLS - 1; i++) {
        if (GET_LEVEL(ch) < (IMO+3)) {
            ch->skills[i].learned = 0;
            ch->skills[i].recognise = FALSE;
        }  else {
            ch->skills[i].learned = 100;
            ch->skills[i].recognise = FALSE;
        }
    }
    ch->specials.affected_by = 0;
    ch->specials.spells_to_learn = 0;
    for (i = 0; i < 5; i++)
        ch->specials.apply_saving_throw[i] = 0;
    for (i = 0; i < 3; i++)
        GET_COND(ch, i) = (GET_LEVEL(ch) == (IMO+3) ? -1 : 24);
}

/* returns the real number of the room with given virtual number */
int real_room(int virtual)
{
    int bot, top, mid;

    bot = 0;
    top = top_of_world;

    /* perform binary search on world-table */
    for (;;)
    {
        mid = (bot + top) / 2;

        if ((world + mid)->number == virtual)
            return(mid);
        if (bot >= top)
        {
            fprintf(stderr, "Room %d does not exist in database\n", virtual);
            return(-1);
        }
        if ((world + mid)->number > virtual)
            top = mid - 1;
        else
            bot = mid + 1;
    }
}






/* returns the real number of the monster with given virtual number */
int real_mobile(int virtual)
{
    int bot, top, mid;

    bot = 0;
    top = top_of_mobt;

    /* perform binary search on mob-table */
    for (;;)
    {
        mid = (bot + top) / 2;

        if ((mob_index + mid)->virtual == virtual)
            return(mid);
        if (bot >= top)
            return(-1);
        if ((mob_index + mid)->virtual > virtual)
            top = mid - 1;
        else
            bot = mid + 1;
    }
}


/* returns the real number of the object with given virtual number */
int real_object(int virtual)
{
    int bot, top, mid;

    bot = 0;
    top = top_of_objt;

    /* perform binary search on obj-table */
    for (;;)
    {
        mid = (bot + top) / 2;

        if ((obj_index + mid)->virtual == virtual)
            return(mid);
        if (bot >= top)
            return(-1);
        if ((obj_index + mid)->virtual > virtual)
            top = mid - 1;
        else
            bot = mid + 1;
    }
}
void move_stashfile(char *victim)	/* move file.x to file.x.y */
{
    char sf1[100],sf2[100],name[16];
    int i;

    strcpy(name,victim);
    for(i=0;name[i];++i)
        if(isupper(name[i]))
            name[i]=tolower(name[i]);
    sprintf(sf1,"%s/%c/%s.x",STASH,name[0],name);
    sprintf(sf2,"%s/%c/%s.x.y",STASH,name[0],name);
    rename(sf1,sf2);
}

/* If success return NULL, else return (-1) */
/* make new stash file for a new player */
int stash_char_new(struct char_data *ch)
{
	char	stashfile[100], name[80];
	FILE	*fl;
	int	i ;

	if( IS_NPC(ch)) {
		log("Is new player NPC ? - strange") ;
		return (-1) ;
    }
	if ( !ch->desc ) {
		log("stash_char_new: no descriptor") ;
		return (-1);
    }
	strcpy(name, GET_NAME(ch));
	for( i = 0 ; name[i] ; ++i)
		if(isupper(name[i]))
			name[i] = tolower(name[i]);
	sprintf(stashfile,"%s/%c/%s.x",STASH,name[0],name);

	if (!(fl = fopen(stashfile, "w"))) {	/* init stash file */
		log("Can't open stash file") ;
		perror("init PC's stash");
		return (-1);
    }
	/* cybrent
       fprintf(fl,"%d\n",KJHRENT);
	*/
	fprintf(fl,"%d\n", NEW_RENT_SYSTEM);
	fclose(fl) ;
	return 0 ;
}

/*  if success return NULL, if fails return -1  */
int stash_char(struct char_data *ch, char *filename)
{
	struct obj_data *p;
	char stashfile[100],name[80];
	char	buf[BUFSIZ] ;
	FILE *fl;
	int i,j;
	void stash_contents(FILE *fl, struct obj_data *p);

	if( IS_NPC(ch)) {
		/* log("saving NPC - ignored") ; */
		return 0 ;
    }
	if ( !ch->desc ) {
		/* this is caused when who forced rent by idling check */
		/* log("stash_char: no descriptor") ; */
		return 0 ;
    }
    strcpy(name,filename ? filename : GET_NAME(ch));
    for(i=0;name[i];++i)
        if(isupper(name[i]))
            name[i]=tolower(name[i]);
    sprintf(stashfile,"%s/%c/%s.x",STASH,name[0],name);
    if (!(fl = fopen(stashfile, "w"))) {		/* remove all data if exist */
		sprintf(buf, "stash_char : Can't write '%s'", stashfile) ;
		log(buf) ;
		return (-1);
    }
    fprintf(fl,"%d\n", NEW_RENT_SYSTEM);
    for(i=0;i<MAX_WEAR;++i)
        if(p=ch->equipment[i]){
			p->wear = i+1;  /* by Marx for auto retrieve*/
            stash_contents(fl, p);
			p->wear = 0;  /* by Marx for auto retrieve*/
        }
    if(ch->carrying)
        stash_contents(fl,ch->carrying);
	if (ch->rent_items)
		stash_contents(fl, ch->rent_items) ;
    fclose(fl);

	return 0 ;	/* successful return */
}

/*  if success return NULL, if fails return -1  */
int stash_charrent(struct char_data *ch)
{
	char stashfile[100],name[80];
	char	buf[BUFSIZ] ;
	FILE *fp;
	int i,j;
	void stash_contents(FILE *fp, struct obj_data *p);

	if(IS_NPC(ch))
		return 0 ;

	if ( !ch->desc ) {
		sprintf(buf, "stash_charrent : %s has no descriptor", GET_NAME(ch)) ;
		log(buf) ;
		return (-1) ;
    }

	strcpy(name, GET_NAME(ch));
	for( i = 0 ; name[i] ; i++ )
		if(isupper(name[i]))
			name[i]=tolower(name[i]);
	sprintf(stashfile,"%s/%c/%s.x",STASH,name[0],name);
	if (!(fp = fopen(stashfile, "w"))) {		/* remove all data if exist */
		sprintf(buf, "stash_charrent : Can't write '%s'", stashfile) ;
		log(buf) ;
		return (-1);
    }
/*
  fprintf(fp, "%d\n", KJHRENT);
*/
	fprintf(fp,"%d\n", NEW_RENT_SYSTEM);
	if (ch->rent_items)
		stash_contents(fp, ch->rent_items) ;
	fclose(fp);

	return 0 ;	/* successful return */
}

/* save one item into stashfile. if item has contents, save it also */
void stash_contents(FILE *fl, struct obj_data *p)
{
    struct obj_data *pc, *pp;
    int j;

	for ( pp = p ; pp != 0 ; pp = pp->next_content) {
		if(pp->obj_flags.type_flag != ITEM_KEY &&
           pp->obj_flags.type_flag != ITEM_BROKEN &&
           !IS_OBJ_STAT(pp, ITEM_NO_RENT) ) {
			fprintf(fl,"%d %d %d", obj_index[pp->item_number].virtual,
                    pp->obj_flags.extra_flags, pp->obj_flags.timer);
			for(j=0;j<4;++j)
				fprintf(fl," %d",pp->obj_flags.value[j]);
			for(j=0;j<2;j++)
				fprintf(fl, " %d %d", pp->affected[j].location,
                        pp->affected[j].modifier ) ;
			fprintf(fl, " %d", pp->wear); /* by Marx for autowear */
			fprintf(fl,"\n");
        }
		if(pc=pp->contains) {
			fprintf(fl, "%s\n", NEW_SYSTEM_BAG_START) ;
			stash_contents(fl,pc);
			fprintf(fl, "%s\n", NEW_SYSTEM_BAG_END) ;
        }
    }
}

/* Read stash file and load objects to player. and remove stash files */
void unstash_char(struct char_data *ch, char *filename)
{
	int unstash_contents(FILE *fp, struct obj_data *bag) ;
	struct obj_data *oneline_to_obj(char *line) ;
	FILE	*fl;
	struct obj_data *obj, *old_obj ;
	int	i, n, newflag,	nread, dummy, tmp[10] ;
	char	stashfile[100], name[100];
	char	line[BUFSIZ], buf[BUFSIZ] ;

	if ( ch == NULL ) {
		log("unstash_char: null character's rent") ;
		return ;
    }

    strcpy(name,filename ? filename : GET_NAME(ch));
    for(i=0;name[i];++i)
        if(isupper(name[i]))
            name[i]=tolower(name[i]);
    sprintf(stashfile, "%s/%c/%s.x", STASH, name[0], name);
    if(!(fl=fopen(stashfile, "r"))){
        sprintf(stashfile,"%s/%c/%s.x.y",STASH,name[0],name);
        if(!(fl=fopen(stashfile, "r"))) {
            log("unstash_char: Fail to open stashfiles") ;
            perror("unstash_char") ;
            return;
		}
    }
	fgets(line, BUFSIZ-1, fl) ;
	if(sscanf(line, "%d", &n) <= 0) {	/* have no content */
		fclose(fl) ;
		return ;
    }

	newflag = 0;
	switch(n) {
    case NEWRENTMAGIC :	newflag = 1 ;
        break ;
    case KJHRENT :	newflag = 2 ;
        break ;
    case NEW_RENT_SYSTEM : newflag = 3 ;
        break ;
    default : sprintf(buf, "unstash_char:%s: Unknown rent system", name) ;
        log(buf) ;
        fclose(fl) ;
        return ;
    } ;

	if ( newflag == 3 ) {
		old_obj = NULL ;
		for (;;) {
			if ( fgets(line, BUFSIZ-1, fl) == NULL )	/* End of File */
				break ;
			sscanf(line, "%s", name) ;
			if ( strcmp(name, NEW_SYSTEM_BAG_END) == 0 ) {
				/* Hmm..strange.. */
				log("unstash_char: starange.. BAG END") ;
				continue ;
            }
			else if ( strcmp(name, NEW_SYSTEM_BAG_START) == 0 ) {
				if ( old_obj == NULL ) {
					log("unstash_char: no BAG but contents!! - ignore") ;
					continue ;
                } 
				else if ( unstash_contents (fl, old_obj) < 0 )
					break ;	/* it's the end of file */
            }
			else {
				obj = oneline_to_obj(line) ;
				if ( obj ) {
					old_obj = obj ;
					obj_to_charrent(obj, ch) ;
                }
            }
        }
    }
	else {
		for(;;){
			if ( fgets(line, BUFSIZ-1, fl) == NULL )	/* End of File */
				break ;
			nread = sscanf(line, "%d %d %d %d %d %d %d %d %d", &n, &tmp[0],
                           &tmp[1], &tmp[2], &tmp[3], &tmp[4], &tmp[5], &tmp[6], &tmp[7]) ;
			if (nread <= 0)
				break;
			else if (nread < 5) {
				sprintf(buf, "unstash_char:%s: rent file corrupted (< 5)", name) ;
				log(buf) ;
				break ;
            }

			if ((obj=read_object(n,VIRTUAL)) == NULL ) {	/* no such item */
				sprintf(buf, "unstash_char: no such item %d", n) ;
				log(buf) ;
				continue ;
            }

			/* cyb  if(n < 1000) continue;	??? I can't understand this ??? */
			obj->obj_flags.value[0]=tmp[0];
			obj->obj_flags.value[1]=tmp[1];
			obj->obj_flags.value[2]=tmp[2];
			obj->obj_flags.value[3]=tmp[3];

			if (newflag == 2 ) {
				if ( nread < 9 ) {
					sprintf(buf, "unstash_char:%s: rent file corrupted (< 9)", name) ;
					log(buf) ;
					continue ;
                }
				obj->affected[0].location=tmp[4];
				obj->affected[0].modifier=tmp[5];
				obj->affected[1].location=tmp[6];
				obj->affected[1].modifier=tmp[7];
            }

            /* obj_to_char(obj,ch);	 for new rent system */
			obj_to_charrent(obj, ch) ;
		}
	}
    fclose(fl);
/* don't do this...  wipe_stash(name);	 delete file.x and file.x.y */
}

int unstash_contents(FILE *fp, struct obj_data *bag)
{
	struct obj_data *oneline_to_obj(char *line) ;
	struct obj_data	*old_obj, *obj ;
	char	line[BUFSIZ], name[BUFSIZ] ;

	old_obj = NULL ;
	for (;;) {
		if ( fgets(line, BUFSIZ-1, fp) == NULL )	/* End of File */
			return (-1) ;
		sscanf(line, "%s", name) ;
		if ( strcmp(name, NEW_SYSTEM_BAG_END) == 0 )
			return 0 ;
		else if ( strcmp(name, NEW_SYSTEM_BAG_START) == 0 ) {
			if ( old_obj == NULL ) {
				log("unstash_contents: strange.. no bag but items? - ignore") ;
				continue ;
            }
			else if ( unstash_contents (fp, old_obj) < 0 ) {	/* end of file */
				return (-1) ;
            }
        }
		else {
			obj = oneline_to_obj(line) ;
			if ( obj ) {
				old_obj = obj ;
				obj_to_obj(obj, bag) ;
            }
        }
    }
	return 0 ;
}

struct obj_data *oneline_to_obj(char *line)
{
	struct obj_data	*obj ;
	int	i, n, nread, tmp[10] ;
	char	buf[BUFSIZ] ;
	int wear = 0; /* by Marx for autowear */

	for ( i = 0 ; i < 10 ; i ++ )
		tmp[i] = 0 ;
	n = 0 ;

	nread = sscanf(line, "%d %d %d %d %d %d %d %d %d %d %d %d", &n, &tmp[0],
            &tmp[1], &tmp[2], &tmp[3], &tmp[4], &tmp[5], &tmp[6], &tmp[7],
            &tmp[8], &tmp[9], &wear) ;
	if (nread <= 0)	/* end of file */
		return NULL ;

	if (nread < 7) {
		log("oneline_to_obj: rent file corrupted (< 7)") ;
		return NULL ;
    }

	if ( nread < 11 ) {
		log("oneline_to_obj: rent file corrupted (< 11)") ;
		for ( i = nread-1 ; i < 10 ; i ++ ) ;
        	tmp[i] = 0 ;
    }

	if ((obj=read_object(n,VIRTUAL)) == NULL ) {	/* no such item */
		sprintf(buf, "oneline_to_obj: no such item %d", n) ;
		log(buf) ;
		return NULL ;
    }

	obj->obj_flags.extra_flags = tmp[0] ;
	obj->obj_flags.timer = tmp[1] ;
	obj->obj_flags.value[0]=tmp[2];
	obj->obj_flags.value[1]=tmp[3];
	obj->obj_flags.value[2]=tmp[4];
	obj->obj_flags.value[3]=tmp[5];
	obj->affected[0].location = tmp[6] ;
	obj->affected[0].modifier = tmp[7] ;
	obj->affected[1].location = tmp[8] ;
	obj->affected[1].modifier = tmp[9] ;
	obj->wear = wear; /* by Marx for autowear */

	return obj ;
}



void wipe_stash(char *filename)		/* clear file.x and file.x.y */
{
    char stashfile[100],name[50];
    int i;

    for(i=0;filename[i];++i)
        name[i]=isupper(filename[i]) ?
            tolower(filename[i]) : filename[i];
    name[i]=0;
    sprintf(stashfile,"%s/%c/%s.x",STASH,name[0],name);
    unlink(stashfile);
    strcat(stashfile,".y");
    unlink(stashfile);
}

void do_checkrent(struct char_data *ch,char *argument, int cmd)
{
    char stashfile[100], name[BUFSIZ], buf[MAX_STRING_LENGTH];
    FILE *fl;
    int i,j,n;

	send_to_char("Sorry, this command is not available now.\n\r", ch) ;
	return ;

	one_argument(argument,name);
	if(! *name) {
		send_to_char("usage: checkrent player_name\n\r", ch) ;
		return;
    }
	for(i=0;name[i];++i)
		if(isupper(name[i]))
			name[i]=tolower(name[i]);

	sprintf(stashfile,"%s/%c/%s.x.y",STASH,name[0],name);
	if(!(fl=fopen(stashfile,"r"))){
		sprintf(buf,"%s has nothing in rent.\n\r",name);
		send_to_char(buf,ch);
		return;
    }
	buf[0] = 0 ;
	for(i=j=0;;){
		if(fscanf(fl,"%d",&n) <= 0) break;
		if(n < 1000) continue;
		if(n > 9999) continue;
		++j;
		sprintf(buf+i,"%5d%c",n,(j==10) ? '\n' : ' ');
		if(j==10) j=0;
		i+=5;
    }
	fclose(fl);
	strcat(buf,"\n\r");
	send_to_char(buf,ch);
	return;
}

void do_extractrent(struct char_data *ch,char *argument, int cmd)
{
	struct char_data	*tmp_ch ;
	char name[MAX_STRING_LENGTH],buf[MAX_STRING_LENGTH];
	FILE *fl;

	send_to_char("Sorry, this command is not available now.\n\r", ch) ;
	return ;

	one_argument(argument,name);
	if(! *name) {
		send_to_char("usage: extractrent player_name\n\r", ch) ;
		return;
    }

	for (tmp_ch = character_list ; tmp_ch; tmp_ch = tmp_ch->next) {
		if ( !IS_NPC(tmp_ch) &&  isname(name, GET_NAME(tmp_ch)) ) {
			if ( ch == tmp_ch ) {
				send_to_char("You are already log on !\n\r", ch) ;
				return ;
            }
			if (GET_LEVEL(ch) < GET_LEVEL(tmp_ch)) {
				send_to_char("You can't touch master's rent file.\n\r", ch) ;
				return ;
            }
			send_to_char("Sorry, player is log on or not fully disconnected.\n\r",ch);
			return ;
        }
    }

	unstash_char(ch,name);
	send_to_char("OK, You must use replacerent to restore items.\n\r",ch);
	sprintf(buf,"%s grabbed rent for %s",GET_NAME(ch), name);
	log(buf);
}

void do_replacerent(struct char_data *ch,char *argument, int cmd)
{
	struct char_data	*tmp_ch ;
	char name[MAX_STRING_LENGTH],buf[MAX_STRING_LENGTH];
	FILE *fl;

	send_to_char("Sorry, this command is not available now.\n\r", ch) ;
	return ;

	one_argument(argument,name);
	if(! *name) {
		send_to_char("usage: replacerent player_name\n\r", ch) ;
		return;
    }
	for (tmp_ch = character_list ; tmp_ch; tmp_ch = tmp_ch->next) {
		if ( !IS_NPC(tmp_ch) && isname(name, GET_NAME(tmp_ch))) {
			if ( ch == tmp_ch ) {
				send_to_char("You are already log on !\n\r", ch) ;
				return ;
            }
			if (GET_LEVEL(ch) < GET_LEVEL(tmp_ch)) {
				send_to_char("You can't touch master's rent file.\n\r", ch) ;
				return ;
            }
			send_to_char("Sorry, player is log on or not fully disconnected.\n\r",ch);
			return ;
        }
    }
	if ( stash_char(ch, name) < 0 ) {
		send_to_char("Saving victim failed. This may cause RENT BUG.\n\r", ch) ;
    }
	else {
		move_stashfile(name);
    }
	sprintf(buf, "OK. You saved your possessions to %s\n\r", name);
	send_to_char(buf ,ch);
	sprintf(buf,"%s replaced rent for %s",GET_NAME(ch),name);
	log(buf);
}

void do_rent(struct char_data *ch, int cmd, char *arg)
{
    char buf[BUFSIZ];
    sh_int save_room;
    int i, cost = 0;
    void wipe_obj(struct obj_data *obj);
    void wipe_objrent(struct obj_data *obj);

    if (IS_NPC(ch))
        return;
    if(cmd){
        if(!IS_SET(world[ch->in_room].room_flags,RENT)){
            send_to_char("You cannot rent here.\n\r",ch);
            return;
        }

        send_to_char("You retire for the night.\n\r",ch);
        act("$n retires for the night.",FALSE,ch,0,0,TO_NOTVICT);
    }
    if (stash_char(ch,0) < 0 ) {	/* clear file.x and save into file.x */
        sprintf(buf, "rent: saving %s failed. FATAL", GET_NAME(ch)) ;
        log (buf) ;
	}
    move_stashfile(ch->player.name);	/* move file.x to file.x.y */
    /*
      for(i=0; i<MAX_WEAR; i++)
      if(ch->equipment[i]){
      extract_obj(unequip_char(ch,i));
      ch->equipment[i]=0;
      }
    */
    wipe_obj(ch->carrying);
    ch->carrying=0;
    for(i=0; i<MAX_WEAR; i++)
        if(ch->equipment[i]) {
            extract_obj(unequip_char(ch,i));	/* extract and free object */
            ch->equipment[i]=0;
        }
    wipe_obj(ch->carrying);	/* for equipment */
    ch->carrying=0;
	wipe_obj(ch->rent_items) ;	/* clear rent items - they are saved already */
	ch->rent_items = 0 ;
    save_room = ch->in_room;
    extract_char(ch);	/* extract items */
    ch->in_room = world[save_room].number;
    save_char(ch, ch->in_room);
	if ( ch->desc) {
		ch->desc->connected = CON_SLCT ;
		send_to_char(MENU, ch) ;
    }
    return;
}

void wipe_obj(struct obj_data *obj)
{
    if(obj){
        wipe_obj(obj->contains);
        wipe_obj(obj->next_content);
        if (obj->in_obj)
            obj_from_obj(obj);
        extract_obj(obj);	/* extract and free object */
    }
}

int file_to_announce(char *name, char *buf[])
{
    FILE *fl;
    char tmp[200], *tmp2;
    int  count ;
    extern int number_of_announce ;

    *buf = '\0';

    if (!(fl = fopen(name, "r")))
    {
        perror("file-to-announce");
        *buf = '\0';
        return(-1);
    }

    count = 0 ;
    do
    {
        fgets(tmp, 199, fl);

        if (!feof(fl))
        {
            if (tmp[0] == 'O') {
                if (buf[count]) free(buf[count]);

                tmp2 = (char *) malloc( strlen(tmp) +2) ;
                strcpy(tmp2, tmp+2) ;
                *(tmp2 + strlen(tmp2)-1) = '\r' ;
                *(tmp2 + strlen(tmp2)) = 0 ;
                buf[count] = tmp2 ;
                
                count ++ ;
            }
        }
    }
    while (!feof(fl) && count < MAX_ANNOUNCE);

    number_of_announce = count ;

    fclose(fl);

    return(0);
}

/* cyb made below */
int file_to_cookie(char *name, char *buf[MAX_FORTUNE_COOKIE])
{
    FILE *fl;
    char tmp[200], *tmp2;
    int  count ;
    extern int number_of_cookie ;

    *buf = '\0';

    if (!(fl = fopen(name, "r")))
    {
        perror("file-to-cookie");
        *buf = '\0';
        return(-1);
    }

    count = 0 ;
    do
    {
        fgets(tmp, 199, fl);

        if (!feof(fl))
        {
            tmp2 = (char *) malloc( strlen(tmp) +2) ;
            strcpy(tmp2, tmp) ;
            *(tmp2 + strlen(tmp2) +1) = 0 ;
            *(tmp2 + strlen(tmp2)) = '\r' ;
            buf[count] = tmp2 ;
            count ++ ;
        }
    }
    while (!feof(fl) && count < MAX_FORTUNE_COOKIE);

    number_of_cookie = count ;

    fclose(fl);

    return(0);
}

char *select_fortune_cookie()
{
	extern char *cookie[MAX_FORTUNE_COOKIE] ;
	extern int number_of_cookie ;

	return(cookie[number(0,number_of_cookie-1)]);
}

char *select_announce()
{
	extern char *announce[MAX_ANNOUNCE] ;
	extern int number_of_annoucne ;
	static int count = 0;

	return(announce[count++ % number_of_announce]);
}

void reconfig_news()
{
	extern char	news[], motd[] ;

	log("reconfiguration news and motd") ;
	/* news and motd is array, so need not free() them */
	news[0] = motd[0] = 0 ;
	file_to_string(NEWS_FILE, news);
	file_to_string(MOTD_FILE, motd);
	file_to_announce(ANNOUNCE_FILE, announce);
}

void do_retrieve(struct char_data *ch, int cmd, char *arg)
{
	extern int	boottime;
	extern long jackpot;
	int item_cost_lists(struct obj_data *p, char *str, int size) ;
	struct obj_data	*tmp_obj, *next_obj ;
	char	buf[BUFSIZ], buf2[BUFSIZ] ;
	int	count, cost=0, running_time ;

	if (IS_NPC(ch))
		return ;

	if(!cmd)
		return ;

	if(!IS_SET(world[ch->in_room].room_flags, RENT)) {
		send_to_char("You must go to Reception to retrieve rent items.\n\r", ch);
		return ;
    }

	if ( ch->rent_items == NULL ) {
		send_to_char("You didn't rent any items.\n\r", ch) ;
		return ;
    }
		
	/* check running time */
	running_time = (30 + time(0) - boottime) / 3600  ;	/* hours */
	if ( running_time >= 19 )	{	/* over than 19 hours */
		sprintf(buf, "Running time is Over 19 hours - No rent fee .\n\r") ;
    }
	else {
		/* calculate rent cost */
		cost = item_cost_lists(ch->rent_items, 0, 0) ;

		if ( running_time >= 16 )	{	/* over than 16 hours */
			send_to_char_han("Running time is Over 16 hours - 50% DC\n\r",
                             "Running time 이 16시간이 넘었습니다 - 50% 할인합니다.\n\r",
                             ch ) ;
			cost = cost / 2 ;
        }

		/* check player's gold - bank first, inventory second */
		if ( ch->bank < cost ) {
			if ( GET_GOLD(ch) < cost ) {
				sprintf(buf, "Sorry, you need %d coins for rent fee !!\n\r", cost) ;
				send_to_char(buf, ch) ;
				return ;
            }
			else {
				GET_GOLD(ch) -= cost ;
				sprintf(buf, "You paid %d coins for RENT FEE (inventory).\n\r", cost) ;
            }
        }
		else {
			ch->bank -= cost ;
			sprintf(buf, "You paid %d coins for RENT FEE (bank money).\n\r", cost) ;
        }
		save_char(ch, NOWHERE) ;
    }

	count = 0 ;
	for ( tmp_obj = ch->rent_items ; tmp_obj ; tmp_obj = next_obj ) {
		next_obj = tmp_obj->next_content ;
		obj_from_charrent(tmp_obj) ;
		obj_to_char(tmp_obj, ch) ;
		if (tmp_obj->wear > 0 && !ch->equipment[tmp_obj->wear-1]) {
			obj_from_char(tmp_obj);  
			equip_char(ch, tmp_obj, tmp_obj->wear-1); /* by Marx */
		}
		tmp_obj->wear = 0;
		count ++ ;
    }

    // jackpot에 rent비가 들어가도록 조정 
	jackpot += (long)(cost * 0.8);

	sprintf(buf2, "You received %d items.\n\r", count) ;
	strcat(buf, buf2) ;
	send_to_char(buf, ch) ;
}

