#define _POSIX_C_SOURCE 1
#define _GNU_SOURCE
#include "common.h"
#include <fcntl.h>
#include <limits.h>
#include <stdint.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

/*
__attribute__((nonnull)) void show_err_r(const char *str,
                                                         const int err)
{
    char buf[1024];
    strerror_r(err, buf, sizeof(buf));
    fprintf(stderr, ERROR_FORMAT, str, buf);
}
*/

__attribute__((nonnull)) int newfile(FILE **fw, const char *pathname)
{
    if (!*fw)
        return EINVAL;
    errno = 0;
    int fd =
        open(pathname, O_CREAT | O_EXCL | O_RDWR | O_NOCTTY, S_IRUSR | S_IWUSR);
    if (fd == -1)
        return OPEN_ERR;
    errno = 0;
    *fw = fdopen(fd, "w+");
    if (!*fw) {  //
        int tmp = errno;
        if (close(fd))
            return CLOSE_ERR;
        errno = tmp;
        return FDOPEN_ERR;
    }
    return 0;
}

__attribute__((nonnull)) int getnum(long *num, const char *str)
{
    char *ed;
    errno = 0;
    long tmp = strtol(str, &ed, 10);
    if ((tmp == LONG_MAX || tmp == LONG_MIN) && errno == ERANGE)
        return FLOW_ERR;
    if (ed == str)
        return NO_DIG_ERR;
    else if (*ed != '\0')
        return EXTRA_DIG_ERR;
    *num = tmp;
    return 0;
}
