#ifndef LYNXDAQ_H
#define LYNXDAQ_H

#define LYNX_IPADDRESS L"10.0.0.3"
#define LYNX_DEFAULT 1

#include <core/GRIDAQThread.h>
#include "Utilities.h"
#include <string>
#import "DevCntl.tlb" rename_namespace("DevCntl")

class LynxDAQ : public GRIDAQThread {
 public:
  LynxDAQ(int num=1, double min=0, double max=65535, double rate=1);
  ~LynxDAQ();

  int AcquireData(int n);

  GRIDAQBaseAccumNode *RegisterDataOutput(QString outName);

  int ConnectToDAQ();
  int LoadConfiguration();
  int Initialize();
  int StartDataAcquisition();
  int StopDataAcquisition();

 private:
  long input;
  DevCntl::IDevicePtr lynx;
  VARIANT Args;  
  ULONG Status;
  variant_t timeBase;

  QDateTime start_time_;
  QDateTime prev_time_;

  void LoadDefaultConfigs();
  void TurnOnHV(long V);
  void TurnOnHV(){TurnOnHV((long)5000);}
  void TurnOffHV();
};

#endif  // LYNXDAQ_H
