CXX:=c++
CXXFLAGS:=-std=c++17 -O2 -Wall -Wextra
INCLUDES:=-Iinclude

.PHONY: all clean

all: libhefraw.a

libhefraw.a: src/hef_bitstream.o src/hef_format.o src/hef_decode.o src/hef_export.o
	ar rcs $@ $^

src/hef_bitstream.o: src/hef_bitstream.cpp include/hef_bitstream.hpp
	${CXX} ${CXXFLAGS} ${INCLUDES} -c -o $@ $<

src/hef_format.o: src/hef_format.cpp include/hef_format.hpp
	${CXX} ${CXXFLAGS} ${INCLUDES} -c -o $@ $<

src/hef_decode.o: src/hef_decode.cpp include/hef_decode.hpp include/hef_bitstream.hpp include/hef_format.hpp
	${CXX} ${CXXFLAGS} ${INCLUDES} -c -o $@ $<

src/hef_export.o: src/hef_export.cpp include/hef_export.hpp
	${CXX} ${CXXFLAGS} ${INCLUDES} -c -o $@ $<

clean:
	rm -f src/*.o libhefraw.a


