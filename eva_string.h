#ifndef EVA_STRING_H
#define EVA_STRING_H

#include <stddef.h>
#include <stdint.h>

typedef char *evastr;

__attribute__((nonnull)) uint32_t evalen(evastr nerv);
evastr evaempty(void);
__attribute__((nonnull)) evastr evanew(const char *src);
evastr evannew(const char *src, uint32_t len);
__attribute__((nonnull)) evastr evadup(const evastr src);
evastr evaLL(long long value);
__attribute__((nonnull)) evastr evacpy(evastr restrict dst,
                                       const char *restrict src);
__attribute__((nonnull)) evastr evancpy(evastr restrict dst,
                                        const char *restrict src,
                                        uint32_t len);
__attribute__((nonnull)) evastr evacat(evastr dst, const char *src);
__attribute__((nonnull)) evastr evancat(evastr dst,
                                        const char *src,
                                        size_t len);
__attribute__((nonnull)) evastr evacateva(evastr dst, const evastr src);
__attribute__((format(printf, 2, 3))) evastr evacatprintf(evastr dst,
                                                          const char *format,
                                                          ...);

__attribute__((nonnull)) evastr evaresize(evastr dst);
void evafree(evastr nerv);

#endif  // EVA_STRING_H
