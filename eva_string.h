#ifndef EVA_STRING_H
#define EVA_STRING_H

#include <stddef.h>
#include <stdint.h>
#include "compiler.h"

typedef char *evastr;
struct EVA {
    uint32_t len;    // used size
    uint32_t space;  // total allocate size
    char str[];
};

__attribute__((nonnull)) static inline uint32_t evalen(evastr nerv)
{
    return ((struct EVA *) (nerv - sizeof(uint32_t) * 2))->len;
}

// c-style string to evastr
evastr evaempty(void);
__attribute__((nonnull)) evastr evanew(const char *src);
evastr evannew(const char *src, uint32_t len);
__attribute__((nonnull)) evastr evadup(const evastr src);
evastr evaLL(long long value);

// copy
__attribute__((nonnull)) evastr evacpy(evastr restrict dst, const char *restrict src);
__attribute__((nonnull)) evastr
    evancpy(evastr restrict dst, const char *restrict src, uint32_t len);

// concatenate
__attribute__((nonnull)) evastr evacat(evastr dst, const char *src);
__attribute__((nonnull)) evastr evancat(evastr dst, const char *src, size_t len);
__attribute__((nonnull)) evastr evacateva(evastr dst, const evastr src);
__attribute__((format(printf, 2, 3))) evastr evacatprintf(evastr dst,
                                                          const char *format,
                                                          ...);

// minimize string size
__attribute__((nonnull)) evastr evaresize(evastr dst);

// destory evastr object
void evafree(evastr nerv);

#endif  // EVA_STRING_H
