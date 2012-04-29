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

#include "SIMDAQThread.h"
#include <iostream>
#include <QDateTime>

SIMDAQThread::SIMDAQThread(int num, double min, double max, double rate) {
  init_ready_ = true;
  // SIMDAQThread Constructor should setup each channel with the same
  // background limits and rate.
  init_chans(num,min,max,rate);
  set_npk(0);
  last_time_ = time(NULL);
  times_.push_back(last_time_);
  addPeak(500,20,100,0);
  addPeak(750,50,100,0);
}

SIMDAQThread::~SIMDAQThread() {
  qDeleteAll(pk_);
  qDeleteAll(bkg_);
}

int SIMDAQThread::addPeak(double value, double sigma, double rate, int chan) {
  // Peaks are added after construction
  if (chan >= nchan_) {
    return -1;
  }
  peak* p = new peak;
  p->chan = chan;
  p->rate = rate;
  p->sigma = sigma;
  p->value = value;
  pk_.push_back(p);
  ++npk_;
  return 0;
}

int SIMDAQThread::removePeak(int index) {
  if (index < 0 || index >= npk_ || npk_ == 0) {
    return -1;
  }
  delete(pk_.takeAt(index));
  --npk_;
  return 0;
}

GRIDAQBaseAccumNode* SIMDAQThread::RegisterDataOutput(QString outName) {
  GRIDAQBaseAccumNode* p = NULL;
  if (outName == "ADCOutput") {
    p = new GRIDAQAccumulator<double>(outName,1e8,5,250);
  } else if (outName == "TS") {
    p = new GRIDAQAccumulator<qint64>(outName,1e8,5,250);
  }
  return p;
}

int SIMDAQThread::AcquireData(int n) {
  return 0;
}

int SIMDAQThread::StartDataAcquisition() {
  start_time_ = QDateTime::currentDateTime();
  prev_time_ = start_time_;
  InitializeAccumulators(start_time_,0,1e8,4,250);
  return 0;
}

void SIMDAQThread::init_chans(int numOfChans, double min, double max,
                              double rate) {
  if (numOfChans > 0) {
    if (init_ready_) {
      bkg_.clear();   
    } else {
      //log << "DAQ not ready to initialize new channels." << endl;
      //Commit//log(GRI//log_ERROR);
    }
    background* b;
    for (int i = 0; i < numOfChans; ++i) {
      b = new background;
      b->max = max;
      b->min = min;
      b->rate = rate;
      b->chan = i;
      bkg_.push_back(b);
    }
    nchan_ = numOfChans;
  } else {
    //log << "Not enough channels, must be at least 1 channel" << endl;
    //Commit//log(GRI//log_ERROR);
  }
}
