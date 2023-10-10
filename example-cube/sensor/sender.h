#ifndef _SENDER_H_
#define _SENDER_H_

#include "imu.h"
#include "QRotation.h"

#define DEFAULT_TARGET_HOST	"localhost"
#define DEFAULT_TARGET_PORT	"2711"


/* class RotationSender periodically sends the current rotation over UDP */

class RotationSender {
  private:
    int sock = -1;
    IMU *imu = NULL;
    bool running = true;
    thread senderThread;
    virtual const QRotation getData() const { return imu->getRotation(); }
    virtual void sendData(const QRotation &r) const;
    void operator()() const;
    
  public:  
    RotationSender(IMU *i, const char *remoteHost=DEFAULT_TARGET_HOST, const char* remotePort=DEFAULT_TARGET_PORT);
    ~RotationSender();
};

#endif
