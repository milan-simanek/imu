#include <unistd.h>
#include <math.h>
#include <sys/select.h>
#include <termios.h>
#include <signal.h>

#include <sys/time.h>
#include <errno.h>
#include <iostream>
#include <fstream>

#ifdef __amd64
# warning "USING FAKE wiringPi.h on intel x86"
  void pinMode(int, int){}
  void wiringPiISR(int pin, int edge, void (*interrupt)()){}
  void wiringPiSetup(){}
#else
# include <wiringPi.h>
#endif

#include "i2c.h"
#include "mpu.h"
#include "mpu9250.h"
#include "imu.h"
#include "sender.h"

using namespace std;

#define INTPIN 		7
#define I2C_DEVICE	"/dev/i2c-1"
#define I2C_ADDR	0x68
#define CALI_FILE       "cali"

DECLARE_INTERRUPT(INTPIN);

class App {
  I2C			i2c;
  Mpu9250		mpu;
  IMU			imu;
  RotationSender	sender;
  
  public:
  App(const char *remoteHost) : i2c(I2C_DEVICE, I2C_ADDR), 
                                mpu(&i2c, INTERRUPT(INTPIN), CALI_FILE), 
                                imu(&mpu),
                                sender(&imu, remoteHost) {
    mpu.setAccelRange(Mpu9250::AccelRange::AR_16G);
    mpu.setGyroRange(Mpu9250::GyroRange::GR_500DPS);  
    mpu.setAccelFilter(Mpu9250::AccelFilter::AF_184HZ);
    mpu.setGyroFilter(Mpu9250::GyroFilter::GF_184HZ);
    mpu.setSampleRate(Mpu9250::SampleRate::SR_100HZ);
    imu.start();
  }
  void loadCali() { ifstream f(CALI_FILE); mpu.readCaliData(f); }
  void saveCali() { ofstream f(CALI_FILE); mpu.writeCaliData(f); }
  void interactive();
};


void App::interactive() {
  char kstate=0;
  struct termios tty_opts;
  tcgetattr(STDIN_FILENO, &tty_opts);
  tty_opts.c_lflag &= ~(ECHO | ICANON);
  tcsetattr(STDIN_FILENO, TCSANOW, &tty_opts);
  for(;;) {
    char c;
    if (read(0, &c, 1)!=1) continue;
    if (kstate=='c') {
      kstate=0;
      switch (c) {
      case '.': cout << "storing calibration data" << endl;
                saveCali();
                break;
      case '-': cout << "loading calibration data" << endl;
                loadCali();
                break;
      }
      continue;
    }
    switch (c) {
    case 'c': kstate='c'; break; 			// calibration subcommand
    case '0': imu.resetRotation(); break;
    case 'm': imu.disableMagCorrection(); cout << "MAG correction off" << endl; break;
    case 'M': imu.enableMagCorrection();  cout << "MAG correction on" << endl; break;
    case 'g': imu.disableGravityCorrection(); cout << "G correction off" << endl; break;
    case 'G': imu.enableGravityCorrection();  cout << "G correction on" << endl; break;
    case 'q': cout << "QUIT" << endl; return;
    }
  }
}

int main(int argc, char *argv[]) {
  wiringPiSetup();
  App a(argc>1 ? argv[1] : NULL);
  a.interactive();
}

