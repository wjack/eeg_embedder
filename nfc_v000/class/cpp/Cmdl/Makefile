# file: $(NEDC_NFC)/class/cpp/Cmdl/Makefile
#

# define the object files (this must go first)
# 
OBJ = cmdl_00.o cmdl_01.o cmdl_02.o cmdl_03.o

# define a dummy target (this must go next)
#
all: $(OBJ)

# define compilation flags
#
CFLAGS += -O2 -c
#CFLAGS += -g -c

# define dependencies
#
DEPS = Cmdl.h ./Makefile

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
	cp Cmdl.h ../../../include/
	ar rvs ../../../lib/libdsp.a cmdl_??.o 
	ranlib ../../../lib/libdsp.a

# define a target to clean the directory
#
clean:
	rm -f cmdl_??.o

#
# end of file
