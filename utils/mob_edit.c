#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <sys/types.h>
#include "mob_create.h"

int  ask_question(void);
long set_affected_by(void);
long set_special_act(void);

int main()
{
        struct char_data mob_data;
        FILE *mobfile;
	int i /*,j */;
	char buff_short[128], buff_long[256], buff_name[64];
	char buff_mob_desc[10][128];
	char *stringcheck;
	// int MAXLENGTH = 128;

	printf("Enter the mob number: ");
        scanf("%d", &mob_data.vnum);
	getc(stdin);
	
	printf("Mob Name-64 characters(ex: Second Sand Sura):\n");
	mob_data.mob_name = fgets(buff_name, 64, stdin);
	
        printf("Short Description(128 characters):\n");
	mob_data.short_descr = fgets(buff_short, 128, stdin);

        printf("Long Description(256 characters):\n");
        mob_data.long_descr = fgets(buff_long, 256, stdin);

	printf("Enter mob description(Maximum 10 lines and finish with '~' character):\n");

	for(i = 0; i < 10; i++)
	{
		mob_data.mob_descr = fgets(buff_mob_desc[i], 128, stdin);
		if(buff_mob_desc[i][0] == '~')
			break;
	}

        printf("Select Special Act from following list:\n");
	mob_data.special_act = 0;
        mob_data.special_act = set_special_act();

        printf("Select affected magic or thing(?) from following list:\n");
	mob_data.affected_by = 0;
        mob_data.affected_by = set_affected_by();

	do
	{
        	printf("Alignment[-1000(devil) <= Alignment <= 1000(Saint)]: ");
		scanf("%d", &mob_data.alignment);
		getc(stdin);
	}while((mob_data.alignment < -1000) || (mob_data.alignment > 1000));

	do
	{
        	printf("Select the class(W/C/T/M): ");
		scanf("%c", &mob_data.class);
		getc(stdin);
		mob_data.class = toupper(mob_data.class); 
	}while((mob_data.class != 'C') && (mob_data.class != 'W')
		&& (mob_data.class != 'T') && (mob_data.class != 'M'));

	do
	{
        	printf("Enter the level(1 <= level <= 43): ");
		scanf("%c", &mob_data.level);
		getc(stdin);
	}while((mob_data.level < 1) || (mob_data.level > 43));

	do
	{
        	printf("Enter the hitpoint rating(0-20): ");
		scanf("%d", &mob_data.hitpoint);
		getc(stdin);
	}while((mob_data.hitpoint < 0) || (mob_data.hitpoint > 21));

	do
	{
        	printf("Enter the AC rating(0-20): ");
		scanf("%d", &mob_data.armor);
		getc(stdin);
	}while((mob_data.armor < 0) || (mob_data.armor > 21));

	do
	{
        	printf("Enter the HR rating(0-20): ");
		scanf("%d", &mob_data.hr);
		getc(stdin);
	}while((mob_data.hr < 0) || (mob_data.hr > 21));

	do
	{
        	printf("Enter the DR rating(0-20): ");
		scanf("%d", &mob_data.dr);
		getc(stdin);
	}while((mob_data.dr < 0) || (mob_data.dr > 21));

	do
	{
        	printf("Enter the gold rating(0-20): ");
		scanf("%lld", &mob_data.gold);
		getc(stdin);
	}while((mob_data.gold < 0) || (mob_data.gold > 21));

	do
	{
        	printf("Enter the experice rating(0-20): ");
		scanf("%lld", &mob_data.xp);
		getc(stdin);
	}while((mob_data.xp < 0) || (mob_data.xp > 21));

	printf("Enter the position:\n");
	printf("1. SLEEPING\n2. RESTING\n3. SITTING\n4. STANDING\n5. SWIMMING\n6. LEVITATED\n7. FLYING\n");
	do
	{
		printf("Choose Number: ");
		scanf("%c", &mob_data.position);
		getc(stdin);
	}while((mob_data.position < 1) || (mob_data.position > 7));

	switch(mob_data.position)
	{
		case 1:
			mob_data.position = 4;
			break;
		case 2:
			mob_data.position = 5;
			break;
		case 3:
			mob_data.position = 6;
			break;
		case 4:
			mob_data.position = 8;
			break;
		case 5:
			mob_data.position = 9;
			break;
		case 6:
			mob_data.position = 10;
			break;
		case 7:
			mob_data.position = 11;
			break;
		default:
			mob_data.position = 8;
			break;
	}

	printf("Enter the sex(M/F): ");
	do
	{
		scanf("%c", &mob_data.sex);
		getc(stdin);
		mob_data.sex = toupper(mob_data.sex); 
	}while((mob_data.sex != 'M') && (mob_data.sex != 'F'));

	mobfile = fopen("mobfile.mob", "w");

        fprintf(mobfile, "#%d\n", mob_data.vnum);
	for(i = 0;;i++)
	{
		stringcheck = mob_data.mob_name + i;
		if(*stringcheck == '\n')
		{
			*stringcheck = '~';
			stringcheck++;
			*stringcheck = '\n';
			stringcheck++;
			*stringcheck = 0;
			break;
		}
	}

	for(i = 0;;i++)
	{
		stringcheck = mob_data.short_descr + i;
		if(*stringcheck == '\n')
		{
			*stringcheck = '~';
			stringcheck++;
			*stringcheck = '\n';
			stringcheck++;
			*stringcheck = 0;
			break;
		}
	}

	fprintf(mobfile, "%s", mob_data.mob_name);
        fprintf(mobfile, "%s", mob_data.short_descr);
	fprintf(mobfile, "%s", mob_data.long_descr);

	fprintf(mobfile, "~\n");

	for(i=0;i < 10; i++)
	{
		if(buff_mob_desc[i][0] == '~')
			break;
		fprintf(mobfile, "%s", buff_mob_desc[i]);
	}


	fprintf(mobfile, "~\n");

	fprintf(mobfile, "%ld %ld %d\n", mob_data.special_act, mob_data.affected_by, mob_data.alignment); 

	fprintf(mobfile, "%c %d %d %d %d %d\n", mob_data.class, mob_data.level, mob_data.hitpoint,
			mob_data.armor, mob_data.hr, mob_data.dr);
	fprintf(mobfile, "%lld %lld %d %c\n", mob_data.gold, mob_data.xp, mob_data.position, mob_data.sex);

        fclose(mobfile);
} 


long set_affected_by()
{
        long affected_by;
        int answer;


        affected_by = 0;

        printf("BLIND?(y/n) ");
        answer = ask_question();
	if(answer)
		affected_by = affected_by | AFF_BLIND;

	printf("INVISIBLE?(y/n) ");
        answer = ask_question();
	if(answer)
		affected_by = affected_by | AFF_INVISIBLE;

	printf("DETECT INVISIBLE? ");
        answer = ask_question();
	if(answer)
		affected_by = affected_by | AFF_DETECT_INVISIBLE;

	printf("INFRAVISION? ");
        answer = ask_question();
	if(answer)
		affected_by = affected_by | AFF_INFRAVISION;

        printf("SANCTUARY? ");
        answer = ask_question();
	if(answer)
		affected_by = affected_by | AFF_SANCTUARY;

        printf("SENSE_LIFE? ");
        answer = ask_question();
	if(answer)
		affected_by = affected_by | AFF_SENSE_LIFE;

	printf("MIRROR IMAGE? ");
        answer = ask_question();
        if(answer)
                affected_by = affected_by | AFF_MIRROR_IMAGE;

        printf("REFLECT DAMAGE? ");
        answer = ask_question();
        if(answer)
                affected_by = affected_by | AFF_REFLECT_DAMAGE;

        /*printf("SPELL BLOCK? ");
        answer = ask_question();
        if(answer)
                affected_by = affected_by | AFF_SPELL_BLOCK;*/

        /*printf("SHADOW FIGURE? ");
        answer = ask_question();
        if(answer)
                affected_by = affected_by | AFF_SHADOW_FIGURE;*/

        printf("SNEAK? ");
        answer = ask_question();
        if(answer)
                affected_by = affected_by | AFF_SNEAK;

        printf("HIDE? ");
        answer = ask_question();
        if(answer)
                affected_by = affected_by | AFF_HIDE;

        /*printf("DEATH?(Not working)");
        answer = ask_question();
        if(answer)
                affected_by = affected_by | AFF_DEATH;*/

        printf("HASTE? ");
        answer = ask_question();
        if(answer)
                affected_by = affected_by | AFF_HASTE;

        printf("IMPROVED HASTE? ");
        answer = ask_question();
        if(answer)
                affected_by = affected_by | AFF_IMPROVED_HASTE;

        printf("LOVE? ");
        answer = ask_question();
        if(answer)
                affected_by = affected_by | AFF_LOVE;

        /*printf("RERAISE? ");
        answer = ask_question();
        if(answer)
                affected_by = affected_by | AFF_RERAISE;*/

	return affected_by;

}

long set_special_act()
{
        long special_act;
        int answer;


        special_act = 0;

        printf("Does this mob have any special act?(y/n) ");
        answer = ask_question();
	if(answer)
		special_act = special_act | ACT_SPEC;

	printf("Stationary Mob?(y/n) ");
        answer = ask_question();
	if(answer)
		special_act = special_act | ACT_SENTINEL;

	printf("Scavenger(pick up staff lying around)?? (y/n) ");
        answer = ask_question();
	if(answer)
		special_act = special_act | ACT_SCAVENGER;

	special_act = special_act | ACT_ISNPC;	/* This bit is set for use with IS_NPC() */

	printf("Nice Thief(Set if a thief should NOT be killed? ");
        answer = ask_question();
	if(answer)
		special_act = special_act | ACT_NICE_THIEF;

        printf("Aggressive? ");
        answer = ask_question();
	if(answer)
		special_act = special_act | ACT_AGGRESSIVE;

        printf("Stay inside own zone(for moving mob)? ");
        answer = ask_question();
	if(answer)
		special_act = special_act | ACT_STAY_ZONE;

	printf("Wimpy? ");
        answer = ask_question();
        if(answer)
                special_act = special_act | ACT_WIMPY;

        printf("FIGHTER's special act? ");
        answer = ask_question();
        if(answer)
                special_act = special_act | ACT_FIGHTER;

        printf("MAGE's special act? ");
        answer = ask_question();
        if(answer)
                special_act = special_act | ACT_MAGE;

        printf("CLERIC's special act? ");
        answer = ask_question();
        if(answer)
                special_act = special_act | ACT_CLERIC;

        printf("THIEF's special act? ");
        answer = ask_question();
        if(answer)
                special_act = special_act | ACT_THIEF;

        printf("PALADIN's special act? ");
        answer = ask_question();
        if(answer)
                special_act = special_act | ACT_PALADIN;

        printf("DRAGON's special act? ");
        answer = ask_question();
        if(answer)
                special_act = special_act | ACT_DRAGON;

        printf("SPITTER's special act? ");
        answer = ask_question();
        if(answer)
                special_act = special_act | ACT_SPITTER;

        printf("SHOOTER's special act? ");
        answer = ask_question();
        if(answer)
                special_act = special_act | ACT_SHOOTER;

        printf("GUARD's special act? ");
        answer = ask_question();
        if(answer)
                special_act = special_act | ACT_GUARD;

	printf("SUPER GUARD's special act? ");
        answer = ask_question();
        if(answer)
                special_act = special_act | ACT_SUPERGUARD;

	printf("GIANT's special act? ");
        answer = ask_question();
        if(answer)
                special_act = special_act | ACT_GIANT;

	printf("HELPER's special act? ");
        answer = ask_question();
        if(answer)
                special_act = special_act | ACT_HELPER;

	printf("ORC's special act? ");
	answer = ask_question();
	if(answer)
		special_act = special_act | ACT_ORC;

	printf("RESCUER's special_act? ");
        answer = ask_question();
        if(answer)
                special_act = special_act | ACT_RESCUER;
	
	printf("SPELL BLOCKER? ");
        answer = ask_question();
        if(answer)
                special_act = special_act | ACT_SPELL_BLOCKER;

        printf("FINISH BLOW? ");
        answer = ask_question();
        if(answer)
                special_act = special_act | ACT_FINISH_BLOW;

	return special_act;

}

int ask_question()
{
        char answer = 'N';
	int ans;

	for(;;)
        {
                scanf("%c", &answer);
		getc(stdin);
		if(answer == 'Y'|| answer == 'N' || answer == 'y' || answer == 'n')
			break;
		else
			printf("Answer Y or N!!!\n");
	}


	if(answer == 'y' || answer == 'Y')
		ans = 1;
	else ans = 0;

        return ans;
}

