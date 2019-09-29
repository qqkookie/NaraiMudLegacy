/*****************************************************************************
*                                                                            *
* Project : Diku-Mud-World-Creator                                           *
*                                                                            *
* Program : Diku-Mud-Zone-Editor                                             *
* Version : 1.0                                                              *
* Last changed on:  27.7.93                                                   *
*                                                                            *
* Code by: Georg Essl (essl@fstgds06.tu-graz.ac.at)                          *
*                                                                            *
*****************************************************************************/

/******* ZONES ******/

/***** Original Includes *****/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <sys/types.h>

/****** Project Includes *****/
#include "strext.h"
#include "dutil.h"
#include "iedit.h"
#include "medit.h"
#include "redit.h"
#include "zedit.h"
#include "dikusim.h"

/****** DEFINES from "structs.h" *****/

#define NOWHERE    -1

/***** TYPEDEFS *****/

typedef   signed char  sbyte;
typedef unsigned char  ubyte;
#if !defined(sgi) && !defined(m88k) &&!defined(_AIX)
typedef unsigned long  ulong;
#endif
typedef   signed short sh_int;


/*** The default initial zone ***/

struct zone_data ze_defaultzone=
{
  "Dummy-Zone",
  30,
  0,
  999,
  1,
  NULL
};

/****** DEFINES *****/

/* Zone-Edit */
#define ZE_COMM '#'
#define ZE_MAXSTRLEN 128

#define ZE_ZONFILE  "tzon.zon"
#define ZE_TEMPFILE "zetemp.zon"

#define ZE_ZRFILE  "zresets.def"
#define ZE_ZWFILE  "zwears.def"
#define ZE_ZDFILE "zdoors.def"

#define ZE_EDITMAX 5

#define ZE_SM_DELETE 1
#define ZE_SM_SAVE   2
#define ZE_SM_INSERT 3

#define ZE_M_QUIT   0

/* Zone-File-Format */
#define ZF_START  '#'
#define ZF_EOS    '~'
#define ZF_MOBL   'M'
#define ZF_OBJL   'O'
#define ZF_OBJG   'G'
#define ZF_OBJE   'E'
#define ZF_OBJP   'P'
#define ZF_DOOR   'D'
#define ZF_OBJR   'R'
#define ZF_ZEND   'S'
#define ZF_EOF    "$~"

#define LM_LOAD   0
#define LM_SAVE   1

#define REAL 0
#define VIRTUAL 1

/***** STRUCTS *****/

/***** Constants *****/

/***** Global Vars *****/

char ze_zonfile[ZE_MAXSTRLEN]=ZE_ZONFILE;
char ze_tempfile[ZE_MAXSTRLEN]=ZE_TEMPFILE;

char ze_dbpath[ZE_MAXSTRLEN]="";

int ze_affbitvec=0;

int zw_max;
int zr_max;
int zd_max;

int ze_changed;
int ze_activechanged;

char **zw_str;
char **zr_str;
char **zd_str;

FILE *ze_workfile;
FILE *zw_file;
FILE *zr_file;
FILE *zd_file;

FILE *ze_temp;

struct zone_data *ze_activezone=NULL;

/***** ERROR-DEFINES *****/

#define ZE_ERR_NOERROR      0
#define ZE_ERR_LINEINVALID  1
#define ZE_ERR_INVALIDSTART 2
#define ZE_ERR_OOMFORZON    3
#define ZE_ERR_STRINGTOOLONG 4
#define ZE_ERR_CANTOPEN     5


/***** Vars from db.c from original DikuMud! *****/

FILE *zon_f;                          /* zon prototypes                  */
struct index_data *zon_index;         /* index table for zone file     */
int top_of_zone = 0;                  /* top of zone index table       */


/* prototypes */
void  dc_NotFound(void);
int   real_zone(int virtual);
struct zone_data *read_zone(int nr, int type);

void  ze_EditNr(struct zone_data *);
void  ze_EditNameList(struct zone_data *);
void  ze_EditTop(struct zone_data *);
void  ze_EditLifespan(struct zone_data *);
void  ze_EditResetMode(struct zone_data *);

/*** A FUNCTION ARRAY (OH GREAT :-D) ***/

void (*ze_edit[ZE_EDITMAX+1])(struct zone_data *) =
{
  ze_EditNr,
  ze_EditNameList,
  ze_EditTop,
  ze_EditLifespan,
  ze_EditResetMode
};

/* Open the zone-file defined in ze_zonfile */

int OpenZoneFile(void)
{
  struct zone_data *wzon;

	if (!(zon_f = fopen(ze_zonfile, "r")))
	{
	  printf("*** Can't open %s-file!\n",ze_zonfile);
	  printf("Want to create a new one (YES/no)? ");
	  if(AskYesNo("no"))
	    exit(0);
	  else
	  {
	    if(!(zon_f = fopen(ze_zonfile,"w")))
	    {
	      printf("*** Can't create %s-file!\n",ze_zonfile);
	      exit(0);
	    }
	    else
	    {
	      fprintf(zon_f,"#0\n");
	      wzon= &ze_defaultzone;
	      fwrite_string(zon_f,wzon->name);
              fprintf(zon_f,"%d %d %d\n",wzon->top,wzon->lifespan,wzon->reset_mode);

              fprintf(zon_f,"S\n");
              fprintf(zon_f,"#99\n");
              fprintf(zon_f,ZF_EOF);
	      fprintf(zon_f,"\n");

	      fclose(zon_f);
	      if (!(zon_f = fopen(ze_zonfile, "r")))
	      {
		printf("*** Can't reopen %s-file!\n",ze_zonfile);
		exit(0);
	      }
	    }
	  }
	}
	zon_index = generate_indices(zon_f, &top_of_zone);
	return(ZE_ERR_NOERROR);
}


/* read a zone from ZON_FILE */

struct zone_data *read_zone(int nr, int type)
{
	struct zone_data *zon;
	int tmp, i;
	long tmp2;
	char chk[50], buf[100];

	i = nr;
	if (type == VIRTUAL)
		if ((nr = real_zone(nr)) < 0)
	{
		sprintf(buf, "Zone (V) %d does not exist in database.", i);
		return(ZE_ERR_NOERROR);
	}

	fseek(zon_f, zon_index[nr].pos, 0);

	CREATE(zon, struct zone_data, 1);

	/* *** string data *** */
	fscanf(zon_f, "#%d\n", &tmp);
	zon->virt_number=tmp;
	zon->name = fread_string(zon_f);

	/* *** numeric data *** */

	fscanf(zon_f, " %d ", &tmp);
	zon->top = tmp;
	fscanf(zon_f, " %d ", &tmp);
	zon->lifespan = tmp;
	fscanf(zon_f, " %d \n", &tmp);
	zon->reset_mode = tmp;

	/* *** reset command *** */

	zon->cmd = 0;
        zon->zone= NULL;

        /* !!! */

/*	while (fscanf(zon_f, " %s \n", chk), *chk != 'S')
	{
		CREATE(new_descr, struct extra_descr_data, 1);

		new_descr->keyword = fread_string(zon_f);
		new_descr->description = fread_string(zon_f);

		new_descr->next = zon->ex_description;
		zon->ex_description = new_descr;
	}

	zone_list = zon;*/

	return (zon);
}

void ze_purgemob(int nr)
{
  int wint;
  int wint2;
  int wint4;

  if(ze_activezone==NULL)
  {
    printf("*** No active zone.\n");
  }
  else
  {
    for(wint=0;wint<ze_activezone->zonemax;wint++)
    {
      if(ze_activezone->zone[wint].room_nr==sim_get_curr_room()) break;
    }
    if(wint==ze_activezone->zonemax)
      dc_NotFound();
    else
    {
      wint4=0;
      for(wint2=0;wint2<ze_activezone->zone[wint].mob_count;wint2++)
      {
        if(ze_activezone->zone[wint].mobs[wint2].nr==nr)
        {
          int wint3;

          free(ze_activezone->zone[wint].mobs[wint2].eq);
          free(ze_activezone->zone[wint].mobs[wint2].inv);

          for(wint3=wint2+1;wint3<ze_activezone->zone[wint].mob_count;wint3++)
          {
            ze_activezone->zone[wint].mobs[wint3-1].nr=ze_activezone->zone[wint].mobs[wint3].nr;
            ze_activezone->zone[wint].mobs[wint3-1].maxexist=ze_activezone->zone[wint].mobs[wint3].maxexist;
            ze_activezone->zone[wint].mobs[wint3-1].eq_count=ze_activezone->zone[wint].mobs[wint3].eq_count;
            ze_activezone->zone[wint].mobs[wint3-1].eq=ze_activezone->zone[wint].mobs[wint3].eq;
            ze_activezone->zone[wint].mobs[wint3-1].inv_count=ze_activezone->zone[wint].mobs[wint3].inv_count;
            ze_activezone->zone[wint].mobs[wint3-1].inv=ze_activezone->zone[wint].mobs[wint3].inv;
          }
          ze_activezone->zone[wint].mob_count--;
          ze_activechanged=1;
          realloc(ze_activezone->zone[wint].mobs,sizeof(struct ze_mob)*ze_activezone->zone[wint].mob_count);
          printf("Mobile purged.\n");
          wint4=1;
          break;
        }
      }
      if(wint4==0)
        dc_NotFound();
    }
  }
}

void ze_purgeobj(int nr)
{
  int wint;
  int wint2;
  int wint4;

  if(ze_activezone==NULL)
  {
    printf("*** No active zone.\n");
  }
  else
  {
    for(wint=0;wint<ze_activezone->zonemax;wint++)
    {
      if(ze_activezone->zone[wint].room_nr==sim_get_curr_room()) break;
    }
    if(wint==ze_activezone->zonemax)
      dc_NotFound();
    else
    {
      wint4=0;
      for(wint2=0;wint2<ze_activezone->zone[wint].obj_count;wint2++)
      {
        if(ze_activezone->zone[wint].objs[wint2].nr==nr)
        {
          int wint3;

          free(ze_activezone->zone[wint].objs[wint2].inv);

          for(wint3=wint2+1;wint3<ze_activezone->zone[wint].obj_count;wint3++)
          {
            ze_activezone->zone[wint].objs[wint3-1].nr=ze_activezone->zone[wint].objs[wint3].nr;
            ze_activezone->zone[wint].objs[wint3-1].maxexist=ze_activezone->zone[wint].objs[wint3].maxexist;
            ze_activezone->zone[wint].objs[wint3-1].inv_count=ze_activezone->zone[wint].objs[wint3].inv_count;
            ze_activezone->zone[wint].objs[wint3-1].inv=ze_activezone->zone[wint].objs[wint3].inv;
          }
          ze_activezone->zone[wint].obj_count--;
          ze_activechanged=1;
          realloc(ze_activezone->zone[wint].objs,sizeof(struct ze_obj)*ze_activezone->zone[wint].obj_count);
          printf("Item purged.\n");
          wint4=1;
          break;
        }
      }
      if(wint4==0)
        dc_NotFound();
    }
  }
}

struct ze_mob *ze_loadmob(struct ze_zone *wzone,int maxe,int mobnr,int mode)
{

  if(wzone->mobs!=NULL)
  {
    wzone->mobs=realloc(wzone->mobs,sizeof(struct ze_mob)*(wzone->mob_count+1));

  }
  else
  {
    wzone->mobs=malloc(sizeof(struct ze_mob));
  }
  wzone->mobs[wzone->mob_count].nr=mobnr;
  wzone->mobs[wzone->mob_count].maxexist=maxe;
  wzone->mobs[wzone->mob_count].eq_count=0;
  wzone->mobs[wzone->mob_count].eq=NULL;
  wzone->mobs[wzone->mob_count].inv_count=0;
  wzone->mobs[wzone->mob_count].inv=NULL;
  wzone->mob_count++;
  return(&(wzone->mobs[wzone->mob_count-1]));
}

void ze_loadobj(struct ze_zone *wzone,int maxe,int objnr,int mode)
{

  if(wzone->objs!=NULL)
  {
    wzone->objs=realloc(wzone->objs,sizeof(struct ze_obj)*(wzone->obj_count+1));

  }
  else
  {
    wzone->objs=malloc(sizeof(struct ze_obj));
  }
  wzone->objs[wzone->obj_count].nr=objnr;
  wzone->objs[wzone->obj_count].maxexist=maxe;
  wzone->objs[wzone->obj_count].inv_count=0;
  wzone->objs[wzone->obj_count].inv=NULL;
  wzone->obj_count++;
}

void ze_setdoorstate(struct ze_zone *wzone,int dir,int state)
{
  wzone->door[dir]=state;
}

void ze_giveobj(struct ze_mob *mob,int obj)
{
  if(mob->inv!=NULL)
  {
    mob->inv=realloc(mob->inv,sizeof(int)*(mob->inv_count+1));
    mob->inv[mob->inv_count]=obj;
  }
  else
  {
    mob->inv=malloc(sizeof(int));
    mob->inv[0]=obj;
  }
  mob->inv_count++;
}

void ze_putobj(struct ze_obj *obj,int obj1)
{
  if(obj->inv!=NULL)
  {
    obj->inv=realloc(obj->inv,sizeof(int)*(obj->inv_count+1));
    obj->inv[obj->inv_count]=obj1;
  }
  else
  {
    obj->inv=malloc(sizeof(int));
    obj->inv[0]=obj1;
  }
  obj->inv_count++;
}

int ze_putroomobj(struct ze_zone *room,int obj1, int obj2)
{
  int i;
  for(i=room->obj_count-1;i>=0;i--)
  {
    if(room->objs[i].nr==obj2)
    {
      ze_putobj(&(room->objs[i]),obj1);
      return(1);
    }
  }
  return(0);
}

void ze_eqobj(struct ze_mob *mob,int obj,int pos)
{
  if(mob->eq!=NULL)
  {
    mob->eq=realloc(mob->eq,sizeof(struct ze_mob_eq)*(mob->eq_count+1));
    mob->eq[mob->eq_count].obj=obj;
    mob->eq[mob->eq_count].pos=pos;
  }
  else
  {
    mob->eq=malloc(sizeof(struct ze_mob_eq));
    mob->eq[0].obj=obj;
    mob->eq[0].pos=pos;
  }
  mob->eq_count++;
}

struct ze_zone *ze_getroom(struct zone_data *zon,int roomnr)
{
  int wint;

  if(zon!=NULL && zon->zone!=NULL)
  {
    for(wint=0;wint<zon->zonemax;wint++)
    {
      if(zon->zone[wint].room_nr==roomnr)
        return(&(zon->zone[wint]));
    }
  }

  return(NULL);
}

struct ze_zone *ze_existroom(struct zone_data *zon,int roomnr)
{
  int wint;


  if(zon->zone!=NULL)
  {
    for(wint=0;wint<zon->zonemax;wint++)
    {
      if(zon->zone[wint].room_nr==roomnr)
        return(&(zon->zone[wint]));
    }

    zon->zone=realloc(zon->zone,(zon->zonemax+1) * sizeof(struct ze_zone));
    zon->zone[zon->zonemax].room_nr=roomnr;
    zon->zone[zon->zonemax].mob_count=0;
    zon->zone[zon->zonemax].obj_count=0;
    zon->zone[zon->zonemax].mobs=NULL;
    zon->zone[zon->zonemax].objs=NULL;
    for(wint=0;wint<6;wint++)
    {
      zon->zone[zon->zonemax].door[wint]=0;
    }
    zon->zonemax++;
    return(&(zon->zone[zon->zonemax-1]));
  }
  else
  {
    zon->zonemax=0;
    zon->zone=malloc(sizeof(struct ze_zone));
    zon->zone[0].room_nr=roomnr;
    zon->zone[0].mob_count=0;
    zon->zone[0].obj_count=0;
    zon->zone[0].mobs=NULL;
    zon->zone[0].objs=NULL;
    for(wint=0;wint<6;wint++)
    {
      zon->zone[0].door[wint]=0;
    }
    zon->zonemax=1;
    return(zon->zone);
  }
}

/* free whole zone */
void ze_free_zone(struct zone_data *wzone)
{
  int wint;
  if(wzone->zone!=NULL)
  {
    for(wint=0;wint<wzone->zonemax;wint++)
    {
      if(wzone->zone[wint].mobs!=NULL)
        free(wzone->zone[wint].mobs);
      if(wzone->zone[wint].objs!=NULL)
        free(wzone->zone[wint].objs);
    }
    free(wzone->zone);
  }
  wzone->zone=NULL;
}

/* free room in zone */
void ze_free_zoneroom(struct zone_data *wzone,int nr,int mode)
{
  int wint;
  int wint2;

  if(wzone->zone!=NULL)
  {
    for(wint=0;wint<wzone->zonemax;wint++)
    {
      if(wzone->zone[wint].room_nr==nr)
      {
        if(wzone->zone[wint].mobs!=NULL)
          free(wzone->zone[wint].mobs);
        if(wzone->zone[wint].objs!=NULL)
          free(wzone->zone[wint].objs);

        wzone->zone[wint].mob_count=0;
        wzone->zone[wint].obj_count=0;
        wzone->zone[wint].mobs=NULL;
        wzone->zone[wint].objs=NULL;

        if(mode==ZE_PURGEDOOR)
        {
          for(wint2=0;wint2<6;wint2++)
          {
            wzone->zone[wint].door[wint2]=0;
          }
        }
        else
          ze_activechanged=1;
      }
    }
  }
}

/* release memory allocated for an zon struct */

void free_zon(struct zone_data *zon)
{
	free(zon->name);
        if(zon->zone!=NULL)
          ze_free_zone(zon);
	free(zon);
}

/* load whole zone */
struct zone_data *ze_load_zone(int nr,int type)
{
	struct zone_data *zon;
        struct ze_zone *wzone;
	int tmp, i;
	long tmp2;
	char chk[50], buf[100];
        int iff,maxe,roomnr,mobnr,objnr1,objnr2,state;
        char com;
        int currmob= -1;
        int currroom= -1;
        struct ze_zone *currroomp=NULL;
        struct ze_mob  *currmobp=NULL;

	i = nr;
	if (type == VIRTUAL)
		if ((nr = real_zone(nr)) < 0)
	{
		return(NULL);  /* !!! zone does not exist, wanna create one? */
	}

	fseek(zon_f, zon_index[nr].pos, 0);

	CREATE(zon, struct zone_data, 1);

	/* *** string data *** */
	fscanf(zon_f, "#%d\n", &tmp);
	zon->virt_number=tmp;
	zon->name = fread_string(zon_f);

	/* *** numeric data *** */

	fscanf(zon_f, " %d ", &tmp);
	zon->top = tmp;
	fscanf(zon_f, " %d ", &tmp);
	zon->lifespan = tmp;
	fscanf(zon_f, " %d \n", &tmp);
	zon->reset_mode = tmp;

	/* *** reset command *** */

	zon->cmd = 0;
        zon->zone= 0;
        zon->zonemax=0;

        /* !!! */

        fgets(chk,50,zon_f);

	while (*chk != 'S')
	{
          switch(*chk)
          {
            case ZF_MOBL:
              sscanf(chk,"%c %d %d %d %d",&com,&iff,&mobnr,&maxe,&roomnr);
              currmob=mobnr;
              wzone=ze_existroom(zon,roomnr);
              currroomp=wzone;
              currroom=roomnr;
              currmobp=ze_loadmob(wzone,maxe,mobnr,LM_LOAD);
            break;
            case ZF_OBJL:
              sscanf(chk,"%c %d %d %d %d",&com,&iff,&objnr1,&maxe,&roomnr);
              wzone=ze_existroom(zon,roomnr);
              currroomp=wzone;
              currroom=roomnr;
              ze_loadobj(wzone,maxe,objnr1,LM_LOAD);
            break;
            case ZF_OBJG:
              sscanf(chk,"%c %d %d %d",&com,&iff,&objnr1,&maxe);
              if(currmob==-1)
              {
                printf("*** Error in Zonefile, lonely G-command!! Ignoring.\n");
                break;
              }
              ze_giveobj(currmobp,objnr1);
            break;
            case ZF_OBJE:
              sscanf(chk,"%c %d %d %d %d",&com,&iff,&objnr1,&maxe,&state);
              if(currmob==-1)
                printf("*** Error in Zonefile, lonely E-command!! Ignoring.\n");
              ze_eqobj(currmobp,objnr1,state);
            break;
            case ZF_OBJP:
              sscanf(chk,"%c %d %d %d %d",&com,&iff,&objnr1,&maxe,&objnr2);
              if(currroomp==NULL || (ze_putroomobj(currroomp,objnr1,objnr2)==0))
                printf("*** Error in Zonefile, Put into nonexisting Object!! Ignoring.\n");
            break;
            case ZF_DOOR:
              sscanf(chk,"%c %d %d %d %d",&com,&iff,&roomnr,&maxe,&state);
              ze_setdoorstate(ze_existroom(zon,roomnr),maxe,state);
            break;
            case ZF_OBJR:
              printf("*** R-command ignored.\n");
            break;
          }

          fgets(chk,50,zon_f);
	}

/*	zone_list = zon;*/

	return (zon);

}

/* Save ze_tempfile with changed zone and copy to ze_zonfile */
int SaveToZETemp(int nr, int mode,struct zone_data *wzon)
{
  struct zone_data *zon;
  int tmp, i,i2;
  long tmp2, j,j2,j3,zon_i;
  long store_pos,store_cpos;
  char chk[50], buf[100];
  char *temp_buff;

  switch(mode)
  {
    case ZE_SM_INSERT: {
			 printf("Do you really want to CREATE/CLONE the zone (YES/no)? ");
			 if(AskYesNo("no"))
			 {
			   printf("Not Done.\n");
			   return(0);
			 }
			 break;
		       }
    case ZE_SM_SAVE: {
		       printf("Do you really want to SAVE the changes (YES/no)? ");
		       if(AskYesNo("no"))
		       {
			 printf("Not Done.\n");
			 return(0);
		       }
		       break;
		     }
    case ZE_SM_DELETE: {
			 printf("Do you really want to DELETE the zone (yes/NO)? ");
			 if(!AskYesNo("yes"))
			 {
			   printf("Not Done.\n");
			   return(0);
			 }

                         if((ze_activezone!=NULL) && (ze_GetRealNumber(ze_activezone->virt_number)==nr))
                         {
                           free_zon(ze_activezone);
                           ze_activezone=NULL;
                         }

			 break;
		       }
  }


  if(mode==ZE_SM_INSERT)
  {
    if (!(zon_index =
	    (struct index_data*) realloc(zon_index,
	    (top_of_zone + 3) * sizeof(struct index_data))))
    {
	    perror("load indices");
	    exit(0);
    }
    top_of_zone++;

    for(i=top_of_zone+1;i>nr;i--)
    {
      zon_index[i].virtual=zon_index[i-1].virtual;
      zon_index[i].pos=zon_index[i-1].pos;
      zon_index[i].cpos=zon_index[i-1].cpos;
    }
    zon_index[nr].virtual=wzon->virt_number;
  }




  rewind(zon_f);

  if (!(ze_temp = fopen(ze_tempfile, "w")))
  {
    printf("Can't open temporary file!!!\n");
    exit(0);
    /*** !!! ***/
  }
  else
  {
    if(zon_index[nr].pos!=0)
    {
      zon_i = zon_index[nr].cpos;
      j = zon_i;

      while(j>0)
      {
	temp_buff=malloc(j*sizeof(char));
	if(temp_buff==NULL)
	  j = j / 2;
	else
	  break;
      }
      if(j==0)
      {
	printf("** CANNOT ALLOCATE 1 SINGLE CHAR!!! HELP!!!\n");
	exit(0);
      }

      j2=0;
      while(j2!=zon_i)
      {
	if((j2+j)>zon_i)
	  j=zon_i-j2;
	j3=fread(temp_buff,sizeof(char),j,zon_f);
	fwrite(temp_buff,sizeof(char),j3,ze_temp);
	j2+=j3;
      }
      free(temp_buff);
    }

    if((mode==ZE_SM_SAVE)||(mode==ZE_SM_INSERT))
    {
      fprintf(ze_temp,"#%d\n",wzon->virt_number);
      fwrite_string(ze_temp,wzon->name);

      fprintf(ze_temp,"%d %d %d\n",wzon->top,wzon->lifespan,wzon->reset_mode);

      if(wzon->zone!=NULL)
      {
        for(tmp=0;tmp<wzon->zonemax;tmp++)
        {
          for(i=0;i<wzon->zone[tmp].mob_count;i++)
          {
            fprintf(ze_temp,"M 0 %d %d %d\n",wzon->zone[tmp].mobs[i].nr,wzon->zone[tmp].mobs[i].maxexist,wzon->zone[tmp].room_nr);
            for(i2=0;i2<wzon->zone[tmp].mobs[i].eq_count;i2++)
            {
              fprintf(ze_temp,"E 1 %d 99 %d\n",wzon->zone[tmp].mobs[i].eq[i2].obj,wzon->zone[tmp].mobs[i].eq[i2].pos);
            }
            for(i2=0;i2<wzon->zone[tmp].mobs[i].inv_count;i2++)
            {
              fprintf(ze_temp,"G 1 %d 99\n",wzon->zone[tmp].mobs[i].inv[i2]);
            }
          }
          for(i=0;i<wzon->zone[tmp].obj_count;i++)
          {
            fprintf(ze_temp,"O 0 %d %d %d\n",wzon->zone[tmp].objs[i].nr,wzon->zone[tmp].objs[i].maxexist,wzon->zone[tmp].room_nr);
            for(i2=0;i2<wzon->zone[tmp].objs[i].inv_count;i2++)
            {
              fprintf(ze_temp,"P 1 %d 99 %d\n",wzon->zone[tmp].objs[i].inv[i2],wzon->zone[tmp].objs[i].nr);
            }
          }
          for(i=0;i<6;i++)
          {
            if(wzon->zone[tmp].door[i]!=0)
            {
              fprintf(ze_temp,"D 0 %d %d %d\n",wzon->zone[tmp].room_nr,i,wzon->zone[tmp].door[i]);
            }
          }
        }
      }

      fprintf(ze_temp,"S\n");

      store_pos=ftell(ze_temp)-zon_index[nr].pos;
    }


    fseek(zon_f, zon_index[nr+1].pos,0);
    fseek(zon_f,0,2);
    tmp2=ftell(zon_f);
    fseek(zon_f, zon_index[nr+1].pos,0);

    j = tmp2-zon_index[nr+1].pos;

    if(j!=0)
    {
      while(j>0)
      {
	temp_buff=malloc(j*sizeof(char));
	if(temp_buff==NULL)
	  j = j / 2;
	else
	  break;
      }
      if(j==0)
      {
	printf("** CANNOT ALLOCATE 1 SINGLE CHAR!!! HELP!!!\n");
	exit(0);
      }

      do
      {
	j2=fread(temp_buff,sizeof(char),j,zon_f);
	if(j2!=0)
	  fwrite(temp_buff,sizeof(char),j2,ze_temp);
      }
      while(j2!=0);

      free(temp_buff);
    }
    fclose(ze_temp);
  }

  switch(mode)
  {
    case ZE_SM_DELETE :
			{
			  j=zon_index[nr+1].pos-zon_index[nr].pos;
			  j2=zon_index[nr+1].cpos-zon_index[nr].cpos;
			  for(i=nr;i<top_of_zone+1;i++)
			  {
			    zon_index[i].virtual=zon_index[i+1].virtual;
			    zon_index[i].pos=zon_index[i+1].pos-j;
			    zon_index[i].cpos=zon_index[i+1].cpos-j2;
			  }
			  if (!(zon_index =
				  (struct index_data*) realloc(zon_index,
				  (top_of_zone+1) * sizeof(struct index_data))))
			  {
				  perror("load indices");
				  exit(0);
			  }
			  top_of_zone--;
			  break;
			}
    case ZE_SM_INSERT :
			{
			  if (!(ze_temp = fopen(ze_tempfile, "r")))
			  {
			    printf("Can't open temporary file!!!\n");
			    exit(0);
			    /*** !!! ***/
			  }

			  fseek(ze_temp,zon_index[nr].pos,0);

			  j3=0;
			  fgets(buf, 81, ze_temp);
			  j3+=strlen(buf);
			  for (;;)
			  {
			    if (fgets(buf, 81, ze_temp))
			    {
			      if (*buf == '#')
				break;
			      else
				j3+=strlen(buf);
			    }
			  }
			  fclose(ze_temp);

			  for(i=nr+1;i<=top_of_zone+1;i++)
			  {
			    zon_index[i].pos+=store_pos;
			    zon_index[i].cpos+=j3;
			  }
			  break;
			}
    case ZE_SM_SAVE   :
			{
			  if (!(ze_temp = fopen(ze_tempfile, "r")))
			  {
			    printf("Can't open temporary file!!!\n");
			    exit(0);
			    /*** !!! ***/
			  }

			  fseek(ze_temp,zon_index[nr].pos,0);


			  j3=0;
			  fgets(buf, 81, ze_temp);
			  j3+=strlen(buf);
			  for (;;)
			  {
			    if (fgets(buf, 81, ze_temp))
			    {
			      if (*buf == '#')
				break;
			      else
				j3+=strlen(buf);
			    }
			  }
			  fclose(ze_temp);
			  store_cpos=zon_index[nr+1].cpos-zon_index[nr].cpos;
			  j2=zon_index[nr+1].pos-zon_index[nr].pos;

			  for(i=nr+1;i<=top_of_zone+1;i++)
			  {
			    zon_index[i].pos+=store_pos-j2;
			    zon_index[i].cpos+=j3-store_cpos;
			  }

			  break;
			}
  }

  fclose(zon_f);
  if (!(zon_f = fopen(ze_zonfile, "w")))
  {
    printf("Can't reopen %s-file for saving!\n",ze_zonfile);
    exit(0);
  }
  if (!(ze_temp = fopen(ze_tempfile, "r")))
  {
    printf("Can't reopen temporary file for copying!");
    exit(0);
  }
  fseek(ze_temp,0,2);
  tmp2=ftell(ze_temp);
  rewind(ze_temp);
  j2=tmp2;
  temp_buff=NULL;
  while(j>0)
  {
    temp_buff=malloc(j2);
    if(temp_buff==NULL)
    {
      j2=j2/2;
    }
    else
    {
      break;
    }
  }
  j3=fread(temp_buff,sizeof(char),j2,ze_temp);
  while(j3!=0)
  {
    fwrite(temp_buff,sizeof(char),j3,zon_f);
    j3=fread(temp_buff,sizeof(char),j2,ze_temp);
  }
  if(temp_buff!=NULL)
    free(temp_buff);
  fclose(ze_temp);
  fclose(zon_f);

  if (!(zon_f = fopen(ze_zonfile, "r")))
  {
    printf("Can't reopen %s-file for reading!\n",ze_zonfile);
    exit(0);
  }
  return(0);
}

/* loadzone for DikuCli */

void ze_LoadZone(int nr)
{
  if(ze_activezone!=NULL)
  {
    if(ze_activechanged==1)
    {
      SaveToZETemp(ze_GetRealNumber(ze_activezone->virt_number),ZE_SM_SAVE,ze_activezone);
      ze_activechanged=0;
    }
    ze_free_zone(ze_activezone);
  }
  ze_activezone=ze_load_zone(nr,VIRTUAL);
}



/* returns the real number of the zone with given virtual number */

int real_zone(int virtual)
{
	int bot, top, mid;

	bot = 0;
	top = top_of_zone;

	/* perform binary search on zon-table */
	for (;;)
	{
		mid = (bot + top) / 2;

		if ((zon_index + mid)->virtual == virtual)
			return(mid);
		if (bot >= top)
			return(-1);
		if ((zon_index + mid)->virtual > virtual)
			top = mid - 1;
		else
			bot = mid + 1;
	}
}

/* Get Real Number for DikuCli */

int ze_GetRealNumber(int wint)
{
  return(real_zone(wint));
}


/* Find a Zone in Database */

char *ze_FindZone(int nr,char *wstr)
{
  int tmp;
  char *name;
  char *tname1;
  char *tname2;
  char tname[ZE_MAXSTRLEN];

  fseek(zon_f, zon_index[nr].pos, 0);

  fscanf(zon_f, "#%d\n", &tmp);
  name  = fread_string(zon_f);
  CREATE(tname1, char , strlen(name)+1);
  strcpy(tname1,name);
  tname2=strtok(tname1," ");
  for(;;)
  {
    if(tname2==NULL)
    {
      free(name);
      free(tname1);
      return(NULL);
    }
    else
    {
      if(!strpcmp(wstr,tname2))
      {
	free(tname1);
	return(name);
      }
      tname2=strtok(NULL," ");
    }
  }
}


void ze_FindZones(char *wstr)
{
  int i;
  char *name;

  printf("Zone-Nr  Name\n");
  printf("-------------------------\n");

  for (i=0; i<=top_of_zone;i++)
  {
    if((name=ze_FindZone(i,wstr))!=NULL)
    {
      printf("[%4.4d] %s\n",zon_index[i].virtual,name);
      free(name);
    }
  }
}

int ze_FindNrZone(int nr, char *wstr)
{
  int i,count;
  char *name;

  count=0;
  for (i=0; i<=top_of_zone;i++)
  {
    if((name=ze_FindZone(i,wstr))!=NULL)
    {
      free(name);
      count++;
      if(count==nr)
      {
	return(i);
      }
    }
    else
      free(name);
  }
  return(-1);
}

int ze_FindNrMobRoom(int nr, int mob, struct ze_zone *room)
{
  int i,count;

  count=0;
  for(i=0;i<room->mob_count;i++)
  {
    if(room->mobs[i].nr==mob)
    {
      count++;
      if(count==nr)
      {
        return(i);
      }
    }
  }
  return(-1);
}

int ze_FindMobRoom(int nr, char *mob, struct ze_zone *room)
{
  int i,count;
  char *name;
  char *tname1;
  char *tname2;

  count=0;
  for(i=0;room!=NULL && i<room->mob_count;i++)
  {
    if((name=me_ReadName(room->mobs[i].nr))!=NULL)
    {
      CREATE(tname1, char , strlen(name)+1);
      strcpy(tname1,name);
      tname2=strtok(tname1," ");
      for(;;)
      {
        if(tname2==NULL)
        {
          free(name);
          free(tname1);
          break;
        }
        else
        {
          if(!strpcmp(mob,tname2))
          {
	    free(tname1);
            free(name);
            count++;
            if(count==nr)
            {
	      return(i);
            }
            break;
          }
          tname2=strtok(NULL," ");
        }
      }
    }
  }
  return(-1);
}

int ze_FindNrItemRoom(int nr, int obj, struct ze_zone *room)
{
  int i,count;

  count=0;
  for(i=0;i<room->obj_count;i++)
  {
    if(room->objs[i].nr==obj)
    {
      count++;
      if(count==nr)
      {
        return(i);
      }
    }
  }
  return(-1);
}

int ze_FindItemRoom(int nr, char *obj, struct ze_zone *room)
{
  int i,count;
  char *name;
  char *tname1;
  char *tname2;

  count=0;
  for(i=0;room!=NULL && i<room->obj_count;i++)
  {
    if((name=ie_ReadName(room->objs[i].nr))!=NULL)
    {
      CREATE(tname1, char , strlen(name)+1);
      strcpy(tname1,name);
      tname2=strtok(tname1," ");
      for(;;)
      {
        if(tname2==NULL)
        {
          free(name);
          free(tname1);
          break;
        }
        else
        {
          if(!strpcmp(obj,tname2))
          {
	    free(tname1);
            free(name);
            count++;
            if(count==nr)
            {
	      return(i);
            }
            break;
          }
          tname2=strtok(NULL," ");
        }
      }
    }
  }
  return(-1);
}


void ze_doclone(struct zone_data *wzon,int wint)
{
  int bot, top, mid, used, i;
  struct zone_data *wzon2;

  bot = 0;
  top = top_of_zone;

  /* perform binary search on zon-table */
  for (;;)
  {
    mid = (bot + top) / 2;

    if ((zon_index + mid)->virtual == wint)
    {
      printf("Given Zone-Number is already used! Clone canceled.\n");
      used=1;
      break;
    }
    if (bot >= top)
    {
      if((zon_index + mid)->virtual <wint)
	mid++;
      used=0;
      break;
    }
    if ((zon_index + mid)->virtual > wint)
      top = mid - 1;
    else
      bot = mid + 1;
  }
  if(!used)
  {
    if(wzon == &ze_defaultzone)
      printf("Creating Dummy-Zone at [%d].\n",wint);
    else
      printf("Cloning Zone from [%d] to [%d].\n",wzon->virt_number,wint);
    CREATE(wzon2, struct zone_data, 1);

    /* The following lines proof the great advantage of zon.o.programming */
    /* and the disadvantage here! */

    wzon2->virt_number=wint;
/*    wzon2->zone_number=mid;*/
    wzon2->top=wzon->top;
    wzon2->lifespan=wzon->lifespan;
    wzon2->reset_mode=wzon->reset_mode;

    CREATE(wzon2->name, char ,strlen(wzon->name)+1);
    strcpy(wzon2->name,wzon->name);

    SaveToZETemp(mid,ZE_SM_INSERT,wzon2);
    free_zon(wzon2);
  }
}

struct zone_data *ze_existzone(struct zone_data *zon,int wint)
{
  struct zone_data *zon2;

  if(zon==NULL)
  {
    printf("There exists no valid zone for this room.\n");
    ze_doclone(&ze_defaultzone,wint);

    if(ze_GetRealNumber(wint)!= -1)
    {
      zon2=read_zone(wint,VIRTUAL);
      return(zon2);
    }
    return(NULL);
  }
  return(zon);
}

/* EDIT THE ITEM */

void  ze_EditNr(struct zone_data *wzon)
{
  char wstr[ZE_MAXSTRLEN];
  int wint;
  char wchr;


  printf("Current Zone Number: #%d\n",wzon->virt_number);
  printf("New Number (%d):",wzon->virt_number);
  gets(wstr);
  if(*wstr=='\0')
  {
    printf("Zone-Nr Unchanged");
    return;
  }
  wint = atoi(wstr);
  if((wint==0)&&(*wstr!='0'))
  {
    printf("** Illegal input\n");
    return;
  }
  if(wint==wzon->virt_number)
  {
    printf("Zone-Nr Unchanged\n");
    return;
  }
  ze_doclone(wzon,wint);
}

void  ze_EditNameList(struct zone_data *wzon)
{
  char wstr[ZE_MAXSTRLEN];
  printf("Old Name List: %s\n",wzon->name);
  printf("Enter New List :");
  gets(wstr);
  if(*wstr=='\0')
  {
    printf("Description unchanged.\n");
  }
  else
  {
    free(wzon->name);
    CREATE(wzon->name, char, strlen(wstr) + 1);
    strcpy(wzon->name,wstr);
    ze_changed=1;
  }
}

void  ze_EditTop(struct zone_data *wzon)
{
  int wint;
  char wstr[ZE_MAXSTRLEN];

  printf("Old Top-Room: %d\n",wzon->top);
  printf("Enter new Top-Room: ");
  gets(wstr);
  wint=atoi(wstr);
  if(((wint==0)&&(*wstr!='0'))||(wint<0))
  {
    printf("Top-Room unchanged.");
    return;
  }
  ze_changed=1;
  wzon->top=wint;
}


void  ze_EditLifespan(struct zone_data *wzon)
{
  int wint;
  char wstr[ZE_MAXSTRLEN];

  printf("Old Lifespan: %d\n",wzon->lifespan);
  printf("Enter new Lifespan: ");
  gets(wstr);
  wint=atoi(wstr);
  if(((wint==0)&&(*wstr!='0'))||(wint<0))
  {
    printf("Lifespan unchanged.");
    return;
  }
  ze_changed=1;
  wzon->lifespan=wint;
}

void  ze_EditResetMode(struct zone_data *wzon)
{
  int wint;
  char wstr[ZE_MAXSTRLEN];

  printf("Old Reset-Mode:");
  if(wzon->reset_mode>zr_max)
    printf("*UNKNOWN* - PLEASE REPORT OR UPDATE -ZRESETS.DEF-!!!\n");
  else
    printf("%s\n",zr_str[wzon->reset_mode]);
  for(wint=0;wint<=zr_max;wint=wint+2)
  {
    if(wint+1<=zr_max)
      printf("%.2d.) %-34.34s %.2d.) %-34.34s\n",wint,zr_str[wint],wint+1,zr_str[wint+1]);
    else
      printf("%.2d.) %s\n",wint,zr_str[wint]);
  }
  wint=zr_max;
  printf("Please enter Number of Reset-Mode (%s):",zr_str[wzon->reset_mode]);
  gets(wstr);
  wint=atoi(wstr);
  if(((wint==0)&&(*wstr!='0'))||(wint<0)||(wint>zr_max))
  {
    printf("Reset-Mode unchanged.\n");
  }
  else
  {
    ze_changed=1;
    wzon->reset_mode=wint;
  }
}

/* Print all zone Datas */

void print_zon(struct zone_data *wzon)
{
  int i;

  printf("\n");
  printf("Zone-Values:\n");
  printf("------------\n");
/*  printf("INTERNAL Zone-Nr       : %d\n",wzon->zone_number); */
  printf("1.)  Zone-Nr       : %d\n",wzon->virt_number);
  printf("2.)  Zone-Name     : %s\n",wzon->name);
  printf("3.)  Zone-Top      : %d\n",wzon->top);
  printf("4.)  Zone-Lifespan : %d\n",wzon->lifespan);
  printf("5.)  Zone-ResetMode: %s\n",zr_str[wzon->reset_mode]);

}

/* Show Zone for DikuCli */

void ze_ShowZone(int nr)
{
  struct zone_data *wzon;

  wzon=read_zone(nr,REAL);
  print_zon(wzon);
  free_zon(wzon);
}


/* Edit all Zone Data */

int EditAllZone(struct zone_data *wzon)
{
  int maxsel;
  char wstr[ZE_MAXSTRLEN];
  int wint;
  char wchr;

  maxsel=ZE_EDITMAX;

  print_zon(wzon);
  do
  {
    printf("Enter: 0 to quit, 1-%d to edit.\n",maxsel);
    gets(wstr);
    wint=atoi(wstr);
    if((wint>0)&&(wint<=ZE_EDITMAX))
    {
      (*ze_edit[wint-1])(wzon);
      print_zon(wzon);
    }
    else
    if(wint!=0)
      printf("** Illegal input. Try again!\n");
  }
  while(wint!=0);

  return(ZE_SM_SAVE);
}

/* Edit a Zone for DikuCli */

void ze_EditZone(int nr)
{
  struct zone_data *wzon;
  int ze_m;

  ze_changed=0;

  wzon=ze_load_zone(nr,REAL);
/*  wzon=read_zone(nr,REAL);   OLD */
  ze_m=EditAllZone(wzon);
  if((ze_changed!=0)||(ze_m==ZE_SM_DELETE))
  {
    if(ze_activezone!=NULL && wzon->virt_number==ze_activezone->virt_number)
    {
      free(ze_activezone->name);
      ze_activezone->name=malloc(strlen(wzon->name)+1);
      strcpy(ze_activezone->name,wzon->name);
      ze_activezone->top=wzon->top;
      ze_activezone->lifespan=wzon->lifespan;
      ze_activezone->reset_mode=wzon->reset_mode;
      SaveToZETemp(nr,ze_m,ze_activezone);
      ze_activechanged=0;
    }
    else
      SaveToZETemp(nr,ze_m,wzon);
  }
  free_zon(wzon);
}

/* Clone a Zone (for DikuCli) */

void ze_CloneZone(int nr1, int nr2)
{
  struct zone_data *wzon;
  if(nr1<0)
    wzon= &ze_defaultzone;
  else
    wzon=read_zone(nr1,REAL);

  ze_doclone(wzon,nr2);

  if(nr1>=0)
    free_zon(wzon);
}

/* Purge a Zone for DikuCli */

void ze_PurgeZone(int nr)
{
  SaveToZETemp(nr,ZE_SM_DELETE,NULL);
}

/* Load a Mobile to Zone for DikuCli */

void ze_LoadMobile(int nr)
{
  if((ze_activezone=ze_existzone(ze_activezone,re_RoomZone(sim_get_curr_room())))==NULL)
  {
    printf("No Item loaded (No valid zone)\n");
    return;
  }
  ze_activechanged=1;
  ze_loadmob(ze_existroom(ze_activezone,sim_get_curr_room()),99,nr,LM_LOAD);
  printf("Mobile loaded\n");
}

/* Give an Item to a Mobile for DikuCli */

void ze_GiveItemMob(int obj,int mob)
{
  struct ze_zone *room;

  ze_activechanged=1;
  room=ze_getroom(ze_activezone,sim_get_curr_room());
  ze_giveobj(&(room->mobs[mob]),obj);
}

/* Wear an Item to a Mobile for DikuCli */

void ze_WearItemMob(int obj,int mob,int pos)
{
  struct ze_zone *room;

  ze_activechanged=1;
  room=ze_getroom(ze_activezone,sim_get_curr_room());
  ze_eqobj(&(room->mobs[mob]),obj,pos);
}

/* Give an Item to a Item for DikuCli */

void ze_GiveItemItem(int obj,int obj2)
{
  struct ze_zone *room;

  ze_activechanged=1;
  room=ze_getroom(ze_activezone,sim_get_curr_room());
  ze_putobj(&(room->objs[obj2]),obj);
}

/* Set DoorState of Room for DikuCli */

void ze_DoorState(int dir,int state)
{
  ze_activechanged=1;
  ze_setdoorstate(ze_existroom(ze_activezone,sim_get_curr_room()),dir,state);
  printf("DoorState set\n");
}

/* Purge a Mobile from Zone for DikuCli */

void ze_PurgeMobile(int nr)
{
  ze_purgemob(nr);
}

/* Load a Item to Zone for DikuCli */

void ze_LoadItem(int nr)
{
  if((ze_activezone=ze_existzone(ze_activezone,re_RoomZone(sim_get_curr_room())))==NULL)
  {
    printf("No Item loaded (No valid zone)\n");
    return;
  }
  ze_activechanged=1;
  ze_loadobj(ze_existroom(ze_activezone,sim_get_curr_room()),99,nr,LM_LOAD);
  printf("Item loaded\n");
}

/* Purge a Item from Zone for DikuCli */

void ze_PurgeItem(int nr)
{
  ze_purgeobj(nr);
}

/* Purge all Mobs and Items from Room in Zone for DikuCli */

void ze_PurgeAll(void)
{
  if(ze_activezone!=NULL)
    ze_free_zoneroom(ze_activezone,sim_get_curr_room(),ZE_NOPURGEDOOR);
}

/* Set FileName of *.zon file */

void ze_SetFile(char *wstr)
{
  strcpy(ze_zonfile,wstr);
}

/* Set TempFile */

void ze_SetTemp(char *wstr)
{
  strcpy(ze_tempfile,wstr);
}

/* Set database path */

void ze_SetDBPath(char *wstr)
{
  strcpy(ze_dbpath,wstr);
}

/* Initialize the ZEdit vars */

void ze_InitZEdit()
{
  struct zone_data *zon;

  ze_changed=0;
  ze_activechanged=0;
  if(GetADef(zw_file,ZE_ZWFILE,ze_dbpath,&zw_str,&zw_max))
    printf("*** Can't open a *.def file!\n");
  if(GetADef(zr_file,ZE_ZRFILE,ze_dbpath,&zr_str,&zr_max))
    printf("*** Can't open a *.def file!\n");
  if(GetADef(zd_file,ZE_ZDFILE,ze_dbpath,&zd_str,&zd_max))
    printf("*** Can't open a *.def file!\n");
  if(OpenZoneFile())
    printf("***** IMPOSSIBLE ERROR *****");
}


void ze_CloseZEdit(void)
{
  if(ze_activezone!=NULL && ze_activechanged==1)
  {
    SaveToZETemp(ze_GetRealNumber(ze_activezone->virt_number),ZE_SM_SAVE,ze_activezone);
    ze_activechanged=0;
  }

  fclose(zon_f);
  FreeADef(zw_str,zw_max);
  FreeADef(zr_str,zr_max);
  FreeADef(zd_str,zd_max);
  if(ze_activezone!=NULL)
  {
    ze_free_zone(ze_activezone);
    free_zon(ze_activezone);
  }
}
