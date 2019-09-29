/*****************************************************************************
 *                                                                            *
 * Project : Diku-Mud-World-Creator                                           *
 *                                                                            *
 * Program : Diku-Mud-Style Command-Line Interface                            *
 * Version : 1.0                                                              *
 * Last changed on: 27.7.93                                                   *
 *                                                                            *
 * Code by: Georg Essl (essl@fstgds06.tu-graz.ac.at)                          *
 *                                                                            *
 *****************************************************************************/

/*** INCLUDES ***/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "medit.h"
#include "iedit.h"
#include "redit.h"
#include "zedit.h"
#include "dikusim.h"
#include "dutil.h"

/*** DEFINES ***/
#define DC_HELPFILE "dikucli.hlp"
#define DC_CONFFILE "iconfig.def"

#define DC_PROMPT "> "
#define DC_MAXSTRLEN 255

#define DC_CONFVARS 11

#define DC_NR_ACMDS 0
#define DC_NR_BCMDS 0
#define DC_NR_CCMDS 3
#define DC_NR_DCMDS 2
#define DC_NR_ECMDS 2
#define DC_NR_FCMDS 0
#define DC_NR_GCMDS 2
#define DC_NR_HCMDS 1
#define DC_NR_ICMDS 7
#define DC_NR_JCMDS 0
#define DC_NR_KCMDS 0
#define DC_NR_LCMDS 2
#define DC_NR_MCMDS 7
#define DC_NR_NCMDS 1
#define DC_NR_OCMDS 0
#define DC_NR_PCMDS 2
#define DC_NR_QCMDS 1
#define DC_NR_RCMDS 7
#define DC_NR_SCMDS 1
#define DC_NR_TCMDS 0
#define DC_NR_UCMDS 1
#define DC_NR_VCMDS 1
#define DC_NR_WCMDS 2
#define DC_NR_XCMDS 0
#define DC_NR_YCMDS 0
#define DC_NR_ZCMDS 7

#define DIR_NORTH   0
#define DIR_EAST    1
#define DIR_SOUTH   2
#define DIR_WEST    3
#define DIR_UP      4
#define DIR_DOWN    5

/*** STRUCTS ***/

#if !defined(__osf__) && !defined(m88k) && !defined(__386BSD__)
typedef struct command
#else
struct command
#endif
{
    char *cmd;
    char *help;
    void (*funct)(char *);
};

#if !defined(__osf__) && !defined(m88k) && !defined(__386BSD__)
typedef struct hashentry
#else
struct hashentry
#endif
{
    int nr;
    struct command *cmds;
};

/*** PROTOTYPES ***/

void dc_NotFound(void);

/* FOR CONFIG FUNCTIONS */

void dc_CMudName(char *);
void dc_CItemFile(char *);
void dc_CIETempF(char *);
void dc_CMobileFile(char *);
void dc_CMETempF(char *);
void dc_CRoomFile(char *);
void dc_CRETempF(char *);
void dc_CZoneFile(char *);
void dc_CZETempF(char *);
void dc_CDataBPath(char *);
void dc_CAffBitVector(char *);

/* FOR CMD FUNCTIONS */

void dc_commands(char *);
void dc_credits(char *);

void dc_down(char *);
void dc_doorstate(char *);

void dc_east(char *);
void dc_exits(char *);

void dc_give(char *);
void dc_goto(char *);

void dc_help(char *);

void dc_iclone(char *);
void dc_icreate(char *);
void dc_iedit(char *);
void dc_ifind(char *);
void dc_ilist(char *);
void dc_ipurge(char *);
void dc_ishow(char *);

void dc_load(char *);
void dc_look(char *);

void dc_mclone(char *);
void dc_mcreate(char *);
void dc_medit(char *);
void dc_mfind(char *);
void dc_mlist(char *);
void dc_mpurge(char *);
void dc_mshow(char *);

void dc_north(char *);

void dc_rclone(char *);
void dc_rcreate(char *);
void dc_redit(char *);
void dc_rfind(char *);
void dc_rlist(char *);
void dc_rpurge(char *);
void dc_rshow(char *);

void dc_purge(char *);
void dc_put(char *);

void dc_quit(char *);

void dc_south(char *);

void dc_up(char *);

void dc_version(char *);

void dc_west(char *);
void dc_wear(char *);

void dc_zclone(char *);
void dc_zcreate(char *);
void dc_zedit(char *);
void dc_zfind(char *);
void dc_zlist(char *);
void dc_zpurge(char *);
void dc_zshow(char *);

/*** GLOBAL VARS ***/

int dc_quitflag;
char lastcommand[DC_MAXSTRLEN];

char MudName[DC_MAXSTRLEN]="";

/***** EXTERNS *****/

extern int zw_max;
extern char **zw_str;
extern int zd_max;
extern char **zd_str;
extern struct zone_data *ze_activezone;

/*** COMMAND CONSTANTS ***/

struct command dc_ccmds[DC_NR_CCMDS]=
{
    {"cmds",NULL,dc_commands},
    {"commands",NULL,dc_commands},
    {"credits",NULL,dc_credits}
};
struct command dc_dcmds[DC_NR_DCMDS]=
{
    {"down",NULL,dc_down},
    {"doorstate",NULL,dc_doorstate}
};
struct command dc_ecmds[DC_NR_ECMDS]=
{
    {"east",NULL,dc_east},
    {"exits",NULL,dc_exits}
};
struct command dc_gcmds[DC_NR_GCMDS]=
{
    {"give",NULL,dc_give},
    {"goto",NULL,dc_goto}
};
struct command dc_hcmds[DC_NR_HCMDS]=
{
    {"help",NULL,dc_help}
};
struct command dc_icmds[DC_NR_ICMDS]=
{
    {"iclone",NULL,dc_iclone},
    {"icreate",NULL,dc_icreate},
    {"iedit",NULL,dc_iedit},
    {"ifind",NULL,dc_ifind},
    {"ilist",NULL,dc_ilist},
    {"ipurge",NULL,dc_ipurge},
    {"ishow",NULL,dc_ishow}
};
struct command dc_lcmds[DC_NR_LCMDS]=
{
    {"look",NULL,dc_look},
    {"load",NULL,dc_load}
};
struct command dc_mcmds[DC_NR_MCMDS]=
{
    {"mclone",NULL,dc_mclone},
    {"mcreate",NULL,dc_mcreate},
    {"medit",NULL,dc_medit},
    {"mfind",NULL,dc_mfind},
    {"mlist",NULL,dc_mlist},
    {"mpurge",NULL,dc_mpurge},
    {"mshow",NULL,dc_mshow}
};
struct command dc_ncmds[DC_NR_NCMDS]=
{
    {"north",NULL,dc_north}
};
struct command dc_pcmds[DC_NR_PCMDS]=
{
    {"purge",NULL,dc_purge},
    {"put",NULL,dc_put}
};
struct command dc_qcmds[DC_NR_QCMDS]=
{
    {"quit",NULL,dc_quit}
};
struct command dc_rcmds[DC_NR_RCMDS]=
{
    {"rclone",NULL,dc_rclone},
    {"rcreate",NULL,dc_rcreate},
    {"redit",NULL,dc_redit},
    {"rfind",NULL,dc_rfind},
    {"rlist",NULL,dc_rlist},
    {"rpurge",NULL,dc_rpurge},
    {"rshow",NULL,dc_rshow}
};
struct command dc_scmds[DC_NR_SCMDS]=
{
    {"south",NULL,dc_south}
};
struct command dc_vcmds[DC_NR_VCMDS]=
{
    {"version",NULL,dc_version}
};
struct command dc_ucmds[DC_NR_UCMDS]=
{
    {"up",NULL,dc_up}
};
struct command dc_wcmds[DC_NR_WCMDS]=
{
    {"west",NULL,dc_west},
    {"wear",NULL,dc_wear}
};
struct command dc_zcmds[DC_NR_ZCMDS]=
{
    {"zclone",NULL,dc_zclone},
    {"zcreate",NULL,dc_zcreate},
    {"zedit",NULL,dc_zedit},
    {"zfind",NULL,dc_zfind},
    {"zlist",NULL,dc_zlist},
    {"zpurge",NULL,dc_zpurge},
    {"zshow",NULL,dc_zshow}
};

struct hashentry dc_cmdhash[26] =
{
    {DC_NR_ACMDS,NULL},
    {DC_NR_BCMDS,NULL},
    {DC_NR_CCMDS,dc_ccmds},
    {DC_NR_DCMDS,dc_dcmds},
    {DC_NR_ECMDS,dc_ecmds},
    {DC_NR_FCMDS,NULL},
    {DC_NR_GCMDS,dc_gcmds},
    {DC_NR_HCMDS,dc_hcmds},
    {DC_NR_ICMDS,dc_icmds},
    {DC_NR_JCMDS,NULL},
    {DC_NR_KCMDS,NULL},
    {DC_NR_LCMDS,dc_lcmds},
    {DC_NR_MCMDS,dc_mcmds},
    {DC_NR_NCMDS,dc_ncmds},
    {DC_NR_OCMDS,NULL},
    {DC_NR_PCMDS,dc_pcmds},
    {DC_NR_QCMDS,dc_qcmds},
    {DC_NR_RCMDS,dc_rcmds},
    {DC_NR_SCMDS,dc_scmds},
    {DC_NR_TCMDS,NULL},
    {DC_NR_UCMDS,dc_ucmds},
    {DC_NR_VCMDS,dc_vcmds},
    {DC_NR_WCMDS,dc_wcmds},
    {DC_NR_XCMDS,NULL},
    {DC_NR_YCMDS,NULL},
    {DC_NR_ZCMDS,dc_zcmds}
};

char ConfigVars[DC_CONFVARS][40] =
{
    "MudName",
    "ItemFile",
    "ItemTempFile",
    "MobileFile",
    "MobileTempFile",
    "RoomFile",
    "RoomTempFile",
    "ZoneFile",
    "ZoneTempFile",
    "DataBasePath",
    "AffBitVector"
};

void (*dc_conf[DC_CONFVARS])(char *) =
{
    dc_CMudName,
    dc_CItemFile,
    dc_CIETempF,
    dc_CMobileFile,
    dc_CMETempF,
    dc_CRoomFile,
    dc_CRETempF,
    dc_CZoneFile,
    dc_CZETempF,
    dc_CDataBPath,
    dc_CAffBitVector
};

/*** FUNCTIONS ***/

void dc_commands(char *wstr)
{
    int i,i2;
    for(i=0;i<26;i++)
    {
        for(i2=0;i2<dc_cmdhash[i].nr;i2++)
        {
            printf("%s  ",dc_cmdhash[i].cmds[i2].cmd);
        }
    }
    printf("\n");
}

void dc_credits(char *wstr)
{
    printf("------ CREDITS ------ \n");
    printf("This Program was coded by:\n");
    printf("Concept and Idea, DikuCli, DikuSim, IEdit, MEdit and ZEdit by:\n");
    printf("  GEORG ESSL (essl@fstgds06.tu-graz.ac.at)\n\n");
    printf("Modifications and REdit by:\n");
    printf("  KENNETH HOLMLUND (holmlund@tp.umu.se)\n");
}

void dc_exits(char *wstr)
{
    int wint,wint2;
    char wstr2[DC_MAXSTRLEN];

    wstr=strtok(wstr," ");
    if(wstr=='\0')
    {
        sim_DoExits(re_GetRealNumber(sim_get_curr_room()));
    }
    else
        printf("*** Syntax: EXITS\n");
}

void dc_give(char *wstr)
{
    int wint,wint2;

    int objnr;
    int mobnr;

    char wstr2[DC_MAXSTRLEN];
    char *wstr3;
    char *wstr4;

    wstr3=strtok(wstr," ");
    wstr4=strtok(NULL," ");

    if(wstr3!='\0')
    {
        wint=atoi(wstr3);
        if(((strcspn(wstr3,"."))==strlen(wstr3))&&((wint>0)||((wint==0)&&(!strcmp(wstr3,"0")))))
        {
            if(ie_GetRealNumber(wint)>=0)
            {
                objnr=wint;
            }
            else
            {
                dc_NotFound();
                return;
            }
        }
        else
        {
            if((wint=strcspn(wstr3,"."))==strlen(wstr3))
            {
                if((wint2=ie_FindNrRealItem(1,wstr3))>=0)
                {
                    objnr=wint2;
                }
                else
                {
                    dc_NotFound();
                    return;
                }
            }
            else
            {
                strncpy(wstr2,wstr3,wint);
                wstr2[wint]='\0';
                wint2=atoi(wstr2);
                wstr3+=wint+1;
                if((wint=ie_FindNrRealItem(wint2,wstr3))>=0)
                {
                    objnr=wint;
                }
                else
                {
                    dc_NotFound();
                    return;
                }
            }
        }


        if(wstr4!='\0')
        {
            wint=atoi(wstr4);
            if(((strcspn(wstr4,"."))==strlen(wstr4))&&((wint>0)||((wint==0)&&(!strcmp(wstr4,"0")))))
            {
                if((wint2=ze_FindNrMobRoom(1,wint,ze_getroom(ze_activezone,sim_get_curr_room())))>=0)
                    ze_GiveItemMob(objnr,wint2);
                else
                    dc_NotFound();
            }
            else
            {
                if((wint=strcspn(wstr4,"."))==strlen(wstr4))
                {
                    if((wint2=ze_FindMobRoom(1,wstr4,ze_getroom(ze_activezone,sim_get_curr_room())))>=0)
                        ze_GiveItemMob(objnr,wint2);
                    else
                        dc_NotFound();
                }
                else
                {
                    strncpy(wstr2,wstr4,wint);
                    wstr2[wint]='\0';
                    wint2=atoi(wstr2);
                    wstr4+=wint+1;
                    if((wint=ze_FindMobRoom(wint2,wstr4,ze_getroom(ze_activezone,sim_get_curr_room())))>=0)
                        ze_GiveItemMob(objnr,wint);
                    else
                        dc_NotFound();
                }
            }

        }
        else
        {
            printf("*** Syntax: GIVE <ObjNr|[Nr.]Pattern> <MobNr|[Nr.]Pattern>\n");
        }
    }
    else
    {
        printf("*** Syntax: GIVE <ObjNr|[Nr.]Pattern> <MobNr|[Nr.]Pattern>\n");
    }
}


void dc_goto(char *wstr)
{
    int wint,wint2;
    char wstr2[DC_MAXSTRLEN];

    wstr=strtok(wstr," ");
    if(wstr=='\0')
        printf("*** Syntax: GOTO <RoomNumber>  or  GOTO <[Nr.]Pattern>\n");
    else
    {
        wint=atoi(wstr);
        if(((strcspn(wstr,"."))==strlen(wstr))&&((wint>0)||((wint==0)&&(!strcmp(wstr,"0")))))
        {
            if((wint2=re_GetRealNumber(wint))>=0)
                sim_DoGoto(wint2);
            else
                dc_NotFound();
        }
        else
        {
            if((wint=strcspn(wstr,"."))==strlen(wstr))
            {
                if((wint2=re_FindNrRoom(1,wstr))>=0)
                    sim_DoGoto(wint2);
                else
                    dc_NotFound();
            }
            else
            {
                strncpy(wstr2,wstr,wint);
                wstr2[wint]='\0';
                wint2=atoi(wstr2);
                wstr+=wint+1;
                if((wint=re_FindNrRoom(wint2,wstr))>=0)
                    sim_DoGoto(wint);
                else
                    dc_NotFound();
            }
        }
    }
}

void dc_help(char *wstr)
{
    char arg[DC_MAXSTRLEN];
    char wchr;
    int i,i2;

    if(sscanf(wstr,"%s",arg)==1)
    {
        wchr=toupper(*arg);
        if((wchr>='A')&&(wchr<='Z'))
        {
            for(i=0;i<dc_cmdhash[wchr-'A'].nr;i++)
            {
                if(!strnccmp(arg,dc_cmdhash[wchr-'A'].cmds[i].cmd,strlen(arg)))
                {
                    printf("%s",dc_cmdhash[wchr-'A'].cmds[i].help);
                    break;
                }
            }
            if(i==dc_cmdhash[wchr-'A'].nr)
                printf("No help for that!!!\n");
        }
    }
    else
    {
        printf("There is HELP available about:\n\n");
        printf("Information:  HELP  CREDITS  VERSION  CMDS\n");
        printf("Item-Editor:  ICLONE  ICREATE  IEDIT  IFIND  ILIST  IPURGE  ISHOW\n");
        printf("Mob-Editor :  MCLONE  MCREATE  MEDIT  MFIND  MLIST  MPURGE  MSHOW\n");
        printf("Room-Editor:  RCLONE  RCREATE  REDIT  RFIND  RLIST  RPURGE  RSHOW\n");
        printf("Zone-Editor:  ZCLONE  ZCREATE  ZEDIT  ZFIND  ZLIST  ZPURGE  ZSHOW\n");
        printf("Simulator  :  EXITS  GIVE  GOTO  LOAD  LOOK  PURGE  PUT  WEAR\n");
        printf("              DOORSTATE  NORTH  EAST  SOUTH  WEST  UP  DOWN\n");
        printf("Other:        QUIT\n");
    }


}

void dc_iclone(char *wstr)
{
    char *wstr3;
    char *wstr4;
    char wstr2[DC_MAXSTRLEN];
    int wint,wint2,wint3;

    wstr3=strtok(wstr," ");
    wstr4=strtok(NULL," ");
    if(wstr4!=NULL)
        wint3=atoi(wstr4);
    else
        wint3=0;
    if((wstr3==NULL)||(wstr4==NULL)||((wint3==0)&&(strcmp(wstr4,"0")))||(wstr3=='\0')||(wstr4=='\0'))
        printf("*** Syntax: ICLONE <ItemNr> <ItemNr> or ICLONE <[Nr.]Pattern> <ItemNr>\n");
    else
    {
        wint=atoi(wstr3);
        if(((strcspn(wstr3,"."))==strlen(wstr3))&&((wint>0)||((wint==0)&&(!strcmp(wstr3,"0")))))
        {
            if((wint2=ie_GetRealNumber(wint))>=0)
                ie_CloneItem(wint2,wint3);
            else
                dc_NotFound();
        }
        else
        {
            if((wint=strcspn(wstr3,"."))==strlen(wstr3))
            {
                if((wint2=ie_FindNrItem(1,wstr3))>=0)
                    ie_CloneItem(wint2,wint3);
                else
                    dc_NotFound();
            }
            else
            {
                strncpy(wstr2,wstr3,wint);
                wstr2[wint]='\0';
                wint2=atoi(wstr2);
                wstr3+=wint+1;
                if((wint=ie_FindNrItem(wint2,wstr3))>=0)
                    ie_CloneItem(wint,wint3);
                else
                    dc_NotFound();
            }
        }
    }
}

void dc_icreate(char *wstr)
{
    int wint;

    wstr=strtok(wstr," ");
    if(wstr!=NULL)
        wint=atoi(wstr);
    else
        wint=0;

    if(wstr!=NULL && (strcspn(wstr,".")==strlen(wstr))&&((wint>0)||((wint==0)&&(!strcmp(wstr,"0")))))
    {
        ie_CloneItem(-1,wint);
    }
    else
        printf("*** Syntax: ICREATE <ItemNumber>\n");
}

void dc_iedit(char *wstr)
{
    int wint,wint2;
    char wstr2[DC_MAXSTRLEN];

    wstr=strtok(wstr," ");
    if(wstr=='\0')
        printf("*** Syntax: IEDIT <ItemNumber>  or  IEDIT <[Nr.]Pattern>\n");
    else
    {
        wint=atoi(wstr);
        if(((strcspn(wstr,"."))==strlen(wstr))&&((wint>0)||((wint==0)&&(!strcmp(wstr,"0")))))
        {
            if((wint2=ie_GetRealNumber(wint))>=0)
                ie_EditItem(wint2);
            else
            {
                ie_CloneItem(-1,wint);
                if((wint2=ie_GetRealNumber(wint))>=0)
                    ie_EditItem(wint2);
            }
        }
        else
        {
            if((wint=strcspn(wstr,"."))==strlen(wstr))
            {
                if((wint2=ie_FindNrItem(1,wstr))>=0)
                    ie_EditItem(wint2);
                else
                    dc_NotFound();
            }
            else
            {
                strncpy(wstr2,wstr,wint);
                wstr2[wint]='\0';
                wint2=atoi(wstr2);
                wstr+=wint+1;
                if((wint=ie_FindNrItem(wint2,wstr))>=0)
                    ie_EditItem(wint);
                else
                    dc_NotFound();
            }
        }

    }
}

void dc_ifind(char *wstr)
{
    wstr=strtok(wstr," ");
    if(wstr!='\0')
        ie_FindItems(wstr);
    else
        printf("*** Syntax: IFIND <SearchPattern>\n");
}

void dc_ilist(char *wstr)
{
    char tmp[] = "*";
    dc_ifind(tmp);
}

void dc_ipurge(char *wstr)
{
    int wint,wint2;

    if (!*wstr) {
        printf("*** Syntax: IPURGE <ItemNumber>\n");
        return;
    }

    wstr=strtok(wstr," ");
    wint=atoi(wstr);
    if((strcspn(wstr,".")==strlen(wstr))&&((wint>0)||((wint==0)&&(!strcmp(wstr,"0")))))
    {
        if((wint2=ie_GetRealNumber(wint))>=0)
            ie_PurgeItem(wint2);
        else
            dc_NotFound();
    }
    else
        printf("*** Syntax: IPURGE <ItemNumber>\n");
}

void dc_ishow(char *wstr)
{
    int wint,wint2;
    char wstr2[DC_MAXSTRLEN];

    wstr=strtok(wstr," ");
    if(wstr=='\0')
        printf("*** Syntax: ISHOW <ItemNr|[Nr.]Pattern>\n");
    else
    {
        wint=atoi(wstr);
        if(((strcspn(wstr,"."))==strlen(wstr))&&((wint>0)||((wint==0)&&(!strcmp(wstr,"0")))))
        {
            if((wint2=ie_GetRealNumber(wint))>=0)
                ie_ShowItem(wint2);
            else
                dc_NotFound();
        }
        else
        {
            if((wint=strcspn(wstr,"."))==strlen(wstr))
            {
                if((wint2=ie_FindNrItem(1,wstr))>=0)
                    ie_ShowItem(wint2);
                else
                    dc_NotFound();
            }
            else
            {
                strncpy(wstr2,wstr,wint);
                wstr2[wint]='\0';
                wint2=atoi(wstr2);
                wstr+=wint+1;
                if((wint=ie_FindNrItem(wint2,wstr))>=0)
                    ie_ShowItem(wint);
                else
                    dc_NotFound();
            }
        }
    }
}

void dc_load(char *wstr)
{
    int wint,wint2;
    char wstr2[DC_MAXSTRLEN];
    char *wstr3;
    char *wstr4;

    wstr3=strtok(wstr," ");
    if(wstr3!='\0')
    {
        switch(toupper(*wstr3))
        {
        case 'M':
            wstr4=strtok(NULL," ");
            if(wstr4!='\0')
            {
                wint=atoi(wstr4);
                if(((strcspn(wstr4,"."))==strlen(wstr4))&&((wint>0)||((wint==0)&&(!strcmp(wstr4,"0")))))
                {
                    if(me_GetRealNumber(wint)>=0)
                        ze_LoadMobile(wint);
                    else
                        dc_NotFound();
                }
                else
                {
                    if((wint=strcspn(wstr4,"."))==strlen(wstr4))
                    {
                        if((wint2=me_FindNrRealMobile(1,wstr4))>=0)
                            ze_LoadMobile(wint2);
                        else
                            dc_NotFound();
                    }
                    else
                    {
                        strncpy(wstr2,wstr4,wint);
                        wstr2[wint]='\0';
                        wint2=atoi(wstr2);
                        wstr4+=wint+1;
                        if((wint=me_FindNrRealMobile(wint2,wstr4))>=0)
                            ze_LoadMobile(wint);
                        else
                            dc_NotFound();
                    }
                }

            }
            break;
        case 'O':
            wstr4=strtok(NULL," ");
            if(wstr4!='\0')
            {
                wint=atoi(wstr4);
                if(((strcspn(wstr4,"."))==strlen(wstr4))&&((wint>0)||((wint==0)&&(!strcmp(wstr4,"0")))))
                {
                    if(ie_GetRealNumber(wint)>=0)
                        ze_LoadItem(wint);
                    else
                        dc_NotFound();
                }
                else
                {
                    if((wint=strcspn(wstr4,"."))==strlen(wstr4))
                    {
                        if((wint2=ie_FindNrRealItem(1,wstr4))>=0)
                            ze_LoadItem(wint2);
                        else
                            dc_NotFound();
                    }
                    else
                    {
                        strncpy(wstr2,wstr4,wint);
                        wstr2[wint]='\0';
                        wint2=atoi(wstr2);
                        wstr4+=wint+1;
                        if((wint=ie_FindNrRealItem(wint2,wstr4))>=0)
                            ze_LoadItem(wint);
                        else
                            dc_NotFound();
                    }
                }

            }
            break;
        default:
            printf("*** Syntax: LOAD MOB <MobNr|[Nr.]Pattern>\n");
            printf("            LOAD OBJ <ObjNr|[Nr.]Pattern>\n");
            break;
        }

    }
    else
    {
        printf("*** Syntax: LOAD MOB <MobNr|[Nr.]Pattern>\n");
        printf("            LOAD OBJ <ObjNr|[Nr.]Pattern>\n");
    }

}

void dc_look(char *wstr)
{
    int wint,wint2;
    char wstr2[DC_MAXSTRLEN];
    char *wstr4;

    wstr=strtok(wstr," ");
    if(wstr=='\0')
    {
        sim_DoLook(re_GetRealNumber(sim_get_curr_room()));
    }
    else
    {
        switch(toupper(*wstr))
        {
        case 'A':
            wstr4=strtok(NULL," ");
            if(wstr4!='\0')
            {
                wint=atoi(wstr4);
                if(((strcspn(wstr4,"."))==strlen(wstr4))&&((wint>0)||((wint==0)&&(!strcmp(wstr4,"0")))))
                {
                    if(ze_FindNrMobRoom(1,wint,ze_getroom(ze_activezone,sim_get_curr_room()))>=0)
                        sim_LookMobile(wint);
                    else
                        dc_NotFound();
                }
                else
                {
                    if((wint=strcspn(wstr4,"."))==strlen(wstr4))
                    {
                        if((wint2=ze_FindMobRoom(1,wstr4,ze_getroom(ze_activezone,sim_get_curr_room())))>=0)
                            sim_LookMobile(wint2);
                        else
                            dc_NotFound();
                    }
                    else
                    {
                        strncpy(wstr2,wstr4,wint);
                        wstr2[wint]='\0';
                        wint2=atoi(wstr2);
                        wstr4+=wint+1;
                        if((wint=ze_FindMobRoom(wint2,wstr4,ze_getroom(ze_activezone,sim_get_curr_room())))>=0)
                            sim_LookMobile(wint);
                        else
                            dc_NotFound();
                    }
                }
            }
            break;
        case 'I':
            wstr4=strtok(NULL," ");
            if(wstr4!='\0')
            {
                wint=atoi(wstr4);
                if(((strcspn(wstr4,"."))==strlen(wstr4))&&((wint>0)||((wint==0)&&(!strcmp(wstr4,"0")))))
                {
                    if(ze_FindNrItemRoom(1,wint,ze_getroom(ze_activezone,sim_get_curr_room()))>=0)
                        sim_LookItem(wint);
                    else
                        dc_NotFound();
                }
                else
                {
                    if((wint=strcspn(wstr4,"."))==strlen(wstr4))
                    {
                        if((wint2=ze_FindItemRoom(1,wstr4,ze_getroom(ze_activezone,sim_get_curr_room())))>=0)
                            sim_LookItem(wint2);
                        else
                            dc_NotFound();
                    }
                    else
                    {
                        strncpy(wstr2,wstr4,wint);
                        wstr2[wint]='\0';
                        wint2=atoi(wstr2);
                        wstr4+=wint+1;
                        if((wint=ze_FindItemRoom(wint2,wstr4,ze_getroom(ze_activezone,sim_get_curr_room())))>=0)
                            sim_LookItem(wint);
                        else
                            dc_NotFound();
                    }
                }
            }
            break;
        case 'N':
            sim_LookDir(DIR_NORTH);
            break;
        case 'E':
            sim_LookDir(DIR_EAST);
            break;
        case 'S':
            sim_LookDir(DIR_SOUTH);
            break;
        case 'W':
            sim_LookDir(DIR_WEST);
            break;
        case 'U':
            sim_LookDir(DIR_UP);
            break;
        case 'D':
            sim_LookDir(DIR_DOWN);
            break;
        default:
            printf("*** SYNTAX: LOOK or LOOK AT <MobNr|[Nr.]Pattern>\n");
            printf("                    LOOK IN <ObjNr|[Nr].Pattern>\n");
            printf("                    LOOK <Direction>\n");
            break;
        }
    }
}


void dc_mclone(char *wstr)
{
    char *wstr3;
    char *wstr4;
    char wstr2[DC_MAXSTRLEN];
    int wint,wint2,wint3;

    wstr3=strtok(wstr," ");
    wstr4=strtok(NULL," ");
    if(wstr4!=NULL)
        wint3=atoi(wstr4);
    else
        wint3=0;
    if((wstr3==NULL)||(wstr4==NULL)||((wint3==0)&&(strcmp(wstr4,"0")))||(wstr3=='\0')||(wstr4=='\0'))
        printf("*** Syntax: MCLONE <MobNr> <MobNr> or MCLONE <[Nr.]Pattern> <MobNr>\n");
    else
    {
        wint=atoi(wstr3);
        if(((strcspn(wstr3,"."))==strlen(wstr3))&&((wint>0)||((wint==0)&&(!strcmp(wstr3,"0")))))
        {
            if((wint2=me_GetRealNumber(wint))>=0)
                me_CloneMobile(wint2,wint3);
            else
                dc_NotFound();
        }
        else
        {
            if((wint=strcspn(wstr3,"."))==strlen(wstr3))
            {
                if((wint2=me_FindNrMobile(1,wstr3))>=0)
                    me_CloneMobile(wint2,wint3);
                else
                    dc_NotFound();
            }
            else
            {
                strncpy(wstr2,wstr3,wint);
                wstr2[wint]='\0';
                wint2=atoi(wstr2);
                wstr3+=wint+1;
                if((wint=me_FindNrMobile(wint2,wstr3))>=0)
                    me_CloneMobile(wint,wint3);
                else
                    dc_NotFound();
            }
        }
    }
}

void dc_mcreate(char *wstr)
{
    int wint;

    wstr=strtok(wstr," ");
    if(wstr!=NULL)
        wint=atoi(wstr);
    else
        wint=0;

    if(wstr!=NULL && (strcspn(wstr,".")==strlen(wstr))&&((wint>0)||((wint==0)&&(!strcmp(wstr,"0")))))
    {
        me_CloneMobile(-1,wint);
    }
    else
        printf("*** Syntax: MCREATE <MobNumber>\n");
}

void dc_medit(char *wstr)
{
    int wint,wint2;
    char wstr2[DC_MAXSTRLEN];

    wstr=strtok(wstr," ");
    if(wstr=='\0')
        printf("*** Syntax: MEDIT <MobNr|[Nr.]Pattern>\n");
    else
    {
        wint=atoi(wstr);
        if(((strcspn(wstr,"."))==strlen(wstr))&&((wint>0)||((wint==0)&&(!strcmp(wstr,"0")))))
        {
            if((wint2=me_GetRealNumber(wint))>=0)
                me_EditMobile(wint2);
            else
            {
                me_CloneMobile(-1,wint);
                if((wint2=me_GetRealNumber(wint))>=0)
                    me_EditMobile(wint2);
            }
        }
        else
        {
            if((wint=strcspn(wstr,"."))==strlen(wstr))
            {
                if((wint2=me_FindNrMobile(1,wstr))>=0)
                    me_EditMobile(wint2);
                else
                    dc_NotFound();
            }
            else
            {
                strncpy(wstr2,wstr,wint);
                wstr2[wint]='\0';
                wint2=atoi(wstr2);
                wstr+=wint+1;
                if((wint=me_FindNrMobile(wint2,wstr))>=0)
                    me_EditMobile(wint);
                else
                    dc_NotFound();
            }
        }
    }
}

void dc_mfind(char *wstr)
{
    wstr=strtok(wstr," ");
    if(wstr!='\0')
        me_FindMobiles(wstr);
    else
        printf("*** Syntax: MFIND <SearchPattern>\n");
}

void dc_mlist(char *wstr)
{
    char tmp[] = "*";
    dc_mfind(tmp);
}

void dc_mpurge(char *wstr)
{
    int wint,wint2;

    if (!*wstr) {
        printf("*** Syntax: MPURGE <MobileNumber>\n");
        return;
    }

    wstr=strtok(wstr," ");
    wint=atoi(wstr);
    if((strcspn(wstr,".")==strlen(wstr))&&((wint>0)||((wint==0)&&(!strcmp(wstr,"0")))))
    {
        if((wint2=me_GetRealNumber(wint))>=0)
            me_PurgeMobile(wint2);
        else
            dc_NotFound();
    }
    else
        printf("*** Syntax: MPURGE <MobileNumber>\n");
}

void dc_mshow(char *wstr)
{
    int wint,wint2;
    char wstr2[DC_MAXSTRLEN];

    wstr=strtok(wstr," ");
    if(wstr=='\0')
        printf("*** Syntax: MSHOW <MobNr|[Nr.]Pattern>\n");
    else
    {
        wint=atoi(wstr);
        if(((strcspn(wstr,"."))==strlen(wstr))&&((wint>0)||((wint==0)&&(!strcmp(wstr,"0")))))
        {
            if((wint2=me_GetRealNumber(wint))>=0)
                me_ShowMobile(wint2);
            else
                dc_NotFound();
        }
        else
        {
            if((wint=strcspn(wstr,"."))==strlen(wstr))
            {
                if((wint2=me_FindNrMobile(1,wstr))>=0)
                    me_ShowMobile(wint2);
                else
                    dc_NotFound();
            }
            else
            {
                strncpy(wstr2,wstr,wint);
                wstr2[wint]='\0';
                wint2=atoi(wstr2);
                wstr+=wint+1;
                if((wint=me_FindNrMobile(wint2,wstr))>=0)
                    me_ShowMobile(wint);
                else
                    dc_NotFound();
            }
        }
    }
}

void dc_purge(char *wstr)
{
    int wint,wint2;
    char wstr2[DC_MAXSTRLEN];
    char *wstr3;
    char *wstr4;

    wstr3=strtok(wstr," ");
    if(wstr3!='\0')
    {
        switch(toupper(*wstr3))
        {
        case 'M':
            wstr4=strtok(NULL," ");
            if(wstr4!='\0')
            {
                wint=atoi(wstr4);
                if(((strcspn(wstr4,"."))==strlen(wstr4))&&((wint>0)||((wint==0)&&(!strcmp(wstr4,"0")))))
                {
                    if(me_GetRealNumber(wint)>=0)
                        ze_PurgeMobile(wint);
                    else
                        dc_NotFound();
                }
                else
                {
                    if((wint=strcspn(wstr4,"."))==strlen(wstr4))
                    {
                        if((wint2=me_FindNrRealMobile(1,wstr4))>=0)
                            ze_PurgeMobile(wint2);
                        else
                            dc_NotFound();
                    }
                    else
                    {
                        strncpy(wstr2,wstr4,wint);
                        wstr2[wint]='\0';
                        wint2=atoi(wstr2);
                        wstr4+=wint+1;
                        if((wint=me_FindNrRealMobile(wint2,wstr4))>=0)
                            ze_PurgeMobile(wint);
                        else
                            dc_NotFound();
                    }
                }

            }
            break;
        case 'O':
            wstr4=strtok(NULL," ");
            if(wstr4!='\0')
            {
                wint=atoi(wstr4);
                if(((strcspn(wstr4,"."))==strlen(wstr4))&&((wint>0)||((wint==0)&&(!strcmp(wstr4,"0")))))
                {
                    if(ie_GetRealNumber(wint)>=0)
                        ze_PurgeItem(wint);
                    else
                        dc_NotFound();
                }
                else
                {
                    if((wint=strcspn(wstr4,"."))==strlen(wstr4))
                    {
                        if((wint2=ie_FindNrRealItem(1,wstr4))>=0)
                            ze_PurgeItem(wint2);
                        else
                            dc_NotFound();
                    }
                    else
                    {
                        strncpy(wstr2,wstr4,wint);
                        wstr2[wint]='\0';
                        wint2=atoi(wstr2);
                        wstr4+=wint+1;
                        if((wint=ie_FindNrRealItem(wint2,wstr4))>=0)
                            ze_PurgeItem(wint);
                        else
                            dc_NotFound();
                    }
                }

            }
            break;
        case 'A':
            ze_PurgeAll();
            break;
        default:
            printf("*** Syntax: PURGE MOB <MobNr|[Nr.]Pattern>\n");
            printf("            PURGE OBJ <ObjNr|[Nr.]Pattern>\n");
            printf("            PURGE ALL\n");
            break;
        }

    }
    else
    {
        printf("*** Syntax: PURGE MOB <MobNr|[Nr.]Pattern>\n");
        printf("            PURGE OBJ <ObjNr|[Nr.]Pattern>\n");
        printf("            PURGE ALL\n");
    }
}

void dc_put(char *wstr)
{
    int wint,wint2;

    int objnr;

    char wstr2[DC_MAXSTRLEN];
    char *wstr3;
    char *wstr4;

    wstr3=strtok(wstr," ");
    wstr4=strtok(NULL," ");

    if(wstr3!='\0')
    {
        wint=atoi(wstr3);
        if(((strcspn(wstr3,"."))==strlen(wstr3))&&((wint>0)||((wint==0)&&(!strcmp(wstr3,"0")))))
        {
            if(ie_GetRealNumber(wint)>=0)
            {
                objnr=wint;
            }
            else
            {
                dc_NotFound();
                return;
            }
        }
        else
        {
            if((wint=strcspn(wstr3,"."))==strlen(wstr3))
            {
                if((wint2=ie_FindNrRealItem(1,wstr3))>=0)
                {
                    objnr=wint2;
                }
                else
                {
                    dc_NotFound();
                    return;
                }
            }
            else
            {
                strncpy(wstr2,wstr3,wint);
                wstr2[wint]='\0';
                wint2=atoi(wstr2);
                wstr3+=wint+1;
                if((wint=ie_FindNrRealItem(wint2,wstr3))>=0)
                {
                    objnr=wint;
                }
                else
                {
                    dc_NotFound();
                    return;
                }
            }
        }


        if(wstr4!='\0')
        {
            wint=atoi(wstr4);
            if(((strcspn(wstr4,"."))==strlen(wstr4))&&((wint>0)||((wint==0)&&(!strcmp(wstr4,"0")))))
            {
                if((wint2=ze_FindNrItemRoom(1,wint,ze_getroom(ze_activezone,sim_get_curr_room())))>=0)
                    ze_GiveItemItem(objnr,wint2);
                else
                    dc_NotFound();
            }
            else
            {
                if((wint=strcspn(wstr4,"."))==strlen(wstr4))
                {
                    if((wint2=ze_FindItemRoom(1,wstr4,ze_getroom(ze_activezone,sim_get_curr_room())))>=0)
                        ze_GiveItemItem(objnr,wint2);
                    else
                        dc_NotFound();
                }
                else
                {
                    strncpy(wstr2,wstr4,wint);
                    wstr2[wint]='\0';
                    wint2=atoi(wstr2);
                    wstr4+=wint+1;
                    if((wint=ze_FindItemRoom(wint2,wstr4,ze_getroom(ze_activezone,sim_get_curr_room())))>=0)
                        ze_GiveItemItem(objnr,wint);
                    else
                        dc_NotFound();
                }
            }

        }
        else
        {
            printf("*** Syntax: PUT <ObjNr|[Nr.]Pattern> <ObjNr|[Nr.]Pattern>\n");
        }
    }
    else
    {
        printf("*** Syntax: PUT <ObjNr|[Nr.]Pattern> <ObjNr|[Nr.]Pattern>\n");
    }
}

void dc_quit(char *wstr)
{
    printf("Are you sure (yes/NO)? ");
    if(AskYesNo("yes"))
        dc_quitflag=1;
}

void dc_rclone(char *wstr)
{
    char *wstr3;
    char *wstr4;
    char wstr2[DC_MAXSTRLEN];
    int wint,wint2,wint3;

    wstr3=strtok(wstr," ");
    wstr4=strtok(NULL," ");
    if(wstr4!=NULL)
        wint3=atoi(wstr4);
    else
        wint3=0;
    if((wstr3==NULL)||(wstr4==NULL)||((wint3==0)&&(strcmp(wstr4,"0")))||(wstr3=='\0')||(wstr4=='\0'))
        printf("*** Syntax: RCLONE <RoomNr> <RoomNr> or RCLONE <[Nr.]Pattern> <RoomNr>\n")
            ;
    else
    {
        wint=atoi(wstr3);
        if(((strcspn(wstr3,"."))==strlen(wstr3))&&((wint>0)||((wint==0)&&(!strcmp(wstr3,"0")))))
        {
            if((wint2=re_GetRealNumber(wint))>=0)
                re_CloneRoom(wint2,wint3);
            else
                dc_NotFound();
        }
        else
        {
            if((wint=strcspn(wstr3,"."))==strlen(wstr3))
            {
                if((wint2=re_FindNrRoom(1,wstr3))>=0)
                    re_CloneRoom(wint2,wint3);
                else
                    dc_NotFound();
            }
            else
            {
                strncpy(wstr2,wstr3,wint);
                wstr2[wint]='\0';
                wint2=atoi(wstr2);
                wstr3+=wint+1;
                if((wint=re_FindNrRoom(wint2,wstr3))>=0)
                    re_CloneRoom(wint,wint3);
                else
                    dc_NotFound();
            }
        }
    }
}

void dc_rcreate(char *wstr)
{
    int wint;

    wstr=strtok(wstr," ");
    if(wstr!=NULL)
        wint=atoi(wstr);
    else
        wint=0;

    if(wstr!=NULL && (strcspn(wstr,".")==strlen(wstr))&&((wint>0)||((wint==0)&&(!
                                                                                strcmp(wstr,"0")))))
    {
        re_CloneRoom(-1,wint);
    }
    else
        printf("*** Syntax: RCREATE <RoomNumber>\n");
}

void dc_redit(char *wstr)
{
    int wint,wint2;
    char wstr2[DC_MAXSTRLEN];
	extern int sim_curroom;

    wstr=strtok(wstr," ");
    if(wstr=='\0') {
		wint = sim_curroom;
		if((wint2=re_GetRealNumber(wint))>=0)
			re_EditRoom(wint2);
		else
		{
			re_CloneRoom(-1,wint);
			if((wint2=re_GetRealNumber(wint))>=0)
				re_EditRoom(wint2);
		}
	}
	else if (*wstr == '-') {
        printf("*** Syntax: REDIT <RoomNr|[Nr.]Pattern>\n");
	}
    else
    {
        wint=atoi(wstr);
        if(((strcspn(wstr,"."))==strlen(wstr))&&((wint>0)||((wint==0)&&(!strcmp(wstr,"0")))))
        {
            if((wint2=re_GetRealNumber(wint))>=0)
                re_EditRoom(wint2);
            else
            {
                re_CloneRoom(-1,wint);
                if((wint2=re_GetRealNumber(wint))>=0)
                    re_EditRoom(wint2);
            }
        }
        else
        {
            if((wint=strcspn(wstr,"."))==strlen(wstr))
            {
                if((wint2=re_FindNrRoom(1,wstr))>=0)
                    re_EditRoom(wint2);
                else
                    dc_NotFound();
            }
            else
            {
                strncpy(wstr2,wstr,wint);
                wstr2[wint]='\0';
                wint2=atoi(wstr2);
                wstr+=wint+1;
                if((wint=re_FindNrRoom(wint2,wstr))>=0)
                    re_EditRoom(wint);
                else
                    dc_NotFound();
            }
        }
    }
}

void dc_rfind(char *wstr)
{
    wstr=strtok(wstr," ");
    if(wstr!='\0')
        re_FindRooms(wstr);
    else
        printf("*** Syntax: RFIND <SearchPattern>\n");
}

void dc_rlist(char *wstr)
{
    char tmp[] = "*";
    dc_rfind(tmp);
}

void dc_rpurge(char *wstr)
{
    int wint,wint2;

    if (!*wstr) {
        printf("*** Syntax: RPURGE <RoomNumber>\n");
        return;
    }

    wstr=strtok(wstr," ");
    wint=atoi(wstr);
    if((strcspn(wstr,".")==strlen(wstr))&&((wint>0)||((wint==0)&&(!strcmp(wstr,"0")))))
    {
        if((wint2=re_GetRealNumber(wint))>=0)
            re_PurgeRoom(wint2);
        else
            dc_NotFound();
    }
    else
        printf("*** Syntax: RPURGE <RoomNumber>\n");
}

void dc_rshow(char *wstr)
{
    int wint,wint2;
    char wstr2[DC_MAXSTRLEN];

    wstr=strtok(wstr," ");
    if(wstr=='\0')
        printf("*** Syntax: RSHOW <RoomNr|[Nr.]Pattern>\n");
    else
    {
        wint=atoi(wstr);
        if(((strcspn(wstr,"."))==strlen(wstr))&&((wint>0)||((wint==0)&&(!strcmp(wstr,"0")))))
        {
            if((wint2=re_GetRealNumber(wint))>=0)
                re_ShowRoom(wint2);
            else
                dc_NotFound();
        }
        else
        {
            if((wint=strcspn(wstr,"."))==strlen(wstr))
            {
                if((wint2=re_FindNrRoom(1,wstr))>=0)
                    re_ShowRoom(wint2);
                else
                    dc_NotFound();
            }
            else
            {
                strncpy(wstr2,wstr,wint);
                wstr2[wint]='\0';
                wint2=atoi(wstr2);
                wstr+=wint+1;
                if((wint=re_FindNrRoom(wint2,wstr))>=0)
                    re_ShowRoom(wint);
                else
                    dc_NotFound();
            }
        }
    }
}

void dc_north(char *wstr)
{
    int wint;
    int wint2;

    if((wint=sim_GetExit(re_GetRealNumber(sim_get_curr_room()),DIR_NORTH))==-1)
    {
        printf("No exit in this direction.\n");
    }
    else
        if((wint2=re_GetRealNumber(wint))>=0)
            sim_DoGoto(wint2);
        else
            dc_NotFound();
}

void dc_east(char *wstr)
{
    int wint;
    int wint2;

    if((wint=sim_GetExit(re_GetRealNumber(sim_get_curr_room()),DIR_EAST))==-1)
    {
        printf("No exit in this direction.\n");
    }
    else
        if((wint2=re_GetRealNumber(wint))>=0)
            sim_DoGoto(wint2);
        else
            dc_NotFound();
}


void dc_south(char *wstr)
{
    int wint;
    int wint2;

    if((wint=sim_GetExit(re_GetRealNumber(sim_get_curr_room()),DIR_SOUTH))==-1)
    {
        printf("No exit in this direction.\n");
    }
    else
        if((wint2=re_GetRealNumber(wint))>=0)
            sim_DoGoto(wint2);
        else
            dc_NotFound();
}

void dc_west(char *wstr)
{
    int wint;
    int wint2;

    if((wint=sim_GetExit(re_GetRealNumber(sim_get_curr_room()),DIR_WEST))==-1)
    {
        printf("No exit in this direction.\n");
    }
    else
        if((wint2=re_GetRealNumber(wint))>=0)
            sim_DoGoto(wint2);
        else
            dc_NotFound();
}

void dc_up(char *wstr)
{
    int wint;
    int wint2;

    if((wint=sim_GetExit(re_GetRealNumber(sim_get_curr_room()),DIR_UP))==-1)
    {
        printf("No exit in this direction.\n");
    }
    else
        if((wint2=re_GetRealNumber(wint))>=0)
            sim_DoGoto(wint2);
        else
            dc_NotFound();
}

void dc_down(char *wstr)
{
    int wint;
    int wint2;

    if((wint=sim_GetExit(re_GetRealNumber(sim_get_curr_room()),DIR_DOWN))==-1)
    {
        printf("No exit in this direction.\n");
    }
    else
        if((wint2=re_GetRealNumber(wint))>=0)
            sim_DoGoto(wint2);
        else
            dc_NotFound();
}

void dc_doorstate(char *wstr)
{
    int wint,wint2;
    char wstr2[DC_MAXSTRLEN];
    char *wstr4;

    wstr=strtok(wstr," ");
    if(wstr!='\0')
    {
        wstr4=strtok(NULL," ");
        if(wstr4!='\0')
        {
            int wint3;

            for(wint3=0;wint3<=zd_max;wint3++)
            {
                if(!strnccmp(wstr4,zd_str[wint3],strlen(wstr4)))
                {
                    wint2=wint3;
                    break;
                }
            }
            if(wint3>zd_max)
            {
                printf("Unknown DoorState.\n");
                return;
            }
            switch(toupper(*wstr))
            {
            case 'N':
                ze_DoorState(DIR_NORTH,wint2);
                break;
            case 'E':
                ze_DoorState(DIR_EAST,wint2);
                break;
            case 'S':
                ze_DoorState(DIR_SOUTH,wint2);
                break;
            case 'W':
                ze_DoorState(DIR_WEST,wint2);
                break;
            case 'U':
                ze_DoorState(DIR_UP,wint2);
                break;
            case 'D':
                ze_DoorState(DIR_DOWN,wint2);
                break;
            default:
                printf("Invalid direction.\n");
                break;
            }
        }
        else
            printf("*** SYNTAX: DOORSTATE <Direction> <State>\n");
    }
    else
    {
        printf("*** SYNTAX: DOORSTATE <Direction> <State>\n");
    }
}

void dc_version(char *wstr)
{
    printf("IEDIT   Version 1.6\n");
    printf("MEDIT   Version 1.1\n");
    printf("REDIT   Version 1.1\n");
    printf("ZEDIT   Version 1.0\n");
    printf("DIKUSIM Version 1.0\n");
    printf("DIKUCLI Version 1.0\n\n");
    printf("DIKUED Project Version 1.0 pl2\n");
}

void dc_wear(char *wstr)
{
    int wint,wint2;

    int objnr;
    int mobnr;

    char wstr2[DC_MAXSTRLEN];
    char *wstr3;
    char *wstr4;
    char *wstr5;

    wstr3=strtok(wstr," ");
    wstr4=strtok(NULL," ");
    wstr5=strtok(NULL," ");

    if(wstr3!='\0')
    {
        wint=atoi(wstr3);
        if(((strcspn(wstr3,"."))==strlen(wstr3))&&((wint>0)||((wint==0)&&(!strcmp(wstr3,"0")))))
        {
            if(ie_GetRealNumber(wint)>=0)
            {
                objnr=wint;
            }
            else
            {
                dc_NotFound();
                return;
            }
        }
        else
        {
            if((wint=strcspn(wstr3,"."))==strlen(wstr3))
            {
                if((wint2=ie_FindNrRealItem(1,wstr3))>=0)
                {
                    objnr=wint2;
                }
                else
                {
                    dc_NotFound();
                    return;
                }
            }
            else
            {
                strncpy(wstr2,wstr3,wint);
                wstr2[wint]='\0';
                wint2=atoi(wstr2);
                wstr3+=wint+1;
                if((wint=ie_FindNrRealItem(wint2,wstr3))>=0)
                {
                    objnr=wint;
                }
                else
                {
                    dc_NotFound();
                    return;
                }
            }
        }


        if(wstr4!='\0')
        {
            wint=atoi(wstr4);
            if(((strcspn(wstr4,"."))==strlen(wstr4))&&((wint>0)||((wint==0)&&(!strcmp(wstr4,"0")))))
            {
                if((wint2=ze_FindNrMobRoom(1,wint,ze_getroom(ze_activezone,sim_get_curr_room())))>=0)
                {
                    if(wstr5!='\0')
                    {
                        int wint3;

                        for(wint3=0;wint3<=zw_max;wint3++)
                        {
                            if(!strnccmp(wstr5,zw_str[wint3],strlen(wstr5)))
                            {
                                ze_WearItemMob(objnr,wint2,wint3);
                                return;
                            }
                        }
                        printf("Unknown WearPosition.\n");
                    }
                    else
                        ze_GiveItemMob(objnr,wint2);
                }
                else
                    dc_NotFound();
            }
            else
            {
                if((wint=strcspn(wstr4,"."))==strlen(wstr4))
                {
                    if((wint2=ze_FindMobRoom(1,wstr4,ze_getroom(ze_activezone,sim_get_curr_room())))>=0)
                    {
                        if(wstr5!='\0')
                        {
                            int wint3;

                            for(wint3=0;wint3<=zw_max;wint3++)
                            {
                                if(!strnccmp(wstr5,zw_str[wint3],strlen(wstr5)))
                                {
                                    ze_WearItemMob(objnr,wint2,wint3);
                                    return;
                                }
                            }
                            printf("Unknown WearPosition.\n");
                        }
                        else
                            ze_GiveItemMob(objnr,wint2);
                    }
                    else
                        dc_NotFound();
                }
                else
                {
                    strncpy(wstr2,wstr4,wint);
                    wstr2[wint]='\0';
                    wint2=atoi(wstr2);
                    wstr4+=wint+1;
                    if((wint=ze_FindMobRoom(wint2,wstr4,ze_getroom(ze_activezone,sim_get_curr_room())))>=0)
                    {
                        if(wstr5!='\0')
                        {
                            int wint3;

                            for(wint3=0;wint3<=zw_max;wint3++)
                            {
                                if(!strnccmp(wstr5,zw_str[wint3],strlen(wstr5)))
                                {
                                    ze_WearItemMob(objnr,wint2,wint3);
                                    return;
                                }
                            }
                            printf("Unknown WearPosition.\n");
                        }
                        else
                            ze_GiveItemMob(objnr,wint);
                    }
                    else
                        dc_NotFound();
                }
            }

        }
        else
        {
            printf("*** Syntax: WEAR <ObjNr|[Nr.]Pattern> <MobNr|[Nr.]Pattern> or\n");
            printf("            WEAR <ObjNr|[Nr.]Pattern> <MobNr|[Nr.]Pattern> <WearPos>\n");
        }
    }
    else
    {
        printf("*** Syntax: WEAR <ObjNr|[Nr.]Pattern> <MobNr|[Nr.]Pattern> or\n");
        printf("            WEAR <ObjNr|[Nr.]Pattern> <MobNr|[Nr.]Pattern> <WearPos>\n");
    }
}

void dc_zclone(char *wstr)
{
    char *wstr3;
    char *wstr4;
    char wstr2[DC_MAXSTRLEN];
    int wint,wint2,wint3;

    wstr3=strtok(wstr," ");
    wstr4=strtok(NULL," ");
    if(wstr4!=NULL)
        wint3=atoi(wstr4);
    else
        wint3=0;
    if((wstr3==NULL)||(wstr4==NULL)||((wint3==0)&&(strcmp(wstr4,"0")))||(wstr3=='\0')||(wstr4=='\0'))
        printf("*** Syntax: ZCLONE <ZoneNr> <ZoneNr> or ZCLONE <[Nr.]Pattern> <ZoneNr>\n");
    else
    {
        wint=atoi(wstr3);
        if(((strcspn(wstr3,"."))==strlen(wstr3))&&((wint>0)||((wint==0)&&(!strcmp(wstr3,"0")))))
        {
            if((wint2=ze_GetRealNumber(wint))>=0)
                ze_CloneZone(wint2,wint3);
            else
                dc_NotFound();
        }
        else
        {
            if((wint=strcspn(wstr3,"."))==strlen(wstr3))
            {
                if((wint2=ze_FindNrZone(1,wstr3))>=0)
                    ze_CloneZone(wint2,wint3);
                else
                    dc_NotFound();
            }
            else
            {
                strncpy(wstr2,wstr3,wint);
                wstr2[wint]='\0';
                wint2=atoi(wstr2);
                wstr3+=wint+1;
                if((wint=ze_FindNrZone(wint2,wstr3))>=0)
                    ze_CloneZone(wint,wint3);
                else
                    dc_NotFound();
            }
        }
    }
}

void dc_zcreate(char *wstr)
{
    int wint;

    wstr=strtok(wstr," ");
    if(wstr!=NULL)
        wint=atoi(wstr);
    else
        wint=0;

    if(wstr!=NULL && (strcspn(wstr,".")==strlen(wstr))&&((wint>0)||((wint==0)&&(!strcmp(wstr,"0")))))
    {
        ze_CloneZone(-1,wint);
    }
    else
        printf("*** Syntax: ZCREATE <ZoneNumber>\n");
}

void dc_zedit(char *wstr)
{
    int wint,wint2;
    char wstr2[DC_MAXSTRLEN];

    wstr=strtok(wstr," ");
    if(wstr=='\0')
        printf("*** Syntax: ZEDIT <ZoneNr|[Nr.]Pattern>\n");
    else
    {
        wint=atoi(wstr);
        if(((strcspn(wstr,"."))==strlen(wstr))&&((wint>0)||((wint==0)&&(!strcmp(wstr,"0")))))
        {
            if((wint2=ze_GetRealNumber(wint))>=0)
                ze_EditZone(wint2);
            else
            {
                ze_CloneZone(-1,wint);
                if((wint2=ze_GetRealNumber(wint))>=0)
                    ze_EditZone(wint2);
            }
        }
        else
        {
            if((wint=strcspn(wstr,"."))==strlen(wstr))
            {
                if((wint2=ze_FindNrZone(1,wstr))>=0)
                    ze_EditZone(wint2);
                else
                    dc_NotFound();
            }
            else
            {
                strncpy(wstr2,wstr,wint);
                wstr2[wint]='\0';
                wint2=atoi(wstr2);
                wstr+=wint+1;
                if((wint=ze_FindNrZone(wint2,wstr))>=0)
                    ze_EditZone(wint);
                else
                    dc_NotFound();
            }
        }

    }
}

void dc_zfind(char *wstr)
{
    wstr=strtok(wstr," ");
    if(wstr!='\0')
        ze_FindZones(wstr);
    else
        printf("*** Syntax: ZFIND <SearchPattern>\n");
}

void dc_zlist(char *wstr)
{
    char tmp[] = "*";
    

    dc_zfind(tmp);
}

void dc_zpurge(char *wstr)
{
    int wint,wint2;

    if (!*wstr) {
        printf("*** Syntax: ZPURGE <ZoneNumber>\n");
        return;
    }

    wstr=strtok(wstr," ");
    wint=atoi(wstr);
    if((strcspn(wstr,".")==strlen(wstr))&&((wint>0)||((wint==0)&&(!strcmp(wstr,"0")))))
    {
        if((wint2=ze_GetRealNumber(wint))>=0)
            ze_PurgeZone(wint2);
        else
            dc_NotFound();
    }
    else
        printf("*** Syntax: ZPURGE <ZoneNumber>\n");
}

void dc_zshow(char *wstr)
{
    int wint,wint2;
    char wstr2[DC_MAXSTRLEN];

    wstr=strtok(wstr," ");
    if(wstr=='\0')
        printf("*** Syntax: ZSHOW <ZoneNr|[Nr.]Pattern>\n");
    else
    {
        wint=atoi(wstr);
        if(((strcspn(wstr,"."))==strlen(wstr))&&((wint>0)||((wint==0)&&(!strcmp(wstr,"0")))))
        {
            if((wint2=ze_GetRealNumber(wint))>=0)
                ze_ShowZone(wint2);
            else
                dc_NotFound();
        }
        else
        {
            if((wint=strcspn(wstr,"."))==strlen(wstr))
            {
                if((wint2=ze_FindNrZone(1,wstr))>=0)
                    ze_ShowZone(wint2);
                else
                    dc_NotFound();
            }
            else
            {
                strncpy(wstr2,wstr,wint);
                wstr2[wint]='\0';
                wint2=atoi(wstr2);
                wstr+=wint+1;
                if((wint=ze_FindNrZone(wint2,wstr))>=0)
                    ze_ShowZone(wint);
                else
                    dc_NotFound();
            }
        }
    }
}

void dc_NotFound(void)
{
    printf("*** Nothing found, matching the parameter!\n");
}

void DoCmdLine(char *wstr)
{
    char wchr;
    char command[DC_MAXSTRLEN];
    int i;

    command[0]='\0';
    sscanf(wstr,"%s ",command);

    wchr=toupper(*command);
    if((wchr>='A')&&(wchr<='Z'))
    {
        for(i=0;i<dc_cmdhash[wchr-'A'].nr;i++)
        {
            if(!strnccmp(command,dc_cmdhash[wchr-'A'].cmds[i].cmd,strlen(command)))
            {
                (*(dc_cmdhash[wchr-'A'].cmds[i].funct))(wstr+strlen(command));
                break;
            }
        }
        if(i==dc_cmdhash[wchr-'A'].nr)
            printf("Unknown command!!!\n");
    }
    else
    {
        switch(wchr)
        {
        case '!': {
            if(strlen(lastcommand)>0)
                DoCmdLine(lastcommand);
            break;
		}
        default : printf("Illegal input!!!\n");
        }
    }
}

void ReadHelpFile(void)
{
    FILE *dc_helpfile;
    char wstr1[DC_MAXSTRLEN];
    char *wstr2;
    char wchr;
    int i;

    if((dc_helpfile=fopen(DC_HELPFILE,"r"))==NULL)
    {
        printf("*** CAN'T OPEN HELP-FILE...\n");
        return;
    }
    else
    {
        while(fgets(wstr1,DC_MAXSTRLEN,dc_helpfile)!=NULL)
        {
            if((wstr2=fread_string(dc_helpfile))==NULL)
            {
                printf("*** ERROR IN FORMAT OF HELP-FILE IN ENTRY: %s - ignored\n",wstr1);
            }
            else
            {
                wchr=toupper(*wstr1);
                if((wchr>='A')&&(wchr<='Z'))
                {
                    for(i=0;i<dc_cmdhash[wchr-'A'].nr;i++)
                    {
                        if(!strnccmp(wstr1,dc_cmdhash[wchr-'A'].cmds[i].cmd,strlen(wstr1)-1))
                        {
                            dc_cmdhash[wchr-'A'].cmds[i].help=wstr2;
                            break;
                        }
                    }
                    if(i==dc_cmdhash[wchr-'A'].nr)
                        printf("*** HELP FOR UNKNOWN COMMAND: %s- ignored\n",wstr1);
                }

            }
        }
        fclose(dc_helpfile);
    }
}

void FreeHelp(void)
{
    int wint1,wint2;

    for(wint1=0;wint1<26;wint1++)
    {
        for(wint2=0;wint2<dc_cmdhash[wint1].nr;wint2++)
        {
            free(dc_cmdhash[wint1].cmds[wint2].help);
        }
    }
}

void SimulateDikuCli(void)
{
    char inputline[DC_MAXSTRLEN];

    printf(" --- DIKU STYLE CLI V1.0 --- \n\n");
    if(*MudName!='\0')
        printf("Running with database for %s.\n\n",MudName);


    sim_DoLook(re_GetRealNumber(sim_get_curr_room()));
    printf("\n");

    while(!dc_quitflag)
    {
        printf(DC_PROMPT);
        gets(inputline);
        if(strlen(inputline)>0)
        {
            DoCmdLine(inputline);
            if(*inputline!='!')
                strcpy(lastcommand,inputline);
            printf("\n");
        }
    }
    return;
}

void dc_CMudName(char *wstr)
{
    strcpy(MudName,wstr);
}

void dc_CItemFile(char *wstr)
{
    ie_SetFile(wstr);
}

void dc_CIETempF(char *wstr)
{
    ie_SetTemp(wstr);
}

void dc_CMobileFile(char *wstr)
{
    me_SetFile(wstr);
}

void dc_CMETempF(char *wstr)
{
    me_SetTemp(wstr);
}

void dc_CRoomFile(char *wstr)
{
    re_SetFile(wstr);
}

void dc_CRETempF(char *wstr)
{
    re_SetTemp(wstr);
}

void dc_CZoneFile(char *wstr)
{
    ze_SetFile(wstr);
}

void dc_CZETempF(char *wstr)
{
    ze_SetTemp(wstr);
}

void dc_CDataBPath(char *wstr)
{
    ie_SetDBPath(wstr);
    me_SetDBPath(wstr);
    ze_SetDBPath(wstr);
}

void dc_CAffBitVector(char *wstr)
{
    if(strcmp(wstr,"0")&&strcmp(wstr,"1"))
        printf("*** Argument for 'AffBitVector' must be 0 or 1!\n");
    else
        ie_SetAffBitVec(atoi(wstr));
}

void DoConfigFile(void)
{
    FILE *dc_conffile;
    char wstr[DC_MAXSTRLEN];
    char *wstr2,*wstr3;
    int wint;

    if(!(dc_conffile=fopen(DC_CONFFILE,"r")))
    {
        printf("*** Error opening the configuration-file... using defaults.\n");
        return;
    }
    else
    {
        while(fgets(wstr,81,dc_conffile))
        {
            if(*wstr!='\n')
            {
                wstr2=strtok(wstr," \t\n");
                for(wint=0;wint<DC_CONFVARS;wint++)
                {
                    if(!strcmp(wstr2,ConfigVars[wint]))
                    {
                        wstr3=strtok(NULL," \t\n");
                        if(*wstr3=='\0')
                            printf("*** Missing argument for ConfVar: %s\n",wstr2);
                        else
                            (*dc_conf[wint])(wstr3);
                        break;
                    }
                }
                if(wint==DC_CONFVARS)
                    printf("*** Unknown ConfigVar '%s'... ignored\n",wstr2);
            }
        }
        fclose(dc_conffile);
    }
}


int main()
{
    DoConfigFile();
    ie_InitIEdit();
    me_InitMEdit();
    re_InitREdit();
    ze_InitZEdit();
    sim_InitDikuSim();
    ReadHelpFile();
    SimulateDikuCli();
    ie_CloseIEdit();
    me_CloseMEdit();
    re_CloseREdit();
    ze_CloseZEdit();
    FreeHelp();
    return 0;
}
