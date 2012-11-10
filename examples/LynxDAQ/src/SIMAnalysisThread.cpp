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
    setFileName(s);
    isPlotting1 = false;
    isPlotting2 = false;

    CreateNewHistogram("Histogram1",8192,0.0,8192.0);
    GetHistogram("Histogram1")->set_rate_mode(true);
    GetHistogram("Histogram1")->set_packet_scale_factor(1);
    CreateNewHistogram("Histogram2",8192,0.0,8192.0);
    GetHistogram("Histogram2")->set_rate_mode(true);
    GetHistogram("Histogram2")->set_packet_scale_factor(0.5);
}

SIMAnalysisThread::~SIMAnalysisThread() {
}

void SIMAnalysisThread::setBinning(int nBins, double xMax){
    return;
}

int SIMAnalysisThread::Analyze() {
    std::cout<<"Starting Analysis Sequence...";

    double* ADC; //bin number or energy
    double* ts_sec; //timestamp in seconds
    int nADC;


    QPair<int, double*> pADC = ReadData<double>("LynxDAQ","ADCOutput");
    QPair<int, double*> pTS = ReadData<double>("LynxDAQ","TS");
    ADC = pADC.second; nADC = pADC.first;
    //std::cout<<nADC<<std::endl;
    ts_sec = pTS.second;

    if(isPlotting1 && nADC > 0){
        UpdateHistogram("Histogram1",ADC,nADC);
    }
    if(isPlotting2 && nADC > 0){
        UpdateHistogram("Histogram2",ADC,nADC);
    }


    if(nADC> 0){
        //Erase old data by iterating through and seeing how old the stored data is relative to the new data:
        //If the data entry is more than dataLength older than dataLength, delete it.
        while(storedEvents.first.size()>0 && ts_sec[nADC-1] - *storedEvents.second.begin() > dataLength){
            storedEvents.first.erase(storedEvents.first.begin());
            storedEvents.second.erase(storedEvents.second.begin());
        }

        //Write new raw data to file and then store it:
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

void SIMAnalysisThread::setFileName(std::string s,bool timestamp, std::string ext){
    if (timestamp){
            std::string temp = QDateTime::currentDateTime().toString("yyyyMMdd__hhmm_ss_zzz").toStdString();
            filename = s +"_"+ temp + ext;
    }
    else{filename = s + ext;}

}

void SIMAnalysisThread::setHistRate(double rate, int histNum){
    if(histNum==1){GetHistogram("Histogram1")->set_packet_scale_factor(rate);}
    else{GetHistogram("Histogram2")->set_packet_scale_factor(rate);}
}
void SIMAnalysisThread::setHistMode(bool x, int histNum){
    if(histNum==1){GetHistogram("Histogram1")->set_rate_mode(x);}
    else{GetHistogram("Histogram2")->set_rate_mode(x);}
}

