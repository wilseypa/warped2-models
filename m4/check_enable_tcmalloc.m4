dnl Check for tcmalloc

dnl Usage: CHECK_ENABLE_TCMALLOC

AC_DEFUN([CHECK_ENABLE_TCMALLOC],
[
    AC_ARG_ENABLE([tcmalloc],
                  [AS_HELP_STRING([--enable-tcmalloc], [Enable tcmalloc_minimal])],
                  [tcmalloc=$enable_tcmalloc],
                  [tcmalloc="no"])

    AS_IF([test "x$tcmalloc" != "xno"],
          [LIBS="${LIBS} -ltcmalloc_minimal";],
          [AC_MSG_ERROR([Invalid value provided for --enable-tcmalloc])])

]) dnl end CHECK_ENABLE_TCMALLOC

