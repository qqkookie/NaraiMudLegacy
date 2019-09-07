#include <stdio.h>
#include <string.h>

main(argc,argv)
int argc;
char *argv[];
{
	char buf[255];
	int n;
	strcpy(buf,argv[1]);
	n=strlen(buf);
	buf[n-4]='\0';
	printf("%s",buf);
}
