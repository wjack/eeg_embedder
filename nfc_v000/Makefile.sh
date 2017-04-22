#!/bin/sh
#
# file: $NEDC_NFC/Makefile.sh
#
# This file contains a simple script to build the NEDC foundation classes.
# It recurses through the directories and runs make. Note that this file
# does not create the necessary directories.
#

# define the number of threads
#
THREADS="-j6";

# remove the existing library:
#  this seems to be the safest thing to do, especially
#  when compilers and architectures change
#
echo "... removing libdsp.a ..."
rm lib/libdsp.a

#------------------------------------------------------------------------------
#
# build the class libraries
#
#------------------------------------------------------------------------------

# compile and install the cpp classes
#
cd class/cpp/Cmdl; make clean; make $THREADS; make install; cd ../../../
cd class/cpp/Edf; make clean; make $THREADS; make install; cd ../../../

#------------------------------------------------------------------------------
#
# build the utilities
#
#------------------------------------------------------------------------------

# compile and install cpp-based utilities
#
cd util/cpp/nedc_print_header; make clean; make $THREADS; make install; cd ../../../
cd util/cpp/nedc_print_signal; make clean; make $THREADS; make install; cd ../../../

# exit gracefully
#
exit 0;
