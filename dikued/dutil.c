/****************************************************************************
 *                                                                          *
 *             dutil.c (c) by Georg Essl in 1992                            *
 *                                                                          *
 ****************************************************************************/

/*** INCLUDES ***/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "dutil.h"


int strnccmp(char *wstr1, char *wstr2,int wint)
{
    int i;

    for (i = 0; (*(wstr1 + i) || *(wstr2 + i)) && (wint>0); i++, wint--)
        if (tolower(*(wstr1 + i)) - tolower(*(wstr2 + i)))
            return (1);

    return(0);
}

/* Ask for Yes or No */

int AskYesNo(char *wstr)
{
    char wstr2[255];

    gets(wstr2);
    if((*wstr2!='\0')&&(!strnccmp(wstr,wstr2,strlen(wstr2))))
        return(1);
    else
        return(0);
}


/* read and allocate space for a '~'-terminated string from a given file */

char *fread_string(FILE *fl)
{
	char *buf, tmp[500];
	char *rslt;
	register char *point;
	int flag;

    if((buf=malloc(MAX_STRING_LENGTH))==NULL)
        return(buf);


	buf[0]='\0';
	buf[1]='\0';

	do
	{
		if (!fgets(tmp, MAX_STRING_LENGTH, fl))
		{
			perror("fread_str");
			exit(0);
		}

		if (strlen(tmp) + strlen(buf) > MAX_STRING_LENGTH)
		{
			printf("***ERROR: fread_string: string too large \n");
			exit(0);
		}
		else
			strcat(buf, tmp);

		for (point = buf + strlen(buf) - 2; point >= buf && isspace(*point);
             point--);
		if (flag = (*point == '~'))
			if (*(buf + strlen(buf) - 3) == '\n')
			{
				*(buf + strlen(buf) - 2) = '\r';
				*(buf + strlen(buf) - 1) = '\0';
			}
			else
				*(buf + strlen(buf) -2) = '\0';
		else
		{
			*(buf + strlen(buf) + 1) = '\0';
			*(buf + strlen(buf)) = '\r';
		}
	}
	while (!flag);

	/* do the allocate boogie  */

	if (strlen(buf) > 0)
	{
		CREATE(rslt, char, strlen(buf) + 1);
		strcpy(rslt, buf);
	}
	else {
		// rslt = 0;
        CREATE(rslt, char, strlen("") + 1);
		strcpy(rslt, "");
    }

    free(buf);

	return(rslt);
}

long flag_convert(char *argument)
{

    long flagval = 0;
    int i;
    char buf[52];

    strcpy(buf, argument);

    for (i = 0; i < strlen(argument); i++)
        if (buf[i] != '0')
            flagval += FLAG(buf[i]);
        else return(flagval);

    // sprintf(buf, "convert flag: %ld",flagval);
    // log(buf);
    return(flagval);
}

/* generate index table for object or monster file */

struct index_data *generate_indices(FILE *fl, int *top)
{
	int i = 0;
	struct index_data *index;
	long pos,tmp2,tmp3,otmp3;
	char buf[82];

	rewind(fl);

	tmp3=0;
	for (;;)
	{
		tmp2=ftell(fl);
		if (fgets(buf, 81, fl))
		{
			otmp3=tmp3;
			tmp3+=strlen(buf);
			if (*buf == '#')
			{
				/* allocate new cell */

				if (!i)						 /* first cell */
					CREATE(index, struct index_data, 1);
				else
					if (!(index =
                          (struct index_data*) realloc(index,
                                                       (i + 1) * sizeof(struct index_data))))
					{
						perror("load indices");
						exit(0);
					}

				/***** WARNING!!! NOT LIKE IN ORIGINAL DIKUMUD CODE !!! ****/

				index[i].pos = tmp2;
				index[i].cpos= otmp3;
				sscanf(buf, "#%d", &index[i].virtual);
				i++;
			}
			else
			{
				if (*buf == '$')	/* EOF */
					break;
			}
		}
		else
		{
			perror("Unexpected end of db-file while generating indices");
			exit(0);
		}
	}
	*top = i - 2;
	return(index);
}


/* string to file */

void fwrite_string(FILE *wfile,char *wstr)
{
    char *tempstr;
    int  i,j;
    if(wstr!=NULL)
    {
        CREATE(tempstr,char ,strlen(wstr)+1);
        j=0;
        for(i=0;i<=strlen(wstr);i++)
        {
            if(wstr[i]!='\r')
            {
                tempstr[j]=wstr[i];
                j++;
            }
        }

        fprintf(wfile,"%s~\n",tempstr);
        free(tempstr);
    }
    else
        fprintf(wfile,"~\n");
}

/* Writes out multi-line strings with new lines in position 'space'.   */
/* Used in e.g. print_room, print_obj and print_mob                    */


void write_string(char *wstr, int space)
{
    int  i,k;
    char *lastc;
    lastc = wstr;

    if(wstr!=NULL)
    {
        for(i=0;i<=strlen(wstr);i++)
        {
            if (*lastc == '\n')
                for (k=0;k<space;k++) 
                    printf(" ");
            if(wstr[i]!='\r')
                putchar(wstr[i]);
            lastc = &wstr[i];
        }
        printf("\n");
    }
    else
        printf("\n");
}


/* Read multi-line strings (adopted from fread_string()) */

char *read_multi_string(void)
{
	char buf[MAX_STRING_LENGTH], tmp[500];
	char *rslt;
	register char *point;
	int flag;

	buf[0]='\0';
	buf[1]='\0';

	do
	{
		gets(tmp);

		if (strlen(tmp) + strlen(buf) > MAX_STRING_LENGTH)
		{
			printf("***ERROR: fread_string: string too large\n");
			exit(0);
		}
		else
			strcat(buf, tmp);
        strcat(buf, "\n");

		for (point = buf + strlen(buf) - 2; point >= buf && isspace(*point);
             point--);
		if (flag = (*point == '~'))
			if (*(buf + strlen(buf) - 3) == '\n')
			{
				*(buf + strlen(buf) - 2) = '\r';
				*(buf + strlen(buf) - 1) = '\0';
			}
			else
				*(buf + strlen(buf) -2) = '\0';
		else
		{
			*(buf + strlen(buf) + 1) = '\0';
			*(buf + strlen(buf)) = '\r';
		}
	}
	while (!flag);

	/* do the allocate boogie  */

	if (strlen(buf) > 0)
	{
		CREATE(rslt, char, strlen(buf) + 1);
		strcpy(rslt, buf);
	}
	else
		rslt = 0;
	return(rslt);
}

/* Get and free *.DEF-files */

int GetADef(FILE *a_file,char *a_fname,char *dbpath,char ***a_str,int *a_max)
{
    int i;
    char *wstr;

    CREATE(wstr, char, strlen(dbpath)+strlen(a_fname)+1);
    strcpy(wstr,dbpath);
    strcat(wstr,a_fname);

    if(!(a_file=fopen(wstr,"r")))
    {
        free(wstr);
        return(DU_ERR_CANTOPEN);
    }

    free(wstr);

    fscanf(a_file,"%d\n",a_max);

    *a_str=malloc(sizeof(char *)* (*a_max+1));
    for(i=0;i<=*a_max;i++)
    {
        (*a_str)[i]=fread_string(a_file);
    }
    fclose(a_file);
    return(DU_ERR_NOERROR);
}

void FreeADef(char **a_str,int a_max)
{
    int i;
    for(i=0;i<=a_max;i++)
    {
        free(a_str[i]);
    }
    free(a_str);
}

