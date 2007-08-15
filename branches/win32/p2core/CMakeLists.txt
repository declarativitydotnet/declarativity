ADD_LIBRARY(p2core 
            loop.C
            p2Time.C
            pel_opcode_decls.gen.h  
	    reporting.C 
	    commonTable.C 
	    table2.C 
	    refTable.C 
	    aggMin.C 
	    aggMax.C 
	    aggCount.C 
	    aggFactory.C 
	    val_str.C 
	    value.C 
	    fdbuf.C
	    ID.C 
	    element.C 
	    tuple.C 
	    plumber.C 
	    elementSpec.C 
	    pel_lexer.C
	    pel_vm.C 
	    val_null.C 
	    val_int32.C 
	    val_uint32.C 
	    val_int64.C 
	    val_uint64.C 
	    val_double.C 
	    val_opaque.C 
	    val_tuple.C 
	    val_time.C 
	    val_id.C 
	    loggerI.C 
	    oper.C 
	    val_ip_addr.C 
	    dot.C 
	    list.C 
	    val_list.C 
	    val_vector.C 
	    val_matrix.C 
	    stage.C 
	    stageRegistry.C 
	    CSVlex.C)
INCLUDE(FindPythonInterp)


# Create custom command for running python
ADD_CUSTOM_COMMAND(OUTPUT ${P2_BINARY_DIR}/p2core/pel_opcode_decls.gen.h 	
						  ${P2_BINARY_DIR}/p2core/pel_opcode_defns.gen.h 
						  ${P2_BINARY_DIR}/p2core/pel_opcode_tokens.gen.h
						  ${P2_BINARY_DIR}/p2core/pel_opcode_descriptions.gen.txt
	               COMMAND ${PYTHON_EXECUTABLE} 
				   ARGS ${P2_SOURCE_DIR}/p2core/pel_gen.py
                   SOURCE ${P2_SOURCE_DIR}/p2core/pel_gen.py
				   COMMENT "Generating PEL lexer")


# Add pel_gen.py outputs to the list of sources
SET(P2_SRCS ${P2_SRCS} ${P2_BINARY_DIR}/p2core/pel_opcode_decls.gen.h ${P2_BINARY_DIR}/p2core/pel_opcode_defns.gen.h ${P2_BINARY_DIR}/p2core/pel_opcode_tokens.gen.h)
# Since these files do not exist yet when cmake is run, mark
# it as generated
SET_SOURCE_FILES_PROPERTIES(${P2_BINARY_DIR}/p2core/pel_opcode_decls.gen.h ${P2_BINARY_DIR}/p2core/pel_opcode_defns.gen.h ${P2_BINARY_DIR}/p2core/pel_opcode_tokens.gen.h GENERATED)

## FindFLEX from http://www.cmake.org/Bug/bug.php?op=show&bugid=4018&pos=4
FIND_PACKAGE(FLEX)
FLEX_TARGET(PelLexer ${P2_SOURCE_DIR}/p2core/pel_lexer.lex ${P2_BINARY_DIR}/p2core/pel_lexer.C)

TARGET_LINK_LIBRARIES(p2core ${CRYPTOLIBS} ${PLATFORM_LIBS}) 
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR})