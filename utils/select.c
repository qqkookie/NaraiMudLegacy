#include "structs.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

main()
{
  FILE *from, *to;
  int i;
  static struct char_file_u temp;
  void get_player_from_old(),put_player_at_new();
  void remove_rentfile();

  from = fopen("players","r");
  while(!feof(from)){
    get_player_from_old(from,&temp);
    if(temp.abilities.str > 18 || temp.level > 44 )
      printf("%d %s\n",temp.abilities.str,temp.name );
  }
}

void remove_rentfile(struct char_file_u *temp)
{
  char buf[100], name[20], *tmp;
  strcpy(name,temp->name);
  tmp=name;
  while(*tmp){
    if(isupper(*tmp)) *tmp=tolower(*tmp);
    tmp++;
  }
  sprintf(buf,"rm stash/%c/%s.x",*name,name);
  system(buf);
  sprintf(buf,"rm stash/%c/%s.x.y",*name,name);
  system(buf);
}

void get_player_from_old(FILE *fp,struct char_file_u *temp)
{
  fread(temp,sizeof(struct char_file_u),1,fp);
}

void put_player_at_new(FILE *fp,struct char_file_u *temp)
{
  fwrite(temp,sizeof(struct char_file_u),1,fp);
}
