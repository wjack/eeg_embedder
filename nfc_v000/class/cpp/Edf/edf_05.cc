// file: $(NEDC_NFC)/class/cpp/Edf/edf_05.cc
//
// This file contains private methods associated with the class Edf
// that perform interpolation algorithms.
//

// Revision History:
//  20150702 (FG): interpolate_average added

// local include files
//
#include "Edf.h"

// method: interpolate_average
//
// arguments:
//  VVectorDouble& new_chano: interpolated channels (output)
//  VVectorDouble& sigi: original signal in which the interpolated 
//                       channels will be added (input)
//  
// return: a boolean value indicating status
//
// This method performs interpolation averaging the adjacent channels
// related to each new channel to be interpolated.
//
bool Edf::interpolate_average(VVectorDouble& new_chano_a,
			      VVectorDouble& sigi_a) {

  // declare local variables
  //
  bool status = true;
  
  // display a debug message
  //
  if (debug_level_d >= Edf::LEVEL_DETAILED) {
    fprintf(stdout,
	    "Edf::interpolate_average(): beginning interpolation"
	    " by average\n");
  }

  // create output space to hold new channels:
  //  resize new channel number of rows to the number of new channels
  //
  Edf::resize(new_chano_a, num_clabels_d, false);

  // store position of each adjacent channel within original signal:
  //  this needs to be done since the interpolation relies on these
  //  positions.
  //
  long* pos[num_clabels_d];

  // loop over new channels numbers and take the position of its
  // adjacent channels one by one
  //
  for(long i = 0; i < num_clabels_d; i++) {

    // resize columns of position to the number of adjacent channels
    // related to new_channel i
    //
    pos[i] = new long[num_adj_chan_d[i]];

    long j_end = num_adj_chan_d[i];

    // loop over adjacent channels of new channel i
    //
    for(long j = 0; j < j_end; j++) {

      // take position of adjacent channel j in the original signal
      //
      if ((pos[i][j] =
	   Edf::get_channel_pos(adj_chan_labels_d[i][j],
				smmode_d)) < 0) {
	fprintf(stdout,
		"**> Edf::interpolation_average(): no match for [%s]\n",
		adj_chan_labels_d[i][j]);
	return false;
      }
    }
  }

  // loop over all channels to be interpolated
  //
  for(long i = 0; i < num_clabels_d ; i++) {

    // number of adjacent channels related to channel i
    //
    long nadj_channels = num_adj_chan_d[i];

    // create output space:
    //  resize the number of columns to the number of columns of the
    //  the adjacent channels.
    //
    long j_end = sigi_a[pos[i][0]].size();
    Edf::resize(new_chano_a[i], j_end, false);

    // loop over the size of adjacent channels' columns
    //
    for(long j = 0; j < j_end; j++) {

      // initialize discrete point j of new channel i
      //
      new_chano_a[i][j] = 0;

      // variable to perform division
      //
      double sum = 1.0 / (double)nadj_channels;

      // loop over the number of adjacent channels
      //
      for(long k = 0; k < nadj_channels; k++) {

	// perform the sum of its discrete points with the values
	// of adjacent channel k - 1:
	//
	new_chano_a[i][j] += sigi_a[pos[i][k]][j];
      }

      // perform division of all elements to complete average
      // of adjacent channels related to new channel i
      //
      new_chano_a[i][j] *= sum;
    }
  }
  
  // display a debug message
  //
  if (debug_level_d >= Edf::LEVEL_DETAILED) {
    fprintf(stdout,
	    "Edf::interpolate_average(): done interpolation"
	    " by average\n");
  }

  // clean up memory allocated to position of adjacent channels
  //
  for (long i = 0; i <  num_clabels_d; i++) {
    delete [] pos[i];
  }
  
  // exit gracefully
  //
  return status;
}

//
// end of file
