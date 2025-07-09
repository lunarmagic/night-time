#pragma once

//#include "types/types.h"

namespace night
{

	inline void random_set_seed(u32 seed)
	{
		srand(seed);
	}

	inline s32 random()
	{
		return rand();
	}

	inline s32 random(s32 range)
	{
		return rand() % range;
	}

	//#define RANDOM_REAL_MOD 100000
	inline real random(real range)
	{
		return ((real)(rand()) / (real)(RAND_MAX)) * range;

	}

}