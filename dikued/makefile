CC = gcc
CFLAGS = -c  -ggdb
HEADERFILES = strext.h dutil.h iedit.h medit.h redit.h zedit.h dikusim.h
dikued : strext.o dutil.o iedit.o medit.o redit.o zedit.o dikusim.o dikucli.o  
strext.o : strext.c $(HEADERFILES)
	$(CC) $(CFLAGS) strext.c
dutil.o : dutil.c $(HEADERFILES)
	$(CC) $(CFLAGS) dutil.c
iedit.o : iedit.c $(HEADERFILES)
	$(CC) $(CFLAGS) iedit.c
medit.o : medit.c $(HEADERFILES)
	$(CC) $(CFLAGS) medit.c
redit.o : redit.c $(HEADERFILES)
	$(CC) $(CFLAGS) redit.c
zedit.o : zedit.c $(HEADERFILES)
	$(CC) $(CFLAGS) zedit.c
dikusim.o : dikusim.c $(HEADERFILES)
	$(CC) $(CFLAGS) dikusim.c
dikucli.o : dikucli.c $(HEADERFILES)
	$(CC) $(CFLAGS) dikucli.c
dikued :  strext.c dutil.c iedit.c medit.c redit.c dikucli.c $(HEADERFILES) \
	dikusim.c zedit.c
	$(CC)   -o dikued strext.o dutil.o iedit.o medit.o redit.o dikucli.o \
		zedit.o dikusim.o

clean: 
	-rm dikued *.o
