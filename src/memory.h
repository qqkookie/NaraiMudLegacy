/*
	memory.h
 */

#define	MEMORY_DESCRIPTOR_DATA	0
#define	MEMORY_CHAR_DATA	1
#define	MEMORY_OBJ_DATA	2
#define	MEMORY_MESSAGE_TYPE	3
#define	MEMORY_ROOM_DATA	4
#define	MEMORY_EXTRA_DESCR_DATA	5
#define	MEMORY_TXT_BLOCK	6
#define	MEMORY_AFFECTED_TYPE	7
#define	MEMORY_FOLLOW_TYPE	8

#define	MEMORY_QUEUE_STRING_32	9
#define	MEMORY_QUEUE_STRING_128	10

#define	MEMORY_STRING_2	11
#define	MEMORY_STRING_16	12
#define	MEMORY_STRING_32	13
#define	MEMORY_STRING_128	14
#define	MEMORY_STRING_256	15
#define	MEMORY_STRING_512	16
#define	MEMORY_STRING_1024	17
#define	MEMORY_STRING_2048	18
#define	MEMORY_STRING_4096	19
#define	MEMORY_STRING_6144	20

#define	MEMORY_RESET_Q_ELEMENT	21

/* if new memory is added, then following size must be updated, too */
#define	MAX_MEMORY_MANAGE_SIZE	22


/*
 *	Declaration of functions.
 */
void init_memory_manager() ;
int malloc_memory_capacity(int typeflag) ;
int malloc_memory_used(int typeflag) ;
char *malloc_general_type(int typeflag) ;
int free_general_type(char *mem, int typeflag) ;
char *malloc_string_type(int size) ;
int free_string_type(char *mem) ;
char *malloc_q_string_type(int size) ;
int free_q_string_type(char *mem) ;
