// file: $(NEDC_NFC)/class/cpp/Cmdl/cmdl_01.cc
//
// This file contains methods that support help functions.
//

// local include files
//
#include "Cmdl.h"

// method: set_usage
//
// arguments:
//  char* fname: filename to be displayed (input)
//
// return: a logical value indicating status
//
// This method displays the usage file.
//
bool Cmdl::set_usage(const char* fname_a) {

  // free space if already allocated
  //
  if (usage_fname_d != (char*)NULL) {
    delete [] usage_fname_d;
  }

  // create space
  //
  usage_fname_d = new char[strlen(fname_a) + 1];

  // copy the string
  //
  strcpy(usage_fname_d, fname_a);

  // exit gracefully
  //
  return true;
}

// method: display_usage
//
// arguments:
//  FILE* fp* file point to be used for I/O (input)
//
// return: a logical value indicating status
//
// This method displays the usage message.
//
bool Cmdl::display_usage(FILE* fp_a) {

  // check for an open stream
  //
  if (fp_a == (FILE*)NULL) {
    return false;
  }

  // check for a null message
  //
  else if (usage_fname_d == (char*)NULL) {
    return false;
  }

  // expand the usage file name
  //
  char tmp_name[MAX_FNAME_SIZE + 1];

  if (expand_filename(tmp_name, usage_fname_d) == false) {
    return false;
  }

  // open the usage file
  //
  FILE* fp_h;

  if ((fp_h = fopen(tmp_name, "r")) == (FILE*)NULL) {
    return false;
  }

  // loop over the file and read it line by line
  //
  char buf[MAX_HLINE_SIZE + 1];

  while (fgets(buf, MAX_HLINE_SIZE, fp_h) != (char*)NULL) {

    // display the line
    //
    fprintf(fp_a, "%s", buf);
  }

  // close the file
  //
  fclose(fp_h);

  // exit gracefully
  //
  return true;
}

// method: set_help
//
// arguments:
//  char* fname: filename to be displayed (input)
//
// return: a logical value indicating status
//
// This method displays the help file.
//
bool Cmdl::set_help(const char* fname_a) {

  // free space if already allocated
  //
  if (help_fname_d != (char*)NULL) {
    delete [] help_fname_d;
  }

  // create space
  //
  help_fname_d = new char[strlen(fname_a) + 1];

  // copy the string
  //
  strcpy(help_fname_d, fname_a);

  // exit gracefully
  //
  return true;
}

// method: display_help
//
// arguments:
//  FILE* fp* file point to be used for I/O (input)
//
// return: a logical value indicating status
//
// This method displays the usage message.
//
bool Cmdl::display_help(FILE* fp_a) {

  // check for an open stream
  //
  if (fp_a == (FILE*)NULL) {
    return false;
  }

  // check for a null message
  //
  else if (help_fname_d == (char*)NULL) {
    return false;
  }

  // expand the help file name
  //
  char tmp_name[MAX_FNAME_SIZE + 1];

  if (expand_filename(tmp_name, help_fname_d) == false) {
    return false;
  }

  // open the help file
  //
  FILE* fp_h;

  if ((fp_h = fopen(tmp_name, "r")) == (FILE*)NULL) {
    return false;
  }

  // loop over the file and read it line by line
  //
  char buf[MAX_HLINE_SIZE + 1];

  while (fgets(buf, MAX_HLINE_SIZE, fp_h) != (char*)NULL) {

    // display the line
    //
    fprintf(fp_a, "%s", buf);
  }

  // close the file
  //
  fclose(fp_h);

  // exit gracefully
  //
  return true;
}

// method: expand_filename
//
// arguments:
//  char* fname_out: expanded filename (output)
//  const char* fname_in: original filename (input)
//
// return: a logical value indicating status
//
// This method expands any environment variables that appear in a name.
//
bool Cmdl::expand_filename(char* fname_out_a, const char* fname_in_a) {

  // loop to file an environment variable
  //
  while (*fname_in_a != (char)NULL) {
    if (*fname_in_a == DELIM_ENV_VAR) {
      fname_in_a++;

      // find the next file delimiter
      //
      char* delim = strchr((char*)fname_in_a, (int)DELIM_DIR);

      // create a string with the environment variable name
      //
      char buf[MAX_ENV_SIZE + 1];
      strncpy(buf, fname_in_a, delim - fname_in_a);
      buf[delim - fname_in_a] = (char)NULL;

      // expand this variable
      //
      strcpy(fname_out_a, getenv(buf));
      fname_out_a += strlen(fname_out_a);
      *fname_out_a++ = DELIM_DIR;

      // skip over the environment variable
      //
      fname_in_a = delim + 1;
    }
    else {
      *fname_out_a++ = *fname_in_a++;
    }
  }

  // finish the output string with a null character
  //
  *fname_out_a = (char)NULL;
  
  // exit gracefully
  //
  return true;
}
