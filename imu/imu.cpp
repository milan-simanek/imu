#include "FVector.h"
#include "Quaternion.h"
#include "mpu.h"
#include "imu.h"

const FVector IMU::G0=FVector(0,0,1);	// direction of gravitation in fixed coordinates
const FVector IMU::M0=FVector(0,1,0);	// direction to magnetic pole in fixed coordinates
const bool use_barrier = true;		// IMU requires each measurement from MPU

void IMU::calculate() {
  MpuData md=mpu->getDataAfter(lastDataSeq,  use_barrier, use_barrier);
  
  QRotation R(md.gyro*dT);		// rotation step
  R=R*orientation;			// total rotation A*B <=> rotate B and then follow rotate A

  /* G-correction */
  FVector g=md.accel;			// detected G-direction in sensor coordinates
  FVector g0 = G0.rotateBy(R);		// required G-direction in sensor coordinates
  QRotation gCorrection(g,g0);		// correction rotation in sensor coordinates
  gCorrection=gCorrection*correctionG;	// influence level
  if (enGCor) R=gCorrection*R;		// apply

  /* M-correction */  
  FVector m=md.mag;			// detected magnetic vector in sensor coordinates
  FVector m0 = M0.rotateBy(R);		// required magnetic field direction in sensor coordinates
  m=g0.crossProduct(m).crossProduct(g0);// projection onto the horizontal plane
  QRotation mCorrection(m, m0);		// correction rotation in sensor coordinates
  mCorrection=mCorrection*correctionM;	// influence level
  if (enMCor) R=mCorrection*R;		// apply

  lastDataSeq=md.seq;
    
  const std::lock_guard<std::mutex> lock(dataMutex);
  orientation=R;
}

void IMU::operator()() {		// IMU calculating thread body
  running=true;
  lastDataSeq=mpu->getDataAfter(mpu->getLastDataSeq(), false, use_barrier).seq;
  while(running) {
    calculate();
  }
  mpu->getDataAfter(lastDataSeq, use_barrier, false);
}

void IMU::start() {
  imuThread=thread([](IMU *p){(*p)();}, this);
}
void IMU::stop() {
  running=false;
  imuThread.join();
}

QRotation IMU::getRotation() const {
  const std::lock_guard<std::mutex> lock(dataMutex);
  return orientation;
}

