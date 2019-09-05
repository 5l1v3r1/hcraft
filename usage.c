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


void version() {
	fprintf( stderr, "hcraft %s\n", VERSION );
	fprintf( stderr, "10.2004 - I)ruid <druid@caughq.org> & intropy <intropy@caughq.org>\n\n" );
}

void usage( char *prog ) {
	version();
	fprintf( stderr, "Usage: %s [options] (-m <mode> | -i <start>:<end>) -h <host> (</full/path/to/remotefile> | <commandline> | <remote-script-uri)\n", prog );
	fprintf( stderr, "  options:\n" );
	fprintf( stderr, "    -h <host>         Target Host\n" );
	fprintf( stderr, "    -i <start>:<end>  Increment through modes from <start> to <end>\n" );
	fprintf( stderr, "    -l                List avalable modes and exit\n" );
	fprintf( stderr, "    -m <mode>         Use mode # <mode> (default = 0)\n" );
	fprintf( stderr, "    -M <modefile>     Use modefile <modefile> (default ./hcraft.modes | /etc/hcraft.modes)\n" );
	fprintf( stderr, "    -p <port>         Target HTTP Port\n" );
/* Add this once the modes file is in a retrievable location
	fprintf( stderr, "    -u         Update modes file\n" );
*/
	fprintf( stderr, "    -v         Increase verbosity (repeat for additional verbosity)\n" );
	fprintf( stderr, "    -V                Print version information and exit\n" );
	exit(-1);
}

