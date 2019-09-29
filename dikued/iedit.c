/*****************************************************************************
*                                                                            *
* Project : Diku-Mud-World-Creator                                           *
*                                                                            *
* Program : Diku-Mud-Item-Editor                                             *
* Version : 1.6                                                              *
* Last changed on: 27.7.93                                                   *
*                                                                            *
* Code by: Georg Essl (essl@fstgds06.tu-graz.ac.at)                          *
*                                                                            *
*****************************************************************************/

/******* ITEMS ******/

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

/****** DEFINES from "structs.h" *****/

#define APPLY_NONE  0
#define NOWHERE    -1
#define ITEM_WEAPON 5

/***** TYPEDEFS *****/

/*typedef char bool;
 typedef char byte;*/

typedef   signed char  sbyte;
typedef unsigned char  ubyte;
#if !defined(sgi) && !defined(m88k) &&!defined(_AIX)
// typedef unsigned long  ulong;
#endif
typedef   signed short sh_int;


/****** STRUCTS from "structs.h" *****/

struct extra_descr_data
{
	char *keyword;                 /* Keyword in look/examine          */
	char *description;             /* What to see                      */
	struct extra_descr_data *next; /* Next in list                     */
};

#define MAX_OBJ_AFFECT 2         /* Used in OBJ_FILE_ELEM *DO*NOT*CHANGE* */
#define OBJ_NOTIMER    -7000000

struct obj_flag_data
{
  int value[4];       /* Values of the item (see list)    */
  char type_flag;     /* Type of item                     */
  long wear_flags;     /* Where you can wear it            */
  long extra_flags;    /* If it hums,glows etc             */
  int weight;         /* Weigt what else                  */
  long cost;           /* Value when sold (gp.)            */
  long cost_per_day;   /* Cost to keep pr. real day        */
  int timer;          /* Timer for object                 */
  long bitvector;     /* To set chars bits                */
};

/* Used in OBJ_FILE_ELEM *DO*NOT*CHANGE* */
struct obj_affected_type
{
  char location;      /* Which ability to change (APPLY_XXX) */
  sh_int  modifier;     /* How much it changes by              */
};

/* ======================== Structure for object ========================= */
struct obj_data
{
  sh_int item_number;            /* Where in data-base               */
  int    virt_number;
  sh_int in_room;                /* In what room -1 when conta/carr  */
  struct obj_flag_data obj_flags;/* Object information               */
  struct obj_affected_type
  affected[MAX_OBJ_AFFECT];      /* Which abilities in PC to change  */
  char *name;                    /* Title of object :get etc.        */
  char *description ;            /* When in room                     */
  char *short_description;       /* when worn/carry/in cont.         */
  char *action_description;      /* What to write when used          */
  struct extra_descr_data *ex_description; /* extra descriptions     */
};
/* ======================================================================= */


/*** The default initial item ***/

struct obj_data ie_defaultitem=
{
  0,
  0,
  0,
  { {0,0,0,0},
    13,         /* TRASH */
    1,          /* Take */
    0,          /* No Extras */
    1,
    1,
    1,
    0,
    0
  },
  {{0,0},{0,0}},
  "dummy fake",
  "a dummy fake floats through the database here.",
  "a dummy fake",
  NULL,
  NULL
};

/****** DEFINES *****/

/* Item-Edit */
#define IE_COMM '#'
#define IE_MAXSTRLEN 128

#define IE_OBJFILE "tobj.obj"
#define IE_TEMPFILE "ietemp.obj"

#define IE_ITFILE  "itypes.def"
#define IE_IWFILE  "iwears.def"
#define IE_IEXFILE "iextras.def"
#define IE_IDLFILE "iliquids.def"
#define IE_ICTFILE "icontain.def"
#define IE_IPAFILE "ipaffect.def"
#define IE_IAFILE  "iapplies.def"
#define IE_ITVFILE "itvals.def"
#define IE_ISFILE  "ispells.def"
#define IE_ITWFILE "iweapons.def"

#define IE_EDITMAX 16

#define IE_SM_DELETE 1
#define IE_SM_SAVE   2
#define IE_SM_INSERT 3

#define IE_M_QUIT   0

/* Item Spells */
#define IS_MAXVALID 255

/* Item-File-Format */
#define IF_START  '#'
#define IF_EOS    '~'
#define IF_EXDISC 'E'
#define IF_AFF    'A'
#define IF_IPAFF  'B'
#define IF_EOF    "$~"

#define REAL 0
#define VIRTUAL 1

/***** STRUCTS *****/

struct if_affected
{
  long ia_affwhat;
};

typedef struct if_fileformat
{
  sh_int if_number;
  char   *if_namelist;
  char   *if_shdisc;
  char   *if_longdisc;
  char   *if_actdisc;
  int    if_value[4];
  int    if_weight;
  long   if_cvalue;
  long   if_ccost;
  int    *If_exdisc;
  struct if_affected affected[MAX_OBJ_AFFECT+1];
} if_fileformat;


struct itv_entry
{
  int special[4];
  char *str[4];
} *itv_entries;

struct itw_entry
{
  int nr;
  char *str;
  struct itw_entry *next;
};

struct itw_entry *itw_first;
struct is_entry *is_first;
int is_valid[IS_MAXVALID+1];

struct is_entry
{
  int nr;
  int potion,scroll,wand,staff;
  int minlevel;
  char *str;
  struct is_entry *next;
};

/***** Constants *****/

const char itv_notused[12]="<NOT USED>";

/***** Global Vars *****/

char ie_objfile[IE_MAXSTRLEN]=IE_OBJFILE;
char ie_tempfile[IE_MAXSTRLEN]=IE_TEMPFILE;

char ie_dbpath[IE_MAXSTRLEN]="";

int ie_affbitvec=0;

int it_max;
int iw_max;
int iex_max;
int idl_max;
int ict_max;
int ipa_max;
int ia_max;

int ie_changed;

char **it_str;
char **iw_str;
char **iex_str;
char **idl_str;
char **ict_str;
char **ipa_str;
char **ia_str;

FILE *ie_workfile;
FILE *it_file;
FILE *iw_file;
FILE *iex_file;
FILE *idl_file;
FILE *ict_file;
FILE *ipa_file;
FILE *ia_file;
FILE *itv_file;
FILE *itw_file;
FILE *is_file;

FILE *ie_temp;

/***** ERROR-DEFINES *****/

#define IE_ERR_NOERROR      0
#define IE_ERR_LINEINVALID  1
#define IE_ERR_INVALIDSTART 2
#define IE_ERR_OOMFOROBJ    3
#define IE_ERR_STRINGTOOLONG 4
#define IE_ERR_CANTOPEN     5


/***** Vars from db.c from original DikuMud! *****/

struct obj_data  *object_list = 0;    /* the global linked list of obj's */
FILE *obj_f;                          /* obj prototypes                  */
struct index_data *obj_index;         /* index table for object file     */
int top_of_objt = 0;                  /* top of object index table       */


/* prototypes */
void  clear_object(struct obj_data *obj);
int   real_object(int virtual);
struct obj_data *read_object(int nr, int type);

void  ie_EditNr(struct obj_data *);
void  ie_EditNameList(struct obj_data *);
void  ie_EditShortDesc(struct obj_data *);
void  ie_EditLongDesc(struct obj_data *);
void  ie_EditActDesc(struct obj_data *);
void  ie_EditType(struct obj_data *);
void  ie_EditExtras(struct obj_data *);
void  ie_EditWear(struct obj_data *);
void  ie_EditWeight(struct obj_data *);
void  ie_EditCost(struct obj_data *);
void  ie_EditCPD(struct obj_data *);
void  ie_EditTimer(struct obj_data *);
void  ie_EditValues(struct obj_data *);
void  ie_EditApply(struct obj_data *);
void  ie_EditAffects(struct obj_data *);
void  ie_EditExtraDesc(struct obj_data *);

/*** A FUNCTION ARRAY (OH GREAT :-D) ***/

void (*ie_edit[IE_EDITMAX+1])(struct obj_data *) =
{
  ie_EditNr,
  ie_EditNameList,
  ie_EditShortDesc,
  ie_EditLongDesc,
  ie_EditActDesc,
  ie_EditType,
  ie_EditExtras,
  ie_EditWear,
  ie_EditWeight,
  ie_EditCost,
  ie_EditCPD,
  ie_EditTimer,
  ie_EditValues,
  ie_EditApply,
  ie_EditAffects,
  ie_EditExtraDesc
};


char getch(void)
{
  int wint1,wint2;
  wint1=getchar();
  wint2=wint1;
  while(wint1!='\n')
    wint1=getchar();
  return(wint2);
}

/* Open the object-file defined in ie_objfile */

int OpenObjFile(void)
{
  struct obj_data *wobj;

	if (!(obj_f = fopen(ie_objfile, "r")))
	{
	  printf("*** Can't open %s-file!\n",ie_objfile);
	  printf("Want to create a new one (YES/no)? ");
	  if(AskYesNo("no"))
	    exit(0);
	  else
	  {
	    if(!(obj_f = fopen(ie_objfile,"w")))
	    {
	      printf("*** Can't create %s-file!\n",ie_objfile);
	      exit(0);
	    }
	    else
	    {
	      fprintf(obj_f,"#0\n");
	      wobj= &ie_defaultitem;
	      fwrite_string(obj_f,wobj->name);
	      fwrite_string(obj_f,wobj->short_description);
	      fwrite_string(obj_f,wobj->description);
	      fwrite_string(obj_f,wobj->action_description);

	      fprintf(obj_f,"%u %lu %lu\n",wobj->obj_flags.type_flag,wobj->obj_flags.extra_flags,wobj->obj_flags.wear_flags);
	      fprintf(obj_f,"%d %d %d %d\n",wobj->obj_flags.value[0],wobj->obj_flags.value[1],wobj->obj_flags.value[2],wobj->obj_flags.value[3]);
	      fprintf(obj_f,"%u %lu %lu\n",wobj->obj_flags.weight,wobj->obj_flags.cost,wobj->obj_flags.cost_per_day);
	      fprintf(obj_f,"#19999\n");
	      fprintf(obj_f,IF_EOF);
	      fprintf(obj_f,"\n");
	      fclose(obj_f);
	      if (!(obj_f = fopen(ie_objfile, "r")))
	      {
		printf("*** Can't reopen %s-file!\n",ie_objfile);
		exit(0);
	      }
	    }
	  }
	}
	obj_index = generate_indices(obj_f, &top_of_objt);
	return(IE_ERR_NOERROR);
}


char *ie_ReadLDesc(int nr)
{
  char *buf;
  int tmp;
  if((nr=real_object(nr))<0)
    return(NULL);

  fseek(obj_f, obj_index[nr].pos, 0);
  fscanf(obj_f,"#%d\n",&tmp);
  buf = fread_string(obj_f);
  free(buf);
  buf = fread_string(obj_f);
  free(buf);
  buf = fread_string(obj_f);
  return(buf);
}

char *ie_ReadSDesc(int nr)
{
  char *buf;
  int tmp;
  if((nr=real_object(nr))<0)
    return(NULL);

  fseek(obj_f, obj_index[nr].pos, 0);
  fscanf(obj_f,"#%d\n",&tmp);
  buf = fread_string(obj_f);
  free(buf);
  buf = fread_string(obj_f);
  return(buf);
}

char *ie_ReadName(int nr)
{
  char *buf;
  int tmp;
  if((nr=real_object(nr))<0)
    return(NULL);

  fseek(obj_f, obj_index[nr].pos, 0);
  fscanf(obj_f,"#%d\n",&tmp);
  buf = fread_string(obj_f);
  return(buf);
}

/* read an object from OBJ_FILE */

struct obj_data *read_object(int nr, int type)
{
	struct obj_data *obj;
	int tmp, i;
	long tmp2;
	char chk[50], buf[100];
    char bitflag[255];
	struct extra_descr_data *new_descr;

	i = nr;
	if (type == VIRTUAL)
		if ((nr = real_object(nr)) < 0)
	{
		sprintf(buf, "Object (V) %d does not exist in database.", i);
		return(IE_ERR_NOERROR);
	}

	fseek(obj_f, obj_index[nr].pos, 0);

	CREATE(obj, struct obj_data, 1);

	clear_object(obj);

	/* *** string data *** */
	fscanf(obj_f, "#%d\n", &tmp);
	obj->virt_number=tmp;
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
	obj->obj_flags.extra_flags = tmp;

    fscanf(obj_f, "%s \n", bitflag);
    if (isdigit(bitflag[0])) tmp = atol(bitflag);
    else tmp = flag_convert(bitflag);
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
	fscanf(obj_f, " %lu \n", &tmp2);
	obj->obj_flags.cost = tmp2;
	fscanf(obj_f, " %lu \n", &tmp2);
	obj->obj_flags.cost_per_day = tmp2;

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

	if(ie_affbitvec)
	{
	  if(*chk == 'B')
	  {
	    fscanf(obj_f," %lu \n", &tmp2);
	    obj->obj_flags.bitvector=tmp2;
	    fscanf(obj_f, " %s \n", chk);
	  }
	}
	obj->in_room = NOWHERE;
	obj->item_number = nr;

	object_list = obj;

	return (obj);
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

	for( this = obj->ex_description ;
		(this != 0);this = next_one )
	{
		next_one = this->next;
		if(this->keyword)
			free(this->keyword);
		if(this->description)
			free(this->description);
		free(this);
	}

	free(obj);
}








/* Clear object */

void clear_object(struct obj_data *obj)
{
	memset(obj, '\0', sizeof(struct obj_data));

	obj->item_number = -1;
	obj->in_room	  = NOWHERE;
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

/* Get Real Number for DikuCli */

int ie_GetRealNumber(int wint)
{
  return(real_object(wint));
}

/* Save ie_tempfile with changed object and copy to ie_objfile */
int SaveToIETemp(int nr, int mode,struct obj_data *wobj)
{
  struct obj_data *obj;
  int tmp, i;
  long tmp2, j,j2,j3,obj_i;
  long store_pos,store_cpos;
  char chk[50], buf[100];
  struct extra_descr_data *new_descr;
  struct extra_descr_data *wext;
  char *temp_buff;

  switch(mode)
  {
    case IE_SM_INSERT: {
			 printf("Do you really want to CREATE/CLONE the item (YES/no)? ");
			 if(AskYesNo("no"))
			 {
			   printf("Not Done.\n");
			   return(0);
			 }
			 break;
		       }
    case IE_SM_SAVE: {
		       printf("Do you really want to SAVE the changes (YES/no)? ");
		       if(AskYesNo("no"))
		       {
			 printf("Not Done.\n");
			 return(0);
		       }
		       break;
		     }
    case IE_SM_DELETE: {
			 printf("Do you really want to DELETE the item (yes/NO)? ");
			 if(!AskYesNo("yes"))
			 {
			   printf("Not Done.\n");
			   return(0);
			 }
			 break;
		       }
  }


  if(mode==IE_SM_INSERT)
  {
    if (!(obj_index =
	    (struct index_data*) realloc(obj_index,
	    (top_of_objt + 3) * sizeof(struct index_data))))
    {
	    perror("load indices");
	    exit(0);
    }
    top_of_objt++;

    for(i=top_of_objt+1;i>nr;i--)
    {
      obj_index[i].virtual=obj_index[i-1].virtual;
      obj_index[i].pos=obj_index[i-1].pos;
      obj_index[i].cpos=obj_index[i-1].cpos;
    }
    obj_index[nr].virtual=wobj->virt_number;
  }




  rewind(obj_f);

  if (!(ie_temp = fopen(ie_tempfile, "w")))
  {
    printf("Can't open temporary file!!!\n");
    exit(0);
    /*** !!! ***/
  }
  else
  {
    if(obj_index[nr].pos!=0)
    {
      obj_i = obj_index[nr].cpos;
      j = obj_i;

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
      while(j2!=obj_i)
      {
	if((j2+j)>obj_i)
	  j=obj_i-j2;
	j3=fread(temp_buff,sizeof(char),j,obj_f);
	fwrite(temp_buff,sizeof(char),j3,ie_temp);
	j2+=j3;
      }
      free(temp_buff);
    }

    if((mode==IE_SM_SAVE)||(mode==IE_SM_INSERT))
    {
      fprintf(ie_temp,"#%d\n",wobj->virt_number);
      fwrite_string(ie_temp,wobj->name);
      fwrite_string(ie_temp,wobj->short_description);
      fwrite_string(ie_temp,wobj->description);
      fwrite_string(ie_temp,wobj->action_description);

      fprintf(ie_temp,"%u %lu %lu\n",wobj->obj_flags.type_flag,wobj->obj_flags.extra_flags,wobj->obj_flags.wear_flags);
      fprintf(ie_temp,"%d %d %d %d\n",wobj->obj_flags.value[0],wobj->obj_flags.value[1],wobj->obj_flags.value[2],wobj->obj_flags.value[3]);
      fprintf(ie_temp,"%u %lu %lu\n",wobj->obj_flags.weight,wobj->obj_flags.cost,wobj->obj_flags.cost_per_day);


      wext=wobj->ex_description;
      while(wext!=NULL)
      {
	fprintf(ie_temp,"E\n");
	fwrite_string(ie_temp,wext->keyword);
	fwrite_string(ie_temp,wext->description);
	wext=wext->next;
      }

      for( i = 0 ; (i < MAX_OBJ_AFFECT); i++)
      {
	if(wobj->affected[i].location!=0)
	{
	  fprintf(ie_temp,"A\n");
	  fprintf(ie_temp,"%d %d\n",wobj->affected[i].location,wobj->affected[i].modifier);
	}
      }

      if(ie_affbitvec)
      {
	if(wobj->obj_flags.bitvector!=0)
	{
	  fprintf(ie_temp,"B\n");
	  fprintf(ie_temp,"%lu\n",wobj->obj_flags.bitvector);
	}
      }
      store_pos=ftell(ie_temp)-obj_index[nr].pos;
    }


    fseek(obj_f, obj_index[nr+1].pos,0);
    fseek(obj_f,0,2);
    tmp2=ftell(obj_f);
    fseek(obj_f, obj_index[nr+1].pos,0);

    j = tmp2-obj_index[nr+1].pos;

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
	j2=fread(temp_buff,sizeof(char),j,obj_f);
	if(j2!=0)
	  fwrite(temp_buff,sizeof(char),j2,ie_temp);
      }
      while(j2!=0);

      free(temp_buff);
    }
    fclose(ie_temp);
  }

  switch(mode)
  {
    case IE_SM_DELETE :
			{
			  j=obj_index[nr+1].pos-obj_index[nr].pos;
			  j2=obj_index[nr+1].cpos-obj_index[nr].cpos;
			  for(i=nr;i<top_of_objt+1;i++)
			  {
			    obj_index[i].virtual=obj_index[i+1].virtual;
			    obj_index[i].pos=obj_index[i+1].pos-j;
			    obj_index[i].cpos=obj_index[i+1].cpos-j2;
			  }
			  if (!(obj_index =
				  (struct index_data*) realloc(obj_index,
				  (top_of_objt+1) * sizeof(struct index_data))))
			  {
				  perror("load indices");
				  exit(0);
			  }
			  top_of_objt--;
			  break;
			}
    case IE_SM_INSERT :
			{
			  if (!(ie_temp = fopen(ie_tempfile, "r")))
			  {
			    printf("Can't open temporary file!!!\n");
			    exit(0);
			    /*** !!! ***/
			  }

			  fseek(ie_temp,obj_index[nr].pos,0);

			  j3=0;
			  fgets(buf, 81, ie_temp);
			  j3+=strlen(buf);
			  for (;;)
			  {
			    if (fgets(buf, 81, ie_temp))
			    {
			      if (*buf == '#')
				break;
			      else
				j3+=strlen(buf);
			    }
			  }
			  fclose(ie_temp);

			  for(i=nr+1;i<=top_of_objt+1;i++)
			  {
			    obj_index[i].pos+=store_pos;
			    obj_index[i].cpos+=j3;
			  }
			  break;
			}
    case IE_SM_SAVE   :
			{
			  if (!(ie_temp = fopen(ie_tempfile, "r")))
			  {
			    printf("Can't open temporary file!!!\n");
			    exit(0);
			    /*** !!! ***/
			  }

			  fseek(ie_temp,obj_index[nr].pos,0);


			  j3=0;
			  fgets(buf, 81, ie_temp);
			  j3+=strlen(buf);
			  for (;;)
			  {
			    if (fgets(buf, 81, ie_temp))
			    {
			      if (*buf == '#')
				break;
			      else
				j3+=strlen(buf);
			    }
			  }
			  fclose(ie_temp);
			  store_cpos=obj_index[nr+1].cpos-obj_index[nr].cpos;
			  j2=obj_index[nr+1].pos-obj_index[nr].pos;

			  for(i=nr+1;i<=top_of_objt+1;i++)
			  {
			    obj_index[i].pos+=store_pos-j2;
			    obj_index[i].cpos+=j3-store_cpos;
			  }

			  break;
			}
  }

  fclose(obj_f);
  if (!(obj_f = fopen(ie_objfile, "w")))
  {
    printf("Can't reopen %s-file for saving!\n",ie_objfile);
    exit(0);
  }
  if (!(ie_temp = fopen(ie_tempfile, "r")))
  {
    printf("Can't reopen temporary file for copying!");
    exit(0);
  }
  fseek(ie_temp,0,2);
  tmp2=ftell(ie_temp);
  rewind(ie_temp);
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
  j3=fread(temp_buff,sizeof(char),j2,ie_temp);
  while(j3!=0)
  {
    fwrite(temp_buff,sizeof(char),j3,obj_f);
    j3=fread(temp_buff,sizeof(char),j2,ie_temp);
  }
  if(temp_buff!=NULL)
    free(temp_buff);
  fclose(ie_temp);
  fclose(obj_f);

  if (!(obj_f = fopen(ie_objfile, "r")))
  {
    printf("Can't reopen %s-file for reading!\n",ie_objfile);
    exit(0);
  }
  return(0);
}

/* Print Bitfield EXTRAS */

void PrintExtras(long wext)
{
  int i,j;
  long shext;
  j=0;
  shext=wext;
  for(i=1;i<=iex_max;i++)
  {
    if((shext&1))
    {
      printf("%s  ",iex_str[i]);
      j=1;
    }
    shext=shext>>1;
  }
  if(!j)
    printf("None\n");
  else
    printf("\n");
}


/* Print Bitfield WEARS */

void PrintWears(long wwear)
{
  int i,j;
  long shwear;
  j=0;
  shwear=wwear;
  for(i=1;i<=iw_max;i++)
  {
    if((shwear&1))
    {
      printf("%s  ",iw_str[i]);
      j=1;
    }
    shwear=shwear>>1;
  }
  if(!j)
    printf("None\n");
  else
    printf("\n");
}

/* Find a Item in Database */

int ie_FindNrRealItem(int nr,char *wstr)
{
  int tmp;
  char *tname1;
  char *tname2;
  char tname[IE_MAXSTRLEN];

  int i,count;
  char *name;

  count=0;
  for (i=0; i<=top_of_objt;i++)
  {
    fseek(obj_f, obj_index[i].pos, 0);

    fscanf(obj_f, "#%d\n", &tmp);
    name  = fread_string(obj_f);
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
        if(!strpcmp(wstr,tname2))
        {
	  free(tname1);
          free(name);
          count++;
          if(count==nr)
          {
	    return(tmp);
          }
        }
        tname2=strtok(NULL," ");
      }
    }
  }
  return(-1);
}

/* Find a Item in Database */

char *ie_FindItem(int nr,char *wstr)
{
  int tmp;
  char *name;
  char *tname1;
  char *tname2;
  char tname[IE_MAXSTRLEN];

  fseek(obj_f, obj_index[nr].pos, 0);

  fscanf(obj_f, "#%d\n", &tmp);
  name  = fread_string(obj_f);
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

void ie_FindItems(char *wstr)
{
  int i;
  char *name;

  printf("Obj-Nr  Name\n");
  printf("-------------------------\n");

  for (i=0; i<=top_of_objt;i++)
  {
    if((name=ie_FindItem(i,wstr))!=NULL)
    {
      printf("[%4.4d] %s\n",obj_index[i].virtual,name);
      free(name);
    }
  }
}

int ie_FindNrItem(int nr, char *wstr)
{
  int i,count;
  char *name;

  count=0;
  for (i=0; i<=top_of_objt;i++)
  {
    if((name=ie_FindItem(i,wstr))!=NULL)
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

/* Print Bitfield Container-Type */

void PrintContainer(int wcont)
{
  int i,j;
  int shcont;
  j=0;
  shcont=wcont;
  for(i=1;i<=ict_max;i++)
  {
    if((shcont&1))
    {
      printf("%s  ",ict_str[i]);
      j=1;
    }
    shcont=shcont>>1;
  }
  if(!j)
    printf("None");
}

/* Print Bitfield AFFECTS */

void PrintAffects(unsigned long waff)
{
  unsigned long i,j;
  unsigned long shaff;
  j=0;
  shaff=waff;
  for(i=1;i<=ipa_max;i++)
  {
    if((shaff&1))
    {
      printf("%s  ",ipa_str[i]);
      j=1;
    }
    shaff=shaff>>1;
  }
  if(!j)
    printf("None\n");
  else
    printf("\n");
}


void ie_doclone(struct obj_data *wobj,int wint)
{
  int bot, top, mid, used, i;
  struct obj_data *wobj2;
  struct extra_descr_data *wext;
  struct extra_descr_data *wext2;
  struct extra_descr_data *wext3;

  bot = 0;
  top = top_of_objt;

  /* perform binary search on obj-table */
  for (;;)
  {
    mid = (bot + top) / 2;

    if ((obj_index + mid)->virtual == wint)
    {
      printf("Given Item-Number is already used! Clone canceled.\n");
      used=1;
      break;
    }
    if (bot >= top)
    {
      if((obj_index + mid)->virtual <wint)
	mid++;
      used=0;
      break;
    }
    if ((obj_index + mid)->virtual > wint)
      top = mid - 1;
    else
      bot = mid + 1;
  }
  if(!used)
  {
    if(wobj == &ie_defaultitem)
      printf("Creating Dummy-Item at [%d].\n",wint);
    else
      printf("Cloning Item from [%d] to [%d].\n",wobj->virt_number,wint);
    CREATE(wobj2, struct obj_data, 1);

    /* The following lines proof the great advantage of obj.o.programming */
    /* and the disadvantage here! */

    wobj2->virt_number=wint;
    wobj2->item_number=mid;
    wobj2->obj_flags.type_flag=wobj->obj_flags.type_flag;
    wobj2->obj_flags.wear_flags=wobj->obj_flags.wear_flags;
    wobj2->obj_flags.extra_flags=wobj->obj_flags.extra_flags;
    wobj2->obj_flags.weight=wobj->obj_flags.weight;
    wobj2->obj_flags.cost=wobj->obj_flags.cost;
    wobj2->obj_flags.cost_per_day=wobj->obj_flags.cost_per_day;
    wobj2->obj_flags.timer=wobj->obj_flags.timer;
    wobj2->obj_flags.bitvector=wobj->obj_flags.bitvector;
    for(i=0;i<4;i++)
      wobj2->obj_flags.value[i]=wobj->obj_flags.value[i];
    for(i=0;i<MAX_OBJ_AFFECT;i++)
    {
      wobj2->affected[i].location=wobj->affected[i].location;
      wobj2->affected[i].modifier=wobj->affected[i].modifier;
    }
    CREATE(wobj2->name, char ,strlen(wobj->name)+1);
    strcpy(wobj2->name,wobj->name);
    CREATE(wobj2->description, char ,strlen(wobj->description)+1);
    strcpy(wobj2->description,wobj->description);
    CREATE(wobj2->short_description, char ,strlen(wobj->short_description)+1);
    strcpy(wobj2->short_description,wobj->short_description);
    if(wobj->action_description!=NULL)
    {
      CREATE(wobj2->action_description, char ,strlen(wobj->action_description)+1);
      strcpy(wobj2->action_description,wobj->action_description);
    }
    else
      wobj2->action_description=NULL;
    wobj2->ex_description=NULL;
    wext=wobj->ex_description;
    wext3=NULL;
    while(wext!=NULL)
    {
      CREATE(wext2, struct extra_descr_data, 1);
      if(wobj2->ex_description==NULL)
	wobj2->ex_description=wext2;
      else
	wext3->next=wext2;
      wext3=wext2;
      CREATE(wext2->keyword,char,strlen(wext->keyword)+1);
      strcpy(wext2->keyword,wext->keyword);
      CREATE(wext2->description,char,strlen(wext->description)+1);
      strcpy(wext2->description,wext->description);
      wext2->next=NULL;
      wext=wext->next;
    }
    SaveToIETemp(mid,IE_SM_INSERT,wobj2);
    free_obj(wobj2);
  }
}

/* EDIT THE ITEM */

void  ie_EditNr(struct obj_data *wobj)
{
  char wstr[IE_MAXSTRLEN];
  int wint;

  printf("Current Item Number: #%d\n",wobj->virt_number);
  printf("New Number (%d):",wobj->virt_number);
  gets(wstr);
  if(*wstr=='\0')
  {
    printf("Item-Nr Unchanged");
    return;
  }
  wint = atoi(wstr);
  if((wint==0)&&(*wstr!='0'))
  {
    printf("** Illegal input\n");
    return;
  }
  if(wint==wobj->virt_number)
  {
    printf("Item-Nr Unchanged\n");
    return;
  }
  ie_doclone(wobj,wint);
}

void  ie_EditNameList(struct obj_data *wobj)
{
  char wstr[IE_MAXSTRLEN];
  printf("Old Name List: %s\n",wobj->name);
  printf("Enter New List :");
  gets(wstr);
  if(*wstr=='\0')
  {
    printf("Description unchanged.\n");
  }
  else
  {
    free(wobj->name);
    CREATE(wobj->name, char, strlen(wstr) + 1);
    strcpy(wobj->name,wstr);
    ie_changed=1;
  }
}

void  ie_EditShortDesc(struct obj_data *wobj)
{
  char wstr[IE_MAXSTRLEN];
  printf("Old ShortDesc: %s\n",wobj->short_description);
  printf("Enter New ShortDesc :");
  gets(wstr);
  if(*wstr=='\0')
  {
    printf("Description unchanged.\n");
  }
  else
  {
    free(wobj->short_description);
    CREATE(wobj->short_description, char, strlen(wstr) + 1);
    strcpy(wobj->short_description,wstr);
    ie_changed=1;
  }
}

void  ie_EditLongDesc(struct obj_data *wobj)
{
  char wstr[IE_MAXSTRLEN];
  printf("Old LongDesc: %s\n",wobj->description);
  printf("Enter New LongDesc :");
  gets(wstr);
  if(*wstr=='\0')
  {
    printf("Description unchanged.\n");
  }
  else
  {
    free(wobj->description);
    CREATE(wobj->description, char, strlen(wstr) + 1);
    strcpy(wobj->description,wstr);
    ie_changed=1;
  }
}

void  ie_EditActDesc(struct obj_data *wobj)
{
  char *wstr;

  if(wobj->action_description==NULL)
    printf("Old ActionDesc: <NOT SET>\n");
  else
  printf("Old ActionDesc:\n%s\n",wobj->action_description);
  ie_changed=1;
  printf("Enter New ActionDesc (End with ~):\n");
  wstr=read_multi_string();
  if(wobj->action_description!=NULL)
    free(wobj->action_description);
  if(*wstr=='\0')
  {
    wobj->action_description=NULL;
    printf("Description cleared.\n");
  }
  else
  {
    CREATE(wobj->action_description, char, strlen(wstr) + 1);
    strcpy(wobj->action_description,wstr);
  }
  free(wstr);
}

void  ie_EditType(struct obj_data *wobj)
{
  int wint;
  char wstr[IE_MAXSTRLEN];

  printf("Old Type:");
  if(wobj->obj_flags.type_flag>it_max)
    printf("*UNKNOWN* - PLEASE REPORT OR UPDATE -ITYPES.DEF-!!!\n");
  else
    printf("%s\n",it_str[wobj->obj_flags.type_flag]);
  for(wint=1;wint<=it_max;wint=wint+2)
  {
    if(wint+1<=it_max)
      printf("%.2d.) %-34.34s %.2d.) %-34.34s\n",wint,it_str[wint],wint+1,it_str[wint+1]);
    else
      printf("%.2d.) %s\n",wint,it_str[wint]);
  }
  wint=it_max;
  printf("Please enter Number of type (%s):",it_str[wobj->obj_flags.type_flag]);
  gets(wstr);
  wint=atoi(wstr);
  if((wint==0)||(wint<1)||(wint>it_max))
  {
    printf("Type unchanged.\n");
  }
  else
  {
    ie_changed=1;
    wobj->obj_flags.type_flag=wint;
  }
}


void  ie_EditExtras(struct obj_data *wobj)
{
  int wint,wint2;
  unsigned long wlong;
  char wstr[IE_MAXSTRLEN];

  for(wint=1;wint<=iex_max;wint=wint+2)
  {
    if(wint+1<=iex_max)
      printf("%.2d.) %-34.34s %.2d.) %-34.34s\n",wint,iex_str[wint],wint+1,iex_str[wint+1]);
    else
      printf("%.2d.) %s\n",wint,iex_str[wint]);
  }
  for(;;)
  {
    printf("\nOld Extras:");
    PrintExtras(wobj->obj_flags.extra_flags);
    printf("Enter 0 to quit or the Number to toggle :");
    gets(wstr);
    wint=atoi(wstr);
    if((wint!=0)&&((wint<1)||(wint>iex_max)))
    {
      printf("Ilegal Input. Try again.\n");
    }
    else
    if(wint==0) break;
    else
    {
      wlong=1;
      for(wint2=1;wint2<wint;wint2++)
      {
	wlong=wlong<<1;
      }
      ie_changed=1;
      wobj->obj_flags.extra_flags=wobj->obj_flags.extra_flags^wlong;
    }
  }
}

void  ie_EditWear(struct obj_data *wobj)
{
  int wint,wint2;
  unsigned long wsint;
  char wstr[IE_MAXSTRLEN];

  for(wint=1;wint<=iw_max;wint=wint+2)
  {
    if(wint+1<=iw_max)
      printf("%.2d.) %-34.34s %.2d.) %-34.34s\n",wint,iw_str[wint],wint+1,iw_str[wint+1]);
    else
      printf("%.2d.) %s\n",wint,iw_str[wint]);
  }
  for(;;)
  {
    printf("\nOld Wears:");
    PrintWears(wobj->obj_flags.wear_flags);
    printf("Enter 0 to quit or the Number to toggle :");
    gets(wstr);
    wint=atoi(wstr);
    if((wint!=0)&&((wint<1)||(wint>iw_max)))
    {
      printf("Ilegal Input. Try again.\n");
    }
    else
    if(wint==0) break;
    else
    {
      wsint=1;
      for(wint2=1;wint2<wint;wint2++)
      {
	wsint=wsint<<1;
      }
      ie_changed=1;
      wobj->obj_flags.wear_flags=wobj->obj_flags.wear_flags^wsint;
    }
  }
}

void  ie_EditWeight(struct obj_data *wobj)
{
  int wint;
  char wstr[IE_MAXSTRLEN];

  printf("Old weight: %d\n",wobj->obj_flags.weight);
  printf("Enter new weight: ");
  gets(wstr);
  wint=atoi(wstr);
  if((wint==0)&&(*wstr!='0'))
  {
    printf("Weight unchanged.");
    return;
  }
  ie_changed=1;
  wobj->obj_flags.weight=wint;
}

void  ie_EditCost(struct obj_data *wobj)
{
  int wint;
  long wint2;
  char wstr[IE_MAXSTRLEN];

  printf("Old cost: %lu\n",wobj->obj_flags.cost);
  printf("Enter new cost: ");
  gets(wstr);
  wint2=atol(wstr);
  if((wint2==0)&&(*wstr!='0'))
  {
    printf("Cost unchanged.");
    return;
  }
  ie_changed=1;
  wobj->obj_flags.cost=wint2;
}

void  ie_EditCPD(struct obj_data *wobj)
{
  int wint;
  long wint2;
  char wstr[IE_MAXSTRLEN];

  printf("Old cost per day: %lu\n",wobj->obj_flags.cost_per_day);
  printf("Enter new cost per day: ");
  gets(wstr);
  wint2=atol(wstr);
  if((wint2==0)&&(*wstr!='0'))
  {
    printf("Cost per day unchanged.");
    return;
  }
  ie_changed=1;
  wobj->obj_flags.cost_per_day=wint2;
}

void  ie_EditTimer(struct obj_data *wobj)
{
  int wint;
  char wstr[IE_MAXSTRLEN];

  printf("Old timer: %d\n",wobj->obj_flags.timer);
  printf("Enter new timer: ");
  gets(wstr);
  wint=atoi(wstr);
  if((wint==0)&&(*wstr!='0'))
  {
    printf("Timer unchanged.");
    return;
  }
  ie_changed=1;
  wobj->obj_flags.timer=wint;
}

void  ie_EditValues(struct obj_data *wobj)
{
  int wint,wint2,wint3;
  unsigned int wsint;
  int type;
  char wstr[IE_MAXSTRLEN];
  struct itw_entry *itw_e1;
  struct is_entry *is_e1;
  struct is_entry *is_e2;

  type=wobj->obj_flags.type_flag;
  for(wint=0;wint<=3;wint++)
  {
    if(itv_entries[type].str[wint]!=itv_notused)
    {
      if(itv_entries[type].special[wint]==0)
      {
	printf("%s (%d):",itv_entries[type].str[wint],wobj->obj_flags.value[wint]);
	gets(wstr);
	wint2=atoi(wstr);
	if((wint2==0)&&(*wstr!='0'))
	  printf("Value unchanged.\n");
	else
	{
	  ie_changed=1;
	  wobj->obj_flags.value[wint]=wint2;
	}
      }
      else
      {
	switch(itv_entries[type].special[wint])
	{
	  case 1 : {
		     is_e1=is_first;
		     is_e2=NULL;
		     wint2=1;
		     while(is_e1!=NULL)
		     {
		       switch(type)
		       {
			 case 2: { wint3 =is_e1->scroll; break; }
			 case 3: { wint3 =is_e1->wand; break; }
			 case 4: { wint3 =is_e1->staff; break; }
			 case 10:{ wint3 =is_e1->potion; break; }
			 default:{
				   wint3=0;
				   printf("** ERROR IN -ITVALS.DEF-. PLEASE REPORT OR CORRECT!!!\n");
				   break;
				 }
		       }


		       if(wint3==1)
		       {
			 printf("%.2d.) %-34.34s",wint2,is_e1->str);
			 if((wint2%2)==0)
			   printf("\n");
			 else
			   printf(" ");
			 is_valid[wint2-1]=is_e1->nr;
			 if(is_e1->nr==wobj->obj_flags.value[wint])
			   is_e2=is_e1;
			 wint2++;
		       }
		       is_e1=is_e1->next;
		     }
		     if((wint2%2)==0)
		       printf("\n");

		     printf("%s ",itv_entries[type].str[wint]);
		     if(is_e2!=NULL)
		       printf("(%s) ",is_e2->str);
		     else
		       printf("(UNKNOWN OR ILLEGAL) ");
		     gets(wstr);
		     wint3=atoi(wstr);
		     if((wint3>0)&&(wint3<wint2))
		     {
		       ie_changed=1;
		       wobj->obj_flags.value[wint]=is_valid[wint3-1];
		     }
		     else
		       printf("Spell-Type unchanged.\n");
		     break;
		   }
	  case 2 : {
		     wint2=1;
		     itw_e1=itw_first;
		     while(itw_e1!=NULL)
		     {
		       printf("%.2d.) %s\n",wint2,itw_e1->str);
		       itw_e1=itw_e1->next;
		       wint2++;
		     }

		     printf("%s ",itv_entries[type].str[wint]);
		     itw_e1=itw_first;
		     while(itw_e1!=NULL)
		     {
		       if(itw_e1->nr==wobj->obj_flags.value[wint])
		       {
			 printf("(%s):",itw_e1->str);
			 break;
		       }
		       itw_e1=itw_e1->next;
		     }
		     if(itw_e1==NULL)
		     {
		       printf("(invalid):");
		     }
		     gets(wstr);
		     wint3=atoi(wstr);
		     if((wint3>0)&&(wint3<wint2))
		     {
		       itw_e1=itw_first;
		       for(wint2=1;wint2<wint3;wint2++)
			 itw_e1=itw_e1->next;
		       ie_changed=1;
		       wobj->obj_flags.value[wint]=itw_e1->nr;
		     }
		     else
		       printf("Weapon-Type unchanged.\n");
		     break;
		   }
	  case 3 : {
		     for(wint2=1;wint2<=ict_max;wint2=wint2+2)
		     {
		       if(wint2+1<=ict_max)
			 printf("%.2d.) %-34.34s %.2d.) %-34.34s\n",wint2,ict_str[wint2],wint2+1,ict_str[wint2+1]);
		       else
			 printf("%.2d.) %s\n",wint2,ict_str[wint2]);
		     }
		     for(;;)
		     {
		       printf("%s (",itv_entries[type].str[wint]);
		       PrintContainer(wobj->obj_flags.value[wint]);
		       printf(").\n");
		       printf("Enter 0 to quit or the Number to toggle :");
		       gets(wstr);
		       wint2=atoi(wstr);
		       if((wint2!=0)&&((wint2<1)||(wint2>ict_max)))
		       {
			 printf("Ilegal Input. Try again.\n");
		       }
		       else
		       if(wint2==0) break;
		       else
		       {
			 wsint=1;
			 for(wint3=1;wint3<wint2;wint3++)
			 {
			   wsint=wsint<<1;
			 }
			 ie_changed=1;
			 wobj->obj_flags.value[wint]=wobj->obj_flags.value[wint]^wsint;
		       }
		     }
		     break;
		   }
	  case 4 : {
		     for(wint2=1;wint2<=idl_max;wint2=wint2+2)
		     {
		       if(wint2+1<=idl_max)
			 printf("%.2d.) %-34.34s %.2d.) %-34.34s\n",wint2,idl_str[wint2],wint2+1,idl_str[wint2+1]);
		       else
			 printf("%.2d.) %s\n",wint2,idl_str[wint2]);
		     }
		     wint2=idl_max;
		     printf("%s (%s): ",itv_entries[type].str[wint],idl_str[wobj->obj_flags.value[wint]+1]);
		     gets(wstr);
		     wint2=atoi(wstr);
		     if((wint2==0)||(wint2<1)||(wint2>idl_max))
		     {
		       printf("Liquid-Type unchanged.\n");
		     }
		     else
		     {
		       ie_changed=1;
		       wobj->obj_flags.value[wint]=wint2-1;
		     }
		     break;
		   }
	  default: {
		     printf("** UNKNOWN VALUES-SPECIAL!! PLEASE REPORT OR CORRECT YOUR -ITVALS.DEF-!\n");
		     break;
		   }
	}
      }
    }
  }
}



void  ie_EditApply(struct obj_data *wobj)
{
  int wint,wint2;
  char wstr[IE_MAXSTRLEN];

  for(wint=1;wint<=ia_max;wint=wint+2)
  {
    if(wint+1<=ia_max)
      printf("%.2d.) %-34.34s %.2d.) %-34.34s\n",wint,ia_str[wint],wint+1,ia_str[wint+1]);
    else
      printf("%.2d.) %s\n",wint,ia_str[wint]);
  }
  printf("0.) None\n");
  for(wint2=0;wint2<MAX_OBJ_AFFECT;wint2++)
  {
    printf("Which %d. apply (%s): ",wint2+1,ia_str[wobj->affected[wint2].location]);
    gets(wstr);
    wint=atoi(wstr);
    if((wint<0)||(wint>ia_max)||((wint==0)&&(*wstr!='0')))
    {
      printf("Apply unchanged.\n");
      wint=wobj->affected[wint2].location;
    }
    if(wint==0)
    {
      ie_changed=1;
      wobj->affected[wint2].location=0;
      wobj->affected[wint2].modifier=0;
    }
    else
    {
      if(wint!=wobj->affected[wint2].location)
	ie_changed=1;
      wobj->affected[wint2].location=wint;
      printf("Which modifier (%d): ",wobj->affected[wint2].modifier);
      gets(wstr);
      wint=atoi(wstr);
      if(wint!=0)
      {
	ie_changed=1;
	wobj->affected[wint2].modifier=wint;
      }
      else
	printf("Modifier unchanged.\n");
    }
  }
}

void  ie_EditAffects(struct obj_data *wobj)
{
  int wint,wint2;
  long wlong;
  char wstr[IE_MAXSTRLEN];

  for(wint=1;wint<=ipa_max;wint=wint+2)
  {
    if(wint+1<=ipa_max)
      printf("%.2d.) %-34.34s %.2d.) %-34.34s\n",wint,ipa_str[wint],wint+1,ipa_str[wint+1]);
    else
      printf("%.2d.) %s\n",wint,ipa_str[wint]);
  }
  for(;;)
  {
    printf("\nOld Affects:");
    PrintAffects(wobj->obj_flags.bitvector);
    printf("Enter 0 to quit or the Number to toggle :");
    gets(wstr);
    wint=atoi(wstr);
    if((wint!=0)&&((wint<1)||(wint>ipa_max)))
    {
      printf("Illegal Input. Try again.\n");
    }
    else
    if(wint==0) break;
    else
    {
      wlong=1;
      for(wint2=1;wint2<wint;wint2++)
      {
	wlong=wlong<<1;
      }
      ie_changed=1;
      wobj->obj_flags.bitvector=wobj->obj_flags.bitvector^wlong;
    }
  }
}

struct extra_descr_data *CreateExtraDesc(wobj,wext)
  struct obj_data *wobj;
  struct extra_descr_data *wext;
{
  struct extra_descr_data *new_descr;

  ie_changed=1;
  CREATE(new_descr, struct extra_descr_data, 1);

  CREATE(new_descr->keyword, char , 6);
  CREATE(new_descr->description, char , 44);
  strcpy(new_descr->keyword,"dummy");
  strcpy(new_descr->description,"This is a Dummy, please make me valid!!");

  new_descr->next = NULL;
  if(wext==NULL)
    wobj->ex_description=new_descr;
  else
    wext->next=new_descr;
  return(new_descr);
}

void  ie_EditExtraDesc(struct obj_data *wobj)
{
  struct extra_descr_data *wext=NULL;
  struct extra_descr_data *wext2=NULL;
  struct extra_descr_data *wext3=NULL;
/*  struct extra_descr_data *CreateExtraDesc(struct obj_data *,struct extra_descr_data *);*/
  struct extra_descr_data *CreateExtraDesc();
  char wstr[IE_MAXSTRLEN];
  char *wstr2;
  int wint;
  int maxsel=3;

  if(wobj->ex_description==NULL)
  {
    wext=CreateExtraDesc(wobj,wext);
  }
  {
    wext=wobj->ex_description;

    do
    {
      printf("1.) Keywords    : %s\n",wext->keyword);
      printf("2.) Description : %s\n",wext->description);
      printf("3.) Next        ");
      if(wext->next==NULL)
	printf(": <NOT SET>\n");
      else
	printf("-> Set   (Not viewed)\n");
      printf("5.) Delete this entry.\n");
      printf("Enter: 1-2 to edit, 3 to goto/create next, 0 to quit and 5 to delete.\n",maxsel-1+'A');
      gets(wstr);
      wint=atoi(wstr);
      switch(wint)
      {
	case 1: {
		  printf("Old keywords: %s\n",wext->keyword);
		  printf("Enter new keywords :");
		  gets(wstr);
		  if(*wstr=='\0')
		  {
		    printf("Keywords unchanged.\n");
		  }
		  else
		  {
		    free(wext->keyword);
		    CREATE(wext->keyword, char, strlen(wstr) + 1);
		    strcpy(wext->keyword,wstr);
		    ie_changed=1;
		  }
		  break;
		}
	case 2: {
		  printf("Old Description:\n%s\n",wext->description);
		  printf("Enter New Description (End with ~):\n");
		  wstr2=read_multi_string();
		  if(*wstr2=='\0')
		  {
		    printf("Description unchanged.\n");
		  }
		  else
		  {
		    free(wext->description);
		    CREATE(wext->description, char, strlen(wstr2) + 1);
		    strcpy(wext->description,wstr2);
		    ie_changed=1;
		  }
		  free(wstr2);
		  break;
		}
	case 3: {
		  wext2=wext;
		  if(wext->next!=NULL)
		    wext=wext->next;
		  else
		    wext=CreateExtraDesc(wobj,wext);
		  break;
		}
	case 5: {
		  printf("Are you sure (yes/NO)? ");
		  if(AskYesNo("yes"))
		  {
		    ie_changed=1;
		    if(wext2==NULL)
		      wobj->ex_description=wext->next;
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
	defaut: {
		  printf("Illegal input, try again.");
		  break;
		}
      }

    }
    while(wint!=0);
  }
}




/* Print all item Datas */

void print_obj(struct obj_data *wobj)
{
  int i;

  printf("\n");
  printf("Item-Values:\n");
  printf("------------\n");
/*  printf("INTERNAL Item-Nr       : %d\n",wobj->item_number); */
  printf("1.)  Item-Nr       : %d\n",wobj->virt_number);
  printf("2.)  Item-NameList : %s\n",wobj->name);
  printf("3.)  Item-ShortDesc: %s\n",wobj->short_description);
  printf("4.)  Item-LongDesc : %s\n",wobj->description);
  printf("5.)  Item-ActDesc  ");
  if(wobj->action_description==NULL)
    printf(": <NOT SET>\n");
  else
  if(wobj->obj_flags.type_flag==ITEM_WEAPON)
    printf(": %s\n",wobj->action_description);
  else
    printf("-> Set   (Not viewed)\n");
  printf("6.)  Item-Type     : ");
  if(wobj->obj_flags.type_flag>it_max)
    printf("*UNKNOWN* - PLEASE REPORT OR UPDATE -ITYPES.DEF-!!!\n");
  else
    printf("%s\n",it_str[wobj->obj_flags.type_flag]);
  printf("7.)  Item-Extras   : ");
  PrintExtras(wobj->obj_flags.extra_flags);
  printf("8.)  Item-Wear     : ");
  PrintWears(wobj->obj_flags.wear_flags);
  printf("9.)  Item-Weigth   : %u\n",wobj->obj_flags.weight);
  printf("10.) Item-Cost     : %lu\n",wobj->obj_flags.cost);
  printf("11.) Item-Cost/Day : %lu\n",wobj->obj_flags.cost_per_day);
  printf("12.) Item-Timer    : %d\n",wobj->obj_flags.timer);
  printf("13.) Values[0..3]  : %d %d %d %d\n",wobj->obj_flags.value[0],wobj->obj_flags.value[1],wobj->obj_flags.value[2],wobj->obj_flags.value[3]);
  printf("14.) Item-Apply    : ");
  for(i=0;i<MAX_OBJ_AFFECT;i++)
  {
    printf("%s (%d)",ia_str[wobj->affected[i].location],wobj->affected[i].modifier);
    if(i!=(MAX_OBJ_AFFECT-1))
      printf(" - ");
    else
      printf("\n");
  }
  printf("15.) Item-Affects  : ");
  PrintAffects(wobj->obj_flags.bitvector);
  printf("16.) Item-ExtraDesc");
  if(wobj->ex_description==NULL)
    printf(": <NOT SET>\n");
  else
    printf("-> Set   (Not viewed)\n");
}

/* Show Item for DikuCli */

void ie_ShowItem(int nr)
{
  struct obj_data *wobj;

  wobj=read_object(nr,REAL);
  print_obj(wobj);
  free_obj(wobj);
}


/* Edit all Item Data */

int EditAllObj(struct obj_data *wobj)
{
  int maxsel;
  char wstr[IE_MAXSTRLEN];
  int wint;

  maxsel=IE_EDITMAX;

  print_obj(wobj);
  do
  {
    printf("Enter: 0 to quit, 1-%d to edit.\n",maxsel);
    gets(wstr);
    wint=atoi(wstr);
    if((wint>0)&&(wint<=IE_EDITMAX))
    {
      (*ie_edit[wint-1])(wobj);
      print_obj(wobj);
    }
    else
    if(wint!=0)
      printf("** Illegal input. Try again!\n");
  }
  while(wint!=0);

  return(IE_SM_SAVE);
}

/* Edit a Item for DikuCli */

void ie_EditItem(int nr)
{
  struct obj_data *wobj;
  int ie_m;

  ie_changed=0;

  wobj=read_object(nr,REAL);
  ie_m=EditAllObj(wobj);
  if((ie_changed!=0)||(ie_m==IE_SM_DELETE))
    SaveToIETemp(nr,ie_m,wobj);
  free_obj(wobj);
}

/* Clone a Item (for DikuCli) */

void ie_CloneItem(int nr1, int nr2)
{
  struct obj_data *wobj;
  if(nr1<0)
    wobj= &ie_defaultitem;
  else
    wobj=read_object(nr1,REAL);

  ie_doclone(wobj,nr2);

  if(nr1>=0)
    free_obj(wobj);
}

/* Purge a Item for DikuCli */

void ie_PurgeItem(int nr)
{
  SaveToIETemp(nr,IE_SM_DELETE,NULL);
}

/* Set FileName of *.obj file */

void ie_SetFile(char *wstr)
{
  strcpy(ie_objfile,wstr);
}

/* Set TempFile */

void ie_SetTemp(char *wstr)
{
  strcpy(ie_tempfile,wstr);
}

/* Set AffBitVec-flag */

void ie_SetAffBitVec(int wint)
{
  ie_affbitvec=wint;
}

/* Set database path */

void ie_SetDBPath(char *wstr)
{
  strcpy(ie_dbpath,wstr);
}

int GetITWDef(void)
{
  int i;
  struct itw_entry *itw_e1;
  struct itw_entry *itw_e2;
  char *wstr;

  itw_first=NULL;

  CREATE(wstr, char, strlen(ie_dbpath)+21);
  strcpy(wstr,ie_dbpath);
  strcat(wstr,IE_ITWFILE);

  if(!(itw_file=fopen(wstr,"r")))
  {
    free(wstr);
    return(IE_ERR_CANTOPEN);
  }

  free(wstr);

  fscanf(itw_file,"%d\n",&i);
  if(i==-1)
    return(IE_ERR_NOERROR);
  CREATE(itw_e1,struct itw_entry,1);
  itw_e1->str=fread_string(itw_file);
  itw_e1->nr=i;
  itw_e1->next=NULL;
  itw_first=itw_e1;
  itw_e2=itw_e1;
  fscanf(itw_file,"%d\n",&i);
  while(i!=-1)
  {
    CREATE(itw_e1,struct itw_entry,1);
    itw_e1->str=fread_string(itw_file);
    itw_e1->nr=i;
    itw_e1->next=NULL;
    itw_e2->next=itw_e1;
    itw_e2=itw_e1;
    fscanf(itw_file,"%d\n",&i);
  }
  fclose(itw_file);
  return(IE_ERR_NOERROR);
}

void FreeITWDef(void)
{
  int i;
  struct itw_entry *itw_e1;
  struct itw_entry *itw_e2;
  itw_e1=itw_first;
  while(itw_e1!=NULL)
  {
    itw_e2=itw_e1->next;
    free(itw_e1->str);
    free(itw_e1);
    itw_e1=itw_e2;
  }
}

int GetITVDef(void)
{
  int i,j;
  int type,index,spec;
  char *wstr;

  CREATE(wstr, char, strlen(ie_dbpath)+21);
  strcpy(wstr,ie_dbpath);
  strcat(wstr,IE_ITVFILE);

  if(!(itv_file=fopen(wstr,"r")))
  {
    free(wstr);
    return(IE_ERR_CANTOPEN);
  }

  free(wstr);

  itv_entries=malloc(sizeof(struct itv_entry)*(it_max+1));
  for(i=0;i<it_max;i++)
  {

    for(j=0;j<=3;j++)
    {
      itv_entries[i].special[j]=0;
      itv_entries[i].str[j]=itv_notused;
    }
  }

  fscanf(itv_file,"%d ",&type);
  while(type!=-1)
  {
    fscanf(itv_file,"%d ",&index);
    fscanf(itv_file,"%d \n",&spec);
    itv_entries[type].str[index]=fread_string(itv_file);
    itv_entries[type].special[index]=spec;
    fscanf(itv_file,"%d ",&type);
  }
  fclose(itv_file);
  return(IE_ERR_NOERROR);
}

void FreeITVDef(void)
{
  int i,j;
  for(i=0;i<it_max;i++)
  {
    for(j=0;j<=3;j++)
    {
      if(itv_entries[i].str[j]!=itv_notused)
	free(itv_entries[i].str[j]);
    }
  }
}

int GetISDef(void)
{
  int i,j;
  struct is_entry *is_e1;
  struct is_entry *is_e2;
  char *wstr;

  is_first=NULL;
  is_e2=NULL;
  CREATE(wstr, char, strlen(ie_dbpath)+21);
  strcpy(wstr,ie_dbpath);
  strcat(wstr,IE_ISFILE);

  if(!(is_file=fopen(wstr,"r")))
  {
    free(wstr);
    return(IE_ERR_CANTOPEN);
  }

  free(wstr);

  for(;;)
  {
    if(fscanf(is_file,"%d ",&i)!=1) break;
    CREATE(is_e1,struct is_entry,1);
    if(is_first==NULL)
      is_first=is_e1;
    else
      is_e2->next=is_e1;
      is_e1->nr=i;
    fscanf(is_file,"%d ",&is_e1->potion);
    fscanf(is_file,"%d ",&is_e1->scroll);
    fscanf(is_file,"%d ",&is_e1->wand);
    fscanf(is_file,"%d ",&is_e1->staff);
    fscanf(is_file,"%d\n",&is_e1->minlevel);
    is_e1->str=fread_string(is_file);
    is_e2=is_e1;
  }
  fclose(is_file);
  return(IE_ERR_NOERROR);
}

void FreeISDef(void)
{
  int i,j;
  struct is_entry *is_e1;
  struct is_entry *is_e2;

  is_e1=is_first;
  while(is_e1!=NULL)
  {
    free(is_e1->str);
    is_e2=is_e1->next;
    free(is_e1);
    is_e1=is_e2;
  }
  is_first=NULL;
}


/* Initialize the IEdit vars */

void ie_InitIEdit()
{
  struct obj_data *obj;

  ie_changed=0;
  if(GetADef(it_file,IE_ITFILE,ie_dbpath,&it_str,&it_max))
    printf("*** Can't open a *.def file!\n");
  if(GetADef(iw_file,IE_IWFILE,ie_dbpath,&iw_str,&iw_max))
    printf("*** Can't open a *.def file!\n");
  if(GetADef(iex_file,IE_IEXFILE,ie_dbpath,&iex_str,&iex_max))
    printf("*** Can't open a *.def file!\n");
  if(GetADef(ipa_file,IE_IPAFILE,ie_dbpath,&ipa_str,&ipa_max))
    printf("*** Can't open a *.def file!\n");
  if(GetADef(ia_file,IE_IAFILE,ie_dbpath,&ia_str,&ia_max))
    printf("*** Can't open a *.def file!\n");
  if(GetADef(idl_file,IE_IDLFILE,ie_dbpath,&idl_str,&idl_max))
    printf("*** Can't open a *.def file!\n");
  if(GetADef(ict_file,IE_ICTFILE,ie_dbpath,&ict_str,&ict_max))
    printf("*** Can't open a *.def file!\n");
  if(GetITWDef())
    printf("*** Can't open a *.def file!\n");
  if(GetITVDef())
    printf("*** Can't open a *.def file!\n");
  if(GetISDef())
    printf("*** Can't open a *.def file!\n");
  if(OpenObjFile())
    printf("***** IMPOSSIBLE ERROR *****");
}


void ie_CloseIEdit(void)
{
  fclose(obj_f);
  FreeADef(it_str,it_max);
  FreeADef(iex_str,iex_max);
  FreeADef(iw_str,iw_max);
  FreeADef(ipa_str,ipa_max);
  FreeADef(ia_str,ia_max);
  FreeADef(idl_str,idl_max);
  FreeADef(ict_str,ict_max);
  FreeITWDef();
  FreeITVDef();
  FreeISDef();
}
