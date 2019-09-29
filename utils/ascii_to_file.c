/*
	ascii_to_file.c - translate ascii file to binary file
*/

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
#include "structs.h"

#define WRITE_FILE	"players.bin"

void build_player_index(char *player_file, int *n, struct char_file_u **np) ;
int read_one_player(FILE *fp, struct char_file_u *p, int num) ;
void read_ability(FILE *fp, struct char_ability_data *np, int num) ;
void read_point(FILE *fp, struct char_point_data *np, int num) ;
void read_skill(FILE *fp, struct char_skill_data *np) ;
void read_affected(FILE *fp, struct affected_type *np) ;
void write_result(int n, struct char_file_u *p) ;
char *fread_string(FILE *fl) ;
void debug(char *str) ;

/*  main procedure */
main(int argc, char *argv[])
{
	struct char_file_u *u_table ;
	int number_of_player ;
	char	name[BUFSIZ] ;

	if ( argc < 2 ) { 
		fprintf(stderr, "Usage: %s player_file\n", argv[0]) ;
		fprintf(stderr, "       adjust the player file and save into %s\n",
			WRITE_FILE) ;
		exit(1) ;
		}
	else {
		strcpy(name, argv[1]) ;
		debug("Start program") ;
		build_player_index(name, &number_of_player, &u_table);
		}

  write_result(number_of_player, u_table);
}

/* generate index table for the player file */
void build_player_index(char *file_name, int *nplayer,
	struct char_file_u **u_table)
{
	int	nr, i;
	char	tmp[BUFSIZ] ;
	FILE	*fp;

	sprintf(tmp, "Open player file %s\n", file_name) ;
	debug(tmp) ;
	if (!(fp = fopen(file_name, "r"))) {
		perror("build player index");
		exit(0);
		}

	/* Read number of players */
	nr = 0 ;
	fscanf(fp, "+%d", &nr) ;
	while(fgetc(fp) !='\n') ;
	*nplayer = nr ;

	sprintf(tmp, "Number of player expected = %d\n", nr) ;
	debug(tmp) ;

  *u_table = (struct char_file_u *) malloc( *nplayer * sizeof(struct char_file_u)) ;
  if (*u_table == NULL) {
    perror("malloc user table") ;
    exit(1) ;
    }

	for ( i = 0 ; i < nr ; i++) {
		if ( read_one_player(fp, &((*u_table)[i]), i) < 0 ) {
			sprintf(tmp, "read error : Number of player acutal = %d\n", i) ;
			debug(tmp) ;
			break ;
			}
		}

	debug("Closing input file") ;
	fclose(fp) ;
}

int read_one_player(FILE *fp, struct char_file_u *p, int num)
{
	int	j ;
	int	tmp1, tmp2, tmp3, tmp4, tmp5, tmp6, tmp7 ;
	char	line[BUFSIZ], *tline ;

	if ( feof(fp) ) {
		debug("read_one_player: END OF FILE ??") ;
		return (-1) ;
		}

	/* read one player - strings first */
	p->title[0] = p->description[0] = NULL ;
	p->name[0] = p->pwd[0] = NULL ;
	tline = fread_string(fp) ;
	if ( tline ) {
		strcpy(p->title, tline) ;
		free(tline) ;
		}
	tline = fread_string(fp) ;
	if ( tline ) {
		strcpy(p->description, tline) ;
		free(tline) ;
		}
	tline = fread_string(fp) ;
	if ( tline ) {
		strcpy(p->name, tline) ;
		free(tline) ;
		}
	tline = fread_string(fp) ;
	if ( tline ) {
		strcpy(p->pwd, tline) ;
		free(tline) ;
		}

	/* read stats */
	if ( fgets(line, BUFSIZ-1, fp) == NULL ) {
		fprintf(stderr, "ERROR: read player %s (%d)\n",
			p->name, num) ;
		exit(1) ;
		}
	sscanf(line, "%d %d %d %d %d %d %d %d %d", &tmp1, &tmp2, &tmp3, &tmp4,
		&(p->played), &(p->life), &tmp5, &tmp6, &tmp7) ;
	p->sex = (byte) tmp1 ;
	p->class = (byte) tmp2 ;
	p->level = (byte) tmp3 ;
	p->birth = (time_t) tmp4 ;
	p->weight = (time_t) tmp5 ;
	p->height = (time_t) tmp6 ;
	p->load_room = (sh_int) tmp7 ;
		
	if ( fgets(line, BUFSIZ-1, fp) == NULL ) {
		fprintf(stderr, "ERROR: read line %d\n", num) ; exit(1) ;
		}
	sscanf(line, "%d %d %d %d %d %d %d %d %d %d", &tmp1,
		&(p->alignment), &tmp2, &tmp3, &(p->bank),
		&(p->conditions[0]), &(p->conditions[1]), &(p->conditions[2]),
		&(p->quest[0]), &(p->quest[1])) ;
	p->spells_to_learn = (byte) tmp1 ;
	p->last_logon = tmp2 ;
	p->act = (unsigned) tmp3 ;

	for ( j = 0 ; j < 18 ; j ++) {
		fscanf(fp, "%d", &(p->reserved[j])) ;
		}
	while ( fgetc(fp) != '\n') ;

	read_ability(fp, &(p->abilities), num) ;
	read_point(fp, &(p->points), num) ;
	read_skill(fp, p->skills) ;
	read_affected(fp, p->affected) ;
	while ( fgetc(fp) != '$') ;
	while ( fgetc(fp) != '\n') ;

	return NULL ;
}

void read_ability(FILE *fp, struct char_ability_data *p, int num)
{
	int	tmp1, tmp2, tmp3, tmp4, tmp5, tmp6 ;
	char	line[BUFSIZ] ;

	if ( fgets(line, BUFSIZ-1, fp) == NULL ) {
		fprintf(stderr, "ERROR: read player %d\n", num) ; exit(1) ;
		}
	sscanf(line, "%d %d %d %d %d %d", &tmp1, &tmp2, &tmp3, &tmp4, &tmp5, &tmp6) ;
	p->str = (sbyte) tmp1 ;
	p->str_add = (sbyte) tmp2 ;
	p->intel = (sbyte) tmp3 ;
	p->wis = (sbyte) tmp4 ;
	p->dex = (sbyte) tmp5 ;
	p->con = (sbyte) tmp6 ;
}

void read_point(FILE *fp, struct char_point_data *p, int num)
{
	int	tmp1, tmp2, tmp3, tmp4, tmp5, tmp6, tmp7, tmp8 ;
	char	line[BUFSIZ] ;

	if ( fgets(line, BUFSIZ-1, fp) == NULL ) {
		fprintf(stderr, "ERROR: read player %d\n", num) ; exit(1) ;
		}
	sscanf(line, "%d %d %d %d %d %d %d %d %d %d %d\n", &tmp1, &tmp2,
		&(p->hit), &(p->max_hit), &tmp3, &tmp4, &tmp5, &tmp6,
		&(p->exp), &tmp7, &tmp8) ;
	p->mana = (sh_int) tmp1 ;
	p->max_mana = (sh_int) tmp2 ;
	p->move = (sh_int) tmp3 ;
	p->max_move = (sh_int) tmp4 ;
	p->armor = (sh_int) tmp5 ;
	p->gold = (unsigned int) tmp6 ;
	p->hitroll = (sh_int) tmp7 ;
	p->damroll = (sh_int) tmp8 ;
}

void read_skill(FILE *fp, struct char_skill_data *p)
{
	int	tmp1, tmp2 ;
	int	i ;

	for ( i = 0 ; i < MAX_SKILLS ; i++) {
		fscanf(fp, "%d %d", &tmp1, &tmp2) ;
		p[i].learned = (byte) tmp1 ;
		p[i].recognise = (bool) tmp2 ;
		}
	while( fgetc(fp) != 'S') ;
	while( fgetc(fp) != '\n') ;
}

void read_affected(FILE *fp, struct affected_type *p)
{
	int	tmp1, tmp2, tmp3, tmp4, tmp5, tmp6 ;
	int	i ;

	for ( i = 0 ; i < MAX_AFFECT ; i++) {
		fscanf(fp, "%d %d %d %d %d %d", &tmp1, &tmp2,
			&tmp3, &tmp4, &tmp5, &tmp6) ;
		p[i].type = (sbyte) tmp1 ;
		p[i].duration = (sh_int) tmp2 ;
		p[i].modifier = (sbyte) tmp3 ;
		p[i].location = (byte) tmp4 ;
		p[i].bitvector = (int) tmp5 ;
		p[i].next = (struct affected_type *) tmp6 ;
		}
	while( fgetc(fp) != 'A') ;
	while( fgetc(fp) != '\n') ;
}

void write_result(int nplayer, struct char_file_u *u_table)
{
	FILE	*fp ;
	int	i ;
	struct char_file_u *p;
	char	tmp[BUFSIZ] ;

	sprintf(tmp, "Open write file %s\n", WRITE_FILE) ;
	debug(tmp) ;
	if ((fp = fopen(WRITE_FILE, "w")) == NULL) {
		perror("write player file");
		exit(0);
		}

	for ( i = 0 ; i < nplayer ; i ++ ) {
      p = &(u_table[i]) ;
      if ( fwrite(p, sizeof(struct char_file_u), 1, fp) <= 0 ) {
			fprintf(stderr, "fwrite : actual number of write = %d\n", i) ;
			exit (1) ;
			}
		}
	fclose(fp);
}

char *fread_string(FILE *fl)
{
  char buf[MAX_STRING_LENGTH], tmp[BUFSIZ];
  char *rslt;
  register char *point;
  int i,flag;

	for ( i = 0 ; i < MAX_STRING_LENGTH ; i ++ ) 
		buf[i] = 0 ;

  do
  {
    if (!fgets(tmp, MAX_STRING_LENGTH, fl))
    {
      perror("fread_str");
      exit(0);

    }
    if (strlen(tmp) + strlen(buf) > MAX_STRING_LENGTH)
    {
      fprintf(stderr, "fread_string: string too large\n");
      buf[70]=0;
      fprintf(stderr,"%s\n",buf);
      exit(0);
    }
    else
      strcat(buf, tmp);

    for (point = buf + strlen(buf) - 2; point >= buf && isspace(*point);
      point--);
    if (flag = (*point == '~')) {
        *(buf + strlen(buf) -2) = NULL;
         }
    else
    {
      *(buf + strlen(buf)) = NULL;
    }
  }
  while (!flag);

  /* do the allocate boogie  */

  if (strlen(buf) > 0)
  {
    rslt = (char *) malloc (strlen(buf) + 1) ;
    strcpy(rslt, buf);
  }
  else
    rslt = 0;
  return(rslt);
}

void debug(char *str)
{
	fprintf(stderr, ":: %s\n", str) ;
}
