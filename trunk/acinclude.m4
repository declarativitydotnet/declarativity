dnl
dnl Find where openssh is
dnl
AC_DEFUN([P2_OPENSSLLIB],
[AC_ARG_WITH([openssllib],
	     AC_HELP_STRING([--with-openssllib],	
	                    [specify dir containing libssl (default is /usr/lib/)]),
	     [p2_openssllib=$withval], [p2_openssllib=/usr/lib/])
AC_CACHE_CHECK([whether to use openssllib],
	       [p2_openssllib], 
	       [p2_openssllib=/usr/lib/])
AC_SUBST(OPENSSL_LIB)
unset OPENSSL_LIB

p2_openssllib=`echo $p2_openssllib | sed -e 's!/$!!'`

if test -d "$p2_openssllib"; then
    if /bin/ls $p2_openssllib | egrep '^libssl.*\.(la|so|a|dylib)$' >/dev/null 2>&1; then
	OPENSSL_LIB="-L${p2_openssllib} -lssl -lcrypto"
	AC_MSG_RESULT([using OpenSSL library from $p2_openssllib])
    else
	AC_MSG_ERROR([Could not find OpenSSL library in $p2_openssllib])
    fi
else
    AC_MSG_ERROR([Could not find OpenSSL library directory $p2_openssllib])
fi
])


AC_DEFUN([P2_OPENSSLINC],
[AC_ARG_WITH([opensslinc],
	     AC_HELP_STRING([--with-opensslinc],	
	                    [specify location for OpenSSL's 'openssl' include directory (default is /usr/include)]),
	     [p2_opensslinc=$withval], [p2_opensslinc=/usr/include])
AC_CACHE_CHECK([whether to use opensslinc],
	       [p2_opensslinc], 
	       [p2_opensslinc=/usr/include])
AC_SUBST(OPENSSL_INC)
unset OPENSSL_INC

p2_opensslinc=`echo $p2_opensslinc | sed -e 's!/$!!'`

if test -d "$p2_opensslinc"; then
    if /bin/ls $p2_opensslinc | grep '^openssl$' >/dev/null 2>&1; then
	OPENSSL_INC="-I${p2_opensslinc}"
	AC_MSG_RESULT([using OpenSSL headers from $p2_opensslinc])
    else
	AC_MSG_ERROR([Could not find OpenSSL headers in $p2_opensslinc])
    fi
else
    AC_MSG_ERROR([Could not find OpenSSL include directory in $p2_opensslinc])
fi

])




dnl
dnl Find where the boost is installed
dnl
AC_DEFUN([P2_BOOSTLIB],
[AC_ARG_WITH([boostlib],
	     AC_HELP_STRING([--with-boostlib],	
	                    [specify dir for boost libs (default is /usr/lib)]),
	     [p2_boostlib=$withval], [p2_boostlib=/usr/lib])
AC_CACHE_CHECK([whether to use boostlib],
	       [p2_boostlib], 
	       [p2_boostlib=/usr/lib])
AC_SUBST(BOOST_LIB)
unset BOOST_LIB

p2_boostlib=`echo $p2_boostlib | sed -e 's!/$!!'`

if test -d "$p2_boostlib"; then
    if ls $p2_boostlib | egrep 'boost_regex.*\.(la|so|a)$' >/dev/null 2>&1; then
	BOOST_LIB="-L${p2_boostlib} -lboost_regex -lboost_date_time"
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
	                    [specify dir for boost include (default is /usr/include)]),
	     [p2_boostinc=$withval], [p2_boostinc=/usr/include])
AC_CACHE_CHECK([whether to use boostinc],
	       [p2_boostinc], 
	       [p2_boostinc=/usr/include])
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
    if ls $p2_pcaplib | egrep 'libpcap.*\.(la|so|a)$' >/dev/null 2>&1; then
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
# LINKFORSHARED is broken in OS X 10.4, so we'll do it explicitly
#    strLinkSpec += " %s" % (dictConfig.get("LINKFORSHARED"))
    strLinkSpec += " -u __dummy -u _PyMac_Error -framework System -framework Python -framework CoreServices -framework Foundation"
print strLinkSpec
        ])
        AC_SUBST([PYTHON_LSPEC], [${az_python_output}])
        AC_MSG_NOTICE([PYTHON_LSPEC=${az_python_output}])
    fi
]
)



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

dnl
dnl Find out whether we have sockaddr_in.sin_len
dnl
AC_DEFUN([AC_CHECK_SOCKADDR_IN_SIN_LEN],
[
	AC_CHECK_MEMBERS([struct sockaddr_in.sin_len])
])


dnl Try to get dynamic linking working right on OS X
DLOPEN="-dlopen"
AC_SUBST(DLOPEN)
DLPREOPEN="-dlpreopen"
AC_SUBST(DLPREOPEN)
