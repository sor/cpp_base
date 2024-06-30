#pragma once

#define _GLIBC_USE_DEPRECATED 0
#define _GLIBCXX_USE_DEPRECATED 0
#define _LIBCPP_ENABLE_DEPRECATION_WARNINGS 2
#define WIN32_LEAN_AND_MEAN

//#define DEPLOY // Activate this line to be able to put the executable in the root folder

#include <cmath>
#include <cstdarg>
#include <cstddef>
#include <cassert>

#include <algorithm>
#include <chrono>
#include <memory>
#include <string>
#include <numbers>
#include <fstream>

#include <array>
#include <vector>
#include <unordered_set>

#if defined( __cpp_lib_format ) && defined( __cpp_lib_print )
#include <format>
#include <print>
#else
#include <fmt/core.h>   // https://fmt.dev/latest/index.html
#endif

namespace JanSordid::Core
{
	using i8    = std::int8_t;
	using i16   = std::int16_t;
	using i32   = std::int32_t;
	using i64   = std::int64_t;
	using u8    = std::uint8_t;
	using u16   = std::uint16_t;
	using u32   = std::uint32_t;
	using u64   = std::uint64_t;

    using isize = std::ptrdiff_t;
    using ssize = std::ptrdiff_t;
	using usize = std::size_t;

	using uint  = unsigned int;
	using std::byte;

	using f32   = float;
	using f64   = double;

    using string        = std::string;
	using string_view   = std::string_view;
	using String        = std::string;
	using StringView    = std::string_view;

	using Clock         = std::chrono::high_resolution_clock;
	using TimePoint     = Clock::time_point;
	using Duration      = Clock::duration;

	using std::min;
	using std::max;
	using std::forward;
	using std::move;
	using std::make_unique;
	using std::make_shared;

	namespace numbers { using namespace std::numbers; }

	template<class T, std::size_t Size>                 using Array         = std::array<T, Size>;
	template<class T>                                   using Vector        = std::vector<T>;

	template<class T, class D = std::default_delete<T>> using unique_ptr    = std::unique_ptr<T,D>;
	template<class T>                                   using shared_ptr    = std::shared_ptr<T>;
	template<class T>                                   using weak_ptr      = std::weak_ptr<T>;


	#if defined( __cpp_lib_format ) && defined( __cpp_lib_print )
	using std::print, std::println, std::format, std::format_string;
	#else
	using fmt::print, fmt::println, fmt::format, fmt::format_string;
	#endif

	#ifdef DEPLOY
	#define BasePath ""             // To put the binaries in the root folder, parallel to e.g. vcpkg.json
	#else
	#define BasePath "../../../../"
	#endif

	#if defined( _DEBUG )
		#define Assert(            ... ) assert( __VA_ARGS__ )
		#define AssertInOptimized( ... ) assert( __VA_ARGS__ )
		#define DebugOnly(         ... ) __VA_ARGS__
		#define OptimizedOnly(     ... )
		#define FinalOnly(         ... ) __VA_ARGS__
		#define IfDebug     if constexpr( true  )
		#define IfOptimized if constexpr( false )
		#define IfFinal     if constexpr( false )
		#define IfNotFinal  if constexpr( true )
	#elif defined( OPTIMIZED )
		#define Assert(            ... )
		#define AssertInOptimized( ... ) assert( __VA_ARGS__ )
		#define DebugOnly(         ... )
		#define OptimizedOnly(     ... ) __VA_ARGS__
		#define FinalOnly(         ... )
		#define IfDebug     if constexpr( false )
		#define IfOptimized if constexpr( true  )
		#define IfFinal     if constexpr( false )
		#define IfNotFinal  if constexpr( true )
	#elif defined( FINAL )
		// Final also counts as an Optimized build, therefore the Optimized defines are active
		#define Assert(            ... )
		#define AssertInOptimized( ... )
		#define DebugOnly(         ... )
		#define OptimizedOnly(     ... ) __VA_ARGS__
		#define FinalOnly(         ... ) __VA_ARGS__
		#define IfDebug     if constexpr( false )
		#define IfOptimized if constexpr( true  )
		#define IfFinal     if constexpr( true  )
		#define IfNotFinal  if constexpr( false  )
	#else
		#error Unknown build configuration
	#endif

	template <typename... T>
	inline void print_once( format_string<T...> fmt, T && ... args )
	{
		static std::unordered_set<string> all;
		const string msg = format( fmt, forward<T>( args )... );
		if( all.find( msg ) == all.end() )
		{
			all.insert( msg );
			print( "{}", msg );
		}
	}
}

namespace JanSordid
{
	using namespace JanSordid::Core;
}

#include <nfdpp.h>      // Wrapped for C++, original from: https://github.com/mlabbe/nativefiledialog/blob/master/README.md
