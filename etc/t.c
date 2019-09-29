
/* ************************************************************************
   *  file: comm.c , Communication module.                   Part of DIKUMUD *
   *  Usage: Communication, central game loop.                               *
   *  Copyright (C) 1990, 1991 - see 'license.doc' for complete information. *
   *  All Rights Reserved                                                    *
   *  Using *any* part of DikuMud without having read license.doc is         *
   *  violating our copyright.
   ************************************************************************* */

#include <errno.h>
#include <stdio.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <string.h>
#include <sys/time.h>
#include <fcntl.h>
#include <signal.h>
#include <setjmp.h>
#include <unistd.h>

int main(int argc, char **argv)
{
    char line[256], *ads; 
    struct in_addr  addr;
    struct hostent  *h_ent; 

	/* NOTE: Show domain name instead of numeric IP addr, if possible.*/
	ads = "143.248.1.177";
	addr = inet_makeaddr(inet_network(ads),inet_addr(ads));
#ifdef NO_DEF
	if ( inet_aton(d->host, &addr ) 
	    && ( h_ent = gethostbyaddr((char*) &addr, sizeof(addr), AF_INET))) 
#endif
	if ( h_ent = gethostbyaddr((char*) &addr, sizeof(addr), AF_INET)) 
	    strcpy(line, h_ent->h_name );
	else
	    strcpy(line , ads);
	puts(line);

}
