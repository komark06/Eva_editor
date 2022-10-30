#if !defined(__COMMON_H__)

#define __COMMON_H__

#include <errno.h>
#include <stdio.h>
#include <string.h>

#define ERROR_FORMAT "%s: %s\n"

// error code of the newfile() function
#define OPEN_ERR -1
#define CLOSE_ERR -2
#define FDOPEN_ERR -3

// error code of the getnum() function
#define FLOW_ERR -1
#define NO_DIG_ERR -2
#define EXTRA_DIG_ERR -3

/*
DESCRIPTION:
    Tell compiler do not change the memory access.
TIME: 2022/9/21
*/

__attribute__((always_inline)) static inline void escape(void *p)
{
    __asm__ volatile("" : : "r"(p) : "memory");
}

/*
DESCRIPTION:
    Convert the value of @err to human-readable string and print to stderr.
    First, print @str and followed by a colon and a blank. Then an error message
corresponding to the value of @err and a new-line.

TIME: 2022/9/10
*/
__attribute__((always_inline)) __attribute__((nonnull)) static inline void
show_err(const char *str, const int err)
{
    fprintf(stderr, ERROR_FORMAT, str, strerror(err));
}

/*
DESCRIPTION:
    Take @errno to readable string and print to stderr.

TIME: 2022/9/10
*/
__attribute__((nonnull)) static inline void show_errno(const char *str)
{
    show_err(str, errno);
}

/*
NOTE: Need to rewrite the function. Current version is not portable.
DESCRIPTION:
    Thread-safety version of the unix_error() function.

TIME: 2022/9/11
*/
/*
__attribute__((nonnull)) void show_err_r(const char *str,
                                                         const int err);
*/

/*
DESCRIPTION:
    Close @stdout and @stderr. If closing @stdout failed, the clearout funtion
will output error message to @stderr.

NOTE:
    The C standard makes no guarantees as to when output to stdout (standard
output) or stderr (standard error) is actually flushed. Before program being
terminated, output stream requires to flush and close. Typically, the clearout()
funtion will call by the the atexit() function in order to close @stdout and
@stderr at the end of the program. Reference:
https://wiki.sei.cmu.edu/confluence/display/c/FIO23-C.+Do+not+exit+with+unflushed+data+in+stdout+or+stderr

TIME: 2022/9/10
*/
static inline void clearout(void)
{
    errno = 0;
    if (fclose(stdout))
        show_errno("fclose");
    fclose(stderr);
}

/*
DESCRIPTION:
    Create a new file whose name is the string pointed to by @pathname.
    File is opened for reading and writing and its position is at the beginning.
    If @pathname is symbolic link, the newfile() funtion shall fail.
    File permission: user read permission and user write permission.

    @info: pointer to store file information.
    @pathname: name of new file.

RETURN VALUE:
    On succeed, return 0 and store FILE pointer to @fw.
    On error, return error value and set @errno. The show_err() function and the
show_errno() function translate @errno to a human readable string.

ERROR:
    EINVAL: invalid argument.
    OPEN_ERR: The open() function failed.
    CLOSE_ERR: The close() function failed.
    FDOPEN_ERR : The fdopen() function failed.

TIME: 2022/9/11
*/
__attribute__((nonnull)) int newfile(FILE **fw, const char *filename);

/*
DESCRIPTION:
    Convert a string to integer.
    @num: pointer to store converted number.
    @str: original string that contain number.

RETURN VALUE:
    On succeed, return 0.
    On error, return negative value.

ERROR:
    NO_DIG_ERR: There is no digit in @str, or @str contain non-digit characters.
    FLOW_ERR: @str will cause overflow or underflow.

TIME: 2022/9/15
*/
__attribute__((nonnull)) int getnum(long *num, const char *str);

/*
DESCRIPTION:
    Test return value of the getnum() function. And print error message to
@stderr.
    @num: return value of the getnum().
    @str: original string to convert to number.

RETURN VALUE:
    If @err is not equal to zero, return -1.
    Else, return 0;

TIME: 2022/8/6
*/
__attribute__((always_inline)) __attribute__((nonnull)) static inline int
check_getnum(int err, const char *str)
{
    switch (err) {
    case FLOW_ERR:
        fprintf(stderr, "[%s] cause overflow or underflow.\n", str);
        goto Fail;
    case NO_DIG_ERR:
        fprintf(stderr, "[%s] contains non-digit characters.\n", str);
        goto Fail;
    case EXTRA_DIG_ERR:
        fprintf(stderr, "[%s] contains extra non-digit characters.\n", str);
        goto Fail;
    }
    return 0;
Fail:
    return -1;
}

#endif  // !defined(__COMMON_H__)
