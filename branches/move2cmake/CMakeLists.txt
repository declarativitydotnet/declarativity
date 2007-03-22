Project(P2)

SUBDIRS(eventLoop p2core elements stages net overlog planner debugger unitTests tests python)

# find our custom CMake modules
SET(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${P2_SOURCE_DIR}/cmake)

FIND_PACKAGE(Boost)
FIND_PACKAGE(OpenSSL)
INCLUDE(FindPythonLibs)
## FindFLEX from http://www.cmake.org/Bug/bug.php?op=show&bugid=4018&pos=4
FIND_PACKAGE(FLEX)
FIND_PACKAGE(BISON)

# tell gcc to treat all .c and .C files as C++
#IF(CMAKE_COMPILER_IS_GNUCC)
	#SET (CMAKE_COMPILER_IS_GNUCXX 1)
#ENDIF(CMAKE_COMPILER_IS_GNUCC)
#IF(${CMAKE_GENERATOR} MATCHES "Visual Studio")
#ADD_DEFINITIONS(/TP) 
#ENDIF(${CMAKE_GENERATOR} MATCHES "Visual Studio")

# SET(CMAKE_CXX_SOURCE_FILE_EXTENSIONS C;M;c++;cc;cpp;cxx;m;mm;c)

# deal with Boost time shtuff: we want nanoseconds!
ADD_DEFINITIONS(-DBOOST_DATE_TIME_POSIX_TIME_STD_CONFIG)

INCLUDE_DIRECTORIES(${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_CURRENT_BINARY_DIR}/overlog	
                    ${CMAKE_CURRENT_BINARY_DIR}/p2core 
					${CMAKE_CURRENT_SOURCE_DIR} 
					${CMAKE_CURRENT_SOURCE_DIR}/eventLoop 
                    ${CMAKE_CURRENT_SOURCE_DIR}/p2core 
                    ${CMAKE_CURRENT_SOURCE_DIR}/elements
                    ${CMAKE_CURRENT_SOURCE_DIR}/stages
                    ${CMAKE_CURRENT_SOURCE_DIR}/net
                    ${CMAKE_CURRENT_SOURCE_DIR}/overlog
                    ${CMAKE_CURRENT_SOURCE_DIR}/planner
                    ${CMAKE_CURRENT_SOURCE_DIR}/debugger
	                ${Boost_INCLUDE_DIRS} ${PYTHON_INCLUDE_PATH} /usr/include)
LINK_DIRECTORIES(${Boost_LIBRARY_DIRS} ${P2_BINARY_DIR}/overlog ${P2_BINARY_DIR}/net ${P2_BINARY_DIR}/stages ${P2_BINARY_DIR}/elements ${P2_BINARY_DIR}/debugger ${P2_BINARY_DIR}/p2core  ${P2_BINARY_DIR}/eventLoop ${P2_BINARY_DIR}/planner)
