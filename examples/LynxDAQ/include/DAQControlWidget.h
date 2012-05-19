#ifndef GRIF_UI_DAQCONTROLWIDGET_H_
#define GRIF_UI_DAQCONTROLWIDGET_H_

#include <QMainWindow>
#include "LynxDAQ.h"
#include <QTimer>

namespace Ui {
    class DAQControlWidget;
}

class DAQControlWidget : public QWidget
{
    Q_OBJECT

public:
    explicit DAQControlWidget(QWidget *parent = 0, LynxDAQ *daq = 0,GRIRegulator *reg = 0);
    ~DAQControlWidget();

  /// State constants
  static const int kDAQStatusUnknown;
  static const int kDAQStatusConnected;
  static const int kDAQStatusStarted;
  static const int kDAQStatusStopped;
  static const int kDAQStatusError;
  static const int kHVStatusUnknown;
  static const int kHVStatusOFF;
  static const int kHVStatusON;
  static const int kHVStatusRamping;

  void set_hv_volts_max(double hv_max);

private:
    LynxDAQ *daq_thread_;
    GRIRegulator *reg;
    Ui::DAQControlWidget *ui_;

    int daq_status_;
    int hv_status_;
    double hv_volts_;
    double hv_volts_max_;
    /// timer to update data
    QTimer* update_timer_;
    /// timer for HV enable
    bool hv_enabled_;
    QTimer* hv_enable_timer_;

signals:

private slots:
//    void StartStopDAQ();
    void EnableHVControl();
    void DisableHVControl();
    void SetHVOn(){daq_thread_->TurnOnHV();}
    void SetHVOff(){daq_thread_->TurnOffHV();}
    void Update();
    void StartStopAcq();
    void Connect(){daq_thread_->ConnectToDAQ();}
 };

#endif // GRIF_UI_DAQCONTROLWIDGET_H_
