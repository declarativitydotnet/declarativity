dnl
dnl Find where the libasync and headers are installed
dnl
AC_DEFUN([P2_SFSLIB],
[AC_ARG_WITH([sfslib],
	     AC_HELP_STRING([--with-sfslib],	
	                    [specify dir for sfs libs (default is /usr/local/lib/sfs)]),
	     [p2_sfslib=$withval], [p2_sfslib=/usr/local/lib/sfs])
AC_CACHE_CHECK([whether to use sfslib],
	       [p2_sfslib], 
	       [p2_sfslib=/usr/local/lib/sfs])
AC_SUBST(SFS_LIBASYNC)
AC_SUBST(SFS_LIBARPC)
unset SFS_LIBASYNC
unset SFS_LIBARPC

p2_sfslib=`echo $p2_sfslib | sed -e 's!/$!!'`

if test -d "$p2_sfslib"; then
    if ls $p2_sfslib | egrep 'libasync*.(la|so|a)$' >/dev/null 2>&1; then
	SFS_LIBASYNC="-L${p2_sfslib} -lasync"
	AC_MSG_RESULT([using libasync from $p2_sfslib])
    else	
	AC_MSG_ERROR([Could not find libasync in $p2_sfslib])
    fi
    if ls $p2_sfslib | egrep 'libarpc*.(la|so|a)$' >/dev/null 2>&1; then
	SFS_LIBARPC="-L${p2_sfslib} -larpc"
	AC_MSG_RESULT([using libarpc from $p2_sfslib])
    else
	AC_MSG_ERROR([Could not find libarpc in $p2_sfslib])
    fi
else
    AC_MSG_ERROR([Could not find sfs lib directory $p2_sfslib])
fi
])

AC_DEFUN([P2_SFSINC],
[AC_ARG_WITH([sfsinc],
	     AC_HELP_STRING([--with-sfsinc],	
	                    [specify dir for sfs libs (default is /usr/local/include/sfs)]),
	     [p2_sfsinc=$withval], [p2_sfsinc=/usr/local/include/sfs])
AC_CACHE_CHECK([whether to use sfsinc],
	       [p2_sfsinc], 
	       [p2_sfsinc=/usr/local/include/sfs])
AC_SUBST(SFS_INC)
unset SFS_INC

p2_sfsinc=`echo $p2_sfsinc | sed -e 's!/$!!'`

if test -d "$p2_sfsinc"; then
    if ls $p2_sfsinc | grep '^async.h$' >/dev/null 2>&1; then
	SFS_INC="-I${p2_sfsinc}"
	AC_MSG_RESULT([using libasync headers from $p2_sfsinc])
    else
	AC_MSG_ERROR([Could not find async.h in $p2_sfsinc])
    fi
else
    AC_MSG_ERROR([Could not find sfs include directory $p2_sfsinc])
fi

])

dnl
dnl Find where the boost is installed
dnl
AC_DEFUN([P2_BOOSTLIB],
[AC_ARG_WITH([boostlib],
	     AC_HELP_STRING([--with-boostlib],	
	                    [specify dir for boost libs (default is /usr/lib/boost)]),
	     [p2_boostlib=$withval], [p2_boostlib=/usr/lib/boost])
AC_CACHE_CHECK([whether to use boostlib],
	       [p2_boostlib], 
	       [p2_boostlib=/usr/lib/boost])
AC_SUBST(BOOST_LIB)
unset BOOST_LIB

p2_boostlib=`echo $p2_boostlib | sed -e 's!/$!!'`

if test -d "$p2_boostlib"; then
    if ls $p2_boostlib | egrep 'boost_regex*.(la|so|a)$' >/dev/null 2>&1; then
	BOOST_LIB="-L${p2_boostlib} -lboost_regex"
	AC_MSG_RESULT([using boost from $p2_boostlib])
    else	
	AC_MSG_ERROR([Could not find boost in $p2_boostlib])
    fi
else
    AC_MSG_ERROR([Could not find boost lib directory $p2_boostlib])
fi
])

AC_DEFUN([P2_BOOSTINC],
[AC_ARG_WITH([boostinc],
	     AC_HELP_STRING([--with-boostinc],	
	                    [specify dir for boost include (default is /usr/include/boost)]),
	     [p2_boostinc=$withval], [p2_boostinc=/usr/include/boost])
AC_CACHE_CHECK([whether to use boostinc],
	       [p2_boostinc], 
	       [p2_boostinc=/usr/include/boost])
AC_SUBST(BOOST_INC)
unset BOOST_INC

p2_boostinc=`echo $p2_boostinc | sed -e 's!/$!!'`

if test -d "$p2_boostinc"; then
    if ls $p2_boostinc | grep '^boost$' >/dev/null 2>&1; then
	BOOST_INC="-I${p2_boostinc}"
	AC_MSG_RESULT([using boost headers from $p2_boostinc])
    else
	AC_MSG_ERROR([Could not find boost in $p2_boostinc])
    fi
else
    AC_MSG_ERROR([Could not find boost include directory $p2_boostinc])
fi

])

dnl
dnl Find where the libpcap and headers are installed
dnl
AC_DEFUN([P2_PCAPLIB],
[AC_ARG_WITH([pcaplib],
	     AC_HELP_STRING([--with-libpcap],	
	                    [specify dir for pcap libs (default is /usr/lib)]),
	     [p2_pcaplib=$withval], [p2_pcaplib=/usr/lib])
AC_CACHE_CHECK([whether to use pcaplib],
	       [p2_pcaplib], 
	       [p2_pcaplib=/usr/lib])
AC_SUBST(PCAP_LIB)
unset PCAP_LIB

p2_pcaplib=`echo $p2_pcaplib | sed -e 's!/$!!'`

if test -d "$p2_pcaplib"; then
    if ls $p2_pcaplib | egrep 'libpcap*.(la|so|a)$' >/dev/null 2>&1; then
	PCAP_LIB="-L${p2_pcaplib} -lpcap"
	AC_MSG_RESULT([using libpcap from $p2_pcaplib])
    else
	AC_MSG_ERROR([Could not find libpcap in $p2_pcaplib])
    fi
else
    AC_MSG_ERROR([Could not find libpcap directory $p2_pcaplib])
fi
])


AC_DEFUN([P2_PCAPINC],
[AC_ARG_WITH([pcapinc],
	     AC_HELP_STRING([--with-pcapinc],	
	                    [specify dir for pcap libs (default is /usr/include)]),
	     [p2_pcapinc=$withval], [p2_pcapinc=/usr/include])
AC_CACHE_CHECK([whether to use pcapinc],
	       [p2_pcapinc], 
	       [p2_pcapinc=/usr/include])
AC_SUBST(PCAP_INC)
unset PCAP_INC

p2_pcapinc=`echo $p2_pcapinc | sed -e 's!/$!!'`

if test -d "$p2_pcapinc"; then
    if ls $p2_pcapinc | grep '^pcap.h$' >/dev/null 2>&1; then
	PCAP_INC="-I${p2_pcapinc}"
	AC_MSG_RESULT([using libpcap headers from $p2_pcapinc])
    else
	AC_MSG_ERROR([Could not find pcap.h in $p2_pcapinc])
    fi
else
    AC_MSG_ERROR([Could not find pcap include directory $p2_pcapinc])
fi
])

##################################################################
#
# Python configuration, snarfed 25 January 2005 from:
#  http://www.gnu.org/software/ac-archive/az_python.html
#


# AZ_PYTHON_DEFAULT( )
# -----------------
# Sets the default to not include Python support.

AC_DEFUN([AZ_PYTHON_DEFAULT],
[
    az_python_use=false
    AM_CONDITIONAL(PYTHON_USE, test x"$az_python_use" = x"true")
])



# AZ_PYTHON_ENABLE( [path] )
# -----------------------------------------------------------------
# Handles the various --enable-python commands.
# Input:
#   $1 is the optional search path for the python executable if needed
# Ouput:
#   PYTHON_USE (AM_CONDITIONAL) is true if python executable found
#   and --enable-python was requested; otherwise false.
#   $PYTHON contains the full executable path to python if PYTHON_ENABLE_USE
#   is true.
#
# Example:
#   AZ_PYTHON_ENABLE( )
#   or
#   AZ_PYTHON_ENABLE( "/usr/bin" )

AC_DEFUN([AZ_PYTHON_ENABLE],
[
    AC_ARG_VAR([PYTHON],[Python Executable Path])

    # unless PYTHON was supplied to us (as a precious variable),
    # see if --enable-python[=PythonExecutablePath], --enable-python,
    # --disable-python or --enable-python=no was given.
    if test -z "$PYTHON"
    then
        AC_MSG_CHECKING(for --enable-python)
        AC_ARG_ENABLE(
            python,
            AC_HELP_STRING([--enable-python@<:@=PYTHON@:>@],
                [absolute path name of Python executable]
            ),
            [
                if test "$enableval" = "yes"
                then
                    # "yes" was specified, but we don't have a path
                    # for the executable.
                    # So, let's searth the PATH Environment Variable.
                    AC_MSG_RESULT(yes)
                    AC_PATH_PROG(
                        [PYTHON],
                        python,
                        [],
                        $1
                    )
                    if test -z "$PYTHON"
                    then
                        AC_MSG_ERROR(no path to python found)
                    fi
                    az_python_use=true
                    AM_CONDITIONAL(PYTHON_USE, test x"$az_python_use" = x"true")
                    AZ_PYTHON_PREFIX( )
                elif test "$enableval" = "no"
                then
                    AC_MSG_RESULT(no)
                    az_python_use=false
                    AM_CONDITIONAL(PYTHON_USE, test x"$az_python_use" = x"true")
                else
                    # $enableval must be the executable path then.
                    AC_SUBST([PYTHON], ["${enableval}"])
                    AC_MSG_RESULT($withval)
                    az_python_use=true
                    AM_CONDITIONAL(PYTHON_USE, test x"$az_python_use" = x"true")
                    AZ_PYTHON_PREFIX( )
                fi
            ],
            [
                # --with-python was not specified.
                AC_MSG_RESULT(no)
                az_python_use=false
                AM_CONDITIONAL(PYTHON_USE, test x"$az_python_use" = x"true")
            ]
        )
    fi

])



# AZ_PYTHON_CSPEC( )
# -----------------
# Set up the c compiler options to compile Python
# embedded programs/libraries in $PYTHON_CSPEC if
# $PYTHON has been defined.

AC_DEFUN([AZ_PYTHON_CSPEC],
[
    AC_ARG_VAR( [PYTHON], [Python Executable Path] )
    if test -n "$PYTHON"
    then
        az_python_prefix=`${PYTHON} -c "import sys; print sys.prefix"`
        if test -z "$az_python_prefix"
        then
            AC_MSG_ERROR([Python Prefix is not known])
        fi
        az_python_execprefix=`${PYTHON} -c "import sys; print sys.exec_prefix"`
        az_python_version=`$PYTHON -c "import sys; print sys.version[[:3]]"`
        az_python_includespec="-I${az_python_prefix}/include/python${az_python_version}"
        if test x"$python_prefix" != x"$python_execprefix"; then
            az_python_execspec="-I${az_python_execprefix}/include/python${az_python_version}"
            az_python_includespec="${az_python_includespec} $az_python_execspec"
        fi
        az_python_ccshared=`${PYTHON} -c "import distutils.sysconfig; print distutils.sysconfig.get_config_var('CFLAGSFORSHARED')"`
        az_python_cspec="${az_python_ccshared} ${az_python_includespec}"
        AC_SUBST([PYTHON_CSPEC], [${az_python_cspec}])
        AC_MSG_NOTICE([PYTHON_CSPEC=${az_python_cspec}])
    fi
])



# AZ_PYTHON_INSIST( )
# -----------------
# Look for Python and set the output variable 'PYTHON'
# to 'python' if found, empty otherwise.

AC_DEFUN([AZ_PYTHON_PATH],
[
    AC_ARG_VAR( [PYTHON], [Python Executable Path] )
    if test -z "$PYTHON"
    then
        AC_MSG_ERROR([Python Executable not found])
    fi
])



# AZ_PYTHON_LSPEC( )
# -----------------
# Set up the linker options to link Python embedded
# programs/libraries in $PYTHON_LSPEC if $PYTHON
# has been defined.

AC_DEFUN([AZ_PYTHON_LSPEC],
[
    AC_ARG_VAR( [PYTHON], [Python Executable Path] )
    if test -n "$PYTHON"
    then
        AZ_PYTHON_RUN([
import sys
import distutils.sysconfig
strUseFrameWork = "--enable-framework"
dictConfig = distutils.sysconfig.get_config_vars( )
strConfigArgs = dictConfig.get("CONFIG_ARGS")
strLinkSpec =  dictConfig.get('LDFLAGS')
if -1 ==  strConfigArgs.find(strUseFrameWork):
    strLibPL = dictConfig.get("LIBPL")
    if strLibPL and (strLibPL != ""):
        strLinkSpec += " -L%s" % (strLibPL)
    strSys = dictConfig.get("SYSLIBS")
    if strSys and (strSys != ""):
        strLinkSpec += " %s" % (strSys)
    strSHL = dictConfig.get("SHLIBS")
    if strSHL and (strSHL != ""):
        strLinkSpec += " %s" % (strSHL)
    # Construct the Python Library Name.
    strTmplte = " -lpython%d.%d"
    if (sys.platform == "win32") or (sys.platform == "os2emx"):
        strTmplte = " -lpython%d%d"
    strWrk = strTmplte % ( (sys.hexversion >> 24),
                            ((sys.hexversion >> 16) & 0xff))
    strLinkSpec += strWrk
else:
    # This is not ideal since it changes the search path
    # for Frameworks which could have side-effects on
    # other included Frameworks.  However, it is necessary
    # where someone has installed more than one frameworked
    # Python.  Frameworks are really only used in MacOSX.
    strLibFW = dictConfig.get("PYTHONFRAMEWORKPREFIX")
    if strLibFW and (strLibFW != ""):
        strLinkSpec += " -F%s" % (strLibFW)
strLinkSpec += " %s" % (dictConfig.get('LINKFORSHARED'))
print strLinkSpec
        ])
        AC_SUBST([PYTHON_LSPEC], [${az_python_output}])
        AC_MSG_NOTICE([PYTHON_LSPEC=${az_python_output}])
    fi
])



# AZ_PYTHON_PATH( )
# -----------------
# Look for Python and set the output variable 'PYTHON'
# to 'python' if found, empty otherwise.

AC_DEFUN([AZ_PYTHON_PATH],
[
    AC_ARG_VAR( [PYTHON], [Python Executable Path] )
    AC_PATH_PROG( PYTHON, python, [], $1 )
    if test -z "$PYTHON"
    then
        AC_MSG_ERROR([Python Executable not found])
    else
        az_python_use=true
    fi
    AM_CONDITIONAL(PYTHON_USE, test "$az_python_use" = "true")
])



# AZ_PYTHON_PREFIX( )
# -------------------
# Use the values of $prefix and $exec_prefix for the corresponding
# values of PYTHON_PREFIX and PYTHON_EXEC_PREFIX.

AC_DEFUN([AZ_PYTHON_PREFIX],
[
    if test -z "$PYTHON"
    then
        AC_MSG_ERROR([Python Executable Path is not known])
    fi
    ax_python_prefix=`${PYTHON} -c "import sys; print sys.prefix"`
    ax_python_execprefix=`${PYTHON} -c "import sys; print sys.exec_prefix"`
    AC_SUBST([PYTHON_PREFIX], ["${ax_python_prefix}"])
    AC_SUBST([PYTHON_EXECPREFIX], ["${ax_python_execprefix}"])
])



# AZ_PYTHON_RUN( PYTHON_PROGRAM )
# -----------------
# Run a Python Test Program saving its output
# in az_python_output and its condition code
# in az_python_cc.

AC_DEFUN([AZ_PYTHON_RUN],
[
    AC_ARG_VAR( [PYTHON], [Python Executable Path] )
    if test -z "$PYTHON"
    then
        AC_MSG_ERROR([Python Executable not found])
    else
        cat >conftest.py <<_ACEOF
$1
_ACEOF
        az_python_output=`$PYTHON conftest.py`
        az_python_cc=$?
        rm conftest.py
        if test -f "conftest.pyc"
        then
            rm conftest.pyc
        fi
    fi
])



# AZ_PYTHON_VERSION_CHECK( VERSION, [ACTION-IF-TRUE], [ACTION-IF-FALSE] )
# -----------------------------------------------------------------------------
# Run ACTION-IF-TRUE if the Python interpreter has version >= VERSION.
# Run ACTION-IF-FALSE otherwise.
# This test uses sys.hexversion instead of the string equivalant (first
# word of sys.version), in order to cope with versions such as 2.2c1.
# hexversion has been introduced in Python 1.5.2; it's probably not
# worth to support older versions (1.5.1 was released on October 31, 1998).

AC_DEFUN([AZ_PYTHON_VERSION_CHECK],
 [
    AC_ARG_VAR( [PYTHON], [Python Executable Path] )
    if test -n "$PYTHON"
    then
        AC_MSG_CHECKING([whether $PYTHON version >= $1])
        AZ_PYTHON_RUN([
import sys, string
# split strings by '.' and convert to numeric.  Append some zeros
# because we need at least 4 digits for the hex conversion.
minver = map(int, string.split('$1', '.')) + [[0, 0, 0]]
minverhex = 0
for i in xrange(0, 4): minverhex = (minverhex << 8) + minver[[i]]
if sys.hexversion >= minverhex:
    sys.exit( 0 )
else:
    sys.exit( 1 )
        ])
        if test $az_python_cc -eq 0
        then
            $2
        m4_ifvaln(
            [$3],
            [else $3]
        )
        fi
    fi
])



# AZ_PYTHON_VERSION_ENSURE( VERSION )
# -----------------
# Insure that the Python Interpreter Version
# is greater than or equal to the VERSION
# parameter.

AC_DEFUN([AZ_PYTHON_VERSION_ENSURE],
[
    AZ_PYTHON_VERSION_CHECK(
        [$1],
        [AC_MSG_RESULT(yes)],
        [AC_MSG_ERROR(too old)]
    )
])



# AZ_PYTHON_WITH( [path] )
# -----------------------------------------------------------------
# Handles the various --with-python commands.
# Input:
#   $1 is the optional search path for the python executable if needed
# Ouput:
#   PYTHON_USE (AM_CONDITIONAL) is true if python executable found
#   and --with-python was requested; otherwise false.
#   $PYTHON contains the full executable path to python if PYTHON_USE
#   is true.
#
# Example:
#   AZ_PYTHON_WITH( )
#   or
#   AZ_PYTHON_WITH("/usr/bin")

AC_DEFUN([AZ_PYTHON_WITH],
[
    AC_ARG_VAR([PYTHON],[Python Executable Path])

    # unless PYTHON was supplied to us (as a precious variable),
    # see if --with-python[=PythonExecutablePath], --with-python,
    # --without-python or --with-python=no was given.
    if test -z "$PYTHON"
    then
        AC_MSG_CHECKING(for --with-python)
        AC_ARG_WITH(
            python,
            AC_HELP_STRING([--with-python@<:@=PYTHON@:>@],
                [absolute path name of Python executable]
            ),
            [
                if test "$withval" = "yes"
                then
                    # "yes" was specified, but we don't have a path
                    # for the executable.
                    # So, let's searth the PATH Environment Variable.
                    AC_MSG_RESULT(yes)
                    AC_PATH_PROG(
                        [PYTHON],
                        python,
                        [],
                        $1
                    )
                    if test -z "$PYTHON"
                    then
                        AC_MSG_ERROR(no path to python found)
                    fi
                    az_python_use=true
                    AM_CONDITIONAL(PYTHON_USE, test x"$az_python_use" = x"true")
                    AZ_PYTHON_PREFIX( )
                elif test "$withval" = "no"
                then
                    AC_MSG_RESULT(no)
                    az_python_use=false
                    AM_CONDITIONAL(PYTHON_USE, test x"$az_python_use" = x"true")
                else
                    # $withval must be the executable path then.
                    AC_SUBST([PYTHON], ["${withval}"])
                    AC_MSG_RESULT($withval)
                    az_python_use=true
                    AM_CONDITIONAL(PYTHON_USE, test x"$az_python_use" = x"true")
                    AZ_PYTHON_PREFIX( )
                fi
            ],
            [
                # --with-python was not specified.
                AC_MSG_RESULT(no)
                az_python_use=false
                AM_CONDITIONAL(PYTHON_USE, test x"$az_python_use" = x"true")
            ]
        )
    fi

])

dnl @synopsis AC_PYTHON_DEVEL
dnl
dnl Checks for Python and tries to get the include path to 'Python.h'.
dnl It provides the $(PYTHON_CPPFLAGS) and $(PYTHON_LDFLAGS) output
dnl variable.
dnl
dnl @category InstalledPackages
dnl @author Sebastian Huber <sebastian-huber@web.de>
dnl @author Alan W. Irwin <irwin@beluga.phys.uvic.ca>
dnl @author Rafael Laboissiere <laboissiere@psy.mpg.de>
dnl @author Andrew Collier <colliera@nu.ac.za>
dnl @version 2004-07-14
dnl @license GPLWithACException

AC_DEFUN([AC_PYTHON_DEVEL],[
	#
	# should allow for checking of python version here...
	#
	AC_REQUIRE([AM_PATH_PYTHON])

	# Check for Python include path
	AC_MSG_CHECKING([for Python include path])
	python_path=`echo $PYTHON | sed "s,/bin.*$,,"`
	for i in "$python_path/include/python$PYTHON_VERSION/" "$python_path/include/python/" "$python_path/" ; do
		python_path=`find $i -type f -name Python.h -print | sed "1q"`
		if test -n "$python_path" ; then
			break
		fi
	done
	python_path=`echo $python_path | sed "s,/Python.h$,,"`
	AC_MSG_RESULT([$python_path])
	if test -z "$python_path" ; then
		AC_MSG_ERROR([cannot find Python include path])
	fi
	AC_SUBST([PYTHON_CPPFLAGS],[-I$python_path])

	# Check for Python library path
	AC_MSG_CHECKING([for Python library path])
	python_path=`echo $PYTHON | sed "s,/bin.*$,,"`
	for i in "$python_path/lib/python$PYTHON_VERSION/config/" "$python_path/lib/python$PYTHON_VERSION/" "$python_path/lib/python/config/" "$python_path/lib/python/" "$python_path/" ; do
		python_path=`find $i -type f -name libpython$PYTHON_VERSION.* -print | sed "1q"`
		if test -n "$python_path" ; then
			break
		fi
	done
	python_path=`echo $python_path | sed "s,/libpython.*$,,"`
	AC_MSG_RESULT([$python_path])
	if test -z "$python_path" ; then
		AC_MSG_ERROR([cannot find Python library path])
	fi
	AC_SUBST([PYTHON_LDFLAGS],["-L$python_path -lpython$PYTHON_VERSION"])
	#
	python_site=`echo $python_path | sed "s/config/site-packages/"`
	AC_SUBST([PYTHON_SITE_PKG],[$python_site])
	#
	# libraries which must be linked in when embedding
	#
	AC_MSG_CHECKING(python extra libraries)
	PYTHON_EXTRA_LIBS=`$PYTHON -c "import distutils.sysconfig; \
                conf = distutils.sysconfig.get_config_var; \
                print conf('LOCALMODLIBS')+' '+conf('LIBS')"
	AC_MSG_RESULT($PYTHON_EXTRA_LIBS)`
	AC_SUBST(PYTHON_EXTRA_LIBS)
])
dnl @synopsis AC_PROG_SWIG([major.minor.micro])
dnl
dnl This macro searches for a SWIG installation on your system. If
dnl found you should call SWIG via $(SWIG). You can use the optional
dnl first argument to check if the version of the available SWIG is
dnl greater than or equal to the value of the argument. It should have
dnl the format: N[.N[.N]] (N is a number between 0 and 999. Only the
dnl first N is mandatory.)
dnl
dnl If the version argument is given (e.g. 1.3.17), AC_PROG_SWIG checks
dnl that the swig package is this version number or higher.
dnl
dnl In configure.in, use as:
dnl
dnl             AC_PROG_SWIG(1.3.17)
dnl             SWIG_ENABLE_CXX
dnl             SWIG_MULTI_MODULE_SUPPORT
dnl             SWIG_PYTHON
dnl
dnl @category InstalledPackages
dnl @author Sebastian Huber <sebastian-huber@web.de>
dnl @author Alan W. Irwin <irwin@beluga.phys.uvic.ca>
dnl @author Rafael Laboissiere <rafael@laboissiere.net>
dnl @author Andrew Collier <abcollier@yahoo.com>
dnl @version 2004-09-20
dnl @license GPLWithACException

AC_DEFUN([AC_PROG_SWIG],[
        AC_PATH_PROG([SWIG],[swig])
        if test -z "$SWIG" ; then
                AC_MSG_WARN([cannot find 'swig' program. You should look at http://www.swig.org])
                SWIG='echo "Error: SWIG is not installed. You should look at http://www.swig.org" ; false'
        elif test -n "$1" ; then
                AC_MSG_CHECKING([for SWIG version])
                [swig_version=`$SWIG -version 2>&1 | grep 'SWIG Version' | sed 's/.*\([0-9][0-9]*\.[0-9][0-9]*\.[0-9][0-9]*\).*/\1/g'`]
                AC_MSG_RESULT([$swig_version])
                if test -n "$swig_version" ; then
                        # Calculate the required version number components
                        [required=$1]
                        [required_major=`echo $required | sed 's/[^0-9].*//'`]
                        if test -z "$required_major" ; then
                                [required_major=0]
                        fi
                        [required=`echo $required | sed 's/[0-9]*[^0-9]//'`]
                        [required_minor=`echo $required | sed 's/[^0-9].*//'`]
                        if test -z "$required_minor" ; then
                                [required_minor=0]
                        fi
                        [required=`echo $required | sed 's/[0-9]*[^0-9]//'`]
                        [required_patch=`echo $required | sed 's/[^0-9].*//'`]
                        if test -z "$required_patch" ; then
                                [required_patch=0]
                        fi
                        # Calculate the available version number components
                        [available=$swig_version]
                        [available_major=`echo $available | sed 's/[^0-9].*//'`]
                        if test -z "$available_major" ; then
                                [available_major=0]
                        fi
                        [available=`echo $available | sed 's/[0-9]*[^0-9]//'`]
                        [available_minor=`echo $available | sed 's/[^0-9].*//'`]
                        if test -z "$available_minor" ; then
                                [available_minor=0]
                        fi
                        [available=`echo $available | sed 's/[0-9]*[^0-9]//'`]
                        [available_patch=`echo $available | sed 's/[^0-9].*//'`]
                        if test -z "$available_patch" ; then
                                [available_patch=0]
                        fi
                        if test $available_major -ne $required_major \
                                -o $available_minor -ne $required_minor \
                                -o $available_patch -lt $required_patch ; then
                                AC_MSG_WARN([SWIG version >= $1 is required.  You have $swig_version.  You should look at http://www.swig.org])
                                SWIG='echo "Error: SWIG version >= $1 is required.  You have '"$swig_version"'.  You should look at http://www.swig.org" ; false'
                        else
                                AC_MSG_NOTICE([SWIG executable is '$SWIG'])
                                SWIG_LIB=`$SWIG -swiglib`
                                AC_MSG_NOTICE([SWIG library directory is '$SWIG_LIB'])
                        fi
                else
                        AC_MSG_WARN([cannot determine SWIG version])
                        SWIG='echo "Error: Cannot determine SWIG version.  You should look at http://www.swig.org" ; false'
                fi
        fi
        AC_SUBST([SWIG_LIB])
])

# SWIG_ENABLE_CXX()
#
# Enable SWIG C++ support.  This affects all invocations of $(SWIG).
AC_DEFUN([SWIG_ENABLE_CXX],[
        AC_REQUIRE([AC_PROG_SWIG])
        AC_REQUIRE([AC_PROG_CXX])
        SWIG="$SWIG -c++"
])

# SWIG_MULTI_MODULE_SUPPORT()
#
# Enable support for multiple modules.  This effects all invocations
# of $(SWIG).  You have to link all generated modules against the
# appropriate SWIG runtime library.  If you want to build Python
# modules for example, use the SWIG_PYTHON() macro and link the
# modules against $(SWIG_PYTHON_LIBS).
#
AC_DEFUN([SWIG_MULTI_MODULE_SUPPORT],[
        AC_REQUIRE([AC_PROG_SWIG])
        SWIG="$SWIG -noruntime"
])

# SWIG_PYTHON([use-shadow-classes = {no, yes}])
#
# Checks for Python and provides the $(SWIG_PYTHON_CPPFLAGS),
# and $(SWIG_PYTHON_OPT) output variables.
#
# $(SWIG_PYTHON_OPT) contains all necessary SWIG options to generate
# code for Python.  Shadow classes are enabled unless the value of the
# optional first argument is exactly 'no'.  If you need multi module
# support (provided by the SWIG_MULTI_MODULE_SUPPORT() macro) use
# $(SWIG_PYTHON_LIBS) to link against the appropriate library.  It
# contains the SWIG Python runtime library that is needed by the type
# check system for example.
AC_DEFUN([SWIG_PYTHON],[
        AC_REQUIRE([AC_PROG_SWIG])
        AC_REQUIRE([AC_PYTHON_DEVEL])
        test "x$1" != "xno" || swig_shadow=" -noproxy"
        AC_SUBST([SWIG_PYTHON_OPT],[-python$swig_shadow])
        AC_SUBST([SWIG_PYTHON_CPPFLAGS],[$PYTHON_CPPFLAGS])
])


dnl @synopsis AC_LIB_WAD
dnl
dnl This macro searches for installed WAD library.
dnl
AC_DEFUN([AC_LIB_WAD],
[
        AC_REQUIRE([AC_PYTHON_DEVEL])
        AC_ARG_ENABLE(wad,
        AC_HELP_STRING([--enable-wad], [enable wad module]),
        [
                case "${enableval}" in
                        no)     ;;
                        *)      if test "x${enableval}" = xyes;
                                then
                                        check_wad="yes"
                                fi ;;
                esac
        ], [])

        if test -n "$check_wad";
        then
                AC_CHECK_LIB(wadpy, _init, [WADPY=-lwadpy], [], $PYTHON_LDFLAGS $PYTHON_EXTRA_LIBS)
                AC_SUBST(WADPY)
        fi
])
