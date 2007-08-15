ADD_LIBRARY(p2debugger tableTracer.C 
 			ruleTracer.C 
			tap.C 
			execRecord.C 
			traceTuple.C
)
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR})