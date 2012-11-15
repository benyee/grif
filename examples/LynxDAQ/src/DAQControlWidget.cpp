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
  update_timer_->setInterval(2000);
  update_timer_->start();

  // set up timer to be used for HV enable
  hv_enable_timer_ = new QTimer();
  hv_enable_timer_->setInterval(500);
  DisableHVControl();

  //Initialize Histogram status boxes:
  QPalette palette;
  palette = ui_->histbox1->palette();
  palette.setColor(ui_->histbox1->backgroundRole(),Qt::yellow);
  ui_->histbox1->setPalette(palette);
  hist_status1 = false;

  palette = ui_->histbox2->palette();
  palette.setColor(ui_->histbox2->backgroundRole(),Qt::yellow);
  ui_->histbox2->setPalette(palette);
  hist_status2 = false;

  //Ensure histogram attributes match default values:
  UpdateHistRate1();
  UpdateHistRate2();
  ToggleRateMode1();
  ToggleRateMode2();

  //Update the page to make sure everything's good to go:
  Update();


  //start the signal and slot pairs...
  connect(update_timer_, SIGNAL(timeout()), this, SLOT(Update()));

  connect(ui_->hvon, SIGNAL(clicked()), this, SLOT(SetHVOn()));
  connect(ui_->hvoff, SIGNAL(clicked()), this, SLOT(SetHVOff()));

  connect(ui_->hvenablebutton, SIGNAL(clicked()), this, SLOT(EnableHVControl()));
  connect(hv_enable_timer_, SIGNAL(timeout()), this, SLOT(DisableHVControl()));

  connect(ui_->simToggle,SIGNAL(clicked()),this,SLOT(ToggleSim()));
  connect(ui_->daqconnect,SIGNAL(clicked()),this,SLOT(Connect()));
  connect(ui_->daqstartstop,SIGNAL(clicked()),this,SLOT(StartStopAcq()));

  connect(ui_->histOnOff1,SIGNAL(clicked()),this,SLOT(ToggleHist1()));
  connect(ui_->histClear1,SIGNAL(clicked()),this,SLOT(ClearHist1()));
  connect(ui_->histOnOff2,SIGNAL(clicked()),this,SLOT(ToggleHist2()));
  connect(ui_->histClear2,SIGNAL(clicked()),this,SLOT(ClearHist2()));

  connect(ui_->packetValue1,SIGNAL(valueChanged(double)),this,SLOT(UpdateHistRate1()));
  connect(ui_->packetValue2,SIGNAL(valueChanged(double)),this,SLOT(UpdateHistRate2()));

  connect(ui_->rateMode1,SIGNAL(clicked()),this,SLOT(ToggleRateMode1()));
  connect(ui_->rateMode2,SIGNAL(clicked()),this,SLOT(ToggleRateMode2()));


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
  if(daq_status_ == kDAQStatusUnknown || daq_thread_->isSimMode()){ hv_status_ = 3;} //not connected to Lynx
  else{
      //Ask Lynx for HV status if connected:
      hv_status_ = daq_thread_->IsHVOn();
      hv_volts_ = daq_thread_->HV();
  }

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
      QString live,real,dead;
      if(daq_thread_->getLiveTime() < 0){
          ui_->liveTime->setText(QString("Live Time: ???"));
          ui_->realTime->setText(QString("Real Time: ???"));
          ui_->deadTime->setText(QString("Dead Time: "));
      }else{
      live.sprintf("%.2f",daq_thread_->getLiveTime()/1000000);
      real.sprintf("%.2f",daq_thread_->getRealTime()/1000000);
      dead.sprintf("%.2f",(daq_thread_->getRealTime()-daq_thread_->getLiveTime())/daq_thread_->getRealTime()*100);
      ui_->liveTime->setText(QString("Live Time: ") + live + ' s');
      ui_->realTime->setText(QString("Real Time: ") + real + ' s');
      ui_->deadTime->setText(QString("Dead Time: ") + dead + '%');
      }
  }

}


//The user has a few seconds after clicking this button before the HV controls are disabled again:
void DAQControlWidget::EnableHVControl() {
  if (daq_status_ != kDAQStatusUnknown && !daq_thread_->isSimMode()) {
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
  if (daq_status_ != kDAQStatusUnknown) {
    hv_enabled_ = false;
    hv_enable_timer_->stop();
    ui_->hvon->setEnabled(false);
    ui_->hvoff->setEnabled(false);
  }
}
void DAQControlWidget::Connect(){
    daq_thread_->ConnectToDAQ();
    ui_->simToggle->setEnabled(false);
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
        live.sprintf("%.2f",daq_thread_->getLiveTime()/1000000);
        real.sprintf("%.2f",daq_thread_->getRealTime()/1000000);
        logfile<<"Live Time: "<<live.toStdString()<<" s"<<'\n';
        logfile<<"Real Time: "<<real.toStdString()<<" s"<<'\n';
        logfile<< "=================================="<<'\n';
        logfile.close();

        if(!reg_started_){  //the regulator hasn't started (acquisition had started outside the GUI)
            daq_thread_->StopDataAcquisition(); //stop acquisition without doing anything else.
            ui_->daqstartstop->setEnabled(true);
            return;
        }

        if(hist_status1){ToggleHist1();}
        if(hist_status2){ToggleHist2();}

        //Otherwise, the regulator has started.
        daq_thread_->StopCollection();

        ui_->daqstartstop->setEnabled(false); //Disable button until it fully stops.
        GRISleep::msleep(2500); //Wait until the appropriate threads have stopped.
        ui_->daqstartstop->setEnabled(true);

    }
    else if(!reg_started_) { //if the regulators haven't been started (this is the first time you've hit start/stop)
        //Send file name/reference time and prevent further file name changes:
        an_thread_->setFileName(ui_->outFileName->toPlainText().toStdString());
        ui_->outFileName->setEnabled(false);
        daq_thread_->setRefTime(ui_->refTime->dateTime());
        ui_ ->refTime->setEnabled(false);

        //Display the acquisition start time:
        ui_->startTime->setText("Start Time: " +QDateTime::currentDateTime().toString("dd.MMM.yyyy hh:mm:ss.zzz"));
        ui_->endTime->setText("End Time: ???");

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
        ui_->startTime->setText("Start Time: " + QDateTime::currentDateTime().toString("dd.MMM.yyyy hh:mm:ss.zzz"));


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

void DAQControlWidget::ToggleHist1(){
    if(daq_status_!=kDAQStatusStarted){
        //std::cout<<hist_status<<endl;
        return;
    }
    if(hist_status1){
        hist_status1 = false;
        QPalette palette;
        palette = ui_->histbox1->palette();
        palette.setColor(ui_->histbox1->backgroundRole(),Qt::yellow);
        ui_->histbox1->setPalette(palette);
        ui_->histStatus1->setText("Status: Not Plotting");
        an_thread_->setPlotStatus(false,1);
        ui_->histEnd1->setText("End Time: " +QDateTime::currentDateTime().toString("dd.MMM.yyyy hh:mm:ss.zzz"));

    }else{
        hist_status1 = true;
        QPalette palette;
        palette = ui_->histbox1->palette();
        palette.setColor(ui_->histbox1->backgroundRole(),Qt::green);
        ui_->histbox1->setPalette(palette);
        ui_->histStatus1->setText("Status: Plotting!");
        an_thread_->setPlotStatus(true,1);
        ui_->histStart1->setText("Start Time: " +QDateTime::currentDateTime().toString("dd.MMM.yyyy hh:mm:ss.zzz"));
        ui_->histEnd1->setText("End Time: ???");
    }
}


void DAQControlWidget::ToggleHist2(){
    if(daq_status_!=kDAQStatusStarted){
        //std::cout<<hist_status<<endl;
        return;
    }
    if(hist_status2){
        hist_status2 = false;
        QPalette palette;
        palette = ui_->histbox2->palette();
        palette.setColor(ui_->histbox2->backgroundRole(),Qt::yellow);
        ui_->histbox2->setPalette(palette);
        ui_->histStatus2->setText("Status: Not Plotting");
        an_thread_->setPlotStatus(false,2);
        ui_->histEnd2->setText("End Time: " +QDateTime::currentDateTime().toString("dd.MMM.yyyy hh:mm:ss.zzz"));

    }else{
        hist_status2 = true;
        QPalette palette;
        palette = ui_->histbox2->palette();
        palette.setColor(ui_->histbox2->backgroundRole(),Qt::green);
        ui_->histbox2->setPalette(palette);
        ui_->histStatus2->setText("Status: Plotting!");
        an_thread_->setPlotStatus(true,2);
        ui_->histStart2->setText("Start Time: " +QDateTime::currentDateTime().toString("dd.MMM.yyyy hh:mm:ss.zzz"));
        ui_->histEnd2->setText("End Time: ???");
    }
}

void DAQControlWidget::ClearHist1(){
    an_thread_->ClearHistogram("Histogram1");
    ui_->histEnd1->setText("End Time: ???");
    if(hist_status1){
        ui_->histStart1->setText("Start Time: " +QDateTime::currentDateTime().toString("dd.MMM.yyyy hh:mm:ss.zzz"));
     }else{
        ui_->histStart1->setText("Start Time: ???");
    }
}

void DAQControlWidget::ClearHist2(){
    an_thread_->ClearHistogram("Histogram2");
    ui_->histEnd2->setText("End Time: ???");
    if(hist_status2){
        ui_->histStart2->setText("Start Time: " +QDateTime::currentDateTime().toString("dd.MMM.yyyy hh:mm:ss.zzz"));
     }else{
        ui_->histStart2->setText("Start Time: ???");
    }
}

void DAQControlWidget::UpdateHistRate1(){
    an_thread_->setHistRate(ui_->packetValue1->value(),1);
}
void DAQControlWidget::UpdateHistRate2(){
    an_thread_->setHistRate(ui_->packetValue2->value(),2);
}

void DAQControlWidget::ToggleRateMode1(){
    an_thread_->setHistMode(ui_->rateMode1->isChecked(),1);
    if(ui_->rateMode1->isChecked()){
        ui_->packetValue1->setEnabled(true);
    }else{ui_->packetValue1->setEnabled(false);}
}
void DAQControlWidget::ToggleRateMode2(){
    an_thread_->setHistMode(ui_->rateMode2->isChecked(),2);
    if(ui_->rateMode2->isChecked()){
        ui_->packetValue2->setEnabled(true);
    }else{ui_->packetValue2->setEnabled(false);}
}



void DAQControlWidget::ToggleSim(){
    bool simStatus = daq_thread_->isSimMode();
    if(simStatus){
        ui_->simToggle->setText("Simulation Mode OFF");
    }else{ui_->simToggle->setText("Simulation Mode ON");}
    daq_thread_->setSimMode(!simStatus);
}
