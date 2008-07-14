CMake (www.cmake.org) is an open-source, cross-platfom build environment. It
supports native build processes on Unix and Windows platforms under a variety
of compilers. It can generate:

- Unix Makefiles
- Apple XCode projects
- KDevelop 3 projects
- Microsoft Visual Studio solutions
- Nmake files

and so on.

Very soon, we will replace the Gnu autoconf/automake/libtool toolset for P2
with cmake, to allow us better cross-platform compatibility (especially for
the upcoming Windows port).

In the interim, you can use either toolchain. To use cmake, install it using
your package manager, or download it and install it from www.cmake.org. For
cleanliness, we will follow cmake convention and do an "out-of-source" build.
So if you've got p2 installed in ~/devel/trunk, you can do as follows:

% mkdir ~/devel/build
% cd ~/devel/build
% cmake ../trunk [-G <output_type>]

Note that there's nothing magic about the directory location I chose -- you
can put your build wherever you like.

The -G flag to cmake controls the kind of output files it generates. On linux
and OS X, the default is "Unix Makefiles".

If you have things installed in reasonable places, this is all you should need
to do. You can then proceed and run make from the root of the ~/devel/build
directory. Cmake will put your output files (flex/bison output, .a's, and
binaries) in fairly natural places under the build directory.

If you run into trouble, you can use "ccmake" instead of cmake in the
instructions above, which will give you a (somewhat crude) curses-based
program that will allow you to type in any paths, etc. that CMake couldn't
figure out on its own

When you want to clean up, you can simply run "rm -rf ~/devel/build". There is
no "make clean" target in cmake, you simply blow away the directory where you
did the build.
