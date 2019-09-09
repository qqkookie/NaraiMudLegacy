#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
#include <stdlib.h>

#define CREATE(result, type, number)  do {\
  if (!((result) = (type *) calloc ((number), sizeof(type))))\
	  { perror("malloc failure"); abort(); } } while(0)

#define MAX_STRING_LENGTH 3000

#define log(str)	fprintf(stderr, "%s\n", str)

#define OverBelow(n)	{ if(n > 10) over ++; if(n < 10) below ++;}

/* Bitvector for 'affected_by' */
#define AFF_BLIND             1
#define AFF_INVISIBLE         2
#define AFF_DETECT_EVIL       4
#define AFF_DETECT_INVISIBLE  8
#define AFF_INFRAVISION       16
#define AFF_SENSE_LIFE        32
#define AFF_DEAF				64 /* by process */
#define AFF_SANCTUARY         128
#define AFF_GROUP             256
#define AFF_DUMB			  512 /* by process */
#define AFF_CURSE             1024
#define AFF_MIRROR_IMAGE      2048
#define AFF_POISON            4096
#define AFF_PROTECT_EVIL      8192
#define AFF_REFLECT_DAMAGE	16384 /* by process */
#define AFF_HOLY_SHIELD			32768
#define AFF_SPELL_BLOCK       65536 /* by process */
#define AFF_SLEEP             131072
#define AFF_SHADOW_FIGURE     262144 /* by process */ 
#define AFF_SNEAK             524288
#define AFF_HIDE              1048576
#define AFF_DEATH			  2097152
#define AFF_CHARM             4194304
#define AFF_FOLLOW            8388608
#define AFF_HASTE	          16777216
#define AFF_IMPROVED_HASTE    33554432
#define AFF_LOVE              67108864 /* by perhaps */
#define AFF_RERAISE           134217728 /* by chase */
#define AFF_ARREST            268435456 /* by sypark */

/* for mobile actions: specials.act */
#define ACT_SPEC         1     /* special routine to be called if exist   */
#define ACT_SENTINEL     2     /* this mobile not to be moved             */
#define ACT_SCAVENGER    4     /* pick up stuff lying around              */
#define ACT_ISNPC        8     /* This bit is set for use with IS_NPC()   */
#define ACT_NICE_THIEF  16     /* Set if a thief should NOT be killed     */
#define ACT_AGGRESSIVE  32     /* Set if automatic attack on NPC's        */
#define ACT_STAY_ZONE   64     /* MOB Must stay inside its own zone       */
#define ACT_WIMPY      128     /* MOB Will flee when injured, and if      */
                               /* aggressive only attack sleeping players */
#define ACT_FIGHTER    256
#define ACT_MAGE       512
#define ACT_CLERIC    1024
#define ACT_THIEF     2048
#define ACT_PALADIN   4096
#define ACT_DRAGON    8192
#define ACT_SPITTER  16384
#define ACT_SHOOTER  32768
#define ACT_GUARD    65536
#define ACT_SUPERGUARD   131072
#define ACT_GIANT        262144
#define ACT_HELPER       524288		/* changed by cyb */
#define ACT_ORC          524288		/* changed by cyb */
#define ACT_RESCUER     1048576
#define ACT_SPELL_BLOCKER  2097152
#define ACT_FINISH_BLOW    4194304

int zone;
int new_zone;

struct {
	int number;
	char *name;
} zone_data[46] = {
		{ 99, "LIMBO" },
		{ 299, "TheCastle" },
		{ 1399, "MelSDoghouse" },
		{ 1499, "Houses" },
		{ 1599, "DirkSCastle" },
		{ 1799, "SHIRE" },
		{ 1899, "TheJale" },
		{ 1999, "TheLands" },
		{ 2099, "ProcessCastle" },
		{ 2199, "TheWasteland" },
		{ 2300, "DragonTower" },
		{ 2399, "Muncie" },
		{ 2699, "TheCorporation" },
		{ 2799, "TheNeverLand" },
		{ 2899, "KingdomOfChok" },
		{ 2999, "TheKeepOfMahnTor" },
		{ 3099, "NorthernMidgaardMainCity" },
		{ 3199, "SouthernPartOfMidgaard" },
		{ 3299, "RiverOfMidgaard" },
		{ 3499, "Graveyard" },
		{ 4099, "MoriaLevel" },
		{ 4199, "MoriaLevel" },
		{ 4330, "TheWamphyriAerie" },
		{ 5099, "TheGreatEasternDesert" },
		{ 5199, "DrowCity" },
		{ 5299, "TheCityOfThalos" },
		{ 6099, "HAONDORLIGHT" },
		{ 6499, "HAONDORDARK" },
		{ 6999, "TheDwarvenKingdom" },
		{ 7099, "Sewer" },
		{ 7199, "SecondSewer" },
		{ 7399, "SEWERMAZE" },
		{ 7899, "TheTunnels" },
		{ 7999, "RedferneSResidence" },
		{ 9099, "Arachnos" },
		{ 9499, "Arena" },
		{ 9699, "DeathKingdom" },
		{ 9771, "Galaxy" },
		{ 9851, "TheDeathStar" },
		{ 12099, "EasyZone" },
		{ 13798, "MountOlympus" },
		{ 15299, "RobotCity" },
		{ 16199, "KingdomOfWee" },
		{ 17099, "OKingdom" },
		{ 18500, "MooDang" },
		{ 33000, "KAIST" }
/* 
{ 20200, "MooDang" },
{ 33000, "KAIST" }
*/
	};

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

#define PRINT_HEADER { \
	printf("\n\nNUM     %-20s     Mobile Names : Lv Ht AC HR DR XP CLS SEX\n", \
			zone_data[zone].name);		\
	printf("-------------------------------------------------------------------------\n"); }

#define IS_SET(flag, bit)	(flag & bit)
void PrintAct(int act)
{
	char buf[256];

	buf[0] = 0;
	if(IS_SET(act, ACT_SENTINEL)) strcat(buf, "NO_MOVE ");
	if(IS_SET(act, ACT_SCAVENGER)) strcat(buf, "PICKUP ");
	if(IS_SET(act, ACT_NICE_THIEF)) strcat(buf, "STEAL ");
	if(IS_SET(act, ACT_AGGRESSIVE)) strcat(buf, "AGGR ");
	if(IS_SET(act, ACT_STAY_ZONE)) strcat(buf, "STAY ");
	if(IS_SET(act, ACT_WIMPY)) strcat(buf, "WIMPY ");
	if(IS_SET(act, ACT_PALADIN)) strcat(buf, "PALADIN ");
	if(IS_SET(act, ACT_DRAGON)) strcat(buf, "DRAGON ");
	if(IS_SET(act, ACT_SPITTER)) strcat(buf, "SPIT ");
	if(IS_SET(act, ACT_SHOOTER)) strcat(buf, "SHOOT ");
	if(IS_SET(act, ACT_GUARD)) strcat(buf, "GUARD ");
	if(IS_SET(act, ACT_SUPERGUARD)) strcat(buf, "SUPERGUARD ");
	if(IS_SET(act, ACT_HELPER)) strcat(buf, "HELPER ");
	if(IS_SET(act, ACT_RESCUER)) strcat(buf, "RESCUER ");
	if(IS_SET(act, ACT_SPELL_BLOCKER)) strcat(buf, "BLOCKER ");
	if(IS_SET(act, ACT_FINISH_BLOW)) strcat(buf, "BLOWER ");

	printf("%65s\n", buf);
}

void PrintAffected(int affected)
{
	char buf[256];

	buf[0] = 0;
	if(IS_SET(affected, AFF_BLIND)) strcat(buf, "BLIND ");
	if(IS_SET(affected, AFF_INVISIBLE)) strcat(buf, "INVIS ");
	if(IS_SET(affected, AFF_DETECT_EVIL)) strcat(buf, "D.EVIL ");
	if(IS_SET(affected, AFF_DETECT_INVISIBLE)) strcat(buf, "D.INV ");
	if(IS_SET(affected, AFF_INFRAVISION)) strcat(buf, "INFRA ");
	if(IS_SET(affected, AFF_SENSE_LIFE)) strcat(buf, "SENSE ");
	if(IS_SET(affected, AFF_DEAF)) strcat(buf, "DEAF ");
	if(IS_SET(affected, AFF_SANCTUARY)) strcat(buf, "SANC ");
	/*
	if(IS_SET(affected, AFF_GROUP)) strcat(buf, "GROUP ");
	*/
	if(IS_SET(affected, AFF_DUMB)) strcat(buf, "DUMB ");
	if(IS_SET(affected, AFF_CURSE)) strcat(buf, "CURSE ");
	if(IS_SET(affected, AFF_MIRROR_IMAGE)) strcat(buf, "M.IMAGE ");
	if(IS_SET(affected, AFF_POISON)) strcat(buf, "POISON ");
	if(IS_SET(affected, AFF_PROTECT_EVIL)) strcat(buf, "P.EVIL ");
	if(IS_SET(affected, AFF_REFLECT_DAMAGE)) strcat(buf, "R.DAM ");
	/*
	if(IS_SET(affected, AFF_HOLY_SHIELD)) strcat(buf, "H.SLD ");
	*/
	if(IS_SET(affected, AFF_SPELL_BLOCK)) strcat(buf, "S.BLOCK ");
	if(IS_SET(affected, AFF_SLEEP)) strcat(buf, "SLEEP ");
	if(IS_SET(affected, AFF_SHADOW_FIGURE)) strcat(buf, "SHADOW ");
	if(IS_SET(affected, AFF_SNEAK)) strcat(buf, "SNEAK ");
	if(IS_SET(affected, AFF_HIDE)) strcat(buf, "HIDE ");
	if(IS_SET(affected, AFF_DEATH)) strcat(buf, "DEATH ");
	/* 
	if(IS_SET(affected, AFF_CHARM)) strcat(buf, "CHARM ");
	if(IS_SET(affected, AFF_FOLLOW)) strcat(buf, "FOLLOW ");
	*/
	if(IS_SET(affected, AFF_HASTE)) strcat(buf, "HASTE ");
	if(IS_SET(affected, AFF_IMPROVED_HASTE)) strcat(buf, "I.HASTE ");
	if(IS_SET(affected, AFF_LOVE)) strcat(buf, "LOVE ");
	if(IS_SET(affected, AFF_RERAISE)) strcat(buf, "RERAISE ");
	if(IS_SET(affected, AFF_ARREST)) strcat(buf, "ARREST ");

	printf("%65s\n", buf);
}

void BalanceMobile(FILE *fp)
{
	char *str, *name, tmp[256];
	int vnum;
	int act, affect, alignment;
	char class, sex;
	int level, hit, ac, hr, dr;
	int exp, gold, pos;
	// int sum, over, below;

	while(1){
		fgets(tmp, 256, fp);

		if(tmp[0] == '$'){
			/* end of file */
			return;
		}

		if(tmp[0] == '#'){
			sscanf(&tmp[1], "%d", &vnum);

			/* string data */
			name = fread_string(fp);
			str = fread_string(fp); free(str);
			str = fread_string(fp); free(str);
			str = fread_string(fp); free(str);

			/* numeric data */
			fscanf(fp, "%d %d %d\n", &act, &affect, &alignment);

			fscanf(fp, "%c %d %d %d %d %d\n",
				&class, &level, &hit, &ac, &hr, &dr);

			fscanf(fp, "%d %d %d %c\n",
				&gold, &exp, &pos, &sex);
		}
		else {
			fprintf(stderr, "error : file format\n");
			exit(1);
		}

		while(vnum > zone_data[zone].number){
			zone ++;
			new_zone = 1;
		}

		if(new_zone){
			PRINT_HEADER;
			new_zone = 0;
		}

		printf("%-5d%40s : %2d %2d %2d %2d %2d %2d ",
			vnum, name, level, hit, ac, hr, dr, exp);
		printf(" %c   %c\n", class, sex);

		PrintAct(act);
		PrintAffected(affect);

#ifdef TTT
		sum = hit + ac + hr + dr;
		over = 0;
		below = 0;

		OverBelow(hit);
		OverBelow(ac);
		OverBelow(hr);
		OverBelow(dr);

		if(over > 3){ /* over = 4 */
			if(sum > 60) printf("Strong, ");

			/* EXP */
			if(exp > 15) printf("Balanced.\n");
			else printf("Small EXP.\n");
		}
		else if(over > 2){ /* over = 3 */
			/* EXP */
			if(exp > 13) printf("Balanced.\n");
			else printf("Small EXP.\n");
		}
		else if(over > 1){ /* over = 2 */
			/* EXP */
			if(exp > 11) printf("Great EXP.\n");
			else if(exp > 10) printf("Balanced.\n");
			else printf("Small EXP.\n");
		}
		else if(over > 0){ /* over = 1 */
			/* EXP */
			if(exp > 10) printf("Great EXP.\n");
			else if(exp > 7) printf("Balanced.\n");
			else printf("Small EXP.\n");
		}
		else { /* over = 0 */
			if(sum < 20) printf("Weak, ");
			/* EXP */
			if(exp > 7) printf("Great EXP.\n");
			else if(exp > 4) printf("Balanced.\n");
			else printf("Small EXP.\n");
		}

#endif
		free(name);
	}
}

void ReadIndex(int *index)
{
	int i, n;
	FILE *fp;
	char str[256];

	for(i = 0; i < 20000; i ++) index[i] = 0;

	if((fp = fopen("mob.max1.zon.sort", "r")) == NULL){
		fprintf(stderr, "file open error(mob.max1.zon.sort)\n");
		return;
	}

	while(1){
		fgets(str, 256, fp);

		if(str[0] == '$') break;

		sscanf(str, "%d", &n);

		index[n] = 1;
	}
	
	fclose(fp);
}

void PrintMobileQ(FILE *fp)
{
	char *str, *name, tmp[256];
	int vnum;
	int act, affect, alignment;
	char class, sex;
	int level, hit, ac, hr, dr;
	int exp, gold, pos;
	// int sum, over, below;
	int index[20000];

	// int num;

	// num = 0;

	ReadIndex(index);

	while(1){
		fgets(tmp, 256, fp);

		if(tmp[0] == '$'){
			/* end of file */
			return;
		}

		if(tmp[0] == '#'){
			sscanf(&tmp[1], "%d", &vnum);

			/* string data */
			name = fread_string(fp);
			str = fread_string(fp); free(str);
			str = fread_string(fp); free(str);
			str = fread_string(fp); free(str);

			/* numeric data */
			fscanf(fp, "%d %d %d\n", &act, &affect, &alignment);

			fscanf(fp, "%c %d %d %d %d %d\n",
				&class, &level, &hit, &ac, &hr, &dr);

			fscanf(fp, "%d %d %d %c\n",
				&gold, &exp, &pos, &sex);
		}
		else {
			fprintf(stderr, "error : file format\n");
			exit(1);
		}

		printf("%d %d %s\n", vnum, level, name);
/*
		if(index[vnum]){
			printf("%d %d %s\n", vnum, level, name);
		}
*/

		free(name);
	}
}

int main(int argc, char **argv)
{
	FILE *fp;

	if((fp = fopen("tinyworld.mob", "r")) == NULL){
		exit(1);
	}

	zone = 0;
	new_zone = 1;

	PrintMobileQ(fp);
	/*
	BalanceMobile(fp);
	*/

	fclose(fp);
}
