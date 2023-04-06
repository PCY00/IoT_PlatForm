#include <iostream>
#include <unistd.h>
#include <wiringPi.h>
#include <iomanip>
using namespace std;

#define USING_DHT11 true
#define DHT_GPIO 22
#define LH_THRESHOLD 26 //low=~14, high=~38 - pick avg.

int main(){
    int humid = 0, temp = 0;
    cout << "Starting..." << endl;
    wiringPiSetupGpio();
    piHiPri(99);
TRYAGAIN:
    unsigned char data[5] = {0,0,0,0,0};
    pinMode(DHT_GPIO, OUTPUT);
    digitalWrite(DHT_GPIO, LOW);
    usleep(18000);
    digitalWrite(DHT_GPIO, HIGH);
    pinMode(DHT_GPIO, INPUT);
    do{delayMicroseconds(1);}while(digitalRead(DHT_GPIO) == HIGH);
    do{delayMicroseconds(1);}while(digitalRead(DHT_GPIO) == LOW);
    do{delayMicroseconds(1);}while(digitalRead(DHT_GPIO) == HIGH);
}
