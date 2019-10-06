#
# Makefile for Narai MUD src directory
# 	tested on Linux and SUN Solaris2.4
#			-by Cookie  Thu Sep 18 01:39:29 KST 1997

BIN 	= ../bin
PROG 	=  $(BIN)/mud

# To compile on SUN solaris with GCC, we have to use gmake

MAKE	= gmake	
CC	= gcc 
LD 	= $(CC)
MKDEP	= makedepend

#
# Use followings for SUN solaris in BSD compatibility mode.
#
INCLUDE	= -I../include #-I/usr/ucbinclude
DFLAGS  = -D__STDC__=0
# LFLAGS  = -L/usr/ucblib -lucb  -lsocket -lnsl
LFLAGS  = -lcrypt

# INCLUDE	= -I../include 
#DFLAGS	= -DHAVE_TERMIOS_H 
#DFLAGS	= 

# LFLAGS	= libdescrypt.a
# LFLAGS	= -lcrypt
# LFLAGS 		= 

OFLAGS	= -O -Wall -g -pipe
#
# MFLAGS: SYPARK RETURN_TO_QUIT RESTART_BONUS INIT_BARE_HAND GHOST
#	  OldMobileFormat New_STASH OLD_DAM_MESSAGE OLD_PARRY
#	  OLD_MIRROR_IMAGE RESET_ZONE_BY_WIZARD OldMobAct
#

# NOTE: -DIMO=41 moved to global.h
# NOTE:  -DSYPARK -DRETURN_TO_QUIT moved to db.player.c
# MFLAGS	= -DSAVE_DESC -DRETURN_TO_QUIT
MFLAGS	= 

CFLAGS	= $(OFLAGS) $(MFLAGS) $(DFLAGS) $(INCLUDE)

SRCS	= \
	act.comm.c act.info.c act.look.c act.move.c act.obj1.c \
	act.obj2.c act.offensive.c act.player.c act.special.c act.wizard.c \
	board.c cast1.c cast2.c comm.c constants.c \
	db.c db.player.c db.zone.c fight.c guild.c \
	handler.c interpreter.c library.c magic.c mob_act.c \
	mob_bal.c point.c spec_mid.c spec_mob.c spec_misc.c \
	special.c spell_parser.c spell1.c spell2.c \
	house.c

OBJS 	= $(SRCS:%.c=../obj/%.o)

# UTILS = list change_level check_pass change_pass cps
UTILS = chpass purge dangun
SCRS = everun backup RESTART RUNME shutmud
SCRIPTS = $(SCRS:%=./utils/%) 

USRCS = $(UTILS:%=utils/%.c)
# UOBJS 	= $(UTILS:%=../obj/%.o)
UPROGS 	= $(UTILS:%=$(BIN)/%)

ALL_SRCS = $(SRCS) $(USRCS) $(SCRIPTS) Makefile

../obj/%.o:  %.c
	$(CC) -c $(CFLAGS) -o $@ $<
../bin/%:  utils/%.c
	$(CC) $(CFLAGS) $(LFLAGS) -o $@ $<

default	: mud

all:	clean depend mud utils

mud :	$(PROG) 

depend	: 
	$(MKDEP) -Y -p ../obj/ $(MFLAGS) $(DFLAGS) $(INCLUDE) $(SRCS) $(USRCS)
	ctags $(SRCS) ../include/*.h 

$(PROG) : $(OBJS) 
	$(LD) -o $@ $^ $(LFLAGS)

utils : $(UPROGS) 
	- /bin/cp -p $(SCRIPTS) $(BIN)
	- /bin/ln -s $(UPROGS) ../users 

$(UPROGS) : $(USRCS)

install : $(PROG) utils
	- /bin/mv -f $(PROG).run $(PROG).bak
	- /bin/mv -f $(PROG) $(PROG).run

uninstall :

clean	:
	- /bin/rm -f $(OBJS) $(PROGNAME)
	- /bin/rm -f Makefile.bak tags core tmp* 

print	: $(ALL_SRCS)
	for f in $(ALL_SRCS) ; do \
	if [ "$$f" -nt .pr-time ] ; then \
		jjjps -F8 $$f | lpr ;\
	fi ; done ;
	date > .pr-time

# DO NOT DELETE

../obj/act.comm.o: ../include/char.h ../include/typedef.h ../include/object.h
../obj/act.comm.o: ../include/global.h ../include/comm.h ../include/actions.h
../obj/act.info.o: ../include/char.h ../include/typedef.h ../include/object.h
../obj/act.info.o: ../include/global.h ../include/comm.h ../include/play.h
../obj/act.info.o: ../include/gamedb.h
../obj/act.look.o: ../include/char.h ../include/typedef.h ../include/object.h
../obj/act.look.o: ../include/global.h ../include/comm.h ../include/play.h
../obj/act.look.o: ../include/actions.h
../obj/act.move.o: ../include/char.h ../include/typedef.h ../include/object.h
../obj/act.move.o: ../include/global.h ../include/comm.h ../include/play.h
../obj/act.move.o: ../include/etc.h
../obj/act.obj1.o: ../include/char.h ../include/typedef.h ../include/object.h
../obj/act.obj1.o: ../include/global.h ../include/comm.h
../obj/act.obj2.o: ../include/char.h ../include/typedef.h ../include/object.h
../obj/act.obj2.o: ../include/global.h ../include/comm.h ../include/play.h
../obj/act.obj2.o: ../include/actions.h
../obj/act.offensive.o: ../include/char.h ../include/typedef.h
../obj/act.offensive.o: ../include/object.h ../include/global.h
../obj/act.offensive.o: ../include/comm.h ../include/play.h
../obj/act.offensive.o: ../include/spells.h ../include/actions.h
../obj/act.player.o: ../include/char.h ../include/typedef.h
../obj/act.player.o: ../include/object.h ../include/global.h
../obj/act.player.o: ../include/comm.h ../include/gamedb.h ../include/etc.h
../obj/act.special.o: ../include/char.h ../include/typedef.h
../obj/act.special.o: ../include/object.h ../include/global.h
../obj/act.special.o: ../include/comm.h ../include/play.h ../include/spells.h
../obj/act.wizard.o: ../include/char.h ../include/typedef.h
../obj/act.wizard.o: ../include/object.h ../include/global.h
../obj/act.wizard.o: ../include/comm.h ../include/gamedb.h
../obj/act.wizard.o: ../include/actions.h ../include/etc.h
../obj/board.o: ../include/char.h ../include/typedef.h ../include/object.h
../obj/board.o: ../include/global.h ../include/comm.h ../include/actions.h
../obj/cast1.o: ../include/char.h ../include/typedef.h ../include/object.h
../obj/cast1.o: ../include/global.h ../include/comm.h ../include/play.h
../obj/cast1.o: ../include/spells.h
../obj/cast2.o: ../include/char.h ../include/typedef.h ../include/object.h
../obj/cast2.o: ../include/global.h ../include/comm.h ../include/play.h
../obj/cast2.o: ../include/spells.h
../obj/comm.o: ../include/char.h ../include/typedef.h ../include/object.h
../obj/comm.o: ../include/global.h ../include/comm.h ../include/gamedb.h
../obj/constants.o: ../include/char.h ../include/typedef.h
../obj/constants.o: ../include/global.h ../include/play.h
../obj/db.o: ../include/char.h ../include/typedef.h ../include/object.h
../obj/db.o: ../include/global.h ../include/comm.h ../include/play.h
../obj/db.o: ../include/gamedb.h
../obj/db.player.o: ../include/char.h ../include/typedef.h
../obj/db.player.o: ../include/object.h ../include/global.h ../include/comm.h
../obj/db.player.o: ../include/gamedb.h
../obj/db.zone.o: ../include/char.h ../include/typedef.h ../include/object.h
../obj/db.zone.o: ../include/global.h ../include/comm.h ../include/gamedb.h
../obj/fight.o: ../include/char.h ../include/typedef.h ../include/object.h
../obj/fight.o: ../include/global.h ../include/comm.h ../include/play.h
../obj/fight.o: ../include/spells.h ../include/etc.h
../obj/guild.o: ../include/char.h ../include/typedef.h ../include/object.h
../obj/guild.o: ../include/global.h ../include/comm.h ../include/play.h
../obj/guild.o: ../include/actions.h ../include/etc.h
../obj/handler.o: ../include/char.h ../include/typedef.h ../include/object.h
../obj/handler.o: ../include/global.h ../include/comm.h ../include/play.h
../obj/handler.o: ../include/etc.h
../obj/interpreter.o: ../include/char.h ../include/typedef.h
../obj/interpreter.o: ../include/object.h ../include/global.h
../obj/interpreter.o: ../include/comm.h ../include/actions.h
../obj/library.o: ../include/char.h ../include/typedef.h ../include/object.h
../obj/library.o: ../include/global.h ../include/comm.h
../obj/magic.o: ../include/char.h ../include/typedef.h ../include/object.h
../obj/magic.o: ../include/global.h ../include/comm.h ../include/play.h
../obj/magic.o: ../include/spells.h
../obj/mob_act.o: ../include/char.h ../include/typedef.h ../include/object.h
../obj/mob_act.o: ../include/global.h ../include/comm.h ../include/play.h
../obj/mob_act.o: ../include/spells.h ../include/actions.h ../include/etc.h
../obj/point.o: ../include/char.h ../include/typedef.h ../include/object.h
../obj/point.o: ../include/global.h ../include/comm.h ../include/play.h
../obj/spec_mid.o: ../include/char.h ../include/typedef.h ../include/object.h
../obj/spec_mid.o: ../include/global.h ../include/comm.h ../include/play.h
../obj/spec_mid.o: ../include/actions.h ../include/gamedb.h
../obj/spec_mob.o: ../include/char.h ../include/typedef.h ../include/object.h
../obj/spec_mob.o: ../include/global.h ../include/comm.h ../include/play.h
../obj/spec_mob.o: ../include/actions.h ../include/spells.h
../obj/spec_misc.o: ../include/char.h ../include/typedef.h
../obj/spec_misc.o: ../include/object.h ../include/global.h ../include/comm.h
../obj/spec_misc.o: ../include/play.h ../include/actions.h
../obj/spec_misc.o: ../include/spells.h ../include/etc.h
../obj/special.o: ../include/char.h ../include/typedef.h ../include/object.h
../obj/special.o: ../include/global.h ../include/comm.h ../include/actions.h
../obj/special.o: ../include/etc.h
../obj/spell_parser.o: ../include/char.h ../include/typedef.h
../obj/spell_parser.o: ../include/object.h ../include/global.h
../obj/spell_parser.o: ../include/comm.h ../include/play.h
../obj/spell_parser.o: ../include/spells.h
../obj/spell1.o: ../include/char.h ../include/typedef.h ../include/object.h
../obj/spell1.o: ../include/global.h ../include/comm.h ../include/play.h
../obj/spell1.o: ../include/spells.h
../obj/spell2.o: ../include/char.h ../include/typedef.h ../include/object.h
../obj/spell2.o: ../include/global.h ../include/comm.h ../include/play.h
../obj/spell2.o: ../include/spells.h ../include/etc.h
../obj/house.o: ../include/char.h ../include/typedef.h ../include/object.h
../obj/house.o: ../include/global.h ../include/comm.h ../include/gamedb.h
../obj/house.o: ../include/actions.h ../include/spells.h ../include/play.h
../obj/utils/chpass.o: ../include/char.h ../include/typedef.h
../obj/utils/chpass.o: ../include/gamedb.h
../obj/utils/purge.o: utils/player.c ../include/char.h ../include/typedef.h
../obj/utils/purge.o: ../include/gamedb.h
../obj/utils/dangun.o: ../include/char.h ../include/typedef.h
../obj/utils/dangun.o: ../include/gamedb.h
