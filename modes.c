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

extern int verbosity;
extern char *modefile;

char *getmode( int reqmode ) {
	FILE *dat;
	char *retbuff;
	char *buffer;

	buffer = malloc(4096);
	if( ! (dat = fopen( modefile, "r" )) ) {
		if( ! (dat = fopen( "/etc/hcraft.modes", "r" )) ) {
			fprintf( stderr, "Error opening hcraft.modes (from ./ or /etc): %s\n", strerror(errno) );
			exit(-1);
		}
	}
	while( fgets( buffer, 4096, dat ) ) {
		buffer[strlen(buffer)-1] = '\0';
		if(verbosity>4) fprintf( stderr, "checking modeline: %s\n", buffer );
		if( strlen(buffer) > 0 && buffer[0] != '#' && isspace(buffer[0]) == 0 ) {
			if( reqmode == atoi(strsep( &buffer, "\t" )) ) { 
				if(verbosity>2) fprintf( stderr, "  Mode Found\n" );
				fclose(dat);
				retbuff = malloc( strlen(buffer)+1 );
				memcpy( retbuff, buffer, strlen(buffer)+1 );
				if(verbosity>3) fprintf( stderr, "  Mode: %s\n", retbuff );
				return retbuff;
			}
		}
	}

	fclose(dat);
	return NULL;
}

int listmodes() {
	FILE *dat;
	int count=0;
	char buffer[2048];

	if( ! (dat = fopen( modefile, "r" )) ) {
		if( ! (dat = fopen( "/etc/hcraft.modes", "r" )) ) {
			fprintf( stderr, "Error opening hcraft.modes (from ./ or /etc): %s\n", strerror(errno) );
			exit(-1);
		}
	}
	while( fgets( buffer, sizeof(buffer), dat ) ) {
		if( buffer[0] != '#' && isspace(buffer[0]) == 0 ) printf( "%s", buffer );
		count++;
	}

	fclose(dat);
	return count;
}

