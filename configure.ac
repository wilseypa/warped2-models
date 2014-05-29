AC_INIT([warped-models2],[1.0])

AC_CONFIG_MACRO_DIR([m4])
AM_INIT_AUTOMAKE([1.11 foreign subdir-objects])
AM_SILENT_RULES([yes])
AC_PREREQ([2.61])

AC_LANG([C++])

dnl Checks for programs.
LT_INIT([disable-shared])
AC_PROG_CXX

m4_include([m4/ax_cxx_compile_stdcxx_11.m4])
AX_CXX_COMPILE_STDCXX_11([noext], [mandatory])

dnl Check for libwarped
AC_CACHE_CHECK([for location of libwarped], [cl_cv_lib_warped],
  [AC_ARG_WITH(warped,
    AS_HELP_STRING([--with-warped],
      [location of libwarped (default is /usr)]),
      [
        case "${withval}" in
          yes) cl_cv_lib_warped=/usr ;;
          no) cl_cv_lib_warped=no ;;
          *) cl_cv_lib_warped=$withval ;;
        esac
      ],
      [cl_cv_lib_warped=/usr]
  )]
)
if test $cl_cv_lib_warped = no ; then
AC_MSG_ERROR(The warped package is required.)
fi
if test $cl_cv_lib_warped != /usr ; then
LDFLAGS="$LDFLAGS -L$cl_cv_lib_warped/lib -lwarped"
CPPFLAGS="$CPPFLAGS -isystem$cl_cv_lib_warped/include"
fi

CXXFLAGS="$CXXFLAGS -Wall -Wextra -pedantic -Werror"

AC_CONFIG_FILES([Makefile])
AC_CONFIG_FILES([models/Makefile])
AC_CONFIG_FILES([models/ping_pong/Makefile])

AC_OUTPUT