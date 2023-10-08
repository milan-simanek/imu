#include "i2c.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <linux/i2c.h>
#include <linux/i2c-dev.h>
#include <sys/ioctl.h>
#include <cstring>
#include <cerrno>

//#define I2C_LOG		"/tmp/i2c.log"
//#define I2C_LOG		"/dev/stdout"

#define MAX_MSGSIZE	32
#define INITIAL_MSGS	2

#ifdef I2C_LOG
int trid=0;
FILE *F;
void printTransaction(const struct i2c_rdwr_ioctl_data &msgset) {
    fprintf(F, "I2C Transaction #%d (messages=%d):\n", trid, msgset.nmsgs);
    for(unsigned i=0;i<msgset.nmsgs;i++) {
        fprintf(F, "  MSG[%d]: ADDR=%d %s LEN=%d BUF={",
                                         i, msgset.msgs[i].addr, (msgset.msgs[i].flags&1)?"RD":"WR", msgset.msgs[i].len);
        for(int b=0; b<msgset.msgs[i].len; b++) fprintf(F, "%02X,", msgset.msgs[i].buf[b]);
        fprintf(F, "}\n");
    }
    fflush(F);
}
#endif

I2C::I2C(const char *device, unsigned char address) : fd(-1), addr(address) {
#ifdef I2C_LOG
  F=fopen(I2C_LOG, "w");
  fprintf(F, "I2C log:\n");
#endif
  msgset.msgs=(struct i2c_msg*)malloc(INITIAL_MSGS*sizeof(struct i2c_msg));
  allocated_msgs=INITIAL_MSGS;
  msgset.nmsgs=0;
  if ((fd = open (device, O_RDWR)) < 0) {
    fprintf(stderr, "Unable to open %s\n", device);
    abort();
  }
}

I2C::~I2C() {
  free(msgset.msgs);
  if (fd>=0) close(fd);
  fd=-1;
}

void I2C::addMsg(BusTransactionType btt) {
  if (msgset.nmsgs==allocated_msgs) {
    msgset.msgs=(struct i2c_msg*)realloc(msgset.msgs, ++allocated_msgs*sizeof(struct i2c_msg));
    if (!msgset.msgs) abort();
  }
  struct i2c_msg *m=msgset.msgs+msgset.nmsgs++;
  m->addr=addr;
  m->flags=btt;
  m->len=0;
  m->buf=NULL;
}

void I2C::beginTransaction(BusTransactionType btt) {
#ifdef I2C_LOG
  trid++;
#endif
  addMsg(btt);
}

void I2C::continueTransaction(BusTransactionType btt) {
  addMsg(btt);
}


void I2C::endTransaction() {
  if (!msgset.nmsgs) { fprintf(stderr, "Warning: I2C: empty transaction!\n"); abort(); }
  if (ioctl(fd, I2C_RDWR, &msgset) < 0 ) {
#ifdef I2C_LOG
      printTransaction(msgset);
#endif
      fprintf(stderr, "I2C: ioctl error %d.\n", errno);

      perror("\t");
      abort();
      return;
  }
#ifdef I2C_LOG
  printTransaction(msgset);
#endif
  buf_used=0;
  msgset.nmsgs=0;
}

void I2C::send(unsigned char *buffer, int count) {
  struct i2c_msg *m=msgset.msgs+(msgset.nmsgs-1);
  m->buf=buffer;
  m->len=count;
}

void I2C::receive(unsigned char *buffer, int count) {
  struct i2c_msg *m=msgset.msgs+(msgset.nmsgs-1);
  m->addr=addr;
  m->len=count;
  m->buf=buffer;
  memset(m->buf, 0, count);
}

void I2C::send(unsigned char data) {
  struct i2c_msg *m=msgset.msgs+(msgset.nmsgs-1);
  if (!m->buf) m->buf=buf+buf_used;
  buf_used++;
  m->buf[m->len++]=data;
}
