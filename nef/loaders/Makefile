CC:=cc
# Allow env overrides for imlib2 flags; otherwise try pkg-config; fallback to common include
IMLIB2_CFLAGS?=$(shell pkg-config --cflags imlib2 2>/dev/null)
IMLIB2_LIBS?=$(shell pkg-config --libs imlib2 2>/dev/null)
DEFAULT_IMLIB2_CFLAGS:=-I/usr/include/Imlib2
CFLAGS:=-fPIC -O2 -Wall -Wextra $(if $(IMLIB2_CFLAGS),$(IMLIB2_CFLAGS),$(DEFAULT_IMLIB2_CFLAGS)) -I../third_party
LDFLAGS:=-shared $(IMLIB2_LIBS)

.PHONY: all clean

all: loader_nef.so

loader_nef.so: loader_nef.c
	${CC} ${CFLAGS} -o $@ $< ${LDFLAGS}

clean:
	rm -f loader_nef.so


