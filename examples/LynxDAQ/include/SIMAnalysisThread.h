// Copyright (C) 2012 Gamma-ray Imaging Framework Team
// 
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 3.0 of the License, or (at your option) any later version.
// 
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Lesser General Public License for more details.
// 
// You should have received a copy of the GNU Lesser General Public
// License along with this library; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
// 
// The license can be found in the LICENSE.txt file.
//
// Contact:
// Dr. Daniel Chivers
// dhchivers@lbl.gov
//*****************************************************
//*****************************************************
//This thread writes time and energy bin data to a .txt file and manages the spectrum/histogram.

#ifndef GRIF_EXAMPLES_SIMULATOR_SIMANALYSISTHREAD_H_
#define GRIF_EXAMPLES_SIMULATOR_SIMANALYSISTHREAD_H_

#include <core/GRIAnalysisThread.h>
#include "QFile"
#include "ActivityCounter.h"
#include <string>

class SIMAnalysisThread : public GRIAnalysisThread {
 public:
    SIMAnalysisThread(double x = 60, std::string s = "out.txt"); //Set the default storage time and output file name.
    ~SIMAnalysisThread();

    int Analyze();
    void setFileName(std::string s,bool timestamp = true, std::string ext = ".txt");
    void setBinning(int nBins, double xMax);

    std::string getFileName(){return filename;}
    double getDataLength(){return dataLength;}
    void setDataLength(double x){dataLength = x;}
    void setPlotStatus(bool x,int histNum){if(histNum==1){isPlotting1 = x;} else{isPlotting2=x;}}
    bool getPlotStatus(int histNum){if(histNum==1){return isPlotting1;}return isPlotting2;}
    void setHistRate(double rate, int histNum);
    void setHistMode(bool x, int histNum);

 private:
    QPair<std::vector<double>,std::vector<double> > storedEvents;
    //This Qpair will store all the events of within the last dataLength of acquisition time.

    double dataLength; //Time interval of data storage.
    //(e.g. Setting dataLength to 10000 will mean that storedEvents has
    //  has the data from the last 10000 [insert units of the timestamp here] of collection)
    //  Default for LynxDAQ output from the TS ReadData is in seconds.

    std::string filename;  //File name for raw data output.

    bool isPlotting1;
    bool isPlotting2;
};

#endif  // GRIF_EXAMPLES_SIMULATOR_SIMANALYSISTHREAD_H_
