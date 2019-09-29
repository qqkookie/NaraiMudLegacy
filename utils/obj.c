/**************************************************************************
*  file: db.c , Database module.                          Part of DIKUMUD *
*  Usage: Loading/Saving chars, booting world, resetting etc.             *
*  Copyright (C) 1990, 1991 - see 'license.doc' for complete information. *
***************************************************************************/

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <time.h>

#include "structs.h"
#include "utils.h"
#include "db.h"
#include "comm.h"
#include "handler.h"
#include "limits.h"
#include "spells.h"

#define NEWRENTMAGIC 29922 /* magic number to tell new rent format from old */
#define KJHRENT      66666 /* kjh number to tell new rent format */
#define NEW_ZONE_SYSTEM
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
char news[MAX_STRING_LENGTH];          /* the news                        */
char motd[MAX_STRING_LENGTH];         /* the messages of today           */
char help[MAX_STRING_LENGTH];         /* the main help page              */
char info[MAX_STRING_LENGTH];         /* the info text                   */


FILE *obj_f;                          /* obj prototypes                  */

struct index_data *obj_index;         /* index table for object file     */
struct help_index_element *help_index = 0;

int top_of_objt = 0;                  /* top of object index table       */
int top_of_helpt;                     /* top of help index table         */

struct time_info_data time_info;  /* the infomation about the time   */
struct weather_data weather_info;  /* the infomation about the weather */

/* local procedures */
void boot_zones(void);
void setup_dir(FILE *fl, int room, int dir);
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


/*************************************************************************
*  routines for booting the system                                       *
*********************************************************************** */


/* body of the booting system */
void boot_db(void)
{
  int i;
  extern int no_specials;

  log("Boot db -- BEGIN.");

  if (!(obj_f = fopen(OBJ_FILE, "r")))
  {
    perror("boot");
    exit(0);
  }

  log("Generating index tables for object files.");
  obj_index = generate_indices(obj_f, &top_of_objt);
      
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

/*************************************************************************
*  procedures for resetting, both play-time and boot-time      *
*********************************************************************** */
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





/*************************************************************************
*  stuff related to the save/load player system                  *
*********************************************************************** */

/* read and allocate space for a '~'-terminated string from a given file */
char *fread_string(FILE *fl)
{
  char buf[MAX_STRING_LENGTH], tmp[500];
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

    if (ch->player.title)
    free(ch->player.title);
  if (ch->player.short_descr)
    free(ch->player.short_descr);
  if (ch->player.long_descr)
    free(ch->player.long_descr);
  if(ch->player.description)
    free(ch->player.description);

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
  GET_AC(ch) = 100; /* Basic Armor */
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
  ch->points.armor = 100;

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

void stash_char(struct char_data *ch, char *filename)
{
  struct obj_data *p;
  char stashfile[100],name[16];
  FILE *fl;
  int i,j;
  void stash_contents(FILE *fl, struct obj_data *p);

  if( IS_NPC(ch) || ! ch->desc) return;
  strcpy(name,filename ? filename : GET_NAME(ch));
  for(i=0;name[i];++i)
    if(isupper(name[i]))
       name[i]=tolower(name[i]);
  sprintf(stashfile,"%s/%c/%s.x",STASH,name[0],name);
  if (!(fl = fopen(stashfile, "w"))) {		/* remove all data if exist */
    perror("saving PC's stash");
    return;
  }
  fprintf(fl,"%d\n",KJHRENT);
  if(ch->carrying)
    stash_contents(fl,ch->carrying);
  for(i=0;i<MAX_WEAR;++i)
    if(p=ch->equipment[i]){
      stash_contents(fl,p);
    }
  fclose(fl);
}

/* save one item into stashfile. if item has contents, save it also */
void stash_contents(FILE *fl, struct obj_data *p)
{
  struct obj_data *pc;
  int j;

  if(p==0) return;
  if(p->obj_flags.type_flag != ITEM_KEY){
    fprintf(fl,"%d",obj_index[p->item_number].virtual);
    for(j=0;j<4;++j)
      fprintf(fl," %d",p->obj_flags.value[j]);
    for(j=0;j<2;j++)
      fprintf(fl," %d %d",p->affected[j].location,p->affected[j].modifier);
    fprintf(fl,"\n");
  }
  if(pc=p->contains)
    stash_contents(fl,pc);
  if(pc=p->next_content)
    stash_contents(fl,pc);
}

/* Read stash file and load objects to player. and remove stash files */
void unstash_char(struct char_data *ch, char *filename)
{
  void wipe_stash(char *filename);
  struct obj_data *obj;
  char stashfile[100],name[100],sf2[100];
  FILE *fl;
  int i,n,newflag,tmp[4];

  strcpy(name,filename ? filename : GET_NAME(ch));
  for(i=0;name[i];++i)
    if(isupper(name[i]))
       name[i]=tolower(name[i]);
  sprintf(stashfile,"%s/%c/%s.x.y",STASH,name[0],name);
  if(!(fl=fopen(stashfile, "r"))){
    sprintf(stashfile,"%s/%c/%s.x",STASH,name[0],name);
    if(!(fl=fopen(stashfile, "r")))
      return;
  }
  newflag=0;
  for(;;){
    if(fscanf(fl,"%d",&n) <= 0) break;
    if(n == NEWRENTMAGIC){
      newflag=1;
      continue;
    }
    if(n==KJHRENT){
      newflag=2;
      continue;
    }
    if(newflag){
      for(i=0;i<4;++i)
        fscanf(fl,"%d",&tmp[i]);
    }
    if(n < 1000) continue;
    obj=read_object(n,VIRTUAL);
    if(obj == 0) continue;
    if(newflag){
      for(i=0;i<4;++i)
        obj->obj_flags.value[i]=tmp[i];
    }
    if(newflag==2){
      for(i=0;i<4;i++)
        fscanf(fl,"%d",&tmp[i]);
      for(i=0;i<2;i++){
        obj->affected[i].location=tmp[i*2];
        obj->affected[i].modifier=tmp[i*2+1];
      }
    } 
    obj_to_char(obj,ch);
  }
  fclose(fl);
  sprintf(sf2,"%s/%c/%s.x.tmp",STASH,name[0],name);
  rename(stashfile,sf2);
  wipe_stash(name);	/* delete file.x and file.x.y */
}

void wipe_stash(char *filename)		/* delete file.x and file.x.y */
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
  char stashfile[100],name[MAX_STRING_LENGTH],buf[MAX_STRING_LENGTH];
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
  for(i=j=0;;){
    if(fscanf(fl,"%d",&n) <= 0) break;
    if(n < 1000) continue;
    if(n > 9999) continue;
    ++j;
    sprintf(buf+i,"%5d%c",n,(j==10) ? '\n' : ' ');
    if(j==10) j=0; i+=5;
  }
  fclose(fl);
  strcat(buf,"\n\r");
  send_to_char(buf,ch);
  return;
}
void do_extractrent(struct char_data *ch,char *argument, int cmd)
{
  char name[MAX_STRING_LENGTH],buf[MAX_STRING_LENGTH];
  FILE *fl;

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
  char name[MAX_STRING_LENGTH],buf[MAX_STRING_LENGTH];
  FILE *fl;

  one_argument(argument,name);
  if(! *name)
    return;
  stash_char(ch,name);
  move_stashfile(name);
  send_to_char("OK.\n\r",ch);
  sprintf(buf,"%s replaced rent for %s",GET_NAME(ch),name);
  log(buf);
}

void do_rent(struct char_data *ch, int cmd, char *arg)
{
  char buf[240];
  sh_int save_room;
  int i;
  void wipe_obj(struct obj_data *obj);

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
  stash_char(ch,0);	/* clear file.x and save into file.x */
  move_stashfile(ch->player.name);	/* move file.x to file.x.y */
  /*
  for(i=0; i<MAX_WEAR; i++)
    if(ch->equipment[i]){
      extract_obj(unequip_char(ch,i));
      ch->equipment[i]=0;
    }
    */
  wipe_obj(ch->carrying);
  for(i=0; i<MAX_WEAR; i++)
     if(ch->equipment[i]) {
       extract_obj(unequip_char(ch,i));
       ch->equipment[i]=0;
     }
  wipe_obj(ch->carrying);
  ch->carrying=0;
  save_room = ch->in_room;
  extract_char(ch);
  ch->in_room = world[save_room].number;
  save_char(ch, ch->in_room);
  return;
}
void wipe_obj(struct obj_data *obj)
{
  if(obj){
    wipe_obj(obj->contains);
    wipe_obj(obj->next_content);
    if (obj->in_obj)
      obj_from_obj(obj);
    extract_obj(obj);
  }
}
