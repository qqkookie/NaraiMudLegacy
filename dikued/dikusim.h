/****************************************************************************
 *                                                                          *
 *         dikusim.h (c) by Georg Essl in 1993                              *
 *                                                                          *
 ****************************************************************************/

/*** PROTOTYPES ***/

void sim_DoLook(int);
void sim_LookMobile(int);
void sim_LookItem(int);
void sim_DoGoto(int);
void sim_DoExits(int);
int  sim_get_curr_room(void);
void sim_set_curr_room(int);
void sim_InitDikuSim(void);
int sim_GetExit(int,int);
void sim_LookDir(int);
