dnl Check for warped

dnl Usage: CHECK_LIB_TCMALLOC

AC_DEFUN([CHECK_LIB_TCMALLOC],
[
    TCMALLOC_PATH="/usr"

    dnl This allows the user to override default include directory
    AC_ARG_WITH([tcmalloc],
                [AS_HELP_STRING([--with-tcmalloc(=/path/to/library)], [location of tcmalloc. Do not include /lib or /include])],
                [
                    AS_IF([test "x$withval" = xyes],
                          [],
                          [TCMALLOC_PATH=${withval}])

                    CPPFLAGS="$CPPFLAGS -isystem$TCMALLOC_PATH/include";
                    LDFLAGS="$LDFLAGS -L$TCMALLOC_PATH/lib";
                    want_tcmalloc="yes";
                ],
                [])


    AS_IF([test "x$want_tcmalloc" = xyes],
          [AC_CHECK_LIB([tcmalloc],
                        [malloc],
                        [],
                        [AC_MSG_ERROR([Could not find tcmalloc.so. Use --with-tcmalloc to specify install path])],
                        [])
          ],
          [])

    AC_ARG_WITH([tcmalloc-minimal],
                [AS_HELP_STRING([--with-tcmalloc-minimal(=/path/to/library)], [location of minimal tcmalloc. Do not include /lib or /include])],
                [
                    AS_IF([test "x$withval" = xyes],
                          [],
                          [TCMALLOC_PATH=${withval}])

                    CPPFLAGS="$CPPFLAGS -isystem$TCMALLOC_PATH/include";
                    LDFLAGS="$LDFLAGS -L$TCMALLOC_PATH/lib";
                    want_tcmalloc_minimal="yes";
                ],
                [])

    AS_IF([test "x$want_tcmalloc_minimal" = xyes],
          [AC_CHECK_LIB([tcmalloc_minimal],
                 [malloc],
                 [],
                 [AC_MSG_ERROR([Could not find tcmalloc_minimal.so. Use --with-tcmalloc-minimal to specify install path])],
                 [])
          ],
          [])

]) dnl end CHECK_LIB_TCMALLOC

