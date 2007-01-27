MAINTAINERCLEANFILES= Makefile.in

noinst_LIBRARIES = liblwserver.a

bin_PROGRAMS = lordsawar_server

liblwserver_a_SOURCES = Server.cpp Client.cpp \
                        ggzgameserver.cpp

lordsawar_server_SOURCES = main.cpp
lordsawar_server_LDADD = liblwserver.a $(LIB_GGZDMOD)
lordsawar_server_LDFLAGS = $(GGZDMOD_LDFLAGS)

INCLUDES = $(LIBSIGC_CFLAGS)  \
           $(LIBGGZ_INCLUDES) \
           $(GGZDMOD_INCLUDES)

AM_CXXFLAGS = -Wall

