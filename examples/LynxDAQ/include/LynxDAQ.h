#ifndef LYNXDAQ_H
#define LYNXDAQ_H

#define LYNX_IPADDRESS L"10.0.0.3" //Default IP address of Lynx
#define LYNX_DEFAULT 1 //Set to 1 to have default settings.  This program has not been tested for non-default settings.

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
  //Check whether the systme is acquiring by seeing whether the current
  //    live time matches that of the previously stored live time.  (If it's acquiring,
  //    LiveTime() will be growing quickly.)
  bool IsAcquiring(){if(simMode){return isSimulating;} return (LiveTime()-currLiveTime>0.01);}
  //Return real and live times in seconds:
  double LiveTime(){if (simMode){return 0.0;} return (double)lynx->GetParameter(DevCntl::Elapsed_Live, input);}
  double RealTime(){if (simMode){return 0.0;} return (double)lynx->GetParameter(DevCntl::Elapsed_Real, input);}

  double getLiveTime(){if (simMode){return 0.0;} return currLiveTime;};
  double getRealTime(){if (simMode){return 0.0;} return currRealTime;};

  void setLiveTime(){if (simMode){return;} currLiveTime = (double)lynx->GetParameter(DevCntl::Elapsed_Live, input);};
  void setRealTime(){if (simMode){return;} currRealTime = (double)lynx->GetParameter(DevCntl::Elapsed_Real, input);};

  bool isSimMode(){return simMode;}
  void setSimMode(bool x){simMode = x;}

  long getNumberofChannels(){if(simMode){return 8192;} return lynx->GetParameter(DevCntl::Input_NumberOfChannels,input);}

  DevCntl::IDevicePtr GetDeviceHandle(){return lynx;}
  long getInputValue(){return input;}

  //Change/get reference time:
  void setRefTime(QDateTime q){ref_time_ = q;}
  QDateTime getRefTime(){return ref_time_;}

 private:
  long input; //Set to 1
  double currLiveTime; //Live and real times currently stored by Lynx (either for this acquisition or a previous one)
  double currRealTime;
  DevCntl::IDevicePtr lynx;
  VARIANT Args; //Lynx related parameter.. has to do with the device's current setup?
  variant_t timeBase; //Lynx related parameter.. has something to do with how timestamps are output.

  QDateTime start_time_;
  QDateTime ref_time_; //Reference time for timestamps.
  qint64 dt; // milliseconds between start_time_ and REF_TIME
  bool isConnected;

  bool simMode;
  bool isSimulating;
  qint64 simModeTime;

  void LoadDefaultConfigs();
};

#endif  // LYNXDAQ_H
