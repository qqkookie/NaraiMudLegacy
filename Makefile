#
# 	Makefile for Narai MUD
#
#	This is Top dir Makefile. See also src/Makefile 
#			- by Cookie 

DISTFILE=CHANGES Makefile README include lib src

#  To use GCC on SUN Solaris, use gmake
MAKE	= gmake

no_default_action:
	
clean	:
	- rm -f Makefile.bak obj/* lib/core 
	- (cd src ; $(MAKE) clean )

all :	

install :
	- mkdir obj bin users
	- chmod 700 obj bin users .
	- (cd src ; $(MAKE) install )	

dist	: clean
	( cd .. ; tar cf nar-dist.tar $(DISTFILE:%=narai/%) )
	gzip ../nar-dist.tar

RESTART: 
	- /bin/mv  users users.old
	- /bin/cp src/utils/RESTART .
	/bin/sh RESTART


