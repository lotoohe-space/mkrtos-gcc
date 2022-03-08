#include "dietdirent.h"
#include <sys/mman.h>
#include <unistd.h>
#include <dirent.h>
#include <stdlib.h>
#include <fcntl.h>

DIR*  opendir ( const char* name ) {
    int   fd = open (name, O_RDONLY | O_DIRECTORY);
    DIR*  t  = NULL;

    if ( fd >= 0 ) {
        unsigned int res;
        if (fcntl (fd, F_SETFD, FD_CLOEXEC) < 0)
            goto lose;
        res = (DIR *) mmap (NULL, PAGE_SIZE, PROT_READ | PROT_WRITE,
                            MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
        if (res <0) {
            t=NULL;
            lose:
            close(fd);
        }
        else {
            t=(void*)res;
            t->fd = fd;
        }
    }


    return t;
}
