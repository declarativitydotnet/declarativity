AC_DEFUN([MH_CXX_HEADER_TOOLS],
[
AC_REQUIRE([AC_PROG_CXX])
AC_MSG_CHECKING(whether tools/Tools.h header exists)
AC_CACHE_VAL(mh_cv_have_header_tools_h,
[
AC_LANG_SAVE
AC_LANG_CPLUSPLUS
AC_TRY_COMPILE([#include <tools/Tools.h>], [Tools::PropertySet p;],
mh_cv_have_header_tools_h=yes,
mh_cv_have_header_tools_h=no)
AC_LANG_RESTORE
])
AC_MSG_RESULT($mh_cv_have_header_tools_h)
if test "$mh_cv_have_header_tools_h" = yes; then
AC_DEFINE(HAVE_TOOLS_H)
else
AC_MSG_FAILURE([cannot find tools/Tools.h. You need to install the Tools library first!])
fi
])
