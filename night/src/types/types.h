#pragma once

#define S32_MAX INT_MAX
#define S32_MIN INT_MIN

namespace night
{
#ifdef NIGHT_USE_DOUBLE_PRECISION
	typedef double real;
#else
	typedef float real;
#endif
	typedef float r32;
	typedef double r64;

	typedef unsigned char u8;
	typedef unsigned short u16;
	typedef unsigned int u32;
	typedef unsigned long u64;
	typedef unsigned long long u128;

	typedef char s8;
	typedef short s16;
	typedef int s32;
	typedef long s64;
	typedef long long s128;
}

