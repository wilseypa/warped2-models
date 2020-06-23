bin_PROGRAMS = corona_sim

corona_sim_SOURCES = \
    graph.hpp \
    diffusion.hpp \
    corona.hpp \
    corona.cpp

corona_sim_CPPFLAGS = -I$(top_srcdir)/deps -I$(top_srcdir)/deps/boost/include -I$(top_srcdir)/deps/bz2filesupport

corona_sim_LDADD = $(top_srcdir)/deps/ppm/libppm.la $(top_srcdir)/deps/bz2filesupport/libbz2support.a $(top_srcdir)/deps/boost/lib/libboost_iostreams.a -lbz2
