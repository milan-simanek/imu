#include "bus.h"

// not implemented yet

class SPI : public Bus {
  public:
    SPI(int csPin);
    
    virtual void beginTransaction(BusTransactionType btt);
    virtual void continueTransaction(BusTransactionType btt);
    virtual void send(unsigned char data);
    virtual void send(unsigned char *buffer, int count);
    virtual void receive(unsigned char *buffer, int count);
    virtual void endTransaction();
};
