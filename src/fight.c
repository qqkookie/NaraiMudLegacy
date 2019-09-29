/* ************************************************************************
*  File: fight.c , Combat module.                         Part of DIKUMUD *
*  Usage: Combat system and messages.                                     *
*  Copyright (C) 1990, 1991 - see 'license.doc' for complete information. *
************************************************************************* */

#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <time.h>

#include "structs.h"
#include "utils.h"
#include "comm.h"
#include "handler.h"
#include "interpreter.h"
#include "db.h"
#include "spells.h"
#include "memory.h"
#include "quest.h"

/* Structures */

struct char_data *combat_list = 0;     /* head of l-list of fighting chars */
struct char_data *combat_next_dude = 0; /* Next dude global trick           */


/* External structures */

extern struct room_data *world;
extern struct message_list fight_messages[MAX_MESSAGES];
extern struct obj_data  *object_list;

/* External procedures */

char *fread_string(FILE *f1);
void stop_follower(struct char_data *ch);
void do_flee(struct char_data *ch, char *argument, int cmd);
void hit(struct char_data *ch, struct char_data *victim, int type);
void wipe_stash(char *filename);

/* Weapon attack texts */
struct attack_hit_type attack_hit_text[] =
{
    {"hit",   "hits"},             /* TYPE_HIT      */
    {"pound", "pounds"},           /* TYPE_BLUDGEON */
    {"pierce", "pierces"},         /* TYPE_PIERCE   */
    {"slash", "slashes"},          /* TYPE_SLASH    */
    {"whip", "whips"},             /* TYPE_WHIP     */
    {"shoot", "shoots"},           /* TYPE_SHOOT    */
    {"bite", "bites"},             /* TYPE_BITE     */
    {"sting", "stings"},           /* TYPE_STING    */
    {"crush", "crushes"}           /* TYPE_CRUSH    */
};

struct attack_hit_type attack_hit_han[] =
{
    {"칩니다",   "칩니다"},		/* TYPE_HIT      */
    {"난타 합니다", "난타 합니다"},	/* TYPE_BLUDGEON */
    {"찌릅니다", "찌릅니다"},		/* TYPE_PIERCE   */
    {"벱니다", "벱니다"},			/* TYPE_SLASH    */
    {"채찍질 합니다", "채찍질 합니다"},	/* TYPE_WHIP     */
    {"쏩니다", "쏩니다"},			/* TYPE_SHOOT    */
    {"물어 뜯습니다", "물어 뜯습니다"},	/* TYPE_BITE     */
    {"톡 쏩니다", "톡 쏩니다"},		/* TYPE_STING    */
    {"부숩니다", "부숩니다"}		/* TYPE_CRUSH    */
};

struct attack_hit_type attack_hit_han_brief[] =
{
    {"침",   "침"},             /* TYPE_HIT      */
    {"난타", "난타"},           /* TYPE_BLUDGEON */
    {"찌름", "찌름"},         /* TYPE_PIERCE   */
    {"벰", "벰"},          /* TYPE_SLASH    */
    {"채찍질함", "채찍질함"},             /* TYPE_WHIP     */
    {"쏨", "쏨"},           /* TYPE_SHOOT    */
    {"물어 뜯음", "물어 뜯음"},             /* TYPE_BITE     */
    {"쏨", "쏨"},           /* TYPE_STING    */
    {"부숨", "부숨"}           /* TYPE_CRUSH    */
};

struct dam_weapon_type {
    char *to_room;
    char *to_char;
    char *to_victim;
} ;

static struct dam_weapon_type dam_weapons[] = {

    {"$n misses $N with $s #W.",                           /*    0    */
     "You miss $N with your #W.",
     "$n miss you with $s #W." },

    {"$n tickles $N with $s #W.",                          /*  1.. 4  */
     "You tickle $N as you #W $M.",
     "$n tickle you as $e #W you." },

    {"$n barely #W $N.",                                   /*  5.. 8  */
     "You barely #W $N.",
     "$n barely #W you."},

    {"$n #W $N.",                                          /*  9.. 12  */
     "You #W $N.",
     "$n #W you."}, 

    {"$n #W $N hard.",                                     /*  13..20  */
     "You #W $N hard.",
     "$n #W you hard."},

    {"$n #W $N very hard.",                                /* 21..30  */
     "You #W $N very hard.",
     "$n #W you very hard."},

    {"$n #W $N extremely hard.",                          /* 31..40  */
     "You #W $N extremely hard.",
     "$n #W you extremely hard."},

    {"$n massacres $N to small fragments with $s #W.",     /* > 40    */
     "You massacre $N to small fragments with your #W.",
     "$n massacres you to small fragments with $s #W."},

    {"$n annihilates $N to micro molecules with $s #W.",   /* > 65    */
     "You annihilate $N to micro molecules with your #W.",
     "$n annihilates you to micro molecules with $s #W."},

    { "$n disintegrates $N with $s #W.",                    /* > 110 */
      "You disintegrate $N with your #W.",
      "$n disintegrates you with $s #W." },

    { "$n sends $N to VACUUM with $s #W.",                    /* > 210 */
      "You send $N to VACUUM with your #W.",
      "$n sends you to VACUUM with $s #W." }
};

static struct dam_weapon_type dam_weapons_brief[] = {

    {"$n misses $N.",                           /*    0    */
     "You miss $N.",
     "$n miss you." },

    {"$n tickles $N.",                          /*  1.. 4  */
     "You tickle $N.",
     "$n tickle you." },

    {"$n barely #W $N.",                                   /*  5.. 8  */
     "You barely #W $N.",
     "$n barely #W you."},

    {"$n #W $N.",                                          /*  9.. 12  */
     "You #W $N.",
     "$n #W you."}, 

    {"$n #W $N hard.",                                     /*  13..20  */
     "You #W $N hard.",
     "$n #W you hard."},

    {"$n #W $N v-hard.",                                /* 21..30  */
     "You #W $N v-hard.",
     "$n #W you v-hard."},

    {"$n #W $N ex-hard.",                          /* 31..40  */
     "You #W $N ex-hard.",
     "$n #W you ex-hard."},

    {"$n massacres $N.",     /* > 40    */
     "You massacre $N.",
     "$n massacres you."},

    {"$n annihilates $N.",   /* > 65    */
     "You annihilate $N.",
     "$n annihilates you."},

    { "$n disintegrates $N.",                    /* > 110 */
      "You disintegrate $N.",
      "$n disintegrates you." },

    { "$n sends $N to VACUUM with $s #W.",                    /* > 210 */
      "You send $N to VACUUM with your #W.",
      "$n sends you to VACUUM with $s #W." }

};

static struct dam_weapon_type han_weapons[] = {
    {"$n님이 $N님을 때리려다 빗나갔습니다.",        /*    0    */
     "당신은 $N님을 때리지 못했습니다.",
     "$n님이 당신을 때리려다 실패했습니다." },

    {"$n님이 $N님을 간지럽게 #W.",                          /*  1.. 4  */
     "당신은 $N님을 간지럽게 #W.",
     "$n님이 당신을 간지럽게 #W." },

    {"$n님이 $N님을 살살 #W.",                                   /*  5.. 8  */
     "당신은 $N님을 살살 #W.",
     "$n님이 당신을 살살 #W."},

    {"$n님이 $N님을 #W.",                                          /*  9.. 12  */
     "당신은 $N님을 #W.",
     "$n님이 당신을 #W."}, 

    {"$n님이 $N님을 세게 #W.",                                     /*  13..20  */
     "당신은 $N님을 세게 #W.",
     "$n님이 당신을 세게 #W."},

    {"$n님이 $N님을 매우 세게 #W.",                                /* 21..30  */
     "당신은 $N님을 매우 세게 #W.",
     "$n님이 당신을 매우 세게 #W."},

    {"$n님이 $N님을 무지막지하게 #W.",                          /* 31..40  */
     "당신은 $N님을 무지막지하게 #W.",
     "$n님이 당신을 무지막지하게 #W."},

    {"$n님이 $N님을 뼈가 으스러지게 #W.",     /* > 40    */
     "당신은 $N님을 뼈가 으스러지게 #W.",
     "$n님이 당신을 뼈가 으스러지게 #W."},

    {"$n님이 $N님을 몸이 산산조각이 나게 #W.",     /* > 65    */
     "당신은 $N님을 몸이 산산조각이 나게 #W.",
     "$n님이 당신을 몸이 산산조각이 나게 #W."},

    {"$n님이 $N님을 몸이 가루가 되게 #W.",     /* > 110    */
     "당신은 $N님을 몸이 가루가 되게 #W.",
     "$n님이 당신을 몸이 가루가 되게 #W."},

    {"$n님이 $N님을 형체가 없도록 #W.",     /* > 210    */
     "당신은 $N님을 형체가 없도록 #W.",
     "$n님이 당신을 형체가 없도록 #W."}
};

static struct dam_weapon_type han_weapons_brief[] = {
    {"$n님이 $N님을 못때림.",        /*    0    */
     "당신은 $N님을 못때림.",
     "$n님이 당신을 못때림." },

    {"$n님이 $N님을 간지럽게 #W.",                          /*  1.. 4  */
     "당신은 $N님을 간지럽게 #W.",
     "$n님이 당신을 간지럽게 #W." },

    {"$n님이 $N님을 살살 #W.",                                   /*  5.. 8  */
     "당신은 $N님을 살살 #W.",
     "$n님이 당신을 살살 #W."},

    {"$n님이 $N님을 #W.",                                          /*  9.. 12  */
     "당신은 $N님을 #W.",
     "$n님이 당신을 #W."}, 

    {"$n님이 $N님을 세게 #W.",                                     /*  13..20  */
     "당신은 $N님을 세게 #W.",
     "$n님이 당신을 세게 #W."},

    {"$n님이 $N님을 거세게 #W.",                                /* 21..30  */
     "당신은 $N님을 거세게 #W.",
     "$n님이 당신을 거세게 #W."},

    {"$n님이 $N님을 무식하게 #W.",                          /* 31..40  */
     "당신은 $N님을 무식하게 #W.",
     "$n님이 당신을 무식하게 #W."},

    {"$n님이 $N님을 으스러지게 #W.",     /* > 40    */
     "당신은 $N님을 으스러지게 #W.",
     "$n님이 당신을 으스러지게 #W."},

    {"$n님이 $N님을 산산조각나게 #W.",     /* > 65    */
     "당신은 $N님을 산산조각나게 #W.",
     "$n님이 당신을 산산조각나게 #W."},

    {"$n님이 $N님을 가루되게 #W.",     /* > 110    */
     "당신은 $N님을 가루되게 #W.",
     "$n님이 당신을 가루되게 #W."},

    {"$n님이 $N님을 형체없도록 #W.",     /* > 210    */
     "당신은 $N님을 형체없도록 #W.",
     "$n님이 당신을 형체없도록 #W."}

};
/* The Fight related routines */
void appear(struct char_data *ch)
{
    act("$n slowly fade into existence.", FALSE, ch,0,0,TO_ROOM);
    if (affected_by_spell(ch, SPELL_INVISIBLE))
        affect_from_char(ch, SPELL_INVISIBLE);
    REMOVE_BIT(ch->specials.affected_by, AFF_INVISIBLE);
}

void load_messages(void)
{
    FILE *f1;
    int i,type;
    struct message_type *messages;
    char chk[100];

    if (!(f1 = fopen(MESS_FILE, "r"))){
        perror("read messages");
        exit(1);
    }

    for (i = 0; i < MAX_MESSAGES; i++)
    { 
        fight_messages[i].a_type = 0;
        fight_messages[i].number_of_attacks=0;
        fight_messages[i].msg = 0;
    }

    fscanf(f1, " %s \n", chk);

    while(*chk == 'M')
    {
        fscanf(f1," %d\n", &type);
        for (i = 0; (i < MAX_MESSAGES) && (fight_messages[i].a_type!=type) &&
                 (fight_messages[i].a_type); i++);
        if(i>=MAX_MESSAGES){
            log("Too many combat messages.");
            exit(1);
        }

        /*
          create(messages,struct message_type,1);
        */
        messages = (struct message_type *) malloc_general_type(MEMORY_MESSAGE_TYPE) ;
        bzero(messages, sizeof(struct message_type)) ;

        fight_messages[i].number_of_attacks++;
        fight_messages[i].a_type=type;
        messages->next=fight_messages[i].msg;
        fight_messages[i].msg=messages;
        messages->die_msg.attacker_msg      = fread_string(f1);
        messages->die_msg.victim_msg        = fread_string(f1);
        messages->die_msg.room_msg          = fread_string(f1);
        messages->miss_msg.attacker_msg     = fread_string(f1);
        messages->miss_msg.victim_msg       = fread_string(f1);
        messages->miss_msg.room_msg         = fread_string(f1);
        messages->hit_msg.attacker_msg      = fread_string(f1);
        messages->hit_msg.victim_msg        = fread_string(f1);
        messages->hit_msg.room_msg          = fread_string(f1);
        messages->god_msg.attacker_msg      = fread_string(f1);
        messages->god_msg.victim_msg        = fread_string(f1);
        messages->god_msg.room_msg          = fread_string(f1);
        fscanf(f1, " %s \n", chk);
    }
    fclose(f1);
}

void update_pos( struct char_data *victim )
{

    if ((GET_HIT(victim) > 0) && (GET_POS(victim) > POSITION_STUNNED)) return;
    else if (GET_HIT(victim) > 0 ) GET_POS(victim) = POSITION_STANDING;
    else if (GET_HIT(victim) <= -11) GET_POS(victim) = POSITION_DEAD;
    else if (GET_HIT(victim) <= -6) GET_POS(victim) = POSITION_MORTALLYW;
    else if (GET_HIT(victim) <= -3) GET_POS(victim) = POSITION_INCAP;
    else GET_POS(victim) = POSITION_STUNNED;

}


/* start one char fighting another (yes, it is horrible, I know... )  */
void set_fighting(struct char_data *ch, struct char_data *vict)
{
    assert(!ch->specials.fighting);

    ch->next_fighting = combat_list;
    combat_list = ch;

    if(IS_AFFECTED(ch,AFF_SLEEP))
        affect_from_char(ch,SPELL_SLEEP);

    ch->specials.fighting = vict;
    GET_POS(ch) = POSITION_FIGHTING;
}



/* remove a char from the list of fighting chars */
void stop_fighting(struct char_data *ch)
{
    struct char_data *tmp;

	if (ch->specials.fighting == NULL)
		return ;

  	if (IS_SET(ch->specials.affected_by,AFF_BERSERK))    {
      	REMOVE_BIT(ch->specials.affected_by,AFF_BERSERK);
		act("$n seems calm down!", FALSE, ch,0,0,TO_ROOM);
		act("You calm down.", FALSE,ch, 0, 0, TO_CHAR);
	}

    if (ch == combat_next_dude)
        combat_next_dude = ch->next_fighting;

    if (combat_list == ch)
        combat_list = ch->next_fighting;
    else
    {
        for (tmp = combat_list; tmp && (tmp->next_fighting != ch); 
             tmp = tmp->next_fighting);
        if (!tmp) {
            log("Char fighting not found Error (fight.c, stop_fighting)");
            abort();
        }
        tmp->next_fighting = ch->next_fighting;
    }

    ch->next_fighting = 0;
    ch->specials.fighting = 0;
    GET_POS(ch) = POSITION_STANDING;
    update_pos(ch);
}



#define MAX_NPC_CORPSE_TIME 5
#define MAX_PC_CORPSE_TIME 40

void make_corpse(struct char_data *ch, int level, int isgroup)
{
	/*  level is killer's level  */
	struct obj_data *make_particle(struct obj_data *obj) ;
    struct obj_data *corpse, *o;
    struct obj_data *money, *otmp; 
    struct obj_data *next_obj, *tmp_obj; 
    char buf[MAX_STRING_LENGTH];
    int i, tmp;

    char *string_dup(char *source);
    struct obj_data *create_money( int amount );

	/*
 create(corpse, struct obj_data, 1);
	*/
	corpse = (struct obj_data *) malloc_general_type(MEMORY_OBJ_DATA) ;
    clear_object(corpse);
 
    corpse->item_number = NOWHERE;
    corpse->in_room = NOWHERE;
    sprintf(buf,"corpse %s", 
            (IS_NPC(ch) ? ch->player.short_descr : GET_NAME(ch)));
    corpse->name = string_dup(buf);

    sprintf(buf, "Corpse of %s is lying here.", 
            (IS_NPC(ch) ? ch->player.short_descr : GET_NAME(ch)));
    corpse->description = string_dup(buf);

    sprintf(buf, "Corpse of %s",
            (IS_NPC(ch) ? ch->player.short_descr : GET_NAME(ch)));
    corpse->short_description = string_dup(buf);

    corpse->contains = ch->carrying;

#ifdef DEATHFIGHT
/* don't put player gold while deathfight */
	if (!IS_NPC(ch))
		GET_GOLD(ch) = 0 ;
#endif 

    if(!isgroup && (GET_GOLD(ch)>0)&&(IS_NPC(ch)||ch->desc)) {
        /* if group mode, gold is shared already */
        money = create_money(GET_GOLD(ch));
        GET_GOLD(ch)=0;
        if (money)
            obj_to_obj(money,corpse);
    }

    corpse->obj_flags.type_flag = ITEM_CONTAINER;
    corpse->obj_flags.wear_flags = ITEM_TAKE;
    corpse->obj_flags.value[0] = 0; /* You can't store stuff in a corpse */
    corpse->obj_flags.value[3] = 1; /* corpse identifyer */
    corpse->obj_flags.weight = GET_WEIGHT(ch)+IS_CARRYING_W(ch);
    if (IS_NPC(ch))
        corpse->obj_flags.timer = MAX_NPC_CORPSE_TIME;
    else
        corpse->obj_flags.timer = MAX_PC_CORPSE_TIME;

    for (i=0; i<MAX_WEAR; i++) {
        if (ch->equipment[i]) {
            otmp = unequip_char(ch,i) ;
            if ( IS_SET(world[ch->in_room].room_flags, DEATH_PLACE) ) {
                /* not broken */
                obj_to_obj(otmp, corpse);
            }
            else if ((!IS_NPC(ch)) && GET_LEVEL(ch) > 10) {	/* check .. can be broken */
                tmp = number(10, 125) ;
                if (IS_OBJ_STAT(otmp, ITEM_GLOW)) tmp += 9 ;  /* harder to break */
                if (IS_OBJ_STAT(otmp, ITEM_HUM)) tmp -= 10 ;    /* easier to break */
                if (IS_OBJ_STAT(otmp, ITEM_BLESS)) tmp += 3 ;    /* harder to break */
                if ( level > tmp ) {/* item is broken !!! cyb  */
                    act("The $o is broken by bitter death !!!.", TRUE, ch, otmp, 0, TO_ROOM);
                    sprintf(buf, "Lost item(killed): %s", otmp->short_description);
                    log(buf) ;
                    if ( otmp->obj_flags.type_flag == ITEM_CONTAINER ) {
                        act("As $o is smashed, some items dumped.", 0,ch,otmp,0,TO_ROOM) ;
                        for ( tmp_obj = otmp->contains ; tmp_obj ; tmp_obj = next_obj ) {
                            next_obj = tmp_obj->next_content ;
                            obj_from_obj(tmp_obj) ;
                            obj_to_room(tmp_obj, ch->in_room) ;
                        }
                    }
                    /* extract_obj( otmp ) ;	/* free object */
                    tmp_obj = make_particle(otmp) ;
                    obj_to_obj(tmp_obj, corpse);
                }
                else /* not broken */
                    obj_to_obj(otmp, corpse);
            }
            else { /* store to corpse */
                obj_to_obj(otmp, corpse);
            }
        }
    }

    ch->carrying = 0;
    IS_CARRYING_N(ch) = 0;
    IS_CARRYING_W(ch) = 0;

    corpse->next = object_list;
    object_list = corpse;

    for(o = corpse->contains; o; o->in_obj = corpse, o = o->next_content);
    object_list_new_owner(corpse, 0);

    obj_to_room(corpse, ch->in_room);
}


/* When ch kills victim */
void change_alignment(struct char_data *ch, struct char_data *victim)
{
    int al, direc;
 
	/* al = (7/8 ch - victim /8 - ch )/12 */
	/*    = - ( ch / 8 + victim /8 ) /12 */
	/*  8 * 12 = 96 */
	/*
      al=(7*GET_ALIGNMENT(ch)-GET_ALIGNMENT(victim))/8;
      al=(al-GET_ALIGNMENT(ch))/12;
	*/
#ifdef DEATHFIGHT
	al = - ( GET_ALIGNMENT(ch) + GET_ALIGNMENT(victim) ) / 80 ;
#else
	al = - ( GET_ALIGNMENT(ch) + GET_ALIGNMENT(victim) ) / 135 ;
#endif 
	GET_ALIGNMENT(ch)+=al;
	if(GET_ALIGNMENT(ch) < -1000) GET_ALIGNMENT(ch) = -1000;
	if(GET_ALIGNMENT(ch) >  1000) GET_ALIGNMENT(ch) =  1000;
}

void death_cry(struct char_data *ch)
{
    struct char_data *victim;
    int door, was_in;

    acthan("Your blood freezes as you hear $ns death cry.",
           "$n님의 처절한 비명소리가 들립니다. 으 소름끼쳐 ~~",  FALSE, ch,0,0,TO_ROOM);
    was_in = ch->in_room;

    for (door = 0; door <= 5; door++) {
        if (CAN_GO(ch, door)) {
            ch->in_room = world[was_in].dir_option[door]->to_room;
            if ( was_in != ch->in_room )
                acthan("Your blood freezes as you hear someones death cry.",
                       "어디선가 소름끼치는 비명소리가 들려옵니다.", FALSE,ch,0,0,TO_ROOM);
            ch->in_room = was_in;
        }
    }
}

void raw_kill(struct char_data *ch, int level, int isgroup)
{
    if (ch->specials.fighting)
        stop_fighting(ch);
    death_cry(ch);

	if (IS_SET(ch->specials.affected_by,AFF_BERSERK))  {
		REMOVE_BIT(ch->specials.affected_by,AFF_BERSERK);
	}

    make_corpse(ch, level, isgroup);	/* level is killer's level */
    extract_char(ch);	/* extract items from player to room */
}

void die(struct char_data *ch, struct char_data *killed_by)
{
	void show_tomb_stone(struct char_data *ch, struct char_data *vic) ;
	int	stash_charrent(struct char_data *ch) ;

    if(!IS_NPC(ch)){
        show_tomb_stone(ch, killed_by) ;
        wipe_stash(GET_NAME(ch));	/* remove rent datas */
        /*	re-save rented data - they are safe */
        if ( stash_charrent(ch) < 0 ) {
            log("die: fail to save rent file") ;
		}
        wipe_obj(ch->rent_items) ;
	
        /* In death_place, no exp loss */
        if ( !IS_SET(world[ch->in_room].room_flags, DEATH_PLACE) ) {
            if(GET_LEVEL(ch)>=20 && GET_EXP(ch)<2000000){
                ch->points.max_hit -= (ch->points.max_hit/50);
                ch->points.max_move -= (ch->points.max_move/50);
                ch->points.max_mana -= (ch->points.max_mana/50);
			}
            if(GET_LEVEL(ch) == (IMO-1))
                gain_exp(ch, -(GET_EXP(ch)/3));
            else if (GET_LEVEL(ch) < 10)
				gain_exp(ch, -(GET_EXP(ch)/4));
			else if (GET_LEVEL(ch) < 20)
				gain_exp(ch, -(GET_EXP(ch)/6));
			else if (GET_LEVEL(ch) < 30)
                gain_exp(ch, -(GET_EXP(ch)/7));
			else 
                gain_exp(ch, -(GET_EXP(ch)/4));
		}
    }

	if ( killed_by )
		raw_kill(ch, GET_LEVEL(killed_by), IS_AFFECTED(killed_by, AFF_GROUP)) ;
	else
		raw_kill(ch, 10, IS_AFFECTED(killed_by, AFF_GROUP)) ;
}

void show_tomb_stone(struct char_data *ch, struct char_data *killed_by)
{
	char	buf[BUFSIZ], buf2[BUFSIZ], tmstr[30] ;
	long	ct ;

	if ( ch==NULL) return ;

	sprintf(buf, "\n\r\n\rName: %s (level %d)\n\r\n\r", GET_NAME(ch),
            GET_LEVEL(ch)) ;
	ct = ch->player.time.birth ;
	strcpy(tmstr, asctime(localtime(&ct))) ;
	tmstr[ strlen(tmstr) -1 ] = 0 ;
	sprintf(buf2, "Birth : %s\n\r", tmstr) ;
	strcat(buf, buf2) ;
	ct = time(0) ;
	strcpy(tmstr, asctime(localtime(&ct))) ;
	tmstr[ strlen(tmstr) -1 ] = 0 ;
	sprintf(buf2, "Death : %s\n\r", tmstr) ;
	strcat(buf, buf2) ;

	if ( killed_by == NULL )
		sprintf(buf2, "Killed by SOMEONE.\n\r") ;
	else
		sprintf(buf2, "Killed by %s ", (IS_NPC(killed_by) ?
                                        killed_by->player.short_descr : GET_NAME(killed_by)) ) ;
	strcat(buf, buf2) ;

	sprintf(buf2, "at %s\n\r", world[ch->in_room].name);
	strcat(buf, buf2) ;
	strcat(buf, "\n\n\r== Press Return ==") ;
/*
  (IS_NPC(ch) ? ch->player.short_descr : GET_NAME(ch)),
  world[victim->in_room].name);
*/

	send_to_char(buf, ch) ;
}

void group_gain(struct char_data *ch, struct char_data *victim)
{
	void share_group_exp(struct char_data *ch,
                         int tot_exp, int tot_gold, int levsum) ;
	char	buf[256], buf2[BUFSIZ];
	int	share, levsum, one_player, have_master, total_exp, total_gold ;
	struct char_data	*master;
	struct follow_type	*f;
    int count_player = 0;
    int group_exp_bonus = 0;

	if ( ch->master )
		have_master = 1 ;
	else
		have_master = 0 ;

	/* count members */
	if ( have_master ) {
		master = ch->master ;
		if ( !IS_AFFECTED(master, AFF_GROUP) ) {	/* only me */
			levsum = GET_LEVEL(ch) ;
            if (!IS_NPC(ch)) count_player++;
        }
		else {	/* count all members : at least Master and I is group */
			levsum = GET_LEVEL(ch) ; /* it's myself */
            if (!IS_NPC(ch)) count_player++;
            
			if ( master->in_room == ch->in_room ) {
				levsum += GET_LEVEL(master) ;
                if (!IS_NPC(master)) count_player++;
            }
			for ( f = master->followers ; f ; f = f->next ) {
				if( f->follower != ch && IS_AFFECTED(f->follower, AFF_GROUP) &&
					(f->follower->in_room == ch->in_room)) {
					/* is not me, grouped and here.. */
					levsum += GET_LEVEL(f->follower) ;
                    if (!IS_NPC(f->follower)) count_player++;
                }
            }
        }
    }
	else {	/* I'm master or only one player (me) */
		levsum = GET_LEVEL(ch) ;
        if (!IS_NPC(ch)) count_player++;
        
		for ( f = ch->followers ; f ; f = f->next ) {
			if( IS_AFFECTED(f->follower, AFF_GROUP) &&
				(f->follower->in_room == ch->in_room)) {
				/* is grouped and here.. */
				levsum += GET_LEVEL(f->follower) ;
                if (!IS_NPC(f->follower)) count_player++;
            }
        }
    }

	if ( levsum < 1 )
		levsum = 1 ;

	/* share experience and gold */
	total_exp = GET_EXP(victim) ;
    total_gold = GET_GOLD(victim);

    // group play bonus
    if (count_player > 2) {
        group_exp_bonus = (total_exp * (count_player - 2)) * 2 / 10;
        group_exp_bonus = MIN(total_exp, group_exp_bonus);
    }
    total_exp += group_exp_bonus;
    
	if ( have_master ) {
		master = ch->master ;
		/* first me.. */
		share_group_exp(ch, total_exp, total_gold, levsum) ;
		change_alignment(ch, victim);

		if ( IS_AFFECTED(master, AFF_GROUP) ) {
			/* at least Master and I is group */
			if ( master->in_room == ch->in_room ) {
				int bonus_exp = 0;
				if (count_player > 3)  
					bonus_exp = (count_player-2)*total_exp*2/100;
				else 
					bonus_exp = 0;

				share_group_exp(master,total_exp+bonus_exp,total_gold,levsum) ;
				change_alignment(master, victim);
            }
			for ( f = master->followers ; f ; f = f->next ) {
				if( f->follower != ch && IS_AFFECTED(f->follower, AFF_GROUP) &&
					(f->follower->in_room == ch->in_room)) {
					/* is not me, grouped and here.. */
					share_group_exp(f->follower, total_exp,
                                    total_gold, levsum) ;
					change_alignment(f->follower, victim);
                }
            }
        }
    }
	else {	/* I'm master or only one player (me) */
		int bonus_exp = 0;
		if (count_player > 3)  
			bonus_exp = (count_player-2)*total_exp/100;
		else 
			bonus_exp = 0;

		share_group_exp(ch, total_exp+bonus_exp, total_gold, levsum) ;
		change_alignment(ch, victim);
		for ( f = ch->followers ; f ; f = f->next ) {
			if( IS_AFFECTED(f->follower, AFF_GROUP) &&
				(f->follower->in_room == ch->in_room)) {
				/* is grouped and here.. */
				share_group_exp(f->follower, total_exp, total_gold, levsum) ;
				change_alignment(f->follower, victim);
            }
        }
    }
}

void share_group_exp(struct char_data *ch,
                     int total_exp, int total_gold, int levsum)
{
	char	buf[BUFSIZ], buf2[BUFSIZ] ;
	int	share_exp, share_gold ;

	share_exp = (total_exp * GET_LEVEL(ch)) / levsum ;
    share_gold = (total_gold * GET_LEVEL(ch)) / levsum ;
	sprintf(buf,"You receive %d experience and %d gold.\n\r",
            share_exp, share_gold);
	sprintf(buf2,"당신은 %d 점의 경험치와 %d 원의 돈을 얻었습니다.\n\r",
            share_exp, share_gold);
	send_to_char_han(buf, buf2, ch);
	gain_exp(ch, share_exp);
    gain_gold(ch, share_gold);
}

char *replace_string(char *str, char *weapon)
{
    static char buf[5][256];
    static int count = 0;
    char *rtn;
    char *cp;

    cp = rtn = buf[count % 5];
    count ++ ;

    for (; *str; str++) {
        if (*str == '#') {
            switch(*(++str)) {
            case 'W' : 
                for (; *weapon; *(cp++) = *(weapon++));
                break;
            default :
                *(cp++) = '#';
                break;
            }
        } else {
            *(cp++) = *str;
        }
        *cp = 0;
    } /* For */
    return(rtn);
}


void dam_message(int dam, struct char_data *ch, struct char_data *victim,
                 int w_type)
{
    struct obj_data *wield;
    char *buf, *buf2, *bufeb, *bufhb;
    int  msg_index ;

    w_type -= TYPE_HIT;   /* Change to base of table with text */

    wield = ch->equipment[WIELD];

    if (dam == 0)		 msg_index = 0 ;
    else if ( dam <= 4 )	msg_index = 1 ;
    else if ( dam <= 8 )	msg_index = 2 ;
    else if ( dam <= 12 )	msg_index = 3 ;
    else if ( dam <= 20 )	msg_index = 4 ;
    else if ( dam <= 30 )	msg_index = 5 ;
    else if ( dam <= 40 )	msg_index = 6 ;
    else if ( dam <= 65 )	msg_index = 7 ;
    else if ( dam <= 110)	msg_index = 8 ;
    else if ( dam <= 210)	msg_index = 9 ;
    else			msg_index = 10 ;

	/* just a kidding */
	// if (msg_index) msg_index = MIN(9, msg_index+1);

    if ( msg_index == 3 || msg_index == 4 || msg_index == 5 || msg_index == 6 ) {
        buf = replace_string ( dam_weapons[msg_index].to_room,
                               attack_hit_text[w_type].plural);
        bufeb = replace_string ( dam_weapons_brief[msg_index].to_room,
                                 attack_hit_text[w_type].plural);
    }
    else {
        buf = replace_string ( dam_weapons[msg_index].to_room,
                               attack_hit_text[w_type].singular);
        bufeb = replace_string ( dam_weapons_brief[msg_index].to_room,
                                 attack_hit_text[w_type].singular);
    }
    buf2 = replace_string ( han_weapons[msg_index].to_room,
                            attack_hit_han[w_type].singular);
    bufhb = replace_string ( han_weapons_brief[msg_index].to_room,
                             attack_hit_han_brief[w_type].singular);
    acthanbrief(buf,buf2,bufeb,bufhb,FALSE, ch, wield, victim, TO_NOTVICT);

    buf = replace_string ( dam_weapons[msg_index].to_char,
                           attack_hit_text[w_type].singular);
    bufeb = replace_string ( dam_weapons_brief[msg_index].to_char,
                             attack_hit_text[w_type].singular);
    buf2 = replace_string ( han_weapons[msg_index].to_char,
                            attack_hit_han[w_type].singular);
    bufhb = replace_string ( han_weapons_brief[msg_index].to_char,
                             attack_hit_han_brief[w_type].singular);
    acthanbrief(buf,buf2,bufeb, bufhb, FALSE, ch, wield, victim, TO_CHAR);

    if ( msg_index == 1 || msg_index == 2 || msg_index == 3 || msg_index == 5
         || msg_index == 7 ) {
        buf = replace_string ( dam_weapons[msg_index].to_victim,
                               attack_hit_text[w_type].plural);
        bufeb = replace_string ( dam_weapons_brief[msg_index].to_victim,
                                 attack_hit_text[w_type].plural);
    }
    else {
        buf = replace_string ( dam_weapons[msg_index].to_victim,
                               attack_hit_text[w_type].singular);
        bufeb = replace_string ( dam_weapons_brief[msg_index].to_victim,
                                 attack_hit_text[w_type].singular);
    }
    buf2 = replace_string ( han_weapons[msg_index].to_victim,
                            attack_hit_han[w_type].singular);
    bufhb = replace_string ( han_weapons_brief[msg_index].to_victim,
                             attack_hit_han_brief[w_type].singular);
    acthanbrief(buf,buf2,bufeb,bufhb, FALSE, ch, wield, victim, TO_VICT);
}

void damage(struct char_data *ch, struct char_data *victim,
            int dam, int attacktype)
{
    char buf[MAX_STRING_LENGTH];
    struct message_type *messages;
    int i,j,nr,max_hit,exp;
    extern int nokillflag;

    int hit_limit(struct char_data *ch);
 
    if(victim==NULL) return;

	if(nokillflag) {
		if((!IS_NPC(ch))&&(!IS_NPC(victim))) {
			if ( !IS_SET(world[ch->in_room].room_flags, DEATH_PLACE) )
				return ;
        }
    }


    if(GET_POS(victim) <= POSITION_DEAD){
        sprintf(buf,"Fight: %s vs. %s",ch->player.name,victim->player.name);
        log(buf);
        return;
    }
/*
  assert(GET_POS(victim) > POSITION_DEAD);
*/
    if ((GET_LEVEL(victim)>=IMO) && !IS_NPC(victim))
        dam=0;
  
    if (victim != ch) {
        if (GET_POS(victim) > POSITION_STUNNED) {
            if (!(victim->specials.fighting))
                set_fighting(victim, ch);
            GET_POS(victim) = POSITION_FIGHTING;
        }

        if (GET_POS(ch) > POSITION_STUNNED) {
            if (!(ch->specials.fighting))
                set_fighting(ch, victim);

            if (IS_NPC(ch) && IS_NPC(victim) &&
                victim->master &&
                !number(0,10) && IS_AFFECTED(victim, AFF_CHARM) &&
                (victim->master->in_room == ch->in_room)) {
                if (ch->specials.fighting)
                    stop_fighting(ch);
                hit(ch, victim->master, TYPE_UNDEFINED);
                return;
            }
        }
    }
    if (victim->master == ch)
        stop_follower(victim);
    if (IS_AFFECTED(ch, AFF_INVISIBLE))
        appear(ch);
    if (IS_AFFECTED(victim, AFF_SANCTUARY))
        dam >>= 1;
    dam=MAX(dam,0);
    GET_HIT(victim)-=dam;
    if (ch != victim && (IS_NPC(ch) || IS_NPC(victim))) {
        if (!IS_NPC(ch) || (GET_EXP(ch) < GET_LEVEL(ch) * 80000)) {
            gain_exp(ch, GET_LEVEL(victim)*dam);
        }
    }
    update_pos(victim);


    if ((attacktype >= TYPE_HIT) && (attacktype <= TYPE_CRUSH)) {
        if (!ch->equipment[WIELD]) {
            dam_message(dam, ch, victim, TYPE_HIT);
        } else {
            dam_message(dam, ch, victim, attacktype);
        }
    } else if(attacktype != TYPE_SHOOT) {

        for(i = 0; i < MAX_MESSAGES; i++) {
            if (fight_messages[i].a_type == attacktype) {
                nr=dice(1,fight_messages[i].number_of_attacks);
                for(j=1,messages=fight_messages[i].msg;(j<nr)&&(messages);j++)
                    messages=messages->next;

                if ( messages == NULL )	{/* message not found */
                    log("fight message not found") ;
                    continue ;
                }
                if (!IS_NPC(victim) && (GET_LEVEL(victim) >= IMO)) {
                    act(messages->god_msg.attacker_msg, FALSE, ch, ch->equipment[WIELD], victim, TO_CHAR);
                    act(messages->god_msg.victim_msg, FALSE, ch, ch->equipment[WIELD], victim, TO_VICT);
                    act(messages->god_msg.room_msg, FALSE, ch, ch->equipment[WIELD], victim, TO_NOTVICT);
                } else if (dam != 0) {
                    if (GET_POS(victim) == POSITION_DEAD) {
                        act(messages->die_msg.attacker_msg, FALSE, ch, ch->equipment[WIELD], victim, TO_CHAR);
                        act(messages->die_msg.victim_msg, FALSE, ch, ch->equipment[WIELD], victim, TO_VICT);
                        act(messages->die_msg.room_msg, FALSE, ch, ch->equipment[WIELD], victim, TO_NOTVICT);
                    } else {
                        act(messages->hit_msg.attacker_msg, FALSE, ch, ch->equipment[WIELD], victim, TO_CHAR);
                        act(messages->hit_msg.victim_msg, FALSE, ch, ch->equipment[WIELD], victim, TO_VICT);
                        act(messages->hit_msg.room_msg, FALSE, ch, ch->equipment[WIELD], victim, TO_NOTVICT);
                    }
                } else { /* Dam == 0 */
                    act(messages->miss_msg.attacker_msg, FALSE, ch, ch->equipment[WIELD], victim, TO_CHAR);
                    act(messages->miss_msg.victim_msg, FALSE, ch, ch->equipment[WIELD], victim, TO_VICT);
                    act(messages->miss_msg.room_msg, FALSE, ch, ch->equipment[WIELD], victim, TO_NOTVICT);
                }
            }
        }
		if ( (attacktype >= SPELL_ARMOR  && attacktype <= SPELL_SUNBURST)
			|| (attacktype >= SPELL_ENERGY_FLOW && attacktype <= SPELL_FIRE_STORM)
			|| (attacktype >= SPELL_CORN_OF_ICE && attacktype <= SPELL_CAUSE_CRITIC)
			|| attacktype == TYPE_UNDEFINED 
			|| attacktype == SPELL_FIRE_BREATH
			|| attacktype == SPELL_FROST_BREATH
			|| attacktype == SPELL_GAS_BREATH
			|| attacktype == SPELL_LIGHTNING_BREATH
			|| attacktype == SKILL_BACKSTAB
			) { 	/* by jmjeong */

			sprintf(buf, "Damage (%d) to $N", dam);
			act(buf, TRUE, victim, 0, victim, TO_ROOM);

			// sprintf(buf, "Damage (%d) to %s\n\r", 
			//		dam, victim->player.name);

			// send_to_room_except(buf, ch->in_room, victim); 
			// send_to_char(buf, ch) ;
		}
    }
    switch (GET_POS(victim)) {
    case POSITION_MORTALLYW:
        act("$n is mortally wounded, and will die soon, if not aided.", TRUE, victim, 0, 0, TO_ROOM);
        act("You are mortally wounded, and will die soon, if not aided.", FALSE, victim, 0, 0, TO_CHAR);
        break;
    case POSITION_INCAP:
        act("$n is incapacitated and will slowly die, if not aided.", TRUE, victim, 0, 0, TO_ROOM);
        act("You are incapacitated an will slowly die, if not aided.", FALSE, victim, 0, 0, TO_CHAR);
        break;
    case POSITION_STUNNED:
        act("$n is stunned, but could regain consciousness again.", TRUE, victim, 0, 0, TO_ROOM);
        act("You're stunned, but could regain consciousness again.", FALSE, victim, 0, 0, TO_CHAR);
        break;
    case POSITION_DEAD:
        act("$n is dead! R.I.P.", TRUE, victim, 0, 0, TO_ROOM);
        act("You are dead!  Sorry...", FALSE, victim, 0, 0, TO_CHAR);
        break;

    default:  /* >= POSITION SLEEPING */
        max_hit=hit_limit(victim);
        if (dam > (max_hit/5))
            act("That Really did HURT!",FALSE, victim, 0, 0, TO_CHAR);
        if (GET_HIT(victim) < (max_hit/5))
            act("You wish that your wounds would stop BLEEDING that much!",
                FALSE,victim,0,0,TO_CHAR);
        if (GET_HIT(victim) < (max_hit/5))
            if( (IS_NPC(victim) && IS_SET(victim->specials.act, ACT_WIMPY))
                ||(!IS_NPC(victim) && IS_SET(victim->specials.act, PLR_WIMPY)) )
                do_flee(victim, "", 0);
        break;  
    }

    if (GET_POS(victim) < POSITION_MORTALLYW)
        if (ch->specials.fighting == victim)
            stop_fighting(ch);

    if (!AWAKE(victim))
        if (victim->specials.fighting)
            stop_fighting(victim);

	if (!IS_NPC(victim) && !(victim->desc)) {
		if ( GET_HIT(victim) < 250 ) {
			if (ch->specials.fighting == victim)
 				stop_fighting(ch) ;
			stop_fighting(victim) ;
        }
		if ( victim->specials.fighting )
			do_flee(victim, "", 0);
		if (!victim->specials.fighting ) {
			act("$n is rescued by divine forces.", FALSE, victim, 0, 0, TO_ROOM);
			if ( GET_POS(victim) != POSITION_DEAD) {
				victim->specials.was_in_room = victim->in_room;
				char_from_room(victim);
				char_to_room(victim, 0);	/* move to VOID */
            }
        }
    }

    if (GET_POS(victim) == POSITION_DEAD) {
        if (IS_NPC(victim)){/* || victim->desc)*/
            if (IS_AFFECTED(ch, AFF_GROUP)) {
                group_gain(ch, victim);
            } else {
                /* Calculate level-difference bonus */
                exp = GET_EXP(victim)/2;
                exp = MAX(exp, 1);
                gain_exp(ch, exp);
                change_alignment(ch, victim);
            }
        }
        if (!IS_NPC(victim) || ((IS_NPC(victim)) && GET_LEVEL(victim) > 43)) {
            sprintf(buf, "<INFO> : %s killed by %s at %s",
                    GET_NAME(victim),
                    (IS_NPC(ch) ? ch->player.short_descr : GET_NAME(ch)),
                    world[victim->in_room].name);
            log(buf);
#ifdef DEATHFIGHT
            if (!IS_NPC(victim)) {	/* show players killed log */
                sprintf(buf, "System >> %s is killed by %s.\n\r", GET_NAME(victim),
                        (IS_NPC(ch) ? ch->player.short_descr : GET_NAME(ch))) ;
                send_to_all(buf) ;
			}
#else
            if (!IS_NPC(victim)) {
				strcat(buf, "\n\r");
                send_to_except(buf, ch);
            }
			else {
				if (number(0,10) > 2) {
					switch (number(1,2)) {
					case 1:
					sprintf(buf, 
						"Aaarrrrrrrrk! %s! You'll regret this!!!!!!!!!",
						GET_NAME(ch));
						break;
					case 2:
					default:
					sprintf(buf, 
						"Aaarrrrrrrrk! %s! I'll revenge you!!!!!!!!!!",
						GET_NAME(ch));
						break;
					}

					do_shout(victim,buf,0);
				}
			}
#endif 
        }
        die(victim, ch) ;	/* victim is killed by ch */
    } /* end of dead... */
}

int berserkthaco(struct char_data *ch)
{
	if (GET_LEVEL(ch) <= 30) 
		return 6;
	if (GET_LEVEL(ch) <= 35)
		return 5;
	if (GET_LEVEL(ch) <= 40)
		return 4;

	return 3;
}

int berserkdambonus(struct char_data *ch, int dam)
{
	if (GET_LEVEL(ch) <= 30) 
		return ((int)dam*1.2);
	if (GET_LEVEL(ch) <= 35)
		return ((int)dam*1.33);
	if (GET_LEVEL(ch) <= 40)
		return ((int)dam*1.5);

	return ((int)dam*1.6);
}

void hit(struct char_data *ch, struct char_data *victim, int type)
{

    struct obj_data *wielded = 0;
    struct obj_data *held = 0;
    int w_type;
    int victim_ac, calc_thaco;
    int dam,prf;
    byte diceroll;
	char buf[255];
	int class;

    extern int thaco[4][IMO+4];
    extern byte backstab_mult[];
    extern struct str_app_type str_app[];
    extern struct dex_app_type dex_app[];

    if ( ch == NULL || victim == NULL)
        return;

    if (ch->in_room != victim->in_room) {
        log("NOT SAME ROOM WHEN FIGHTING!");
        return;
    }

	if (is_solved_quest(ch, QUEST_REMORTAL)) {
		class = GET_CLASS_O(ch);
	}
	else {
		class = GET_CLASS(ch);
	}

    prf=0;
    if(IS_NPC(victim) && GET_LEVEL(victim)>GET_LEVEL(ch)+10){
        if(number(1,25)==1) prf=1;
    }
    if(IS_AFFECTED(victim,AFF_MIRROR_IMAGE) &&
       3*number(1,GET_LEVEL(ch)) < number(1,GET_LEVEL(victim)) )
        prf=2;
    if( victim->skills[SKILL_PARRY].learned > 0 ){
        if(victim->skills[SKILL_PARRY].learned>
           number(1,300)- ( GET_LEVEL(victim)-GET_LEVEL(ch) ) ) {
			if (class == CLASS_THIEF || class == CLASS_WARRIOR) {
				// 바뀌기 전 class가 thief나 warrior이거나
				// remortal을 안한 char인 경우 제대로
				// 
				prf=1;
			}
			else if (number(1,2) == 1) {
				// remortal을 하고, m이나 c에서 왔다면
				// 절반의 확률로 parry를 하도록
				// 
				prf=1;
			}
		}
    }
    if(prf){
        if(prf==1){
            send_to_char_han("You parry successfully.\n\r",
                             "공격을 피했습니다.\n\r",victim);
            acthanbrief("$n parries successfully.", "$n님이 가볍게 피합니다.",
                        "$n parries.", "$n님이 피함.", FALSE, victim,0,0,TO_ROOM);
        }
        else if(prf==2){
            send_to_char_han("You hit illusion. You are confused.\n\r",
                             "당신은 그림자(?)를 칩니다.\n\r",ch);
            acthanbrief("$n hits illusion, looks confused.",
                        "$n님이 그림자(?)를 칩니다.",
                        "$n hits illusion.",
                        "$n님이 그림자를 침.", FALSE, ch,0,0,TO_ROOM);
        }
    }

    else {
        if (ch->equipment[HOLD])
            held = ch->equipment[HOLD];

        if (ch->equipment[WIELD] &&
            (ch->equipment[WIELD]->obj_flags.type_flag == ITEM_WEAPON)) {
            wielded = ch->equipment[WIELD];
            switch (wielded->obj_flags.value[3]) {
            case 0  :
            case 1  :
            case 2  : w_type = TYPE_WHIP; break;
            case 3  : w_type = TYPE_SLASH; break;
            case 4  :
            case 5  :
            case 6  : w_type = TYPE_CRUSH; break;
            case 7  : w_type = TYPE_BLUDGEON; break;
            case 8  :
            case 9  :
            case 10 :
            case 11 : w_type = TYPE_PIERCE; break;
            case 12 : w_type = TYPE_BITE; break;
            case 13 : w_type = TYPE_STING; break;
            case 14 : w_type = TYPE_SHOOT; break;
                
            default : w_type = TYPE_HIT; break;
            }
        } else {
            if (IS_NPC(ch) && (ch->specials.attack_type >= TYPE_HIT))
                w_type = ch->specials.attack_type;
            else
                w_type = TYPE_HIT;
        }

        /* Calculate the raw armor including magic armor */
        /* The lower AC, the better                      */

        if (!IS_NPC(ch)) {
            // calc_thaco  = thaco[GET_CLASS(ch)-1][GET_LEVEL(ch)];
            calc_thaco  = thaco[class-1][GET_LEVEL(ch)];
		}
        else
            /* THAC0 for monsters is set in the HitRoll */
            calc_thaco = 20;

        calc_thaco -= str_app[STRENGTH_APPLY_INDEX(ch)].tohit;
        calc_thaco -= GET_HITROLL(ch);

		/* add for berserk skill by jmjeong */
		if (IS_SET(ch->specials.affected_by, AFF_BERSERK)) {
			calc_thaco += berserkthaco(ch);
		}
        diceroll = number(1,20);

        victim_ac  = GET_AC(victim)/10;

        if (AWAKE(victim))
            victim_ac += dex_app[GET_DEX(victim)].defensive;

        victim_ac = MAX(-20, victim_ac);  /* -20 is lowest */

/*
		for debugging:

		sprintf(buf, "calc_thaco(%d), diceroll(%d), victim_ac(%d)\n\r",
			calc_thaco, diceroll, victim_ac);
		send_to_char(buf, ch);
*/

        if ((diceroll < 20) && AWAKE(victim) &&
            ((diceroll==1) || ((calc_thaco-diceroll) > victim_ac))) {
            if (type == SKILL_BACKSTAB)
                damage(ch, victim, 0, SKILL_BACKSTAB);
            else
                damage(ch, victim, 0, w_type);
        } else {

            dam  = str_app[STRENGTH_APPLY_INDEX(ch)].todam;
            dam += GET_DAMROLL(ch);

            if (!wielded) {
                dam += dice(ch->specials.damnodice, ch->specials.damsizedice);
            } else {
                dam += dice(wielded->obj_flags.value[1], wielded->obj_flags.value[2]);
            }

            if (GET_POS(victim) < POSITION_FIGHTING)
                dam *= 1.0+(POSITION_FIGHTING-GET_POS(victim))/3.0;
            /* Position  sitting  x 1.33 */
            /* Position  resting  x 1.66 */
            /* Position  sleeping x 2.00 */
            /* Position  stunned  x 2.33 */
            /* Position  incap    x 2.66 */
            /* Position  mortally x 3.00 */

			/* by jmjeong */
			if (IS_SET(ch->specials.affected_by, AFF_BERSERK) &&
					w_type >= TYPE_HIT ) {
				/* check to see if berserked and using a weapon */
				dam = berserkdambonus(ch, dam);
			}

            if(ch->skills[SKILL_EXTRA_DAMAGING].learned>0 
									&& type != SKILL_BACKSTAB) {	
                if(ch->skills[SKILL_EXTRA_DAMAGING].learned>number(1,500)-4*GET_LEVEL(ch) )
                    dam +=number(dam/10,dam/2);
            }

            dam = MAX(1, dam);  /* Not less than 0 damage */

            if (type == SKILL_BACKSTAB) {
				dam *= backstab_mult[GET_LEVEL(ch)];
				// 현 class를 조사해서 remortal한 char이 예전에 thief가
				// 아니라면 damage 감소
				//
				if (class == CLASS_MAGIC_USER || class == CLASS_CLERIC) {
					dam = dam * 2 / 3;	
				}
				else if (class == CLASS_WARRIOR) {
					dam = dam * 4 / 5;	
				}
                damage(ch, victim, dam, SKILL_BACKSTAB);
            } else
                damage(ch, victim, dam, w_type);
        }
    } /* end parry */
}

/* control the fights going on */
/* called every 2 seconds from cc */
void perform_violence(void)
{
	void saint_extra_damage(struct char_data *ch, struct char_data *victim,
                            struct obj_data *weapon) ;
	extern void magic_weapon_hit() ;
	struct char_data *ch;
	int i,dat=100;
	struct obj_data *weapon;
	// int class;

	/*
		if (is_solved_quest(ch, QUEST_REMORTAL)) {
			class = GET_CLASS_O(ch);
		}
		else {
			class = GET_CLASS(ch);
		}
	*/

    for (ch = combat_list; ch; ch=combat_next_dude)
    {
        combat_next_dude = ch->next_fighting;
        assert(ch->specials.fighting);

        if (AWAKE(ch) && (ch->in_room==ch->specials.fighting->in_room)) {
            weapon = ch->equipment[WIELD];
            if( weapon && weapon->obj_flags.gpd > 0 && weapon->obj_flags.value[0] > 0 &&
                IS_OBJ_STAT( weapon, ITEM_MAGIC ) )
                magic_weapon_hit(ch, ch->specials.fighting, weapon );
            if ( !IS_NPC(ch) && number(1,3) == 1 && GET_ALIGNMENT(ch) > 620) {
                saint_extra_damage(ch, ch->specials.fighting, weapon) ;
            }
            hit(ch, ch->specials.fighting, TYPE_UNDEFINED);
            if(!IS_NPC(ch)){
                if(ch->skills[SKILL_DOUBLE].learned>0){	
                    if(ch->skills[SKILL_DOUBLE].learned>number(1,200)-4*GET_LEVEL(ch)){
						if (is_solved_quest(ch, QUEST_REMORTAL) &&
								(GET_CLASS_O(ch) == CLASS_MAGIC_USER || 
								 GET_CLASS_O(ch) == CLASS_CLERIC ) ) {
							if (number(1,5) > 3) 
								// 원 class가 warrior나 thief가 아닐 경우 2/5 확률로
								hit(ch,ch->specials.fighting,TYPE_UNDEFINED);
						}
						else 
							hit(ch,ch->specials.fighting,TYPE_UNDEFINED);
					}
                }
                if(ch->skills[SKILL_QUADRUPLE].learned>0){
                    if(ch->skills[SKILL_QUADRUPLE].learned>number(1,450)-8*GET_LEVEL(ch) ){
						if (is_solved_quest(ch, QUEST_REMORTAL) &&
								(GET_CLASS_O(ch) == CLASS_MAGIC_USER || 
								 GET_CLASS_O(ch) == CLASS_CLERIC ) ) {
							if (number(1,3) > 1) 
								// 2/3 확률로
								hit(ch,ch->specials.fighting,TYPE_UNDEFINED);
						}
						else 
							hit(ch,ch->specials.fighting,TYPE_UNDEFINED);
					}
                    if(ch->skills[SKILL_QUADRUPLE].learned>number(1,450)-6*GET_LEVEL(ch) ) {
						if (is_solved_quest(ch, QUEST_REMORTAL) &&
								(GET_CLASS_O(ch) == CLASS_MAGIC_USER || 
								 GET_CLASS_O(ch) == CLASS_CLERIC ) ) {
							if (number(1,3) > 2) 
								// 1/3 확률로
								hit(ch,ch->specials.fighting,TYPE_UNDEFINED);
						}
						else 
							hit(ch,ch->specials.fighting,TYPE_UNDEFINED);
                    }
                    if (GET_CLASS(ch) == CLASS_WARRIOR 
							&& GET_LEVEL(ch)+number(1,60) > 85)
                        hit(ch,ch->specials.fighting,TYPE_UNDEFINED);
                }
            }
            if(IS_NPC(ch)){
                if(!IS_SET(ch->specials.act,ACT_CLERIC) &&
                   !IS_SET(ch->specials.act,ACT_MAGE) ) 
                    dat=7;
                else dat=15;
                for(i=dat;i<GET_LEVEL(ch);i+=dat)
                    hit(ch,ch->specials.fighting,TYPE_UNDEFINED);
            }
            if(IS_AFFECTED(ch,AFF_HASTE))
                hit(ch,ch->specials.fighting,TYPE_UNDEFINED);
            if(IS_AFFECTED(ch,AFF_IMPROVED_HASTE)){
				// if (class == CLASS_MAGIC_USER) {
					hit(ch,ch->specials.fighting,TYPE_UNDEFINED);
					hit(ch,ch->specials.fighting,TYPE_UNDEFINED);
				/*
				}
				else {
					hit(ch,ch->specials.fighting,TYPE_UNDEFINED);
					if (number(1,2) == 1)
						hit(ch,ch->specials.fighting,TYPE_UNDEFINED);
				}
				*/
            }
        } else { /* Not in same room */
            stop_fighting(ch);
        }
    }
}

void saint_extra_damage(struct char_data *ch, struct char_data *victim,
                        struct obj_data *weapon)
{
	char	*buf, *buf2 ;
	int	w_type ;

	if ( ch == NULL || victim == NULL )
		return ;

	w_type = 0 ;
	if ( weapon ) {
		switch (weapon->obj_flags.value[3]) {
        case 0  :
        case 1  :
        case 2  : w_type = TYPE_WHIP - TYPE_HIT ; break;
        case 3  : w_type = TYPE_SLASH - TYPE_HIT ; break;
        case 4  :
        case 5  :
        case 6  : w_type = TYPE_CRUSH - TYPE_HIT ; break;
        case 7  : w_type = TYPE_BLUDGEON - TYPE_HIT ; break;
        case 8  :
        case 9  :
        case 10 :
        case 11 : w_type = TYPE_PIERCE - TYPE_HIT ; break;
        default : w_type = TYPE_HIT - TYPE_HIT ; break;
        }
    }

	buf = replace_string ("Holy aura strikes $N with your #W.",
                          attack_hit_text[w_type].singular);
	buf2 = replace_string ("거룩한 광채가 일더니 $N님을 #W.",
                           attack_hit_han[w_type].singular);
	acthan(buf,buf2,FALSE, ch, 0, victim, TO_CHAR);

	buf = replace_string ("Holy aura strikes $N with $n's #W.",
                          attack_hit_text[w_type].singular);
	buf2 = replace_string ("거룩한 광채가 일더니 $N님을 #W.",
                           attack_hit_han[w_type].singular);
	acthan(buf,buf2,FALSE, ch, 0, victim, TO_NOTVICT);

	buf = replace_string ("Holy aura strikes you with $n's #W.",
                          attack_hit_text[w_type].singular);
	buf2 = replace_string ("거룩한 광채가 일더니 당신을 #W.",
                           attack_hit_han[w_type].singular);
	acthan(buf,buf2,FALSE, ch, 0, victim, TO_VICT);

	if (number(1,3) == 1)
		cast_cure_critic(GET_LEVEL(ch),ch,"",SPELL_TYPE_SPELL,ch,0);

	if ( number(0, (GET_ALIGNMENT(ch) - 600 ) / 10) > 17 ) {
		if( GET_POS(victim) > POSITION_STUNNED )
			GET_POS(victim) = POSITION_STUNNED ;
    }
	else {
		if( GET_POS(victim) > POSITION_RESTING )
			GET_POS(victim) = POSITION_RESTING ;
    }
}

