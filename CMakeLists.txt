cmake_minimum_required( VERSION 3.20 )

# Unused
#list( APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake" )

project( cpp_base LANGUAGES CXX )

set( CMAKE_C_STANDARD               17  )
set( CMAKE_CXX_STANDARD             20  )
set( CMAKE_CXX_STANDARD_REQUIRED    ON  )
set( CMAKE_CXX_EXTENSIONS           OFF )

find_package( Git REQUIRED )

if( NOT GIT_FOUND )
    message( FATAL_ERROR "git not found" )
endif()

set( SYNC_LOCKFILE "build/submodules.lock" )
file( LOCK ${SYNC_LOCKFILE} )

    if( GIT_FOUND AND EXISTS "${PROJECT_SOURCE_DIR}/.gitmodules" )
        option( GIT_SUBMODULE "Check submodules during build" ON )
        if( GIT_SUBMODULE )
            message( STATUS "Update submodules" )
            execute_process(
                    COMMAND             ${GIT_EXECUTABLE} submodule update --init --recursive
                    WORKING_DIRECTORY   ${CMAKE_SOURCE_DIR} # this was CMAKE_CURRENT_SOURCE_DIR
                    RESULT_VARIABLE     GIT_SUBMOD_RESULT )

            if( NOT GIT_SUBMOD_RESULT EQUAL "0" )
                message( FATAL_ERROR "'git submodule update --init --recursive' failed with ${GIT_SUBMOD_RESULT}, please checkout submodules" )
            endif()
        endif()
    endif()

file( LOCK ${SYNC_LOCKFILE} RELEASE )

if( "${CMAKE_BUILD_TYPE}" STREQUAL "Debug" )
    set( IS_DEBUG TRUE )
else()
    set( IS_DEBUG FALSE )
endif()

option( USE_DeployBinary    "Deploy the binary in the base directory"   OFF )   # For distribution of final product
option( USE_FMT_HeaderOnly  "Use header-only implementation of FMT"     ON  )
option( USE_NFD_Native      "Use native encoding in NFD"                OFF )

option( USE_OpenMP          "Use OpenMP"        ON  )   # No-brain SIMD and MP
option( USE_GTK             "Use GTK"           OFF )   # Cross platform GUI Toolkit
option( USE_RTTI            "Use RTTI"          ON  )   # Don't need dynamic_cast or type_id? Then don't pay for them
option( USE_Exceptions      "Use Exceptions"    ON  )   # Don't need exceptions? Then don't pay for them

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
    find_package( PkgConfig REQUIRED )
    pkg_check_modules( GTKMM_VARS REQUIRED IMPORTED_TARGET gtkmm-4.0 )
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


if( WIN32 ) # Not only MSVC, also Clang on Windows
    add_compile_definitions(
            NOMINMAX
            WIN32_LEAN_AND_MEAN
            _CRT_SECURE_CPP_OVERLOAD_STANDARD_NAMES=1
    )
endif()

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
    set( CMAKE_CXX_FLAGS         "${CMAKE_CXX_FLAGS}         -Wno-unused-parameter -Wno-c++98-compat" )

    set( CMAKE_CXX_FLAGS         "${CMAKE_CXX_FLAGS}         -Werror=deprecated-copy -Werror=deprecated-copy-dtor" )
    set( CMAKE_CXX_FLAGS         "${CMAKE_CXX_FLAGS}         -Woverloaded-virtual" )

    set( CMAKE_C_FLAGS       "${CMAKE_C_FLAGS}           -Wno-unknown-pragmas" )
    set( CMAKE_CXX_FLAGS     "${CMAKE_CXX_FLAGS}         -Wno-unknown-pragmas" )

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
        src/entity0.cpp
        src/entity0.h
        src/entity1.h
        src/entity2.h
        src/entity3.h
        src/entity4.h
)

set_property(TARGET hello PROPERTY INTERPROCEDURAL_OPTIMIZATION ${USE_IPO})
set_property(TARGET bench PROPERTY INTERPROCEDURAL_OPTIMIZATION ${USE_IPO})

target_include_directories( hello
        PRIVATE
        include

        #${GTKMM_INCLUDE_DIRS}
        ${GTKMM_VARS_INCLUDE_DIRS}
        #include/gtkmm-4.0/gtkmm
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
