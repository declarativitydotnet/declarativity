dnl
dnl Find where the libasync and headers are installed
dnl
AC_DEFUN(P2_SFSLIB,
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

AC_DEFUN(P2_SFSINC,
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
dnl Find where the libpcap and headers are installed
dnl
AC_DEFUN(P2_PCAPLIB,
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


AC_DEFUN(P2_PCAPINC,
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
