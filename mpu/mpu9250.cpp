#include "mpu9250.h"
#include "bus.h"
#include "tstamp.h"
#include <unistd.h>

//#define DBG_REGS_MPU
//#define DBG_REGS_AK

#include "registers-mpu.h"
#include "registers-ak.h"

#define AK_ADDR 0x0C		// AK8963 I2C address
//#define MAGNETOMETER_16BIT	// if defined, magnetometer precision on 16 bits (otherwise 14 bits)

#define ms *1000			// 1ms=1000μs
#define TO_MPU_RESET		10 ms	// time required to reset MPU-9250 [μs]
#define TO_AK_RESET		10 ms	// time required to reset AK8963 [μs]
#define TO_AK_MODE_CHANGE	1 ms	// time required to change mode of AK8963 [μs]


struct data16b {			// 16-bit data read from HW
  union {
    unsigned char byte[2];
    int16_t short_int;
  };
  // data16b is sometimes interpretted as little endian and sometimes as big endian
#if __BYTE_ORDER == __BIG_ENDIAN
  int16_t asBE() { return short_int; }
  int16_t asLE() { return byte[0] | byte[1]<<8; }
#else
  int16_t asLE() { return short_int; }
  int16_t asBE() { return byte[0]<<8 | byte[1]; }
#endif
};

struct data16bVector {
  data16b x,y,z;
  FVector asLE() { return FVector(x.asLE(), y.asLE(), z.asLE()); }
  FVector asBE() { return FVector(x.asBE(), y.asBE(), z.asBE()); }
};

struct rawData_t {	// raw data read from MPU-9250
  data16bVector accel;
  data16bVector gyro;
  data16bVector mag;
  uint8_t status;
};

void Mpu9250::writeRegister(uint8_t reg, uint8_t data, uint8_t verifyMask) {
#ifdef DBG_REGS_MPU
  cerr << "writeRegister(" 
       << regMPUname[reg] << "{" << dec << (int)reg << hex << "=0x" << (int)reg << "}, " << regMPUDataString(reg, data) << ")" << endl;
#endif
  uint8_t buf[2];
  buf[0]=reg;
  buf[1]=data;
  bus->beginTransaction(bttWrite);
  bus->send(buf, 2);
  bus->endTransaction();
  
  if (!verifyMask) return;
  uint8_t verify=readRegister(reg);
  if ((verify ^ data)&verifyMask) {
    cerr << "ERROR: cannot write to MPU9250 register[" << regMPUname[reg] << "{" << (int)reg << "}]: data written " << (int)data 
         << " but data verified " << (int)(verify) << endl;
    abort();
  }
}

void Mpu9250::readRegisters(uint8_t reg, unsigned count, uint8_t* buf) {
#ifdef DBG_REGS_MPU
  cerr << "readRegisters(" << (regMPUname[reg]+"{"+to_string(reg)+"}, "+to_string(count)+")                      ").substr(0,28);
#endif
  bus->beginTransaction(bttWrite);
  bus->send(&reg, 1);
  bus->continueTransaction(bttRead);
  bus->receive(buf, count);
  bus->endTransaction();
#ifdef DBG_REGS_MPU
  cerr << "->";
  if (count==1)
    cerr << regMPUDataString(reg, buf[0]);
  else for(unsigned i=0; i<count; i++) cerr << uppercase << hex << (int)(buf[i]) << " ";
  cerr << endl;
#endif
}

uint8_t Mpu9250::readRegister(uint8_t reg) {
  uint8_t data;
  readRegisters(reg, 1, &data);
  return data;
}
  

void Mpu9250::scheduleAKTransfer(uint8_t reg, uint8_t flag) {
  writeRegister(REGMPU::I2C_SLV4_ADDR, AK_ADDR<<BIT_I2C_ID_4 | flag);
  writeRegister(REGMPU::I2C_SLV4_REG, reg);
  writeRegister(REGMPU::I2C_SLV4_CTRL, I2C_SLV4_EN, 0xFF-I2C_SLV4_EN);
  while (readRegister(I2C_MST_STATUS) & I2C_SLV4_DONE) usleep(0);
}

void Mpu9250::writeAKRegister(uint8_t reg, uint8_t data) {
  // use slave4
#ifdef DBG_REGS_AK
  cerr << "  writeAKRegister(" << regAKname[reg] << "=" << dec << (int)reg <<hex << "=0x" << (int)reg << ", " << regAKDataString(reg, data) << ")" << endl;
#endif
  writeRegister(REGMPU::I2C_SLV4_DO, data);
  scheduleAKTransfer(reg, 0/* 0=write */);
}

uint8_t Mpu9250::readAKRegister(uint8_t reg) {
  // use slave4
  scheduleAKTransfer(reg, I2C_SLV4_RNW);
  return readRegister(REGMPU::I2C_SLV4_DI); 
}

#ifdef MAGNETOMETER_16BIT
#   define MAG16_FLAG	BIT
#else
#   define MAG16_FLAG	0
#endif

void Mpu9250::setAKMode(uint8_t mode) {
  if (modeAK==mode) return;	// AK8963 is already in that mode
  // when changing the mode we should frist change mode to POWER_DOWN
  if (modeAK!=MODE_POWER_DOWN && mode!=MODE_POWER_DOWN) setAKMode(MODE_POWER_DOWN);
  for(int retry=0; retry<10; retry++) {
    writeAKRegister(REGAK::CNTL1, MAG16_FLAG | (mode<<BIT_MODE)); // always 16-bits operation
    modeAK=mode;
    usleep(TO_AK_MODE_CHANGE);
    if ((MAG16_FLAG|(mode<<BIT_MODE))==readAKRegister(REGAK::CNTL1)) return;
    cerr << "Fail to set AK8963 mode 0x" << hex << (int)mode << dec << " retrying..." << endl;  
  } while(1);
  abort();
}

void Mpu9250::readAKAdjustment() {
  const float max_uT=4912.0f;
  const uint16_t max_value=0x7FF8; /* at 16-bits operations */
  // H = value * max_uT/max_value * ((ASA-128)*0.5/128+1) = value * rangeScaleMag;
  // rangeScaleMag = max_uT/max_value * ((ASA-128)*0.5/128+1) = ASA*factor - shift;
  const float factor = max_uT/max_value/256;
  const float shift = max_uT/max_value/2;
  setAKMode(MODE_FUSE_ROM);
  rangeScaleMag=FVector(readAKRegister(REGAK::ASAX)*factor-shift,
                        readAKRegister(REGAK::ASAY)*factor-shift,
                        readAKRegister(REGAK::ASAZ)*factor-shift);
  setAKMode(MODE_POWER_DOWN);
}

void Mpu9250::waitForAK(int acceptableMS) {
  uint8_t id;
  const int maxDelayMS=1000;
  for(int i=0;i<maxDelayMS;i++) {
    id=readAKid();
    if (id>0 && id<0xFF) {
      if (i>=acceptableMS) cerr << "Warning: waitForAK() waited " << i << " ms." << endl;
      return;
    }
  }
  cerr << "wait for AK failed." << endl;
  abort();
}

uint8_t Mpu9250::readMPUid() { return readRegister(REGMPU::WHO_AM_I); }              // read MPU chip ID
uint8_t Mpu9250::readAKid()  { return readAKRegister(REGAK::WIA); }                  // read AK chip ID


void Mpu9250::setAccelRangeNL(AccelRange range) {
  writeRegister(REGMPU::ACCEL_CONFIG, range<<BIT_ACCEL_FS_SEL);
  rangeScaleAccel = G0/(16384>>range);
}
void Mpu9250::setGyroRangeNL(GyroRange range) {
  const float factor=M_PI/180*250;
  writeRegister(REGMPU::GYRO_CONFIG, range<<BIT_GYRO_FS_SEL);
  rangeScaleGyro = factor/(32768>>range);
}
void Mpu9250::setAccelFilterNL(AccelFilter filter) {
  writeRegister(REGMPU::ACCEL_CONFIG2, filter<<BIT_A_DLPFCFG);
}
void Mpu9250::setGyroFilterNL(GyroFilter filter) {
  writeRegister(REGMPU::CONFIG, filter<<BIT_DLPF_CFG);
}
void Mpu9250::setSampleRateNL(SampleRate rate) {
  // set AK8963 to Power Down
  setAKMode((rate<SR_8HZ) /* means higher frequency */ ? MODE_CONT_MEASURE_100HZ : MODE_CONT_MEASURE_8HZ);
  writeRegister(REGMPU::SMPLRT_DIV, rate);
}


MpuData Mpu9250::loadBaseData() {
  rawData_t rawData;
  baseData.seq++;
  {
    unique_lock<mutex> Lock(hwmtx);
    readRegisters(REGMPU::FIFO_R_W, fifoItemSize, (unsigned char*)&rawData);
    updatePendingDataVolume(-fifoItemSize);
  }
  baseData.accel = rawData.accel.asBE() * rangeScaleAccel;
  baseData.gyro  = rawData.gyro.asBE()  * rangeScaleGyro;
  if (rawData.status & HOFL) {
    cerr << "warning: magnetometer overflow" << endl;
    return baseData;	// magnetometer overflow, data not valid, keep old data
  }
  FVector magRot = rawData.mag.asLE() * rangeScaleMag; 		// AK8963 has rotated coordinates for magnetometer
  if (!(rawData.status & BITM)) magRot*=4.0f;			// 14-bits operation
  baseData.mag = FVector(magRot.y, magRot.x, -magRot.z);	// transformation to MPU-9250 coordinates
                        
  return baseData;
}

void Mpu9250::checkDataPresence() {
  unique_lock<mutex> HWLock(hwmtx);
  data16b buf;
  unsigned bytes;
  readRegisters(REGMPU::FIFO_COUNTH, 2, buf.byte);
  bytes = buf.asBE() & 0x1FFF;
  setPendingDataVolume(bytes);
}

Mpu9250::Mpu9250(Bus *b, const InterruptRegistrator& intReg, const string &caliFileName) : Mpu(caliFileName), bus(b) {
  intReg.enableInt(this);
  init();
  start();
}

void Mpu9250::init() {
  unique_lock<mutex> lock(hwmtx);
  uint8_t id;
  /* reset MPU9250 */
  writeRegister(REGMPU::PWR_MGMT_1, H_RESET, 0);				// hard reset
  usleep(TO_MPU_RESET);
  writeRegister(REGMPU::USER_CTRL, FIFO_RST|I2C_MST_RST|SIG_COND_RST, 0);	// reset FIFO, I2C-master, signal path
  usleep(TO_MPU_RESET);
  /* setup connection to AK8963 */
  writeRegister(REGMPU::USER_CTRL, I2C_MST_EN);					// enable I2C master
  writeRegister(REGMPU::I2C_MST_CTRL, I2C_MST_CLK_421KHZ);			// I2C master clock
  /* reset AK8963 */
  setAKMode(MODE_POWER_DOWN);							// AK: power down
  writeAKRegister(REGAK::CNTL2, SRST);						// soft reset
  for(;;) {
    usleep(TO_AK_RESET);
    if (!readAKRegister(REGAK::CNTL2)) break;
    cerr << "AK8963 Reset" << endl;
  };
  /* check device ID */
  id=readMPUid();
  switch (id) {
  case 113: cerr << "MPU-9250 detected." << endl; break;
  case 115: cerr << "MPU-9255 detected." << endl; break;
  default: cerr << "ERROR: unknown MPU ID=" << (int)id << endl;
  }
  waitForAK();
  id=readAKid();
  switch (id) {
  case 72: cerr << "AK8963 magnetometer detected." << endl; break;
  default: cerr << "ERROR: unknown magnetometer ID=" << (int)id << endl;
  }
  /* prepare and self-test */
  readAKAdjustment();

  /* set default sensitivity */
  writeRegister(REGMPU::PWR_MGMT_1, CLKSEL_BEST);	// clock source
  writeRegister(REGMPU::PWR_MGMT_2, 0);			// enable ACCEL+GYRO

  setAccelRangeNL(AccelRange::AR_16G);
  setGyroRangeNL(GyroRange::GR_2000DPS);
  setAccelFilterNL(AccelFilter::AF_184HZ);
  setGyroFilterNL(GyroFilter::GF_184HZ);
  setSampleRateNL(SampleRate::SR_100HZ);

  writeRegister(REGMPU::PWR_MGMT_1, CLKSEL_BEST);
}

void Mpu9250::start() {
  unique_lock<mutex> lock(hwmtx);
  const int AK_data_count = 3*2+1; // 3-axes 2-bytes + status
  // enable periodic magnetometer data transfer via I2C slave 1
  writeRegister(REGMPU::I2C_SLV1_ADDR, AK_ADDR<<BIT_I2C_ID_1|I2C_SLV1_RNW);
  writeRegister(REGMPU::I2C_SLV1_REG, REGAK::HXL);
  writeRegister(REGMPU::I2C_SLV1_CTRL, I2C_SLV1_EN | (AK_data_count<<BIT_I2C_SLV1_LENG));

  /* configure normal data transfer */
  writeRegister(REGMPU::INT_PIN_CFG, 0); 	// INT by 50us level 1, cleared by reading INT_STATUS

  // enable interrupts
  writeRegister(REGMPU::USER_CTRL, FIFO_EN_ | I2C_MST_EN);
  writeRegister(REGMPU::FIFO_EN, ACCEL|GYRO_XOUT|GYRO_YOUT|GYRO_ZOUT|SLV_1/*mag*/);
  
  writeRegister(INT_ENABLE, RAW_RDY_EN); 	// set INT to data ready
}

void Mpu9250::stop() {
  unique_lock<mutex> lock(hwmtx);
  writeRegister(INT_ENABLE, 0);
  writeRegister(REGMPU::FIFO_EN, 0);
  writeRegister(REGMPU::I2C_SLV1_CTRL, 0);
}


void Mpu9250::interrupt() {
  checkDataPresence();
  checkHWData();
}


