#ifndef ACTIVITYCOUNTER_H
#define ACTIVITYCOUNTER_H

class ActivityCounter{
public:
    ActivityCounter(int input_ROIL, int input_ROIR,int input_eventsPerBin = 50,double input_dt = 1);
    ~ActivityCounter();

private:
    int leftROI;
    int rightROI;

    int eventsPerBin;
    double dt;
    bool binMode;

    vector<double> energy;
    vector<double> timestamp;

    vector<double> rate;
    vector<double> time;

    void setMode(bool x){binMode = x;}
    bool getMode(){return binMode;}
    void setBinRate(double x);
    double getBinRate();

    void resetLROI(int x){leftROI = x;}
    void resetRROI(int x){rightROI = x;}
    int* getROI(int x){int* out = {leftROI, rightROI}; return out;}

    void addToList(vector<double> e, vector<double> t);
};

#endif // ACTIVITYCOUNTER_H
