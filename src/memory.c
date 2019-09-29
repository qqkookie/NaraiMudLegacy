#include <stdio.h>
#include "structs.h"
#include "memory.h"
#include "db.h"

#define	MEM_SIZE_TYPE0	sizeof(struct descriptor_data)
#define	MEM_SIZE_TYPE1	sizeof(struct char_data)
#define	MEM_SIZE_TYPE2	sizeof(struct obj_data)
#define	MEM_SIZE_TYPE3	sizeof(struct message_type)
#define	MEM_SIZE_TYPE4	sizeof(struct room_data)
#define	MEM_SIZE_TYPE5	sizeof(struct extra_descr_data)
#define	MEM_SIZE_TYPE6	sizeof(struct txt_block)
#define	MEM_SIZE_TYPE7	sizeof(struct affected_type)
#define	MEM_SIZE_TYPE8	sizeof(struct follow_type)

/* from type9 - string type */
/* type 9 and type 10 - used in get_from_q() and write_to_q() */
#define	MEM_SIZE_TYPE9	32
#define	MEM_SIZE_TYPE10	128
/* from type 10 - general string type */
#define	MEM_SIZE_TYPE11	2
#define	MEM_SIZE_TYPE12	16
#define	MEM_SIZE_TYPE13	32
#define	MEM_SIZE_TYPE14	128
#define	MEM_SIZE_TYPE15	256
#define	MEM_SIZE_TYPE16	512
#define	MEM_SIZE_TYPE17	1024
#define	MEM_SIZE_TYPE18	2048
#define	MEM_SIZE_TYPE19	4096
#define	MEM_SIZE_TYPE20	6144

/* in db.h */
#define	MEM_SIZE_TYPE21	sizeof(struct reset_q_element)

#define MEMORY_TYPE0	MEMORY_DESCRIPTOR_DATA
#define MEMORY_TYPE1	MEMORY_CHAR_DATA
#define MEMORY_TYPE2	MEMORY_OBJ_DATA
#define MEMORY_TYPE3	MEMORY_MESSAGE_TYPE
#define MEMORY_TYPE4	MEMORY_ROOM_DATA
#define MEMORY_TYPE5	MEMORY_EXTRA_DESCR_DATA
#define MEMORY_TYPE6	MEMORY_TXT_BLOCK
#define MEMORY_TYPE7	MEMORY_AFFECTED_TYPE
#define MEMORY_TYPE8	MEMORY_FOLLOW_TYPE

#define MEMORY_TYPE9 MEMORY_QUEUE_STRING_32
#define MEMORY_TYPE10 MEMORY_QUEUE_STRING_128
#define MEMORY_TYPE11 MEMORY_STRING_2
#define MEMORY_TYPE12 MEMORY_STRING_16
#define MEMORY_TYPE13 MEMORY_STRING_32
#define MEMORY_TYPE14 MEMORY_STRING_128
#define MEMORY_TYPE15 MEMORY_STRING_256
#define MEMORY_TYPE16 MEMORY_STRING_512
#define MEMORY_TYPE17 MEMORY_STRING_1024
#define MEMORY_TYPE18 MEMORY_STRING_2048
#define MEMORY_TYPE19 MEMORY_STRING_4096
#define MEMORY_TYPE20 MEMORY_STRING_6144

#define MEMORY_TYPE21 MEMORY_RESET_Q_ELEMENT

/*
	NOTE : RESERVED size must be smaller than STACK size
*/
#define	STACK_TYPE0	60
#define	STACK_TYPE1	4000
#define	STACK_TYPE2	4000
#define	STACK_TYPE3	300
#define	STACK_TYPE4	300
#define	STACK_TYPE5	5000
#define	STACK_TYPE6	3000
#define	STACK_TYPE7	300
#define	STACK_TYPE8	300

#define	STACK_TYPE9	1000
#define	STACK_TYPE10	1000

#define	STACK_TYPE11	10000
#define	STACK_TYPE12	14000
#define	STACK_TYPE13	20000
#define	STACK_TYPE14	28000
#define	STACK_TYPE15	10000
#define	STACK_TYPE16	8000
#define	STACK_TYPE17	300
#define	STACK_TYPE18	600
#define	STACK_TYPE19	200
#define	STACK_TYPE20	50

#define	STACK_TYPE21	50

#define	RESERVED_TYPE0	54
#define	RESERVED_TYPE1	4000
#define	RESERVED_TYPE2	4000
#define	RESERVED_TYPE3	270
#define	RESERVED_TYPE4	270
#define	RESERVED_TYPE5	5000
#define	RESERVED_TYPE6	2500
#define	RESERVED_TYPE7	270
#define	RESERVED_TYPE8	270

#define	RESERVED_TYPE9	900
#define	RESERVED_TYPE10	900

#define	RESERVED_TYPE11	1000
#define	RESERVED_TYPE12	10000
#define	RESERVED_TYPE13	10000
#define	RESERVED_TYPE14	28000
#define	RESERVED_TYPE15	9000
#define	RESERVED_TYPE16	4000
#define	RESERVED_TYPE17	140
#define	RESERVED_TYPE18	340
#define	RESERVED_TYPE19	70
#define	RESERVED_TYPE20	10

#define	RESERVED_TYPE21	50

typedef struct {
	int	id ;
	int	stack_size ;
	int	reserved ;
	int	struct_size ;
	char	**stack ;
	char	*spool ;
} Memory_manage;

typedef struct {
	char	**stack ;	/* stack array */
	int	sp ;		/* stack pointer */
	int	stack_size ;
} STACK ;

/*	Local functions */
STACK *malloc_select_stack(int typeflag) ;
int InitStack(STACK *st, int size, char **stack_array) ;
int Pop(STACK *st) ;
int Push(char *x, STACK *st) ;
int Emptystack(STACK *st) ;
int Fullstack(STACK *st) ;
char *Top(STACK *st) ;
char *Popstack(STACK *st) ;
int StackCapacity(STACK *st) ;
int StackUsed(STACK *st) ;
int StackInfo(int id, int *stack_size, int *reserved, int *used,
              int *struct_size) ;
void dump_lastcom() ;
void dump_lastqueue() ;
void lastcom_add(char *str) ;

/* memory table */
Memory_manage memory_book[MAX_MEMORY_MANAGE_SIZE] = {
	{ MEMORY_TYPE0, STACK_TYPE0, RESERVED_TYPE0, MEM_SIZE_TYPE0},
	{ MEMORY_TYPE1, STACK_TYPE1, RESERVED_TYPE1, MEM_SIZE_TYPE1},
	{ MEMORY_TYPE2, STACK_TYPE2, RESERVED_TYPE2, MEM_SIZE_TYPE2},
	{ MEMORY_TYPE3, STACK_TYPE3, RESERVED_TYPE3, MEM_SIZE_TYPE3},
	{ MEMORY_TYPE4, STACK_TYPE4, RESERVED_TYPE4, MEM_SIZE_TYPE4},
	{ MEMORY_TYPE5, STACK_TYPE5, RESERVED_TYPE5, MEM_SIZE_TYPE5},
	{ MEMORY_TYPE6, STACK_TYPE6, RESERVED_TYPE6, MEM_SIZE_TYPE6},
	{ MEMORY_TYPE7, STACK_TYPE7, RESERVED_TYPE7, MEM_SIZE_TYPE7},
	{ MEMORY_TYPE8, STACK_TYPE8, RESERVED_TYPE8, MEM_SIZE_TYPE8},
	{ MEMORY_TYPE9, STACK_TYPE9, RESERVED_TYPE9, MEM_SIZE_TYPE9},
	{ MEMORY_TYPE10, STACK_TYPE10, RESERVED_TYPE10, MEM_SIZE_TYPE10},
	{ MEMORY_TYPE11, STACK_TYPE11, RESERVED_TYPE11, MEM_SIZE_TYPE11},
	{ MEMORY_TYPE12, STACK_TYPE12, RESERVED_TYPE12, MEM_SIZE_TYPE12},
	{ MEMORY_TYPE13, STACK_TYPE13, RESERVED_TYPE13, MEM_SIZE_TYPE13},
	{ MEMORY_TYPE14, STACK_TYPE14, RESERVED_TYPE14, MEM_SIZE_TYPE14},
	{ MEMORY_TYPE15, STACK_TYPE15, RESERVED_TYPE15, MEM_SIZE_TYPE15},
	{ MEMORY_TYPE16, STACK_TYPE16, RESERVED_TYPE16, MEM_SIZE_TYPE16},
	{ MEMORY_TYPE17, STACK_TYPE17, RESERVED_TYPE17, MEM_SIZE_TYPE17},
	{ MEMORY_TYPE18, STACK_TYPE18, RESERVED_TYPE18, MEM_SIZE_TYPE18},
	{ MEMORY_TYPE19, STACK_TYPE19, RESERVED_TYPE19, MEM_SIZE_TYPE19},
	{ MEMORY_TYPE20, STACK_TYPE20, RESERVED_TYPE20, MEM_SIZE_TYPE20},
	{ MEMORY_TYPE21, STACK_TYPE21, RESERVED_TYPE21, MEM_SIZE_TYPE21},
} ;

/* stack headers */
STACK type0_header, type1_header, type2_header, type3_header, type4_header ;
STACK type5_header, type6_header, type7_header, type8_header, type9_header ;
STACK type10_header, type11_header, type12_header, type13_header ;
STACK type14_header, type15_header, type16_header, type17_header ;
STACK	type18_header, type19_header, type20_header, type21_header ;

/* memory spool */
char type0_spool[RESERVED_TYPE0 * MEM_SIZE_TYPE0] ;
char type1_spool[RESERVED_TYPE1 * MEM_SIZE_TYPE1] ;
char type2_spool[RESERVED_TYPE2 * MEM_SIZE_TYPE2] ;
char type3_spool[RESERVED_TYPE3 * MEM_SIZE_TYPE3] ;
char type4_spool[RESERVED_TYPE4 * MEM_SIZE_TYPE4] ;
char type5_spool[RESERVED_TYPE5 * MEM_SIZE_TYPE5] ;
char type6_spool[RESERVED_TYPE6 * MEM_SIZE_TYPE6] ;
char type7_spool[RESERVED_TYPE7 * MEM_SIZE_TYPE7] ;
char type8_spool[RESERVED_TYPE8 * MEM_SIZE_TYPE8] ;

char type9_spool[RESERVED_TYPE9 * MEM_SIZE_TYPE9] ;
char type10_spool[RESERVED_TYPE10 * MEM_SIZE_TYPE10] ;

char type11_spool[RESERVED_TYPE11 * MEM_SIZE_TYPE11] ;
char type12_spool[RESERVED_TYPE12 * MEM_SIZE_TYPE12] ;
char type13_spool[RESERVED_TYPE13 * MEM_SIZE_TYPE13] ;
char type14_spool[RESERVED_TYPE14 * MEM_SIZE_TYPE14] ;
char type15_spool[RESERVED_TYPE15 * MEM_SIZE_TYPE15] ;
char type16_spool[RESERVED_TYPE16 * MEM_SIZE_TYPE16] ;
char type17_spool[RESERVED_TYPE17 * MEM_SIZE_TYPE17] ;
char type18_spool[RESERVED_TYPE18 * MEM_SIZE_TYPE18] ;
char type19_spool[RESERVED_TYPE19 * MEM_SIZE_TYPE19] ;
char type20_spool[RESERVED_TYPE20 * MEM_SIZE_TYPE20] ;

char type21_spool[RESERVED_TYPE21 * MEM_SIZE_TYPE21] ;

/* stacks */
char *type0_stack[STACK_TYPE0] ;
char *type1_stack[STACK_TYPE1] ;
char *type2_stack[STACK_TYPE2] ;
char *type3_stack[STACK_TYPE3] ;
char *type4_stack[STACK_TYPE4] ;
char *type5_stack[STACK_TYPE5] ;
char *type6_stack[STACK_TYPE6] ;
char *type7_stack[STACK_TYPE7] ;
char *type8_stack[STACK_TYPE8] ;

char *type9_stack[STACK_TYPE9] ;
char *type10_stack[STACK_TYPE10] ;
char *type11_stack[STACK_TYPE11] ;
char *type12_stack[STACK_TYPE12] ;
char *type13_stack[STACK_TYPE13] ;
char *type14_stack[STACK_TYPE14] ;
char *type15_stack[STACK_TYPE15] ;
char *type16_stack[STACK_TYPE16] ;
char *type17_stack[STACK_TYPE17] ;
char *type18_stack[STACK_TYPE18] ;
char *type19_stack[STACK_TYPE19] ;
char *type20_stack[STACK_TYPE20] ;

char *type21_stack[STACK_TYPE21] ;

void init_memory_manager()
{
	Memory_manage	*p ;
	STACK	*stackhead ;
	int	i, j ;

	/* init stack */
	memory_book[0].stack = type0_stack ;
	memory_book[1].stack = type1_stack ;
	memory_book[2].stack = type2_stack ;
	memory_book[3].stack = type3_stack ;
	memory_book[4].stack = type4_stack ;
	memory_book[5].stack = type5_stack ;
	memory_book[6].stack = type6_stack ;
	memory_book[7].stack = type7_stack ;
	memory_book[8].stack = type8_stack ;
	memory_book[9].stack = type9_stack ;
	memory_book[10].stack = type10_stack ;
	memory_book[11].stack = type11_stack ;
	memory_book[12].stack = type12_stack ;
	memory_book[13].stack = type13_stack ;
	memory_book[14].stack = type14_stack ;
	memory_book[15].stack = type15_stack ;
	memory_book[16].stack = type16_stack ;
	memory_book[17].stack = type17_stack ;
	memory_book[18].stack = type18_stack ;
	memory_book[19].stack = type19_stack ;
	memory_book[20].stack = type20_stack ;
	memory_book[21].stack = type21_stack ;


	/* init spool */
	memory_book[0].spool = type0_spool ;
	memory_book[1].spool = type1_spool ;
	memory_book[2].spool = type2_spool ;
	memory_book[3].spool = type3_spool ;
	memory_book[4].spool = type4_spool ;
	memory_book[5].spool = type5_spool ;
	memory_book[6].spool = type6_spool ;
	memory_book[7].spool = type7_spool ;
	memory_book[8].spool = type8_spool ;
	memory_book[9].spool = type9_spool ;
	memory_book[10].spool = type10_spool ;
	memory_book[11].spool = type11_spool ;
	memory_book[12].spool = type12_spool ;
	memory_book[13].spool = type13_spool ;
	memory_book[14].spool = type14_spool ;
	memory_book[15].spool = type15_spool ;
	memory_book[16].spool = type16_spool ;
	memory_book[17].spool = type17_spool ;
	memory_book[18].spool = type18_spool ;
	memory_book[19].spool = type19_spool ;
	memory_book[20].spool = type20_spool ;
	memory_book[21].spool = type21_spool ;

	for ( i = 0 ; i < MAX_MEMORY_MANAGE_SIZE ; i++ ) {
		p = &memory_book[i] ;
		if ((stackhead = malloc_select_stack(p->id)) == NULL) {
			log("init_memory_spool: no such stack exist") ;
			exit(1) ;
        }
		if (InitStack(stackhead, p->stack_size, p->stack) < 0 ) {
			log("init_memory_spool: can't init stack") ;
			exit(1) ;
        }
    }

	for ( i = 0 ; i < MAX_MEMORY_MANAGE_SIZE ; i++) {
		p = &memory_book[i] ;
		stackhead = malloc_select_stack(p->id) ;
		for ( j = 0 ; j < p->reserved ; j++ ) {
			if ( !Fullstack(stackhead) )
				Push((char *) (p->spool + p->struct_size * j), stackhead) ;
        }
    }
}

/* Return stack capacity (used %) */
int malloc_memory_capacity(int typeflag)
{
	STACK	*stack ;

	if ((stack = malloc_select_stack(typeflag)) != NULL)
		return ( StackCapacity(stack) ) ;
	else
		return (-1) ;
}

int malloc_memory_used(int typeflag)
{
	STACK	*stack ;

	if ((stack = malloc_select_stack(typeflag)) != NULL)
		return ( StackUsed(stack) ) ;
	else
		return (-1) ;
}

int malloc_memory_info(int typeflag, int *stack_size, int *reserved, int *used,
                       int *struct_size)
{
	if ( StackInfo(typeflag, stack_size, reserved, used, struct_size) < 0)
		return (-1) ;
	else
		return 0 ;
}

STACK *malloc_select_stack(int typeflag)
{
	/*
      Notice : in case of string , it must be selected in
      malloc_string_type(), and free_string_type()
	*/
	switch (typeflag) {
    case MEMORY_TYPE0 : return (&type0_header) ;
    case MEMORY_TYPE1 : return (&type1_header) ;
    case MEMORY_TYPE2 : return (&type2_header) ;
    case MEMORY_TYPE3 : return (&type3_header) ;
    case MEMORY_TYPE4 : return (&type4_header) ;
    case MEMORY_TYPE5 : return (&type5_header) ;
    case MEMORY_TYPE6 : return (&type6_header) ;
    case MEMORY_TYPE7 : return (&type7_header) ;
    case MEMORY_TYPE8 : return (&type8_header) ;
    case MEMORY_TYPE9 : return (&type9_header) ;
    case MEMORY_TYPE10 : return (&type10_header) ;
    case MEMORY_TYPE11 : return (&type11_header) ;
    case MEMORY_TYPE12 : return (&type12_header) ;
    case MEMORY_TYPE13 : return (&type13_header) ;
    case MEMORY_TYPE14 : return (&type14_header) ;
    case MEMORY_TYPE15 : return (&type15_header) ;
    case MEMORY_TYPE16 : return (&type16_header) ;
    case MEMORY_TYPE17 : return (&type17_header) ;
    case MEMORY_TYPE18 : return (&type18_header) ;
    case MEMORY_TYPE19 : return (&type19_header) ;
    case MEMORY_TYPE20 : return (&type20_header) ;
    case MEMORY_TYPE21 : return (&type21_header) ;
    default : return NULL ;	/* unknown typeflag */
    }
}

char *malloc_general_type(int typeflag)
{
	STACK	*stack ;
	char	*mem, buf[BUFSIZ] ;
	static int	emptyflag[MAX_MEMORY_MANAGE_SIZE] ;
	
	if ((stack = malloc_select_stack(typeflag)) == NULL) {
		sprintf(buf, "malloc_general_type: unknown typeflag %d", typeflag) ;
		log(buf) ;
		dump_lastcom() ;
		dump_lastqueue() ;
		abort() ;
		/* exit(2) ; */
    }

	if ( Emptystack(stack) ) {
		if ( emptyflag[typeflag] == 0 ) {
			sprintf(buf, "malloc_general_type: empty stack type %d", typeflag) ;
			log(buf) ;
			emptyflag[typeflag] = 1 ;
        }
		if ((mem = (char *) malloc(memory_book[typeflag].struct_size)) == NULL) {
			sprintf(buf, "malloc_general_type: Malloc failure (size = %d)",
                    memory_book[typeflag].struct_size);
			log(buf) ;
			dump_lastcom() ;
			dump_lastqueue() ;
			abort() ;
			/* exit(2) ; */
        }
    }
	else {	/* we have mem in stack */
		mem = Popstack(stack) ;
    }

	return mem ;
}

int free_general_type(char *mem, int typeflag)
{
	STACK	*stack ;
	char	buf[BUFSIZ] ;
	static int	fullflag[MAX_MEMORY_MANAGE_SIZE] ;

	if ( mem == NULL ) {
		log("free_general_type: attempt to free NULL pointer") ;
		return 0 ;
    }

	if ((stack = malloc_select_stack(typeflag)) == NULL) {
		sprintf(buf, "malloc_general_type: unknown typeflag %d", typeflag) ;
		log(buf) ;
		return (-1) ;
    }

	if ( Fullstack(stack) ) {
		/* we have no solution */
		if ( fullflag[typeflag] == 0 ) {
			sprintf(buf, "free_general_type: stack type %d full", typeflag) ;
			log(buf) ;
			fullflag[typeflag] = 1 ;
        }
    }
	else {
		Push((char *) mem, stack) ;
    }

	return 0 ;
}

char *malloc_string_type(int size)
{
	STACK	*stack ;
	char	*mem, buf[BUFSIZ] ;
	static int	emptyflag = 0 ;
	
	if ( size <= 0 ) return NULL ;
	else if ( size <= 2 )	stack = &type11_header ;
	else if ( size <= 16 )	stack = &type12_header ;
	else if ( size <= 32 )	stack = &type13_header ;
	else if ( size <= 128 )	stack = &type14_header ;
	else if ( size <= 256 )	stack = &type15_header ;
	else if ( size <= 512 )	stack = &type16_header ;
	else if ( size <= 1024 )	stack = &type17_header ;
	else if ( size <= 2048 )	stack = &type18_header ;
	else if ( size <= 4096 )	stack = &type19_header ;
	else if ( size <= 6144 )	stack = &type20_header ;
	else {
		sprintf(buf, "malloc_string_type: Too big string (%d)", size) ;
		log(buf) ;
		if ((mem = (char *) malloc(size)) == NULL) {
			log("can't alloc that big string") ;
			dump_lastcom() ;
			dump_lastqueue() ;
			abort() ;
			/* exit(2) ; */
        }
		mem[0] = 0;
		return mem ;
    }

	if ( Emptystack(stack) ) {
		if ( emptyflag == 0 ) {
			sprintf(buf, "malloc_string_type: empty stack (size = %d)", size) ;
			log(buf) ;
			emptyflag = 1 ;
        }
		if ((mem = (char *) malloc(size)) == NULL) {
			sprintf(buf, "malloc_string_type: Malloc failure (size = %d)", size);
			log(buf) ;
			dump_lastcom() ;
			dump_lastqueue() ;
			abort() ;
			/* exit(2) ; */
        }
    }
	else {	/* we have mem in stack */
		mem = Popstack(stack) ;
    }

	mem[0] = 0 ;	/* init string */
	return mem ;
}

int free_string_type(char *mem)
{
	STACK *find_memory_stack(char *mem) ;
	STACK	*stack ;
	char	buf[BUFSIZ], *tmp1, *tmp2 ;
	int	size ;
	static int fullflag = 0 ;

	if ( mem == NULL ) {
		log("free_string_type: attempt to free NULL pointer") ;
		return 0 ;
    }

	/* find proper stack */
	if (( stack = find_memory_stack(mem)) == NULL ) {
		/* now check size of string */
		if (( size = strlen(mem) ) < 0 ) {
			log("free_string_type: string length < 0, can't guess size") ;
			return 0 ;
        }
		if ( size >= 6144 )	stack = &type20_header ;	/* size to 6144 */
		else if ( size >= 4096 )	stack = &type19_header ;	/* size to 4096 */
		else if ( size >= 2048 )	stack = &type18_header ;
		else if ( size >= 1024 )	stack = &type17_header ;
		else if ( size >= 512 )	stack = &type16_header ;
		else if ( size >= 256 )	stack = &type15_header ;
		else if ( size >= 128 )	stack = &type14_header ;
		else if ( size >= 32 )	stack = &type13_header ;
		else if ( size >= 16 )	stack = &type12_header ;
		else if ( size >= 2 )	stack = &type11_header ;
		else {	/* Too small string to manage */
			/* too small string : no solution */
			/* "free_string_type: Too small string %d" */
			return 0 ;
        }
    }

	/* Now we found proper stack to store */

	if ( Fullstack(stack) ) {
		/* we have no solution */
		if ( fullflag == 0 ) {
			sprintf(buf, "free_string_type: stack size %d full", size) ;
			log(buf) ;
			fullflag = 1 ;
        }
    }
	else {
		Push((char *) mem, stack) ;
    }

	return 0 ;
}

STACK *find_memory_stack(char *mem)
{
	Memory_manage	*p ;
	int	i ;
	char	*pstart, *pend ;

	for ( i = 0 ; i < MAX_MEMORY_MANAGE_SIZE ; i++ ) {
		p = &memory_book[i] ;
		pstart = p->spool ;
		pend = pstart + p->reserved * p->struct_size - 1 ;
		if ( pstart >= mem && pend <= mem )
			return ( malloc_select_stack(p->id) ) ;
    }

	return NULL ;	/* find fails */
}

char *malloc_q_string_type(int size)
{
	STACK	*stack ;
	char	*mem, buf[BUFSIZ] ;
	static int	emptyflag = 0 ;
	
	if ( size <= 0 ) return NULL ;
	else if ( size <= 32 )	stack = &type9_header ;
	else if ( size <= 128 )	stack = &type10_header ;
	else {
		/* Error handling is in malloc_string_type() */
		mem = malloc_string_type(size) ;
		mem[0] = 0;
		return mem ;
    }

	if ( Emptystack(stack) ) {
		if ( emptyflag == 0 ) {
			sprintf(buf,"malloc_q_string_type: empty stack (size = %d)",size);
			log(buf) ;
			emptyflag = 1 ;
        }
		if ((mem = malloc_string_type(size)) == NULL) {
			sprintf(buf,"malloc_q_string_type: Malloc failure (size = %d)",size);
			log(buf) ;
			dump_lastcom() ;
			dump_lastqueue() ;
			abort() ;
			/* exit(2) ; */
        }
    }
	else {	/* we have mem in queue stack */
		mem = Popstack(stack) ;
    }

	mem[0] = 0 ;	/* init string */
	return mem ;
}

int free_q_string_type(char *mem)
{
	STACK	*stack ;
	char	buf[BUFSIZ] ;
	static char	*size32_start, *size32_end ;
	static char *size128_start, *size128_end ;
	char	*tmp_p ;
	int	size ;
	static int fullflag = 0 ;
	static int sizemarker = 0 ;

	if ( mem == NULL ) {
		log("free_q_string_type: attempt to free NULL pointer") ;
		return 0 ;
    }

	if ( sizemarker == 0 ) {
		size32_start = &type9_spool[0] ;
		size32_end = &type9_spool[RESERVED_TYPE9 * MEM_SIZE_TYPE9 - 1] ;
		size128_start = &type10_spool[0] ;
		size128_end = &type10_spool[RESERVED_TYPE10 * MEM_SIZE_TYPE10 - 1] ;
		sizemarker = 1 ;
    }

	/* Check if mem is with in queue stack */
	if ( mem >= size32_start && mem <= size32_end )
        stack = &type9_header ;
	else if ( mem >= size128_start && mem <= size128_end )
        stack = &type10_header ;
	else {
		/* If not, just free using free_string_type()  */
		free_string_type(mem) ;
		return 0 ;
    }

	if ( Fullstack(stack) ) {
		if ( fullflag == 0 ) {
			sprintf(buf, "free_q_string_type: stack size %d full", size) ;
			log(buf) ;
			fullflag = 1 ;
        }
		/* then, try to using free_string_type */
		free_string_type(mem) ;
    }
	else {
		Push((char *) mem, stack) ;
    }

	return 0 ;
}

/*******************************
	Stack operations
*******************************/

#define GOOD	0
#define BAD	-1

int InitStack(STACK *st, int size, char **stack_array)
{
	if (st == NULL)
		return (-1) ;

	st->stack_size = size ;	/* stack size */
	st->sp = size ;	/* stack pointer */
	st->stack = stack_array ;	/* stack array */

	return 0 ;
}

int Pop(st)		/* pop one element from stack */
    STACK	*st ;
{
	if ( st == NULL ) {
		/* puts("Pop: Stack is not initialized") ; */
		return BAD ;
    }
	if ( Emptystack(st) ) {
		/* puts("Pop: Stack empty") ; */
		return BAD ;
    }

	(st->sp)++ ;
	return GOOD ;
}

int Push(char *x, STACK *st)		/* Push x into stack */
{
	if ( st == NULL ) {
		/* puts("Push: Stack is not initialized") ; */
		return BAD ;
    }
	if ( Fullstack(st) ) {
		/* puts("Push: Stack full") ;*/
		return BAD ;
    }

	st->stack[--(st->sp)] = x ;
	return ( GOOD ) ;
}

int Emptystack(st)		/* return true if stack is empty */
    STACK	*st ;
{
	return ( st->sp >= st->stack_size ) ;
}

int Fullstack(st)		/* return true if stack is full */
    STACK	*st ;
{
	return ( st->sp <= 0 ) ;
}

char *Top(st)		/* return top element of stack */
    STACK	*st ;
{
	return ( st->stack[st->sp] ) ;
}

char *Popstack(st)
    STACK	*st ;
{
	char	*tmp ;

	tmp = Top(st) ;
	Pop(st) ;
	return (tmp) ;
}

int StackCapacity(STACK *st)
{
	int	capacity, used ;

	used = st->stack_size - st->sp ;
	capacity = (int) 100 * used / st->stack_size ;
	return ( capacity ) ;
}

int StackUsed(STACK *st)
{
	return ( st->stack_size - st->sp ) ;
}

int StackInfo(int id, int *stack_size, int *reserved, int *used,
              int *struct_size)
{
	STACK	*stack ;

	if ((stack = malloc_select_stack(id)) == NULL)
		return (-1) ;

	if ( stack_size )
		*stack_size = stack->stack_size ;
	if ( reserved )
		*reserved =	memory_book[id].reserved ;
	if ( used )
		*used = stack->stack_size - stack->sp ;
	if ( struct_size )
		*struct_size = memory_book[id].struct_size ;

	return 0 ;
}

/*
	Procedure for lastcommand - cold 
*/

#define MAX_LASTCOM	50
static char lastcombuf[MAX_LASTCOM][BUFSIZ] ;
int lastcom_pointer = 0 ;

void dump_lastcom()
{
	int	i, point ;

	log("Dump last commands") ;

	point = lastcom_pointer ;
	for ( i = 0 ; i < MAX_LASTCOM ; i ++) {
		if ( lastcombuf[point][0] == 0 ) {
			point = (point +1) % MAX_LASTCOM ;
			continue ;
        }
		fprintf(stderr, "LAST: %s\n", lastcombuf[point]) ;
		point = (point +1) % MAX_LASTCOM ;
    }
}

void lastcom_add(char *str)
{
	strcpy(lastcombuf[lastcom_pointer], str) ;
	lastcom_pointer = (lastcom_pointer +1) % MAX_LASTCOM ;
}

#undef MAX_LASTCOM

#define MAX_LASTQUEUE	50
static char lastqueuebuf[MAX_LASTQUEUE][BUFSIZ] ;
int lastqueue_pointer = 0 ;

void dump_lastqueue()
{
	int	i, point ;

	log("Dump last queue") ;

	point = lastqueue_pointer ;
	for ( i = 0 ; i < MAX_LASTQUEUE ; i ++) {
		if ( lastqueuebuf[point][0] == 0 ) {
			point = (point +1) % MAX_LASTQUEUE ;
			continue ;
        }
		fprintf(stderr, "QUEUE: %s\n", lastqueuebuf[point]) ;
		point = (point +1) % MAX_LASTQUEUE ;
    }
}

void lastqueue_add(char *str)
{
	strcpy(lastqueuebuf[lastqueue_pointer], str) ;
	lastqueue_pointer = (lastqueue_pointer +1) % MAX_LASTQUEUE ;
}

#undef MAX_LASTQUEUE
