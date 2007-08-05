Project(P2)

SUBDIRS(eventLoop p2core elements stages net overlog planner debugger unitTests tests python)

# Main decisions
SET(BUILD_SHARED_LIBS ON)



# find our custom CMake modules
SET(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${P2_SOURCE_DIR}/cmake)

# Look for main packages we need
FIND_PACKAGE(Boost)
FIND_PACKAGE(OpenSSL)
INCLUDE(FindPythonLibs)
## FindFLEX from http://www.cmake.org/Bug/bug.php?op=show&bugid=4018&pos=4
FIND_PACKAGE(FLEX)
FIND_PACKAGE(BISON)

# Look for include files
INCLUDE (CheckIncludeFiles)
# usage: CHECK_INCLUDE_FILES (<header> <RESULT_VARIABLE> )
CHECK_INCLUDE_FILES( rpc/rpc.h HAVE_RPC_RPC_H)
CHECK_INCLUDE_FILES( "rpc/types.h;rpc/xdr.h" HAVE_RPC_XDR_H)

# Check for unusual functions
INCLUDE(CheckSymbolExists)
CHECK_SYMBOL_EXISTS(xdr_uint32_t "rpc/types.h;rpc/xdr.h" HAVE_XDR_UINT32_T)
CHECK_SYMBOL_EXISTS(xdr_u_int32_t "rpc/types.h;rpc/xdr.h" HAVE_XDR_U_INT32_T)
# Check for exp10, which is a g++ special.  CHECK_SYMBOL_EXISTS fires off gcc, not g++, so we
# write a custom check.
INCLUDE(CheckCXXSourceCompiles)
CHECK_CXX_SOURCE_COMPILES("#include <math.h> 
main(){double i = exp10(2.0);}" HAVE_EXP10)

# Output the config.h file
CONFIGURE_FILE(${CMAKE_CURRENT_SOURCE_DIR}/config.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config.h)

# Avoid linker warnings on Darwin related to circular dependencies among shared libs.
IF(${CMAKE_SYSTEM} MATCHES Darwin)
	SET(CMAKE_SHARED_LIBRARY_CREATE_C_FLAGS "${CMAKE_SHARED_LIBRARY_CREATE_C_FLAGS}  -flat_namespace -undefined suppress")
	SET(CMAKE_SHARED_LIBRARY_CREATE_CXX_FLAGS "${CMAKE_SHARED_LIBRARY_CREATE_CXX_FLAGS}  -flat_namespace -undefined suppress")
ENDIF(${CMAKE_SYSTEM} MATCHES Darwin)

# add random environment variable definitions
# deal with Boost time shtuff: we want nanoseconds!
ADD_DEFINITIONS(-DBOOST_DATE_TIME_POSIX_TIME_STD_CONFIG)

# set include path for this and all subdirs
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_CURRENT_BINARY_DIR}/overlog	
                    ${CMAKE_CURRENT_BINARY_DIR}/p2core 
		    ${CMAKE_CURRENT_SOURCE_DIR}/eventLoop 
                    ${CMAKE_CURRENT_SOURCE_DIR}/elements
                    ${CMAKE_CURRENT_SOURCE_DIR}/stages
                    ${CMAKE_CURRENT_SOURCE_DIR}/net
                    ${CMAKE_CURRENT_SOURCE_DIR}/overlog
                    ${CMAKE_CURRENT_SOURCE_DIR}/planner
                    ${CMAKE_CURRENT_SOURCE_DIR}/debugger
		    ${Boost_INCLUDE_DIRS} ${PYTHON_INCLUDE_PATH} /usr/include)

# set linker path for this and all subdirs
LINK_DIRECTORIES(${Boost_LIBRARY_DIRS} 
		${P2_BINARY_DIR}/overlog 
		${P2_BINARY_DIR}/net 
		${P2_BINARY_DIR}/stages 
		${P2_BINARY_DIR}/elements 
		${P2_BINARY_DIR}/debugger 
		${P2_BINARY_DIR}/p2core  
		${P2_BINARY_DIR}/eventLoop 
		${P2_BINARY_DIR}/planner)

# uncomment the following line to get CMake variables to print to screen
#INCLUDE(CMakePrintSystemInformation)
