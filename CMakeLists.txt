cmake_minimum_required( VERSION 3.20 )
project( cpp_base )

set( CMAKE_C_STANDARD   17 )
set( CMAKE_CXX_STANDARD 20 )

set( VCPKG_VERBOSE      OFF )
set( FMT_HEADER_ONLY    1 )

set( CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin )
set( VCPKG_DIR  "${CMAKE_SOURCE_DIR}/vcpkg/" CACHE PATH "Location of VCPKG" )

find_package( Git REQUIRED )

if( NOT GIT_FOUND )
    message( FATAL_ERROR "git not found" )
endif()

set( VCPKG_LOCKFILE "build/install-vcpkg.lock" )
file( LOCK ${VCPKG_LOCKFILE} )

    if( NOT CMAKE_TOOLCHAIN_FILE )
        message( STATUS "No toolchain file given, installing VCPKG" )
        #option( UPDATE_VCPKG "Enable updating VCPKG" ON )
        if( NOT EXISTS "${VCPKG_DIR}" )
            message( STATUS "Cloning ${VCPKG_DIR}" )
            execute_process(
                    COMMAND             ${GIT_EXECUTABLE} clone https://github.com/Microsoft/vcpkg.git ${VCPKG_DIR}
                    WORKING_DIRECTORY   ${CMAKE_CURRENT_SOURCE_DIR}
                    RESULT_VARIABLE     GIT_RESULT )

            if( NOT GIT_RESULT EQUAL "0" )
                message( FATAL_ERROR "Execution of git failed with ${GIT_RESULT}" )
            endif()
        #elseif( UPDATE_VCPKG )
        #    message( STATUS "Pulling ${VCPKG_DIR}" )
        #    execute_process(
        #            COMMAND             ${GIT_EXECUTABLE} pull -q
        #            WORKING_DIRECTORY   ${VCPKG_DIR}
        #            RESULT_VARIABLE     GIT_RESULT )
        #
        #    if( NOT GIT_RESULT EQUAL "0" )
        #        message( FATAL_ERROR "Execution of git failed with ${GIT_RESULT}" )
        #    endif()
        else()
            message( STATUS "VCPKG is already installed" )
        endif()
        return() # nothing more to do for the VCPKG installer

    # WTF: Sometimes this already is the full path, sometimes relative
    elseif( EXISTS "${CMAKE_TOOLCHAIN_FILE}" OR
            EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/${CMAKE_TOOLCHAIN_FILE}")
        message( STATUS "Toolchain file exists: ${CMAKE_TOOLCHAIN_FILE}" )
    else()
        message( FATAL_ERROR "Toolchain file does not exist yet: ${CMAKE_TOOLCHAIN_FILE}\nRun the 'Install VCPKG' option" )
        file( LOCK ${VCPKG_LOCKFILE} RELEASE )
        return()
    endif()

file( LOCK ${VCPKG_LOCKFILE} RELEASE )


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


message("-- Using Compiler:")
execute_process(
        COMMAND             ${CMAKE_CXX_COMPILER} --version
        WORKING_DIRECTORY   ${CMAKE_CURRENT_SOURCE_DIR} )


# Why does MSVC exist, but not CLANG or GCC?
if( CMAKE_CXX_COMPILER_ID MATCHES "Clang" )
    set( CLANG 1 )
elseif( CMAKE_CXX_COMPILER_ID MATCHES "GNU" )
    set( GCC 1 )
endif()

option( USE_OpenMP "Use OpenMP" ON )
if( USE_OpenMP )
    find_package( OpenMP )
endif()

option( USE_RTTI       "Use RTTI"       ON )    # Don't need dynamic_cast or type_id? Then don't pay for them
option( USE_Exceptions "Use Exceptions" ON )    # Don't need exceptions? Then don't pay for them

if( MSVC )
    add_compile_options( /W4 /wd4100 /we4456 /utf-8 ) #  no char8_t: /Zc:char8_t-  - whole program optimization: /GL

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
    set( CMAKE_CXX_FLAGS         "${CMAKE_CXX_FLAGS}         -Wno-unused-parameter" )   # baseclass functions have plenty unused parameters

    set( CMAKE_CXX_FLAGS         "${CMAKE_CXX_FLAGS}         -Werror=deprecated-copy -Werror=deprecated-copy-dtor" )
    set( CMAKE_CXX_FLAGS         "${CMAKE_CXX_FLAGS}         -Woverloaded-virtual" )

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

        set( CMAKE_C_FLAGS       "${CMAKE_C_FLAGS}           -Wno-unknown-attributes" ) # clang 15 does not know [[(un)likely]]
        set( CMAKE_CXX_FLAGS     "${CMAKE_CXX_FLAGS}         -Wno-unknown-attributes" ) # clang 15 does not know [[(un)likely]]

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

    # Default to do some minor optimizations of debug builds
    set( CMAKE_C_FLAGS_DEBUG     "${CMAKE_C_FLAGS_DEBUG}     -Og" )
    set( CMAKE_CXX_FLAGS_DEBUG   "${CMAKE_CXX_FLAGS_DEBUG}   -Og" )

    # Enable asserts by filtering out NDEBUG on Optimized builds (which is called RELWITHDEBINFO in CMAKE)
    string(REGEX REPLACE " -DNDEBUG" "" CMAKE_C_FLAGS_RELWITHDEBINFO   "${CMAKE_C_FLAGS_RELWITHDEBINFO}" )
    string(REGEX REPLACE " -DNDEBUG" "" CMAKE_CXX_FLAGS_RELWITHDEBINFO "${CMAKE_CXX_FLAGS_RELWITHDEBINFO}" )

    # CARE: Here be Dragons
    #set( CMAKE_C_FLAGS_RELEASE   "${CMAKE_C_FLAGS_RELEASE}   -ffast-math" )
    #set( CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -ffast-math" )
endif()

if( FALSE )
    message("CMAKE_CXX_FLAGS_DEBUG          (Debug)     is ${CMAKE_CXX_FLAGS_DEBUG}")
    message("CMAKE_CXX_FLAGS_RELWITHDEBINFO (Optimized) is ${CMAKE_CXX_FLAGS_RELWITHDEBINFO}")
    message("CMAKE_CXX_FLAGS_RELEASE        (Final)     is ${CMAKE_CXX_FLAGS_RELEASE}")
endif ()

if( CMAKE_BUILD_TYPE MATCHES Debug
        OR CMAKE_BUILD_TYPE MATCHES RelWithDebInfo )
    message( STATUS "IPO / LTO disabled (only active in Final builds)" )
elseif( CMAKE_BUILD_TYPE MATCHES Release )
    include(CheckIPOSupported)
    check_ipo_supported(RESULT USE_IPO OUTPUT IPO_ERROR )
    if( USE_IPO )
        message( STATUS "IPO / LTO enabled" )
    else()
        message( STATUS "IPO / LTO not supported: <${IPO_ERROR}>" )
    endif()
endif()

add_compile_options( "$<$<CONFIG:DEBUG>:-D_DEBUG>" ) # not DEBUG because of a stupid old define
add_compile_options( "$<$<CONFIG:RELWITHDEBINFO>:-DOPTIMIZED>" )
add_compile_options( "$<$<CONFIG:RELEASE>:-DFINAL>" )


find_package( fmt                         CONFIG REQUIRED )
find_package( unofficial-nativefiledialog CONFIG REQUIRED )
find_package( benchmark                   CONFIG REQUIRED )
find_package( Eigen3                      CONFIG REQUIRED )


add_executable( hello
        include/global.h
        include/nfdpp.h

        include/nfdpp.cpp

        src/hello.cpp
)

add_executable( bench
        include/global.h

        src/bench.cpp
)

if( USE_IPO )
    set_property(TARGET hello PROPERTY INTERPROCEDURAL_OPTIMIZATION TRUE)
    set_property(TARGET bench PROPERTY INTERPROCEDURAL_OPTIMIZATION TRUE)
endif()


target_include_directories( hello
        PRIVATE
        include
)

target_include_directories( bench
        PRIVATE
        include
)


target_link_libraries( hello
        PRIVATE
        $<IF:${FMT_HEADER_ONLY},                        fmt::fmt-header-only,   fmt::fmt>
        $<TARGET_NAME_IF_EXISTS:unofficial::nativefiledialog::nfd>
        $<TARGET_NAME_IF_EXISTS:OpenMP::OpenMP_CXX>
)

target_link_libraries( bench
        PRIVATE
        $<IF:${FMT_HEADER_ONLY},                        fmt::fmt-header-only,   fmt::fmt>
        $<TARGET_NAME_IF_EXISTS:unofficial::nativefiledialog::nfd>
        $<TARGET_NAME_IF_EXISTS:OpenMP::OpenMP_CXX>
        $<TARGET_NAME_IF_EXISTS:Eigen3::Eigen>
        $<TARGET_NAME_IF_EXISTS:benchmark::benchmark>
        $<TARGET_NAME_IF_EXISTS:benchmark::benchmark_main>
)
