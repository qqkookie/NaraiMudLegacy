/*
	tclifork.c - telnet service program
*/
#include <errno.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <netinet/in.h>
#include <netdb.h>
#include <string.h>
#include <arpa/telnet.h>
#include <sys/time.h>
#include <fcntl.h>
#include <sys/signal.h>

#define	MAX_MSG_LENGTH	6144
#define	MAX_HOSTNAME	255

#define	TIME_OUT_SEC	0
#define	TIME_OUT_USEC	100000

#define	REMOTE_HOST	"143.248.1.6"
#define	REMOTE_PORT	4001

#define	LOCAL_HOST	"localhost"
#define	LOCAL_PORT	4001

/* check bits : for filtering */
#define	CH_UNKNOWN	0
#define	CH_NORMAL_CODE	1
#define	CH_HAN_1_AND_2	2
#define	CH_HAN_ONLY_2	3
#define	CH_NOISE	4
#define	CH_SPACES	5

/* last bits : for filtering */
#define	LAST_NORMAL	0
#define	LAST_HAN_FIRST	1
#define	LAST_HAN_SECOND	2
#define	LAST_SPACES	3

int typecheck(unsigned char ch) ;
void hangul_filter(char *res, char *str) ;

void log(char *msg) ;
void	quit(), quit_int(), quit_pipe() ;
char	*quit_msg = "Unknown" ;
int	debugflag = 0 ;

main()
{
	int	status ;
	int	local_mother_s, remote_s, local_s ;
	fd_set	in_set, out_set, exc_set ;

	log("init signal") ;
	init_signal() ;

	log("open local mother socket") ;
	if ((local_mother_s = open_mother_socket()) < 0 ) {
		fprintf(stderr, "Can't open local port\n") ;
		exit(1) ;
		}

	if ((local_s = get_new_connection(local_mother_s)) < 0 ) {
		fprintf(stderr, "Can't get new connection from localhost\n") ;
		exit(1) ;
		}

	if ((remote_s = open_remote_connection()) < 0 ) {
		fprintf(stderr, "Can't open remote port\n") ;
		send_to_local("Can't reach to server host\n", local_s) ;
		close(local_s) ;
		exit(1) ;
		}

	serving_loop(remote_s, local_s) ;
	log("End client") ;
}


/* return mother socket descriptor */
int open_mother_socket()
{
	int s;
	char *opt;
	char hostname[MAX_HOSTNAME+1];
	struct sockaddr_in sa;
	struct hostent *hp;
	struct linger ld;
	char	buf[BUFSIZ] ;

	bzero(&sa, sizeof(struct sockaddr_in));
	/*
		gethostname(hostname, MAX_HOSTNAME);
		log(hostname);
		hp = gethostbyname(hostname);
	*/
	sprintf(buf, "Local host (%s)", LOCAL_HOST) ;
	log(buf) ;
	hp = gethostbyname(LOCAL_HOST);
	if (hp == NULL) {
		perror("gethostbyname");
		return (-1) ;
		}

	sa.sin_family = hp->h_addrtype;
	sa.sin_port  = htons(LOCAL_PORT);

	log("init socket") ;
	if ((s = socket(AF_INET, SOCK_STREAM, 0)) < 0 ) {
		perror("Init-socket");
		return (-1) ;
		}

	if (setsockopt (s, SOL_SOCKET, SO_REUSEADDR,
		(char *) &opt, sizeof (opt)) < 0) {
		perror ("setsockopt REUSEADDR");
		return (-1) ;
		}
	ld.l_onoff = 0;
	ld.l_linger = 0;
	if (setsockopt(s, SOL_SOCKET, SO_LINGER, &ld, sizeof(ld)) < 0) {
		perror("setsockopt LINGER");
		return (-1) ;
		}

	log("binding") ;
	if (bind(s,(struct sockaddr *) &sa, sizeof(sa)) < 0) {
		perror("bind");
		close(s);
		return (-1) ;
		}
	listen(s, 3);

	return(s);
}

int get_new_connection(int mother_s)
{
	fd_set	in_set, out_set, exc_set ;
	int	new_s, new_length ;
	int	status ;
	struct sockaddr_in	new_addr ;
	char	buf[BUFSIZ] ;

	while(1) {
		if ( check_socket(mother_s, &in_set, &out_set, &exc_set) < 0 ) {
			log("get_new_connection: local socket error") ;
			return (-1) ;
			}
		if ( FD_ISSET(mother_s, &in_set)) {
			/* new connection */
			new_length = sizeof(new_addr) ;
			new_s = accept(mother_s, (struct sockaddr *)&new_addr, &new_length);
			sprintf(buf, "New connection from %s (d=%d)",
				(char *)inet_ntoa(new_addr.sin_addr), new_s);
			log(buf) ;
			status = fork() ;
			if (status < 0) {	/* fork fail */
				send_to_local("MAX DESCRIPTOR: Can't alloc anymore.\n", new_s) ;
				close(new_s) ;
				continue ;
				}
			else if ( status == 0 ) {	/* children */
				send_to_local("CONNECTED\n", new_s) ;
				return new_s ;
				}
			else {	/* parent */
				close(new_s) ;
				continue ;	/* waiting */
				}
			}
	}

	return NULL ;
}

serving_loop(int remote_s, int local_s)
{
	fd_set	in_set, out_set, exc_set ;
	char	msg[MAX_MSG_LENGTH], filmsg[MAX_MSG_LENGTH] ;

	bzero(msg, MAX_MSG_LENGTH) ;

	while (1) {
		/* check remote socket */
		if ( check_socket(remote_s, &in_set, &out_set, &exc_set) < 0 ) {
			close(remote_s) ;
			log("serving_loop: remote socket error") ;
			return (-1) ;
			}
		if ( FD_ISSET(remote_s, &exc_set)) {
			/* connection closed from remote host  */
			send_to_local("Rejected from remote host\n", local_s) ;
			return (-1) ;
			}
		if ( FD_ISSET(remote_s, &in_set)) {
			/* message arrived */
			msg[0] = NULL ;
			if ( read_remote_connection(msg, remote_s) < 0 ) {
				send_to_local("Connection close by remote host\n", local_s) ;
				return (-1) ;
				}
			if (debugflag)
				printf("REMOTE IN_SET: %s%%\n", msg) ;
			send_to_local(msg, local_s) ;
			}
		/*
		if ( FD_ISSET(remote_s, &out_set)) {
			fprintf(stderr, "REMOTE OUT_SET:\n") ;
			}
		*/

		/* check local socket */
		if ( check_socket(local_s, &in_set, &out_set, &exc_set) < 0 ) {
			fprintf(stderr, "serving_loop: local socket error") ;
			return (-1) ;
			}
		if ( FD_ISSET(local_s, &exc_set)) {
			/* connection closed from local host  */
			/* close this socket */
			log("serving_loop: user lost connection") ;
			return (-1) ;
			}
		if ( FD_ISSET(local_s, &in_set)) {
			/* message arrived from user : sent it to remote host */
			if ( get_from_local(msg, local_s) < 0 ) {
				log("serving_loop: closing by user") ;
				return (-1) ;
				}
			if (debugflag)
				printf("LOCAL IN_SET: %s%%\n", msg) ;
			hangul_filter(filmsg, msg) ;
			send_remote_connection(filmsg, remote_s) ;
			}
		/*
		if ( FD_ISSET(local_s, &out_set)) {
			fprintf(stderr, "OUT_SET: ") ;
			}
		*/
		}

	return NULL ;
}

int check_socket(int s, fd_set *in_set, fd_set *out_set, fd_set *exc_set)
{
	int	maxdesc ;
	struct timeval	null_time, time_out ;

	null_time.tv_sec = 0 ;
	null_time.tv_usec = 0 ;
	time_out.tv_sec = TIME_OUT_SEC ;
	time_out.tv_usec = TIME_OUT_USEC ;

	maxdesc = s ;

	FD_ZERO(in_set) ;
	FD_ZERO(out_set) ;
	FD_ZERO(exc_set) ;
	FD_SET(s, in_set) ;
	FD_SET(s, out_set) ;
	FD_SET(s, exc_set) ;
	null_time.tv_sec = 0 ;
	null_time.tv_usec = 0 ;
	if ( select(maxdesc + 1, in_set, out_set, exc_set, &null_time) < 0 ) {
		perror("Selection poll") ;
		return (-1) ;
		}
	time_out.tv_sec = TIME_OUT_SEC ;
	time_out.tv_usec = TIME_OUT_USEC ;
	if ( select(0, (fd_set *)0, (fd_set *)0, (fd_set *)0, &time_out)< 0) {
		perror("Select sleep") ;
		return (-1) ;
		}

	return NULL ;
}

int open_remote_connection()
{
	int	s ;
	struct sockaddr_in	serv_addr ;
	char	ch[1], line[BUFSIZ], buf[BUFSIZ];

	bzero((char *)&serv_addr, sizeof(serv_addr)) ;
	serv_addr.sin_family = AF_INET ;
	serv_addr.sin_addr.s_addr = inet_addr(REMOTE_HOST) ;
	serv_addr.sin_port = htons(REMOTE_PORT) ;

	if ((s = socket(AF_INET, SOCK_STREAM, 0)) < 0 ) {
		log("open_remote_connection: socket open failed") ;
		return (-1) ;
		}

	if (connect(s, (struct sockaddr_in *)&serv_addr, sizeof(serv_addr))<0){
		log("open_remote_connection: connect failed") ;
		return (-1) ;
		}

	/* connected to remote host */

	return (s) ;
}

int send_remote_connection(char *msg, int s)
{
	int	length ;

	if ( msg == NULL )
		return (-1) ;

	length = strlen(msg) ;
	if (write(s, msg, length) < 0) {
		log("send_remote_connection: write failed") ;
		quit_msg = "Write failed" ;
		quit();
		}

	return NULL ;
}

int read_remote_connection(char *msg, int s)
{
	int	nread ;
	if ( msg == NULL )
		return NULL ;

	if ((nread = read(s, msg, MAX_MSG_LENGTH -1)) < 0) {
		log("read_remote_connection: read failed") ;
		return (-1) ;
		}

	msg[nread] = NULL ;
	return NULL ;
}

int send_to_local(char *msg, int s)
{
	int	length ;

	length = strlen(msg) ;
	if (write(s, msg, length) < 0) {
		log("send_to_local: write failed") ;
		quit_msg = "Write failed" ;
		quit();
		}
}

int get_from_local(char *msg, int s)
{
	int	nread ;

	if ((nread = read(s, msg, MAX_MSG_LENGTH -1)) < 0) {
		log("get_from_local: read failed") ;
		return (-1) ;
		}
	msg[nread] = NULL ;

	return NULL ;
}

init_signal()
{
	signal(SIGINT, quit_int);
	signal(SIGPIPE, quit_pipe);
}

void quit_int()
{
	quit_msg = "Interrupt : Exit" ;
	quit() ;
}

void quit_pipe()
{
	quit_msg = "Pipe error : Exit" ;
	quit() ;
}

void quit()
{
	int	status ;

	log(quit_msg) ;
	wait(&status) ;
	exit(status) ;
}

void log(char *msg)
{
	long	ct;
	char	*str ;
	static int count = 0 ;

	ct = time(0) ;
	str = asctime(localtime(&ct)) ;
	if ( count++ % 10 == 0 ) {
		*(str + strlen(str) - 1) = '\0';
		fprintf(stderr, "Current time : %s\n", str);
		*(str + strlen(str) - 5) = '\0';
		}
	else {
		*(str + strlen(str) - 6) = '\0';
		}
	fprintf(stderr, "%s :: %s\n", &str[11], msg);
}

void hangul_filter(char *res, char *str)
{
	char	ch ;
	int	lastchar ;
	char	*p ;

	lastchar = LAST_SPACES ;
	for ( p = str ; ( ch = *p ) ; p++ ) {
		switch(typecheck((unsigned char) ch)) {
			case	CH_NORMAL_CODE :
				lastchar = LAST_NORMAL ;
				*res++ = ch ;
				break ;
			case	CH_HAN_1_AND_2 :
				if ( lastchar == LAST_HAN_FIRST ) {
					*res++ = ch ;
					lastchar = LAST_HAN_SECOND ;
					}
				else  {	/* LAST_NORMAL, LAST_SPACES, LAST_HAN_SECOND */
					*res++ = ch ;
					lastchar = LAST_HAN_FIRST ;
					}
				break ;
			case	CH_HAN_ONLY_2 :	/* It must be the first letter of hangul */
				if ( lastchar == LAST_HAN_FIRST ) {
					*res++ = ch ;
					lastchar = LAST_HAN_SECOND ;
					}
				break ;
			case	CH_NOISE :
				break ;
			case	CH_SPACES :
				lastchar = LAST_SPACES ;
				*res++ = ch ;
				break ;
			default :
				*res++ = '?' ;
			}
		}

	*res = NULL ;
}

int typecheck(unsigned char ch)
{
	if ( ch > ' ' && ch <= '~' )
		return CH_NORMAL_CODE ;

	if ( ch >= 160 ) {
		if ( ch <= 200 )
			return CH_HAN_1_AND_2 ;
		else if ( ch <= 255 )
			return CH_HAN_ONLY_2 ;
		else
			return CH_UNKNOWN ;
		}

	if ( ch == ' ' || ch == '\r' || ch == '\n' || ch == '\t')
		return CH_SPACES ;

	return CH_NOISE ;
}
