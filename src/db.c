/**************************************************************************
*  file: db.c , Database module.                          Part of DIKUMUD *
*  Usage: Loading/Saving chars, booting world, resetting etc.             *
*  Copyright (C) 1990, 1991 - see 'license.doc' for complete information. *
***************************************************************************/

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
#include <signal.h>

#include "structs.h"
#include "utils.h"
#include "db.h"
#include "comm.h"
#include "handler.h"
#include "limit.h"
#include "spells.h"

#include "mob_bal.c"

#define KJHRENT      66666 /* kjh number to tell new rent format */
#define SYPARKRENT   900176 /* sypark student id :) */
#define KNIFE_RENT   77777 /* Equiped Rent by Knife */

#define NEW_ZONE_SYSTEM

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
char imotd[MAX_STRING_LENGTH];		  /* MOTD for immortals				 */
char motd[MAX_STRING_LENGTH];         /* the messages of today           */
char help[MAX_STRING_LENGTH];         /* the main help page              */
char plan[MAX_STRING_LENGTH];         /* the info text                   */
char wizards[MAX_STRING_LENGTH];      /* the wizards text                */

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
extern int regen_percent;
extern int regen_time_percent;
extern int regen_time;
void load_messages(void);
void weather_and_time ( int mode );
void assign_command_pointers ( void );
void assign_spell_pointers ( void );
void log(char *str);
int dice(int number, int size);
int number(int from, int to);
void boot_social_messages(void);
struct help_index_element *build_help_index(FILE *fl, int *num);
void assign_mobiles(void);
void assign_objects(void);
void assign_rooms(void);
int MIN(int a, int b);
int MAX(int a, int b);
int str_cmp(char *arg1, char *arg2);
char *one_argument(char *arg, char *first_arg);
void wear(struct char_data *ch, struct obj_data *obj, int where_flag);

/* from act.wizard.c */
void roll_abilities(struct char_data *ch);

/* quest */
void init_quest(void);

/* for 대림사 */
/* these are also defined in spec_procs.c */
#define FOURTH_JANGRO		11132
#define SON_OGONG			11101

extern struct char_data *fourth_jangro;
extern struct char_data *son_ogong;

/*************************************************************************
*  routines for booting the system                                       *
*********************************************************************** */


/* body of the booting system */
void boot_db(void)
{
  int i;
  extern int no_specials;

  log("Boot db -- BEGIN.");

  log("Resetting the game time:");
  reset_time();

  log("Reading news, credits, help-page, plan, wizards, and motd.");
  file_to_string(NEWS_FILE, news);
  file_to_string(CREDITS_FILE, credits);
  file_to_string(IMOTD_FILE, imotd);
  file_to_string(MOTD_FILE, motd);
  file_to_string(HELP_PAGE_FILE, help);
  file_to_string(PLAN_FILE, plan);
  file_to_string(WIZARDS_FILE, wizards);

  log("Opening mobile, object and help files.");
  if (!(mob_f = fopen(MOB_FILE, "r")))
  {
    perror("boot");
    exit(0);
  }
  log("Mob done");

  if (!(obj_f = fopen(OBJ_FILE, "r")))
  {
    perror("boot");
    exit(0);
  }
  log("Obj done");
  if (!(help_fl = fopen(HELP_KWRD_FILE, "r")))
      log("   Could not open help file.");
  else 
    help_index = build_help_index(help_fl, &top_of_helpt);
  log("help index done.");

  init_quest();

  log("quest manager : initialize.");

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
  log("Boot db -- DONE.");
}

/* reset the time in the game from file */
void reset_time(void)
{
  char buf[MAX_STRING_LENGTH];

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
    exit(0);
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
          exit(0);
        }
      }
    
      player_table[nr].nr = nr;

      CREATE(player_table[nr].name, char,
         strlen(dummy.name) + 1);
      for (i = 0; 
	   (*(player_table[nr].name + i) = LOWER(*(dummy.name + i))); 
	   i++);
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
  struct index_data *index = NULL;
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
            exit(0);
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
      exit(0);
    }
  }
  *top = i - 2;
  return(index);
}



/* load the rooms */
/* new version of boot_world */
/* modified by ares */

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
    exit(0);
  }
  
  while (1) {
    fgets(file_name, 99, all_files);
    
    if (file_name[0] == '$')
      break;    /* end of file */
    
    len = strlen(file_name) - 1;
    if (file_name[len] == '\n' || file_name[len] == '\r')
      file_name[len] = 0;
    if (!(fl = fopen(file_name, "r"))) {
      perror("boot_zones");
      perror(file_name);
      exit(0);
    }
    
    do {
      fscanf(fl, " #%d\n", &virtual_nr);
      temp = fread_string(fl);
      if ((flag = (*temp != '$'))) { /* a new record to be read */
	allocate_room(room_nr);
	world[room_nr].number = virtual_nr;
	world[room_nr].name = temp;
	world[room_nr].description = fread_string(fl);
	
	if (top_of_zone_table >= 0) {
	  fscanf(fl, " %*d ");
	  /* OBS: Assumes ordering of input rooms */
	  if (world[room_nr].number <=
	      (zone ? zone_table[zone-1].top : -1)) {
	    fprintf(stderr, "Room nr %d is below zone %d.\n",
		    room_nr, zone);
            fprintf(stderr, "DEBUG: %d, %s\n",
                   world[room_nr].number, world[room_nr].name);
	    exit(0);
	  }
	  while (world[room_nr].number > zone_table[zone].top)
	    if (zone > top_of_zone_table) {
	      fprintf(stderr, "Room %d is outside of any zone.\n",
		      virtual_nr);
	      exit(0);
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
	world[room_nr].light = 0; /* Zero light sources */
	
	for (tmp = 0; tmp <= 5; tmp++)
	  world[room_nr].dir_option[tmp] = 0;
	
	world[room_nr].ex_description = 0;
	
	while (1) {
	  fscanf(fl, " %s \n", chk);
	  if (*chk == 'D')  /* direction field */
	    setup_dir(fl, room_nr, atoi(chk + 1));
	  else if (*chk == 'E') { /* extra description field */
	    CREATE(new_descr, struct extra_descr_data, 1);
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
    } while (flag);
    
    free(temp);  /* cleanup the area containing the terminal $  */
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
  
  if (!(fl = fopen(WORLD_FILE, "r")))
  {
    perror("fopen");
    log("boot_world: could not open world file.");
    exit(0);
  }

  do
  {
    fscanf(fl, " #%d\n", &virtual_nr);
    temp = fread_string(fl);
    if (flag = (*temp != '$')) 
    {
      allocate_room(room_nr);
      world[room_nr].number = virtual_nr;
      world[room_nr].name = temp;
      world[room_nr].description = fread_string(fl);

      if (top_of_zone_table >= 0)
      {
        fscanf(fl, " %*d ");
        if (world[room_nr].number <= (zone ? zone_table[zone-1].top : -1))
        {
          fprintf(stderr, "Room nr %d is below zone %d.\n",
            room_nr, zone);
          exit(0);
        }
        while (world[room_nr].number > zone_table[zone].top)
          if (++zone > top_of_zone_table)
          {
            fprintf(stderr, "Room %d is outside of any zone.\n",
              virtual_nr);
            exit(0);
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

      for (;;)
      {
        fscanf(fl, " %s \n", chk);

        if (*chk == 'D') 
          setup_dir(fl, room_nr, atoi(chk + 1));
        else if (*chk == 'E')  
        {
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
  }
  while (flag);

  free(temp); 

  fclose(fl);
  top_of_world = --room_nr;
}
*/

void allocate_room(int new_top)
{
  struct room_data *new_world;

  if (new_top)
  { 
    if (!(new_world = (struct room_data *) 
      realloc(world, (new_top + 1) * sizeof(struct room_data))))
    {
      perror("alloc_room");
      exit(0);
    } 
  }
  else
    CREATE(new_world, struct room_data, 1);

  world = new_world;
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

/* new version of boot_zone : by ares */
/* read lib/zone/.zon files */

#define ALL_ZONE_FILE "zone/zone_files"

void load_zones(int zon)
{
	FILE *fl;
	char buf[255],*check;
	int cmd_no = 0, expand;

	if ( zon > top_of_zone_table )
		return;
	fl = fopen(zone_table[zon].filename,"r");
	if (!fl)
	{
		sprintf(buf,"Error in reading zone file '%s'",zone_table[zon].filename);
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
			else
				if (!(zone_table[zon].cmd =
					(struct reset_com *) realloc(zone_table[zon].cmd, 
					(cmd_no + 1) * sizeof(struct reset_com)))) {
					perror("reset command load");
					perror(zone_table[zon].filename);
					exit(0);
				}
		}
		expand = 1;
		fscanf(fl, " "); /* skip blanks */
		fscanf(fl, "%c", &zone_table[zon].cmd[cmd_no].command);

		/* end of each zone file */
		if (zone_table[zon].cmd[cmd_no].command == 'S') {
			fclose(fl);
			break;
		}

		if (zone_table[zon].cmd[cmd_no].command == '*') {
			expand = 0;
			fgets(buf, 80, fl); /* skip command */
			continue;
		}

		fscanf(fl, " %d %d %d", 
			(int *)&zone_table[zon].cmd[cmd_no].if_flag,
			&zone_table[zon].cmd[cmd_no].arg1,
			&zone_table[zon].cmd[cmd_no].arg2);

		if (zone_table[zon].cmd[cmd_no].command == 'M' ||
			zone_table[zon].cmd[cmd_no].command == 'O' ||
			zone_table[zon].cmd[cmd_no].command == 'E' ||
			zone_table[zon].cmd[cmd_no].command == 'P' ||
			zone_table[zon].cmd[cmd_no].command == 'D')
			fscanf(fl, " %d", &zone_table[zon].cmd[cmd_no].arg3);

		fgets(buf, 80, fl);  /* read comment */

		cmd_no++;
	}
}

void boot_zones(void)
{
	FILE *all_files;
	FILE *fl;
	int zon = 0, cmd_no = 0, expand;
	char *check, buf[81];
	char file_name[100];
	int len;

	if (!(all_files = fopen(ALL_ZONE_FILE, "r"))) {
		perror("boot_zones (zone_files)");
		exit(0);
	}

	while (1) {
		fgets(file_name, 99, all_files);

		if (file_name[0] == '$')
			break;    /* end of file */

		len = strlen(file_name) - 1;
		if (file_name[len] == '\n' || file_name[len] == '\r')
			file_name[len] = 0;
		if (!(fl = fopen(file_name, "r"))) {
			perror("boot_zones");
			perror(file_name);
			exit(0);
		}

		check = fread_string(fl);

		/* alloc a new zone */
		if (!zon)
			CREATE(zone_table, struct zone_data, 1);
		else
			if (!(zone_table = (struct zone_data *) realloc(zone_table,
				(zon + 1) * sizeof(struct zone_data)))) {
				perror("boot_zones realloc");
				perror(file_name);
				exit(0);
			}

		zone_table[zon].name = check;
		CREATE(zone_table[zon].filename,char, strlen(file_name)+1); // +1 is correct size.
		strcpy(zone_table[zon].filename,file_name);
		fscanf(fl, " %d ", &zone_table[zon].top);
		fscanf(fl, " %d ", &zone_table[zon].lifespan);
		fscanf(fl, " %d ", &zone_table[zon].reset_mode);

		/* read the command table */
		cmd_no = 0;
		for (expand = 1;;) {
			if (expand) {
				if (!cmd_no)
					CREATE(zone_table[zon].cmd, struct reset_com, 1);
				else
					if (!(zone_table[zon].cmd =
						(struct reset_com *) realloc(zone_table[zon].cmd, 
						(cmd_no + 1) * sizeof(struct reset_com)))) {
						perror("reset command load");
						perror(file_name);
						exit(0);
					}
			}
			expand = 1;
			fscanf(fl, " "); /* skip blanks */
			fscanf(fl, "%c", &zone_table[zon].cmd[cmd_no].command);

			/* end of each zone file */
			if (zone_table[zon].cmd[cmd_no].command == 'S') {
				fclose(fl);
				break;
			}

			if (zone_table[zon].cmd[cmd_no].command == '*') {
				expand = 0;
				fgets(buf, 80, fl); /* skip command */
				continue;
			}

			fscanf(fl, " %d %d %d", 
				(int *)&zone_table[zon].cmd[cmd_no].if_flag,
				&zone_table[zon].cmd[cmd_no].arg1,
				&zone_table[zon].cmd[cmd_no].arg2);

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
	fclose(all_files);
}
#undef ALL_ZONE_FILE

/* original boot_zone : read tinyworld.zon */
/* load the zone table and command tables */
/*
void boot_zones(void)
{
  FILE *fl;
  int zon = 0, cmd_no = 0, ch, expand, tmp;
  char *check, buf[81];

  if (!(fl = fopen(ZONE_FILE, "r")))
  {
    perror("boot_zones");
    exit(0);
  }

  for (;;)
  {
    fscanf(fl, " #%*d\n");
    check = fread_string(fl);

    if (*check == '$')
      break; 

    if (!zon)
      CREATE(zone_table, struct zone_data, 1);
    else
      if (!(zone_table = (struct zone_data *) realloc(zone_table,
        (zon + 1) * sizeof(struct zone_data))))
        {
          perror("boot_zones realloc");
          exit(0);
        }

    zone_table[zon].name = check;
    fscanf(fl, " %d ", &zone_table[zon].top);
    fscanf(fl, " %d ", &zone_table[zon].lifespan);
    fscanf(fl, " %d ", &zone_table[zon].reset_mode);

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
            exit(0);
          }

      expand = 1;

      fscanf(fl, " ");
      fscanf(fl, "%c", 
        &zone_table[zon].cmd[cmd_no].command);
      
      if (zone_table[zon].cmd[cmd_no].command == 'S')
        break;

      if (zone_table[zon].cmd[cmd_no].command == '*')
      {
        expand = 0;
        fgets(buf, 80, fl);
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

/* read a mobile from MOB_FILE */
struct char_data *read_mobile(int nr, int type)
{
	int i;
	struct char_data *mob;
	long tmp, tmp2, tmp3;
	char letter;
	int level, level2, level3, abil, class;
	char buf[256];
	extern struct spell_info_type spell_info[];

	i = nr;
	if (type == VIRTUAL)
		if ((nr = real_mobile(nr)) < 0) {
			sprintf(buf, "Mobile (V) %d does not exist in database.", i);
			log(buf);
			return(0);
		}

	fseek(mob_f, mob_index[nr].pos, 0);
	CREATE(mob, struct char_data, 1);
	clear_char(mob);

	/***** String data *** */
    
	mob->player.name = fread_string(mob_f);
	mob->player.short_descr = fread_string(mob_f);
	mob->player.long_descr = fread_string(mob_f);
	mob->player.description = fread_string(mob_f);
	mob->player.title = 0;

	/* *** Numeric data *** */

	fscanf(mob_f, "%ld ", &tmp);
	mob->specials.act = tmp;
	SET_BIT(mob->specials.act, ACT_ISNPC);

	fscanf(mob_f, " %ld ", &tmp);
	mob->specials.affected_by = tmp;

	fscanf(mob_f, " %ld \n", &tmp);
	mob->specials.alignment = tmp;

	/* set mob's class */
	fscanf(mob_f, " %c ", &letter);
	switch (letter)
	{
		case 'M':
			GET_CLASS(mob) = CLASS_MAGIC_USER;
			break;
		case 'C':
			GET_CLASS(mob) = CLASS_CLERIC;
			break;
		case 'T':
			GET_CLASS(mob) = CLASS_THIEF;
			break;
		case 'W':
			GET_CLASS(mob) = CLASS_WARRIOR;
			break;
		default:
			/* random selection */
			GET_CLASS(mob) = number(1, 4);
	}
	class = GET_CLASS(mob);

	/* set mob's level */
	fscanf(mob_f, " %ld ", &tmp);
	level = MIN(43, tmp);
	level2 = level * level;
	level3 = level2 * level;

	GET_LEVEL(mob) = level; 

	/* level dependent data */
	abil = level / 4 + 5;
	switch(class){
		case CLASS_MAGIC_USER :
				mob->abilities.str = number(abil >> 1, abil);
				mob->abilities.str_add = 0;
				mob->abilities.intel = number(abil, 18);
				mob->abilities.wis = number(abil, 17);
				mob->abilities.dex = number(abil >> 1, abil);
				mob->abilities.con = number(abil >> 1, abil);
			break;
		case CLASS_CLERIC :
				mob->abilities.str = number(abil >> 1, abil);
				mob->abilities.str_add = 0;
				mob->abilities.intel = number(abil, 17);
				mob->abilities.wis = number(abil, 18);
				mob->abilities.dex = number(abil >> 1, abil);
				mob->abilities.con = number(abil >> 1, abil);
			break;
		case CLASS_THIEF :
				mob->abilities.str = number(abil, 18);
				mob->abilities.str_add = 0;
				mob->abilities.intel = number(abil >> 1, abil);
				mob->abilities.wis = number(abil >> 1, abil);
				mob->abilities.dex = number(abil, 18);
				mob->abilities.con = number(abil, 17);
			break;
		case CLASS_WARRIOR :
				mob->abilities.str = 18;
				mob->abilities.str_add = number(1, MAX(abil << 3, 100));
				mob->abilities.intel = number(abil >> 1, abil);
				mob->abilities.wis = number(abil >> 1, abil);
				mob->abilities.dex = number(abil, 17);
				mob->abilities.con = number(abil, 18);
			break;
	}
	mob->points.mana = mob->points.max_mana =
		number(level2, level * mob->abilities.wis * mob->abilities.intel);
	mob->points.move = mob->points.max_move =
		number(level2, level * mob->abilities.dex * mob->abilities.dex);

	/* set hit */
	fscanf(mob_f, " %ld ", &tmp);
	if(tmp < 2){  /* tmp == 1 */
		mob->points.hit = mob->points.max_hit =
			number(level3, mob_bal_hit[level-1][0]);
	}
	else if(tmp > 20){
		mob->points.hit = mob->points.max_hit =
			number(mob_bal_hit[level-1][19], level3*tmp);
	}
	else { /* 1 < tmp <= 20 */
		mob->points.hit = mob->points.max_hit =
			number(mob_bal_hit[level-1][tmp-2],mob_bal_hit[level-1][tmp-1]);
	}

	/* set armor */
	fscanf(mob_f, " %ld ", &tmp);
	if(tmp < 2){  /* tmp == 1 */
		tmp2 =  100 - mob_bal_ac[level-1][0];
		mob->points.armor = 100 - number(level, tmp2);
	}
	else if(tmp > 20){ /* tmp > 20 */
		tmp2 =  100 - mob_bal_ac[level-1][19];
		mob->points.armor = 100 - number(tmp2, level*tmp);
	}
	else { /* 1 < tmp <= 20 */
		tmp2 =  100 - mob_bal_ac[level-1][tmp-2];
		tmp3 =  100 - mob_bal_ac[level-1][tmp-1];
		mob->points.armor = 100 - number(tmp2, tmp3);
	}

	/* set hitroll */
	fscanf(mob_f, " %ld ", &tmp);
	if(tmp < 2){  /* tmp == 1 */
		mob->points.hitroll = number(level, mob_bal_hr[level-1][0]);
	}
	else if(tmp > 20){
		mob->points.hitroll =
			number(mob_bal_hr[level-1][19], level*tmp);
	}
	else { /* 1 < tmp <= 20 */
		mob->points.hitroll =
			number(mob_bal_hr[level-1][tmp-2],mob_bal_hr[level-1][tmp-1]);
	}

	/* set damdice */
	mob->specials.damnodice = number(level/3, level/2) + 1;
	mob->specials.damsizedice = number(level/4, level/3) + 1;

	/* set damroll */
	fscanf(mob_f, " %ld \n", &tmp);
	if(tmp < 2){  /* tmp == 1 */
		mob->points.damroll = number(level, mob_bal_dr[level-1][0]);
	}
	else if(tmp > 20){
		mob->points.damroll =
			number(mob_bal_dr[level-1][19], level*tmp);
	}
	else { /* 1 < tmp <= 20 */
		mob->points.damroll =
			number(mob_bal_dr[level-1][tmp-2],mob_bal_dr[level-1][tmp-1]);
	}

	/* set gold */
	fscanf(mob_f, " %ld ", &tmp);
	mob->points.gold = tmp;

	/* set XP */
	fscanf(mob_f, " %ld ", &tmp);
	if(tmp < 2){  /* tmp == 1 */
		mob->points.exp = number(level3, mob_bal_exp[level-1][0]);
	}
	else if(tmp > 20){
		mob->points.exp =
			number(mob_bal_exp[level-1][19], level3*level*tmp/10);
	}
	else { /* 1 < tmp <= 20 */
		mob->points.exp =
			number(mob_bal_exp[level-1][tmp-2],mob_bal_exp[level-1][tmp-1]);
	}

	/* set position */
	fscanf(mob_f, " %ld ", &tmp);
	mob->specials.default_pos = mob->specials.position = tmp;

	/* set sex */
	fscanf(mob_f, " %c \n", &letter);
	switch(letter){
		case 'N' : mob->player.sex = 0; break;
		case 'M' : mob->player.sex = 1; break;
		case 'F' : mob->player.sex = 2; break;
		default  : mob->player.sex = number(0, 2);
	}

	mob->player.guild = 0;
	mob->player.pk_num = 0;
	mob->player.pked_num = 0;

	mob->player.weight = number(100, 200);
	mob->player.height = number(100, 200);

	for (i = 0; i < 3; i++)
		GET_COND(mob, i) = -1;

	mob->specials.apply_saving_throw[SAVING_PARA] =
		100 - number(GET_LEVEL(mob) >> 1, GET_LEVEL(mob));
	mob->specials.apply_saving_throw[SAVING_PETRI] =
		100 - number(GET_LEVEL(mob) >> 1, GET_LEVEL(mob));
	mob->specials.apply_saving_throw[SAVING_BREATH] =
		100 - number(GET_LEVEL(mob) >> 1, GET_LEVEL(mob));

	switch(class){
		case CLASS_MAGIC_USER :
		case CLASS_CLERIC     :
				mob->specials.apply_saving_throw[SAVING_HIT_SKILL] =
					100 - number(GET_LEVEL(mob) >> 1, GET_LEVEL(mob));
				mob->specials.apply_saving_throw[SAVING_SPELL] =
					100 - number(GET_LEVEL(mob), GET_LEVEL(mob) << 1);
				break;
		case CLASS_THIEF      :
		case CLASS_WARRIOR    :
				mob->specials.apply_saving_throw[SAVING_HIT_SKILL] =
					100 - number(GET_LEVEL(mob), GET_LEVEL(mob) << 1);
				mob->specials.apply_saving_throw[SAVING_SPELL] =
					100 - number(GET_LEVEL(mob) >> 1, GET_LEVEL(mob));
				break;
	}

	mob->tmpabilities = mob->abilities;

	mob->player.time.birth = time(0);
	mob->player.time.played  = 0;
	mob->player.time.logon  = time(0);

	for (i = 0; i < MAX_WEAR; i++) /* Initialisering Ok */
		mob->equipment[i] = 0;

	mob->nr = nr;
	mob->desc = 0;

	for (i = 0; i < MAX_SKILLS; i++) {
		if(spell_info[i].min_level[class-1] <= level){
			mob->skills[i].learned
				= (level>40)?99:number(level,spell_info[i].max_skill[class-1]);
		}
		else {
			mob->skills[i].learned = 0;
		}
		mob->skills[i].skilled = (level>40)?level:0;
		mob->skills[i].recognise = 0;
	}	

	if(level > 40) mob->regeneration = level << 4;
	else if(level > 35) mob->regeneration = level << 2;
	else mob->regeneration = level << 1;

	/* quest */
	mob->quest.type = 0;
	mob->quest.data = 0;
	mob->quest.solved = 0;

	/* insert in list */
	mob->next = character_list;
	character_list = mob;

	mob_index[nr].number++;

	if(mob_index[nr].virtual == SON_OGONG)
		son_ogong = mob;
	else if(mob_index[nr].virtual == FOURTH_JANGRO)
		fourth_jangro = mob;

	mob->regened = 0;
	mob->specials.fighting = 0;

	return(mob);
}

#ifdef OldMobileFormat
struct char_data *read_mobile(int nr, int type)
{
	int i;
	long tmp, tmp2, tmp3;
	struct char_data *mob;
	char buf[100];
	char letter;
	int data;
	int abil;
	static int mob_abil[44] = {
		4,
		5, 5, 5, 6, 6, 6, 7, 7, 7, 7,
		8, 8, 8, 9, 9, 9, 10, 10, 10, 10,
		11, 11, 12, 12, 13, 13, 14, 14, 15, 15,
		15, 15, 15, 16, 16, 16, 17, 17, 18, 18,
		18, 18, 18 };

	i = nr;
	if (type == VIRTUAL)
		if ((nr = real_mobile(nr)) < 0) {
			sprintf(buf, "Mobile (V) %d does not exist in database.", i);
			return(0);
		}

	fseek(mob_f, mob_index[nr].pos, 0);

	CREATE(mob, struct char_data, 1);
	clear_char(mob);

	/***** String data *** */
    
	mob->player.name = fread_string(mob_f);
	mob->player.short_descr = fread_string(mob_f);
	mob->player.long_descr = fread_string(mob_f);
	mob->player.description = fread_string(mob_f);
	mob->player.title = 0;

	/* *** Numeric data *** */

	fscanf(mob_f, "%d ", &tmp);
	mob->specials.act = tmp;
	SET_BIT(mob->specials.act, ACT_ISNPC);

	fscanf(mob_f, " %d ", &tmp);
	mob->specials.affected_by = tmp;

	fscanf(mob_f, " %d ", &tmp);
	mob->specials.alignment = tmp;

	fscanf(mob_f, " %c \n", &letter);

	/* new easy monster, for easy balancing */
	if (letter == 'X') {
		/* set mob's class */
		fscanf(mob_f, " %c ", &letter);
		switch (letter)
		{
			case 'M':
				mob->player.class = CLASS_MAGIC_USER;
				break;
			case 'C':
				mob->player.class = CLASS_CLERIC;
				break;
			case 'T':
				mob->player.class = CLASS_THIEF;
				break;
			case 'W':
				mob->player.class = CLASS_WARRIOR;
				break;
			default:
				log("mob must have a class.");
				exit(1);
		}

		/* set mob's level */
		fscanf(mob_f, " %ld ", &tmp);
		tmp = MIN(43, tmp);
		GET_LEVEL(mob) = tmp; 

		/* level dependent data */
		abil = mob_abil[tmp];
		mob->abilities.str   = number(abil >> 1, abil);
		mob->abilities.str_add = 0;
		mob->abilities.intel = number(abil >> 1, abil); 
		mob->abilities.wis   = number(abil >> 1, abil);
		mob->abilities.dex   = number(abil >> 1, abil);
		mob->abilities.con   = number(abil >> 1, abil);

		mob->points.mana = mob->points.max_mana =
		number(GET_LEVEL(mob) * 10, GET_LEVEL(mob) * 20);
		mob->points.move = mob->points.max_move =
		number(GET_LEVEL(mob) * 20, GET_LEVEL(mob) * 30);
		/* end of level dependent data */

		/* set hardness */
		fscanf(mob_f, " %c \n", &letter);
		switch (letter) {
			/* A : most hard, E : mose easy */
			case 'A':
				abil = dice(GET_LEVEL(mob), 3);
				mob->points.hitroll = abil;
				mob->points.damroll = abil;
				abil = dice(GET_LEVEL(mob), 5);
				mob->points.armor = 100 - abil;
				if (GET_LEVEL(mob) < 10)
					abil = GET_LEVEL(mob);
				else if (GET_LEVEL(mob) < 20)
					abil = GET_LEVEL(mob) << 1;
				else if (GET_LEVEL(mob) < 30)
					abil = GET_LEVEL(mob) << 2;
				else if (GET_LEVEL(mob) < 40)
					abil = GET_LEVEL(mob) << 3;
				else
					abil = GET_LEVEL(mob) << 4;
				abil = dice(abil, 200);
				GET_PLAYER_MAX_HIT(mob) = GET_HIT(mob) = abil;
				abil = MAX(5, GET_LEVEL(mob) >> 1);
				abil = number(5, abil);
				mob->specials.damnodice = abil;
				mob->specials.damsizedice = abil;
				break;
			case 'B':
				abil = dice(GET_LEVEL(mob), 2);
				mob->points.hitroll = abil;
				mob->points.damroll = abil;
				abil = dice(GET_LEVEL(mob), 4);
				mob->points.armor = 100 - abil;
				if (GET_LEVEL(mob) < 10)
					abil = GET_LEVEL(mob);
				else if (GET_LEVEL(mob) < 20)
					abil = GET_LEVEL(mob) << 1;
				else if (GET_LEVEL(mob) < 30)
					abil = GET_LEVEL(mob) << 2;
				else if (GET_LEVEL(mob) < 40)
					abil = GET_LEVEL(mob) << 3;
				else
					abil = GET_LEVEL(mob) << 4;
				abil = dice(abil, 100);
				GET_PLAYER_MAX_HIT(mob) = GET_HIT(mob) = abil;
				abil = MAX(4, GET_LEVEL(mob) >> 2);
				abil = number(4, abil);
				mob->specials.damnodice = abil;
				mob->specials.damsizedice = abil;
				break;
			case 'C':
				abil = GET_LEVEL(mob);
				mob->points.hitroll = abil;
				mob->points.damroll = abil;
				abil = dice(GET_LEVEL(mob), 4);
				mob->points.armor = 100 - abil;
				if (GET_LEVEL(mob) < 10)
					abil = GET_LEVEL(mob);
				else if (GET_LEVEL(mob) < 20)
					abil = GET_LEVEL(mob) << 1;
				else if (GET_LEVEL(mob) < 30)
					abil = GET_LEVEL(mob) << 2;
				else if (GET_LEVEL(mob) < 40)
					abil = GET_LEVEL(mob) << 3;
				else
					abil = GET_LEVEL(mob) << 4;
				abil = dice(abil, 50);
				GET_PLAYER_MAX_HIT(mob) = GET_HIT(mob) = abil;
				abil = MAX(1, GET_LEVEL(mob) >> 3);
				abil = number(1, abil);
				mob->specials.damnodice = abil;
				mob->specials.damsizedice = abil;
				break;
			case 'D':
				abil = number(GET_LEVEL(mob) >> 1, GET_LEVEL(mob));
				mob->points.hitroll = abil;
				mob->points.damroll = abil;
				abil = dice(GET_LEVEL(mob), 3);
				mob->points.armor = 100 - abil;
				if (GET_LEVEL(mob) < 10)
					abil = GET_LEVEL(mob);
				else if (GET_LEVEL(mob) < 20)
					abil = GET_LEVEL(mob) << 1;
				else if (GET_LEVEL(mob) < 30)
					abil = GET_LEVEL(mob) << 2;
				else if (GET_LEVEL(mob) < 40)
					abil = GET_LEVEL(mob) << 3;
				else
					abil = GET_LEVEL(mob) << 4;
				abil = dice(abil, 30);
				GET_PLAYER_MAX_HIT(mob) = GET_HIT(mob) = abil;
				abil = MAX(1, GET_LEVEL(mob) >> 3);
				abil = number(1, abil);
				mob->specials.damnodice = abil;
				abil >>= 1;
				abil = MAX(1, abil);
				mob->specials.damsizedice = abil;
				break;
			case 'E':
				abil = number(1, GET_LEVEL(mob));
				mob->points.hitroll = abil;
				mob->points.damroll = abil;
				abil = dice(GET_LEVEL(mob), 2);
				mob->points.armor = 100 - abil;
				if (GET_LEVEL(mob) < 10)
					abil = GET_LEVEL(mob);
				else if (GET_LEVEL(mob) < 20)
					abil = GET_LEVEL(mob) << 1;
				else if (GET_LEVEL(mob) < 30)
					abil = GET_LEVEL(mob) << 2;
				else if (GET_LEVEL(mob) < 40)
					abil = GET_LEVEL(mob) << 3;
				else
					abil = GET_LEVEL(mob) << 4;
				abil = dice(abil, 20);
				GET_PLAYER_MAX_HIT(mob) = GET_HIT(mob) = abil;
				abil = MAX(1, GET_LEVEL(mob) >> 3);
				abil = number(1, abil);
				abil >>= 1;
				abil = MAX(1, abil);
				mob->specials.damnodice = abil;
				mob->specials.damsizedice = abil;
				break;
			default:
				log("mob's hardness is unknown.");
				exit(1);
		}

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

		mob->player.guild = 0;
		mob->player.pk_num = 0;
		mob->player.pked_num = 0;

		mob->player.weight = number(100, 200);
		mob->player.height = number(100, 200);

		for (i = 0; i < 3; i++)
			GET_COND(mob, i) = -1;
		for (i = 0; i < 5; i++)
			mob->specials.apply_saving_throw[i] =
				100 - number(GET_LEVEL(mob) >> 1, GET_LEVEL(mob) << 1);
	}
	else if (letter == 'S') {
		/* set mob's level */
		fscanf(mob_f, " %ld ", &tmp);
		tmp = MIN(43, tmp);
		GET_LEVEL(mob) = tmp; 

		abil = mob_abil[tmp];
		mob->abilities.str   = number(abil >> 1, abil);
		mob->abilities.str_add = 0;
		mob->abilities.intel = number(abil >> 1, abil); 
		mob->abilities.wis   = number(abil >> 1, abil);
		mob->abilities.dex   = number(abil >> 1, abil);
		mob->abilities.con   = number(abil >> 1, abil);
		/*
		mob->abilities.str   = 18;
		mob->abilities.intel = 11; 
		mob->abilities.wis   = 11;
		mob->abilities.dex   = 18;
		mob->abilities.con   = 11;
		*/

		mob->player.guild = 0;
		mob->player.pk_num = 0;
		mob->player.pked_num = 0;
		fscanf(mob_f, " %ld ", &tmp);
		mob->points.hitroll = 20 - tmp;
    
		fscanf(mob_f, " %ld ", &tmp);
		mob->points.armor = 10*tmp;

		fscanf(mob_f, " %ldd%ld+%ld ", &tmp, &tmp2, &tmp3);
		mob->points.max_hit = dice(tmp, tmp2)+tmp3;
		mob->points.hit = mob->points.max_hit;

		fscanf(mob_f, " %ldd%ld+%ld \n", &tmp, &tmp2, &tmp3);
		mob->points.damroll = tmp3;
		mob->specials.damnodice = tmp;
		mob->specials.damsizedice = tmp2;

		mob->points.mana = mob->points.max_mana =
		number(GET_LEVEL(mob) * 10, GET_LEVEL(mob) * 20);
		mob->points.move = mob->points.max_move =
		number(GET_LEVEL(mob) * 20, GET_LEVEL(mob) * 30);

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
		mob->player.class = number(1, 4);
		mob->player.weight = 200;
		mob->player.height = 198;
		for (i = 0; i < 3; i++)
			GET_COND(mob, i) = -1;
		for (i = 0; i < 5; i++)
			mob->specials.apply_saving_throw[i] =
				100 - number(GET_LEVEL(mob) >> 1, GET_LEVEL(mob) << 1);
	}
	else {  /* The old monsters are down below here */
		perror("Old monster exit! (db.c, read_mobile)");
		log("Old monster exit in mob file! (db.c, read_mobile)");
		exit(0);
	}

	mob->tmpabilities = mob->abilities;

	mob->player.time.birth = time(0);
	mob->player.time.played  = 0;
	mob->player.time.logon  = time(0);

	for (i = 0; i < MAX_WEAR; i++) /* Initialisering Ok */
		mob->equipment[i] = 0;

	mob->nr = nr;
	mob->desc = 0;

	data = MIN(95, GET_LEVEL(mob) * 2);
	for (i = 0; i < MAX_SKILLS; i++) {
		mob->skills[i].learned = data;
		mob->skills[i].recognise = 0;
	}	
	mob->regeneration = 0;

	/* insert in list */

	mob->next = character_list;
	character_list = mob;

	mob_index[nr].number++;

	return(mob);
}
#endif /* OldMobileFormat */

/* read an object from OBJ_FILE */
struct obj_data *read_object(int nr, int type)
{
  struct obj_data *obj;
  int tmp, i;
  char chk[50], buf[100];
  struct extra_descr_data *new_descr;

  i = nr;
  if (type == VIRTUAL)
    if ((nr = real_object(nr)) < 0)
  {
    sprintf(buf, "Object (V) %d does not exist in database.", i);
    return(0);
  }

  fseek(obj_f, obj_index[nr].pos, 0);

  CREATE(obj, struct obj_data, 1);

  clear_object(obj);

  /* *** string data *** */

  obj->name = fread_string(obj_f);
  obj->short_description = fread_string(obj_f);
  obj->description = fread_string(obj_f);
  obj->action_description = fread_string(obj_f);

  /* *** numeric data *** */

  fscanf(obj_f, " %d ", &tmp);
  obj->obj_flags.type_flag = tmp;
  fscanf(obj_f, " %d ", &tmp);
  obj->obj_flags.extra_flags = tmp;
  fscanf(obj_f, " %d ", &tmp);
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
    CREATE(new_descr, struct extra_descr_data, 1);

    new_descr->keyword = fread_string(obj_f);
    new_descr->description = fread_string(obj_f);

    new_descr->next = obj->ex_description;
    obj->ex_description = new_descr;
  }

  for( i = 0 ; (i < MAX_OBJ_AFFECT) && (*chk == 'A') ; i++)
  {
    fscanf(obj_f, " %d ", &tmp);
    obj->affected[i].location = tmp;
    fscanf(obj_f, " %d \n", &tmp);
    obj->affected[i].modifier = tmp;
    fscanf(obj_f, " %s \n", chk);
  }

  for (;(i < MAX_OBJ_AFFECT);i++)
  {
    obj->affected[i].location = APPLY_NONE;
    obj->affected[i].modifier = 0;
  }

  obj->in_room = NOWHERE;
  obj->next_content = 0;
  obj->carried_by = 0;
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
  int i;
  struct reset_q_element *update_u, *temp;

  /* enqueue zones */

  for (i = 0; i <= top_of_zone_table; i++)
  {
    if (zone_table[i].age < (zone_table[i].lifespan*regen_time_percent/100)&&
      zone_table[i].reset_mode)
      (zone_table[i].age)++;
    else
      if (zone_table[i].age < ZO_DEAD && zone_table[i].reset_mode)
      {
      /* enqueue zone */

      CREATE(update_u, struct reset_q_element, 1);
 
      update_u->zone_to_reset = i;
      update_u->next = 0;

      if (!reset_q.head)
        reset_q.head = reset_q.tail = update_u;
      else
      {
        reset_q.tail->next = update_u;
        reset_q.tail = update_u;
      }

      zone_table[i].age = ZO_DEAD;
      }
  }

  /* dequeue zones (if possible) and reset */

  for (update_u = reset_q.head; update_u; update_u = update_u->next) 
    if (zone_table[update_u->zone_to_reset].reset_mode == 2 ||
      is_empty(update_u->zone_to_reset))
    {
    reset_zone(update_u->zone_to_reset);

    /* dequeue */

    if (update_u == reset_q.head)
      reset_q.head = reset_q.head->next;
    else
    {
      for (temp = reset_q.head; temp->next != update_u;
        temp = temp->next);

      if (!update_u->next)
        reset_q.tail = temp;

      temp->next = update_u->next;


    }

    free(update_u);
    break;
    } 
}




#ifndef OLD_ZONE_SYSTEM

/*
	index : real number
*/
struct char_data *get_mobile_index(int index)
{
  struct char_data *ch;

  for(ch = character_list; ch; ch = ch->next){
    if(IS_NPC(ch) && ch->nr == index){
      	return ch;
    }
  }

  /* error */
  return NULL;
}

#define ZCMD zone_table[zone].cmd[cmd_no]

void reset_zone(int zone)
{
  int cmd_no;
  char last_cmd = 1;
  struct char_data *mob = NULL;
  struct obj_data *obj, *obj_to;
  char buf[256];

  /* item regen !!! */
  /*
  	if (regen == 1) then regen all items.
  */
  int regen;
  int real_load;
  
  regen = (number(0, regen_time/zone_table[zone].lifespan) == 0);

  real_load = 0;
  for (cmd_no = 0;;cmd_no++) {
    if (ZCMD.command == 'S')
      break;
    if (last_cmd || !ZCMD.if_flag)
      switch(ZCMD.command) {
      case 'M': /* read a mobile */
        if (mob_index[ZCMD.arg1].number < ZCMD.arg2) {
          mob = read_mobile(ZCMD.arg1, REAL);
          char_to_room(mob, ZCMD.arg3);
          last_cmd = 'M';
          if(regen) mob->regened = 1;
          real_load = 1;
        } else {
          mob = get_mobile_index(ZCMD.arg1);
          if(mob == NULL) last_cmd = 0;
          else {
             if(mob->regened) last_cmd = 0;
             else {
               last_cmd = 'M';
               if(regen) mob->regened = 1;
               real_load = 0;
             }
          }
        }
      break;

      case 'O': /* read an object */
        /*
        if (obj_index[ZCMD.arg1].number < ZCMD.arg2)
        */
        if (ZCMD.arg3 >= 0) {
          if (!get_obj_in_list_num(ZCMD.arg1,world[ZCMD.arg3].contents)) {
            obj = read_object(ZCMD.arg1, REAL);
			if ( (	obj->obj_flags.type_flag == ITEM_KEY ) ||
				 (	!IS_SET(obj->obj_flags.extra_flags,ITEM_NOLOAD ) ) ||
				 (  regen == 1 ) )
				 /*
				 (	number(1,100) <= regen_percent ) )
				 */
			{
           		obj_to_room(obj, ZCMD.arg3);
            	last_cmd = 'O';
			}
			else
				extract_obj(obj);
          }
        } else {
          obj = read_object(ZCMD.arg1, REAL);
		  if ( (	obj->obj_flags.type_flag == ITEM_KEY ) ||
			   (	!IS_SET(obj->obj_flags.extra_flags,ITEM_NOLOAD) ) ||
			   (	regen == 1 ) )
			   /*
			   (	number(1,100) <= regen_percent ) )
			   */
		  {

          	 obj->in_room = NOWHERE;
             last_cmd = 'O';
		  }
		  else
				extract_obj(obj);
		}
      break;

      case 'P': /* object to object */
        /*
        if (obj_index[ZCMD.arg1].number < ZCMD.arg2 &&
        */
        if(
			(last_cmd == 'O' || last_cmd == 'P' ||
			 last_cmd == 'E' || last_cmd == 'G')) {
          obj = read_object(ZCMD.arg1, REAL);
			if ( (	obj->obj_flags.type_flag == ITEM_KEY ) ||
				 (	!IS_SET(obj->obj_flags.extra_flags,ITEM_NOLOAD) ) ||
				 (  regen == 1 ) )
				 /*
				 (	number(1,100) <= regen_percent ) )
				 */
			{
          		obj_to = get_obj_num(ZCMD.arg3);
				if ( obj_to )
				{
          			obj_to_obj(obj, obj_to);
          			last_cmd = 'P';
				} else
					extract_obj(obj);
			}
			else
				extract_obj(obj);
        }
      break;

      case 'G': /* obj_to_char */
        /*
        if (obj_index[ZCMD.arg1].number < ZCMD.arg2 &&
        */
        if (
			(last_cmd == 'M' || last_cmd == 'G' || last_cmd == 'E')) {    
          obj = read_object(ZCMD.arg1, REAL);
			if ( (	obj->obj_flags.type_flag == ITEM_KEY )  ||
				 (	!IS_SET(obj->obj_flags.extra_flags,ITEM_NOLOAD) ) )
				/*
				 (	number(1,100) <= regen_percent ) )
				 */
			{
				if ( mob  && real_load)
				{
 	         		obj_to_char(obj, mob);
    	      		last_cmd = 'G';
				} else
					extract_obj(obj);
			}
			else
				extract_obj(obj);
        }
      break;

      case 'E': /* object to equipment list */
        /*
        if (obj_index[ZCMD.arg1].number < ZCMD.arg2 &&
        */
        if (
			(last_cmd == 'M' || last_cmd == 'G' || last_cmd == 'E')) {    
          	obj = read_object(ZCMD.arg1, REAL);
			if (	obj->obj_flags.type_flag == ITEM_KEY ) 
				 /*
				 (	!IS_SET(obj->obj_flags.extra_flags,ITEM_NOLOAD) ) )
				 (	number(1,100) <= regen_percent ) )
				 */
			{
				if ( mob  && real_load)
				{
          			equip_char(mob, obj, ZCMD.arg3);
          			last_cmd = 'E';
				} else
					extract_obj(obj);
			}
			else if (  regen == 1 ) {
				if( mob ){
          			equip_char(mob, obj, ZCMD.arg3);
          			last_cmd = 'E';
				}
				else extract_obj(obj);
			}
			else extract_obj(obj);
        }
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
        last_cmd = 'D';
      break;

      default:
        sprintf(buf, "Undefd cmd in reset table; zone %d cmd %d.\n\r",
          zone, cmd_no);
        log(buf);
        exit(0);
      break;
    }
    else
      last_cmd = 0;

  }

  zone_table[zone].age = 0;
}

#undef ZCMD

#else	// OLD_ZONE_SYSTEM


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
        exit(0);
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
    if (!i->connected)
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
      exit(0);
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
    CREATE(ch->player.title, char, strlen(st->title) + 1);
    strcpy(ch->player.title, st->title);
  }
  else
    GET_TITLE(ch) = 0;
  if (*st->description) {
    CREATE(ch->player.description, char, 
      strlen(st->description) + 1);
    strcpy(ch->player.description, st->description);
  }
  else
    ch->player.description = 0;

  ch->player.pk_num = st->pk_num; /* by process */
  ch->player.pked_num = st->pked_num; /* by process */
  ch->player.guild = st->guild; /* by process */
  for(i = 0; i< MAX_GUILD_SKILLS ; i++ ) /* by process */
   ch->player.guild_skills[i] = st->guild_skills[i];

  ch->player.time.birth = st->birth;
  ch->player.time.played = st->played;
  ch->player.time.logon  = time(0);
  ch->player.weight = st->weight;
  ch->player.height = st->height;
  ch->abilities = st->abilities;
  ch->tmpabilities = st->abilities;
  ch->points = st->points;
  for (i = 0; i <= MAX_SKILLS - 1; i++){
    ch->skills[i].learned = st->skills[i].learned;
    ch->skills[i].skilled = st->skills[i].skilled;
    ch->skills[i].recognise = st->skills[i].recognise;
  }
  ch->specials.spells_to_learn = st->spells_to_learn;
  ch->specials.alignment    = st->alignment;
  ch->specials.act          = st->act;
  ch->specials.carry_weight = 0;
  ch->specials.carry_items  = 0;

  ch->points.armor          = st->points.armor;
  ch->points.hitroll        = st->points.hitroll;
  ch->points.damroll        = st->points.damroll;

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

  /* remortal */
  ch->player.remortal = st->remortal;

  ch->regeneration          = 0;
  CREATE(GET_NAME(ch), char, strlen(st->name) +1);
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
} /* store_to_char */

void store_to_char_for_transform(struct char_file_u *st, struct char_data *ch)
{
  int i;

  GET_SEX(ch) = st->sex;
  GET_CLASS(ch) = st->class;
  GET_LEVEL(ch) = st->level;
  ch->player.short_descr = 0;
  ch->player.long_descr = 0;
  if (*st->title) {
    RECREATE(ch->player.title, char, strlen(st->title) + 1);
    strcpy(ch->player.title, st->title);
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
  ch->points.armor          = 101-st->points.armor;
  ch->points.hitroll        = 0;
  ch->points.damroll        = 0;

  RECREATE(GET_NAME(ch), char, strlen(st->name) +1);
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
  st->class    = GET_CLASS(ch);
  st->level    = GET_LEVEL(ch);
  st->abilities = ch->abilities;
  st->points    = ch->points;
  st->alignment       = ch->specials.alignment;
  st->spells_to_learn = ch->specials.spells_to_learn;
  st->act             = ch->specials.act;
  st->points.armor   = ch->points.armor;
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
  st->pk_num = ch->player.pk_num; /* by process */
  st->pked_num = ch->player.pked_num; /* by process */
  st->guild= ch->player.guild; /* by process */
  for(i = 0;i < MAX_GUILD_SKILLS ; i++ ) 
	st->guild_skills[i] = ch->player.guild_skills[i]; /* by process */

  for (i = 0; i <= MAX_SKILLS - 1; i++){
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

  /* remortal */
  st->remortal = ch->player.remortal;

  if ( GET_NAME(ch) )
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
} /* Char to store */

/* create a new entry in the in-memory index table for the player file */
int create_entry(char *name)
{
  int i;

  if (top_of_p_table == -1) {
    CREATE(player_table, struct player_index_element, 1);
    top_of_p_table = 0;
  } else if (!(player_table = (struct player_index_element *) 
      realloc(player_table, sizeof(struct player_index_element) * 
      (++top_of_p_table + 1)))) {
      perror("create entry");
      exit(1);
    }
  CREATE(player_table[top_of_p_table].name, char , strlen(name) + 1);
  /* copy lowercase equivalent of name to table field */
  for (i = 0; 
       (*(player_table[top_of_p_table].name + i) = LOWER(*(name + i)));
       i++);
  player_table[top_of_p_table].nr = top_of_p_table;
  return (top_of_p_table);
}

/* save_char_nocon is made for coin copy bug fixing by dsshin */

void save_char_nocon(struct char_data *ch, sh_int load_room )
{
  FILE *fl;
  int player_i;
  struct char_file_u st;
  
  int find_name(char *name);
  
  if ((player_i = find_name(GET_NAME(ch))) >= 0) {
    if (!(fl = fopen(PLAYER_FILE, "r+"))) {
      perror("Opening player file for reading. (db.c, save_char_nocon)");
      exit(0);
    }
    fseek(fl, (long) (player_table[player_i].nr *
		      sizeof(struct char_file_u)), 0);
    fread(&st, sizeof(struct char_file_u), 1, fl);
    char_to_store(ch, &st);
    st.load_room = load_room;
    
    fseek(fl, (long) (player_table[player_i].nr *
		      sizeof(struct char_file_u)), 0);
          fwrite(&st, sizeof(struct char_file_u), 1, fl);
	  
          fclose(fl);
  }
  else return;
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


  if ((expand = (ch->desc->pos > top_of_p_file))) {
    strcpy(mode, "a");
    top_of_p_file++;
  } else
    strcpy(mode, "r+");
  char_to_store(ch, &st);
  st.load_room = load_room;
  if ( ch->desc->pwd[0] != NUL )
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

/* rewrite player file to delete one character */
void delete_char(struct char_data *ch)
{
    struct char_file_u element;
    FILE *old, *new;
    int i, skip;
    char fname[256];

    if (IS_NPC(ch) || !ch->desc)
        return;

    if (!(old = fopen(PLAYER_FILE, "r")))
        return;
    strcpy(fname, PLAYER_FILE);
    strcat(fname, ".tmp");
    if (!(new = fopen(fname, "w")))
        return;

    skip = ch->desc->pos;
    for (i = 0; i < top_of_p_file; i++) {
        fread(&element, sizeof(struct char_file_u), 1, old);
        if (i != skip)
            fwrite(&element, sizeof(struct char_file_u), 1, new);
    }
    top_of_p_file--;

    fclose(old);
    fclose(new);
    unlink(PLAYER_FILE);
    rename(fname, PLAYER_FILE);
}

int remove_entry(struct char_data *ch)
{
    int i;
    int need_copy;
    struct player_index_element *tmp_player_table;
    struct char_data *tmp_ch;

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
            player_table[i + 1].nr--;
            tmp_player_table[i] = player_table[i + 1];
        }
        /* update already playing player's pos */
        for (tmp_ch = character_list; tmp_ch; tmp_ch = tmp_ch->next) {
            if (tmp_ch->desc && tmp_ch->desc->pos > need_copy)
                tmp_ch->desc->pos--;
        }
        /* remove old entry */
		free(player_table);
     
        /* assign new entry */
        player_table = tmp_player_table;
    }
    top_of_p_table--;
    return 1;
}

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
      exit(0);
    }
    if (strlen(tmp) + strlen(buf) > MAX_STRING_LENGTH)
    {
      log("fread_string: string too large (db.c)");
      buf[70]=0;
      fprintf(stderr,"%s\n",buf);
      exit(0);
    }
    else
      strcat(buf, tmp);

    for (point = buf + strlen(buf) - 2; point >= buf && isspace(*point);
      point--);    
    if ((flag = (*point == '~')))
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
    CREATE(rslt, char, strlen(buf) + 1);
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

  free(GET_NAME(ch));

  if (ch->player.title) {
    free(ch->player.title);
  }
  if (ch->player.short_descr){
    free(ch->player.short_descr);
  }
  if (ch->player.long_descr){
    free(ch->player.long_descr);
  }
  if(ch->player.description){
    free(ch->player.description);
  }

  for (af = ch->affected; af; af = af->next) 
    affect_remove(ch, af);

  free(ch);
}







/* release memory allocated for an obj struct */
void free_obj(struct obj_data *obj)
{
  struct extra_descr_data *this, *next_one;

  free(obj->name);
  if(obj->description)
    free(obj->description);
  if(obj->short_description)
    free(obj->short_description);
  if(obj->action_description)
    free(obj->action_description);

  for( this = obj->ex_description ; (this != 0);this = next_one ) {
    next_one = this->next;
    if(this->keyword)
      free(this->keyword);
    if(this->description)
      free(this->description);
    free(this);
	this=NULL;
  }

  free(obj);
}


/* read contents of a text file, and place in buf */
int file_to_string(char *name, char *buf)
{
  FILE *fl;
  char tmp[200];

  *buf = '\0';

  if (!(fl = fopen(name, "r")))
  {
    perror("file-to-string");
    *buf = '\0';
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
  GET_AC(ch) = 1; /* Basic Armor */
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
	int remortal[4] = {1, 2, 4, 8};

	/* *** if this is our first player --- he be God *** */
	if (top_of_p_table < 0) {
		GET_EXP(ch) = 1;
		GET_LEVEL(ch) = IMO + 3;
		void set_title(struct char_data *ch); 
		set_title(ch);
	}
	else
		GET_LEVEL(ch) = 0;

	ch->player.short_descr = 0;
	ch->player.long_descr = 0;
	ch->player.description = 0;

	ch->player.time.birth = time(0);
	ch->player.time.played = 0;
	ch->player.time.logon = time(0);

	roll_abilities(ch);

	ch->player.remortal = remortal[GET_CLASS(ch) - 1];
/*
	switch (GET_CLASS(ch)) {
		case CLASS_MAGIC_USER:
			ch->abilities.str = number(7, 11);
			ch->abilities.str_add = 0;
			ch->abilities.intel = number(13, 15);
			ch->abilities.wis = number(13, 15);
			ch->abilities.dex = number(7, 11);
			ch->abilities.con = number(8, 11);
			ch->points.armor = number(95, 100);
			break;
		case CLASS_CLERIC:
			ch->abilities.str = number(8, 12);
			ch->abilities.str_add = 0;
			ch->abilities.intel = number(12, 14);
			ch->abilities.wis = number(12, 14);
			ch->abilities.dex = (8, 12);
			ch->abilities.con = (8, 12);
			ch->points.armor = number(95, 100);
			break;
		case CLASS_THIEF:
			ch->abilities.str = number(10, 13);
			ch->abilities.str_add = 0;
			ch->abilities.intel = number(7, 10);
			ch->abilities.wis = number(7, 10);
			ch->abilities.dex = number(13, 15);
			ch->abilities.con = number(10, 13);
			ch->points.armor = number(90, 95);
			break;
		case CLASS_WARRIOR:
			ch->abilities.str = number(11, 15);
			ch->abilities.str_add = 0;
			ch->abilities.intel = number(7, 10);
			ch->abilities.wis = number(7, 10);
			ch->abilities.dex = number(10, 13);
			ch->abilities.con = number(10, 15);
			ch->points.armor = number(90, 95);
			break;
	}
*/
	GET_HIT(ch) = GET_PLAYER_MAX_HIT(ch) = 0;
	GET_MANA(ch) = GET_PLAYER_MAX_MANA(ch) = 0;
	GET_MOVE(ch) = GET_PLAYER_MAX_MOVE(ch) = 0;

	/* make favors for sex */
	if (ch->player.sex == SEX_MALE) {
		ch->player.weight = number(120,180);
		ch->player.height = number(160,200);
	}
	else {
		ch->player.weight = number(100,160);
		ch->player.height = number(150,180);
	}

	/* quest */
	ch->quest.type = 0;
	ch->quest.data = 0;
	ch->quest.solved = 0;

	/* wimpyness */
	ch->specials.wimpyness = 0;

	/* initial bonus */
#ifdef BETA_TEST
	GET_GOLD(ch) = 10000000000L;
	ch->quest.solved = 50;
	ch->specials.damnodice = 10;
	ch->specials.damsizedice = 10;
#else
	ch->points.gold = 1000;
#endif

	for (i = 0; i <= MAX_SKILLS - 1; i++) {
		if (GET_LEVEL(ch) < IMO + 3) {
			ch->skills[i].learned = 0;
			ch->skills[i].skilled = 0;
			ch->skills[i].recognise = 0;
		}
		else {
			ch->skills[i].learned = 100;
			ch->skills[i].skilled = 0;
			ch->skills[i].recognise = 0;
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
  char sf1[256],sf2[256],name[100];
  int i;

  strcpy(name,victim);
  for(i=0;name[i];++i)
    if(isupper(name[i]))
       name[i]=tolower(name[i]);
  sprintf(sf1,"%s/%c/%s.x",STASH,name[0],name);
  sprintf(sf2,"%s/%c/%s.x.y",STASH,name[0],name);
  rename(sf1,sf2);
}

void stash_char(struct char_data *ch)
{
  struct obj_data *p;
  char stashfile[256],name[100];
  FILE *fl;
  int i;
  unsigned int mask;
  void stash_contents(FILE *fl, struct obj_data *p, int wear_flag);
  char buf[MAX_OUTPUT_LENGTH];
  
  mask = sigmask(SIGUSR1) | sigmask(SIGUSR2) | sigmask(SIGINT) |
    sigmask(SIGBUS ) | sigmask(SIGSEGV) |
    sigmask(SIGPIPE) | sigmask(SIGALRM) | sigmask(SIGTERM) |
    sigmask(SIGURG ) | sigmask(SIGXCPU) | sigmask(SIGHUP);
  
  if (!ch) return;
  if (IS_NPC(ch) || ! ch->desc)
    return;
  if ( GET_NAME(ch) )
    strcpy(name, GET_NAME(ch));
  else	return;
  
  for (i = 0; name[i]; ++i)
    if (isupper(name[i]))
      name[i] = tolower(name[i]);
  sprintf(stashfile, "%s/%c/%s.x.y",STASH,name[0],name);
  
  sprintf(buf, "Stash : %s\n", stashfile);
  log(buf);
  
  sigsetmask(mask);
  if (!(fl = fopen(stashfile, "w"))) {		/* remove all data if exist */
    perror("saving PC's stash");
    sigsetmask(0);
    return;
  }
  
  fprintf(fl,"%d\n",KJHRENT);
  for (i = 0; i < MAX_WEAR; ++i)
    if ((p = ch->equipment[i])) {
      if ( p != NULL )
	stash_contents(fl, p, i);
    }
  if (ch->carrying)
    stash_contents(fl, ch->carrying, -1);
  
  fflush(fl);
  fclose(fl);
  sigsetmask(0);
}

void stash_contentsII(FILE *fp, struct obj_data *o, int wear_flag)
{
  struct obj_data *oc;
  int i;
  
  if (!o)
    return;
  if (o->obj_flags.type_flag != ITEM_KEY && !IS_OBJ_STAT(o, ITEM_NORENT)) {
    if ((oc = o->contains))
      stash_contentsII(fp, oc, ((wear_flag >= 0) ? -2 : (wear_flag - 1)));
    fprintf(fp, "%d", obj_index[o->item_number].virtual);
    fprintf(fp, " %d", wear_flag);
    for (i = 0; i < 4; i++)
      fprintf(fp, " %d", o->obj_flags.value[i]);
    for (i = 0; i < 2; i++)
      fprintf(fp, " %d %d", o->affected[i].location,
	      o->affected[i].modifier);
    // #ifdef SYPARK
    fprintf(fp," %d %d",o->obj_flags.extra_flags,o->obj_flags.gpd);
    fprintf(fp,"\n");
    fprintf(fp,"%s\n",o->name);
    fprintf(fp,"%s\n",o->short_description);
    fprintf(fp,"%s\n",o->description);
    // #endif
    fprintf(fp,"\n");
  }
  if ((oc = o->next_content))
    stash_contentsII(fp, oc, wear_flag);
}

/* Read stash file and load objects to player. and remove stash files */
/* save one item into stashfile. if item has contents, save it also */
/* old version */
void stash_contents(FILE *fl, struct obj_data *p, int wear_flag)
{
  struct obj_data *pc;
  int j;

  if(!fl) return;
  if(p==0) return;
  
  if(p->obj_flags.type_flag != ITEM_KEY && !IS_OBJ_STAT(p,ITEM_NORENT) ) {
    if((pc=p->contains))
      stash_contents(fl,pc, wear_flag >= 0 ? -2 : wear_flag - 1);
    
    fprintf(fl,"%d",obj_index[p->item_number].virtual);
    fprintf(fl," %d", wear_flag);
    for(j=0;j<4;++j)
      fprintf(fl," %d",p->obj_flags.value[j]);
    for(j=0;j<2;j++)
      fprintf(fl," %d %d",p->affected[j].location,p->affected[j].modifier);
    // #ifdef SYPARK
    fprintf(fl," %d %d",p->obj_flags.extra_flags,p->obj_flags.gpd);
    fprintf(fl,"\n");
    fprintf(fl,"%s\n",p->name);
    fprintf(fl,"%s\n",p->short_description);
    fprintf(fl,"%s\n",p->description);
    // #endif
  }
  
  if((pc=p->next_content))
    stash_contents(fl,pc, wear_flag);
}

/* Read stash file and load objects to player. and remove stash files */
/* old version */
void unstash_char(struct char_data *ch, char *filename)
{
  void wipe_stash(char *filename);
  struct obj_data *obj;
  char stashfile[256],name[100];
  FILE *fl;
  int i, n, tmp[4];
  char	tmp_str[255],*str;
  char buf[MAX_OUTPUT_LENGTH];
  
  /* for Knife rent */
#define MAX_RENT_ITEM 1000
  struct obj_data *item_stackp[MAX_RENT_ITEM];
  int item_stack[MAX_RENT_ITEM];
  int stack_count = 0;
  int where;
  unsigned int mask;
  static int loc_to_where[22] = {
    0, 1, 1, 2, 2, 3, 4, 5, 6, 7,
    8, 14, 9, 10, 11, 11, 12, 13, 15, 15,
    16, 17 };
#undef MAX_RENT_ITEM
  
  mask = sigmask(SIGUSR1) | sigmask(SIGUSR2) | sigmask(SIGINT) |
    sigmask(SIGBUS ) | sigmask(SIGSEGV) |
    sigmask(SIGPIPE) | sigmask(SIGALRM) | sigmask(SIGTERM) |
    sigmask(SIGURG ) | sigmask(SIGXCPU) | sigmask(SIGHUP);
  if (!ch) return;
  if (IS_NPC(ch) || ! ch->desc)
    return;
  if (GET_NAME(ch))
    strcpy(name,filename ? filename : GET_NAME(ch));
  else	return;
  for(i=0;name[i];++i)
    if(isupper(name[i]))
      name[i]=tolower(name[i]);
  sigsetmask(mask);
  sprintf(stashfile,"%s/%c/%s.x.y",STASH,name[0],name);
  if(!(fl=fopen(stashfile, "r"))){
    sprintf(stashfile,"%s/%c/%s.x",STASH,name[0],name);
    if(!(fl=fopen(stashfile, "r"))) {
      sigsetmask(0);
      return;
    }
  }
  
  sprintf(buf, "Unstash : %s\n", stashfile);
  log(buf);
  
  fscanf(fl,"%d",&n);
  if(n != KJHRENT) {
    log("File format error in unstash_char. (db.c)");
    sigsetmask(0);
    fclose(fl);
    return;
  }
  
  for(;;){
    if(fscanf(fl,"%d",&n) <= 0)
      break;
    
    fscanf(fl, "%d", &where);
    for(i=0;i<4;++i)
      fscanf(fl,"%d",&tmp[i]);
    obj=read_object(n,VIRTUAL);
    if(obj == 0)
      continue;
    for(i=0;i<4;++i)
      obj->obj_flags.value[i]=tmp[i];
    for(i=0;i<4;i++)
      fscanf(fl,"%d",&tmp[i]);
    for(i=0;i<2;i++){
      obj->affected[i].location=tmp[i*2];
      obj->affected[i].modifier=tmp[i*2+1];
    }
    // #ifdef	SYPARK
    fscanf(fl,"%d",&tmp[0]);
    if ( tmp[0] != -1 )
      obj->obj_flags.extra_flags = tmp[0];
    fscanf(fl,"%d",&tmp[0]);
    if ( tmp[0] != -1 )
      obj->obj_flags.gpd = tmp[0];
    fgets(tmp_str,255,fl);
    fgets(tmp_str,255,fl);
    tmp_str[strlen(tmp_str)-1] = 0;
    if ( strlen(tmp_str) != 0 )	{
      str = malloc(strlen(tmp_str)+1);
      strcpy(str,tmp_str);
      free(obj->name);
      obj->name = str;
    }
    fgets(tmp_str,255,fl);
    tmp_str[strlen(tmp_str)-1] = 0;
    if ( strlen(tmp_str) != 0 )	{
      str = malloc(strlen(tmp_str)+1);
      strcpy(str,tmp_str);
      free(obj->short_description);
      obj->short_description = str;
    }
    fgets(tmp_str,255,fl);
    tmp_str[strlen(tmp_str)-1] = 0;
    if ( strlen(tmp_str) != 0 )	{
      str = malloc(strlen(tmp_str)+1);
      strcpy(str,tmp_str);
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

#ifdef NEW_STASH /* fail */
void stash_contents(FILE *fp, struct obj_data *o, int wear_flag)
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
void unstash_char(struct char_data *ch, char *filename)
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
  static int loc_to_where[22] = {
    0, 1, 1, 2, 2, 3, 4, 5, 6, 7,
    8, 14, 9, 10, 11, 11, 12, 13, 15, 15,
    16, 17 };
  
  if (GET_NAME(ch))
    strcpy(name, filename ? filename : GET_NAME(ch));
  else return;
  
  for (i = 0; name[i]; i++)
    name[i] = tolower(name[i]);
  sprintf(fname, "%s/%c/%s.x.y", STASH, name[0], name);
  
  if (!(fp = fopen(fname, "rb"))) {
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
      obj->affected[i].modifier = value[(i << 1) +1];
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

#endif

void wipe_stash(char *filename)		/* delete id.x and id.x.y */
{
  char stashfile[256],name[50];
  int i;
  
  for(i=0;filename[i];++i)
    name[i]=isupper(filename[i]) ?
      tolower(filename[i]) : filename[i];
  name[i]=0;
  sprintf(stashfile,"%s/%c/%s.x",STASH,name[0],name);
  unlink(stashfile);
  sprintf(stashfile,"%s/%c/%s.x.y",STASH,name[0],name);
  unlink(stashfile);
}

void do_checkrent(struct char_data *ch,char *argument, int cmd)
{
  char stashfile[256],name[MAX_INPUT_LENGTH],buf[MAX_STRING_LENGTH];
  char str[255];
  FILE *fl;
  int i,j,n;
  
  one_argument(argument,name);
  if(! *name)
    return;
  for(i=0;name[i];++i)
    if(isupper(name[i]))
      name[i]=tolower(name[i]);
  sprintf(stashfile,"%s/%c/%s.x.y",STASH,name[0],name);
  if(!(fl=fopen(stashfile,"r"))){
    sprintf(buf,"%s has nothing in rent.\n\r",name);
    send_to_char(buf,ch);
    return;
  }
  buf[0]=0;
  fscanf(fl,"%d",&n);
  for(i=j=0;;){
    if(fscanf(fl,"%d",&n) <= 0) break;
    if(n < 1000) continue;
    if(n > 99999) continue;
    ++j;
    sprintf(buf+i,"%5d%c",n,(j==10) ? '\n' : ' ');
    if(j==10) 
	j=0;
    i+=5;
    fgets(str,255,fl);
    fgets(str,255,fl);
    fgets(str,255,fl);
    fgets(str,255,fl);
  }
  fclose(fl);
  strcat(buf,"\n\r");
  send_to_char(buf,ch);
  return;
}
void do_extractrent(struct char_data *ch,char *argument, int cmd)
{
  char name[MAX_INPUT_LENGTH],buf[MAX_OUTPUT_LENGTH];

  one_argument(argument,name);
  if(! *name)
    return;
  unstash_char(ch,name);
  send_to_char("OK.\n\r",ch);
  sprintf(buf,"%s grabbed rent for %s",GET_NAME(ch),name);
  log(buf);
}
void do_replacerent(struct char_data *ch,char *argument, int cmd)
{
  char name[MAX_INPUT_LENGTH],buf[MAX_OUTPUT_LENGTH];

  one_argument(argument,name);
  if(! *name)
    return;
  stash_char(ch);
  move_stashfile(name);
  send_to_char("OK.\n\r",ch);
  sprintf(buf,"%s replaced rent for %s",GET_NAME(ch),name);
  log(buf);
}

void do_rent(struct char_data *ch, int cmd, char *arg)
{
	// sh_int save_room;
	int i;
	void wipe_obj(struct obj_data *obj);

	if (IS_NPC(ch))
		return;
	if (cmd) {
		if (!IS_SET(world[ch->in_room].room_flags, RENT)) {
			send_to_char("You cannot rent here.\n\r", ch);
			return;
		}
		send_to_char("You retire for the night.\n\r", ch);
		act("$n retires for the night.", FALSE, ch, 0, 0, TO_NOTVICT);
	}
	stash_char(ch);	/* clear file.x and save into file.x */
	move_stashfile(ch->player.name);	/* move file.x to file.x.y */
	wipe_obj(ch->carrying);
	for (i = 0; i < MAX_WEAR; i++)
		if (ch->equipment[i]) {
			extract_obj(unequip_char(ch,i));
			ch->equipment[i] = 0;
		}
	wipe_obj(ch->carrying);
	ch->carrying = 0;
	// save_room = ch->in_room;
	extract_char(ch);
/*
	ch->in_room = world[save_room].number;
	save_char(ch, ch->in_room);
*/
	return;
}

void wipe_obj(struct obj_data *obj)
{
	if (obj) {
		wipe_obj(obj->contains);
		wipe_obj(obj->next_content);
		if (obj->in_obj)
			obj_from_obj(obj);
		extract_obj(obj);
	}
}
