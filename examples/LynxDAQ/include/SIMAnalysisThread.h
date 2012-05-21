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

#ifndef GRIF_EXAMPLES_SIMULATOR_SIMANALYSISTHREAD_H_
#define GRIF_EXAMPLES_SIMULATOR_SIMANALYSISTHREAD_H_

#include <core/GRIAnalysisThread.h>
#include "QFile"
#include "ActivityCounter.h"
#include <string>

class SIMAnalysisThread : public GRIAnalysisThread {
 public:
    SIMAnalysisThread(double x = 60, std::string s = "out.txt");
    ~SIMAnalysisThread();

    int Analyze();

    std::string getFileName(){return filename;}
    void setFileName(std::string s){filename = s;}

 private:
    QPair<std::vector<double>,std::vector<double> > storedEvents;
    //This Qpair will store all the events of the last minute.

    double dataLength; //Time interval of data storage.
    //(e.g. Setting dataLength to 10000 will mean that storedEvents has
    //  has the data from the last 10000 [insert units of the timestamp here] of collection)
    //  Default for LynxDAQ is microseconds.

    std::string filename;  //File name for raw data output.
};

#endif  // GRIF_EXAMPLES_SIMULATOR_SIMANALYSISTHREAD_H_
