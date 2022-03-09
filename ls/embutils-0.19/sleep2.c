/*
 * possible usages:
 *
 *   sleep 12		sleep 12 seconds
 *   sleep 12m		sleep 12 minutes
 *   sleep 12h		sleep 12 hours
 *   sleep 12d		sleep 12 days
 *   sleep 12.345	sleep 12 seconds and 345 milliseconds
 *   sleep 12:34:56	sleep until 12:34:56 o'clock
 *   sleep :34:56	sleep until  *:34:56 o'clock
 *   sleep 12:34	sleep until 12:34:00 o'clock
 *   sleep :34		sleep until  *:34:00 o'clock
 *
 */

#include <stdio.h>
#include <time.h>
#include <unistd.h>
#include <sys/time.h>


static const char  usage_msg [] = 
    "Usage:\tsleep [OPTION]\n"
    "\tsleep NUMBER[SUFFIX]\n"
    "\tsleep NUMBER.NUMBER\n"
    "\tsleep HOUR:MIN:SEC\n"
    "\tsleep :MIN:SEC\n"
    "\n"
    "Pause for NUMBER seconds or until HOUR:MIN:SEC\n"
    "SUFFIX may be s to keep seconds, m for minutes, h for hours or d for days.\n"
    "\n"
    "  --help\tdisplay this help and exit\n"
    "  --version\toutput version information and exit\n"
    "\n"
    "Report bugs to <pfk@uni-jena.de>.\n";

static const char  version_msg [] = 
    "sleep 3.0 (Kutils)\n"
    "Written by <pfk@uni-jena.de>\n";

static const char  too_few_args [] =
    "sleep: too few arguments\n"
    "Try 'sleep --help' for more information.\n";

static const char  try_help [] =
    "'\nTry 'sleep --help' for more information.\n";


static void message ( const char* p )
{
    write ( 2, p, strlen (p) );
}


// Read a non negative integer number
// Modifies the pointer (*p) to the first non-digit character in (*p)
// no overrun check

static unsigned long  readno ( char const* * pp )
{
    const char*    p   = *pp;
    unsigned long  ret = 0;
    
    while ( (unsigned int)(*p - '0') < 10u )
        ret = 10*ret + (unsigned int)(*p++ - '0');
    *pp = p;
    return ret;
}

// Read the fractional part of a number. gets a pointer to the first
// character which has a weight of 10^8. The next numbers have the weights 10^7, 10^6 ...
// Modifies the pointer (*p) to the first non-digit character in (*p)

static inline unsigned long  readno_ns ( char const* * pp )
{
    const char*    p   = *pp;
    unsigned long  mul = 1000000000;
    unsigned long  ret = 0;
    
    while ( (unsigned int)(*p - '0') < 10u )
        mul /= 10,
        ret += mul * (unsigned int)(*p++ - '0');
    *pp = p;
    return ret;
}

//
//  Computer the difference between localtime and UTC in seconds. Can be a value between
//  -43200 and +43200. Daylight switching is handled correctly.
//

time_t  timezonediff ( time_t value )
{
    struct tm    t;
    time_t       diff;

    localtime_r ( &value, &t ); diff  = ((t.tm_wday * 24 + t.tm_hour) * 60 + t.tm_min) * 60 + t.tm_sec;
    gmtime_r    ( &value, &t ); diff -= ((t.tm_wday * 24 + t.tm_hour) * 60 + t.tm_min) * 60 + t.tm_sec;
    
    if ( diff < -43200 )
        diff += 7 * 86400;
    if ( diff > +43200 )
        diff -= 7 * 86400;
    return diff;
}


int  main ( int argc, char** argv )
{
    int              hour = 0;
    int              min  = 0;
    time_t           sec  = 0;
    long             nsec = 0;
    int              clockmode = 0;    	// not waiting a time in seconds, but for a specified time
    const char*      p;
    struct timespec  tv;	       	// das ist der Typ mit Sekunden und Nanosekunden
    struct timeval   t;			// das ist der Typ mit Sekunden und Mikrosekunden
    
    
    if ( argc <= 1 ) {
        message ( too_few_args );
        return 1;
    }
    
    p = argv[1];
    
    switch (*p) {
    case '0':
    case '1':
    case '2':
    case '3':
    case '4':
    case '5':
    case '6':
    case '7':
    case '8':
    case '9': 
	sec = readno (&p);
	switch (*p) {
	case 'd':
	case 'D': 
	    sec *= 24;
	case 'h':
	case 'H': 
	    sec *= 60;
	case 'm':
	case 'M': 
	    sec *= 60;
	case '\0':
	case 's':
	case 'S': 
	    break;
	case '.': 
	    p++;
	    nsec = readno_ns (&p);
	    break;
	case ':':
	    clockmode = 1;
	    hour = sec;
	    sec  = 0;
	    p++;
	    min = readno (&p);
	    switch (*p) {
	    case ':': 
		p++;
		sec = readno (&p);
	    case '\0':
		break;
	    default:
	        goto error;
	    }
	    break;
	default:
	    goto error;
	}
	break;
    case ':': 
	clockmode = 1;
	hour = -1;
	p++;
	min = readno (&p);
	switch (*p) {
	case ':': 
	    p++;
	    sec = readno (&p);
	case '\0':
	    break;
	default:
	    goto error;
	}
	break;
    case '-':
        if ( 0 == strcmp (p, "--help") ) {
            message ( usage_msg );
            return 1;
        }
        if ( 0 == strcmp (p, "--version") ) {
            message ( version_msg );
            return 1;
        }
    default:
    error:
        message ( "sleep: illegal argument '");
        message ( argv[1] );
        message ( try_help );
	return 0;
    }

    
    if (clockmode) {

        gettimeofday ( &t, NULL );
        
        t.tv_sec += timezonediff (t.tv_sec);
        t.tv_sec %= 86400;

#ifdef DEBUG
        printf ("Time is    %2d:%02d:%02d\n", t.tv_sec/3600, t.tv_sec%3600/60, t.tv_sec%60 );
        printf ("Wait until %2d:%02d:%02d\n", hour, min, sec );
#endif        
        
        // waits 1 ns too less. This eases some calculations
        sec = sec + 60*min + 3600*hour - 1;

        // no hour specified, so wait until the next possible hour
        if ( hour == -1 ) {
            while ( sec < t.tv_sec )
                sec += 3600;
        }
        
        // wait less than 0 seconds, then add 24 hours more
        // may be it is better to always map to [-1:00:00, 24:00:00] instead of [0:00:0, 24:00:00]
        // currently a 'sleep 20:15' waits nearly 24 hours when executed at 20:15:01
        if ( sec < t.tv_sec )
            sec += 86400;
      
#ifdef DEBUG            
        printf ("Wait until %2d:%02d:%02d.999999999\n", sec/3600, sec%3600/60, sec%60 );
#endif        
        
	tv.tv_sec  = sec       - t.tv_sec;
	tv.tv_nsec = 999999999 - t.tv_usec * 1000; 
    }
    else {
	tv.tv_sec  = sec;
	tv.tv_nsec = nsec; 
    }
    
    
#ifdef DEBUG    
    printf ( "Waiting %u.%09u sec\n", tv.tv_sec, tv.tv_nsec );
#endif    
    
    while (nanosleep ( &tv, &tv )) 
	;
    
    return 0;
}

/* end of sleep.c */
