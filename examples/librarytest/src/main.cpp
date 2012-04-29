#include <iostream>
#include <time.h>
//#include "stdafx.h"
#include "utilities.h"
#include "PrintUtilities.h"


#ifndef _WIN32_WINNT		// Allow use of features specific to Windows XP or later.
#define _WIN32_WINNT 0x0501	// Change this to the appropriate value to target other versions of Windows.
#endif


using namespace std;

int main()
{
    DevCntl::IDevicePtr lynx = Utilities::Device ();

    //Name of output file:
    string filename = "output.txt";

    try
    {
        //See page 11 of PDF for what input # means.
        long input = 1;

        //Open a connection to the device
        bstr_t lynxAddress = Utilities::getLynxAddress ();
        lynx->Open (Utilities::getLocalAddress (lynxAddress), lynxAddress);

        //Display the name of the lynx
        cout << "You are connected to: " << Utilities::GetString ((bstr_t)(lynx->GetParameter (DevCntl::Network_MachineName, (short) 0))) << "\n";

        //Gain ownership
        lynx->LockInput (_bstr_t("administrator"), _bstr_t("password"), input);

        //Disable all acquisition
        Utilities::disableAcquisition(lynx, input);

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

        //Turn on HV
        //This doesn't seem to work - need to manually turn on HV.
        variant_t voltageStatus = VARIANT_TRUE;
        lynx->PutParameter (DevCntl::Input_VoltageStatus, input, &voltageStatus);

        variant_t voltage = (long)5000;
        lynx->PutParameter(DevCntl::Input_Voltage, input, &voltage);

        //Wait till ramped
        while(VARIANT_TRUE == (VARIANT_BOOL)(variant_t)(lynx->GetParameter (DevCntl::Input_VoltageRamping, input)))
        {
            cout << "HVPS is ramping...\n";
            Sleep (500);
        }

        //Set the current memory group
        variant_t group = 1L;
        lynx->PutParameter (DevCntl::Input_CurrentGroup, input, &group);

        //Check for Windows 2000 Operating System
        BOOL bIsW2K = Utilities::IsWindows2000 ();
        if (bIsW2K)
            Utilities::UpdateForWindows2K (lynx);

        //Clear the memory
        lynx->Control (DevCntl::Clear, input, &Args);

        //Start the acquisition
        lynx->Control (DevCntl::Start, input, &Args);

        //Bump priority of this app up
        SetThreadPriority (GetCurrentThread (), THREAD_PRIORITY_HIGHEST);

        variant_t timeBase;

        //While acquiring display acquisition info
        ULONG Status=DevCntl::Waiting;

        while(((0 != (DevCntl::Busy & Status)) || (0 != (DevCntl::Waiting & Status))) &&
              !HIBYTE(GetAsyncKeyState(VK_ESCAPE)))
        {
            Status=(ULONG) lynx->GetParameter (DevCntl::Input_Status, input);

            //Get the list data
            variant_t listB = lynx->GetListData (input);

            //Display current time repeatedly: (as well as number of "flags"?)
            //For some reason, commenting out this section alters the timestamp displayed at each event... does Get1DSafeArrayElement alter listB somehow?
            /*
            if (bIsW2K)	//For Windows 2000 OS, it returns local time directly...
                cout << "Start time: "  << Utilities::GetString ((bstr_t) (Utilities::Get1DSafeArrayElement (listB, 1)));
            else		// for all other OS, it returns UTC time and we have to convert to local time...
                cout << "Start time: "  << Utilities::GetString ((bstr_t)Utilities::UTCToLocalTime (Utilities::Get1DSafeArrayElement (listB, 1)));

            cout << "; Live time (uS): " << Utilities::GetString ((bstr_t)(Utilities::Get1DSafeArrayElement (listB, 3)))
                 << "; Real time (uS): " << Utilities::GetString ((bstr_t)(Utilities::Get1DSafeArrayElement (listB, 2)))
                 << "; Timebase (nS): " << Utilities::GetString ((bstr_t)(timeBase=Utilities::Get1DSafeArrayElement (listB, 4)))
                 << "; Flags: " << Utilities::GetString ((bstr_t)(Utilities::Get1DSafeArrayElement (listB, 5)))
                 << "\n";
            */

            variant_t vMode = Utilities::Get1DSafeArrayElement (listB, 0);
            timeBase=Utilities::Get1DSafeArrayElement (listB, 4);

            //See which date was received and present it
            long inputMode = (LONG) lynx->GetParameter (DevCntl::Input_Mode, input);
            if (inputMode == DevCntl::List && !((bool) vMode))
            {
                variant_t listD = Utilities::Get1DSafeArrayElement (listB, 6);
                for (int i = 0; i < Utilities::GetCount (listD); i++)
                    cout << "Event: "  << Utilities::GetString ((bstr_t)(Utilities::Get1DSafeArrayElement (listD, i))) <<"\n";
            }
            else if (inputMode == DevCntl::Tlist && ((bool) vMode))
            {
                variant_t tlistD = Utilities::Get1DSafeArrayElement (listB, 6);
                PrintUtilities::reconstructAndOutputTlistData(tlistD, timeBase, false, filename);
            }
        }

        //Return preset mode to none.
        variant_t noPreset = variant_t ((LONG) DevCntl::PresetNone);
        lynx->PutParameter (DevCntl::Preset_Options, input, &noPreset);
    }
    CATCH_EX (lynx);
    cout << "\nProgram complete\nPress <Enter> key to exit program";
    while(!HIBYTE(GetAsyncKeyState(VK_RETURN))) {Sleep(100);}
    return 0;
    return 0;
}

