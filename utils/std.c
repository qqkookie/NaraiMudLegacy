#include <stdio.h>
#include <math.h>

get_std(int level, int *hitroll, int *damage, int *armor, int *defense,
	int *experience, *int gold) 
{
	int	n ;
	
	int	tmp_damage, tmp_hitroll, tmp_defense, tmp_armor, tmp_gold ;
	int	dam_def, xp, tmp1, tmp2 ;

	dam_def = pow((double) level, (double) 3.5) ;
	tmp1 = 1 + 0.2 * level * level ;
	tmp2 = 1.5 * level ;
	if ( tmp1 > tmp2 )
		tmp_damage = tmp1 ;
	else
		tmp_damage = tmp2 ;
	tmp_defense = dam_def/tmp_damage + 5 ;
	tmp_hitroll = 0.8*level ;
	tmp_armor = 100 - 7*level ;
	xp = 2 * level*level*level * log ((double) level) + 25 ;
	tmp_gold = 0.1 * xp ;

	*hitroll = tmp_hitroll ;
	*damage = tmp_damage ;
	*armor = tmp_armor ;
	*defense = tmp_defense ;
	*experience = xp ;
	*gold = tmp_gold ;

}
