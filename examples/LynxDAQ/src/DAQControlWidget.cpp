#include <QPalette>

#include "DAQControlWidget.h"
#include "ui_DAQControlWidget.h"

const int DAQControlWidget::kDAQStatusUnknown     = 0;
const int DAQControlWidget::kDAQStatusConnected   = 1;
const int DAQControlWidget::kDAQStatusStarted     = 2;

const int DAQControlWidget::kHVStatusUnknown      = 3;
const int DAQControlWidget::kHVStatusON           = 1;
const int DAQControlWidget::kHVStatusOFF          = 0;
const int DAQControlWidget::kHVStatusRamping      = 2;

DAQControlWidget::DAQControlWidget(QWidget *parent, LynxDAQ *daq, GRIRegulator *r) :
  QWidget(parent),
  ui_(new Ui::DAQControlWidget) {
  ui_->setupUi(this);

  daq_thread_ = daq;
  reg = r;

  if(daq->IsConnected()){
    daq_status_ = 1;
    if(daq->IsAcquiring()){ daq_status_=2;}
  }
  else{ daq_status_ = 0; hv_status_ = 3;}

  set_hv_volts_max(-5000);
  if(daq_status_ != 0){
      hv_status_ = daq_thread_->IsHVOn();
      hv_volts_ = daq_thread_->HV();
  }

  // set up a timer to update widget
  update_timer_ = new QTimer();
  update_timer_->setInterval(500);
  update_timer_->start();

  // set up timer to be used for HV enable
  hv_enable_timer_ = new QTimer();
  hv_enable_timer_->setInterval(500);
  DisableHVControl();

  //start the signal and slot pairs...
  connect(update_timer_, SIGNAL(timeout()), this, SLOT(Update()));

  connect(ui_->hvon, SIGNAL(clicked()), this, SLOT(SetHVOn()));
  connect(ui_->hvoff, SIGNAL(clicked()), this, SLOT(SetHVOff()));

  connect(ui_->hvenablebutton, SIGNAL(clicked()), this, SLOT(EnableHVControl()));
  connect(hv_enable_timer_, SIGNAL(timeout()), this, SLOT(DisableHVControl()));

  connect(ui_->daqconnect,SIGNAL(clicked()),this,SLOT(Connect()));
  connect(ui_->daqstartstop,SIGNAL(clicked()),this,SLOT(StartStopAcq()));
}


DAQControlWidget::~DAQControlWidget() {
  delete ui_;
  delete update_timer_;
  delete hv_enable_timer_;
}


void DAQControlWidget::set_hv_volts_max(double hv_max) {
  if (hv_max < 0.) {
    hv_volts_max_ = hv_max;
    ui_->hvprogressbar->setRange(0, abs((int)hv_volts_max_));
  }
}


void DAQControlWidget::Update() {
  QPalette palette;

  if(daq_thread_->IsConnected()){
    daq_status_ = 1;
    if(daq_thread_->IsAcquiring()){
        daq_status_=2;
    }
  }
  else{ daq_status_ = 0; }

  // DAQ status
  switch (daq_status_) {
    case kDAQStatusUnknown:
      ui_->daqstatus->setText(QString("Status: Unknown"));
      palette = ui_->daqbox->palette();
      palette.setColor(ui_->daqbox->backgroundRole(), Qt::red);
      ui_->daqbox->setPalette(palette);
      ui_->hvenablebutton->setEnabled(false);
      break;
    case kDAQStatusConnected:
      ui_->daqstatus->setText(QString("Status: DAQ Connected, not acquiring"));
      palette = ui_->daqbox->palette();
      palette.setColor(ui_->daqbox->backgroundRole(), Qt::yellow);
      ui_->daqbox->setPalette(palette);
      ui_->hvenablebutton->setEnabled(true);
      break;
    case kDAQStatusStarted:
      ui_->daqstatus->setText(QString("Status: DAQ acquiring"));
      palette = ui_->daqbox->palette();
      palette.setColor(ui_->daqbox->backgroundRole(), Qt::green);
      ui_->daqbox->setPalette(palette);
      ui_->hvenablebutton->setEnabled(true);
      break;
    default:
      break;
  }

  // HV status
  if(daq_status_){
      hv_status_ = daq_thread_->IsHVOn();
      hv_volts_ = daq_thread_->HV();
  }
  else{ hv_status_ = 3;}

  // HV status
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

  if (hv_status_ == kHVStatusUnknown) {
    ui_->hvvoltage->setText(QString("Voltage: Unknown"));
  } else {
    ui_->hvvoltage->setText(QString("Voltage: ")+QString::number((int)hv_volts_));
  }
  ui_->hvprogressbar->setValue(abs((int)hv_volts_));
}


//void DAQControlWidget::StartStopDAQ() {
//  if (daq_status_ == kDAQStatusConnected) {
//    // Start DAQ
//    daq_thread_->StartDataAcquisition();
//    daq_status_ = kDAQStatusStarted;
//    ui_->daqbutton->setText(QString("Stop DAQ"));
//  } else if (daq_status_ == kDAQStatusStarted) {
//    // Stop DAQ
//    daq_thread_->StopDataAcquisition();
//    daq_status_ = kDAQStatusStopped;
//    ui_->daqbutton->setText(QString("Start DAQ"));
//  } else if (daq_status_ == kDAQStatusUnknown || daq_status_ == kDAQStatusUnknown) {
//    ui_->daqbutton->setEnabled(false);
//    ui_->hvonoffbutton->setEnabled(false);
//  }
//}


void DAQControlWidget::EnableHVControl() {
  if (daq_status_ != kHVStatusUnknown) {
    if (!hv_enabled_) {
      hv_enabled_ = true;
      hv_enable_timer_->start();
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

void DAQControlWidget::StartStopAcq(){
    if (daq_status_ == 2){reg->Stop(); GRISleep::msleep(5000); }
    else {reg->Start();}
}
