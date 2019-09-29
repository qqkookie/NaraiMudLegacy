/*****************************************************************************
 *                                                                            *
 * Project : Diku-Mud-World-Creator                                           *
 *                                                                            *
 * Program : Diku-Mud-Mobile-Editor                                           *
 * Version : 1.1                                                              *
 * Last changed on: 27.7.93                                                   *
 *                                                                            *
 * Code by: Georg Essl (essl@fstgds06.tu-graz.ac.at)                          *
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
#include "medit.h"

/****** DEFINES *****/

/* Mobile-Edit */
#define ME_MAXSTRLEN 128

#define ME_MOBFILE "tmob.mob"
#define ME_TEMPFILE "metemp.mob"

#define ME_MSFILE  "msex.def"
#define ME_MPFILE  "mpos.def"
#define ME_MCFILE  "mclass.def"
#define ME_MAFILE  "mactions.def"

#define ME_SM_DELETE 1
#define ME_SM_SAVE   2
#define ME_SM_INSERT 3

#define ME_M_QUIT   0

#define ME_FLAGS_NONE     0
#define ME_FLAGS_SIMPLE    1

#define ME_EDITMAX 18

#define REAL 0
#define VIRTUAL 1

#define MF_EOF    "$~"

#define MAX_SKILLS  100    /* Used in CHAR_FILE_U *DO*NOT*CHANGE* */
#define MAX_AFFECT  32    /* Used in CHAR_FILE_U *DO*NOT*CHANGE* */
#define MAX_WEAR    18

/***** ERROR-DEFINES *****/

#define ME_ERR_NOERROR      0

/***** TYPEDEFS *****/

/*typedef char bool;
  typedef char byte;*/

typedef   signed char  sbyte;
typedef unsigned char  ubyte;
#if !defined(sgi) && !defined(m88k) &&!defined(_AIX)
// typedef unsigned long  ulong;
#endif
typedef   signed short sh_int;

/***** Structs *****/

struct  ability_data
{
	sbyte  str;
	sbyte  str_add;
	sbyte  intel;
	sbyte  wis;
	sbyte  dex;
	sbyte  con;
};

struct mob_data
{
	int    mob_number;
	int    virt_number;

	char   *name;
	char   *short_descr;               /* NPC action name             */
	char   *long_descr;                /* NPC default pos. descrip.   */
	char   *description;               /* what you see when 'look at' */
	sh_int armor;                      /* armor class                 */
	long   hitnodice;
	long   hitsizedice;
	long   hitadd;                    /* hit roll modifier           */
	long   damnodice;                  /* number of damage dice (NPC) */
	long   damsizedice;                /*   size of damage dice (NPC) */
	long   damadd;                    /* damage roll modifier        */
	ubyte  position;                   /* standing or ...             */
	ubyte  default_pos;                /* default position for NPC    */
	ubyte  carry_items;                /* number of items carried     */
	sh_int carry_weight;               /* carried weight              */
	ulong  affected_by;                /* bitvector for spell affects */
	int    apply_saving_throw[5];

	ubyte  sex;
	ubyte  class;
	ubyte  level;
	sh_int weight;
	sh_int height;
	ulong  flags;          /* was ->specials.act      */
	sbyte  condition[3];   /* full/thirsty/drunk      */

	sh_int  mana;
	sh_int  max_mana;
	sh_int  hit;
	sh_int  max_hit;
	sh_int  move;
	sh_int  max_move;
	sh_int  alignment;
	long    gold;

	int	thac0;
	long    exp;

	int	medit_flags;

	struct ability_data abilities;
};

/***** Externs *****/

extern void PrintAffects(unsigned long);

extern int ipa_max;
extern char **ipa_str;

/***** Global Vars *****/

char me_mobfile[ME_MAXSTRLEN]=ME_MOBFILE;
char me_tempfile[ME_MAXSTRLEN]=ME_TEMPFILE;

char me_dbpath[ME_MAXSTRLEN]="";

int ms_max;
int mp_max;
int mc_max;
int ma_max;

int me_changed;

char **ms_str;
char **mp_str;
char **mc_str;
char **ma_str;

FILE *me_workfile;
FILE *ms_file;
FILE *mp_file;
FILE *mc_file;
FILE *ma_file;

FILE *me_temp;

FILE *mob_f;

struct mob_data me_defaultmob =
{
    0,
    0,
    "noname monster",
    "Noname Monster",
    "The Noname Monster is waiting here for nothing.",
    "You see a dummy Monster.",
    0,
    1,1,1,
    1,1,1,
    8,8,
    0,
    0,
    0,
    0,0,0,0,0,
    0,
    1,
    1,
    1,
    1,
    0,
    -1, -1, -1,
    0,0,
    20,20,
    80,80,
    0,
    15,
    1,
    20,
    1,
    10,0,10,10,10,10
};

/***** Vars from db.c from original DikuMud! *****/

struct index_data *mob_index;         /* index table for mobile file     */
int top_of_mobt = 0;                  /* top of mobile index table       */

/*** Edit Protos ***/

void  me_EditNr(struct mob_data *);
void  me_EditNameList(struct mob_data *);
void  me_EditShortDesc(struct mob_data *);
void  me_EditLongDesc(struct mob_data *);
void  me_EditDesc(struct mob_data *);
void  me_EditActions(struct mob_data *);
void  me_EditAffects(struct mob_data *);
void  me_EditAlignment(struct mob_data *);
void  me_EditLevel(struct mob_data *);
void  me_EditThac0(struct mob_data *);
void  me_EditArmor(struct mob_data *);
void  me_EditHitpoints(struct mob_data *);
void  me_EditDamage(struct mob_data *);
void  me_EditGold(struct mob_data *);
void  me_EditExp(struct mob_data *);
void  me_EditPosition(struct mob_data *);
void  me_EditDefPos(struct mob_data *);
void  me_EditSex(struct mob_data *);

/*** A FUNCTION ARRAY ***/

void (*me_edit[ME_EDITMAX+1])(struct mob_data *) =
{
    me_EditNr,
    me_EditNameList,
    me_EditShortDesc,
    me_EditLongDesc,
    me_EditDesc,
    me_EditActions,
    me_EditAffects,
    me_EditAlignment,
    me_EditLevel,
    me_EditThac0,
    me_EditArmor,
    me_EditHitpoints,
    me_EditDamage,
    me_EditGold,
    me_EditExp,
    me_EditPosition,
    me_EditDefPos,
    me_EditSex
};

/* Free mobile data memory */

void free_mob(struct mob_data *mob)
{
    if(mob->name)
        free(mob->name);
    if(mob->description)
        free(mob->description);
    if(mob->short_descr)
        free(mob->short_descr);
    if(mob->long_descr)
        free(mob->long_descr);

    free(mob);
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

char *me_ReadLDesc(int nr)
{
    char *buf;
    int tmp;
    if((nr=real_mobile(nr))<0)
        return(NULL);

    fseek(mob_f, mob_index[nr].pos, 0);
    fscanf(mob_f,"#%d\n",&tmp);
    buf = fread_string(mob_f);
    free(buf);
    buf = fread_string(mob_f);
    free(buf);
    buf = fread_string(mob_f);
    return(buf);
}

char *me_ReadName(int nr)
{
    char *buf;
    int tmp;
    if((nr=real_mobile(nr))<0)
        return(NULL);

    fseek(mob_f, mob_index[nr].pos, 0);
    fscanf(mob_f,"#%d\n",&tmp);
    buf = fread_string(mob_f);
    return(buf);
}

/* read a mobile from MOB_FILE */
struct mob_data *read_mobile(int nr, int type)
{
	int i, skill_nr;
	long tmp, tmp2, tmp3;
	struct mob_data *mob;
	char chk[10], buf[100];
	char letter;
    char bitflag[255];

	i = nr;
	if (type == VIRTUAL)
		if ((nr = real_mobile(nr)) < 0)
        {
            sprintf(buf, "Mobile (V) %d does not exist in database.", i);
            return(0);
        }

	fseek(mob_f, mob_index[nr].pos, 0);

	CREATE(mob, struct mob_data, 1);

	/***** String data *** */

	fscanf(mob_f, "#%d\n", &tmp);
	mob->virt_number=tmp;
	mob->name = fread_string(mob_f);
	mob->short_descr = fread_string(mob_f);
	mob->long_descr = fread_string(mob_f);
	mob->description = fread_string(mob_f);

	/* *** Numeric data *** */

    fscanf(mob_f, "%s ", bitflag);
    if (isdigit(bitflag[0])) tmp = atol(bitflag);
    else {
        // log(mob->player.name);
        // log(bitflag);
        tmp = flag_convert(bitflag);
    }
	mob->flags = tmp;

    fscanf(mob_f, "%s ", bitflag);
    if (isdigit(bitflag[0])) tmp = atol(bitflag);
    else tmp = flag_convert(bitflag);
	mob->affected_by = tmp;

	fscanf(mob_f, " %ld ", &tmp);
	mob->alignment = tmp;

	fscanf(mob_f, " %c \n", &letter);

	if (letter == 'S') {
		mob->medit_flags=ME_FLAGS_SIMPLE;
		/* The new easy monsters */
		fscanf(mob_f, " %d ", &tmp);
		mob->level = tmp;

		fscanf(mob_f, " %d ", &tmp);
		mob->thac0 = tmp;

		fscanf(mob_f, " %d ", &tmp);
		mob->armor = tmp;

		fscanf(mob_f, " %dd%d+%d ", &tmp, &tmp2, &tmp3);
		mob->hitadd = tmp3;
		mob->hitnodice = tmp;
		mob->hitsizedice = tmp2;

		fscanf(mob_f, " %dd%d+%d \n", &tmp, &tmp2, &tmp3);
		mob->damadd = tmp3;
		mob->damnodice = tmp;
		mob->damsizedice = tmp2;

		fscanf(mob_f, " %lu ", &tmp);
		mob->gold = tmp;

		fscanf(mob_f, " %ld \n", &tmp);
		mob->exp = tmp;

		fscanf(mob_f, " %d ", &tmp);
		mob->position = tmp;

		fscanf(mob_f, " %d ", &tmp);
		mob->default_pos = tmp;

		fscanf(mob_f, " %d \n", &tmp);
		mob->sex = tmp;

	} else {  /* The old monsters are down below here */

		mob->medit_flags=0;
		fscanf(mob_f, " %d ", &tmp);
		mob->abilities.str = tmp;

		fscanf(mob_f, " %d ", &tmp);
		mob->abilities.intel = tmp;

		fscanf(mob_f, " %d ", &tmp);
		mob->abilities.wis = tmp;

		fscanf(mob_f, " %d ", &tmp);
		mob->abilities.dex = tmp;

		fscanf(mob_f, " %d \n", &tmp);
		mob->abilities.con = tmp;

		fscanf(mob_f, " %d ", &tmp);
		fscanf(mob_f, " %d ", &tmp2);

		mob->hitadd = tmp-1;
		mob->hitnodice = 1;
		mob->hitsizedice = tmp2 - tmp + 1;

		fscanf(mob_f, " %d ", &tmp);
		mob->armor = tmp;

		fscanf(mob_f, " %d ", &tmp);
		mob->mana = tmp;
		mob->max_mana = tmp;

		fscanf(mob_f, " %d ", &tmp);
		mob->move = tmp;
		mob->max_move = tmp;

		fscanf(mob_f, " %d ", &tmp);
		mob->gold = tmp;

		fscanf(mob_f, " %d \n", &tmp);
		mob->exp = tmp;

		fscanf(mob_f, " %d ", &tmp);
		mob->position = tmp;

		fscanf(mob_f, " %d ", &tmp);
		mob->default_pos = tmp;

		fscanf(mob_f, " %d ", &tmp);
		mob->sex = tmp;

		fscanf(mob_f, " %d ", &tmp);
		mob->class = tmp;

		fscanf(mob_f, " %d ", &tmp);
		mob->level = tmp;

		fscanf(mob_f, " %d ", &tmp);  /*** IGNORE AGE (orig Diku) ***/

		fscanf(mob_f, " %d ", &tmp);
		mob->weight = tmp;

		fscanf(mob_f, " %d \n", &tmp);
		mob->height = tmp;

		for (i = 0; i < 3; i++)
		{
			fscanf(mob_f, " %d ", &tmp);
			mob->condition[i] = tmp;
		}
		fscanf(mob_f, " \n ");

		for (i = 0; i < 5; i++)
		{
			fscanf(mob_f, " %d ", &tmp);
			mob->apply_saving_throw[i] = tmp;
		}

		fscanf(mob_f, " \n ");
	}

	mob->mob_number = nr;

	return(mob);
}

/* Open the mobile-file defined in me_mobfile */

int OpenMobFile(void)
{
    struct mob_data *wmob;

	if (!(mob_f = fopen(me_mobfile, "r")))
	{
        printf("*** Can't open %s-file!\n",me_mobfile);
        printf("Want to create a new one (YES/no)? ");
        if(AskYesNo("no"))
            exit(0);
        else
        {
            if(!(mob_f = fopen(me_mobfile,"w")))
            {
                printf("*** Can't create %s-file!\n",me_mobfile);
                exit(0);
            }
            else
            {
                fprintf(mob_f,"#0\n");
                wmob= &me_defaultmob;
                fwrite_string(mob_f,wmob->name);
                fwrite_string(mob_f,wmob->short_descr);
                fwrite_string(mob_f,wmob->long_descr);
                fwrite_string(mob_f,wmob->description);

                fprintf(mob_f,"%d %d %d S\n",wmob->flags,wmob->affected_by,wmob->alignment);
                fprintf(mob_f,"%d %d %d %ldd%ld+%ld %ldd%ld+%ld\n",wmob->level,wmob->thac0,wmob->armor,wmob->hitnodice,wmob->hitsizedice,wmob->hitadd,
                        wmob->damnodice,wmob->damsizedice,wmob->damadd);

                fprintf(mob_f,"%lu %u\n",wmob->gold,wmob->exp);
                fprintf(mob_f,"%d %d %d\n",wmob->position,wmob->default_pos,wmob->sex);
                fprintf(mob_f,"#19999\n");
                fprintf(mob_f,MF_EOF);
                fprintf(mob_f,"\n");
                fclose(mob_f);
                if (!(mob_f = fopen(me_mobfile, "r")))
                {
                    printf("*** Can't reopen %s-file!\n",me_mobfile);
                    exit(0);
                }
            }
        }
	}
	mob_index = generate_indices(mob_f, &top_of_mobt);
	return(ME_ERR_NOERROR);
}


/* Find a Mobile in Database */

int me_FindNrRealMobile(int nr,char *wstr)
{
    int tmp;
    char *tname1;
    char *tname2;
    char tname[ME_MAXSTRLEN];

    int i,count;
    char *name;

    count=0;
    for (i=0; i<=top_of_mobt;i++)
    {
        fseek(mob_f, mob_index[i].pos, 0);

        fscanf(mob_f, "#%d\n", &tmp);
        name  = fread_string(mob_f);
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

/* Find a Mobile in Database */

char *me_FindMobile(int nr,char *wstr)
{
    int tmp;
    char *name;
    char *tname1;
    char *tname2;
    char tname[ME_MAXSTRLEN];

    fseek(mob_f, mob_index[nr].pos, 0);

    fscanf(mob_f, "#%d\n", &tmp);
    name  = fread_string(mob_f);
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

void me_FindMobiles(char *wstr)
{
    int i;
    char *name;

    printf("Mob-Nr  Name\n");
    printf("-------------------------\n");

    for (i=0; i<=top_of_mobt;i++)
    {
        if((name=me_FindMobile(i,wstr))!=NULL)
        {
            printf("[%4.4d] %s\n",mob_index[i].virtual,name);
            free(name);
        }
    }
}

int me_FindNrMobile(int nr, char *wstr)
{
    int i,count;
    char *name;

    count=0;
    for (i=0; i<=top_of_mobt;i++)
    {
        if((name=me_FindMobile(i,wstr))!=NULL)
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


/* Save me_tempfile with changed mobile and copy to me_mobfile */
int SaveToMETemp(int nr, int mode,struct mob_data *wmob)
{
    struct mob_data *mob;
    int tmp, i;
    long tmp2, j,j2,j3,mob_i;
    long store_pos,store_cpos;
    char chk[50], buf[100];
    char *temp_buff;

    switch(mode)
    {
    case ME_SM_INSERT: {
        printf("Do you really want to CREATE/CLONE the mobile (YES/no)? ");
        if(AskYesNo("no"))
        {
            printf("Not Done.\n");
            return(0);
        }
        break;
    }
    case ME_SM_SAVE: {
        printf("Do you really want to SAVE the changes (YES/no)? ");
        if(AskYesNo("no"))
        {
            printf("Not Done.\n");
            return(0);
        }
        break;
    }
    case ME_SM_DELETE: {
        printf("Do you really want to DELETE the mobile (yes/NO)? ");
        if(!AskYesNo("yes"))
        {
            printf("Not Done.\n");
            return(0);
        }
        break;
    }
    }


    if(mode==ME_SM_INSERT)
    {
        if (!(mob_index =
              (struct index_data*) realloc(mob_index,
                                           (top_of_mobt + 3) * sizeof(struct index_data))))
        {
            perror("load indices");
            exit(0);
        }
        top_of_mobt++;

        for(i=top_of_mobt+1;i>nr;i--)
        {
            mob_index[i].virtual=mob_index[i-1].virtual;
            mob_index[i].pos=mob_index[i-1].pos;
            mob_index[i].cpos=mob_index[i-1].cpos;
        }
        mob_index[nr].virtual=wmob->virt_number;
    }




    rewind(mob_f);

    if (!(me_temp = fopen(me_tempfile, "w")))
    {
        printf("Can't open temporary file!!!\n");
        exit(0);
        /*** !!! ***/
    }
    else
    {
        if(mob_index[nr].pos!=0)
        {
            mob_i = mob_index[nr].cpos;
            j = mob_i;

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
            while(j2!=mob_i)
            {
                if((j2+j)>mob_i)
                    j=mob_i-j2;
                j3=fread(temp_buff,sizeof(char),j,mob_f);
                fwrite(temp_buff,sizeof(char),j3,me_temp);
                j2+=j3;
            }
            free(temp_buff);
        }

        if((mode==ME_SM_SAVE)||(mode==ME_SM_INSERT))
        {
            fprintf(me_temp,"#%d\n",wmob->virt_number);
            fwrite_string(me_temp,wmob->name);
            fwrite_string(me_temp,wmob->short_descr);
            fwrite_string(me_temp,wmob->long_descr);
            fwrite_string(me_temp,wmob->description);

            if(wmob->medit_flags&ME_FLAGS_SIMPLE)
            {
                fprintf(me_temp,"%lu %lu %d S\n",wmob->flags,wmob->affected_by,wmob->alignment);
                fprintf(me_temp,"%d %d %d %ldd%ld+%ld %ldd%ld+%ld\n",wmob->level,wmob->thac0,wmob->armor,wmob->hitnodice,wmob->hitsizedice,wmob->hitadd,
                        wmob->damnodice,wmob->damsizedice,wmob->damadd);

                fprintf(me_temp,"%lu %ld\n",wmob->gold,wmob->exp);
                fprintf(me_temp,"%d %d %d\n",(int)wmob->position,(int)wmob->default_pos,(int)wmob->sex);
            }
            else
            {
                fprintf(me_temp,"%lu %lu %ld X\n",wmob->flags,wmob->affected_by,wmob->alignment);
            }

            store_pos=ftell(me_temp)-mob_index[nr].pos;
        }

        fseek(mob_f, mob_index[nr+1].pos,0);
        fseek(mob_f,0,2);
        tmp2=ftell(mob_f);
        fseek(mob_f, mob_index[nr+1].pos,0);

        j = tmp2-mob_index[nr+1].pos;

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
                j2=fread(temp_buff,sizeof(char),j,mob_f);
                if(j2!=0)
                    fwrite(temp_buff,sizeof(char),j2,me_temp);
            }
            while(j2!=0);

            free(temp_buff);
        }
        fclose(me_temp);
    }

    switch(mode)
    {
    case ME_SM_DELETE :
    {
        j=mob_index[nr+1].pos-mob_index[nr].pos;
        j2=mob_index[nr+1].cpos-mob_index[nr].cpos;
        for(i=nr;i<top_of_mobt+1;i++)
        {
            mob_index[i].virtual=mob_index[i+1].virtual;
            mob_index[i].pos=mob_index[i+1].pos-j;
            mob_index[i].cpos=mob_index[i+1].cpos-j2;
        }
        if (!(mob_index =
              (struct index_data*) realloc(mob_index,
                                           (top_of_mobt+1) * sizeof(struct index_data))))
        {
            perror("load indices");
            exit(0);
        }
        top_of_mobt--;
        break;
    }
    case ME_SM_INSERT :
    {
        if (!(me_temp = fopen(me_tempfile, "r")))
        {
            printf("Can't open temporary file!!!\n");
            exit(0);
            /*** !!! ***/
        }

        fseek(me_temp,mob_index[nr].pos,0);

        j3=0;
        fgets(buf, 81, me_temp);
        j3+=strlen(buf);
        for (;;)
        {
            if (fgets(buf, 81, me_temp))
            {
                if (*buf == '#')
                    break;
                else
                    j3+=strlen(buf);
            }
        }
        fclose(me_temp);

        for(i=nr+1;i<=top_of_mobt+1;i++)
        {
            mob_index[i].pos+=store_pos;
            mob_index[i].cpos+=j3;
        }
        break;
    }
    case ME_SM_SAVE   :
    {
        if (!(me_temp = fopen(me_tempfile, "r")))
        {
            printf("Can't open temporary file!!!\n");
            exit(0);
            /*** !!! ***/
        }

        fseek(me_temp,mob_index[nr].pos,0);


        j3=0;
        fgets(buf, 81, me_temp);
        j3+=strlen(buf);
        for (;;)
        {
            if (fgets(buf, 81, me_temp))
            {
                if (*buf == '#')
                    break;
                else
                    j3+=strlen(buf);
            }
        }
        fclose(me_temp);
        store_cpos=mob_index[nr+1].cpos-mob_index[nr].cpos;
        j2=mob_index[nr+1].pos-mob_index[nr].pos;

        for(i=nr+1;i<=top_of_mobt+1;i++)
        {
            mob_index[i].pos+=store_pos-j2;
            mob_index[i].cpos+=j3-store_cpos;
        }

        break;
    }
    }

    fclose(mob_f);
    if (!(mob_f = fopen(me_mobfile, "w")))
    {
        printf("Can't reopen %s-file for saving!\n",me_mobfile);
        exit(0);
    }
    if (!(me_temp = fopen(me_tempfile, "r")))
    {
        printf("Can't reopen temporary file for copying!");
        exit(0);
    }
    fseek(me_temp,0,2);
    tmp2=ftell(me_temp);
    rewind(me_temp);
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
    j3=fread(temp_buff,sizeof(char),j2,me_temp);
    while(j3!=0)
    {
        fwrite(temp_buff,sizeof(char),j3,mob_f);
        j3=fread(temp_buff,sizeof(char),j2,me_temp);
    }
    if(temp_buff!=NULL)
        free(temp_buff);
    fclose(me_temp);
    fclose(mob_f);

    if (!(mob_f = fopen(me_mobfile, "r")))
    {
        printf("Can't reopen %s-file for reading!\n",me_mobfile);
        exit(0);
    }
    return(0);
}

/* Print Bitfield Actions */

void PrintActions(unsigned long wact)
{
    unsigned long i,j;
    unsigned long shact;
    j=0;
    shact=wact;
    for(i=1;i<=ma_max;i++)
    {
        if((shact&1))
        {
            printf("%s  ",ma_str[i]);
            j=1;
        }
        shact=shact>>1;
    }
    if(!j)
        printf("None\n");
    else
        printf("\n");
}

void me_doclone(struct mob_data *wmob,int wint)
{
    int bot, top, mid, used, i;
    struct mob_data *wmob2;

    bot = 0;
    top = top_of_mobt;

    /* perform binary search on mob-table */
    for (;;)
    {
        mid = (bot + top) / 2;

        if ((mob_index + mid)->virtual == wint)
        {
            printf("Given Mobile-Number is already used! Clone canceled.\n");
            used=1;
            break;
        }
        if (bot >= top)
        {
            if((mob_index + mid)->virtual <wint)
                mid++;
            used=0;
            break;
        }
        if ((mob_index + mid)->virtual > wint)
            top = mid - 1;
        else
            bot = mid + 1;
    }
    if(!used)
    {
        if(wmob == &me_defaultmob)
            printf("Creating Dummy-Mobile at [%d].\n",wint);
        else
            printf("Cloning Mobile from [%d] to [%d].\n",wmob->virt_number,wint);
        CREATE(wmob2, struct mob_data, 1);

        /* The following lines proof the great advantage of obj.o.programming */
        /* and the disadvantage here! */

        wmob2->virt_number=wint;
        wmob2->mob_number=mid;

        wmob2->medit_flags=wmob->medit_flags;

        wmob2->flags=wmob->flags;
        wmob2->affected_by=wmob->affected_by;
        wmob2->alignment=wmob->alignment;

        wmob2->level=wmob->level;
        wmob2->thac0=wmob->thac0;
        wmob2->armor=wmob->armor;

        wmob2->hitnodice=wmob->hitnodice;
        wmob2->hitsizedice=wmob->hitsizedice;
        wmob2->hitadd=wmob->hitadd;

        wmob2->damnodice=wmob->damnodice;
        wmob2->damsizedice=wmob->damsizedice;
        wmob2->damadd=wmob->damadd;

        wmob2->gold=wmob->gold;
        wmob2->exp=wmob->exp;
        wmob2->position=wmob->position;
        wmob2->default_pos=wmob->default_pos;
        wmob2->sex=wmob->sex;

        CREATE(wmob2->name, char ,strlen(wmob->name)+1);
        strcpy(wmob2->name,wmob->name);
        CREATE(wmob2->long_descr, char ,strlen(wmob->long_descr)+1);
        strcpy(wmob2->long_descr,wmob->long_descr);
        CREATE(wmob2->short_descr, char ,strlen(wmob->short_descr)+1);
        strcpy(wmob2->short_descr,wmob->short_descr);
        if(wmob->description!=NULL)
        {
            CREATE(wmob2->description, char ,strlen(wmob->description)+1);
            strcpy(wmob2->description,wmob->description);
        }
        else
            wmob2->description=NULL;

        SaveToMETemp(mid,ME_SM_INSERT,wmob2);
        free_mob(wmob2);
    }
}

/* EDIT THE MOBILE */

void  me_EditNr(struct mob_data *wmob)
{
    char wstr[ME_MAXSTRLEN];
    int wint;

    printf("Current Mobile Number: #%d\n",wmob->virt_number);
    printf("New Number (%d):",wmob->virt_number);
    gets(wstr);
    if(*wstr=='\0')
    {
        printf("Mobile-Nr Unchanged");
        return;
    }
    wint = atoi(wstr);
    if((wint==0)&&(*wstr!='0'))
    {
        printf("** Illegal input\n");
        return;
    }
    if(wint==wmob->virt_number)
    {
        printf("Mobile-Nr Unchanged\n");
        return;
    }
    me_doclone(wmob,wint);
}

void  me_EditNameList(struct mob_data *wmob)
{
    char wstr[ME_MAXSTRLEN];
    printf("Old Name List: %s\n",wmob->name);
    printf("Enter New List :");
    gets(wstr);
    if(*wstr=='\0')
    {
        printf("Description unchanged.\n");
    }
    else
    {
        free(wmob->name);
        CREATE(wmob->name, char, strlen(wstr) + 1);
        strcpy(wmob->name,wstr);
        me_changed=1;
    }
}

void  me_EditShortDesc(struct mob_data *wmob)
{
    char wstr[ME_MAXSTRLEN];
    printf("Old ShortDesc: %s\n",wmob->short_descr);
    printf("Enter New ShortDesc :");
    gets(wstr);
    if(*wstr=='\0')
    {
        printf("Description unchanged.\n");
    }
    else
    {
        free(wmob->short_descr);
        CREATE(wmob->short_descr, char, strlen(wstr) + 1);
        strcpy(wmob->short_descr,wstr);
        me_changed=1;
    }
}

void  me_EditLongDesc(struct mob_data *wmob)
{
    char *wstr;
    printf("Old LongDesc: %s\n",wmob->long_descr);
    printf("Enter New LongDesc (End with ~):\n");
    wstr=read_multi_string();
    if(*wstr=='\0')
    {
        printf("Description unchanged.\n");
    }
    else
    {
        free(wmob->long_descr);
        CREATE(wmob->long_descr, char, strlen(wstr) + 1);
        strcpy(wmob->long_descr,wstr);
        me_changed=1;
    }
    free(wstr);
}

void  me_EditDesc(struct mob_data *wmob)
{
    char *wstr;

    if(wmob->description==NULL)
        printf("Old Description: <NOT SET>\n");
    else
        printf("Old Description:\n%s\n",wmob->description);
    me_changed=1;
    printf("Enter New Description (End with ~):\n");
    wstr=read_multi_string();
    if(wmob->description!=NULL)
        free(wmob->description);
    if(*wstr=='\0')
    {
        wmob->description=NULL;
        printf("Description cleared.\n");
    }
    else
    {
        CREATE(wmob->description, char, strlen(wstr) + 1);
        strcpy(wmob->description,wstr);
    }
    free(wstr);
}

void  me_EditActions(struct mob_data *wmob)
{
    int wint,wint2;
    unsigned long wlong;
    char wstr[ME_MAXSTRLEN];

    for(wint=1;wint<=ma_max;wint=wint+2)
    {
        if(wint+1<=ma_max)
            printf("%.2d.) %-34.34s %.2d.) %-34.34s\n",wint,ma_str[wint],wint+1,ma_str[wint+1]);
        else
            printf("%.2d.) %s\n",wint,ma_str[wint]);
    }
    for(;;)
    {
        printf("\nOld Actions:");
        PrintActions(wmob->flags);
        printf("Enter 0 to quit or the Number to toggle :");
        gets(wstr);
        wint=atoi(wstr);
        if((wint!=0)&&((wint<1)||(wint>ma_max)))
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
                me_changed=1;
                wmob->flags=wmob->flags^wlong;
            }
    }
}

void  me_EditAffects(struct mob_data *wmob)
{
    int wint,wint2;
    long wlong;
    char wstr[ME_MAXSTRLEN];

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
        PrintAffects(wmob->affected_by);
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
                me_changed=1;
                wmob->affected_by=wmob->affected_by^wlong;
            }
    }
}

void  me_EditAlignment(struct mob_data *wmob)
{
    int wint;
    char wstr[ME_MAXSTRLEN];

    printf("Old alignment: %d\n",wmob->alignment);
    printf("Enter new alignment: ");
    gets(wstr);
    wint=atoi(wstr);
    if((wint==0)&&(*wstr!='0'))
    {
        printf("Alignment unchanged.");
        return;
    }
    me_changed=1;
    wmob->alignment=wint;
}

void  me_EditLevel(struct mob_data *wmob)
{
    int wint;
    char wstr[ME_MAXSTRLEN];

    printf("Old level: %d\n",wmob->level);
    printf("Enter new level: ");
    gets(wstr);
    wint=atoi(wstr);
    if(((wint==0)&&(*wstr!='0'))||(wint<0))
    {
        printf("Level unchanged.");
        return;
    }
    me_changed=1;
    wmob->level=wint;
}

void  me_EditThac0(struct mob_data *wmob)
{
    int wint;
    char wstr[ME_MAXSTRLEN];

    printf("Old thac0: %d\n",wmob->thac0);
    printf("Enter new thac0: ");
    gets(wstr);
    wint=atoi(wstr);
    if((wint==0)&&(*wstr!='0'))
    {
        printf("Thac0 unchanged.");
        return;
    }
    me_changed=1;
    wmob->thac0=wint;
}

void  me_EditArmor(struct mob_data *wmob)
{
    int wint;
    char wstr[ME_MAXSTRLEN];

    printf("Old armor: %d\n",wmob->armor);
    printf("Enter new armor: ");
    gets(wstr);
    wint=atoi(wstr);
    if((wint==0)&&(*wstr!='0'))
    {
        printf("Armor unchanged.");
        return;
    }
    me_changed=1;
    wmob->armor=wint;
}

void  me_EditHitpoints(struct mob_data *wmob)
{
    int wint;
    long wint2;
    char wstr[ME_MAXSTRLEN];

    printf("Old Hitpoints: %ldd%ld+%ld\n",wmob->hitnodice,wmob->hitsizedice,wmob->hitadd);
    printf("Enter new number of dices: ");
    gets(wstr);
    wint2=atol(wstr);
    if((wint2==0)&&(*wstr!='0'))
    {
        printf("Number of dices unchanged.\n");
    }
    else
    {
        me_changed=1;
        wmob->hitnodice=wint2;
    }
    printf("Enter new size of dice: ");
    gets(wstr);
    wint2=atol(wstr);
    if((wint2==0)&&(*wstr!='0'))
    {
        printf("Size of dice unchanged.\n");
    }
    else
    {
        me_changed=1;
        wmob->hitsizedice=wint2;
    }
    printf("Enter new hit-add: ");
    gets(wstr);
    wint2=atol(wstr);
    if((wint2==0)&&(*wstr!='0'))
    {
        printf("Hit-add unchanged.");
    }
    else
    {
        me_changed=1;
        wmob->hitadd=wint2;
    }
}

void  me_EditDamage(struct mob_data *wmob)
{
    int wint;
    long wint2;
    char wstr[ME_MAXSTRLEN];

    printf("Old Damage: %ldd%ld+%ld\n",wmob->damnodice,wmob->damsizedice,wmob->damadd);
    printf("Enter new number of dices: ");
    gets(wstr);
    wint2=atol(wstr);
    if((wint2==0)&&(*wstr!='0'))
    {
        printf("Number of dices unchanged.\n");
    }
    else
    {
        me_changed=1;
        wmob->damnodice=wint2;
    }
    printf("Enter new size of dice: ");
    gets(wstr);
    wint2=atol(wstr);
    if((wint2==0)&&(*wstr!='0'))
    {
        printf("Size of dice unchanged.\n");
    }
    else
    {
        me_changed=1;
        wmob->damsizedice=wint2;
    }
    printf("Enter new dam-add: ");
    gets(wstr);
    wint2=atol(wstr);
    if((wint2==0)&&(*wstr!='0'))
    {
        printf("Dam-add unchanged.");
    }
    else
    {
        me_changed=1;
        wmob->damadd=wint2;
    }
}

void  me_EditGold(struct mob_data *wmob)
{
    int wint;
    long wint2;
    char wstr[ME_MAXSTRLEN];

    printf("Old gold: %lu\n",wmob->gold);
    printf("Enter new gold: ");
    gets(wstr);
    wint2=atol(wstr);
    if((wint2==0)&&(*wstr!='0'))
    {
        printf("Gold unchanged.");
        return;
    }
    me_changed=1;
    wmob->gold=wint2;
}

void  me_EditExp(struct mob_data *wmob)
{
    int wint;
    long wint2;
    char wstr[ME_MAXSTRLEN];

    printf("Old ExpPoints: %lu\n",wmob->exp);
    printf("Enter new ExpPoints: ");
    gets(wstr);
    wint2=atol(wstr);
    if((wint2==0)&&(*wstr!='0'))
    {
        printf("ExpPoints unchanged.");
        return;
    }
    me_changed=1;
    wmob->exp=wint2;
}

void  me_EditPosition(struct mob_data *wmob)
{
    int wint;
    char wstr[ME_MAXSTRLEN];

    printf("Old Position:");
    if(wmob->position>mp_max)
        printf("*UNKNOWN* - PLEASE REPORT OR UPDATE -MPOS.DEF-!!!\n");
    else
        printf("%s\n",mp_str[wmob->position]);
    for(wint=0;wint<=mp_max;wint=wint+2)
    {
        if(wint+1<=mp_max)
            printf("%.2d.) %-34.34s %.2d.) %-34.34s\n",wint,mp_str[wint],wint+1,mp_str[wint+1]);
        else
            printf("%.2d.) %s\n",wint,mp_str[wint]);
    }
    wint=mp_max;
    printf("Please enter Number of position (%s):",mp_str[wmob->position]);
    gets(wstr);
    wint=atoi(wstr);
    if(((wint==0)&&(*wstr!='0'))||(wint<0)||(wint>mp_max))
    {
        printf("Position unchanged.\n");
    }
    else
    {
        me_changed=1;
        wmob->position=wint;
    }
}

void  me_EditDefPos(struct mob_data *wmob)
{
    int wint;
    char wstr[ME_MAXSTRLEN];

    printf("Old Default Position:");
    if(wmob->default_pos>mp_max)
        printf("*UNKNOWN* - PLEASE REPORT OR UPDATE -MPOS.DEF-!!!\n");
    else
        printf("%s\n",mp_str[wmob->default_pos]);
    for(wint=0;wint<=mp_max;wint=wint+2)
    {
        if(wint+1<=mp_max)
            printf("%.2d.) %-34.34s %.2d.) %-34.34s\n",wint,mp_str[wint],wint+1,mp_str[wint+1]);
        else
            printf("%.2d.) %s\n",wint,mp_str[wint]);
    }
    wint=mp_max;
    printf("Please enter Number of position (%s):",mp_str[wmob->default_pos]);
    gets(wstr);
    wint=atoi(wstr);
    if(((wint==0)&&(*wstr!='0'))||(wint<0)||(wint>mp_max))
    {
        printf("Default Position unchanged.\n");
    }
    else
    {
        me_changed=1;
        wmob->default_pos=wint;
    }
}

void  me_EditSex(struct mob_data *wmob)
{
    int wint;
    char wstr[ME_MAXSTRLEN];

    printf("Old Sex:");
    if(wmob->sex>ms_max)
        printf("*UNKNOWN* - PLEASE REPORT OR UPDATE -MPOS.DEF-!!!\n");
    else
        printf("%s\n",ms_str[wmob->sex]);
    for(wint=0;wint<=ms_max;wint=wint+2)
    {
        if(wint+1<=ms_max)
            printf("%.2d.) %-34.34s %.2d.) %-34.34s\n",wint,ms_str[wint],wint+1,ms_str[wint+1]);
        else
            printf("%.2d.) %s\n",wint,ms_str[wint]);
    }
    wint=ms_max;
    printf("Please enter Number of Sex (%s):",ms_str[wmob->sex]);
    gets(wstr);
    wint=atoi(wstr);
    if(((wint==0)&&(*wstr!='0'))||(wint<0)||(wint>ms_max))
    {
        printf("Sex unchanged.\n");
    }
    else
    {
        me_changed=1;
        wmob->sex=wint;
    }
}

/* Print all mobile Datas */

void print_mob(struct mob_data *wmob)
{
    int i;

    printf("\n");
    printf("Mobile-Values:\n");
    printf("--------------\n");
/*  printf("INTERNAL Mob-Nr       : %d\n",wmob->mob_number); */
    printf("1.)  Mob-Nr       : %d\n",wmob->virt_number);
    printf("2.)  Mob-NameList : %s\n",wmob->name);
    printf("3.)  Mob-ShortDesc: %s\n",wmob->short_descr);
    printf("4.)  Mob-LongDesc : %s\n",wmob->long_descr);
    printf("5.)  Mob-Desc     ");
    if(wmob->description==NULL)
        printf(": <NOT SET>\n");
    else
        printf(": %s\n",wmob->description);
    printf("6.)  Mob-Actions  : ");
    PrintActions(wmob->flags);
    printf("7.)  Mob-Affects  : ");
    PrintAffects(wmob->affected_by);
    printf("8.)  Mob-Alignment: %d\n",wmob->alignment);
    if(wmob->medit_flags&ME_FLAGS_SIMPLE)
    {
        printf("9.)  Mob-Level    : %d\n",wmob->level);
        printf("10.) Mob-Thac0    : %d\n",wmob->thac0);
        printf("11.) Mob-ArmorCl. : %d\n",wmob->armor);
        printf("12.) Mob-Hitpoints: %ldd%ld+%ld\n",wmob->hitnodice,wmob->hitsizedice,wmob->hitadd);
        printf("13.) Mob-Damage   : %ldd%ld+%ld\n",wmob->damnodice,wmob->damsizedice,wmob->damadd);
        printf("14.) Mob-Gold     : %lu\n",wmob->gold);
        printf("15.) Mob-ExpPoints: %ld\n",wmob->exp);
        printf("16.) Mob-Position : ");
        if(wmob->position>mp_max)
            printf("*UNKNOWN* - PLEASE REPORT OR UPDATE -MPOS.DEF-!!!\n");
        else
            printf("%s\n",mp_str[wmob->position]);
        printf("17.) Mob-DefPos   : ");
        if(wmob->default_pos>mp_max)
            printf("*UNKNOWN* - PLEASE REPORT OR UPDATE -MPOS.DEF-!!!\n");
        else
            printf("%s\n",mp_str[wmob->default_pos]);
        printf("18.) Mob-Sex      : ");
        if(wmob->sex>ms_max)
            printf("*UNKNOWN* - PLEASE REPORT OR UPDATE -MSEX.DEF-!!!\n");
        else
            printf("%s\n",ms_str[wmob->sex]);
    }
    else
        printf("--> OLD MONSTERS NOT YET SUPPORTED! <--\n");
}

/* Get Real Number for DikuCli */

int me_GetRealNumber(int wint)
{
    return(real_mobile(wint));
}

/* Edit all Mobile Data */

int EditAllMob(struct mob_data *wmob)
{
    int maxsel;
    char wstr[ME_MAXSTRLEN];
    int wint;

    maxsel=ME_EDITMAX;

    print_mob(wmob);
    do
    {
        printf("Enter: 0 to quit, 1-%d to edit.\n",maxsel);
        gets(wstr);
        wint=atoi(wstr);
        if((wint>0)&&(wint<=ME_EDITMAX))
        {
            (*me_edit[wint-1])(wmob);
            print_mob(wmob);
        }
        else
            if(wint!=0)
                printf("** Illegal input. Try again!\n");
    }
    while(wint!=0);

    return(ME_SM_SAVE);
}

/* Edit a Mobile for DikuCli */

void me_EditMobile(int nr)
{
    struct mob_data *wmob;
    int me_m;

    me_changed=0;

    wmob=read_mobile(nr,REAL);
    me_m=EditAllMob(wmob);
    if((me_changed!=0)||(me_m==ME_SM_DELETE))
        SaveToMETemp(nr,me_m,wmob);
    free_mob(wmob);
}

/* Clone a Mobile (for DikuCli) */

void me_CloneMobile(int nr1, int nr2)
{
    struct mob_data *wmob;
    if(nr1<0)
        wmob= &me_defaultmob;
    else
        wmob=read_mobile(nr1,REAL);

    me_doclone(wmob,nr2);

    if(nr1>=0)
        free_mob(wmob);
}

/* Purge a Mobile for DikuCli */

void me_PurgeMobile(int nr)
{
    SaveToMETemp(nr,ME_SM_DELETE,NULL);
}

/* Show Mobile for DikuCli */

void me_ShowMobile(int nr)
{
    struct mob_data *wmob;

    wmob=read_mobile(nr,REAL);
    print_mob(wmob);
    free_mob(wmob);
}

/* Set FileName of *.mob file */

void me_SetFile(char *wstr)
{
    strcpy(me_mobfile,wstr);
}

/* Set TempFile */

void me_SetTemp(char *wstr)
{
    strcpy(me_tempfile,wstr);
}

/* Set database path */

void me_SetDBPath(char *wstr)
{
    strcpy(me_dbpath,wstr);
}

/* Initialize the MEdit vars */

void me_InitMEdit()
{

    me_changed=0;
    if(GetADef(ms_file,ME_MSFILE,me_dbpath,&ms_str,&ms_max))
        printf("*** Can't open a *.def file!\n");
    if(GetADef(mp_file,ME_MPFILE,me_dbpath,&mp_str,&mp_max))
        printf("*** Can't open a *.def file!\n");
    if(GetADef(mc_file,ME_MCFILE,me_dbpath,&mc_str,&mc_max))
        printf("*** Can't open a *.def file!\n");
    if(GetADef(ma_file,ME_MAFILE,me_dbpath,&ma_str,&ma_max))
        printf("*** Can't open a *.def file!\n");
    if(OpenMobFile())
        printf("***** IMPOSSIBLE ERROR *****");
}


void me_CloseMEdit(void)
{
    fclose(mob_f);
    FreeADef(ms_str,ms_max);
    FreeADef(mp_str,mp_max);
    FreeADef(mc_str,mc_max);
    FreeADef(ma_str,ma_max);
}
