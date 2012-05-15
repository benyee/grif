#include "LynxDAQ.h"


LynxDAQ::LynxDAQ(int num, double min, double max, double rate) {
    lynx = Utilities::Device();
    input = 1;
    VariantInit (&Args);

    packNum = 0;
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
    try{
        //Open a connection to the device
        bstr_t lynxAddress;
        if (LYNX_DEFAULT){
        lynxAddress = LYNX_IPADDRESS;
        }
        else{lynxAddress = Utilities::getLynxAddress ();}
        lynx->Open(Utilities::getLocalAddress (lynxAddress), lynxAddress);

        //Display the name of the lynx
        cout << "You are connected to: " << Utilities::GetString ((bstr_t)(lynx->GetParameter (DevCntl::Network_MachineName, (short) 0))) << "\n";

        //Gain ownership
        lynx->LockInput (_bstr_t("administrator"), _bstr_t("password"), input);
        cout<<"gained ownership"<<endl;
        Utilities::disableAcquisition(lynx, input);
        cout<<"disabled acq"<<endl;
    }catch(int e){
    }
    return 0;
}

int LynxDAQ::LoadConfiguration(){
    if(LYNX_DEFAULT){
        LoadDefaultConfigs();
        return 0;
    }

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

    //Set the current memory group
    variant_t group = 1L;
    lynx->PutParameter (DevCntl::Input_CurrentGroup, input, &group);

    return 0;
}

int LynxDAQ::StartDataAcquisition() {
  start_time_ = QDateTime::currentDateTime();
  prev_time_ = start_time_;
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
        LONG j=0;

        vector<double> dummyADC(numE/2);
        vector<unsigned __int64> dummy_ts(numE/2);


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

            dummyADC[j/2] = (double)recEvent;
            dummy_ts[j/2] = (unsigned __int64)(Time*cnv);
            Time=0;
            j+=2;
        }

        dummyADC.resize(j/2);
        dummy_ts.resize(j/2);
        qint64* packNums = new qint64[j/2];
        for (int k = 0; k < j/2; k++){
            packNums[k]=packNum;
        }

        //Dummy data:
        PostData<double>(j/2, "ADCOutput",&dummyADC[0],packNums);
        PostData<unsigned __int64>(j/2, "TS",&dummy_ts[0],packNums);


        delete [] packNums;
        //packNum keeps track of how many packets we've sent using PostData,
        packNum++;
    //}
  return 0;
}

int LynxDAQ::StopDataAcquisition(){
    cout<<"inside stop data acquisition"<<endl;
    Utilities::disableAcquisition(lynx, input);
    cout<<"disabled acq"<<endl;
    return 0;
}

void LynxDAQ::LoadDefaultConfigs(){

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
