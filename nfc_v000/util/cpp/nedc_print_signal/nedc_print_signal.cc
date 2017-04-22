// file: $(NEDC_NFC)/util/cpp/nedc_print_signal/nedc_print_signal.cc
//
// this is the driver program for a simple program that
// dumps the EDF signal data.
//
// usage:
//  print_signal chan_number start_sample num_samples filename
//
// modified:
//  20160609 (JM): fixed debugging output
//  20160212 (SL): modified to use the Cmdl class
//  20140301 (JP): added multichannel updated to use the new EDF I/O functions
//  20140214 (JP): updated to use the new EDF I/O functions
//  20131129 (JP): initial version
//
// This version of print_signal uses a very simplistic approach
// to reading and displaying an EDF file's signal data.
//

// local include files
//
#include <Edf.h>
#include <Cmdl.h>

// define the help and usage messages
//
#define NEDC_NFC_USAGE_MSG "$NEDC_NFC/util/cpp/nedc_print_signal/nedc_print_signal.usage"
#define NEDC_NFC_HELP_MSG "$NEDC_NFC/util/cpp/nedc_print_signal/nedc_print_signal.help"

// main: nedc_print_signal
//
// This is a driver program that reads EDF files, either using
// file lists or direct files, and prints the designated values.
//
int main(int argc, const char** argv) {

  // declare local variables
  //
  int status = 0;
  vector< vector<double> > sig_scaled;
  vector< vector<double> > sig_unscaled;

  // initialize a Command Line object
  //
  Cmdl cmdl(Edf::LEVEL_NONE);
  cmdl.set_usage(NEDC_NFC_USAGE_MSG);
  cmdl.set_help(NEDC_NFC_HELP_MSG);

  // create an Edf object
  //
  Edf edf(Edf::LEVEL_NONE);

  // add options
  //
  long channel_number = 0;
  cmdl.add_option("-channel_number", &channel_number);
  
  long start_sample = 0;
  cmdl.add_option("-start_sample", &start_sample);
  
  long num_samples = 500;
  cmdl.add_option("-number_samples", &num_samples);

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

  // get the position of the first file argument
  //
  long arg_pos = cmdl.get_first_arg_pos();
  
  // read the signal
  //
  if (!edf.read_edf(sig_unscaled, (char*)argv[arg_pos], false, true)) {
    fprintf(stdout, "  **> nedc_print_signal: error opening (%s) - unscaled\n",
	    (char*)argv[arg_pos]);
  }

  if (!edf.read_edf(sig_scaled, (char*)argv[arg_pos], true, true)) {
    fprintf(stdout, "  **> nedc_print_signal: error reading (%s) - scaled\n",
	    (char*)argv[arg_pos]);
  }

  // display some diagnostic information
  //
  fprintf(stdout, "number of channels = %ld\n", (long int)sig_scaled.size());
  fprintf(stdout, "\n");

  // case 1: channel number nonnegative
  //
  if (channel_number >= 0) {
    fprintf(stdout, "channel: %ld   start: %ld\n",
	    channel_number, start_sample);
    for (long i = start_sample; i < start_sample + num_samples; i++) {
      fprintf(stdout, "%ld %ld: (s: %f) (u: %d)\n", channel_number, i,
	      sig_scaled[channel_number][i],
	      (short)sig_unscaled[channel_number][i]);
    }
  }

  // case 2: channel number negative
  //
  else {
    for (long n = 0; n < sig_scaled.size(); n++) {
      fprintf(stdout, "channel: %ld   sample: %ld\n", n, start_sample);
      for (long i = start_sample; i < start_sample + num_samples; i++) {
	fprintf(stdout, " sig[%ld][%ld] =  (s: %f) (u: %d)\n",
		n, i, sig_scaled[n][i],	(short)sig_unscaled[n][i]);
      }
    }
  }

  // clean up an Edf object
  //
  edf.cleanup();

  // exit gracefully
  //
  return(status);
}
