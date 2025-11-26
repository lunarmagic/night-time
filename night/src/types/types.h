#pragma once

#define S8_MAX INT8_MAX
#define S8_MIN INT8_MIN
#define U8_MAX UINT8_MAX
#define U8_MIN UINT8_MIN

#define S16_MAX INT16_MAX
#define S16_MIN INT16_MIN
#define U16_MAX UINT16_MAX
#define U16_MIN UINT16_MIN

#define S32_MAX INT32_MAX
#define S32_MIN INT32_MIN
#define U32_MAX UINT32_MAX
#define U32_MIN UINT32_MIN

#define S64_MAX INT64_MAX
#define S64_MIN INT64_MIN
#define U64_MAX UINT64_MAX
#define U64_MIN UINT64_MIN

namespace night
{
#ifdef NIGHT_USE_DOUBLE_PRECISION
	typedef double real;
#else
	typedef float real;
#endif

	typedef float r32;
	typedef double r64;

	typedef bool b8;

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

