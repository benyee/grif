#include "LynxDAQ.h"


LynxDAQ::LynxDAQ() {
    lynx = Utilities::Device();
    input = 1;
    VariantInit (&Args);

    isConnected = false;

    currLiveTime = 0;
    currRealTime = 0;
}

LynxDAQ::~LynxDAQ() {
    delete(lynx);
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
    if(isConnected){return 0;}
    try{
        //Open a connection to the device
        bstr_t lynxAddress;
        if (LYNX_DEFAULT){lynxAddress = LYNX_IPADDRESS;}
        else{lynxAddress = Utilities::getLynxAddress ();}
        lynx->Open(Utilities::getLocalAddress (lynxAddress), lynxAddress);

        //Display the name of the lynx
        cout << "You are connected to: " << Utilities::GetString ((bstr_t)(lynx->GetParameter (DevCntl::Network_MachineName, (short) 0))) << "\n";
        isConnected = true;


        //Update current times:
        currLiveTime = LiveTime();
        currRealTime = RealTime();
    }catch(int e){
    }
    return 0;
}

int LynxDAQ::LoadConfiguration(){
    //Gain ownership
    lynx->LockInput (_bstr_t("administrator"), _bstr_t("password"), input);
    cout<<"Gained ownership."<<endl;

    if(LYNX_DEFAULT){
        LoadDefaultConfigs();
        return 0;
    }

    Utilities::disableAcquisition(lynx, input);
    cout<<"Disabled acquisition."<<endl;

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
    variant_t presetMode = variant_t (DevCntl::PresetRealTime);
    lynx->PutParameter (DevCntl::Preset_Options, input, &presetMode);
    if (DevCntl::PresetNone != (LONG) presetMode)
    {
        variant_t preset = variant_t ((double)2);
        lynx->PutParameter ((DevCntl::PresetLiveTime == (LONG) presetMode) ? DevCntl::Preset_Live : DevCntl::Preset_Real, input, &preset);
    }
    return 0;
}

int LynxDAQ::Initialize(){
    //Clear data and time
    lynx->Control (DevCntl::Clear, input, &Args);

    TurnOnHV();

    //Wait till ramped
    while(VARIANT_TRUE == (VARIANT_BOOL)(variant_t)(lynx->GetParameter (DevCntl::Input_VoltageRamping, input)))
    {
        cout << "HVPS is ramping... voltage is at "<<HV()<<endl;
        Sleep (500);
    }

    //Set the current memory group
    variant_t group = 1L;
    lynx->PutParameter (DevCntl::Input_CurrentGroup, input, &group);

    return 0;
}

int LynxDAQ::StartDataAcquisition() {
  start_time_ = QDateTime::currentDateTime();
  InitializeAccumulators(start_time_,0);

  //Clear the memory and start the acquisition
  lynx->Control (DevCntl::Clear, input, &Args);
  lynx->Control (DevCntl::Start, input, &Args);

  return 0;
}

int LynxDAQ::AcquireData(int n) {
    //Get the list data
    variant_t listB = lynx->GetListData (input);

    //Set the proper time base:
    timeBase=Utilities::Get1DSafeArrayElement (listB, 4);
    double cnv = (double)timeBase/1000; //Convert to uS

    variant_t tlistD = Utilities::Get1DSafeArrayElement (listB, 6);
    LONG numE = Utilities::GetCount (tlistD);
    static const long ROLLOVERBIT=0x00008000;
    static unsigned __int64 RolloverTime=0;
    unsigned short recTime=0, recEvent=0;
    unsigned __int64 Time=0;
    LONG i=0;

    vector<double> ADC;
    vector<qint64> ts;


    for(Time=0, i=0; i<numE; i+=2) {
        recEvent = Utilities::Get1DSafeArrayElement (tlistD, i);
        recTime = Utilities::Get1DSafeArrayElement (tlistD, i + 1);

        if (!(recTime&ROLLOVERBIT)) {
            Time = RolloverTime | (recTime & 0x7FFF);
        }
        else {
            long LSBofTC = 0;
            long MSBofTC = 0;
            LSBofTC |= (recTime & 0x7FFF) << 15;
            MSBofTC |= recEvent << 30;
            RolloverTime = MSBofTC | LSBofTC;

            //goto next event
            continue;
        }

        ADC.push_back((double)recEvent);
        ts.push_back((qint64)(Time*cnv));
        Time=0;
        //cout << "Event: " << ADC.back()<< "; Time (uS): " << ts.back()<< " (LynxDAQ)"<<endl;
    }

    PostData<double>(ADC.size(), "ADCOutput",&ADC[0],&ts[0]);
    PostData<qint64>(ADC.size(), "TS",&ts[0],&ts[0]);
    //cout<<"Just posted"<<endl;

    currRealTime = RealTime();
    currLiveTime = LiveTime();
    return 0;
}

int LynxDAQ::StopDataAcquisition(){
    cout<<"About to disable acquisition..."<<endl;
    Utilities::disableAcquisition(lynx, input);
    cout<<"Disabled Acquisition"<<endl;

    lynx->Control (DevCntl::Clear, input, &Args);
    return 0;
}

void LynxDAQ::LoadDefaultConfigs(){

    Utilities::disableAcquisition(lynx, input);
    cout<<"Disabled acquisition."<<endl;

    //Set the acq mode
    // see page 32 of pdf for documentation on different list modes
    // List just gives bin #'s, TList includes a timestamp.
    variant_t specMode = DevCntl::Tlist;
    lynx->PutParameter (DevCntl::Input_Mode, input, &specMode);

    //Set the external sync to master mode & disable
    lynx->PutParameter(DevCntl::Input_ExternalSyncMode, input, &_variant_t(DevCntl::SyncMaster));
    lynx->PutParameter(DevCntl::Input_ExternalSyncStatus, input, &_variant_t((LONG) 0));

    //Set list acq mode mode
    ULONG mode= (LONG) DevCntl::TimeBase100ns;
    mode |= (LONG) DevCntl::LatchOnFastDiscPedge;
    lynx->PutParameter(DevCntl::Input_ListAcqOptions, input, &_variant_t(mode));

    //Setup the presets
    variant_t presetMode = variant_t (DevCntl::PresetNone);
    lynx->PutParameter (DevCntl::Preset_Options, input, &presetMode);
}
void LynxDAQ::TurnOnHV(long v){
    //Turn on HV
    variant_t voltageStatus = VARIANT_TRUE;
    lynx->PutParameter (DevCntl::Input_VoltageStatus, input, &voltageStatus);

    //Set HV to v
    variant_t voltage = v;
    lynx->PutParameter(DevCntl::Input_Voltage, input, &voltage);
}
void LynxDAQ::TurnOffHV(){
    //Turn off HV
    variant_t voltageStatus = VARIANT_FALSE;
    lynx->PutParameter (DevCntl::Input_VoltageStatus, input, &voltageStatus);
}
int LynxDAQ::IsHVOn(){
    if(VARIANT_TRUE == (VARIANT_BOOL)(variant_t)(lynx->GetParameter (DevCntl::Input_VoltageRamping, input))){
        return 2;
    }
    else if(VARIANT_TRUE == (VARIANT_BOOL)(variant_t)(lynx ->GetParameter(DevCntl::Input_VoltageStatus,input))){
        return 1;
    }
    return 0;
}
double LynxDAQ::HV(){
    double volt = lynx->GetParameter(DevCntl::Input_VoltageReading, input);
    return (double)volt;
}

