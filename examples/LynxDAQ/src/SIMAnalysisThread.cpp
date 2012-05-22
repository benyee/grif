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

#include <QPair>
#include <iomanip>
#include <iostream>
#include <fstream>
#include "SIMAnalysisThread.h"

SIMAnalysisThread::SIMAnalysisThread(double x, std::string s) {
    dataLength = x;
    filename = s;
}

SIMAnalysisThread::~SIMAnalysisThread() {
}

int SIMAnalysisThread::Analyze() {
    double* ADC; //bin number or energy
    double* ts_sec; //timestamp in seconds
    int nADC;

    QPair<int, double*> pADC = ReadData<double>("LynxDAQ","ADCOutput");
    ADC = pADC.second; nADC = pADC.first;

    QPair<int, double*> pTS = ReadData<double>("LynxDAQ","TS");
    ts_sec = pTS.second;

    std::vector<double>::iterator itr_oldadc = storedEvents.first.begin();
    std::vector<double>::iterator itr_oldts = storedEvents.second.begin();

    if(nADC> 0){
        //Erase old data by iterating through and seeing how current
        //  the timestamp is relative to the new data:
        while(itr_oldadc < storedEvents.first.end() && ts_sec[nADC-1] - *itr_oldts > dataLength){
            storedEvents.first.erase(storedEvents.first.begin());
            storedEvents.second.erase(storedEvents.second.begin());
            itr_oldadc++;
            itr_oldts++;
        }

        //Write new raw data to file and store it:
        std::fstream outfile(filename,std::ios::app);
        for(int i = 0; i < nADC; i++){
            outfile<< ADC[i]<<'\t'<<std::setprecision(25)<<ts_sec[i]<<'\n';
            storedEvents.first.push_back(ADC[i]);
            storedEvents.second.push_back(ts_sec[i]);
        }
        outfile.close();
    }
    std::cout<<"Finished analysis sequence."<<std::endl;

    return 0;
}

