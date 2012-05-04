#include "LynxDAQ.h"

#import "DevCntl.tlb" rename_namespace("DevCntl")

LynxDAQ::LynxDAQ(int num, double min, double max, double rate) {
}

LynxDAQ::~LynxDAQ() {
     lynx = Utilities::Device();
     input = 1;
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
    /*(
    //Open a connection to the device
    lynx->Open ('10.0.0.7', '10.0.0.3');

    //Display the name of the lynx
    cout << "You are connected to: " << (lynx->GetParameter (DevCntl::Network_MachineName, (short) 0)) << "\n";
    */
    try{
        //Open a connection to the device
        bstr_t lynxAddress = L"10.0.0.3";//Utilities::getLynxAddress ();
        lynx->Open(Utilities::getLocalAddress (lynxAddress), lynxAddress);
        //Display the name of the lynx
        cout << "You are connected to: " << Utilities::GetString ((bstr_t)(lynx->GetParameter (DevCntl::Network_MachineName, (short) 0))) << "\n";

        //Gain ownership
        lynx->LockInput (_bstr_t("administrator"), _bstr_t("password"), input);
    }catch(int e){
    }
    //CATCH_EX(lynx);
    return 0;
}

int LynxDAQ::Initialize(){
    //Disable all acquisition
    Utilities::disableAcquisition(lynx, input);

    TurnOnHV();

    return 0;
}

int LynxDAQ::LoadConfiguration(){
    /*
    //Set the acq mode
    // see page 32 of pdf for documentation on different list modes
    // List just gives bin #'s, TList includes a timestamp.
    variant_t specMode = variant_t ((LONG) Utilities::getListMode ());
    lynx->PutParameter (DevCntl::Input_Mode, input, &specMode);

    //Set the external sync to master mode & disable
    lynx->PutParameter(DevCntl::Input_ExternalSyncMode, input, &_variant_t(DevCntl::SyncMaster));
    lynx->PutParameter(DevCntl::Input_ExternalSyncStatus, input, &_variant_t((LONG) 0));

    //Set list acq mode mode
    cout << "\nTimebase: (0=1uSec or 1=.1uSec): ";
    ULONG mode=0;
    ULONG Val=0;
    cin >> Val;
    if (Val) {mode=(LONG) DevCntl::TimeBase100ns;}
    else {mode = (LONG) 0;}
    cout << "\nTrigger on: (0=Peak Detect or 1=Fast Discriminator): ";
    cin >> Val;
    if (Val) {mode |= (LONG) DevCntl::LatchOnFastDiscPedge;}
    lynx->PutParameter(DevCntl::Input_ListAcqOptions, input, &_variant_t(mode));

    //Setup the presets
    variant_t presetMode = variant_t ((LONG) Utilities::getPresetMode ());
    lynx->PutParameter (DevCntl::Preset_Options, input, &presetMode);
    if (DevCntl::PresetNone != (LONG) presetMode)
    {
        variant_t preset = variant_t ((double) Utilities::getFloat ("Enter the preset value (S): ", 0.0f, 3.40282e+038f));
        lynx->PutParameter ((DevCntl::PresetLiveTime == (LONG) presetMode) ? DevCntl::Preset_Live : DevCntl::Preset_Real, input, &preset);
    }

    //Clear data and time
    VARIANT Args;
    VariantInit (&Args);
    lynx->Control (DevCntl::Clear, input, &Args);
    */
    return 0;
}

int LynxDAQ::StartDataAcquisition() {
  start_time_ = QDateTime::currentDateTime();
  prev_time_ = start_time_;
  InitializeAccumulators(start_time_,0,1e8,4,250);
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

void LynxDAQ::TurnOnHV(long v){
    //Turn on HV
    variant_t voltageStatus = VARIANT_TRUE;
    lynx->PutParameter (DevCntl::Input_VoltageStatus, input, &voltageStatus);

    //Set HV to v
    variant_t voltage = v;
    lynx->PutParameter(DevCntl::Input_Voltage, input, &voltage);

    //Wait till ramped
    while(VARIANT_TRUE == (VARIANT_BOOL)(variant_t)(lynx->GetParameter (DevCntl::Input_VoltageRamping, input)))
    {
        cout << "HVPS is ramping...\n";
        Sleep (500);
    }
}

void LynxDAQ::TurnOffHV(){
    //Turn off HV
    variant_t voltageStatus = VARIANT_FALSE;
    lynx->PutParameter (DevCntl::Input_VoltageStatus, input, &voltageStatus);
}
