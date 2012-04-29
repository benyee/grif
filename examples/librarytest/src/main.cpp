#include <iostream>
#include <time.h>
//#include "stdafx.h"
#include "utilities.h"
#include <QLibrary>


#ifndef _WIN32_WINNT		// Allow use of features specific to Windows XP or later.
#define _WIN32_WINNT 0x0501	// Change this to the appropriate value to target other versions of Windows.
#endif


using namespace std;

int main()
{
    /*QLibrary myLib("C:/Canberra/LynxCOM_SDKInstaller/Communications");
    typedef void(*MyPrototype)();
    MyPrototype myFunction = (MyPrototype) myLib.resolve("");
    if (myFunction){
        myFunction();
    }*/
    cout << "Hello World!" << endl;
    return 0;
}

