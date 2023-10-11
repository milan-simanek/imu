#ifndef _CALI_SIMPLE_H_
#define _CALI_SIMPLE_H_

#include "calibrator.h"

class CaliAccelSimple : public MpuCalibrator {
  public:
    using MpuCalibrator::MpuCalibrator;
    virtual bool calibrate();
};

class CaliGyroSimple : public MpuCalibrator {
  public:
    using MpuCalibrator::MpuCalibrator;
    virtual bool calibrate();
};

class CaliMagSimple : public MpuCalibrator {
    static constexpr int avgCoef = 8;         			// floating average factor
    FVector avg;						// floating average
  public:
    using MpuCalibrator::MpuCalibrator;
    virtual bool calibrate();
    void calculateOne(int dir);
    void average(const FVector &sample) { 
      constexpr float avgFading = 1.0f-1.0f/avgCoef;
      avg*=avgFading; avg+=sample; 
    }
};

#endif
