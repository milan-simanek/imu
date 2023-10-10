#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/ip.h>
#include <arpa/inet.h>

float input(const char **argv, int ix) {
  float val;
  if (1==sscanf(argv[ix], "%f", &val)) {
    printf("Error in %d argument (%s).\n", ix, argv[ix]);
    exit(1);
  }
  return val;
}

int main(int argc, const char *argv[]) {
  float r,x,y,z;
  int s, blen;
  struct sockaddr_in sin;
  char buf[100];
  
  if (argc!=5) {
    printf("this command requires 4 arguments: [rotation angle, x, y, z] (but only %d arguments recognized)\n", argc);
    printf("X-axis: points to the right\n"
           "Y-axis: points forward (in the screen viewing direction)\n"
           "Z-axis: points upward\n"
           "rotation angle is specified in degrees clockwise in the direction of [x,y,z]\n");
    return 1;
  }
  r=input(argv, 1);
  x=input(argv, 2);
  y=input(argv, 3);
  z=input(argv, 4);
  
  s=socket(AF_INET, SOCK_DGRAM, 0);
  if (s==-1) {
    perror("cannot create socket");
    return 2;
  }
  sin.sin_family=AF_INET;
  sin.sin_addr.s_addr=inet_addr("127.0.0.1");
  sin.sin_port=htons(2711);
  
  blen=sprintf(buf, "%f %f %f %f", r, x, y, z);
  
  sendto(s, buf, blen+1, 0, (struct sockaddr*)&sin, sizeof(sin));
  return 0;
}
