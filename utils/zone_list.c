#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define MAX_ITEMS 1000

static int com_func(i,j)
int *i,*j;
{
	return (*i-*j);
}

int main(int argc,char *argv[])
{
	FILE *fp;
	char buf[81];
	char zonename[81];
	char zonefile[100];
	char chk,old_chk=0;
	int flag,a1,a2,a3;
	int a,b,c;
	int line=2;
	int items[MAX_ITEMS];
	int no_item=0,i,old_item=0;

	fp=fopen(argv[1],"r");
	if(fp==NULL)
		exit(1);
	fgets(zonename,80,fp);
	/*printf("Zone name : %s \n",buf);*/
	fscanf(fp," %d %d %d ",&a,&b,&c);
/*	printf("top : %d,life span : %d,reset mode : %d",a,b,c);*/
	while(1)
	{
		line++;
		fscanf(fp," ");
		fscanf(fp,"%c",&chk);
		if(chk=='S')
		{
			fclose(fp);
			break;
		}
		else if(chk=='*')
		{
			fgets(buf,80,fp);
			continue;
		}
		else 
		{
			fscanf(fp," %d %d %d",&flag,&a1,&a2);
			if(chk=='M'||chk=='O'||chk=='E'||chk=='P'||chk=='D')
				fscanf(fp," %d",&a3);
			fgets(buf,80,fp);
			switch(chk)
			{
			case 'M':
				old_chk='M';
				break;		
			case 'G':
				if(old_chk=='M'||old_chk=='G'||old_chk=='E')
				{
/*				printf("GIVE %d with limit(%d),flag(%d)\n",
					a1,a2,flag);*/
				old_chk='G';
				items[no_item++]=a1;
				}
				else
				{
					fprintf(stderr,"GIVE ERROR in line %d!\n",line);
				}
				break;		
			case 'O':
/*				printf("OBJ(%d) in %d limit(%d),flag(%d)\n",
					a1,a3,a2,flag);*/
				old_chk='O';
				items[no_item++]=a1;
				break;		
			case 'P':
				if( old_chk=='O' || old_chk=='P' )
				{
/*				printf("OBJ(%d) into %d with limit(%d),flag(%d)\n",
					a1,a3,a2,flag);*/
					old_chk='P';
				items[no_item++]=a1;
				}
				else
				{
				fprintf(stderr,"ERROR in line %d!\n",line);
				old_chk=' ';
				}
				break;		
			case 'D':
				old_chk='D';
				break;
			case 'E':
				if( old_chk=='E' || old_chk=='G' || old_chk=='M' )
				{
/*				printf("OBJ(%d) in %d limit(%d),flag(%d)\n",
					a1,a3,a2,flag);*/
				old_chk='E';
				items[no_item++]=a1;
				}
				else
				{
				fprintf(stderr,"ERROR in line %d!\n",line);
				old_chk=' ';
				}
				break;		
			default:
				fprintf(stderr,"ERROR in line %d!\n",line);
				break;
			}
		}
	}
	qsort(items,no_item,sizeof(int),com_func);

	for(i=0;i<strlen(zonename);i++)
	{
		if(zonename[i]==' ') zonename[i]='_';
	}
	zonename[i-2]='\0';
	sprintf(zonefile,"../object/%s.lst",zonename);
	fp=fopen(zonefile,"w");
	for(i=0;i<no_item;i++)
	{
		if(old_item!=items[i])
		{
		fprintf(fp,"%d\n",items[i]);
		old_item=items[i];
		}
	}
	fclose(fp);
}
