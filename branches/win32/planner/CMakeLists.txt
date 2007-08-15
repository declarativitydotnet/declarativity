ADD_LIBRARY(p2planner 
			tableStore.C 
			planContext.C 
			localize_context.C 
			eca_context.C 
			ruleStrand.C 
			netPlanner.C 
			planner.C 
			stageStrand.C
			)
			
# need to explicitly mention dependencies on ol_lexer.h since CMake can't 
# figure it out.
ADD_DEPENDENCIES(p2planner p2overlog p2core)	

TARGET_LINK_LIBRARIES(p2planner p2core)

INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR} ${P2_BINARY_DIR}/overlog)
