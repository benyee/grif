
#include <QPalette>

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
    //Create/clear histogram:
    QString histname = "Hist";
    if(!(an_thread_->CreateNewHistogram(histname,300,0.0,1000.0))){
        an_thread_->ClearHistogram(histname);
    }

    // Set up GUI window
    QMainWindow *win1 = new QMainWindow();
    GRIHist1DWidget *histDraw1 = new GRIHist1DWidget(win1);
    win1->setCentralWidget(histDraw1);
    histDraw1->set_hist(an_thread_->GetHistogram("hist"));
    histDraw1->Initialize();
    histDraw1->set_foreground_color(Qt::cyan);
    histDraw1->set_background_color(Qt::darkBlue);
    histDraw1->set_outline_color(Qt::cyan);
    if(ui_->convertButton->isChecked()){
        histDraw1->set_xlabel("Channel");
    }else{
        histDraw1->set_xlabel("Energy");
    }
    histDraw1->set_ylabel("Counts");
    win1->resize(450,300);
    win1->show();


}



