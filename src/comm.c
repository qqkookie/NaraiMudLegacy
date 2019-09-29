/* ************************************************************************
*  file: comm.c , Communication module.                   Part of DIKUMUD *
*  Usage: Communication, central game loop.                               *
*  Copyright (C) 1990, 1991 - see 'license.doc' for complete information. *
*  All Rights Reserved                                                    *
*  Using *any* part of DikuMud without having read license.doc is         *
*  violating our copyright.
************************************************************************* */

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <netinet/in.h>
#include <netdb.h>
#include <string.h>
#include <sys/time.h>
#include <fcntl.h>
#include <unistd.h>
#include <signal.h>
#include <setjmp.h>

#include "structs.h"
#include "utils.h"
#include "comm.h"
#include "interpreter.h"
#include "handler.h"
#include "db.h"

#ifndef DFLT_DIR
#define DFLT_DIR "lib"        /* default port */
#endif  
#ifndef DFLT_PORT
#define DFLT_PORT 4000        /* default port */
#endif  

#define MAX_NAME_LENGTH 15
#define MAX_HOSTNAME 256
#define MAXFDALLOWED 200
#define MAXOCLOCK 1200

extern int errno;

extern int no_echo;
extern struct room_data *world;    /* In db.c */
extern int top_of_world;            /* In db.c */
extern struct time_info_data time_info;  /* In db.c */
extern char help[];

extern int no_echo_telnet( struct descriptor_data *d );
extern int echo_telnet( struct descriptor_data *d );

/* local globals */

struct descriptor_data *descriptor_list, *next_to_process;
jmp_buf env;

int s;
int slow_death = 0;  /* Shut her down, Martha, she's sucking mud */
int shutdowngame = 0;    /* clean shutdown */
int no_specials = 0; /* Suppress ass. of special routines */
int boottime;
int maxdesc, avail_descs;
int tics = 0;        /* for extern checkpointing */
int nonewplayers = 0;
int nostealflag = 1;
int nokillflag = 1;
int noshoutflag = 0;
int nochatflag = 0;
int nologin = 0;	/* cyb : no log in more */
struct descriptor_data *xo;

void logsig(int sig);
void hupsig(int sig);
void shutdown_request(int sig);
void checkpointing(int sig);

int get_from_q(struct txt_q *queue, char *dest);
/* write_to_q is in comm.h for the macro */
int run_the_game(int port);
int game_loop(int s);
int init_socket(int port);
int new_connection(int s);
int new_descriptor(int s);
int process_output(struct descriptor_data *t);
int process_input(struct descriptor_data *t);
void close_sockets(int s);
void close_socket(struct descriptor_data *d);
struct timeval timediff(struct timeval *a, struct timeval *b);
void flush_queues(struct descriptor_data *d);
void nonblock(int s);
void parse_name(struct descriptor_data *desc, char *arg);
void freaky(struct descriptor_data *d);


/* extern fcnts */

struct char_data *make_char(char *name, struct descriptor_data *desc);
void boot_db(void);
void zone_update(void);
void record_player_number(void);
int is_korean(struct descriptor_data *d);
void affect_update( void ); /* In spells.c */
void point_update( void );  /* In limits.c */
void free_char(struct char_data *ch);
void log(char *str);
void mobile_activity(void);
void string_add(struct descriptor_data *d, char *str);
void perform_violence(void);
void stop_fighting(struct char_data *ch);
void show_string(struct descriptor_data *d, char *input);
void save_char(struct char_data *ch, sh_int load_room);

/* *********************************************************************
*  main game loop and related stuff               *
********************************************************************* */

int main(int argc, char **argv)
{
	char *get_version() ;	/* return version */
    int port;
    char buf[512];
    char *dir;

    close(0); close(1);
    port = DFLT_PORT;
    dir = DFLT_DIR;
    strcpy(baddomain[0],"131.215");
    baddoms=1;
    if (argc == 2) {
        if (!ISDIGIT(*argv[1])) {
            fprintf(stderr, "Usage: %s [ port # ]\n", 
                    argv[0]);
            exit(1);
        } else if ((port = atoi(argv[1])) <= 1024) {
            fprintf(stderr,"Illegal port #\n");
            exit(1);
        }
    }
    sprintf(buf, "Running game on port %d.", port);
    log(buf);
	
    sprintf(buf, "Version: %s\n", get_version()) ;
    log(buf);
    if (chdir(dir) < 0) {
        perror("chdir");
        exit(1);
    }
    srandom(boottime=time(0));
    run_the_game(port);
    return(0);
}

extern void no_echo_local(int);

/* Init sockets, run game, and cleanup sockets */
int run_the_game(int port)
{
    struct descriptor_data *point,*next_point;
    void signal_setup(void);
    void saveallplayers();

    descriptor_list = NULL;
    log("Signal trapping.");
    signal_setup();
    log("Opening mother connection.");
    s = init_socket(port);
    boot_db();
    log("Entering game loop.");
    no_echo_local( s );
    game_loop(s);
    saveallplayers();
    sleep(3);
    close_sockets(s);
    sleep(5);
    shutdown(s,2);	/* shutdown socket */
    log("Normal termination of game.");

    return 0;
}

void saveallplayers()
{
    struct descriptor_data *pt, *npt;
	char	buf[BUFSIZ] ;

    for (pt = descriptor_list; pt; pt = npt) {
        npt = pt->next;
        if(pt->connected == CON_PLYNG)
            if(pt->character){
                /* send_to_char("Auto Save.\n\r", pt->character); */
                save_char(pt->character,NOWHERE);
                if (stash_char(pt->character,0) < 0) {
					sprintf(buf, "auto: saving %s failed", GET_NAME(pt->character));
					log(buf) ;
                }
            }
    }
}

// global data
extern struct char_data *character_list;

// criminal tag
void process_player_tag()
{
    static struct char_data *i;

    for (i = character_list; i; i = i->next) {
		// 1/30확률이거나 saint인 경우에 무시
		//
		if (number(1,50) == 1 || GET_ALIGNMENT(i)  > 620) {
            if (!IS_NPC(i) && IS_SET(i->specials.act, PLR_CRIMINAL)) {
                REMOVE_BIT(i->specials.act, PLR_CRIMINAL);
                send_to_char("Your fault is forgotten.\n\r",i);
            }
		}
	}
}

extern void move_stashfile(char *victim);

int game_loop(int s)
{
    int tmp_room, old_len;
    fd_set input_set, output_set, exc_set;
    struct timeval last_time, now, timespent, timeout, null_time;
    static struct timeval opt_time;
    char comm[MAX_INPUT_LENGTH];
    struct descriptor_data *t, *point, *next_point;
    sigset_t newmask, oldmask;
    int pulse = 0, xoclock = 0;
    char prmpt[32];
    int  ctmp ;
    void apart_save();

    null_time.tv_sec = 0;
    null_time.tv_usec = 0;
    opt_time.tv_usec = OPT_USEC;  /* Init time values */
    opt_time.tv_sec = 0;
    gettimeofday(&last_time, (struct timezone *) 0);

    maxdesc = s;
    avail_descs = MAXFDALLOWED;

    sigemptyset(&newmask);
    sigaddset(&newmask, SIGUSR1);
    sigaddset(&newmask, SIGUSR2);
    sigaddset(&newmask, SIGINT);
    sigaddset(&newmask, SIGBUS);
    sigaddset(&newmask, SIGSEGV);
    sigaddset(&newmask, SIGPIPE);
    sigaddset(&newmask, SIGALRM);
    sigaddset(&newmask, SIGTERM);
    sigaddset(&newmask, SIGURG);
    sigaddset(&newmask, SIGXCPU);
    sigaddset(&newmask, SIGHUP);
/*  mask = sigmask(SIGUSR1) | sigmask(SIGUSR2) | sigmask(SIGINT) |
    sigmask(SIGBUS ) | sigmask(SIGSEGV) |
    sigmask(SIGPIPE) | sigmask(SIGALRM) | sigmask(SIGTERM) |
    sigmask(SIGURG ) | sigmask(SIGXCPU) | sigmask(SIGHUP);
*/
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
        if (sigprocmask(SIG_BLOCK, &newmask, &oldmask) <0) {
			perror("SIG_BLOCK error");
			exit(-1);
        }
/*    sigsetmask(mask); */
        if(select(maxdesc+1, &input_set, &output_set, &exc_set, &null_time) < 0) {
            perror("Select poll");
            exit(1);
        }
        if(select(0, (fd_set *) 0, (fd_set *) 0, (fd_set *) 0, &timeout) < 0) {
            perror("Select sleep");
            exit(1);
        }
        /*  sigsetmask(0); */
        if (sigprocmask(SIG_SETMASK, &oldmask, NULL) <0) {
            perror("SIG_SETMASK error");
            exit(-1);
        }

        /* Respond to whatever might be happening */
        /* New connection? */
        if (FD_ISSET(s, &input_set))
            if (new_descriptor(s) < 0)
                perror("New connection");

        /* kick out the freaky folks */
        for (point = descriptor_list; point; point = next_point) {
            next_point = point->next;   
            if (FD_ISSET(point->descriptor, &exc_set)) {
                freaky(point);
                FD_CLR(point->descriptor, &input_set);
                FD_CLR(point->descriptor, &output_set);
                log("Kicked out a freaky folk.\n\r");
                close_socket(point);
            }
        }
        for (point = descriptor_list; point; point = next_point) {
            next_point = point->next;
            if (FD_ISSET(point->descriptor, &input_set))
                if (process_input(point) < 0) {
                    FD_CLR(point->descriptor, &input_set);
                    FD_CLR(point->descriptor, &output_set);
                    if (point->connected == CON_PLYNG) {
                        stash_char(point->character,0);
                        move_stashfile(point->character->player.name);
                    }
                    close_socket(point);
                }
        }
        /* process_commands; */
        for (point = descriptor_list; point; point = next_to_process) {
            next_to_process = point->next;
            if ((--(point->wait) <= 0) && get_from_q(&point->input, comm)) {
                if ( no_echo )
                    SEND_TO_Q( "\n", point );
                echo_telnet( point ); no_echo = 0;
                if (point->character && point->connected == CON_PLYNG &&
                    point->character->specials.was_in_room !=  NOWHERE) {
                    if (point->character->in_room != NOWHERE)
                        char_from_room(point->character);
                    char_to_room(point->character, 
                                 point->character->specials.was_in_room);
                    point->character->specials.was_in_room = NOWHERE;
                    act("$n has returned.",  TRUE, point->character, 0, 0, TO_ROOM);
                }
                point->wait = 1;
                if (point->character)
                    point->character->specials.timer = 0;
                point->prompt_mode = 1;
                if (point->str)
                    string_add(point, comm);
                else if (!point->connected)  {
                    if (point->showstr_point)
                        show_string(point, comm);
                    else {
                        point->wait += command_interpreter(point->character, comm);
                        ++point->ncmds;
                    }
                }
                else	{
                    nanny(point, comm); 
                    if ( no_echo )	{
                        no_echo_telnet( point );
                    }
                }
            }
        }
        for (point = descriptor_list; point; point = next_point) {
            next_point = point->next;   
            if (FD_ISSET(point->descriptor, &exc_set)) {
                freaky(point);
                FD_CLR(point->descriptor, &input_set);
                FD_CLR(point->descriptor, &output_set);
                log("Kicked out a freaky folk.\n\r");
                close_socket(point);
            }
        }
        for (point = descriptor_list; point; point = next_point) {
            next_point = point->next;
            xo=point;
            if (FD_ISSET(point->descriptor, &output_set) && point->output.head) 
                if(process_output(point) < 0){
                    FD_CLR(point->descriptor, &input_set);
                    FD_CLR(point->descriptor, &output_set);
                    if (point->connected == CON_PLYNG) {
                        stash_char(point->character,0);
                        move_stashfile(point->character->player.name);
                    }
                    close_socket(point);
                } else
                    point->prompt_mode = 1;
        }
        /* give the people some prompts */
        ++xoclock;
        if(xoclock == MAXOCLOCK) xoclock=0;
        for (point = descriptor_list; point; point = point->next){
            if((!point->connected) && (!point->original) &&
               (GET_LEVEL(point->character) < IMO) && (point->descriptor==xoclock)){
                save_char(point->character,NOWHERE);
                stash_char(point->character,0);
            }
            if (point->prompt_mode) {
                if (point->str)
                    write_to_descriptor(point->descriptor, "] ");
                else if (!point->connected)
                    if (point->showstr_point)
                        write_to_descriptor(point->descriptor,
                                            "*** Press return ***");
                    else {
                        sprintf(prmpt, "\[%d,%d,%d] ",
                                GET_HIT(point->character),GET_MANA(point->character),
                                GET_MOVE(point->character));
                        write_to_descriptor(point->descriptor, prmpt);
                    }
                point->prompt_mode = 0;
            }
        }
        setjmp(env);

        /* handle heartbeat stuff */
        /* Note: pulse now changes every 1/20 sec  */

        pulse++;
        if (!(pulse % PULSE_ZONE)) {          /* called every 10 sec */
            zone_update();
        }
        if (!(pulse % PULSE_MOBILE)) {        /* called every 10 sec */
            mobile_activity();
        }
        if (!(pulse % PULSE_VIOLENCE)) {      /* called every 2 sec */
            perform_violence();
        }
        ctmp = (SECS_PER_MUD_HOUR * PASSES_PER_SEC);   /* for 1 minute */
        
        if (!(pulse % ctmp)){
            weather_and_time(1);
            affect_update();
            point_update();
            check_idle_player();
            
            record_player_number();
            saveallplayers();
            
            gomulsang_save();
            apart_save();
                        
            zapper(); 

			process_player_tag();		// criminal tag
        }
		if (shutdowngame) {
            saveallplayers();
            
            gomulsang_save();
			apart_save();
		}

        if (!(pulse % (ctmp * 15))) {  // 15 minutes
            announce_notify();
        }

        if (pulse >= 600 * 60 * PASSES_PER_SEC) {
            pulse = 0;
        }
        tics++;
    }
}

#define 	REBOOT_TIME  20
zapper()      /* auto shutdown */
{
	static int count = 0;
	extern int shutdowngame;

	
	if (++count > 60*REBOOT_TIME-1) {
		send_to_all("Rebooting.. come back in a minute or two.\r\n");
	}
	if (count > 60*REBOOT_TIME+1) {
		shutdowngame = 1;
	}
	return;
}

// 15분 마다 불리는 코드 
// 
announce_notify()      /* 공지사항을 notify(jmjeong) */
{
	extern char announce[];
    char *select_announce();
	static int count = 0;
	extern long jackpot;

	char buf[MAX_STRING_LENGTH] = "\n\r[새소식] ";     

    strncat(buf, select_announce(), MAX_STRING_LENGTH);
    strncat(buf, "\n\r자세한 정보는 NEWS나 게시판을 참조하세요\n\r",
            MAX_STRING_LENGTH);
    
    send_to_all(buf);
    
	// 4 시간이 지났고, 돈의 액수가 2M가 넘었다면 player에게 공지. ^^
	if (++count > 4*4 && jackpot > 2*1000000 ) {
		sprintf(buf, "\n\r현재 Casino의 Jackpot 금액은 %ld입니다.\n\r", jackpot);
		send_to_all(buf);
	}

	return;
}

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
    if( strncmp(d->host,KAIST,strlen(KAIST)-1) 
        && strncmp(d->host,POSTECH,strlen(POSTECH)-1) 
        && strncmp(d->host,HANA,strlen(HANA)-1) 
        && strncmp(d->host,KREO,strlen(KREO)-1) 
        && strncmp(d->host,SEOUL,strlen(SEOUL)-1) 
        && strncmp(d->host,KAIST2,strlen(KAIST2)-1) 
        && strncmp(d->host,KOTEL,strlen(KOTEL)-1) )
        return 0;
    else return 1;
}

#define MAX_FOREIGN 20

int more_than_20(struct descriptor_data *d)
{
    int nooff=0;
    struct descriptor_data *tmp;
    int t, tod, dow;

    t=time(0)+32400;
    tod=(t%86400)/3600;
    dow=(t/86400)%7;
    if(dow<4 || tod<=9 || tod >= 17) return 0;
    for(tmp=descriptor_list;tmp;tmp=tmp->next){
        if(!is_korean(tmp)) nooff++;
    }
    if(nooff>MAX_FOREIGN) return 1;
    else return 0;
}

extern char *connected_types[];

void record_player_number()
{
    char line[256];
    int t, tod, in_d=0, out_d=0;
    struct descriptor_data *d;
    int m=0,n=0;
    static int most=0;
    t=time(0)+32400;       /* 32400 is 9 hour */
                           /* 86400 is a day */
    tod = (t%3600);
    if( tod > 3500 ) {     /* in every 51-59 */
        line[0]=0;
        for (d=descriptor_list;d;d=d->next) {
            ++m;
            if (d->original){
                sprintf(line+strlen(line),"%3d%2d:",d->descriptor,
                        d->original->specials.timer);
                sprintf(line+strlen(line), "%-14s%2d ",
                        GET_NAME(d->original),GET_LEVEL(d->original));
            } else if (d->character){
                sprintf(line+strlen(line),"%3d%2d:",d->descriptor,
                        d->character->specials.timer);
                sprintf(line+strlen(line), "%-14s%2d ",
                        (d->connected==CON_PLYNG) ? GET_NAME(d->character) : "Not in game",
                        GET_LEVEL(d->character));
            } else
                sprintf(line+strlen(line), "%3d%9s%10s ",
                        d->descriptor,"  UNDEF  ",connected_types[d->connected]);
            sprintf(line+strlen(line),"%-15s",d->host);
            if(is_korean(d)) in_d++;
            else out_d++;
            if(!(n%2)){
                strcat(line,"|");
            } else {
                log(line);
                line[0]=0;
            }
            ++n;
        }
        if(n%2){
            log(line);
        }

        if(m > most) most=m;
        sprintf(line,"%s%d/%d active connections",
                (n%2) ? "\n\r" : "",m,most);
        log(line);
        sprintf(line,"from korea: %d from abroad %d", in_d, out_d );
        log(line);
        t=30+time(0)-boottime;
        sprintf(line,"Running time %d:%02d",t/3600,(t%3600)/60);
        log(line);
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
        return(0);

    tmp = queue->head;
    strcpy(dest, queue->head->text);
    queue->head = queue->head->next;

    free(tmp->text);
    free(tmp);

    return(1);
}

void write_to_q(char *txt, struct txt_q *queue)
{
    struct txt_block *new;

    CREATE(new, struct txt_block, 1);
    CREATE(new->text, char, strlen(txt) + 1);

    strcpy(new->text, txt);

    /* Q empty? */
    if (!queue->head) {
        new->next = NULL;
        queue->head = queue->tail = new;
    } else {
        queue->tail->next = new;
        queue->tail = new;
        new->next = NULL;
    }
}

struct timeval timediff(struct timeval *a, struct timeval *b)
{
    struct timeval rslt, tmp;

    tmp = *a;

    if ((rslt.tv_usec = tmp.tv_usec - b->tv_usec) < 0)
    {
        rslt.tv_usec += 1000000;
        --(tmp.tv_sec);
    }
    if ((rslt.tv_sec = tmp.tv_sec - b->tv_sec) < 0)
    {
        rslt.tv_usec = 0;
        rslt.tv_sec =0;
    }
    return(rslt);
}

/* Empty the queues before closing connection */
void flush_queues(struct descriptor_data *d)
{
    char dummy[MAX_STRING_LENGTH];

    while (get_from_q(&d->output, dummy));
    while (get_from_q(&d->input, dummy));
}

/* ******************************************************************
*  socket handling               *
****************************************************************** */

int init_socket(int port)
{
    int s;
    char *opt;
    char hostname[MAX_HOSTNAME+1];
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
    sa.sin_port  = htons(port);
    s = socket(AF_INET, SOCK_STREAM, 0);
    if (s < 0) {
        perror("Init-socket");
        exit(1);
    }
    if (setsockopt (s, SOL_SOCKET, SO_REUSEADDR,
                    (char *) &opt, sizeof (opt)) < 0) 
    {
        perror ("setsockopt REUSEADDR");
        exit (1);
    }
    ld.l_onoff = 0;
    ld.l_linger = 0;
    if (setsockopt(s, SOL_SOCKET, SO_LINGER, &ld, sizeof(ld)) < 0) {
        perror("setsockopt LINGER");
        exit(1);
    }
    if (bind(s,(struct sockaddr *) &sa, sizeof(sa)) < 0) {
        perror("bind");
        close(s);
        exit(1);
    }
    listen(s, 3);
    return(s);
}

int new_connection(int s)
{
    struct sockaddr_in isa;
    /* struct sockaddr peer; */
    int i;
    int t;
    char buf[100];

    i = sizeof(isa);
    getsockname(s, (struct sockaddr *) &isa, &i);
    if ((t = accept(s, (struct sockaddr *) &isa, &i)) < 0) {
        perror("Accept");
        return(-1);
    }
    nonblock(t);
    return(t);
}

int new_descriptor(int s)
{
    int desc;
    struct descriptor_data *newd;
    int i,size;
    struct sockaddr_in sock;
    struct hostent *from;
    char buf[10];

    if ((desc = new_connection(s)) < 0)
        return (-1);
    CREATE(newd, struct descriptor_data, 1);
    size = sizeof(sock);
    if (getpeername(desc, (struct sockaddr *) &sock, &size) < 0) {
        perror("getpeername");
        *newd->host = 0;
    } else {
        strncpy(newd->host,(char *)inet_ntoa(sock.sin_addr),16);
    }
    if((maxdesc+1) >= avail_descs){
        write_to_descriptor(desc, "Sorry.. The game is full...\n\r");
        write_to_descriptor(desc, "Wait at least one minute before retrying.\n\r");
        close(desc);
        flush_queues(newd);
        free((char *)newd);
        return(0);
    } else
        if (desc > maxdesc)
            maxdesc = desc;
    if(unfriendly_domain(newd->host)) {
        write_to_descriptor(desc,
                            "Sorry, the game is unavailable from your site.\n\r");
        fprintf(stderr,"Reject from %s\n",newd->host);
        close(desc);
        flush_queues(newd);
        free((char *)newd);
        return(-1);
    }
    /* cyb  if(more_than_20(newd)) {
       write_to_descriptor(desc,
       "Sorry, the site for foreign players are full.\n\r");
       fprintf(stderr,"Reject from %s\n",newd->host);
       close(desc);
       flush_queues(newd);
       free((char *)newd);
       return(-1);
       } */
    /* init desc data */
    newd->ncmds = 0; newd->contime = time(0);
    newd->descriptor = desc;
    newd->connected = 1;
    newd->wait = 1;
    newd->prompt_mode = 0;
    *newd->buf = '\0';
    newd->str = 0;
    newd->showstr_head = 0;
    newd->showstr_point = 0;
    *newd->last_input= '\0';
    newd->output.head = NULL;
    newd->input.head = NULL;
    newd->next = descriptor_list;
    newd->character = 0;
    newd->original = 0;
    newd->snoop.snooping = 0;
    newd->snoop.snoop_by = 0;

    /* prepend to list */

    descriptor_list = newd;

    SEND_TO_Q(GREETINGS, newd);
    if(nonewplayers){
        SEND_TO_Q("WARNING:\n\r",newd);
        SEND_TO_Q("No NEW characters are being accepted right now.\n\r\n\r",newd);
    }
    if(nologin){
        SEND_TO_Q("Notice:\n\r",newd);
        SEND_TO_Q("System is wating for rebooting, try later. \n\r\n\r",newd);
    }
    else {
        SEND_TO_Q("By what name do you wish to be known? ", newd);
    }

    return(0);
}

unfriendly_domain(h)
    char *h;
{
    int i;

    for(i=0;i<baddoms;++i){
        if(strncmp(h,baddomain[i],strlen(baddomain[i]))==0){
            return(1);
        }
    }
    return(0);
}

int process_output(struct descriptor_data *t)
{
    char i[MAX_STRING_LENGTH + 1];

    if(!t->prompt_mode && !t->connected)
        if (write_to_descriptor(t->descriptor, "\n\r") < 0)
            return(-1);
    while (get_from_q(&t->output, i)) {  
        if(t->snoop.snoop_by) {
            write_to_q("% ",&t->snoop.snoop_by->desc->output);
            write_to_q(i,&t->snoop.snoop_by->desc->output);
        }
        if (write_to_descriptor(t->descriptor, i))
            return(-1);
    }
    if (!t->connected && !(t->character && !IS_NPC(t->character) && 
                           IS_SET(t->character->specials.act, PLR_COMPACT)))
        if (write_to_descriptor(t->descriptor, "\n\r") < 0)
            return(-1);
    return(1);
}

#include <sys/ioctl.h>

int write_to_descriptor(int desc, char *txt)
{
    int sofar, thisround, total;
    int w,x,y,z;
    char buf[256];

    total = strlen(txt);
    sofar = 0;
    do {
/*
  ioctl(desc,SIOCGPGRP,&w);
  ioctl(desc,SIOCATMARK,&x);
  ioctl(desc,SIOCGHIWAT,&y);
  ioctl(desc,SIOCGLOWAT,&z);
  sprintf(buf,"IOCTL: %d %d %x %x",w,x,y,z);
  log(buf);
*/  thisround = write(desc, txt + sofar, total - sofar);
 if (thisround < 0) {
     perror("Write to socket");
     return(-1);
 } else if(thisround==0) {
     log("Zero in write_to_descriptor");
 } else {
     sofar += thisround;
 }
    } while (sofar < total);
    return(0);
}

int process_input(struct descriptor_data *t)
{
    int sofar, thisround, begin, squelch, i, k, flag;
    char tmp[MAX_INPUT_LENGTH+2], buffer[MAX_INPUT_LENGTH + 60];

    sofar = 0;
    flag = 0;
    begin = strlen(t->buf);
    /* Read in some stuff */
    do {
        if ((thisround = read(t->descriptor, t->buf + begin + sofar, 
                              MAX_STRING_LENGTH - (begin + sofar) - 1)) > 0)
            sofar += thisround;    
        else
            if (thisround < 0)
                if(errno != EWOULDBLOCK) {
                    perror("Read1 - ERROR");
                    return(-1);
                } else
                    break;
            else {
                log("EOF encountered on socket read.");
                return(-1);
            }
    }
    while (!ISNEWL(*(t->buf + begin + sofar - 1)));  
    *(t->buf + begin + sofar) = 0;

    /* if no newline is contained in input, return without proc'ing */
    for (i = begin; !ISNEWL(*(t->buf + i)); i++)
        if (!*(t->buf + i))
            return(0);

    /* input contains 1 or more newlines; process the stuff */
    for (i = 0, k = 0; *(t->buf + i);) {
        if (!ISNEWL(*(t->buf + i)) && !(flag = (k >= (MAX_INPUT_LENGTH - 2))))
            if(*(t->buf + i) == '\b')   /* backspace */
                if (k)  /* more than one char ? */
                {
                    if (*(tmp + --k) == '$')
                        k--;        
                    i++;
                }
                else
                    i++;  /* no or just one char.. Skip backsp */
            else 
                if (*(t->buf+i) == -1 )
                {
                    if(*(t->buf+i)&&!ISNEWL(*(t->buf+i))) i++;
                    if(*(t->buf+i)&&!ISNEWL(*(t->buf+i))) i++;
                    if(*(t->buf+i)&&!ISNEWL(*(t->buf+i))) i++;
                }
                else 
                    if (!isascii(*(t->buf+i)) && !ISNEWL(*(t->buf+i+1)))
                    {
                        *(tmp + k++) = *(t->buf + i++);
                        *(tmp + k++) = *(t->buf + i++);
                    }     /* hangul routine */
                    else
                        if (isascii(*(t->buf + i)) && isprint(*(t->buf + i)))
                        {
                            /* trans char, double for '$' (printf)  */
                            if ((*(tmp + k) = *(t->buf + i)) == '$')
                                *(tmp + ++k) = '$';
                            k++;
                            i++;
                        }
                        else
                            i++;
        else {
            *(tmp + k) = 0;
            if(*tmp == '!')
                strcpy(tmp,t->last_input);
            else
                strcpy(t->last_input,tmp);
            write_to_q(tmp, &t->input);
            if(t->snoop.snoop_by) {
                write_to_q("% ",&t->snoop.snoop_by->desc->output);
                write_to_q(tmp,&t->snoop.snoop_by->desc->output);
                write_to_q("\n\r",&t->snoop.snoop_by->desc->output);
            }
            if (flag) {
                sprintf(buffer, "Line too long. Truncated to:\n\r%s\n\r", tmp);
                if (write_to_descriptor(t->descriptor, buffer) < 0)
                    return(-1);
                /* skip the rest of the line */
                for (; !ISNEWL(*(t->buf + i)); i++);
            }
            /* find end of entry */
            for (; ISNEWL(*(t->buf + i)); i++);
            /* squelch the entry from the buffer */
            for (squelch = 0;; squelch++)
                if ((*(t->buf + squelch) = 
                     *(t->buf + i + squelch)) == '\0')
                    break;
            k = 0;
            i = 0;
        }
    }
    return(1);
}

void close_sockets(int s)
{
    log("Closing all sockets.");
    while(descriptor_list)
        close_socket(descriptor_list);
    close(s);
}

void close_socket(struct descriptor_data *d)
{
    struct affected_type *af;
    struct descriptor_data *tmp;
    char buf[100];

    close(d->descriptor);
    flush_queues(d);
    if (d->descriptor == maxdesc)
        --maxdesc;
    /* Forget snooping */
    if (d->snoop.snooping)
        d->snoop.snooping->desc->snoop.snoop_by = 0;
    if (d->snoop.snoop_by) {
        send_to_char("Your victim is no longer among us.\n\r",d->snoop.snoop_by);
        d->snoop.snoop_by->desc->snoop.snooping = 0;
    }
    if (d->character)
        if (d->connected == CON_PLYNG) {
            save_char(d->character, NOWHERE);
            act("$n has lost $s link.", TRUE, d->character, 0, 0, TO_ROOM);
            sprintf(buf, "Closing link to: %s.", GET_NAME(d->character));
            log(buf);
            d->character->desc = 0;
        } else {
            sprintf(buf, "Losing player: %s.", GET_NAME(d->character));
            log(buf);
            free_char(d->character);
        }
    else
        log("Losing descriptor without char.");

    if (next_to_process == d)    /* to avoid crashing the process loop */
        next_to_process = next_to_process->next;   

    if (d == descriptor_list) /* this is the head of the list */
        descriptor_list = descriptor_list->next;
    else  /* This is somewhere inside the list */
    {
        /* Locate the previous element */
        for (tmp = descriptor_list; (tmp->next != d) && tmp; 
             tmp = tmp->next);
    
        tmp->next = d->next;
    }
    if (d->showstr_head)
        free(d->showstr_head);
    free(d);
}

void nonblock(int s)
{
    if (fcntl(s, F_SETFL, O_NONBLOCK) == -1)
    {
        perror("Noblock");
        exit(1);
    }
}



/* ****************************************************************
*  Public routines for system-to-player-communication        *
**************************************************************** */



void send_to_char(char *messg, struct char_data *ch)
{
    if ( ch && ch->desc && messg)
        write_to_q(messg, &ch->desc->output);
}

void send_to_char_han(char *msgeng, char *msghan, struct char_data *ch)
{
    
    if (ch && ch->desc && msgeng && msghan ) {
        if ((IS_SET(ch->specials.act, PLR_KOREAN)))	/* korean */
            write_to_q(msghan, &ch->desc->output);
        else						/* english */
            write_to_q(msgeng, &ch->desc->output);
    }
}

void send_to_all(char *messg)
{
    struct descriptor_data *i;

    if (messg)
        for (i = descriptor_list; i; i = i->next)
            if (!i->connected)
                write_to_q(messg, &i->output);
}

void send_to_outdoor(char *messg)
{
    struct descriptor_data *i;

    if (messg)
        for (i = descriptor_list; i; i = i->next)
            if (!i->connected)
                if (OUTSIDE(i->character))
                    write_to_q(messg, &i->output);
}

void send_to_except(char *messg, struct char_data *ch)
{
    struct descriptor_data *i;

    if (messg && ch)
        for (i = descriptor_list; i; i = i->next)
            if (ch->desc != i && !i->connected)
                write_to_q(messg, &i->output);
}

void send_to_room(char *messg, int room)
{
    struct char_data *i;

    if (messg)
        for (i = world[room].people; i; i = i->next_in_room)
            if (i->desc)
                write_to_q(messg, &i->desc->output);
}

void send_to_room_except(char *messg, int room, struct char_data *ch)
{
    struct char_data *i;

    if (messg)
        for (i = world[room].people; i; i = i->next_in_room)
            if (i != ch && i->desc)
                write_to_q(messg, &i->desc->output);
}

void send_to_room_except_two
(char *messg, int room, struct char_data *ch1, struct char_data *ch2)
{
    struct char_data *i;

    if (messg)
        for (i = world[room].people; i; i = i->next_in_room)
            if (i != ch1 && i != ch2 && i->desc)
                write_to_q(messg, &i->desc->output);
}

/* higher-level communication */

void act(char *str, int hide_invisible, struct char_data *ch,
         struct obj_data *obj, void *vict_obj, int type)
{
    register char *strp, *point, *i;
    struct char_data *to;
    char buf[MAX_STRING_LENGTH];
    char buf2[BUFSIZ];

    if (!str)
        return;
    if (!*str)
        return;

    if (type == TO_VICT)
        to = (struct char_data *) vict_obj;
    else if (type == TO_CHAR)
        to = ch;
    else
        to = world[ch->in_room].people;

    for (; to; to = to->next_in_room)
    {
        if (to->desc && ((to != ch) || (type == TO_CHAR)) &&  
            (CAN_SEE(to, ch) || !hide_invisible) && AWAKE(to) &&
            !((type == TO_NOTVICT) && (to == (struct char_data *) vict_obj)))
        {
            for (strp = str, point = buf;;)
                if (*strp == '$')
                {
                    switch (*(++strp))
                    {
                    case 'n': i = PERS(ch, to); break;
                    case 'N': i = PERS((struct char_data *) vict_obj, to); break;
                    case 'm': i = HMHR(ch); break;
                    case 'M': i = HMHR((struct char_data *) vict_obj); break;
                    case 's': i = HSHR(ch); break;
                    case 'S': i = HSHR((struct char_data *) vict_obj); break;
                    case 'e': i = HSSH(ch); break;
                    case 'E': i = HSSH((struct char_data *) vict_obj); break;
                    case 'o': i = OBJN(obj, to); break;
                    case 'O': i = OBJN((struct obj_data *) vict_obj, to); break;
                    case 'p': i = OBJS(obj, to); break;
                    case 'P': i = OBJS((struct obj_data *) vict_obj, to); break;
                    case 'a': i = SANA(obj); break;
                    case 'A': i = SANA((struct obj_data *) vict_obj); break;
                    case 'T': i = (char *) vict_obj; break;
                    case 'F': i = fname((char *) vict_obj); break;
                    case '$': i = "$"; break;
                    default:
                        log("Illegal $-code to act():");
                        log(str);
                        break;
                    }
                    if ( i ==  NULL ) {
                        log("act: null pointer encountered") ;
                        sprintf(buf2, "name: %s to %s\n", GET_NAME(ch), GET_NAME(to));
                        log(buf2);
                        return ;
                    }
                    while (*point = *(i++))
                        ++point;
                    ++strp;
                }
                else if (!(*(point++) = *(strp++)))
                    break;

            *(--point) = '\n';
            *(++point) = '\r';
            *(++point) = '\0';

            write_to_q(CAP(buf), &to->desc->output);
        }
        if ((type == TO_VICT) || (type == TO_CHAR))
            return;
    }
}

void acthan(char *streng,char *strhan,int hide_invisible,struct char_data *ch,
            struct obj_data *obj, void *vict_obj, int type)
{
    register char *strp, *str, *point, *i;
    struct char_data *to;
    char buf[MAX_STRING_LENGTH];
    char buf2[BUFSIZ];

    if (!streng || !strhan)
        return;
    if (!*streng || !*strhan)
        return;

    if (type == TO_VICT)
        to = (struct char_data *) vict_obj;
    else if (type == TO_CHAR)
        to = ch;
    else
        to = world[ch->in_room].people;

    for (; to; to = to->next_in_room)
    {
        if (to->desc && ((to != ch) || (type == TO_CHAR)) &&  
            (CAN_SEE(to, ch) || !hide_invisible) && AWAKE(to) &&
            !((type == TO_NOTVICT) && (to == (struct char_data *) vict_obj)))
        {
            if ((IS_SET(to->specials.act, PLR_KOREAN))) {
                str = strhan ;
            }
            else {
                str = streng ;
            }
            for (strp = str, point = buf;;)
                if (*strp == '$')
                {
                    switch (*(++strp))
                    {
                    case 'n': i = PERS(ch, to); break;
                    case 'N': i = PERS((struct char_data *) vict_obj, to); break;
                    case 'm': i = HMHR(ch); break;
                    case 'M': i = HMHR((struct char_data *) vict_obj); break;
                    case 's': i = HSHR(ch); break;
                    case 'S': i = HSHR((struct char_data *) vict_obj); break;
                    case 'e': i = HSSH(ch); break;
                    case 'E': i = HSSH((struct char_data *) vict_obj); break;
                    case 'o': i = OBJN(obj, to); break;
                    case 'O': i = OBJN((struct obj_data *) vict_obj, to); break;
                    case 'p': i = OBJS(obj, to); break;
                    case 'P': i = OBJS((struct obj_data *) vict_obj, to); break;
                    case 'a': i = SANA(obj); break;
                    case 'A': i = SANA((struct obj_data *) vict_obj); break;
                    case 'T': i = (char *) vict_obj; break;
                    case 'F': i = fname((char *) vict_obj); break;
                    case '$': i = "$"; break;
                    default:
                        log("Illegal $-code to act():");
                        log(str);
                        break;
                    }
                    if ( i ==  NULL ) {
                        log("acthan: null pointer encountered") ;
                        sprintf(buf2, "name: %s to %s\n", GET_NAME(ch), GET_NAME(to));
                        log(buf2);
                        return ;
                    }
                    while (*point = *(i++))
                        ++point;
                    ++strp;
                }
                else if (!(*(point++) = *(strp++)))
                    break;

            *(--point) = '\n';
            *(++point) = '\r';
            *(++point) = '\0';

            write_to_q(CAP(buf), &to->desc->output);
        }
        if ((type == TO_VICT) || (type == TO_CHAR))
            return;
    }
}

void acthanbrief(char *streng,char *strhan,char *strengbrief,char *strhanbrief,
                 int hide_invisible,struct char_data *ch,
                 struct obj_data *obj, void *vict_obj, int type)
{
    register char *strp, *str, *point, *i;
    struct char_data *to;
    char buf[MAX_STRING_LENGTH];

    if (!streng || !strhan || !strengbrief || !strhanbrief)
        return;
    if (!*streng || !*strhan || !*strengbrief || !*strhanbrief)
        return;

    if (type == TO_VICT)
        to = (struct char_data *) vict_obj;
    else if (type == TO_CHAR)
        to = ch;
    else
        to = world[ch->in_room].people;

    for (; to; to = to->next_in_room)
    {
        if (to->desc && ((to != ch) || (type == TO_CHAR)) &&  
            (CAN_SEE(to, ch) || !hide_invisible) && AWAKE(to) &&
            !((type == TO_NOTVICT) && (to == (struct char_data *) vict_obj)))
        {
            if ((IS_SET(to->specials.act, PLR_KOREAN))) {
                if ((IS_SET(to->specials.act, PLR_BRIEF))) {
                    str = strhanbrief ;
                }
                else {
                    str = strhan ;
                }
            }
            else {
                if ((IS_SET(to->specials.act, PLR_BRIEF))) {
                    str = strengbrief ;
                }
                else {
                    str = streng ;
                }
            }
            for (strp = str, point = buf;;)
                if (*strp == '$')
                {
                    switch (*(++strp))
                    {
                    case 'n': i = PERS(ch, to); break;
                    case 'N': i = PERS((struct char_data *) vict_obj, to); break;
                    case 'm': i = HMHR(ch); break;
                    case 'M': i = HMHR((struct char_data *) vict_obj); break;
                    case 's': i = HSHR(ch); break;
                    case 'S': i = HSHR((struct char_data *) vict_obj); break;
                    case 'e': i = HSSH(ch); break;
                    case 'E': i = HSSH((struct char_data *) vict_obj); break;
                    case 'o': i = OBJN(obj, to); break;
                    case 'O': i = OBJN((struct obj_data *) vict_obj, to); break;
                    case 'p': i = OBJS(obj, to); break;
                    case 'P': i = OBJS((struct obj_data *) vict_obj, to); break;
                    case 'a': i = SANA(obj); break;
                    case 'A': i = SANA((struct obj_data *) vict_obj); break;
                    case 'T': i = (char *) vict_obj; break;
                    case 'F': i = fname((char *) vict_obj); break;
                    case '$': i = "$"; break;
                    default:
                        log("Illegal $-code to act():");
                        log(str);
                        break;
                    }
                    while (*point = *(i++))
                        ++point;
                    ++strp;
                }
                else if (!(*(point++) = *(strp++)))
                    break;

            *(--point) = '\n';
            *(++point) = '\r';
            *(++point) = '\0';

            write_to_q(CAP(buf), &to->desc->output);
        }
        if ((type == TO_VICT) || (type == TO_CHAR))
            return;
    }
}

void freaky(struct descriptor_data *d)
{
    char buf[128];

    sprintf(buf,"%d %d %s",
            d->connected,
            d->descriptor,
            d->original ? d->original->player.name : d->character->player.name);
    log(buf);
}
void signal_setup(void)
{
    struct itimerval itime;
    struct timeval interval;

    signal(SIGUSR2, shutdown_request);
    signal(SIGPIPE, SIG_IGN);
    signal(SIGALRM, logsig);

    siginterrupt(SIGHUP , 1);
    siginterrupt(SIGINT , 1);
    siginterrupt(SIGTERM, 1);
    siginterrupt(SIGSEGV, 1);
    siginterrupt(SIGBUS , 1);

    signal(SIGHUP , hupsig);
    signal(SIGINT , hupsig);
    signal(SIGTERM, hupsig);
    signal(SIGSEGV, hupsig);
    signal(SIGBUS , hupsig);

    interval.tv_sec = 900;    /* 15 minutes */
    interval.tv_usec = 0;
    itime.it_interval = interval;
    itime.it_value = interval;
    setitimer(ITIMER_VIRTUAL, &itime, 0);
    signal(SIGVTALRM, checkpointing);
}
void checkpointing(int sig)
{
    extern int tics;
  
    if (!tics) {
        log("CHECKPOINT shutdown: tics not updated");
        abort();
    }
    else
        tics = 0;
    log("checkpointing");
}
void shutdown_request(int sig)
{
    extern int shutdowngame;

    send_to_all("Shut down signal has been received.\n\r");
    log("Received USR2 - shutdown request");
    shutdowngame = 1;
}

/* kick out players etc */
void hupsig(int sig)
{
    void saveallplayers();
    extern int shutdowngame ;
    char s[MAX_STRING_LENGTH];
    static int signalcounter=0;

    sprintf(s,"SIG: %d %s(%d) %s in %s(%d)",sig,xo->character->player.name,
            xo->character->player.level, xo->host, world[xo->character->in_room].name,
            xo->character->in_room) ;
    log(s); 
    ++signalcounter;
    /* saveallplayers();
       if(signalcounter > 3)
       exit(1);
    */  
    if (signalcounter > 500)		/* too much signal.. */
        shutdowngame = 1 ;
    longjmp(env,-1);
}
void logsig(int sig)
{
    log("Signal received. Ignoring.");
}
