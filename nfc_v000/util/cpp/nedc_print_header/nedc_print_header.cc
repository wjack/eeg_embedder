// file: $(NEDC_NFC)/util/cpp/nedc_print_header/nedc_print_header.cc
//
// this is the driver program for a simple program that
// dumps the EDF header.
//
// usage:
//  nedc_print_header x1.edf x2.edf x3.list ...
//
// modified:
//  20160212 (SL): modified to use the Cmdl class
//  20140214 (JP): upgraded to use the new I/O in the EDF class
//  20131129 (JP): initial version
//
// This version of nedc_print_header uses a very simplistic approach
// to reading and procssing an EDF header. It has been tested across
// the entire TUH EEG corpus.
//

// local include files
//
#include <Edf.h>
#include <Cmdl.h>

// define the help and usage messages
//
#define NEDC_NFC_USAGE_MSG "$NEDC_NFC/util/cpp/nedc_print_header/nedc_print_header.usage"
#define NEDC_NFC_HELP_MSG "$NEDC_NFC/util/cpp/nedc_print_header/nedc_print_header.help"

// main: nedc_print_header
//
// This is a driver program that reads EDF files, either using
// file lists or direct files, and processes them.
//
int main(int argc, const char** argv) {

  // declare local variables
  //
  char edf_fname[Edf::MAX_LSTR_LENGTH];
  bool flag_list = false;
  int status = 0;
  
  // create a Command Line object
  //
  Cmdl cmdl(Edf::LEVEL_NONE);
  cmdl.set_usage(NEDC_NFC_USAGE_MSG);
  cmdl.set_help(NEDC_NFC_HELP_MSG);

  // create an Edf object
  //
  Edf edf(Edf::LEVEL_NONE);

  // branch on the status of parsing, checking for usage and help messages
  //
  if ((argc == 1) || (cmdl.parse(argc, argv) == false)) {
    cmdl.display_usage(stdout);
    return (status);
  }
  else if (cmdl.get_help_status() == true) {
    cmdl.display_help(stdout);
    return (status);
  }    

  // display an informational message
  //
  fprintf(stdout, "beginning argument processing...\n");

  // main processing loop: loop over all input filenames
  //
  long num_files_att = 0;
  long num_files_proc = 0;

  for (long i = cmdl.get_first_arg_pos(); i < argc; i++) {

    // if it is an edf file, process it
    //
    if (edf.is_edf((char*)argv[i])) {
      num_files_att++;
      fprintf(stdout, "  %6ld: %s\n", num_files_att, (char*)argv[i]);
      if (edf.print_header((char*)argv[i], (FILE*)stdout) == true) {
	num_files_proc++;
      }
      edf.cleanup();
    }

    // else: treat it as a file list
    //
    else {

      // display an informational message
      //
      fprintf(stdout, " opening list %s...\n", (char*)argv[i]);

      // open the list
      //
      FILE* fp = fopen(argv[i], "r");
      if (fp == (FILE*)NULL) {
	fprintf(stdout, " **> print_header: error opening file list (%s)\n",
		argv[i]);
	return(status = false);
      }

      // loop over all files
      //
      while (fscanf(fp, "%s", edf_fname) == 1) {
	num_files_att++;
	fprintf(stdout, "  %6ld: %s\n", num_files_att, edf_fname);
	if (edf.print_header(edf_fname, (FILE*)stdout) == true) {
	  num_files_proc++;
	}
	edf.cleanup();
      }

      // close the list
      //
      fclose(fp);
    }
  }

  // display the results
  //
  fprintf(stdout, "\nprocessed %ld out of %ld files successfully\n",
	  num_files_proc, num_files_att);

  // exit gracefully
  //
  return(status);
}
