#include "bus.h"
#include <linux/i2c-dev.h>

#define I2C_BUFSIZE	16

class I2C : public Bus {
    int fd;					// I2C file descriptor
    struct i2c_rdwr_ioctl_data msgset;		// transaction definition data structure (current transaction)
    unsigned allocated_msgs=0;			// allocated message structures
    unsigned char buf[I2C_BUFSIZE];		// buffer for data
    unsigned buf_used=0;			// bytes used from buf
    unsigned char addr;				// I2C slave address
    
  private:
    void addMsg(BusTransactionType btt);	// add new message into the current transaction
    
  public:
    I2C(const char *device, unsigned char address);
    ~I2C();
    
    virtual void beginTransaction(BusTransactionType btt);
    virtual void continueTransaction(BusTransactionType btt);
    virtual void send(unsigned char data);
    virtual void send(unsigned char *buffer, int count);
    virtual void receive(unsigned char *buffer, int count);
    virtual void endTransaction();
};
