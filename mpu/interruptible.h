#ifndef _INTERRUPTIBLE_H_
#define _INTERRUPTIBLE_H_

/* WiringPi library provides a support to bind interrupt handler to GPIO pin, but does not provide
   possibility to bind a context to the interrupt handler. It means interrupt handler must be a static
   void handler() function and it cannot be a nonstatic class member. It means different instances of
   a class cannot bind to different interrupts.
   
   This source code solves the problem: Class Interrupteble is a base class for any derived class which
   receives interrupts to class instances. InterruptRegistrator class is a base class for template classes
   to receive different interrupt signals.
*/


#ifdef __amd64
# warning "USING FAKE wiringPi.h on intel x86"
  void pinMode(int, int);
  void wiringPiISR(int pin, int edge, void (*interrupt)());
  #define INPUT 1
  #define INT_EDGE_RISING 1
#else
# include <wiringPi.h>
#endif

class Interruptible {
  protected:
    virtual void interrupt() = 0;
  friend class InterruptRegistrator;
};

class InterruptRegistrator {
  protected:
    static inline void callInterrupt(Interruptible* o) { o->interrupt(); }
  public:
    virtual void enableInt(Interruptible *o) const = 0;
    virtual void disableInt() const { enableInt(nullptr); }
};

#define DECLARE_INTERRUPT(pin) DECLARE_INTERRUPT_(pin)
#define DECLARE_INTERRUPT_(pin)								\
  class InterruptRegistrator##pin : public InterruptRegistrator {			\
    static Interruptible *object;							\
    static void interrupt() { if (object) callInterrupt(object); }			\
    virtual void enableInt(Interruptible *o) const { object=o; }			\
  public:										\
    InterruptRegistrator##pin() {  							\
      pinMode(pin, INPUT);              						\
      wiringPiISR(pin, INT_EDGE_RISING, InterruptRegistrator##pin::interrupt);		\
    }											\
  };											\
  Interruptible *InterruptRegistrator##pin::object=nullptr

#define INTERRUPT(pin) INTERRUPT_(pin)
#define INTERRUPT_(pin) InterruptRegistrator##pin()

#endif
