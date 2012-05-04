#ifndef LYNXDAQ_H
#define LYNXDAQ_H

#include <core/GRIDAQThread.h>
#include "Utilities.h"
#include <string>
//#import "DevCntl.tlb" rename_namespace("DevCntl")

class LynxDAQ : public GRIDAQThread {
 public:
  LynxDAQ(int num=1, double min=0, double max=65535, double rate=1);
  ~LynxDAQ();

  int AcquireData(int n);

  GRIDAQBaseAccumNode *RegisterDataOutput(QString outName);

  int ConnectToDAQ();
  int Initialize();
  int LoadConfiguration();

  //Called at the beginning each run.
  int StartDataAcquisition();

  //not needed now, called at the end of each run.
  int StopDataAcquisition() { return 0; }

 private:
  long input;
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
  void TurnOnHV(long V);
  void TurnOnHV(){TurnOnHV((long)5000);}
  void TurnOffHV();
};

#endif  // LYNXDAQ_H
