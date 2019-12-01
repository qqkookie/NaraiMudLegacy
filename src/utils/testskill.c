
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>



/* creates a random number in interval [from;to] */
int number(int from, int to)
{
    register int d, tmp;

    if (from == to)
        return from;
    if (from > to) {
        tmp = from;
        from = to;
        to = tmp;
    }

    d = to - from + 1;

    if (d < 2)
        d = 2;
    return ((random() % d) + from);
}


int main()
{
    int lev_ch, dex_ch, hr_ch, skilled, learned, ac_vic, lev_vic, dex_vic;
    int i, j, old_percent, old_succ, new_percent, new_succ, dam , old_dam;
    /* static int list[] = { 0, 10, 30, 50, 100, -1, -1 } ; */
    /* static int list[] = { 20, 25, 30, 33, 35, 38, 40, -1, -1 } ; */
    /* static int list[] = { 0, 10, 30, 50, 100, -1, -1 } ; */
    // static int list[] = { 10, 30, 100, 500, -1 } ;
    static int list[] = { 10000, 100000, 500000, 1000000, -1 } ;
    int weight, price;

    srand(time(0));
    lev_ch = 40;
    dex_ch = 18;
    hr_ch = 150;
    learned = 95 ;
    skilled = 0 ;
    skilled = 30 ;
    skilled = 50 ;
    skilled = 100 ;

    ac_vic = -200;
    lev_vic = 40;
    dex_vic = 16;

    weight = 30;
    price = 100000;

for ( j = 0 ; list[j] != -1 ; j++ ) {
    // weight = list[j];
    price = list[j];

    old_succ = new_succ = 0 ;
    for(i=0; i < 10000; i++ ) {
    /*
	old_percent = number(1, 150) - (lev_ch + dex_ch) +
	    2 * (lev_vic + dex_vic) - (skilled >> 3);
	new_percent = number(1,101) - ( ac_vic + hr_ch )/16 -10
		    - ( lev_ch + 2* dex_ch )
		    + 2 * ( lev_vic + 2*dex_vic) - skilled / 6;
    */

/*
	dam = (int) sqrt((double)500000)+ (int)(sqrt((double)(50)))*100;
	dam = (int) sqrt((double)500000)+ (int)(sqrt((double)(450)))*100;
*/
	dam = price/1000 + weight*2 +20;
	dam *= (lev_ch + 18*2 + 10 + 18 +20 );
	dam *= ( lev_ch + skilled /8 );
	dam /= 100 ;
	old_dam = price /1000 + ( lev_ch + 18 + 80 + 18 ) ;
	old_dam *= ( lev_ch + skilled /8 );

	old_percent = number(0,100) -lev_ch + 2* lev_vic - (skilled >>2) ;
	new_percent = 1000;

	if( old_percent < learned ) old_succ++;
	if( new_percent < learned ) new_succ++;
    }
    printf("J=%d, skill value = %d old_dam = %d dam = %d\n", j , list[j], old_dam, dam );
    printf("Try = %d, old succ = %d, new succ = %d \n", i, old_succ, new_succ);
    printf("ignore %d\n", dex_vic+ac_vic+hr_ch+dex_ch);
}

}


