# -*- mode: CMAKE; cmake-tab-width: 4 -*-
#

FILE(WRITE ${CMAKE_BINARY_DIR}/glad_version.h.in
    "#define GLAD_VERSION_MAJOR \"@glad_VERSION_MAJOR@\"\n"
    "#define GLAD_VERSION_MINOR \"@glad_VERSION_MINOR@\"\n"
    "#define GLAD_VERSION_PATCH \"@glad_VERSION_PATCH@\"\n"
    "\n"
    "#define GLAD_VERSION_STR \"@glad_VERSION_STR@\"\n"
    "#define GLAD_VERSION @glad_VERSION@\n"
    "\n"
    "#define GLAD_REVISION \"@glad_REVISION@\"\n"
    "#define GLAD_GIT_REV \"@glad_REVISION@\"\n"
)

FILE(WRITE ${CMAKE_BINARY_DIR}/version.cmake
    "# REVISION(glad_REVISION)
     EXECUTE_PROCESS(
       #COMMAND ${GIT_EXECUTABLE} --version
       COMMAND date \"+%M\"
       OUTPUT_VARIABLE VERSION
       OUTPUT_STRIP_TRAILING_WHITESPACE
     )
     CONFIGURE_FILE(\${SRC} \${DST} @ONLY)
"
)

INCLUDE_DIRECTORIES(${CMAKE_BINARY_DIR})

ADD_CUSTOM_TARGET(
    version
    ${CMAKE_COMMAND} -D SRC=${CMAKE_BINARY_DIR}/glad_version.h.in
                     -D DST=${CMAKE_BINARY_DIR}/glad_version.h
                     -P ${CMAKE_BINARY_DIR}/version.cmake
)


macro(PRINTLIST _MSG _LST)
  message(STATUS "${_MSG}:")
  foreach(f ${_LST})
    message("    ${f}")
  endforeach(f)
endmacro(PRINTLIST)

#set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -std=c++0x")
#set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -std=c++0x")

#
#

include_directories(
    ${PROJECT_SOURCE_DIR}/src
    ${PROJECT_BINARY_DIR})

include_directories(${HDF5_INCLUDE_DIRS} ${HDF5_INCLUDE_DIR})
#
# GLPS
#


# local packages
FIND_PACKAGE(BISON)
FIND_PACKAGE(FLEX)

BISON_TARGET(glps_parser
    ${CMAKE_CURRENT_SOURCE_DIR}/parser.y
    ${CMAKE_CURRENT_BINARY_DIR}/parser.tab.c)
IF(WIN32)
    FLEX_TARGET(glps_scanner
        ${CMAKE_CURRENT_SOURCE_DIR}/scanner.l
        ${CMAKE_CURRENT_BINARY_DIR}/scanner.lex.c
        COMPILE_FLAGS "--wincompat --header-file=${CMAKE_CURRENT_BINARY_DIR}/scanner.lex.h")
ELSE(WIN32)
    FLEX_TARGET(glps_scanner
        ${CMAKE_CURRENT_SOURCE_DIR}/scanner.l
        ${CMAKE_CURRENT_BINARY_DIR}/scanner.lex.c
        COMPILE_FLAGS "--header-file=${CMAKE_CURRENT_BINARY_DIR}/scanner.lex.h")
ENDIF(WIN32)

ADD_FLEX_BISON_DEPENDENCY(glps_scanner glps_parser)

#include_directories(${PROJECT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR})
set(glps_SRC 
    ${BISON_glps_parser_OUTPUTS}
    ${FLEX_glps_scanner_OUTPUTS}
    ${CMAKE_CURRENT_SOURCE_DIR}/glpsfuncs.c
    ${CMAKE_CURRENT_SOURCE_DIR}/elemtree.c
    )

#    ${CMAKE_CURRENT_SOURCE_DIR}/glpsutil.cc)
 
if(BUILD_TESLA_SHARED)
    add_library(glps SHARED ${glps_SRC})
endif(BUILD_TESLA_SHARED)


#MESSAGE(STATUS "Flex: ${FLEX_EXECUTABLE}")
#MESSAGE(STATUS "${BISON_glps_parser_OUTPUTS}")
#MESSAGE(STATUS "${FLEX_glps_scanner_OUTPUTS}")
#MESSAGE(STATUS "${glps_SRC}")
#
# Create target for the parser
#
#FIND_PROGRAM(LEX_EXE flex)
#IF(LEX_EXE STREQUAL "LEX_EXE-NOTFOUND")
#    MESSAGE(FATAL_ERROR "ERROR: flex not found!")
#ENDIF(LEX_EXE STREQUAL "LEX_EXE-NOTFOUND")#
#
#FIND_PROGRAM(YACC_EXE bison )
#IF(YACC_EXE STREQUAL "YACC_EXE-NOTFOUND")
#    MESSAGE(FATAL_ERROR "ERROR: yacc not found!")
#ENDIF(YACC_EXE STREQUAL "YACC_EXE-NOTFOUND")

#MESSAGE(STATUS "Found lex : ${LEX_EXE}")
#MESSAGE(STATUS "Found yacc: ${YACC_EXE}")

# Create custom command for flex/lex (note the outputs)
# -i generate case-insensitive scanner
#ADD_CUSTOM_COMMAND(TARGET glps-static PRE_BUILD
#    #SOURCE ${PROJECT_SOURCE_DIR}/src/glps/scanner.l
#    COMMAND ${LEX_EXE} 
#    ARGS -Ca -i -o${CMAKE_CURRENT_BINARY_DIR}/scanner.lex.c 
#        ${PROJECT_SOURCE_DIR}/src/glps/scanner.l
#    DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/parser.tab.c
#    OUTPUTS ${CMAKE_CURRENT_BINARY_DIR}/scanner.lex.c
#)

# Create custom command for bison/yacc (note the DEPENDS)
#ADD_CUSTOM_COMMAND(TARGET glps-static
#    #SOURCE ${PROJECT_SOURCE_DIR}/src/glps/parser.y
#    COMMAND ${YACC_EXE}
#    ARGS -t -v -d -y ${PROJECT_SOURCE_DIR}/src/glps/parser.y 
#        -o ${CMAKE_CURRENT_BINARY_DIR}/parser.tab.c
#    OUTPUTS ${CMAKE_CURRENT_BINARY_DIR}/parser.tab.c
#)

#
# Since parser.c does not exists yet when cmake is run, mark
# it as generated
set_source_files_properties(${CMAKE_CURRENT_BINARY_DIR}/parser.tab.h GENERATED)
set_source_files_properties(${CMAKE_CURRENT_BINARY_DIR}/parser.tab.c GENERATED)
set_source_files_properties(${CMAKE_CURRENT_BINARY_DIR}/scanner.lex.h GENERATED)
set_source_files_properties(${CMAKE_CURRENT_BINARY_DIR}/scanner.lex.c GENERATED)

add_library(glps-static STATIC ${glps_SRC})
# The library target "glps" already has a default OUTPUT_NAME of "glps",
# so we don't need to change it.  The library target "glps-static" has a
# default OUTPUT_NAME of "glps-static", so change it.
set_target_properties(glps-static PROPERTIES OUTPUT_NAME "glps"
    INCLUDE_DIRECTORIES "${CMAKE_CURRENT_SOURCE_DIR};${CMAKE_CURRENT_BINARY_DIR};${HDF5_INCLUDE_DIR}")
# Now the library target "glps-static" will be named "glps.lib" with MS
# tools.  This conflicts with the "glps.lib" import library corresponding
# to "glps.dll", so we add a "lib" prefix (which is default on other
# platforms anyway):
set_target_properties(glps-static PROPERTIES PREFIX "lib")

# parse the full version number from glps.h and include in GLPS_FULL_VERSION
#file(READ ${CMAKE_CURRENT_SOURCE_DIR}/glpsfuncs.h _glps_h_contents)
#string(REGEX REPLACE ".*#define[ \t]+GLPS_VERSION[ \t]+\"([0-9A-Za-z.]+)\".*"
#    "\\1" GLPS_FULL_VERSION ${_glps_h_contents})


if(BUILD_TESLA_SHARED)
    install(TARGETS glps DESTINATION lib${LIB_SUFFIX})
endif(BUILD_TESLA_SHARED)

install(TARGETS glps-static DESTINATION lib${LIB_SUFFIX})
install(FILES glpsfuncs.h DESTINATION include)


add_executable(aplattice aplattice.cc)
TARGET_LINK_LIBRARIES(aplattice glps-static)

#
# Packaging
#

include(InstallRequiredSystemLibraries)
#set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/License.txt")
#set(CPACK_PACKAGE_VERSION_MAJOR "${tesla_VERSION_MAJOR}")
#set(CPACK_PACKAGE_VERSION_MINOR "${tesla_VERSION_MINOR}")

#MESSAGE(STATUS "nsis-package-name: ${CPACK_NSIS_PACKAGE_NAME}")

#set(tesla_HEADERS  ${CMAKE_CURRENT_BINARY_DIR}/tesla_config.h)
#install(FILES ${tesla_HEADERS} DESTINATION include/tesla)

set(CPACK_SOURCE_FILES "${CMAKE_SOURCE_DIR}/src")
set(CPACK_SOURCE_FILE_LICENSE 
    "${CMAKE_CURRENT_SOURCE_DIR}/License.txt")

set(CPACK_PACKAGE_VERSION_MAJOR "${tesla_VERSION_MAJOR}")
set(CPACK_PACKAGE_VERSION_MINOR "${tesla_VERSION_MINOR}")
set(CPACK_PACKAGE_VERSION_PATCH "${tesla_VERSION_PATCH}")
set(CPACK_SOURCE_PACKAGE_FILE_NAME "tesla-${tesla_VERSION_STR}")

list(APPEND CPACK_SOURCE_IGNORE_FILES ".*o$")
list(APPEND CPACK_SOURCE_IGNORE_FILES ".gitignore$")
list(APPEND CPACK_SOURCE_IGNORE_FILES "/regression/")
list(APPEND CPACK_SOURCE_IGNORE_FILES "/test-suite/")
list(APPEND CPACK_SOURCE_IGNORE_FILES "/debian/")
list(APPEND CPACK_SOURCE_IGNORE_FILES "/python/doc/_build")
list(APPEND CPACK_SOURCE_IGNORE_FILES "/python/doc/html")
list(APPEND CPACK_SOURCE_IGNORE_FILES "/util/")
list(APPEND CPACK_SOURCE_IGNORE_FILES "/Makefile")

set(CPACK_SOURCE_STRIP_FILES "/tesla_config.h.in")

message(STATUS "IGNORE:${CPACK_SOURCE_IGNORE_FILES}")
set(CPACK_PACKAGE_CONTACT "lyyang@bnl.gov")

if(WIN32)
    set(CPACK_GENERATOR "NSIS")
else(WIN32)
    set(CPACK_GENERATOR "TGZ")

    # rpm
    #if (BUILD_TESLA_RPM)
    #    set(CPACK_GENERATOR ${CPACK_GENERATOR} "RPM")
    #endif(BUILD_TESLA_RPM)

    # debian
    #if (BUILD_TESLA_DEB)
    #set(CPACK_GENERATOR ${CPACK_GENERATOR} "DEB")
    #endif(BUILD_TESLA_DEB)

    set(CPACK_SOURCE_GENERATOR "TGZ")
endif(WIN32)
set(CPACK_SOURCE_PACKAGE_FILE_NAME "tesla-${tesla_VERSION_STR}")

# the last step of CPack
include(CPack)


message(STATUS "")
message(STATUS "-------------- Summary --------------")
message("cmake: ${CMAKE_VERSION}")
message("buildtype     = ${CMAKE_BUILD_TYPE}")
message("prefix        = ${CMAKE_INSTALL_PREFIX} ${CKE_INSTALL_LIBDIR} ${LIB_SFFIX}")
message("version       = ${tesla_VERSION_STR}")
message("revision      = ${tesla_REVISION}")
message("HDF5:")
message("  parallel = ${HDF5_IS_PARALLEL}")
message("  inc      = ${HDF5_INCLUDE_DIRS}")
message("  libs     = ${HDF5_LIBRARIES}")
message("  C libs   = ${HDF5_C_LIBRARIES}")
message("  HL lib   = ${HDF5_HL_LIBRARIES}")
message("Boost:")
message("  static   = ${Boost_USE_STATIC_LIBS}")
message("  multithreaded = ${Boost_USE_MULTITHREADED}")
message("  lib      = ${Boost_LIBRARIES}")
message("MPI:")
message("  inc      = ${MPI_INCLUDE_PATH}")
message("  lib      = ${MPI_LIBRARIES}")
message("  clib     = ${MPI_C_LIBRARY}")
message("  extra    = ${MPI_EXTRA_LIBRARY}")
message("  clibs    = ${MPI_C_LIBRARIES}")
message("  link     = ${MPI_LINK_FLAGS}")
message("  flags    = ${MPI_COMPILE_FLAGS}")
message("GSL:")
message("  inc      = ${GSL_INCLUDE_DIR}")
message("  libs     = ${GSL_LIBRARIES}")
message("Doxygen: ${DOXYGEN_EXECUTABLE}")
message("Latex: ${LATEX_COMPILER}, ${PDFLATEX_COMPILER}")

#message(STATUS "doc    : ${BUILD_TESLA_DOCUMENT}")
#message(STATUS "test   : ${BUILD_TESLA_TEST}")
#message(STATUS "example: ${BUILD_TESLA_EXAMPLE}")
#message(STATUS "deb    : ${BUILD_TESLA_DEB}")
#message(STATUS "rpm    : ${BUILD_TESLA_RPM}")
message(STATUS "--------------------------------------")
message(STATUS "cpack-package-version: ${CPACK_PACKAGE_VERSION}")
message(STATUS "")

