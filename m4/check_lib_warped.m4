dnl Check for warped

dnl Usage: CHECK_LIB_WARPED

AC_DEFUN([CHECK_LIB_WARPED],
[
    dnl This allows the user to override default include directory
    AC_ARG_WITH([warped],
                [AS_HELP_STRING([--with-warped(=/path/to/library)], [location of warped. Do not include /lib or /include])],
                [CPPFLAGS="$CPPFLAGS -isystem$withval/include"; LDFLAGS="$LDFLAGS -L$withval/lib";],
                [])

    AC_CHECK_HEADER([warped.hpp],
                    [AC_MSG_RESULT([Successfully found warped.hpp.])],
                    [AC_MSG_ERROR([Could not find warped.hpp. Use --with-warped to specify install path])])

    AC_CHECK_LIB([warped],
                 [warped_is_present],
                 [],
                 [AC_MSG_ERROR([Could not find libwarped.so. Use --with-warped to specify install path])],
                 [])

]) dnl end CHECK_LIB_WARPED

