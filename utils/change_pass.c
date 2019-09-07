
#include <stdio.h>

#include "structs.h"

main()
{
	struct char_file_u st;
	char name[20];
	char passwd[20];
	long offset;
	FILE *FL;

	if( !(FL=fopen("players","r+") ) ) {
		printf("Error open file");
		exit(-1);
	}

	printf("Name? ");
	scanf("%s",name);
	printf("Passwd? ");
	scanf("%s",passwd);

	printf("%s\n",name);

	while(1) {
		fread( &st , sizeof( struct char_file_u ) , 1 , FL );
		if(strcmp(name,st.name)==0) {
			printf("found\n");

			printf("Changing\n");
			strcpy(st.pwd,(char *)crypt(passwd,st.name,10));
			*(st.pwd+10)='\0';

			offset=ftell(FL);
			offset-=sizeof(struct char_file_u);
			rewind(FL);
			fseek( FL , offset , 0 );
			fwrite( &st , sizeof( struct char_file_u ) , 1 , FL );
			printf("successful\n");

			break;
		}
	}
	fclose(FL);
}

