########################################################################
#
#  @(#)$Id:$
#
# This file is distributed under the terms in the attached LICENSE file.
# If you do not find this file, copies can be found by writing to:
# Intel Research Berkeley, 2150 Shattuck Avenue, Suite 1300,
# Berkeley, CA, 94704.  Attention:  Intel License Inquiry.
# Or
# UC Berkeley EECS Computer Science Division, 387 Soda Hall #1776, 
# Berkeley, CA,  94707. Attention: P2 Group.
#
# DESCRIPTION: Makefile for the P2 core library
#

ADD_LIBRARY(p2pel 
            pel_opcode_decls.gen.h  
            pel_lexer.C
            pel_vm.C 
            secureUtil.C 
            secureUtil1.C 
)
INCLUDE(FindPythonInterp)


# Create custom command for running python
ADD_CUSTOM_COMMAND(OUTPUT ${CMAKE_CURRENT_SOURCE_DIR}/pel_opcode_decls.gen.h    
                          ${CMAKE_CURRENT_SOURCE_DIR}/pel_opcode_defns.gen.h 
                          ${CMAKE_CURRENT_SOURCE_DIR}/pel_opcode_tokens.gen.h
                          ${CMAKE_CURRENT_SOURCE_DIR}/pel_opcode_descriptions.gen.txt
                   COMMAND ${PYTHON_EXECUTABLE} 
                   ARGS ${CMAKE_CURRENT_SOURCE_DIR}/pel_gen.py
                   WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
		   DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/pel_gen.py
                   COMMENT "Generating PEL lexer")


# Add pel_gen.py outputs to the list of sources
SET(P2_SRCS ${P2_SRCS} ${CMAKE_CURRENT_SOURCE_DIR}/pel_opcode_decls.gen.h ${CMAKE_CURRENT_SOURCE_DIR}/pel_opcode_defns.gen.h ${CMAKE_CURRENT_SOURCE_DIR}/pel_opcode_tokens.gen.h)
# Since these files do not exist yet when cmake is run, mark
# them as generated
SET_SOURCE_FILES_PROPERTIES(${CMAKE_CURRENT_SOURCE_DIR}/pel_opcode_decls.gen.h ${CMAKE_CURRENT_SOURCE_DIR}/pel_opcode_defns.gen.h ${CMAKE_CURRENT_SOURCE_DIR}/pel_opcode_tokens.gen.h GENERATED)

## FindFLEX from http://www.cmake.org/Bug/bug.php?op=show&bugid=4018&pos=4
FIND_PACKAGE(FLEX)
FLEX_TARGET(PelLexer ${P2_SOURCE_DIR}/p2core/pel/pel_lexer.lex ${CMAKE_CURRENT_BINARY_DIR}/pel_lexer.C)

TARGET_LINK_LIBRARIES(p2pel boost_regex${TOOLSET}-mt-1_35 boost_date_time${TOOLSET}-mt-1_35 crypto ssl) 

INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR})

INSTALL(TARGETS p2pel LIBRARY DESTINATION lib
		      ARCHIVE DESTINATION lib)
