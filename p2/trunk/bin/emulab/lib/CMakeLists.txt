########################################################################
# This file is distributed under the terms in the attached LICENSE file.
# If you do not find this file, copies can be found by writing to:
# Intel Research Berkeley, 2150 Shattuck Avenue, Suite 1300,
# Berkeley, CA, 94704.  Attention:  Intel License Inquiry.
# Or
# UC Berkeley EECS Computer Science Division, 387 Soda Hall #1776, 
# Berkeley, CA,  94707. Attention: P2 Group.
#

INCLUDE(FindPythonInterp)

CONFIGURE_FILE(${CMAKE_CURRENT_SOURCE_DIR}/emulab.in 
               ${CMAKE_CURRENT_BINARY_DIR}/emulab.py)

CONFIGURE_FILE(${CMAKE_CURRENT_SOURCE_DIR}/emulabclient.in 
               ${CMAKE_CURRENT_BINARY_DIR}/emulabclient.py)

CONFIGURE_FILE(${CMAKE_CURRENT_SOURCE_DIR}/mkns2.in 
               ${CMAKE_CURRENT_BINARY_DIR}/mkns2.py)

CONFIGURE_FILE(${CMAKE_CURRENT_SOURCE_DIR}/sshxmlrpc.in 
               ${CMAKE_CURRENT_BINARY_DIR}/sshxmlrpc.py)

CONFIGURE_FILE(${CMAKE_CURRENT_SOURCE_DIR}/topology.in 
               ${CMAKE_CURRENT_BINARY_DIR}/topology.py)
