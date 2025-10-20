CXX:=c++
# Allow env overrides for imlib2 flags; otherwise try pkg-config; fallback to common include
IMLIB2_CFLAGS?=$(shell pkg-config --cflags imlib2 2>/dev/null)
IMLIB2_LIBS?=$(shell pkg-config --libs imlib2 2>/dev/null)
DEFAULT_IMLIB2_CFLAGS:=-I/usr/include/Imlib2
CXXFLAGS:=-fPIC -O2 -Wall -Wextra -std=c++17 $(if $(IMLIB2_CFLAGS),$(IMLIB2_CFLAGS),$(DEFAULT_IMLIB2_CFLAGS)) -I../third_party
LDFLAGS:=-shared $(IMLIB2_LIBS)

# Optional libhefraw integration
LIBHEFRAW_PATH?=../decoder/libhefraw.a
HEFRAW_CFLAGS:=-I../decoder/include
HEFRAW_LIBS:=-L../decoder -lhefraw

.PHONY: all clean

all: loader_nef.so

loader_nef.so: loader_nef.cpp
	${CXX} ${CXXFLAGS} ${HEFRAW_CFLAGS} -o $@ $< ${LDFLAGS} ${HEFRAW_LIBS}

clean:
	rm -f loader_nef.so


