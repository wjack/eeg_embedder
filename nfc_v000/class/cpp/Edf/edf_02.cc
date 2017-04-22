// file: $(NEDC_NFC)/class/cpp/Edf/edf_02.cc
//
// This file contains channel and signal manipulation methods.
//

// Revision History
//  20150701 (FG): apply_montage method modified to update the header labels
//                 regarding the values parsed for the montage specified in the
//                 parameter file
//
//  20150630 (FG): interpolation moved to Edf class
//                 methods added: interpolation_average
//
//  20150612 (FG): added interpolation functionality
//                 methods added: add_channel
//                               select_channel
//
//  20141219 (AH): apply_montage updated to account for negative gain
//
//  20141216 (AH): set_signal_dimensions is updated so it only update
//                 the frequency for channels that their original freq.
//                 is the same as channel 0 (EEG channels)
//

// local include files
//
#include "Edf.h"

// method: select_channel
//
// arguments:
//  VectorDouble& sigo: selected channel (output)
//  VVectorDouble& sigi: signal from file (input)
//  char* sstr: a character string containing a label string (input)
//
// return: a boolean value indicating status
//
// This method retrieves the signal data associated with a channel based on 
// the provided channel label. This method differs from select method 
// in the way that the latter modifies the header information.  
//
bool Edf::select_channel(VVectorDouble& sigo_a,
		         VVectorDouble& sigi_a,
		         char* sstr_a) {

  // selected match_mode
  //
  MATCH_MODE scmmode = smmode_d;

  // display debug information
  //
  if (debug_level_d >= LEVEL_DETAILED) {
    fprintf(stdout,
	    "Edf::select_channel(): beginning channel selection [%s]\n",
	    sstr_a);
  }
  
  // find the corresponding channel in the input signal
  //
  long pos;

  // get the position of desired channel label
  //
  if ((pos = Edf::find_match(sstr_a,
			     hdr_ghdi_nsig_rec_d, hdr_chan_labels_d,
			     scmmode)) < 0) {
    fprintf(stdout,
	    "**> Edf::select_channel(): no match for [%s]\n",
	    sstr_a);
    return false;
  }    
      
  if (debug_level_d >= LEVEL_DETAILED) {
    fprintf(stdout,
            "Edf::select_channel(): mapping channel %ld [%s]\n",
            pos, hdr_chan_labels_d[pos]);
  }
  
  // display debug information
  //
  if (debug_level_d >= LEVEL_DETAILED) {
    fprintf(stdout, "Edf::select_channel(): done with channel selection\n");
  }

  // create output space
  //
  long sigi_end = sigi_a.size();

  // make sure it has the same size
  //
  Edf::resize(sigo_a, sigi_end, false);

  // create output space
  //
  sigi_end = sigi_a[0].size();

  // make sure it has the same size
  //
  Edf::resize(sigo_a[0], sigi_end, false);

  // copy the data 
  //
  for (long j = 0; j < sigi_end; j++) {
    sigo_a[0][j] = sigi_a[pos][j];
  }       

  // exit gracefully
  //
  return true;
}

// method: select
//
// arguments:
//  VVectorDouble& sigo: selected signal (output)
//  VVectorDouble& sigi:  signal from file (input)
//  char* sstr: a character string containing a label string (input)
//  MATCH_MODE match_mode: match mode (input)
//
// return: a boolean value indicating status
//
// This method selects channels within a signal based on the provided
// channel labels. The argument match_mode controls the precision with which
// the labels are matched.
//
// Note that the arguments refer to labels coming from the parameter
// file. The final signal is organized according to the order specified
// in the montage specs in the parameter file.
//
// Note also that if the input "(null)", then the entire input
// signal is selected.
//
bool Edf::select(VVectorDouble& sigo_a,
		 VVectorDouble& sigi_a,
		 char* sstr_a, MATCH_MODE match_mode_a) {

  // declare local variables
  //
  bool status = true;

  // save the match mode
  //
  smmode_d = match_mode_a;

  // display debug information
  //
  if (debug_level_d >= LEVEL_DETAILED) {
    fprintf(stdout, "Edf::select(): beginning channel selection [%s] [%ld]\n",
	    sstr_a, (long)smmode_d);
  }

  // mode "(null)": copy everything
  //
  if (strcmp(sstr_a, NULL_NAME) == 0) {
    return Edf::copy_signal(sigo_a, num_slabels_d, slabels_d,
			    sigi_a, hdr_ghdi_nsig_rec_d, hdr_chan_labels_d);
  }
  
  // mode "everything else": process the channel selection field
  //
  else {

    // parse the channel selection field
    //
    status = Edf::parse_line(num_slabels_d, slabels_d, sstr_a, (char*)COMMA);
    if (!status) {
      fprintf(stdout,
	      "**> Edf::select(): error parsing channel tag [%s]\n",
	      sstr_a);
      return false;
    }

    if (debug_level_d >= LEVEL_FULL) {
      fprintf(stdout, "Edf::select(): parsed channel labels: [%s]\n", sstr_a);
      for (long i = 0; i < num_slabels_d; i++) {
	fprintf(stdout, "\t%ld: [%s]\n", i, slabels_d[i]);
      }
    }
    
    // create output space
    //
    Edf::resize(sigo_a, num_slabels_d, false);
    
    // create space for information associated with the new channels
    //
    long npos[num_slabels_d];
    
    char* new_chan_labels[num_slabels_d];
    char* new_chan_trans_type[num_slabels_d];
    char* new_chan_phys_dim[num_slabels_d];
    double new_chan_phys_min[num_slabels_d];
    double new_chan_phys_max[num_slabels_d];
    long new_chan_dig_min[num_slabels_d];
    long new_chan_dig_max[num_slabels_d];
    char* new_chan_prefilt[num_slabels_d];
    long new_chan_rec_size[num_slabels_d];

    // loop over the selected channels:
    //  since each channel can have a different channel size, we need
    //  to save the channel sizes and, of course, the labels, as we
    //  go along.
    //
    for (long i = 0; i < num_slabels_d; i++) {
      
      // find the corresponding channel in the input signal
      //
      long pos;
      if ((pos = Edf::find_match(slabels_d[i],
				 hdr_ghdi_nsig_rec_d, hdr_chan_labels_d,
				 match_mode_a)) < 0) {
	fprintf(stdout,
		"**> Edf::select(): no match for [%s]\n",
		slabels_d[i]);
	return false;
      }    
      
      if (debug_level_d >= LEVEL_DETAILED) {
	fprintf(stdout,
		"Edf::select(): mapping channel %ld [%s] to channel %ld [%s]\n",
		pos, hdr_chan_labels_d[pos], i, slabels_d[i]);
      }
      
      // copy channel-specific information
      //
      new_chan_labels[i] = new char[strlen(hdr_chan_labels_d[pos]) + 1];
      strcpy(new_chan_labels[i], hdr_chan_labels_d[pos]);
      
      new_chan_trans_type[i] =
	new char[strlen(hdr_chan_trans_type_d[pos]) + 1];
      strcpy(new_chan_trans_type[i], hdr_chan_trans_type_d[pos]);

      new_chan_phys_dim[i] =
	new char[strlen(hdr_chan_phys_dim_d[pos]) + 1];
      strcpy(new_chan_phys_dim[i], hdr_chan_phys_dim_d[pos]);

      new_chan_phys_min[i] = hdr_chan_phys_min_d[pos];
      new_chan_phys_max[i] = hdr_chan_phys_max_d[pos];
      new_chan_dig_min[i] = hdr_chan_dig_min_d[pos];
      new_chan_dig_max[i] = hdr_chan_dig_max_d[pos];

      new_chan_prefilt[i] =
	new char[strlen(hdr_chan_prefilt_d[pos]) + 1];
      strcpy(new_chan_prefilt[i], hdr_chan_prefilt_d[pos]);

      new_chan_rec_size[i] = hdr_chan_rec_size_d[pos];

      // create output space
      //
      long j_end = sigi_a[pos].size();
      Edf::resize(sigo_a[i], j_end, false);
      
      // copy the data
      //
      for (long j = 0; j < j_end; j++) {
	sigo_a[i][j] = sigi_a[pos][j];
      }
    }

    // adjust the header information:
    //  note we have to do this after the channels were selected
    //  to avoid corrupting the header while channels are being copied
    //
    for (long i = 0; i < num_slabels_d; i++) {

      Edf::resize(hdr_chan_labels_d[i],
		  strlen(new_chan_labels[i]) + 1, false);
      strcpy(hdr_chan_labels_d[i], new_chan_labels[i]);

      Edf::resize(hdr_chan_trans_type_d[i],
		  strlen(new_chan_trans_type[i]) + 1, false);
      strcpy(hdr_chan_trans_type_d[i], new_chan_trans_type[i]);

      Edf::resize(hdr_chan_phys_dim_d[i],
		  strlen(new_chan_phys_dim[i]) + 1, false);
      strcpy(hdr_chan_phys_dim_d[i], new_chan_phys_dim[i]);

      hdr_chan_phys_min_d[i] = new_chan_phys_min[i];
      hdr_chan_phys_max_d[i] = new_chan_phys_max[i];
      hdr_chan_dig_min_d[i] = new_chan_dig_min[i];
      hdr_chan_dig_max_d[i] = new_chan_dig_max[i];

      Edf::resize(hdr_chan_prefilt_d[i],
		  strlen(new_chan_prefilt[i]) + 1, false);
      strcpy(hdr_chan_prefilt_d[i], new_chan_prefilt[i]);

      hdr_chan_rec_size_d[i] = new_chan_rec_size[i];
    }

    // clean up memory
    //
    Edf::cleanup(new_chan_labels, num_slabels_d);
    Edf::cleanup(new_chan_trans_type, num_slabels_d);
    Edf::cleanup(new_chan_phys_dim, num_slabels_d);
    Edf::cleanup(new_chan_prefilt, num_slabels_d);

    // update the number of channels and the header size - this is a really
    // critical step that preserves the integrity of the header
    //
    hdr_ghdi_nsig_rec_d = num_slabels_d;
    hdr_ghdi_hsize_d = compute_header_size(hdr_ghdi_nsig_rec_d);
  }
  
  // display debug information
  //
  if (debug_level_d >= LEVEL_DETAILED) {
    fprintf(stdout, "Edf::select(): done with channel selection\n");
  }

  // exit gracefully
  //
  return true;
}

// method: remove
//
// arguments:
//  VVectorDouble& sigo: selected signal (output)
//  VVectorDouble& sigi:  signal from file (input)
//  char* sstr: a character string containing a label string (input)
//  MATCH_MODE matmode: match mode (input)
//
// return: a boolean value indicating status
//
// This method removes channels within a signal based on the provided
// channel labels.
//
bool Edf::remove(VVectorDouble& sigo_a,
		 VVectorDouble& sigi_a,
		 char* sstr_a, MATCH_MODE match_mode_a) {

  // declare local variables
  //
  bool status = true;

  // save the match mode
  //
  smmode_d = match_mode_a;

  // display debug information
  //
  if (debug_level_d >= LEVEL_DETAILED) {
    fprintf(stdout, "Edf::remove(): beginning channel selection [%s] [%ld]\n",
	    sstr_a, (long)smmode_d);
  }

  // mode "(null)": copy everything
  //
  if (strcmp(sstr_a, NULL_NAME) == 0) {
    fprintf(stdout,
	    "**> Edf::remove(): can't remove all channels [%s]\n",
	    sstr_a);
    return false;
  }

  // parse the channel selection field
  //
  status = Edf::parse_line(num_slabels_d, slabels_d, sstr_a, (char*)COMMA);
  if (!status) {
    fprintf(stdout,
	    "**> Edf::remove(): error parsing channel tag [%s]\n",
	    sstr_a);
    return false;
  }

  // build a list of channels to be saved by excluding those that
  // match the remove list
  //
  char sstr[MAX_LSTR_LENGTH];
  memset(sstr, (int)0, MAX_LSTR_LENGTH);
  long num_keep_labels = 0;
  
  for (long i = 0; i < hdr_ghdi_nsig_rec_d; i++) {

    // search for a match
    //
    long num_matches = 0;
    long num_tries = 0;
    while((num_tries < num_slabels_d) && (num_matches == 0)) {
      if (strstr(hdr_chan_labels_d[i], slabels_d[num_tries]) != (char*)NULL) {
	num_matches++;
      }
      num_tries++;
    }

    // add this to the output string
    //
    if (num_matches == 0) {
      if (num_keep_labels != 0) {
	strcat(sstr, COMMA);
	strcat(sstr, SPACE);
      }
      strcat(sstr, hdr_chan_labels_d[i]);
      num_keep_labels++;
    }
  }
    
  // display debug information
  //
  if (debug_level_d >= LEVEL_DETAILED) {
    fprintf(stdout, "Edf::remove(): done with channel selection\n");
  }

  // exit gracefully
  //
  return Edf::select(sigo_a, sigi_a, sstr, match_mode_a);
}

// method: apply_montage
//
// arguments:
//  VVectorDouble& sigo: selected signal (output)
//  VVectorDouble& sigi: signal from file (input)
//  char* mstr: a character string containing a montage label string (input)
//  MATCH_MODE match_mode: a value indicating the type of match (input)
//
// return: a boolean value indicating status
//
// This method computes a montage by differencing channels. Note that a
// key design choice was that if the channel is not available in the
// input signal, a warning message is printed and the output channel
// is zeroed out.
//
bool Edf::apply_montage(VVectorDouble& sigo_a,
			VVectorDouble& sigi_a,
			char** mstr_a, MATCH_MODE match_mode_a) {

  // declare local variables
  //
  bool status = true;
  long test;
  // display debug information
  //
  if (debug_level_d >= LEVEL_DETAILED) {
    fprintf(stdout, "Edf::apply_montage(): beginning apply_montage\n");
  }

  // save the match mode
  //
  mmmode_d = match_mode_a;

  // check the mode
  //
  if (strcmp(mstr_a[0], Edf::NULL_NAME) == 0) {
    return Edf::copy_signal(sigo_a, num_mlabels_d, mlabels1_d,
			    sigi_a, num_slabels_d, slabels_d);
  }
  
  // parse the montage string
  //
  if (debug_level_d >= LEVEL_DETAILED) {
    fprintf(stdout, "Edf::apply_montage(): parsing the montage string\n");
  }

  // create output space
  //
  if (num_mlabels_d != sigo_a.size()) {
    Edf::resize(sigo_a, num_mlabels_d, false);
  }
  
  // loop over all channels
  //
  for (long i = 0; i < num_mlabels_d; i++) {
    
    // case 1: no differencing
    //
    if (mlabels2_d[i] == (char)NULL) {

      // find the corresponding channel in the input signal
      //
      long pos;
      if ((pos = Edf::find_match(mlabels1_d[i],
				 num_slabels_d, slabels_d,
				 match_mode_a)) < 0) {

	// when a label is not found, always display a warning message
	// and arbitrarily set the channel index to 0.
	//
	fprintf(stdout, "**> Edf::apply_montage(): no match for [%s]\n",
		mlabels1_d[i]);
      }    

      // swap the data
      //
      if (debug_level_d >= LEVEL_FULL) {
	fprintf(stdout,
		"Edf::apply_montage(): copying channel %ld [%s] "
		"to channel %ld\n", pos, mlabels1_d[i], i);
      }

      // create space
      //
      long j_end = sigi_a[0].size();
      if (pos >= 0) {
	j_end = sigi_a[pos].size();
      }
      if (j_end != sigo_a[i].size()) {
	Edf::resize(sigo_a[i], j_end, false);
      }

      // copy the data:
      //  zero out the data if the channel is not found
      //
      if (pos < 0) {
	for (long j = 0; j < j_end; j++) {
	  sigo_a[i][j] = 0;
	}
      }
      else {
	for (long j = 0; j < j_end; j++) {
	  sigo_a[i][j] = sigi_a[pos][j];
	}
      }
    }

    // case 2: differencing
    //
    else {

      // find the corresponding channel numbers in the input signal
      //
      long pos1;
      if ((pos1 = Edf::find_match(mlabels1_d[i],
				  num_slabels_d, slabels_d,
				  match_mode_a)) < 0) {

	// when a label is not found, always display a warning message
	// and arbitrarily set the channel index to 0.
	//
	fprintf(stdout, "**> Edf::apply_montage(): no match for [%s]\n",
		mlabels1_d[i]);
      }

      long pos2;
      if ((pos2 = Edf::find_match(mlabels2_d[i],
				  num_slabels_d, slabels_d,
				  match_mode_a)) < 0) {
	fprintf(stdout,	"**> Edf::apply_montage(): no match for [%s]\n",
		mlabels2_d[i]);
      }

      // difference the data
      //
      if (debug_level_d >= LEVEL_FULL) {
	fprintf(stdout,
		"Edf::apply_montage(): computing channel %ld [%s] -- channel %ld [%s]\n",
		pos1, mlabels1_d[i], pos2, mlabels2_d[i]);
      }

      // create space:
      //  if one of the labels has not been found, use channel 0 and
      //  write zeroes to the signal.
      //
      if ((pos1 < 0) || (pos2 < 0)) {
	pos1 = 0;
	pos2 = 0;
      }
      long j_end = sigi_a[pos1].size();
      if (j_end != sigo_a[i].size()) {
	Edf::resize(sigo_a[i], j_end, false);
      }

      // copy the data:
      //  according to  convention in computing montage we
      //  have to multiply with -1 to acoount for negative
      //  gain (a remainder from analog world)
      //
      for (long j = 0; j < j_end; j++) {
	sigo_a[i][j] = -(sigi_a[pos1][j] - sigi_a[pos2][j]);
      }
    }
  }

  // adjust only the channel labels:
  //  note we have to do this after the channels were selected
  //  to avoid corrupting the header while channels are being copied
  //
  for (long i = 0; i < num_mlabels_d; i++) {

    // write the label specified by the parameter file
    //
    Edf::resize(hdr_chan_labels_d[i], strlen(mchan_d[i]) + 1);
    strcpy(hdr_chan_labels_d[i], mchan_d[i]);
  }

  // update the number of channels and the header size - this is a really
  // critical step that preserves the integrity of the header
  //
  hdr_ghdi_nsig_rec_d = num_mlabels_d;
  hdr_ghdi_hsize_d = compute_header_size(hdr_ghdi_nsig_rec_d);

  // display debug information
  //
  if (debug_level_d >= LEVEL_DETAILED) {
    fprintf(stdout, "Edf::apply_montage(): done with apply_montage\n");
  }
  
  // exit gracefully
  //
  return status;
}

// method: test_signal
//
// arguments:
//  VVectorDouble& sig: the EEG signal data (input/output)
//  const char* mode: specifies the type of signal (input)
//
// return: a boolean value indicating status
//
// This method is provided as a simple way to generate a signal for
// debugging. It must be called after read() because the signal
// an all its parameters must exist. It is not intended for general
// use. It was originally developed to debug the front end code. It
// replaces each channel with the exact same signal (e.g., sinewaves).
//
bool Edf::test_signal(VVectorDouble& sig_a, const char* mode_a) {
 
  // declare local variables
  //
  bool status = true;

  // create test signal only if parameter is not (null)
  //
 if (strcmp(mode_a, "(null)") != 0) {

   // display debugging information
   //
   if (debug_level_d >= LEVEL_DETAILED) {
    fprintf(stdout, "Edf::test_signal(): creating signal [%s]\n", mode_a);
   }

   // loop for all channels
   //
   long nc = sig_a.size();
   double fs_inv = 1.0 / hdr_sample_frequency_d;

   for (long i = 0; i < nc; i++) {
     
     // loop over all samples
     //
     long ns = sig_a[i].size();

    for (long j = 0; j < ns; j++) {

      // case 1: single sinewave
      //
      if (strcmp(mode_a, "single_sinewave") == 0) {

	// compute a single sinewave
	//
	double t = (double)j * fs_inv;
	double arg = 2 * M_PI * EDF_TST_F4 * t;
	sig_a[i][j] = EDF_TST_A4 * sin(arg);
      }

      // case 2: three sinewaves
      //
      else if (strcmp(mode_a, "three_sinewaves") == 0) {

	// compute the arguments of the sine functions
	//
	double t = (double)j * fs_inv;
	double arg1 = 2 * M_PI * EDF_TST_F4 * t;
	double arg2 = 2 * M_PI * EDF_TST_F6 * t;
	double arg3 = 2 * M_PI * EDF_TST_F7 * t;

	// compute the sine and sum
	//
	sig_a[i][j] = EDF_TST_A4 * sin(arg1) +
	  EDF_TST_A6 * sin(arg2) + EDF_TST_A7 * sin(arg3);
      }

      // case 3: six sinewaves
      //
      else if (strcmp(mode_a, "six_sinewaves") == 0) {

	// compute the arguments of the sine functions
	//
	double t = (double)j * fs_inv;
	double arg1 = 2 * M_PI * EDF_TST_F1 * t;
	double arg2 = 2 * M_PI * EDF_TST_F2 * t;
	double arg3 = 2 * M_PI * EDF_TST_F3 * t;
	double arg4 = 2 * M_PI * EDF_TST_F4 * t;
	double arg5 = 2 * M_PI * EDF_TST_F5 * t;
	double arg6 = 2 * M_PI * EDF_TST_F6 * t;

	// compute the sine and sum
	//
	sig_a[i][j] = EDF_TST_A1 * sin(arg1) + EDF_TST_A2 * sin(arg2) +
	  EDF_TST_A3 * sin(arg3) + EDF_TST_A4 * sin(arg4) +
	  EDF_TST_A5 * sin(arg5) + EDF_TST_A6 * sin(arg6);
      }

      // case 4: rectified_sinewave
      //
      else if (strcmp(mode_a, "rectified_sinewave") == 0) {

	// compute the arguments of the sine function
	//
	double t = (double)j * fs_inv;
	double arg1 = 2 * M_PI * EDF_TST_F1 * t;
	double sum = sin(arg1);

	// compute the sine and sum
	//
	if (sum > 0) {
	  sig_a[i][j] = EDF_TST_A1 * sum;
	}
	else {
	  sig_a[i][j] = 0.0;
	}
      }

      // case 5: sine_plus_noise
      //
      else if (strcmp(mode_a, "sine_plus_noise") == 0) {

	// compute a single sinewave plus noise
	//
	double t = (double)j * fs_inv;
	double arg = 2 * M_PI * EDF_TST_F4 * t;
	sig_a[i][j] = EDF_TST_A4 * (sin(arg) + drand48());
      }

      // else: error
      //
      else {
	status = false;
      }

      // display debugging information
      //
      if ((debug_level_d >= LEVEL_FULL) && (j < DEF_DBG_NS)) {
	fprintf(stdout, "\tsig[%ld][%ld] = %f\n", i, j, sig_a[i][j]);
      }
    }
  }

  // display debugging information
  //
  if (debug_level_d >= LEVEL_DETAILED) {
    fprintf(stdout, "Edf::test_signal(): done creating signal\n");
  }

  // exit gracefully
  //
  return status;
 }

 else {
   
   //exit gracefully
   //
   return status;
 }
}

// method: deidentify
//
// arguments:
//  char* label: used as the subject identifier (input)
//  char* subj: subject id (input)
//  char* sess: session id (input)
//  char* tech: technician id (input)
//
// return: a boolean value indicating status
//
// This method overwrites selected fields in a header.
//
bool Edf::deidentify(char* label_a, char* subj_a, char* sess_a, char* tech_a) {

  // declare local variables
  //
  bool status = true;

  // display debug information
  //
  if (debug_level_d >= LEVEL_FULL) {
    fprintf(stdout, "Edf::deidentify(): starting deidentify\n");

    // display the initial values that we will be changing
    //
    fprintf(stdout, " ghdi_file_type = [%s]\n", hdr_ghdi_file_type_d);
    fprintf(stdout, " lpti_patient_id = [%s]\n", hdr_lpti_patient_id_d);
    fprintf(stdout, " lpti_dob = [%s]\n", hdr_lpti_dob_d);
    fprintf(stdout, " lpti_full_name = [%s]\n", hdr_lpti_full_name_d);
    fprintf(stdout, " lpti_age = [%s]\n", hdr_lpti_age_d);
    fprintf(stdout, " lrci_eeg_id = [%s]\n", hdr_lrci_eeg_id_d);
    fprintf(stdout, " lrci_tech = [%s]\n", hdr_lrci_tech_d);
  }

  // (0) change the file type to generic EDF
  //
  memcpy(hdr_ghdi_file_type_d, EDF_FTYP, EDF_FTYP_BSIZE);

  // (1) clear the patient ID
  //
  strcpy(hdr_lpti_patient_id_d, subj_a);
  Edf::pad_whitespace(hdr_lpti_patient_id_d, EDF_LPTI_TSIZE);

  // (2) convert the dob to the beginning of the year
  //
  strncpy(hdr_lpti_dob_d, "01-JAN", 6);

  // (3) convert the full name
  //
  strcpy(hdr_lpti_full_name_d, subj_a);
  Edf::pad_whitespace(hdr_lpti_patient_id_d, EDF_LPTI_TSIZE);

  // (4) compute the approximate age based on the difference between
  //     the dob and the start date
  //
  char* str1 = rindex(hdr_lpti_dob_d, '-');
  str1++;
  char* str2 = rindex(hdr_lrci_start_date_d, '-');
  str2++;
  long num_years = atoi(str2) - atoi(str1);
  sprintf(hdr_lpti_age_d, "Age:%d", (short)num_years);
  Edf::pad_whitespace(hdr_lpti_age_d, EDF_LPTI_TSIZE);
  
  // (5) replace the eeg_id
  //
  sprintf(hdr_lrci_eeg_id_d, "%s_%s", subj_a, sess_a);
  Edf::pad_whitespace(hdr_lrci_eeg_id_d, EDF_LRCI_TSIZE);
  
  // (6) replace technician
  //
  strcpy(hdr_lrci_tech_d, tech_a);
  Edf::pad_whitespace(hdr_lrci_tech_d, EDF_LRCI_TSIZE);

  // display debug information
  //
  if (debug_level_d >= LEVEL_FULL) {
    fprintf(stdout, "Edf::deidentify(): done with deidentify\n");

    // display the new values
    //
    fprintf(stdout, " ghdi_file_type = [%s]\n", hdr_ghdi_file_type_d);
    fprintf(stdout, " lpti_patient_id = [%s]\n", hdr_lpti_patient_id_d);
    fprintf(stdout, " lpti_dob = [%s]\n", hdr_lpti_dob_d);
    fprintf(stdout, " lpti_full_name = [%s]\n", hdr_lpti_full_name_d);
    fprintf(stdout, " lpti_age = [%s]\n", hdr_lpti_age_d);
    fprintf(stdout, " lrci_eeg_id = [%s]\n", hdr_lrci_eeg_id_d);
    fprintf(stdout, " lrci_tech = [%s]\n", hdr_lrci_tech_d);
  }

  // exit gracefully
  //
  return true;
}

// method: set_start_time
//
// arguments:
//  char* st_time: new start time (input)
//
// return: a boolean value indicating status
//
// This method overwrites the start time for a signal. Note that the time
// should be limited to 8 characters.
//
bool Edf::set_start_time(char* st_time_a) {

  // copy the time
  //
  strcpy(hdr_ghdi_start_time_d, st_time_a);

  // exit gracefully
  //
  return true;
}

// method: increment_start_time
//
// arguments:
//  long num_secs: number of seconds to add to start time
//
// return: a boolean value indicating status
//
// Edf files contain a start time field. This method adds num_secs of time
// to that field. Because the field is stored as a character string,
// a conversion must be done.
//
// Note that this method is limited to a 24-hour clock. It does not
// increment times into the next day.
//
// Note also that the number of seconds is specified as an integer.
//
bool Edf::increment_start_time(long num_secs_a) {

  // convert the current start time to secs
  //
  long hrs, mins, secs;

  sscanf(hdr_ghdi_start_time_d, "%2ld.%2ld.%2ld", &hrs, &mins, &secs);

  // increment secs and limit it to a value in the range [0,60]
  //
  secs += num_secs_a;

  // limit secs to [0, 60]
  //
  while (secs > 59) {
    secs -= 60;
    mins += 1;
  }

  // limit mins to [0, 60]
  //
  while (mins > 59) {
    mins -= 60;
    hrs += 1;
  }

  // limit hrs to [0, 24]
  //
  while (hrs > 24) {
    hrs -= 24;
  }

  // convert back to a string
  //
  sprintf(hdr_ghdi_start_time_d, "%2ld.%2ld.%2ld", hrs, mins, secs);

  // exit gracefully
  //
  return true;
}

// method: set_signal_dimensions
//
// arguments:
//  VVectorDouble& sig: signal (input)
//
// return: a boolean value indicating status
//
// This method copies one signal's parameters into the Edf internal data.
//
bool Edf::set_signal_dimensions(VVectorDouble& sig_a) {

  // resize the output
  //
  long nchan = sig_a.size();

  // update the number of records and the duration:
  //  note that EDF files really want to use a record duration of 1 sec
  //
  hdr_ghdi_nsig_rec_d = nchan;
  hdr_ghdi_num_recs_d = sig_a[0].size() / hdr_sample_frequency_d;
  hdr_ghdi_dur_rec_d = 1;
  long orig_chan0_size = hdr_chan_rec_size_d[0];
  for (long i = 0; i < hdr_ghdi_nsig_rec_d; i++) {
    // only  update the sampling freq. for  different channels
    // if the origianl sampling freq. of the channel is the same as the
    // first channel.
    //
    
    if (hdr_chan_rec_size_d[i] == orig_chan0_size)
      hdr_chan_rec_size_d[i] = hdr_sample_frequency_d;
  }

  // exit gracefully
  //
  return true;
}

// method: copy_signal
//
// arguments:
//  VVectorDouble& sigo: selected signal (output)
//  VVectorDouble& sigi: signal from file (input)
//
// return: a boolean value indicating status
//
// This method copies one signal data structure to another.
//
bool Edf::copy_signal(VVectorDouble& sigo_a, VVectorDouble& sigi_a) {

  // resize the output
  //
  long nchan = sigi_a.size();
  Edf::resize(sigo_a, nchan, false);

  // loop over each channel
  //
  for (long i = 0; i < nchan; i++) {
    Edf::resize(sigo_a[i], sigi_a[i].size(), false);
    sigo_a[i].assign(sigi_a[i]);
  }

  // exit gracefully
  //
  return true;
}

// method: copy_signal
//
// arguments:
//  VVectorDouble& sigo: selected signal (output)
//  long& nlo: the number of labels (output)
//  cha** labelso: the label array (output)
//  VVectorDouble& sigi:  signal from file (input)
//  long nli: the number of labels (input)
//  char** labelsi: the label array (input)
//
// return: a boolean value indicating status
//
// This method copies the signal data from input to output,
// and also copies the channel labels. The latter is done because
// this same routine is used for channel selection and montage processing.
// The arrays are passed as arguments because the source arrays are
// different for channel selection and montage processing.
//
bool Edf::copy_signal(VVectorDouble& sigo_a, long& nlo_a, char** labelso_a,
		      VVectorDouble& sigi_a, long nli_a, char** labelsi_a) {

  // display debug information
  //
  if (debug_level_d >= LEVEL_DETAILED) {
    fprintf(stdout, "Edf::copy_signal(): starting to copy signal\n");
  }
  
  // create output space
  //
  nlo_a = sigi_a.size();
  Edf::resize(sigo_a, nlo_a, false);

  // save all the channels
  //
  for (long i = 0; i < nlo_a; i++) {

    // create space
    //
    long j_end = sigi_a[i].size();
    Edf::resize(sigo_a[i], j_end, false);

    // copy the data
    //
    for (long j = 0; j < j_end; j++) {
      sigo_a[i][j] = sigi_a[i][j];
    }
  }

  // display debug information
  //
  if (debug_level_d >= LEVEL_DETAILED) {
    fprintf(stdout, "Edf::copy_signal(): done copying signal\n");
    fprintf(stdout, "Edf::copy_signal(): starting copying labels\n");
  }
  
  // copy the labels
  //  note that it is assumed that the labels array has
  //  had space allocated for it.
  //
  for (long i = 0; i < nlo_a; i++) {
    Edf::resize(labelso_a[i], strlen(labelsi_a[i]) + 1);
    strcpy(labelso_a[i], labelsi_a[i]);
  }

  // display debug information
  //
  if (debug_level_d >= LEVEL_DETAILED) {
    fprintf(stdout, "Edf::copy_signal(): done copying labels\n");
  }
  
  // exit gracefully
  //
  return true;
}

// method: add_interp_channel
//
// arguments:
//  VVectorDouble& sigo: new signal (output)
//  VVectorDouble& sigi: original signal (input)
//  VectorDouble& chani: new interpolated channels (input)
//  INTERPOLATE_OMODE int_omode: output mode (input)
//  
// return: a boolean value indicating status
//
// This method adds the interpolated channels in the original set of
// channels and updates the header information.
//
// Note that all information regarding the new channels and their adjacent
// channels are stored in protected variables once the parameter file is
// parsed. Therefore, those variables are used in this method to add the new
// channels.
//
// Note also that this method will preserve the original set of channels,
// or copy just the new channels depending on the output mode.
//
bool Edf::add_interp_channel(VVectorDouble& sigo_a, VVectorDouble& sigi_a, 
			     VVectorDouble& chani_a,
			     INTERPOLATE_OMODE int_omode_a) {
  
  // the vector containing the positions of the adjacent_channels:
  //  as the new channel is being interpolated regarding its adjacent
  //  channels, their position in the original signal should be
  //  known. These positions will be used to create the header
  //  specific information related to new channels. 
  //
  long* pos_adj_channels[num_clabels_d];

  // variable containing the old number of channels:
  //  as the interpolation mode outputs can change either to 
  //  new channel, adjacent channels, or all channels, the old 
  //  number of channels might change.
  // 
  long old_num_channels = 0;

  // variable conatining the new number of channels
  //  as the interpolation mode outputs can change either to 
  //  new channel, adjacent channels, or all channels, the new 
  //  number of channels might change as well.
  //
  long new_num_channels = 0;

  // create output space to store the new set of channels:
  //  these vectors will work as auxiliary variables to 
  //  update the header.
  //  
  char* new_chan_labels[num_clabels_d];
  char* new_chan_trans_type[num_clabels_d];
  char* new_chan_phys_dim[num_clabels_d];
  double new_chan_phys_min[num_clabels_d];
  double new_chan_phys_max[num_clabels_d];
  long new_chan_dig_min[num_clabels_d];
  long new_chan_dig_max[num_clabels_d];
  char* new_chan_prefilt[num_clabels_d];
  long new_chan_rec_size[num_clabels_d];  

  // display debug information
  //
  if (debug_level_d >= LEVEL_DETAILED) {
    fprintf(stdout,
	    "Edf::add_channel(): beginning addition of new channels\n");
  }

  // loop over new channels numbers and take the position of its
  // adjacent channels one by one
  //
  for (long i = 0; i < num_clabels_d; i++) {

    // number of adjacent channels related to
    // new channel i
    //
    long nadj_chans = num_adj_chan_d[i];
    
    // resize columns of position to the number of adjacent channels
    // related to new_channel i
    //
    pos_adj_channels[i] = new long[nadj_chans];
    
    // loop over adjacent channels of new channel i
    //
    for (long j = 0; j < nadj_chans; j++) {

      // find the corresponding channel in the input signal
      //
      long pos;
      
      // match mode:
      //  smmode_d has the last value assigned to it. That means when
      //  interpolation is performed, the actual match_mode might
      //  be saved. 
      //  
      if ((pos = Edf::find_match(adj_chan_labels_d[i][j],
			         hdr_ghdi_nsig_rec_d, hdr_chan_labels_d,
			         smmode_d)) < 0) {
        fprintf(stdout,
	        "**> Edf::add_channel(): no match for [%s]\n",
	        adj_chan_labels_d[i][j]);

        return false;
      }

      // take position of adjacent channel j in the original signal 
      //
      pos_adj_channels[i][j] = pos;
    } 
  }  

  // case 1: preserve all channels including the interpolated channels:
  //  in this case, all the information regarding the original signal
  //  should be preserved, including its channels discrete points and
  //  the header specific information.
  //
  if (int_omode_a == INTOMODE_CONCAT) {

    // save number of channels preserved
    //
    old_num_channels = Edf::get_num_channels_file();
  }

  // case 2: preserve just the interpolated channel:
  //  in this case, just the new channel will be preserved and the specific 
  //  information stored will be from any of its adjacent channels.
  //
  else if (int_omode_a == INTOMODE_REPLACE) {

    // save number of channels preserved
    //
    old_num_channels = 0;
  }
  
  // case 3: unknown
  //
  else {
    fprintf(stdout, "**> Edf::add_channel(): unknown output mode \n");
  }  

  // new number of channels
  //
  new_num_channels = old_num_channels + num_clabels_d;
    
  // resize output signal to the final size
  //  the output signal rows might be resized to the number of new channels
  //  since the last row will represent the new channel. 
  //
  Edf::resize(sigo_a, new_num_channels, false);   

  // add new channel's information to new variables:  
  //  update header information related to the new channel.
  //  it copies the header information of one of the adjacent channels
  //  since the new channel is the interpolation of them.
  //
  for (long i = 0; i < num_clabels_d; i++) {    
    new_chan_labels[i] = new char[strlen(new_chan_labels_d[i]) + 1];
    strcpy(new_chan_labels[i], new_chan_labels_d[i]);

    new_chan_trans_type[i] =
    new char[strlen(hdr_chan_trans_type_d[pos_adj_channels[i][0]]) + 1];
    strcpy(new_chan_trans_type[i],
	   hdr_chan_trans_type_d[pos_adj_channels[i][0]]); 

    new_chan_phys_dim[i] =
    new char[strlen(hdr_chan_phys_dim_d[pos_adj_channels[i][0]]) + 1];
    strcpy(new_chan_phys_dim[i], hdr_chan_phys_dim_d[pos_adj_channels[i][0]]); 

    new_chan_phys_min[i] = hdr_chan_phys_min_d[pos_adj_channels[i][0]];
    new_chan_phys_max[i] = hdr_chan_phys_max_d[pos_adj_channels[i][0]];
    new_chan_dig_min[i] = hdr_chan_dig_min_d[pos_adj_channels[i][0]];
    new_chan_dig_max[i] = hdr_chan_dig_max_d[pos_adj_channels[i][0]];
    new_chan_dig_max[i] = hdr_chan_dig_max_d[pos_adj_channels[i][0]];

    new_chan_prefilt[i] =
    new char[strlen(hdr_chan_prefilt_d[pos_adj_channels[i][0]]) + 1];
    strcpy(new_chan_prefilt[i], hdr_chan_prefilt_d[pos_adj_channels[i][0]]);

    new_chan_rec_size[i] = hdr_chan_rec_size_d[pos_adj_channels[i][0]];
  }
    
  // save the signal data from the original channels
  //
  for (long i = 0; i < old_num_channels; i++) {
    // create output space
    //
    long j_end = sigi_a[i].size();
    Edf::resize(sigo_a[i], j_end, false);

    // copy the data
    //
    for (long j = 0; j < j_end; j++) {
      sigo_a[i][j] = sigi_a[i][j];
    }       
  }

  // save new channel's information:   
  //  update header information related to the new channel.
  //  it copies the header information of one of its  adjacent
  //  channels since the new channel is the interpolation of them.
  //
  for (long i = old_num_channels; i < new_num_channels; i++) {
    Edf::resize(hdr_chan_labels_d[i],
    	        strlen(new_chan_labels[i - old_num_channels]) + 1, false);
    strcpy(hdr_chan_labels_d[i],new_chan_labels[i - old_num_channels]);

    Edf::resize(hdr_chan_trans_type_d[i],
		strlen(new_chan_trans_type[i - old_num_channels]) + 1, false);
    strcpy(hdr_chan_trans_type_d[i],new_chan_trans_type[i - old_num_channels]);


    Edf::resize(hdr_chan_phys_dim_d[i],
    	        strlen(new_chan_phys_dim[i - old_num_channels]) + 1, false);
    strcpy(hdr_chan_phys_dim_d[i],new_chan_phys_dim[i - old_num_channels]);

    hdr_chan_phys_min_d[i] = new_chan_phys_min[i - old_num_channels]; 
    hdr_chan_phys_max_d[i] = new_chan_phys_max[i - old_num_channels]; 
    hdr_chan_dig_min_d[i] = new_chan_dig_min[i - old_num_channels]; 
    hdr_chan_dig_max_d[i] = new_chan_dig_max[i - old_num_channels]; 

    Edf::resize(hdr_chan_prefilt_d[i],
    	        strlen(new_chan_prefilt[i - old_num_channels]) + 1, false);
    strcpy(hdr_chan_prefilt_d[i],new_chan_prefilt[i - old_num_channels]);

    hdr_chan_rec_size_d[i] = new_chan_rec_size[i - old_num_channels];

    // add the new channel's signal data:
    // create output space to hold size of each new channel
    //
    long j_end = chani_a[i - old_num_channels].size();
    Edf::resize(sigo_a[i], j_end, false);
    
    // copy the data
    //
    for (long j = 0; j < j_end; j++) {
      sigo_a[i][j] = chani_a[i - old_num_channels][j];
    }
  }

  // clean up memory
  //
  Edf::cleanup(new_chan_labels, num_clabels_d);
  Edf::cleanup(new_chan_trans_type, num_clabels_d);
  Edf::cleanup(new_chan_phys_dim, num_clabels_d);
  Edf::cleanup(new_chan_prefilt, num_clabels_d);

  // loop over number of new channels and clean memory allocated
  // to position of adjacent channels 
  //
  for (long i = 0; i < num_clabels_d; i++) {
    delete [] pos_adj_channels[i];
  }

  // update the number of channels and the header size 
  //
  hdr_ghdi_nsig_rec_d = new_num_channels;
  hdr_ghdi_hsize_d = compute_header_size(hdr_ghdi_nsig_rec_d);

  // display debug information
  //
  if (debug_level_d >= LEVEL_DETAILED) {
    fprintf(stdout, 
            "Edf::add_channel(): done with addition of new channels\n");
  }
  
  // exit gracefully
  //
  return true;
}

// method: interpolate
//
// arguments:
//  VVectorDouble& sigo: new set of channels (input/output)
//  MATCH_MODE match_mode: match mode (input)
//  INTERPOLATE_MODE mode: interpolate mode (input)
//  INTERPOLATE_OMODE omode: output mode (input)
//  
// return: a boolean value indicating status
//
// This method branches to the interpolation mode selected by the
// parameter file and add the interpolated channels signal.
//
// Note that this method will update the header specific information.
//
bool Edf::interpolate(VVectorDouble& sigo_a,
		      MATCH_MODE match_mode_a,
		      INTERPOLATE_MODE mode_a,
		      INTERPOLATE_OMODE omode_a) {

  // declare local variables
  //
  VVectorDouble sig_t;
  bool status = false;

  // save match_mode
  //
  smmode_d = match_mode_a;
  
  // variable to store the new_channels discrete points
  //
  VVectorDouble new_channels;

  // display a debug message
  //
  if (debug_level_d >= Edf::LEVEL_DETAILED) {
    fprintf(stdout,
	    "Edf::interpolate(): beginning interpolation\n");
  }

  // branch on the interpolate mode
  //
  // case 1: interpolation by average
  //
  if (mode_a == INTMODE_AVERAGE) {

    // display debug information
    //
    if (debug_level_d >= Edf::LEVEL_DETAILED) {
      fprintf(stdout, " Edf::interpolate(): average mode (%lu)\n",
	      (long)mode_a);
    }
    if (!(status = interpolate_average(new_channels, sigo_a))) {
      return status;
    }
  }
    
  // case 2: unknown
  //
  else {
    fprintf(stdout,
	    "**> Edf::interpolate(): unknown mode \n");
  }

  // return the new signal
  //
  // copy the original signal:
  //  make a copy of the original signal to add the
  //  interpolated channels into the new signal.
  //
  if (!(status = Edf::copy_signal(sig_t, sigo_a))) {
    return status;
  }
  
  // add interpolated channels into the original set of channels
  //
  if (!(status = Edf::add_interp_channel(sigo_a, sig_t,
					 new_channels, omode_a))) {
    return status;
  }    

  // display a debug message
  //
  if (debug_level_d >= Edf::LEVEL_FULL) {
    fprintf(stdout,
	    "Edf::interpolate(): original signal: %ld channels\n",
	    (long)sig_t.size());
    fprintf(stdout,
	    "Edf::interpolate(): new signal: %ld channels\n",
	    (long)sigo_a.size());
  }
  if (debug_level_d >= Edf::LEVEL_DETAILED) {
    fprintf(stdout, "Edf::interpolate(): done interpolating channels\n");
  }
  
  // exit gracefully
  //
  return status;
}

//
// end of file

