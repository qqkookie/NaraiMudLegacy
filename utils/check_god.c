#include <stdio.h>

#include "structs.h"

#define FILE_NAME1 "players"
#define FALSE 0
#define TRUE 1

int main()
{
	FILE *old;
	struct char_file_u ch;
	int ok = TRUE;
	// char buf[256];
	int count = 0;
	static int a[4];
	int dan_count = 0;

	old = fopen(FILE_NAME1, "r");
	do {
		ok = fread(&ch, sizeof(ch), 1, old);
		if (ok == FALSE)
			break;
	    if(ch.level == 44)
		printf("NAME:%s   NAME(D): %d  LEVEL: %d HP: %d GUILD: %d\n\r",
				ch.name,ch.name[0],ch.level, ch.points.max_hit, ch.guild);

	    a[ch.guild]++;	
	    count++;
	    if(ch.level == 40)
		dan_count++;

	} while(!feof(old));
	printf("total : %d\n", count);
	printf("%d danguns total\n", dan_count);
	printf("None : %d, Police : %d, Outlaw : %d, Assasin : %d\n",
		a[0], a[1], a[2], a[3]);
	fclose(old);
}
