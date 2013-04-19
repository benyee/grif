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
#include "SIMAnalysisThread.h"

SIMAnalysisThread::SIMAnalysisThread(double x, std::string inp_rawfilename, int inp_maxLineCount) {
    dataLength = x;
    isPlotting1 = false;
    isPlotting2 = false;
    isOutputFileOpen = false;

    maxLineCount = inp_maxLineCount;
    lineCount = 0;
    fileNumber = 0;
    setFileName(inp_rawfilename, false);

    int nChan = 8192;
    double minChan = 0.0;
    double maxChan = 8192.0;
    CreateNewHistogram("Histogram1",nChan,minChan,maxChan);
    GetHistogram("Histogram1")->set_rate_mode(false);
    CreateNewHistogram("Histogram2",nChan,minChan,maxChan);
    GetHistogram("Histogram2")->set_rate_mode(true);
    GetHistogram("Histogram2")->set_packet_scale_factor(0.1);
}

SIMAnalysisThread::~SIMAnalysisThread() {
    closeFile();
}

void SIMAnalysisThread::setBinning(int nBins, double xMax){
    return;
}

int SIMAnalysisThread::Analyze() {
    //std::cout<<"Starting Analysis Sequence...";

    int nADC;

    QPair<int, double*> pADC = ReadData<double>("LynxDAQ","ADCOutput");
    QPair<int, double*> pTS = ReadData<double>("LynxDAQ","TS");
    QPair<int, double*> pLiveTime = ReadData<double>("LynxDAQ","LiveTime");
    nADC = pADC.first;

    if(nADC> 0){
        double* ADC; //bin number or energy
        double* ts_sec; //timestamp in seconds
        double liveTime;
        ADC = pADC.second; ts_sec = pTS.second; liveTime = pLiveTime.second[0];
        //Trying to figure out why the live time is sometimes zero:
        if(liveTime < 1e4){
            std::cout<<"HELLOOOOO"<<std::endl;
            std::cout<<"liveTime ="<<liveTime<<std::endl;
            std::cout<<ADC[0]<<'\t'<<ts_sec[0]<<std::endl;
            std::cout<<"Length of data: "<<nADC<<std::endl;
            liveTime = pLiveTime.second[nADC-1];
        }

        if(isPlotting1){
            UpdateHistogram("Histogram1",ADC,nADC);
        }
        if(isPlotting2){
            UpdateHistogram("Histogram2",ADC,nADC);
        }

        //I'm getting rid of this temporarily to see whether it helps...
        /*
        //Erase old data by iterating through and seeing how old the stored data is relative to the new data:
        //If the data entry is more than dataLength older than dataLength, delete it.
        while(storedEvents.first.size()>0 && ts_sec[nADC-1] - *storedEvents.second.begin() > dataLength){
            storedEvents.first.erase(storedEvents.first.begin());
            storedEvents.second.erase(storedEvents.second.begin());
        }*/

        //Write new raw data to file and then store it:
        for(int i = 0; i < nADC; i++){
            //outfile<< ADC[i]<<'\t'<<std::setprecision(25)<<ts_sec[i]<<'\t'<<std::setprecision(25)<<liveTime<<'\n';
            outfile<< ADC[i]<<'\t'<<floor(pow(10.0,6)*ts_sec[i])/pow(10.0,6)<<'\n';
            lineCount++;
            //storedEvents.first.push_back(ADC[i]);
            //storedEvents.second.push_back(ts_sec[i]);
        }
        outfile<< -9999 <<'\t'<<(int)liveTime<<'\n';

        if(lineCount >= maxLineCount){
            closeFile();
            fileNumber++;
            setFileName(filename,false);
            openFile();
            lineCount = 0;
        }

        //std::cout<<"Analysis ts_sec: "<<std::endl;
        //std::cout<<ts_sec[nADC-1]<<std::endl;

    }

    //std::cout<<"Finished analysis sequence."<<std::endl;

    return 0;
}

void SIMAnalysisThread::setFileName(std::string s,bool restart, std::string ext){
    if(restart){
        std::string temp = QDateTime::currentDateTime().toString("yyyyMMdd__hhmm_ss_zzz").toStdString();
        rawfilename =  s +"_"+ temp;
    }

    if (restart) {fileNumber = 0;}
    filename = "C:/Users/BeARING/Dropbox/UCB Air Monitor/Data/Roof/current/" +rawfilename +"_" + QString::number(fileNumber).toStdString()+ ext;
}

void SIMAnalysisThread::setHistRate(double rate, int histNum){
    if(histNum==1){GetHistogram("Histogram1")->set_packet_scale_factor(rate);}
    else{GetHistogram("Histogram2")->set_packet_scale_factor(rate);}
}
void SIMAnalysisThread::setHistMode(bool x, int histNum){
    if(histNum==1){GetHistogram("Histogram1")->set_rate_mode(x);}
    else{GetHistogram("Histogram2")->set_rate_mode(x);}
}

void SIMAnalysisThread::openFile(){
    if(isOutputFileOpen){ return;}
    outfile = std::fstream(filename,std::ios::app);
    isOutputFileOpen = true;
}

