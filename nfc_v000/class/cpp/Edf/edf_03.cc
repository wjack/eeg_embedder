// file: $(NEDC_NFC)/class/cpp/Edf/edf_03.cc
//
// This file contains private methods associated with the class Edf.
//

// Revision History
//
//  20160801 (JM): increased flexibility of the replace directory option
//  20160722 (SL): added create_directory method and replace directory
//                 functionality
//  20150630 (FG): check_montage updated to output missing channels regarding
//                 verbosity level selected
//
//  20150623 (GS): modified the parse_montage method
//
//  20150613 (FG): added check_montage functionallity
//    		   methods added: check_montage
//                   		  get_montage_channels
//
//  20150612 (FG): added interpolation functionallity
//     		   methods added: get_channel_pos
//                                        
//  20150522 (SL): modified size of tbuf to account for null character
//
// local include files
//
#include "Edf.h"

// method: get_labels
//
// arguments:
//  char** labels:  output filename (input / output)
//
// return: an integer value containing the number of labels
//
// This method returns the labels found in an EDF header.
//
long Edf::get_labels(char** labels_a) {

  // get the number of channels
  //
  long num_channels = Edf::get_num_channels_file();
  for (int i = 0; i < num_channels; i++) {
    strcpy(labels_a[i], hdr_chan_labels_d[i]);
  }

  // exit gracefully
  //
  return num_channels;
}

// method: create_filename
//
// arguments:
//  char* oname:  output filename (output)
//  char* iname:  input filename (input)
//  char* odir:   output directory (input)
//  char* oext:   output extension (input)
//
// return: a boolean indicating status
//
// This method parses a filename and generates a new directory name
// and file extension (e.g., ./foo.edf => /data/isip/foo.feat).
// The parser is very rigid. Note:
//  (1) if odir is empty, the new file is put in the same directory
//      as the input.
//
bool Edf::create_filename(char* oname_a, char* iname_a,
			  char* odir_a, char* oext_a, char* odir_repl_a) {

  // resolve the absolute path of the filename
  //
  char abs_path[MAX_MSTR_LENGTH];
  //realpath(iname_a, abs_path);
  strcpy(iname_a, abs_path);

  // remove the dsk_raid portion of the absolute path
  //
  char tmp[MAX_MSTR_LENGTH];
  iname_a++;
  strcpy(tmp, iname_a);
  char* tmp_fixed = index(tmp, (int)SLASH[0]);
  strcpy(iname_a, tmp_fixed);
  
  // locate the file extension delimiter and directory delimiter
  //
  char* epos = rindex(iname_a, (int)DOT[0]);
  char* dpos = rindex(iname_a, (int)SLASH[0]);
  long dlen = dpos - iname_a;
  long olen = 0;
  long odir_len = strlen(odir_a);
  long odir_repl_len = strlen(odir_repl_a);
  long iname_len = strlen(iname_a);
  long dpos_len = strlen(dpos);

  //local variables
  //
  bool status;
  long odir_dpos_len = 0;
  
  // find the last slash of the output directory
  //
  char* odir_dpos = rindex(odir_a, (int)SLASH[0]);
  if (odir_dpos != NULL) {
    odir_dpos_len = strlen(odir_dpos);
  }
  
  // if the last slash is at the very end, remove it
  //
  if (odir_dpos_len == 1) {
    odir_a[odir_len - 1] = (char)NULL;
    odir_len--;
  }

  // branch if the replace directory option is set
  //
  if (strcmp(odir_repl_a, NULL_NAME) != 0) {

    // find the last slash of the replace directory
    //
    char* rdir_dpos = rindex(odir_repl_a, (int)SLASH[0]);
    long rdir_dpos_len = strlen(rdir_dpos);
    
    // if the last slash is at the very end, remove it
    //
    if (rdir_dpos_len == 1) {
      odir_repl_a[odir_repl_len - 1] = (char)NULL;
      odir_repl_len--;
    }
  
    // get the dirname of the input file
    //
    char idir[MAX_MSTR_LENGTH];
    strncpy(idir, iname_a, iname_len - dpos_len);
    *(idir + iname_len - dpos_len) = (char)NULL;
    
    // compare the replace directory to the parent directory of the
    // input directory
    //
    if (strcmp(odir_repl_a, idir) != 0 ) {
      
      // define temporary variables for the replacement
      //
      char tmp_buff[MAX_MSTR_LENGTH];
      char tmp[MAX_MSTR_LENGTH];

      // copy the input path name minus the replaced part to the tmp buffer
      //
      strcpy(tmp_buff, iname_a + odir_repl_len);

      // get size of buffer
      //
      long tmp_len = strlen(tmp_buff);

      // copy the part of the path that will be appended to the output minus
      // the filename to tmp
      //
      strncpy(tmp, tmp_buff, tmp_len-dpos_len);
      *(tmp + tmp_len - dpos_len) = (char)NULL;

      // append the path-replace to the output directory and restore the odir
      //
      strcpy(tmp_buff, odir_a);
      strcat(tmp_buff, tmp);

      odir_a = tmp_buff;

      // create necessary directories
      //
      if ((status = create_directory(tmp_buff)) == false) {
	fprintf(stdout, "**> error in Edf::create_filename(): %s\n",
		(char*)odir_a);
      }
    }
    // if the paths are the same, print a warning message
    //
    else {
      fprintf(stdout, "%s %s\n",
	      "**> Edf::create_filename():",
	      "replace directory and input file directory match");	      
    }
  }

  fprintf(stdout, "building directory portion of filename\n");
  
  // build the directory part of the filename:
  //
  // case: output directory is not specified
  //
  if (strcmp(odir_a, NULL_NAME) == 0) {
    if (dpos != (char*)NULL) {
      strncpy(oname_a, iname_a, dpos - iname_a);
      *(oname_a + dlen) = (char)NULL;
      olen += dpos - iname_a;
    }
    else {
      *oname_a = (char)NULL;
      olen = 0;
    }
  }

  // case: output directory is specified or doesn't exist
  //
  else {

    // make sure there is a directory path in the name
    //
    if (dpos != (char*)NULL) {
      strcpy(oname_a, odir_a);
      olen += strlen(odir_a);
    }

    // else if a new directory is specified
    //
    else if (odir_len != 0) {
      strcpy(oname_a, odir_a);
      olen += odir_len;
    }

    // else: no directory path
    //
    else {
      *oname_a = (char)NULL;
      olen = 0;
    }
  }

  // add a directory extension if a directory was specified
  //
  if (olen > 0) {
    strcat(oname_a, SLASH);
    olen++;
  }

  // concatenate the basename
  //
  if (dpos != (char)NULL) {
    strncat(oname_a, dpos + 1, epos - dpos - 1);
    olen += epos - dpos - 1;
  }
  else {
    strncat(oname_a, iname_a, epos - iname_a);
    olen += epos - iname_a;
  }
  oname_a[olen] =(char)NULL;

  // add the extension
  //
  if (strcmp(oext_a, NULL_NAME) == 0) {
    return false;
  }

  oname_a[olen] = DOT[0];
  oname_a[olen + 1] = (char)NULL;
  olen++;

  strcat(oname_a, oext_a);

  // exit gracefully
  //
  return true;
}

// method: create_filelist
//
// arguments:
//  char** onames:  output filelist (output)
//  long num_names: the number of names to be created
//  char* iname:    input filename (input)
//  char* omod:     output filename permuter (input)
//
// return: a boolean indicating status
//
// This method essentially permutes filenames according to the template
// provided in the omod argument. Typically, it creates names of the form:
//
//  foo_ch00.edf
//  foo_ch01.edf
//  ...
//
// This method is used to convert multichannel edf files to htk format.
//
bool Edf::create_filelist(char** onames_a, long num_names_a,
			  char* iname_a, char* omod_a) {

  // declare local variables
  //
  char tmp_buf[MAX_LSTR_LENGTH];
  char tmp_mod[MAX_MSTR_LENGTH];

  // find the delimiter
  //
  char* delim = rindex(iname_a, DOT[0]);

  // loop over all filenames
  //
  for (long i = 0; i < num_names_a; i++) {

    // copy the first part of the name
    //
    strncpy(tmp_buf, iname_a, delim - iname_a);
    tmp_buf[delim - iname_a] = (char)NULL;
  
    // add the modifier
    //
    sprintf(tmp_mod, omod_a, i);
    strcat(tmp_buf, tmp_mod);

    // add the last part of the name back
    //
    strcat(tmp_buf, delim);

    // create space and copy
    //
    onames_a[i] = new char[strlen(tmp_buf) + 1];
    strcpy(onames_a[i], tmp_buf);
  }

  // exit gracefully
  //
  return true;
}

// method: print_header
//
// arguments:
//  FILE* fp: file pointer to send the output (input)
//  char* prefix: a prefix to use before each field is printed
//
// return: an boolean value indicating status
//
// This method dumps the header to a file pointer (typically stdout).
// It is assummed the header is already in memory.
//
bool Edf::print_header(FILE* fp_a, char* prefix_a) {

  // (1) version information
  //
  fprintf(fp_a, "%sBlock 1: Version Information\n", prefix_a);
  fprintf(fp_a, "%s version = [%s]\n\n", prefix_a, hdr_version_d);

  // (2) local patient information
  //
  fprintf(fp_a, "%sBlock 2: Local Patient Information\n", prefix_a);
  fprintf(fp_a, "%s lpti_patient_id = [%s]\n",
	  prefix_a, hdr_lpti_patient_id_d);
  fprintf(fp_a, "%s lpti_gender = [%s]\n",
	  prefix_a, hdr_lpti_gender_d);
  fprintf(fp_a, "%s lpti_dob = [%s]\n",
	  prefix_a, hdr_lpti_dob_d);
  fprintf(fp_a, "%s lpti_full_name = [%s]\n",
	  prefix_a, hdr_lpti_full_name_d);
  fprintf(fp_a, "%s lpti_age = [%s]\n\n",
	  prefix_a, hdr_lpti_age_d);

  // (3) local recording information
  //
  fprintf(fp_a, "%sBlock 3: Local Recording Information\n", prefix_a);
  fprintf(fp_a, "%s lrci_start_date_label = [%s]\n",
	  prefix_a, hdr_lrci_start_date_label_d);
  fprintf(fp_a, "%s lrci_start_date = [%s]\n",
	  prefix_a, hdr_lrci_start_date_d);
  fprintf(fp_a, "%s lrci_eeg_id = [%s]\n",
	  prefix_a, hdr_lrci_eeg_id_d);
  fprintf(fp_a, "%s lrci_tech = [%s]\n",
	  prefix_a, hdr_lrci_tech_d);
  fprintf(fp_a, "%s lrci_machine = [%s]\n\n",
	  prefix_a, hdr_lrci_machine_d);

  // (4) general header information
  //
  fprintf(fp_a, "%sBlock 4: General Header Information\n", prefix_a);
  fprintf(fp_a, "%s ghdi_start_date = [%s]\n",
	  prefix_a, hdr_ghdi_start_date_d);
  fprintf(fp_a, "%s ghdi_start_time = [%s]\n",
	  prefix_a, hdr_ghdi_start_time_d);
  fprintf(fp_a, "%s ghdi_hsize = [%ld]\n",
	  prefix_a, hdr_ghdi_hsize_d);
  fprintf(fp_a, "%s ghdi_file_type = [%s]\n",
	  prefix_a, hdr_ghdi_file_type_d);
  fprintf(fp_a, "%s ghdi_reserved = [%s]\n",
	  prefix_a, hdr_ghdi_reserved_d);
  fprintf(fp_a, "%s ghdi_num_recs = [%ld]\n",
	  prefix_a, hdr_ghdi_num_recs_d);
  fprintf(fp_a, "%s ghdi_dur_rec = [%ld]\n",
	  prefix_a, hdr_ghdi_dur_rec_d);
  fprintf(fp_a, "%s ghdi_nsig_rec = [%ld]\n\n",
	  prefix_a, hdr_ghdi_nsig_rec_d);

  // (5) channel-specific information
  //
  fprintf(fp_a, "%sBlock 5: Channel-Specific Information\n", prefix_a);
  fprintf(fp_a,"%s chan_labels (%ld) = ", prefix_a, hdr_ghdi_nsig_rec_d);

  for (int i = 0; i < hdr_ghdi_nsig_rec_d - 1; i++) {
    fprintf(fp_a, "[%s], ", hdr_chan_labels_d[i]);
  }
  fprintf(fp_a, "[%s]\n", hdr_chan_labels_d[hdr_ghdi_nsig_rec_d - 1]);
  
  fprintf(fp_a,"%s chan_trans_type (%ld) = ", prefix_a, hdr_ghdi_nsig_rec_d);
  for (int i = 0; i < hdr_ghdi_nsig_rec_d - 1; i++) {
    fprintf(fp_a, "[%s], ", hdr_chan_trans_type_d[i]);
    fprintf(stdout,"[%s], ", hdr_chan_trans_type_d[i]);
  }
  fprintf(fp_a, "[%s]\n", hdr_chan_trans_type_d[hdr_ghdi_nsig_rec_d - 1]);

  fprintf(fp_a,"%s chan_phys_dim (%ld) = ", prefix_a, hdr_ghdi_nsig_rec_d);
  for (int i = 0; i < hdr_ghdi_nsig_rec_d - 1; i++) {
    fprintf(fp_a, "[%s], ", hdr_chan_phys_dim_d[i]);
  }
  fprintf(fp_a, "[%s]\n", hdr_chan_phys_dim_d[hdr_ghdi_nsig_rec_d - 1]);

  fprintf(fp_a,"%s chan_phys_min (%ld) = ", prefix_a, hdr_ghdi_nsig_rec_d);
  for (int i = 0; i < hdr_ghdi_nsig_rec_d - 1; i++) {
    fprintf(fp_a, "[%10.3f], ", hdr_chan_phys_min_d[i]);
  }
  fprintf(fp_a, "[%10.3f]\n", hdr_chan_phys_min_d[hdr_ghdi_nsig_rec_d - 1]);

  fprintf(fp_a,"%s chan_phys_max (%ld) = ", prefix_a, hdr_ghdi_nsig_rec_d);
  for (int i = 0; i < hdr_ghdi_nsig_rec_d - 1; i++) {
    fprintf(fp_a, "[%10.3f], ", hdr_chan_phys_max_d[i]);
  }
  fprintf(fp_a, "[%10.3f]\n", hdr_chan_phys_max_d[hdr_ghdi_nsig_rec_d - 1]);

  fprintf(fp_a,"%s chan_dig_min (%ld) = ", prefix_a, hdr_ghdi_nsig_rec_d);
  for (int i = 0; i < hdr_ghdi_nsig_rec_d - 1; i++) {
    fprintf(fp_a, "[%10ld], ", hdr_chan_dig_min_d[i]);
  }
  fprintf(fp_a, "[%10ld]\n", hdr_chan_dig_min_d[hdr_ghdi_nsig_rec_d - 1]);

  fprintf(fp_a,"%s chan_dig_max (%ld) = ", prefix_a, hdr_ghdi_nsig_rec_d);
  for (int i = 0; i < hdr_ghdi_nsig_rec_d - 1; i++) {
    fprintf(fp_a, "[%10ld], ", hdr_chan_dig_max_d[i]);
  }
  fprintf(fp_a, "[%10ld]\n", hdr_chan_dig_max_d[hdr_ghdi_nsig_rec_d - 1]);

  fprintf(fp_a,"%s chan_prefilt (%ld) = ", prefix_a, hdr_ghdi_nsig_rec_d);
  for (int i = 0; i < hdr_ghdi_nsig_rec_d - 1; i++) {
    fprintf(fp_a, "[%s], ", hdr_chan_prefilt_d[i]);
  }
  fprintf(fp_a, "[%s]\n", hdr_chan_prefilt_d[hdr_ghdi_nsig_rec_d - 1]);

  fprintf(fp_a,"%s chan_rec_size (%ld) = ", prefix_a, hdr_ghdi_nsig_rec_d);
  for (int i = 0; i < hdr_ghdi_nsig_rec_d - 1; i++) {
    fprintf(fp_a, "[%10ld], ", hdr_chan_rec_size_d[i]);
  }
  fprintf(fp_a, "[%10ld]\n", hdr_chan_rec_size_d[hdr_ghdi_nsig_rec_d - 1]);

  fprintf(fp_a, "%s\n", prefix_a);

  // (6) derived values
  //
  fprintf(fp_a, "%s hdr_sample_frequency = %10.1f\n",
	  prefix_a, hdr_sample_frequency_d);
  fprintf(fp_a, "%s hdr_num_channels_signal = %10ld\n",
	  prefix_a, hdr_num_channels_signal_d);
  fprintf(fp_a, "%s hdr_num_channels_annotation = %10ld\n",
	  prefix_a, hdr_num_channels_annotation_d);
  fprintf(fp_a, "%s duration of recording (secs) = %10.1f\n",
	  prefix_a, (float)(hdr_ghdi_dur_rec_d * hdr_ghdi_num_recs_d));
  
  // exit gracefully
  //
  return true;
}

// method: print_header
//
// arguments:
//  char* fname: filename (input)
//  FILE* fp: file pointer to send the output (input)
//  char* prefix: a prefix to use before each field is printed
//
// return: an boolean value indicating status
//
// This method dumps the header to a file pointer (typically stdout).
//
bool Edf::print_header(char* fname_a, FILE* fp_a, char* prefix_a) {

  // declare local variables
  //
  VVectorDouble sig;

  // read the header from a file:
  //  note that we will ignore the signal data
  //
  if (!Edf::read_edf(sig, fname_a, false, false)) {
    fprintf(stdout, "**> Edf::print_header(): error opening (%s)\n", fname_a);
  }

  // read the header from a file:
  //  note that we will ignore the signal data
  //
  Edf::print_header(fp_a, prefix_a);

  // exit gracefully
  //
  return true;
}

// method: check_montage
//
// arguments:
//  char* missing_channels: string containing the missing labels (output)
//  char* fname: EDF file (input)
//  long nmontage: number of montage channels (input)
//  MATCH_MODE match_mode: match mode desired (input)
//  char** montage: montage channels (input)
//
// return: a boolean value indicating if the file contains proper
//         channel assignments.
//
// This method checks if the header of an EDF file contains the channels
// specified in montage.
//
// Note that the memory for missing_channels is allocated in the
// check_montage utility driver program. Since the input can be either
// a single EDF file or a list, memory for missing_channels is allocated
// and deallocated constantly.
//
// Note also that missing_channels is a string instead of an array of
// strings. That way, the missing channels can be outputed easily without
// the need of loops.
//
bool Edf::check_montage(char* missing_channels_a,
			char* fname_a, long nmontage_a,
			MATCH_MODE match_mode_a, char** montage_a) {

  // declare local variables
  //
  VVectorDouble sig;
  long missing_count = 0;
  bool show_missing_channels = false;
  bool fail_file = false;
    
  // read the header from a file:
  //  ignore the signal data
  //
  if (!Edf::read_edf(sig, fname_a, false, false)) {
    fprintf(stdout, "**> Edf::check_montage(): error opening (%s)\n", fname_a);
    return false;
  }

  // check verbosity level to additional information about the error
  // 
  if ( Edf::get_verbosity() >= Edf::LEVEL_SHORT) {
    show_missing_channels = true;
  }
  
  // loop over all channels in montage
  //
  for (long i = 0; i < nmontage_a; i++) {

    // hold the position of montage channel i
    //
    long pos;

    if ((pos = Edf::find_match(montage_a[i],
			       hdr_ghdi_nsig_rec_d, hdr_chan_labels_d,
			       match_mode_a)) < 0) {


      // case 1: output the missing channels
      //
      if (show_missing_channels == true) {
	fail_file = true;
	
	if(missing_count == 0) {

	  // assign the first channel not found to
	  // missing_channels_a 
	  //
	  strcpy(missing_channels_a, montage_a[i]);
	}
	else {

	  // concatenate the other channels missing
	  //
	  strcat(missing_channels_a, COMMA);
	  strcat(missing_channels_a, SPACE);
	  strcat(missing_channels_a, montage_a[i]);
	}

	missing_count++;
      }
      
      // case 2: just output if there is a missing channel
      //
      else {
	// channel i of montage was not found:
	//  file does not contain proper channel assignments
	//  for the montage specified.
	//
	return false;
      }
    }     
  }

  // file does not contain proper channel assignments:
  //  this boolean variable handles the output of the missing channels.
  //
  if (fail_file == true) {
    return false;
  }
  
  // file contain proper channel assignments
  //
  return true;
}

// method: get_header
//
// arguments:
//  FILE* fp: file pointer (input)
//
// return: a boolean value indicating status
//
// This method fetches the header from an EDF file.
//
bool Edf::get_header(FILE* fp_a) {

  // declare local variables
  //
  long nbytes = 0;
  long num_items;
  char buf[MAX_TMP_BSIZE];
  char cbuf[EDF_LPTI_TSIZE];

  // display debug information
  //
  if (debug_level_d >= LEVEL_FULL) {
    fprintf(stdout, "Edf::get_header(): fetching an EDF header\n");
  }

  // rewind the file
  //
  rewind(fp_a);

  // (1) version information
  //
  if (debug_level_d >= LEVEL_FULL) {
    fprintf(stdout, "Edf::get_header(): fetching (1)\n");
  }

  if ((nbytes = fread(hdr_version_d, 1, EDF_VERS_BSIZE, fp_a)) !=
      EDF_VERS_BSIZE) {
    return false;
  }
  hdr_version_d[EDF_VERS_BSIZE] = (char)NULL;
  if (strcmp(hdr_version_d, EDF_VERS) != 0) {
    return false;
  }

  // (2) local patient information
  //
  // Unfortunately, some EDF files don't contain all the information
  // they should. This often occurs because the deidenitification
  // process overwrites this information. So we zero out the buffers
  // that won't be filled if the information is missing.
  //
  if (debug_level_d >= LEVEL_FULL) {
    fprintf(stdout, "Edf::get_header(): fetching (2)\n");
  }

  hdr_lpti_patient_id_d[0] = (char)NULL;
  hdr_lpti_gender_d[0] = (char)NULL;
  hdr_lpti_dob_d[0] = (char)NULL;
  hdr_lpti_full_name_d[0] = (char)NULL;
  hdr_lpti_age_d[0] = (char)NULL;
  
  if ((nbytes = fread(buf, 1, EDF_LPTI_BSIZE, fp_a)) !=
      EDF_LPTI_BSIZE) {
    return false;
  }
  buf[EDF_LPTI_BSIZE] = (char)NULL;

  num_items = sscanf(buf, "%s%s%s%s%s",
		     hdr_lpti_patient_id_d, hdr_lpti_gender_d,
		     hdr_lpti_dob_d, hdr_lpti_full_name_d, hdr_lpti_age_d);

  if (num_items == 0) {
    return false;
  }

  // (3) local recording information
  //
  // Unfortunately, some EDF files don't contain all the information
  // they should. This often occurs because the deidenitification
  // process overwrites this information. So we zero out the buffers
  // that won't be filled if the information is missing.
  //
  if (debug_level_d >= LEVEL_FULL) {
    fprintf(stdout, "Edf::get_header(): fetching (3)\n");
  }

  hdr_lrci_start_date_label_d[0] = (char)NULL;
  hdr_lrci_start_date_d[0] = (char)NULL;
  hdr_lrci_eeg_id_d[0] = (char)NULL;
  hdr_lrci_tech_d[0] = (char)NULL;
  hdr_lrci_machine_d[0] = (char)NULL;

  if ((nbytes = fread(buf, 1, EDF_LRCI_BSIZE, fp_a)) !=
      EDF_LRCI_BSIZE) {
    return false;
  }
  buf[EDF_LRCI_BSIZE] = (char)NULL;

  num_items = sscanf(buf, "%s%s%s%s%s",
		     hdr_lrci_start_date_label_d, hdr_lrci_start_date_d,
		     hdr_lrci_eeg_id_d, hdr_lrci_tech_d, hdr_lrci_machine_d);
  if (num_items == 0) {
    return false;
  }

  // (4) general header information
  //
  // get the fourth block of data (non-local information)
  //
  if (debug_level_d >= LEVEL_FULL) {
    fprintf(stdout, "Edf::get_header(): fetching (4)\n");
  }

  if ((nbytes = fread(buf, 1, EDF_GHDI_BSIZE, fp_a)) !=
      EDF_GHDI_BSIZE) {
    return false;
  }
  buf[EDF_GHDI_BSIZE] = (char)NULL;

  memcpy(hdr_ghdi_start_date_d, buf+ 0, 8);
  hdr_ghdi_start_date_d[8] = (char)NULL;
  memcpy(hdr_ghdi_start_time_d, buf + 8, 8);
  hdr_ghdi_start_time_d[8] = (char)NULL;
  memcpy(&cbuf, buf + 16, 8);
  cbuf[8] = (char)NULL;
  hdr_ghdi_hsize_d = atof(cbuf);
  memcpy(hdr_ghdi_file_type_d, buf + 24, 5);
  hdr_ghdi_file_type_d[5] = (char)NULL;
  memcpy(hdr_ghdi_reserved_d, buf + 29, 39);
  hdr_ghdi_reserved_d[40] = (char)NULL;
  memcpy(&cbuf, buf + 68, 8);
  cbuf[8] = (char)NULL;
  hdr_ghdi_num_recs_d = atoi(cbuf);
  memcpy(&cbuf, buf + 76, 8);
  cbuf[8] = (char)NULL;
  hdr_ghdi_dur_rec_d = atoi(cbuf);
  memcpy(&cbuf, buf + 84, 4);
  cbuf[4] = (char)NULL;
  hdr_ghdi_nsig_rec_d = atoi(cbuf);

  // (5) contains channel-specific data
  //
  // (5a) read channel labels
  //
  if (debug_level_d >= LEVEL_FULL) {
    fprintf(stdout, "Edf::get_header(): fetching (5a)\n");
  }

  long buf_end = hdr_ghdi_nsig_rec_d * EDF_LABL_BSIZE;
  if ((nbytes = fread(buf, 1, buf_end, fp_a)) != buf_end) {
    return false;
  }
  buf[buf_end] = (char)NULL;

  hdr_num_channels_annotation_d = 0;
  for (int i = 0; i < hdr_ghdi_nsig_rec_d; i++) {

    // create space for the labels
    //
    Edf::resize(hdr_chan_labels_d[i], EDF_LABL_BSIZE + 1, false);

    // look for the annotation labels
    //
    char tbuf[EDF_LABL_BSIZE+1];
    strncpy(tbuf, &buf[EDF_LABL_BSIZE * i], EDF_LABL_BSIZE);
    tbuf[EDF_LABL_BSIZE] = (char)NULL;
    Edf::trim_whitespace_and_upcase(hdr_chan_labels_d[i], tbuf);
    if ((strstr(hdr_chan_labels_d[i], ANNOTATION) == 0)) {
      hdr_num_channels_annotation_d++;
    }
  }

  // (5b) read the transducer type
  //
  if (debug_level_d >= LEVEL_FULL) {
    fprintf(stdout, "Edf::get_header(): fetching (5b)\n");
  }

 buf_end = hdr_ghdi_nsig_rec_d * EDF_TRNT_BSIZE;
  if ((nbytes = fread(buf, 1, buf_end, fp_a)) != buf_end) {
    return false;
  }
  buf[buf_end] = (char)NULL;

  for (int i = 0; i < hdr_ghdi_nsig_rec_d; i++) {

    // create space for the transducer type
    //
    Edf::resize(hdr_chan_trans_type_d[i], EDF_TRNT_BSIZE + 1, false);

    // copy it
    //
    strncpy(hdr_chan_trans_type_d[i],
	    &buf[EDF_TRNT_BSIZE * i], EDF_TRNT_BSIZE);
    hdr_chan_trans_type_d[i][EDF_TRNT_BSIZE] = (char)NULL;
  }

  // (5c) read the physical dimension
  //
  if (debug_level_d >= LEVEL_FULL) {
    fprintf(stdout, "Edf::get_header(): fetching (5c)\n");
  }

  buf_end = hdr_ghdi_nsig_rec_d * EDF_PDIM_BSIZE;
  if ((nbytes = fread(buf, 1, buf_end, fp_a)) != buf_end) {
    return false;
  }
  buf[buf_end] = (char)NULL;

  for (int i = 0; i < hdr_ghdi_nsig_rec_d; i++) {

    // create space for the phys dimension
    //
    Edf::resize(hdr_chan_phys_dim_d[i], EDF_PDIM_BSIZE + 1, false);

    // copy it
    //
    strncpy(hdr_chan_phys_dim_d[i],
	    &buf[EDF_PDIM_BSIZE * i], EDF_PDIM_BSIZE);
    hdr_chan_phys_dim_d[i][EDF_PDIM_BSIZE] = (char)NULL;
  }

  // (5d) read the physical minimum
  //
  if (debug_level_d >= LEVEL_FULL) {
    fprintf(stdout, "Edf::get_header(): fetching (5d)\n");
  }

  buf_end = hdr_ghdi_nsig_rec_d * EDF_PMIN_BSIZE;
  if ((nbytes = fread(buf, 1, buf_end, fp_a)) != buf_end) {
    return false;
  }
  buf[buf_end] = (char)NULL;

  for (int i = 0; i < hdr_ghdi_nsig_rec_d; i++) {
    char tstr[EDF_PMIN_BSIZE + 1];
    strncpy(tstr, &buf[EDF_PMIN_BSIZE * i], EDF_PMIN_BSIZE);
    tstr[EDF_PMIN_BSIZE] = (char)NULL;
    hdr_chan_phys_min_d[i] = atof(tstr);
  }

  // (5e) read the physical maximum
  //
  if (debug_level_d >= LEVEL_FULL) {
    fprintf(stdout, "Edf::get_header(): fetching (5e)\n");
  }

  buf_end = hdr_ghdi_nsig_rec_d * EDF_PMAX_BSIZE;
  if ((nbytes = fread(buf, 1, buf_end, fp_a)) != buf_end) {
    return false;
  }
  buf[buf_end] = (char)NULL;

  for (int i = 0; i < hdr_ghdi_nsig_rec_d; i++) {
    char tstr[EDF_PMAX_BSIZE + 1];
    strncpy(tstr, &buf[EDF_PMAX_BSIZE * i], EDF_PMAX_BSIZE);
    tstr[EDF_PMAX_BSIZE] = (char)NULL;
    hdr_chan_phys_max_d[i] = atof(tstr);
  }

  // (5f) read the digital minimums
  //
  if (debug_level_d >= LEVEL_FULL) {
    fprintf(stdout, "Edf::get_header(): fetching (5f)\n");
  }

  buf_end = hdr_ghdi_nsig_rec_d * EDF_DMIN_BSIZE;
  if ((nbytes = fread(buf, 1, buf_end, fp_a)) != buf_end) {
    return false;
  }
  buf[buf_end] = (char)NULL;

  for (int i = 0; i < hdr_ghdi_nsig_rec_d; i++) {
    char tstr[EDF_DMIN_BSIZE + 1];
    strncpy(tstr, &buf[EDF_DMIN_BSIZE * i], EDF_DMIN_BSIZE);
    tstr[EDF_DMIN_BSIZE] = (char)NULL;
    hdr_chan_dig_min_d[i] = atoi(tstr);
  }

  // (5g) read the digital maximums
  //
  if (debug_level_d >= LEVEL_FULL) {
    fprintf(stdout, "Edf::get_header(): fetching (5g)\n");
  }

  buf_end = hdr_ghdi_nsig_rec_d * EDF_DMAX_BSIZE;
  if ((nbytes = fread(buf, 1, buf_end, fp_a)) != buf_end) {
    return false;
  }
  buf[buf_end] = (char)NULL;

  for (int i = 0; i < hdr_ghdi_nsig_rec_d; i++) {
    hdr_chan_dig_max_d[i] = 32767.0;
    char tstr[EDF_DMAX_BSIZE + 1];
    strncpy(tstr, &buf[EDF_DMAX_BSIZE * i], EDF_DMAX_BSIZE);
    tstr[EDF_DMAX_BSIZE] = (char)NULL;
    hdr_chan_dig_max_d[i] = atoi(tstr);
  }

  // (5h) read the prefilt labels
  //
  if (debug_level_d >= LEVEL_FULL) {
    fprintf(stdout, "Edf::get_header(): fetching (5h)\n");
  }

  buf_end = hdr_ghdi_nsig_rec_d * EDF_PREF_BSIZE;
  if ((nbytes = fread(buf, 1, buf_end, fp_a)) != buf_end) {
    return false;
  }
  buf[buf_end] = (char)NULL;

  for (int i = 0; i < hdr_ghdi_nsig_rec_d; i++) {

    // create space for the prefilt
    //
    Edf::resize(hdr_chan_prefilt_d[i], EDF_PREF_BSIZE + 1, false);

    // copy it
    //
    strncpy(hdr_chan_prefilt_d[i],
	    &buf[EDF_PREF_BSIZE * i], EDF_PREF_BSIZE);
    hdr_chan_prefilt_d[i][EDF_PREF_BSIZE] = (char)NULL;
  }


  // (5i) read the rec sizes
  //
  if (debug_level_d >= LEVEL_FULL) {
    fprintf(stdout, "Edf::get_header(): fetching (5i)\n");
  }

  buf_end = hdr_ghdi_nsig_rec_d * EDF_RECS_BSIZE;
  if ((nbytes = fread(buf, 1, buf_end, fp_a)) != buf_end) {
    return false;
  }
  buf[buf_end] = (char)NULL;

  for (int i = 0; i < hdr_ghdi_nsig_rec_d; i++) {
    char tstr[EDF_RECS_BSIZE + 1];
    strncpy(tstr, &buf[EDF_RECS_BSIZE * i], EDF_RECS_BSIZE);
    tstr[EDF_RECS_BSIZE] = (char)NULL;
    hdr_chan_rec_size_d[i] = atoi(tstr);
  }

  // (5j) the last chunk of the header is reserved space
  // that we don't need to read
  //
  if (debug_level_d >= LEVEL_FULL) {
    fprintf(stdout, "Edf::get_header(): fetching (5j)\n");
  }

  // (6) compute some derived values
  //
  if (debug_level_d >= LEVEL_FULL) {
    fprintf(stdout, "Edf::get_header(): fetching (6)\n");
  }

  hdr_sample_frequency_d = (float)hdr_chan_rec_size_d[0] /
    (float)hdr_ghdi_dur_rec_d;
  hdr_num_channels_signal_d = hdr_ghdi_nsig_rec_d -
    hdr_num_channels_annotation_d;

  // exit gracefully
  //
  if (debug_level_d >= LEVEL_FULL) {
    fprintf(stdout, "Edf::get_header(): done fetching an EDF header\n");
  }
  return true;
}

// method: put_header
//
// arguments:
//  FILE* fp: file pointer (input)
//
// return: a boolean value indicating status
//
// This method writes the header to a binary edf file.
//
bool Edf::put_header(FILE* fp_a) {

  // declare local variables
  //
  long i;
  long len;
  long nbytes = 0;
  long num_items;
  char buf[MAX_TMP_BSIZE];
  char cbuf[EDF_LPTI_TSIZE];

  // set size of tbuf so that it accounts for null character
  //
  char tbuf[EDF_LPTI_BSIZE + 1]; 

  // clean up fields that might have bad characters
  //
  memset(hdr_ghdi_reserved_d, (int)SPACE[0], 39);

  // keep a counter of the number of bytes written for debug purposes
  //
  long nbytes_written = 0;

  // display debug information
  //
  if (debug_level_d >= LEVEL_FULL) {
    fprintf(stdout, "Edf::put_header(): putting an EDF header\n");
  }

  // rewind the file
  //
  rewind(fp_a);

  // (1) version information
  //
  if ((nbytes = fwrite(hdr_version_d, 1, EDF_VERS_BSIZE, fp_a)) !=
      EDF_VERS_BSIZE) {
    return false;
  }
  nbytes_written += nbytes;

  if (debug_level_d >= LEVEL_FULL) {
    fprintf(stdout, "Edf::put_header(): wrote section (1) (%ld / %ld, %ld)\n",
	    nbytes, nbytes_written, hdr_ghdi_hsize_d);
  }

  // (2) local patient information:
  //     we need to fill null characters with spaces
  //
  num_items = sprintf(buf, "%s %s %s %s %s",
		      hdr_lpti_patient_id_d, hdr_lpti_gender_d,
		      hdr_lpti_dob_d, hdr_lpti_full_name_d, hdr_lpti_age_d);

  len = strlen(buf);
  for (long i = len; i <= EDF_LPTI_BSIZE; i++) {
    buf[i] = SPACE[0];
  }

  if ((nbytes = fwrite(buf, 1, EDF_LPTI_BSIZE, fp_a)) !=
      EDF_LPTI_BSIZE) {
    return false;
  }
  nbytes_written += nbytes;

  if (debug_level_d >= LEVEL_FULL) {
    fprintf(stdout, "Edf::put_header(): wrote section (2) (%ld / %ld, %ld)\n",
	    nbytes, nbytes_written, hdr_ghdi_hsize_d);
  }

  // (3) local recording information
  //
  num_items = sprintf(buf, "%s %s %s %s %s",
		      hdr_lrci_start_date_label_d, hdr_lrci_start_date_d,
		      hdr_lrci_eeg_id_d, hdr_lrci_tech_d, hdr_lrci_machine_d);
  Edf::pad_whitespace(buf, EDF_LRCI_BSIZE);

  if ((nbytes = fwrite(buf, 1, EDF_LRCI_BSIZE, fp_a)) !=
      EDF_LRCI_BSIZE) {
    return false;
  }
  nbytes_written += nbytes;

  if (debug_level_d >= LEVEL_FULL) {
    fprintf(stdout, "Edf::put_header(): wrote section (3) (%ld / %ld, %ld)\n",
	    nbytes, nbytes_written, hdr_ghdi_hsize_d);	    
  }

  // (4) general header information
  //
  memcpy(buf +  0, hdr_ghdi_start_date_d, 8);
  memcpy(buf +  8, hdr_ghdi_start_time_d, 8);
  sprintf(cbuf, "%ld", hdr_ghdi_hsize_d);
  Edf::pad_whitespace(cbuf, 8);
  memcpy(buf + 16, &cbuf, 8);
  memcpy(buf + 24, hdr_ghdi_file_type_d, 5);
  memcpy(tbuf, hdr_ghdi_reserved_d, 39);
  memcpy(buf + 29, tbuf, 39);
  sprintf(cbuf, "%ld", hdr_ghdi_num_recs_d);
  Edf::pad_whitespace(cbuf, 8);
  memcpy(buf + 68, &cbuf, 8);
  sprintf(cbuf, "%ld", hdr_ghdi_dur_rec_d);
  Edf::pad_whitespace(cbuf, 8);
  memcpy(buf + 76, &cbuf, 8);
  sprintf(cbuf, "%ld", hdr_ghdi_nsig_rec_d);
  Edf::pad_whitespace(cbuf, 4);
  memcpy(buf + 84, &cbuf, 4);

  if ((nbytes = fwrite(buf, 1, EDF_GHDI_BSIZE, fp_a)) !=
      EDF_GHDI_BSIZE) {
    return false;
  }
  nbytes_written += nbytes;

  if (debug_level_d >= LEVEL_FULL) {
    fprintf(stdout, "Edf::put_header(): wrote section (4) (%ld / %ld, %ld)\n",
	    nbytes, nbytes_written, hdr_ghdi_hsize_d);	    
  }

  // (5) contains channel-specific data
  //
  // (5a) write channel labels
  //

  
  for (int i = 0; i < hdr_ghdi_nsig_rec_d; i++) {
    sprintf(tbuf, "%s", hdr_chan_labels_d[i]);
    Edf::pad_whitespace(tbuf, EDF_LABL_BSIZE);
    tbuf[EDF_LABL_BSIZE] = (char)NULL;
    strcpy(&buf[EDF_LABL_BSIZE * i], tbuf);
  }

  long buf_end = hdr_ghdi_nsig_rec_d * EDF_LABL_BSIZE;
  if ((nbytes = fwrite(buf, 1, buf_end, fp_a)) != buf_end) {
    return false;
  }
  nbytes_written += nbytes;

  if (debug_level_d >= LEVEL_FULL) {
    fprintf(stdout, "Edf::put_header(): wrote section (5a) (%ld / %ld, %ld)\n",
	    nbytes, nbytes_written, hdr_ghdi_hsize_d);	    
  }

  // (5b) write the transducer type
  //
  for (int i = 0; i < hdr_ghdi_nsig_rec_d; i++) {
    sprintf(tbuf, "%s", hdr_chan_trans_type_d[i]);
    Edf::pad_whitespace(tbuf, EDF_TRNT_BSIZE);
    tbuf[EDF_TRNT_BSIZE] = (char)NULL;
    strcpy(&buf[EDF_TRNT_BSIZE * i], tbuf);
  }


  buf_end = hdr_ghdi_nsig_rec_d * EDF_TRNT_BSIZE;
  if ((nbytes = fwrite(buf, 1, buf_end, fp_a)) != buf_end) {
    return false;
  }
  nbytes_written += nbytes;

  if (debug_level_d >= LEVEL_FULL) {
    fprintf(stdout, "Edf::put_header(): wrote section (5b) (%ld / %ld, %ld)\n",
	    nbytes, nbytes_written, hdr_ghdi_hsize_d);	    
  }

  // (5c) write the physical dimension
  //
  for (int i = 0; i < hdr_ghdi_nsig_rec_d; i++) {
    sprintf(tbuf, "%s", hdr_chan_phys_dim_d[i]);
    Edf::pad_whitespace(tbuf, EDF_PDIM_BSIZE);
    tbuf[EDF_PDIM_BSIZE] = (char)NULL;
    strcpy(&buf[EDF_PDIM_BSIZE * i], tbuf);
  }

  buf_end = hdr_ghdi_nsig_rec_d * EDF_PDIM_BSIZE;
  if ((nbytes = fwrite(buf, 1, buf_end, fp_a)) != buf_end) {
    return false;
  }
  nbytes_written += nbytes;

  if (debug_level_d >= LEVEL_FULL) {
    fprintf(stdout, "Edf::put_header(): wrote section (5c) (%ld / %ld, %ld)\n",
	    nbytes, nbytes_written, hdr_ghdi_hsize_d);	    
  }

  // (5d) write the physical minimum
  //
  for (int i = 0; i < hdr_ghdi_nsig_rec_d; i++) {
    sprintf(tbuf, "%8.2f", hdr_chan_phys_min_d[i]);
    Edf::pad_whitespace(tbuf, EDF_PMIN_BSIZE);
    tbuf[EDF_PMIN_BSIZE] = (char)NULL;
    strcpy(&buf[EDF_PMIN_BSIZE * i], tbuf);
  }

  buf_end = hdr_ghdi_nsig_rec_d * EDF_PMIN_BSIZE;
  if ((nbytes = fwrite(buf, 1, buf_end, fp_a)) != buf_end) {
    return false;
  }
  nbytes_written += nbytes;

  if (debug_level_d >= LEVEL_FULL) {
    fprintf(stdout, "Edf::put_header(): wrote section (5d) (%ld / %ld, %ld)\n",
	    nbytes, nbytes_written, hdr_ghdi_hsize_d);	    
  }

  // (5e) write the physical maximum:
  //      note that we play a little game with the formatting to make
  //      sure a positive number uses all 8 characters. this preserves
  //      a little bit of precision, but creates a bias for the signal
  //      since the min/max values are not symmetric.
  //
  for (int i = 0; i < hdr_ghdi_nsig_rec_d; i++) {
    if (hdr_chan_phys_max_d[i] < 0) {
      sprintf(tbuf, "%8.2f", hdr_chan_phys_max_d[i]);
    }
    else {
      sprintf(tbuf, "%8.3f", hdr_chan_phys_max_d[i]);
    }      
    Edf::pad_whitespace(tbuf, EDF_PMAX_BSIZE);
    tbuf[EDF_PMAX_BSIZE] = (char)NULL;
    strcpy(&buf[EDF_PMAX_BSIZE * i], tbuf);
  }

  buf_end = hdr_ghdi_nsig_rec_d * EDF_PMAX_BSIZE;
  if ((nbytes = fwrite(buf, 1, buf_end, fp_a)) != buf_end) {
    return false;
  }
  nbytes_written += nbytes;

  if (debug_level_d >= LEVEL_FULL) {
    fprintf(stdout, "Edf::put_header(): wrote section (5e) (%ld / %ld, %ld)\n",
	    nbytes, nbytes_written, hdr_ghdi_hsize_d);	    
  }

  // (5f) write the digital minimums
  //
  for (int i = 0; i < hdr_ghdi_nsig_rec_d; i++) {
    sprintf(tbuf, "%ld", hdr_chan_dig_min_d[i]);
    Edf::pad_whitespace(tbuf, EDF_DMIN_BSIZE);
    tbuf[EDF_DMIN_BSIZE] = (char)NULL;
    strcpy(&buf[EDF_DMIN_BSIZE * i], tbuf);
  }

  buf_end = hdr_ghdi_nsig_rec_d * EDF_DMIN_BSIZE;
  if ((nbytes = fwrite(buf, 1, buf_end, fp_a)) != buf_end) {
    return false;
  }
  nbytes_written += nbytes;

  if (debug_level_d >= LEVEL_FULL) {
    fprintf(stdout, "Edf::put_header(): wrote section (5f) (%ld / %ld, %ld)\n",
	    nbytes, nbytes_written, hdr_ghdi_hsize_d);	    
  }

  // (5g) write the digital maximums
  //
  for (int i = 0; i < hdr_ghdi_nsig_rec_d; i++) {
    sprintf(tbuf, "%ld", hdr_chan_dig_max_d[i]);
    Edf::pad_whitespace(tbuf, EDF_DMAX_BSIZE);
    tbuf[EDF_DMAX_BSIZE] = (char)NULL;
    strcpy(&buf[EDF_DMAX_BSIZE * i], tbuf);
  }

  buf_end = hdr_ghdi_nsig_rec_d * EDF_DMAX_BSIZE;
  if ((nbytes = fwrite(buf, 1, buf_end, fp_a)) != buf_end) {
    return false;
  }
  nbytes_written += nbytes;

  if (debug_level_d >= LEVEL_FULL) {
    fprintf(stdout, "Edf::put_header(): wrote section (5g) (%ld / %ld, %ld)\n",
	    nbytes, nbytes_written, hdr_ghdi_hsize_d);	    
  }

  // (5h) write the prefilt labels
  //
  for (int i = 0; i < hdr_ghdi_nsig_rec_d; i++) {
    sprintf(tbuf, "%s", hdr_chan_prefilt_d[i]);
    Edf::pad_whitespace(tbuf, EDF_PREF_BSIZE);
    tbuf[EDF_PREF_BSIZE] = (char)NULL;
    strcpy(&buf[EDF_PREF_BSIZE * i], tbuf);
  }

  buf_end = hdr_ghdi_nsig_rec_d * EDF_PREF_BSIZE;
  if ((nbytes = fwrite(buf, 1, buf_end, fp_a)) != buf_end) {
    return false;
  }
  nbytes_written += nbytes;

  if (debug_level_d >= LEVEL_FULL) {
    fprintf(stdout, "Edf::put_header(): wrote section (5h) (%ld / %ld, %ld)\n",
	    nbytes, nbytes_written, hdr_ghdi_hsize_d);	    
  }

  // (5i) write the rec sizes
  //
  for (int i = 0; i < hdr_ghdi_nsig_rec_d; i++) {
    sprintf(tbuf, "%ld", hdr_chan_rec_size_d[i]);
    Edf::pad_whitespace(tbuf, EDF_RECS_BSIZE);
    tbuf[EDF_RECS_BSIZE] = (char)NULL;
    strcpy(&buf[EDF_RECS_BSIZE * i], tbuf);
  }

  buf_end = hdr_ghdi_nsig_rec_d * EDF_RECS_BSIZE;
  if ((nbytes = fwrite(buf, 1, buf_end, fp_a)) != buf_end) {
    return false;
  }
  nbytes_written += nbytes;

  if (debug_level_d >= LEVEL_FULL) {
    fprintf(stdout, "Edf::put_header(): wrote section (5i) (%ld / %ld, %ld)\n",
	    nbytes, nbytes_written, hdr_ghdi_hsize_d);	    
  }

  // (5j) the last chunk of the header is reserved space:
  //      write blanks

  long nbytes_left = hdr_ghdi_hsize_d - nbytes_written;
  memset(buf, SPACE[0], nbytes_left);
  if ((nbytes = fwrite(buf, 1, nbytes_left, fp_a)) != nbytes_left) {
    return false;
  }
  nbytes_written += nbytes;

  if (debug_level_d >= LEVEL_FULL) {
    fprintf(stdout, "Edf::put_header(): wrote section (5j) (%ld / %ld, %ld)\n",
	    nbytes, nbytes_written, hdr_ghdi_hsize_d);	    
  }

  // exit gracefully
  //
  if (debug_level_d >= LEVEL_FULL) {
    fprintf(stdout, "Edf::put_header(): done wrote an EDF header\n");
  }
  return true;
}

// method: uppercase
//
// arguments:
//  char* str: a string containing mixed case (input/output)
//
// return: a logical value indicating status
//
// This method uppercases a string.
//
bool Edf::uppercase(char* str_a) {

  // loop over all characters
  //
  while(*str_a != (char)NULL) {
    *str_a = toupper((unsigned char)*str_a);
    str_a++;
  }

  // exit gracefully
  //
  return true;
}

// method: find_match
//
// arguments:
//  char* label: a label containing the desired channel (input)
//  long nl:     number of labels
//  char** lbls: an array of labels
//  MATCH_MODE matmode: an enum indicating the precision of the match (input)
//
// return: a long integer containing the channel number,
//         or -1 if it is not found.
//
// This method finds the channel in the set of labels in the header.
// Two match modes are supported: exact (true) and partial (false).
//
long Edf::find_match(char* label_a, long nl_a, char** lbls_a,
		     MATCH_MODE matmode_a) {

  // loop over all labels
  //
  for (long i = 0; i < nl_a; i++) {

    // case: exact match
    //
    if ((matmode_a == MATMODE_EXACT) && (strcmp(lbls_a[i], label_a) == 0)) {
      return i;
    }

    // case: partial match
    //
    else if ((matmode_a == MATMODE_PARTIAL) &&
	     (strstr(lbls_a[i], label_a) != (char*)NULL)) {
      return i;
    }
  }
  
  // exit ungracfully
  //
  return ERR_MATCH;
}

// method: trim_whitespace_and_upcase
//
// arguments:
//  char* str_out: string with whitespace removed (output)
//  char* str_in:  original string (input)
//
// return: a boolean value indicating status
//
// This method removes leading and trailing whitespace from a string.
// It also upcases the string.
//
bool Edf::trim_whitespace_and_upcase(char* str_out_a, char* str_in_a) {

  // declare local variables:
  //  we need the beginning and end of the string
  //
  long lstr = strlen(str_in_a);
  char* str_beg = str_in_a;
  char* str_end = str_in_a + lstr;

  // find the first non-space character
  //
  while ((*str_beg == SPACE[0]) && (str_beg < str_end)) {
    str_beg++;
  }

  // find the last non-space character
  //
  str_end--;
  while ((*str_end == SPACE[0]) && (str_end > str_beg)) {
    str_end--;
  }

  // copy the trimmed string
  //
  char* str_o = str_out_a;
  for (char* ptr = str_beg; ptr <= str_end; ptr++) {
    *str_o++ = toupper(*str_beg++);
  }
  *str_o = (char)NULL;

  // exit gracefully
  //
  return true;
}

// method: pad_whitespace
//
// arguments:
//  char* str: string with whitespace added (output)
//  long len: desired length (input)
//
// return: a boolean value indicating status
//
// This method added trailing whitespace so the edf header remains
// the proper size.
//
bool Edf::pad_whitespace(char* str_a, long len_a) {

  // declare local variables:
  //  we need the beginning and end of the string
  //
  long lstr = strlen(str_a);

  // add whitespace
  //
  for (long i = lstr; i < len_a; i++) {
    str_a[i] = SPACE[0];
  }

  // add a null character just to be safe:
  //  this is a little dangerous - the string must have an
  //  extra element for this. all the strings declared in the
  //  header file should.
  //
  str_a[len_a] = (char)NULL;

  // exit gracefully
  //
  return true;
}

// method: parse_line
//
// arguments:
//  long& nl: the number of labels (output)
//  char** labels: an array of strings containing the labels (output)
//  char* str: a string containing the label list (input)
//  char* delim: represents the string that is used as the delimiter
// return: a boolean indicating status
//
// Note that EEG labels usually have spaces in them which can't be ignored.
//
// Note that the character string array labels is assumed to be
// allocated, but each individual element is not defined.
//
bool Edf::parse_line(long& nl_a, char** labels_a, char* str_a, char* delim) {

  // declare local variables
  //
  char* tmp_ptr = str_a;
  char* str_end = str_a + strlen(str_a);
  
  // iterate over all characters
  //
  nl_a = 0;

  while (tmp_ptr < str_end) {

    // trim off leading whitespace
    //
    while (isspace(*tmp_ptr)) {
      tmp_ptr++;
    }

    // find the next comma
    //
    char* pos_c = strstr(tmp_ptr, delim);
   
    //  note that if one doesn't exist, it is the end of the line. in this
    //  case we go to the end of the line and then step back to the first
    //  non-whitespace.
    //
    if (pos_c == (char*)NULL) {
      pos_c = str_end - 1;
      while (isspace(*pos_c)) {
     	pos_c--;
      }

      // increment by one to be consistent with cases where we are
      // not at the end of the line
      //
      pos_c++;
    }
   
    // compute the length and create space
    //
    long label_len = pos_c - tmp_ptr;
    labels_a[nl_a] = new char [label_len + 1];

    // copy and uppercase the string
    //
    for (long i = 0; i < label_len; i++) {
      labels_a[nl_a][i] = toupper(*tmp_ptr++);
    }
    labels_a[nl_a][label_len] = (char)NULL;
    
    // increment counters
    //
    nl_a++;
    tmp_ptr++;
  }
  
  // exit gracefully
  //
  return true;
}

// method: parse_interp_channels
//
// arguments:
//  long& nl_i: the number of labels (output)
//  char*** labels1_a: an array of an array of strings containing the adjacent
//                     channels (output)
//  VectorLong& num_chans_a: the number of adjacent channels (output)
//  char** nchans_a: contains the channel that we want to interpolate (output)
//  char* str_a: a string containing the label list (input)
//
// return: a boolean indicating status
//
// This method parses a line assumed to be in the format:
//  str = EEG FP10: EEG FP1-REF, EEG FP2-REF, EEG FP3-REF, EEG FP4-REF
//
// The first variable represents the new channel and all the other channels
// represent the adjacent channels. Note that EEG labels usually have
// spaces in them which can't be ignored.
//
bool Edf:: parse_interp_channels(long& nl_i, char*** labels1_a,
				 VectorLong& num_chans_a,
				 char** nchans_a, char** str_a){

  if (debug_level_d >= LEVEL_FULL) {
    fprintf(stdout,
	    "Edf::parse_interp_channels(): entering parse_interp_channels\n");
  }

  //declare local variables
  //
  char* tmp_nlabels[MAX_NCHANS];
  char* tmp_adlabels[MAX_NCHANS];  
  
  Edf::resize(num_chans_a, num_clabels_d);

  // loops over the whole array
  //
  for (long i = 0; i < num_clabels_d; i++) {

   // initialzes the local variables
   //
   long pc = 0;
   long num_adj_chans = 0;

   // parses the new channel
   //
   bool status = Edf::parse_line(pc, tmp_nlabels, str_a[i], (char*)COLON);
    
    // displays debugging information
    //
    if (!status) {
      fprintf(stdout,
	      "**> Edf::parse_interp_channels(): invalid interp channels spec [%s]\n",
	      str_a[i]);
      return false;
    }
    
    // gets the length of the new channel
    //  
    long nc = strlen(tmp_nlabels[0]);

    // gets the new channels in an array
    //
    nchans_a[i] = new char [nc+1];
    
    // copies the temporary value onto channels
    //
    strcpy(nchans_a[i], tmp_nlabels[0]);
    
    // parses up to the colon
    // 
    char* pos_c = strstr(str_a[i], COLON);
  
    // gets rid of the colon on pos_c
    //
    while (pos_c[0] == COLON[0]) {
      pos_c++;
    }
  
    // parses the adjacent channels
    //
    status = Edf::parse_line(num_adj_chans, tmp_adlabels, pos_c, (char*)COMMA);
  
    // stores the number of adjacent channels
    //
    num_chans_a[i] = num_adj_chans;

    // copies the temporary adjacent channels to lables1
    //
    labels1_a[i] = new char*[num_adj_chans];
    for (long j = 0; j < num_adj_chans; j++) {
      labels1_a[i][j] = new char [strlen(tmp_adlabels[j]) + 1];
      strcpy(labels1_a[i][j], tmp_adlabels[j]);
    }

    // clean memory
    //
    Edf::cleanup(tmp_nlabels, pc);
    
    // clean memory
    //
    Edf::cleanup(tmp_adlabels, num_adj_chans);    
  }

  // assign output variable related to the number of new channels
  //
  nl_i = num_clabels_d;

  // display debug information
  //
  if (debug_level_d >= LEVEL_FULL) {
    fprintf(stdout,
	    "Edf::parse_interp_channels(): leaving parse_interp_channels\n");
  }

  // exit gracefully
  //
  return true;
}

// method: parse_montage
//
// arguments:
//  long& nl_a:       the number of labels (output)
//  char** labels1_a: an array of strings containing the labels (output)
//  char** labels2_a: an array of strings containing the labels (output)
//  char* num_chans_a: an array of the channel numbers (output)
//  char** olabels_a: an array of the name of each channel (output)
//  char** str_a:     an array of strings containing the label list (input)
//
// return: a boolean indicating status
//
// This method parses a line and produces a list of labels showing
// which channels are to be differenced.
//
bool Edf::parse_montage(long& nl_a, char** labels1_a, char** labels2_a,
			char** olabels_a, long* num_chans_a, char** str_a) {
  
  // display debug information
  //
  if (debug_level_d >= LEVEL_FULL) {
    fprintf(stdout, "Edf::parse_montage(): entering parse_montage\n");
  }
 
  // declarations of all the local variables
  //
  char* tmp_chan[MAX_NCHANS];
  char* tmp_num[MAX_NCHANS];
  char* tmp_op[MAX_NCHANS];
  char* tmp_labels1[MAX_NCHANS];
  char* tmp_labels2[MAX_NCHANS];
  
  // sets the output variable to the number of channel labels
  //
  nl_a = num_mlabels_d;

  // loops over all the elements in the montage_d array
  //
  for (long i = 0; i < num_mlabels_d; i++) {
    
    // initializes variables
    //
    long np_tmp = 0;
    long nl_tmp = 0;
    long nn_tmp = 0;

    //Parses the first element which is the channel number
    //
    bool status = Edf::parse_line(nn_tmp, tmp_num, str_a[i], (char*)COMMA);

    if (!status) {
      fprintf(stdout,
	      "**> Edf::parse_montage(): invalid montage spec [%s]\n",
	      str_a[i]);
      return false;
    }

    // converts the string output a long variable
    //
    long channel = atol(tmp_num[0]);

    for(long j = 0; j < nn_tmp; j++){
      delete [] tmp_num [j];
    }

    // ensures there is not a missing channel
    //
    if (i != channel) {
      fprintf(stdout,
	      "**> Edf::parse_montage(): missing channel [%ld]\n",
	      i);
      return false;
    }

    // fills the array with the channel number
    //
    num_chans_a[i] = channel;
    chan_num_d[i] = channel;

    // parses the string up to the comma (cuts off the first channel)
    //
    char* pos_c = strstr(str_a[i], COMMA);

    // displays debugging information
    //
    if (debug_level_d >= LEVEL_FULL) {
      fprintf(stdout,
	    "Edf::parse_montage(): entering parse_line to find chan_labels\n");
    }

    // cuts off any leading comma
    //
    while (pos_c[0] == COMMA[0]) {
      pos_c++;
    }
    
    // parses up to the colon which is the name of the channel
    //
    status = Edf::parse_line(np_tmp, tmp_chan, pos_c, (char*)COLON);

    if (!status) {
      fprintf(stdout,
	      "**> Edf::parse_montage(): invalid montage name\n");
      return false;
    }

    // gets the length of the channel label
    //
    long s = strlen(tmp_chan[0]);

    // Copies the temporary variable onto the chan_labels array
    // Copies the temporary variable onto the Edf.h variable mchan_d
    //
    olabels_a[i] = new char[s+1];
    strcpy(olabels_a[i], tmp_chan[0]);

    for (long j = 0; j < np_tmp; j++) {
      delete [] tmp_chan[j];
    }
    
    // parses up to the colon (this isolates the montage labels)
    //
    pos_c = strstr(pos_c, COLON);

    // gets rid of any leading colon
    //
    while (pos_c[0] == COLON[0]) {
      pos_c++;
    }
    while (isspace(*pos_c)) {
      pos_c++;
    }
   
    // gets rid of leading spaces
    //
    while (isspace(*pos_c)) {
      pos_c++;
    }

    // gets the length of the labels (unparse)
    //
    long label_len = strlen(pos_c);
    
    // initializes temporary variable
    //
    tmp_op[nl_tmp] = new char [label_len+1];
    
    // copies the pos_co string to op_tmp
    //
    strcpy(tmp_op[nl_tmp], pos_c);

    tmp_op[nl_tmp][label_len] = (char) NULL;

    if (debug_level_d >= LEVEL_FULL) {
      fprintf(stdout, "Edf::parse_montage(): entering parse_operands()\n");
    }

    long test;
    status = Edf::parse_operands(test, tmp_labels1, tmp_labels2, nl_tmp+1,
				 tmp_op);
    
    // copies the temporary value to the mlabels1 and mlabels2
    //
    for (long j = 0; j < nl_tmp+1; j++) {
      delete [] tmp_op[j];
    }
    
    long lab_len = strlen(tmp_labels1[0]);
    labels1_a[i] = new char [lab_len+1];
    strcpy(labels1_a[i], tmp_labels1[0]);
    labels1_a[i][lab_len] = (char) NULL;

    lab_len = strlen(tmp_labels2[0]);
    labels2_a[i] = new char [lab_len+1];
    strcpy(labels2_a[i], tmp_labels2[0]);
    labels2_a[i][lab_len] = (char) NULL;
    
    for(long j = 0; j < test; j++){
      delete [] tmp_labels1[j];
      delete [] tmp_labels2[j];
    }
  }

  // display debug information
  //
  if (debug_level_d >= LEVEL_FULL) {
    fprintf(stdout,
	  "Edf::parse_montage(): leaving parse_montage\n");  
  }
  
  // exit gracefully
  //
  return true;
  }

  
// method: parse_operands
//
// arguments:
//  long& nlo:      the number of labels (output)
//  char** labels1: an array of strings containing the labels (output)
//  char** labels2: an array of strings containing the labels (output)
//  long nl:        the number of labels (input)
//  char** labels:  a single array of strings containing the labels (input)
//
// return: a boolean indicating status
//
// This method parses an array of labels of the form "x -- y" and
// splits them into separate arrays.
//
bool Edf::parse_operands(long& nlo_a, char** labels1_a, char** labels2_a,
			 long nl_a, char** labels_a) {

  // create output space
  //
  nlo_a = nl_a;

  // loop over all labels
  //
  for (long i = 0; i < nl_a; i++) {

    // find the delimiter
    //
    char* sptr = strstr(labels_a[i], DASHDASH);

    // case 1: no delimiter - copy the channel as is
    //
    if (sptr == (char*)NULL) {
      long len = strlen(labels_a[i]);
      labels1_a[i] = new char[len + 1];
      strcpy(labels1_a[i], labels_a[i]);
      trim_whitespace_and_upcase(labels1_a[i], labels1_a[i]);
      labels2_a[i] = new char[1];
      labels2_a[i] = (char*)NULL;
    }

    // case 2: delimiter - parse the string into two pieces
    //
    else {

      // copy the first part of the string
      //
      long len = sptr - labels_a[i] - 1;
      labels1_a[i] = new char[len + 1];
      strncpy(labels1_a[i], labels_a[i], len);
      trim_whitespace_and_upcase(labels1_a[i], labels1_a[i]);
      *(labels1_a[i]+len) = (char)NULL;

      // copy the secon part of the string
      //
      len = sptr + 3 - labels_a[i] - 1;
      labels2_a[i] = new char[len + 1];
      strncpy(labels2_a[i], sptr + 3, len);
      trim_whitespace_and_upcase(labels2_a[i], labels2_a[i]);
      *(labels2_a[i]+len) = (char)NULL;
    }
  }

  // exit gracefully
  //
  return true;
}

// method: get_matching_filenames
//
// arguments:
//  char** fnames: list of filenames (output)
//  long& nf: number of filenames (output)
//  char* bfn: base filename (input)
//  char* fmt: filename modifier format specification (input)
//
// return: an boolean value indicating status
//
// This method permutes the basename and searches a directory for
// matching filenames.
//
// Note that this method allocates memory inside of fnames, so that
// memory must be cleaned up by the calling method.
//
bool Edf::get_matching_filenames(char** fnames_a, long& nf_a,
				 char* bfn_a, char* fmt_a) {

  // declare some string buffers to hold filename pieces
  //
  char dirname[MAX_LSTR_LENGTH];
  char bname[MAX_MSTR_LENGTH];
  char ext[MAX_SSTR_LENGTH];
  char fname[MAX_LSTR_LENGTH];

  // display debug information
  //
  if (debug_level_d >= LEVEL_DETAILED) {
    fprintf(stdout, "Edf::get_matching_filenames(): getting filenames\n");
  }

  // get the directory name
  //
  if (Edf::create_matching_filename(dirname, bname, ext, fname,
				    0, bfn_a, fmt_a) == false) {
    return false;
  }
  if (debug_level_d >= LEVEL_FULL) {
    fprintf(stdout,
	    " Edf::get_matching_filenames(): [dir, base, ext, full] [%s %s %s %s]\n",
	    dirname, bname, ext, fname);
  }

  // display debug information
  //
  if (debug_level_d >= LEVEL_DETAILED) {
    fprintf(stdout, " Edf::get_matching_filenames(): fname = [%s]\n", fname);
  }

  // open the directory:
  //  note that if a directory is not specified, dirname is null,
  //  and opendir handles this okay.
  //
  DIR *dir;
  struct dirent *ent;

  if ((dir = opendir(dirname)) == NULL) {
    return false;
  }

  // loop over all files starting with channel no. 0. loop until
  // the first file is not found.
  //
  nf_a = 0;
  bool found = true;
  
  while (found) {

    // form a new filename
    //
    if (Edf::create_matching_filename(dirname, bname, ext, fname,
				      nf_a, bfn_a, fmt_a) == false) {
      closedir(dir);
      return false;
    }

    if (debug_level_d >= LEVEL_FULL) {
      fprintf(stdout,
	      " Edf::get_matching_filenames(): [dir, base, ext, full] [%s %s %s %s]\n",
	      dirname, bname, ext, fname);
    }

    // find it in the directory
    //
    rewinddir(dir);
    bool match = false;
    while ((ent = readdir(dir)) != NULL) {
      if (strcmp(ent->d_name, bname) == 0) {
	match = true;
	break;
      }
    }
    if (match == true) {
      fnames_a[nf_a] = new char[strlen(fname) + 1];
      strcpy(fnames_a[nf_a], fname);
      nf_a++;
    }
    else {
      found = false;
      break;
    }

    // display debugging information
    //
    if (debug_level_d >= LEVEL_FULL) {
      fprintf(stdout, " Edf::get_matching_filenames(): %ld: [%s]\n",
	      nf_a - 1, fnames_a[nf_a - 1]);
    }
  }

  // close the directory
  //
  closedir(dir);

  // display debug information
  //
  if (debug_level_d >= LEVEL_DETAILED) {
    fprintf(stdout, "Edf::get_matching_filenames(): done getting filenames\n");
  }

  // exit gracefully
  //
  return true;
}

// method: create_matching_filenames
//
// arguments:
// 
//  char* dirname: directory name (output)
//  char* bname: base filename (no directory, no extention) (output) 
//  char* ext: file extention (output)
//  char* fname: matching filename (output)
//  long nf: index of file/channel (input)
//  char* bfn: base filename (input)
//  char* fmt: filename modifier format specification (input)
//
// return: an boolean value indicating status
//
// This method permutes a filename according to a spec provided by the user.
// The full pathname is returned (fname) along with its components
// (dirname + bname + 'fmt' + ext).
//
// Note that this method assumes memory is allocated.
//
bool Edf::create_matching_filename(char* dirname_a, char* bname_a,
				   char* ext_a, char* fname_a,
				   long nf_a, char* bfn_a, char* fmt_a) {

  // declare some string buffers needed to hold filename pieces
  //
  char fmt_string[MAX_MSTR_LENGTH];

  // display debug information
  //
  if (debug_level_d >= LEVEL_DETAILED) {
    fprintf(stdout,
	    "Edf::create_matching_filename(): [nf, bfn, fmt] [%ld, %s, %s]\n",
	    nf_a, bfn_a, fmt_a);
  }

  // locate some critical points in the input filename
  //
  char* ddelim = rindex(bfn_a, SLASH[0]);
  char* edelim = rindex(bfn_a, DOT[0]);

  // parse the input filename into a directory
  //
  if (ddelim == (char*)NULL) {
    strcpy(dirname_a, DOT);
  }
  else {
    strncpy(dirname_a, bfn_a, ddelim - bfn_a);
    dirname_a[ddelim - bfn_a] = (char)NULL;
  }

  // parse the input filename into a basename and extension
  //
  if (edelim == (char*)NULL) {
    return false;
  }
  else {
    if (ddelim == (char*)NULL) {
      strncpy(bname_a, bfn_a, edelim - bfn_a);
      bname_a[edelim - bfn_a] = (char)NULL;
    }
    else {
      strncpy(bname_a, ddelim + 1, edelim - (ddelim + 1));
      bname_a[edelim - (ddelim + 1)] = (char)NULL;
    }
  }
  strcpy(ext_a, edelim);

  // build a filename template for use with sprintf
  //
  strcpy(fmt_string, bname_a);
  strcat(fmt_string, fmt_a);
  strcat(fmt_string, ext_a);

  // form a new filename
  //
  sprintf(bname_a, fmt_string, nf_a);
  strcpy(fname_a, dirname_a);
  strcat(fname_a, SLASH);
  strcat(fname_a, bname_a);

  if (debug_level_d >= LEVEL_FULL) {
    fprintf(stdout,
	    "Edf::create_matching_filename(): new filename [%s]\n", fname_a);
  }

  // display debugging information
  //
  if (debug_level_d >= LEVEL_DETAILED) {
    fprintf(stdout,
	    "Edf::create_matching_filename(): done creating filename...\n");
  }

  // exit gracefully
  //
  return true;
}

// method: compute_header_size
//
// arguments:
//  long nchan: number of channels
//
// return: a long value containing the size of the header in bytes
//
// Apparently, an EDF header is designed to be equal to: nchan * 256 + 256.
// This method encapsulates this.
//
long Edf::compute_header_size(long nchan_a) {

  // return the size
  //
  return nchan_a * EDF_BSIZE + EDF_BSIZE;
}

// method: get_channel_pos
//
// arguments:
//  char* label: a label containing the desired channel (input)
//  MATCH_MODE matmode: an enum indicating the precision of the match (input)
//
// return: a long integer containing the channel position
//         or -1 if it is not found.
//
// This method returns the position of channel in the set of labels in
// the header. In fact this method performs the same function of find_match.
// However, this method does not receive variables related to the header as
// the latter does.  
//
long Edf::get_channel_pos(char* label_a, MATCH_MODE match_mode_a) {

  // save the match mode:
  //  there are other methods depending on the selected match mode. 
  // 
  smmode_d = match_mode_a;
  
  // declare local variables
  //
  long pos;
  
  // take the position of label_a within the set of channels
  //
  if ((pos = Edf::find_match(label_a, hdr_ghdi_nsig_rec_d, 
                             hdr_chan_labels_d, smmode_d)) < 0) {
    fprintf(stdout,
	   "**> Edf::get_channel_pos(): channel [%s] not found\n",
           label_a);

    // return -1 if position was not found
    //
    return ERR_MATCH;
  }

  // return channel position
  //
  return pos;
}

// method: get_montage_channels
//
// arguments:
//  long& nl_a: the number of channels (output)
//  char** channels_a: the array containing all the channels specified (output)
//
// return: a boolean value indicating status.
//
// This method returns the unique channels of a montage specification and
// the total number of all channels specified.
//
// Note that it returns all channels with no duplication.
//
bool Edf::get_montage_channels(long& nl_a, char** channels_a) {
  
  // declare local variables
  //
  bool status = true;

  // variable to hold the current index of channels_a
  //
  long new_occurrence = 0;

  // flag to check duplication 
  //
  bool found;
  
  // loop over all channels in mlabels1_d 
  //
  for (long i = 0; i < num_mlabels_d; i++) { 

    // set flag to false
    //
    found = false;

    // loop over the values stored in channels_a so far
    //
    for (long j = 0; j < new_occurrence; j++) {
      
      // check if the current channel i was not stored yet
      //
      if (strcmp( mlabels1_d[i], channels_a[j]) == 0) {

	// set flag to true
	//
	found = true;

	// get out from loop to check channel i + 1
	//
	break;
      }
    }

    // copy the first occurrence of a channel in montage specification:
    //  since no duplication was found, store channel i into channels_a.
    //
    if (found == false) {
      channels_a[new_occurrence] = new char[strlen(mlabels1_d[i]) + 1];
      strcpy(channels_a[new_occurrence++], mlabels1_d[i]);
    }

  }

  // loop over all channels in mlabels2_d
  //
  for (long i = 0; i < num_mlabels_d; i++) {

    // set flag to false
    //    
    found = false;

    // check if the current channel i was not stored yet
    //
    for (long j = 0; j < new_occurrence; j++) {
      
      if (strcmp(mlabels2_d[i], channels_a[j]) == 0) {

	// set flag to true
	//
	found = true;

	// get out from loop to check channel i + 1
	//
	break;
      }
    }
    
    // copy the first occurrence of a channel in montage specification:
    //  since no duplication was found, store channel i into channels_a.
    //
    if (found == false) {
      channels_a[new_occurrence] = new char[strlen(mlabels2_d[i]) + 1];
      strcpy(channels_a[new_occurrence++], mlabels2_d[i]);
    }   
  }

  // total number of different channels
  // specified by the montage.
  //
  nl_a = new_occurrence;

  // exit gracefully
  //
  return status;
}

// method: create_directory
//
// arguments:
//  char* path:   input path (input)
//  mode_t mode:  permissions for the directories to be created
//
// return: a boolean indicating status
//
// This method creates the directories in the input path if the directories do
// not exist.
//
bool Edf::create_directory(char* path_a, mode_t mode_a) {

  // const cast for the input path
  //
  char* p = const_cast<char*>(path_a);

  // Do mkdir for each slash until end of string or error
  //
  while (*p != (char)NULL) {

    // Skip first character
    //
    p++;

    // Find first slash or end
    //
    while(*p != (char)NULL && *p != SLASH[0]) p++;

    // Remember value of the path
    //
    char v = *p;

    // Write end of string at p
    //
    *p = (char)NULL;

    // Create folder from path to the null char inserted at p
    //
    if(mkdir(path_a, mode_a) == ERR_FILE && errno != EEXIST) {
      *p = v;
      return false;
    }
    
    // Restore path
    //
    *p = v;

  }

  return true;
}
//
// end of file
