lib_LIBRARIES = libbuiltins.a

libbuiltins_a_SOURCES = echo.c exit.c export.c cd.c unset.c dot.c builtins.h

libbuiltins_a_CPPFLAGS = -I$(top_srcdir)/src
libbuiltins_a_CFLAGS = -std=c99 -Werror -Wall -Wextra -Wvla -pedantic

noinst_LIBRARIES = libbuiltins.a
