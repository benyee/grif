#ifndef GRIF_UI_DAQCONTROLWIDGET_H_
#define GRIF_UI_DAQCONTROLWIDGET_H_

#include <QMainWindow>
#include "LynxDAQ.h"
#include "SIMAnalysisThread.h"
#include <QTimer>
#include <string>

/*  This widget serves as a GUI control for the Lynx DAQ system. */

namespace Ui {
    class DAQControlWidget;
}

class DAQControlWidget : public QWidget
{
    Q_OBJECT

public:
    explicit DAQControlWidget(QWidget *parent = 0, LynxDAQ *daq = 0, SIMAnalysisThread *sat = 0,GRIRegulator *reg = 0);
    ~DAQControlWidget();

  /// State constants
  static const int kDAQStatusUnknown;
  static const int kDAQStatusConnected;
  static const int kDAQStatusStarted;
  static const int kHVStatusUnknown;
  static const int kHVStatusOFF;
  static const int kHVStatusON;
  static const int kHVStatusRamping;

  void set_hv_volts_max(double hv_max);


private:
    LynxDAQ *daq_thread_; /// Pointer to the Lynx DAQ class, provides access to the Lynx hardware
    SIMAnalysisThread *an_thread_; /// Pointer to the analysis class, provides access to data output
    GRIRegulator *reg; /// Pointer to regulator, allows us to start/stop the threads
    Ui::DAQControlWidget *ui_;

    bool reg_started_; /// whether the regulator has been started
    int daq_status_;
    QTimer* update_timer_;/// timer to update data

    int hv_status_;
    double hv_volts_; /// current voltage
    double hv_volts_max_; /// max voltage possible
    bool hv_enabled_;
    QTimer* hv_enable_timer_; /// timer for HV enable

    bool hist_status;

signals:

private slots:
    void EnableHVControl(); //Allow access to HV On/Off buttons.
    void DisableHVControl(); //Disable access to HV On/Off buttons.
    void SetHVOn(){daq_thread_->TurnOnHV();}
    void SetHVOff(){daq_thread_->TurnOffHV();}
    void Update();  //Refreshes the GUI, checking for new signals/updates
    void StartStopAcq();  //Deals with what happens when you press the Start/Stop button
    void Connect(){daq_thread_->ConnectToDAQ();}
    void ToggleHist();
    void ClearHist(){an_thread_->ClearHistogram("Histogram");}
};

#endif // GRIF_UI_DAQCONTROLWIDGET_H_
