/*
 *  GRICalibration.h
 *  
 *
 *  Created by Austin Benson on 8/10/10.
 *  Copyright 2010 __MyCompanyName__. All rights reserved.
 *
 */

#include "TH1D.h"
#include "TSpectrum"
#include "GRIhistogramer.h"


class GRICalibration {

	public:
	
	//constructors
	GRICalibration(TH1D *, histogrammer *, int, double *);
	
	//destructor
	~GRICalibration();
	
	//histogram
	TH1D *hist;
	
	//root class that analyzes histograms
	//peaks stored in spect->fPosition
	TSpectrum *spect;
	
	//data
	struct linearOffsetData {
		float slope;
		float offset;
		float *peaks;
		float *energies;
	} linOffsetData;
	
	//marker
	int ID;
	
	//find and set data in linOffsetData
	int findLinOffset();
	
}
