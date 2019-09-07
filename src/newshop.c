/*
used goods shops
made by Process(wdshin@eve.kaist.ac.kr)
*/
#include "structs.h"
#include "newshop.h"

/* constants */

/* virtual number of room */
int used_shop_rooms[]=
{
	0, /* NOT USED */
};

int used_shop_storage_rooms[]=
{
	0, /* NOT USED */
};
int find_shop_index(int room_nr)
{
	int i,result;
	int flag=FALSE;

	for(i=0;used_shop_rooms[i];i++)
	{
		if(room_nr==used_shop_rooms[i])
		{
			flag=TRUE;
			result=i;
		}
	}
	if(flag)
		return result;
	else
		return 0;
}
int used_shop(struct char_data *ch,int cmd,char *arg)
{
	int shop_num;
	struct room_data *storage_room;
	struct obj_data *storage,*i;
	char buf[MAX_STRING_LENGTH],one_line[MAX_STRING_LENGTH];
	int no_of_items;
	struct obj_data *item_sorted_list,*tmp_obj,*min_obj;

	shop_num=find_shop_index(world[ch->in_room].number)
	if(shop_num==0)
	{
		log("ERROR in find_shop_index!");
		send_to_char("Ooops. bug???\n\r",ch);
		return TRUE;
	}

	storage_room=world[real_room(used_shop_storage_rooms[shop_num])];
	storage=storage_room.contents;

	for(i=storage;i;i->next_content)
	{
		for(j=i->next_content;j;j->next_content)
		{
		}
	}

	strcpy(buf,,"");
	if(cmd==59) /* list */
	{
	/* lists goods' name and value in the storage room */
		for(i=storage;i;i=i->next_content)
		{
		}
	}
	else if(cmd==56) /* buy */
	{
	/* get the goods from the storage room and give it to character */
	}
	else if(cmd==57) /* sell */
	{
	/* get the goods from the character and transfer it to storage room */
	}
	else if(cmd==299) /* view */
	{
	/* identify the goods' status */
	}
	return FALSE;
}
