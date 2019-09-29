/****************************************************************************
 *                                                                          *
 *          strext.c (c) by Georg Essl in 1992                              *
 *                                                                          *
 ****************************************************************************/


#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "strext.h"

/****************************************************************************
 *                                                                          *
 *                     int strpcmp(char * , char *)                         *
 *                                                                          *
 * Author : Georg Essl                                                      *
 * Date   : 19.3.1992                                                       *
 *                                                                          *
 * Usage:                                                                   *
 *   Argument 2 (String 2) is compared with a WILDCARD-patterned string     *
 *   in Arument 1 (String 1).                                               *
 *                                                                          *
 * Wildcards:                                                               *
 *   * .... (GREP-Like) Representing any amount of chars including zero.    *
 *   ? .... (Like '.' in GREP) Representing exactly one char.               *
 *   [AB].. (GREP-Like) Compare with A and B for match                      *
 *   ! .... Next char is NOT a Wildcard.                                    *
 *                                                                          *
 *   Any other Char will be compared like with strcmp() from STRING.H       *
 *                                                                          *
 * Bugs: None.                                                              *
 *                                                                          *
 * Note: Not Wildcardchar-cmp possibility given yet.                        *
 *       strpcmp == 0 if String1 Wildcard-equal to String2                  *
 *       strpcmp != 0 if not.                                               *
 *       This is same as strcmp() does, but strpcmp does NOT a              *
 *       greater/lesser-compare!                                            *
 *                                                                          *
 ****************************************************************************/

int strpcmp(char *wstr1 , char *wstr2)
{
  for(;;)
  {
    switch(*wstr1)
    { case '\0' : if(*wstr2=='\0') return(SPC_EQUAL);
		  else
		    return(SPC_NOTEQUAL);
      case SPC_WILDSCHAR :
		  if((*wstr2)!='\0')
		  {
		    wstr1++;
		    wstr2++;
		  }
                  else
		    return(SPC_NOTEQUAL);
                  break;
      case SPC_WILDMCHAR :
		  if ((*(wstr1+1))=='\0') return(SPC_EQUAL);
		  while(strpcmp(wstr1+1,wstr2))
                  {
                    wstr2++;
		    if(*wstr2=='\0') return(SPC_NOTEQUAL);
                  }
		  return(SPC_EQUAL);
      case SPC_WILDOBRA :
		  wstr1++;
		  while((*wstr1!='\0')&&(*wstr1!=SPC_WILDCBRA)&&(*wstr1!=*wstr2))
		    wstr1++;
		  if(*wstr1==*wstr2)
		  {
		    do
		      wstr1++;
		    while((*wstr1!='\0')&&(*wstr1!=']'));
		    if(*wstr1==']') wstr1++;
		    wstr2++;
		    break;
		  }
		  else
		    return(SPC_NOTEQUAL);
      case SPC_WILDIGNORE :
		  wstr1++;
      default : if(*wstr1 == *wstr2)
		{
		  wstr1++;
		  wstr2++;
		}
		else
		  return(SPC_NOTEQUAL);
    }
  }
}
