#include <wiringPi.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <linux/i2c-dev.h>
#include <math.h>
#include <signal.h>

#define Tracking_Right1 0   // BCM_GPIO 17 -> WiringPi 0
#define Tracking_Right2 7   // BCM_GPIO 4  -> WiringPi 7
#define Tracking_Left1 2    // BCM_GPIO 27 -> WiringPi 2
#define Tracking_Left2 3    // BCM_GPIO 22 -> WiringPi 3

#define I2C_ADDR 0x16

int fd;

void write_u8(int reg, int data) {
    unsigned char buffer[2] = {reg, data};
    if (write(fd, buffer, 2) != 2) {
        printf("write_u8 I2C error\n");
    }
}

void write_reg(int reg) {
    unsigned char buffer[1] = {reg};
    if (write(fd, buffer, 1) != 1) {
        printf("write_reg I2C error\n");
    }
}

void write_array(int reg, int *data, int length) {
    unsigned char buffer[length + 1];
    buffer[0] = reg;
    for (int i = 0; i < length; i++) {
        buffer[i + 1] = data[i];
    }
    if (write(fd, buffer, length + 1) != length + 1) {
        printf("write_array I2C error\n");
    }
}

void Ctrl_Car(int l_dir, int l_speed, int r_dir, int r_speed) {
    int reg = 0x01;
    int data[4] = {l_dir, l_speed, r_dir, r_speed};
    write_array(reg, data, 4);
}

void Control_Car(int speed1, int speed2) {
    int dir1 = (speed1 < 0) ? 0 : 1;
    int dir2 = (speed2 < 0) ? 0 : 1;
    Ctrl_Car(dir1, abs(speed1), dir2, abs(speed2));
}

void Car_Stop() {
    printf("Car_Stop\n"); // 디버깅 출력
    int reg = 0x02;
    write_u8(reg, 0x00);
}
/*
void Car_Run(int speed1, int speed2) {
    Ctrl_Car(1, speed1, 1, speed2);
}

void Car_Back(int speed1, int speed2) {
    Ctrl_Car(0, speed1, 0, speed2);
}

void Car_Left(int speed1, int speed2) {
    Ctrl_Car(0, speed1, 1, speed2);
}

void Car_Right(int speed1, int speed2) {
    Ctrl_Car(1, speed1, 0, speed2);
}

void Car_Spin_Left(int speed1, int speed2) {
    Ctrl_Car(0, speed1, 1, speed2);
}

void Car_Spin_Right(int speed1, int speed2) {
    Ctrl_Car(1, speed1, 0, speed2);
}
*/

void setup() {
    if (wiringPiSetup() == -1) {
        printf("WiringPi setup failed!\n");
    }
    
    pinMode(Tracking_Right1, INPUT);
    pinMode(Tracking_Right2, INPUT);
    pinMode(Tracking_Left1, INPUT);
    pinMode(Tracking_Left2, INPUT);
    
    fd = open("/dev/i2c-1", O_RDWR);
    if (fd < 0) {
        printf("I2C setup error\n");
        exit(1);
    }
    if (ioctl(fd, I2C_SLAVE, I2C_ADDR) < 0) {
        printf("I2C setup failed!\n");
    }
}

void tracking_function() {
    int Tracking_Left1Value = digitalRead(Tracking_Left1);
    int Tracking_Left2Value = digitalRead(Tracking_Left2);
    int Tracking_Right1Value = digitalRead(Tracking_Right1);
    int Tracking_Right2Value = digitalRead(Tracking_Right2);
    
    if ((Tracking_Left1Value == LOW || Tracking_Left2Value == LOW) && Tracking_Right2Value == LOW) {
        printf("%d %d %d %d\n", Tracking_Left1Value, Tracking_Left2Value, Tracking_Right1Value, Tracking_Right2Value);
        Control_Car(70, -30);
        delay(200); // 0.2 seconds
    } else if (Tracking_Left1Value == LOW && (Tracking_Right1Value == LOW || Tracking_Right2Value == LOW)) {
        printf("elif1\n");
        printf("%d %d %d %d\n", Tracking_Left1Value, Tracking_Left2Value, Tracking_Right1Value, Tracking_Right2Value);
        Control_Car(-30, 70);
        delay(200);
    } else if (Tracking_Left1Value == LOW) {
        printf("elif2\n");
        printf("%d %d %d %d\n", Tracking_Left1Value, Tracking_Left2Value, Tracking_Right1Value, Tracking_Right2Value);
        Control_Car(-70, 70);
        delay(50);
    } else if (Tracking_Right2Value == LOW) {
        printf("elif3\n");
        printf("%d %d %d %d\n", Tracking_Left1Value, Tracking_Left2Value, Tracking_Right1Value, Tracking_Right2Value);
        Control_Car(70, -70);
        delay(50);
    } else if (Tracking_Left2Value == LOW && Tracking_Right1Value == HIGH) {
        printf("elif4\n");
        printf("%d %d %d %d\n", Tracking_Left1Value, Tracking_Left2Value, Tracking_Right1Value, Tracking_Right2Value);
        Control_Car(-60, 60);
        delay(20);
    } else if (Tracking_Left2Value == HIGH && Tracking_Right1Value == LOW) {
        printf("elif5\n");
        printf("%d %d %d %d\n", Tracking_Left1Value, Tracking_Left2Value, Tracking_Right1Value, Tracking_Right2Value);
        Control_Car(60, -60);
        delay(20);
    } else if (Tracking_Left2Value == LOW && Tracking_Right1Value == LOW) {
        printf("elif6\n");
        printf("%d %d %d %d\n", Tracking_Left1Value, Tracking_Left2Value, Tracking_Right1Value, Tracking_Right2Value);
        Control_Car(70, 70);
    }
    
}

void signalHandler(int sig) {
    Car_Stop();
    printf("Program terminated\n");
    exit(0);
}

int main(void) {
    setup();
    signal(SIGINT, signalHandler); // Handle Ctrl+C to stop the car

    while (1) {
        tracking_function();
    }

    return 0;
}
