#ifndef LYNXDAQ_H
#define LYNXDAQ_H

#include <core/GRIDAQThread.h>
#include "Utilities.h"
//#import "DevCntl.tlb" rename_namespace("DevCntl")

class LynxDAQ : public GRIDAQThread {
 public:
  LynxDAQ(int num=1, double min=0, double max=65535, double rate=1);
  ~LynxDAQ();

  int AcquireData(int n);

  GRIDAQBaseAccumNode *RegisterDataOutput(QString outName);

  // No DAQ to connect to
  int ConnectToDAQ();

  // not needed now
  int Initialize() { return 0; }

  // not needed now
  int LoadConfiguration() { return 0; }

  //Called at the beginning each run.
  int StartDataAcquisition();

  //not needed now, called at the end of each run.
  int StopDataAcquisition() { return 0; }

 private:
  DevCntl::IDevicePtr lynx;
  int npk_;
  int nchan_;
  QDateTime start_time_;
  QDateTime prev_time_;
  QList<peak*> pk_;
  QList<background*> bkg_;
  bool init_ready_;
  time_t last_time_;
  QVector<time_t> times_;
};

#endif  // LYNXDAQ_H
