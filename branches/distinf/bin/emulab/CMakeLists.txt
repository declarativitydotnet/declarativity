########################################################################
# This file is distributed under the terms in the attached LICENSE file.
# If you do not find this file, copies can be found by writing to:
# Intel Research Berkeley, 2150 Shattuck Avenue, Suite 1300,
# Berkeley, CA, 94704.  Attention:  Intel License Inquiry.
# Or
# UC Berkeley EECS Computer Science Division, 387 Soda Hall #1776, 
# Berkeley, CA,  94707. Attention: P2 Group.
#

SUBDIRS(lib)

INCLUDE(FindPythonInterp)

CONFIGURE_FILE(${CMAKE_CURRENT_SOURCE_DIR}/build.ns 
               ${CMAKE_CURRENT_BINARY_DIR}/build.ns COPYONLY)

CONFIGURE_FILE(${CMAKE_CURRENT_SOURCE_DIR}/runexpr.in 
               ${CMAKE_CURRENT_BINARY_DIR}/runexpr.py)

INSTALL(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/scripts DESTINATION bin
        FILE_PERMISSIONS OWNER_EXECUTE OWNER_WRITE OWNER_READ GROUP_EXECUTE GROUP_READ WORLD_EXECUTE WORLD_READ 
        DIRECTORY_PERMISSIONS OWNER_EXECUTE OWNER_WRITE OWNER_READ GROUP_EXECUTE GROUP_READ WORLD_EXECUTE WORLD_READ)
