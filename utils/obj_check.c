/************************
	item_check program
 ***********************/

#include <stdio.h>
#include "structs.h"
#include "utils.h"

/* local procedures */
char *fread_string(FILE *fl) ;
void clear_object(struct obj_data *obj) ;

/* local variables */
int last_number = 0 ;
int wearing = 0 ;
int item_types = 0 ;

/* body of the booting system */
main(int argc, char *argv[])
{
	FILE	*obj_f ;
	extern int wearing, item_types ;

	if ( argc < 4 ) {
		fprintf(stderr, "Usage: %s object_file {wear | type} flag\n", argv[0]) ;
		exit(1) ;
		}

	if ( strcmp(argv[2], "type") == 0 ) {
		item_types = atoi(argv[3]) ;
		}
	else {
		wearing = atoi(argv[3]) ;
		}

	if (!(obj_f = fopen(argv[1], "r"))) {
		perror("main");
		exit(1);
		}

	check_object(obj_f);
      
}

check_item_print(struct obj_data *obj)
{
	static int	number = 0 ;

	/* in if state ment condition state can be below choice */
	/* GET_ITEM_TYPE(obj) == ITEM_BOAT and this can be one of
		ITEM_LIGHT ITEM_SCROLL ITEM_WAND ITEM_STAFF ITEM_WEAPON
		ITEM_FIREWEAPON ITEM_MISSILE ITEM_TREASURE ITEM_ARMOR ITEM_POTION
		ITEM_WORN ITEM_OTHER ITEM_TRASH ITEM_TRAP ITEM_CONTAINER ITEM_NOTE 
		ITEM_DRINKCON ITEM_KEY ITEM_FOOD ITEM_MONEY ITEM_PEN  ITEM_BOAT
	*/

	/* CAN_WEAR(obj, ITEM_TAKE) and this can be one of
		ITEM_TAKE ITEM_WEAR_FINGER ITEM_WEAR_NECK
		ITEM_WEAR_BODY ITEM_WEAR_HEAD ITEM_WEAR_LEGS ITEM_WEAR_FEET
		ITEM_WEAR_HANDS ITEM_WEAR_ARMS ITEM_WEAR_SHIELD ITEM_WEAR_ABOUT
		ITEM_WEAR_WAISTE ITEM_WEAR_WRIST ITEM_WIELD ITEM_HOLD ITEM_THROW
		ITEM_WEAR_KNEE ITEM_WEAR_ABOUTLEGS
	*/
	/* you can print GET_OBJ_WEIGHT(obj) */
	/* There is extra flags using IS_OBJ_STAT(obj, stat).
		stat is one of ITEM_GLOW ITEM_HUM ITEM_DARK ITEM_LOCK ITEM_EVIL 
		ITEM_INVISIBLE ITEM_MAGIC ITEM_NODROP ITEM_BLESS ITEM_ANTI_GOOD 
		ITEM_ANTI_EVIL ITEM_ANTI_NEUTRAL ITEM_NOLOAD ITEM_ANTI_MAGE
		ITEM_ANTI_CLERIC ITEM_ANTI_THIEF ITEM_ANTI_WARRIOR
	*/

	if ( wearing ) {
		if (CAN_WEAR(obj, wearing)) {
			printf("%6d   %s\n", item_cost(obj), obj->short_description) ;
			}
		}
	if ( item_types ) {
		if (GET_ITEM_TYPE(obj) == item_types) {
			printf("%6d   %s\n", item_cost(obj), obj->short_description) ;
			}
		}
		
/*
	if (CAN_WEAR(obj, ITEM_TAKE) && GET_ITEM_TYPE(obj) != ITEM_KEY) {
		number ++ ;
		printf("%4d: %5d (%5d) %s\n", number, item_cost(obj), obj->item_number,
				obj->short_description) ;
		printf("%6d   %s\n", GET_OBJ_WEIGHT(obj), obj->short_description) ;
		}
	if (CAN_WEAR(obj, ITEM_TAKE) && GET_ITEM_TYPE(obj) != ITEM_KEY) {
		number ++ ;
		printf("%4d: %5d (%5d) %s\n", number, item_cost(obj), obj->item_number,
				obj->short_description) ;
		printf("%6d   %s\n", item_cost(obj), obj->short_description) ;
		}
		*/
}

check_object(FILE *obj_f)
{
	char	line[BUFSIZ], buf[BUFSIZ], chk[BUFSIZ], *status ;
	struct obj_data *obj;
	int	tmp, i ;
	struct extra_descr_data *new_descr;
	int	flag ;
	extern int	last_number ;

	flag = 0 ;
	obj = (struct obj_data *) malloc(sizeof(struct obj_data));
	clear_object(obj);
	status = fgets(buf, 81, obj_f) ;
	for (;;) {
		if (flag) {
			check_item_print(obj) ;
			}
		if (status) {
			if (*buf == '#') {
				flag = 1 ;
				clear_object(obj);

				sscanf(buf, "#%d", &last_number);
  				obj->item_number = last_number ;

				/* *** string data *** */
				obj->name = fread_string(obj_f);
				obj->short_description = fread_string(obj_f);
				obj->description = fread_string(obj_f);
				obj->action_description = fread_string(obj_f);

				/* *** numeric data *** */
				fscanf(obj_f, " %d ", &tmp);
				obj->obj_flags.type_flag = tmp;
				fscanf(obj_f, " %d ", &tmp);
				obj->obj_flags.extra_flags = tmp;
				fscanf(obj_f, " %d ", &tmp);
				obj->obj_flags.wear_flags = tmp;
				fscanf(obj_f, " %d ", &tmp);
				obj->obj_flags.value[0] = tmp;
				fscanf(obj_f, " %d ", &tmp);
				obj->obj_flags.value[1] = tmp;
				fscanf(obj_f, " %d ", &tmp);
				obj->obj_flags.value[2] = tmp;
				fscanf(obj_f, " %d ", &tmp);
				obj->obj_flags.value[3] = tmp;
				fscanf(obj_f, " %d ", &tmp);
				obj->obj_flags.weight = tmp;
				fscanf(obj_f, " %d \n", &tmp);
				obj->obj_flags.cost = tmp;
				fscanf(obj_f, " %d \n", &tmp);
				obj->obj_flags.gpd = tmp;

				status = fgets(line, 81, obj_f) ;
				if (*line == '#') {
					strcpy(buf, line);
					continue ;
					}
				while (sscanf(line, " %s \n", chk), *chk == 'E') {
					new_descr = (struct extra_descr_data *) 
						malloc ( sizeof(struct extra_descr_data)) ;
					new_descr->keyword = fread_string(obj_f);
					new_descr->description = fread_string(obj_f);
					new_descr->next = obj->ex_description;
					obj->ex_description = new_descr;
					status = fgets(line, 81, obj_f) ;
					}
				if (*line == '#') {
					strcpy(buf, line);
					continue ;
					}

				for( i = 0 ; (i < MAX_OBJ_AFFECT) && (*chk == 'A') ; i++) {
					fscanf(obj_f, " %d ", &tmp);
					obj->affected[i].location = tmp;
					fscanf(obj_f, " %d \n", &tmp);
					obj->affected[i].modifier = tmp;
					status = fgets(line, 81, obj_f) ;
					if (*line == '#')
						break ;
					else
						sscanf(line, " %s \n", chk);
					}
				if (*line == '#') {
					strcpy(buf, line);
					continue ;
					}

				}
			else if (*buf == '$')  /* EOF */
  				break;
			}
		else {
			fprintf(stderr, "Last item: %d\n", last_number) ;
			perror("generate indices");
			exit(0);
    		}
  		}
}

/*************************************************************************
*  stuff related to the save/load player system                  *
*********************************************************************** */

/* read and allocate space for a '~'-terminated string from a given file */
char *fread_string(FILE *fl)
{
  char buf[MAX_STRING_LENGTH], tmp[500];
  char *rslt;
  register char *point;
  int flag;
	extern int	last_number ;

  bzero(buf, MAX_STRING_LENGTH);
  do
  {
    if (!fgets(tmp, MAX_STRING_LENGTH, fl))
    {
		fprintf(stderr, "Last item: %d\n", last_number) ;
      perror("fread_str");
      exit(0);
    }
    if (strlen(tmp) + strlen(buf) > MAX_STRING_LENGTH)
    {
      fprintf(stderr, "fread_string: string too large.\n");
		fprintf(stderr, "Last item: %d\n", last_number) ;
      exit(1);
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
	 rslt = (char *) malloc ( strlen(buf) + 1 ) ;
    strcpy(rslt, buf);
  }
  else
    rslt = 0;
  return(rslt);
}

void clear_object(struct obj_data *obj)
{
  bzero(obj, sizeof(struct obj_data));

  obj->item_number = -1;
  obj->in_room    = NOWHERE;
}

