# Inertial Measurement Unit 
- based on Raspberry Pi and MPU9250
- C++ implementation
- using efficient and accurate quaternions calculations

## Project parts
- [quaternions](quaternions) implementation of floating point 3D-vectors, quaternions, spatial rotation objects and related operations
- [mpu](mpu) implementation of Motion Processing Unit software (MPU); it provides acceleration, rotation and magnetic field data
- [imu](imu) implementation of Inertial Measurement Unit (IMU); it consumes MPU data and calculates absolute orientation and position
- [example-cube](example-cube) example application which uses IMU to display a cube rotated by data from IMU
  
