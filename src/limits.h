/* ************************************************************************
*  file: limits.h , Limit/Gain control module             Part of DIKUMUD *
*  Usage: declaration of title type                                       *
************************************************************************* */

#ifndef LIMITS_H
#define LIMITS_H

#include "structs.h"
/* Public Procedures */
int mana_limit(struct char_data *ch);
int hit_limit(struct char_data *ch);
int move_limit(struct char_data *ch);

struct title_type
{
	char *title_m;
	char *title_f;
	int exp;
};

#endif
