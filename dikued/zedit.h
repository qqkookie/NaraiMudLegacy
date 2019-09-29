/****************************************************************************
 *                                                                          *
 *           zedit.h (c) by Georg Essl in 1993                              *
 *                                                                          *
 ****************************************************************************/

/*** DEFINES ***/

#define ZE_NOPURGEDOOR 0
#define ZE_PURGEDOOR 1

/* Zone-Editing + Simulating Structs */
struct ze_mob_eq
{
  int pos;
  int obj;
};


struct ze_mob
{
  int nr;
  int maxexist;
  int eq_count;
  struct ze_mob_eq *eq;
  int inv_count;
  int *inv;
};

struct ze_obj
{
  int nr;
  int maxexist;
  int inv_count;
  int *inv;
};

struct ze_zone
{
  int room_nr;
  int door[6];
  int mob_count;
  int obj_count;
  struct ze_mob *mobs;
  struct ze_obj *objs;
};

/****** STRUCTS from "db.h" *****/

/* structure for the reset commands */
struct reset_com
{
	char command;   /* current command                      */
	int  if_flag;   /* if TRUE: exe only if preceding exe'd */
	int arg1;       /*                                      */
	int arg2;       /* Arguments to the command             */
	int arg3;       /*                                      */

	/*
	*  Commands:              *
	*  'M': Read a mobile     *
	*  'O': Read an object    *
	*  'G': Give obj to mob   *
	*  'P': Put obj in obj    *
	*  'G': Obj to char       *
	*  'E': Obj to char equip *
	*  'D': Set state of door *
	*/
};



/* zone definition structure. for the 'zone-table'   */
struct zone_data
{
	char *name;             /* name of this zone                  */
        int virt_number;
	int lifespan;           /* how long between resets (minutes)  */
	int age;                /* current age of this zone (minutes) */
	int top;                /* upper limit for rooms in this zone */

	int reset_mode;         /* conditions for reset (see below)   */
	struct reset_com *cmd;  /* command table for reset	           */

        int zonemax;
        struct ze_zone *zone;
	/*
	*  Reset mode:                              *
	*  0: Don't reset, and don't update age.    *
	*  1: Reset if no PC's are located in zone. *
	*  2: Just reset.                           *
	*/
};

/*** PROTOTYPES ***/

void ze_FindZones(char *);
void ze_InitIEdit(void);
void ze_CloseIEdit(void);
int  ze_GetRealNumber(int);
void ze_GiveItemMob(int,int);
void ze_GiveItemItem(int,int);
void ze_WearItemMob(int,int,int);
int  ze_FindNrZone(int,char *);
int  ze_FindNrMobRoom(int,int,struct ze_zone *);
int  ze_FindMobRoom(int,char *,struct ze_zone *);
int  ze_FindNrItemRoom(int,int,struct ze_zone *);
int  ze_FindItemRoom(int,char *,struct ze_zone *);
void ze_ShowZone(int);
void ze_EditZone(int);
void ze_PurgeZone(int);
void ze_PurgeMobile(int);
void ze_PurgeItem(int);
void ze_PurgeAll(void);
void ze_CloneZone(int,int);
void ze_DoorState(int,int);

void ze_SetFile(char *);
void ze_SetTemp(char *);
void ze_SetDBPath(char *);

void ze_InitZEdit(void);
void ze_CloseZEdit(void);

void ze_LoadZone(int);
void ze_LoadMobile(int);
void ze_LoadItem(int);

void ze_free_zoneroom(struct zone_data *,int,int);
int ze_GetRealNumber(int);

struct ze_zone *ze_getroom(struct zone_data *,int);

