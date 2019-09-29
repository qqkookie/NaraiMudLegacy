
#define ISLET(c) ( c > ' ' || c < 0 )
main(int argc, char **argv)
{
        char *cp ;
        cp = argv[1];

                printf(" before  1 %d:\n",cp ) ;
        while(isspace(*cp)) cp++;
                printf(" aftert 1 %d\n", cp ) ;
                printf(" let = %c   %d(0x%x)\n", *cp, *cp, *cp ) ;

        cp = argv[1];
                printf(" before  2 %d:\n",cp ) ;
        while(!ISLET(*cp)) cp++;
                printf(" aftert  2 %d\n", cp ) ;
                printf(" let = %c   %d(0x%x)\n", *cp, *cp, *cp ) ;
        if (*cp == '\0' || strncmp(cp, "/last", 5) == 0) {
                printf(" last chat :\n" ) ;
                return;
        }
        printf(" normal hat :%s\n",argv[1] ) ;
}
