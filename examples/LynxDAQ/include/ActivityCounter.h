#ifndef ACTIVITYCOUNTER_H
#define ACTIVITYCOUNTER_H

#include <vector>
#include <QPair>

class ActivityCounter{
public:
    ActivityCounter(int input_ROIL, int input_ROIR,double input_currtime = 0,double input_dt = 1);
    ~ActivityCounter();

    void setBinRate(double x){dt = x;}
    double getBinRate(){return dt;}

    void resetLROI(int x){leftROI = x;}
    void resetRROI(int x){rightROI = x;}
    int* getROI(){int out[2] = {leftROI,rightROI}; return out;}

    QPair<std::vector<double>,std::vector<double> > getRawData();
    QPair<std::vector<double>,std::vector<double> > getPlotData();

    void addData(std::vector<double> e, std::vector<double> t);
    void computeRate();
    void clearData();

private:
    int leftROI;
    int rightROI;

    double dt;
    double currtime;

    std::vector<double> energy;
    std::vector<double> timestamp;

    std::vector<double> rate;
    std::vector<double> time;

};

#endif // ACTIVITYCOUNTER_H
