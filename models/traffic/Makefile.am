bin_PROGRAMS = traffic_sim

traffic_sim_SOURCES = traffic.hpp traffic.cpp

traffic_sim_CPPFLAGS = -isystem$(top_srcdir)/deps/rnd -I$(top_srcdir)/deps

traffic_sim_LDADD=$(top_builddir)/deps/rnd/libRnd.la
