/* **********************************************************************
   *  file: gamedb.h , Database module.  		Part of DIKUMUD *
   *  Usage: Loading/Saving chars and booting world.			*
   ********************************************************************** */

#include "typedef.h"

/* --------------      Some Login banners    ----------------- */

#define  MAGICKEY	"op Sesame$"

#define MENU         \
"GOOD luck to you, Adventurous MUDDER.\r\n\r\n\
0) Get away from Mud.\r\n\
1) Into the MUD and Enjoy playing.\r\n\
2) Modify your description.\r\n\
3) Change secret key.\r\n\
4) Delete character.\r\n\r\n\
   What number? : "


#define GREETINGS \
"\r\n\r\n\
                   -- * ---<--<--<@-@>-->-->-- * -- * --\r\n\
       -----===<< * * *   N  A  R  A  I - M  U  D   * * * >>===-----\r\n\
                   -- * ---<--<--<@-@>-->-->-- * -- * --\r\n\r\n\
                Land of the Brave, the Beauty and the Beast.\r\n\r\n\
                      Continued from Old NaraiMUD\r\n\
                    Derived from EVE-MUD and KIT-MUD\r\n\r\n\
                    This MUD started 1997. 9. 11. .....\r\n\r\n"

#define WELC_MESSG \
"-------------<< N-A-R-A-I >>-----------<< M-U-D >>---------------\n\r\
Welcome to the Living Nightmare. May your visit here be... Interesting.\r\n\r\n\
Heavenly voice whisperes 'Your fate and destiny lies beyond this world...'.\r\n\
Yo! poor Mortals... Worship the Great Old Goddess!
Fall on your knees and kiss her feet.\r\n\
Glory to the Goddess, Glory to Narai...\r\n\
\r\n   -----===    GOLDEN RULES OF NARAI    ===-----\r\n\
\r\n   **** PK is LEGAL for any time, any way and any reason!\r\n\
\r\n   **** NO REIMBURSE !!!\r\n\
\r\n   **** MULTI/ROBOT PLAYING is *NOT* allowed.\r\n\r\n"

#define STORY     \
"Once upon a time, long and long years ago...... Ummm...\r\n\
 ....Cookie doesn't say more. You'd better entering the Game.\r\n\r\n"

/*********************************************************************/

/* ------------   data files used by the game system   ------------------*/ 
/*  default directory is defined in comm.h : usually lib */

#define WORLD_FILE        "tinyworld.wld"	/* room definitions */
#define MOB_FILE          "tinyworld.mob"	/* monster prototypes */
#define OBJ_FILE          "tinyworld.obj"	/* object prototypes */
#define ZONE_FILE         "tinyworld.zon"	/* zone defs & command tables */

#define SHOP_FILE "tinyworld.shp" 
#define ALL_WORLD_FILE "world/world_files"


#ifdef NO_DEF
#define CREDITS_FILE      "credits"	/* for the 'credits' command  */
#define IMOTD_FILE	  "imotd"	/* MOTD for immortals         */
#define TIME_FILE         "time"	/* game calendar information  */
#define HELP_PAGE_FILE    "help"	/* for HELP <CR>              */
#endif  	/* NO_DEF */ 

#define PLAYER_FILE       "players"	/* the player database        */
#define MESS_FILE         "messages"	/* damage message             */
#define SOCMESS_FILE      "actions"	/* messgs for social acts     */
/* NOTE: Chnage HELP file name from "help_table" to simpler "help".   */
#define HELP_KWRD_FILE    "help"	/* for HELP <keywrd>          */
#define PAINT_DIR 	  "paints"      /* NOTE: NEW! paints directory   */

/* NOTE: "wizards" + "credits" -> "wizards" ; "motd" + "imotd" -> "motd"  */
#define NEWS_FILE         "news"	/* for the 'news' command     */
#define NEWS_OLD_FILE 	  "news_old"    /* NOTE: NEW! archived news   */
#define MOTD_FILE         "motd"	/* messages of today          */
#define PLAN_FILE         "plan"	/* for god's plan             */
#define WIZARDS_FILE      "wizards"	/* for wizards command : cyb  */

#define STASH		  "stash"	/* stash directory	      */
#define STASH_EXT	  "sta"
/* NOTE: stash file name changed from "name.x.y" to "name.sta" */


/* ================================================================= */
/* NOTE: "players" in save file structure */

/*  CHAR_FILE_U *DO*NOT*CHANGE */
struct char_file_u {
    byte sex, class, level;
    time_t birth;
    int played, life;
    ubyte weight, height;
    char title[80];
    ubyte guild_skills[MAX_GUILD_SKILLS];
    char description[300];
    int pked_num;
    int no_of_change_guild;
    byte guild;
    int pk_num;
    sh_int load_room;
    struct char_ability_data abilities;
    struct char_point_data points;
    struct char_skill_data skills[MAX_SKILLS];
    struct affected_type affected[MAX_AFFECT];
    struct quest_data quest;
    ubyte spells_to_learn;
    int alignment;
    time_t last_logon;
    unsigned act;
    unsigned long bank;
    char name[20];
    char pwd[11];
    int conditions[3];
    byte remortal;
    int wimpyness;

    /* new attributes */
    byte damnodice;
    byte damsizedice;
    byte page_len; 	/* NOTE: page_string() Page length */
    char filler1; 	/* NOTE: justify 4 byte aligntment boundary */
    time_t jail_time; 	/* NOTE: New. Time to be released from jail  */

    byte reserved[52];  /* 60 - 8 byte *//* NOTE: Reduced by 6 byte */
};

/* --------------------------------------------------------------- */

struct player_index_element {
    char *name;
    int index;
}; 

extern struct player_index_element *player_table;  /* index to player file */
extern int top_of_p_table;

/* --------------- public procedures in db.c --------------------- */

#define REAL 0
#define VIRTUAL 1

struct char_data *read_mobile(int nr, int type); 
/* read an object from OBJ_FILE */
extern struct obj_data *read_object(int nr, int type);

void save_char(struct char_data *ch);
/* void init_player(struct char_data *ch); */

void stash_char(struct char_data *ch); 
void unstash_char(struct char_data *ch, char *stashname);
void wipe_stash(char *name);

/* returns the real number of the room with given virtual number */
extern int real_room(int virtual);

/* returns the real number of the monster with given virtual number */
extern int real_mobile(int virtual);

/* returns the real number of the object with given virtual number */
extern int real_object(int virtual);

/* read and allocate space for a '~'-terminated string from a given file */
extern char *fread_string(FILE * fl);

/* NOTE: Increase max buf size to 10-fold (30K), and save memory, too  */
/*     If 2nd parameter is null, use malloc()'ed memory and return it.   */
/* read contents of a text file, and place in buf */
extern char *file_to_string(char *name, char *sbuf);

extern char *lookup_db(char *keyword);

/* --------------------------------------------------------------- */
