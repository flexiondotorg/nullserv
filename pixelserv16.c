/*
* pixelserv.c a small mod to public domain
** server.c -- a stream socket server demo
* from http://beej.us/guide/bgnet/
* single pixel http string from
* http://proxytunnel.sourceforge.net/pixelserv.php
* V1 Proof of concept mstombs www.linkysinfo.org 06/09/09
* V2 usleep after send to delay socket close 08/09/09
* V3 TCP_NODELAY not usleep 09/09/09
* V4 daemonize with syslog 10/09/09
* V5 usleep back in 10/09/09
* V6 only use IPV4, add linger and shutdown to avoid need for sleep 11/09/09
* Consistent exit codes and version stamp
* V7 use shutdown/read/shutdown to cleanly flush and close connection
* V8 add inetd and listening IP option
* V9 minimalize
* V10 make inetd mode compiler option -DINETD_MODE
* V11 debug TCP_NODELAY back and MSG_DONTWAIT flag on send
* V12 Change read to recv with MSG_DONTWAIT and add MSG_NOSIGNAL on send
* V13 DONTWAIT's just trigger RST connection closing so remove
* V14 Back to V8 fork(), add header "connection: close"" and reformat pixel def
* V15 add command line options for variable port 2nd March 2010
* V16 add command line option for ifname, add SO_LINGER2 to not hang in FIN_WAIT2
*/
#define VERSION "V16"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h> /* for TCP_NODELAY */
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <signal.h>
#include <syslog.h>
#include <net/if.h> /* for IFNAMSIZ */

#define PORT "80"  // the port users will be connecting to

#define BACKLOG 30 // how many pending connections queue will hold

#define CHAR_BUF_SIZE 1024 //surprising how big requests can be with cookies etc

/** handler to ensure no orphan sub processes left */
void sigchld_handler (int s)
{
	while ( waitpid (-1, NULL, WNOHANG) > 0 );
}

/** Handler for the SIGTERM signal (kill) */
static void sigterm (int sig)
{
	signal (sig, SIG_IGN);	/* Ignore this signal while we are quitting */
	syslog (LOG_NOTICE, "exit on SIGTERM");
	exit (1);
}

#ifdef TEST
/** get sockaddr, IPv4 or IPv6: */
void *get_in_addr (struct sockaddr *sa)
{
	if (sa->sa_family == AF_INET) {
		return &(( (struct sockaddr_in*) sa )->sin_addr);
	}

	return &(( (struct sockaddr_in6*) sa )->sin6_addr);
}
#endif

int main (int argc, char *argv[])
{
	int sockfd, new_fd;  // listen on sock_fd, new connection on new_fd
	struct sockaddr_storage their_addr; // connector's address information
	socklen_t sin_size;
	int yes = 1;
	int n = 5;
#ifdef TEST
	char s[INET6_ADDRSTRLEN];
#endif
	int rv;
    char ip_addr[INET_ADDRSTRLEN] = "0.0.0.0";
	int use_ip = 0;
	char buf[CHAR_BUF_SIZE];
	char port[INET_ADDRSTRLEN] = PORT; /* not sure how long this can be */
	int i;
	char ifname[IFNAMSIZ] = "br0";
    static const unsigned char httpnullpixel[] = 
        "HTTP/1.1 200 OK\r\n"
        "Content-type: image/gif\r\n"
        "Accept-ranges: bytes\r\n"
        "Content-length: 43\r\n"
        "Connection: close\r\n"
        "\r\n"
        "GIF89a\1\0\1\0\200\0\0\377\377\377\0\0\0\41\371\4\1\0\0\0\0\54\0\0\0\0\1\0\1\0\0\2\2\104\1\0;" ;

	struct addrinfo hints, *servinfo;

	/* command line arguments processing */
	for (i = 1; i < argc; i++)
	{
		if (argv[i][0] != '-') {
	        /* assume its a listening IP address */
            strncpy (ip_addr, argv[i], INET_ADDRSTRLEN);
            ip_addr[INET_ADDRSTRLEN - 1] = '\0';
            use_ip = 1;		
		}
		else {
		    switch (argv[i][1])
		    {
#ifdef INETD_MODE		
		    case 'i' :
	            /* first check if "-i" option is used */
			    fwrite ( httpnullpixel, 1, (sizeof httpnullpixel) - 1, stdout );
			    /* flush read buffer ? */
			    exit (EXIT_SUCCESS);		
#endif
            case 'n' :
            	if ( (i + 1) < argc ) {
			        strncpy (ifname, argv[++i], IFNAMSIZ);
		            ifname[IFNAMSIZ - 1] = '\0';
			    }
			    else {
				    printf ("Option -%c takes one argument\n", argv[i][1]);
			    }

                break;
           
		    case 'p' :
			    if ( (i + 1) < argc ) {
			        strncpy (port, argv[++i], sizeof port);
		            port[(sizeof port) - 1] = '\0';
			    }
			    else {
				    printf ("Option -%c takes one argument\n", argv[i][1]);
			    }
			
			    break;

		    default :
			    printf ("Unknown opt: %s\n", argv[i]);
			    printf ("Usage:%s [-i] [IP] [-p %s] [-n IF]\n", argv[0], port);
			    printf ("i = inetd mode, IP or hostname to listen, p = port No/name IF = interface name (def %s)\n", ifname);
			    exit (EXIT_FAILURE);
		    }
		}
	}

	openlog ("pixerlserv", LOG_PID | LOG_CONS | LOG_PERROR, LOG_DAEMON);
	syslog ( LOG_INFO, "%s %s compiled: %s from %s", argv[0], VERSION, __DATE__ " " __TIME__  , __FILE__ );

#ifndef TEST
	if ( daemon (0, 0) < 0 ) {
		syslog (LOG_ERR, "failed to daemonize, exit: %m");
		exit (EXIT_FAILURE);
	}
#endif

	memset (&hints, 0, sizeof hints);
	hints.ai_family = AF_INET; //AF_UNSPEC; /* AF_INET restricts to IPV4 */
	hints.ai_socktype = SOCK_STREAM;
	if (use_ip == 0) {
		hints.ai_flags = AI_PASSIVE; /* use my IP */
	}

	if ((rv = getaddrinfo (use_ip ? ip_addr : NULL, port, &hints, &servinfo) )) {
		syslog ( LOG_ERR, "getaddrinfo: %s", gai_strerror (rv) );
		exit (EXIT_FAILURE);
	}

	if ( (( sockfd = socket (servinfo->ai_family, servinfo->ai_socktype, servinfo->ai_protocol) ) < 0)
	    ||
	     (setsockopt ( sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof (int) ) < 0)
	    ||
	     (setsockopt ( sockfd, SOL_SOCKET, SO_BINDTODEVICE, ifname, IFNAMSIZ ) < 0) // only use selected i/f
	    ||
	     (setsockopt ( sockfd, SOL_TCP, TCP_NODELAY, &yes, sizeof (int) ) < 0)      // send short packets straight away
	    ||
         (setsockopt ( sockfd, SOL_TCP, TCP_LINGER2, (void *)&n, sizeof (n) ) < 0)  // try to prevent hanging processes in FIN_WAIT2
	    ||
	     (bind ( sockfd, servinfo->ai_addr, servinfo->ai_addrlen ) < 0)
	    ||
	     (listen (sockfd, BACKLOG) < 0) ) {
		syslog (LOG_ERR, "Abort: %m");
		exit (EXIT_FAILURE);
	}

	freeaddrinfo (servinfo); /* all done with this structure */

    {
    	struct sigaction sa;
	    /* reap all dead processes */
	    sa.sa_handler = sigchld_handler; 
	    sigemptyset (&sa.sa_mask);
	    sa.sa_flags = SA_RESTART;
	    if ( sigaction (SIGCHLD, &sa, NULL) < 0 ) {
		    syslog (LOG_ERR, "SIGCHLD: %m");
		    exit (EXIT_FAILURE);
	    }

	    /* set signal handler */
	    memset (&sa, 0, sizeof (struct sigaction));
	    sa.sa_handler = sigterm;
	    if ( sigaction (SIGTERM, &sa, NULL) < 0 ) {
		    syslog (LOG_ERR, "SIGTERM %m");
		    exit (EXIT_FAILURE);
	    }
    }

	syslog (LOG_NOTICE, "Listening on %s:%s", ip_addr, port);

	while(1) {  /* main accept() loop */
		sin_size = sizeof their_addr;
		new_fd = accept ( sockfd, (struct sockaddr *) &their_addr, &sin_size );
		if (new_fd < 0) {
			syslog (LOG_WARNING, "accept: %m");
			continue;
		}

#ifdef TEST
  inet_ntop (their_addr.ss_family, get_in_addr( (struct sockaddr *) &their_addr ), s, sizeof s);
  printf ("server: got connection from %s\n", s);
		
/* read a line from the request */
  if (( rv = recv (new_fd, buf, CHAR_BUF_SIZE- 1, 0) ) < 0) {
     perror ("recv");
     exit (1);
  }
    
  buf[rv] = '\0';
//  printf("\nreceived %d bytes '%s'\n", rv, buf);

  char *method = strtok (buf, " ");
//  printf("method: '%s'\n", method);

  char *path = strtok (NULL, " ");
//  printf("path: '%s'\n", path);

  char *protocol = strtok (NULL, "\r");
//  printf("protocol: '%s'\n", protocol);
  
  char *subpath = strtok (path,"?");
//  printf("subpath: '%s'\n", subpath);

  char *ext = strrchr (subpath, '.');
  printf ("ext: '%s'\n", ext);

if ( (ext == NULL) || (strcmp (ext,".js") != 0) ) {
  printf ("not a js\n");
}
#endif

		if ( !fork () ) { 
			/* this is the child process */
			close (sockfd); /* child doesn't need the listener */
			rv = send ( new_fd, httpnullpixel, (sizeof httpnullpixel) - 1, 0 );
			if (rv < 0) {
				syslog (LOG_WARNING, "send: %m");
			}

			/* clean way to flush read buffers and close connection */
    			if (shutdown (new_fd, SHUT_WR) == 0) {
          			while ( read (new_fd, buf, sizeof buf) > 0 );
			}

    		shutdown (new_fd, SHUT_RD);
			close (new_fd);
			exit (EXIT_SUCCESS);
		}

		close (new_fd);  /* parent doesn't need this */
	}

	return (EXIT_SUCCESS);
}

