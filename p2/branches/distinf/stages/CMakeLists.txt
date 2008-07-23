find_package(LibXml++)

add_definitions(${LIBXMLPP_DEFINITIONS})
include_directories(${LIBXMLPP_INCLUDE_DIR})
link_libraries(${LIBXMLPP_LIBRARIES})
link_directories(${LIBXMLPP_LINK_DIR})


ADD_LIBRARY(p2stages 
            stageLoader.C
            tokenizer.C
            CSVstage.C 
            PRLstage.C 
            SQLiteLoadStage.C
            SQLiteSaveStage.C)

TARGET_LINK_LIBRARIES(p2stages p2core sqlite3)

INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR})

INSTALL(TARGETS p2stages LIBRARY DESTINATION lib
			 ARCHIVE DESTINATION lib)

ADD_DEFINITIONS(-DBOOST_UBLAS_ENABLE_PROXY_SHORTCUTS)
