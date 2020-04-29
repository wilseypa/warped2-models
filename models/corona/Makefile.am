SUBDIRS = config

bin_PROGRAMS = corona_sim

corona_sim_SOURCES = \
    graph.hpp \
    diffusion.hpp \
    corona.hpp \
    corona.cpp

corona_sim_CPPFLAGS = -I$(top_srcdir)/deps

corona_sim_LDADD = $(top_srcdir)/deps/ppm/libppm.la
