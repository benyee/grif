#include "ActivityCounter.h"

ActivityCounter::ActivityCounter(int input_ROIL, int input_ROIR,int input_eventsPerBin,double input_dt){
    leftROI = input_ROIL;
    rightROI = input_ROIR;

    eventsPerBin = input_eventsPerBin;
    dt = input_dt;
}
ActivityCounter::~ActivityCounter(){}


ActivityCounter::getBinRate(){
    if(binMode){return dt;}
    return (double)eventsPerBin;
}
ActivityCounter::setBinRate(double x){
    eventsPerBin = (int)x;
    dt = x;
}

void addToList(vector<double> e, vector<double> t){
    energy.insert(energy.end(),e.begin(),e.end());
    timestamp.insert(timestamp.end(),t.begin(),t.end());
}
