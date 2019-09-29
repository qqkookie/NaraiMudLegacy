/* ************************************************************************
*  file: act.obj2.c , Implementation of commands.         Part of DIKUMUD *
*  Usage : Commands mainly using objects.                                 *
*  Copyright (C) 1990, 1991 - see 'license.doc' for complete information. *
************************************************************************* */

#include <stdio.h>
#include <string.h>
#include <assert.h>

#include "structs.h"
#include "utils.h"
#include "comm.h"
#include "interpreter.h"
#include "handler.h"
#include "db.h"
#include "spells.h"
#include "memory.h"

/* extern variables */

extern struct str_app_type str_app[];
extern struct room_data *world;
extern struct descriptor_data *descriptor_list;
extern struct room_data *world;
extern char *drinks[];
extern int drink_aff[][3];

/* extern functions */

struct obj_data *get_object_in_equip_vis(struct char_data *ch,
                                         char *arg, struct obj_data **equipment, int *j);
/* change strdup as string_dup to avoid conflict from strings.h */
char *string_dup(char *source);




void weight_change_object(struct obj_data *obj, int weight)
{
    struct obj_data *tmp_obj;
    struct char_data *tmp_ch;

	/* by jmjeong */
	GET_OBJ_WEIGHT(obj) += weight;
	if (GET_OBJ_WEIGHT(obj) < 0) GET_OBJ_WEIGHT(obj) = 0;

    if (obj->in_room != NOWHERE) {
        // GET_OBJ_WEIGHT(obj) += weight;
    } else if (tmp_ch = obj->carried_by) {
        obj_from_char(obj);
        // GET_OBJ_WEIGHT(obj) += weight;
        obj_to_char(obj, tmp_ch);
    } else if (tmp_obj = obj->in_obj) {
        obj_from_obj(obj);
        // GET_OBJ_WEIGHT(obj) += weight;
        obj_to_obj(obj, tmp_obj);
    } else {
        log("Unknown attempt to subtract weight from an object.");
    }
}



void name_from_drinkcon(struct obj_data *obj)
{
    int i;
    char *new_name;

    for(i=0; (*((obj->name)+i)!=' ') && (*((obj->name)+i)!='\0'); i++)  ;

    if (*((obj->name)+i)==' ') {
        new_name=string_dup((obj->name)+i+1);
        free_string_type(obj->name);
        obj->name=new_name;
    }
}



void name_to_drinkcon(struct obj_data *obj,int type)
{
    char *new_name;
    extern char *drinknames[];

	/*
      create(new_name,char,strlen(obj->name)+strlen(drinknames[type])+2);
	*/
	new_name = malloc_string_type(strlen(obj->name)+strlen(drinknames[type])+2);
	if ( new_name )
		sprintf(new_name,"%s %s",drinknames[type],obj->name);
    free_string_type(obj->name);
    obj->name=new_name;
}



void do_drink(struct char_data *ch, char *argument, int cmd)
{
    char buf[100];
    struct obj_data *temp;
    struct affected_type af;
    int amount,i;

    one_argument(argument,buf);
    if(!strcmp("sundew",buf)){
        temp = get_obj_in_list_vis(ch,buf,world[ch->in_room].contents);
    } else {
        temp = get_obj_in_list_vis(ch,buf,ch->carrying);
    }
    if(!temp)
    {
        act("You can't find it!",FALSE,ch,0,0,TO_CHAR);
        return;
    }
    if (temp->obj_flags.type_flag!=ITEM_DRINKCON)
    {
        act("You can't drink from that!",FALSE,ch,0,0,TO_CHAR);
        return;
    }

    if(GET_COND(ch,DRUNK)>20) /* The pig is drunk */
    {
        act("You simply fail to reach your mouth!", FALSE, ch, 0, 0, TO_CHAR);
        act("$n tried to drink but missed $s mouth!", TRUE, ch, 0, 0, TO_ROOM);
        return;
    }

    if((GET_COND(ch,FULL)>32)&&(GET_COND(ch,THIRST)>0)) /* Stomach full */
    {
        act("Your stomach can't contain anymore!",FALSE,ch,0,0,TO_CHAR);
        return;
    }

    if (temp->obj_flags.type_flag==ITEM_DRINKCON){
        if(temp->obj_flags.value[1]>0)  /* Not empty */
        {  
            sprintf(buf,"$n drinks %s from $p",drinks[temp->obj_flags.value[2]]);
            act(buf, TRUE, ch, temp, 0, TO_ROOM);
            sprintf(buf,"You drink the %s.\n\r",drinks[temp->obj_flags.value[2]]);
            send_to_char(buf,ch);
            if(temp->obj_flags.value[2]==LIQ_NECTAR)
                cast_cure_critic(GET_LEVEL(ch),ch,"",SPELL_TYPE_POTION,ch,0);
            if(temp->obj_flags.value[2]==LIQ_GOLDEN_NECTAR)
                cast_heal(GET_LEVEL(ch),ch,"",SPELL_TYPE_POTION,ch,0);
            if(temp->obj_flags.value[2]==LIQ_SUNDEW)
                cast_cure_light(GET_LEVEL(ch),ch,"",SPELL_TYPE_POTION,ch,0);


/*      if (drink_aff[temp->obj_flags.value[2]][DRUNK] > 0 )
        amount = (25-GET_COND(ch,THIRST))/drink_aff[temp->obj_flags.value[2]][DRUNK];
        else
        amount = number(3,10);   */
            amount = 5;

            amount = MIN(amount, temp->obj_flags.value[1]);

            weight_change_object(temp, -amount/5);  /* Subtract amount */

            gain_condition(ch,DRUNK,(int)((int)drink_aff
                                          [temp->obj_flags.value[2]][DRUNK]*amount)/4);

            gain_condition(ch,FULL,(int)((int)drink_aff
                                         [temp->obj_flags.value[2]][FULL]*amount)/4);

            gain_condition(ch,THIRST,(int)((int)drink_aff
                                           [temp->obj_flags.value[2]][THIRST]*amount)/4);

            if(GET_COND(ch,DRUNK)>20)
                act("You feel drunk.",FALSE,ch,0,0,TO_CHAR);

            if(GET_COND(ch,THIRST)>32)
                act("You do not feel thirsty.",FALSE,ch,0,0,TO_CHAR);

            if(GET_COND(ch,FULL)>32)
                act("You are full.",FALSE,ch,0,0,TO_CHAR);

            switch(temp->obj_flags.value[3]) {
            case 1 :    /* The shit was poisoned ! */
                act("Ooups, it tasted rather strange ?!!?",FALSE,ch,0,0,TO_CHAR);
                act("$n chokes and utters some strange sounds.",
                    TRUE,ch,0,0,TO_ROOM);
                af.type = SPELL_POISON;
                af.duration = amount*3;
                af.modifier = 0;
                af.location = APPLY_NONE;
                af.bitvector = AFF_POISON;
                affect_join(ch,&af, FALSE, FALSE);
                break;
            case 2 :    /* hit point restore */
                act("You feel momentarily warm.",FALSE,ch,0,0,TO_CHAR);
                acthan("$n blushes.", "$n님의 얼굴이 붉어집니다.",
                       TRUE,ch,0,0,TO_ROOM);
                GET_HIT(ch) = MIN(GET_MAX_HIT(ch),GET_HIT(ch) + 
                                  dice(drink_aff[temp->obj_flags.value[2]][DRUNK], 8));
                break;
            case 3 :    /* mana restore */
                act("You feel momentarily warm.",FALSE,ch,0,0,TO_CHAR);
                act("$n blushes.", TRUE,ch,0,0,TO_ROOM);
                GET_MANA(ch) = MIN(GET_MAX_MANA(ch),GET_MANA(ch) + 
                                   dice(drink_aff[temp->obj_flags.value[2]][DRUNK], 7));
                break ;
            case 4 :    /* move restore */
                break;
            case 0 :    /* nothing */
            default : break ;
            }

            /* empty the container, and no longer poison. */
            temp->obj_flags.value[1] -= amount;
            if(!temp->obj_flags.value[1]) {  /* The last bit */
                temp->obj_flags.value[2]=0;
                temp->obj_flags.value[3]=0;
                name_from_drinkcon(temp);
            }
            return;

        }
    }
    act("It's empty already.",FALSE,ch,0,0,TO_CHAR);
    return;
}

void do_eat(struct char_data *ch, char *argument, int cmd)
{
    char buf[100];
    struct obj_data *temp;
    struct affected_type af;
    extern struct index_data *obj_index;
    char *select_fortune_cookie();

    one_argument(argument,buf);

    if(!(temp = get_obj_in_list_vis(ch,buf,ch->carrying))) {
        act("You can't find it!",FALSE,ch,0,0,TO_CHAR);
        return;
    }
    if((temp->obj_flags.type_flag != ITEM_FOOD) && (GET_LEVEL(ch) < (IMO+1))) {
        act("Your stomach refuses to eat that!?!",FALSE,ch,0,0,TO_CHAR);
        return;
    }
    if(GET_COND(ch,FULL)>32) /* Stomach full */ {  
        act("You are too full to eat more!",FALSE,ch,0,0,TO_CHAR);
        return;
    }
    act("$n eats $p",TRUE,ch,temp,0,TO_ROOM);
    act("You eat the $o.",FALSE,ch,temp,0,TO_CHAR);
    gain_condition(ch,FULL,temp->obj_flags.value[0]);
    if(GET_COND(ch,FULL)>32)
        act("You are full.",FALSE,ch,0,0,TO_CHAR);
    if((temp->obj_flags.value[3] == 1) && (GET_LEVEL(ch)<IMO)) {
        act("Ooups, it tasted rather strange ?!!?",FALSE,ch,0,0,TO_CHAR);
        act("$n coughs and utters some strange sounds.",FALSE,ch,0,0,TO_ROOM);
        af.type = SPELL_POISON;
        af.duration = temp->obj_flags.value[0]*2;
        af.modifier = 0;
        af.location = APPLY_NONE;
        af.bitvector = AFF_POISON;
        affect_join(ch,&af, FALSE, FALSE);
    }
    if(temp->obj_flags.value[3] >=2) {
        act("Wow, you feel great !!",FALSE,ch,0,0,TO_CHAR);
        act("$n looks better now.",FALSE,ch,0,0,TO_ROOM);
        GET_HIT(ch) = MIN(GET_MAX_HIT(ch),GET_HIT(ch)
                          + dice(2,4) * temp->obj_flags.value[3]);
        GET_MANA(ch) = MIN(GET_MAX_MANA(ch),GET_MANA(ch)
                           + dice(2,2) * temp->obj_flags.value[3]);
        GET_MOVE(ch) = MIN(GET_MAX_MOVE(ch),GET_MOVE(ch)
                           + dice(2,3) * temp->obj_flags.value[3]);
        update_pos(ch) ;

		WAIT_STATE(ch, PULSE_VIOLENCE/8);
    }
    if ( obj_index[temp->item_number].virtual == 2701 ) { /* fortune cookie */
        send_to_char("There is some scripts inside the cookie. It reads..\n\r",ch);
        send_to_char(select_fortune_cookie(), ch);
    }
    if ( obj_index[temp->item_number].virtual == 16012 ) { /* Do-ra-ji */
        act("햐~ 기분이 좋은데 ?",FALSE,ch,0,0,TO_CHAR);
        act("$n님께서 기운을 차립니다.",FALSE,ch,0,0,TO_ROOM);
        GET_MANA(ch) = MIN(GET_MAX_MANA(ch),GET_MANA(ch) + dice(10,5) );
        update_pos(ch) ;
    }
    if ( obj_index[temp->item_number].virtual == 16011 ) { /* San-sam */
        act("끄으으억 !!",FALSE,ch,0,0,TO_CHAR);
        act("$n님이 갑자기 힘이 솟나 봅니다.",FALSE,ch,0,0,TO_ROOM);
        GET_HIT(ch) = MIN(GET_MAX_HIT(ch),GET_HIT(ch) + dice(20,10) );
        GET_MANA(ch) = MIN(GET_MAX_MANA(ch),GET_MANA(ch) + dice(20,10) );
        GET_MOVE(ch) = MIN(GET_MAX_MOVE(ch),GET_MOVE(ch) + dice(20,10) );
        update_pos(ch) ;
    }
    extract_obj(temp);
}

void do_jun(struct char_data *ch, char *argument, int cmd)
{
	send_to_char("If you want to junk, type 'junk'.\n\r", ch) ;
}

void do_junk(struct char_data *ch, char *argument, int cmd)
{
    char buf[BUFSIZ];
    struct obj_data *temp;

	if ( *argument == 0 ) {
		send_to_char("Junk what?\n\r", ch) ;
		return ;
    }

    one_argument(argument,buf);

    if(!(temp = get_obj_in_list_vis(ch,buf,ch->carrying))) {
        act("You can't find it!",FALSE,ch,0,0,TO_CHAR);
        return;
    }
    if (IS_OBJ_STAT(temp, ITEM_NODROP)) {
        act("You can't junk it! Cursed?",FALSE,ch,0,0,TO_CHAR);
        return;
    }
    act("$n junks $p",TRUE,ch,temp,0,TO_ROOM);
    act("You junk the $o.",FALSE,ch,temp,0,TO_CHAR);
    extract_obj(temp);
}

void do_pour(struct char_data *ch, char *argument, int cmd)
{
    char arg1[MAX_STRING_LENGTH];
    char arg2[MAX_STRING_LENGTH];
    char buf[MAX_STRING_LENGTH];
    struct obj_data *from_obj;
    struct obj_data *to_obj;
    int amount;

    argument_interpreter(argument, arg1, arg2);

    if(!*arg1) /* No arguments */
    {
        act("What do you want to pour from?",FALSE,ch,0,0,TO_CHAR);
        return;
    }

    if(!strcmp("sundew",arg1)){
        from_obj = get_obj_in_list_vis(ch,arg1,world[ch->in_room].contents);
    } else {
        from_obj = get_obj_in_list_vis(ch,arg1,ch->carrying);
    }
    if(!from_obj)
    {
        act("You can't find it!",FALSE,ch,0,0,TO_CHAR);
        return;
    }

    if(from_obj->obj_flags.type_flag!=ITEM_DRINKCON)
    {
        act("You can't pour from that!",FALSE,ch,0,0,TO_CHAR);
        return;
    }

    if(from_obj->obj_flags.value[1]==0)
    {
        act("The $p is empty.",FALSE, ch,from_obj, 0,TO_CHAR);
        return;
    }

    if(!*arg2)
    {
        act("Where do you want it? Out or in what?",FALSE,ch,0,0,TO_CHAR);
        return;
    }

    if(!str_cmp(arg2,"out"))
    {
		if(!strcmp("sundew",arg1)) {
			act("You can't make sundew empty.",FALSE,ch,0,0,TO_CHAR);
			return;
		}
        act("$n empties $p", TRUE, ch,from_obj,0,TO_ROOM);
        act("You empty the $p.", FALSE, ch,from_obj,0,TO_CHAR);

        /* liquid 5 is weight 1 - by cold */
        weight_change_object(from_obj, -from_obj->obj_flags.value[1]/5); /* Empty */

        from_obj->obj_flags.value[1]=0;
        from_obj->obj_flags.value[2]=0;
        from_obj->obj_flags.value[3]=0;
        name_from_drinkcon(from_obj);
    
        return;

    }

    if(!(to_obj = get_obj_in_list_vis(ch,arg2,ch->carrying)))
    {
        act("You can't find it!",FALSE,ch,0,0,TO_CHAR);
        return;
    }

    if(to_obj->obj_flags.type_flag!=ITEM_DRINKCON)
    {
        act("You can't pour anything into that.",FALSE,ch,0,0,TO_CHAR);
        return;
    }
    if((to_obj->obj_flags.value[1]!=0)&&
       (to_obj->obj_flags.value[2]!=from_obj->obj_flags.value[2]))
    {
        act("There is already another liquid in it!",FALSE,ch,0,0,TO_CHAR);
        return;
    }
    if(!(to_obj->obj_flags.value[1]<to_obj->obj_flags.value[0]))
    {
        act("There is no room for more.",FALSE,ch,0,0,TO_CHAR);
        return;
    }
    if(from_obj == to_obj)
    {
        act("That would be silly.",FALSE,ch,0,0,TO_CHAR);
        return;
    }
    sprintf(buf,"You pour the %s into the %s.\n\r",
            drinks[from_obj->obj_flags.value[2]],arg2);
    send_to_char(buf,ch);

    /* New alias */
    if (to_obj->obj_flags.value[1]==0) 
        name_to_drinkcon(to_obj,from_obj->obj_flags.value[2]);

    /* First same type liq. */
    to_obj->obj_flags.value[2]=from_obj->obj_flags.value[2];

    /* Then how much to pour */
    from_obj->obj_flags.value[1]-= (amount=
                                    (to_obj->obj_flags.value[0]-to_obj->obj_flags.value[1]));

    to_obj->obj_flags.value[1]=to_obj->obj_flags.value[0];

    if(from_obj->obj_flags.value[1]<0)    /* There was to little */
    {
        to_obj->obj_flags.value[1]+=from_obj->obj_flags.value[1];
        amount += from_obj->obj_flags.value[1];
        from_obj->obj_flags.value[1]=0;
        from_obj->obj_flags.value[2]=0;
        from_obj->obj_flags.value[3]=0;
        name_from_drinkcon(from_obj);
    }

    /* Then the poison boogie */
    to_obj->obj_flags.value[3]=
        (to_obj->obj_flags.value[3]||from_obj->obj_flags.value[3]);

    /* And the weight boogie */

    weight_change_object(from_obj, -amount/5);
    weight_change_object(to_obj, amount/5);   /* Add weight */

    return;
}

void do_sip(struct char_data *ch, char *argument, int cmd)
{
    struct affected_type af;
    char arg[MAX_STRING_LENGTH];
    char buf[MAX_STRING_LENGTH];
    struct obj_data *temp;

    one_argument(argument,arg);

    if(!(temp = get_obj_in_list_vis(ch,arg,ch->carrying)))
    {
        act("You can't find it!",FALSE,ch,0,0,TO_CHAR);
        return;
    }

    if(temp->obj_flags.type_flag!=ITEM_DRINKCON)
    {
        act("You can't sip from that!",FALSE,ch,0,0,TO_CHAR);
        return;
    }

    if(GET_COND(ch,DRUNK)>10) /* The pig is drunk ! */
    {
        act("You simply fail to reach your mouth!",FALSE,ch,0,0,TO_CHAR);
        act("$n tries to sip, but fails!",TRUE,ch,0,0,TO_ROOM);
        return;
    }

    if(!temp->obj_flags.value[1])  /* Empty */
    {
        act("But there is nothing in it?",FALSE,ch,0,0,TO_CHAR);
        return;
    }

    act("$n sips from the $o",TRUE,ch,temp,0,TO_ROOM);
    sprintf(buf,"It tastes like %s.\n\r",drinks[temp->obj_flags.value[2]]);
    send_to_char(buf,ch);

    gain_condition(ch,DRUNK,(int)(drink_aff[temp->obj_flags.value[2]][DRUNK]/4));

    gain_condition(ch,FULL,(int)(drink_aff[temp->obj_flags.value[2]][FULL]/4));

    gain_condition(ch,THIRST,(int)(drink_aff[temp->obj_flags.value[2]][THIRST]/4));

	/* To implement weight of liquid == 0.2, by cold */
    if ( temp->obj_flags.value[1] % 5 == 0 )
        weight_change_object(temp, -1);  /* Subtract one unit */

    if(GET_COND(ch,DRUNK)>10)
        act("You feel drunk.",FALSE,ch,0,0,TO_CHAR);

    if(GET_COND(ch,THIRST)>20)
        act("You do not feel thirsty.",FALSE,ch,0,0,TO_CHAR);

    if(GET_COND(ch,FULL)>20)
        act("You are full.",FALSE,ch,0,0,TO_CHAR);

    if(temp->obj_flags.value[3]&&!IS_AFFECTED(ch,AFF_POISON)) /* The shit was poisoned ! */
    {
        act("But it also had a strange taste!",FALSE,ch,0,0,TO_CHAR);

        af.type = SPELL_POISON;
        af.duration = 3;
        af.modifier = 0;
        af.location = APPLY_NONE;
        af.bitvector = AFF_POISON;
        affect_to_char(ch,&af);
    }

    temp->obj_flags.value[1]--;

    if(!temp->obj_flags.value[1])  /* The last bit */
    {
        temp->obj_flags.value[2]=0;
        temp->obj_flags.value[3]=0;
        name_from_drinkcon(temp);
    }

    return;

}


void do_taste(struct char_data *ch, char *argument, int cmd)
{
    struct affected_type af;
    char arg[MAX_STRING_LENGTH];
    char buf[MAX_STRING_LENGTH];
    struct obj_data *temp;

    one_argument(argument,arg);

    if(!(temp = get_obj_in_list_vis(ch,arg,ch->carrying)))
    {
        act("You can't find it!",FALSE,ch,0,0,TO_CHAR);
        return;
    }

    if(temp->obj_flags.type_flag==ITEM_DRINKCON)
    {
        do_sip(ch,argument,0);
        return;
    }

    if(!(temp->obj_flags.type_flag==ITEM_FOOD))
    {
        act("Taste that?!? Your stomach refuses!",FALSE,ch,0,0,TO_CHAR);
        return;
    }

    act("$n tastes the $o", FALSE, ch, temp, 0, TO_ROOM);
    act("You taste the $o", FALSE, ch, temp, 0, TO_CHAR);

    gain_condition(ch,FULL,1);

    if(GET_COND(ch,FULL)>20)
        act("You are full.",FALSE,ch,0,0,TO_CHAR);

    if(temp->obj_flags.value[3]&&!IS_AFFECTED(ch,AFF_POISON)) /* The shit was poisoned ! */
    {
        act("Ooups, it did not taste good at all!",FALSE,ch,0,0,TO_CHAR);

        af.type = SPELL_POISON;
        af.duration = 2;
        af.modifier = 0;
        af.location = APPLY_NONE;
        af.bitvector = AFF_POISON;
        affect_to_char(ch,&af);
    }

    temp->obj_flags.value[0]--;

    if(!temp->obj_flags.value[0])  /* Nothing left */
    {
        act("There is nothing left now.",FALSE,ch,0,0,TO_CHAR);
        extract_obj(temp);
    }

    return;

}



/* functions related to wear */

perform_wear(struct char_data *ch, struct obj_data *obj_object, int keyword)
{
    char buffer[MAX_STRING_LENGTH];
    struct char_data *i;

    switch(keyword) {
    case 0 :
        act("$n light $p and holds it.", FALSE, ch, obj_object,0,TO_ROOM);
        break;
    case 1 : 
        act("$n wears $p on $s finger.", TRUE, ch, obj_object,0,TO_ROOM);
        break;
    case 2 : 
        act("$n wears $p around $s neck.", TRUE, ch, obj_object,0,TO_ROOM);
        break;
    case 3 : 
        act("$n wears $p on $s body.", TRUE, ch, obj_object,0,TO_ROOM);
        break;
    case 4 : 
        act("$n wears $p on $s head.",TRUE, ch, obj_object,0,TO_ROOM);
        break;
    case 5 : 
        act("$n wears $p on $s legs.",TRUE, ch, obj_object,0,TO_ROOM);
        break;
    case 6 : 
        act("$n wears $p on $s feet.",TRUE, ch, obj_object,0,TO_ROOM);
        break;
    case 7 : 
        act("$n wears $p on $s hands.",TRUE, ch, obj_object,0,TO_ROOM);
        break;
    case 8 : 
        act("$n wears $p on $s arms.",TRUE, ch, obj_object,0,TO_ROOM);
        break;
    case 9 : 
        act("$n wears $p about $s body.",TRUE, ch, obj_object,0,TO_ROOM);
        break;
    case 10 : 
        act("$n wears $p about $s waist.",TRUE, ch, obj_object,0,TO_ROOM);
        break;
    case 11 : 
        act("$n wears $p around $s wrist.",TRUE, ch, obj_object,0,TO_ROOM);
        break;
    case 12 : 
        act("$n wields $p.",TRUE, ch, obj_object,0,TO_ROOM);
        break;
    case 13 : 
        act("$n grabs $p.",TRUE, ch, obj_object,0,TO_ROOM);
        break;
    case 14 : 
        act("$n starts using $p as a shield.", TRUE, ch, obj_object,0,TO_ROOM);
        break;
    case 15 :
        act("$n wears $p around $s knee.",TRUE, ch, obj_object,0,TO_ROOM);
        break;
    case 16 :
        act("$n wears $p about $s legs.",TRUE, ch, obj_object,0,TO_ROOM);
        break;
    }
}


void wear(struct char_data *ch, struct obj_data *obj_object, int keyword)
{
    char buffer[MAX_STRING_LENGTH];
    int  gpd ;

    gpd = obj_object->obj_flags.gpd ;
    if(IS_OBJ_STAT(obj_object,ITEM_NOLOAD) ) {
        if ( gpd == 0 && (GET_LEVEL(ch) < 10)){
            sprintf(buffer,"You are too lowly to use %s.\n\r",
                    obj_object->short_description);
            send_to_char(buffer,ch);
            return;
        }
        if ( gpd < IMO && (GET_LEVEL(ch) < gpd)) {
            sprintf(buffer,"You can use %s from %d level.\n\r",
                    obj_object->short_description, gpd);
            send_to_char(buffer,ch);
            return;
        }
    }
    switch(keyword) {
    case 0: {  /* LIGHT SOURCE */
        if (ch->equipment[WEAR_LIGHT])
            send_to_char("You are already holding a light source.\n\r", ch);
        else {
            send_to_char("Ok.\n\r", ch);
            perform_wear(ch,obj_object,keyword);
            obj_from_char(obj_object);
            equip_char(ch,obj_object, WEAR_LIGHT);
            if (obj_object->obj_flags.value[2])
                world[ch->in_room].light++;
        }
    } break;

    case 1: {
        if (CAN_WEAR(obj_object,ITEM_WEAR_FINGER)) {
            if ((ch->equipment[WEAR_FINGER_L]) && (ch->equipment[WEAR_FINGER_R])) {
                send_to_char(
                    "You are already wearing something on your fingers.\n\r", ch);
            } else {
                perform_wear(ch,obj_object,keyword);
                if (ch->equipment[WEAR_FINGER_L]) {
                    sprintf(buffer, "You put the %s on your right finger.\n\r", 
                            fname(obj_object->name));
                    send_to_char(buffer, ch);
                    obj_from_char(obj_object);
                    equip_char(ch, obj_object, WEAR_FINGER_R);
                } else {
                    sprintf(buffer, "You put the %s on your left finger.\n\r", 
                            fname(obj_object->name));
                    send_to_char(buffer, ch);
                    obj_from_char(obj_object);
                    equip_char(ch, obj_object, WEAR_FINGER_L);
                }
            }
        } else {
            send_to_char("You can't wear that on your finger.\n\r", ch);
        }
    } break;
    case 2: {
        if (CAN_WEAR(obj_object,ITEM_WEAR_NECK)) {
            if ((ch->equipment[WEAR_NECK_1]) && (ch->equipment[WEAR_NECK_2])) {
                send_to_char("You can't wear any more around your neck.\n\r", ch);
            } else {
                send_to_char("OK.\n\r", ch);
                perform_wear(ch,obj_object,keyword);
                if (ch->equipment[WEAR_NECK_1]) {
                    obj_from_char(obj_object);
                    equip_char(ch, obj_object, WEAR_NECK_2);
                } else {
                    obj_from_char(obj_object);
                    equip_char(ch, obj_object, WEAR_NECK_1);
                }
            }
        } else {
            send_to_char("You can't wear that around your neck.\n\r", ch);
        }
    } break;
    case 3: {
        if (CAN_WEAR(obj_object,ITEM_WEAR_BODY)) {
            if (ch->equipment[WEAR_BODY]) {
                send_to_char("You already wear something on your body.\n\r", ch);
            } else {
                send_to_char("OK.\n\r", ch);
                perform_wear(ch,obj_object,keyword);
                obj_from_char(obj_object);
                equip_char(ch,  obj_object, WEAR_BODY);
            }
        } else {
            send_to_char("You can't wear that on your body.\n\r", ch);
        }
    } break;
    case 4: {
        if (CAN_WEAR(obj_object,ITEM_WEAR_HEAD)) {
            if (ch->equipment[WEAR_HEAD]) {
                send_to_char("You already wear something on your head.\n\r", ch);
            } else {
                send_to_char("OK.\n\r", ch);
                perform_wear(ch,obj_object,keyword);
                obj_from_char(obj_object);
                equip_char(ch, obj_object, WEAR_HEAD);
            }
        } else {
            send_to_char("You can't wear that on your head.\n\r", ch);
        }
    } break;
    case 5: {
        if (CAN_WEAR(obj_object,ITEM_WEAR_LEGS)) {
            if (ch->equipment[WEAR_LEGS]) {
                send_to_char("You already wear something on your legs.\n\r", ch);
            } else {
                send_to_char("OK.\n\r", ch);
                perform_wear(ch,obj_object,keyword);
                obj_from_char(obj_object);
                equip_char(ch, obj_object, WEAR_LEGS);
            }
        } else {
            send_to_char("You can't wear that on your legs.\n\r", ch);
        }
    } break;
    case 6: {
        if (CAN_WEAR(obj_object,ITEM_WEAR_FEET)) {
            if (ch->equipment[WEAR_FEET]) {
                send_to_char("You already wear something on your feet.\n\r", ch);
            } else {
                send_to_char("OK.\n\r", ch);
                perform_wear(ch,obj_object,keyword);
                obj_from_char(obj_object);
                equip_char(ch, obj_object, WEAR_FEET);
            }
        } else {
            send_to_char("You can't wear that on your feet.\n\r", ch);
        }
    } break;
    case 7: {
        if (CAN_WEAR(obj_object,ITEM_WEAR_HANDS)) {
            if (ch->equipment[WEAR_HANDS]) {
                send_to_char("You already wear something on your hands.\n\r", ch);
            } else {
                send_to_char("OK.\n\r", ch);
                perform_wear(ch,obj_object,keyword);
                obj_from_char(obj_object);
                equip_char(ch, obj_object, WEAR_HANDS);
            }
        } else {
            send_to_char("You can't wear that on your hands.\n\r", ch);
        }
    } break;
    case 8: {
        if (CAN_WEAR(obj_object,ITEM_WEAR_ARMS)) {
            if (ch->equipment[WEAR_ARMS]) {
                send_to_char("You already wear something on your arms.\n\r", ch);
            } else {
                send_to_char("OK.\n\r", ch);
                perform_wear(ch,obj_object,keyword);
                obj_from_char(obj_object);
                equip_char(ch, obj_object, WEAR_ARMS);
            }
        } else {
            send_to_char("You can't wear that on your arms.\n\r", ch);
        }
    } break;
    case 9: {
        if (CAN_WEAR(obj_object,ITEM_WEAR_ABOUT)) {
            if (ch->equipment[WEAR_ABOUT]) {
                send_to_char("You already wear something about your body.\n\r", ch);
            } else {
                send_to_char("OK.\n\r", ch);
                perform_wear(ch,obj_object,keyword);
                obj_from_char(obj_object);
                equip_char(ch, obj_object, WEAR_ABOUT);
            }
        } else {
            send_to_char("You can't wear that about your body.\n\r", ch);
        }
    } break;
    case 10: {
        if (CAN_WEAR(obj_object,ITEM_WEAR_WAISTE)) {
            if (ch->equipment[WEAR_WAISTE]) {
                send_to_char("You already wear something about your waist.\n\r",
                             ch);
            } else {
                send_to_char("OK.\n\r", ch);
                perform_wear(ch,obj_object,keyword);
                obj_from_char(obj_object);
                equip_char(ch,  obj_object, WEAR_WAISTE);
            }
        } else {
            send_to_char("You can't wear that about your waist.\n\r", ch);
        }
    } break;
    case 11: {
        if (CAN_WEAR(obj_object,ITEM_WEAR_WRIST)) {
            if ((ch->equipment[WEAR_WRIST_L]) && (ch->equipment[WEAR_WRIST_R])) {
                send_to_char(
                    "You already wear something around both your wrists.\n\r", ch);
            } else {
                perform_wear(ch,obj_object,keyword);
                obj_from_char(obj_object);
                if (ch->equipment[WEAR_WRIST_L]) {
                    sprintf(buffer, "You wear the %s around your right wrist.\n\r", 
                            fname(obj_object->name));
                    send_to_char(buffer, ch);
                    equip_char(ch,  obj_object, WEAR_WRIST_R);
                } else {
                    sprintf(buffer, "You wear the %s around your left wrist.\n\r", 
                            fname(obj_object->name));
                    send_to_char(buffer, ch);
                    equip_char(ch, obj_object, WEAR_WRIST_L);
                }
            }
        } else {
            send_to_char("You can't wear that around your wrist.\n\r", ch);
        }
    } break;

    case 15:{
        if (CAN_WEAR(obj_object,ITEM_WEAR_KNEE)) {
            if ((ch->equipment[WEAR_KNEE_R]) && (ch->equipment[WEAR_KNEE_L])) {
                send_to_char("You already wear something around your both knees.\n\r", ch);
            } else {
                perform_wear(ch,obj_object,keyword);
                obj_from_char(obj_object);
                if (ch->equipment[WEAR_KNEE_L]) {
                    sprintf(buffer, "You put the %s on your right knee.\n\r",
                            fname(obj_object->name));
                    send_to_char(buffer, ch);
                    equip_char(ch, obj_object, WEAR_KNEE_R);
                } else {
                    sprintf(buffer, "You put the %s on your left knee.\n\r",
                            fname(obj_object->name));
                    send_to_char(buffer, ch);
                    equip_char(ch, obj_object, WEAR_KNEE_L);
                }
            }
        } else {
            send_to_char("You cant'wear that on around your both knees.\n\r", ch);       }
    } break;

    case 16: {
        if (CAN_WEAR(obj_object,ITEM_WEAR_ABOUTLEGS)) {
            if (ch->equipment[WEAR_ABOUTLEGS]) {
                send_to_char("You already wear something about your legs.\n\r", ch);
            } else {
                send_to_char("OK.\n\r", ch);
                perform_wear(ch,obj_object,keyword);
                obj_from_char(obj_object);
                equip_char(ch, obj_object, WEAR_ABOUTLEGS);
            }
        } else {
            send_to_char("You can't wear that about your legs.\n\r", ch);
        }
    } break;
    case 12:
        if (CAN_WEAR(obj_object,ITEM_WIELD)) {
            if (ch->equipment[WIELD]) {
                send_to_char("You are already wielding something.\n\r", ch);
            } else {
                /* Cleric execption has been removed, and is temporarily placed */
                /* at the end of this file                                      */

                if (GET_OBJ_WEIGHT(obj_object) >
                    str_app[STRENGTH_APPLY_INDEX(ch)].wield_w) {
                    send_to_char("It is too heavy for you to use.\n\r",ch);
                } else {
                    send_to_char("OK.\n\r", ch);
                    perform_wear(ch,obj_object,keyword);
                    obj_from_char(obj_object);
                    equip_char(ch, obj_object, WIELD);
                }
            }
        } else {
            send_to_char("You can't wield that.\n\r", ch);
        }
        break;

    case 13:
        if (CAN_WEAR(obj_object,ITEM_HOLD)) {
            if (ch->equipment[HOLD]) {
                send_to_char("You are already holding something.\n\r", ch);
            } else {
                /* Cleric execption has been removed, and is temporarily placed */
                /* at the end of this file                                      */

                send_to_char("OK.\n\r", ch);
                perform_wear(ch,obj_object,keyword);
                obj_from_char(obj_object);
                equip_char(ch, obj_object, HOLD);
            }
        } else {
            send_to_char("You can't hold this.\n\r", ch);
        }
        break;
    case 14: {
        if (CAN_WEAR(obj_object,ITEM_WEAR_SHIELD)) {
            if ((ch->equipment[WEAR_SHIELD])) {
                send_to_char(
                    "You are already using a shield\n\r", ch);
            } else {
                perform_wear(ch,obj_object,keyword);
                sprintf(buffer, "You start using the %s.\n\r", 
                        fname(obj_object->name));
                send_to_char(buffer, ch);
                obj_from_char(obj_object);
                equip_char(ch, obj_object, WEAR_SHIELD);
            }
        } else {
            send_to_char("You can't use that as a shield.\n\r", ch);
        }
    } break;
    case -1: {
        sprintf(buffer,"Wear %s where?.\n\r", fname(obj_object->name));
        send_to_char(buffer, ch);
    } break;
    case -2: {
        sprintf(buffer,"You can't wear the %s.\n\r", fname(obj_object->name));
        send_to_char(buffer, ch);
    } break;
    default: {
        log("Unknown type called in wear.");
    } break;
    }
}
int where_wear(struct obj_data *obj_object)
{
    if (CAN_WEAR(obj_object,ITEM_WEAR_KNEE)) return( 15 );
    if (CAN_WEAR(obj_object,ITEM_WEAR_ABOUTLEGS)) return( 16 );
    if (CAN_WEAR(obj_object,ITEM_WEAR_SHIELD)) return( 14 );
    if (CAN_WEAR(obj_object,ITEM_WEAR_WRIST)) return( 11 );
    if (CAN_WEAR(obj_object,ITEM_WEAR_WAISTE)) return( 10 );
    if (CAN_WEAR(obj_object,ITEM_WEAR_ARMS)) return( 8 );
    if (CAN_WEAR(obj_object,ITEM_WEAR_HANDS)) return( 7 );
    if (CAN_WEAR(obj_object,ITEM_WEAR_FEET)) return( 6 );
    if (CAN_WEAR(obj_object,ITEM_WEAR_LEGS)) return( 5 );
    if (CAN_WEAR(obj_object,ITEM_WEAR_ABOUT)) return( 9 );
    if (CAN_WEAR(obj_object,ITEM_WEAR_HEAD)) return( 4 );
    if (CAN_WEAR(obj_object,ITEM_WEAR_BODY)) return( 3 );
    if (CAN_WEAR(obj_object,ITEM_WEAR_FINGER)) return( 1 );
    if (CAN_WEAR(obj_object,ITEM_WEAR_NECK)) return( 2 );
    return(-2);
}
void do_wear(struct char_data *ch, char *argument, int cmd) {
	char arg1[MAX_STRING_LENGTH];
	char arg2[MAX_STRING_LENGTH];
	char buf[256];
	char buffer[MAX_STRING_LENGTH];
	struct obj_data *obj_object,*next_object;
	int keyword;
	static char *keywords[] = {
		"finger",
		"neck",
		"body",
		"head",
		"legs",
		"aboutlegs",
		"knee",
		"feet",
		"hands",
		"arms",
		"about",
		"waist",
		"wrist",
		"shield",
		"\n"
    };

    argument_interpreter(argument, arg1, arg2);
    if (*arg1) {
        if(strcmp(arg1,"all")==0){
            for(obj_object = ch->carrying ; obj_object ; obj_object=next_object){
                next_object = obj_object->next_content;
                if(CAN_SEE_OBJ(ch,obj_object)){
                    keyword = where_wear(obj_object);
                    if(keyword < 0) continue;
                    wear(ch , obj_object , keyword);
                }
            }
            return;
        }
        obj_object = get_obj_in_list_vis(ch, arg1, ch->carrying);
        if (obj_object) {
            if (*arg2) {
                keyword = search_block(arg2, keywords, FALSE); /* Partial Match */
                if (keyword == -1) {
                    sprintf(buf, "%s is an unknown body location.\n\r", arg2);
                    send_to_char(buf, ch);
                } else {
                    wear(ch, obj_object, keyword+1);
                }
            } else {
                keyword = where_wear(obj_object);
                wear(ch, obj_object, keyword);
            }
        } else {
            sprintf(buffer, "You do not seem to have the '%s'.\n\r",arg1);
            send_to_char(buffer,ch);
        }
    } else {
        send_to_char("Wear what?\n\r", ch);
    }
}
void do_wield(struct char_data *ch, char *argument, int cmd) {
    char arg1[MAX_STRING_LENGTH];
    char arg2[MAX_STRING_LENGTH];
    char buffer[MAX_STRING_LENGTH];
    struct obj_data *obj_object;
    int keyword = 12;

    argument_interpreter(argument, arg1, arg2);
    if (*arg1) {
        obj_object = get_obj_in_list_vis(ch, arg1, ch->carrying);
        if (obj_object) {
            wear(ch, obj_object, keyword);
        } else {
            sprintf(buffer, "You do not seem to have the '%s'.\n\r",arg1);
            send_to_char(buffer,ch);
        }
    } else {
        send_to_char("Wield what?\n\r", ch);
    }
}


void do_grab(struct char_data *ch, char *argument, int cmd)
{
    char arg1[MAX_STRING_LENGTH];
    char arg2[MAX_STRING_LENGTH];
    char buffer[MAX_STRING_LENGTH];
    struct obj_data *obj_object;
    int keyword = 13;

    argument_interpreter(argument, arg1, arg2);
    if (*arg1) {
        obj_object = get_obj_in_list(arg1, ch->carrying);
        if (obj_object) {
            if (obj_object->obj_flags.type_flag == ITEM_LIGHT)
                wear(ch, obj_object, WEAR_LIGHT);
            else
                wear(ch, obj_object, 13);
        } else {
            sprintf(buffer, "You do not seem to have the '%s'.\n\r",arg1);
            send_to_char(buffer,ch);
        }
    } else {
        send_to_char("Hold what?\n\r", ch);
    }
}


void do_remove(struct char_data *ch, char *argument, int cmd)
{
    char arg1[MAX_STRING_LENGTH];
    char buffer[MAX_STRING_LENGTH];
    char buffer1[MAX_STRING_LENGTH];
    struct obj_data *obj_object;
    struct char_data *tmp_char;
    int i, j;

    one_argument(argument, arg1);

    if (*arg1) {
        if(strcmp(arg1,"all")==0) {
            for (j = 0; j < MAX_WEAR ; j++) {
                if (ch->equipment[j]) {
                    if (CAN_SEE_OBJ(ch, ch->equipment[j])) {
                        if (ch->equipment[j]->obj_flags.type_flag==ITEM_LIGHT &&
                            ch->equipment[j]->obj_flags.value[2] &&
                            world[ch->in_room].light > 0) {
                            world[ch->in_room].light--;
                        }
                        act("You stop using $p.",
                            FALSE,ch,ch->equipment[j],0,TO_CHAR);
                        act("$n stops using $p.",
                            TRUE,ch,ch->equipment[j],0,TO_ROOM);

                        obj_to_char(unequip_char(ch, j), ch);
                    }
                }
            }
            return;
        }
        obj_object = get_object_in_equip_vis(ch, arg1, ch->equipment, &j);
        if (obj_object) {
            if (CAN_CARRY_N(ch) != IS_CARRYING_N(ch)) {

                obj_to_char(unequip_char(ch, j), ch);

                if (obj_object->obj_flags.type_flag == ITEM_LIGHT)
                    if (obj_object->obj_flags.value[2])
                        if ( world[ch->in_room].light > 0)
                            world[ch->in_room].light--;

                act("You stop using $p.",FALSE,ch,obj_object,0,TO_CHAR);
                act("$n stops using $p.",TRUE,ch,obj_object,0,TO_ROOM);

            } else {
                send_to_char("You can't carry that many items.\n\r", ch);
            }
        } else {
            send_to_char("You are not using it.\n\r", ch);
        }
    } else {
        send_to_char("Remove what?\n\r", ch);
    }
}

