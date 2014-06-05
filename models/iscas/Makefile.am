bin_PROGRAMS = iscas_sim

iscas_sim_SOURCES = \
    Component.hpp \
    DFlipFlop.cpp \
    DFlipFlop.hpp \
    events.cpp \
    events.hpp \
    InputObject.cpp \
    InputObject.hpp \
    LogicGate.cpp \
    LogicGate.hpp \
    main.cpp

iscas_sim_CPPFLAGS = -isystem$(top_srcdir)/deps/re2 -I$(top_srcdir)/deps
iscas_sim_LDADD=$(top_builddir)/deps/re2/libre2.la
