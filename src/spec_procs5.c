/* ************************************************************************
 *  file: spec_procs3.c , Special module.                  Part of DIKUMUD *
 *  Usage: Procedures handling special procedures for object/room/mobile   *
 *  Made by Choi Yeon Beom in KAIST                                        *
 ************************************************************************* */
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
#include <stdlib.h>

#include "structs.h"
#include "utils.h"
#include "comm.h"
#include "interpreter.h"
#include "handler.h"
#include "db.h"
#include "spells.h"
#include "limits.h"
#include "mob_magic.h"		/* by cyb */
#include "quest.h"	/* by cyb */
#include "command.h"

/* The above array makes the procedure generalized enough that it can be
   attached to as many other mobs on your MUD as you wish.  The format
   of each entry is:

   {<mob vnum>, <behavior flags>, <object to be defended, -1 if none>,
   <direction to block>, <blocking room mob is in, -1 if none>}

*/

int mob_defender (struct char_data *ch, int cmd, char *argument) {

    /* Written by Onivel of Jedi, 4-93.  I make no claims that this is the
       cleanest or best way of doing things.  I do claim that it works and
       and does what we want it to do on Jedi.  -- If you like it, feel free
       to use it.  If not *shrug* */

	char buf[255], buf2[255];
    /*
      Deal with blocking the exits first.  -- The old "You'll have to
      go through me first!" mentality.
    */

    if (cmd == COM_FLEE && !IS_NPC(ch)) {
        sprintf(buf2,"Mob prevents you from %sing!\n\r", cmd == COM_FLEE ?
                "flee" : "retreat");
        send_to_char(buf2,ch);
        sprintf(buf2,"Mob prevents %s from %sing!\n\r",GET_NAME(ch), 
                cmd == COM_FLEE ? "flee" : "retreat");
        send_to_room_except(buf2,ch->in_room, ch);
        return TRUE;
    }  

    if (cmd == COM_UP) {
        sprintf(buf2,"Mob tells you, 'You have to go through me first.'\n\r");
        send_to_char(buf2,ch);
        sprintf(buf2,"Mob blocks %s's way!\n\r",GET_NAME(ch));
        send_to_room_except(buf2,ch->in_room,ch);
        return TRUE;
    }
	if (cmd) return FALSE;

    if (IS_NPC(ch) && GET_HIT(ch) < GET_MAX_HIT(ch)*0.8 && !number(0, 3)) {
        act("$n utters the words 'pzar maximus'.",1,ch,0,0,TO_ROOM);
        GET_HIT(ch) += number ((GET_MAX_HIT(ch)/15), (GET_MAX_HIT(ch)/20));
        GET_HIT(ch) = MIN(GET_HIT(ch), GET_MAX_HIT(ch));
    }
    return FALSE;
}

#define BASE_MONEY 500
long jackpot = BASE_MONEY;
 
int slotmachine(struct char_data *ch, int cmd, char *arg)
{
 	int c, i[3], ind;
 	char buf[255];
    int basemoney = BASE_MONEY;
    int base_mul = 1;
 
 	if(cmd != COM_PULL && cmd != COM_BALANCE) return(FALSE);

	if (GET_LEVEL(ch) > IMO && cmd == COM_BALANCE) {
		sprintf(buf, "JackPot amount = %ld\n\r", jackpot);
		send_to_char(buf, ch);
		return(TRUE);
	}

    base_mul = jackpot / 2800000;       // 2.8M
    if (base_mul < 1) base_mul = 1;

    basemoney *= base_mul;
		
 	if(GET_GOLD(ch) < basemoney) {
   		send_to_char("You don't have enough gold!\n\r", ch);
   		return(TRUE);
 	}
	else {
		sprintf(buf, "You inserted %d coins!\n\r", basemoney);
   		send_to_char(buf, ch);
	}
 
 	if(jackpot < 0)
   		jackpot = basemoney; /* always at LEAST have 2000 in there */
 
 	GET_GOLD(ch) -= basemoney;
 	jackpot += basemoney;
 
 	for(c = 0; c <= 2; c++) {
   		i[c] = number(1, 28);
    	switch(i[c]) {
		case 1:
		case 2:
		case 3:
		case 4:
		case 5:
		case 6:
		case 7:  i[c] = 0;
            sprintf(buf, "Slot %d: Lemon\n\r", c);
            send_to_char(buf, ch);
            break;
		case 8:
		case 9:
		case 10:
		case 11:
		case 12:
		case 13:
		case 14: i[c] = 1;
            sprintf(buf, "Slot %d: Orange\n\r", c);
            send_to_char(buf, ch);
            break;
		case 15:
		case 16:
		case 17:
		case 18:
		case 19:
		case 20: i[c] = 2;
            sprintf(buf, "Slot %d: Banana\n\r", c);
            send_to_char(buf, ch);
            break;
		case 21:
		case 22:
		case 23:
		case 24:
		case 25: i[c] = 3;
            sprintf(buf, "Slot %d: Peach\n\r", c);
            send_to_char(buf, ch);
            break;
		case 26:
		case 27: i[c] = 4;
            sprintf(buf, "Slot %d: Bar\n\r", c);
            send_to_char(buf, ch);
            break;
		case 28: i[c] = 5;
            sprintf(buf, "Slot %d: Gold\n\r", c);
            send_to_char(buf, ch);
            break;
    	}
 
  	}
 
 	if((i[0] == i[1]) && (i[1] == i[2])) {
		/* Ok, they've won, now how much? */
   		send_to_char("You've won!\n\r", ch); 
 
		switch(i[0]) {
		case 0: ind = basemoney * 2; /* Give them back what they put in */
            break;
		case 1: ind = basemoney*4 * 2;
            break;
		case 2: ind = basemoney*8 * 2;
            break;
		case 3: ind = basemoney*20 * 2;
            break;
		case 4: ind = basemoney*40 * 2;
            break;
		case 5: ind = jackpot; /* Wow! We've won big! */
            act("Sirens start sounding and lights start flashing everywhere!", FALSE, ch, 0, 0, TO_ROOM);
            sprintf(buf, "<INFO> : %s won JackPot(%d) !!!", 
                    ch->player.name, ind);
            log(buf);
            send_to_except(buf, ch);
            break;
		}
 
    	if(ind > jackpot)
      		ind = jackpot; /* Can only win as much as there is */
 
		sprintf(buf, "You have won %d coins!\n\r", ind);
		send_to_char(buf, ch);

		GET_GOLD(ch) += ind;
		jackpot -= ind;
		return(TRUE);
  	}

	WAIT_STATE(ch, PULSE_VIOLENCE/4);
 	send_to_char("Sorry, you didn't win.\n\r", ch);
 	return(TRUE);
}


/* BaseBalll By Marx */
#define BASEBALL_SIZE 4
#define MAX_BBAMT 1000000
#define MAX_BBGUESS 10

struct baseball_data
{
	struct char_data *player;
	int	amt;
	char number[10];
	sh_int try_num;
	char guess[MAX_BBGUESS][10];
	char score[MAX_BBGUESS][2];
	struct baseball_data *next;
} *bbdata_list = NULL;

int baseball(struct char_data *ch, int cmd, char *arg)
{
	struct baseball_data *bbdata;
	char buf[255], argm[100];
	char tguess[9];
	int amt;
	int strike, ball;
	int i, j;
    extern void do_say(struct char_data *ch, char *argument, int cmd);


    if (IS_NPC(ch))
        return(FALSE);

	if(cmd != COM_PULL && cmd != COM_SAY && cmd != COM_BALANCE) return(FALSE);  

	strike = 0;
	ball = 0;


	for (bbdata = bbdata_list; bbdata != NULL; bbdata = bbdata->next)
		if (bbdata->player == ch) break;

    // 사용자 수만큼 malloc?
    //   나중에 메모리 free할 time 결정 필요(jmjeong)
    //
	if (bbdata == NULL || bbdata->player != ch) {
		bbdata = (struct baseball_data *)malloc(sizeof(struct baseball_data));
		bbdata->player = ch;
		bbdata->try_num = -1;
		bbdata->next = bbdata_list;
		memset(bbdata->number, 0x00, sizeof(bbdata->number));
		bbdata_list = bbdata;
	}
	

	if(cmd == COM_PULL) {
		if(! *arg){
            send_to_char("You must specify an amount.\n\r",ch);
            return(TRUE);
        }

		one_argument(arg, argm);
		amt = atoi(argm);

		if (amt <= 0) {
			send_to_char("Amount must be positive.\n\r",ch);
			return(TRUE);
		}

		if (amt > GET_GOLD(ch)) {
			send_to_char("You don't have enough gold!\n\r",ch);
			return(TRUE);
		}

		if (amt > MAX_BBAMT) {
			sprintf(buf, "You can insert under %d coins!\n\r", MAX_BBAMT);
			send_to_char(buf, ch);
			return(TRUE);
		}

		GET_GOLD(ch)-=amt;    
		bbdata->amt = amt;
		bbdata->try_num = 0;

		for (i = 0; i < BASEBALL_SIZE; i++) {
			bbdata->number[i] = number(0,9) + '0';
			for (j = 0; j < i; j++) {
				if (bbdata->number[i] == bbdata->number[j]) {
					i--;
					break;
				}
			}
		}


		sprintf(buf,"Number Generator : I made %d digits number. Guess it!\n\r",
                BASEBALL_SIZE);	
		send_to_char(buf, ch);
	} else if (cmd == COM_SAY) {
		one_argument(arg, argm);
		if (strlen(argm) != BASEBALL_SIZE) {
			do_say(ch,arg,0);
			return(TRUE);
		}
		for (i = 0; i < BASEBALL_SIZE; i++) {
			if (!isdigit(argm[i])) {
				do_say(ch,arg,0);
				return(TRUE);
			}
		}

		if (bbdata->try_num < 0) {
			send_to_char("Try pull first.\n\r", ch);
			return(TRUE);
		}


		sprintf(tguess, "%0*d", BASEBALL_SIZE, atoi(argm));

		for (i = 0; i < BASEBALL_SIZE; i++)
			for (j = 0; j < BASEBALL_SIZE; j++) {
				if (tguess[i] == bbdata->number[j]) {
					if (i == j) strike++; 
					else ball++;
				}
				if ((tguess[i] == tguess[j]) && (i != j)) {
					send_to_char("Illegal number. Try another.\n\r", ch);
					return(TRUE);
				}
			}

		strcpy(bbdata->guess[bbdata->try_num], tguess);
		bbdata->score[bbdata->try_num][0] = strike;
		bbdata->score[bbdata->try_num][1] = ball;
		bbdata->try_num++;

		sprintf(buf, "Try %d. You guessed %s. S:%d B:%d.\n\r",
                bbdata->try_num, tguess, strike, ball);
		send_to_char(buf, ch);

		if (strike == BASEBALL_SIZE) {
			amt = 0;
			switch(bbdata->try_num) {
            case 1 : amt = bbdata->amt * 100; break;
            case 2 : amt = bbdata->amt * 16; break;
            case 3 : amt = bbdata->amt * 4; break;
            case 4 : amt = bbdata->amt * 2.3; break;
            case 5 : amt = bbdata->amt * 1.5; break;
            case 6 : amt = bbdata->amt * 1; break;
            case 7 : amt = bbdata->amt * 0.8; break;
            case 8 : amt = bbdata->amt * 0.5; break;
            case 9 : amt = bbdata->amt * 0.1; break;
			}

			GET_GOLD(ch) += amt;
			sprintf(buf, "BINGO!! You have won %d coins in %d attempts.\n\r",
                    amt, bbdata->try_num);
			send_to_char(buf, ch);
			if (bbdata->try_num < 3) {
				sprintf(buf, "<INFO> : %s won BaseBall in %d attemps(%d)!!!\n\r"
                        , ch->player.name, bbdata->try_num, amt);
                send_to_except(buf, ch);
			}
			bbdata->try_num = -1;
		}
		
		if (bbdata->try_num >= MAX_BBGUESS) {
			sprintf(buf,"You can't find the number in %d attemps. You lose.\n\r"
                    , MAX_BBGUESS);
			send_to_char(buf, ch);
			bbdata->try_num = -1;
		}
	} else if (cmd == COM_BALANCE) {
		if (bbdata->try_num < 0) {
			send_to_char("You are not in game.\n\r",ch);
			return(TRUE);
		} else if (bbdata->try_num == 0) {
			send_to_char("You didn't guess any number.\n\r",ch);
		}

		sprintf(buf, "Your guessing list:\n\r");
		send_to_char(buf, ch);
		for (i = 0; i < bbdata->try_num; i++) {
			sprintf(buf, "Try %d : %s %dS%dB\n\r", i+1, bbdata->guess[i],
                    bbdata->score[i][0], bbdata->score[i][1]);
			send_to_char(buf, ch);
		}

	}
	
	return(TRUE);
}
