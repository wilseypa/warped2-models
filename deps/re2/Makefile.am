LIBRE2_H_FILES= \
    re2/filtered_re2.h \
    re2/re2.h \
    re2/set.h \
    re2/stringpiece.h \
    re2/variadic_function.h \
    util/arena.h \
    util/atomicops.h \
    util/benchmark.h \
    util/flags.h \
    util/logging.h \
    util/mutex.h \
    util/pcre.h \
    util/random.h \
    util/sparse_array.h \
    util/sparse_set.h \
    util/test.h \
    util/utf.h \
    util/util.h \
    util/valgrind.h \
    re2/filtered_re2.h \
    re2/prefilter.h \
    re2/prefilter_tree.h \
    re2/prog.h \
    re2/re2.h \
    re2/regexp.h \
    re2/set.h \
    re2/stringpiece.h \
    re2/unicode_casefold.h \
    re2/unicode_groups.h \
    re2/variadic_function.h \
    re2/walker-inl.h

LIBRE2_CC_FILES = \
    re2/bitstate.cc \
    re2/compile.cc \
    re2/dfa.cc \
    re2/filtered_re2.cc \
    re2/mimics_pcre.cc \
    re2/nfa.cc \
    re2/onepass.cc \
    re2/parse.cc \
    re2/perl_groups.cc \
    re2/prefilter.cc \
    re2/prefilter_tree.cc \
    re2/prog.cc \
    re2/re2.cc \
    re2/regexp.cc \
    re2/set.cc \
    re2/simplify.cc \
    re2/tostring.cc \
    re2/unicode_casefold.cc \
    re2/unicode_groups.cc \
    util/arena.cc \
    util/benchmark.cc \
    util/hash.cc \
    util/pcre.cc \
    util/random.cc \
    util/rune.cc \
    util/stringpiece.cc \
    util/stringprintf.cc \
    util/strutil.cc \
    util/thread.cc \
    util/valgrind.cc

noinst_LTLIBRARIES = libre2.la

libre2_la_CPPFLAGS = -w -Wno-error -I$(top_srcdir)/deps/re2
libre2_la_LDFLAGS = -lpthread

libre2_la_SOURCES = \
    $(LIBRE2_CC_FILES) \
    $(LIBRE2_H_FILES)