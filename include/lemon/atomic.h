#include <stdlib.h>

// val 自增1，类似于i++
#define ATOMIC_INC(val) (void)__sync_add_and_fetch((val), 1)
// val 自减1，类似于i--
#define ATOMIC_DEC(val) (void)__sync_sub_and_fetch((val), 1)

#define ATOMIC_ADD(val, addv) (void)__sync_fetch_and_add((val), addv)
#define ATOMIC_SUB(val, addv) (void)__sync_fetch_and_sub((val), addv)

#define ATOMIC_CAS(val, cmpv, newv) __sync_val_compare_and_swap((val), (cmpv), (newv))
