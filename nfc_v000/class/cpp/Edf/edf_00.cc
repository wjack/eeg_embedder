// file: $(NEDC_NFC)/class/cpp/Edf/edf_00.cc
//
// This file contains basic required methods such as constructors
// and destructors.
//

// Revision History
//  20160615 (JM): addition of FFMT_NAME_03 definition for Kaldi files
//
//  20150630 (FG): interpolation moved to Edf class
//
//  20150629 (GS): added set methods for parsing code
//                 methods added: set_interp_chans
//                                set_montage_chans
//                                parse_aux
//
//  20150612 (FG): added interpolation functionality
//

// local include files
//
#include "Edf.h"

//-----------------------------------------------------------------------------
//
// basic required methods
//
//-----------------------------------------------------------------------------

// method: default constructor
//
// arguments:
//  long debug_level: the debug level (input)
//
// return: none
//
// This method implements the detaul constructor for the Fe class.
//
Edf::Edf(long debug_level_a) {

  // set the debug level
  //
  debug_level_d = debug_level_a;
  verbosity_d = DEF_LEVEL;

  // display debugging information
  //
  if (debug_level_d >= LEVEL_FULL) {
    fprintf(stdout, "Edf(): initalizing an EDF object\n");
  }

  // initialize variables
  //
  fn_d = (char*)NULL;
  fp_d = (FILE*)NULL;

  // initialize variables related to EDF header processing
  //
  // (1) contains the version of the file
  //
  if (debug_level_d >= LEVEL_FULL) {
    fprintf(stdout, "Edf(): initalizing (1)\n");
  }

  hdr_version_d[0] = (char)NULL;

  // (2) contains local patient information (lpti)
  //
  if (debug_level_d >= LEVEL_FULL) {
    fprintf(stdout, "Edf(): initalizing (2)\n");
  }

  hdr_lpti_patient_id_d[0] = (char)NULL;
  hdr_lpti_gender_d[0] = (char)NULL;
  hdr_lpti_dob_d[0] = (char)NULL;
  hdr_lpti_full_name_d[0] = (char)NULL;
  hdr_lpti_age_d[0] = (char)NULL;

  // (3) contains local recording information (lrci)
  //
  if (debug_level_d >= LEVEL_FULL) {
    fprintf(stdout, "Edf(): initalizing (3)\n");
  }

  hdr_lrci_start_date_label_d[0] = (char)NULL;
  hdr_lrci_start_date_d[0] = (char)NULL;
  hdr_lrci_eeg_id_d[0] = (char)NULL;
  hdr_lrci_tech_d[0] = (char)NULL;
  hdr_lrci_machine_d[0] = (char)NULL;

  // (4) contains general header information (ghdi)
  //
  if (debug_level_d >= LEVEL_FULL) {
    fprintf(stdout, "Edf(): initalizing (4)\n");
  }

  hdr_ghdi_start_date_d[0] = (char)NULL;
  hdr_ghdi_start_time_d[0] = (char)NULL;
  hdr_ghdi_hsize_d = -1;
  hdr_ghdi_file_type_d[0] = (char)NULL;
  hdr_ghdi_reserved_d[0] = (char)NULL;
  hdr_ghdi_num_recs_d = -1;
  hdr_ghdi_dur_rec_d = -1;
  hdr_ghdi_nsig_rec_d = -1;

  // (5) contains channel-specific data
  //
  if (debug_level_d >= LEVEL_FULL) {
    fprintf(stdout, "Edf(): initalizing (5)\n");
  }

  for (long i = 0; i < MAX_NCHANS; i++) {
    hdr_chan_labels_d[i] = (char*)NULL;
    hdr_chan_trans_type_d[i] = (char*)NULL;
    hdr_chan_phys_dim_d[i] = (char*)NULL;
    hdr_chan_phys_min_d[i] = 0;
    hdr_chan_phys_max_d[i] = 0;
    hdr_chan_dig_min_d[i] = 0;
    hdr_chan_dig_max_d[i] = 0;
    hdr_chan_prefilt_d[i] = (char*)NULL;
    hdr_chan_rec_size_d[i] = 0;
  }

  // variables related to interpolation
  //
  num_clabels_d = -1;
  
  // variable containing new labels
  // related to interpolation.
  //
  for (long i = 0; i < MAX_NCHANS; i++) {
    new_chan_labels_d[i] = (char*)NULL;
    adj_chan_labels_d[i] = (char**)NULL;
  }

  // (6) define some derived values
  //
  if (debug_level_d >= LEVEL_FULL) {
    fprintf(stdout, "Edf(): initalizing (6)\n");
  }

  hdr_sample_frequency_d = -1;
  hdr_num_channels_proc_d = -1;
  hdr_num_channels_signal_d = -1;
  hdr_num_channels_annotation_d = -1;

  // (7) additional derived values
  //
  if (debug_level_d >= LEVEL_FULL) {
    fprintf(stdout, "Edf(): initalizing (7)\n");
  }

  smmode_d = DEF_MATCH_MODE;
  mmmode_d = DEF_MATCH_MODE;  
  num_slabels_d = -1;
  num_mlabels_d = -1;

  for (long i = 0; i < MAX_NCHANS; i++) {
    mchan_d[i] = (char*)NULL;
    slabels_d[i] = (char*)NULL;
    mlabels1_d[i] = (char*)NULL;
    mlabels2_d[i] = (char*)NULL;
  }

  // display debugging information
  //
  if (debug_level_d >= LEVEL_FULL) {
    fprintf(stdout, "Edf(): done initalizing an EDF object\n");
  }

  // htk-related parameters
  //
  fdur_d = DEF_FDUR;
  strcpy(fnmod_d, DEF_FNMOD);

  // exit gracefully
  //
};

// method: destructor
//
// arguments: none
//
// return: none
//
// This method implements the destructor.
//
Edf::~Edf() {

  // display debugging information
  //
  if (debug_level_d >= LEVEL_FULL) {
    fprintf(stdout, "~Edf(): begin destroying an EDF object\n");
  }

  // clean up memory
  //
  Edf::cleanup();

  // display debugging information
  //
  if (debug_level_d >= LEVEL_FULL) {
    fprintf(stdout, "~Edf(): end of destroying an EDF object\n");
  }

  // exit gracefully
  //
}

// method: resize
// 
// arguments:
//  char*& v: vector to be resized (input)
//  long size: new size (input)
//  bool preserve: if true, retain existing values (input)
//
// return: a logical value indicating status
//
bool Edf::resize(char*& v_a, long size_a, bool preserve_a) {

  // save the old pointer
  //
  char* old_v = v_a;

  // allocate new memory
  //
  v_a = new char[size_a];

  // copy data if necessary
  //
  if (preserve_a == true) {
    long copy_size = Edf::min(size_a, strlen(old_v));
    memcpy(v_a, old_v, copy_size);
  }

  // if non-null, delete memory
  //
  if (old_v != (char*)NULL) {
    delete [] old_v;
  }

  // exit gracefully
  //
  return true;
}

// method: resize
// 
// arguments:
//  VectorBool& v: vector to be resized (input)
//  long size: new size (input)
//  bool preserve: if true, save existing values (input)
//
// return: a logical value indicating status
//
// Because Boost vectors error when you call resize with the same
// size, this stupid method simply checks whether the vector needs
// to be resized and does so if needed.
//
bool Edf::resize(VectorBool& v_a, long size_a, bool preserve_a) {

  // check the size
  //
  if (v_a.size() != size_a) {
    v_a.resize(size_a, preserve_a);
  }

  // exit gracefully
  //
  return true;
}

// method: resize
// 
// arguments:
//  VectorLong& v: vector to be resized (input)
//  long size: new size (input)
//  bool preserve: if true, save existing values (input)
//
// return: a logical value indicating status
//
// Because Boost vectors error when you call resize with the same
// size, this stupid method simply checks whether the vector needs
// to be resized and does so if needed.
//
bool Edf::resize(VectorLong& v_a, long size_a, bool preserve_a) {

  // check the size
  //
  if (v_a.size() != size_a) {
    v_a.resize(size_a, preserve_a);
  }

  // exit gracefully
  //
  return true;
}

// method: resize
// 
// arguments:
//  VectorDouble& v: vector to be resized (input)
//  long size: new size (input)
//  bool preserve: if true, save existing values (input)
//
// return: a logical value indicating status
//
// Because Boost vectors error when you call resize with the same
// size, this stupid method simply checks whether the vector needs
// to be resized and does so if needed.
//
bool Edf::resize(VectorDouble& v_a, long size_a, bool preserve_a) {

  // check the size
  //
  if (v_a.size() != size_a) {
    v_a.resize(size_a, preserve_a);
  }

  // exit gracefully
  //
  return true;
}

// method: resize
// 
// arguments:
//  VVectorLong& v: vector to be resized (input)
//  long size: new size (input)
//  bool preserve: if true, save existing values (input)
//
// return: a logical value indicating status
//
// Because Boost vectors error when you call resize with the same
// size, this stupid method simply checks whether the vector needs
// to be resized and does so if needed.
//
bool Edf::resize(VVectorLong& v_a, long size_a, bool preserve_a) {

  // check the size
  //
  if (v_a.size() != size_a) {
    v_a.resize(size_a, preserve_a);
  }

  // exit gracefully
  //
  return true;
}

// method: resize
// 
// arguments:
//  VVectorDouble& v: vector to be resized (input)
//  long size: new size (input)
//  bool preserve: if true, save existing values (input)
//
// return: a logical value indicating status
//
// Because Boost vectors error when you call resize with the same
// size, this stupid method simply checks whether the vector needs
// to be resized and does so if needed.
//
bool Edf::resize(VVectorDouble& v_a, long size_a, bool preserve_a) {

  // check the size
  //
  if (v_a.size() != size_a) {
    v_a.resize(size_a, preserve_a);
  }

  // exit gracefully
  //
  return true;
}

// method: resize
// 
// arguments:
//  VVVVectorDouble& v: vector to be resized (input)
//  long size: new size (input)
//  bool preserve: if true, save existing values (input)
//
// return: a logical value indicating status
//
// Because Boost vectors error when you call resize with the same
// size, this stupid method simply checks whether the vector needs
// to be resized and does so if needed.
//
bool Edf::resize(VVVectorDouble& v_a, long size_a, bool preserve_a) {

  // check the size
  //
  if (v_a.size() != size_a) {
    v_a.resize(size_a, preserve_a);
  }

  // exit gracefully
  //
  return true;
}

// method: resize
// 
// arguments:
//  MatrixDouble& m: matrix to be resized (input)
//  long size1: new number of rows (input)
//  long size2: new number of cols (input)
//  bool preserve: if true, save existing values (input)
//
// return: a logical value indicating status
//
// Because Boost vectors and matrices error when you call resize with the same
// size, this stupid method simply checks whether the vector needs
// to be resized and does so if needed.
//
bool Edf::resize(MatrixDouble& m_a, long size1_a, long size2_a,
		 bool preserve_a) {

  // check the size
  //
  if ((m_a.size1() != size1_a) || (m_a.size2() != size2_a)) {
    m_a.resize(size1_a, size2_a, preserve_a);
  }

  // exit gracefully
  //
  return true;
}

// method: resize
// 
// arguments:
//  VMatrixDouble& m: matrix to be resized (input)
//  long size: new size (input)
//  bool preserve: if true, save existing values (input)
//
// return: a logical value indicating status
//
// Because Boost vectors and matices error when you call resize with the same
// size, this stupid method simply checks whether the vector needs
// to be resized and does so if needed.
//
bool Edf::resize(VMatrixDouble& m_a, long size_a, bool preserve_a) {

  // check the size
  //
  if (m_a.size() != size_a) {
    m_a.resize(size_a, preserve_a);
  }

  // exit gracefully
  //
  return true;
}

// method: cleanup
//
// arguments: none
//
// return: a boolean value indicating status
//
// This method deletes memory allocated during processing.
//
bool Edf::cleanup() {

  // display debug information
  //
  if (debug_level_d >= LEVEL_FULL) {
    fprintf(stdout, "Edf::cleanup(): starting clean up of memory\n");
  }

  // clear space for filename storage
  //
  if (debug_level_d >= LEVEL_FULL) {
    fprintf(stdout, "Edf::cleanup(): filename storage\n");
  }

  if (fn_d != (char*)NULL) {
    delete [] fn_d;
    fn_d = (char*)NULL;
  }
  fp_d = (FILE*)NULL;

  // clear space for labels
  //
  if (debug_level_d >= LEVEL_FULL) {
    fprintf(stdout, "Edf::cleanup(): labels\n");
  }

  for (int i = 0; i < MAX_NCHANS; i++) {
    if (hdr_chan_labels_d[i] != (char*)NULL) {
      delete [] hdr_chan_labels_d[i];
      hdr_chan_labels_d[i] = (char*)NULL;
    }
  }

  // clear space for chan_trans_type
  //
  if (debug_level_d >= LEVEL_FULL) {
    fprintf(stdout, "Edf::cleanup(): chan_trans_type\n");
  }

  for (int i = 0; i < hdr_ghdi_nsig_rec_d; i++) {
    if (hdr_chan_trans_type_d[i] != (char*)NULL) {
      delete [] hdr_chan_trans_type_d[i];
      hdr_chan_trans_type_d[i] = (char*)NULL;
    }
  }

  // clear space for chan_phys_dim
  //
  if (debug_level_d >= LEVEL_FULL) {
    fprintf(stdout, "Edf::cleanup(): chan_phys_dim\n");
  }

  for (int i = 0; i < MAX_NCHANS; i++) {
    if (hdr_chan_phys_dim_d[i] != (char*)NULL) {
      delete [] hdr_chan_phys_dim_d[i];
      hdr_chan_phys_dim_d[i] = (char*)NULL;
    }
  }

  // clear space for chan_prefilt
  //
  if (debug_level_d >= LEVEL_FULL) {
    fprintf(stdout, "Edf::cleanup(): chan_prefilt\n");
  }

  for (int i = 0; i < MAX_NCHANS; i++) {
    if (hdr_chan_prefilt_d[i] != (char*)NULL) {
      delete [] hdr_chan_prefilt_d[i];
      hdr_chan_prefilt_d[i] = (char*)NULL;
    }
  }

  // clear space for adj_chan_labels_d
  //
  if (debug_level_d >= LEVEL_FULL) {
    fprintf(stdout, "Edf::cleanup(): adj_channel_labels_d\n");
  }

  for (int i = 0; i < MAX_NCHANS; i++) {
    if (adj_chan_labels_d[i] != (char**)NULL) {
      delete [] adj_chan_labels_d[i];
      adj_chan_labels_d[i] = (char**)NULL;
    }
  }
    
  // clear space for new_chan_labels_d
  //
  if (debug_level_d >= LEVEL_FULL) {
    fprintf(stdout, "Edf::cleanup(): new_channel_label_d\n");
  }

  for (int i = 0; i < MAX_NCHANS; i++) {
    if (new_chan_labels_d[i] != (char*)NULL) {
      delete [] new_chan_labels_d[i];
      new_chan_labels_d[i] = (char*)NULL;
    }
  }

  // clear space for selected and montage labels
  //
  if (debug_level_d >= LEVEL_FULL) {
    fprintf(stdout, "Edf::cleanup(): selection and montage labels\n");
  }
  cleanup_labels();

  // display debug information
  //
  if (debug_level_d >= LEVEL_FULL) {
    fprintf(stdout, "Edf::cleanup(): done cleaning up memory\n");
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
bool Edf::cleanup(char** strs_a, long num_strs_a) {

  // loop over the elements and delete them
  //
  for (long i = 0; i < num_strs_a; i++) {
    delete [] strs_a[i];
    strs_a[i] = (char*)NULL;
  }

  // exit gracefully
  //
  return true;
}

// method: cleanup_labels
//
// arguments: none
//
// return: a boolean value indicating status
//
// This method deletes memory allocated for label processing.
// We provide this method because labels must be processed with
// each input file, so memory is cleaned frequently.
//
bool Edf::cleanup_labels() {

  // display a debug message
  //
  if (debug_level_d >= LEVEL_FULL) {
    fprintf(stdout, "Edf::cleanup_labels(): starting to clean up labels\n");
  }

  // clean up labels
  //
  for (long i = 0; i < MAX_NCHANS; i++) {
    if (mchan_d[i] != (char*)NULL) {
      delete [] mchan_d[i];
      mchan_d[i] = (char*)NULL;
    }
    if (slabels_d[i] != (char*)NULL) {
      delete [] slabels_d[i];
      slabels_d[i] = (char*)NULL;
    }
    if (mlabels1_d[i] != (char*)NULL) {
      delete [] mlabels1_d[i];
      delete [] mlabels2_d[i];
      mlabels1_d[i] = (char*)NULL;
      mlabels2_d[i] = (char*)NULL;
    }
  }

  // reset counters
  //
  num_slabels_d = -1;
  num_mlabels_d = -1;
  
  // display a debug message
  //
  if (debug_level_d >= LEVEL_FULL) {
    fprintf(stdout, "Edf::cleanup_labels(): done cleaning up labels\n");
  }

  // exit gracefully
  //
  return true;
}

// method: convert_debug_from_string
//
// arguments:
//  long& var: the integer value (output)
//  char* str: the string value (input)
//
// return: a logical value indicating status
//
// This method converts a text string to a numeric value. It is a very
// fragile, hard-coded method, and is only used to parse the parameter file.
//
bool Edf::convert_debug_from_string(long& val_a, char* str_a) {

  // check the type
  //
  if (strcmp(str_a, LEVEL_NONE_NAME) == 0) {
    val_a = LEVEL_NONE;
  }
  else if (strcmp(str_a, LEVEL_BRIEF_NAME) == 0) {
    val_a = LEVEL_BRIEF;
  }
  else if (strcmp(str_a, LEVEL_SHORT_NAME) == 0) {
    val_a = LEVEL_SHORT;
  }
  else if (strcmp(str_a, LEVEL_MEDIUM_NAME) == 0) {
    val_a = LEVEL_MEDIUM;
  }
  else if (strcmp(str_a, LEVEL_DETAILED_NAME) == 0) {
    val_a = LEVEL_DETAILED;
  }
  else if (strcmp(str_a, LEVEL_FULL_NAME) == 0) {
    val_a = LEVEL_FULL;
  }
  else {
    return false;
  }

  // exit gracefully
  //
  return true;
}

// method: get_nvp
//
// arguments:
//  char* name:  the name (output)
//  char* value: the value (output)
//
// return: a boolean indicating status
//
// This method parses a line assumed to be in the form of a name/value
// pair. The method returns two arguments - things to the left of the equal
// and things to the right of equal:
//
//  [name] = [value units]
//
// Note that things to right start at the first non-whitespace character.
//
bool Edf::get_nvp(char* name_a, char* value_a, char* buf_a) {

  // locate the delimiter
  //
  char* ptr = strstr(buf_a, VALUE_DELIMITER);
  if (ptr == (char*)NULL) {
    return false;
  }

  // strip the variable name
  //
  char tmp_buf[MAX_LSTR_LENGTH];
  long len = (long)(ptr - buf_a);
  strncpy(tmp_buf, buf_a, len);
  tmp_buf[len] = (char)NULL;
  sscanf(tmp_buf, "%s", name_a);

  // strip leading whitespace
  //
  ptr++;
  while (isspace(*ptr)) {
    ptr++;
  }

  // compute the length
  //
  len = (long)(buf_a + strlen(buf_a) - ptr);
  strncpy(value_a, ptr, len);
  value_a[len] = (char)NULL;

  // exit gracefully
  //
  return true;
}

// method: find_vname
//
// arguments:
//  char* name:  the name (input)
//  char** vnames: a null-terminated list of names (input)
//
// return: a long containing the location of the variable name
//
// This method locates the position of a variable name in the
// variable name list.
//
long Edf::find_vname(char* name_a, char** vnames_a) {

  // loop over all names
  //
  long i = 0;

  while (vnames_a[i] != (char*)NULL) {
    if (strcmp(vnames_a[i], name_a) == 0) {
      return i;
    }
    i++;
  }

  // exit (un)gracefully: name not found
  //
  return -1;
}

// method: set_var
//
// arguments:
//  long pos: the position of the variable (input)
//  char* value: the value as a character string (input)
//  char** vtypes: a list of variable types (input)
//  void** vptrs: a list of void pointers to variables (input)
//
// return: a logical value indicating status
//
// This method set a variable from its position in the var name list.
// This method is a little tricky because it is using void pointers
// to do the heavy lifting.
//
bool Edf::set_var(long pos_a, char* value_a, char** vtypes_a, void** vptrs_a) {

  // check the type
  //
  if (strcmp(vtypes_a[pos_a], VTYPE_NAME_00) == 0) {
    strcpy((char*)vptrs_a[pos_a], value_a);
  }
  else if (strcmp(vtypes_a[pos_a], VTYPE_NAME_01) == 0) {
    sscanf(value_a, "%ld", (long*)vptrs_a[pos_a]);
  }
  else if (strcmp(vtypes_a[pos_a], VTYPE_NAME_02) == 0) {
    sscanf(value_a, "%f", (float*)vptrs_a[pos_a]);
  }
  else if (strcmp(vtypes_a[pos_a], VTYPE_NAME_03) == 0) {
    sscanf(value_a, "%lf", (double*)vptrs_a[pos_a]);
  }
  else {
    return false;
  }

  // exit gracefully
  //
  return true;
}

// method: set_interp_chans
//
// arguments:
//  long new_channels_a: the number of new channels to interpolate (input)
//
// return: a logical value indicating status
//
// This method sets the number of channels to be interpolated.
//      
bool Edf::set_interp_chans(long new_channels_a) {
  num_clabels_d = new_channels_a;
  if (new_channels_a == 0) {
    return false;
  }
  else {
    return true;
  }
}

// method: set_montage_chans
//
// arguments:
//  long num_montage_a: the number of montage channels (input)
//
// return: a logical value indicating status
//
// This method sets the number of channels to be computed using a montage.
// 
bool Edf::set_montage_chans(long num_montage_a) {
  num_mlabels_d = num_montage_a;
  if (num_montage_a == 0) {
    return false;
  }
  else {
    return true;
  }
}

// method: parse_aux
//
// arguments:
//  char** str_a: the montage and interpolation information (input)
//
// return: a logical value indicating status
//
// This method sets the variables mlabels1_d, mlabels2_d, mchan_d, chan_num_d.
// It parses the montage and interpolation information.
//                
bool Edf::parse_aux(char** strm_a, char** stri_a) {

  // declare local variables
  //
  long num_chans, num_new_chans;

  // parse the montage information
  //
  
  bool status = parse_montage(num_chans, mlabels1_d, mlabels2_d, mchan_d,
			      chan_num_d, strm_a);
  
  if (!status) {
    fprintf(stdout,
	    "Edf::parse_aux(): could not parse montage auxiliary information\n");
    return false;
  }

  // parse the interpolation information
  //
  status = parse_interp_channels(num_new_chans, adj_chan_labels_d,
			       num_adj_chan_d, new_chan_labels_d, stri_a);
  if (!status) {
    fprintf(stdout,
	    "Edf::parse_aux(): could not parse auxiliary interpolation information\n");
    return false;
  }

  // exit gracefully
  //
  return status;
}

//-----------------------------------------------------------------------------
//
// we define non-integral constants in the default constructor
//
//-----------------------------------------------------------------------------

// constants: class name
//
const char* Edf::CLASS_NAME("Edf");

// constants: debug level
//
long Edf::debug_level_d = Edf::DEF_LEVEL;
long Edf::verbosity_d = Edf::DEF_LEVEL;

// constants: parameter file parsing
//
const char* Edf::NULL_NAME("(null)");
const char* Edf::VALUE_DELIMITER("=");
const char* Edf::DEF_FNMOD("_ch%03d");

const char* Edf::SPACE = " ";
const char* Edf::SLASH = "/";
const char* Edf::DOT = ".";
const char* Edf::COMMA = ",";
const char* Edf::COMMENT = "#";
const char* Edf::DASHDASH = "--";
const char* Edf::ANNOTATION = "Annotation";
const char* Edf::COLON = ":";

// post-processing exceptions related parameters
//
const char* Edf::ENERGY("ENERGY_NO_DELTA-DELTA");
const char* Edf::CEPSTRAL("CEPSTRAL_NO_DELTA-DELTA");
const char* Edf::MINMAX_DIFF("DMINMAX_EGY_NO_DELTA-DELTA");

// constants: variable type names
//
const char* Edf::VTYPE_NAME_00("string");
const char* Edf::VTYPE_NAME_01("long");
const char* Edf::VTYPE_NAME_02("float");
const char* Edf::VTYPE_NAME_03("double");

// constants: EDF header
//
const char* Edf::EDF_VERS("0       ");
const char* Edf::EDF_FTYP("EDF  ");

// constants: Kaldi header
//
const char* Edf::KALDI_HEADER_STR("BFM");

// constants: debug / verbosity levels
//
const char* Edf::LEVEL_NONE_NAME("none");
const char* Edf::LEVEL_BRIEF_NAME("brief");
const char* Edf::LEVEL_SHORT_NAME("short");
const char* Edf::LEVEL_MEDIUM_NAME("medium");
const char* Edf::LEVEL_DETAILED_NAME("detailed");
const char* Edf::LEVEL_FULL_NAME("full");

// constants: file format names
//
const char* Edf::FFMT_NAME_00("edf");
const char* Edf::FFMT_NAME_01("raw");
const char* Edf::FFMT_NAME_02("htk");
const char* Edf::FFMT_NAME_03("kaldi");
const char* Edf::DEF_FFMT_NAME(Edf::FFMT_NAME_00);

// constants: test signal related
//
double Edf::EDF_TST_F1 = 1.0;
double Edf::EDF_TST_A1 = 10.0;
double Edf::EDF_TST_F2 = 2.0;
double Edf::EDF_TST_A2 = 10.0;
double Edf::EDF_TST_F3 = 3.0;
double Edf::EDF_TST_A3 = 10.0;
double Edf::EDF_TST_F4 = 4.0;
double Edf::EDF_TST_A4 = 10.0;
double Edf::EDF_TST_F5 = 5.0;
double Edf::EDF_TST_A5 = 10.0;
double Edf::EDF_TST_F6 = 8.0;
double Edf::EDF_TST_A6 = 10.0;
double Edf::EDF_TST_F7 = 16.0;
double Edf::EDF_TST_A7 = 10.0;

// constants: feature file related
//
double Edf::DEF_FDUR = 1.0;

// HTK constants
//
double Edf::HTK_FDUR_SCALE = 100e-06;

//
// end of file
