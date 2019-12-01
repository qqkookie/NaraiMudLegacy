/* *************************************************************************
   *  file: comm.h , Communication module.                Part of DIKUMUD  *
   *  Usage: connection structures and messaging 			   *
   ************************************************************************* */

// Kuldge to convert deprecated sigsetmask() call to sigprocmask() call

#define  sigsetmask(m) __mysigsetmask(m)

extern int sigsetmask(int mask);

/* ================= Connection Descriptor =================== */

/* NOTE: For switch/return char  */
struct snoop_data {
    struct char_data *snooping;
    struct char_data *snoop_by;
};

/* NOTE: Input/Output queue structures  */
struct txt_block {
    char *text;
    struct txt_block *next;
};

struct txt_q {
    struct txt_block *head;
    struct txt_block *tail;
};

#define MAX_LAST_INPUT_COUNT 20

/* ==========+++     PC Player connection data     +++=========== */

struct descriptor_data {
    char name[20];	/* NOTE: NEW! player name of this connection */
    int descriptor;	/* file descriptor for socket */
    /* NOTE: Old :char host[16] (Numeric IP addr) is extended to  */
    /*       New :char host[30] to accomodate full DNS hostname. */
    char host[30];	/* hostname                   */
    int ncmds, contime;
    char pwd[12];	/* password                   */
    int pos;		/* position in player-file    */
    int connected;	/* mode of 'connectedness'    */
    int wait;		/* wait for how many loops    */
    char *showstr_head;	/* for paging through texts  */
    char *showstr_point;

    char **str;		/* for the modify-str system  */
    int max_str;	/* -                          */

    /* char *title;  */ /* new attribute!!! for posting */
    /* NOTE: multi-purpose scratch buffer (used as posting title, too) */
    char scratch[MAX_BUFSIZ];
    byte page_len;	/* NOTE: page_string() page length */

    int prompt_mode;			/* control of prompt-printing	*/
    char ibuf[MAX_STRING_LENGTH];	/* buffer for raw input		*/
    /* NOTE: OLD: char buf[MAX_STRING_LENGTH]. To avoid confusion    */
    char last_input[MAX_LAST_INPUT_COUNT][MAX_INPUT_LENGTH]; /* for use of !! */
    int last_input_count;
    struct txt_q output;		/* q of strings to send		*/
    struct txt_q input;			/* q of unprocessed input	*/
    struct char_data *character;	/* linked to char		*/
    struct char_data *original;		/* original char		*/
    struct snoop_data snoop;		/* to snoop people.		*/
    struct descriptor_data *next;	/* link to next descriptor	*/
};

/* modes of connectedness */

#define CON_PLYNG	0
#define CON_NME		1
#define CON_NMECNF	2
#define CON_PWDNRM	3
#define CON_PWDGET	4
#define CON_PWDCNF	5
#define CON_QSEX	6
#define CON_RMOTD	7
#define CON_SLCT	8
#define CON_EXDSCR	9
#define CON_QCLASS	10
#define CON_LDEAD	11
#define CON_PWDNEW	12
#define CON_PWDNEW1	13
#define CON_PWDNCNF	14
#define CON_QSTATE	15
#define CON_SET		16
#define CON_DELCNF	17
#define CON_DELCNF2	18
#define CON_IMOTD	19
/* NOTE: for more connection state in nanny() */
#define CON_NEWCON	20
#define CON_ENTER	21
/* NOTE: New connection state to close_socket() */
#define CON_CLOSE	22

/*	Global varaible	*/
extern struct descriptor_data  *descriptor_list;

/*
#define STORY     \
"Once upon a time, long and long years ago...... Ummm...\r\n\
....Cookie doesn't say more. You'd better entering the Game.\r\n\r\n"
*/

extern char login_menu[];
extern char login_banner[];
extern char login_welcome[];

/* ==============    Genral Printing proc's    ============== */

#define SEND_TO_Q(messg, desc)  write_to_q((messg), &(desc)->output)

extern int write_to_descriptor(int desc, char *txt);
extern void write_to_q(char *txt, struct txt_q *queue);

/* ---------------------------------------------------------- */
extern void send_to_all(char *messg);
extern void send_to_char(char *messg, struct char_data *ch);
extern void send_to_char_han(char *messg_eng, char *msg_han,
	struct char_data *ch);
extern char *MSGSTR(char *msg, struct char_data *ch);

extern void send_to_room(char *messg, int room);
extern void send_to_room_except(char *messg, int room, struct char_data *ch);

void page_string(struct descriptor_data *d, char *str, int keep_internal);
void do_say(struct char_data *ch, char *arg, int cmd);

void act(char *str, int hide_invisible, struct char_data *ch,
	 struct obj_data *obj, void *vict_obj, int type);
void acthan(char *seng, char *shan, int hide_invisible, struct char_data *ch,
	    struct obj_data *obj, void *vict_obj, int type);

#define TO_ROOM    0
#define TO_VICT    1
#define TO_NOTVICT 2
#define TO_CHAR    3

/* NOTE: NEW MACRO:  Select english or korean message string according
   to ch's locale selection. Useful for sprintf().      */
#define STRHAN( a, b, ch ) \
	((IS_SET((ch)->specials.act, PLR_KOREAN)) ? (b):(a))

/* -------------  act() like expansion macro   ------------- */
/* NOTE: Following macros are defined in "act.comm.c", too */

#define PERS(ch, vict)   ( CAN_SEE(vict, ch) ? \
    (!IS_NPC(ch) ? (ch)->player.name : (ch)->player.short_descr) : "someone" )

#define HSHR(ch) ((ch)->player.sex ?          \
  (((ch)->player.sex == 1) ? "his" : "her") : "its")

#define HMHR(ch) ((ch)->player.sex ?           \
  (((ch)->player.sex == 1) ? "him" : "her") : "it")

/* -------------------------------------------------------------- */
