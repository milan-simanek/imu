#define DBG_REGS

#include <unistd.h>
#include <string>
#include <fstream>
#include "mpu.h"

#define MAX_DATA_BURST	2	// max. number of measurements read in a single interrupt


void Mpu::calculateValues(const MpuData &baseData) {
  unique_lock<mutex> Lock(mtx);
  data.accel = baseData.accel * caliScaleAccel - caliBiasAccel;
  data.gyro  = baseData.gyro  * caliScaleGyro  - caliBiasGyro; 
  data.mag   = baseData.mag   * caliScaleMag   - caliBiasMag;
  data.seq   = baseData.seq;
  barrier=nextBarrier; nextBarrier=0;
  newData.notify_all();
}

Mpu::Mpu(const string &caliFileName) {
  resetCaliData();
  if (caliFileName.empty()) return;
  ifstream caliFile=ifstream(caliFileName);
  readCaliData(caliFile);
}

void Mpu::resetCaliData() {
  unique_lock<mutex> Lock(mtx);
  caliScaleAccel=caliScaleGyro=caliScaleMag=FVector(1.0f, 1.0f, 1.0f);
  caliBiasAccel=caliBiasGyro=caliBiasMag=FVector();
}
    
void Mpu::readCaliData(istream &is) {
  unique_lock<mutex> Lock(mtx);
  while (is.good()) {
    string s;
    is >> s;
    if (s=="AS") is >> caliScaleAccel;
    if (s=="AB") is >> caliBiasAccel;
    if (s=="GS") is >> caliScaleGyro;
    if (s=="GB") is >> caliBiasGyro;
    if (s=="MS") is >> caliScaleMag;
    if (s=="MB") is >> caliBiasMag;
  }
}

void Mpu::writeCaliData(ostream &os) {
  unique_lock<mutex> Lock(mtx);
  os << "AS " << caliScaleAccel << endl;
  os << "AB " << caliBiasAccel << endl;
  os << "GS " << caliScaleGyro << endl;
  os << "GB " << caliBiasGyro << endl;
  os << "MS " << caliScaleMag << endl;
  os << "MB " << caliBiasMag << endl;
}

unsigned Mpu::getLastDataSeq() {
  return data.seq;	// need not to lock; worst case: if seq changes during copying then it is recognized as different
}

MpuData Mpu::getDataAfter(unsigned lastSeq, bool bar, bool nextBar) {
  unique_lock<mutex> Lock(mtx);
  if (lastSeq==data.seq) newData.wait(Lock, [this, lastSeq ]{ return data.seq!=lastSeq; });
  if (nextBar) nextBarrier++;
  if (bar && barrier) if (!--barrier) {
    MpuData d=data;
    Lock.unlock();
    checkHWData();
    return d;
  }
  return data;
}

void Mpu::setPendingDataVolume(unsigned v) {
  unique_lock<mutex> Lock(mtx);
  pendingDataVolume=v;
}
void Mpu::updatePendingDataVolume(int diff) {
  unique_lock<mutex> Lock(mtx);
  pendingDataVolume+=diff;
}
unsigned Mpu::getPendingDataVolume() {
  unique_lock<mutex> Lock(mtx);
  return pendingDataVolume;
}


void Mpu::checkHWData() {	// no lock hold
  for(int retry=0; retry<MAX_DATA_BURST; retry++) {
    {  unique_lock<mutex> Lock(mtx);
       if (hwLoading) return;					// another thread is already loading....
       if (!isHWDataAvailable(pendingDataVolume)) return;	// still no data in HW
       if (barrier && !isHWFull(pendingDataVolume)) return;	// some consumer cannot miss current data
       hwLoading=true;
    }
    calculateValues(loadBaseData());
    hwLoading=false;
  }
}

bool Mpu::isHWFull(unsigned vol) {
  return vol>0;
}
bool Mpu::isHWDataAvailable(unsigned vol) {
  return vol>0;
}
