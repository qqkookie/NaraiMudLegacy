/****************************************************************************
 *                                                                          *
 *           redit.h by Kenneth Holmlund                                    *
 *           With much help from Georg Essl...                              *
 *                                                                          *
 ****************************************************************************/

/*** PROTOTYPES ***/

int  re_FindNrRoom(int,char *);
void re_FindRooms(char *);
void re_InitREdit(void);
void re_CloseREdit(void);
void re_SetDBPath(char *);
int re_GetRealNumber(int);

void re_SetFile(char *);
void re_SetTemp(char *);

void re_CloneRoom(int,int);
void re_EditRoom(int);
void re_PurgeRoom(int);
void re_ShowRoom(int);

int  re_RoomZone(int);
