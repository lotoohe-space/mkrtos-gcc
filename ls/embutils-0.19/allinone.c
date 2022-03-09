/*
 *  So, das erste Multifunktionspaket
 *    Jedes sollte gerade knapp 4 Kbyte groß sein, da die meisten Dateisysteme mit Clustergrößen von 4 KByte arbeiten.
 *    Naja, jetzt bin ich schon bei 7,2 kByte :-( Man kann aber '#define ALL' auskommentieren und sollte dann bei knapp
 *    4 KByte landen ...
 *
 *  Intel:
 *    n=archbasenameclearchvtdirnamedmesgdomainnameenvechofalsehostnamepwdsleepsyncteetruettyunamewhoamiwhichyes
 *    diet gcc -Os -mpreferred-stack-boundary=2 -fomit-frame-pointer -falign-labels=0 -malign-jumps=0 -malign-loops=0 -malign-functions=0 -o $n -s $n.c write12.S
 *    elftrunc $n $n
 *
 *  andere:
 *    n=archbasenameclearchvtdirnamedmesgdomainnameenvechofalsehostnamepwdsleepsyncteetruettyunamewhoamiwhichyes
 *    diet gcc -Os -mpreferred-stack-boundary=2 -fomit-frame-pointer -falign-labels=0 -malign-jumps=0 -malign-loops=0 -malign-functions=0 -o $n -s $n.c write1.c write2.c
 *    elftrunc $n $n
 *
 *  Name the executable arch and make symbolic or hardlinks to this file named after the tool which it should be:
 *    arch, basename, clear, chvt, dirname, dmesg, domainname, env, echo, false, hostname, pwd, sleep, sync, tee, true, tty, uname, whoami, which, yes
 *  These are 21 tools with 5628 bytes => 351 bytes per tool.
 *
 *  Single tools vs. combined tool:                11072 bytes vs. 5628 bytes
 *  Single tools vs. combined tool (1KB cluster):  20480 bytes vs. 6144 bytes
 *  Single tools vs. combined tool (2KB cluster):  36864 bytes vs. 6144 bytes
 *  Single tools vs. combined tool (4KB cluster):  65536 bytes vs. 8192 bytes
 *
 *  If you're not defining ALL you don't have env and whoami. You get a 2504 byte binary with 14 functions.
 *  This is 178 byte/function.
 */

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <limits.h>
#include <string.h>
#include <unistd.h>
#include <pwd.h>
#include <fcntl.h>
#ifdef __linux__
#include <linux/vt.h>
#endif
#include <stdlib.h>
#include <errno.h>
#include <time.h>
#include <signal.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <sys/utsname.h>
#include "write12.h"

#ifdef __OpenBSD__
extern char** environ;
#endif

extern int  __syscall_syslog ( int type, char* bufp, int len );


#define ALL

#ifndef PATH_MAX
# define PATH_MAX  4096
#endif


const char  clearscreen [] = "\e[H\e[J";
const char  nl          [] = "\n";
const char  spc         [] = " ";
const char  point       [] = ".";
const char  unknown     [] = "unknown";
const char  tty0        [] = "/dev/tty0";
const char  console     [] = "/dev/console";
const char  nametoolong [] = "name too long\n";
const char  mustbesuper [] = "must be superuser\n";
const char  notpermit   [] = "Operation not permitted\n";
const char  notatty     [] = "not a tty\n";
const char  y           [] = "y";
const char  quiet       [] = "-quiet";
const char  silent      [] = "-silent";
#ifdef ALL
const char  illegal     [] = "only allowed:\tarch, basename, clear, chvt, dirname, dmesg, domainname, echo, env, false, hostname, pwd, sleep, sync, tee, true, tty, uname, which, whoami, yes\n";
#else
const char  illegal     [] = "only allowed:\tarch, basename, clear, chvt, dirname, dmesg, domainname, echo, false, hostname, pwd, sleep, sync, tee, true, tty, uname, which, yes\n";
#endif


static void  __attribute__((regparm(1))) disp ( const char *s )
{
    static int  notfirst;

    if ( notfirst )
        notfirst = __write1 (spc);
    __write1 (s);
}


static void  __attribute__ ((regparm(1))) interpret_esc ( unsigned char* src )
{
    unsigned char *dst = src;

    do {
        if ( (*dst = *src) == '\\' ) {
            switch (*++src) {
            case 'a' : *dst = '\a'; break;
            case 'b' : *dst = '\b'; break;
            case 'f' : *dst = '\f'; break;
            case 'n' : *dst = '\n'; break;
            case 't' : *dst = '\t'; break;
            case 'v' : *dst = '\v'; break;
            case '\\': *dst = '\\'; break;
            default:
                *dst = 0;
                while ( (unsigned int)(*src - '0') < 8u )
                    *dst = *dst * 8 + (unsigned int)(*src++ - '0');
                src--;
                break;
            }
        }
        dst++;
    } while ( *src++ );
}



#define _(x1,x2,x3,x4)  (((((((x4)<<8)|(x3))<<8)|(x2))<<8)|(x1))
#define M        1
#define N        2
#define R        4
#define S        8
#define P       16
#define V       32
#define D       64


int  main ( int argc, char** argv )
{
    register char*  p;
    char*           q;
    unsigned int    flags;
    int             fd;
    struct passwd*  pwd;
    static struct utsname  un;
    static char     buff [PATH_MAX + 1];
    static char     buf  [32 * 1024];

 
    p = strrchr ( argv[0], '/' );
    if (p)
        p = p + 1;
    else
        p = argv[0];

#ifdef __i386__
    switch ( *(unsigned long*)p ) {
#else
    switch ( _(p[0],p[1],p[2],p[3]) ) {
#endif
  
    case _('a','r','c','h'):    // arch
        flags = M;
        goto uname2;

    case _('b','a','s','e'):    // basename
        if ( argv[1] ) {
            p = strrchr ( argv[1], '/' );
            if (p)
                p = p + 1;
            else
                p = argv[1];
            if ( argv[2] ) {
                q = p + strlen (p) - strlen (argv[2]);
                if ( q >= p  &&  strcmp (q, argv[2]) == 0)
                    *q = '\0';
            }
            __write1 ( p );
            __write1 ( nl );
        }
        break;

#ifdef __linux__
    case _('c','h','v','t'):    // chvt
        if ( argv[1] == NULL )
            return 1;
        flags = atoi ( argv[1] );
        fd = open ( tty0, O_RDONLY );
        if ( fd < 0 ) fd = open ( console, O_RDONLY) ;
        if ( fd < 0 ) return 1;
        if ( ioctl (fd, VT_ACTIVATE, flags)  ||  ioctl ( fd, VT_WAITACTIVE, flags) )
            return 1;
        close (fd);
        break;
#endif

    case _('c','l','e','a'):    // clear
        __write1 ( clearscreen );
        break;

#ifdef __linux__
    case _('d','m','e','s'):    // dmesg
	__syscall_syslog ( 3, buf, sizeof(buf) );
#endif

	while ( (p = *++argv) != NULL ) {
	    if ( *p == '-' ) {
		while (*++p) {
		    if ( *p == 'n' ) {
			if ( __syscall_syslog ( 8, 0, atoi ( *++argv ) ) ) {
			    __write2 ( notpermit );
			    return 1;
			}
			return 0;
		    }
		}
	    }
	}
	
	for ( p = buf;; ) {
	    if ( p[0] == '<' ) {
		p = strchr (p, '>');
		if (p == NULL)
		    break;
		p++;
	    }
	    q = p;
	    p   = strchr ( p, '\n' );
	    if ( p == NULL )
		break;
	    p++;
	    write ( 1, q, p-q );
	}
	break;

    case _('d','i','r','n'):    // dirname
        if ( argv[1] ) {
            p = strrchr ( argv[1], '/');
            if (p) {
                *p = '\0';
                p = argv[1];
            } else {
                p = (char*)point;
            }
            __write1 ( p );
            __write1 ( nl );
        }
        break;

    case _('d','o','m','a'):    // domainname
        if ( argv[1] != NULL ) {
            if ( setdomainname ( argv[1], strlen (argv[1]) ) ) {
         error: switch ( errno ) {
                case EINVAL: __write2 ( nametoolong ); return 1;
                case EPERM : __write2 ( mustbesuper ); return 1;
                default    : return 1;
                case 0     : break;
                }
            }
        } else {
#ifdef __linux__
            flags = D;
            goto uname2;
#else
	    char buf[1024];
	    int l;
	    getdomainname(buf,sizeof(buf));
	    buf[sizeof(buf)-1]=0;
	    l=strlen(buf);
	    buf[l]='\n';
	    write(1,buf,l+1);
#endif
        }
        break;

    case _('e','c','h','o'):    // echo
	{
	    int    escape  = 0;
	    char*  blank   = (char*)spc+1;
	    char*  newline = (char*)nl;

	    while ( (p = *++argv) != NULL ) {
		__write1 ( blank );
		if ( *p == '-' ) {
		    while ( *++p ) {
			switch ( *p ) {
			case 'n': newline = (char*)nl+1; break;
			case 'e': escape  = 1;    break;
			case 'E': escape  = 0;    break;
			}
		    }
		} else {
		    if ( escape )
			interpret_esc ( p );
		    __write1 ( p );
		    blank = (char*)spc;
		}
	    }
	    __write1 ( newline );
	}
    break;

#ifdef ALL
    case _('e','n','v','\0'):   // env
        while ( (p = *++argv) != NULL ) {
            if ( p[0] == '-' ) {
                if ( p[1] == '\0' ) {
                   environ[0] = NULL;
                } else {
                    while (*++p)
                        switch ( *p ) {
                        case 'i': environ[0] = NULL; break;
                        case 'u': putenv (*++argv);  goto while_break;
                        }
        while_break:;
                }
            } else {
                if ( strchr (p, '=') ) {
                    putenv ( p );
                } else {
                    execvp ( p, ++argv );
                    return 0;
                }
            }
        }

        while ( *environ ) {
            __write1 ( *environ++ );
            __write1 ( nl );
        }
        return 1;
#endif

    case _('f','a','l','s'):    // false
        return 1;
      
    case _('h','o','s','t'):    // hostname
        if ( argv[1] != NULL ) {
            if ( sethostname ( argv[1], strlen(argv[1]) ) )
                goto error;
        } else {
            flags = N;
            goto uname2;
        }
        break;

    case _('p','w','d','\0'):   // pwd
        p = getcwd ( buff, PATH_MAX );
        if (p == NULL )
            p = (char*)point;
        __write1 ( p );
        __write1 ( nl );
        break;

    case _('s','l','e','e'):    // sleep
	{
	    unsigned int  sec;
	    
	    p = argv[1];
	    
	    if ( p == NULL )
		return 1;
	    
	    sec = 0;
	    while ( (unsigned int)(*p -'0') < 10u )
	        sec = 10u*sec + (*p++ -'0');
	    
	    switch (*p) {
	    case 'd': fd = 24, sec *= fd;
	    case 'h': fd = 60, sec *= fd;
	    case 'm': fd = 60, sec *= fd;
	    }
	    sleep (sec);
	}
	break;

    case _('s','y','n','c'):    // sync
        sync (); return 0;

    case _('t','e','e','\0'):   // tee
	{
	    int   attrib = O_WRONLY | O_CREAT | O_TRUNC;
	    int   len;
	    int   j;
	    int   k;
	    int*  fds = alloca ( argc * sizeof(int) );
	    
	    fds [k = 0] = 1;		/* stdout */
	    
	    while ( (p = *++argv) != NULL ) {
		if ( *p == '-' ) {
		    while (*++p) {
			switch (*p) {
			case 'i': 
			    signal ( SIGINT, SIG_IGN ); 
			    break;
			case 'a': 
			    attrib = O_WRONLY | O_APPEND | O_CREAT; 
			    break;
			}
		    }
		} else {
		    
		    if ( (fds [++k] = open (p, attrib, 0644) ) < 0 ) {
			__write2 ( "could not open " ); 
			__write2 ( p ); 
			__write2 ( nl );
		    }
		}
	    }
	    
	    while ( (len = read ( 0, buf, sizeof(buf) )) != 0 ) {
		
		if ( len < 0 ) {
		    __write2 ( "read error.\n" );
		    return 1;
		}
		for ( j = 0; j <= k; j++ ) {
		    if ( fds[j] >= 0 )
			if ( write ( fds[j], buf, len ) != len )
			    __write2 ( "short write (disk full?)\n" );
		}
	    }
	}
	break;
        
    case _('t','r','u','e'):    // true
        break;

    case _('t','t','y','\0'):   // tty
        q     = ttyname (0);
        flags = 0;
        if ( (p = argv[1]) != NULL ) {
            if ( *p++ == '-' ) {
                switch (*p) {
                case '-':
                        if ( strcmp(p,quiet) == 0  ||  strcmp(p,silent) == 0 )
                case 's':
                            flags = 1;
                        break;
                default:
                        break;
                }
            }
        }
        if ( q == NULL ) {
            if ( flags == 0 )
                __write2 ( notatty );
            return 1;
        }
        if ( flags == 0 ) {
            __write1 ( q );
            __write1 ( nl );
        }
        break;

    case _('u','n','a','m'):    // uname
        flags = 0;
        while ( (p = *++argv) != NULL )
            if ( *p == '-' )
                while (*++p)
                    switch (*p) {
                    case 'm': flags |= M; break;
                    case 'n': flags |= N; break;
                    case 'r': flags |= R; break;
                    case 's': flags |= S; break;
                    case 'p': flags |= P; break;
                    case 'v': flags |= V; break;
                    case 'a': flags |= (M|N|R|S|P|V); break;
                    }
    uname2:
        uname (&un);
        if (flags == 0)
            flags |= S;
        if (flags & S) disp ( un.sysname    );
        if (flags & N) disp ( un.nodename   );
        if (flags & R) disp ( un.release    );
        if (flags & V) disp ( un.version    );
        if (flags & M) disp ( un.machine    );
        if (flags & P) disp ( unknown       );
#ifdef __linux__
        if (flags & D) disp ( un.domainname );
#endif
        __write1 ( nl );
        break;

    case _('w','h','i','c'):    // which
	{
	    struct stat  ss;
	    char*        path;
	    char*        pathend;
	    int          all = 0;
	    char*        bufptr;
	    int          len1;
	    int          len2;
	    
	    while ( (p = *++argv) != NULL ) {
		
		if ( 0 == strcmp (p, "-a" ) ) {
		    all |= 1;
		    continue;
		}
		
		path  = getenv ("PATH");
		
		while ( path != NULL ) {
		    pathend = strchr (path, ':');
		    if ( pathend == NULL ) 
			pathend = strchr (path, '\0');
		    
		    len1          = pathend - path;
		    len2          = strlen (p);
		    bufptr        = alloca ( len1 + len2 + 2 );
		    memcpy ( bufptr, path, len1 );
		    bufptr [len1] = '/';
		    memcpy ( bufptr + len1 + 1, p, len2 + 1 );
		    
		    if ( stat (bufptr, &ss) == 0 ) {
			__write1 ( bufptr );
			__write1 ( nl );
			if (all == 0) 
			    break;
		    }
		    
		    if (*pathend == '\0')
			break;
		    path = pathend + 1;
		}
	    }
	    
	}
	break;
	
#ifdef ALL
    case _('w','h','o','a'):    // whoami
        if ( (pwd = getpwuid (geteuid()) ) == NULL )
            return 1;
        __write1 ( pwd -> pw_name );
        __write1 ( nl );
        break;
#endif	

    case _('y','e','s','\0'):   // yes
        while (1) {
            struct timespec  t;
            if (argv[1] == NULL)
                __write1 ( y );
            else
                for ( flags = 0; ; ) {
                    __write1 ( argv[++flags] );
                    if ( flags >= argc-1 )
                        break;
                    __write1 ( spc );
                }         
            t.tv_nsec = __write1 ( nl );
            t.tv_sec  = 0;                      // avoid silly terminal and CPU flooding
            nanosleep ( &t, NULL );
        }  

    default        :
        __write2 (illegal);
        return 127;
    }
  
    return 0;
}
