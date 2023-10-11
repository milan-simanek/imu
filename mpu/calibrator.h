#ifndef _CALIBRATOR_H_
#define _CALIBRATOR_H_

#include "mpu.h"
#include <sys/select.h>
#include <unistd.h>

class MpuCalibrator {
  private:
    Mpu	*mpu;
    unsigned seq;

  public:
    MpuCalibrator(Mpu *m) : mpu(m) { seq=mpu->getLastDataSeq(); }
    
    virtual bool calibrate() = 0;		// returns TRUE if calibration is successfull
    MpuData getData() { MpuData d=mpu->getDataAfter(seq, false, false); seq=d.seq; return d; }
    void compensateAccel(const FVector &bias, const FVector &scale) const { mpu->compensateAccel(bias, scale); }
    void compensateGyro(const FVector &bias, const FVector &scale) const { mpu->compensateGyro(bias, scale); }
    void compensateMag(const FVector &bias, const FVector &scale) const { mpu->compensateMag(bias, scale); }
    bool keyPressed() {
       fd_set fdsr;
       char c;
       struct timeval tv = {0,0};
       FD_ZERO(&fdsr);
       FD_SET(0, &fdsr);
       if (1!=select(1, &fdsr, NULL, NULL, &tv)) return false;
       read(0, &c, 1);
       return true;
    }
};








#endif
