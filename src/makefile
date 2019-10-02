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
       	spec_mob.c special.c spell1.c spell2.c spell_parser.c

#UTILS       = change_level change_pass chstashform unchstashform check_pass \
#              check_player list_player mob_create mob_edit zone_item \
#              zone_list zone_print mob_balance check_god fix_noname

UTILS       = chpass dangun dupname purge test_skill \
	      list what_item_in_zone

# SCRIPTS     = everun backup RESTART RUNME shutmud
SCRIPTS     = runmud.sh killmud.sh backup.sh

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
	- if [ -x $(INSTDIR)/$(PROG).run ] ; then \
	      /bin/mv -f $(INSTDIR)/$(PROG).run $(INSTDIR)/$(PROG).old ; fi
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

# -I$(INC) act.comm.c act.info.c act.look.c act.move.c act.obj1.c act.obj2.c act.offensive.c act.player.c act.special.c act.wizard.c board.c cast1.c cast2.c comm.c constants.c db.c db.player.c db.zone.c fight.c guild.c handler.c house.c interpreter.c library.c magic.c mob_act.c mob_bal.c point.c spec_mid.c spec_misc.c spec_mob.c special.c spell_parser.c spell1.c spell2.c utils/chpass.c utils/dangun.c utils/dupname.c utils/list.c utils/player.c utils/purge.c utils/test_skill.c utils/what_item_in_zone.c

act.comm.o: act.comm.c   \
    $(INC)/char.h $(INC)/typedef.h \
    $(INC)/object.h $(INC)/global.h \
    $(INC)/comm.h $(INC)/actions.h

act.info.o: act.info.c   \
    $(INC)/char.h $(INC)/typedef.h \
    $(INC)/object.h $(INC)/global.h \
    $(INC)/comm.h $(INC)/play.h \
    $(INC)/gamedb.h

act.look.o: act.look.c   \
    $(INC)/char.h $(INC)/typedef.h  \
    $(INC)/object.h \
    $(INC)/global.h   \
    $(INC)/comm.h $(INC)/play.h $(INC)/actions.h

act.move.o: act.move.c   \
    $(INC)/char.h $(INC)/typedef.h  \
    $(INC)/object.h $(INC)/global.h  \
    $(INC)/comm.h $(INC)/play.h \
    $(INC)/etc.h

act.obj1.o: act.obj1.c   \
    $(INC)/char.h $(INC)/typedef.h \
    $(INC)/object.h $(INC)/global.h \
    $(INC)/comm.h

act.obj2.o: act.obj2.c   \
    $(INC)/char.h $(INC)/typedef.h \
    $(INC)/object.h $(INC)/global.h \
    $(INC)/comm.h $(INC)/play.h \
    $(INC)/actions.h

act.offensive.o: act.offensive.c  \
    $(INC)/char.h $(INC)/typedef.h \
    $(INC)/object.h $(INC)/global.h \
    $(INC)/comm.h \
    $(INC)/play.h $(INC)/spells.h $(INC)/actions.h

act.player.o: act.player.c   \
    $(INC)/char.h $(INC)/typedef.h \
    $(INC)/object.h $(INC)/global.h \
    $(INC)/comm.h $(INC)/gamedb.h \
    $(INC)/play.h $(INC)/etc.h

act.special.o: act.special.c  \
    $(INC)/char.h \
    $(INC)/typedef.h  \
    $(INC)/object.h $(INC)/global.h  \
    $(INC)/comm.h $(INC)/play.h $(INC)/spells.h

act.wizard.o: act.wizard.c   \
    $(INC)/char.h $(INC)/typedef.h  \
    $(INC)/object.h $(INC)/global.h \
    $(INC)/comm.h $(INC)/gamedb.h \
    $(INC)/actions.h $(INC)/etc.h

board.o: board.c   \
    $(INC)/char.h $(INC)/typedef.h \
    $(INC)/object.h $(INC)/global.h \
    $(INC)/comm.h $(INC)/actions.h

cast1.o: cast1.c   \
    $(INC)/char.h $(INC)/typedef.h  \
    $(INC)/object.h $(INC)/global.h  \
    $(INC)/comm.h $(INC)/play.h \
    $(INC)/spells.h

cast2.o: cast2.c   \
    $(INC)/char.h $(INC)/typedef.h  \
    $(INC)/object.h $(INC)/global.h  \
    $(INC)/comm.h $(INC)/play.h \
    $(INC)/spells.h

comm.o: comm.c   \
    $(INC)/char.h $(INC)/typedef.h \
    $(INC)/object.h $(INC)/global.h \
    $(INC)/comm.h $(INC)/gamedb.h

constants.o: constants.c   \
    $(INC)/char.h $(INC)/typedef.h  \
    $(INC)/global.h   \
    $(INC)/play.h

db.o: db.c   \
    $(INC)/char.h $(INC)/typedef.h  \
    $(INC)/object.h \
    $(INC)/global.h  $(INC)/comm.h \
    $(INC)/play.h $(INC)/gamedb.h

db.player.o: db.player.c   \
    $(INC)/char.h $(INC)/typedef.h  \
    $(INC)/object.h $(INC)/global.h  \
    $(INC)/comm.h $(INC)/gamedb.h

db.zone.o: db.zone.c   \
    $(INC)/char.h $(INC)/typedef.h  \
    $(INC)/object.h $(INC)/global.h  \
    $(INC)/comm.h $(INC)/gamedb.h

fight.o: fight.c   \
    $(INC)/char.h $(INC)/typedef.h  \
    $(INC)/object.h $(INC)/global.h  \
    $(INC)/comm.h $(INC)/play.h \
    $(INC)/spells.h $(INC)/etc.h

guild.o: guild.c   \
    $(INC)/char.h $(INC)/typedef.h \
    $(INC)/object.h $(INC)/global.h \
    $(INC)/comm.h $(INC)/play.h \
    $(INC)/actions.h $(INC)/etc.h

handler.o: handler.c   \
    $(INC)/char.h $(INC)/typedef.h  \
    $(INC)/object.h $(INC)/global.h  \
    $(INC)/comm.h $(INC)/play.h $(INC)/etc.h

house.o: house.c   \
    $(INC)/char.h $(INC)/typedef.h  \
    $(INC)/object.h $(INC)/global.h  \
    $(INC)/comm.h $(INC)/gamedb.h \
    $(INC)/actions.h $(INC)/spells.h $(INC)/play.h

interpreter.o: interpreter.c  \
    $(INC)/char.h $(INC)/typedef.h \
    $(INC)/object.h $(INC)/global.h \
    $(INC)/comm.h \
    $(INC)/actions.h

library.o: library.c   \
    $(INC)/char.h $(INC)/typedef.h  \
    $(INC)/object.h \
    $(INC)/global.h  $(INC)/comm.h

magic.o: magic.c   \
    $(INC)/char.h $(INC)/typedef.h  \
    $(INC)/object.h $(INC)/global.h  \
    $(INC)/comm.h $(INC)/play.h \
    $(INC)/spells.h

mob_act.o: mob_act.c   \
    $(INC)/char.h $(INC)/typedef.h  \
    $(INC)/object.h $(INC)/global.h  \
    $(INC)/comm.h $(INC)/play.h \
    $(INC)/spells.h $(INC)/actions.h $(INC)/etc.h

mob_bal.o: mob_bal.c

point.o: point.c   \
    $(INC)/char.h $(INC)/typedef.h  \
    $(INC)/object.h $(INC)/global.h  \
    $(INC)/comm.h $(INC)/play.h

spec_mid.o: spec_mid.c   \
    $(INC)/char.h $(INC)/typedef.h \
    $(INC)/object.h $(INC)/global.h \
    $(INC)/comm.h $(INC)/play.h \
    $(INC)/actions.h $(INC)/gamedb.h

spec_misc.o: spec_misc.c   \
    $(INC)/char.h $(INC)/typedef.h  \
    $(INC)/object.h $(INC)/global.h  \
    $(INC)/comm.h $(INC)/play.h \
    $(INC)/actions.h $(INC)/spells.h $(INC)/etc.h

spec_mob.o: spec_mob.c   \
    $(INC)/char.h $(INC)/typedef.h  \
    $(INC)/object.h $(INC)/global.h  \
    $(INC)/comm.h $(INC)/play.h \
    $(INC)/actions.h $(INC)/spells.h

special.o: special.c   \
    $(INC)/char.h $(INC)/typedef.h  \
    $(INC)/object.h $(INC)/global.h  \
    $(INC)/comm.h $(INC)/actions.h $(INC)/etc.h

spell_parser.o: spell_parser.c  \
    $(INC)/char.h $(INC)/typedef.h \
    $(INC)/object.h $(INC)/global.h \
    $(INC)/comm.h \
    $(INC)/play.h $(INC)/spells.h

spell1.o: spell1.c   \
    $(INC)/char.h $(INC)/typedef.h  \
    $(INC)/object.h $(INC)/global.h  \
    $(INC)/comm.h $(INC)/play.h \
    $(INC)/spells.h

spell2.o: spell2.c   \
    $(INC)/char.h $(INC)/typedef.h \
    $(INC)/object.h $(INC)/global.h \
    $(INC)/comm.h $(INC)/play.h \
    $(INC)/spells.h $(INC)/etc.h

chpass.o: utils/chpass.c   \
    $(INC)/char.h $(INC)/typedef.h \
    $(INC)/gamedb.h

dangun.o: utils/dangun.c   \
    $(INC)/char.h $(INC)/typedef.h  \
    $(INC)/gamedb.h

dupname.o: utils/dupname.c   \
    $(INC)/char.h $(INC)/typedef.h \
    $(INC)/gamedb.h

list.o: utils/list.c 

player.o: utils/player.c   \
    $(INC)/char.h $(INC)/typedef.h \
    $(INC)/gamedb.h

purge.o: utils/purge.c   \
    $(INC)/char.h $(INC)/typedef.h \
    $(INC)/gamedb.h

test_skill.o: utils/test_skill.c
   
what_item_in_zone.o: utils/what_item_in_zone.c 
   
