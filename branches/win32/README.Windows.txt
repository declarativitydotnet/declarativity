Brief install notes for now ... to be expanded later.


- tortoisesvn 1.4.4 from http://tortoisesvn.net/downloads
- CMake 2.4 from http://www.cmake.org/HTML/Download.html
- visual c++ 2005 express from http://msdn.microsoft.com/vstudio/express/downloads/ 
- Microsoft Windows SDK http://www.microsoft.com/downloads/details.aspx?FamilyID=c2b1e300-f358-4523-b479-f53d234cdccf&displaylang=en&Hash=1NLJoDA6OJ1cwNisYRKWdJhFYM7EsrH65Eik6B%2b4Q5PEYAi6kP4ms0nFVESe8dSL17l3iIFwV8EW4e%2bNZIFznA%3d%3d
and run Start->Programs->Microsoft Windows SDK->Visual Studio Registration->Integrate Windows SDK with Visual Studio 2005
- boost 1_34_0 from http://www.boost-consulting.com/products/free
- python 2.5.1: http://www.python.org/download/releases/2.5.1
- SSL 0.9.8e http://www.slproweb.com/download/Win32OpenSSL-0_9_8e.exe
- bison 2.31 and flex from cygwin: www.cygwin.com
	run installer, choose devel->bison and devel->flex.  Nothing else is needed!

- In Visual Studio:
	Tools->Options...->Projects and Solutions->VC++ Directories
        Pull down Show Directories For:
		 choose  Include Files
	Add C:\cygwin\usr\include

(This may not be necessary)
- Start->Control Panel->System->Advanced->Environment Variables
      User Variables
      Add "C:\Program Files\boost\boost_1_34_0\lib" to PATH
      Restart Visual Studio


Configure CMake:

- Boost_INCLUDE_DIR = C:/Program Files/boost/boost_1_34_0
- OPENSSL_INCLUDE_DIR = C:/OpenSSL/include
- SSL_EAY_DEBUG = C:/OpenSSL/lib/VC/ssleay32MDd.lib
- SSL_EAY_RELEASE=C:/OpenSSL/lib/VC/ssleay32MD.lib

Run again and get more errors.
- BISON_EXECUTABLE=C:/cygwin/bin/bison.exe
- FLEX_EXECUTABLE=C:/cygwin/bin/flex.exe
- FL_LIBRARY=C:/cygwin/usr/lib/libfl.a
- PYTHON_DEBUG_LIBRARY=C:/Python25/libs/libpython25.a
- PYTHON_EXECUTABLE=C:/Python25/python.exe
- PYTHON_INCLUDE_PATH-=C:/Python25/include
- PYTHON_LIBRARY=C:/Python25/libs/python25.lib