// file: $(NEDC_NFC)/class/cpp/Edf/edf_04.cc
//
// This file contains private methods associated with the class Edf
// that perform basic math operations.
//

// local include files
//
#include "Edf.h"

// method: clip
// 
// arguments:
//  double value: value to be clipped (input)
//  double min: minimum allowed value (input)
//  double max: maximum allowed value (input)
//
// return: a double value in the range [min, max]
//  
double Edf::clip(double value_a, double min_a, double max_a) {

  // check the range
  //
  if (value_a > max_a) {
    return max_a;
  }
  else if (value_a < min_a) {
    return min_a;
  }

  // exit gracefully
  //
  else {
    return value_a;
  }
}

// method: clip
// 
// arguments:
//  long value: value to be clipped (input)
//  long min: minimum allowed value (input)
//  long max: maximum allowed value (input)
//
// return: a long value in the range [min, max]
//  
long Edf::clip(long value_a, long min_a, long max_a) {

  // check the range
  //
  if (value_a > max_a) {
    return max_a;
  }
  else if (value_a < min_a) {
    return min_a;
  }

  // exit gracefully
  //
  else {
    return value_a;
  }
}

// method: max
// 
// arguments:
//  VectorDouble& input: data to be searched (input)
//
// return: a double value containing the maximum value
//  
double Edf::max(VectorDouble& input_a) {

  // declare local variables
  //
  long i_end = input_a.size();
  double sum;

  // do some bounds checking
  //
  if (i_end <= 0) {
    fprintf(stdout, "Edf::max(): zero-length vector\n");
    return 0;
  }
  else if (i_end == 1) {
    return input_a[0];
  }
  else {
    sum = input_a[0];
  }

  // loop over all elements
  //
  for (long i = 1; i < i_end; i++) {
    if (input_a[i] > sum) {
      sum = input_a[i];
    }
  }

  // exit gracefully
  //
  return sum;
}

// method: max
// 
// arguments:
//  double val1: operand 1 (input)
//  double val2: operand 2 (input)
//
// return: a double value containing the maximum value of the two operands
//  
double Edf::max(double val1, double val2) {
  if (val1 > val2) {
    return val1;
  }
  else {
    return val2;
  }
}

// method: min
// 
// arguments:
//  VectorDouble& input: data to be searched (input)
//
// return: a double value containing the minmum value
//  
double Edf::min(VectorDouble& input_a) {

  // declare local variables
  //
  long i_end = input_a.size();
  double sum;

  // do some bounds checking
  //
  if (i_end <= 0) {
    fprintf(stdout, "Edf::min(): zero-length vector\n");
    return 0;
  }
  else if (i_end == 1) {
    return input_a[0];
  }
  else {
    sum = input_a[0];
  }

  // loop over all elements
  //
  for (long i = 1; i < i_end; i++) {
    if (input_a[i] < sum) {
      sum = input_a[i];
    }
  }

  // exit gracefully
  //
  return sum;
}

// method: min
// 
// arguments:
//  double val1: operand 1 (input)
//  double val2: operand 2 (input)
//
// return: a double value containing the minimum value of the two operands
//  
double Edf::min(double val1, double val2) {
  if (val1 < val2) {
    return val1;
  }
  else {
    return val2;
  }
}

// method: shift
// 
// arguments:
//  VectorDouble& v: vector to be shifted (input)
//  long incr: shift increment (input)
//
// return: a logical value indicating status
//
// This method shifts elements in a vector and resizes the vector.
//
bool Edf::shift(VectorDouble& v_a, long incr_a) {

  // check the direction
  //
  if (incr_a == 0) {
    return true;
  }
  else if (incr_a < 0) {
    return false;
  }

  // shift data
  //
  long new_size = v_a.size() - 1;
  long loc = incr_a;
  for (long i = 0; i < new_size; i++) {
    v_a[i] = v_a[incr_a];
    incr_a++;
  }

  // resize
  //
  Edf::resize(v_a, new_size, true);

  // exit gracefully
  //
  return true;
}

//
// end of file
