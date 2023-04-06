#include <iostream>
#include <wiringPi.h>
using namespace std;

#define PIN_PIR 5

void sensePIR(){
    static int count = 0;
    cout << "Somebody sensed " << count++ << '\n';
}

int main(int argc, char *argv[]){
    wiringPiSetupGpio();
    pinMode(PIN_PIR, INPUT);

    wiringPiISR(PIN_PIR, INT_EDGE_RISING, &sensePIR);

    for(;;) {
        
    }
    return 0;
}
