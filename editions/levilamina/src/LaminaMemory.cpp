// The official LL loader requires the actual shared allocator overrides.
// Include exactly once; never export a marker without these operators.
#define LL_MEMORY_OPERATORS
#include "ll/api/memory/MemoryOperators.h"
