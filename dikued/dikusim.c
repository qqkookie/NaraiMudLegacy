/*****************************************************************************
 *                                                                            *
 * Project : Diku-Mud-World-Creator                                           *
 *                                                                            *
 * Program : Diku-Mud-World-Simulator                                         *
 * Version : 1.0                                                              *
 * Last changed on: 27.7.93                                                   *
 *                                                                            *
 * Code by: Georg Essl (essl@fstgds06.tu-graz.ac.at)                          *
 *                                                                            *
 *****************************************************************************/

/******* SIMULATION ******/

/***** Original Includes *****/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <sys/types.h>

/****** Project Includes *****/
#include "strext.h"
#include "dutil.h"
#include "dikusim.h"
#include "iedit.h"
#include "medit.h"
#include "zedit.h"

/****** DEFINES *****/

/* Diku-Sim */
#define SIM_MAXSTRLEN 128

#define SIM_M_QUIT   0

#define REAL 0
#define VIRTUAL 1

/***** ERROR-DEFINES *****/

#define SIM_ERR_NOERROR      0

/***** TYPEDEFS *****/


typedef   signed char  sbyte;
typedef unsigned char  ubyte;
#if !defined(sgi) && !defined(m88k) &&!defined(_AIX)
// typedef unsigned long  ulong;
#endif
typedef   signed short sh_int;

/***** Structs *****/
struct dirs
{
	int    isset;              /* Is this direction defined ?     */
    char   *gen_desc;          /* What look <direction> gives     */
    char   *doorname;          /* Key words for the door name     */
    ulong  doorflag;           /* Bit vector for the door flags   */
    int    keynumber;          /* Key number (itemno/level)       */
    int    toroomno;           /* Where does it lead ?            */
};

struct extra_desc
{
	char	*key;		   /* Keyword for the extra objects   */
    char    *desc;             /* Description of the the object   */
    struct extra_desc *next;   /* Next in list                    */
};

struct room_data
{
	int room_number;
    int virt_number;

    char *name;               /* Name of the room                  */
    char *description;        /* Room description                  */

    int zone_nr;              /* Number of the zone it belongs to  */
    ulong flags;              /* Bitvector for room flags          */
    int sector;               /* Sector Type of the room           */
    struct dirs exits[6];     /* Union containing info about exits */
    /* 0=N,1=E,2=S,3=W,4=U,5=D           */
    struct extra_desc *extra;  /* Union for the extra objects       */
};

/***** Global Vars *****/

int sim_curroom;
int sim_currzone;

/***** EXTERNS *****/

extern int zw_max;
extern char **zw_str;
extern char **zd_str;

extern const char *direction[6];
extern struct zone_data *ze_activezone;

/***** Vars from db.c from original DikuMud! *****/

extern struct index_data *room_index;         /* index table for room file       */

/*** PROTOS ***/

struct room_data *read_room(int,int);
void free_room(struct room_data *);

/*** FUNCTIONS ***/

int sim_get_curr_room(void)
{
    return(sim_curroom);
}

void sim_set_curr_room(int wint)
{
    sim_curroom=wint;
}

void sim_lookdir(struct ze_zone *zone,struct room_data *room,int dir)
{
    if((room->exits[dir].isset==1) && (room->exits[dir].gen_desc!=NULL))
    {
        printf("%s\n",room->exits[dir].gen_desc);
    }
    else
        printf("You see nothing special.\n");

    if(room->exits[dir].doorname!=NULL)
    {
        printf("Door (%s) ",room->exits[dir].doorname);
        if(room->exits[dir].keynumber!=-1)
            printf("Key [%d]",room->exits[dir].keynumber);
        if(zone!=NULL)
        {
            printf(" is %s.\n",zd_str[zone->door[dir]]);
        }
        else
            printf(" is %s.\n",zd_str[0]);
    }
}

void sim_look(struct room_data *room)
{
    int wint;
    int wint2;
    char *buf;
    int door;
    char  buf2[255] ;
    static char *exits[] = {
        "n ", "e ", "s ",
        "w ", "u ", "d "
    };
    static char *closedexits[] = {
        "(n) ", "(e) ", "(s) ",
        "(w) ", "(u) ", "(d) "
    };

    printf("%s [%d]\n",room->name,room->virt_number);
    printf(room->description);

    buf2[0] = 0 ;
    for (door = 0; door < 6; door++) {
        if (room->exits[door].isset ==1) {
            strcat(buf2, exits[door]) ;
        }
    }

    if (*buf2) {
        printf("< Exits: %s>\n\r", buf2) ;
    }
    else
        printf("\n\r< Exits: None >\n\r");

    if(ze_activezone!=NULL && ze_activezone->zone!=NULL)
    {
        for(wint2=0;wint2<ze_activezone->zonemax;wint2++)
        {
            if(ze_activezone->zone[wint2].room_nr==room->virt_number) break;
        }

        if(wint2!=ze_activezone->zonemax)
        {
            for(wint=0;wint<ze_activezone->zone[wint2].mob_count;wint++)
            {
                buf=me_ReadLDesc(ze_activezone->zone[wint2].mobs[wint].nr);
                if(buf!=NULL)
                {
                    printf("%s",buf);
                    free(buf);
                }
                else
                    printf("Nonexisting Mob [%d].",ze_activezone->zone[wint2].mobs[wint].nr);
            }
            for(wint=0;wint<ze_activezone->zone[wint2].obj_count;wint++)
            {
                buf=ie_ReadLDesc(ze_activezone->zone[wint2].objs[wint].nr);
                if(buf!=NULL)
                {
                    printf("%s\n",buf);
                    free(buf);
                }
                else
                    printf("Nonexisting Obj [%d].\n",ze_activezone->zone[wint2].objs[wint].nr);
            }
        }
    }
}

void sim_lookinv(int *inv,int inv_count)
{
    int i;
    char *name;

    for(i=0;i<inv_count;i++)
    {
        name=ie_ReadSDesc(inv[i]);
        if(name!=NULL)
        {
            printf("%s\n",name);
            free(name);
        }
        else
        {
            printf("Nonexisting Obj [%d].\n",inv[i]);
        }
    }
}

void sim_exits(struct room_data *room)
{
    int wint;
    int wint2=0;

    printf("Exits:\n");
    for(wint=0;wint<6;wint++)
    {
        printf("%s: ",direction[wint]);
        if(room->exits[wint].isset==1)
        {
            printf("[%d]",room->exits[wint].toroomno);
        }
        else
            printf("None");
        if(room->exits[wint].isset!=0 && room->exits[wint].doorname!=NULL)
        {
            printf(" (Description Set)\n");
        }
        else
        {
            printf("\n");
        }
    }
}

/* Look at Mobile for DikuCli */

void sim_LookMobile(int nr)
{
    int i,i2;
    char *name;
    struct ze_zone *room;

    if((room=ze_getroom(ze_activezone,sim_get_curr_room()))!=NULL)
    {
        printf("The Mobile has equipted:\n");

        for(i=0;i<room->mobs[nr].eq_count;i++)
        {
            i2=room->mobs[nr].eq[i].pos;
            name=ie_ReadSDesc(room->mobs[nr].eq[i].obj);
            if(i2<1 || i2>zw_max)
                printf("On invalid position: ");
            else
                printf("%s : ",zw_str[i2]);
            if(name!=NULL)
            {
                printf("%s\n",name);
                free(name);
            }
            else
                printf("Nonexisting Obj [%d].\n",room->mobs[nr].eq[i].obj);
        }
        printf("You peek into the inventory:\n");
        sim_lookinv(room->mobs[nr].inv,room->mobs[nr].inv_count);
    }
}

/* Look at Item for DikuCli */

void sim_LookItem(int nr)
{
    struct ze_zone *room;

    if((room=ze_getroom(ze_activezone,sim_get_curr_room()))!=NULL)
    {
        printf("The Item contains:\n");
        sim_lookinv(room->objs[nr].inv,room->objs[nr].inv_count);
    }
}

/* Look in Direction for DikuCli */

void sim_LookDir(int dir)
{
    struct ze_zone *room;
    struct room_data *wroom;

    room=ze_getroom(ze_activezone,sim_get_curr_room());
    wroom=read_room(sim_get_curr_room(),VIRTUAL);
    sim_lookdir(room,wroom,dir);
    free_room(wroom);

}

/* Look for DikuCli */

void sim_DoLook(int nr)
{
    struct room_data *wroom;

    wroom=read_room(nr,REAL);
    if(wroom->zone_nr!=sim_currzone)
    {
        ze_LoadZone(wroom->zone_nr);
        sim_currzone=wroom->zone_nr;
    }
    sim_look(wroom);
    free_room(wroom);
}

/* Goto for DikuCli */

void sim_DoGoto(int nr)
{
    sim_set_curr_room(room_index[nr].virtual);
    sim_DoLook(nr);
}

/* Exits for DikuCli */

void sim_DoExits(int nr)
{
    struct room_data *wroom;

    wroom=read_room(nr,REAL);
    sim_exits(wroom);
    free_room(wroom);
}

/* Get Exit Room Nr for DikuCli */

int sim_GetExit(int nr,int dir)
{
    struct room_data *wroom;
    int wint;

    wroom=read_room(nr,REAL);
    if(wroom->exits[dir].isset==1)
    {
        wint=wroom->exits[dir].toroomno;
        free_room(wroom);
        return(wint);
    }
    else
    {
        free_room(wroom);
        return(-1);
    }
}

/* Initialize the DikuSim vars */

void sim_InitDikuSim(void)
{
    sim_curroom=room_index->virtual;
    sim_currzone= -1;
}


