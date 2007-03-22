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
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR} ${P2_BINARY_DIR}/overlog 			
					../elements 
					../net 
					../debugger)
