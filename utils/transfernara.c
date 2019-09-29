/*
  remove player data below 20 level and unloged too long
*/
#include "structs.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

char namelist[2000][20];

main()
{
  FILE *from, *to;
  int i;
  static struct char_file_u temp;
  void get_player_from_old(),put_player_at_new();
  void remove_rentfile();

  from = fopen("players","r");
  to = fopen("trans.players","w");
  if(from==0 || to==0){
    printf("FILE OPEN ERROR\n");
    exit(1);
  }
  i=0;
  while(!feof(from)){
    get_player_from_old(from,&temp);
    if(temp.level<=20 && temp.last_logon<time(0)-(24*3600*7*temp.level)){
      remove_rentfile(&temp);
      continue;
    }
    strcpy(namelist[i],temp.name);
    i++;
    put_player_at_new(to,&temp);
  }
}

int there_is_name(struct char_file_u *temp)
{
    int i=0;

    while( *namelist[i] ){
        if( !strcmp( namelist[i], temp->name ) ) return 1;
        i++;
    }
    return 0;
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
