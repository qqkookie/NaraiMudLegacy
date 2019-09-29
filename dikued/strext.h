/****************************************************************************
 *                                                                          *
 *          strext.h (c) by Georg Essl in 1992                              *
 *                                                                          *
 ****************************************************************************/

/*** DEFINES ***/

/* For: strpcmp() */

#define SPC_EQUAL      0          /* same as strcmp() in strings.h */
#define SPC_NOTEQUAL   1
#define SPC_WILDIGNORE '!'        /* Ignore Wildchar Char */
#define SPC_WILDSCHAR  '?'        /* Wildchar for single Char */
#define SPC_WILDMCHAR  '*'        /* Wildchar for multiple Chars (+none) */
#define SPC_WILDOBRA   '['        /* Wildchar for some Chars open bracket */
#define SPC_WILDCBRA   ']'        /* Wildchar for some Chars close bracket */

/*** Prototypes ***/

int strpcmp(char *, char *);      /* Prototype for strpcmp() */
