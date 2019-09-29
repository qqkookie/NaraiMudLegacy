/* ************************************************************************
*  file: modify.c                                         Part of DIKUMUD *
*  Usage: Run-time modification (by users) of game variables              *
*  Copyright (C) 1990, 1991 - see 'license.doc' for complete information. *
************************************************************************ */


#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <time.h>

#include "structs.h"
#include "utils.h"
#include "interpreter.h"
#include "handler.h"
#include "db.h"
#include "comm.h"
#include "memory.h"

#define TP_MOB    0
#define TP_OBJ     1
#define TP_ERROR  2


void show_string(struct descriptor_data *d, char *input);



char *string_fields[] =
{
    "name",
    "short",
    "long",
    "description",
    "title",
    "delete-description",
    "\n"
};




/* maximum length for text field x+1 */
int length[] =
{
    15,
    60,
    256,
    240,
    60
};




char *skill_fields[] = 
{
    "learned",
    "affected",
    "duration",
    "recognize",
    "\n"
};




int max_value[] =
{
    255,
    255,
    10000,
    1
};

/* ************************************************************************
*  modification of malloc'ed strings                                      *
************************************************************************ */

/* Add user input to the 'current' string (as defined by d->str) */
void string_add(struct descriptor_data *d, char *str)
{
    int terminator = 0, length = 0;
	char tmp_buf[MAX_STRING_LENGTH] ;

    /* determine if this is the terminal string, and truncate if so */
    if (terminator = (*str == '@')) {
        *str = '\0';
    }
  
    if (!(*d->str))
    {
        if (strlen(str) > d->max_str)
        {
            send_to_char("String too long - Truncated.\n\r",
                         d->character);
            *(str + d->max_str) = '\0';
            terminator = 1;
        }
        /*
          create(*d->str, char, strlen(str) + 3);
        */
        *d->str = malloc_string_type(strlen(str) + 3);
	
        strcpy(*d->str, str);
    }
    else
    {
        if (strlen(str) + strlen(*d->str) > d->max_str)
        {
            send_to_char("String too long. Last line skipped.\n\r",
                         d->character);
            terminator = 1;
        }
        else 
        {
            strcpy(tmp_buf, *d->str) ;
            length = strlen(*d->str) ;
            free_string_type(*d->str) ;
            *d->str = malloc_string_type( length+ strlen(str) + 3) ;
            /*
              if (!(*d->str = (char *) realloc(*d->str, strlen(*d->str) + 
              strlen(str) + 3)))
            */
            if ( *d->str == NULL) 
            {
                perror("string_add");
                exit(1);
            }
            strcpy(*d->str, tmp_buf);
            strcat(*d->str, str);
        }
    }

    if (terminator)
    {
        d->str = 0;
        if (d->connected == CON_EXDSCR)
        {
            SEND_TO_Q(MENU, d);
            d->connected = CON_SLCT;
        }
    }
    else
        strcat(*d->str, "\n\r");
}


#undef MAX_STR

/* interpret an argument for do_string */
void quad_arg(char *arg, int *type, char *name, int *field, char *string)
{
    char buf[MAX_STRING_LENGTH];
    int i;

    /* determine type */
    arg = one_argument(arg, buf);
    if (is_abbrev(buf, "char"))
        *type = TP_MOB;
    else if (is_abbrev(buf, "obj"))
        *type = TP_OBJ;
    else
    {
        *type = TP_ERROR;
        return;
    }

    /* find name */
    arg = one_argument(arg, name);

    /* field name and number */
    arg = one_argument(arg, buf);
    if (!(*field = old_search_block(buf, 0, strlen(buf), string_fields, 0)))
        return;

    /* string */
    for (; isspace(*arg); arg++);
    for (; *string = *arg; arg++, string++);

    return;
}
  
   


/* modification of malloc'ed strings in chars/objects */
void do_string(struct char_data *ch, char *arg, int cmd)
{
    char name[MAX_STRING_LENGTH], string[MAX_STRING_LENGTH];
    int field, type;
    struct char_data *mob;
    struct obj_data *obj;
    struct extra_descr_data *ed, *tmp;

    if (IS_NPC(ch))
        return;

    quad_arg(arg, &type, name, &field, string);

    if (type == TP_ERROR) {
        send_to_char(
            "Syntax: string ('obj'|'char') <name> <field> [<string>].\n\r",ch);
        return;
    }

    if (!field) {
        send_to_char("No field by that name. Try 'help string'.\n\r",
                     ch);
        return;
    }

    if (type == TP_MOB) {
        /* locate the beast */
        if (!(mob = get_char_vis(ch, name))) {
            send_to_char("I don't know anyone by that name...\n\r",
                         ch);
            return;
        }

        switch(field) {
        case 1:
            if (!IS_NPC(mob) && GET_LEVEL(ch) < (IMO+3)) {
                send_to_char("You can't change that field for players.", ch);
                return;
            }
            ch->desc->str = &GET_NAME(mob);
            if (!IS_NPC(mob))
                send_to_char(
                    "WARNING: You have changed the name of a player.\n\r", ch);
            break;
        case 2:
            if (!IS_NPC(mob)) {
                send_to_char(
                    "That field is for monsters only.\n\r", ch);
                return;
            }
            ch->desc->str = &mob->player.short_descr;
            break;
        case 3:
            if (!IS_NPC(mob)) {
                send_to_char(
                    "That field is for monsters only.\n\r", ch);
                return;
            }
            ch->desc->str = &mob->player.long_descr;
            break;
        case 4:ch->desc->str = &mob->player.description; break;
        case 5:
            if (IS_NPC(mob)) {
                send_to_char("Monsters have no titles.\n\r", ch);
                return;
            }
            ch->desc->str = &mob->player.title;
            break;
        default:
            send_to_char(
                "That field is undefined for monsters.\n\r", ch);
            return;
            break;
        }
    } else {  /* type == TP_OBJ */
        /* locate the object */
        if (!(obj = get_obj_vis(ch, name))) {
            send_to_char("Can't find such a thing here..\n\r", ch);
            return;
        }
        switch(field) {
        case 1: ch->desc->str = &obj->name; break;
        case 2: ch->desc->str = &obj->short_description; break;
        case 3: ch->desc->str = &obj->description; break;
        case 4:
            if (!*string) {
                send_to_char("You have to supply a keyword.\n\r", ch);
                return;
            }
            /* try to locate extra description */
            for (ed = obj->ex_description; ; ed = ed->next)
                if (!ed) /* the field was not found. create a new one. */
                {
                    /*
                      create(ed , struct extra_descr_data, 1);
                    */
                    ed = (struct extra_descr_data *) malloc_general_type(
                        MEMORY_EXTRA_DESCR_DATA) ;
                    ed->next = obj->ex_description;
                    obj->ex_description = ed;
                    /*
                      create(ed->keyword, char, strlen(string) + 1);
                    */
                    ed->keyword = malloc_string_type(strlen(string) + 1) ;
                    strcpy(ed->keyword, string);
                    ed->description = 0;
                    ch->desc->str = &ed->description;
                    send_to_char("New field.\n\r", ch);
                    break;
                }
                else if (!str_cmp(ed->keyword, string)) /* the field exists */
                {
                    free_string_type(ed->description);
                    ed->description = 0;
                    ch->desc->str = &ed->description;
                    send_to_char(
                        "Modifying description.\n\r", ch);
                    break;
                }
            ch->desc->max_str = MAX_STRING_LENGTH ;
            return; /* the stndrd (see below) procedure does not apply here */
            break;
        case 6: 
            if (!*string) {
                send_to_char("You must supply a field name.\n\r", ch);
                return;
            }
            /* try to locate field */
            for (ed = obj->ex_description; ; ed = ed->next)
                if (!ed) {
                    send_to_char("No field with that keyword.\n\r", ch);
                    return;
                }
                else if (!str_cmp(ed->keyword, string))
                {
                    free_string_type(ed->keyword);
                    if (ed->description)
                        free_string_type(ed->description);
            
                    /* delete the entry in the desr list */            
                    if (ed == obj->ex_description)
                        obj->ex_description = ed->next;
                    else
                    {
                        for(tmp = obj->ex_description; tmp->next != ed; 
                            tmp = tmp->next);
                        tmp->next = ed->next;
                    }
                    free_general_type((char *)ed, MEMORY_EXTRA_DESCR_DATA) ;

                    send_to_char("Field deleted.\n\r", ch);
                    return;
                }
            break;        
        default:
            send_to_char(
                "That field is undefined for objects.\n\r", ch);
            return;
            break;
        }
    }

    if (*ch->desc->str)
    {
        free_string_type(*ch->desc->str);
    }

    if (*string)   /* there was a string in the argument array */
    {
        if (strlen(string) > length[field - 1])
        {
            send_to_char("String too long - truncated.\n\r", ch);
            *(string + length[field - 1]) = '\0';
        }
        /*
          create(*ch->desc->str, char, strlen(string) + 1);
        */
		*ch->desc->str = malloc_string_type(strlen(string) + 1) ;
        strcpy(*ch->desc->str, string);
        ch->desc->str = 0;
        send_to_char("Ok.\n\r", ch);
    }
    else          /* there was no string. enter string mode */
    {
        send_to_char("Enter string. terminate with '@'.\n\r", ch);
        *ch->desc->str = 0;
        ch->desc->max_str = length[field - 1];
    }
}
      
/* db stuff *********************************************** */


/* One_Word is like one_argument, execpt that words in quotes "" are */
/* regarded as ONE word                                              */

char *one_word(char *argument, char *first_arg )
{
    int found, begin, look_at;

    found = begin = 0;

    do
    {
        for ( ;isspace(*(argument + begin)) && !is_hangul(*(argument+begin)) ;
              begin++);

        if (*(argument+begin) == '\"') {  /* is it a quote */

            begin++;

            for (look_at=0; (*(argument+begin+look_at) >= ' ' || 
                             is_hangul(*(argument+begin+look_at))) && 
                     (*(argument+begin+look_at) != '\"') ; look_at++)
                *(first_arg + look_at) = LOWER(*(argument + begin + look_at));

            if (*(argument+begin+look_at) == '\"')
                begin++;

        } else {

            for (look_at=0; (*(argument+begin+look_at) > ' ' ||
                             is_hangul(*(argument+begin+look_at)))  ; look_at++)
                *(first_arg + look_at) = LOWER(*(argument + begin + look_at));

        }

        *(first_arg + look_at) = '\0';
        begin += look_at;
    }
    while (fill_word(first_arg));

    return(argument+begin);
}


struct help_index_element *build_help_index(FILE *fl, int *num)
{
    int nr = -1, issorted, i;
    struct help_index_element *list = 0, mem;
    char buf[81], tmp[81], *scan;
    long pos;

    for (;;)
    {
        pos = ftell(fl);
        fgets(buf, 81, fl);
        *(buf + strlen(buf) - 1) = '\0';
        scan = buf;
        for (;;)
        {
            /* extract the keywords */
            scan = one_word(scan, tmp);

            if (!*tmp)
                break;

            if (!list)
            {
                CREATE(list, struct help_index_element, 1);
                nr = 0;
            }
            else
                RECREATE(list, struct help_index_element, ++nr + 1);

            list[nr].pos = pos;
            /*
              create(list[nr].keyword, char, strlen(tmp) + 1);
            */
            list[nr].keyword = malloc_string_type(strlen(tmp) + 1) ;
            strcpy(list[nr].keyword, tmp);
        }
        /* skip the text */
        do
            fgets(buf, 81, fl);
        while (*buf != '#');
        if (*(buf + 1) == '~')
            break;
    }
    /* we might as well sort the stuff */
    do
    {
        issorted = 1;
        for (i = 0; i < nr; i++)
            if (str_cmp(list[i].keyword, list[i + 1].keyword) > 0)
            {
                mem = list[i];
                list[i] = list[i + 1];
                list[i + 1] = mem;
                issorted = 0;
            }
    }
    while (!issorted);

    *num = nr;
    return(list);
}



void page_string(struct descriptor_data *d, char *str, int keep_internal)
{
    if (!d)
        return;

    if (keep_internal) {
        /*
          create(d->showstr_head, char, strlen(str) + 1);
		*/
		d->showstr_head = malloc_string_type(strlen(str) + 1) ;
        strcpy(d->showstr_head, str);
        d->showstr_point = d->showstr_head;
    }
    else
        d->showstr_point = str;

    show_string(d, "");
}

void show_string(struct descriptor_data *d, char *input)
{
    char buffer[MAX_STRING_LENGTH], buf[MAX_INPUT_LENGTH];
    register char *scan, *chk;
    int lines = 0, toggle = 1;

    one_argument(input, buf);

    if (*buf) {
        if (d->showstr_head) {
            free_string_type(d->showstr_head);
            d->showstr_head = 0;
        }
        d->showstr_point = 0;
        return;
    }

    /* show a chunk */
    for (scan = buffer;; scan++, d->showstr_point++)
        if((((*scan = *d->showstr_point) == '\n') || (*scan == '\r')) &&
           ((toggle = -toggle) < 0))
            lines++;
        else if (!*scan || (lines >= 22)) {
            *scan = '\0';
            SEND_TO_Q(buffer, d);

            /* see if this is the end (or near the end) of the string */
            for (chk = d->showstr_point; isspace(*chk); chk++);
            if (!*chk) {
                if (d->showstr_head) {
                    free_string_type(d->showstr_head);
                    d->showstr_head = 0;
                }
                d->showstr_point = 0;
            }
            return;
        }
}

