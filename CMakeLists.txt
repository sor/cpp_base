cmake_minimum_required( VERSION 3.20 )

if( "${CMAKE_BUILD_TYPE}" STREQUAL "Debug" )
	set( IS_DEBUG     TRUE  )
	set( IS_OPTIMIZED FALSE )
	set( IS_FINAL     FALSE )
elseif( "${CMAKE_BUILD_TYPE}" STREQUAL "RelWithDebInfo" ) # I call this Optimized
	set( IS_DEBUG     FALSE )
	set( IS_OPTIMIZED TRUE  )
	set( IS_FINAL     FALSE )
elseif( "${CMAKE_BUILD_TYPE}" STREQUAL "Release" ) # I call this Final
	set( IS_DEBUG     FALSE )
	set( IS_OPTIMIZED FALSE )
	set( IS_FINAL     TRUE  )
else()
	message( SEND_ERROR "Unknown Build Type" )
endif()

# TODO: Port things over from SDL_base

option( USE_DeployBinary    "Deploy the binary in the base directory"    OFF )   # For distribution of final product
option( USE_FMT_HeaderOnly  "Use header-only implementation of FMT"      ON  )
option( USE_NFD_Native      "Use native encoding instead of UTF8 in NFD" OFF )
option( USE_Unity_Builds    "Compile multiple source files at once"      OFF )
option( USE_StaticLink      "Statically-Link the binary (only Linux)"    ${IS_FINAL} )

option( USE_OpenMP          "Use OpenMP"       ON  )   # No-brain SIMD and MP
option( USE_RTTI            "Use RTTI"         ON  )   # Don't need dynamic_cast or type_id? Then don't pay for them
option( USE_Exceptions      "Use Exceptions"   ON  )   # Don't need exceptions? Then don't pay for them
option( USE_TargetNative    "Use march=native" OFF )   # Don't need to run this somewhere else? Build for THIS machine only

option( USE_GTK             "Use GTK"          ON  )   # Cross platform GUI Toolkit

if( UNIX AND NOT APPLE AND NOT DEFINED VCPKG_TARGET_TRIPLET AND NOT USE_StaticLink )
	set( VCPKG_TARGET_TRIPLET "x64-linux-dynamic" CACHE STRING "" )
endif()


project( cpp_base LANGUAGES CXX )

set( CMAKE_C_STANDARD               17  )
set( CMAKE_CXX_STANDARD             20  )
set( CMAKE_CXX_STANDARD_REQUIRED    ON  )
set( CMAKE_CXX_EXTENSIONS           OFF )

set( CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib )
set( CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib )
set( CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin )


find_package( fmt           CONFIG REQUIRED )
find_package( nfd           CONFIG REQUIRED )
find_package( benchmark     CONFIG REQUIRED )
find_package( nlohmann_json CONFIG REQUIRED )
if( USE_OpenMP )
	find_package( OpenMP           REQUIRED )
endif()
if( USE_GTK )
	find_package( PkgConfig        REQUIRED )
	pkg_check_modules( GTKMM_VARS  REQUIRED IMPORTED_TARGET gtkmm-4.0 )
endif()


# Why does MSVC exist, but not CLANG or GCC?
if( CMAKE_CXX_COMPILER_ID MATCHES "Clang" )
	set( CLANG 1 )
elseif( CMAKE_CXX_COMPILER_ID MATCHES "GNU" )
	set( GCC 1 )
endif()


add_compile_definitions(
	$<$<CONFIG:DEBUG>:_DEBUG>               # not DEBUG because of a stupid old define
	$<$<CONFIG:RELWITHDEBINFO>:OPTIMIZED>
	$<$<CONFIG:RELEASE>:FINAL> )

if( GTKMM_VARS_FOUND )
	add_compile_definitions( USE_GTK )
endif()

if( WIN32 ) # Not only MSVC, also Clang on Windows
	add_compile_definitions(
		NOMINMAX
		WIN32_LEAN_AND_MEAN
		_CRT_SECURE_CPP_OVERLOAD_STANDARD_NAMES=1
	)
endif ()

if( GCC OR CLANG )
	add_compile_definitions(
		_GLIBCXX_USE_DEPRECATED=0
		_LIBCPP_ENABLE_DEPRECATION_WARNINGS=1
	)
endif()


message( STATUS "-- Using Compiler:")
execute_process(
	COMMAND             ${CMAKE_CXX_COMPILER} --version
	WORKING_DIRECTORY   ${CMAKE_CURRENT_SOURCE_DIR} )


if( MSVC )
	add_compile_options( /W4 /wd4100 /we4456 /utf-8 ) #  no char8_t: /Zc:char8_t-  - whole program optimization: /GL
	add_compile_options( /constexpr:steps 104857600 )

	if( NOT USE_RTTI )
		add_compile_options( /GR- )
	endif()

	if( NOT USE_Exceptions )
		add_compile_options( "-D_HAS_EXCEPTIONS=0" )
		#add_compile_options( "-D_STATIC_CPPLIB" )
	endif()

else()
	# Needs to be before all the -Wno-*
	set( CMAKE_C_FLAGS           "${CMAKE_C_FLAGS}           -Wall -Wextra -Werror=parentheses -Werror=uninitialized -Werror=return-type -Werror=unused-result" )
	set( CMAKE_CXX_FLAGS         "${CMAKE_CXX_FLAGS}         -Wall -Wextra -Werror=parentheses -Werror=uninitialized -Werror=return-type -Werror=unused-result" )

	set( CMAKE_C_FLAGS           "${CMAKE_C_FLAGS}           -Wno-unused-parameter" )   # baseclass functions have plenty unused parameters
	set( CMAKE_CXX_FLAGS         "${CMAKE_CXX_FLAGS}         -Wno-unused-parameter -Wno-c++98-compat" ) # Clang only?

	set( CMAKE_CXX_FLAGS         "${CMAKE_CXX_FLAGS}         -Werror=deprecated-copy -Werror=deprecated-copy-dtor" )
	set( CMAKE_CXX_FLAGS         "${CMAKE_CXX_FLAGS}         -Woverloaded-virtual" )

	set( CMAKE_C_FLAGS           "${CMAKE_C_FLAGS}           -Wno-unknown-pragmas" )
	set( CMAKE_CXX_FLAGS         "${CMAKE_CXX_FLAGS}         -Wno-unknown-pragmas" )

	if( GCC )
		set( CMAKE_C_FLAGS       "${CMAKE_C_FLAGS}           -Wshadow -Werror=shadow=local" )
		set( CMAKE_CXX_FLAGS     "${CMAKE_CXX_FLAGS}         -Wshadow -Werror=shadow=local" )

		set( CMAKE_CXX_FLAGS     "${CMAKE_CXX_FLAGS}         -Wdelete-non-virtual-dtor" )
		#set( CMAKE_CXX_FLAGS     "${CMAKE_CXX_FLAGS}         -Wdeprecated-dynamic-exception-spec -Wdeprecated-implementations -Wdeprecated-literal-operator -Wdeprecated-redundant-constexpr-static-def" )

		set( CMAKE_CXX_FLAGS     "${CMAKE_CXX_FLAGS}         -Wno-volatile" ) # senseless C++20 deprecation that has been de-deprecated for C++23, works in GCC 11 but not Clang 15

		# TODO: move properly
		set( CMAKE_CXX_FLAGS     "${CMAKE_CXX_FLAGS}         -fconstexpr-ops-limit=127124200 -fconstexpr-depth=127" )

	elseif( CLANG )
		# TODO: Clang uses different names
		set( CMAKE_C_FLAGS       "${CMAKE_C_FLAGS}           -Wshadow-all -Werror=shadow" )
		set( CMAKE_CXX_FLAGS     "${CMAKE_CXX_FLAGS}         -Wshadow-all -Werror=shadow" )

		set( CMAKE_CXX_FLAGS     "${CMAKE_CXX_FLAGS}         -Wdelete-non-virtual-dtor -Wdeprecated-dynamic-exception-spec -Wdeprecated-implementations" )
		#set( CMAKE_CXX_FLAGS     "${CMAKE_CXX_FLAGS}         -Wdeprecated-literal-operator -Wdeprecated-redundant-constexpr-static-def" )

		#set( CMAKE_C_FLAGS       "${CMAKE_C_FLAGS}           -Wno-unknown-attributes" ) # clang 15 does not know [[(un)likely]]
		#set( CMAKE_CXX_FLAGS     "${CMAKE_CXX_FLAGS}         -Wno-unknown-attributes" ) # clang 15 does not know [[(un)likely]]

		set( CMAKE_CXX_FLAGS     "${CMAKE_CXX_FLAGS}         -Werror=inconsistent-missing-override" )
		set( CMAKE_CXX_FLAGS     "${CMAKE_CXX_FLAGS}         -Wweak-vtables" )

		# TODO: move properly
		set( CMAKE_CXX_FLAGS     "${CMAKE_CXX_FLAGS}         -fconstexpr-steps=127124200 -fconstexpr-depth=127" )

	endif()


	if( NOT USE_RTTI )
		set( CMAKE_CXX_FLAGS     "${CMAKE_CXX_FLAGS}         -fno-rtti" )
	endif()

	if( NOT USE_Exceptions )
		set( CMAKE_CXX_FLAGS     "${CMAKE_CXX_FLAGS}         -fno-exceptions" )
	endif()

	# (How) Does this work for MSVC?
	if( OPENMP_FOUND )
		set( CMAKE_C_FLAGS       "${CMAKE_C_FLAGS}           ${OpenMP_C_FLAGS}"   )
		set( CMAKE_CXX_FLAGS     "${CMAKE_CXX_FLAGS}         ${OpenMP_CXX_FLAGS}" )
	else()
		# Silence unknown OpenMP pragmas
		set( CMAKE_C_FLAGS       "${CMAKE_C_FLAGS}           -Wno-unknown-pragmas" )
		set( CMAKE_CXX_FLAGS     "${CMAKE_CXX_FLAGS}         -Wno-unknown-pragmas" )
	endif()

	# Only build for this machine
	set( CMAKE_C_FLAGS           "${CMAKE_C_FLAGS}           -march=native" )
	set( CMAKE_CXX_FLAGS         "${CMAKE_CXX_FLAGS}         -march=native" )
	#set( CMAKE_C_FLAGS           "${CMAKE_C_FLAGS}           -march=znver3" )
	#set( CMAKE_CXX_FLAGS         "${CMAKE_CXX_FLAGS}         -march=znver3" )
	#set( CMAKE_C_FLAGS           "${CMAKE_C_FLAGS}           -march=nocona" )
	#set( CMAKE_CXX_FLAGS         "${CMAKE_CXX_FLAGS}         -march=nocona" )
	#set( CMAKE_C_FLAGS           "${CMAKE_C_FLAGS}           -march=x86-64-v3" )
	#set( CMAKE_CXX_FLAGS         "${CMAKE_CXX_FLAGS}         -march=x86-64-v3" )

	if( CLANG )
		# Get messages from the vectorizer
		#set( CMAKE_C_FLAGS       "${CMAKE_C_FLAGS}           -Rpass=loop-vectorize" )
		#set( CMAKE_CXX_FLAGS     "${CMAKE_CXX_FLAGS}         -Rpass=loop-vectorize" )
	endif()

	# Default to do some minor optimizations of debug builds
	set( CMAKE_C_FLAGS_DEBUG     "${CMAKE_C_FLAGS_DEBUG}     -Og" )
	set( CMAKE_CXX_FLAGS_DEBUG   "${CMAKE_CXX_FLAGS_DEBUG}   -Og" )

	# Enable asserts by filtering out NDEBUG on Optimized builds (which is called RELWITHDEBINFO in CMAKE)
	string(REGEX REPLACE " -DNDEBUG" "" CMAKE_C_FLAGS_RELWITHDEBINFO   "${CMAKE_C_FLAGS_RELWITHDEBINFO}" )
	string(REGEX REPLACE " -DNDEBUG" "" CMAKE_CXX_FLAGS_RELWITHDEBINFO "${CMAKE_CXX_FLAGS_RELWITHDEBINFO}" )

	# CARE: Here be Dragons
	#set( CMAKE_C_FLAGS_RELEASE   "${CMAKE_C_FLAGS_RELEASE}   -ffast-math" )
	#set( CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -ffast-math" )

	# Cleanup unnecessary whitespace
	string(REGEX REPLACE "[ \t\r\n]+" " " CMAKE_C_FLAGS   "${CMAKE_C_FLAGS}" )
	string(REGEX REPLACE "[ \t\r\n]+" " " CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}" )
endif()

if( FALSE )
	message("CMAKE_CXX_FLAGS                (General)   is ${CMAKE_CXX_FLAGS}")
	message("CMAKE_CXX_FLAGS_DEBUG          (Debug)     is ${CMAKE_CXX_FLAGS_DEBUG}")
	message("CMAKE_CXX_FLAGS_RELWITHDEBINFO (Optimized) is ${CMAKE_CXX_FLAGS_RELWITHDEBINFO}")
	message("CMAKE_CXX_FLAGS_RELEASE        (Final)     is ${CMAKE_CXX_FLAGS_RELEASE}")
endif ()

#target_link_options(arena PRIVATE
#    $<$<LINKER:LLD>:--icf=all>
#    $<$<LINKER:Gold>:--icf=all>
#    $<$<LINKER:MSVC>:/OPT:ICF>
#)

if( CMAKE_BUILD_TYPE MATCHES Debug
		OR CMAKE_BUILD_TYPE MATCHES RelWithDebInfo )
	message( STATUS "IPO / LTO disabled (only active in Final builds)" )
elseif( CMAKE_BUILD_TYPE MATCHES Release )
	include( CheckIPOSupported )
	check_ipo_supported( RESULT USE_IPO OUTPUT IPO_ERROR )
	if( USE_IPO )
		message( STATUS "IPO / LTO enabled" )
	else()
		message( STATUS "IPO / LTO not supported: <${IPO_ERROR}>" )
	endif()
endif()


add_executable( hello
	src/sor/core.cpp
	src/sor/core.hpp

	src/global.hpp

	src/hello.cpp
)

add_executable( bench
	src/sor/core.cpp
	src/sor/core.hpp

	src/global.hpp

	src/bench.cpp
	src/bench.hpp
	src/bench_oop.cpp
	src/bench_proc.cpp
	src/bench_procopt.cpp
	src/bench_dod.cpp

	src/entity0.cpp
	src/entity0.hpp
	src/entity1.hpp
	src/entity2.hpp
	src/entity3.hpp
	src/entity4.hpp
)

set_property(TARGET hello PROPERTY UNITY_BUILD ${USE_Unity_Builds})
set_property(TARGET hello PROPERTY INTERPROCEDURAL_OPTIMIZATION ${USE_IPO})

set_property(TARGET bench PROPERTY UNITY_BUILD ${USE_Unity_Builds})
set_property(TARGET bench PROPERTY INTERPROCEDURAL_OPTIMIZATION ${USE_IPO})


if( FALSE )
	get_cmake_property(_variableNames VARIABLES)
	list (SORT _variableNames)
	foreach (_variableName ${_variableNames})
		message(STATUS "${_variableName}=${${_variableName}}")
	endforeach()
endif()


target_include_directories( hello
	PRIVATE
	include

	${GTKMM_VARS_INCLUDE_DIRS}
)

target_link_libraries( hello
	PRIVATE
	$<IF:$<BOOL:${USE_FMT_HeaderOnly}>, fmt::fmt-header-only, fmt::fmt>

	$<TARGET_NAME_IF_EXISTS:OpenMP::OpenMP_CXX>
	$<TARGET_NAME_IF_EXISTS:nfd::nfd>

	$<TARGET_NAME_IF_EXISTS:PkgConfig::GTKMM_VARS>
)

target_include_directories( bench
	PRIVATE
	include
)

target_link_libraries( bench
	PRIVATE
	$<IF:$<BOOL:${USE_FMT_HeaderOnly}>, fmt::fmt-header-only, fmt::fmt>
	$<TARGET_NAME_IF_EXISTS:nfd::nfd>

	$<TARGET_NAME_IF_EXISTS:OpenMP::OpenMP_CXX>
	$<TARGET_NAME_IF_EXISTS:Eigen3::Eigen>

	$<TARGET_NAME_IF_EXISTS:benchmark::benchmark>
	$<TARGET_NAME_IF_EXISTS:benchmark::benchmark_main>
)
