#include "eva_string.h"
#include <limits.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include "compiler.h"
#include "eva_alloc.h"

#define EVA_T(x) ((eva_t *) ((x) -evaoffset()))
#define Realptr(x) ((x) -evaoffset())

#define MAX_ALLOCATE_SIZE 1024

#ifdef __cplusplus
extern "C" {
#endif

typedef struct EVA eva_t;

static inline size_t evaoffset(void)
{
    return sizeof(uint32_t) * 2;
}

__attribute__((nonnull)) static inline size_t evaspace(evastr nerv)
{
    return EVA_T(nerv)->space;
}

static size_t digit10(const unsigned long long val)
{
    if (val < 10)
        return 1;
    if (val < 100)
        return 2;
    if (val < 1000)
        return 3;
    if (val < 1000000000000) {    // 10 to the power of 12
        if (val < 100000000) {    // 10 to the power of 8
            if (val < 1000000) {  // 10 to the power of 6
                if (val < 10000)
                    return 4;
                return 5 + (val >= 100000);  // 10 to the power of 6
            }
            return 7 + (val >= 10000000);  // 10 to the power of 17
        }
        if (val < 10000000000)               // 10 to the power of 10
            return 9 + (val >= 1000000000);  // 10 to the power of 9
        return 11 + (val >= 100000000000);   // 10 to the power of 11
    }
    return 12 + digit10(val / 1000000000000);  // 10 to the power of 12
}

__attribute__((nonnull)) static inline eva_t *evagrow(evastr src, uint32_t len)
{
    eva_t *obj = EVA_T(src);
    if (obj->space >= len)
        return obj;
    if (len < MAX_ALLOCATE_SIZE)
        len *= 2;
    else if (unlikely(len > UINT32_MAX - MAX_ALLOCATE_SIZE))
        return NULL;
    else
        len += MAX_ALLOCATE_SIZE;
    if (unlikely(sizeof(eva_t) > SIZE_MAX - 1 - len))
        return NULL;
    obj = eva_realloc(obj, sizeof(eva_t) + len + 1);
    if (unlikely(!obj))
        return NULL;
    obj->space = len;
    return obj;
}


evastr evaempty(void)
{
    return evannew("", 0);
}

__attribute__((nonnull)) evastr evanew(const char *src)
{
    return evannew(src, strlen(src));
}

evastr evannew(const char *src, uint32_t len)
{
    if (unlikely(sizeof(eva_t) > SIZE_MAX - 1 - len))
        return NULL;
    eva_t *obj = eva_malloc(sizeof(eva_t) + len + 1);
    if (unlikely(!obj))
        return NULL;
    obj->len = len;
    obj->space = len;
    if (src)
        memcpy(obj->str, src, len);
    else
        memset(obj->str, '\0', len);
    obj->str[len] = '\0';
    return obj->str;
}

__attribute__((nonnull)) evastr evadup(const evastr src)
{
    return evannew(src, evalen(src));
}

evastr evaLL(long long value)
{
    char buf[21];  // long long stinrg contains 20 characters , and we need one
                   // character for null terminated.
    size_t size = 0;
    unsigned long long val;
    if (value < 0) {
        buf[0] = '-';
        val = -value;
        size++;
    } else {
        val = value;
    }
    size += digit10(val);
    size_t temp = size - 1;
    do {
        buf[temp] = '0' + val % 10;
        val /= 10;
        temp--;
    } while (val);
    buf[size] = '\0';
    return evannew(buf, size);
}

__attribute__((nonnull)) evastr
    evancpy(evastr restrict dst, const char *restrict src, const uint32_t len)
{
    eva_t *obj = evagrow(dst, len);
    if (!obj)
        return NULL;
    memcpy(obj->str, src, len);
    obj->len = len;
    obj->str[len] = '\0';
    return obj->str;
}

__attribute__((nonnull)) evastr evacpy(evastr restrict dst, const char *restrict src)
{
    return evancpy(dst, src, strlen(src));
}

__attribute__((nonnull)) evastr evancat(evastr dst, const char *src, const size_t len)
{
    eva_t *obj = EVA_T(dst);
    if (unlikely(len > UINT32_MAX - obj->len))
        return NULL;
    obj = evagrow(dst, len + obj->len);
    if (!obj)
        return NULL;
    memcpy(obj->str + obj->len, src, len);
    obj->len += len;
    obj->str[obj->len] = '\0';
    return obj->str;
}

__attribute__((nonnull)) evastr evacat(evastr dst, const char *src)
{
    return evancat(dst, src, strlen(src));
}

__attribute__((nonnull)) evastr evacateva(evastr dst, const evastr src)
{
    return evancat(dst, src, evalen(src));
}

__attribute__((nonnull)) evastr evacatprintf(evastr dst, const char *format, ...)
{
    va_list args, temp;
    va_start(args, format);
    va_copy(temp, args);  // Because after function call vsnprintf, value of
                          // args is undefined. So we copy args to temp.
    char Sbuf[1024], *buf = Sbuf;
    int len = vsnprintf(NULL, 0, format, args) + 1;
    va_end(args);
    if (unlikely(len < 0))
        goto Fail;
    len++;
    if ((size_t) len > sizeof(Sbuf)) {
        buf = eva_calloc(len, sizeof(char));
        if (!buf)
            goto Fail;
    }
    if (unlikely(vsnprintf(buf, len, format, temp) < 0))
        goto Fail;
    va_end(temp);
    dst = evacat(dst, buf);
    if (buf != Sbuf)
        evafree(buf);
    return dst;
Fail:
    if (buf != Sbuf)
        evafree(buf);
    return NULL;
}

__attribute__((nonnull)) evastr evaresize(evastr dst)
{
    if (evaspace(dst) == 0)
        return dst;
    return eva_realloc(Realptr(dst), sizeof(eva_t) + evalen(dst) + 1);
}

void evafree(evastr nerv)
{
    if (nerv)
        free(Realptr(nerv));
}

#if defined(EVA_TEST)

#include <errno.h>
#include "simpletest.h"
#include "xorrand.h"

#define MAX_STR_SIZE 1024

static inline void randstr(char *str, size_t len)
{
    for (size_t i = 0; i < len; ++i)
        str[i] = RAND() % 26 + 65;
    str[len] = '\0';
}

static int testevannew(unsigned int count)
{
    while (count) {
        char str[MAX_STR_SIZE];
        size_t len = RAND() % sizeof(str);
        randstr(str, len);
        evastr string = evannew(str, len);
        if (!string)
            return 1;
        if (memcmp(str, string, len + 1)) {
            evafree(string);
            return -1;
        }
        if (evalen(string) != len) {
            evafree(string);
            return -2;
        }
        evafree(string);
        count--;
    }
    return 0;
}

static int testevaLL(unsigned int count)
{
    while (count) {
        long long val = RAND();
        evastr string = evaLL(val);
        if (!string)
            return 1;
        char *ed;
        errno = 0;
        long long test = strtoll(string, &ed, 10);
        if ((errno == ERANGE && (test == LLONG_MAX || test == LLONG_MIN)) ||
            (errno != 0 && val == 0)) {
            return 2;
        }
        if (test != val) {
            evafree(string);
            return -1;
        }
        if (*ed != '\0') {
            evafree(string);
            return -2;
        }
        evafree(string);
        count--;
    }
    return 0;
}

static int testevancpy(unsigned int count)
{
    // Copy from larger string
    for (unsigned int i = 0; i < count; ++i) {
        evastr string = evaempty();
        if (!string)
            return 1;
        char str[MAX_STR_SIZE];
        size_t len = RAND() % sizeof(str);
        randstr(str, len);
        evastr test = evancpy(string, str, len);
        if (!test) {
            evafree(string);
            return 1;
        }
        string = test;
        if (memcmp(string, str, len + 1)) {
            evafree(string);
            return -1;
        }
        if (evalen(string) != len) {
            evafree(string);
            return -2;
        }
        evafree(string);
    }

    // Copy frome shorter string
    for (unsigned int i = 0; i < count; ++i) {
        const char evangelion[] = "evangelion";
        evastr src = evanew(evangelion);
        if (!src)
            return 1;
        char str[sizeof(evangelion) - 2];
        size_t len = RAND() % sizeof(str);
        randstr(str, len);
        src = evancpy(
            src, str,
            len);  // No need to check NULL because original string is larger
        if (memcmp(src, str, len + 1)) {
            evafree(src);
            return -3;
        }
        if (evalen(src) != len) {
            evafree(src);
            return -4;
        }
        evafree(src);
    }
    return 0;
}

static int testevancat(unsigned int count)
{
    while (count) {
        char str[MAX_STR_SIZE];
        size_t len = RAND() % sizeof(len);
        randstr(str, len);
        evastr string = evancat(evaempty(), str, len);
        if (!string)
            return 1;
        if (memcmp(string, str, len)) {
            evafree(string);
            return -1;
        }
        if (evalen(string) != len) {
            evafree(string);
            return -2;
        }
        evafree(string);
        count--;
    }
    return 0;
}

int main(void)
{
    seed_rand(seeds);
    const unsigned int count = 100000;

    printf("Each test for " LIGHT_CYAN "%u" NORMAL " times.\n", count);
    test_this("evannew", testevannew(count) == 0);
    test_this("evaLL", testevaLL(count) == 0);
    test_this("evancpy", testevancpy(count) == 0);
    test_this("evancat", testevancat(count) == 0);
    return 0;
}

#endif  // defined(EVA_TEST)

#ifdef __cplusplus
}
#endif
