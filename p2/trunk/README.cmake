CMake (www.cmake.org) is an open-source, cross-platform build environment. It
supports native build processes on Unix and Windows platforms under a variety
of compilers. It can generate:

- Unix Makefiles
- Apple XCode projects
- KDevelop 3 projects
- Microsoft Visual Studio solutions
- Nmake files

and so on.

To use cmake, install it using your package manager, or download it and
install it from www.cmake.org. If you've got the p2 source in
~/devel/trunk, you can do as follows:

% cd ~/devel/trunk
% cmake . [-G <output_type>]

Note that there's nothing magic about the directory location I chose -- you
can put your build outside the source if you like.

The -G flag to cmake controls the kind of output files it generates. On linux
and OS X, the default is "Unix Makefiles".

If you have things installed in reasonable places, this is all you should need
to do. You can then proceed and run make from the root of the ~/devel/trunk
directory. Cmake will put your output files (flex/bison output, .a's, and
binaries) in fairly natural places under the directory.

If you run into trouble, you can use "ccmake" instead of cmake in the
instructions above, which will give you a (somewhat crude) curses-based
program that will allow you to type in any paths, etc. that CMake
couldn't figure out on its own.  Alternatively, you can edit
CMakeCache.txt after cmake generates it.  There is no need to run "make
clean" after modifying these files; cmake will rebuild targets as
necessary.


