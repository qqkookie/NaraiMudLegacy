/****************************************************************************
 *                                                                          *
 *           iedit.h (c) by Georg Essl in 1992                              *
 *                                                                          *
 ****************************************************************************/

/*** PROTOTYPES ***/

void ie_FindItems(char *);
void ie_InitIEdit(void);
void ie_CloseIEdit(void);
int  ie_GetRealNumber(int);
int  ie_FindNrItem(int,char *);
int  ie_FindNrRealItem(int,char *);
void ie_ShowItem(int);
void ie_EditItem(int);
void ie_PurgeItem(int);
void ie_CloneItem(int,int);

void ie_SetFile(char *);
void ie_SetTemp(char *);
void ie_SetAffBitVec(int);
void ie_SetDBPath(char *);

char *ie_ReadLDesc(int);
char *ie_ReadSDesc(int);
char *ie_ReadName(int);
