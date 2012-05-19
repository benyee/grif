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

SIMAnalysisThread::SIMAnalysisThread() {
    RatePlotter = new ActivityCounter(0,8000,500000);
}

SIMAnalysisThread::~SIMAnalysisThread() {
}

int SIMAnalysisThread::Analyze() {
    double* ADC;
    qint64* ts;
    int nADC;

    QPair<int, double*> pADC = ReadData<double>("LynxDAQ","ADCOutput");
    ADC = pADC.second; nADC = pADC.first;

    QPair<int, qint64*> pTS = ReadData<qint64>("LynxDAQ","TS");
    ts = pTS.second;

    RatePlotter->addData(nADC,ADC,ts);

    QPair<std::vector<double>,std::vector<qint64> > activity = RatePlotter->getPlotData();
    std::vector<double> e = activity.first;
    std::vector<qint64> t = activity.second;
    std::vector<double>::iterator itr_e = e.begin();
    std::vector<qint64>::iterator itr_t = t.begin();

    for(itr_e = e.begin(); itr_e < e.end(); itr_e++){
        std::cout<<"The rate at time "<<*itr_t<<" is "<<*itr_e<<std::endl;
        itr_t++;
    }
    std::cout<<"Analyzed"<<std::endl;

    return 0;
}

