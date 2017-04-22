# file: $(NEDC_NFC)/class/cpp/Edf/Makefile
#

# define the object files (this must go first)
# 
OBJ = edf_00.o edf_01.o edf_02.o edf_03.o edf_04.o edf_05.o

# define a dummy target (this must go next)
#
all: $(OBJ)

# define compilation flags
#
CFLAGS += -O2 -c
#CFLAGS += -g -c

# define dependencies
#
DEPS = Edf.h ./Makefile

# define include files
#
INCLUDES = -I../../../include/

# define a dummy target
#
all: $(OBJ)

# define a target to make sure all source files are compiled
#
%.o: %.cc $(DEPS) 
	g++ $(CFLAGS) $(INCLUDES) -o $@ $< 

# define a special target to install the code
#
install:
	cp Edf.h ../../../include/
	ar rvs ../../../lib/libdsp.a edf_??.o 
	ranlib ../../../lib/libdsp.a

# define a target to clean the directory
#
clean:
	rm -f edf_??.o

#
# end of file
