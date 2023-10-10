# Example application: Cube

This is a demo application. It uses IMU to get sensor absolute orientation in order to ratate a cube on screen.
The application has 2 parts:

* [sensor](sensor) It runs on Raspberry Pi connected to MPU9250 sensor via I2C. It implements IMU and sends the current orientation periodically via UDP to the other part.
* [x11](x11) OpenGL part receiving data via UDP and displaying and rotating a cube. It should run on a Linux machine with graphical adapter and OpenGL library.
  
