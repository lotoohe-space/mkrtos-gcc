/* 
 *  diet gcc -mpreferred-stack-boundary=2 -malign-functions=0 -malign-jumps=0 -malign-loops=0 -fomit-frame-pointer -Os -o md5sum md5sum.c; elftrunc md5sum md5sum
 */

#include <stdlib.h>
#include <fcntl.h>
#include <md5.h>

#ifndef O_LARGEFILE
#define O_LARGEFILE 0
#endif

static 
void  do_md5 ( int fd, char hexl [17], char hexd [34] ) 
{
    MD5_CTX             context;
    uint8_t             digest [16];
    ssize_t             len;
    unsigned long long  filelen = 0;
    char*               t;
    int                 i;
    int                 c;
    static char         buf  [16384];
    
    MD5Init ( &context );
    
    while ( (len = read ( fd, buf, sizeof(buf) )) > 0 ) {
	filelen += len;
	MD5Update ( &context, buf, len );
    }

    t = hexl + 16;
    *t = ' ';
    do {
        c = ((unsigned int)filelen & 0xf) + '0';
	*--t = c > '9' ? c+'a'-10-'0' : c;
	filelen >>= 4;
    } while ( t > hexl );
    
    MD5Final ( digest, &context );
    t = hexd;
    for ( i = 0; i < 16; i++ ) {
	c = (digest[i] >> 4) + '0';
	*t++ = c > '9' ? c+'a'-10-'0' : c;
	c = (digest[i] & 0xf) + '0';
	*t++ = c > '9' ? c+'a'-10-'0' : c;
    }
    t[0] = t[1] = ' ';
}


int  main ( int argc, char** argv ) 
{
    int          len = 0;
    char         hexl [17];
    char         hexd [34];
    int          fd;
    char*        name;

    if ( argc <= 1 )
        goto only_stdin;
    
    while ( (name = *++argv) != NULL ) {
    
	if ( name[0] == '-'  &&  name[1] == 'n'  &&  name[2] == '\0' ) {
	    len = 1;
	    continue;
	}
	if ( name[0] == '-'  &&  name[1] == '\0' ) {
only_stdin: name = "-";
	    fd   = 0;
	} else {
	    if ((fd = open ( name, O_RDONLY | O_LARGEFILE )) < 0)
	        continue;
	}
	    
	do_md5 ( fd, hexl, hexd );
	if ( len ) 
	    write ( 1, hexl, 17 );
	write ( 1, hexd, 34 );
	write ( 1, name, strlen(name) );
	write ( 1, "\n", 1 );
	if ( fd > 0 )
	    close ( fd );
    }
    
    return 0;
}
