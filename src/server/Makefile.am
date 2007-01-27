MAINTAINERCLEANFILES= Makefile.in

noinst_LIBRARIES = libflserver.a

bin_PROGRAMS = freelords_server

libflserver_a_SOURCES = Server.cpp Client.cpp \
                        ggzgameserver.cpp

freelords_server_SOURCES = main.cpp
freelords_server_LDADD = libflserver.a $(LIB_GGZDMOD)
freelords_server_LDFLAGS = $(GGZDMOD_LDFLAGS)

INCLUDES = $(LIBSIGC_CFLAGS)  \
           $(LIBGGZ_INCLUDES) \
           $(GGZDMOD_INCLUDES)

AM_CXXFLAGS = -Wall

