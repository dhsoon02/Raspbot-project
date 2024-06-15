// tracking.h

#ifndef TRACKING_H
#define TRACKING_H

#include <wiringPi.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <linux/i2c-dev.h>
#include <math.h>
#include <signal.h>

// GPIO pin definitions
#define Tracking_Right1 0   // BCM_GPIO 17 -> WiringPi 0
#define Tracking_Right2 7   // BCM_GPIO 4  -> WiringPi 7
#define Tracking_Left1 2    // BCM_GPIO 27 -> WiringPi 2
#define Tracking_Left2 3    // BCM_GPIO 22 -> WiringPi 3

// I2C address
#define I2C_ADDR 0x16

// Function prototypes
void write_u8(int reg, int data);
void write_reg(int reg);
void write_array(int reg, int *data, int length);
void Ctrl_Car(int l_dir, int l_speed, int r_dir, int r_speed);
void Control_Car(int speed1, int speed2);
void Car_Stop();
// void Car_Run(int speed1, int speed2);
// void Car_Back(int speed1, int speed2);
// void Car_Left(int speed1, int speed2);
// void Car_Right(int speed1, int speed2);
// void Car_Spin_Left(int speed1, int speed2);
// void Car_Spin_Right(int speed1, int speed2);
void setup();
void tracking_function();
void signalHandler(int sig);

#endif // TRACKING_H
