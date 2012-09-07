#ifndef ANALYSISWIDGET_H
#define ANALYSISWIDGET_H


#include <QMainWindow>
#include "LynxDAQ.h"
#include "SIMAnalysisThread.h"
#include <QTimer>
#include <string>

/*  This widget serves as a GUI control for the Lynx DAQ system. */

namespace Ui {
    class AnalysisWidget;
}

class AnalysisWidget : public QWidget
{
    Q_OBJECT

public:
    explicit AnalysisWidget(QWidget *parent = 0,  LynxDAQ *daq = 0,SIMAnalysisThread *sat = 0,GRIRegulator *reg = 0);
    ~AnalysisWidget();

private:
    LynxDAQ *daq_thread_; /// Pointer to the Lynx DAQ class, provides access to the Lynx hardware
    SIMAnalysisThread *an_thread_; /// Pointer to the analysis class, provides access to data output
    GRIRegulator *reg; /// Pointer to regulator, allows us to start/stop the threads
    Ui::AnalysisWidget *ui_;

    bool reg_started_; /// whether the regulator has been started
    int daq_status_;
    QTimer* update_timer_;/// timer to update data

signals:

private slots:
    void Update();  //Refreshes the GUI, checking for new signals/updates
    void SetEndTimeToCurrent();
    void CreateHistogram();
};

#endif // ANALYSISWIDGET_H
