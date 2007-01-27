MAINTAINERCLEANFILES= Makefile.in
CLEANFILES = *~
DEPENDSON = ${top_builddir}/src/string_tokenizer.o

noinst_PROGRAMS = extractv
extractv_SOURCES = extractv.cpp
extractv_LDADD = $(DEPENDSON) 

AM_CXXFLAGS = -Wall
INCLUDES = -I${srcdir}/../

${top_builddir}/src/string_tokenizer.o: ${srcdir}/../string_tokenizer.cpp
	${CXX} ${CPPFLAGS} -c -o ${top_builddir}/src/string_tokenizer.o ${srcdir}/../string_tokenizer.cpp

