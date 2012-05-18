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
  void LoadDefaultConfigs();

  void TurnOnHV(long V);
  void TurnOnHV(){TurnOnHV((long)5000);}
  void TurnOffHV();
  int IsHVOn(); //Returns 0 if off, 1 if on, 2 if ramping
  double HV(); //Returns current value of the voltage

  bool IsConnected(){return isConnected;}
  bool IsAcquiring(){return isAcquiring;}
  //Return real and live times in seconds:
  double RealTime(){return (double)DevCntl::Elapsed_Live/100000;}
  double LiveTime(){return (double)DevCntl::Elapsed_Real/100000;}


 private:
  long input;
  DevCntl::IDevicePtr lynx;
  VARIANT Args;
  variant_t timeBase;

  QDateTime start_time_;
  bool isConnected;
  bool isAcquiring;

};

#endif  // LYNXDAQ_H
