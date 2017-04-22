# file: $(NEDC_NFC)/util/cpp/nedc_print_signal/Makefile
#

# define compilation flags
#
CFLAGS += -O2
#CFLAGS += -g

# define source and object files
#
SRC = nedc_print_signal.cc
OBJ = nedc_print_signal.o

# define dependencies
#
DEPS = ../../../class/cpp/Edf/Edf.h \
	 ../../../lib/libdsp.a

# define include files
#
INCLUDES = -I../../../include/

# define a target for the application
#
all: nedc_print_signal

# define a target to link the application
#
nedc_print_signal: $(OBJ) $(DEPS)
	g++ -I../../../include/ $(CFLAGS) -o nedc_print_signal \
	nedc_print_signal.o \
	-L../../../lib -ldsp \
	-lm

# define a target to compile the application
#
nedc_print_signal.o: $(SRC) $(DEPS)
	g++ $(CFLAGS) -c $(SRC) $(INCLUDES) -o $(OBJ)

# define an installation target
#
install:
	cp nedc_print_signal ../../../bin/

# define a target to clean the directory
#
clean:
	rm -f nedc_print_signal nedc_print_signal.o

#
# end of file
