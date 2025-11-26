#pragma once

#include <memory>
#include <array>
#include <vector>
#include <deque>
#include <stack>
#include <map>
#include <unordered_map>
#include <unordered_set>
#include <functional>
#include <queue>
#include <list>
#include <set>
#include <sstream>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <typeinfo>
#include <typeindex>
#include <type_traits>
#include <fstream>
#include <algorithm>
#include <execution>
#include <future>
#include <mutex>
#include <filesystem>
#include <chrono>
#include <optional>
#include <cmath>
#include <iostream>
#include <initializer_list>
#include <utility>


#include "types/types.h"

#define SWAP(x, y) std::swap(x, y)
#define MIN(a, b) (a < b ? a : b)
#define MAX(a, b) (a > b ? a : b)
#define CLAMP(x, lower_bound, upper_bound) MIN(MAX(x, lower_bound), upper_bound)
#define BIT(x) (1 << x)
#define SORT3(a, b, c) {if(b < a) SWAP(a, b); if(c < b) SWAP(b, c); if(b < a) SWAP(a, b);}

namespace night
{
	template<typename T1, typename T2>
	using pair = std::pair<T1, T2>;

	template<typename... Args>
	using tuple = std::tuple<Args...>;

	template<typename T>
	using queue = std::queue<T>;

	template<typename T, s32 S>
	using array = std::array<T, S>;

	template<typename T>
	using vector = std::vector<T>;

	template<typename T>
	using deque = std::deque<T>;

	template<typename T>
	using stack = std::stack<T>;

	template<typename T>
	using list = std::list<T>;

	template<typename K, typename T>
	using map = std::map<K, T>;

	template<typename K, typename T, typename _Pr = std::less<K>>
	using multimap = std::multimap<K, T, _Pr>;

	template<typename K, typename T>
	using umap = std::unordered_map<K, T>;

	template<typename K, typename T>
	using umultimap = std::unordered_multimap<K, T>;

	template<typename T>
	using set = std::set<T>;

	template<typename T>
	using uset = std::unordered_set<T>;

	template<typename T>
	using function = std::function<T>;

	using string = std::string;

	template<typename... Args>
	inline auto bind(Args&&... args)
	{
		return std::bind(std::forward<Args&&>(args)...);
	}

	constexpr auto placeholder_1 = std::placeholders::_1;
	constexpr auto placeholder_2 = std::placeholders::_2;
	constexpr auto placeholder_3 = std::placeholders::_3;
	constexpr auto placeholder_4 = std::placeholders::_4;
	constexpr auto placeholder_5 = std::placeholders::_5;
	// TODO: implement rest.

	using sstream = std::stringstream;

	using ostream = std::ostream;

	//using fstream = std::fstream;
	using ofstream = std::ofstream;
	using ifstream = std::ifstream;

	template<typename T>
	using initializer_list = std::initializer_list<T>;

	template<class T>
	struct is_c_str
		: std::integral_constant<
		bool,
		std::is_same<char const*, typename std::decay<T>::type>::value ||
		std::is_same<char*, typename std::decay<T>::type>::value
		> {
	};

	template<typename T>
	inline string to_string(const T& t)
	{
		if constexpr (std::is_same_v<T, string>)
		{
			return t;
		}
		else if constexpr (is_c_str<T>::value)
		{
			return string(t);
		}
		else
		{
			return std::to_string(t);
		}
	}

	using type_info = std::type_info;
	using type_index = std::type_index;

	namespace execution
	{
		using parallel_policy = std::execution::parallel_policy;
		using sequenced_policy = std::execution::sequenced_policy;
		constexpr parallel_policy const& par = std::execution::par;
		constexpr sequenced_policy const& seq = std::execution::seq;
	}

	template <class _ExPo, class _FwdIt, class _Fn>
	inline void for_each(_ExPo&& expo, _FwdIt first, _FwdIt last, _Fn fn)
	{
#if defined(NIGHT_DISABLE_MULTI_THREADING) || defined(NIGHT_DB_RENDERER_ENABLE_ALGO_RENDER)
		std::for_each(execution::seq, first, last, fn);
#else
		std::for_each(expo, first, last, fn);
#endif
	}

	// TODO: use NIGHT_DISABLE_MULTI_THREADING
	template<typename T>
	using future = std::future<T>;

	template<typename R, typename FN, typename... Args>
	inline std::future<R> async(FN fn, Args&&... args)
	{
		return std::async(fn, args...);
	}

	using mutex = std::mutex;
	
	template<typename T>
	void lock_guard(T&& m)
	{
		std::lock_guard(m);
	}

	template<typename T>
	using numeric_limits = std::numeric_limits<T>;

	//template<typename T>
	//struct numeric_limits
	//{
	//
	//};
	//
	//template<>
	//struct numeric_limits<s8>
	//{
	//	inline static s8 min = INT8_MIN;
	//	inline static s8 max = INT8_MAX;
	//};
	//
	//
	//template<>
	//struct numeric_limits<r32>
	//{
	//	constexpr static r32 min = -INFINITY;
	//	constexpr static r32 max = INFINITY;
	//};
	//
	//template<>
	//struct numeric_limits<r64>
	//{
	//	constexpr static r64 min = -INFINITY;
	//	constexpr static r64 max = INFINITY;
	//};
	//
	//template<>
	//struct numeric_limits<s32>
	//{
	//	constexpr static s32 min = INT32_MIN;
	//	constexpr static s32 max = INT32_MAX;
	//};
	//
	//template<>
	//struct numeric_limits<u32>
	//{
	//	constexpr static u32 min = 0;
	//	constexpr static u32 max = UINT32_MAX;
	//};
	//
	//template<>
	//struct numeric_limits<s64>
	//{
	//	constexpr static s64 min = INT64_MIN;
	//	constexpr static s64 max = INT64_MAX;
	//};
	//
	//template<>
	//struct numeric_limits<u64>
	//{
	//	constexpr static u64 min = 0;
	//	constexpr static u64 max = UINT64_MAX;
	//};

	namespace filesystem
	{
		//if (std::filesystem::exists(path) && std::filesystem::is_directory(path))
		//{
		//	for (const auto& entry : std::filesystem::directory_iterator(path))

		using directory_iterator = std::filesystem::directory_iterator;
		using directory_entry = std::filesystem::directory_entry;
		using path = std::filesystem::path;

		inline b8 exists(string const& path)
		{
			return std::filesystem::exists(path);
		}

		inline b8 is_directory(string const& path)
		{
			return std::filesystem::is_directory(path);
		}
	}
}
