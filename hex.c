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

/* Convert a text string into URI sytlized hex */
char *hex_encode( char *text ) {
   int x, y = 0;
   char *newtext;

   newtext = malloc((strlen(text)+1)*3);
   for( x=0; x<strlen(text); x++ ) {
      switch( text[x] ) {
      case '/' :
         newtext[y] = text[x];
         y++;
         break;
      default :
         /* encode character to hex */
         newtext[y] = '%';
         snprintf( &newtext[y+1], 3, "%x", (unsigned int) text[x] );
         y = y + 3;
         break;
      }
   }
   newtext[y] = '\0';

   return( newtext );
}
