// file: $(NEDC_NFC)/class/cpp/Cmdl/Cmdl.h
//

// Revision History:
//  20160209 (JP): revised and restructured.
//  20160205 (SL): initial version
//

// make sure definitions are only made once
//
#ifndef NEDC_CMDL
#define NEDC_CMDL

// include standard libraries
//

// local include files
//
#include <Edf.h>	   // debug flags

// Cmdl: a class that handles the parsing of command line arguments.
//  This class is designed to make command line parsing common across
//  all our C++ tools.
//
class  Cmdl {

  //--------------------------------------------------------------------------
  //
  // public constants
  //
  //--------------------------------------------------------------------------
public:

  // define the class name
  //
  static const char* CLASS_NAME;

  //----------------------------------------
  //
  // other important constants
  //
  //----------------------------------------

  // define the maximum number of options
  //
  static const long MAX_NOPTS = 99;

  // define the max help line size for a help message
  //
  static const long MAX_HLINE_SIZE = 99;

  // define the max filename size
  //
  static const long MAX_FNAME_SIZE = 499;

  // define the max option value size
  //
  static const long MAX_OPTVAL_SIZE = 99;

  // define the max environment variable name size
  //
  static const long MAX_ENV_SIZE = 99;

  // define the character used to delimit options
  //
  static const char DELIM_OPTION = '-';

  // define the character used to delimit environment variables
  //
  static const char DELIM_ENV_VAR = '$';

  // define the character used to delimit a directory
  //
  static const char DELIM_DIR = '/';

  //----------------------------------------
  //
  // default values and arguments
  //
  //----------------------------------------

  // default values for message-related parameters
  //
  static const char* DEF_OPT_HELP;
  static const char* DEF_OPT_USAGE;

  static const long DEF_POS_HELP;
  static const long DEF_POS_USAGE;

  //--------------------------------------------------------------------------
  //
  // protected data
  //
  //--------------------------------------------------------------------------
protected:

  // define a debug level
  //
  static long debug_level_d;
  static long verbosity_d;

  // help message related variables
  //
  bool usage_d;
  bool help_d;
  char* usage_fname_d;
  char* help_fname_d;

  // command line data structures:
  //  these decode the type of things. there is one for each supported type.
  //
  long num_opts_d;
  char** opts_names_d;
  char** opts_char_d;
  bool** opts_bool_d;
  long** opts_long_d;
  float** opts_float_d;

  // argument related variables
  //
  long first_arg_pos_d;

  //--------------------------------------------------------------------------
  //
  // required public methods
  //
  //--------------------------------------------------------------------------
public:

  // method name
  //
  inline static const char* name() {
    return CLASS_NAME;
  }

  // method: destructor
  //
  ~Cmdl();

  // method: default constructor
  //
  Cmdl(long debug_level = Edf::DEF_LEVEL);

  //--------------------------------------------------------------------------
  //
  // other public methods
  //
  //--------------------------------------------------------------------------
public:

  // status methods
  //
  bool get_usage_status() {
    return usage_d;
  }
  bool get_help_status() {
    return help_d;
  }

  long get_first_arg_pos() {
    return first_arg_pos_d;
  }

  // set methods
  //
  bool set_usage(const char* fname);
  bool set_help(const char* fname);

  // display methods
  //
  bool display_usage(FILE* fp);
  bool display_help(FILE* fp);

  // environment methods
  //
  bool expand_filename(char* exp_name, const char* orig_name);

  // add option methods
  //
  bool add_option(const char* name, char* val);
  bool add_option(const char* name, bool* val);
  bool add_option(const char* name, long* val);
  bool add_option(const char* name, float* val);

  // parse methods
  //
  bool parse(int argc, const char** argv);
  bool is_help(int argc, const char** argv);
  bool is_usage(int argc, const char** argv);

  // debug methods
  //
  bool debug(FILE* fp);

  //---------------------------------------------------------------------------
  //
  // private methods
  //
  //---------------------------------------------------------------------------
private:

  // memory management methods
  //
  bool cleanup();
  bool cleanup(char** strs, long num_strs);

  // unique matching
  //
  long unique_match(char* str);
};

// end of include file
//
#endif
