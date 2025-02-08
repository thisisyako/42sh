AC_INIT([42sh], [1.0], [42sh@assistants.epita.fr])
AM_INIT_AUTOMAKE([subdir-objects foreign])
AM_SILENT_RULES([yes])

AM_PROG_AR
AC_PROG_RANLIB
AC_PROG_CC

# Test suite configuration
AC_CONFIG_TESTDIR([tests])
AC_CONFIG_FILES([tests/Makefile])

AC_CONFIG_FILES([
    Makefile
    src/Makefile
    src/internal_state/Makefile
    src/ast/Makefile
    src/parser/Makefile
    src/lexer/Makefile
    src/builtins/Makefile
    src/io_backend/Makefile
    src/expansion/Makefile
    src/dict/Makefile
])
AC_OUTPUT
