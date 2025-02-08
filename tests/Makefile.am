# Common flags
AM_CPPFLAGS = -I$(top_srcdir)/src
AM_LDADD = -lcriterion

# List of tests
TESTS = test_expansion test_lexer
check_PROGRAMS = $(TESTS)

# Sources and libraries for each test
test_lexer_SOURCES = test_lexer.c
test_lexer_LDADD = $(AM_LDADD) $(top_builddir)/src/lexer/liblexer.a

test_expansion_SOURCES = test_expansion.c
test_expansion_LDADD = $(AM_LDADD) $(top_builddir)/src/expansion/libexpansion.a \
					   $(top_builddir)/src/internal_state/libinternal_state.a \
					   $(top_builddir)/src/dict/libdict.a \
					   $(top_builddir)/src/lexer/liblexer.a \
					   $(top_builddir)/src/io_backend/libio_backend.a \
					   $(top_builddir)/src/ast/libast.a \
					   $(top_builddir)/src/builtins/libbuiltins.a \
					   $(top_builddir)/src/parser/libparser.a


# Ensure make clean removes test binaries
CLEANFILES = $(check_PROGRAMS) *.log

# Run additional script during `make check`
check-local:
	./step_1.sh
	./step_2.sh
	./step_3.sh
	./step_4.sh
	
