# Defines
#
#  LIBXMLPP_FOUND - the system has libxml++
#  LIBXMLPP_INCLUDE_DIR - the include directory
#  LIBXMLPP_LINK_DIR - the link directory
#  LIBXMLPP_LIBRARIES - the libraries needed to use libxml++
#  LIBXMLPP_DEFINITIONS - the compiler switches needed to use libxml++

# Based on the FindLibXml2 script,
# Copyright (c) 2006, Alexander Neundorf, <neundorf@kde.org>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

IF (LIBXMLPP_INCLUDE_DIR AND LIBXMLPP_LIBRARIES)
   # in cache already
   SET(LibXmlpp_FIND_QUIETLY TRUE)
ENDIF (LIBXMLPP_INCLUDE_DIR AND LIBXMLPP_LIBRARIES)

IF (WIN32)
   SET(LIBXMLPP_DEFINITIONS "-I\"C:/Program Files/Common Files/GTK/2.0/include\" -I\"C:/Program Files/Common Files/GTK/2.0/lib/libxml++-2.6/include\" -I\"C:/Program Files/Common Files/GTK/2.0/include/glibmm-2.4\" -I\"C:/Program Files/Common Files/GTK/2.0/lib/glibmm-2.4/include\" -I\"C:/Program Files/Common Files/GTK/2.0/include/glib-2.0\" -I\"C:/Program Files/Common Files/GTK/2.0/lib/glib-2.0/include\"")
   SET(_LibXmlppIncDir "C:/Program Files/Common Files/GTK/2.0/include")
   SET(_LibXmlppLinkDir "C:/Program Files/Common Files/GTK/2.0/lib")
   SET(LIBXMLPP_LINK_DIR "C:/Program Files/Common Files/GTK/2.0/lib")
   SET(LIBXMLPP_LIBRARIES xml++-2.6 glibmm-2.4)
ELSE (WIN32)
   # use pkg-config to get the directories and then use these values
   # in the FIND_PATH() and FIND_LIBRARY() calls
   INCLUDE(UsePkgConfig)
  PKGCONFIG(libxml++-2.6 _LibXmlppIncDir _LibXmlppLinkDir _LibXmlppLinkFlags _LibXmlppCflags)
   SET(LIBXMLPP_DEFINITIONS ${_LibXmlppCflags})
ENDIF (WIN32)

FIND_PATH(LIBXMLPP_INCLUDE_DIR libxml++/libxml++.h
   PATHS ${_LibXmlppIncDir} ${_LibXmlppIncDir}/libxml++-2.6
   PATH_SUFFIXES libxml2
   )

IF (NOT WIN32)
FIND_LIBRARY(LIBXMLPP_LIBRARIES NAMES xml++-2.6 libxml2
   PATHS
   ${_LibXmlppLinkDir}
   )
ENDIF (NOT WIN32)

IF (LIBXMLPP_INCLUDE_DIR AND LIBXMLPP_LIBRARIES)
   SET(LIBXMLPP_FOUND TRUE)
ELSE (LIBXMLPP_INCLUDE_DIR AND LIBXMLPP_LIBRARIES)
   SET(LIBXMLPP_FOUND FALSE)
ENDIF (LIBXMLPP_INCLUDE_DIR AND LIBXMLPP_LIBRARIES)

IF (LIBXMLPP_FOUND)
   IF (NOT LibXmlpp_FIND_QUIETLY)
      MESSAGE(STATUS "Found LibXml++: ${LIBXMLPP_LIBRARIES}")
   ENDIF (NOT LibXmlpp_FIND_QUIETLY)
ELSE (LIBXMLPP_FOUND)
   IF (LibXmlpp_FIND_REQUIRED)
      MESSAGE(SEND_ERROR "Could NOT find LibXml++")
   ENDIF (LibXmlpp_FIND_REQUIRED)
ENDIF (LIBXMLPP_FOUND)

MARK_AS_ADVANCED(LIBXMLPP_INCLUDE_DIR LIBXMLPP_LIBRARIES)
