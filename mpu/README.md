# MPU - Motion Processing Unit

The core of MPU is implemented in [mpu.cpp](mpu.cpp) in class Mpu. The class represents a generic MPU device, provides data to
consumers, allows guarantee that a consumer cannot miss data, implements systematic error corrections (bias, scale) and thread safe locking.

The MPU data is provided to consumers in struct MpuData, which contains vectors of acceleration, rotation and magnetic field measured at the same time.
The data structure contains also sequence number of this data.

Any consumer thread can get the next available data by calling getDataAfter(unsigned lastSeq, bool barrier=false, bool nextBarrier=false), which returns the MpuData struct or waits until 
data newer than lastSeq is available. By setting nextBarrier=true the consumer requires not to miss any data till the next call of getDataAfter(). The parameter barrier has to have 
the same value as parameter nextBarrier in previous call of this function (removing the barrier on the currently read data).

File mpu9250.cpp contains derived class Mpu9250 implementing Mpu using some of MPU-9250 + AK8963 family chips. The class contains MPU-9250 register access, uses HW based FIFO,
thread safe locking and interrupts. Class Mpu9250 assumes the chip is connected some communication channel (e.g. SPI or I2C) generalized in class Bus (bus.cpp). 
Per object-instance interrupts are implemented in interruptible.h (classes Interruptible and InterruptRegistrator). 

Chip MPU-9250 contains a lot of internal registers and bit constants. They are listed in registers-mpu.regs and registers-AK.regs. Both files are used as a source to generate C++ header files using registers.awk script.
The script also generates useful value-to-register-name mapping data structures for debug purposes.

Class Bus is an abstract class which defines basic I/O operations and I/O transactions. The real implementation for I2C is located in class I2C (i2c.cpp). An alternative SPI class is not implemented yet.
