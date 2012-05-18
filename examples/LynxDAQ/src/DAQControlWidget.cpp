#include <QPalette>

#include "DAQControlWidget.h"
#include "ui_DAQControlWidget.h"

const int DAQControlWidget::kDAQStatusUnknown     = 0;
const int DAQControlWidget::kDAQStatusConnected   = 1;
const int DAQControlWidget::kDAQStatusStarted     = 2;
const int DAQControlWidget::kDAQStatusStopped     = 3;
const int DAQControlWidget::kDAQStatusError       = 4;

const int DAQControlWidget::kHVStatusUnknown      = 10;
const int DAQControlWidget::kHVStatusON           = 11;
const int DAQControlWidget::kHVStatusOFF          = 12;
const int DAQControlWidget::kHVStatusRampingUp    = 13;
const int DAQControlWidget::kHVStatusRampingDown  = 14;

DAQControlWidget::DAQControlWidget(QWidget *parent, LynxDAQ *daq) :
  QWidget(parent),
  ui_(new Ui::DAQControlWidget) {
  ui_->setupUi(this);

  daq_status_ = kDAQStatusUnknown;
  hv_status_ = kHVStatusUnknown;
  hv_volts_ = 0.0;
  hv_volts_max_ = -1.0;

  // for now, say DAQ and HV are known
  daq_status_ = kDAQStatusConnected;
  hv_status_ = kHVStatusOFF;
  hv_volts_ = 0.0;
  set_hv_volts_max(-2500.0);

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
//  connect(ui_->daqbutton, SIGNAL(clicked()), this, SLOT(StartStopDAQ()));
  connect(ui_->hvonoffbutton, SIGNAL(clicked()), this, SLOT(SetHVOnOff()));
  connect(ui_->hvenablebutton, SIGNAL(clicked()), this, SLOT(EnableHVControl()));
  connect(hv_enable_timer_, SIGNAL(timeout()), this, SLOT(DisableHVControl()));
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

  // DAQ status
  switch (daq_status_) {
    case kDAQStatusUnknown:
      ui_->daqstatus->setText(QString("Status: Unknown"));
      palette = ui_->daqbox->palette();
      palette.setColor(ui_->daqbox->backgroundRole(), Qt::lightGray);
      ui_->daqbox->setPalette(palette);
      ui_->hvenablebutton->setEnabled(false);
      break;
    case kDAQStatusConnected:
      ui_->daqstatus->setText(QString("Status: DAQ Connected"));
      palette = ui_->daqbox->palette();
      palette.setColor(ui_->daqbox->backgroundRole(), Qt::green);
      ui_->daqbox->setPalette(palette);
      ui_->hvenablebutton->setEnabled(true);
      break;
    case kDAQStatusStarted:
      ui_->daqstatus->setText(QString("Status: DAQ Started"));
      palette = ui_->daqbox->palette();
      palette.setColor(ui_->daqbox->backgroundRole(), Qt::green);
      ui_->daqbox->setPalette(palette);
      ui_->hvenablebutton->setEnabled(true);
      break;
    case kDAQStatusStopped:
      ui_->daqstatus->setText(QString("Status: DAQ Stopped"));
      palette = ui_->daqbox->palette();
      palette.setColor(ui_->daqbox->backgroundRole(), Qt::green);
      ui_->daqbox->setPalette(palette);
      ui_->hvenablebutton->setEnabled(true);
      break;
    case kDAQStatusError:
      ui_->daqstatus->setText(QString("Status: DAQ Error"));
      palette = ui_->daqbox->palette();
      palette.setColor(ui_->daqbox->backgroundRole(), Qt::red);
      ui_->daqbox->setPalette(palette);
      ui_->hvenablebutton->setEnabled(false);
      break;
    default:
      break;
  }

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
    case kHVStatusRampingUp:
      ui_->hvstatus->setText(QString("Status: HV Ramping Up"));
      palette = ui_->hvbox->palette();
      palette.setColor(ui_->hvbox->backgroundRole(), Qt::yellow);
      ui_->hvbox->setPalette(palette);
      hv_volts_ -= 20.;
      if (hv_volts_ <= hv_volts_max_) {
        hv_volts_ = hv_volts_max_;
        hv_status_ = kHVStatusON;
      }
      break;
    case kHVStatusON:
      ui_->hvstatus->setText(QString("Status: HV ON"));
      palette = ui_->hvbox->palette();
      palette.setColor(ui_->hvbox->backgroundRole(), Qt::green);
      ui_->hvbox->setPalette(palette);
      break;
    case kHVStatusRampingDown:
      ui_->hvstatus->setText(QString("Status: HV Ramping Down"));
      palette = ui_->hvbox->palette();
      palette.setColor(ui_->hvbox->backgroundRole(), Qt::yellow);
      ui_->hvbox->setPalette(palette);
      hv_volts_ += 40.;
      if (hv_volts_ >= 0.) {
        hv_volts_ = 0.;
        hv_status_ = kHVStatusOFF;
      }
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
      ui_->hvonoffbutton->setEnabled(true);
    } else if (hv_enabled_) {
      DisableHVControl();
    }
  }
}


void DAQControlWidget::DisableHVControl() {
  if (daq_status_ != kHVStatusUnknown) {
    hv_enabled_ = false;
    hv_enable_timer_->stop();
    ui_->hvonoffbutton->setEnabled(false);
  }
}


void DAQControlWidget::SetHVOnOff() {
  if (hv_enabled_ && hv_status_ == kHVStatusON) {
    ui_->hvstatus->setText(QString("Status: HV Ramping Down"));
    hv_status_ = kHVStatusRampingDown;
  } else if (hv_enabled_ && hv_status_ == kHVStatusOFF) {
    ui_->hvstatus->setText(QString("Status: HV Ramping Up"));
    hv_status_ = kHVStatusRampingUp;
  }
}
