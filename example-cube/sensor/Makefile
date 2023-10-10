MODS=IVector FVector Quaternion QRotation mpu mpu9250 i2c imu sender app
DEPDIRS=../../imu ../../mpu ../../quaternions

OBJS=$(patsubst %,%.o,$(MODS))
CPP=g++
CC=gcc
FLAGS=-g -Wall -pedantic -Wno-switch -iquote . $(DEPDIRS:%=-iquote %)
empty=
space=$(empty) $(empty)
VPATH=$(subst $(space),:,$(DEPDIRS))


all: app


-include .*.d
mpu9250.o: registers-mpu.h registers-ak.h

app: $(OBJS)
	$(CPP) -lwiringPi -lpthread -o $@ $^
	
.c.o:
	$(CC) $(FLAGS) -c -MMD -MF .$@.d -o $@ $<

.cpp.o:
	$(CPP) $(FLAGS) -c -MMD -MF .$@.d -o $@ $<
	
%.h: ../../mpu/%.regs ../../mpu/registers.awk
	awk -f ../../mpu/registers.awk -v REG=$$(echo $@ | sed 's/^.*-\(.*\).h$$/\1/') $< > $@

clean:
	rm -f app *.o  *~ registers-*.h .*.d
