SUBDIRS = config

bin_PROGRAMS = epidemic_sim

epidemic_sim_SOURCES = \
    Person.hpp \
    Graph.hpp \
    DiffusionNetwork.hpp \
    DiseaseModel.hpp \
    epidemic.hpp \
    epidemic.cpp

epidemic_sim_CPPFLAGS = -I$(top_srcdir)/deps

epidemic_sim_LDADD = $(top_srcdir)/deps/ppm/libppm.la
