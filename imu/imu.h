#ifndef __IMU_H__
#define __IMU_H__

#include <mutex>
#include <thread>
#include "FVector.h"
#include "QRotation.h"
#include "mpu.h"


class IMU {
 private:
  static const FVector G0;		// direction of gravitation in fixed coordinates
  static const FVector M0;		// direction to magnetic pole in fixed coordinates
  static constexpr int Hz=100;		// measurement frequency
  static constexpr float dT=1.0f/Hz;	// time difference
  
  bool enMCor=true, 			// do correction of direction using compass
       enGCor=true;			// dp correction of direction using gravitation
  bool running;
       
  QRotation orientation;		// orientation of sensor in fixed coordinates

  float correctionG = 0.005f;		// influence of G-correction
  float correctionM = 0.001f;		// influence of M-correction

  int dbg;				// debug level
  Mpu *mpu;				// MPU sensor
  mutable std::mutex dataMutex;
  thread imuThread;			// thread calculating the orientation
  int lastDataSeq=0;			// last data seq-id from MPU

  void calculate();
  void operator()();
  
 public:
  IMU(Mpu *m) : mpu(m) {}
  ~IMU() { stop(); }
  
  void start();
  void stop();
  void resetRotation() 			{ orientation=QRotation(); }
  QRotation getRotation() const;
  void enableMagCorrection()		{ enMCor=true; }
  void disableMagCorrection()		{ enMCor=false; }
  void enableGravityCorrection()	{ enGCor=true; }
  void disableGravityCorrection()	{ enGCor=false; }

};

#endif
