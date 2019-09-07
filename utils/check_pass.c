
#include <stdio.h>
#include <ctype.h>
#include "structs.h"

main()
{
	struct char_file_u st;
	char name[20];
	char passwd[20];
	long offset;
	FILE *FL;
	int num=0;

	if( !(FL=fopen("players.tmp","r+") ) ) {
		printf("Error open file");
		exit(-1);
	}

	printf("Name? ");
	scanf("%s",name);
	printf("Passwd? ");
	scanf("%s",passwd);

	while(1) {
		if(fread( &st , sizeof( struct char_file_u ) , 1 , FL )==0) break;
		printf("Processing #%d %s\n",num++,st.name);
		st.name[0]=tolower(st.name[0]);
		if(strcmp(name,st.name)==0) {
			printf("found\n");

			printf("Changing\n");
			if(strncmp(st.pwd,(char *)crypt(passwd,st.pwd),10)==0) {
				printf("Okie\n");
			} else {
				printf("False\n");
			}

			break;
		}
	}
	fclose(FL);
}
