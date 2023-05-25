#include <iostream>
#include <wiringPi.h>
using namespace std;
#define PIN_TRIG 23
#define PIN_ECHO 24
#define RANGE_MAX 200
#define RANGE_MIN 0

int main(int argc, char *argv[]){
    wiringPiSetupGpio();
    unsigned int T, L;
    pinMode(PIN_TRIG, OUTPUT);
    pinMode(PIN_ECHO, INPUT);

    while(1){
        digitalWrite(PIN_TRIG, LOW);
        delayMicroseconds(2);
        digitalWrite(PIN_TRIG, HIGH);
        delayMicroseconds(20);
        digitalWrite(PIN_TRIG, LOW);

        while(digitalRead(PIN_ECHO) == LOW);

        unsigned int startTime = micros();
        while(digitalRead(PIN_ECHO) == HIGH);
        T = micros() - startTime;
        L = T / 58.2;

        if(L <= RANGE_MIN || L >= RANGE_MAX){
            cout << "-1" << endl;
        }else{
            cout << "Distance is " << L << "cm" << endl;
        }
        delay(100);
    }
}
