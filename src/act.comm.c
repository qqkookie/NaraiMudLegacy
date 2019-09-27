
/* ************************************************************************
*  file: act.comm.c , Implementation of commands.         Part of DIKUMUD *
*  Usage : Communication.                                                 *
*  Copyright (C) 1990, 1991 - see 'license.doc' for complete information. *
************************************************************************* */

#include <stdio.h>
#include <string.h>
#include <time.h>
#include <assert.h>

#include "structs.h"
#include "utils.h"
#include "comm.h"
#include "interpreter.h"
#include "handler.h"
#include "db.h"
#include "spells.h"

#include <sys/time.h>

/* extern variables */
extern char history[20][MAX_STRING_LENGTH];
extern int his_start, his_end;

extern struct room_data *world;
extern struct descriptor_data *descriptor_list;

int file_to_string(char *name, char *buf);

/* 말하기 */
void do_say(struct char_data *ch, char *argument, int cmd)
{
  int i;
  static char buf[MAX_STRING_LENGTH];

  if(IS_SET(ch->specials.act,PLR_DUMB_BY_WIZ)&&GET_LEVEL(ch)<IMO+3)
  {
	return;
  }
  for (i = 0; *(argument + i) == ' '; i++);

  if (!*(argument + i))
    send_to_char("Yes, but WHAT do you want to say?\n\r", ch);
  else {
    sprintf(buf,"You say '%s'\n\r", argument + i);
    send_to_char(buf,ch);
    sprintf(buf,"$n says '%s'", argument + i);
    act(buf,FALSE,ch,0,0,TO_ROOM);
  }
}

void do_sayh(struct char_data *ch, char *argument, int cmd)
{		/* hangul say */
  int i;
  static char buf[MAX_STRING_LENGTH], buf2[MAX_STRING_LENGTH];

  if(IS_SET(ch->specials.act,PLR_DUMB_BY_WIZ)&&GET_LEVEL(ch)<IMO+3)
  {
	return;
  }
  for (i = 0; *(argument + i) == ' '; i++);

  if (!*(argument + i))
    send_to_char_han("Yes, but WHAT do you want to say?\n\r",
      "예? 뭐라고 말해요 ?\n\r", ch);
  else {
    sprintf(buf,"You say '%s'\n\r", argument + i);
    sprintf(buf2,"'%s' 하고 말합니다\n\r", argument + i);
    /* English - Korean display act() , english text first.. */
    send_to_char_han(buf, buf2, ch);

    sprintf(buf,"$n says '%s'", argument + i);
    sprintf(buf2,"$n 님이 '%s' 하고 말합니다", argument + i);
    /* English - Korean display act() , english text first.. */
    acthan(buf,buf2,FALSE,ch,0,0,TO_ROOM);
  }
}
/* 외치기 */
void do_shout(struct char_data *ch, char *argument, int cmd)
{
  static char buf[MAX_STRING_LENGTH], buf1[MAX_STRING_LENGTH];
  struct descriptor_data *i;
  extern int noshoutflag;

  if(IS_SET(ch->specials.act,PLR_DUMB_BY_WIZ)&&GET_LEVEL(ch)<IMO+3)
  {
	return;
  }
  if(noshoutflag && !IS_NPC(ch) && (GET_LEVEL(ch) < IMO)){
    send_to_char("I guess you can't shout now?\n\r",ch);
    return;
  }
  if(!IS_NPC(ch))
  if (IS_SET(ch->specials.act, PLR_NOSHOUT)) {
    send_to_char("You can't shout!!\n\r", ch);
    return;
  }
  for (; *argument == ' '; argument++);
  if (!(*argument))
    send_to_char("Shout? Yes! Fine! Shout we must, but WHAT??\n\r", ch);
  else {
    send_to_char("Ok.\n\r", ch);
    sprintf(buf1, "$n shouts '%s'", argument);

    // sprintf(buf, "%s shouts '%s'\n\r", GET_NAME(ch), argument);

    time_t tt = time(NULL);
    strftime(buf, 127, "%F %H:%M", localtime(&tt));
    sprintf(buf + strlen(buf)," %s >> %s\n\r", GET_NAME(ch), argument);
    assert(his_end>=0&&his_end<20);
    strcpy(history[his_end], &buf[5]);		// omit year part
    his_end++;
    if((his_end%20)==(his_start%20))
    {
    	his_end=his_start%20;
    	his_start=(his_start+1)%20;
    }

    for (i = descriptor_list; i; i = i->next)
	{
        if (i->character != ch && !i->connected && i->character &&
      !IS_SET(i->character->specials.act, PLR_EARMUFFS))
        act(buf1, 0, ch, 0, i->character, TO_VICT);
	}

#ifdef CHATLOG
    extern FILE *chatlogfp;
    if (chatlogfp == NULL)
	chatlogfp = fopen(CHATLOG, "a");
    fputs(buf, chatlogfp);
    fflush(chatlogfp);
    /* 20110117 by Moon */
#endif

  }
}
/* 얘기하기 */
void do_tell(struct char_data *ch, char *argument, int cmd)
{
  struct char_data *vict;
  char *s, name[100], message[MAX_INPUT_LENGTH],
  buf[MAX_OUTPUT_LENGTH];

  if(IS_SET(ch->specials.act,PLR_DUMB_BY_WIZ)&&GET_LEVEL(ch)<IMO+3)
  {
	return;
  }
  half_chop(argument,name,message);
  if(!*name || !*message)
    send_to_char("Who do you wish to tell what??\n\r", ch);
  else if (!(vict = get_char_vis(ch, name)))
    send_to_char("No-one by that name here..\n\r", ch);
  else if (ch == vict)
    send_to_char("You try to tell yourself something.\n\r", ch);
  else if (GET_POS(vict) == POSITION_SLEEPING) {
    act("$E can't hear you.",FALSE,ch,0,vict,TO_CHAR);
  } else if ((!IS_SET(vict->specials.act,PLR_NOTELL)) ||
             ((GET_LEVEL(ch) >= IMO) && (GET_LEVEL(ch) > GET_LEVEL(vict)))){
    if(IS_NPC(ch))
      s=ch->player.short_descr;
    else
      s=CAN_SEE(vict,ch) ? GET_NAME(ch) : "Someone";
    sprintf(buf,"%s tells you '%s'\n\r",s,message);
    send_to_char(buf, vict);
	sprintf(vict->specials.reply_who,"%s",GET_NAME(ch));
    sprintf(buf,"You tell %s '%s'\n\r",GET_NAME(vict),message);
    send_to_char(buf, ch);
  } else {
    act("$E isn't listening now.",FALSE,ch,0,vict,TO_CHAR);
  }
}
void do_reply(struct char_data *ch,char *argument,int cmd)
{
	char buf[MAX_STRING_LENGTH];
	sprintf(buf,"%s %s",ch->specials.reply_who,argument);
	do_tell(ch,buf,0);
}
/* 보내기 */
void do_send(struct char_data *ch, char *argument, int cmd)
{
  struct char_data *vict;
  char *s, name[100], paint_name[300], message[200],
  buf[MAX_STRING_LENGTH], paint[MAX_STRING_LENGTH];

  if(IS_SET(ch->specials.act,PLR_DUMB_BY_WIZ)&&GET_LEVEL(ch)<IMO+3)
  {
	return;
  }
  half_chop(argument,name,message);
  sprintf(paint_name,"paints/%s", message);
  if(!*name || !*message)
    send_to_char("Who do you wish to tell what??\n\r", ch);
  else if (!(vict = get_char_vis(ch, name)))
    send_to_char("No-one by that name here..\n\r", ch);
  else if (file_to_string(paint_name,paint) == -1)
    send_to_char("No such paint prepared.\n\r",ch);
  else if ((!IS_SET(vict->specials.act,PLR_NOTELL)) ||
             ((GET_LEVEL(ch) >= IMO) && (GET_LEVEL(ch) > GET_LEVEL(vict)))){
    send_to_char(paint, vict);
    sprintf(buf,"You send %s %s\n\r",GET_NAME(vict),message);
    send_to_char(buf, ch);
    if(IS_NPC(ch))
      s=ch->player.short_descr;
    else
      s=CAN_SEE(vict,ch) ? GET_NAME(ch) : "Someone";
    sprintf(buf,"%s sends you '%s'\n\r",s,message);
    send_to_char(buf, vict);
  } else {
    act("$E isn't listening now.",FALSE,ch,0,vict,TO_CHAR);
  }
}
/*  그룹 말하기 */
void do_gtell(struct char_data *ch, char *argument, int cmd)
{
 struct char_data *k;
 struct follow_type *f;
 char *s, buf[200];

 if(IS_SET(ch->specials.act,PLR_DUMB_BY_WIZ)&&GET_LEVEL(ch)<IMO+3)
 {
	return;
 }
 if (!(k=ch->master))
  k = ch;
 if(IS_NPC(ch))
   s=ch->player.short_descr;
 else
   s=GET_NAME(ch);
  if (k&&IS_AFFECTED(k,AFF_GROUP)){
    sprintf(buf,"** %s ** '%s'\n\r",s,argument);
    send_to_char(buf, k);
  }
 for (f=k->followers;f;f=f->next) {
  if (f->follower&&IS_AFFECTED(f->follower,AFF_GROUP)){
    sprintf(buf,"** %s ** '%s'\n\r",s,argument);
    send_to_char(buf, f->follower);
  }
 }
/* send_to_char("Ok.\n\r", ch); */
}
/* 속삭이기 */
void do_whisper(struct char_data *ch, char *argument, int cmd)
{
  struct char_data *vict;
  char name[100], message[MAX_INPUT_LENGTH],
    buf[MAX_OUTPUT_LENGTH];

  if(IS_SET(ch->specials.act,PLR_DUMB_BY_WIZ)&&GET_LEVEL(ch)<IMO+3)
  {
	return;
  }
  half_chop(argument,name,message);

  if(!*name || !*message)
    send_to_char("Who do you want to whisper to.. and what??\n\r", ch);
  else if (!(vict = get_char_room_vis(ch, name)))
    send_to_char("No-one by that name here..\n\r", ch);
  else if (vict == ch)
  {
    act("$n whispers quietly to $mself.",FALSE,ch,0,0,TO_ROOM);
    send_to_char(
      "You can't seem to get your mouth close enough to your ear...\n\r",
       ch);
  }
  else
  {
    sprintf(buf,"$n whispers to you, '%s'",message);
    act(buf, FALSE, ch, 0, vict, TO_VICT);
/*    send_to_char("Ok.\n\r", ch); */
    act("$n whispers something to $N.", FALSE, ch, 0, vict, TO_NOTVICT);
  }
}

/* 물어보기 */
void do_ask(struct char_data *ch, char *argument, int cmd)
{
  struct char_data *vict;
  char name[100], message[MAX_INPUT_LENGTH],
    buf[MAX_OUTPUT_LENGTH];

  if(IS_SET(ch->specials.act,PLR_DUMB_BY_WIZ)&&GET_LEVEL(ch)<IMO+3)
  {
	return;
  }
  half_chop(argument,name,message);

  if(!*name || !*message)
    send_to_char("Who do you want to ask something.. and what??\n\r", ch);
  else if (!(vict = get_char_room_vis(ch, name)))
    send_to_char("No-one by that name here..\n\r", ch);
  else if (vict == ch)
  {
    act("$n quietly asks $mself a question.",FALSE,ch,0,0,TO_ROOM);
    send_to_char("You think about it for a while...\n\r", ch);
  }
  else
  {
    sprintf(buf,"$n asks you '%s'",message);
    act(buf, FALSE, ch, 0, vict, TO_VICT);
/*    send_to_char("Ok.\n\r", ch); */
    act("$n asks $N a question.",FALSE,ch,0,vict,TO_NOTVICT);
  }
}

void do_lastchat(struct char_data *ch, char *argument, int cmd)
{
  /*struct descriptor_data *i;
  struct char_data *victim;*/
  int j;

  for(j=his_start;j!=his_end;j=(j+1)%20)
  {
    assert(history[j]);
    send_to_char(history[j],ch);
  }
  return;
}

#define MAX_NOTE_LENGTH MAX_STRING_LENGTH      /* arbitrary */
/* 쓰기 */
void do_write(struct char_data *ch, char *argument, int cmd)
{
    struct obj_data *paper = 0, *pen = 0;
    char papername[MAX_INPUT_LENGTH], penname[MAX_INPUT_LENGTH],
	buf[MAX_OUTPUT_LENGTH];

	if(IS_SET(ch->specials.act,PLR_DUMB_BY_WIZ)&&GET_LEVEL(ch)<IMO+3)
	{
		return;
	}
    argument_interpreter(argument, papername, penname);

    if (!ch->desc)
   	return;

    if (!*papername)  /* nothing was delivered */
    {
	send_to_char(
            "Write? with what? ON what? what are you trying to do??\n\r", ch);
	return;
    }
    if (*penname) /* there were two arguments */
    {
	if (!(paper = get_obj_in_list_vis(ch, papername, ch->carrying)))
	{
            sprintf(buf, "You have no %s.\n\r", papername);
	    send_to_char(buf, ch);
	    return;
	}
	if (!(pen = get_obj_in_list_vis(ch, penname, ch->carrying)))
	{
	    sprintf(buf, "You have no %s.\n\r", penname);
	    send_to_char(buf, ch);
	    return;
	}
    }
    else  /* there was one arg.let's see what we can find */
    {
	if (!(paper = get_obj_in_list_vis(ch, papername, ch->carrying)))
	{
            sprintf(buf, "There is no %s in your inventory.\n\r", papername);
	    send_to_char(buf, ch);
	    return;
	}
	if (paper->obj_flags.type_flag == ITEM_PEN)  /* oops, a pen.. */
	{
	    pen = paper;
	    paper = 0;
	}
	else if (paper->obj_flags.type_flag != ITEM_NOTE)
	{
	    send_to_char("That thing has nothing to do with writing.\n\r", ch);
	    return;
	}

	/* one object was found. Now for the other one. */
	if (!ch->equipment[HOLD])
	{
	    sprintf(buf, "You can't write with a %s alone.\n\r", papername);
	    send_to_char(buf, ch);
	    return;
	}
	if (!CAN_SEE_OBJ(ch, ch->equipment[HOLD]))
	{
	    send_to_char("The stuff in your hand is invisible! Yeech!!\n\r", ch);
	    return;
	}

	if (pen) paper = ch->equipment[HOLD];
	else pen = ch->equipment[HOLD];
    }

    /* ok.. now let's see what kind of stuff we've found */
    if (pen->obj_flags.type_flag != ITEM_PEN)
    {
	act("$p is no good for writing with.",FALSE,ch,pen,0,TO_CHAR);
    }
    else if (paper->obj_flags.type_flag != ITEM_NOTE)
    {
	act("You can't write on $p.", FALSE, ch, paper, 0, TO_CHAR);
    }
    else if (paper->action_description)
	send_to_char("There's something written on it already.\n\r", ch);
    else
    {
	/* we can write - hooray! */

	send_to_char("Ok.. go ahead and write.. end the note with a @.\n\r",
            ch);
	act("$n begins to jot down a note.", TRUE, ch, 0,0,TO_ROOM);
	ch->desc->str = &paper->action_description;
	ch->desc->max_str = MAX_NOTE_LENGTH;
	ch->desc->title = 0;
    }
}

