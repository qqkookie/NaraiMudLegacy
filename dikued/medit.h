/****************************************************************************
 *                                                                          *
 *           medit.h (c) by Georg Essl in 1992                              *
 *                                                                          *
 ****************************************************************************/

/*** PROTOTYPES ***/

int  me_FindNrMobile(int,char *);
int  me_FindNrRealMobile(int,char *);
void me_FindMobiles(char *);
int  me_GetRealNumber(int);
void me_InitMEdit(void);
void me_CloseMEdit(void);
void me_SetDBPath(char *);
void me_SetFile(char *);
void me_SetTemp(char *);

void me_CloneMobile(int,int);
void me_EditMobile(int);
void me_PurgeMobile(int);
void me_ShowMobile(int);

char *me_ReadLDesc(int);
char *me_ReadName(int);
