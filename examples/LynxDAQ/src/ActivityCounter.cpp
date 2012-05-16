#include "ActivityCounter.h"

ActivityCounter::ActivityCounter(int input_ROIL, int input_ROIR,double input_currtime,double input_dt){
    leftROI = input_ROIL;
    rightROI = input_ROIR;

    dt = input_dt;
    currtime = input_currtime;
}

ActivityCounter::~ActivityCounter(){}

void ActivityCounter::addData(std::vector<double> e, std::vector<double> t){
    energy.insert(energy.end(),e.begin(),e.end());
    timestamp.insert(timestamp.end(),t.begin(),t.end());
    computeRate();
}

void ActivityCounter::computeRate(){
    std::vector<double>::iterator itr_e;
    std::vector<double>::iterator itr_t;

    itr_e = energy.begin();
    itr_t = timestamp.begin();
    int count = 0;

    //Loop over raw data:
    while(itr_e < energy.end()){
        if(*itr_t < currtime+dt){
            //Count # of hits within our ROI in the current time interval:
            if(*itr_e > leftROI && *itr_e<rightROI ){
                count++;
            }
            itr_e++;
            itr_t++;
        }
        else if(*itr_t > currtime){
            //Log the count rate and central time of the time interval:
            rate.push_back((double)count/dt);
            time.push_back(currtime+dt/2);
            //Reset counter, move to the next time interval.
            count = 0;
            currtime += dt;
        }
        else{
            itr_e++;
            itr_t++;
        }
    }
}

void ActivityCounter::clearData(){
    energy.resize(0);
    timestamp.resize(0);
    rate.resize(0);
    time.resize(0);
}

QPair<std::vector<double>,std::vector<double> > ActivityCounter::getRawData(){
    QPair<std::vector<double>,std::vector<double> > out;
    out.first = energy;
    out.second = timestamp;
    return out;
}
QPair<std::vector<double>,std::vector<double> > ActivityCounter::getPlotData(){
    QPair<std::vector<double>,std::vector<double> > out;
    out.first = rate;
    out.second = time;
    return out;
}
