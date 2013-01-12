#include "LynxDAQ.h"

LynxDAQ::LynxDAQ() {
    ref_time_ = QDateTime(QDate(2012,5,1));  //May 1st, 2012 12:00am

    lynx = Utilities::Device();
    input = 1;
    VariantInit (&Args);
    isConnected = false;

    currLiveTime = 0;
    currRealTime = 0;

    simMode = false;
    simModeTime = 0;
    isSimulating = 0;
}

LynxDAQ::~LynxDAQ() {
}


GRIDAQBaseAccumNode* LynxDAQ::RegisterDataOutput(QString outName) {
    //Initialize accumulators.
    //Need to make sure the variable types match the variable types that you post with PostData
  GRIDAQBaseAccumNode* p = NULL;
  if (outName == "ADCOutput") {
    p = new GRIDAQAccumulator<double>(outName,1e6,5,250);
  } else if (outName == "TS") {
    p = new GRIDAQAccumulator<double>(outName,1e6,5,250);
  }
  return p;
}

int LynxDAQ::ConnectToDAQ(){//Open a connection to the device
    if(simMode){
        isConnected = true;
        currLiveTime = LiveTime();
        currRealTime = RealTime();
        return 0;
    }

    prevTime = 0;
    numRollovers = 0;
    try{
        if(isConnected){ //If already connected, recreate a new instance of lynx.
            //Release ownership
            lynx->UnlockInput (_bstr_t("administrator"), _bstr_t("password"), input);
            cout<<"Released ownership."<<endl;

            lynx = Utilities::Device();
            isConnected=false;
        }
        bstr_t lynxAddress;
        if (LYNX_DEFAULT){lynxAddress = LYNX_IPADDRESS;}
        else{lynxAddress = Utilities::getLynxAddress ();} //Ask for Lynx address if not on default.
        lynx->Open(Utilities::getLocalAddress (lynxAddress), lynxAddress);

        //Display the name of the lynx
        cout << "You are connected to: " << Utilities::GetString ((bstr_t)(lynx->GetParameter (DevCntl::Network_MachineName, (short) 0))) << "\n";
        isConnected = true;

        //Gain ownership
        lynx->LockInput (_bstr_t("administrator"), _bstr_t("password"), input);
        cout<<"Gained ownership."<<endl;

        //Update current times:
        currLiveTime = LiveTime();
        currRealTime = RealTime();
    }catch(int e){
    }
    return 0;
}

int LynxDAQ::LoadConfiguration(){
    if(LYNX_DEFAULT && !simMode){LoadDefaultConfigs();}

    //The code below was supplied by Lynx to manually set the configuration.
    //  It has not been tested with the rest of this DAQ thread.

    /*
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
    */
    return 0;
}

int LynxDAQ::Initialize(){
    if(simMode){
        return 0;
    }
    cout<<"Initializing..."<<endl;

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
    cout<<"Starting Data Acquisition"<<endl;
  start_time_ = QDateTime::currentDateTime();
  dt = ref_time_.msecsTo(start_time_);
  InitializeAccumulators(start_time_,0);

  if(!simMode){
      //Clear the memory and start the acquisition
      lynx->Control (DevCntl::Clear, input, &Args);
      lynx->Control (DevCntl::Start, input, &Args);
  }else{isSimulating = 1;}

  return 0;
}

int LynxDAQ::AcquireData(int n) {
    //cout<<"Acquiring Data..."<<endl;


    if(simMode && isSimulating){
        //We will generate a numData amount of random data points.
        int numData = floor((double)rand()/RAND_MAX*8);
        vector<double> ADC;
        vector<double> ts_sec;
        vector<qint64> ts;
        qint64 Time = 0;

        for(int i = 0; i<numData;i++){
            //Generate a random event, with a timestamp a random time ahead of the previous one
            ADC.push_back(ceil((double)rand()/RAND_MAX*8192));
            if(i == 0){
                Time = ceil((double)rand()/RAND_MAX*1e4)+simModeTime;
            }else{
                Time = ceil((double)rand()/RAND_MAX*1e4)+Time;
            }
            ts.push_back(-QDateTime::currentDateTime().secsTo(start_time_)*1e6);
            ts_sec.push_back(Time/1e6);

            if(i==numData-1){
                simModeTime = Time;
            }
        }

        std::cout<<simModeTime<<endl;
        PostData<double>(numData, "ADCOutput",&ADC[0],&ts[0]);
        PostData<double>(numData, "TS",&ts_sec[0],&ts[0]);
        //cout<<"Posting simulated data..."<<endl;


        return 0;
    }else if(simMode){
        return 0;
    }
    //Get the list data from lynx
    variant_t listB = lynx->GetListData (input);

    //Set the proper time base:
    timeBase=Utilities::Get1DSafeArrayElement (listB, 4);
    double cnv = (double)timeBase/1000; //Convert to uS

    //Get Event/Time list:
    variant_t tlistD = Utilities::Get1DSafeArrayElement (listB, 6);

    //This gives you the live time associated with the packet...
    //variant_t tlistE = Utilities::Get1DSafeArrayElement(listB,3);
    //std::cout<<(int)tlistE<<"HELLOOOO"<<std::endl;

    //# of elements in tlistD (note that this is NOT the # of events due to rollover bits)
    LONG numE = Utilities::GetCount (tlistD);

    static const long ROLLOVERBIT=0x00008000;
    static unsigned __int64 RolloverTime=0;
    unsigned short recTime=0, recEvent=0;
    unsigned __int64 Time=0;
    LONG i=0;

    vector<double> ADC;
    vector<double> ts_sec;
    vector<qint64> ts;


    for(Time=0, i=0; i<numE; i+=2) {
        recEvent = Utilities::Get1DSafeArrayElement (tlistD, i);
        recTime = Utilities::Get1DSafeArrayElement (tlistD, i + 1);

        //Deal with the fact that some timestamps may be sent in two entries due to excess length:
        if (!(recTime&ROLLOVERBIT)) {
            Time = RolloverTime | (recTime & 0x7FFF);
        }
        else {
            unsigned long LSBofTC = 0;
            unsigned long MSBofTC = 0;
            LSBofTC |= (recTime & 0x7FFF) << 15;
            MSBofTC |= recEvent << 30;
            RolloverTime = MSBofTC | LSBofTC;

            //go to next event
            continue;
        }

        //Store the data to be posted:
        ADC.push_back((double)recEvent);

        //Pass along timestamp for GRIF as the current acquisition time:
        ts.push_back(start_time_.secsTo(QDateTime::currentDateTime())*1e6);
        //ts.push_back((qint64)(Time*cnv));

        //Check if clock has reset (Note that the -1e8 is there just in case there's a small blip in Time):
        if(Time < min(prevTime,prevTime-1e8)){numRollovers++; cout<<Time<<'\t'<<prevTime<<'\t'<<numRollovers<<endl;}
        prevTime = Time;
        //Clock resets every 2e32/1e7 seconds
        //Edit the time stamp so that it's in seconds relative to ref_time
        ts_sec.push_back((double)(Time*cnv)/1e6+(double)dt/1000+numRollovers*pow(2.0,32)/1.0e7);

        //Reset the timestamp and repeat
        Time=0;
    }

    PostData<double>(ADC.size(), "ADCOutput",&ADC[0],&ts[0]);
    PostData<double>(ADC.size(), "TS",&ts_sec[0],&ts[0]);

    //Update the real/live times - this is important for checking if the system is in acquisition.
    currRealTime = RealTime();
    currLiveTime = LiveTime();

    return 0;
}

int LynxDAQ::StopDataAcquisition(){
    if(simMode){
        isSimulating = 0;
        return 0;
    }

    //cout<<"About to disable acquisition..."<<endl;
    Utilities::disableAcquisition(lynx, input);
    cout<<"Disabled Acquisition"<<endl;

    //Update the real/live times - this is important for checking if the systme is in acquisition.
    currRealTime = RealTime();
    currLiveTime = LiveTime();

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
    if(simMode){
        return 0;
    }
    else if(VARIANT_TRUE == (VARIANT_BOOL)(variant_t)(lynx->GetParameter (DevCntl::Input_VoltageRamping, input))){
        return 2;
    }
    else if(VARIANT_TRUE == (VARIANT_BOOL)(variant_t)(lynx ->GetParameter(DevCntl::Input_VoltageStatus,input))){
        return 1;
    }
    return 0;
}

double LynxDAQ::HV(){
    if(simMode){return 0;}
    double volt = lynx->GetParameter(DevCntl::Input_VoltageReading, input);
    return volt;
}



