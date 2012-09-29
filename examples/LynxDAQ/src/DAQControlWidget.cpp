#include <QPalette>

#include "DAQControlWidget.h"
#include "ui_DAQControlWidget.h"

#include <iomanip>
#include <iostream>
#include <fstream>

const int DAQControlWidget::kDAQStatusUnknown     = 0;
const int DAQControlWidget::kDAQStatusConnected   = 1;
const int DAQControlWidget::kDAQStatusStarted     = 2;

const int DAQControlWidget::kHVStatusUnknown      = 3;
const int DAQControlWidget::kHVStatusON           = 1;
const int DAQControlWidget::kHVStatusOFF          = 0;
const int DAQControlWidget::kHVStatusRamping      = 2;

DAQControlWidget::DAQControlWidget(QWidget *parent, LynxDAQ *daq, SIMAnalysisThread *sat, GRIRegulator *r) :
  QWidget(parent),
  ui_(new Ui::DAQControlWidget) {
  ui_->setupUi(this);

  //Retrieve DAQ, analysis thread, and regulator pointers:
  daq_thread_ = daq;
  an_thread_ = sat;
  reg = r;
  reg_started_ = false;

  set_hv_volts_max(-5000);

  // set up a timer to update widget
  update_timer_ = new QTimer();
  update_timer_->setInterval(1000);
  update_timer_->start();

  // set up timer to be used for HV enable
  hv_enable_timer_ = new QTimer();
  hv_enable_timer_->setInterval(500);
  DisableHVControl();

  //Update the page to make sure everything's good to go:
  Update();

  QPalette palette;
  palette = ui_->histbox->palette();
  palette.setColor(ui_->histbox->backgroundRole(),Qt::yellow);
  ui_->histbox->setPalette(palette);
  hist_status = false;

  //start the signal and slot pairs...
  connect(update_timer_, SIGNAL(timeout()), this, SLOT(Update()));

  connect(ui_->hvon, SIGNAL(clicked()), this, SLOT(SetHVOn()));
  connect(ui_->hvoff, SIGNAL(clicked()), this, SLOT(SetHVOff()));

  connect(ui_->hvenablebutton, SIGNAL(clicked()), this, SLOT(EnableHVControl()));
  connect(hv_enable_timer_, SIGNAL(timeout()), this, SLOT(DisableHVControl()));

  connect(ui_->daqconnect,SIGNAL(clicked()),this,SLOT(Connect()));
  connect(ui_->daqstartstop,SIGNAL(clicked()),this,SLOT(StartStopAcq()));

  connect(ui_->histOnOff,SIGNAL(clicked()),this,SLOT(ToggleHist()));
  connect(ui_->histClear,SIGNAL(clicked()),this,SLOT(ClearHist()));
}


DAQControlWidget::~DAQControlWidget() {
  delete ui_;
  delete update_timer_;
  delete hv_enable_timer_;
  delete daq_thread_;
  delete an_thread_;
  delete reg;
}


void DAQControlWidget::set_hv_volts_max(double hv_max) {
  if (hv_max < 0.) {
    hv_volts_max_ = hv_max;
    ui_->hvprogressbar->setRange(0, abs((int)hv_volts_max_));
  }
}


void DAQControlWidget::Update() {
  QPalette palette;

  //Check connection/acquisition status:
  if(daq_thread_->IsConnected()){
    daq_status_ = kDAQStatusConnected;
    if(daq_thread_->IsAcquiring()){
        daq_status_=kDAQStatusStarted;
    }

    //Connect button no longer necessary, but Start/Stop is now needed:
    ui_->daqconnect->setEnabled(false);
    ui_->daqstartstop->setEnabled(true);

  }
  else{
      daq_status_ = kDAQStatusUnknown;

      //User must first connect to Lynx before starting data acquisition:
      ui_->daqconnect->setEnabled(true);
      ui_->daqstartstop->setEnabled(false);
  }

  //Allow the user to change output file name and reference time unless one of the cases below says otherwise.
  ui_ ->outFileName->setEnabled(true);
  ui_ ->refTime->setEnabled(true);
  //Allow the user to alter HV unless one of the cases below says otherwise
  ui_->hvenablebutton->setEnabled(true);

  // DAQ status - the first 4 lines in each case update the display of the first box.
  switch (daq_status_) {
    case kDAQStatusUnknown:
      ui_->daqstatus->setText(QString("Status: Unknown"));
      palette = ui_->daqbox->palette();
      palette.setColor(ui_->daqbox->backgroundRole(), Qt::lightGray);
      ui_->daqbox->setPalette(palette);

      //Disable the HV button since you're not connected anyway:
      ui_->hvenablebutton->setEnabled(false);
      break;
    case kDAQStatusConnected:
      ui_->daqstatus->setText(QString("Status: DAQ Connected, not acquiring"));
      palette = ui_->daqbox->palette();
      palette.setColor(ui_->daqbox->backgroundRole(), Qt::yellow);
      ui_->daqbox->setPalette(palette);
      break;
    case kDAQStatusStarted:
      ui_->daqstatus->setText(QString("Status: DAQ acquiring"));
      palette = ui_->daqbox->palette();
      palette.setColor(ui_->daqbox->backgroundRole(), Qt::green);
      ui_->daqbox->setPalette(palette);

      //Make sure that the output file name and reference time cannot be changed mid-acquisition:
      ui_->outFileName->setEnabled(false);
      ui_ ->refTime->setEnabled(false);
      break;
    default:
      break;
  }

  // HV status
  if(daq_status_ != kDAQStatusUnknown){
      //Ask Lynx for HV status if connected:
      hv_status_ = daq_thread_->IsHVOn();
      hv_volts_ = daq_thread_->HV();
  }
  else{ hv_status_ = 3;} //not connected to Lynx

  //First 4 lines of each case update the display in the HV information box.
  switch (hv_status_) {
    case kHVStatusUnknown:
      ui_->hvstatus->setText(QString("Status: Unknown"));
      palette = ui_->hvbox->palette();
      palette.setColor(ui_->hvbox->backgroundRole(), Qt::lightGray);
      ui_->hvbox->setPalette(palette);
      break;
    case kHVStatusOFF:
      ui_->hvstatus->setText(QString("Status: HV OFF"));
      palette = ui_->hvbox->palette();
      palette.setColor(ui_->hvbox->backgroundRole(), Qt::red);
      ui_->hvbox->setPalette(palette);
      break;
    case kHVStatusRamping:
      ui_->hvstatus->setText(QString("Status: HV Ramping"));
      palette = ui_->hvbox->palette();
      palette.setColor(ui_->hvbox->backgroundRole(), Qt::yellow);
      ui_->hvbox->setPalette(palette);
      break;
    case kHVStatusON:
      ui_->hvstatus->setText(QString("Status: HV ON"));
      palette = ui_->hvbox->palette();
      palette.setColor(ui_->hvbox->backgroundRole(), Qt::green);
      ui_->hvbox->setPalette(palette);
      break;
    default:
      break;
  }

  //Display voltage reading:
  if (hv_status_ == kHVStatusUnknown) {
    ui_->hvvoltage->setText(QString("Voltage: Unknown"));
  } else {
    ui_->hvvoltage->setText(QString("Voltage: ")+QString::number((int)hv_volts_));
  }
  //Set voltage progress bar:
  ui_->hvprogressbar->setValue(abs((int)hv_volts_));

  //If connected, update the live and real times.
  if(daq_status_!=kDAQStatusUnknown){
      QString live,real;
      live.sprintf("%.2f",daq_thread_->LiveTime()/1000000);
      real.sprintf("%.2f",daq_thread_->RealTime()/1000000);
      ui_->liveTime->setText(QString("Live Time: ") + live + ' s');
      ui_->realTime->setText(QString("Real Time: ") + real + ' s');
  }
}

//The user has a few seconds after clicking this button before the HV controls are disabled again:
void DAQControlWidget::EnableHVControl() {
  if (daq_status_ != kHVStatusUnknown) {
    if (!hv_enabled_) {
      hv_enabled_ = true;
      hv_enable_timer_->start();  //Starts HV timer.  If this expires, HV control will be disabled again.
      ui_->hvon->setEnabled(true);
      ui_->hvoff->setEnabled(true);
    } else if (hv_enabled_) {
      DisableHVControl();
    }
  }
}

void DAQControlWidget::DisableHVControl() {
  if (daq_status_ != kHVStatusUnknown) {
    hv_enabled_ = false;
    hv_enable_timer_->stop();
    ui_->hvon->setEnabled(false);
    ui_->hvoff->setEnabled(false);
  }
}

//What happens when you press Start/Stop:
void DAQControlWidget::StartStopAcq(){
    //Connect first if not connected already.
    if(!daq_thread_->IsConnected()){Connect(); return;}

    ui_->daqstartstop->setEnabled(false);
    if (daq_status_ == kDAQStatusStarted){ //if acquisition has already started
        //Display the acquisition end time:
        ui_->endTime->setText("End Time: " + QDateTime::currentDateTime().toString("dd.MMM.yyyy hh:mm:ss.zzz"));

        //Record the action in the log file:
        std::fstream logfile(ui_->logFileName->toPlainText().toStdString()+".txt",std::ios::app);
        logfile<< QDateTime::currentDateTime().toString("(yyyy, MMM dd; hh:mm:ss.zzz) ").toStdString() + " Stopping acquisition"<<'\n';
        QString live,real;
        live.sprintf("%.2f",daq_thread_->LiveTime()/1000000);
        real.sprintf("%.2f",daq_thread_->RealTime()/1000000);
        logfile<<"Live Time: "<<live.toStdString()<<" s"<<'\n';
        logfile<<"Real Time: "<<real.toStdString()<<" s"<<'\n';
        logfile<< "=================================="<<'\n';
        logfile.close();

        if(!reg_started_){  //the regulator hasn't started (acquisition had started outside the GUI)
            daq_thread_->StopDataAcquisition(); //stop acquisition without doing anything else.
            ui_->daqstartstop->setEnabled(true);
            return;
        }

        //Otherwise, the regulator has started.
        daq_thread_->StopCollection();

        ui_->daqstartstop->setEnabled(false); //Disable button until it fully stops.
        GRISleep::msleep(2500); //Wait until the appropriate threads have stopped.
        ui_->daqstartstop->setEnabled(true);

        if(hist_status){
            ToggleHist();
        }
    }
    else if(!reg_started_) { //if the regulators haven't been started (this is the first time you've hit start/stop)
        //Send file name/reference time and prevent further file name changes:
        an_thread_->setFileName(ui_->outFileName->toPlainText().toStdString());
        ui_->outFileName->setEnabled(false);
        daq_thread_->setRefTime(ui_->refTime->dateTime());
        ui_ ->refTime->setEnabled(false);

        //Display the acquisition start time:
        ui_->startTime->setText("Start Time: " +QDateTime::currentDateTime().toString("dd.MMM.yyyy hh:mm:ss.zzz"));
        ui_->startTime->setText("End Time: ???");

        ui_->daqstartstop->setEnabled(false); //Disable button until it starts up
        reg->Start(); //Start the regulators.
        reg_started_ = true;

        //Record the action in the log file:
        std::fstream logfile(ui_->logFileName->toPlainText().toStdString()+".txt",std::ios::app);
        logfile<< "=================================="<<'\n';
        logfile<< QDateTime::currentDateTime().toString("(yyyy, MMM dd; hh:mm:ss.zzz) ").toStdString() + " Starting acquisition"<<'\n';
        logfile.close();

        GRISleep::msleep(1000);//Wait until the appropriate threads have stopped.
        ui_->daqstartstop->setEnabled(true);
    }
    else{  //regulators have already started, but acquisition is currently stopped:
        //Send file name/reference time and prevent further file name changes:
        an_thread_->setFileName(ui_->outFileName->toPlainText().toStdString());
        ui_->outFileName->setEnabled(false);
        daq_thread_->setRefTime(ui_->refTime->dateTime());
        ui_ ->refTime->setEnabled(false);

        //Display the new acquisition start time:
        ui_->startTime->setText(QDateTime::currentDateTime().toString("dd.MMM.yyyy hh:mm:ss.zzz"));


        //Record the action in the log file:
        std::fstream logfile(ui_->logFileName->toPlainText().toStdString()+".txt",std::ios::app);
        logfile<< "=================================="<<'\n';
        logfile<< QDateTime::currentDateTime().toString("(yyyy, MMM dd; hh:mm:ss.zzz) ").toStdString() + " Starting acquisition"<<'\n';
        logfile.close();

        //Start data acquisition without restarting all the regulators and stuff:
        daq_thread_->StartCollection();
    }
    ui_->daqstartstop->setEnabled(true);
}

void DAQControlWidget::ToggleHist(){
    if(daq_status_!=kDAQStatusStarted){
        return;
    }
    if(hist_status){
        hist_status = false;
        QPalette palette;
        palette = ui_->histbox->palette();
        palette.setColor(ui_->histbox->backgroundRole(),Qt::yellow);
        ui_->histbox->setPalette(palette);
        ui_->histstatus->setText("Status: Not Plotting");
        an_thread_->setPlotStatus(false);
        ui_->histEnd->setText("End Time: " +QDateTime::currentDateTime().toString("dd.MMM.yyyy hh:mm:ss.zzz"));

    }else{
        hist_status = true;
        QPalette palette;
        palette = ui_->histbox->palette();
        palette.setColor(ui_->histbox->backgroundRole(),Qt::green);
        ui_->histbox->setPalette(palette);
        ui_->histstatus->setText("Status: Plotting!");
        an_thread_->setPlotStatus(true);
        ui_->histStart->setText("Start Time: " +QDateTime::currentDateTime().toString("dd.MMM.yyyy hh:mm:ss.zzz"));
        ui_->histEnd->setText("End Time: ???");
    }
}
