#
# Makefile for New Narai MUD
#
# For GNU make only.

PROG    = mud-kit
INCDIR  = ./include
UTLSRC  = ./utils
OBJDIR  = ./obj
INSTDIR = ../bin
VPATH   = .:$(INCDIR):$(UTLSRC):

# To use gdb, use gcc for Linux, cc (= clang) for FreeBSD
# CC      = clang
# CC      = gcc
CC	= gcc-9
# CC      = cc

# MAKE    = gmake
MAKE    = make
LD      = $(CC)

OFLAGS  = -O0 -g -Wall -Wextra -Werror -pipe
# OFLAGS  = -O -g -Wall -pipe

INCLUDE = -I$(INCDIR)

# MFLAGS: RETURN_TO_QUIT INIT_BARE_HAND GHOST
#         OldMobileFormat OLD_DAM_MESSAGE OLD_PARRY
#         OLD_MIRROR_IMAGE OldMobAct
#
# MFLAGS    = -DIMO=41 -DRETURN_TO_QUIT
#
# MFLAGS are moved to "include/global.h"

# Set initial gold and quest in "act.player.c"
DFLAGS= -DBETA_TEST
# DFLAGS=

CFLAGS  = $(OFLAGS) $(DFLAGS) $(INCLUDE)

# LDFLAGS    = libdescrypt.a
LDFLAGS  = -lcrypt -g

SRCS    = \
	act.comm.c act.info.c act.look.c act.move.c act.obj1.c act.obj2.c \
	act.offensive.c act.player.c act.special.c act.wizard.c board.c \
       	cast1.c cast2.c comm.c constants.c db.c db.player.c db.zone.c \
	fight.c guild.c handler.c house.c interpreter.c library.c \
       	magic.c mob_act.c mob_bal.c point.c spec_mid.c spec_misc.c \
       	spec_mob.c special.c spell1.c spell2.c spell_parser.c spec_misc2.c

UTILS       = chpass checkchar purgechar listchar zoneitem zonelist zoneprint \
	      mobbalance mobcreate mobedit testskill dostashform undostashform

# SCRIPTS     = everun backup RESTART RUNME shutmud
SCRIPTS     = runmud.sh killmud.sh

OBJS        = $(SRCS:%.c=$(OBJDIR)/%.o)
PROG_BIN    = $(OBJDIR)/$(PROG)
U_BINS      = $(UTILS:%=$(OBJDIR)/%)
CTAGS_SRC   = $(SRCS) $(INCDIR)/*.h

$(OBJDIR)/%.o :  %.c
	$(CC) -c $(CFLAGS) -o $@ $<

$(OBJDIR)/% :  $(OBJDIR)/%.o
	$(LD) -o $@ $< $(LDFLAGS)

default :    objdir $(PROG_BIN)

all:    $(PROG_BIN) utils

$(PROG_BIN) : $(OBJS)
	$(LD) -o $@ $^ $(LDFLAGS)

objdir:
	if [ ! -d $(OBJDIR) ] ; then mkdir $(OBJDIR); fi

utils : $(U_BINS)

clean:
	- /bin/rm -f $(OBJS) $(UTILS:%=$(OBJDIR)/%.o) tags core
	- /bin/rm -f $(PROG_BIN) $(U_BINS)

install : $(PROG_BIN) $(U_BINS)
	# - if [ -x $(INSTDIR)/$(PROG).run ] ; then \
	#	/bin/mv -f $(INSTDIR)/$(PROG).run $(INSTDIR)/$(PROG).old ; fi
	/bin/cp -p $(PROG_BIN) $(INSTDIR)/$(PROG).new
	/bin/cp -p $(U_BINS) $(INSTDIR)
	/bin/cp -p $(SCRIPTS:%=$(UTLSRC)/%) $(INSTDIR)
	/bin/chmod 700 $(SCRIPTS:%=$(INSTDIR)/%)

uninstall :
	- /bin/rm -f $(INSTDIR)/$(PROG).new $(INSTDIR)/$(PROG).run $(INSTDIR)/$(PROG).old
	- /bin/rm -f $(UTILS:%=$(INSTDIR)/%) $(SCRIPTS:%=$(INSTDIR)/%)

ctags :
	- ctags $(CTAGS_SRC)


#======================================================================

# DO NOT DELETE
INC=$(INCDIR)
OBJ=$(OBJDIR)

# -I$(INC) act.comm.c act.info.c act.look.c act.move.c act.obj1.c act.obj2.c act.offensive.c act.player.c act.special.c act.wizard.c board.c cast1.c cast2.c comm.c constants.c db.c db.player.c db.zone.c fight.c guild.c handler.c house.c interpreter.c library.c magic.c mob_act.c mob_bal.c point.c spec_mid.c spec_misc.c spec_mob.c special.c spell_parser.c spell1.c spell2.c utils/chpass.c utils/checkchar.c utils/purgechar.c utils/listchar.c utils/zoneitem.c utils/zonelist.c utils/zoneprint.c utils/mobbalance.c utils/mobcreate.c utils/mobedit.c utils/testskill.c utils/dostashform.c utils/undostashform.c

$(OBJ)/act.comm.o: act.comm.c   \
    $(INC)/char.h $(INC)/typedef.h \
    $(INC)/object.h $(INC)/global.h \
    $(INC)/comm.h $(INC)/actions.h

$(OBJ)/act.info.o: act.info.c   \
    $(INC)/char.h $(INC)/typedef.h \
    $(INC)/object.h $(INC)/global.h \
    $(INC)/comm.h $(INC)/play.h \
    $(INC)/gamedb.h

$(OBJ)/act.look.o: act.look.c   \
    $(INC)/char.h $(INC)/typedef.h  \
    $(INC)/object.h \
    $(INC)/global.h   \
    $(INC)/comm.h $(INC)/play.h $(INC)/actions.h

$(OBJ)/act.move.o: act.move.c   \
    $(INC)/char.h $(INC)/typedef.h  \
    $(INC)/object.h $(INC)/global.h  \
    $(INC)/comm.h $(INC)/play.h \
    $(INC)/etc.h

$(OBJ)/act.obj1.o: act.obj1.c   \
    $(INC)/char.h $(INC)/typedef.h \
    $(INC)/object.h $(INC)/global.h \
    $(INC)/comm.h $(INC)/etc.h

$(OBJ)/act.obj2.o: act.obj2.c   \
    $(INC)/char.h $(INC)/typedef.h \
    $(INC)/object.h $(INC)/global.h \
    $(INC)/comm.h $(INC)/play.h \
    $(INC)/actions.h

$(OBJ)/act.offensive.o: act.offensive.c  \
    $(INC)/char.h $(INC)/typedef.h \
    $(INC)/object.h $(INC)/global.h \
    $(INC)/comm.h \
    $(INC)/play.h $(INC)/spells.h $(INC)/actions.h

$(OBJ)/act.player.o: act.player.c   \
    $(INC)/char.h $(INC)/typedef.h \
    $(INC)/object.h $(INC)/global.h \
    $(INC)/comm.h $(INC)/gamedb.h \
    $(INC)/play.h $(INC)/etc.h

$(OBJ)/act.special.o: act.special.c  \
    $(INC)/char.h \
    $(INC)/typedef.h  \
    $(INC)/object.h $(INC)/global.h  \
    $(INC)/comm.h $(INC)/play.h $(INC)/spells.h

$(OBJ)/act.wizard.o: act.wizard.c   \
    $(INC)/char.h $(INC)/typedef.h  \
    $(INC)/object.h $(INC)/global.h \
    $(INC)/comm.h $(INC)/gamedb.h \
    $(INC)/actions.h $(INC)/etc.h

$(OBJ)/board.o: board.c   \
    $(INC)/char.h $(INC)/typedef.h \
    $(INC)/object.h $(INC)/global.h \
    $(INC)/comm.h $(INC)/actions.h

$(OBJ)/cast1.o: cast1.c   \
    $(INC)/char.h $(INC)/typedef.h  \
    $(INC)/object.h $(INC)/global.h  \
    $(INC)/comm.h $(INC)/play.h \
    $(INC)/spells.h

$(OBJ)/cast2.o: cast2.c   \
    $(INC)/char.h $(INC)/typedef.h  \
    $(INC)/object.h $(INC)/global.h  \
    $(INC)/comm.h $(INC)/play.h \
    $(INC)/spells.h

$(OBJ)/comm.o: comm.c   \
    $(INC)/char.h $(INC)/typedef.h \
    $(INC)/object.h $(INC)/global.h \
    $(INC)/comm.h $(INC)/gamedb.h $(INC)/etc.h

$(OBJ)/constants.o: constants.c   \
    $(INC)/char.h $(INC)/typedef.h  \
    $(INC)/global.h   \
    $(INC)/play.h

$(OBJ)/db.o: db.c   \
    $(INC)/char.h $(INC)/typedef.h  \
    $(INC)/object.h \
    $(INC)/global.h  $(INC)/comm.h \
    $(INC)/play.h $(INC)/gamedb.h \
    $(INC)/spells.h

$(OBJ)/db.player.o: db.player.c   \
    $(INC)/char.h $(INC)/typedef.h  \
    $(INC)/object.h $(INC)/global.h  \
    $(INC)/comm.h $(INC)/gamedb.h

$(OBJ)/db.zone.o: db.zone.c   \
    $(INC)/char.h $(INC)/typedef.h  \
    $(INC)/object.h $(INC)/global.h  \
    $(INC)/comm.h $(INC)/gamedb.h

$(OBJ)/fight.o: fight.c   \
    $(INC)/char.h $(INC)/typedef.h  \
    $(INC)/object.h $(INC)/global.h  \
    $(INC)/comm.h $(INC)/play.h \
    $(INC)/spells.h $(INC)/etc.h

$(OBJ)/guild.o: guild.c   \
    $(INC)/char.h $(INC)/typedef.h \
    $(INC)/object.h $(INC)/global.h \
    $(INC)/comm.h $(INC)/play.h \
    $(INC)/actions.h $(INC)/etc.h

$(OBJ)/handler.o: handler.c   \
    $(INC)/char.h $(INC)/typedef.h  \
    $(INC)/object.h $(INC)/global.h  \
    $(INC)/comm.h $(INC)/play.h $(INC)/etc.h

$(OBJ)/house.o: house.c   \
    $(INC)/char.h $(INC)/typedef.h  \
    $(INC)/object.h $(INC)/global.h  \
    $(INC)/comm.h $(INC)/gamedb.h \
    $(INC)/actions.h $(INC)/spells.h $(INC)/play.h

$(OBJ)/interpreter.o: interpreter.c  \
    $(INC)/char.h $(INC)/typedef.h \
    $(INC)/object.h $(INC)/global.h \
    $(INC)/comm.h \
    $(INC)/actions.h

$(OBJ)/library.o: library.c   \
    $(INC)/char.h $(INC)/typedef.h  \
    $(INC)/object.h \
    $(INC)/global.h  $(INC)/comm.h

$(OBJ)/magic.o: magic.c   \
    $(INC)/char.h $(INC)/typedef.h  \
    $(INC)/object.h $(INC)/global.h  \
    $(INC)/comm.h $(INC)/play.h \
    $(INC)/spells.h $(INC)/etc.h

$(OBJ)/mob_act.o: mob_act.c   \
    $(INC)/char.h $(INC)/typedef.h  \
    $(INC)/object.h $(INC)/global.h  \
    $(INC)/comm.h $(INC)/play.h \
    $(INC)/spells.h $(INC)/actions.h $(INC)/etc.h

$(OBJ)/mob_bal.o: mob_bal.c

$(OBJ)/point.o: point.c   \
    $(INC)/char.h $(INC)/typedef.h  \
    $(INC)/object.h $(INC)/global.h  \
    $(INC)/comm.h $(INC)/play.h $(INC)/etc.h

$(OBJ)/spec_mid.o: spec_mid.c   \
    $(INC)/char.h $(INC)/typedef.h \
    $(INC)/object.h $(INC)/global.h \
    $(INC)/comm.h $(INC)/play.h \
    $(INC)/actions.h $(INC)/gamedb.h $(INC)/etc.h

$(OBJ)/spec_misc.o: spec_misc.c   \
    $(INC)/char.h $(INC)/typedef.h  \
    $(INC)/object.h $(INC)/global.h  \
    $(INC)/comm.h $(INC)/play.h \
    $(INC)/actions.h $(INC)/spells.h $(INC)/etc.h

$(OBJ)/spec_mob.o: spec_mob.c   \
    $(INC)/char.h $(INC)/typedef.h  \
    $(INC)/object.h $(INC)/global.h  \
    $(INC)/comm.h $(INC)/play.h \
    $(INC)/actions.h $(INC)/spells.h $(INC)/etc.h

$(OBJ)/special.o: special.c   \
    $(INC)/char.h $(INC)/typedef.h  \
    $(INC)/object.h $(INC)/global.h  \
    $(INC)/comm.h $(INC)/actions.h $(INC)/etc.h

$(OBJ)/spell_parser.o: spell_parser.c  \
    $(INC)/char.h $(INC)/typedef.h \
    $(INC)/object.h $(INC)/global.h \
    $(INC)/comm.h \
    $(INC)/play.h $(INC)/spells.h

$(OBJ)/spell1.o: spell1.c   \
    $(INC)/char.h $(INC)/typedef.h  \
    $(INC)/object.h $(INC)/global.h  \
    $(INC)/comm.h $(INC)/play.h \
    $(INC)/spells.h

$(OBJ)/spell2.o: spell2.c   \
    $(INC)/char.h $(INC)/typedef.h \
    $(INC)/object.h $(INC)/global.h \
    $(INC)/comm.h $(INC)/play.h \
    $(INC)/spells.h $(INC)/etc.h

$(OBJ)/chpass.o: utils/chpass.c   \
    $(INC)/char.h $(INC)/typedef.h \
    $(INC)/gamedb.h

$(OBJ)/checkchar.o: utils/checkchar.c   \
    $(INC)/char.h $(INC)/typedef.h  \
    $(INC)/gamedb.h

$(OBJ)/purgechar.o: utils/purgechar.c   \
    $(INC)/char.h $(INC)/typedef.h \
    $(INC)/gamedb.h

$(OBJ)/listchar.o: utils/listchar.c   \
    $(INC)/char.h $(INC)/typedef.h  \
    $(INC)/gamedb.h

$(OBJ)/zoneitem.o: utils/zoneitem.c

$(OBJ)/zonelist.o: utils/zonelist.c

$(OBJ)/zoneprint.o: utils/zoneprint.c

$(OBJ)/mobbalance.o: utils/mobbalance.c

$(OBJ)/mobcreate.o: utils/mobcreate.c utils/mobcreate.h

$(OBJ)/mobedit.o: utils/mobedit.c utils/mobcreate.h

$(OBJ)/testskill.o: utils/testskill.c

$(OBJ)/dostashform.o: utils/dostashform.c

$(OBJ)/undostashform.o: utils/undostashform.c

