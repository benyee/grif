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

#include <hist/GRIHistWidget.h>
#include <hist/GRIHist1DWidget.h>

#include "AnalysisWidget.h"
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

    DAQControlWidget* controlGUI = new DAQControlWidget(0,lynx,AMC1,reg);
    controlGUI->show();


    QMainWindow *win1 = new QMainWindow();
    GRIHist1DWidget *histDraw1 = new GRIHist1DWidget(win1);
    win1->setCentralWidget(histDraw1);
    histDraw1->set_hist(AMC1->GetHistogram("Histogram1"));
    histDraw1->Initialize();
    histDraw1->set_foreground_color(Qt::cyan);
    histDraw1->set_background_color(Qt::darkBlue);
    histDraw1->set_outline_color(Qt::cyan);
    histDraw1->set_xlabel("Channel");
    histDraw1->set_ylabel("Counts");
    win1->resize(450,300);
    win1->show();

    QMainWindow *win2 = new QMainWindow();
    GRIHist1DWidget *histDraw2 = new GRIHist1DWidget(win2);
    win2->setCentralWidget(histDraw2);
    histDraw2->set_hist(AMC1->GetHistogram("Histogram2"));
    histDraw2->Initialize();
    histDraw2->set_foreground_color(Qt::cyan);
    histDraw2->set_background_color(Qt::darkBlue);
    histDraw2->set_outline_color(Qt::cyan);
    histDraw2->set_xlabel("Channel");
    histDraw2->set_ylabel("Counts");
    win2->resize(450,300);
    win2->show();

    //Ignore for now:
//    AnalysisWidget* analysisGUI = new AnalysisWidget(0,lynx,AMC1,reg);
//    analysisGUI->show();

    /*
    //Included for all programs:
    reg->Start();
    GRISleep::msleep(5000);
    reg->Stop();
    GRISleep::msleep(5000);
    std::cout << "done..." << std::endl;
    */


//    // Set up some test GUI windows
//    QMainWindow *win1 = new QMainWindow();
//    GRIHist1DWidget *histDraw1 = new GRIHist1DWidget(win1);
//    win1->setCentralWidget(histDraw1);
//    histDraw1->setWindowTitle("ADC Channel 0");
//    histDraw1->set_hist(AMC1->GetHistogram("ADC Channel 0"));
//    histDraw1->Initialize();
//    histDraw1->set_foreground_color(Qt::cyan);
//    histDraw1->set_background_color(Qt::darkBlue);
//    histDraw1->set_outline_color(Qt::cyan);
//    histDraw1->set_xlabel("Channel");
//    histDraw1->set_ylabel("Counts");
//    win1->setWindowTitle("1D Histogram");
//    win1->resize(450,300);
//    win1->show();

    return app.exec();
}

