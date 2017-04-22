# file: $(NEDC_NFC)/util/cpp/nedc_print_header/Makefile
#

# define compilation flags
#
CFLAGS += -O2
#CFLAGS += -g

# define source and object files
#
SRC = nedc_print_header.cc
OBJ = nedc_print_header.o

# define dependencies
#
DEPS = ../../../class/cpp/Edf/Edf.h \
	../../../lib/libdsp.a

# define include files
#
INCLUDES = -I../../../include/

# define a target for the application
#
all: nedc_print_header

# define a target to link the application
#
nedc_print_header: $(OBJ) $(DEPS)
	g++  -I../../../include/ $(CFLAGS) -o nedc_print_header \
	nedc_print_header.o \
	-L../../../lib -ldsp \
	-lm

# define a target to compile the application
#
nedc_print_header.o: $(SRC) $(DEPS)
	g++ $(CFLAGS) -c $(SRC) $(INCLUDES) -o $(OBJ)

# define an installation target
#
install:
	cp nedc_print_header ../../../bin/

# define a target to clean the directory
#
clean:
	rm -f nedc_print_header nedc_print_header.o

#
# end of file
