# -*- mode: CMAKE; cmake-tab-width: 4 -*-
#
cmake_minimum_required(VERSION 3.2)
project(glad C CXX)

# version
set(glad_VERSION_MAJOR 4)
set(glad_VERSION_MINOR 2)
set(glad_VERSION_PATCH 3)
set(glad_VERSION 0x040202)

set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake ${CMAKE_ROOT})

set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake ${CMAKE_ROOT})

# more debug messages
set(CMAKE_VERBOSE_MAKEFILE ON)

# default build type is release
if(NOT CMAKE_BUILD_TYPE)
    #set(CMAKE_BUILD_TYPE Release CACHE STRING
    #    "Choose the type of build, options are: "
    #    "None Debug Release RelWithDebInfo MinSizeRel."
    #    FORCE)
    set(CMAKE_BUILD_TYPE Release)
    set(CMAKE_VERBOSE_MAKEFILE OFF)
endif(NOT CMAKE_BUILD_TYPE)

option(BUILD_DOCUMENTS "Use doxygen to generate documentation" OFF)
#option(BUILD_TESTS "Build unit test" OFF)
#option(BUILD_EXAMPLES "Build example" ON)
option(BUILD_SHARED_LIBS "Build shared library" OFF)
option(BUILD_GIT_REV "Build the git revision code" OFF)

#
# get date of today 
#
macro(TODAY RESULT)
    if (WIN32)
        execute_process(COMMAND "cmd" "/c" "date" "/T"
            OUTPUT_VARIABLE ${RESULT})
        #MESSAGE(STATUS "result: '${RESULT}'")
        string(REGEX REPLACE "[a-zA-Z ]+(..)/(..)/(....).*" "\\3_\\1_\\2"
            ${RESULT} ${${RESULT}})
    elseif(UNIX)
        execute_process(COMMAND "date" "+%H/%d/%m/%Y"
            OUTPUT_VARIABLE ${RESULT})
        # strip the '\n'
        string(REGEX REPLACE "(..)/(..)/(..)/(....).*" "\\4\\3\\2_\\1"
            ${RESULT} ${${RESULT}})
    else (WIN32)
        message(SEND_ERROR "date not implemented")
        set(${RESULT} 000000)
    endif (WIN32)
endmacro (TODAY)


# GIT revision string, e.g. "v2.0.0-2-20110812_16-g812a0b8"
macro(REVISION RESULT)
    find_program(GIT git)
    if(GIT)
        set(GIT_REPOSITORY 1)
        # get the "git describe" and status of unstaged files
        if(UNIX)
            execute_process(COMMAND "git" "describe" "--long"
                WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
                OUTPUT_VARIABLE REV_SHA1_ 
                ERROR_VARIABLE ERR_OUTPUT
                RESULT_VARIABLE FAILED1
                OUTPUT_STRIP_TRAILING_WHITESPACE)

            execute_process(COMMAND "git" "diff" "--shortstat"
                WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
                OUTPUT_VARIABLE REV_STAT_ 
                ERROR_VARIABLE ERR_OUTPUT
                RESULT_VARIABLE FAILED2)
            #message(STATUS "result_v: ${FAILED1},${FAILED2} repo: ${GIT_REPOSITORY}")
        elseif(WIN32)
            execute_process(COMMAND "cmd" "/c" "git" "describe" "--long"
                WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
                OUTPUT_VARIABLE REV_SHA1_ RESULT_VARIABLE FAILED1)
            execute_process(COMMAND "cmd" "/c" "git" "diff" "--shortstat"
                WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
                OUTPUT_VARIABLE REV_STAT_ RESULT_VARIABLE FAILED2)
        endif(UNIX)

        if(FAILED1 OR FAILED2)
            set(GIT_REPOSITORY 0)
        endif(FAILED1 OR FAILED2)

        if (REV_STAT_)
            message(STATUS "Git unstaged files: ${CMAKE_CURRENT_SOURCE_DIR}")
        endif(REV_STAT_)

        if(GIT_REPOSITORY)
            string(STRIP ${REV_SHA1_} REV_SHA1)
            today(DATE)
            if(REV_STAT_)
                string(STRIP ${REV_STAT_} REV_STAT)
                SET(${RESULT} "${REV_SHA1}-${DATE}-dirty")
            else(REV_STAT_)
                set(${RESULT} "${REV_SHA1}-${DATE}")
            endif(REV_STAT_)
        else(GIT_REPOSITORY)
            message(STATUS "${CMAKE_CURRENT_SOURCE_DIR} is not a git repository")
            set(${RESULT} "")
        endif(GIT_REPOSITORY)

        # message(STATUS "revision: '${REV_SHA1}' ${DATE}")
        # like: "v2.0.0-2-20110812-g812a0b8"
        #string(REGEX REPLACE "([v0-9.]+)(-?[0-9]*)(-g[a-z0-9]*)$" 
        #       "\\1\\2-${DATE}\\3" ${RESULT} ${REV_SHA1})
        #string(STRIP ${REV_STAT_} REV_SHORT_STAT_)
        #string(REGEX MATCHALL ".*\n" ";" REV_STAT_LIST_ ${REV_STAT_})
        #message(STATUS "s: ${REV_STAT_LIST_}")
        #list(GET ${REV_STAT_LIST_} -1 REV_UNSTAGED_)
        #list(LENGTH ${REV_STAT_LIST_} L)
        #message(STATUS "length(${REV_STAT_LIST_})=${L}")
        #message(STATUS "stat: ${REV_UNSTATED_}")
    else(GIT)
        message(STATUS "git not found, rev string will be empty.")
        SET(${RESULT} "")
    endif(GIT)
endmacro(REVISION)

set(glad_VERSION_STR 
    "${glad_VERSION_MAJOR}.${glad_VERSION_MINOR}.${glad_VERSION_PATCH}")

# set proper glad_REVISION name, with git SHA1 or not
if(BUILD_GIT_REV)
    REVISION(glad_REVISION)
else()
    set(glad_REVISION "${glad_VERSION_STR}-${CMAKE_BUILD_TYPE}")
endif()
message(STATUS "rev: ${glad_REVISION}")

# default build is for "Release"
if(CMAKE_COMPILER_IS_GNUCXX AND CMAKE_SIZEOF_VOID_P EQUAL 8)
  # GCC compilers on 64-bit machines require -fPIC for shared libraries or libs
  # linked into shared libraries.
  set(CMAKE_C_FLAGS_RELEASE   "-O2 -fPIC -ansi -DNDEBUG")
  set(CMAKE_CXX_FLAGS_RELEASE "-std=c++0x -O2 -fPIC -DNDEBUG")
  # debug, HDF5 does not pass -pedantic option
  set(CMAKE_C_FLAGS_DEBUG "-Wall -Wno-variadic-macros -ggdb3 -fPIC -ansi")
  set(CMAKE_CXX_FLAGS_DEBUG "-std=c++0x -Wall -Wno-variadic-macros -ggdb3 -fPIC")
elseif(CMAKE_COMPILER_IS_GNUCXX)
  set(CMAKE_C_FLAGS_RELEASE   "-O2 -fPIC -ansi -DNDEBUG")
  set(CMAKE_CXX_FLAGS_RELEASE "-std=c++0x -O2 -fPIC -DNDEBUG")
  # debug
  set(CMAKE_C_FLAGS_DEBUG "-Wall -Wno-variadic-macros -ggdb3 -fPIC -ansi")
  set(CMAKE_CXX_FLAGS_DEBUG "-std=c++0x -Wall -Wno-variadic-macros -ggdb3 -fPIC")
elseif(MSVC) # MS-Windows Visual Studio, both 32 and 64 bits, disable warnings
  set(CMAKE_C_FLAGS   "${CMAKE_C_FLAGS}   /MD /fp:fast /w /wd4820 /wd4365")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /MD /fp:fast /w /wd4820 /wd4365")
  set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /FIXED:NO") #/NOLOGO
endif()


if(${CMAKE_SYSTEM_NAME} STREQUAL "Linux")
  message(STATUS "Linux")
  find_package(HDF5 NAMES hdf5 REQUIRED COMPONENTS static C HL)
elseif(${CMAKE_SYSTEM_NAME} STREQUAL "Windows")
  message(STATUS "Using Windows")
  set(CMAKE_VERBOSE_MAKEFILE ON)
  set(GSL_ROOT_DIR "C:/devel/libs/gsl_2.1")
  #set(BOOST_ROOT "E:/Home/apps/boost_1_57_0")
  set(BOOST_ROOT "C:/devel/libs/boost_1.59.0")
  set(Boost_USE_STATIC_LIBS ON)
  set(Boost_USE_MULTITHREADED ON)
  #set(BOOST_LIBRARYDIR "${BOOST_ROOT}/lib64-msvc-10.0")
  #set(HDF5_ROOT "C:/devel/libs/HDF5-1.8.16-win64")
  # see hdf5-config.cmake in cmake-2.8.14 windows installer
  #set(HDF5_DIR "C:/devel/libs/HDF5-1.8.16-win64/cmake")
  #set(HDF5_ROOT_DIR_HINT "C:/devel/libs/HDF5-1.8.16-win64/cmake")
  #set(HDF5_DIR "C:\devel\libs\HDF5-1.8.16-win64\cmake")
  set(HDF5_DIR "C:/Program Files/HDF_Group/HDF5/1.8.16/cmake")
  set(HDF5_USE_STATIC_LIBRARIES ON)
  find_package(HDF5 NAMES hdf5 REQUIRED COMPONENTS static C HL)
  #find_package(HDF5 NAMES hdf5)
  message(STATUS "HDF5_LIBRARIES: ${HDF5_LIBRARIES}")
  message(STATUS "HDF5_C_LIBRARIES: ${HDF5_C_LIBRARIES}")
  message(STATUS "HDF5_HL_LIBRARIES: ${HDF5_HL_LIBRARIES}")
elseif(${CMAKE_SYSTEM_NAME} STREQUAL "Darwin")
    set(HDF5_USE_STATIC_LIBRARIES ON)
    set(HDF5_ROOT "/opt/local")
    ## this form will ask for hdf5Config.cmake or hdf5-config.cmake
    # find_package(HDF5 NAMES hdf5 REQUIRED COMPONENTS static C HL)
    find_package(HDF5 REQUIRED COMPONENTS C HL)
else()
    message(STATUS "Unknown system: ${CMAKE_SYSTEM_NAME}")
endif()


add_subdirectory(src)

