########################################################################
#
#  @(#)$Id: Makefile.am 1246 2007-07-16 23:57:51Z maniatis $
#
# This file is distributed under the terms in the attached LICENSE file.
# If you do not find this file, copies can be found by writing to:
# Intel Research Berkeley, 2150 Shattuck Avenue, Suite 1300,
# Berkeley, CA, 94704.  Attention:  Intel License Inquiry.
# Or
# UC Berkeley EECS Computer Science Division, 387 Soda Hall #1776, 
# Berkeley, CA,  94707. Attention: P2 Group.
#

INCLUDE(FindPythonInterp)

CONFIGURE_FILE(${CMAKE_CURRENT_SOURCE_DIR}/p2terminal.in ${CMAKE_CURRENT_BINARY_DIR}/p2terminal.py)
CONFIGURE_FILE(${CMAKE_CURRENT_SOURCE_DIR}/p2insert.in ${CMAKE_CURRENT_BINARY_DIR}/p2insert.py)
CONFIGURE_FILE(${CMAKE_CURRENT_SOURCE_DIR}/loadManyChords.in ${CMAKE_CURRENT_BINARY_DIR}/loadManyChords.py)
CONFIGURE_FILE(${CMAKE_CURRENT_SOURCE_DIR}/planetlab.in ${CMAKE_CURRENT_BINARY_DIR}/planetlab.py)
CONFIGURE_FILE(${CMAKE_CURRENT_SOURCE_DIR}/monolithic.in ${CMAKE_CURRENT_BINARY_DIR}/monolithic.py)
CONFIGURE_FILE(${CMAKE_CURRENT_SOURCE_DIR}/psetup.in ${CMAKE_CURRENT_BINARY_DIR}/psetup.py)
CONFIGURE_FILE(${CMAKE_CURRENT_SOURCE_DIR}/setupMonolithic.in ${CMAKE_CURRENT_BINARY_DIR}/setupMonolithic.py)
CONFIGURE_FILE(${CMAKE_CURRENT_SOURCE_DIR}/dfrun.in ${CMAKE_CURRENT_BINARY_DIR}/dfrun.py)


