#include "LynxDAQ.h"

#import "DevCntl.tlb" rename_namespace("DevCntl")

LynxDAQ::LynxDAQ(int num, double min, double max, double rate) {
}

LynxDAQ::~LynxDAQ() {
}


GRIDAQBaseAccumNode* LynxDAQ::RegisterDataOutput(QString outName) {
  GRIDAQBaseAccumNode* p = NULL;
  if (outName == "ADCOutput") {
    p = new GRIDAQAccumulator<double>(outName,1e8,5,250);
  } else if (outName == "TS") {
    p = new GRIDAQAccumulator<qint64>(outName,1e8,5,250);
  }
  return p;
}

int LynxDAQ::ConnectToDAQ(){
    //Open a connection to the device
    lynx->Open ('10.0.0.7', '10.0.0.3');

    //Display the name of the lynx
    cout << "You are connected to: " << (lynx->GetParameter (DevCntl::Network_MachineName, (short) 0)) << "\n";

    return 0;
}

int LynxDAQ::AcquireData(int n) {
    //Dummy data:
    double* dummyADC = new double[5];
    qint64* dummy_ts = new qint64[5];
    PostData<double>(5, "ADCOutput",dummyADC,dummy_ts);
    PostData<qint64>(5, "TS",dummy_ts,dummy_ts);
  return 0;
}

int LynxDAQ::StartDataAcquisition() {
  start_time_ = QDateTime::currentDateTime();
  prev_time_ = start_time_;
  InitializeAccumulators(start_time_,0,1e8,4,250);
  return 0;
}
