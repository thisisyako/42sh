SUBDIRS = ast \
          parser \
          lexer \
          io_backend \
          builtins \
		  internal_state \
		  dict \
          expansion

bin_PROGRAMS = 42sh

42sh_SOURCES = 42sh.c

42sh_CPPFLAGS = -I$(top_srcdir)/src
42sh_CFLAGS = -std=c99 -Werror -Wall -Wextra -Wvla -pedantic
42sh_LDADD = internal_state/libinternal_state.a \
			 io_backend/libio_backend.a \
             ast/libast.a \
             parser/libparser.a \
             lexer/liblexer.a \
             builtins/libbuiltins.a \
			 dict/libdict.a \
             expansion/libexpansion.a
