// IBM Engine: memory pool glue for the features' own modules.

#pragma once

#include "Common/GameMemory.h"

// Memory pool glue with the pool size given here, so new modules need no entry in the engine's
// pool table (GameMemoryInitPools). The null memory build (DISABLE_GAMEMEMORY) has no pools.
#ifdef DISABLE_GAMEMEMORY
#define IBM_MEMORY_POOL_GLUE(ARGCLASS, ARGPOOLNAME, ARGINITIAL, ARGOVERFLOW) MEMORY_POOL_GLUE_WITHOUT_GCMP(ARGCLASS)
#else
#define IBM_MEMORY_POOL_GLUE(ARGCLASS, ARGPOOLNAME, ARGINITIAL, ARGOVERFLOW) MEMORY_POOL_GLUE_WITH_EXPLICIT_CREATE(ARGCLASS, ARGPOOLNAME, ARGINITIAL, ARGOVERFLOW)
#endif
