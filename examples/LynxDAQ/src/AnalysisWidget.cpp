
#include <QPalette>
#include <QInputDialog>

#include "AnalysisWidget.h"
#include "ui_AnalysisWidget.h"

#include <config/GRILoader.h>
#include <config/GRIUserLoader.h>
#include <core/GRIDefines.h>
#include <core/GRIMemoryManager.h>
#include <core/GRIRegulator.h>
#include <hist/GRIHistWidget.h>
#include <hist/GRIHist1DWidget.h>
#include <hist/GRIHist2DWidget.h>
#include <hist/GRIHist1DGridWidget.h>
#include <hist/GRIHist1DGridScrollWidget.h>

#include <iomanip>
#include <iostream>
#include <fstream>

AnalysisWidget::AnalysisWidget(QWidget *parent,  LynxDAQ *daq,SIMAnalysisThread *sat, GRIRegulator *r) :
  QWidget(parent),
  ui_(new Ui::AnalysisWidget) {
  ui_->setupUi(this);

  //Retrieve DAQ, analysis thread, and regulator pointers:
  daq_thread_ = daq;
  an_thread_ = sat;
  reg = r;
  reg_started_ = false;

  // set up a timer to update widget
  update_timer_ = new QTimer();
  update_timer_->setInterval(500);
  update_timer_->start();

  //Update the page to make sure everything's good to go:
  Update();

  //start the signal and slot pairs...
  connect(update_timer_, SIGNAL(timeout()), this, SLOT(Update()));
  connect(ui_->updateHist, SIGNAL(clicked()), this, SLOT(CreateHistogram()));
  connect(ui_->useCurrentTime, SIGNAL(clicked()), this, SLOT(SetEndTimeToCurrent()));

  //Disable equation editing:
  ui_->c0->setEnabled(false);
  ui_->c1->setEnabled(false);
  ui_->c2->setEnabled(false);
}


AnalysisWidget::~AnalysisWidget() {
  delete ui_;
  delete update_timer_;
  delete an_thread_;
  delete reg;
}


void AnalysisWidget::Update() {
    if (ui_->convertButton->isChecked()){
        ui_->c0->setEnabled(true);
        ui_->c1->setEnabled(true);
        ui_->c2->setEnabled(true);
    }else{
        ui_->c0->setEnabled(false);
        ui_->c1->setEnabled(false);
        ui_->c2->setEnabled(false);
    }

}

void AnalysisWidget::SetEndTimeToCurrent(){
    ui_->endTime->setDateTime(QDateTime::currentDateTime());
}

void AnalysisWidget::CreateHistogram(){
    //Get the number of channels:
    int numChans;
    //If not connected, user must manually input the number of channels:
    if(!(daq_thread_->IsConnected())){
        bool canContinue = false;
            while (!canContinue){
            bool ok;
            QString text = QInputDialog::getText(0,"Number of Bins Required",
                "Enter the number of bins or hit cancel and connect to the detector first:",
                                                 QLineEdit::Normal,QString::null, &ok);
            canContinue = true;
            if(ok && (text.toInt() > 100000 || text.toInt() <= 0)){
                canContinue = false;
            }
            else if( ok && !text.isEmpty()) {
                numChans = text.toInt();
                std::cout<<numChans<<endl;
            } else {
                return;
            }
        }
    }else{
        numChans = (int)(daq_thread_->getNumberofChannels());
    }

    //Create/clear histogram:
    QString histname = "Hist";
    if(an_thread_->CreateNewHistogram(histname,numChans,0,numChans)==0){
    }else{
        an_thread_->ClearHistogram(histname);
    }


    // Set up GUI window
    QMainWindow *win1 = new QMainWindow();
    GRIHist1DWidget *histDraw1 = new GRIHist1DWidget(win1);
    win1->setCentralWidget(histDraw1);
    histDraw1->set_hist(an_thread_->GetHistogram(histname));
    histDraw1->Initialize();
    histDraw1->set_foreground_color(Qt::cyan);
    histDraw1->set_background_color(Qt::darkBlue);
    histDraw1->set_outline_color(Qt::cyan);
    if(ui_->convertButton->isChecked()){
        histDraw1->set_xlabel("Energy");
    }else{
        histDraw1->set_xlabel("Channel");
    }
    histDraw1->set_ylabel("Counts");
    win1->resize(450,300);
    win1->show();

    //Time window:
    int startTime = (ui_->refTime->dateTime()).secsTo(ui_->startTime->dateTime());
    int endTime = (ui_->refTime->dateTime()).secsTo(ui_->endTime->dateTime());

    //Open the file:
    std::ifstream logfile(ui_->dataFileName->toPlainText().toStdString()+".txt");
    while(!logfile.eof()){
        //Get the next line in the file and store it:
        std::string temp;
        std::getline(logfile,temp);
        //Last line of the file is thrown out since it's usually a carriage return: (break 1 line early)
        if(logfile.eof()){
            break;
        }

        //Find out index of tab in data list:
        int firstEntryEnd = temp.find_first_of("\t");

        //Get the timestamp for this row of data:
        QString qstrTime = QString::fromStdString(temp.substr(firstEntryEnd+1));
        double time = qstrTime.toDouble();
        //If it's within the desired time range, add the channel # to our list:
        if(time > startTime && time < endTime){
            QString chan = QString::fromStdString(temp.substr(0,firstEntryEnd));
            //chan.append(chan.toDouble());
            double doubleChan[1]; doubleChan[0] = chan.toDouble();
            an_thread_->UpdateHistogram(histname,doubleChan,1);
        }
    }
    logfile.close();


}



