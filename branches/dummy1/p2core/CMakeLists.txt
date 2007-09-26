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

ADD_LIBRARY(p2core 
            pel_opcode_decls.gen.h  
            CSVlex.C
            ID.C 
            aggFactory.C 
            commitManager.C
            commonTable.C 
            compileUtil.C
            dot.C 
            element.C 
            elementSpec.C 
            fdbuf.C
            list.C 
            loggerI.C 
            loop.C
            oper.C
            p2Time.C 
            pel_lexer.C
            pel_vm.C 
            plumber.C 
            refTable.C 
            reporting.C
            scheduler.C 
            table2.C 
            tableManager.C
            tuple.C 
            val_double.C 
            val_id.C 
            val_int32.C 
            val_int64.C 
            val_ip_addr.C 
            val_list.C 
            val_matrix.C
            val_null.C 
            val_opaque.C 
            val_str.C 
            val_time.C 
            val_tuple.C 
            val_uint32.C 
            val_uint64.C 
            val_vector.C 
            value.C 
            xdrbuf.C 
)
INCLUDE(FindPythonInterp)


# Create custom command for running python
ADD_CUSTOM_COMMAND(OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/pel_opcode_decls.gen.h    
                          ${CMAKE_CURRENT_BINARY_DIR}/pel_opcode_defns.gen.h 
                          ${CMAKE_CURRENT_BINARY_DIR}/pel_opcode_tokens.gen.h
                          ${CMAKE_CURRENT_BINARY_DIR}/pel_opcode_descriptions.gen.txt
                   COMMAND ${PYTHON_EXECUTABLE} 
                   ARGS ${CMAKE_CURRENT_SOURCE_DIR}/pel_gen.py
                   SOURCE ${CMAKE_CURRENT_SOURCE_DIR}/pel_gen.py
                   COMMENT "Generating PEL lexer")


# Add pel_gen.py outputs to the list of sources
SET(P2_SRCS ${P2_SRCS} ${CMAKE_CURRENT_BINARY_DIR}/pel_opcode_decls.gen.h ${CMAKE_CURRENT_BINARY_DIR}/pel_opcode_defns.gen.h ${CMAKE_CURRENT_BINARY_DIR}/pel_opcode_tokens.gen.h)
# Since these files do not exist yet when cmake is run, mark
# them as generated
SET_SOURCE_FILES_PROPERTIES(${CMAKE_CURRENT_BINARY_DIR}/pel_opcode_decls.gen.h ${CMAKE_CURRENT_BINARY_DIR}/pel_opcode_defns.gen.h ${CMAKE_CURRENT_BINARY_DIR}/pel_opcode_tokens.gen.h GENERATED)

## FindFLEX from http://www.cmake.org/Bug/bug.php?op=show&bugid=4018&pos=4
FIND_PACKAGE(FLEX)
FLEX_TARGET(PelLexer ${P2_SOURCE_DIR}/p2core/pel_lexer.lex ${CMAKE_CURRENT_BINARY_DIR}/pel_lexer.C)

TARGET_LINK_LIBRARIES(p2core boost_regex boost_date_time crypto ssl) 
#TARGET_LINK_LIBRARIES(p2core libeay32MDd ws2_32) 

INCLUDE_DIRECTORIES(${CMAKE_CURRENT_BINARY_DIR})
