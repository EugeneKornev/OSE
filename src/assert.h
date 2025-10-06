#pragma once

#include "panic.h"

#ifdef DEBUG
#define assert(cond) ((cond) ? (void)(0) : panic(__FILE__ ": %d: assert " #cond " failed", __LINE__))
#else
#define assert(cond) ((void)(0))
#endif
