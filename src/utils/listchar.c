/* *****************************************************
    More detailed listing about Dangun statistics.
    Modified from OLD list.c
   ***************************************************** */
#include <stdio.h>
#include <string.h>
#include <time.h>
#include "char.h"
#include "gamedb.h"

#define FILE_NAME "players"

/*
char *REMO(byte remortal)
{
	static char remoed[10];
	remoed[0] = (remortal & REMORTAL_MAGIC_USER)? 'M' : ' ' ;
	remoed[1] = (remortal & REMORTAL_CLERIC) ? 'C' : ' ' ;
	remoed[3] = (remortal & REMORTAL_THIEF) ? 'T' : ' ' ;
	remoed[4] = (remortal & REMORTAL_WARRIOR ) ? 'W' : ' ' ;
	return(remoed);
}
*/

FILE *fp;

void show_player(struct char_file_u *st)
{
    char remoed[10], *tp;
    int c;
    static char *guild[] = { "None", "Police", "Outlaw", "Assasin", };

    strcpy(remoed, "MCTW");
    for (c = 0 ; c < 4  ; c++ )
	if ( !(st->remortal & ( 01 << c )))
	    remoed[c] = ' ' ;

    printf("%-8s :: %c %2d HP: %5d  MA: %5d MV %5d / HD: %02dd%02d "
	    " AC: %4d DR: %3d\n",
	st->name, "UMCTWU"[st->class], st->level,
	st->points.max_hit, st->points.max_mana,st->points.max_move,
	st->damnodice, st->damsizedice, st->points.armor, st->points.damroll);

    tp = ctime( &(st->last_logon));
    tp[16] = '\0';
    printf("%6s  REMO: %4s  %10lld M GOLD  SEX: %c  GUILD: %s  LOG: %s\n\n",
	    "", remoed, (st->bank/1000 + st->points.gold/1000)/1000,
	    "NMF"[st->sex], guild[st->guild] , tp);
}

void danguns()
{
    struct char_file_u ch;
    int count, dan_count, wiz_count, non_dan, all_remo;
    int c, nremo ;
    int a[4];

    count = 0;
    wiz_count = non_dan = dan_count  = all_remo = 0;
    a[0] = a[1] = a[2] = a[3] = 0 ;

    while( fread(&ch, sizeof(ch), 1, fp) > 0 ) {
	count++;

	nremo = 0 ;
	for (c = 0 ; c < 4  ; c++ )
	    if ( (ch.remortal & ( 01 << c )))
		nremo++;

	if ( nremo <=1 && ch.level < 40 ) 	/* Not dangun */
	    continue ;

	show_player(&ch);

	a[ch.guild]++;

	if(ch.level > 40)
		wiz_count++;
	else if(ch.level == 40) {
	    dan_count++;
	    if ( nremo == 4 )
		all_remo++ ;
	}
	else
		non_dan++;

    }
    printf("    Total player : \t%3d\n", count);
    printf("    Wizard :\t\t%3d\n    Dangun :\t\t%3d\n"
    	   "    Remoed non-dangun :\t%3d\n",
    		wiz_count, dan_count, non_dan ) ;
    printf("    All remo dangun :\t%3d     (out of %3d remoed players)\n" ,
   		all_remo, (dan_count + non_dan )) ;

    printf("\nRemo player GUILD::   None: %d,   Police: %d,   Outlaw: %d,  "
    		" Assasin: %d\n\n", a[0], a[1], a[2], a[3]);
}

int main(int argc, char **argv )
{
    struct char_file_u ch;

    fp = fopen(FILE_NAME, "r");

    if (  argc > 1 && argv[1] ){
	while( fread(&ch, sizeof(ch), 1, fp) > 0 ) {
	    if ( strcasecmp(ch.name, argv[1]) == 0) {
		show_player(&ch);
		return 0;
	    }
	}
	printf("\nCan not find such player\n");
    }
    else
	danguns();
    return 0;
}

