#ifndef __BUS_H_
#define __BUS_H_

/* Class Bus represents a generic communication interface which allows to communicate
   with a hardware device in transactions. Each transaction containes one or more send/receive messages.
   
   The transaction starts with beginTransaction(), messages in a transaction are separated by
   continueTransaction(). Function endTransaction() finishes the transaction definition and executes it.
*/
   

enum BusTransactionType {
  bttWrite=0, 
  bttRead=1 
};

class Bus {
  public:
    virtual void beginTransaction(BusTransactionType btt) = 0;
    virtual void continueTransaction(BusTransactionType btt) = 0;
    virtual void send(unsigned char data) = 0;
    virtual void send(unsigned char *buffer, int count) { while(count--) send(*buffer++); }
    virtual void receive(unsigned char *buffer, int count) = 0;
    virtual void endTransaction() = 0;
};

#endif
