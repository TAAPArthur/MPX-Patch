CC := gcc
CPPFLAGS ?= -D_XOPEN_SOURCE=500
CFLAGS ?= -std=c99 -pedantic -Wall -Wextra -Os -Wno-unused-parameter
LDFLAGS ?= -s -lX11 -lXtst -lXi

HEADERS := MPXPatch.h

all: mpx-patch

mpx-patch: ${HEADERS} MPXPatch.c
	${CC} -o $@ ${CPPFLAGS} ${CFLAGS}  MPXPatch.c ${LDFLAGS}



.PHONY: all clean check install
