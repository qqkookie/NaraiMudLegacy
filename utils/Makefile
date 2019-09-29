#
#  Makefile 
#

INC = ../header
CC=cc 
CFLAGS = -g -I$(INC) -DIMO=41 

list : list.c $(INC)/structs.h
	$(CC) $(CFLAGS) -s -o list list.c

syn : syntax_check.c $(INC)/structs.h $(INC)/utils.h $(INC)/db.h \
	$(INC)/comm.h $(INC)/handler.h $(INC)/limits.h
	$(CC) $(CFLAGS) -s -o syn syntax_check.c

select : select.c $(INC)/structs.h
	$(CC) $(CFLAGS) -s -o select select.c

manage: manage.c $(INC)/structs.h $(INC)/utils.h $(INC)/spells.h $(INC)/db.h
	$(CC) $(CFLAGS) -o manage manage.c

delete: del_player.c $(INC)/structs.h $(INC)/utils.h $(INC)/spells.h $(INC)/db.h
	$(CC) $(CFLAGS) -o delete del_player.c

show: show.c $(INC)/structs.h $(INC)/utils.h $(INC)/spells.h $(INC)/db.h
	$(CC) $(CFLAGS) -o show show.c

chname: chname.c $(INC)/structs.h $(INC)/utils.h $(INC)/spells.h $(INC)/db.h
	$(CC) $(CFLAGS) -o chname chname.c

del2: del2.c $(INC)/structs.h $(INC)/utils.h $(INC)/spells.h $(INC)/db.h
	$(CC) $(CFLAGS) -o del2 del2.c

reviv: reviv.c $(INC)/structs.h $(INC)/utils.h $(INC)/spells.h $(INC)/db.h
	$(CC) $(CFLAGS) -o reviv reviv.c

portable : portable.c $(INC)/structs.h
	$(CC) $(CFLAGS) -o portable portable.c

transfer: transfer.c
	$(CC) -o transfer transfer.c

obj_check : obj_check.o cost.o $(INC)/structs.h
	$(CC) $(CFLAGS) -o obj_check obj_check.o cost.o

obj_check.o : obj_check.c $(INC)/structs.h

cost.o : cost.o $(INC)/structs.h

mob_check : mob_check.c $(INC)/structs.h $(INC)/utils.h
	$(CC) $(CFLAGS) -o mob_check mob_check.c

mob_check2 : mob_check2.c $(INC)/structs.h $(INC)/utils.h
	$(CC) $(CFLAGS) -o mob_check2 mob_check2.c -lm

std2f : std_to_file.c $(INC)/structs.h
	$(CC) $(CFLAGS) -o std2f std_to_file.c

f2asc : file_to_ascii.c $(INC)/structs.h
	$(CC) $(CFLAGS) -o f2asc file_to_ascii.c

asc2f : ascii_to_file.c $(INC)/structs.h
	$(CC) $(CFLAGS) -o asc2f ascii_to_file.c

cliso : cliso.c
	$(CC) $(CFLAGS) -o cliso cliso.c

tclifork : tclifork.c
	$(CC) $(CFLAGS) -o tclifork tclifork.c

cliso3 : cliso3.o filter.o
	$(CC) $(CFLAGS) -o cliso3 cliso3.o filter.o

cliso4 : cliso4.o filter.o
	$(CC) $(CFLAGS) -o cliso4 cliso4.o filter.o

ara : ara.o filter.o
	$(CC) $(CFLAGS) -o ara ara.o filter.o

#  END OF MAKE FILE 
