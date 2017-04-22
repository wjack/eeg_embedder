// file: $(NEDC_NFC)/class/cpp/Edf/Edf.h
//

// Revision History:
//
//  20160722 (SL): added create_directory method
//  20160711 (JP): removed some includes and cleaned up the code
//
//  20160621 (JM): added Kaldi ark functionality
//                 methods added: write_features_kaldi
//                                read_features_kaldi
//                                is_kaldi
//
//  20150705 (JP): updated for gcc v5.1.0
//
//  20150702 (FG): interpolation moved to Edf class
//                 methods added: interpolate
//                                interpolate_average
//
//  20150623 (GS): edited parse_montage method
//                 methods added: set_num_chans
//
//  20150613 (FG): added check_montage functionality
//                 methods added: check_montage
//                                get_montage_channels
// 
//  20150612 (FG): added interpolation functionality
//                 methods added: add_channel
//                                select_channel
//                                get_chan_pos
//  
//  20141216 (AH): added set_sample_frequency method
//
//
// make sure definitions are only made once
//
#ifndef AUTOEEG_EDF
#define AUTOEEG_EDF

// include local libraries
//
#include <boost/numeric/ublas/vector.hpp>
#include <boost/numeric/ublas/matrix.hpp>

// set up associated namespaces
//
using namespace boost::numeric::ublas;

// include standard C libraries
//
#include <stdio.h>         // stdin, stdout, etc.
#include <stdlib.h>        // basic C built-in functions
#include <math.h>          // math libraries
#include <errno.h>         // system error numbers
#include <sys/stat.h>      // system libraries
#include <string.h>        // C string processing
#include <limits.h>        // need to know the size of some C data types
#include <dirent.h>        // directory processing

// special definitions:
//  these typedefs are using to describe signal and fature vectors
//
typedef vector<bool> VectorBool;
typedef vector<long> VectorLong;
typedef vector<double> VectorDouble;

typedef vector<VectorLong> VVectorLong;
typedef vector<VectorDouble> VVectorDouble;

typedef vector<VVectorDouble> VVVectorDouble;

typedef matrix<double> MatrixDouble;
typedef vector<MatrixDouble> VMatrixDouble;

// Edf: a class that supports simple manipulation of EDF files. This
// includes reading the header, loading the signal data, and
// writing various feature file formats.
//
class Edf {

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
  // string constants
  //
  //----------------------------------------
  
  // values used in parameter files
  //
  static const char* NULL_NAME;
  static const char* VALUE_DELIMITER;
  static const char* VALUE_FNMOD;

  // values use to parse filenames and ASCII text
  //
  static const char* SPACE;
  static const char* SLASH;
  static const char* DOT;
  static const char* COMMA;
  static const char* COMMENT;
  static const char* DASHDASH;
  static const char* ANNOTATION;
  static const char* COLON;

  // post-processing exceptions related parameters
  //
  static const char* ENERGY;
  static const char* CEPSTRAL;
  static const char* MINMAX_DIFF;
  
  // variable type names
  //
  static const char* VTYPE_NAME_00;
  static const char* VTYPE_NAME_01;
  static const char* VTYPE_NAME_02;
  static const char* VTYPE_NAME_03;

  // i/o-related parameters:
  //  line length is set to the maximum length of an input line expected
  //  sstr is the short string length (used for values)
  //  mstr is the medium string length (used for character strings)
  //  lstr is the long string length (used for extended strings)
  //
  static const unsigned int MAX_LINE_LENGTH = 2048;
  static const unsigned int MAX_SSTR_LENGTH = 39;
  static const unsigned int MAX_MSTR_LENGTH = 199;
  static const unsigned int MAX_LSTR_LENGTH = 999;

  // signal and feature i/o related parameters
  //
  static const long MAX_NCHANS = 256;
  static const long MAX_TMP_BSIZE = 99999;
  
  
  //----------------------------------------
  //
  // EDF header-related constants:
  //  most sections of the header can be read in fixed length blocks.
  //  this reduces I/O overhead.
  //
  //----------------------------------------

  static const long EDF_BSIZE = 256;
  static const long FLIST_BSIZE = 32768;

  static const long EDF_VERS_BSIZE =  8;
  static const char* EDF_VERS;
  static const char* EDF_FTYP;
  
  static const long EDF_LPTI_BSIZE = 80;
  static const long EDF_LPTI_TSIZE = 119;

  static const long EDF_LRCI_BSIZE = 80;
  static const long EDF_LRCI_TSIZE = EDF_LPTI_TSIZE;
  static const long EDF_LRCI_RSIZE = EDF_LPTI_BSIZE;

  static const long EDF_GHDI_BSIZE =  8 + 8 + 8 + 5 + 39 + 8 + 8 + 4;
  static const long EDF_GHDI_TSIZE = EDF_LPTI_TSIZE;

  static const long EDF_LABL_BSIZE = 16;
  static const long EDF_TRNT_BSIZE = 80;
  static const long EDF_PDIM_BSIZE =  8;
  static const long EDF_PMIN_BSIZE =  8;
  static const long EDF_PMAX_BSIZE =  8;
  static const long EDF_DMIN_BSIZE =  8;
  static const long EDF_DMAX_BSIZE =  8;
  static const long EDF_PREF_BSIZE = EDF_TRNT_BSIZE;
  static const long EDF_RECS_BSIZE =  8;

  static const long EDF_FTYP_BSIZE =  5;

  //----------------------------------------
  //
  // channel selection-related constants
  //
  //----------------------------------------

  // enumerations related to selection mode
  //
  enum SELECT_MODE {SELMODE_SELECT = 0, SELMODE_REMOVE,
		    DEF_SELECT_MODE = SELMODE_SELECT};

  // enumerations related to match mode
  //
  enum MATCH_MODE {MATMODE_EXACT = 0, MATMODE_PARTIAL,
		   DEF_MATCH_MODE = MATMODE_EXACT};

  // enumerations related to selection mode
  //
  enum INTERPOLATE_MODE {INTMODE_NONE = 0, INTMODE_AVERAGE,  
		         DEF_INTERPOLATE_MODE = INTMODE_NONE};
  
  // enumerations related to interpolate output mode
  //
  enum INTERPOLATE_OMODE {INTOMODE_REPLACE = 0, INTOMODE_CONCAT,   
			  DEF_INTERPOLATE_OMODE = INTOMODE_CONCAT};  

  //----------------------------------------
  //
  // test signal-related constants
  //
  //----------------------------------------

  static double EDF_TST_F1;
  static double EDF_TST_A1;
  static double EDF_TST_F2;
  static double EDF_TST_A2;
  static double EDF_TST_F3;
  static double EDF_TST_A3;
  static double EDF_TST_F4;
  static double EDF_TST_A4;
  static double EDF_TST_F5;
  static double EDF_TST_A5;
  static double EDF_TST_F6;
  static double EDF_TST_A6;
  static double EDF_TST_F7;
  static double EDF_TST_A7;

  //----------------------------------------
  //
  // feature file-related constants
  //
  //----------------------------------------

  // default values
  //
  static double DEF_FDUR;
  static const char* DEF_FNMOD;
  static const char* DEF_FFMT_NAME;

  // output feature file generation-related parameters
  //
  static const char* FFMT_NAME_00;
  static const char* FFMT_NAME_01;
  static const char* FFMT_NAME_02;
  static const char* FFMT_NAME_03;

  // enumerations related to output file generation
  //
  enum FFMT {FFMT_EDF = 0, FFMT_RAW, FFMT_HTK, FFMT_KALDI,
	     DEF_FFMT = FFMT_EDF};
  
  // HTK constants:
  //  note that even though HTK scales fdur by 100ns, we use a
  //  larger value here because it helps HTK work better
  //
  static double HTK_FDUR_SCALE;

  // Kaldi constants
  //
  static const long KALDI_HEADER_FIXED_SIZE = 15;
  static const long KALDI_VECTOR_FIXED_LOC = 11;
  static const long KALDI_FRAME_FIXED_LOC = 6;
  static const char KALDI_HEADER_EOT = 4;
  static const char* KALDI_HEADER_STR;
  
  //----------------------------------------
  //
  // error codes
  //
  //----------------------------------------

  static const long NO_ERR = 0;
  static const long ERR_MATCH = -1;
  static const long ERR_FILE = -1;
  static const long ERR_FEAT = -2;
  static const long ERR = 99999;

  //----------------------------------------
  //
  // debug constants
  //
  //----------------------------------------

  // debug levels (shared by debug_level and verbosity)
  //
  static const char* LEVEL_NONE_NAME;
  static const char* LEVEL_BRIEF_NAME;
  static const char* LEVEL_SHORT_NAME;
  static const char* LEVEL_MEDIUM_NAME;
  static const char* LEVEL_DETAILED_NAME;
  static const char* LEVEL_FULL_NAME;

  static const long LEVEL_NONE = 0;
  static const long LEVEL_BRIEF = 5;
  static const long LEVEL_SHORT = 10;
  static const long LEVEL_MEDIUM = 15;
  static const long LEVEL_DETAILED = 20;
  static const long LEVEL_FULL = 25;
  static const long DEF_LEVEL = LEVEL_NONE;
  
  static const int DEF_DBG_NS = 10;
  static const int DEF_DBG_NF = 3;

  //---------------------------------------------------------------------------
  //
  // protected data
  //
  //---------------------------------------------------------------------------
protected:

  // define a debug level
  //
  static long debug_level_d;
  static long verbosity_d;

  // define parameters related to standard C file processing
  //
  char* fn_d;
  FILE* fp_d;

  // define header blocksizes
  //
  // (1) contains the version of the file
  //
  char hdr_version_d[EDF_VERS_BSIZE + 1];

  // (2) contains local patient information (lpti)
  //
  char hdr_lpti_patient_id_d[EDF_LPTI_TSIZE+1];
  char hdr_lpti_gender_d[EDF_LPTI_TSIZE+1];
  char hdr_lpti_dob_d[EDF_LPTI_TSIZE+1];
  char hdr_lpti_full_name_d[EDF_LPTI_TSIZE+1];
  char hdr_lpti_age_d[EDF_LPTI_TSIZE+1];

  // (3) contains local recording information (lrci)
  //
  char hdr_lrci_start_date_label_d[EDF_LRCI_TSIZE+1];
  char hdr_lrci_start_date_d[EDF_LRCI_TSIZE+1];
  char hdr_lrci_eeg_id_d[EDF_LRCI_TSIZE+1];
  char hdr_lrci_tech_d[EDF_LRCI_TSIZE+1];
  char hdr_lrci_machine_d[EDF_LRCI_TSIZE+1];

  // (4) contains general header information (ghdi)
  //
  char hdr_ghdi_start_date_d[EDF_LRCI_TSIZE+1];
  char hdr_ghdi_start_time_d[EDF_LRCI_TSIZE+1];
  long hdr_ghdi_hsize_d;
  char hdr_ghdi_file_type_d[EDF_LRCI_TSIZE+1];
  char hdr_ghdi_reserved_d[EDF_LRCI_RSIZE+1];
  long hdr_ghdi_num_recs_d;
  long hdr_ghdi_dur_rec_d;
  long hdr_ghdi_nsig_rec_d;

  // (5) contains channel-specific data
  //
  char* hdr_chan_labels_d[MAX_NCHANS];
  char* hdr_chan_trans_type_d[MAX_NCHANS];
  char* hdr_chan_phys_dim_d[MAX_NCHANS];
  double hdr_chan_phys_min_d[MAX_NCHANS];
  double hdr_chan_phys_max_d[MAX_NCHANS];
  long hdr_chan_dig_min_d[MAX_NCHANS];
  long hdr_chan_dig_max_d[MAX_NCHANS];
  char* hdr_chan_prefilt_d[MAX_NCHANS];
  long hdr_chan_rec_size_d[MAX_NCHANS];

  // hold the number of adjacent channels related to each new channel
  // 
  VectorLong num_adj_chan_d;            

  // hold the labels of adjacent channels:
  //  note that the labels are related o new channel i
  //
  char** adj_chan_labels_d[MAX_NCHANS];

  // hold the labels to each new channel
  //
  char* new_chan_labels_d[MAX_NCHANS];   
  
  // (6) define some derived values
  //
  float hdr_sample_frequency_d;
  long hdr_num_channels_proc_d;
  long hdr_num_channels_signal_d;
  long hdr_num_channels_annotation_d;
  long hdr_num_adj_channels_d;

  // (7) additional derived values
  //
  
  // label processing:
  //  we need to store local copies of the channel selection
  //  labels and the montage labels. these are dynamic in
  //  that they reflect whatever the last selection or
  //  montage requested.
  //
  MATCH_MODE smmode_d;
  MATCH_MODE mmmode_d;
  
  long num_slabels_d;
  long num_mlabels_d;
  long num_clabels_d;

  char* slabels_d[MAX_NCHANS];
  char* mlabels1_d[MAX_NCHANS];
  char* mlabels2_d[MAX_NCHANS];
  long chan_num_d[MAX_NCHANS];
  char* mchan_d[MAX_NCHANS];

  // htk-related parameters
  //
  double fdur_d;
  char fnmod_d[MAX_SSTR_LENGTH];

  //---------------------------------------------------------------------------
  //
  // required public methods (edf_00)
  //
  //---------------------------------------------------------------------------
public:

  // method: name
  //
  static const char* name() {
    return CLASS_NAME;
  }

  // method: set debug level
  //
  long set_debug(int debug_level) {
    return (debug_level_d = debug_level);
  }

  // method: get debug level
  //
  long get_debug() {
    return debug_level_d;
  }

  // method: set verbosity
  //
  long set_verbosity(int verbosity) {
    return (verbosity_d = verbosity);
  }

  // method: get verbosity
  //
  long get_verbosity() {
    return verbosity_d;
  }

  // method: default constructor
  //
  Edf(long debug_level = DEF_LEVEL);

  // method: destructor
  //
  ~Edf();

  //---------------------------------------------------------------------------
  //
  // public methods: i/o (edf_01)
  //
  //---------------------------------------------------------------------------
public:

  // read/write signal data (EDF files) (edf_01)
  //
  bool read_edf(VVectorDouble& sig, char* fn,
		bool sc = true, bool rsig = true);
  bool write_edf(VVectorDouble& sig, char* fn);

  // read feature data (edf_01)
  //
  bool read_features(VVVectorDouble& feat, char* fname);
  bool read_features_raw(VVVectorDouble& feat, char* fn);
  bool read_features_htk(VVVectorDouble& feat, char* fn);
  bool read_features_kaldi(VVVectorDouble& feat, char* fn);

  // write feature data (edf_01)
  //
  bool write_features(VVVectorDouble& feat, char* fname, FFMT fffmt);
  bool write_features_raw(VVVectorDouble& feat, char* fn);
  bool write_features_htk(VVVectorDouble& feat, char* fn);
  bool write_features_kaldi(VVVectorDouble& feat, char* fn);

  // informational methods (edf_01)
  //
  bool is_edf(const char* fname);
  bool is_filelist(char* fname);
  bool is_raw(char* fname);
  bool is_htk(char* fname);
  bool is_kaldi(char* fname);
  bool debug(VVectorDouble& sig, FILE* fp = stdout);
  bool debug(VVVectorDouble& feat, FILE* fp = stdout);

  // get/set for the frame duration
  //
  double get_fdur() {
    return fdur_d;
  }

  float set_fdur(double fdur) {
    return (fdur_d = fdur);
  }

  // get/set for the filename modifier
  //
  bool get_filename_modifier(char* fnmod) {
    return strcpy(fnmod, fnmod_d);
  }

  bool set_filename_modifier(char* fnmod) {
    return strcpy(fnmod_d, fnmod);
  }

  // get duration
  //
  double get_duration() {
    return (float)(hdr_ghdi_dur_rec_d * hdr_ghdi_num_recs_d);
  }

  //---------------------------------------------------------------------------
  //
  // public methods: channel selection and manipulation (edf_02)
  //
  //---------------------------------------------------------------------------
public:

  // channel selection methods
  //
  bool select_channel(VVectorDouble& sig_out,
	              VVectorDouble& sig_in,
	              char* cselect);
  
  bool select(VVectorDouble& sig_out,
	      VVectorDouble& sig_in,
	      char* cselect, MATCH_MODE matmode);

  bool remove(VVectorDouble& sig_out,
	      VVectorDouble& sig_in,
	      char* cselect, MATCH_MODE matmode);

  bool apply_montage(VVectorDouble& sigo,
		     VVectorDouble& sigi,
		     char** mselect, MATCH_MODE matmode);
  
  // test signal methods
  //
  bool test_signal(VVectorDouble& sig, const char* mode);

  // methods to deidentify data
  //
  bool deidentify(char* label, char* subj, char* sess, char* tech);

  // interpolation methods
  //
  bool add_interp_channel(VVectorDouble& sig_out,
			  VVectorDouble& sig_in,
			  VVectorDouble& chan_in,
			  INTERPOLATE_OMODE omode);
  
  bool interpolate(VVectorDouble& sig,
		   MATCH_MODE matmode,
		   INTERPOLATE_MODE mode,
		   INTERPOLATE_OMODE omode);

  //---------------------------------------------------------------------------
  //
  // public methods: infrastructure methods (edf_00)
  //
  //---------------------------------------------------------------------------
public:

  // resizing methods
  //
  bool resize(char*& str, long size, bool preserve = false);

  bool resize(VectorBool& v, long size, bool preserve = false);
  bool resize(VectorLong& v, long size, bool preserve = false);
  bool resize(VectorDouble& v, long size, bool preserve = false);

  bool resize(VVectorLong& v, long size, bool preserve = false);
  bool resize(VVectorDouble& v, long size, bool preserve = false);

  bool resize(VVVectorDouble& v, long size, bool preserve = false);

  bool resize(MatrixDouble& v, long size1, long size2, bool preserve = false);
  bool resize(VMatrixDouble& v, long nchan, bool preserve = false);

  // memory management methods
  //
  bool cleanup();
  bool cleanup(char** strs, long num_strs);
  bool cleanup_labels();

  // conversion from strings to ints / enums
  //
  bool convert_debug_from_string(long& value, char* str);

  // parameter file parsing and string methods
  //  note that parse_aux parses the interpolation and montage specs
  //
  bool get_nvp(char* name, char* value, char* buf);
  long find_vname(char* name, char** vnames);
  bool set_var(long pos, char* value, char** vtypes, void** vptrs);
  bool parse_aux(char** strm, char** stri);

  // parses the channel numbers for interpolation and montage
  //
  bool set_interp_chans(long new_channels);
  bool set_montage_chans(long num_montage);

  //---------------------------------------------------------------------------
  //
  // public methods: signal manipulation methods (edf_02)
  //
  //---------------------------------------------------------------------------
public:

  // signal manipulation methods
  //
  bool set_start_time(char* st_time);
  bool increment_start_time(long num_seconds);  
  bool set_signal_dimensions(VVectorDouble& sig);

  float set_sample_frequency(float hdr_sample_a) {
    return (hdr_sample_frequency_d = hdr_sample_a);
  }
  
  bool copy_signal(VVectorDouble& sigo, VVectorDouble& sigi);

  bool copy_signal(VVectorDouble& sigo, long& nlo, char** labelso,
		   VVectorDouble& sigi, long nli, char** labelsi);

  //---------------------------------------------------------------------------
  //
  // public methods: informational methods (edf_03)
  //
  //---------------------------------------------------------------------------
public:

  // get methods
  //
  float get_sample_frequency() {return hdr_sample_frequency_d;}
  long get_num_channels_file() {return hdr_ghdi_nsig_rec_d;}
  long get_num_channels_proc() {return hdr_num_channels_proc_d;}
  long get_rec_size(long chan) {return hdr_chan_rec_size_d[chan];}
  long get_labels(char** labels);

  // returns the position of a channel in the signal
  //
  long get_channel_pos(char* label, MATCH_MODE matmode);

  // check_montage methods
  //
  bool get_montage_channels(long& nchannels, char** channels);
  
  bool check_montage(char* missing_channels, char* fname, long nmontage,
		     MATCH_MODE matmode,char** montage_channels);
  
  // filename manipulation methods
  //
  bool create_filename(char* oname, char* iname,
		       char* odir, char* oext, char* odir_repl);
  bool create_filelist(char** onames, long nnames, char* iname, char* fnmod);
  bool create_directory(char* path, \
	      mode_t mode = S_IRWXU | S_IRGRP |  S_IXGRP | S_IROTH | S_IXOTH);

  // display methods
  //
  bool print_header(FILE* fp, char* prefix = (char*)"\t");
  bool print_header(char* fname, FILE* fp, char* prefix = (char*)"\t");

  //---------------------------------------------------------------------------
  //
  // public methods: linear algebra methods (edf_04)
  //
  //---------------------------------------------------------------------------
public:

  // math and vector functions
  //
  double clip(double value, double min, double max);
  long clip(long value, long min, long max);
  double max(VectorDouble& input);
  double max(double val1, double val2);
  double min(VectorDouble& input);
  double min(double val1, double val2);
  bool shift(VectorDouble& v, long incr);

  //---------------------------------------------------------------------------
  //
  // private methods
  //
  //---------------------------------------------------------------------------
private:

  // special file I/O methods (edf_01)
  //
  bool read_htk_channel(VVectorDouble& feat, double& fdur, char* fname);
  bool write_htk_channel(VVectorDouble& feat, double fdur, char* fname);

  bool is_big_endian();
  bool swap_bytes(void* buf, void* value, long nbytes);
  
  // methods to get/put edf file data (edf_03)
  //
  bool get_header(FILE* fp);
  bool put_header(FILE* fp);
  long compute_header_size(long num_channels);

  // string processing methods (edf_03)
  //
  bool uppercase(char* str);
  long find_match(char* label, long nl, char** lbls, MATCH_MODE matmode);
  bool trim_whitespace_and_upcase(char* str_o, char* str_i);
  bool pad_whitespace(char* str, long len);

  // parsing methods (edf_03)
  //
  bool parse_line(long& nl, char** labels, char* str, char* c);
  bool parse_montage(long& nl, char** labels1, char** labels2,
		     char** chan_labels, long* num_chans, char** str);
  bool parse_operands(long& nl_o, char** labels1, char** labels2,
		      long nl_i, char** labels_i);
  bool parse_interp_channels(long& nl_i, char*** labels1,
			     VectorLong& num_chans, char** newChannel,
			     char** str);

  // filename processing methods (edf_03)
  //
  bool get_matching_filenames(char** fnames, long& nf, char* bfn, char* fmt);
  bool create_matching_filename(char* dirname, char* bname, char* ext,
				char* fname, long nf, char* bfn, char* fmt);

  // interpolation methods (edf_05)
  //
  bool interpolate_average(VVectorDouble& new_chan,
			   VVectorDouble& sig);
  //
  // end of class
};

// end of include file
//
#endif
