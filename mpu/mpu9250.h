#ifndef _MPU9250_H_
#define _MPU9250_H_

/* Class Mpu9250 implements MPU device based on Invensense MPU-9250 chip 
   It uses on-chip FIFO and interrupts.
   
   MPU9250 chip:
   - ACCEL sensor: uses right-handed coordinate system (printed on PCB)
   - GYRO sensor: uses the same axes as ACCEL and positive value means positive rotation
                   (positive rotation rotates to the right when viewed in the direction of the rotation axis)
   AK8963 chip:
   - HALL sensor: X-axis and Y-axis are swapped (with respect to MPU9250)
                  Z-axis has the opposite direction (with respect to MPU9250)
                  HALL is transformed into MPU9250 coordinates in loadBaseData()
*/

#include "mpu.h"
#include "interruptible.h"
#include "FVector.h"
#include <cstdint>

#define LockHW unique_lock<mutex> lock(hwmtx);

class Bus;
class Mpu9250 : public Mpu, Interruptible {
  public:
    enum AccelRange	{ AR_2G, AR_4G, AR_8G, AR_16G};
    enum GyroRange	{ GR_250DPS, GR_500DPS, GR_1000DPS, GR_2000DPS};
    enum AccelFilter	{ AF_460HZ, AF_184HZ, AF_92HZ, AF_41HZ, AF_20HZ, AF_10HZ, AF_5HZ};
    enum GyroFilter	{ GF_460HZ, GF_184HZ, GF_92HZ, GF_41HZ, GF_20HZ, GF_10HZ, GF_5HZ};
    enum SampleRate	{ SR_1000HZ, SR_500HZ, SR_333HZ, SR_250HZ, SR_200HZ, SR_166HZ, 
			  SR_125HZ=7, SR_100HZ=9, SR_77HZ=12, SR_66HZ=14, SR_50HZ=19,
			  SR_40HZ=24, SR_33HZ=29, SR_30HZ=32, SR_25HZ=39, SR_20HZ=49,
			  SR_17HZ=59, SR_10HZ=99, SR_8HZ=124, SR_5HZ=199, SR_4HZ=249};

  private:
    const unsigned fifoItemSize = 3*2		// ACCEL X+Y+Z x 16 bits
                                + 3*2		// GYRO X+Y+Z x 16 bits
                                + 3*2		// MAG X+Y+Z x 16 bits
                                + 1;		// ST2

    const unsigned fifoSize = 0x1FFF;		// the total size of on-chip FIFO

    Bus *bus;					// communication channel (i2c or spi)
    uint8_t modeAK = -1;			// the current mode in AK8963 CNTL1 register
    MpuData baseData;				// measured data in standard units (m.s-2, rad/s, uT)
    float rangeScaleAccel, rangeScaleGyro;	// conversion factor from raw integer data to baseData
    FVector rangeScaleMag;			// conversion factor from raw integer data to baseData
    mutex hwmtx;				// exclusive access to the device

    void writeRegister(uint8_t reg, uint8_t data, uint8_t verifyMask=0xFF);	// write to MPU-9250 register
    void readRegisters(uint8_t reg, unsigned count, uint8_t* buf);		// read MPU-9250 registers
    uint8_t readRegister(uint8_t reg);						// read a single MPU-9250 register
    void scheduleAKTransfer(uint8_t reg, uint8_t flag);				// setup MPU-9250 to proxy command to AK8963
    void writeAKRegister(uint8_t reg, uint8_t data);				// write to AK8963 register
    uint8_t readAKRegister(uint8_t reg);					// read AK8963 register
    void setAKMode(uint8_t mode);						// set AK8963 mode
    void readAKAdjustment();							// read built-in magnetometer adjustment
    void waitForAK(int acceptableMS=10); 					// wait until AK8963 is ready to communicate
    uint8_t readMPUid();							// read MPU chip ID
    uint8_t readAKid();								// read AK chip ID

    /* set sensitivity and bandwith filtering ranges without locking */
    void setAccelRangeNL(AccelRange range);
    void setGyroRangeNL(GyroRange range);
    void setAccelFilterNL(AccelFilter filter);
    void setGyroFilterNL(GyroFilter filter);
    void setSampleRateNL(SampleRate rate);
    
    virtual MpuData loadBaseData();						// load and return current values from HW
    void checkDataPresence();							// check how many measured data are available in HW fifo
    virtual bool isHWFull(unsigned vol) { return vol + 2*fifoItemSize >= fifoSize; }	// near HW fifo overflow?
    virtual bool isHWDataAvailable(unsigned vol) { return vol >= fifoItemSize; }	// is data available when vol bytes in FIFO?

    void init();	// initialize MPU+AK registers    
    void start();	// start data measurement	(could be public)
    void stop();	// stop data mesaurement	(could be public)
    void interrupt();	// interrupt routine

  public:
    Mpu9250(Bus *bus, const InterruptRegistrator &intReg, const string &caliFileName);

    /* set sensitivity and bandwith filtering ranges */
    void setAccelRange(AccelRange range)	{ LockHW setAccelRangeNL(range); }
    void setGyroRange(GyroRange range)		{ LockHW setGyroRangeNL(range); }
    void setAccelFilter(AccelFilter filter)	{ LockHW setAccelFilterNL(filter); }
    void setGyroFilter(GyroFilter filter)	{ LockHW setGyroFilterNL(filter); }
    void setSampleRate(SampleRate rate)		{ LockHW setSampleRateNL(rate); }
    
};

#endif
