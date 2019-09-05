#define VERSION "1.0.0"

extern int errno;

/* usage.c */
void version();
void usage( char *prog );

/* modes.c */
char *getmode( int reqmode );
int listmodes();

/* hex.c */
char *hex_encode( char *text );
