// file: $(NEDC_NFC)/class/cpp/Cmdl/cmdl_03.cc
//
// This file contains methods that support parsing.
//

// local include files
//
#include "Cmdl.h"

// method: parse
//
// arguments:
//  int argc: the number of command line arguments (input)
//  const char** argv: the command line values (input)
//
// return: a logical value indicating status
//
// This method parses a command line.
//
bool Cmdl::parse(int argc_a, const char** argv_a) {

  // declare a variable for the position of the first argument to be
  // processed after the options have been parsed
  //
  first_arg_pos_d = 1;

  // check for a help option
  //
  if (is_help(argc_a, argv_a) == true) {
    return true;
  }

  // else: parse the arguments one by one
  //
  else {

    // loop over all arguments - skip the program name
    //
    for (long i = 1; i < argc_a; i++) {

      // look for a unique match
      //
      long pos = unique_match((char*)argv_a[i]);

      // case: multiple matches - error off and generate a usage message
      //
      if (pos == (long)-2) {
	usage_d = true;
	return false;
      }

      // case: help message
      //
      else if (pos == DEF_POS_HELP) {
	help_d = true;
	return false;
      }

      // case: help message
      //
      else if (pos == DEF_POS_USAGE) {
	usage_d = true;
	return false;
      }

      // case: a single match - set the value using the next arg
      //
      else if (pos >= 0) {

	// if it is a char, copy the string and increment the counter
	//
	if (opts_char_d[pos] != (char*)NULL) {
	  strcpy(opts_char_d[pos], argv_a[i+1]);
	  first_arg_pos_d = i + 2;
	  i++;
	}

	// else if it is bool, set the value but don't increment the counter
	//
	else if (opts_bool_d[pos] != (bool*)NULL) {
	  *(opts_bool_d[pos]) = true;
	  first_arg_pos_d = i + 1;
	}

	// else if it is long, set the value using the next argument
	// and increment the counter
	//
	else if (opts_long_d[pos] != (long*)NULL) {
	  *(opts_long_d[pos]) = atoi(argv_a[i+1]);
	  first_arg_pos_d = i + 2;
	  i++;
	}

	// else if it is a float, set the value using the next argument
	// and increment the counter
	//
	else if (opts_float_d[pos] != (float*)NULL) {
	  *(opts_float_d[pos]) = atof(argv_a[i+1]);
	  first_arg_pos_d = i + 2;
	  i++;
	}
      }	

      // case: no match and it was an option - trigger the usage message
      //
      else if ((pos < 0) && (argv_a[i][0] == DELIM_OPTION)) {
	usage_d = true;
	return false;
      }	
    }
  }

  // exit gracefully
  //
  return true;
}

// method: is_help
//
// arguments:
//  int argc: the number of command line arguments (input)
//  const char** argv: the command line values (input)
//
// return: a logical value indicating status
//
// This method returns true if a help argument was specified.
//
bool Cmdl::is_help(int argc_a, const char** argv_a) {

  // declare local variables
  //
  long pos = -1;

  // do a unique match
  //
  for (long i = 0; i < argc_a; i++) {
    if ((pos = unique_match((char*)argv_a[i])) == DEF_POS_HELP) {
      return (help_d = true);
    }
  }

  // exit gracefully
  //
  return false;
}
