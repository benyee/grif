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
}

SIMAnalysisThread::~SIMAnalysisThread() {
}

int SIMAnalysisThread::Analyze() {
  double* ADC;
  unsigned __int64* ts;
  int nADC;

  QPair<int, double*> pADC = ReadData<double>("LynxDAQ","ADCOutput");
  ADC = pADC.second; nADC = pADC.first;

  QPair<int, unsigned __int64*> pTS = ReadData<unsigned __int64>("LynxDAQ","TS");
  ts = pTS.second;

  for(int i = 0; i<nADC; i++){
      std::cout << "\nEvent: " << ADC[i]<< "; Time (uS): " << ts[i]<< endl;
  }
  return 0;
}

