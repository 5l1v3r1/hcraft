#include <arpa/inet.h>
#include <ctype.h>
#include <errno.h>
#include <libgen.h>
#include <netdb.h>
#include <netinet/in.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include "hcraft.h"

int verbosity=0;
char *modefile = "./hcraft.modes";


int main( int argc, char *argv[] ) {
	int x, sock, bytes;
	int arg, lastarg, commandlen;
	unsigned long mode = 0;
	unsigned long endmode = 0;
	char ch;
	char *prog;
	char *description;
	char *remotehost;
	long remoteport;
	char *httptype;
	char *argstring;
	char http_command[4096];
	struct in_addr address;
	struct sockaddr_in addr;
	struct hostent *h;
	char *modebuff;
	char *modechnk;
	char *reqpart[256];
	char *reqpartchnk;
	char *last_reqpart;
	char *chbuf;

	prog = basename(argv[0]);

	/* Defaults */
	httptype = "GET";
	remotehost = NULL;
	remoteport = 80;

	/* Option Handler */
	while( (ch = getopt(argc, argv, "+Vh:i:lm:M:p:v")) != EOF ) {
		switch( ch ) {
			case 'V':
				version(); 
				exit(0);
			case 'h':
				remotehost = optarg;
				break;
			case 'i':
				mode = atol(strsep(&optarg, ":"));
				endmode = atol(strsep(&optarg, "\0"));
				break;
			case 'l':
				listmodes();
				exit(0);
			case 'm':
				mode = atol(optarg);
				endmode = atol(optarg);
				break;
			case 'M':
				modefile = optarg;
				break;
			case 'p':
				remoteport = atol(optarg);
				break;
			case 'v':
				verbosity++;
				break;
			default:
				usage( prog );
		}
	}

	/* Output Header */
	fprintf( stderr, "hcraft - HTTP Request Exploit Framework\n" );
	fprintf( stderr, "I)ruid <druid@caughq.org> & intropy <intropy@caughq.org>\n\n" );

	/* Make sure we have our required arguments */
	if( ! remotehost || ! argv[1] ) usage( prog );
	if( mode == 0 ) {
		fprintf( stderr, "Unspecified Mode.  Use %s -l to list available modes.\n", prog );
		exit(-1);
	}

	/* If incremental, make sure the range is valid */
	if( mode > endmode ) {
		fprintf( stderr, "Invalid mode range for -i: %ld > %ld.\n", mode, endmode );
		exit(-1);
	}

	/* If not incremental, get and validate requested mode */
	if( mode == endmode && ! (modebuff = getmode(mode)) ) {
		fprintf( stderr, "Invalid Mode.  Use %s -l to list available modes.\n", prog );
		exit(-1);
	}

	if(verbosity && mode != endmode) fprintf( stderr, "Entering Incremental Mode: %ld:%ld\n", mode, endmode );
	for( ; mode <= endmode; mode++ ) {
		if( ! (modebuff = getmode(mode)) ) continue;

		/* Strip out any excess tabs */
		while( strlen((description = strsep( &modebuff, "\t" ))) == 0 );
		/* Strip leading and trailing quotes from description */
		memcpy( &description[0], &description[1], strlen(description)-1 );
		description[strlen(description)-2] = '\0';
		if(verbosity) fprintf( stderr, "Using mode %ld: %s\n", mode, description );

		/* Strip out any excees tabs */
		while( strlen((httptype = strsep( &modebuff, "\t" ))) == 0 );
		for( x = 0; httptype[x]; x++ ) httptype[x] = toupper(httptype[x]);
		if(verbosity) fprintf( stderr, "Using HTTP %s Method\n", httptype );

		/* Strip out any excees tabs */
		while( strlen((modechnk = strsep( &modebuff, "\t" ))) == 0 );
		free(modebuff);
		/* Parse last modeline chunk (as delimited by tabs) into reqparts array */
		x = 0;
		while( (modebuff = strsep( &modechnk, " " )) ) {
			reqpart[x] = malloc(strlen(modebuff)+1);
			memcpy( &reqpart[x], &modebuff, strlen(modebuff) );
			if(verbosity>2) fprintf( stderr, "reqpart[%d] = [%s]\n", x, reqpart[x] );
			x++;
		}
		reqpart[x] = NULL;

		/* Replace reqpart variables with values */
		x = -1;
		while( reqpart[++x] ) {
			if(verbosity>2) fprintf( stderr, "Checking [%s] for variable replacement: ", reqpart[x] );
			if( reqpart[x][0] == '%' && reqpart[x][1] == '%' && \
			    reqpart[x][strlen(reqpart[x])-2] == '%' && reqpart[x][strlen(reqpart[x])-1] == '%' ) {

				/* All of argv[] */	
				if( strcmp( reqpart[x], "%%ARGV%%" ) == 0 ) {
					arg = optind;
					if( ! argv[arg] ) {
						fprintf( stderr, "Required Commandline Argument not supplied: %s\n", reqpart[x] );
						exit(-1);
					}
					if(verbosity>2) fprintf( stderr, "[%s", argv[arg] );
					argstring = hex_encode( argv[arg] );
					while( argv[++arg] ) {
						if(verbosity>2) fprintf( stderr, " %s", argv[arg] );
						reqpart[x] = hex_encode( argv[arg] );
						argstring = realloc( argstring, strlen(argstring) + strlen("%20") + strlen(reqpart[x]) + 1 );
						strcat( argstring, "%20" );
						strcat( argstring, reqpart[x] );
					}
					if(verbosity>2) fprintf( stderr, "] " );
					reqpart[x] = argstring;
				}

				/* Range of argv[] elements */
				if( strncmp( reqpart[x], "%%ARGV", 6 ) == 0 && (reqpartchnk = strchr( reqpart[x], ':' )) ) {
printf( "reqpartchnk[0] = [%s]\n", reqpartchnk );
printf( "reqpartchnk[1] = [%s]\n", &reqpartchnk[1] );
					arg = optind + (atoi( &reqpart[x][6] ) - 1);
					if( isdigit(reqpartchnk[1]) )
						lastarg = optind + atoi(&reqpartchnk[1]) - 1;
					else
						lastarg = argc - 1;
					if( arg > lastarg ) {
						fprintf( stderr, "Mode Has Bad Commandline Argument Range: %s (%d > %d)\n", reqpart[x], arg, lastarg );
						exit(-1);
					}
					if(verbosity>1) fprintf( stderr, "Processing arguments %d through %d\n", arg, lastarg );
					if(verbosity>2) fprintf( stderr, "[%s", argv[arg] );
					argstring = hex_encode( argv[arg++] );
					for( ; arg <= lastarg; arg++ ) {
						if( ! argv[arg] ) {
							fprintf( stderr, "Required Commandline Argument not supplied: %s\n", reqpart[x] );
							exit(-1);
						}
						if(verbosity>2) fprintf( stderr, " %s", argv[arg] );
						reqpart[x] = hex_encode( argv[arg] );
						argstring = realloc( argstring, strlen(argstring) + strlen("%20") + strlen(reqpart[x]) + 1 );
						strcat( argstring, "%20" );
						strcat( argstring, reqpart[x] );
					}
					if(verbosity>2) fprintf( stderr, "] " );
					reqpart[x] = argstring;
				}

				/* Single element of argv[] */
				if( strncmp( reqpart[x], "%%ARGV", 6 ) == 0 && strcmp( reqpart[x], "%%ARGV%%" ) != 0 ) {
					arg = optind + (atoi( &reqpart[x][6] ) - 1);
					if( ! argv[arg] ) {
						fprintf( stderr, "Required Commandline Argument not supplied: %s\n", reqpart[x] );
						exit(-1);
					}
					if(verbosity>2) fprintf( stderr, "[%s] ", argv[arg] );
					reqpart[x] = hex_encode( argv[arg] );
				}

				if( strncmp( reqpart[x], "%%REP", 5 ) == 0 ) {
					arg = atoi( &reqpart[x][5] );
					reqpart[x] = malloc((strlen(last_reqpart)*arg)+1);
					reqpart[x][0] = '\0';
					for( ; arg > 0; arg-- ) strcat( reqpart[x], last_reqpart );
				}

			}
			if(verbosity>3) fprintf( stderr, "(%s)", reqpart[x] );
			if(verbosity>2) fprintf( stderr, "\n" );
			last_reqpart = reqpart[x];
		}

		/* Build HTTP request */
		if(verbosity>1) fprintf( stderr, "Building %s request...\n", httptype );
		snprintf( http_command, sizeof(http_command), "%s ", httptype );
		commandlen = strlen(httptype)+1;
		x = 0;
		while( reqpart[x] ) {
			commandlen += strlen(reqpart[x]);
			strcat( http_command, reqpart[x] );
			x++;
		}
		strcat( http_command, "\n" );
		commandlen += 1;
		if(verbosity>2) fprintf( stderr, "%s", http_command );

		/* Convert remotehost into binary network byte order */
		if( inet_aton( remotehost, &address ) == 0 ) {
			if( ! (h = gethostbyname( remotehost )) ) {
				perror(remotehost);
				exit(-1);
			}
			memcpy( &address.s_addr, h->h_addr_list[0], sizeof(unsigned long) );
		}

		/* Build socket */
		if ( (sock = socket( PF_INET, SOCK_STREAM, IPPROTO_TCP )) == -1 ) {
			fprintf( stderr, "socket: %s\n", strerror(errno) );
		}
	
		/* Connect our socket to remote host */
		addr.sin_family = PF_INET;
		addr.sin_addr.s_addr = address.s_addr;
		addr.sin_port = htons(remoteport);
		if( (connect( sock, (struct sockaddr *) &addr, sizeof(addr) )) == -1 ) {
			fprintf( stderr, "connect: %s\n", strerror(errno) );
			exit(-1);
		}

		/* Send GET request */
		if(verbosity) fprintf( stderr, "Sending %s request: %s", httptype, http_command );
		if( (write( sock, &http_command, commandlen )) == -1 ) {
			fprintf( stderr, "write: %s\n", strerror(errno) );
			exit(-1);
		}
		if(verbosity>1) for( x = 0; x < commandlen; x++ ) fprintf( stderr, "%c", http_command[x] );
		if(verbosity>2) for( x = 0; x < commandlen; x++ ) fprintf( stderr, "\\x%02x", (unsigned int) http_command[x] );
		if(verbosity>2) fprintf( stderr, "\n" );

		/* Read in response and write it to file */
		chbuf = malloc(1);
		while( (bytes = read( sock, chbuf, 1 )) > 0 ) {
			printf( "%c", chbuf[0] );
		}
		if( bytes == -1 ) {
			fprintf( stderr, "read: %s\n", strerror(errno) );
			exit(-1);
		}

		/* cleanup */
		close(sock);
	}

	exit(0);
}
