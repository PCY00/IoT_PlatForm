#include <iostream>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <iomanip>
#include <sys/ioctl.h>
#include <linux/i2c.h>
#include <linux/i2c-dev.h>

using namespace std;

#define HEX(x) setw(2) << setfill('0') << hex << (int)(x)

#define DEVID 0x00
#define POWER_CTL 0x2D

#define DATA_FORMAT 0x31
#define DATAX0 0x32
#define DATAX1 0x33
#define DATAY0 0x34
#define DATAY1 0x35
#define DATAZ0 0x36
#define DATAZ1 0x37

#define BUFFER_SIZE 0x40
unsinged char dataBuffer[BUFFER_SIZE];

int writeRegister(int file, unsigned char address, char value){
    unsigned char buffer[2];
    buffer[0] = address;
    buffer[1] = value;
    if(write(file, buffer, 2) != 2){
        cout << "Failed write to the device" << endl;
        return 1;
    }
    return 0;
}

int readRegister(int file){
    writeRegister(file, 0x00, 0x00);

    if(read(file, dataBuffer, BUFFER_SIZE) != BUFFER_SIZE){
        cout << "Failed to read in the full buffer" << endl;
        return 1;
    }
    if(dataBuffer[DEVID] != 0xE5){
        cout << "Failed to verify device ID" << endl;
        return 1;
    }
    return 0;
}

short combineValues(unsigned char msb, unsigned char lsb){
    return ((short)msb << 8 | lsb);    
}

int main(){
    cout << "Starting the ADXL345 sensor application" << endl;
    if((file = open("/dev/i2c-1", O_RDWR)) < 0){
        cout << "Failed to open the bus" << endl;
        return 1;
    }
    if(ioctl(file, I2C_SLAVE, 0x53) < 0){
        cout << "Failed to connect to the sensor" << endl;
        return 1;
    }
    writeRegister(file, POWER_CTL, 0x08);
    writeRegister(file, DATA_FORMAT, 0x00);
    readRegister(file);

    cout << "The Device ID is: " << HEX(dataBuffer[DEVID]) << endl;
    cout << "The Power register is: " << HEX(dataBuffer[POWER_CTL]) << endl;
    cout << "The Data Format register is: " << HEX(dataBuffer[DATA_FORMAT]) << endl;
    cout << dec << endl;

    int count = 0;
    while(count < 60){
        short x = combineValues(dataBuffer[DATAX1], dataBuffer[DATAX0]);
        short y = combineValues(dataBuffer[DATAY1], dataBuffer[DATAY0]);
        short z = combineValues(dataBuffer[DATAZ1], dataBuffer[DATAZ0]);

        cout << "X=" << x << " Y=" << y << " Z=" << z << " sample=" << count << "     \r" << flush;
        usleep(1000000);
        readRegister(file);
        count++;
    }
    close(file);
    return 0;
}
