CC := gcc
CFLAGS ?= -std=c99 -pedantic -Os
LDFLAGS ?= -s -lX11 -lXtst -lXi

HEADERS := MPXPatch.h

all: mpx-patch

mpx-patch: ${HEADERS} MPXPatch.c
	${CC} -o $@ ${CFLAGS}  MPXPatch.c ${LDFLAGS}



.PHONY: all clean check install
