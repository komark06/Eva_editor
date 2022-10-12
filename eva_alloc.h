#if !defined(__EVA_ALLOC_H__)
#include <stdlib.h>
#define e_malloc malloc
#define e_calloc calloc
#define e_realloc realloc
#define e_free free

#ifdef __cplusplus
extern "C" {
#endif

static inline void *eva_malloc(size_t size)
{
    return e_malloc(size);
}
static inline void eva_free(void *ptr)
{
    e_free(ptr);
}
static inline void *eva_calloc(size_t nmemb, size_t size)
{
    return e_calloc(nmemb, size);
}
static inline void *eva_realloc(void *ptr, size_t size)
{
    return e_realloc(ptr, size);
}

#ifdef __cplusplus
extern "C"
}
#endif

#endif  // !defined(__EVA_ALLOC_H__)