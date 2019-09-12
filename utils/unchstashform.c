#include	<stdio.h>
#include	<unistd.h>

int main(int argc,char *argv[])
{
	int	i;
	FILE	*f,*t;
	char	str[255]; //,str2[255];
	if(argc<2)
	{
		printf("usage: inverse <filename1> <filename2> ...\n");
		return 1;
	}
	for(i=1;i<argc;i++)
	{
		f = fopen(argv[i],"r");
		if ( f == NULL )
			continue;
		t = fopen("tmpstash","w");
		if ( t == NULL )
			continue;
		fgets(str,255,f);
		fputs(str,t);
		fgets(str,255,f);
		while ( !feof(f) )
		{
			fprintf(t,"%s",str);
			fgets(str,255,f);
			fgets(str,255,f);
			fgets(str,255,f);
			fgets(str,255,f);
		}
		unlink(argv[i]);
		rename("tmpstash",argv[i]);
		fclose(f);
		fclose(t);
	}
}
