#
# Makefile for Narai MUD
#
#

CC	= gcc

INCLUDE	= -I../include -I/usr/local/include
WFLAGS	= -Wall
OFLAGS	= -O
DEBUG	= -g
#
# MFLAGS: SYPARK RETURN_TO_QUIT RESTART_BONUS INIT_BARE_HAND GHOST
#	  OldMobileFormat New_STASH OLD_DAM_MESSAGE OLD_PARRY
#	  OLD_MIRROR_IMAGE RESET_ZONE_BY_WIZARD OldMobAct
#
MFLAGS	= -DIMO=41 -DSYPARK -DRETURN_TO_QUIT -DRESTART_BONUS

DFLAGS	= -DHAVE_TERMIOS_H 

CFLAGS	= $(INCLUDE) $(WFLAGS) $(OFLAGS) $(DEBUG) $(DFLAGS) $(MFLAGS) -pipe
LFLAGS	= -lcrypt 

.SRCDIR	= $(.CURDIR)/src
.OBJDIR	= $(.CURDIR)/obj
.BINDIR	= $(.CURDIR)/bin

SRCS	= act.comm.c act.informative.c act.movement.c act.obj1.c act.obj2.c \
	act.offensive.c act.other.c act.social.c act.wizard.c board.c comm.c \
	constants.c db.c fight.c guild_command.c guild_constant.c \
	guild_informative.c guild_skills.c handler.c interpreter.c limit.c \
	magic.c magic2.c magic3.c magic_weapon.c mail.c mob_magic.c mobact.c \
	modify.c newcom.c quest.c shop.c spec_assign.c spec_procs.c \
	spec_procs2.c spec_procs3.c spell_parser.c spells1.c spells2.c \
	spells3.c utility.c version.c weather.c daerimsa.c gbisland.c

UTILS	= list.c change_level.c check_pass.c change_pass.c cps.c

OBJS	= $(SRCS:S/.c$/.o/g)
UOBJS	= $(UTILS:S/.c$/.o/g)
UFILES	= $(UTILS:S/^/$(.CURDIR)\/utils\//g)

CFILES	= $(SRCS:S/^/$(.SRCDIR)\//g)
OFILES	= $(OBJS:S/^/$(.OBJDIR)\//g) $(UOBJS:S/^/$(.OBJDIR)\//g)
OFILES_new	= $(OBJS:S/^/$(.OBJDIR)\//g)

default	: dms

#	all	: clean depend dms utils bin
all	: clean_new depend dms bin_new

dep	: depend
depend	: check_objdir
	mkdep $(CFLAGS) $(MFLAGS) $(CFILES)
	mkdep -a $(CFLAGS) $(UFILES)

dms	: check_objdir $(OBJS)
	$(CC)	-o $(.BINDIR)/nms.tmp $(CFLAGS) $(OBJS) $(LFLAGS)

utils	: list change_level check_pass change_pass cps

#
# Utilities
#
list	: list.o
	$(CC)	$(CFLAGS) -s -o $(.BINDIR)/list list.o 

change_level: change_level.o
	$(CC)	$(CFLAGS) -s -o $(.BINDIR)/change_level change_level.o 

check_pass: check_pass.o
	$(CC)	$(CFLAGS) -s -o $(.BINDIR)/check_pass check_pass.o -lcrypt 

change_pass: change_pass.o
	$(CC)	$(CFLAGS) -s -o $(.BINDIR)/change_pass change_pass.o -lcrypt

cps: cps.o
	$(CC)	$(CFLAGS) -s -o $(.BINDIR)/cps cps.o -lcrypt
#
#

check_objdir:
	@if ! test -d $(.OBJDIR); then \
		mkdir -p $(.OBJDIR); \
		if ! test -d $(.OBJDIR); then \
			echo "Unable to create $(.OBJDIR)."; \
		fi; \
		echo "OBJDIR $(.OBJDIR) created."; \
	fi


clean	:
	/bin/rm $(OFILES)

clean_new	:
	rm $(.CURDIR)/nms.tmp
	/bin/rm $(OFILES_new)

bin_new	:
	mv $(.BINDIR)/nms.tmp $(.CURDIR)/nms.tmp
	
bin	:
	mv $(.BINDIR)/nms.tmp $(.CURDIR)/nms.tmp