#
# 	Makefile for KIT Classic MUD
#
#	This is Top dir Makefile. See also src/Makefile 
#			- by Cookie 

DISTFILE=CHANGE.md Makefile README.md VERSION lib src
LIBFILE=actions help lpaints tinyworld.mob \
	tinyworld.obj tinyworld.shp world zone

#  To use GCC on SUN Solaris, use gmake
MAKE	= make

no_default_action:
	
clean	:
	- rm -f Makefile.bak users/core mud-*.pid 
	- (cd src ; $(MAKE) clean )

all :	

install :
	- mkdir src/obj bin users
	- chmod 700 bin users .
	- (cd src ; $(MAKE) install )	

dist	: clean
	( cd .. ; tar cf kit-dist.tar $(DISTFILE:%=narai/%) )
	gzip ../nar-dist.tar

