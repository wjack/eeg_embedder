// file: $(NEDC_NFC)/class/cpp/Cmdl/cmdl_00.cc
//
// This file contains basic required methods such as constructors
//  and destructors.

// Revision History
//  20150209 (JP): revised and restructured
//  20150208 (SL): initial version
//

// local include files
//
#include "Cmdl.h"

//-----------------------------------------------------------------------------
//
// basic required methods
//
//-----------------------------------------------------------------------------

// method: default constructor
//
Cmdl::Cmdl(long debug_level_a) {

  // set the debug level
  //
  debug_level_d = debug_level_a;
  verbosity_d = Edf::DEF_LEVEL;

  // display debugging information
  //
  if (debug_level_d >= Edf::LEVEL_FULL) {
    fprintf(stdout, "Cmdl(): initalizing a Cmdl object\n");
  }

  // initialize protected data
  //
  usage_d = false;
  help_d = false;
  usage_fname_d = (char*)NULL;
  help_fname_d = (char*)NULL;

  // initialize the option arrays:
  //  note that the help and usage options are always available
  //
  opts_names_d = new char*[MAX_NOPTS];
  opts_char_d = new char*[MAX_NOPTS];
  opts_bool_d = new bool*[MAX_NOPTS];
  opts_long_d = new long*[MAX_NOPTS];
  opts_float_d = new float*[MAX_NOPTS];

  for (long i = 0; i < MAX_NOPTS; i++) {
    opts_names_d[i] = (char*)NULL;
    opts_char_d[i] = (char*)NULL;
    opts_bool_d[i] = (bool*)NULL;
    opts_long_d[i] = (long*)NULL;
    opts_float_d[i] = (float*)NULL;
  }

  // set the first two entries to help and usage
  //
  num_opts_d = 2;
  opts_names_d[DEF_POS_HELP] = new char[strlen(DEF_OPT_HELP) + 1];
  opts_names_d[DEF_POS_USAGE] = new char[strlen(DEF_OPT_USAGE) + 1];
  strcpy(opts_names_d[DEF_POS_HELP], DEF_OPT_HELP);
  strcpy(opts_names_d[DEF_POS_USAGE], DEF_OPT_USAGE);

  opts_char_d[DEF_POS_HELP] = (char*)NULL;
  opts_bool_d[DEF_POS_HELP] = (bool*)NULL;
  opts_long_d[DEF_POS_HELP] = (long*)NULL;
  opts_float_d[DEF_POS_HELP] = (float*)NULL;

  opts_char_d[DEF_POS_USAGE] = (char*)NULL;
  opts_bool_d[DEF_POS_USAGE] = (bool*)NULL;
  opts_long_d[DEF_POS_USAGE] = (long*)NULL;
  opts_float_d[DEF_POS_USAGE] = (float*)NULL;

  // initialize the position of the first argument
  //
  first_arg_pos_d = -1;

  // display debugging information
  //
  if (debug_level_d >= Edf::LEVEL_FULL) {
    fprintf(stdout, "Cmdl(): done initalizing a Cmdl object\n");
  }

  // exit gracefully
  //
};

// method: destructor
//
//  arguments: none
//
//  return: none
//
//  This method implements the destructor.
//
Cmdl::~Cmdl() {

  // display debugging information
  //
  if (debug_level_d >= Edf::LEVEL_FULL) {
    fprintf(stdout, "~Cmdl(): begin destroying a Cmdl object\n");
  }

  // clean up memory
  //
  Cmdl::cleanup();

  // display debugging information
  //
  if (debug_level_d >= Edf::LEVEL_FULL) {
    fprintf(stdout, "~Cmdl(): end of destroying a Cmdl object\n");
  }

  // exit gracefully
  //
}

// method: debug
//
// arguments:
//  FILE* fp: stream to use to display debug information (output)
//
// return: a logical value indicating status
//  
bool Cmdl::debug(FILE* fp_a) {

  // display the default help and usage options
  //
  fprintf(fp_a, "the help option name is: %s\n\n", DEF_OPT_HELP);
  fprintf(fp_a, "the usage option name is: %s\n\n", DEF_OPT_USAGE);

  // display the current option names
  //
  fprintf(fp_a, "number of options is: %d\n", num_opts_d);
  for (long i = 0; i < num_opts_d; i++) {
    if (opts_char_d[i] != (char*)NULL) {
      fprintf(fp_a, " option no. %3d: %s (type = char)\n",
	      i, opts_names_d[i]);
    }
    else if (opts_bool_d[i] != (bool*)NULL) {
      fprintf(fp_a, " option no. %3d: %s (type = bool)\n",
	      i, opts_names_d[i]);
    }
    else if (opts_long_d[i] != (long*)NULL) {
      fprintf(fp_a, " option no. %3d: %s (type = long)\n",
	      i, opts_names_d[i]);
    }
    else if (opts_float_d[i] != (float*)NULL) {
      fprintf(fp_a, " option no. %3d: %s (type = float)\n",
	      i, opts_names_d[i]);
    }
    else {
      fprintf(fp_a, " option no. %3d: %s (type = *** error ***)\n",
	      i, opts_names_d[i]);
    }
  }

  // exit gracefully
  //
  return true;
}

//-----------------------------------------------------------------------------
//
// private methods
//
//-----------------------------------------------------------------------------

// method: cleanup
//
// arguments: none
//
// return: a boolean value indicating status
//
// This method deletes memory allocated during processing.
//
bool Cmdl::cleanup() {

  // display debug information
  //
  if (debug_level_d >= Edf::LEVEL_FULL) {
    fprintf(stdout, "Cmdl::cleanup(): starting clean up of memory\n");
  }

  // clean up the options variables
  //
  cleanup(opts_names_d, MAX_NOPTS);

  // clean up the help and usage filenames
  //
  if (help_fname_d != (char*)NULL) {
    delete [] help_fname_d;
    help_fname_d = (char*)NULL;
  }
  if (usage_fname_d != (char*)NULL) {
    delete [] usage_fname_d;
    usage_fname_d = (char*)NULL;
  }

  // display debug information
  //
  if (debug_level_d >= Edf::LEVEL_FULL) {
    fprintf(stdout, "Cmdl::cleanup(): done cleaning up memory\n");
  }
  
  // exit gracefully
  //
  return true;
}

// method: cleanup
// 
// arguments:
//  char** strs: array of character pointers
//  long num_strs: number of elements
//
// return: a logical value indicating status
//  
bool Cmdl::cleanup(char** strs_a, long num_strs_a) {

  // loop over the elements and delete them
  //
  for (long i = 0; i < num_strs_a; i++) {
    if (strs_a[i] != (char*)NULL) {
      delete [] strs_a[i];
    }
    strs_a[i] = (char*)NULL;
  }

  // exit gracefully
  //
  return true;
}

// method: unique_match
//
// arguments:
//  char* str: string to be matched (input)
//
// return: an integer value with the position of the match or -1 if no match,
//         or -2 if multiple matches
//  
// This method checks an argument against the list of options
// and determines if there is a unique match.
//
long Cmdl::unique_match(char* str_a) {

  // declare local variables
  //
  long num_matches = (long)0;
  long len_str = strlen(str_a);
  long pos_match = -1;

  // loop over all options
  //
  for (long i = 0; i < num_opts_d; i++) {
    if (strncmp(str_a, opts_names_d[i], len_str) == (long)0) {
      pos_match = i;
      num_matches++;
    }
    if (num_matches > (long)1) {
      return (long)-2;
    }
  }

  // exit gracefully
  //
  return pos_match;
}

//-----------------------------------------------------------------------------
//
// we define non-integral constants in the default constructor
//
//-----------------------------------------------------------------------------

// constants: class name
//
const char* Cmdl::CLASS_NAME("Cmdl");

// constants: debug level
//
long Cmdl::debug_level_d = Edf::DEF_LEVEL;
long Cmdl::verbosity_d = Edf::DEF_LEVEL;

// constants: default values
//
const char* Cmdl::DEF_OPT_HELP("-help");
const char* Cmdl::DEF_OPT_USAGE("-usage");

const long Cmdl::DEF_POS_HELP = (long)0;
const long Cmdl::DEF_POS_USAGE = (long)1;

//
// end of file
