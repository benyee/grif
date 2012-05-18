#include "ActivityCounter.h"

ActivityCounter::ActivityCounter(int input_ROIL, int input_ROIR,double input_dt,double input_currtime){
    leftROI = input_ROIL;
    rightROI = input_ROIR;

    dt = input_dt;
    currtime = input_currtime;

    count = 0;
}

ActivityCounter::~ActivityCounter(){}


void ActivityCounter::addData(int numE, double* e, qint64* t){
    int i = 0;

    //Loop over raw data:
    while(i<numE){
        if(t[i]< currtime+dt){
            //Count # of hits within our ROI in the current time interval:
            if(e[i] > leftROI && e[i]<rightROI ){
                count++;
            }
            i++;
        }
        else if(t[i] > currtime){
            //Log the count rate and central time of the time interval:
            rate.push_back((double)count/dt);
            time.push_back((qint64)(currtime+dt/2));
            //Reset counter, move to the next time interval.
            count = 0;
            currtime += dt;
        }
        else{i++;}
    }
    std::cout<<"currtime:"<<currtime<<"..."<<t[1]<<"hello"<<t[2]<<std::endl;
}


QPair<std::vector<double>,std::vector<qint64> > ActivityCounter::getPlotData(){
    QPair<std::vector<double>,std::vector<qint64> > out;
    out.first = rate;
    out.second = time;
    return out;
}
