//#define DBG
#include <sys/socket.h>
#include <netinet/ip.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <string.h>

#include "sender.h"
#include "imu.h"
#include "Quaternion.h"

#define REFRESH_HZ	25	// fixed 25Hz refresh rate

void RotationSender::sendData(const QRotation &r) const {
  char buf[100];
  int len, rv;
  float theta;

  theta=2*atan2(r.FVector::norm(), r.r);
  len=sprintf(buf, "%f %f %f %f\n", theta*(180.0f/3.14f), r.x, r.y, r.z);
#ifdef DBG
  fprintf(stdout, "Sending data: %s\n", buf);
#endif

  rv=send(sock, buf, len, 0);
  if (rv==-1) {
    cerr << "send->" << rv << "; errno=" << errno << endl;
    perror("###### send error");
  }
}

void RotationSender::operator()() const {
  while (running) {
    sendData(getData());
    usleep(1000000/(REFRESH_HZ));	// fixed 25Hz refresh rate
  }
}


RotationSender::RotationSender(IMU *i, const char *remoteHost, const char *remotePort) : imu (i), running(true) {
  struct addrinfo *ai, *p, hints;

  memset(&hints, 0, sizeof(struct addrinfo));  
  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_DGRAM; 
  
  if (getaddrinfo(remoteHost, remotePort, &hints, &ai)) {
    cerr << "cannot resolve remote host: resolver failed" << endl;
    abort();
  }
  for (p=ai;; p=p->ai_next) {
    if (!p) {
      cerr << "cannot connect to the remote host" << endl;
      abort();
    } 
    sock = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
    if (sock == -1) continue;
    if (!connect(sock, p->ai_addr, p->ai_addrlen)) break;
    close(sock);
  }

  senderThread=thread([](RotationSender *p){(*p)();}, this);
}

RotationSender::~RotationSender() {
  running=false;
  senderThread.join();
  close(sock);
  sock=-1;
  imu=nullptr;
}
