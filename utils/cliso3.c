/*
	cliso3.c - telnet service program (v 2.0)
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
#define	MAX_DESCRIPTOR	20
#define	MAX_HOSTNAME	255

#define	TIME_OUT_SEC	0
#define	TIME_OUT_USEC	100000

#define	REMOTE_HOST	"143.248.37.32"
#define	REMOTE_PORT	4001

#define	LOCAL_HOST	"localhost"
#define	DEFAULT_LOCAL_PORT	4001

#define	MAX_ALLOWED_ERROR	30

/* check bits : for filtering */
#define	CH_UNKNOWN	0
#define	CH_NORMAL_CODE	1
#define	CH_HAN_1_AND_2	2
#define	CH_HAN_ONLY_2	3
#define	CH_NOISE	4
#define	CH_SPACES	5
#define	CH_EOF	6
#define	CH_SPECIAL	7

/* last bits : for filtering */
#define	LAST_NORMAL	0
#define	LAST_HAN_FIRST	1
#define	LAST_HAN_SECOND	2
#define	LAST_SPACES	3
#define	LAST_EOF	4

/* Struct for sockets */
struct socket_lib {
	int	local_des ;	/* local descriptor */
	int	remote_des ;	/* remote descriptor */
	} ;

/* file id */
char fileid[] = "@(#)cliso3.c	1.17 95/02/02" ;

/* Mother socket descriptor */
int mother_sd ;

/* Procedure decleration */
void init_socket_array(struct socket_lib *book) ;
int open_mother_socket(int port) ;
int get_new_connection(int mother_s, struct socket_lib *book) ;
int find_empty_array(struct socket_lib *book) ;
int serving_loop(struct socket_lib *book) ;
int check_one_socket(int s, fd_set *in_set, fd_set *out_set, fd_set *exc_set) ;
int check_sockets(fd_set *in_set, fd_set *out_set, fd_set *exc_set,
	struct socket_lib *book) ;
int open_remote_connection(int s) ;
int send_remote_connection(char *msg, int l, int s) ;
int read_remote_connection(char *msg, int s) ;
int send_to_local(char *msg, int s) ;
int send_n_to_local(char *msg, int l, int s) ;
int get_from_local(char *msg, int s) ;
void init_signal(), quit_int(), log(char *msg) ;
int hangul_filter2(char *res, int l, char *str) ;
void pr_msg(char *str) ;
int count_connection(struct socket_lib *book) ;
void report_book(int mother_s, struct socket_lib *book) ;

main(int argc, char *argv[])
{
	struct socket_lib socket_book[MAX_DESCRIPTOR] ;
	int	num_error, local_mother_s, local_port ;
	int	nstep ;

	if ( argc > 1 ) {
		if ((local_port = atoi(argv[1])) < 1024 ) {
			log("Irregal port") ;
			exit(1) ;
			}
		}
	else {
		local_port = DEFAULT_LOCAL_PORT ;
		}
		
	num_error = 0 ;

	/*  print version  */
	log(fileid) ;

	log("init signal") ;
	init_signal() ;

	log("init socket array") ;
	init_socket_array(socket_book) ;

	log("closing stdin, stdout") ;
	close(0) ;
	close(1) ;

	log("open local mother socket") ;
	if ((local_mother_s = open_mother_socket(local_port)) < 0 ) {
		fprintf(stderr, "Can't open local port\n") ;
		exit(1) ;
		}

	nstep = 0 ;
	while (1) {
		if ( nstep ++ > 90000 ) {
			report_book(local_mother_s, socket_book) ;
			nstep = 0 ;
			}
		if (get_new_connection(local_mother_s, socket_book) < 0 ) {
			fprintf(stderr, "Can't get new connection from localhost anymore\n") ;
			if ( ++num_error > MAX_ALLOWED_ERROR ) {
				log("Exit from get_new_connection") ;
				break ;
				}
			}
		if ( serving_loop(socket_book) < 0 ) {
			/* error in select */
			if ( ++num_error > MAX_ALLOWED_ERROR ) {
				log("Exit from serving_loop") ;
				break ;
				}
			}
		}

	log("End client") ;
}

void init_socket_array(struct socket_lib *book)
{
	int	i ;

	for ( i = 0 ; i < MAX_DESCRIPTOR ; i ++ ) {
		book[i].local_des = -1 ;
		book[i].remote_des = -1 ;
		}
}

/* return mother socket descriptor */
int open_mother_socket(int port)
{
	int s;
	char *opt;
	char hostname[MAX_HOSTNAME+1];
	struct sockaddr_in sa;
	struct hostent *hp;
	struct linger ld;
	char	buf[BUFSIZ] ;

	bzero(&sa, sizeof(struct sockaddr_in));
	gethostname(hostname, MAX_HOSTNAME);
	sprintf(buf, "Local host (%s)", hostname) ;
	log(buf) ;
	hp = gethostbyname(LOCAL_HOST);
	if (hp == NULL) {
		perror("gethostbyname");
		return (-1) ;
		}

	sa.sin_family = hp->h_addrtype;
	sa.sin_port  = htons(port);

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

int get_new_connection(int mother_s, struct socket_lib *book)
{
	fd_set	in_set, out_set, exc_set ;
	int	new_s, new_length ;
	int	remote_s, ind ;
	struct sockaddr_in	new_addr ;
	char	buf[BUFSIZ] ;

	if ( check_one_socket(mother_s, &in_set, &out_set, &exc_set) < 0 ) {
		log("get_new_connection: local socket error") ;
		perror() ;
		return (-1) ;
		}
	if ( FD_ISSET(mother_s, &in_set)) {
		/* new connection */
		new_length = sizeof(new_addr) ;
		new_s = accept(mother_s, (struct sockaddr *)&new_addr, &new_length);
		if ( new_s < 0 ) {	/* error */
			log("get_new_connection: accept failed") ;
			perror() ;
			return (-1) ;
			}
		sprintf(buf, "[%d] New connection from %s (d=%d)",
			count_connection(book), (char *)inet_ntoa(new_addr.sin_addr), new_s);
		log(buf) ;

		if ((ind = find_empty_array(book)) < 0 ) {
			send_to_local("Sorry, port is full. Try another port\n", new_s) ;
			sprintf(buf, "port full : closing %d", new_s) ;
			log(buf) ;
			close(new_s) ;
			return NULL ;
			}

		/* send message to connector */
		send_to_local("Trying to connect remote host ... ", new_s) ;

		/* insert to descriptor array */
		book[ind].local_des = new_s ;
		if ((remote_s = open_remote_connection(new_s)) < 0 ) {
			send_to_local("Can't reach to server host now\n", new_s) ;
			sprintf(buf, "Can't open remote port : closing %d", new_s) ;
			log(buf) ;
			book[ind].local_des = -1 ;
			close(new_s) ;
			}
		else {
			send_to_local("Connected.\n", new_s) ;
			book[ind].remote_des = remote_s ;
			}
		}

	return NULL ;
}

int count_connection(struct socket_lib *book)
{
	int	i, count ;

	for ( i = 0, count = 0 ; i < MAX_DESCRIPTOR ; i ++ ) {
		if ( book[i].local_des >= 0 )
			count ++ ;
		}

	return count ;
}

int find_empty_array(struct socket_lib *book)
{
	int	i ;

	for ( i = 0 ; i < MAX_DESCRIPTOR ; i ++ ) {
		if ( book[i].local_des < 0 )
			return i ;
		}
	return (-1) ;
}

int serving_loop(struct socket_lib *book)
{
	fd_set	in_set, out_set, exc_set ;
	char	msg[MAX_MSG_LENGTH], filmsg[MAX_MSG_LENGTH] ;
	char	buf[BUFSIZ] ;
	int	i, local_s, remote_s, cmd, nread ;

	/* bzero(msg, MAX_MSG_LENGTH) ; */
	msg[0] = NULL ;
	buf[0] = NULL ;

	/* check socket */
	if ( check_sockets(&in_set, &out_set, &exc_set, book) < 0 ) {
		/* ERROR in select */
		return (-1) ;
		}

	for ( i = 0 ; i < MAX_DESCRIPTOR ; i ++ ) {
		local_s = book[i].local_des ;
		remote_s = book[i].remote_des ;
		if ( local_s >= 0 ) {
			if ( FD_ISSET(remote_s, &exc_set)) {
				/* connection closed from remote host  */
				send_to_local("Rejected from remote host\n", local_s) ;
				book[i].local_des = -1 ;
				close(local_s) ;
				close(remote_s) ;
				continue ;
				}
			if ( FD_ISSET(remote_s, &in_set)) {
				/* message arrived */
				msg[0] = NULL ;
				if ((nread = read_remote_connection(msg, remote_s)) < 0 ) {
					send_to_local("Connection close by remote host\n", local_s) ;
					sprintf(buf, "by remote host : closing %d", local_s) ;
					log(buf) ;
					book[i].local_des = -1 ;
					close(local_s) ;
					close(remote_s) ;
					continue ;
					}
				else {
					send_n_to_local(msg, nread, local_s) ;
					}
				}

			/* check local socket */
			if ( FD_ISSET(local_s, &exc_set)) {
				/* connection closed from local host  */
				/* close this socket */
				sprintf(buf, "EXC: lost connection %d", local_s) ;
				log(buf) ;
				book[i].local_des = -1 ;
				close(local_s) ;
				close(remote_s) ;
				continue ;
				}
			if ( FD_ISSET(local_s, &in_set)) {
				/* message arrived from user : sent it to remote host */
				if ((nread = get_from_local(msg, local_s)) < 0 ) {
					sprintf(buf, "IN : closing by user %d", local_s) ;
					log(buf) ;
					book[i].local_des = -1 ;
					close(local_s) ;
					close(remote_s) ;
					}
				else {
#ifdef DEBUGFLAG
					fprintf(stderr, "before filter: %s (%d)", msg, nread) ;
					pr_msg(msg) ;
#endif
					nread = hangul_filter2(filmsg, nread, msg) ;
#ifdef DEBUGFLAG
					fprintf(stderr, "after filter: %s (%d)", filmsg, nread) ;
					pr_msg(filmsg) ;
#endif
					send_remote_connection(filmsg, nread, remote_s) ;
					}
				}
			}	/* end of if local_s >= 0 */
		}	/* end of for */

	return NULL ;
}

int check_one_socket(int s, fd_set *in_set, fd_set *out_set, fd_set *exc_set)
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

int check_sockets(fd_set *in_set, fd_set *out_set, fd_set *exc_set,
	struct socket_lib *book)
{
	int	i, maxdesc, local_des, remote_des ;
	struct timeval	null_time, time_out ;

	null_time.tv_sec = 0 ;
	null_time.tv_usec = 0 ;
	time_out.tv_sec = TIME_OUT_SEC ;
	time_out.tv_usec = TIME_OUT_USEC ;

	maxdesc = -1 ;

	FD_ZERO(in_set) ;
	FD_ZERO(out_set) ;
	FD_ZERO(exc_set) ;
	for ( i = 0 ; i < MAX_DESCRIPTOR ; i ++ ) {
		local_des = book[i].local_des ;
		remote_des = book[i].remote_des ;
		if ( local_des >= 0 ) {
			if ( maxdesc < local_des )
				maxdesc = local_des ;
			if ( maxdesc < remote_des )
				maxdesc = remote_des ;
			FD_SET(local_des, in_set) ;
			/* need not  FD_SET(local_des, out_set) ; */
			FD_SET(local_des, exc_set) ;
			FD_SET(remote_des, in_set) ;
			/* need not  FD_SET(remote_des, out_set) ; */
			FD_SET(remote_des, exc_set) ;
			}
		}
	null_time.tv_sec = 0 ;
	null_time.tv_usec = 0 ;
	if ( select(maxdesc + 1, in_set, out_set, exc_set, &null_time) < 0 ) {
		perror("check_sockets: poll") ;
		return (-1) ;
		}
	time_out.tv_sec = TIME_OUT_SEC ;
	time_out.tv_usec = TIME_OUT_USEC ;
	if ( select(0, (fd_set *)0, (fd_set *)0, (fd_set *)0, &time_out)< 0) {
		perror("check_sockets: sleep") ;
		return (-1) ;
		}

	return NULL ;
}

int open_remote_connection(int new_s)
{
	extern int errno ;
	int	s ;
	struct sockaddr_in	serv_addr ;

	bzero((char *)&serv_addr, sizeof(serv_addr)) ;
	serv_addr.sin_family = AF_INET ;
	serv_addr.sin_addr.s_addr = inet_addr(REMOTE_HOST) ;
	serv_addr.sin_port = htons(REMOTE_PORT) ;

	if ((s = socket(AF_INET, SOCK_STREAM, 0)) < 0 ) {
		log("open_remote_connection: socket open failed") ;
		send_to_local("Server capacity full. Try the other port.\n", new_s) ;
		perror("socket") ;
		return (-1) ;
		}

	if (connect(s, (struct sockaddr_in *)&serv_addr, sizeof(serv_addr))<0){
		log("open_remote_connection: connect failed") ;
		if ( errno == ECONNREFUSED ) {
			log("Remote system is not ready. -- REFUSED --") ;
			send_to_local("Remote system is not ready. Try later.\n", new_s) ;
			}
		if ( errno == ENETUNREACH ) {
			log("Network is unreachable..") ;
			send_to_local("Network is unreachable...\n", new_s) ;
			}
		if ( errno == ETIMEDOUT ) {
			log("Connection time out !") ;
			send_to_local("Connection time out !\n", new_s) ;
			}
		perror("connect") ;
		close(s) ;	/* close remote socket */
		return (-1) ;
		}

	/* connected to remote host */

	return (s) ;
}

int send_remote_connection(char *msg, int length, int s)
{
	if ( msg == NULL )
		return (-1) ;

	if (write(s, msg, length) < 0) {
#ifdef DEBUGFLAG
		log("send_remote_connection: write failed") ;
#endif
		return (-1) ;
		}
	return NULL ;
}

int read_remote_connection(char *msg, int s)
{
	int	nread ;
	if ( msg == NULL )
		return NULL ;

	msg[0] = NULL ;
	if ((nread = read(s, msg, MAX_MSG_LENGTH -1)) <= 0) {
#ifdef DEBUGFLAG
		log("read_remote_connection: read failed") ;
#endif
		return (-1) ;
		}

	msg[nread] = NULL ;
	return nread ;
}

int send_to_local(char *msg, int s)
{
	int	length ;

	length = strlen(msg) ;
	if (write(s, msg, length) < 0) {
#ifdef DEBUGFLAG
		log("send_to_local: write failed") ;
#endif
		return (-1);
		}
	return NULL ;
}

int send_n_to_local(char *msg, int length, int s)
{
	if (write(s, msg, length) < 0) {
#ifdef DEBUGFLAG
		log("send_n_to_local: write failed") ;
#endif
		return (-1);
		}
	return NULL ;
}

int get_from_local(char *msg, int s)
{
	int	nread ;

	if ( msg == NULL ) {
		log("get_from_local: invalid msg pointer") ;
		return (-1) ;
		}

	msg[0] = NULL ;
	if ((nread = read(s, msg, MAX_MSG_LENGTH -1)) <= 0) {
		msg[0]= NULL ;
#ifdef DEBUGFLAG
		log("get_from_local: read failed") ;
#endif
		return (-1) ;
		}
	msg[nread] = NULL ;

	return nread ;
}

void init_signal()
{
	signal(SIGINT, quit_int);
	signal(SIGPIPE, SIG_IGN);
}

void quit_int()
{
	log("interrupt signal : Exit") ;
	exit(1) ;
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

void pr_msg(char *str)
{
	int	i, leng ;
	unsigned char	ch ;

	leng = strlen(str) ;
	fprintf(stderr, "(") ;
	for ( i = 0 ; i < leng ; i ++ ) {
		ch = str[i] ;
		fprintf(stderr, "%d ", ch) ;
		}
	fprintf(stderr, ")\n") ;
}

void report_book(int mother_s, struct socket_lib *book)
{
	int	i, nactive ;

	fprintf(stderr, "-- Report socket libraries --\n") ;
	fprintf(stderr, "-- Mother SD : %d\n", mother_s) ;
	nactive = 0 ;
	for ( i = 0 ; i < MAX_DESCRIPTOR ; i ++ ) {
		fprintf(stderr, "-- book %2d : local %2d, remote %2d\n",
			i, book[i].local_des, book[i].remote_des ) ;
		if ( book[i].local_des >= 0 )
			nactive ++ ;
		}
	fprintf(stderr, "-- Total active socket : %d\n", nactive) ;
}
