
#include "typedef.h"

/* The following defs are for obj_data  */

/* ---------------    struct obj-flag_data    ---------------- */

struct obj_flag_data {
    int value[4];	/* Values of the item (see list)    */
    byte type_flag;	/* Type of item			    */
    int wear_flags;	/* Where you can wear it	    */
    int extra_flags;	/* If it hums,glows etc	   	    */
    int weight;	/* Weigt what else	   	    */
    int cost;		/* Value when sold (gp.)	    */
    int timer;		/* Timer for object	   	    */
    long bitvector;	/* To set chars bits	   	    */
    int gpd;		/* general purpose data	   	    */
};

#define OBJ_NOTIMER		-7000000

/* For 'type_flag' */
/* NOTE: REMOVED rarely used ITEMTYPE */
// NOTE: Revive ITEM_MISSILE, ITEM_BOAT
// Add ITEM_WINGS, Remove ITEM_TRASH to ITEM_OTHER
#define ITEM_LIGHT		1
#define ITEM_SCROLL		2
#define ITEM_WAND		3
#define ITEM_STAFF		4
#define ITEM_WEAPON		5
#define ITEM_FIREWEAPON 	6
// NOTE: it was ITEM_MISSILE
#define ITEM_PROJECTILE		7
#define ITEM_TREASURE		8
#define ITEM_ARMOR		9
#define ITEM_POTION		10
#define ITEM_WORN		11
#define ITEM_OTHER		12
// NOTE: ITEM_TRASH, ITEM_TRAP, ITEM_PEN, ITEM_CIGA/DRUG MOVED TO ITEM_OTHER
// #define ITEM_TRASH		13
// #define ITEM_TRAP		14 */
#define ITEM_CONTAINER 		15
#define ITEM_NOTE		16
#define ITEM_DRINKCON		17
#define ITEM_KEY		18
#define ITEM_FOOD		19
#define ITEM_MONEY		20
/* NOTE: #define ITEM_PEN		21 */
#define ITEM_BOAT		22
#define ITEM_WINGS		23
/* NOTE: #define ITEM_CIGA		23 */
/* NOTE: #define ITEM_DRUG		24 */

// NOTE: sub item type (obj->value[0]): No longer used
// #define ITEMSUB_BOAT 		7
// #define ITEMSUB_WING 		8

/* Bitvector For 'wear_flags' */

#define ITEM_TAKE		1
#define ITEM_WEAR_FINGER	2
#define ITEM_WEAR_NECK		4
#define ITEM_WEAR_BODY		8
#define ITEM_WEAR_HEAD		16
#define ITEM_WEAR_LEGS		32
#define ITEM_WEAR_FEET		64
#define ITEM_WEAR_HANDS		128
#define ITEM_WEAR_ARMS		256
#define ITEM_WEAR_SHIELD	512
#define ITEM_WEAR_ABOUT		1024
#define ITEM_WEAR_WAISTE	2048
#define ITEM_WEAR_WRIST		4096
#define ITEM_WIELD		8192
#define ITEM_HOLD		16384
#define ITEM_THROW		32768
#define ITEM_WEAR_KNEE		65536
#define ITEM_WEAR_ABOUTLEGS	131072
#define ITEM_WEAR_BACK		262144	/* added by process */

/* Bitvector for 'extra_flags' */

#define ITEM_GLOW		1
#define ITEM_HUM		2
#define ITEM_DARK		4
#define ITEM_LOCK		8
#define ITEM_EVIL		16
#define ITEM_INVISIBLE		32
#define ITEM_MAGIC		64
#define ITEM_NODROP		128
#define ITEM_BLESS		256
#define ITEM_ANTI_GOOD		512	/* not usable by good people    */
#define ITEM_ANTI_EVIL		1024	/* not usable by evil people    */
#define ITEM_ANTI_NEUTRAL	2048	/* not usable by neutral people */
#define ITEM_NOLOAD		4096
#define ITEM_ANTI_MAGE		8192	/* not usable by mage */
#define ITEM_ANTI_CLERIC	16384	/* not usable by cleric */
#define ITEM_ANTI_THIEF		32768	/* not usable by thief */
#define ITEM_ANTI_WARRIOR	65536	/* not usable by warrior */
#define ITEM_NORENT		131072	/* cannot rent ,added by process */
#define ITEM_ANTI_POLICE	262144	/* not usable by police */
#define ITEM_ANTI_OUTLAW	524288	/* not usable by outlaw */
#define ITEM_ANTI_ASSASSIN	1048576	/* not usable by assasin */
#define ITEM_NOCOPY		2097152	/* do not SAVE in the locker room */

/* Some different kind of liquids */

#define LIQ_WATER		0
#define LIQ_BEER		1
#define LIQ_WINE		2
#define LIQ_ALE			3
#define LIQ_DARKALE		4
#define LIQ_WHISKY		5
#define LIQ_LEMONADE		6
#define LIQ_FIREBRT		7
#define LIQ_LOCALSPC		8
#define LIQ_SLIME		9
#define LIQ_MILK		10
#define LIQ_TEA			11
#define LIQ_COFFE		12
#define LIQ_BLOOD		13
#define LIQ_SALTWATER		14
#define LIQ_SUNDEW		15
#define LIQ_NECTAR		16
#define LIQ_GOLDEN_NECTAR	17
#define LIQ_MAKOLI		18
#define LIQ_CONDITIOIN		19

/* for containers  - value[1] */

#define CONT_CLOSEABLE		1
#define CONT_PICKPROOF		2
#define CONT_CLOSED		4
#define CONT_LOCKED		8

/* ---------------------------------------------------------------- */

/* Used in OBJ_FILE_ELEM *DO*NOT*CHANGE* */
struct obj_affected_type {
    byte location;	/* Which ability to change (APPLY_XXX) */
    sbyte modifier;	/* How much it changes by              */
};

/* NOTE: Extra description to 'look at' */
struct extra_descr_data {
    char *keyword;	/* Keyword in look/examine  */
    char *description;	/* What to see              */
    struct extra_descr_data *next;	/* Next in list */
};

#define MAX_OBJ_AFFECT 2	/* Used in OBJ_FILE_ELEM *DO*NOT*CHANGE* */

/* ======================== Structure for object ========================= */

struct obj_data {
    sh_int item_number;			/* Where in data-base		    */
    sh_int in_room;			/* In what room -1 when conta/carr  */
    struct obj_flag_data obj_flags;	/* Object information		    */
    struct obj_affected_type
        affected[MAX_OBJ_AFFECT];	/* Which abilities in PC to change  */
    char *name;				/* Title of object :get etc.	    */
    char *description;			/* When in room			    */
    char *short_description;		/* when worn/carry/in cont.	    */
    char *action_description;		/* What to write when used	    */
    struct extra_descr_data *ex_description;	/* extra descriptions	    */
    struct char_data *carried_by;	/* Carried by :NULL in room/conta   */

    struct obj_data *in_obj;		/* In what object NULL when none    */
    struct obj_data *contains;		/* Contains objects		    */

    struct obj_data *next_content;	/* For 'contains' lists		    */
    struct obj_data *next;		/* For the object list		    */
};

/* ---------------------------------------------------------------- */
/* Object And Carry related macros */

#define X98(sub, obj)  \
  ( (( !IS_SET((obj)->obj_flags.extra_flags, ITEM_INVISIBLE) ||   \
       IS_AFFECTED((sub),AFF_DETECT_INVISIBLE) ) &&               \
       !IS_AFFECTED((sub),AFF_BLIND)) &&    \
     ( IS_LIGHT(sub->in_room) || IS_AFFECTED((sub),AFF_INFRAVISION)) )

#define CAN_SEE_OBJ(sub,obj) (X98(sub,obj)||IS_WIZARD(sub))

#define GET_ITEM_TYPE(obj) ((obj)->obj_flags.type_flag)

#define CAN_WEAR(obj, part) (IS_SET((obj)->obj_flags.wear_flags,part))

#define GET_OBJ_WEIGHT(obj) ((obj)->obj_flags.weight)

#define CAN_CARRY_W(ch) (str_app[STRENGTH_APPLY_INDEX(ch)].carry_w + CARRY_WEIGHT_BASE)

#define CAN_CARRY_N(ch) (GET_DEX(ch)/2+GET_LEVEL(ch)/2 + 5 + CARRY_NUM_BASE)

/* NOTE: OLD IS_CARRYING_W() is renamed as GET_CARRYING_W()  */
#define GET_CARRYING_W(ch) ((ch)->specials.carry_weight)

/* NOTE: OLD IS_CARRYING_N() is renamed as GET_CARRYING_N()  */
#define GET_CARRYING_N(ch) ((ch)->specials.carry_items)

#define IS_OBJ_STAT(obj,stat) (IS_SET((obj)->obj_flags.extra_flags,stat))

/* NOTE: NEW! macro to get VIRUAL number of the object */
#define GET_OBJ_VIRTUAL(obj) (obj_index[(obj)->item_number].virtual)

#define OBJ_VALUE0(obj) ((obj)->obj_flags.value[0])
#define OBJ_VALUE1(obj) ((obj)->obj_flags.value[1])
#define OBJ_VALUE2(obj) ((obj)->obj_flags.value[2])
#define OBJ_VALUE3(obj) ((obj)->obj_flags.value[3])

#ifdef UNUSED_CODE
/* NOTE: Remove hardly used macro */
#define CAN_CARRY_OBJ(ch,obj)  \
   (((GET_CARRYING_W(ch) + GET_OBJ_WEIGHT(obj)) <= CAN_CARRY_W(ch)) &&   \
    ((GET_CARRYING_N(ch) + 1) <= CAN_CARRY_N(ch)))

#define CAN_GET_OBJ(ch, obj)   \
   (CAN_WEAR((obj), ITEM_TAKE) && CAN_CARRY_OBJ((ch),(obj)) &&  \
    CAN_SEE_OBJ((ch),(obj)))
#endif 		/* UNUSED_CODE */

/* ========================= Structure for room ========================== */
/*  NOTE: Room exit direction	info 				*/

struct room_direction_data {
    char *general_description;	/* When look DIR.    */
    char *keyword;	/* for open/close 		*/
    sh_int exit_info;	/* Exit info 			*/
    sh_int key;		/* Key's number (-1 for no key)    */
    sh_int to_room;	/* Where direction leeds (NOWHERE) */
};

/* For 'exit_info types' */

#define EX_ISDOOR		1
#define EX_CLOSED		2
#define EX_LOCKED		4
#define EX_RSCLOSED		8
#define EX_RSLOCKED		16
#define EX_PICKPROOF		32
#define EX_NOPHASE		64

/* ----------------------------------------------------------- */

struct room_data {
    sh_int number;		/* Rooms number			*/
    sh_int zone;		/* Room zone (for resetting)	*/
    int sector_type;		/* sector type (move/hide)	*/
    char *name;			/* Rooms name 'You are ...'	*/
    char *description;		/* Shown when entered		*/
    struct extra_descr_data *ex_description;	/* for examine/look   */
    struct room_direction_data *dir_option[6];	/* Directions	      */
    int room_flags;		/* DEATH,DARK ... etc		*/
    byte light;			/* Number of lightsources in room      */
    int (*funct) (struct char_data * ch, int cmd, char *arg);
				/* special procedure		*/

    struct obj_data *contents;	/* List of items in room	*/
    struct char_data *people;	/* List of NPC / PC in room	*/
};

/* ======================================================================= */

/* The following defs are for room_data  */

#define NOWHERE    -1	/* nil reference for room-database    */

/* Bitvector For 'room_flags' */

#define DARK			1
#define NOSUMMON		2
#define NO_MOB			4
#define INDOORS			8
// #define LAWFUL			16
// #define NEUTRAL			32
// #define CHAOTIC			64
#define NO_MAGIC		128
// #define TUNNEL			256
#define PRIVATE			512
#define OFF_LIMITS		1024
#define RENT			2048
#define NORELOCATE		4096
#define EVERYZONE		8192	/* tar char to all zone */
// #define JAIL			16384	/* added by process */ /* NOTE: Removed */
#define ROOM_NOTRACK		16384	/* NOTE: Track won't go through    */
#define RESTROOM		32768	/* added by process */

/* NOTE: Additional room_flags bits for "house.c"  */
#define ROOM_HOUSE		(1 << 20)  /* (R) Room is a house       */
#define ROOM_HOUSE_CRASH	(1 << 21)  /* (R) House needs saving    */
#define ROOM_BFS_MARK		(1 << 22)  /* (R) breath-first srch mark */


/* For 'dir_option' */

#define NORTH			0
#define EAST			1
#define SOUTH			2
#define WEST			3
#define UP			4
#define DOWN			5

/* For 'sector types' */

#define SECT_INSIDE		0
#define SECT_CITY		1
#define SECT_FIELD		2
#define SECT_FOREST		3
#define SECT_HILLS		4
#define SECT_MOUNTAIN		5
#define SECT_WATER_SWIM		6
#define SECT_WATER_NOSWIM	7
#define SECT_SKY		8

/* ----------    Room and Door related macros    ------------  */

#define IS_DARK(room)  (!world[room].light \
			&& IS_SET(world[room].room_flags, DARK))

#define IS_LIGHT(room)  (world[room].light \
			|| !IS_SET(world[room].room_flags, DARK))

#define OUTSIDE(ch) (!IS_SET(world[(ch)->in_room].room_flags,INDOORS))

#define EXIT(ch, door)  (world[(ch)->in_room].dir_option[door])

#define CAN_GO(ch, door) (EXIT(ch,door) && (EXIT(ch,door)->to_room != NOWHERE)\
                          && !IS_SET(EXIT(ch, door)->exit_info, EX_CLOSED))

/* NOTE: from "house.c" */
#define ROOM_FLAGS(rn)		(world[(rn)].room_flags)
#define ROOM_FLAGGED(loc, flag) (IS_SET(ROOM_FLAGS(loc), (flag)))

#define TOROOM(room, dir) (world[room].dir_option[dir] ? \
			    world[room].dir_option[dir]->to_room : NOWHERE)

#define	NUM_OF_DIRS		6

/* ------  Global variables about all objects and world  ------- */

extern struct index_data *obj_index;
extern struct obj_data *object_list;
extern struct room_data *world;
extern int top_of_world;   /* ref to the top element of world */

/* ---------------------------------------------------------------- */
