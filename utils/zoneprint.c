/* 
	zone list program by atre@paradise.kaist.ac.kr
	usage : zone [options] zone-file
	options
		-M -O, -G, -P, -R, -E, -D : disable display-command
		-N : print mobile if maximum is 1
*/
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
#include <stdlib.h>

#include "structs.h"
#include "utils.h"

#undef log

#define log(str) mylog(str)
#define mylog(str) fprintf(stderr, str)

char *fread_string(FILE *fl);

int Mflag = 1;
int Oflag = 1;
int Gflag = 1;
int Pflag = 1;
int Rflag = 1;
int Eflag = 1;
int Dflag = 1;

int Nflag = 0;

int target = 0;

void PrintZone(char command, int if_flag, int arg1, int arg2, int arg3)
{
	static char *equip[] = {
      "LIGHT",
      "FINGER RIGHT",
      "FINGER LEFT",
      "NECK 1",
      "NECK 2",
      "BODY",
      "HEAD",
      "LEGS",
      "FEET",
      "HANDS",
      "ARMS",
      "SHIELD",
      "ABOUT",
      "WAISTE",
      "WRIST RIGHT",
      "WRIST LEFT",
      "WIELD",
      "HOLD"
	};

	static char *door[] = {
		"opened",
		"closed",
		"closed and locked"
	};

	static char *exit[] = {
		"north",
		"east",
		"south",
		"west",
		"up",
		"down"
	};

	switch(command){
		case 'M' :
			if(Mflag){
				printf("Load Mobile %d in %d, with maximum %d\n",
					arg1, arg3, arg2);
			}
			if(Nflag){
				if(arg2 == 1){
					printf("%d with maximum 1\n", arg1);
				}
			}
			if(if_flag){
				target = arg1;
			}
			break;
		case 'O' :
			if(Oflag){
				printf("Load Object %d in %d, with maximum %d\n",
					arg1, arg3, arg2);
			}
			if(if_flag){
				target = arg1;
			}
			break;
		case 'G' :
			if(Gflag){
				printf("Give Object %d to %d, with maximum %d\n",
					arg1, target, arg2);
			}
			break;
		case 'P' :
			if(Pflag){
				printf("Put Object %d in %d, with maximum %d\n",
					arg1, arg3, arg2);
			}
			break;
		case 'R' :
			if(Rflag){
				printf("Remove Object %d in %d\n",
					arg1, arg2);
			}
			break;
		case 'E' :
			if(Eflag){
				printf("Equip Object %d to %d on %s, with maximum %d\n",
					arg1, target, equip[arg3], arg2);
			}
			break;
		case 'D' :
			if(Dflag){
				printf("Make Door to %s in %d %s\n",
					exit[arg2], arg1, door[arg3]);
			}
			break;
		default  :
			fprintf(stderr, "Invalid Command(%c) in Zone data file\n", command);
	}
}

void ReadZone(FILE *fp)
{
	char *name;
	int lifespan, /* age, */ top;
	int reset_mode;
	// int zon = 0, cmd_no = 0 , expand;
	char command;
	int arg1, arg2, arg3, if_flag;
	// char *check, buf[81];
	// char file_name[100];
	// int len;
	char buf[81];

	name = fread_string(fp);

	fscanf(fp, " %d ", &top);
	fscanf(fp, " %d ", &lifespan);
	fscanf(fp, " %d ", &reset_mode);

	/* print zone header */
	printf("Zone       : %s\n", name);
	printf("top        : %d\n", top);
	printf("lifespan   : %d\n", lifespan);
	printf("reset mode : %d\n", reset_mode);

	/* read the command table */
	while(1){
		fscanf(fp, " ");	/* skip space */
		fscanf(fp, "%c", &command);

		/* end of each zone file */
		if (command == 'S') {
			return;
		}

		if (command == '*') {
			fgets(buf, 80, fp); /* skip command */
			continue;
		}

		fscanf(fp, " %d %d %d", 
			&if_flag,
			&arg1,
			&arg2);

		if(command == 'M' || command == 'O' ||
			command == 'E' || command == 'P' ||
			command == 'D') fscanf(fp, " %d", &arg3);

		fgets(buf, 80, fp);  /* read comment */

		PrintZone(command, if_flag, arg1, arg2, arg3);
	}
}

int main(int argc, char **argv)
{
	int i;
	char *filename;
	char cmd;
	FILE *fp;

	filename = NULL;

	for(i = 1; i < argc; i ++){
		if(*argv[i] == '-'){
			cmd = argv[i][1];
			switch(cmd){
				case 'm' :
				case 'M' : Mflag = 0; break;
				case 'o' :
				case 'O' : Oflag = 0; break;
				case 'g' :
				case 'G' : Gflag = 0; break;
				case 'p' :
				case 'P' : Pflag = 0; break;
				case 'r' :
				case 'R' : Rflag = 0; break;
				case 'e' :
				case 'E' : Eflag = 0; break;
				case 'd' :
				case 'D' : Dflag = 0; break;
				case 'n' :
				case 'N' :
					Nflag = 1;
					Mflag = Oflag = Gflag = Pflag = Rflag = Eflag = Dflag = 0;
					break;
				default  :
					fprintf(stderr, "%s : invalid parameter %s\n",
						argv[0], argv[i]);
			}
		}
		else {
			filename = argv[i];
		}
	}

	if(!filename){
		fprintf(stderr, "Usage : %s [options] zone-filename\n", argv[0]);
		exit(1);
	}

	if((fp = fopen(filename, "r")) == NULL){
		fprintf(stderr, "Error : file open failure(%s).\n", filename);
		fprintf(stderr, "Usage : %s [options] zone-filename\n", argv[0]);
		exit(1);
	}

	ReadZone(fp);

	fclose(fp);
}

// ==============================================================


/* clear char_data */
void clear_char(struct char_data *ch)
{
  bzero(ch, sizeof(struct char_data));

  ch->in_room = NOWHERE;
  ch->specials.was_in_room = NOWHERE;
  ch->specials.position = POSITION_STANDING;
  ch->specials.default_pos = POSITION_STANDING;
  GET_AC(ch) = 1; /* Basic Armor */
}

/* read and allocate space for a '~'-terminated string from a given file */
char *fread_string(FILE *fl)
{
  char buf[MAX_STRING_LENGTH], tmp[MAX_STRING_LENGTH];
  char *rslt;
  register char *point;
  int flag;

  bzero(buf, MAX_STRING_LENGTH);
  do
  {
    if (!fgets(tmp, MAX_STRING_LENGTH, fl))
    {
      log("fread_str");
      exit(0);
    }
    if (strlen(tmp) + strlen(buf) > MAX_STRING_LENGTH)
    {
      log("fread_string: string too large (db.c)");
      buf[70]=0;
      fprintf(stderr,"%s\n",buf);
      exit(0);
    }
    else
      strcat(buf, tmp);

    for (point = buf + strlen(buf) - 2; point >= buf && isspace(*point);
      point--);    
    if ((flag = (*point == '~')))
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

