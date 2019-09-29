# Build kit mud

.SUFFIXES: .c

%.o: %.c
	$(CC) -c $(CFLAG) $< $(INCL)
%.d: %.c
	@echo generating dependency for $<
	@ $(SHELL) -ec 'gcc -M $(CPPFLAGS) $< | sed '\''s/$*\.o/& $@/g'\'' > $@'

CC= cc
CFLAG= -O -DIMO=41 -ggdb
INCL = 
LIBS= -lcrypt

SRCS = $(wildcard *.c)
OBJS = $(SRCS:.c=.o)
DEPN = $(SRCS:.c=.d)

all: kit

kit: $(OBJS)
	$(CC) -o ../$@ $(OBJS) $(LIBS)

clean:
	-rm $(OBJS) 

veryclean:
	-rm $(OBJS) $(DEPN) kit

tags:
	etags $(SRCS) *.h

include $(DEPN)
