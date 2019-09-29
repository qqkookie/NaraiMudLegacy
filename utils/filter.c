/*
	filter.c - filtering noise
*/
#include <stdio.h>
#define	TELCMDS
#include <arpa/telnet.h>

/* in dal1, do not support telcmd_ok() */
#ifdef DAL1
#define	TELCMD_OK(x)	((x) <= IAC && (x) >= EOR)
#endif

#define	STATE_READY	0
#define	STATE_EOF	1
#define	STATE_H1	2
#define	STATE_END	3

#define	CH_EOF	0
#define	CH_H12	1
#define	CH_H2	2
#define	CH_ASC	3
#define	CH_NOISE	4
#define	CH_END	5

unsigned char	tmp_ch ;	/* temporary */
int get_ch(char *s, int l, int *cur, unsigned char *ch) ;
void unget_ch(char *s, int l, int *cur) ;

int hangul_filter2(char *filterd, int length, char *str)
{
	int	state, current, ctype ;
	char	*filsave ;
	unsigned char	ch ;

	filsave = filterd ;	/* save current pointer */
	current = 0 ;

	state = STATE_READY ;
	while( state != STATE_END ) {
		switch(state) {
		case STATE_READY :
			ctype = get_ch(str, length, &current, &ch) ;
			switch( ctype ) {
				case CH_EOF :
					tmp_ch = ch ; state = STATE_EOF ;
					break ;
				case CH_H12 :
					tmp_ch = ch ; state = STATE_H1 ;
					break ;
				case CH_H2 :
					state = STATE_READY ;
					break ;
				case CH_ASC :
					*filterd++ = ch ;
					state = STATE_READY ;
					break ;
				case CH_NOISE :
					state = STATE_READY ;
					break ;
				case CH_END :
					*filterd = NULL ;
					state = STATE_END ;
					break ;
				}
			break ;	/* of state ready */
		case STATE_EOF :
			ctype = get_ch(str, length, &current, &ch) ;
			if ( TELCMD_OK(ch) ) {
				*filterd++ = tmp_ch ;
				*filterd++ = ch ;
				if ( ch >= WILL && ch <= DONT ) {
					ctype = get_ch(str, length, &current, &ch) ;
					*filterd++ = ch ;
					}
				state = STATE_READY ;
				}
			else {
				unget_ch(str, length, &current) ;
				state = STATE_READY ;
				}
			break ;	/* of state eof */
		case STATE_H1 :
			ctype = get_ch(str, length, &current, &ch) ;
			switch ( ctype ) {
				case CH_EOF :
					/* ignore tmp_ch */
					tmp_ch = EOF ;	state = STATE_EOF ;
					break ;
				case CH_H12 :
				case CH_H2 :
					*filterd++ = tmp_ch ;
					*filterd++ = ch ;
					state = STATE_READY ;
					break ;
				case CH_ASC :
					/* ignore tmp_ch */
					*filterd++ = ch ;
					state = STATE_READY ;
					break ;
				case CH_NOISE :
					/* ignore current ch */
					state = STATE_H1 ;
					break ;
				case CH_END :
					/* ignore tmp_ch */
					*filterd = NULL ;
					state = STATE_END ;
					break ;
				}
			break ;	/* of state h1 */
		}	/* end of switch state */
	}	/* end of while */

	/* return number of character transformed */
	return ( filterd - filsave ) ;
}

int get_ch(char *str, int length, int *point, unsigned char *ch)
{
	int	ctype ;
	unsigned char	tch ;

	if ( *point >= length )
		return CH_END ;

	*ch = tch = (unsigned char) str[*point] ;
	(*point) ++ ;

	if ( tch == 255 )
		return CH_EOF ;

	if (( tch >= ' ' && tch <= '~') || tch == '\n' || tch == '\r'
		|| tch == '\t' )
		return CH_ASC ;

	if ( tch >= 160 ) {
		if ( tch <= 200 )
			return CH_H12 ;
		if ( tch <= 254 )
			return CH_H2 ;
		}

	return CH_NOISE ;
}

void unget_ch(char *str, int leng, int *point)
{
	if ( *point > 0 )
		(*point) -- ;
}
