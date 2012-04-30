#include <iostream>

#include <stdio.h>
#include <stdlib.h>
#include <QtGui/QApplication>
#include <QtCore/QCoreApplication>
#include <QtGui/QtGui>
#include <QtCore>
#include <QLinkedList>
#include <Qt>
#include "GRICLI.h"
#include "GRIDefines.h"
#include "GRILoader.h"
#include "GRIUserLoader.h"
#include "GRIMemoryManager.h"
#include "GRIRegulator.h"

using namespace std;

int main(int argc, char* argv[])
{
    QCoreApplication app(argc, argv);
    GRIMemoryManager *mm = new GRIMemoryManager();
    GRIRegulator *reg = new GRIRegulator(mm);

    // the following will be included for all programs
    GRILoader *loader = new GRIUserLoader(reg);
    loader->InitRegulatorDetails();
    reg->Start();
    GRISleep::msleep(5000);
    reg->Stop();
    GRISleep::msleep(10000);
    std::cout << "done..." << std::endl;

    cout << "Hello World!" << endl;
    return 0;
}

