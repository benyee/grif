#include <iostream>

#include <stdio.h>
#include <stdlib.h>
#include <QtGui>
#include <QtCore/QCoreApplication>
#include <QtCore>
#include <QLinkedList>
#include <Qt>
#include <tools/GRICLI.h>
#include <core/GRIDefines.h>
#include <config/GRILoader.h>
#include <config/GRIUserLoader.h>
#include <core/GRIMemoryManager.h>
#include <core/GRIRegulator.h>

#include "DAQControlWidget.h"
#include "SIMAnalysisThread.h"

using namespace std;

int main(int argc, char* argv[])
{
    QApplication app(argc, argv);
    GRIMemoryManager *mm = new GRIMemoryManager();
    GRIRegulator *reg = new GRIRegulator(mm);

    // the following will be included for all programs
    GRILoader *loader = new GRIUserLoader(reg);
    loader->InitRegulatorDetails();

    // get the processes
    QLinkedList<GRIProcessThread*> *p = reg->get_processes();
    // be careful not to remove anything
    LynxDAQ *lynx  = (LynxDAQ *)p->takeFirst();
    SIMAnalysisThread *AMC1 = (SIMAnalysisThread *)p->takeFirst();
    p->push_back(lynx);
    p->push_back(AMC1);

    DAQControlWidget* gui = new DAQControlWidget(0,lynx,reg);
    gui->show();

    /*
    //Included for all programs:
    reg->Start();
    GRISleep::msleep(5000);
    reg->Stop();
    GRISleep::msleep(5000);
    std::cout << "done..." << std::endl;
    */

    return app.exec();
}

