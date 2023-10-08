#ifndef _MPU_H_
#define _MPU_H_

/* class Mpu represents a generic motion processing unit sensor. The public interface provides
   a function to get current motion data: 3-axes rotation in rad/s, 3-axes acceleration in m/s²
   and 3-axes magnetometer in μT.
   
   The data is atomically (thread safe) copied in data structure MpuData, to be consistent.
   
   Each data has a sequence number. A consumer of MPU data calls getDataAfter(...) which blocks
   until there is a new data available. The consumer may miss the data if it calls getDataAfter(...)
   too late. To prevent data miss the consumer can provide nextBarrier=true parameter resulting next 
   time MPU holds until getDataAfter(...) with barrier=true is called.
   
   Mpu class allows data calibration.
   
   Mpu uses right-handed coordinate system 
   (i.e. the X-axis points to the right, the Y-axis points forward, and the Z-axis points up)

   Positive rotation in a right-handed system about the Z-axis rotates the X-axis toward the Y-axis
   (i.e. rotates to the right when viewed in the direction of the rotation axis)

*/
   

#include <iostream>
#include <mutex>
#include <condition_variable>
#include <string>

#include "FVector.h"

using namespace std;

struct MpuData {	// data structure to hold the result of MPU measurement
  unsigned seq = 0;
  FVector accel,	// m.s-2
          gyro, 	// rad/s
          mag;		// μT
};

class Mpu {
  public:
    const float G0=9.80665f;	// average gravity on Earth
    // total magnetic field on Earth: 25-65 μT
    //    Czech Republic: magnetic inclination +65° (total 43μT = 20μT horizontally + 27μT vertically)
    
  private:
    mutex mtx;					// mutex for locking MPU-local data structures
    condition_variable newData;			// consumers of data should wake up
    bool hwLoading=false;			// when true a thread is downloading data from HW
    MpuData data;				// calculated data
    unsigned barrier=0, nextBarrier=0;		// how many consumers cannot miss data
    unsigned pendingDataVolume=0;		// amount of data in HW waiting to be loaded (HW dependent units)

    FVector caliScaleAccel, caliBiasAccel, 	// calibration corrections
            caliScaleGyro, caliBiasGyro,
            caliScaleMag, caliBiasMag;

    void calculateValues(const MpuData &baseData);	// calculate data from baseData (apply calibration)
    
  public:
    Mpu(const string &caliFileName);
    
    /* calibration methods */
    void resetCaliData();    			// reset calibration to transparent values (Scale=1.0, Bias=0.0)
    void readCaliData(istream &is);
    void writeCaliData(ostream &os);

    /* methods for MPU data consumers */
    unsigned getLastDataSeq();			// the last data sequence number available
    MpuData getDataAfter(unsigned lastSeq, bool barrier=false, bool nextBarrier=false);	// get a copy of new data after lastSeq
    
  protected:                        
    /* interface to lower layer (hw specific class) */
    void setPendingDataVolume(unsigned v);
    void updatePendingDataVolume(int diff);
    unsigned getPendingDataVolume();
    void checkHWData();				// check for new data in HW, and load it if possible
    virtual MpuData loadBaseData() = 0;		// load and return current values from HW
    virtual bool isHWFull(unsigned vol); 	// is vol amount of data in HW FIFO risky for overflow?
    virtual bool isHWDataAvailable(unsigned vol); // is vol enough to be able read a new data set?
};

#endif
