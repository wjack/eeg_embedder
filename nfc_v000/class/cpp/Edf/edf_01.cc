// file: $(NEDC_NFC)/class/cpp/Edf/edf_01.cc
//
// This file contains basic I/O methods such as read and write.
//

// local include files
//
#include "Edf.h"

// method: read_edf
//
// arguments:
//  VVectorDouble& sig: the EEG signal data (output)
//  char* fn: input filename (input)
//  bool sc: scale the signal based on header data (input)
//           the default is to do the standard EDF scaling
//  bool rsig: if true, read the signal (input)
//
// return: a logical value indicating status
//
// This method opens an EDF file, reads the signal data channel by channel
// into the signal matrix, and closes the file.
//
bool Edf::read_edf(VVectorDouble& sig_a, char* fn_a,
		   bool sc_a, bool rsig) {

  // copy the filename
  //
  Edf::resize(fn_d, strlen(fn_a) + 1);
  strcpy(fn_d, fn_a);

  // display debug information
  //
  if (debug_level_d >= LEVEL_FULL) {
    fprintf(stdout, "Edf::read_edf(): opening an EDF file (%s)\n", fn_a);
  }

  // open the file
  //
  if ((fp_d = fopen(fn_a, "r")) == (FILE*)NULL) {
    fprintf(stdout, "**> Edf::read_edf(): error opening (%s)\n", fn_a);
    return false;
  }
  
  // load the header
  //
  if (!Edf::get_header(fp_d)) {
    fprintf(stdout, "**> Edf::read_edf(): error in get_header (%s)\n",
	    fn_a);
    return false;
  }
  if (rsig == false) {
    fclose(fp_d);
    return true;
  }

  // display debug information
  //
  if (debug_level_d >= LEVEL_DETAILED) {
    Edf::print_header(stdout);
  }

  // position the file to the beginning of the data
  // using the header information
  //
  if (debug_level_d >= LEVEL_DETAILED) {
    fprintf(stdout, "Edf::read_edf(): beginning reading of data\n");
  }

  if (fseek(fp_d, hdr_ghdi_hsize_d, SEEK_SET)) {
    return false;
  }

  if (debug_level_d >= LEVEL_DETAILED) {
    fprintf(stdout, "Edf::read_edf(): file positioning complete\n");
  }

  // create space to hold the entire signal
  //
  Edf::resize(sig_a, hdr_ghdi_nsig_rec_d, false);

  for (long i = 0; i < hdr_ghdi_nsig_rec_d; i++) {
    Edf::resize(sig_a[i], hdr_ghdi_num_recs_d * hdr_chan_rec_size_d[i], false);

    if ((debug_level_d >= LEVEL_DETAILED) && (i < DEF_DBG_NF)) {
      fprintf(stdout,
	      "Edf::read(): sig_a dimensions (%ld row, %ld cols)\n",
	      i, hdr_ghdi_num_recs_d * hdr_chan_rec_size_d[i]);
    }
  }

  if (debug_level_d >= LEVEL_DETAILED) {
    fprintf(stdout, "Edf::read_edf(): signal vector resized\n");
  }

  // create temporary space for I/O
  //
  long bsize = 0;
  for (long i = 0; i < sig_a.size(); i++) {
    bsize = Edf::max(hdr_chan_rec_size_d[i], bsize);
  }
  short int buf[bsize];

  // loop over all records
  //
  for (long i = 0; i < hdr_ghdi_num_recs_d; i++) {

    // loop over all channels
    //
    for (long j = 0; j < hdr_ghdi_nsig_rec_d; j++) {

      // display debug message
      //
      if ((debug_level_d >= LEVEL_FULL) && (i < DEF_DBG_NF) &&
	  (j < DEF_DBG_NF)) {
	fprintf(stdout,
		"Edf::read_edf(): reading record no. [%ld %ld]\n", i, j);
      }

      // read the data
      //
      long num_samps = hdr_chan_rec_size_d[j];
      long num_samps_read = fread(buf, sizeof(short int), num_samps, fp_d);

      if (num_samps_read != num_samps) {
	return false;
      }

      if ((debug_level_d >= LEVEL_FULL) && (i < DEF_DBG_NF) &&
	  (j < DEF_DBG_NF)) {
	fprintf(stdout,
		"Edf::read_edf(): [%ld %ld] read data complete\n", i, j);
      }

      // compute scale factors:
      //  the data must be scaled from digital to physical signal levels.
      //  a dc offset is also computed according to the standard. 
      //  note that for some data, the max and min values are zero,
      //  so we must check this. if it is zero, we ignore it by making
      //  the scale factor 1 and the bias 0;
      //
      double sum_n = hdr_chan_phys_max_d[j] - hdr_chan_phys_min_d[j];
      double sum_d = (double)(hdr_chan_dig_max_d[j] - hdr_chan_dig_min_d[j]);
      double sum = 1.0;
      double dc = 0;
      if (sum_d != 0) {
	sum = sum_n / sum_d;
	dc = hdr_chan_phys_max_d[j] - sum * (double)hdr_chan_dig_max_d[j];
      }

      if ((debug_level_d >= LEVEL_FULL) && (i < DEF_DBG_NF) &&
	  (j < DEF_DBG_NF)) {
	fprintf(stdout,
		"Edf::read_edf(): [%ld %ld] dc offset = %f (%f, %f, %f)\n",
		i, j, dc, sum_n, sum_d, sum);
      }

      // transfer the data to the double precision output
      //
      long offset = i * hdr_chan_rec_size_d[j];
      for (int k = 0; k < num_samps; k++) {
	if (sc_a == true) {
	  sig_a[j][offset++] = sum * (double)buf[k] + dc;
	}
	else {
	  sig_a[j][offset++] = (double)buf[k];
	}
      }

      if ((debug_level_d >= LEVEL_FULL) && (i < DEF_DBG_NF) &&
	  (j < DEF_DBG_NF)) {
	fprintf(stdout,
		"Edf::read_edf(): [%ld %ld] data transfer complete\n", i, j);
      }
    }

    // display debug message
    //
    if ((debug_level_d >= LEVEL_FULL) && (i < 2)) {
      fprintf(stdout, "Edf::read_edf(): done reading record no. %ld\n", i);
    }
  }
  
  // display debug information
  //
  if (debug_level_d >= LEVEL_FULL) {
    fprintf(stdout, "Edf::read_edf(): closing an EDF file\n");
  }

  // close the file
  //
  if (fp_d != (FILE*)NULL) {
    if (fclose(fp_d) == EOF) {
      fprintf(stdout, "**> Edf::read_edf(): error closing (%s)\n", fn_d);
      return false;
    }
  }

  // display debug information
  //
  if (debug_level_d >= LEVEL_FULL) {
    fprintf(stdout, "Edf::read_edf(): done closing an EDF file\n");
  }

  // exit gracefully
  //
  return true;
}

// method: write_edf
//
// arguments:
//  VVectorDouble& sig: signal data (input)
//  char* fn: output filename (input)
//
// return: a logical value indicating status
//
// This method write everything in a header and file except the
// annotation information.
//
bool Edf::write_edf(VVectorDouble& sig_a, char* fn_a) {

  // declare local variables
  //
  char tmp_buf[MAX_MSTR_LENGTH];
  long status = true;
  int32_t isum;
  
  // display debugging information
  //
  if (debug_level_d >= LEVEL_DETAILED) {
    fprintf(stdout, "Edf::write_edf(): beginning write edf\n");
  }

  // change the file type to generic EDF
  //
  memcpy(hdr_ghdi_file_type_d, EDF_FTYP, EDF_FTYP_BSIZE);

  // open the file
  //
  FILE* fp = fopen(fn_a, "w");
  if (fp == (FILE*)NULL) {
    return false;
  }

  // write the header
  //
  if (debug_level_d >= LEVEL_DETAILED) {
    fprintf(stdout, "Edf::write_edf(): writing header\n");
  }
  if (!Edf::put_header(fp)) {
     fclose(fp);
      return false;
  }

  if (debug_level_d >= LEVEL_DETAILED) {
    fprintf(stdout, "Edf::write_edf(): done writing header\n");
  }

  // position the file to the beginning of the data
  // using the header information
  //
  if (fseek(fp, hdr_ghdi_hsize_d, SEEK_SET)) {
    return false;
  }

  if (debug_level_d >= LEVEL_DETAILED) {
    fprintf(stdout, "Edf::write_edf(): file positioning complete\n");
  }

  // create temporary space
  //
  long bsize = 0;
  for (long i = 0; i < sig_a.size(); i++) {
    bsize = Edf::max(hdr_chan_rec_size_d[i], bsize);
  }
  short int buf[bsize];

  if (debug_level_d >= LEVEL_DETAILED) {
    fprintf(stdout,
	    "Edf::write_edf(): temp space created [%ld samples]\n", bsize);
  }

  // loop over all records
  //
  if (debug_level_d >= LEVEL_DETAILED) {
    fprintf(stdout,
	    " Edf::write_edf(): starting main write loop [%ld recs]\n",
	    hdr_ghdi_num_recs_d);
  }

  for (long i = 0; i < hdr_ghdi_num_recs_d; i++) {

    // display debug message
    //
    if ((debug_level_d >= LEVEL_FULL) && (i < DEF_DBG_NF)) {
      fprintf(stdout,
	      " Edf::write_edf(): writing record no. %ld out of %ld\n",
	      i, hdr_chan_rec_size_d[i]);
    }

    // loop over all channels
    //
    for (long j = 0; j < hdr_ghdi_nsig_rec_d; j++) {

      // compute scale factors (see the read method): in this case
      //  we invert the scale factor.
      //
      double sum_n = hdr_chan_phys_max_d[j] - hdr_chan_phys_min_d[j];
      double sum_d = hdr_chan_dig_max_d[j] - hdr_chan_dig_min_d[j];
      double sum = 1.0;
      double dc = 0;
      if (sum_d != 0) {
	sum = sum_n / sum_d;
	dc = hdr_chan_phys_max_d[j] - sum * hdr_chan_dig_max_d[j];
      }
      double isum = 1.0 / sum;

      if ((debug_level_d >= LEVEL_DETAILED) && (i < DEF_DBG_NF) &&
	  (j < DEF_DBG_NF)) {
	fprintf(stdout,	" %s %ld block: %ld size = %ld] %s [%f %f %f %f]\n",
		"  Edf::write_edf(): [rec: ", i, j, hdr_chan_rec_size_d[j],
		"[n/d/scale/dc] =", sum_n, sum_d, sum, dc);
      }

      // transfer the data to the double precision output
      //
      long num_samps = hdr_chan_rec_size_d[j];
      long offset = i * hdr_chan_rec_size_d[j];
      for (int k = 0; k < num_samps; k++) {
	buf[k] = round(((sig_a[j][offset++] - dc) * isum));
      }

      // write the data
      //
      long num_samps_write = fwrite(buf, sizeof(short int), num_samps, fp);
      if (num_samps_write != num_samps) {
	return false;
      }
    }

    // display debug message
    //
    if ((debug_level_d >= LEVEL_FULL) && (i < DEF_DBG_NF)) {
      fprintf(stdout, " Edf::write_edf(): done writing record no. %ld\n", i);
    }
  }
  
  // close the file
  //
  fclose(fp);

  // display debugging information
  //
  if (debug_level_d >= LEVEL_DETAILED) {
    fprintf(stdout, "Edf::write_edf(): end of edf write\n");
  }
  
  // exit gracefully
  //
  return status;
}

// method: read_features
//
// arguments:
//  VVVectorDouble& feat: feature data (output)
//  char* fn: input filename (input)
//
// return: a logical value indicating status
//
// This method reads feature data from a file. It branches on the
// type of the file.
//
// This method is a little fragile since raw and htk files don't really
// have identifiers. If the exact filename doesn't exist, it must be
// an htk file, since these are written as multiple single channel files
// with a modified filename.
//
bool Edf::read_features(VVVectorDouble& feat_a, char* fn_a) {

  // declare local variables
  //
  FFMT ffmt;
  
  // display debugging information
  //
  if (debug_level_d >= LEVEL_DETAILED) {
    fprintf(stdout,
	    "Edf::read_features(): begin reading a feature file\n");
  }

  // check the file type using informational methods
  //
  if (is_htk(fn_a)) {
    ffmt = FFMT_HTK;
  }
  else if (is_kaldi(fn_a)) {
    ffmt = FFMT_KALDI;
  }
  else if (is_raw(fn_a)) {
    ffmt = FFMT_RAW;
  }
  else {
    fprintf(stdout, "Edf::read_features(): The file type is not supported\n"); 
    return false;
  }

  // check for one of three things: raw, htk, kaldi
  //
  if (ffmt == FFMT_RAW) {
    return Edf::read_features_raw(feat_a, fn_a);
  }
  else if (ffmt == FFMT_HTK) {
    return Edf::read_features_htk(feat_a, fn_a);
  }
  else if (ffmt == FFMT_KALDI) {
    return Edf::read_features_kaldi(feat_a, fn_a);
  }
  else {
    fprintf(stdout, "Edf::read_features(): Error choosing input type\n"); 
    return false; 
  }
  
  // display debugging information
  //
  if (debug_level_d >= LEVEL_DETAILED) {
    fprintf(stdout,
	    "Edf::read_features(): done reading a feature file\n");
  }

  // exit gracefully
  //
  return true;
}

// method: read_features_raw
//
// arguments:
//  VVVectorDouble& feat: feature data (output)
//  char* fn: input filename (input)
//
// return: a logical value indicating status
//
// This method reads feature data from a raw file.
//
bool Edf::read_features_raw(VVVectorDouble& feat_a, char* fn_a) {

  // declare local variables
  //
  long nbytes = 0;
  long status = true;
  
  // display debugging information
  //
  if (debug_level_d >= LEVEL_DETAILED) {
    fprintf(stdout, "Edf::read_raw(): beginning binary read [%s]\n", fn_a);
  }

  // open the file
  //
  FILE* fp = fopen(fn_a, "r");
  if (fp == (FILE*)NULL) {
    return false;
  }

  // get the size of the file
  //
  fseek(fp, 0, SEEK_END);
  long fsize = ftell(fp);
  rewind(fp);

  // read and check the dimensions
  //
  int32_t dims[2];
  if (fread(dims, sizeof(int32_t), 2, fp) != 2) {
    fclose(fp);
    return false;
  }
  if ((dims[0] < 0) || (dims[1] < 0) ||
      (((long)dims[0]*(long)dims[1]) > fsize)) {
    fclose(fp);
    return false;
  }

  // size the feature vector
  //
  nbytes += 2 * sizeof(int32_t);
  Edf::resize(feat_a, dims[0], false);
  for (long i = 0; i < dims[0]; i++) {
    Edf::resize(feat_a[i], dims[1], false);
  }

  if (debug_level_d >= LEVEL_FULL) {
    fprintf(stdout, "Edf::read_raw(): (nchan, nframes) %ld %ld\n",
	    (long)dims[0], (long)dims[1]);
  }

  // loop over all channels and frames
  //
  for (long i = 0; i < dims[0]; i++) {
    for (long j = 0; j < dims[1]; j++) {

      // read the vector size from the file
      //
      int32_t ndim;
      if (fread(&ndim, sizeof(int32_t), 1, fp) != 1) {      
	fclose(fp);
	return false;
      }
      nbytes += sizeof(int32_t);
      Edf::resize(feat_a[i][j], ndim, false);

      // read the vector
      //
      float tmp_buf[ndim];
      if (fread(tmp_buf, sizeof(float), ndim, fp) != ndim) {
	fclose(fp);
	return false;
      }
      nbytes += ndim * sizeof(float);

      // convert the float to a double
      //
      for (long k = 0; k < ndim; k++) {
	feat_a[i][j][k] = tmp_buf[k];
      }

      if (debug_level_d >= LEVEL_FULL) {
	fprintf(stdout,
		"Edf::read_raw(): (chan, nf, fdim) %ld %ld %ld [%f]\n",
		i, j, (long)feat_a[i][j].size(), feat_a[i][j][0]);
      }
    }
  }
  
  // close the file
  //
  fclose(fp);

  // display debugging information
  //
  if (debug_level_d >= LEVEL_DETAILED) {
    fprintf(stdout, "Edf::read_raw(): end of binary read [%ld bytes]\n",
	    nbytes);
  }

  // exit gracefully
  //
  return status;
};

// method: read_features_htk
//
// arguments:
//  VVVectorDouble& feat: feature matrix (output)
//  char* fn: base filename (input)
//
// return: a logical value indicating status
//
// This method reads each channel of a multichannel HTK signal into
// a single feature vector data structure. In this version, the user
// provides a list of filenames which are mapped to channel 0, channel 1,
// ... channel N. Since HTK feature files can only hold one channel of
// feature data, an EDF file must be split into multiple files.
//
bool Edf::read_features_htk(VVVectorDouble& feat_a, char* fn_a) {

  // display debugging information
  //
  if (debug_level_d >= LEVEL_DETAILED) {
    fprintf(stdout,
	    "Edf::read_features_htk(): begin reading htk formatted files\n");
  }

  // build the filelist
  //
  long nf;
  char* fnames[MAX_NCHANS];
  if (Edf::get_matching_filenames(fnames, nf, fn_a, fnmod_d) == false) {
    return false;
  }

  if (debug_level_d >= LEVEL_MEDIUM) {
    for (long i = 0; i < nf; i++) {
      fprintf(stdout,
	      " Edf::read_features_htk(): channel %ld - [%s]\n", i, fnames[i]);
    }
  }

  // create space for the feature data
  //
  if (feat_a.size() != nf) {
    Edf::resize(feat_a, nf, false);
  }

  // loop over all files in the list
  //
  for(long i = 0; i < nf; i++){
    if (read_htk_channel(feat_a[i], fdur_d, fnames[i]) == false) {
      fprintf(stdout,
	      "**> Edf::read_features_htk(): error reading channel %ld - [%s]\n",
	      i, fnames[i]);
      return false;
    }
  }

  // clean up memory
  //
  Edf::cleanup(fnames, nf);

  // display debugging information
  //
  if (debug_level_d >= LEVEL_DETAILED) {
    fprintf(stdout,
	    "Edf::read_features_htk(): done reading htk formatted files\n");
  }
  
  // exit gracefully
  //
  return true;
}

// method: read_features_kaldi
//
// arguments:
//  VVVectorDouble& feat: feature matrix (output)
//  char* fn: base filename (input)
//
// return: a logical value indicating status
//
// This method reads each channel of a multichannel Kaldi ARK file into
// a single feature vector structure.
//
bool Edf::read_features_kaldi(VVVectorDouble& feat_a, char* fn_a) {

  // declare local variables
  //
  unsigned char buf[sizeof(float)];

  // check if we need to byte swap
  //
  bool big_endian = Edf::is_big_endian();

  // display debugging information
  //
  if (debug_level_d >= LEVEL_DETAILED) {
    fprintf(stdout,
	    "Edf::read_features_kaldi(): begin reading kaldi formatted file");
    if (big_endian = false) {
      fprintf(stdout, " byte order = little endian\n"); 
    }
    else {
      fprintf(stdout, " byte order = big endian\n");
    }
  }

  // check the size of a float:
  //  a float must be 32 bits long for this code to work properly, so we
  //  check this and make the program crash if this isn't the case.
  //
  char static_assert_float32[1 - (2 * ((sizeof(float) * CHAR_BIT) != 32))];

  // open the input Kaldi file for reading
  //
  FILE* fp = fopen(fn_a, "r");
  if (fp == (FILE*)NULL) {
    return false;
  }

  // get the name from the header and its length
  //
  char hname[MAX_MSTR_LENGTH];
  long hname_length = strlen(hname);

  // get the header size
  //
  long header_size = KALDI_HEADER_FIXED_SIZE + hname_length;
  
  // go to frame size and read it
  //
  int32_t frame_size;
  fseek(fp, hname_length + KALDI_FRAME_FIXED_LOC, SEEK_SET);
  if (fread(&buf, sizeof(int32_t), 1, fp) != 1) {
    fclose(fp);
    return false;
  }
  if (big_endian) {
    Edf::swap_bytes(&frame_size, buf, sizeof(int32_t));
  }
  else {
    memcpy(&frame_size, buf, sizeof(int32_t));
  }

  // go to vector size and read it
  //
  int32_t vec_size;
  fseek(fp,
	KALDI_VECTOR_FIXED_LOC - KALDI_FRAME_FIXED_LOC - sizeof(float),
	SEEK_CUR);
  if (fread(&buf, sizeof(int32_t), 1, fp) != 1) {
      fclose(fp);
      return false;
    }
  if (big_endian) {
    Edf::swap_bytes(&vec_size, buf, sizeof(int32_t));
  }
  else {
    memcpy(&vec_size, buf, sizeof(int32_t));
  }

  // calculate the difference in ark access indexes
  //
  int32_t ark_index_dif = vec_size * frame_size * sizeof(float) + header_size;

  // find the end of file
  //
  fseek(fp, 0, SEEK_END);
  long eof = ftell(fp);
  
  // find the number of channels in the file
  //
  long num_channels = 0;
  fseek(fp, 0, SEEK_SET);
  while (ftell(fp) < eof) {
    num_channels += 1;
    fseek(fp, ark_index_dif, SEEK_CUR);
  }

  // initialize the feature matrix:
  //  resize the matrix so it can fit the information from each channe;
  //
  if (feat_a.size() != num_channels) {
    Edf::resize(feat_a, num_channels, false);
  }
  
  // resize each channel of the matrix so it can fit all of the frames
  //
  for (long i = 0; i < num_channels; i++) {
    if (feat_a[i].size() != frame_size){
      Edf::resize(feat_a[i], frame_size, false);
    }

    // resize each frame of the matrix so it can fit all of the vectors
    //
    for (long j = 0; j < frame_size; j++) {
      if (feat_a[i][j].size() != vec_size) {
	Edf::resize(feat_a[i][j], vec_size, false);
      }
    }
  }
  
  // initialize variables to read features
  //
  float sum;
  float tmp_buf[vec_size];
  
  // start reading the features:
  //  Kaldi ark files are written in little endian format. This loop will loop
  //  through all of the channels in the file and skip the headers to go right
  //  to the feature information. Then, it will loop through the frames and
  //  store each vector a temporary buffer. Perform an endian swap if the
  //  system is not little endian, and then store the value into the feature
  //  matrix.
  //
  fseek(fp, 0, SEEK_SET);

  for (long i = 0; i < num_channels; i++) {

    // skip the header for each channel
    //
    fseek(fp, header_size, SEEK_CUR);

    // loop over all frames and elements
    //
    for (long j = 0; j < frame_size; j++) {

      // read the vector from the file
      //
      if (fread(tmp_buf, sizeof(float), vec_size, fp) != vec_size) {
	fclose(fp);
	return false;
      }
      
      // convert the vector to a double
      //
      for (long k = 0; k < vec_size; k++){

	// byte swap if necessary
	//
	if (big_endian) {
	  Edf::swap_bytes(&sum, &tmp_buf[k], sizeof(float));
	}
	else {
	  sum = tmp_buf[k];
	}

	// covert to single precision floating point number
	//
	feat_a[i][j][k] = sum; 
      }
    }
  }
  
  // close the file
  //
  fclose(fp);
  
  // display debugging information
  //
  if (debug_level_d >= LEVEL_DETAILED) {
    fprintf(stdout,
	    " Edf::read_features_kaldi(): end of reading features");
  }

  // exit gracefully
  //
  return true;
}

// method: write_features
//
// arguments:
//  VVVectorDouble& feat: feature data (output)
//  char* fn: output filename (input)
//  FFMT ffmt: feature file format specifier (input)
//
//
// return: a logical value indicating status
//
// This method writes feature to a file. It branches on the
// type of the file.
//
bool Edf::write_features(VVVectorDouble& feat_a, char* fn_a, FFMT ffmt_a) {

  // check for one of three things: htk, raw, or kaldi
  //
  if (ffmt_a == FFMT_HTK) {
    return Edf::write_features_htk(feat_a, fn_a);
  }

  // case 2: if it is a kaldi feature file
  //
  else if (ffmt_a == FFMT_KALDI) {
    return Edf::write_features_kaldi(feat_a, fn_a);
  }
  
  // case 3: if it is a raw feature file
  //
  else if (ffmt_a == FFMT_RAW) {
    return Edf::write_features_raw(feat_a, fn_a);
  }

  // case 4: unrecognized filetype
  //
  else {
    fprintf(stdout, "Edf::write_features(): unrecognized filetype\n");
  }

  // exit gracefully
  //
  return true;
}

// method: write_features_raw
//
// arguments:
//  VVVectorDouble& feat: matrix (input)
//  char* fn: output filename (input)
//
// return: a logical value indicating status
//
// This method writes feature vectors to a file. They are written
// frame by frame. For each frame, a vector is written.
// The file is written as a binary file in the following order:
//
//  (1) number of rows (number of channels) (4-byte int)
//  (2) number of cols (number of frames)   (4-byte int)
//  (3) channel #0, frame #0:
//      no. of features                     (4-byte int)
//      features                            (4-byte float)
//      channel #0, frame #1: ...
//      channel #0, last frame: ...
//      channel #1, frame #0: ...
// 
// All data is written in a binary format.
//
bool Edf::write_features_raw(VVVectorDouble& feat_a, char* fn_a) {

  // declare local variables
  //
  int32_t isum;
  long nbytes = 0;
  long status = true;
  
  // display debugging information
  //
  if (debug_level_d >= LEVEL_DETAILED) {
    fprintf(stdout, "Edf::write_raw(): beginning binary write\n");
  }

  // open the file
  //
  FILE* fp = fopen(fn_a, "w");
  if (fp == (FILE*)NULL) {
    return false;
  }

  // write the dimensions
  //
  int32_t dims[2];
  dims[0] = feat_a.size();
  dims[1] = feat_a[0].size();
  if (fwrite(dims, sizeof(int32_t), 2, fp) != 2) {
    fclose(fp);
    return false;
  }
  nbytes += 2 * sizeof(int32_t);

  if (debug_level_d >= LEVEL_FULL) {
    fprintf(stdout, "Edf::write_features_raw(): (nchan, nframes) %ld %ld\n",
	    (long)dims[0], (long)dims[1]);
  }

  // loop over all channels and frames
  //
  for (long i = 0; i < dims[0]; i++) {
    for (long j = 0; j < dims[1]; j++) {

      if (debug_level_d >= LEVEL_FULL) {
	fprintf(stdout,
		" Edf::write_features_raw(): (chan, nf, fdim) %ld %ld %ld\n",
		i, j, (long)feat_a[i][j].size());
      }

      // grab the vector size and write it to the file
      //
      int32_t ndim = feat_a[i][j].size();
      if (fwrite(&ndim, sizeof(int32_t), 1, fp) != 1) {      
	fclose(fp);
	return false;
      }

      // convert the double vector to a float
      //
      float tmp_buf[ndim];
      for (long k = 0; k < ndim; k++) {
	tmp_buf[k] = (float)feat_a[i][j][k];
      }

      // write the vector
      //
      if (fwrite(tmp_buf, sizeof(float), ndim, fp) != ndim) {
	fclose(fp);
	return false;
      }
      nbytes += sizeof(int32_t) + ndim * sizeof(float);
    }
  }

  // close the file
  //
  fclose(fp);

  // display debugging information
  //
  if (debug_level_d >= LEVEL_DETAILED) {
    fprintf(stdout,
	    "Edf::write_features_raw(): end of binary write [%ld bytes]\n",
	    nbytes);
  }
  
  // exit gracefully
  //
  return status;
};

// method: write_features_htk
//
// arguments:
//  VVVectorDouble& feat: feature matrix (input)
//  char* fn: output filenames - one per channel (input)
//
// return: a logical value indicating status
//
// This method writes each channel of a multichannel EDF signal into
// a one channel per file format file in an HTK binary format.
//
// See write_htk_channel for more information on why the frame duration
// is needed as an argument.
// 
// All data is written in a binary format.
//
bool Edf::write_features_htk(VVVectorDouble& feat_a, char* fn_a) {

  // display debugging information
  //
  if (debug_level_d >= LEVEL_DETAILED) {
    fprintf(stdout,
	    "Edf::write_features_htk(): begin writing htk formatted files\n");
  }

  // create a filelist
  //
  long nf = feat_a.size();
  char* fnames[nf];
  if (Edf::create_filelist(fnames, nf, fn_a, fnmod_d) == false) {
    return false;
  }

  if (debug_level_d >= LEVEL_MEDIUM) {
    for (long i = 0; i < nf; i++) {
      fprintf(stdout,
	      " Edf::write_features_htk(): channel %ld - [%s]\n",
	      i, fnames[i]);
    }
  }

  // loop over all files in the list
  //
  for(long i = 0; i < nf; i++){
    if (write_htk_channel(feat_a[i], fdur_d, fnames[i]) == false) {
      fprintf(stdout,
	      "**> Edf::write_features_htk(): error writing channel %ld - [%s]\n",
	      i, fnames[i]);
      return false;
    }
  }

  // clean up memory
  //
  Edf::cleanup(fnames, nf);

  // display debugging information
  //
  if (debug_level_d >= LEVEL_DETAILED) {
    fprintf(stdout,
	    "Edf::write_features_htk(): done writing htk formatted files\n");
  }
  
  // exit gracefully
  //
  return true;
}

// method: write_features_kaldi
//
// arguments:
//  VVVectorDouble& feat: feature matrix (input)
//  char* fn: output filename (input)
//
// return: a logical value indicating status
//
// This method writes each channel of a multichannel EDF signal into
// one file in a Kaldi ARK format.
//
bool Edf::write_features_kaldi(VVVectorDouble& feat_a, char* fn_a) {
  
  // declare local variables
  //
  unsigned char buf[sizeof(float)];
  
  // check if we need to byte swap
  //
  bool big_endian = Edf::is_big_endian();

  // display debugging information
  //
  if (debug_level_d >= LEVEL_DETAILED) {
    fprintf(stdout,
	    "Edf::write_features_kaldi(): begin writing kaldi formatted files");
    if (big_endian == false) {
      fprintf(stdout, " byte order = little endian\n");
    }
    else {
      fprintf(stdout, " byte order = big endian\n");
    }
  }

  // check the size of a float:
  //  a float must be 32 bits long for this code to work properly, so we
  //  check this and make the program crash if this isn't the case.
  //
  char static_assert_float32[1 - (2 * ((sizeof(float) * CHAR_BIT) != 32))];

  // open the output Kaldi ark file for writing
  //
  FILE* fp = fopen(fn_a, "w");
  if (fp == (FILE*)NULL){
    return false;
  }

  // get the basename of the input edf file from the header
  //
  long nc = feat_a.size();
  char* onames[nc];
  Edf::create_filelist(onames, nc, fn_a, fnmod_d);
  
  // loop through the channels of the feature matrix
  //
  for (long i = 0; i < nc; i++) {

    // remove the extension and any slashes from the name in the list
    //
    char* oname_no_ext = strtok(onames[i], DOT);
    char* token = strtok(oname_no_ext, SLASH);
    while (token != NULL) {
      oname_no_ext = token;
      token = strtok(NULL, SLASH);
    }
    
    // check the vector size:
    //  The actual vector size must be less than
    //  2**15 bytes (no. elements * 4 bytes per element).
    //
    long vec_size = feat_a[i][0].size();
    if ((vec_size * sizeof(float)) >= pow(2, sizeof(int16_t) * CHAR_BIT - 1)) {
      fprintf(stdout, "%s %s [%ld] [%ld]\n",
	      "**> Edf::write_features_kaldi():",
	      "byte count of frame is too large",
	      vec_size * sizeof(float),
	      (long)pow(2, sizeof(int16_t) * CHAR_BIT - 1));
      fclose(fp);
      return false;
    }

    //check that the vector size is the same for all frames of data
    //
    int32_t num_frames = feat_a[i].size();
    for (long j = 0; j < num_frames; j++) {
      if (feat_a[i][j].size() != vec_size) {
	fprintf(stdout, "%s %s [%ld] [%ld]\n",
		"**> Edf::write_features_kaldi():",
		"feature vectors must have the same size",
		(long)feat_a[i][j].size(), vec_size);
	fclose(fp);
	return false;
      }
    }

    // get the basename of the input edf file, and then write the header
    //
    if (fprintf(fp, "%s %c%s %c", oname_no_ext,
		NULL, KALDI_HEADER_STR, KALDI_HEADER_EOT) < 0){
      fprintf(stdout, "%s"
	      "**> Edf::write_features_kaldi():",
	      "error writing to header");
      fclose(fp);
      return false;
    }
    
    // write number of frames
    //
    if (big_endian) {
      Edf::swap_bytes(buf, &num_frames, sizeof(int32_t));
    }
    else {
      memcpy(buf, &num_frames, sizeof(int32_t));
    }
    if (fwrite(&buf, sizeof(int32_t), 1, fp) != 1){
      fclose(fp);
      return false;
    }

    // write second formatted string
    //
    if (fprintf(fp, "%c", KALDI_HEADER_EOT) < 0) {
      fprintf(stdout,
	      "%s %s - [%s]",
	      "Edf::write_features_kaldi():",
	      "error writing second formatted string",
	      fn_a);
      fclose(fp);
      return false;
    }

    // write vector size
    //
    if (big_endian) {
      Edf::swap_bytes(buf, &vec_size, sizeof(int32_t));
    }
    else {
      memcpy(buf, &vec_size, sizeof(int32_t));
    }
    if (fwrite(&buf, sizeof(int32_t), 1, fp) != 1) {
      fclose(fp);
      return false;
    }
    
    // display debugging information
    //
    if (debug_level_d >= LEVEL_DETAILED) {
      fprintf(stdout, "%s %s (%ld %ld %ld)\n",
	      "Edf::write_features_kaldi():",
	      "(nf, fdim, fdur)",
	      (long)num_frames, vec_size, (long int)fdur_d);
    }

    // initialize variables for writing the features
    //
    float sum;
    float tmp_buf[vec_size];

    // loop over all frames and vectors to write the data:
    //  Kaldi ark files are written in little endian format, so the values
    //  written from the feautre matrix to the output file may have to be
    //  byte-swapped if the system is not little endian. The data from the
    //  feature matrix is then written to the output Kaldi Ark file.
    //
    for (long j = 0; j < num_frames; j++) {
      for (long k = 0; k < vec_size; k++) {
	
	// convert to a single precision floating point number
	//
	sum = feat_a[i][j][k];

	// byte swap if necessary
	//
	if (big_endian) {
	  Edf::swap_bytes(&tmp_buf[k], &sum, sizeof(float));
	}
	else {
	  tmp_buf[k] = sum;
	}
      }

      // write the vector to the file
      //
      if (fwrite(tmp_buf, sizeof(float), vec_size, fp) != vec_size) {
	fclose(fp);
	return false;
      }
    }
  }
  
  // close the file
  //
  fclose(fp);

  // display debugging information
  //
  if (debug_level_d >= LEVEL_DETAILED) {
    fprintf(stdout, "%s %s",
	    "Edf::write_features_kaldi():",
	    "done writing kaldi formatted file\n");
  }

  // exit gracefully
  //
  return true;
}
  
// method: is_edf
//
// arguments:
//  const char* fn_a: input filename
//
// return: an boolean value that is true if the file is an EDF file
//
// This method tests whether a file is an EDF file by examining the
// first 8 bytes in the header.
//
bool Edf::is_edf(const char* fn_a) {

  // declare local variables
  //
  bool status;
  
  // open the file
  //
  FILE* fp = fopen(fn_a, "r");
  if (fp == (FILE*)NULL) {
    return false;
  }

  // read the first field
  //
  if (fread(hdr_version_d, 1, EDF_VERS_BSIZE, fp) != EDF_VERS_BSIZE) {
    fclose(fp);
    return false;
  }
  hdr_version_d[EDF_VERS_BSIZE] = (char)NULL;

  // check if it is the correct sequence
  //
  if (strcmp(hdr_version_d, EDF_VERS) == 0) {
    status = true;
  }
  else {
    status = false;
  }

  // close the file
  //
  fclose(fp);

  // exit gracefully
  //
  return status;
}

// method: is_filelist
//
// arguments:
//  char* fname: the filename to be checked (input)
//
// return: a boolean value that is true if the file contains filenames
//
// This method attempts to determine if a file is a filelist or
// a binary file. It uses a heuristic check of the byte sequences.
//
// Note that this method is extremely funky in the way it checks
// the type of file. Since HTK multichannel files use a basename,
// the file does not technically exist. Therefore, if an open
// of fname_a fails, it might possibly be an HTK file.
//
bool Edf::is_filelist(char* fname_a) {

  // declare local variables
  //
  bool status = true;

  // declare a buffer to be used to scan for an unprintable character
  //
  char buf[FLIST_BSIZE];
  
  // open the file
  //
  FILE* fp = fopen(fname_a, "r");
  if (fp == (FILE*)NULL) {
    return false;
  }

  // read a fixed number of bytes
  //
  long nbytes = fread(buf, 1, FLIST_BSIZE, fp);

  // search for non-ASCII characters
  //
  long i = 0;
  while (i < nbytes) {
    if ((!isspace(buf[i])) && (!isprint(buf[i]))) {
      status = false;
      break;
    }
    i++;
  }

  // close the file
  //
  fclose(fp);
  
  // exit gracefully
  //
  return status;
}

// method: is_raw
//
// arguments:
//  char* fname: the filename to be checked (input)
//
// return: a boolean value that is true if the file is a raw file
//
// This method looks for specific header information, such as number
// of channels. It is not very robust and is very heuristic.
//
bool Edf::is_raw(char* fname_a) {

  // declare local variables
  //
  bool status = false;
  unsigned char buf[FLIST_BSIZE];

  // open the file
  //
  FILE* fp = fopen(fname_a, "r");
  if (fp == (FILE*)NULL) {
    return false;
  }

  // read a block and look for non-ASCII characters
  //
  long nbytes = fread(buf, sizeof(unsigned char), FLIST_BSIZE, fp);
  if (nbytes <= 0) {
    fclose(fp);
    return false;
  }
  long n = 0;
  long non_ascii = false;
  while (n < nbytes) {
    if ((!isspace(buf[n])) && (!isprint(buf[n]))) {
      non_ascii = true;
      break;
    }
    n++;
  }
  if (non_ascii == false) {
    fclose(fp);
    return false;
  }

  // read and check the dimensions
  //
  int32_t dims[2];
  rewind(fp);
  if (fread(dims, sizeof(int32_t), 2, fp) != 2) {
    fclose(fp);
    return false;
  }
  if ((dims[0] < 0) || (dims[1] < 0)) {
    fclose(fp);
    return false;
  }

  // close the file
  //
  fclose(fp);

  // exit gracefully
  //
  return true;
}

// method: is_htk
//
// arguments:
//  char* fname: the filename to be checked (input)
//
// return: a boolean value that is true if the file is an htk file
//
// This method looks for specific header information, such as the htk code.
//
bool Edf::is_htk(char* fname_a) {

  // declare local variables
  //
  double fdur;
  bool status = false;
  unsigned char buf[4];

  // build the modified filename
  //
  char* onames[1];
  Edf::create_filelist(onames, 1, fname_a, fnmod_d);

  // check if we need to byte swap
  //
  bool big_endian = Edf::is_big_endian();

  // display debugging information
  //
  if (debug_level_d >= LEVEL_DETAILED) {
    fprintf(stdout, " Edf::is_htk(): beginning check [%s] ", fname_a);
    if (big_endian == false) {
      fprintf(stdout, " byte order = little endian\n");
    }
    else {
      fprintf(stdout, " byte order = big endian\n");
    }
  }

  // check the size of a float:
  //  a float must be 32 bits long for this code to work properly. so we
  //  check this and make the program crash if this isn't the case.
  //
  char static_assert_float32[1 - (2 * ((sizeof(float) * CHAR_BIT) != 32))];

  // open the file for reading
  //
  FILE* fp = fopen(onames[0], "r");
  if (fp == (FILE*)NULL) {
    return false;
  }

  // read the number of frames
  //
  int32_t num_frames;

  if (fread(&buf, sizeof(int32_t), 1, fp) != 1) {
    fclose(fp);
    return false;
  }
  if (!big_endian) {
    Edf::swap_bytes(&num_frames, buf, sizeof(int32_t));
  }
  else {
    memcpy(&num_frames, buf, sizeof(int32_t));
  }

  // read the frame duration in 100 mic-sec units
  //
  int32_t ifdur;
  if (fread(&buf, sizeof(int32_t), 1, fp) != 1) {
    fclose(fp);
    return false;
  }
  if (!big_endian) {
    Edf::swap_bytes(&ifdur, buf, sizeof(int32_t));
  }
  else {
    memcpy(&ifdur, buf, sizeof(int32_t));
  }
  fdur = ifdur * HTK_FDUR_SCALE;

  // read the sample size: how many bytes in an entire frame
  //
  int16_t sample_size;
  if (fread(&buf, sizeof(int16_t), 1, fp) != 1) {
    fclose(fp);
    return false;
  }
  if (!big_endian) {
    Edf::swap_bytes(&sample_size, buf, sizeof(int16_t));
  }
  else {
    memcpy(&sample_size, buf, sizeof(int16_t));
  }
  long vec_size = sample_size / sizeof(float);

  // read the HTK code
  //
  int16_t htk_code;
  if (fread(&buf, sizeof(int16_t), 1, fp) != 1) {
    fclose(fp);
    return false;
  }
  if (!big_endian) {
    Edf::swap_bytes(&htk_code, buf, sizeof(int16_t));
  }
  else {
    memcpy(&htk_code, buf, sizeof(int16_t));
  }

  // check the htk code to determine if this is an htk file
  //
  if ((htk_code == 9) && (num_frames > 0) && (vec_size > 0) &&
      (fdur > 0) && (sample_size > 0)) {
    status = true;
  }
  else {
    status = false;
  }

  // close the file
  //
  fclose(fp);

  // display debugging information
  //
  if (debug_level_d >= LEVEL_DETAILED) {
    fprintf(stdout, "%s %s (%ld %ld %f %ld %ld)\n",
	    "  Edf::htk_read_channel():",
	    "(nf, fdim, fdur, ssize, htk_code)",
	    (long)num_frames, vec_size, fdur, (long)sample_size, (long)htk_code);
  }
  
  // clean up memory
  //
  delete [] onames[0];

  // exit gracefully
  //
  return status;
}

// method: is_kaldi
//
// arguments:
//  char* fname: the filename to be checked (input)
//
// return: a boolean value that is true if the file is a kaldi file
//
// This method looks for header information at kaldi-specific byte locations.
//
bool Edf::is_kaldi(char *fname_a) {

  // declare local variables
  //
  unsigned char buf[sizeof(float)];
  bool status = false;

  // check if we need to byte swap
  //
  bool big_endian = Edf::is_big_endian();

  // display debugging information
  //
  if (debug_level_d >= LEVEL_DETAILED) {
    fprintf(stdout, " Edf::is_kaldi(): beginning check [%s] ", fname_a);
    if (big_endian == false) {
      fprintf(stdout, " byte order = little endian\n");
    }
    else {
      fprintf(stdout, " byte order = big endian\n");
    }
  }

  // open the file for reading
  //
  FILE* fp = fopen(fname_a, "r");
  if (fp == (FILE*)NULL) {
    return false;
  }

  // get size of header
  //
  char hname[MAX_MSTR_LENGTH];

  // check the size of a float:
  //  a float most be 32 bits long for this code to work properly. So we
  //  check this and make the program crash if this isn't the case.
  //
  char static_assert_float32[1 - (2 * ((sizeof(float) * CHAR_BIT) != 32))];

  // get the size of the name from the header
  //
  long hname_size = strlen(hname);

  // go to end of name in header
  //
  fseek(fp, hname_size, SEEK_SET);

  // go to BFM string and read it
  //
  char bfm_str[MAX_SSTR_LENGTH];
  fseek(fp, 1, SEEK_CUR);
  if (fread(&bfm_str, strlen(KALDI_HEADER_STR), 1, fp) != 1) {
    fclose(fp);
    return false;
  }
  
  // go to frame size and read it
  //
  int32_t frame_size;
  fseek(fp, KALDI_FRAME_FIXED_LOC + hname_size, SEEK_SET);
  if (fread(&buf, sizeof(int32_t), 1, fp) != 1) {
      fclose(fp);
      return false;
    }
  if (big_endian) {
    Edf::swap_bytes(&frame_size, buf, sizeof(int32_t));
  }
  else {
    memcpy(&frame_size, buf, sizeof(int32_t));
  }

  // go to vector size and read it
  //
  int32_t vec_size;
  fseek(fp,
	KALDI_VECTOR_FIXED_LOC - KALDI_FRAME_FIXED_LOC - sizeof(float),
	SEEK_CUR);
  if (fread(&buf, sizeof(int32_t), 1, fp) != 1) {
    fclose(fp);
    return false;
  }
  if (big_endian) {
    Edf::swap_bytes(&vec_size, buf, sizeof(int32_t));
  }
  else {
    memcpy(&vec_size, buf, sizeof(int32_t));
  }

  // check the values read from the file to see if it is a kaldi ark file
  //
  if ((frame_size > 0) && (vec_size > 0) &&
      !strcmp(bfm_str, KALDI_HEADER_STR)) {
    status = true;
  }

  // close the file
  //
  fclose(fp);

  // display debugging information
  //
  if (debug_level_d >= LEVEL_DETAILED) {
    fprintf(stdout, "%s %s (%s %ld %ld)\n",
	    "  Edf::is_kaldi():",
	    "(modified file name from header, frame size, vector size)",
	    hname, (long)frame_size, (long)vec_size);
  }

  // exit gracefully
  //
  return status;
}

// method: debug
//
// arguments:
//  VVectorDouble& sig: the signal to be debugged (input)
//  FILE* fp: the output file pointer (input)
//
// return: a logical value indicating status
//
// This method generates some simple debug information for a signal.
//
bool Edf::debug(VVectorDouble& sig_a, FILE* fp_a) {

  // display the channel information
  //
  fprintf(fp_a, "no. channels = %ld\n", (long)sig_a.size());
  fprintf(fp_a, "no. samples per channel = %ld\n", (long)sig_a[0].size());

  // display some signal values
  //
  long max_nchans = (long)Edf::min(3, sig_a.size());
  long max_nsamps = (long)Edf::min(3, sig_a[0].size());

  for (long i = 0; i < max_nchans; i++) {
    fprintf(fp_a, "channel number %ld:\n", i);
    for (long j = 0; j < max_nsamps; j++) {
      fprintf(fp_a, "sig[%ld][%ld] = %f\n", i, j, sig_a[i][j]);
    }
  }

  // exit gracefully
  //
  return true;
}

// method: debug
//
// arguments:
//  VVVectorDouble& feat: the feature stream to be debugged (input)
//  FILE* fp: the output file pointer (input)
//
// return: a logical value indicating status
//
// This method generates some simple debug information for a feature stream.
//
bool Edf::debug(VVVectorDouble& feat_a, FILE* fp_a) {

  // display the channel information
  //
  fprintf(fp_a, "no. channels = %ld\n", (long)feat_a.size());
  fprintf(fp_a, "no. frames per channel = %ld\n", (long)feat_a[0].size());
  fprintf(fp_a, "no. features per frame = %ld\n", (long)feat_a[0][0].size());

  // display some signal values
  //
  long max_nchans = (long)Edf::min(3, feat_a.size());
  long max_nframes = (long)Edf::min(3, feat_a[0].size());

  for (long i = 0; i < max_nchans; i++) {
    fprintf(fp_a, "channel number %ld:\n", i);
    for (long j = 0; j < max_nframes; j++) {
      fprintf(fp_a, "frame number %ld:\n", j);
      for (long k = 0; j < feat_a[i][j].size(); j++) {
	fprintf(fp_a, "feat[%ld][%ld][%ld] = %f\n", i, j, k, feat_a[i][j][k]);
      }
    }
  }

  // exit gracefully
  //
  return true;
}

// method: read_htk_channel
//
// arguments:
//  VVectorDouble& feat: feature matrix (output)
//  double& fdur: frame duration used by the features (output)
//  char* fn: output filename (input)
//
// return: a logical value indicating status
//
// This method read a single channel of HTK feature data from a file.
// 
// All data is written in a binary format.
//
bool Edf::read_htk_channel(VVectorDouble& feat_a, double& fdur_a, char* fn_a) {

  // declare local variables
  //
  unsigned char buf[4];

  // check if we need to byte swap
  //
  bool big_endian = Edf::is_big_endian();

  // display debugging information
  //
  if (debug_level_d >= LEVEL_DETAILED) {
    fprintf(stdout, " Edf::read_htk_channel(): beginning read [%s] ", fn_a);
    if (big_endian == false) {
      fprintf(stdout, " byte order = little endian\n");
    }
    else {
      fprintf(stdout, " byte order = big endian\n");
    }
  }

  // check the size of a float:
  //  a float must be 32 bits long for this code to work properly. so we
  //  check this and make the program crash if this isn't the case.
  //
  char static_assert_float32[1 - (2 * ((sizeof(float) * CHAR_BIT) != 32))];

  // open the file for reading
  //
  FILE* fp = fopen(fn_a, "r");
  if (fp == (FILE*)NULL) {
    return false;
  }

  // read the number of frames
  //
  int32_t num_frames;

  if (fread(&buf, sizeof(int32_t), 1, fp) != 1) {
    fclose(fp);
    return false;
  }
  if (!big_endian) {
    Edf::swap_bytes(&num_frames, buf, sizeof(int32_t));
  }
  else {
    memcpy(&num_frames, buf, sizeof(int32_t));
  }
  if (feat_a.size() != num_frames) {
    Edf::resize(feat_a, num_frames, false);
  }

  // read the frame duration in 100 msec units
  //
  int32_t ifdur;
  if (fread(&buf, sizeof(int32_t), 1, fp) != 1) {
    fclose(fp);
    return false;
  }
  if (!big_endian) {
    Edf::swap_bytes(&ifdur, buf, sizeof(int32_t));
  }
  else {
    memcpy(&ifdur, buf, sizeof(int32_t));
  }
  fdur_a = ifdur * HTK_FDUR_SCALE;

  // read the sample size: how many bytes in an entire frame
  //
  int16_t sample_size;
  if (fread(&buf, sizeof(int16_t), 1, fp) != 1) {
    fclose(fp);
    return false;
  }
  if (!big_endian) {
    Edf::swap_bytes(&sample_size, buf, sizeof(int16_t));
  }
  else {
    memcpy(&sample_size, buf, sizeof(int16_t));
  }
  long vec_size = sample_size / sizeof(float);

  // read the HTK code
  //
  int16_t htk_code;
  if (fread(&buf, sizeof(int16_t), 1, fp) != 1) {
    fclose(fp);
    return false;
  }
  if (!big_endian) {
    Edf::swap_bytes(&htk_code, buf, sizeof(int16_t));
  }
  else {
    memcpy(&htk_code, buf, sizeof(int16_t));
  }
  if (htk_code != 9) {
    fprintf(stdout,
	    "**> Edf::read_htk_channel(): error reading htk code [%d != 9]\n",
	    htk_code);
    fclose(fp);
    return false;
  }

  // display debugging information
  //
  if (debug_level_d >= LEVEL_DETAILED) {
    fprintf(stdout, "%s %s (%ld %ld %f %ld %ld)\n",
	    "  Edf::htk_read_channel():",
	    "(nf, fdim, fdur, ssize, htk_code)",
	    (long)num_frames, vec_size, fdur_a, (long)sample_size, (long)htk_code);
  }
  
  // loop over all frames and all elements
  //
  float sum;
  float tmp_buf[vec_size];

  for (long i = 0; i < num_frames; i++){

    // make space for the output
    //
    if (feat_a[i].size() != vec_size) {
      Edf::resize(feat_a[i], vec_size, false);
    }

    // read the vector from the file
    //
    if (fread(tmp_buf, sizeof(float), vec_size, fp) != vec_size) {
      fclose(fp);
      return false;
    }
    
    // convert the vector to a double
    //
    for (long j = 0; j < vec_size; j++) {

      // byte swap if necessary
      //
      if (!big_endian) {
	Edf::swap_bytes(&sum, &tmp_buf[j], sizeof(float));
      }
      else {
	sum = tmp_buf[j];
      }

      // convert to a single precision floating point number
      //
      feat_a[i][j] = sum;
    }
  }

  // close the file
  //
  fclose(fp);

  // display debugging information
  //
  if (debug_level_d >= LEVEL_DETAILED) {
    fprintf(stdout,
	    " Edf::read_htk_channel(): end of htk_channel read [%s]\n",
	    fn_a);
  }
  
  // exit gracefully
  //
  return true;
}

// method: write_htk_channel
//
// arguments:
//  VVectorDouble& feat: feature matrix (input)
//  double fdur: frame duration used by the features (input)
//  char* fn: output filename (input)
//
// return: a logical value indicating status
//
// This method writes feature vectors to a file in an HTK binary format.
// 
// All data is written in a binary format.
//
bool Edf::write_htk_channel(VVectorDouble& feat_a, double fdur_a, char* fn_a) {

  // declare local variables
  //
  unsigned char buf[4];

  // check if we need to byte swap
  //
  bool big_endian = Edf::is_big_endian();

  // display debugging information
  //
  if (debug_level_d >= LEVEL_DETAILED) {
    fprintf(stdout, "Edf::write_htk_channel(): beginning write");
    if (big_endian == false) {
      fprintf(stdout, " byte order = little endian\n");
    }
    else {
      fprintf(stdout, " byte order = big endian\n");
    }
  }

  // check the size of a float:
  //  a float must be 32 bits long for this code to work properly. so we
  //  check this and make the program crash if this isn't the case.
  //
  char static_assert_float32[1 - (2 * ((sizeof(float) * CHAR_BIT) != 32))];

  // check the vector size:
  //  The actual vector size must be less than
  //  2**15 bytes (no. elements * 4 bytes per element).
  //
  long vec_size = feat_a[0].size();
  
  if  ((vec_size * sizeof(float)) >= pow(2, sizeof(int16_t) * CHAR_BIT - 1)) {
    fprintf(stdout, "%s %s [%ld] [%ld]\n",
	    "**> Edf::write_htk_channel():",
	    "byte count of frame is too large",
	    vec_size * sizeof(float),
	    (long)pow(2, sizeof(int16_t) * CHAR_BIT - 1));
    return false;
  }

  // check that the vector size is the same for all frames of data
  //
  int32_t num_frames = feat_a.size();

  for (long i = 0; i < num_frames; i++){
    if (feat_a[i].size() != vec_size) {
      fprintf(stdout, "%s %s [%ld] [%ld]\n",
	      "**> Edf::write_htk_channel():",
	      "feature vectors must have the same size",
	      (long)feat_a[i].size(), vec_size);
      return false;
    }
  }
      
  // open the file for writing
  //
  FILE* fp = fopen(fn_a, "w");
  if (fp == (FILE*)NULL) {
    return false;
  }

  // write number of frames
  //
  if (!big_endian) {
    Edf::swap_bytes(buf, &num_frames, sizeof(int32_t));
  }
  else {
    memcpy(buf, &num_frames, sizeof(int32_t));
  }
  if (fwrite(&buf, sizeof(int32_t), 1, fp) != 1) {
    fclose(fp);
    return false;
  }

  // write frame duration in 100 mic-sec units
  //
  int32_t ifdur = round(fdur_a / HTK_FDUR_SCALE);
  if (!big_endian) {
    Edf::swap_bytes(buf, &ifdur, sizeof(int32_t));
  }
  else {
    memcpy(buf, &ifdur, sizeof(int32_t));
  }
  if (fwrite(&buf, sizeof(int32_t), 1, fp) != 1) {
    fclose(fp);
    return false;
  }

  // write sample size: how many bytes in an entire frame
  //
  int16_t sample_size = vec_size * sizeof(float);
  if (!big_endian) {
    Edf::swap_bytes(buf, &sample_size, sizeof(int16_t));
  }
  else {
    memcpy(buf, &sample_size, sizeof(int16_t));
  }
  if (fwrite(&buf, sizeof(int16_t), 1, fp) != 1) {
    fclose(fp);
    return false;
  }

  // write HTK code:
  //
  //  A two-byte integer that determines what is in the file.
  //  The first 6 bits are the data type (9 means user defined)
  //  and other bits are used for energy, existence of delta and
  //  delta-delta, comperssion etc. Here we just need the simple case
  //  with its 16 bits set to: 0000000000001001 = 9
  // 
  int16_t htk_code = 9;
  if (!big_endian) {
    Edf::swap_bytes(buf, &htk_code, sizeof(int16_t));
  }
  else {
    memcpy(buf, &htk_code, sizeof(int16_t));
  }
  if (fwrite(&buf, sizeof(int16_t), 1, fp) != 1) {
    fclose(fp);
    return false;
  }

  // display debugging information
  //
  if (debug_level_d >= LEVEL_DETAILED) {
    fprintf(stdout, "%s %s (%ld %ld %f %ld %ld)\n",
	    "  Edf::htk_write_channel():",
	    "(nf, fdim, fdur, ssize, htk_code)",
	    (long)num_frames, vec_size, fdur_a, (long)sample_size, (long)htk_code);
  }
  
  // loop over all frames and all elements
  //
  float sum;
  float tmp_buf[vec_size];

  for (long i = 0; i < num_frames; i++){
    for (long j = 0; j < vec_size; j++) {

      // convert to a single precision floating point number
      //
      sum = feat_a[i][j];

      // byte swap if necessary
      //
      if (!big_endian) {
	Edf::swap_bytes(&tmp_buf[j], &sum, sizeof(float));
      }
      else {
	tmp_buf[j] = sum;
      }
    }
    
    // write the vector to the file
    //
    if (fwrite(tmp_buf, sizeof(float), vec_size, fp) != vec_size) {
      fclose(fp);
      return false;
    }
  }
  
  // close the file
  //
  fclose(fp);

  // display debugging information
  //
  if (debug_level_d >= LEVEL_DETAILED) {
    fprintf(stdout,
	    " Edf::write_htk_channel(): end of htk_channel write [%s]\n",
	    fn_a);
  }
  
  // exit gracefully
  //
  return true;
}

// method: is_big_endian
//
// arguments: none
//
// return: a logical value that is true if the machine architecture
//         is big endian
//
// HTK writes data in big endian format, so we must byte-swap if we are
// on a little endian machine.
// 
// All data is written in a binary format.
//
bool Edf::is_big_endian() {

  // declare a two-byte int to have a value of 1
  //
  unsigned int i = 1;
  char* c = (char*) &i;

  // if the first byte is set, it is little Endian
  //
  if (*c) {
    return false;
  }

  // else: big endian
  //
  else {
    return true;
  }
}

// method: swap_bytes
//
// arguments:
//
// return: a logical value that is true
//
// This is meant to be a highly portable method for swapping bytes. Only
// two-byte and four-byte swaps are currently supported.
// 
bool Edf::swap_bytes(void* buf_a, void* value_a, long nbytes) {

  // declare some pointers to avoid compilter warnings on type conversions
  //
  unsigned char* dst = (unsigned char*)buf_a;
  unsigned char* src = (unsigned char*)value_a;

  // case 1: two bytes - flip the bytes
  //
  if (nbytes == 2) {
    dst[0] = src[1];
    dst[1] = src[0];
  }

  // case 2: four bytes - reindex the bytes
  //
  else if (nbytes == 4) {
    dst[0] = src[3];
    dst[1] = src[2];
    dst[2] = src[1];
    dst[3] = src[0];
  }

  // else: unsupported type
  //
  else {
    return false;
  }

  // exit gracefully
  //
  return true;
}

//
// end of file
