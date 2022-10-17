#if !defined(__EVA_ALLOC_H__)
#include <stdlib.h>

#ifdef __cplusplus
extern "C" {
#endif

static inline void *eva_malloc(size_t size)
{
    return malloc(size);
}
static inline void eva_free(void *ptr)
{
    free(ptr);
}
static inline void *eva_calloc(size_t nmemb, size_t size)
{
    return calloc(nmemb, size);
}
static inline void *eva_realloc(void *ptr, size_t size)
{
    return realloc(ptr, size);
}

#ifdef __cplusplus
extern "C"
}
#endif

#endif  // !defined(__EVA_ALLOC_H__)