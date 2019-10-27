
#include <stdio.h>
#include <string.h>
#include <ctype.h>

#ifndef __FreeBSD__
#include <crypt.h>
#endif 
#include <unistd.h>

#include "structs.h"

int main()
{
	struct char_file_u st;
	char name[20];
	char passwd[20];
	long offset;
	FILE *FL;
	int num=0;

	if( !(FL=fopen("players","r+") ) ) {
		printf("Error open 'players' file\n");
		exit(-1);
	}

	printf("Name? ");
	scanf("%s",name);
	printf("Passwd? ");
	scanf("%s",passwd);

	printf("%s\n",name);

	while(1) {
		fread( &st , sizeof( struct char_file_u ) , 1 , FL );
		printf("Processing #%d %s\n",num++,st.name);
		st.name[0]=tolower(st.name[0]);
		if(strcmp(name,st.name)==0) {
			printf("Found and changing password of [%s] : ", st.name);
			strcpy(st.pwd,(char *)crypt(passwd,st.name));
			*(st.pwd+10)='\0';

			offset=ftell(FL);
			offset-=sizeof(struct char_file_u);
			rewind(FL);
			fseek( FL , offset , 0 );
			fwrite( &st , sizeof( struct char_file_u ) , 1 , FL );
			printf("success.\n");

			break;
		}
	}
	fclose(FL);
}

