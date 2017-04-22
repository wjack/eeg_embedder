// file: $(NEDC_NFC)/class/cpp/Cmdl/cmdl_02.cc
//
// This file contains methods that support adding and manipulating options.
//

// local include files
//
#include "Cmdl.h"

// method: add_option
//
// arguments:
//  char* name: the name of the option (input)
//  char* val: the variable to be used to store the value (input)
//
// return: a logical value indicating status
//
// This method associates an option name with a variable.
//
bool Cmdl::add_option(const char* name_a, char* val_a) {

  // add the name to the stack
  //
  opts_names_d[num_opts_d] = new char[strlen(name_a) + 1];
  strcpy(opts_names_d[num_opts_d], name_a);
  
  // add the pointer to the destination variable for the correct type,
  // and zero out all the other types
  //
  opts_char_d[num_opts_d] = val_a;
  opts_bool_d[num_opts_d] = (bool*)NULL;
  opts_long_d[num_opts_d] = (long*)NULL;
  opts_float_d[num_opts_d] = (float*)NULL;

  // increment the number of options
  //
  num_opts_d++;

  // exit gracefully
  //
  return true;
}

// method: add_option
//
// arguments:
//  char* name: the name of the option (input)
//  bool* val: the variable to be used to store the value (input)
//
// return: a logical value indicating status
//
// This method associates an option name with a variable.
//
bool Cmdl::add_option(const char* name_a, bool* val_a) {

  // add the name to the stack
  //
  opts_names_d[num_opts_d] = new char[strlen(name_a) + 1];
  strcpy(opts_names_d[num_opts_d], name_a);
  
  // add the pointer to the destination variable for the correct type,
  // and zero out all the other types
  //
  opts_char_d[num_opts_d] = (char*)NULL;
  opts_bool_d[num_opts_d] = val_a;
  opts_long_d[num_opts_d] = (long*)NULL;
  opts_float_d[num_opts_d] = (float*)NULL;

  // increment the number of options
  //
  num_opts_d++;

  // exit gracefully
  //
  return true;
}

// method: add_option
//
// arguments:
//  char* name: the name of the option (input)
//  long* val: the variable to be used to store the value (input)
//
// return: a logical value indicating status
//
// This method associates an option name with a variable.
//
bool Cmdl::add_option(const char* name_a, long* val_a) {

  // add the name to the stack
  //
  opts_names_d[num_opts_d] = new char[strlen(name_a) + 1];
  strcpy(opts_names_d[num_opts_d], name_a);
  
  // add the pointer to the destination variable for the correct type,
  // and zero out all the other types
  //
  opts_char_d[num_opts_d] = (char*)NULL;
  opts_bool_d[num_opts_d] = (bool*)NULL;
  opts_long_d[num_opts_d] = val_a;
  opts_float_d[num_opts_d] = (float*)NULL;

  // increment the number of options
  //
  num_opts_d++;

  // exit gracefully
  //
  return true;
}

// method: add_option
//
// arguments:
//  char* name: the name of the option (input)
//  float* val: the variable to be used to store the value (input)
//
// return: a logical value indicating status
//
// This method associates an option name with a variable.
//
bool Cmdl::add_option(const char* name_a, float* val_a) {

  // add the name to the stack
  //
  opts_names_d[num_opts_d] = new char[strlen(name_a) + 1];
  strcpy(opts_names_d[num_opts_d], name_a);
  
  // add the pointer to the destination variable for the correct type,
  // and zero out all the other types
  //
  opts_char_d[num_opts_d] = (char*)NULL;
  opts_bool_d[num_opts_d] = (bool*)NULL;
  opts_long_d[num_opts_d] = (long*)NULL;
  opts_float_d[num_opts_d] = val_a;

  // increment the number of options
  //
  num_opts_d++;

  // exit gracefully
  //
  return true;
}
