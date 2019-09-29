/*****************************************************************************
*                                                                            *
* Project : Diku-Mud-World-Creator                                           *
*                                                                            *
* Program : Diku-Mud-Room-Editor                                             *
* Version : 1.1                                                              *
* Last changed on: 27.7.93                                                   *
*                                                                            *
* Coded by                                                                   *
*   Kenneth Holmlund (holmlund@tp.umu.se)                                    *
*   With much help from Georg Essl's medit.c and iedit.c                     *
*                                                                            *
*****************************************************************************/

/***** Original Includes *****/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <sys/types.h>

/****** Project Includes *****/
#include "strext.h"
#include "dutil.h"
#include "redit.h"
#include "zedit.h"
#include "dikusim.h"

/****** DEFINES *****/

/* Room-Edit */
#define RE_MAXSTRLEN 128

#define RE_ROOMFILE "twld.wld"
#define RE_TEMPFILE "retemp.wld"

#define RE_DFILE "rdoors.def"
#define RE_SFILE "rsects.def"
#define RE_FFILE "rflags.def"

#define RE_SM_DELETE 1
#define RE_SM_SAVE   2
#define RE_SM_INSERT 3

#define RE_M_QUIT   0

#define RE_FLAGS_NONE     0
#define RE_FLAGS_GRIMNE_NEW 1
#define RE_FLAGS_SIMPLE 2

#define RE_EDITMAX 13
#define RE_EDITDOORMAX 6

#define REAL 0
#define VIRTUAL 1

#define RF_EOF    "$~"


/***** ERROR-DEFINES *****/

#define RE_ERR_NOERROR      0

/***** TYPEDEFS *****/


typedef   signed char  sbyte;
typedef unsigned char  ubyte;
#if !defined(sgi) && !defined(m88k) &&!defined(_AIX)
typedef unsigned long  ulong;
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

struct room_data re_defaultroom =
{
	0,
	0,
	"A dummy room",
        "You are standing inside a dummy room, which should get real.",
        0,
        0,
	0,
        0, NULL,NULL, 0, 0, 0,
        0, NULL,NULL, 0, 0, 0,
        0, NULL,NULL, 0, 0, 0,
        0, NULL,NULL, 0, 0, 0,
        0, NULL,NULL, 0, 0, 0,
        0, NULL,NULL, 0, 0, 0,
        NULL
};
	

/***** Global Vars *****/

extern struct zone_data *ze_activezone;
char re_roomfile[RE_MAXSTRLEN]=RE_ROOMFILE;
char re_tempfile[RE_MAXSTRLEN]=RE_TEMPFILE;

char re_dbpath[RE_MAXSTRLEN]="";

const char *direction[6] = 
{
	"North",
        "East ",
        "South",
        "West ",
        "Up   ",
        "Down "
};

int rd_max;
int rf_max;
int rs_max;

int re_changed;

char **rd_str;
char **rf_str;
char **rs_str;

FILE *re_workfile;
FILE *rd_file;
FILE *rf_file;
FILE *rs_file;

FILE *re_temp;

FILE *room_f;

/***** Vars from db.c from original DikuMud! *****/

struct index_data *room_index;         /* index table for room file       */
int top_of_roomt = 0;                  /* top of room index table         */

/*** Edit Protos ***/
void  re_EditNr(struct room_data *);
void  re_EditName(struct room_data *);
void  re_EditDesc(struct room_data *);
void  re_EditZone(struct room_data *);
void  re_EditFlags(struct room_data *);
void  re_EditSec(struct room_data *);
void  re_EditNorth(struct room_data *);
void  re_EditEast(struct room_data *);
void  re_EditSouth(struct room_data *);
void  re_EditWest(struct room_data *);
void  re_EditUp(struct room_data *);
void  re_EditDown(struct room_data *);
void  re_EditExtraDesc(struct room_data *);

void  re_EditDoor(struct room_data *, int);

/*** A FUNCTION ARRAY ***/

void (*re_edit[RE_EDITMAX+1])(struct room_data *) =
{
 re_EditNr,
 re_EditName,
 re_EditDesc,
 re_EditZone,
 re_EditFlags,
 re_EditSec,
 re_EditNorth,
 re_EditEast,
 re_EditSouth,
 re_EditWest,
 re_EditUp,
 re_EditDown,
 re_EditExtraDesc,
};

/* Door Edit Protos */
void red_EditToRoom(struct room_data *, int);
void red_EditDesc(struct room_data *, int);
void red_EditName(struct room_data *, int);
void red_Key(struct room_data *, int);
void red_Flags(struct room_data *, int);
void red_Purge(struct room_data *, int);

/* Function Array */
void (*red_edit[RE_EDITDOORMAX+1])(struct room_data *, int) =
{
 red_EditToRoom,
 red_EditDesc,
 red_EditName,
 red_Key,
 red_Flags,
 red_Purge
};

/* Free room data memory */

void free_room(struct room_data *room)
{
  int wint;
  struct extra_desc *exptr;
  struct extra_desc *exptr2;

  if(room->name)
    free(room->name);
  if(room->description)
    free(room->description);

  for(wint=0;wint<6;wint++)
  {
    if(room->exits[wint].isset==1)
    {
      if(room->exits[wint].gen_desc!=NULL)
        free(room->exits[wint].gen_desc);
      if(room->exits[wint].doorname!=NULL)
        free(room->exits[wint].doorname);
    }
  }

  exptr=room->extra;
  while(exptr!=NULL)
  {
    if(exptr->key!=NULL)
      free(exptr->key);
    if(exptr->desc!=NULL)
      free(exptr->desc);
    exptr2=exptr->next;
    free(exptr);
    exptr=exptr2;
  }
  free(room);
}

/* returns the real number of the room with given virtual number */
int real_room(int virtual)
{
	int bot, top, mid;

	bot = 0;
	top = top_of_roomt;

	/* perform binary search on room-table */
	for (;;)
	{
		mid = (bot + top) / 2;

		if ((room_index + mid)->virtual == virtual)
			return(mid);
		if (bot >= top)
			return(-1);
		if ((room_index + mid)->virtual > virtual)
			top = mid - 1;
		else
			bot = mid + 1;
	}
}

/* read a room from ROOM_FILE */
struct room_data *read_room(int nr, int type)
{
	int i, skill_nr, idir;
	long tmp, tmp2, tmp3;
	struct room_data *room;
        struct extra_desc *new_extra;
	char letter, buf[100];

	i = nr;
	if (type == VIRTUAL)
		if ((nr = real_room(nr)) < 0)
	{
		sprintf(buf, "Room (V) %d does not exist in database.", i);
		return(0);
	}

	fseek(room_f, room_index[nr].pos, 0);

	CREATE(room, struct room_data, 1);

	/***** String data *** */

	fscanf(room_f, "#%d\n", &tmp);
	room->virt_number=tmp;
	room->name = fread_string(room_f);
	room->description = fread_string(room_f);

	/* *** Numeric data *** */

	fscanf(room_f, "%ld ", &tmp);
	room->zone_nr = tmp;
	fscanf(room_f, " %lu ", &tmp);
	room->flags = tmp;
	fscanf(room_f, " %ld ", &tmp);
	room->sector = tmp;


        for (idir=0;idir<6;idir++)
        {
          room->exits[idir].isset = 0;
        }

        while (fscanf(room_f, " %c%d \n", &letter, &tmp), letter == 'D')
        {

          idir = tmp;
          if (idir<6||idir>=0)
          {
            room->exits[idir].isset = 1;                /*Direction idir is used*/
            room->exits[idir].gen_desc = fread_string(room_f);
            room->exits[idir].doorname = fread_string(room_f);
            fscanf(room_f,"%d", &tmp);
            room->exits[idir].doorflag = tmp;
            fscanf(room_f,"%d", &tmp);
            room->exits[idir].keynumber = tmp;
            fscanf(room_f,"%d\n", &tmp);
            room->exits[idir].toroomno = tmp;
          }
          else
          {
            free(fread_string(room_f));
            free(fread_string(room_f));
            fscanf(room_f,"%d %d %d\n", &tmp, &tmp2, &tmp3);
          }
        }

        room->extra = NULL;
        while(letter== 'E')
        {
          CREATE(new_extra, struct extra_desc, 1);
          new_extra->key = fread_string(room_f);
          new_extra->desc = fread_string(room_f);
          new_extra->next = room->extra;
          room->extra = new_extra;
          fscanf(room_f, " %c \n", &letter);
        }
	room->room_number = nr;

	return(room);
}

/* Open the room-file defined in re_roomfile */

int OpenRoomFile(void)
{
  struct room_data *wroom;
  int idir;

	if (!(room_f = fopen(re_roomfile, "r")))
	{
	  printf("*** Can't open %s-file!\n",re_roomfile);
	  printf("Want to create a new one (YES/no)? ");
	  if(AskYesNo("no"))
	    exit(0);
	  else
	  {
	    if(!(room_f = fopen(re_roomfile,"w")))
	    {
	      printf("*** Can't create %s-file!\n",re_roomfile);
	      exit(0);
	    }
	    else
	    {
              wroom= &re_defaultroom;
	      fprintf(room_f,"#%d\n", wroom->virt_number);
	      fwrite_string(room_f,wroom->name);
	      fwrite_string(room_f,wroom->description);
              fprintf(room_f,"%d %lu %d\n", wroom->zone_nr, wroom->flags, wroom->sector);
              for (idir = 0; idir < 6;idir++) {
                if (wroom->exits[idir].isset==1)
                {
                    fprintf(room_f,"D%d\n", idir);
                    fwrite_string(room_f,wroom->exits[idir].gen_desc);
                    fwrite_string(room_f,wroom->exits[idir].doorname);
                    fprintf(room_f,"%lu %d %d\n", wroom->exits[idir].doorflag,
                              wroom->exits[idir].keynumber, wroom->exits[idir].toroomno);
                }
              }
              fprintf(room_f,"S\n");
              fprintf(room_f,"#9999\n%s",RF_EOF);

	      fclose(room_f);
	      if (!(room_f = fopen(re_roomfile, "r")))
	      {
		printf("*** Can't reopen %s-file!\n",re_roomfile);
		exit(0);
	      }
	    }
	  }
	}
	room_index = generate_indices(room_f, &top_of_roomt);
	return(RE_ERR_NOERROR);
}


/* Find a Room in Database */

char *re_FindRoom(int nr,char *wstr)
{
  int tmp;
  char *name;
  char *tname1;
  char *tname2;
  char tname[RE_MAXSTRLEN];

  fseek(room_f, room_index[nr].pos, 0);

  fscanf(room_f, "#%d\n", &tmp);
  name  = fread_string(room_f);
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

void re_FindRooms(char *wstr)
{
  int i;
  char *name;

  printf("Room-Nr  Name\n");
  printf("-------------------------\n");

  for (i=0; i<=top_of_roomt;i++)
  {
    if((name=re_FindRoom(i,wstr))!=NULL)
    {
      printf("[%4.4d] %s\n",room_index[i].virtual,name);
      free(name);
    }
  }
}

int re_FindNrRoom(int nr, char *wstr)
{
  int i,count;
  char *name;

  count=0;
  for (i=0; i<=top_of_roomt;i++)
  {
    if((name=re_FindRoom(i,wstr))!=NULL)
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

/* Save re_tempfile with changed room and copy to re_roomfile */
int SaveToRETemp(int nr, int mode,struct room_data *wroom)
{
  struct room_data *room;
  int tmp, i, idir;
  long tmp2, j,j2,j3,room_i;
  long store_pos,store_cpos;
  char chk[50], buf[100];
  char *temp_buff;
  struct extra_desc *new_extra;
  struct extra_desc *wext;

  switch(mode)
  {
    case RE_SM_INSERT: {
			 printf("Do you really want to CREATE/CLONE the room (YES/no)? ");
			 if(AskYesNo("no"))
			 {
			   printf("Not Done.\n");
			   return(0);
			 }
			 break;
		       }
    case RE_SM_SAVE: {
		       printf("Do you really want to save the changes (YES/no)? ");
		       if(AskYesNo("no"))
		       {
			 printf("Not Done.\n");
			 return(0);
		       }
		       break;
		     }
    case RE_SM_DELETE: {
			 printf("Do you really want to delete the room (yes/NO)? ");
			 if(!AskYesNo("yes"))
			 {
			   printf("Not Done.\n");
			   return(0);
			 }

                         if(ze_activezone!=NULL)
                         {
                           ze_free_zoneroom(ze_activezone,nr,ZE_PURGEDOOR);
                         }

			 break;
		       }
  }


  if(mode==RE_SM_INSERT)
  {
    if (!(room_index =
	    (struct index_data*) realloc(room_index,
	    (top_of_roomt + 3) * sizeof(struct index_data))))
    {
	    perror("load indices");
	    exit(0);
    }
    top_of_roomt++;

    for(i=top_of_roomt+1;i>nr;i--)
    {
      room_index[i].virtual=room_index[i-1].virtual;
      room_index[i].pos=room_index[i-1].pos;
      room_index[i].cpos=room_index[i-1].cpos;
    }
    room_index[nr].virtual=wroom->virt_number;
  }




  rewind(room_f);

  if (!(re_temp = fopen(re_tempfile, "w")))
  {
    printf("*** Can't open temporary room file!!!\n");
    exit(0);
    /*** !!! ***/
  }
  else
  {
    if(room_index[nr].pos!=0)
    {
      room_i = room_index[nr].cpos;
      j = room_i;

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
      while(j2!=room_i)
      {
	if((j2+j)>room_i)
	  j=room_i-j2;
	j3=fread(temp_buff,sizeof(char),j,room_f);
	fwrite(temp_buff,sizeof(char),j3,re_temp);
	j2+=j3;
      }
      free(temp_buff);
    }

    if((mode==RE_SM_SAVE)||(mode==RE_SM_INSERT))
    {
      fprintf(re_temp,"#%d\n",wroom->virt_number);
      fwrite_string(re_temp,wroom->name);
      fwrite_string(re_temp,wroom->description);
      fprintf(re_temp,"%d %lu %d\n", wroom->zone_nr, wroom->flags, wroom->sector);
      for (idir = 0; idir < 6;idir++) {
        if (wroom->exits[idir].isset==1)
        {
           fprintf(re_temp,"D%d\n", idir);
           fwrite_string(re_temp,wroom->exits[idir].gen_desc);
           fwrite_string(re_temp,wroom->exits[idir].doorname);
           fprintf(re_temp,"%lu %d %d\n", wroom->exits[idir].doorflag,
                   wroom->exits[idir].keynumber, wroom->exits[idir].toroomno);
         }
       }
       wext=wroom->extra;
       while(wext!=NULL)
       {
         fprintf(re_temp,"E\n");
         fwrite_string(re_temp,wext->key);
         fwrite_string(re_temp,wext->desc);
         wext=wext->next;
       }
       fprintf(re_temp,"S\n");

      store_pos=ftell(re_temp)-room_index[nr].pos;
    }

    fseek(room_f, room_index[nr+1].pos,0);
    fseek(room_f,0,2);
    tmp2=ftell(room_f);
    fseek(room_f, room_index[nr+1].pos,0);

    j = tmp2-room_index[nr+1].pos;

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
	j2=fread(temp_buff,sizeof(char),j,room_f);
	if(j2!=0)
	  fwrite(temp_buff,sizeof(char),j2,re_temp);
      }
      while(j2!=0);

      free(temp_buff);
    }
    fclose(re_temp);
  }

  switch(mode)
  {
    case RE_SM_DELETE :
			{
			  j=room_index[nr+1].pos-room_index[nr].pos;
			  j2=room_index[nr+1].cpos-room_index[nr].cpos;
			  for(i=nr;i<top_of_roomt+1;i++)
			  {
			    room_index[i].virtual=room_index[i+1].virtual;
			    room_index[i].pos=room_index[i+1].pos-j;
			    room_index[i].cpos=room_index[i+1].cpos-j2;
			  }
			  if (!(room_index =
				  (struct index_data*) realloc(room_index,
				  (top_of_roomt+1) * sizeof(struct index_data))))
			  {
				  perror("load indices");
				  exit(0);
			  }
			  top_of_roomt--;
			  break;
			}
    case RE_SM_INSERT :
			{
			  if (!(re_temp = fopen(re_tempfile, "r")))
			  {
			    printf("*** Can't open temporary room file!!!\n");
			    exit(0);
			    /*** !!! ***/
			  }

			  fseek(re_temp,room_index[nr].pos,0);

			  j3=0;
			  fgets(buf, 81, re_temp);
			  j3+=strlen(buf);
			  for (;;)
			  {
			    if (fgets(buf, 81, re_temp))
			    {
			      if (*buf == '#')
				break;
			      else
				j3+=strlen(buf);
			    }
			  }
			  fclose(re_temp);

			  for(i=nr+1;i<=top_of_roomt+1;i++)
			  {
			    room_index[i].pos+=store_pos;
			    room_index[i].cpos+=j3;
			  }
			  break;
			}
    case RE_SM_SAVE   :
			{
			  if (!(re_temp = fopen(re_tempfile, "r")))
			  {
			    printf("*** Can't open temporary room file!!!\n");
			    exit(0);
			    /*** !!! ***/
			  }

			  fseek(re_temp,room_index[nr].pos,0);


			  j3=0;
			  fgets(buf, 81, re_temp);
			  j3+=strlen(buf);
			  for (;;)
			  {
			    if (fgets(buf, 81, re_temp))
			    {
			      if (*buf == '#')
				break;
			      else
				j3+=strlen(buf);
			    }
			  }
			  fclose(re_temp);
			  store_cpos=room_index[nr+1].cpos-room_index[nr].cpos;
			  j2=room_index[nr+1].pos-room_index[nr].pos;

			  for(i=nr+1;i<=top_of_roomt+1;i++)
			  {
			    room_index[i].pos+=store_pos-j2;
			    room_index[i].cpos+=j3-store_cpos;
			  }

			  break;
			}
  }

  fclose(room_f);
  if (!(room_f = fopen(re_roomfile, "w")))
  {
    printf("*** Can't reopen %s-file for saving!\n",re_roomfile);
    exit(0);
  }
  if (!(re_temp = fopen(re_tempfile, "r")))
  {
    printf("*** Can't reopen temporary file for copying!");
    exit(0);
  }
  fseek(re_temp,0,2);
  tmp2=ftell(re_temp);
  rewind(re_temp);
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
  j3=fread(temp_buff,sizeof(char),j2,re_temp);
  while(j3!=0)
  {
    fwrite(temp_buff,sizeof(char),j3,room_f);
    j3=fread(temp_buff,sizeof(char),j2,re_temp);
  }
  if(temp_buff!=NULL)
    free(temp_buff);
  fclose(re_temp);
  fclose(room_f);

  if (!(room_f = fopen(re_roomfile, "r")))
  {
    printf("*** Can't reopen %s-file for reading!\n",re_roomfile);
    exit(0);
  }
  return(0);
}

/* Print Bitfield Room Flags */

void PrintRFlags(unsigned long wact)
{
  unsigned long i,j;
  unsigned long shact;
  j=0;
  shact=wact;
  for(i=1;i<=rf_max;i++)
  {
    if((shact&1))
    {
      printf("%s  ",rf_str[i]);
      j=1;
    }
    shact=shact>>1;
  }
  if(!j)
    printf("None\n");
  else
    printf("\n");
}

/* Print Bitfield Door Flags */

void PrintDFlags(unsigned long wact)
{
  unsigned long i,j;
  unsigned long shact;
  j=0;
  shact=wact;
  for(i=1;i<=rd_max;i++)
  {
    if((shact&1))
    {
      printf("%s  ",rd_str[i]);
      j=1;
    }
    shact=shact>>1;
  }
  if(!j)
    printf("None\n");
  else
    printf("\n");
}

void re_doclone(struct room_data *wroom,int wint)
{
  int bot, top, mid, used, i, idir;
  struct room_data *wroom2;
  struct extra_desc *wext;
  struct extra_desc *wext2;
  struct extra_desc *wext3;

  bot = 0;
  top = top_of_roomt;

  /* perform binary search on room-table */
  for (;;)
  {
    mid = (bot + top) / 2;

    if ((room_index + mid)->virtual == wint)
    {
      printf("Given Room-Number is already used! Clone canceled.\n");
      used=1;
      break;
    }
    if (bot >= top)
    {
      if((room_index + mid)->virtual <wint)
	mid++;
      used=0;
      break;
    }
    if ((room_index + mid)->virtual > wint)
      top = mid - 1;
    else
      bot = mid + 1;
  }
  if(!used)
  {
    if(wroom == &re_defaultroom)
      printf("Creating Dummy-Room at [%d].\n",wint);
    else
      printf("Cloning Room from [%d] to [%d].\n",wroom->virt_number,wint);
    CREATE(wroom2, struct room_data, 1);

    /* The following lines proof the great advantage of obj.o.programming */
    /* and the disadvantage here! Yes, this really sucks!! (KH)*/

    wroom2->virt_number=wint;
    wroom2->room_number=mid;
    wroom2->zone_nr=wroom->zone_nr;
    wroom2->flags=wroom->flags;
    wroom2->sector=wroom->sector;
    for (idir=0;idir<6;idir++)
    {
       wroom2->exits[idir].isset = wroom->exits[idir].isset;
       if (wroom->exits[idir].gen_desc!=NULL)
       {
         CREATE(wroom2->exits[idir].gen_desc, char, strlen(wroom->exits[idir].gen_desc)+1);
         strcpy(wroom2->exits[idir].gen_desc,wroom->exits[idir].gen_desc);
       }
       else
         wroom2->exits[idir].gen_desc=NULL;
       if (wroom->exits[idir].doorname!=NULL)
       {
       CREATE(wroom2->exits[idir].doorname,char, strlen(wroom->exits[idir].doorname)+1);
       strcpy(wroom2->exits[idir].doorname, wroom->exits[idir].doorname);
       }
       else
         wroom2->exits[idir].doorname = NULL;
       wroom2->exits[idir].doorflag = wroom->exits[idir].doorflag;
       wroom2->exits[idir].keynumber = wroom->exits[idir].keynumber;
       wroom2->exits[idir].toroomno = wroom->exits[idir].toroomno;
    }
    CREATE(wroom2->name, char ,strlen(wroom->name)+1);
    strcpy(wroom2->name,wroom->name);
    if(wroom->description!=NULL)
    {
      CREATE(wroom2->description, char ,strlen(wroom->description)+1);
      strcpy(wroom2->description,wroom->description);
    }
    else
      wroom2->description=NULL;

    wext=wroom->extra;
    wext3=NULL;
    while(wext!=NULL)
    {
      CREATE(wext2, struct extra_desc, 1);
      if(wroom2->extra==NULL)
        wroom2->extra=wext2;
      else
        wext3->next=wext2;
      wext3=wext2;
      CREATE(wext2->key,char,strlen(wext->key)+1);
      strcpy(wext2->key,wext->key);
      CREATE(wext2->desc,char,strlen(wext->desc)+1);
      strcpy(wext2->desc,wext->desc);
      wext2->next=NULL;
      wext=wext->next;
    }

    SaveToRETemp(mid,RE_SM_INSERT,wroom2);
    free_room(wroom2);
  }
}

/* EDIT THE ROOM */

void  re_EditNr(struct room_data *wroom)
{
  char wstr[RE_MAXSTRLEN];
  int wint;

  printf("Current Room Number: #%d\n",wroom->virt_number);
  printf("New Number (%d):",wroom->virt_number);
  gets(wstr);
  if(*wstr=='\0')
  {
    printf("Room-Nr Unchanged");
    return;
  }
  wint = atoi(wstr);
  if((wint==0)&&(*wstr!='0'))
  {
    printf("** Illegal input\n");
    return;
  }
  if(wint==wroom->virt_number)
  {
    printf("Room-Nr Unchanged\n");
    return;
  }
  re_doclone(wroom,wint);
}

void  re_EditName(struct room_data *wroom)
{
  char wstr[RE_MAXSTRLEN];
  printf("Old Name : %s\n",wroom->name);
  printf("Enter New Name:");
  gets(wstr);
  if(*wstr=='\0')
  {
    printf("Description unchanged.\n");
  }
  else
  {
    free(wroom->name);
    CREATE(wroom->name, char, strlen(wstr) + 1);
    strcpy(wroom->name,wstr);
    re_changed=1;
  }
}

void  re_EditDesc(struct room_data *wroom)
{
  char *wstr;
  printf("Old Description: ");
  if(wroom->description==NULL)
    printf("<NOT SET>\n");
  else
    write_string(wroom->description, strlen("Old Description: "));
  re_changed=1;
  printf("Enter New Description (End with ~):\n");
  wstr=read_multi_string();
  if(wroom->description!=NULL)
    free(wroom->description);
  if(wstr==NULL)
  {
    wroom->description=NULL;
    printf("Description cleared.\n");
  }
  else
  {
    CREATE(wroom->description, char, strlen(wstr) + 1);
    strcpy(wroom->description,wstr);
  }
  free(wstr);
}

void re_EditZone(struct room_data *wroom)
{
  int wint;
  char wstr[RE_MAXSTRLEN];

  printf("Old zone number: %d\n", wroom->zone_nr);
  printf("Enter new zone number: ");
  gets(wstr);
  wint=atoi(wstr);
  if((wint==0)&&(*wstr!='0')||(wint<0))
  {
    printf("Alignment unchanged.");
    return;
  }
  re_changed=1;
  wroom->zone_nr=wint;
}

void re_EditFlags(struct room_data *wroom)
{
  int wint, wint2;
  unsigned long wlong;
  char wstr[RE_MAXSTRLEN];

  for(wint=1;wint<=rf_max;wint=wint+2)
  {
    if(wint+1<=rf_max)
      printf("%.2d.) %-34.34s %.2d.) %-34.34s\n",wint,rf_str[wint],wint+1,rf_str[wint+1]);
    else
      printf("%.2d.) %s\n",wint,rf_str[wint]);
  }
  for(;;)
  {
    printf("\nOld Flags:");
    PrintRFlags(wroom->flags);
    printf("Enter 0 to quit or the Number to toggle :");
    gets(wstr);
    wint=atoi(wstr);
    if((wint!=0)&&((wint<1)||(wint>rf_max)))
    {
      printf("Ilegal Input. Try again.\n");
    }
    else if(wint==0) 
         break;
    else
    {
      wlong=1;
      for(wint2=1;wint2<wint;wint2++)
      {
        wlong=wlong<<1;
      }
      re_changed=1;
      wroom->flags=wroom->flags^wlong;
    }
  }
}

void re_EditSec(struct room_data *wroom)
{
  int wint;
  char wstr[RE_MAXSTRLEN];
  for (wint=1;wint<=rs_max;wint=wint+2)
  {
    if(wint+1<=rs_max)
      printf("%.2d.) %-34.34s %.2d.) %-34.34s\n",
          wint-1,rs_str[wint],wint,rs_str[wint+1]);
    else
      printf("%.2d.) %s\n",wint-1,rs_str[wint]);
   }
   printf("New number of sector type (%s): ", rs_str[wroom->sector+1]);
   gets(wstr);
   wint=atoi(wstr);
   if((wint==0)&&(*wstr!='0')||wint >=rs_max|| wint <= 0)
   {
     printf("Sector type not changed!\n");
   }
   else
   {
     re_changed=1;
     wroom->sector=wint;
   }
}

void  re_EditNorth(struct room_data *wroom)
{
  re_EditDoor(wroom,0);
}

void  re_EditEast(struct room_data *wroom)
{
  re_EditDoor(wroom,1);
}

void  re_EditSouth(struct room_data *wroom)
{
  re_EditDoor(wroom,2);
}

void  re_EditWest(struct room_data *wroom)
{
  re_EditDoor(wroom,3);
}

void  re_EditUp(struct room_data *wroom)
{
  re_EditDoor(wroom,4);
}

void  re_EditDown(struct room_data *wroom)
{
  re_EditDoor(wroom,5);
}

struct extra_desc *CreateExtra(wroom,wext)
  struct room_data *wroom;
  struct extra_desc *wext;
{
  struct extra_desc *new_extra;

  re_changed=1;
  CREATE(new_extra, struct extra_desc, 1);

  CREATE(new_extra->key, char , 6);
  CREATE(new_extra->desc, char , 44);
  strcpy(new_extra->key,"dummy");
  strcpy(new_extra->desc,"This is a Dummy, please make me valid!!");

  new_extra->next = NULL;
  if(wext==NULL)
    wroom->extra=new_extra;
  else
    wext->next=new_extra;
  return(new_extra);
}

void  re_EditExtraDesc(struct room_data *wroom)
{
  struct extra_desc *wext=NULL;
  struct extra_desc *wext2=NULL;
  struct extra_desc *wext3=NULL;
/*  struct extra_desc *CreateExtra(struct room_data *,struct extra_desc *);*/
  struct extra_desc *CreateExtra();
  char wstr[RE_MAXSTRLEN];
  char *wstr2;
  int wint;
  int maxsel=3;

  if(wroom->extra==NULL)
  {
    wext=CreateExtra(wroom,wext);
  }
  {
    wext=wroom->extra;

    do
    {
      printf("1.) Keywords    : %s\n",wext->key);
      printf("2.) Description : ");
      if (wext->desc==NULL)
        printf("<NOT SET>\n");
      else
        write_string(wext->desc, strlen("2.) Description : "));
      printf("3.) Next        ");
      if(wext->next==NULL)
        printf(":<NOT SET>\n");
      else
        printf("-> Set   (Not viewed)\n");
      printf("4.) Delete this entry.\n");
      printf("Enter: 1-2 to edit, 3 to goto/create next, 0 to quit and 4 to delete.\n",maxsel-1+'A');
      gets(wstr);
      wint=atoi(wstr);
      switch(wint)
      {
        case 1: {
                  printf("Old keywords: %s\n",wext->key);
                  printf("Enter new keywords :");
                  gets(wstr);
                  if(*wstr=='\0')
                  {
                    printf("Keywords unchanged.\n");
                  }
                  else
                  {
                    free(wext->key);
                    CREATE(wext->key, char, strlen(wstr) + 1);
                    strcpy(wext->key,wstr);
                    re_changed=1;
                  }
                  break;
                }
        case 2: {
                  printf("Old Description: ");
                  if (wext->desc==NULL)
                    printf("<NOT SET>\n");
                  else
                    write_string(wext->desc, strlen("Old Description: "));
                  printf("Enter New Description (End with ~):\n");
                  wstr2=read_multi_string();
                  if(wstr2==NULL)
                  {
                    printf("Description cleared.\n");
                    wext->desc = NULL;
                    re_changed = 1;
                  }
                  else
                  {
                    free(wext->desc);
                    CREATE(wext->desc, char, strlen(wstr2) + 1);
                    strcpy(wext->desc,wstr2);
                    re_changed=1;
                  }
                  free(wstr2);
                  break;
                }
        case 3: {
                  wext2=wext;
                  if(wext->next!=NULL)
                    wext=wext->next;
                  else
                    wext=CreateExtra(wroom,wext);
                  break;
                }
        case 4: {
                  printf("Are you sure (yes/NO)? ");
                  if(AskYesNo("yes"))
                  {
                    re_changed=1;
                    if(wext2==NULL)
                      wroom->extra=wext->next;
                    else
                      wext2->next=wext->next;
                    wext3=wext->next;
                    free(wext);
                    if(wext3!=NULL)
                      wext=wext3;
                    else
                      return;
                  }
                  break;
                }

        case 0: break;
        default: {
                  printf("Illegal input, try again.");
                  break;
                }
      }

    }
    while(wint!=0);
  }
}

/* Print options for the Door editor */
void print_door(struct room_data *wroom, int idir)
{
   printf(" 1.) Exit-%s to  : [%d]\n", *(direction+idir),wroom->exits[idir].toroomno);
   printf(" 2.) Description    : ");
   if (wroom->exits[idir].gen_desc==NULL)
      printf("<NOT SET>\n");
   else
      write_string(wroom->exits[idir].gen_desc, strlen(" 2.) Description    : "));
   printf(" 3.) Door-Name      : ");
   if (wroom->exits[idir].doorname==NULL)
      printf("<NOT SET>\n");
   else
      printf("%s\n",wroom->exits[idir].doorname);
   printf(" 4.) Key-No         : ");
   if (wroom->exits[idir].keynumber == -1)
      printf("<NO LOCK>\n");
   else
      printf("%d\n", wroom->exits[idir].keynumber);
   printf(" 5.) Door-Flags     : ");
   PrintDFlags(wroom->exits[idir].doorflag);
   printf(" 6.) Purge This Exit\n");
}

/* Edit one of the exits: "idir" */

void  re_EditDoor(struct room_data *wroom, int idir)
{
   int wint, maxsel;
   unsigned long wlong;
   char wstr[RE_MAXSTRLEN];

   maxsel=RE_EDITDOORMAX;
   if (wroom->exits[idir].isset != 1)
   {
     printf("Do You want to open this exit (YES/no) ?");
     if(AskYesNo("no"))
       return;
     else
       wroom->exits[idir].isset = 1;
       re_changed = 1;
   }

   print_door(wroom,idir);
   do
   {
     printf("Enter: 0 to quit, 1-%d to edit.\n",maxsel);
     gets(wstr);
     wint=atoi(wstr);
     if((wint>0)&&(wint<=RE_EDITDOORMAX))
     {
       (*red_edit[wint-1])(wroom,idir);
       if (wroom->exits[idir].isset!=1)
          return;
       print_door(wroom, idir);
     }
     else
     if(wint!=0)
       printf("** Illegal input. Try again!\n");
   }
   while(wint!=0);
}

/* Functions for the door edit array */

void red_EditToRoom(struct room_data *wroom, int idir)
{
  char wstr[RE_MAXSTRLEN];
  int wint;

  printf("Current Exit is to  Room: [%d]\n",wroom->exits[idir].toroomno);
  printf("New Number : ");
  gets(wstr);
  if(*wstr=='\0')
  {
    printf("Exit Unchanged");
    return;
  }
  wint = atoi(wstr);
  if((wint==0)&&(*wstr!='0')||(wint<-1))
  {
    printf("** Illegal input\n");
    return;
  }
  if(wint==wroom->exits[idir].toroomno)
  {
    printf("Exit Unchanged\n");
    return;
  }
  re_changed = 1;
  wroom->exits[idir].toroomno = wint;
}

void red_EditDesc(struct room_data *wroom, int idir)
{
  char *wstr;
  printf("Old Description: ");
  if(wroom->exits[idir].gen_desc==NULL)
    printf("<NOT SET>\n");
  else
    write_string(wroom->exits[idir].gen_desc, strlen("Old Description: "));
  re_changed=1;
  printf("Enter New Description (End with ~):\n");
  wstr=read_multi_string();
  if(wroom->exits[idir].gen_desc!=NULL)
    free(wroom->exits[idir].gen_desc);
  if(wstr==NULL)
  {
    wroom->exits[idir].gen_desc=NULL;
    printf("Description cleared.\n");
  }
  else
  {
    CREATE(wroom->exits[idir].gen_desc, char, strlen(wstr) + 1);
    strcpy(wroom->exits[idir].gen_desc,wstr);
  }
  free(wstr);
}
void red_EditName(struct room_data *wroom, int idir)
{
  char wstr[RE_MAXSTRLEN];
  printf("Old Door-Name : ");
  if(wroom->exits[idir].doorname==NULL)
    printf("<NOT SET>\n");
  else
    printf("%s\n",wroom->exits[idir].doorname);

  printf("Enter new name :");
  gets(wstr);
  if(*wstr=='\0')
  {
    printf("Door-Name unchanged.\n");
  }
  else
  {
    free(wroom->exits[idir].doorname);
    CREATE(wroom->exits[idir].doorname, char, strlen(wstr) + 1);
    strcpy(wroom->exits[idir].doorname,wstr);
    re_changed=1;
  }
}
void red_Key(struct room_data *wroom, int idir)
{
{
  char wstr[RE_MAXSTRLEN];
  int wint;

  printf("Current Key-No : [%d]\n",wroom->exits[idir].keynumber);
  printf("New Number (-1 for <NO LOCK>) : ");
  gets(wstr);
  if(*wstr=='\0')
  {
    printf("Key Number Unchanged");
    return;
  }
  wint = atoi(wstr);
  if((wint==0)&&(*wstr!='0')||(wint<-1))
  {
    printf("** Illegal input\n");
    return;
  }
  if(wint==wroom->exits[idir].keynumber)
  {
    printf("Key Number Unchanged\n");
    return;
  }
  re_changed = 1;
  wroom->exits[idir].keynumber = wint;
}
}
void red_Flags(struct room_data *wroom, int idir)
{
  int wint, wint2;
  unsigned long wlong;
  char wstr[RE_MAXSTRLEN];

  for(wint=1;wint<=rd_max;wint=wint+2)
  {
    if(wint+1<=rd_max)
      printf("%.2d.) %-34.34s %.2d.) %-34.34s\n",wint,rd_str[wint],wint+1,rd_str[
wint+1]);
    else
      printf("%.2d.) %s\n",wint,rd_str[wint]);
  }
  for(;;)
  {
    printf("\nOld Flags:");
    PrintDFlags(wroom->exits[idir].doorflag);
    printf("Enter 0 to quit or the Number to toggle :");
    gets(wstr);
    wint=atoi(wstr);
    if((wint!=0)&&((wint<1)||(wint>rd_max)))
    {
      printf("**Illegal Input. Try again.\n");
    }
    else if(wint==0)
         break;
    else
    {
      wlong=1;
      for(wint2=1;wint2<wint;wint2++)
      {
        wlong=wlong<<1;
      }
      re_changed=1;
      wroom->exits[idir].doorflag=wroom->exits[idir].doorflag^wlong;
    }
  }
}
void red_Purge(struct room_data *wroom, int idir)
{
  printf("Do you really want to purge this exit (YES/no) ? ");
  if (AskYesNo("no"))
  {
    printf("Not purged");
    return;
  }
  else
  {
    re_changed = 1;
    wroom->exits[idir].isset = 0;
    if(wroom->exits[idir].gen_desc!=NULL)
      free(wroom->exits[idir].gen_desc);
    if(wroom->exits[idir].doorname!=NULL)
      free(wroom->exits[idir].doorname);
  }
}

void print_room(struct room_data *wroom)
{
  int i,idir;
  struct extra_desc *wext;

  printf("\n");
  printf("Room-Values:\n");
  printf("--------------\n");
/*  printf("INTERNAL Room-Nr       : %d\n",wroom->room_number); */
  printf(" 1.)  Room-Nr       : %d\n",wroom->virt_number);
  printf(" 2.)  Room-Name     : %s\n",wroom->name);
  printf(" 3.)  Room-Desc     : ");
  if(wroom->description==NULL)
    printf("<NOT SET>\n");
  else
    write_string(wroom->description, strlen(" 3.)  Room-Desc     : "));
  printf(" 4.)  Room-Zone     : %d\n", wroom->zone_nr);
  printf(" 5.)  Room-Flags    : ");
  PrintRFlags(wroom->flags);
  printf(" 6.)  Room-Sector   : %s\n", rs_str[wroom->sector+1]);
  for (idir=0;idir<6;idir++)
  {
    printf("%2d.)  Exit-%s to : ", 7+idir,*(direction+idir));
    if (wroom->exits[idir].isset == 1)
    {
      printf("[%d]\n ",  wroom->exits[idir].toroomno);
      if (wroom->exits[idir].gen_desc==NULL)
          printf("     Desc          :<NOT SET>\n");
      else
          printf("     Desc          : %s\n", wroom->exits[idir].gen_desc);
      if (wroom->exits[idir].doorname==NULL)
          printf("      Door-Name     :<NOT SET>\n");
      else
          printf("      Door-Name     : %s\n",wroom->exits[idir].doorname);
      printf("      Key-No        :");
      if (wroom->exits[idir].keynumber == -1)
         printf("<NO LOCK>\n");
      else
         printf(" %d\n", wroom->exits[idir].keynumber);
      printf("      Door-Flags    : ");
      PrintDFlags(wroom->exits[idir].doorflag);
    }
    
    else if (wroom->exits[idir].isset == -1)
       printf("<Illegal direction>\n");
    else
       printf("<NOT SET>\n");
  }
  printf("13.)  Extra-Desc    : ");
  wext = wroom->extra;
  if (wext==NULL)
   printf("<NOT SET>\n");
  else 
  {
   printf("\n");
   while (wext!=NULL)
   {
     printf("            Keyword : %s\n", wext->key);
     printf("            Desc    : ");
     if (wext->desc==NULL)
        printf("<NOT SET>\n");
     else
        write_string(wext->desc, strlen("            Desc    : "));
     wext=wext->next;
   }
  }
  

}

int re_RoomZone(int nr)
{
  struct room_data *wroom;
  int wint;

  wroom=read_room(nr,VIRTUAL);
  wint=wroom->zone_nr;
  free_room(wroom);
  return(wint);
}

/* Get Real Number for DikuCli */

int re_GetRealNumber(int wint)
{
  return(real_room(wint));
}

/* Edit all Room Data */

int EditAllRoom(struct room_data *wroom)
{
  int maxsel;
  char wstr[RE_MAXSTRLEN];
  int wint;

  maxsel=RE_EDITMAX;

  print_room(wroom);
  do
  {
    printf("Enter: 0 to quit, 1-%d to edit.\n",maxsel);
    gets(wstr);
    wint=atoi(wstr);
    if((wint>0)&&(wint<=RE_EDITMAX))
    {
      (*re_edit[wint-1])(wroom);
      print_room(wroom);
    }
    else
    if(wint!=0)
      printf("** Illegal input. Try again!\n");
  }
  while(wint!=0);

  return(RE_SM_SAVE);
}

/* Edit a Room for DikuCli */

void re_EditRoom(int nr)
{
  struct room_data *wroom;
  int re_m;

  re_changed=0;

  wroom=read_room(nr,REAL);
  re_m=EditAllRoom(wroom);
  if((re_changed!=0)||(re_m==RE_SM_DELETE))
    SaveToRETemp(nr,re_m,wroom);
  free_room(wroom);
}

/* Clone a Room (for DikuCli) */

void re_CloneRoom(int nr1, int nr2)
{
  struct room_data *wroom;
  if(nr1<0)
    wroom= &re_defaultroom;
  else
    wroom=read_room(nr1,REAL);

  re_doclone(wroom,nr2);

  if(nr1>=0)
    free_room(wroom);
}

/* Purge a Room for DikuCli */

void re_PurgeRoom(int nr)
{
  if(sim_get_curr_room()!=room_index[nr].virtual)
    SaveToRETemp(nr,RE_SM_DELETE,NULL);
  else
    printf("Please leave the room before purging it.\n");
}

/* Show Room for DikuCli */

void re_ShowRoom(int nr)
{
  struct room_data *wroom;

  wroom=read_room(nr,REAL);
  print_room(wroom);
  free_room(wroom);
}

/* Set FileName of *.wld file */

void re_SetFile(char *wstr)
{
  strcpy(re_roomfile,wstr);
}

/* Set TempFile */

void re_SetTemp(char *wstr)
{
  strcpy(re_tempfile,wstr);
}

/* Set database path */

void re_SetDBPath(char *wstr)
{
  strcpy(re_dbpath,wstr);
}

/* Initialize the REdit vars */

void re_InitREdit()
{

  re_changed=0;
  if(GetADef(rd_file,RE_DFILE,re_dbpath,&rd_str,&rd_max))
    printf("*** Can't open a *.def file!\n");
  if(GetADef(rf_file,RE_FFILE,re_dbpath,&rf_str,&rf_max))
    printf("*** Can't open a *.def file!\n");
  if(GetADef(rs_file,RE_SFILE,re_dbpath,&rs_str,&rs_max))
    printf("*** Can't open a *.def file!\n");
  if(OpenRoomFile())
    printf("***** IMPOSSIBLE ERROR *****");
}


void re_CloseREdit(void)
{
  fclose(room_f);
  FreeADef(rd_str,rd_max);
  FreeADef(rf_str,rf_max);
  FreeADef(rs_str,rs_max);
}
