bin_PROGRAMS = pandemic_sim

pandemic_sim_SOURCES = \
    graph.hpp \
    diffusion.hpp \
    pandemic.hpp \
    pandemic.cpp

pandemic_sim_CPPFLAGS = -I$(top_srcdir)/deps

pandemic_sim_LDADD = $(top_srcdir)/deps/ppm/libppm.la
