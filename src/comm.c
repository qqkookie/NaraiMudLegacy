/* ************************************************************************
   *  file: comm.c , Communication module.                   Part of DIKUMUD *
   *  Usage: Communication, central game loop.                               *
   *  Copyright (C) 1990, 1991 - see 'license.doc' for complete information. *
   *  All Rights Reserved                                                    *
   *  Using *any* part of DikuMud without having read license.doc is         *
   *  violating our copyright.
   ************************************************************************* */

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <sys/time.h>
#include <signal.h>
#include <errno.h>
#include <setjmp.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <arpa/telnet.h>
#include <netdb.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <termio.h>


/* 
#include <sys/types.h> 
#ifndef HAVE_TERMIOS_H
#include <termio.h>
#include <sys/ioctl.h>
#else
#include <termios.h>
#endif 
#include <sys/wait.h>
*/ 

#include "char.h"
#include "object.h"
#include "global.h"
#include "comm.h"
#include "gamedb.h"

#ifndef DFLT_DIR
#define DFLT_DIR "lib"	/* default directory */
#endif	/* DFLT_DIR */
#ifndef DFLT_PORT
#define DFLT_PORT 4001	/* default port */
#endif	/* DFLT_PORT */

#define MAX_NAME_LENGTH 15
#define MAX_HOSTNAME 256
#define OPT_USEC 200000	/* time delay corresponding to 4 passes/sec */
#define MAXFDALLOWED 200
/* #define MAXOCLOCK 1200 */ 
#define MAXOCLOCK 720		/* NOTE: Auto save in 3 min */

#define PULSE_MOBILE            39
#define PULSE_MOBILE2           13 

#define PULSE_ZONE              240
#define PULSE_VIOLENCE          12
#define WAIT_SEC                4
#define WAIT_ROUND              4 


/* local globals */

struct descriptor_data *descriptor_list, *next_to_process;
jmp_buf env;

extern int errno; 
extern int baddoms;
extern char baddomain[][32];	/* NOTE: defined in "act.wizard.c"  */

int s;
int slow_death = 0;	/* Shut her down, Martha, she's sucking mud */
int shutdowngame = 0;	/* clean shutdown */
int no_specials = 0;	/* Suppress ass. of special routines */
int boottime;

int no_echo = 0;

/* reboot_time = 24 hour */
u_long reboot_time = 86400;

int maxdesc, avail_descs;
int tics = 0;		/* for extern checkpointing */
int nonewplayers = 0;
int nostealflag = 0;
int nokillflag = 0;
int noshoutflag = 0;
int nochatflag = 0;
int nodisarmflag = 0;	/* for disarm .Chase written */
int noenchantflag = 0;	/* noenchant!! */
int regen_percent = 50;
int regen_time_percent = 66;
int regen_time = 200;
/* NOTE: Logging level. 0..5 : See command_interpreter() in interpreter.c */ 
/* NOTE: Default loglevel 1 -> 2 (include say, tell,.. ) */ 
int loglevel = 2;	/* NOTE: 1 = log chat/shout */
int noplayingflag = 0;	/* NOTE: Prohibit entering game to play      */ 

struct descriptor_data *xo;

int process_output(struct descriptor_data *t);
int get_from_q(struct txt_q *queue, char *dest); 
void close_socket(struct descriptor_data *d);

/* ******************************************************************* */
/* NOTE: Use ring buffer for output to improve performance by 
      saving up to 98% of malloc() and free() call for output queue buffer. */
/* Additionally, it will reduce "down" due to queue malloc() failure.	*/

#define TB_RING_MAX	128 	/* NOTE: Number of ring buffer elements */
#define TB_BSIZ		128	/* NOTE: Size of each output text buffer.  */
/* NOTE: 
   TB_RING_MAX may needs performance tunning. More player, larger TB_RING_MAX. 
     It is related to sum of output queue length average of all players.
     Watch ring 'hit ratio' in hourly user list log. Over 90%-95% is OK.
   TB_BSIZ will hardly need to adjust.
     It is related to upper size averge of each output text in queue.
 */ 
/* NOTE: Output queue element + static buffer pair */
struct txt_ring_st {
    struct txt_block block;
    char   txt_buf[TB_BSIZ];
};

static struct txt_ring_st txt_pool[TB_RING_MAX];  /* ring element pool */
static struct txt_block *txt_ring[TB_RING_MAX];	  /* real ring buffer  */
static int    txt_ring_head, txt_ring_tail;
/* NOTE: for ring hit ratio statistics only */
static int    tb_pooled = 0 , tb_alloced = 0 ; 

/* *********************************************************************
   *  main game loop and related stuff				       *
   ********************************************************************* */

int main(int argc, char **argv)
{
    int port;
    char buf[512];
    char *dir;
    void run_the_game(int port);

    close(0);
    close(1);
    port = DFLT_PORT;
    dir = DFLT_DIR;
    /* NOTE: set site you wish to refuse playing. Currently no such site */
    strcpy(baddomain[0], "99.99.99.99");
    baddoms = 1;
    if (argc == 2) {
	if (!ISDIGIT(*argv[1])) {
	    fprintf(stderr, "Usage: %s [ port # ]\n", argv[0]);
	    exit(0);
	}
	else if ((port = atoi(argv[1])) <= 1024) {
	    fprintf(stderr, "Illegal port #\n");
	    exit(0);
	}
    }
    sprintf(buf, "Running game on port %d.", port);
    log(buf);
    if (chdir(dir) < 0) {
	perror("chdir");
	exit(0);
    }
    srandom(boottime = time(0));
    run_the_game(port);
    return (0);
}

/* Init sockets, run game, and cleanup sockets */
void run_the_game(int port)
{
    int i;
    void signal_setup(void);
    void saveallplayers(void);
    void log_pid(void);
    int init_socket(int port);
    void no_echo_local(int fd);
    void game_loop(int s);
    void transall(int room);
    void close_sockets(int s);
    extern void boot_db(void);

    descriptor_list = NULL;
    log("Signal trapping.");
    signal_setup();
    log("Opening mother connection.");
    s = init_socket(port);
    boot_db();
    /* NOTE: write out my pid to "lib/pid" file */
    log_pid();

    /* NOTE: init ring of free output buffer before entering game   */
    /*	     At first, free ring is FULL state. (all buffers are free)  */
    for( i = 0; i < TB_RING_MAX; i++ ) {
	txt_pool[i].block.text = txt_pool[i].txt_buf;
	txt_pool[i].block.next = NULL ;
	txt_ring[i] = &(txt_pool[i].block) ;
    }
    txt_ring_head = 0 ;
    txt_ring_tail = TB_RING_MAX -1 ;

    log("Entering game loop.");
    no_echo_local(s); 
    game_loop(s);
    log("DOWN??????????SAVE ALL CHARS???????");
    transall(3001);
    saveallplayers();
    close_sockets(s);
    shutdown(s, 2);
    log("Normal termination of game.");
}

void transall(int room)
{
    struct descriptor_data *pt, *npt;

    for (pt = descriptor_list; pt; pt = npt) {
	npt = pt->next;
	if (pt->connected == CON_PLYNG)
	    if (pt->character) {
		char_from_room(pt->character);
		char_to_room(pt->character, real_room(room));
		send_to_char( "\n\r\n\r\n\r\n\rShutdown System has transferred you!\n\r\n\r", pt->character);
		process_output(pt);
	    }
    }
}

void saveallplayers()
{
    struct descriptor_data *pt, *npt;

    for (pt = descriptor_list; pt; pt = npt) {
	npt = pt->next;
	if (pt->connected == CON_PLYNG)
	    if (pt->character) {
		save_char(pt->character);
		stash_char(pt->character);
	    }
    }
}

void game_loop(int s)
{
    fd_set input_set, output_set, exc_set;
    struct timeval last_time, now, timespent, timeout, null_time;
    static struct timeval opt_time;
    char comm[MAX_INPUT_LENGTH];
    struct descriptor_data *point, *next_point;
    int mask;
    char prmpt[32];
    int process_input(struct descriptor_data *t);
    int new_descriptor(int s);
    void freaky(struct descriptor_data *d);
    void no_echo_telnet(struct descriptor_data *d);
    void echo_telnet(struct descriptor_data *d);
    void zapper(void);
    void heart_beat(void);
    void release_player( struct descriptor_data *d);
    struct timeval timediff(struct timeval *a, struct timeval *b);

    extern void string_add(struct descriptor_data *d, char *str);
    extern void show_string(struct descriptor_data *d, char *input);
    extern void nanny(struct descriptor_data *d, char *arg);

    null_time.tv_sec = 0;
    null_time.tv_usec = 0;
    opt_time.tv_usec = OPT_USEC;	/* Init time values */
    opt_time.tv_sec = 0;
    gettimeofday(&last_time, (struct timezone *) 0);

    maxdesc = s;
    avail_descs = MAXFDALLOWED;

    mask = sigmask(SIGUSR1) | sigmask(SIGUSR2) | sigmask(SIGINT) |
	sigmask(SIGBUS) | sigmask(SIGSEGV) |
	sigmask(SIGPIPE) | sigmask(SIGALRM) | sigmask(SIGTERM) |
	sigmask(SIGURG) | sigmask(SIGXCPU) | sigmask(SIGHUP);

    /* Main loop */
    while (!shutdowngame) {
	/* Check what's happening out there */
	FD_ZERO(&input_set);
	FD_ZERO(&output_set);
	FD_ZERO(&exc_set);
	FD_SET(s, &input_set);
	for (point = descriptor_list; point; point = point->next) {
	    FD_SET(point->descriptor, &input_set);
	    FD_SET(point->descriptor, &exc_set);
	    FD_SET(point->descriptor, &output_set);
	}

	/* check out the time */
	gettimeofday(&now, (struct timezone *) 0);
	timespent = timediff(&now, &last_time);
	timeout = timediff(&opt_time, &timespent);
	last_time.tv_sec = now.tv_sec + timeout.tv_sec;
	last_time.tv_usec = now.tv_usec + timeout.tv_usec;

	if (last_time.tv_usec >= 1000000) {
	    last_time.tv_usec -= 1000000;
	    last_time.tv_sec++;
	}
	sigsetmask(mask);
	if (select(maxdesc + 1, &input_set, &output_set, &exc_set, &null_time) < 0) {
	    perror("Select poll");
	    exit(0);
	}
	if (select(0, (fd_set *) 0, (fd_set *) 0, (fd_set *) 0, &timeout) < 0) {
	    perror("Select sleep");
	    exit(1);
	}

	sigsetmask(0);
	/* Respond to whatever might be happening */
	/* New connection? */
	if (FD_ISSET(s, &input_set))
	    if (new_descriptor(s) < 0)
		perror("New connection");

	/* kick out the freaky folks */
	for (point = descriptor_list; point; point = point->next) {
	    if (FD_ISSET(point->descriptor, &exc_set)) 
		freaky(point);
	}
	for (point = descriptor_list; point; point = point->next) {
	    if (FD_ISSET(point->descriptor, &input_set))
		if (process_input(point) < 0)
		    freaky(point); 
	}
	/* process_commands; */
	for (point = descriptor_list; point; point = next_to_process) {
	    next_to_process = point->next;
	    if ((--(point->wait) <= 0) && get_from_q(&point->input, comm)) {
	    /* NOTE: in noecho mode, we need carrige return and line feed  */
		if (no_echo)
		    SEND_TO_Q("\r\n", point);
		echo_telnet(point);
		no_echo = 0;
		if (point->character && point->connected == CON_PLYNG &&
		    point->character->specials.was_in_room != NOWHERE) {
		    if (point->character->in_room != NOWHERE)
			char_from_room(point->character);
		    char_to_room(point->character,
				 point->character->specials.was_in_room);
		    point->character->specials.was_in_room = NOWHERE;
		    act("$n has returned.", TRUE, point->character, 0, 0, TO_ROOM);
		}
		point->wait = 1;
		if (point->character)
		    point->character->specials.timer = 0;
		point->prompt_mode = 1;
		if (point->str)
		    string_add(point, comm);
		else if (!point->connected) {
		    if (point->showstr_point)
			show_string(point, comm);
		    else {
			point->wait += command_interpreter(point->character, comm);
			++point->ncmds;
		    }
		}
		else {
		    nanny(point, comm);
		    if (no_echo) {
			no_echo_telnet(point);
		    }
		}
	    }
	}
	for (point = descriptor_list; point; point = point->next) {
	    if (FD_ISSET(point->descriptor, &exc_set)) 
		freaky(point);
	}
	for (point = descriptor_list; point; point = point->next) {
	    xo = point;
	    if (FD_ISSET(point->descriptor, &output_set) && point->output.head)
		if (process_output(point) < 0)
		    freaky(point);
		else
		    point->prompt_mode = 1;
	}
	/* give the people some prompts */
	for (point = descriptor_list; point; point = next_point) { 
	    /* NOTE: point may be freed in close_socket() */
	    next_point = point->next;
	    /* NOTE: freaky connection or leaving player done here. */
	    if (point->connected == CON_CLOSE) {
		FD_CLR(point->descriptor, &input_set );
		FD_CLR(point->descriptor, &output_set );

		close_socket(point);
		continue;
	    }
	    /* NOTE: Auto crash save moved to heart_beat()  */ 

	    if (point->prompt_mode) {
		if (point->str)
		    write_to_descriptor(point->descriptor, "] ");
		else if (!point->connected)
		    if (point->showstr_point)
			write_to_descriptor(point->descriptor, "*** Press return ***");
		    else { 

#ifdef  DISPLAY_DAMAGE
    /* NOTE: Display damages in each turn of violence.
	Used for mob balancing. Don't activate this in production version. */
	    /* NOTE: FOR TESTING:*/
	    static int myhit= 0 , yourhit= 0;
	    char buf[200];
	    if (GET_LEVEL(point->character) < IMO 
		&& (point->character->specials.fighting) ) {
		sprintf(buf, "(%d,%d) ",
		myhit - GET_HIT(point->character),
		yourhit - GET_HIT(point->character->specials.fighting));
		myhit = GET_HIT(point->character);
		yourhit = GET_HIT(point->character->specials.fighting);
		write_to_descriptor(point->descriptor, buf);
	    }
#endif		/* DISPLAY_DMAGE */

			sprintf(prmpt, "\[%d,%d,%d] ",
			    GET_HIT(point->character),
		    GET_MANA(point->character), GET_MOVE(point->character));
			write_to_descriptor(point->descriptor, prmpt);
			/* follows are for auto-rescue and auto-assist */
/*
	   if(point->character->master->specials.fighting &&
		   !point->character->specials.fighting &&
		   IS_SET(point->character->specials.act, PLR_AUTO_ASSIST)) {
	       do_assist(point->character,"",0);
	   } 
 */
		    }
		point->prompt_mode = 0;
	    }
	}
	setjmp(env);

	/* NOTE: heartbeat stuff to heart_beat() */
	zapper();
	heart_beat();
	tics++;
    }
}

/* NOTE: NEW! handle heartbeat stuff. It was part of game_loop() */
/* Note: pulse now changes every 1/4 sec  */
void heart_beat(void)
{ 
    void record_player_number(void);

    extern void zone_update(void);
    extern void mobile_activity(void);
    extern void mobile_activity2(void);
    extern void perform_violence(void);
    extern void weather_and_time(int mode);
    extern void affect_update(void);	/* In handler.c */
    extern void point_update(void);	/* In point.c */ 
    extern void	House_save_all(void);
    static pulse = 0;
    struct descriptor_data *point;

    pulse = ++pulse % 2400 ;

    if (!(pulse % PULSE_ZONE)) {
	zone_update();
	record_player_number();
    }
    if (!(pulse % PULSE_MOBILE)) {
	mobile_activity();
    }
    /* added by atre */
    else if (!(pulse % PULSE_MOBILE2)) {
	mobile_activity2();
    }

    if (!(pulse % PULSE_VIOLENCE)) {
	perform_violence();
    }
    if (!(pulse % (SECS_PER_MUD_HOUR * 4 + 1))) {
	weather_and_time(1);
	affect_update();
	point_update();
    }

    /* NOTE: Auto Crash save code moved form game_loop() */
    for (point = descriptor_list; point; point = point->next ) { 
	if ((point->descriptor == ( pulse % MAXOCLOCK)) 
	    && (point->connected == CON_PLYNG) && (point->character) 
	    && (!point->original) && (GET_LEVEL(point->character) < IMO)) { 
	    save_char(point->character);
	    stash_char(point->character);
	}
    }
    if ((!pulse % MAXOCLOCK))
	House_save_all();
}

void zapper(void)
{			/* auto shutdown */
    /* int dow,tod,t; */
    time_t t;
    static int flag = 0;

    t = time(0) - boottime;
    if (t > reboot_time + 60 && flag) {
	send_to_all("SHUTDOWN MESSAGE FROM SYSTEM!!!!!!\n\r\n\r\n\r");
	send_to_all("Shutdown immediately!\n\r");
	shutdowngame = 1;
    }
    if (t > reboot_time && !flag) {
	flag = 1;
	send_to_all("SHUTDOWN MESSAGE FROM SYSTEM!!!!!!\n\r\n\r\n\r");
	send_to_all("Shutdown after 1 minute!\n\r");
    }
}

/* ******************************************************************
   *  general utility stuff (for local use)                   *
   ****************************************************************** */

int get_from_q(struct txt_q *queue, char *dest)
{
    struct txt_block *tmp;

    /* Q empty? */
    if (!queue->head)
	return (0);

    tmp = queue->head;
    strcpy(dest, queue->head->text);
    queue->head = queue->head->next;

    /* NOTE:  If txt_block is from ring, return it to free ring  */
    if ( tmp >= &(txt_pool[0].block) 
	    && tmp <= &(txt_pool[TB_RING_MAX-1].block) ){
	txt_ring_tail = (txt_ring_tail+1) % TB_RING_MAX;
	txt_ring[txt_ring_tail] = tmp;
	return(1);
    }

/* NOTE IMPORTANT : Debugging purpose: remove when done.
   Next IF statement is to check run time behavior of get_from_q(). 
   I experinced some random segment violation error
   while creating brand-new player   */
    if (tmp->text)
	free(tmp->text);
    else
	log("OOPS! please tell this to Cookie");
    free(tmp);

    return (1);
}

void write_to_q(char *txt, struct txt_q *queue)
{
    struct txt_block *new;
    int len;

    len = strlen(txt) ;
    /* NOTE : If free ring is not empty, allocate from ring */
    if ( len+1 < TB_BSIZ && txt_ring_head != txt_ring_tail ) {
	tb_pooled++;
	new = txt_ring[txt_ring_head];
	txt_ring_head = (txt_ring_head+1) % TB_RING_MAX;
    }
    else {
	tb_alloced++;
	CREATE(new, struct txt_block, 1);
	CREATE(new->text, char, len + 1);
    }

    strcpy(new->text, txt);

    /* Q empty? */
    if (!queue->head) {
	new->next = NULL;
	queue->head = queue->tail = new;
    }
    else {
	queue->tail->next = new;
	queue->tail = new;
	new->next = NULL;
    }
}

/* NOTE: Input buffer size reduced from MAX_STRING_LENGTH to MAX_BUFSIZ */
int process_input(struct descriptor_data *t)
{
    int sofar, thisround, begin, i, k, flag;
    char tmp[MAX_INPUT_LENGTH + 2], buffer[MAX_INPUT_LENGTH + 60];
    int lic, j;

    sofar = 0;
    flag = 0;
    begin = strlen(t->ibuf);
    /* Read in some stuff */
    do {
	if ((thisround = read(t->descriptor, t->ibuf + begin + sofar,
			      MAX_BUFSIZ - (begin + sofar) - 1)) > 0)
	    sofar += thisround;
	else if (thisround < 0)
	    if (errno != EWOULDBLOCK) {
		perror("Read1 - ERROR");
		return (-1);
	    }
	    else
		break;
	else {
	    log("EOF encountered on socket read.");
	    return (-1);
	}
    } while (!ISNEWL(*(t->ibuf + begin + sofar - 1)));

    *(t->ibuf + begin + sofar) = 0;

    /* if no newline is contained in input, return without proc'ing */
    for (i = begin; !ISNEWL(*(t->ibuf + i)); i++)
	if (!*(t->ibuf + i))
	    return (0);

    /* input contains 1 or more newlines; process the stuff */
    for (i = 0, k = 0; *(t->ibuf + i);) {
	if (!ISNEWL(*(t->ibuf + i)) &&
	    !(flag = (k >= (MAX_INPUT_LENGTH - 2)))) {
	    if (*(t->ibuf + i) == '\b') {	/* backspace */
		if (k) {	/* more than one char ? */
		    if (*(tmp + --k) == '$')
			k--;
		}
		i++;	/* no or just one char.. Skip backsp */
	    }
	    /* strange stuff, i can't understand! */
	    else if (*(t->ibuf + i) == -1) {
		if (*(t->ibuf + i) && !ISNEWL(*(t->ibuf + i)))
		    i++;
		if (*(t->ibuf + i) && !ISNEWL(*(t->ibuf + i)))
		    i++;
		if (*(t->ibuf + i) && !ISNEWL(*(t->ibuf + i)))
		    i++;
	    }
	    else if (!isascii(*(t->ibuf + i)) && !ISNEWL(*(t->ibuf + i + 1))) {
		*(tmp + k++) = *(t->ibuf + i++);
		*(tmp + k++) = *(t->ibuf + i++);
	    }		/* hangul routine */
	    /* strange stuff, i can't understand! */
	    else if (isascii(*(t->ibuf + i)) && isprint(*(t->ibuf + i))) {
		if ((*(tmp + k) = *(t->ibuf + i)) == '$')
		    *(tmp + ++k) = '$';
		k++;
		i++;
	    }
	    else
		i++;
	}
	else {
	    *(tmp + k) = 0;
	    if (tmp[0] == '!') {
		if (tmp[1] == '!' || tmp[1] == 0) {
		    lic = t->last_input_count;
		    if (lic == 0)
			lic = MAX_LAST_INPUT_COUNT - 1;
		    else
			lic--;
		    strcpy(tmp, t->last_input[lic]);
		}
		else {
		    lic = t->last_input_count - 1;
		    for (j = 0; j < MAX_LAST_INPUT_COUNT; j++, lic--) {
			if (lic < 0)
			    lic = MAX_LAST_INPUT_COUNT - 1;
			if (strncmp(t->last_input[lic], &tmp[1],
				    strlen(&tmp[1])) == 0) {
			    strcpy(tmp, t->last_input[lic]);
			    break;
			}
		    }
		}
	    }
	    else {
		strcpy(t->last_input[t->last_input_count], tmp);
		t->last_input_count++;
		t->last_input_count %= 10;
	    }
	    write_to_q(tmp, &t->input);
	    if (t->snoop.snoop_by) {
		write_to_q("% ", &t->snoop.snoop_by->desc->output);
		write_to_q(tmp, &t->snoop.snoop_by->desc->output);
		write_to_q("\n\r", &t->snoop.snoop_by->desc->output);
	    }
	    if (flag) {
		sprintf(buffer, "Line too long. Truncated to:\n\r%s\n\r", tmp);
		if (write_to_descriptor(t->descriptor, buffer) < 0)
		    return (-1);
		/* skip the rest of the line */
		for (; !ISNEWL(*(t->ibuf + i)); i++) ;
	    }
	    /* find end of entry */
	    for (; ISNEWL(*(t->ibuf + i)); i++) ;
	    /* squelch the entry from the buffer */
	    /* for (squelch = 0;; squelch++) if ((*(t->ibuf + squelch) =
	       *(t->ibuf + i + squelch)) == '\0') break; */
	    /* forbid infinite loop */
	    strncpy(t->ibuf, t->ibuf + i, MAX_BUFSIZ);
	    t->ibuf[MAX_BUFSIZ - 1] = 0;
	    k = 0;
	    i = 0;
	}
    }
    return (1);
}

int process_output(struct descriptor_data *t)
{
    char i[MAX_STRING_LENGTH*2 ];

    if (!t->prompt_mode && !t->connected)
	if (write_to_descriptor(t->descriptor, "\r\n") < 0)
	    return (-1);
    while (get_from_q(&t->output, i)) {
	if (t->snoop.snoop_by) {
	    write_to_q("% ", &t->snoop.snoop_by->desc->output);
	    write_to_q(i, &t->snoop.snoop_by->desc->output);
	}
	if (write_to_descriptor(t->descriptor, i))
	    return (-1);
    }
    if (!t->connected && !(t->character && !IS_NPC(t->character) &&
			   IS_SET(t->character->specials.act, PLR_COMPACT)))
	if (write_to_descriptor(t->descriptor, "\r\n") < 0)
	    return (-1);
    return (1);
}

int write_to_descriptor(int desc, char *txt)
{
    int sofar, thisround, total;

    /* int *nil; */

    total = strlen(txt);
    if (total == 0)
	return 0;
    sofar = 0;
    do {
    /* 
	ioctl(desc,SIOCGPGRP,&w);
	ioctl(desc,SIOCATMARK,&x);
	ioctl(desc,SIOCGHIWAT,&y);
	ioctl(desc,SIOCGLOWAT,&z);
	sprintf(buf,"IOCTL: %d %d %x %x",w,x,y,z);
    */
	thisround = write(desc, txt + sofar, total - sofar);
	if (thisround < 0) {
	    perror("Write to socket");
	    return (-1);
	}
	else {
	    sofar += thisround;
	}
    } while (sofar < total);
    return (0);
}

/* Empty the queues before closing connection */
void flush_queues(struct descriptor_data *d)
{
    char dummy[MAX_STRING_LENGTH + 1];

    while (get_from_q(&d->output, dummy)) ;
    while (get_from_q(&d->input, dummy)) ;
}

struct timeval timediff(struct timeval *a, struct timeval *b)
{
    struct timeval rslt, tmp;

    tmp = *a;

    if ((rslt.tv_usec = tmp.tv_usec - b->tv_usec) < 0) {
	rslt.tv_usec += 1000000;
	--(tmp.tv_sec);
    }
    if ((rslt.tv_sec = tmp.tv_sec - b->tv_sec) < 0) {
	rslt.tv_usec = 0;
	rslt.tv_sec = 0;
    }
    return (rslt);
}

/* ******************************************************************
   *  socket handling						    *
   ****************************************************************** */

int init_socket(int port)
{
    int s;
    char *opt;
    char hostname[MAX_HOSTNAME + 1];
    struct sockaddr_in sa;
    struct hostent *hp;
    struct linger ld;

    bzero(&sa, sizeof(struct sockaddr_in));

    gethostname(hostname, MAX_HOSTNAME);
    log(hostname);
    hp = gethostbyname(hostname);
    if (hp == NULL) {
	perror("gethostbyname");
	exit(1);
    }
    sa.sin_family = hp->h_addrtype;
    sa.sin_port = htons(port);
    s = socket(AF_INET, SOCK_STREAM, 0);
    if (s < 0) {
	perror("Init-socket");
	exit(1);
    }
    if (setsockopt(s, SOL_SOCKET, SO_REUSEADDR,
		   (char *) &opt, sizeof(opt)) < 0) {
	perror("setsockopt REUSEADDR");
	exit(1);
    }
    ld.l_onoff = 0;
    ld.l_linger = 0;
    if (setsockopt(s, SOL_SOCKET, SO_LINGER, &ld, sizeof(ld)) < 0) {
	perror("setsockopt LINGER");
	exit(1);
    }
    if (bind(s, (struct sockaddr *) &sa, sizeof(sa)) < 0) {
	perror("bind");
	close(s);
	exit(1);
    }
    listen(s, 3);
    return (s);
}

int new_connection(int s)
{
    struct sockaddr_in isa;
    void nonblock(int s);

    /* struct sockaddr peer; */
    int i;
    int t;

    i = sizeof(isa);
    getsockname(s, (struct sockaddr *) &isa, &i);
    if ((t = accept(s, (struct sockaddr *) &isa, &i)) < 0) {
	perror("Accept");
	return (-1);
    }
    nonblock(t);
    return (t);
}

int new_descriptor(int s)
{
    int desc;
    struct descriptor_data *newd;
    int i, size;
    struct sockaddr_in sock;
    struct hostent  *h_ent; 

    int unfriendly_domain(char *h);

    if ((desc = new_connection(s)) < 0)
	return (-1);

    CREATE(newd, struct descriptor_data, 1);
    size = sizeof(sock);
    if (getpeername(desc, (struct sockaddr *) &sock, &size) < 0) {
	perror("getpeername");
	/* NOTE: prevent Null d->host */
	strcpy(newd->host, "Unknown Host");
    }
    else {
	/* strncpy(newd->host, (char *) inet_ntoa(sock.sin_addr), 16); */

	/* NOTE: Show domain name instead of numeric IP addr, if possible.*/
	if (( h_ent = gethostbyaddr((char*) &sock.sin_addr, 
		sizeof(struct in_addr), AF_INET)))
	    strncpy(newd->host, h_ent->h_name, sizeof(newd->host )-1);
	else
	    strncpy(newd->host, (char *) inet_ntoa(sock.sin_addr), 16);
	(newd->host)[ sizeof(newd->host)-1 ] = '\0';
    }

    if ((maxdesc + 1) >= avail_descs) {
	write_to_descriptor(desc, "Sorry.. The game is full...\n\r");
	write_to_descriptor(desc, "Wait at least one minute before retrying.\n\r");
	close(desc);
	flush_queues(newd);
	free((char *) newd);
	return (0);
    }
    else if (desc > maxdesc)
	maxdesc = desc;
    if (unfriendly_domain(newd->host)) {
	write_to_descriptor(desc,
		      "Sorry, the game is unavailable from your site.\n\r");
	fprintf(stderr, "Reject from %s\n", newd->host);
	close(desc);
	flush_queues(newd);
	free((char *) newd);
	return (-1);
    }
    /* cyb
    if(more_than_20(newd)) {
	write_to_descriptor(desc, 
	    "Sorry, the site for foreign players are full.\n\r");
	    fprintf(stderr,"Reject from %s\n",newd->host);
	    close(desc); 
	    flush_queues(newd);
	    free((char *)newd);
	    return(-1);
    } */

    /* init desc data */
    newd->ncmds = 0;
    newd->contime = time(0);
    newd->descriptor = desc;
    /* NOTE: initial STATE(d) is wait for player name */
    newd->connected = CON_NME;
    newd->wait = 1;
    newd->prompt_mode = 0;
    *newd->ibuf = '\0';
    newd->str = 0;
    newd->showstr_head = 0;
    newd->showstr_point = 0;
    for (i = 0; i < MAX_LAST_INPUT_COUNT; i++)
	newd->last_input[i][0] = '\0';
    newd->last_input_count = 0;
    newd->output.head = NULL;
    newd->input.head = NULL;
    newd->character = 0;
    newd->original = 0;
    newd->snoop.snooping = 0;
    newd->snoop.snoop_by = 0;

    /* NOTE: clear string data of descriptor */
    newd->pwd[0] = 0;
    newd->name[0] = 0;
    newd->scratch[0] = 0;

    /* prepend to list */ 
    newd->next = descriptor_list;
    descriptor_list = newd;

    SEND_TO_Q("\r\n", newd);
    SEND_TO_Q(GREETINGS, newd);
    SEND_TO_Q("\r\n", newd);
    if (nonewplayers) {
	SEND_TO_Q("WARNING:\n\r", newd);
	SEND_TO_Q("No NEW characters are being accepted right now.\n\r\n\r", newd);
    }
    SEND_TO_Q("By what name do you wish to be known? ", newd);

    return (0);
}

void nonblock(int s)
{
    if (fcntl(s, F_SETFL, FNDELAY) == -1) {
	perror("Noblock");
	exit(1);
    }
}

void close_sockets(int s)
{ 
    log("Closing all sockets.");
    while (descriptor_list)
	close_socket(descriptor_list);
    close(s);
}

/* NOTE: Freaky connection will be marked to be close_socket() */
void freaky(struct descriptor_data *d)
{
    char buf[MAX_BUFSIZ];
    extern void return_original(struct char_data *ch);
    extern void free_char(struct char_data *ch);

    /* NOTE: Stop log it at normal log level */
    if ( loglevel == 4 ) {
	sprintf(buf, "Kicked out a freaky folk. %s (%d) st: %d." ,
		d->name, d->descriptor, d->connected); 
	log(buf); 
    }
    if (d->character)
	return_original(d->character);
    
    if(d->character) {
	if( d->connected == CON_PLYNG ) {
	    save_char(d->character);
	    stash_char(d->character);
	} 
	else {
	    free_char(d->character);
	    d->character = 0;
	}
    }
    d->connected = CON_CLOSE; /* NOTE: mark for closing connection */
}

void close_socket(struct descriptor_data *d)
{
    struct descriptor_data *tmp;
    char buf[MAX_BUFSIZ];

    if (!d)
	return;

    /* NOTE: First send out pending output in output queue.     */
    /* Flush queue next. Then close descriptor last. */
    process_output(d);
    flush_queues(d);
    close(d->descriptor);
    if (d->descriptor == maxdesc)
	--maxdesc;
    /* Forget snooping */
    if (d->snoop.snooping)
	d->snoop.snooping->desc->snoop.snoop_by = 0;
    if (d->snoop.snoop_by) {
	send_to_char("Your victim is no longer among us.\n\r", d->snoop.snoop_by);
	d->snoop.snoop_by->desc->snoop.snooping = 0;
    } 

    /* NOTE: d->character is not freed here, but freaky() */ 
    if ( d->character ) {
	if ( d->character->desc ) {
	    act("$n was kicked out of here and lost $s link. ",
		TRUE, d->character, 0, 0, TO_ROOM);
	    sprintf(buf, "Losing player: %s.", GET_NAME(d->character));
	    d->character->desc = 0;
	}
	else {
	    act("$n has lost $s link.", TRUE, d->character, 0, 0, TO_ROOM);
	    sprintf(buf, "Closing link to: %s.", GET_NAME(d->character));
	}
	d->character = 0;
    }
    else if (d->name[0]) 
	    sprintf(buf, "Close connection to: %s.", d->name); 
    else
	strcpy(buf, "Losing descriptor without name.");

    log(buf);

    if (next_to_process == d)	/* to avoid crashing the process loop */
	next_to_process = next_to_process->next;

    if (d == descriptor_list)	/* this is the head of the list */
	descriptor_list = descriptor_list->next;
    else {		/* This is somewhere inside the list */
	/* Locate the previous element */
	for (tmp = descriptor_list; (tmp->next != d) && tmp;
	     tmp = tmp->next) ;

	tmp->next = d->next;
    }
    /* NOTE: free() half-done string */
    if (d->str && *(d->str)) {
	free(*(d->str));
	*(d->str) = 0;
    }
    
    if (d->showstr_head) {
	free(d->showstr_head);
    }
    free(d);
}


#ifdef NO_DEF
/* NOTE: This is_korean() is hopelessly out of date, now    */
/* NOTE: more_than_20() is obsolete and meaningless */

#define KAIST "143.248."
#define POSTECH "141.223."
#define HANA "128.134."
#define KOTEL "147.6."
#define KREO "134.75."
#define SEOUL "147.46"
#define KAIST2 "137.68."
#define GOLDSTAR "156.147"

int is_korean(struct descriptor_data *d)
{
    if (strncmp(d->host, KAIST, strlen(KAIST) - 1)
	&& strncmp(d->host, POSTECH, strlen(POSTECH) - 1)
	&& strncmp(d->host, HANA, strlen(HANA) - 1)
	&& strncmp(d->host, KREO, strlen(KREO) - 1)
	&& strncmp(d->host, SEOUL, strlen(SEOUL) - 1)
	&& strncmp(d->host, KAIST2, strlen(KAIST2) - 1)
	&& strncmp(d->host, KOTEL, strlen(KOTEL) - 1))
	return 0;
    else
	return 1;
}

#define MAX_FOREIGN 20

int more_than_20(struct descriptor_data *d)
{
    int nooff = 0;
    struct descriptor_data *tmp;
    int t, tod, dow;

    t = time(0) + 32400;
    tod = (t % 86400) / 3600;
    dow = (t / 86400) % 7;
    if (dow < 4 || tod <= 9 || tod >= 17)
	return 0;
    for (tmp = descriptor_list; tmp; tmp = tmp->next) {
	if (!is_korean(tmp))
	    nooff++;
    }
    if (nooff > MAX_FOREIGN)
	return 1;
    else
	return 0;
}
#endif				/* NO_DEF */ 

int unfriendly_domain(char *h)
{
    int i;

    for (i = 0; i < baddoms; ++i) {
	/* NOTE: Need fixing. h, baddomain[] is DNS addr format */
	if (strncmp(h, baddomain[i], strlen(baddomain[i])) == 0) {
	    return (1);
	}
    }
    return (0);
}

void record_player_number()
{
    char line[MAX_STRING_LENGTH]; 
    time_t t;
    static int lasttics = 0; 
    char *tmstr;
    extern int list_users(struct char_data *ch, char *line );

    /* NOTE: do this *ONCE* on beginning of each hour,  */
    if (((time(0) %3600) > 100 ) || ( tics < ( lasttics + 1000 ))) 
	return;
    lasttics = tics ; 

    /* NOTE: Log current day and time.	*/
    t = time(0);
    tmstr = asctime(localtime(&t));
    tmstr[16] = '\0';
    sprintf(line, "=======  Current Time is %s.  =======\n", tmstr);

    list_users(NULL, line+strlen(line) );
    log(line);

    /* NOTE: Meaningless..
    sprintf(line,"from korea: %d from abroad %d", in_d, out_d ); log(line);
    */
    t = 30 + time(0) - boottime;
    sprintf(line, "Running time  %2ld:%02ld", t / 3600, (t % 3600) / 60);
    log(line);
    /* NOTE: For statistics only */
    sprintf(line, "Txt block: ring: %d,  malloc: %d. Hit ratio: %d%%",
	    tb_pooled, tb_alloced, (100 * tb_pooled)/(tb_pooled+tb_alloced+1));
    log(line); 
}


void signal_setup(void)
{
    struct itimerval itime;
    struct timeval interval;
    /* NOTE: Signal handlers */
    void logsig(int sig);
    void hupsig(int sig);
    void shutdown_request(int sig);
    void checkpointing(int sig);

    // siginterrupt(SIGHUP, 1);
    signal(SIGHUP, hupsig);
    // siginterrupt(SIGINT, 1);
    signal(SIGINT, hupsig);
    /* NOTE: catch SIGSEGV, SIGBUS. Report place and time of error */
    // siginterrupt(SIGSEGV, 1);
    signal(SIGSEGV, hupsig);
    // siginterrupt(SIGBUS, 1);
    signal(SIGBUS, hupsig);
    signal(SIGPIPE, SIG_IGN);
    signal(SIGALRM, logsig);
    /* NOTE: Don't catch SIGTERM. Terminate immediately */
    /* siginterrupt(SIGTERM, 1); signal(SIGTERM, hupsig); */
    signal(SIGUSR2, shutdown_request);

    interval.tv_sec = 900;	/* 15 minutes */
    interval.tv_usec = 0;
    itime.it_interval = interval;
    itime.it_value = interval;
    setitimer(ITIMER_VIRTUAL, &itime, 0);
    signal(SIGVTALRM, checkpointing);
}

void checkpointing(int sig)
{ 
    struct char_data *ch, *t;
    int i, j ;

    if (!tics) {
	log("CHECKPOINT shutdown: tics not updated");
	saveallplayers();
	abort();
    }
    else
	tics = 0;
    log("checkpointing");

    /* NOTE: FOR DEBUGGING: Check validity of char list */
    for( ch = character_list, i = 0 ; ch ; ch = ch->next, i++ ) 
	for( t = character_list, j = 0;  j < i ; t = t->next, j++ )
	    if ( ch == t ) {
		char buf[MAX_BUFSIZ];
		log("FATAL: Loop in charter list!!");
		sprintf(buf, " Char %x (%d-th and %d-th) %s ", 
			(unsigned) ch, i, j, GET_NAME(ch)); 
	    } 
}

void shutdown_request(int sig)
{
    send_to_all("Shut down signal has been received.\n\r");
    log("Received USR2 - shutdown request");
    shutdowngame = 1;
}

/* kick out players etc */
void hupsig(int sig)
{
    char s[MAX_BUFSIZ];

    signal(sig, SIG_IGN);
    /* NOTE : Is anybody there? */
    if (xo && xo->character && xo->connected == CON_PLYNG) {
	sprintf(s, "SIG: %d %s(%d) %s in %s(%d)", sig, GET_NAME(xo->character),
		GET_LEVEL(xo->character), xo->host, world[xo->character->in_room].name,
		xo->character->in_room);
	log(s);
	/* NOTE: Don't save players potentially danerous signals */
	if (sig == SIGHUP || sig == SIGINT || sig == SIGQUIT || sig == SIGTERM)
	    saveallplayers();
    }
    else
	log("Signal received. But I'm confused.");
    /* NOTE: BSD signal don't restore sig handler */
    signal(sig, hupsig);
    longjmp(env, -1);
} 

void logsig(int sig)
{
    log("Signal received. Ignoring.");
    signal(sig, logsig);
}

/* NOTE: I need to know pid , write my pid number to "lib/pid" file   */
void log_pid(void)
{
    FILE *pidfile;
    char buf[80];
    pid_t pid = getpid();

    sprintf(buf, "INFO: PID: %d\n", pid);
    log(buf);

    if ((pidfile = fopen("pid", "w"))) {
	fprintf(pidfile, " %d \n", pid);
	fclose(pidfile);
    }
}

/* NOTE: OLD WAIT_STATE() was macro. now use this function. */
int WAIT_STATE(struct char_data *ch, int cycle) 
{
#define WAIT_STATE_MACRO(ch, cycle)  \
	((ch)->desc ? (ch)->desc->wait = (cycle) : 0 )
    return(WAIT_STATE_MACRO(ch, cycle));
}

/*********************************************************************
	This is for try to control the echo on telnet
	running on diku system:
	First call the no_echo_local when boot
	then call (no_)echo_telnet in the function nanny in the
	interpreter.c

	(don't ask me to explain you all the system,
	it's just shit but it works!)

	copyright (C) Big Cat rochat_p@elde.epfl.ch
			      balelec@eldi.epfl.ch

	Was performed on MUME : lbdsun4.epfl.ch 4000
				128.178.77.5 4000

(This head in comments is part of the code, and can't be removed !!!)
**********************************************************************/

#ifndef HAVE_TERMIOS_H

void echo_local(int fd)
{
    struct termio io;

    ioctl(fd, TCGETA, &io);
    io.c_line = 0;
    io.c_lflag |= ECHO;
    ioctl(fd, TCSETA, &io);
}

void no_echo_local(int fd)
{
    struct termio io;

    ioctl(fd, TCGETA, &io);
    io.c_cc[VMIN] = 1;
    io.c_cc[VTIME] = 0;
    io.c_line = 0;
    io.c_lflag &= ~ECHO;
    ioctl(fd, TCSETA, &io);
}

#else

void echo_local(int fd)
{
    struct termios io;

    ioctl(fd, TIOCGETA, &io);
    io.c_lflag |= ECHO;
    ioctl(fd, TIOCSETA, &io);
}

void no_echo_local(int fd)
{
    struct termios io;

    ioctl(fd, TIOCGETA, &io);
    io.c_cc[VMIN] = 1;
    io.c_cc[VTIME] = 0;
    io.c_lflag &= ~ECHO;
    ioctl(fd, TIOCSETA, &io);
}

#endif 		/* HAVE_TERMIOS_H */

void no_echo_telnet(struct descriptor_data *d)
{
    char buf[5];

    sprintf(buf, "%c%c%c", IAC, WILL, TELOPT_ECHO);
    write(d->descriptor, buf, 3);
}

void echo_telnet(struct descriptor_data *d)
{
    char buf[5];

    sprintf(buf, "%c%c%c", IAC, WONT, TELOPT_ECHO);
    write(d->descriptor, buf, 3);
}

