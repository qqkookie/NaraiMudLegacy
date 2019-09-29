/*
	Check Porability.
	To transform "player file" to other machine ,
	  we must check size of types first.
*/
#include <stdio.h>
#include <sys/types.h>
#include "structs.h"

struct portable {
	char	*name ;
	int	expected, actual ;
	} ;

struct portable check[13] = {
	{"char", 1},		/* expected size byte */
	{"short", 2},
	{"int", 4},
	{"long", 4},
	{"float", 4},
	{"double", 8},
	{"pointer", 4},
	{"time_t", 4},
	{"file_u", 1228},
	{"ability", 12},
	{"point", 32},
	{"skill", 2},
	{"affected", 16}
} ;
main()
{
	int	i ;
	extern struct portable check[] ;

	init() ;
	/* elementary structure */
	printf("Elementary types\n") ;
	for ( i = 0 ; i < 8 ; i++) {
		printf("  %-8s = %2d ( %c= %2d)\n",check[i].name,
			check[i].actual,
			(check[i].actual == check[i].expected) ? '=' : '!' ,
			check[i].expected) ;
		}

	printf("\nStructures\n") ;
	for ( ; i < 13 ; i++) {
		printf("  %-8s = %2d ( %c= %2d)\n",check[i].name,
			check[i].actual,
			(check[i].actual == check[i].expected) ? '=' : '!' ,
			check[i].expected) ;
		}
}

init()
{
	extern struct portable check[] ;

	check[0].actual = sizeof(char) ;
	check[1].actual = sizeof(short) ;
	check[2].actual = sizeof(int) ;
	check[3].actual = sizeof(long) ;
	check[4].actual = sizeof(float) ;
	check[5].actual = sizeof(double) ;
	check[6].actual = sizeof(int *) ;
	check[7].actual = sizeof(time_t) ;
	check[8].actual = sizeof(struct char_file_u) ;
	check[9].actual = sizeof(struct char_ability_data) ;
	check[10].actual = sizeof(struct char_point_data) ;
	check[11].actual = sizeof(struct char_skill_data) ;
	check[12].actual = sizeof(struct affected_type) ;
}
