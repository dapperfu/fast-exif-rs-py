CC:=cc
CFLAGS:=-fPIC -O2 -Wall -Wextra -I/usr/include/imlib2
LDFLAGS:=-shared

.PHONY: all clean

all: loader_nef.so

loader_nef.so: loader_nef.c
	${CC} ${CFLAGS} -o $@ $< ${LDFLAGS}

clean:
	rm -f loader_nef.so


