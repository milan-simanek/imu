#include <iostream>
#include <iomanip>
#include "FVector.h"
#include "mpu.h"
#include "calibrator.h"
#include "cali-simple.h"

using namespace std;

bool CaliAccelSimple::calibrate() {
  constexpr int samples = 200;
  constexpr float G_thr=Mpu::G0 * 0.8;	// 80% G
  string s="      ";
  FVector min, max, sum;

  cout << "Calibration requires to rotate the sensor and keep it standing on all 6 sides." << endl;
  for(int count=20; count-- && s!="######";) {
    cout << "[" << s << "] - rotate sensor to stand on another side and press enter when not moving" << endl;
    getchar();
    cout << "... measuring data ..." << endl;
    // take samples and find min / max 
    sum=FVector();
    for (int i=0; i<samples; i++) sum+=getData().accel;
    sum/=samples;
    max.raiseUp(sum);
    min.raiseDown(sum);

    if (max.x >  G_thr) s[0]='#';
    if (max.y >  G_thr) s[1]='#';
    if (max.z >  G_thr) s[2]='#';
    if (min.x < -G_thr) s[3]='#';
    if (min.y < -G_thr) s[4]='#';
    if (min.z < -G_thr) s[5]='#';
  }
  if (s!="######") { cerr << "Calibration failed." << endl; return false; }
  cout << "OK, calibration finished." << endl;
  compensateAccel((max+min)/2, (max-min)/Mpu::G0/2);
  return true;  
}

bool CaliGyroSimple::calibrate() {
  constexpr int samples = 200;
  FVector sum;

  cout << "Gyroscope calibration requires to keep sensor in a fixed position." << endl;
  for (int i=0; i<samples; i++) {
    if (!(i&7)) cout << "." << flush;
    sum+=getData().gyro;
  }
  cout << "OK, calibration finished." << endl;
  compensateGyro(sum/samples, FVector(1.0f));
  return true;
}

void CaliMagSimple::calculateOne(int dir) {
  MpuData d=getData();
  average(d.mag);
  float f;
  if (dir>>1 == 0) f=d.mag.x; else cout << "\t" << d.mag.x;
  if (dir>>1 == 1) f=d.mag.y; else cout << "\t" << d.mag.y;
  if (dir>>1 == 2) f=d.mag.z; else cout << "\t" << d.mag.z;
  if (f<0.0f) dir++;
  cout << (dir&1 ? "\trotate sensor to the opposite direction!" : 
                   "\t                                        ") << "\r" << flush;
}

bool CaliMagSimple::calibrate() {
  const int samples = 100;
  FVector max, min;

  cout << endl << "This calibration method requires to rotate the sensor and find the extreme in magnetic field in all 6 directions." << endl;
  for(int dir=0; dir<6; dir++) {
    for(;;) {
      cout << "calibrating sensor in direction: " << (dir&1 ? '-' : ' ')<< (char)('X'+(dir>>1)) << endl;
      cout << "Try to rotate the sensor in space to get both numbers as close to zero as possible (and press any key):" << endl;    
      cout << fixed << setprecision(4) << showpos;
      do calculateOne(dir); while (!keyPressed());
      cout << endl << "measuring - do not rotate sensor!" << endl;
      for(int i=0; i<samples; i++) calculateOne(dir);
      FVector mag2=avg*avg;
      float m=mag2.x;
      if (m<mag2.y) m=mag2.y;
      if (m<mag2.z) m=mag2.z;
      m*=100.0f/(mag2.x+mag2.y+mag2.z);
      cout << endl << setprecision(4) << "magnetometer aligned to " << m << "%" << endl;
      if (m>98) break;
      cout << "Calibration failed for this direction. Please try again." << endl;
    };
    cout << internal << setprecision(6);
    cout << endl << "Direction " << (dir+1) << "/" << 6 << " done." << endl;
    max.raiseUp(avg);
    min.raiseDown(avg);
  }
  cout << "OK, calibration finished." << endl;
  min/=avgCoef; max/=avgCoef;
  FVector scale=(max-min)/2;
  float avgscale=(scale.x+scale.y+scale.z)/3;
  compensateMag((max+min)/2, scale/avgscale);
  return false;
}
