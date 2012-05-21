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
  LynxDAQ();
  ~LynxDAQ();

  int AcquireData(int n);

  GRIDAQBaseAccumNode *RegisterDataOutput(QString outName);

  int ConnectToDAQ();
  int LoadConfiguration();
  int Initialize();
  int StartDataAcquisition();
  int StopDataAcquisition();

  void TurnOnHV(long V);
  void TurnOnHV(){TurnOnHV((long)5000);}
  void TurnOffHV();
  int IsHVOn(); // Returns 0 if off, 1 if on, 2 if ramping
  double HV(); // Returns current value of the voltage

  bool IsConnected(){return isConnected;}
  bool IsAcquiring(){return (LiveTime()-currLiveTime>0.01);}
  //Return real and live times in seconds:
  double LiveTime(){return (double)lynx->GetParameter(DevCntl::Elapsed_Live, input);}
  double RealTime(){return (double)lynx->GetParameter(DevCntl::Elapsed_Real, input);}


 private:
  long input;
  double currLiveTime;
  double currRealTime;
  DevCntl::IDevicePtr lynx;
  VARIANT Args;
  variant_t timeBase;

  QDateTime start_time_;
  qint64 dt; // milliseconds between start_time_ and May 1st, 2012, 12:00:00 am.
  bool isConnected;

  void LoadDefaultConfigs();
};

#endif  // LYNXDAQ_H
