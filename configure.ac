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
m4_include([m4/check_lib_warped.m4])
CHECK_LIB_WARPED

m4_include([m4/check_lib_tcmalloc.m4])
CHECK_LIB_TCMALLOC

CXXFLAGS="$CXXFLAGS -Wall -Wextra -pedantic -Werror"

AC_CONFIG_FILES([Makefile])
AC_CONFIG_FILES([deps/Makefile])
AC_CONFIG_FILES([deps/re2/Makefile])
AC_CONFIG_FILES([deps/ppm/Makefile])
AC_CONFIG_FILES([models/Makefile])
AC_CONFIG_FILES([models/phold/Makefile])
AC_CONFIG_FILES([models/pcs/Makefile])
AC_CONFIG_FILES([models/airport/Makefile])
AC_CONFIG_FILES([models/epidemic/Makefile])
AC_CONFIG_FILES([models/traffic/Makefile])
AC_CONFIG_FILES([models/volcano/Makefile])
AC_CONFIG_FILES([models/wildfire/Makefile])
AC_CONFIG_FILES([models/sandpile/Makefile])
AC_CONFIG_FILES([models/synthetic/Makefile])

AC_OUTPUT
