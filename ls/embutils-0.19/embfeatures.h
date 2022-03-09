/* ls: */

/* use /etc/passwd and /etc/group to look up names for uid and gid?
 * This adds about 1900 bytes on x86. */
#define SUPPORT_SYMBOLIC_UID_GID

/* support -h and -H?  This adds about 1200 bytes on x86. */
#define SUPPORT_HUMAN_READABLE_SIZE

/* use long long to output file size (ls will always use stat64, but
 * won't display sizes >4gig properly).  This adds about 1250 bytes on x86. */
//#define SUPPORT_64BIT_FILESIZE

/* these will only be used if stdout is a tty, adds about 1200 bytes on
 * x86.*/
#define SUPPORT_COLORS

/* tar: */
#define TAR_SUPPORT_SYMBOLIC_UID

#if defined(__alpha__) || defined(__mips64) || defined(__sparc_v9__) || defined(__x86_64__) || defined(__ia64__)
#undef SUPPORT_64BIT_FILESIZE
#endif
