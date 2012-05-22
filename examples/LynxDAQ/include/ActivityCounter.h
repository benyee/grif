#ifndef ACTIVITYCOUNTER_H
#define ACTIVITYCOUNTER_H

#include <vector>
#include <QPair>
#include <iostream>

/*  This keeps track of the activity in a certain ROI.  Create one instance for each ROI you want to track.
  It will examine the data and count the number of hits in "dt"-width time windows.  After it has traversed
  all the data in a given time window, it will calculate the activity appropriately (counts divided by dt) and
  move onto the next time window to count.  The program updates "currtime" when it needs to move to the next time window.

  Use addData() to pass it raw data - the data can be in packets, you will just call addData() multiple times.
  Retrieve the activity data (time stamped by the central time of each time window) using getPlotData().

  Please note that I haven't had too much time to test this, so it might be VERY buggy.
  */
class ActivityCounter{
public:
    ActivityCounter(int input_ROIL, int input_ROIR,double input_dt,double input_currtime = 0);
    ~ActivityCounter();

    void setTimeStep(double x){dt = x;}
    double getTimeStep(){return dt;}
    /// set currtime (left boundary of current time interval)
    void setTime(double x){currtime = x;}

    void resetLROI(int x){leftROI = x;}
    void resetRROI(int x){rightROI = x;}
    int* getROI(){int out[2] = {leftROI,rightROI}; return out;}

    //Returns the activity and corresponding times as vectors.
    QPair<std::vector<double>,std::vector<qint64> > getPlotData();

    //Incorporates new data into the counter.
    //  numE is the length of e and t.
    //  e is the event energy or bin number. (same units as leftROI and rightROI)
    //  t is the timestamp of the corresponding element in e  (same units as dt, currtime)
    void addData(int numE, double* e, qint64* t);

    //Clears the stored data.  Note that this does not reset the counter for the current
    //  time window, nor does it reset the current time variable.
    void clearData(){rate.resize(0); time.resize(0);}

private:
    int leftROI;
    int rightROI;

    double dt;
    double currtime; //Arbitrary time counter, should be in same units as dt and your input timestamps.
    int count; //# of counts in the current time interval (between currtime and currtime+dt)

    std::vector<double> rate; //List of activity data points
    std::vector<qint64> time; //Times associated with rate vector.  (Center time of each time interval.)

};

#endif // ACTIVITYCOUNTER_H
