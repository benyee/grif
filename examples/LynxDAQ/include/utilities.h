/**
 * Utility class
 * 
 * @author Rashesh Patel
 *
 */

//Provided by Canberra

#include <memory>
#include <iostream>
#include <string>
#include <Winsock2.h>
#include <Iptypes.h>
#include <Iphlpapi.h>

#import "DevCntl.tlb" rename_namespace("DevCntl")

using namespace std;

#ifndef UTILITIES_HEADER__
#define UTILITIES_HEADER__

/**
 * Commom exceptions handler.
 */

#define CATCH_EX(lpLynx)\
catch(exception ex)\
{\
	cout << "Exception caught. Details: " << ex.what() << "\n";\
}\
catch(_com_error& err)\
{\
	cout << "Exception caught. " << err.Description() << "\n";\
}\
catch(long Stat)\
{\
	cout << "Exception caught. Details: " << Stat << "\n";\
}\
catch(...)\
{\
	cout << "Exception caught. Details: " << lynx->GetLastError () << "\n";\
}

class Utilities 
{
private:

	/**
	 * Do not want anyone creating an instance
	 * of this data type
	 *
	 */
	Utilities() 
	{
	}
	
	/**
	 * This method will read information from
	 * the standard input stream
	 * 
	 * @return The info
	 */
	static string readLine()
	{
		string line ("");
		cin >> line;
		return line;
	}
	
public:

	
	/**
	 * This method will return a string that describes the
	 * meaning of the various states contained in the status
	 * parameter.
	 * 
	 * @param status.  The status returned by the device
	 * @return. The description
	 */
	static bstr_t GetStatusDescription(int status) {
		bstr_t statMsg="Idle ";
		if (0 != (status&DevCntl::Busy)) statMsg="Busy ";
		if (0 != (status&DevCntl::APZinprog)) statMsg+="APZ ";
		if (0 != (status&DevCntl::Diagnosing)) statMsg+="Diagnosing ";
		if (0 != (status&DevCntl::ExternalTriggerEvent)) statMsg+="Ext trig ";
		if (0 != (status&DevCntl::Fault)) statMsg+="Fault ";
		if (0 != (status&DevCntl::GroupComplete)) statMsg+="Group complete ";
		if (0 != (status&DevCntl::HVramping)) statMsg+="HVPS ramping ";
		if (0 != (status&DevCntl::Idle)) statMsg+="Idle ";
		if (0 != (status&DevCntl::PresetCompReached)) statMsg+="Comp Preset reached ";
		if (0 != (status&DevCntl::PresetTimeReached)) statMsg+="Time Preset reached ";
		if (0 != (status&DevCntl::PresetSweepsReached)) statMsg+="Sweeps Preset reached ";
		if (0 != (status&DevCntl::Rebooting)) statMsg+="Rebooting ";
		if (0 != (status&DevCntl::UpdatingImage)) statMsg+="Updating firmware ";
		if (0 != (status&DevCntl::Waiting)) statMsg+="Waiting ";
		if (0 != (status&DevCntl::AcqNotStarted)) statMsg+="Acquisition not started because preset already reached ";
		if (0 != (status&DevCntl::OverflowStop)) statMsg+="Acquisition stopped because channel contents overflowed ";
		if (0 != (status&DevCntl::ExternalStop)) statMsg+="Acquisition stopped because of external stop ";
		if (0 != (status&DevCntl::ManualStop)) statMsg+="Acquisition stopped because of manual stop ";
		return statMsg;
	}

	/**
	 * This method will read information from
	 * the standard input stream
	 * 
	 * @param text. The text to display to the user on the console
	 * @return.  The text entered by the user
	 */
	static bstr_t readLine(string text)
	{
		cout << text;
		string line ("");
		cin >> line;
		return bstr_t(line.data());
	}

	/**
	 * This method will return the ip address of the local
	 * network adapter
	 * @param-dest.  The destination/lynx address.
	 * @return  The value
	 */
	static _bstr_t getLocalAddress(_bstr_t dest) 
	{
		string localAddr ("");
		string destAddr (dest);
		
		try 
		{
			ULONG InfSize=0;
			LONG Stat = GetAdaptersInfo(NULL, &InfSize);
			if (ERROR_BUFFER_OVERFLOW != Stat) throw Stat;		

			std::auto_ptr<char> Inf(new char[InfSize]);
			memset((char *) Inf.get(), 0, InfSize);

			//Get a list of all available network adapters
			Stat = GetAdaptersInfo(reinterpret_cast<IP_ADAPTER_INFO *>(Inf.get()), &InfSize);
			if (NO_ERROR != Stat) throw Stat;
			
			DWORD Index = 0;
			Stat=GetBestInterface(inet_addr(destAddr.c_str()), &Index);
			if (NO_ERROR != Stat) {throw Stat;}

			IP_ADAPTER_INFO *Info = reinterpret_cast<IP_ADAPTER_INFO *>(Inf.get());
			while(Info) {				
				if (Index == Info->Index) {
					localAddr = Info->IpAddressList.IpAddress.String;
					break;
				}					
				Info = Info->Next;
			}
		}
		catch(LONG lastError)
		{		
			cout << "getLocalAddress - Error#" << lastError;
		}
		catch(...)
		{
			long lastError = WSAGetLastError();
			cout << "getLocalAddress - Error#" << lastError;
		}

		//convert to bstr_t
		_bstr_t bstrlocalAddr (localAddr.c_str());
		return bstrlocalAddr;
	}

	/**
	 * This method will return the ip address of the lynx
	 * that has been entered via the C++ console
	 * 
	 * @return  The value
	 */
	static _bstr_t getLynxAddress() 
	{
		string lynxAddr ("");
		bool error = false;
		struct hostent* remoteHost;
		
		try 
		{
			do 
			{
				error = false;
				cout << "Enter the IP address of your Lynx: (a.b.c.d): ";
				lynxAddr = readLine();

				try 
				{
					//Initialize Socket
					WSADATA wsaData;
					if (WSAStartup(MAKEWORD(1, 1), &wsaData) != 0) 
						throw WSAGetLastError();

					//If the user input is an alpha name for the host, use gethostbyname()
					//If not, get host by addr (assume IPv4)
					if (isalpha (lynxAddr [0])) // host address is a name 
					{
						//if hostname terminated with newline '\n', remove and zero-terminate 
						if (lynxAddr [lynxAddr.length () - 1] == '\n') 
							lynxAddr [lynxAddr.length () - 1] = '\0'; 
						remoteHost = gethostbyname (lynxAddr.c_str ());
					}
					else  
					{
						unsigned int addr = inet_addr (lynxAddr.c_str ());
						remoteHost = gethostbyaddr ((char *)&addr, 4, AF_INET);

						//It is not catestrophic for this case so punt and
						//return the IP address
						if (NULL == remoteHost) {
							_bstr_t bstrLynxAddr (lynxAddr.c_str());
							return bstrLynxAddr;
						}
					}

					//check for errors
					if (WSAGetLastError() != 0) 
						throw WSAGetLastError();
					else
					{
						in_addr * address = (in_addr *) remoteHost->h_addr;
						lynxAddr = inet_ntoa (*address);
					}
				}
				catch(...)
				{
					throw;
				}
			} while(error);
		}
		catch(...)
		{
			long lastError = WSAGetLastError();
			cout << "getLynxAddress - Error#" << lastError;
		}

		//convert to bstr_t
		_bstr_t bstrLynxAddr (lynxAddr.c_str());
		return bstrLynxAddr;
	}
	
	/**
	 * This method will return the spectral acquisition mode
	 * that has been entered by the java console
	 * @return.  The value
	 */
	static int getSpectralMode () 
	{
		int mode = DevCntl::Pha;
		bool error = false;

		try 
		{
			do 
			{
				error = false;
				cout << "Select the acquisition mode: (0=Pha, 1=Dlfc) ";
				mode = atoi (readLine().c_str ());
				if ((0 != mode) && (1 != mode))
				{
					error=true;
				}
				else 
				{
					if (1 == mode) 
						mode = DevCntl::Dlfc;
					else 
						mode = DevCntl::Pha;
				}
			} while(error);
		}
		catch(...) 
		{
			cout << "getSpectralMode - Invalid input.";
		}
		return mode;
	}
	
	/**
	 * This method will return the list mode to
	 * use to acquire data.  The list mode is specified
	 * via the java console
	 * 
	 * @return. The value
	 */
	static int getListMode () 
	{
		int mode = DevCntl::Pha;
		bool error = false;
		
		try 
		{
			do 
			{
				error = false;
				cout << "Select the acquisition mode: (0=List, 1=Tlist) ";
				mode = atoi (readLine ().c_str ());
				if ((0 != mode) && (1 != mode))
				{
					error = true;
				}
				else 
				{
					if (1 == mode) 
						mode = DevCntl::Tlist;
					else 
						mode = DevCntl::List;
				}
			} while (error);
		}
		catch(...) 
		{
			cout << "getListMode - Invalid input.";
		}
		return mode;
	}
	
	/**
	 * This method will return the preset mode
	 * that has been entered by the java console
	 * @return.  The value
	 */
	static int getPresetMode () 
	{
		int mode = DevCntl::PresetNone;
		bool error = false;
		
		try 
		{
			do 
			{
				error = false;
				cout << "Select the acquisition mode: (0=None, 1=Real, 2=Live) ";
				mode = atoi (readLine ().c_str ());
				if ((0 != mode) && (1 != mode) && (2 != mode))
				{
					error = true;
				}
				else 
				{
					switch(mode) 
					{
					case 0:
						mode = DevCntl::PresetNone;
						break;
					case 1:
						mode = DevCntl::PresetRealTime;
						break;
					default:
						mode = DevCntl::PresetLiveTime;
						break;
					}
				}
			} while (error);
		}
		catch(...) 
		{
			cout << "getPresetMode - Invalid input.";
		}
		return mode;
	}
	
	/**
	 * This method will return the MCS preset mode
	 * that has been entered by the java console
	 * @return.  The value
	 */
	static int getMCSPresetMode() 
	{
		int mode = DevCntl::PresetNone;
		bool error = false;
		
		try
		{
			do 
			{
				error = false;
				cout << "Select the acquisition mode: (0=None, 1=Sweeps) ";
				mode = atoi (readLine ().c_str ());
				if ((0 != mode) && (1 != mode))
				{
					error = true;
				}
				else 
				{
					switch(mode) 
					{
					case 0:
						mode = DevCntl::PresetNone;
						break;
					default:
						mode = DevCntl::PresetSweeps;
						break;
					}
				}
			} while (error);
		}
		catch(...) 
		{
			cout << "getMCSPresetMode - Invalid input.";
		}
		return mode;
	}

	/**
	 * This method will return the a floating point value
	 * that has been entered by the java console
	 * @return.  The value
	 */
	static float getFloat (string text, float min, float max) 
	{
		float val=0.0f;
		bool error = false;
		
		try 
		{
			do 
			{
				error = false;
				cout << text;
				val = (float) atof (readLine ().c_str ());
				if ((val < min) || (val > max)) 
				{
					error = true;
				}
			} while (error);
		}
		catch(...) 
		{
			cout << "getFloat - Invalid input.";
		}
		return val;
	}
	
	/**
	 * This method will return the a integer value
	 * that has been entered by the java console
	 * @return.  The value
	 */
	static int getInt (string text, int min, int max) 
	{
		int val=0;
		bool error = false;
		
		try 
		{
			do 
			{
				error = false;
				cout << text;
				val = atoi (readLine ().c_str ());
				if ((val < min) || (val > max)) 
				{
					error = true;
				}
			} while(error);
		}
		catch(...) 
		{
			cout << "getInt - Invalid input.";
		}
		return val;
	}

	/**
	 * This method will create Device pointer.
	 * 
	 * @Device Pointer
	 */
	static DevCntl::IDevicePtr Device ()
	{
		try
        {
            DevCntl::IDevicePtr pLynx;
			while (1)
			{
				//Create instince of Device pointer
                HRESULT hr = pLynx.CreateInstance (__uuidof(DevCntl::Device));
                if (FAILED(hr))
                {
                    if (hr != 0x800401f0)
						throw hr;
					else
					{
						//Init COM Library...
						if (FAILED (hr = CoInitialize (NULL)))
							throw hr;
					}
				}
                else
					break;
			}
			return pLynx;
		}
		catch (...)
		{
			cout << "GetDevice - Failed to create Lynx interface.\n";
		}
		return NULL;
	}

	
	/**
	 * This method will create DeviceEx pointer.
	 * 
	 * @DeviceEx Pointer
	 */
	static DevCntl::IDeviceExPtr DeviceEx ()
	{
		try
		{
			DevCntl::IDeviceExPtr pLynx;
			while (1)
			{
				//Create instince of Device pointer
				HRESULT hr = pLynx.CreateInstance (__uuidof(DevCntl::DeviceEx));
				if (FAILED(hr))
				{
					if (hr != 0x800401f0)
						throw hr;
					else
					{
						//Init COM Library...
						if (FAILED (hr = CoInitialize (NULL)))
							throw hr;
					}
				}
				else
					break;
			}
			return pLynx;
		}
		catch (...)
		{
			cout << "DeviceEx - Failed to create Lynx interface.\n";
		}
		return NULL;
	}

	/**
	 * This method will convert bstr to string
	 * 
	 * @string
	 */
	static string GetString (bstr_t bstrString)
	{
		string strString (bstrString);
		return strString;
	}

	/**
	 * This method will create a safe array of the list of codes provided.
	 * 
	 * @VARIANT containing safe arrray
	 */
	static _variant_t Get1DSafeArray (long codeList [], int maxCount)
	{
		HRESULT hRes = S_OK;
		SAFEARRAY *psfArr=NULL;
		VARIANT vVal;
		SAFEARRAYBOUND psfBounds[1];

		try
		{
			//Init
			VariantInit(&vVal);

			//Setup bounds on safe array for output vVal
			psfBounds[0].cElements = maxCount;
			psfBounds[0].lLbound = 0;

			//Create safe array to store parameters
			psfArr = SafeArrayCreate(VT_I4, 1, psfBounds);
			if(psfArr == NULL) 
				throw E_OUTOFMEMORY;

			//Lock data
			hRes = SafeArrayLock(psfArr);
			if (FAILED(hRes)) 
				throw hRes;

			long plIndex[1];
			long code = 0L;

			//Transfer data to safe array
			for (int i = 0; i < (long) maxCount; i++) 
			{
				//is valid code??
				if (codeList [i] <= 0)
					continue;

				plIndex[0] = i;
				code = codeList [i];
				hRes = SafeArrayPutElement (psfArr, plIndex, (void *) &code);
				if (FAILED(hRes)) 
					throw hRes;
			}
		}
		catch (...)
		{
			cout << "Get1DSafeArray error.\n";
		}

		if (psfArr) 
		{
			SafeArrayUnlock(psfArr);
			vVal.vt = VT_ARRAY | VT_I4;
			vVal.parray = psfArr;
		}
		_variant_t v=_variant_t(vVal);
		VariantClear(&vVal);
		return v;
	}

	/**
	 * This method will create a safe array of the list of floats
	 * 
	 * @VARIANT containing safe arrray
	 */
	static _variant_t Get1DSafeArray (float List [], int maxCount)
	{
		HRESULT hRes = S_OK;
		SAFEARRAY *psfArr=NULL;
		VARIANT vVal;
		SAFEARRAYBOUND psfBounds[1];

		try
		{
			//Init
			VariantInit(&vVal);

			//Setup bounds on safe array for output vVal
			psfBounds[0].cElements = maxCount;
			psfBounds[0].lLbound = 0;

			//Create safe array to store parameters
			psfArr = SafeArrayCreate(VT_R4, 1, psfBounds);
			if(psfArr == NULL) 
				throw E_OUTOFMEMORY;

			//Lock data
			hRes = SafeArrayLock(psfArr);
			if (FAILED(hRes)) 
				throw hRes;

			long plIndex[1];
			float v = 0.0F;

			//Transfer data to safe array
			for (int i = 0; i < (long) maxCount; i++) 
			{
				
				plIndex[0] = i;
				v = List [i];
				hRes = SafeArrayPutElement (psfArr, plIndex, (void *) &v);
				if (FAILED(hRes)) 
					throw hRes;
			}
		}
		catch (...)
		{
			cout << "Get1DSafeArray error.\n";
		}

		if (psfArr) 
		{
			SafeArrayUnlock(psfArr);
			vVal.vt = VT_ARRAY | VT_R4;
			vVal.parray = psfArr;
		}
		_variant_t v=_variant_t(vVal);
		VariantClear(&vVal);
		return v;
	}

	/**
	 * This method will get specified index value from supplied safe array.
	 * 
	 * @variant_t element at given location from safe array
	 */
	static int GetCount (variant_t &vVal)
	{
		if (((vVal.vt & VT_SAFEARRAY) != VT_SAFEARRAY) && ((vVal.vt & VT_ARRAY) != VT_ARRAY))
			return 0;

		return vVal.parray->rgsabound[0].cElements;
	}

	/**
	 * This method will get specified index value from supplied safe array.
	 * 
	 * @variant_t element at given location from safe array
	 */
	static _variant_t Get1DSafeArrayElement (variant_t &vVal, int index)
	{
		HRESULT hResult = S_OK;
		VARIANT vValue;
		VariantInit(&vValue);

		long plIndex[1];
		try
		{
			if (index < GetCount (vVal))
			{
				plIndex [0] = index;
				if ((vVal.vt & VT_I2) == VT_I2)
				{
					hResult = SafeArrayGetElement (vVal.parray, plIndex, &(vValue.iVal));
					vValue.vt = VT_I2;
				}
				else if ((vVal.vt & VT_I4) == VT_I4)
				{
					hResult = SafeArrayGetElement (vVal.parray, plIndex, &(vValue.lVal));
					vValue.vt = VT_I4;
				}
				else
					hResult = SafeArrayGetElement (vVal.parray, plIndex, &vValue);

				if(FAILED(hResult))
					throw hResult;
			}
		}
		catch (...)
		{
			cout << "Get1DSafeArrayElement error\n";
		}
		_variant_t v=_variant_t(vValue);
		VariantClear(&vValue);
		return v;
	}

	/**
	 * This method will convert UTC time to local time.
	 * 
	 * @local time in variant_t
	 */
	static VARIANT UTCToLocalTime (VARIANT& vtTime)
	{
		//Clear the Now systemtime structure
		SYSTEMTIME SystemTime, Now;
		memset (&SystemTime, 0, sizeof (SystemTime));
		memset (&Now, 0, sizeof (Now));

		//Get the timezone infomation structure.
		TIME_ZONE_INFORMATION Zone;
		DWORD a = GetTimeZoneInformation (&Zone);

		//Convert the Device variant to a systemtime structure
		if (vtTime.vt != VT_DATE)
		{
			__int64 T = vtTime.llVal;
			FILETIME UtcTime = *(reinterpret_cast<FILETIME *>(&T));
			FileTimeToSystemTime (&UtcTime, &SystemTime);
		}
		else
			VariantTimeToSystemTime (vtTime.dblVal, &SystemTime);

		//Using the Device systemtime and the current time zone information
		//Calculate the current local time.
		SystemTimeToTzSpecificLocalTime (&Zone, &SystemTime, &Now);

		// Convert the current local time to a variant
		double dTemp = 0.0;
		SystemTimeToVariantTime(&Now, &dTemp);

		VARIANT v;
		VariantInit (&v);
		v.date = dTemp;
		v.vt = VT_DATE;
		return v;
	}

	/**
	 * This method will check if current operating system is Windows 2000 system or not.
	 *
	 * @TRUE if Windows 2000 Opearing System else FALSE
	 */
	static BOOL IsWindows2000 ()
	{
		BOOL bIsW2K = FALSE;
		OSVERSIONINFO VersionInfo;
		ZeroMemory (&VersionInfo, sizeof (OSVERSIONINFO));
		VersionInfo.dwOSVersionInfoSize = sizeof (OSVERSIONINFO);
		if (GetVersionEx (&VersionInfo) == TRUE)
		{
			//is this operating system Windows 2000??
			if (VersionInfo.dwMajorVersion == 5 && VersionInfo.dwMinorVersion == 0)
				bIsW2K = TRUE;
		}

		return bIsW2K;
	}

	/**
	 * This method will set appropriate flags to get
	 * correct data (specially for VT_I8 data types) from the lynx device.
	 * Note: By setting this flag to get Time, we will loose 
	 *		 milli second precision. 
	 *		(Please refer to Communication Libraries document for more information)
	 * @
	 */
	static void UpdateForWindows2K (DevCntl::IDevicePtr lynx)
	{
		VARIANT vWin2K;
		VariantInit (&vWin2K);
		vWin2K.boolVal = VARIANT_TRUE;
		vWin2K.vt = VT_BOOL;
		lynx->PutProperty (_bstr_t ("ConvertFileTime2OleTime"), &vWin2K);
	}

	/**
	 * This method will reconstruct the time events for time
	 * stamped list mode before displaying on output
	 * @param tlistD.	The time stamped list data buffer
	 * @param timeBase.  The timebase (nS)
	 * @param clear. Whether to reset the time counter
	 */
	static void reconstructAndOutputTlistData(variant_t &tlistD, int timeBase, bool clear)
	{
		static const long ROLLOVERBIT=0x00008000;		
		static unsigned __int64 RolloverTime=0;
		if (clear) RolloverTime=0;
		LONG numE = Utilities::GetCount (tlistD);
		unsigned short recTime=0, recEvent=0;
		unsigned __int64 Time=0;
		double cnv=timeBase;
		LONG i=0;

		cnv /= 1000; //Convert to uS
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

			cout << "\nEvent: " << recEvent << "; Time (uS): " << Time*cnv << endl;			
			Time=0;
		}
	}
	/**
	 * This method will stop all forms of data acquisition which includes
     *  any of the following:
     *       -) SCA collection
     *       -) Auxiliary counter collection
     *       -) PHA
     *       -) MCS
     *       -) MSS
     *       -) DLFC
     *       -) List
     *       -) Tlist
	 * @param lynx.  The device interface
	 * @param input. The input
	 */
	static void disableAcquisition(DevCntl::IDevicePtr &lynx, LONG input)
	{
		_bstr_t user(L"administrator");
		_bstr_t pwd(L"password");
		
		VARIANT args;
		VariantInit (&args);
		try {
			//Make sure we own the input
			lynx->LockInput(user, pwd, input);
		}
		catch(...) {}
		
		try {
			//Stop acquisition
			lynx->Control(DevCntl::Stop, input, &args);
		}
		catch(...) {}

		try {
			//Abort acquisition (only needed for MSS or MCS collections)
			lynx->Control(DevCntl::Abort, input, &args);
		}
		catch(...) {}
		
		_variant_t disable((int) 0);
		try {
			//Stop SCA collection
			lynx->PutParameter(DevCntl::Input_SCAstatus, input, &disable);
		}
		catch(...) {}
		
		try {
			//Stop aux counters collection
			lynx->PutParameter(DevCntl::Counter_Status, input, &disable);
		}
		catch(...) {}
	}
	/**
	 * This method will stop all forms of data acquisition which includes
     *  any of the following:
     *       -) SCA collection
     *       -) Auxiliary counter collection
     *       -) PHA
     *       -) MCS
     *       -) MSS
     *       -) DLFC
     *       -) List
     *       -) Tlist
	 * @param lynx.  The device interface
	 * @param input. The input
	 */
	static void disableAcquisition(DevCntl::IDeviceExPtr &lynx, LONG input)
	{
		_bstr_t user(L"administrator");
		_bstr_t pwd(L"password");
		
		VARIANT args;
		VariantInit (&args);
		try {
			//Make sure we own the input
			lynx->LockInput(user, pwd, input);
		}
		catch(...) {}
		
		try {
			//Stop acquisition
			lynx->Control(DevCntl::Stop, input, &args);
		}
		catch(...) {}
		
		try {
			//Abort acquisition (only needed for MSS or MCS collections)
			lynx->Control(DevCntl::Abort, input, &args);
		}
		catch(...) {}
		
		_variant_t disable((int) 0);
		try {
			//Stop SCA collection
			lynx->PutParameter(DevCntl::Input_SCAstatus, input, &disable);
		}
		catch(...) {}
		
		try {
			//Stop aux counters collection
			lynx->PutParameter(DevCntl::Counter_Status, input, &disable);
		}
		catch(...) {}
	}
};

#endif //UTILITIES_HEADER__
