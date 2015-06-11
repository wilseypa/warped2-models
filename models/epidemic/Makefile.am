bin_PROGRAMS = epidemic_sim

epidemic_sim_SOURCES = \
    Person.hpp \
    RandomNumGenerator.hpp \
    WattsStrogatzModel.hpp \
    DiffusionNetwork.hpp \
    DiseaseModel.hpp \
    epidemic.hpp \
    epidemic.cpp

epidemic_sim_CPPFLAGS = -isystem$(top_srcdir)/deps/rnd -I$(top_srcdir)/deps

epidemic_sim_LDADD=$(top_builddir)/deps/rnd/libRnd.la
