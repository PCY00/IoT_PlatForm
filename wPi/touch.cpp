#include <iostream>
#include <wiringPi.h>
using namespace std;

#define PIN_TOUCH 5
#define DEBOUNCE_TIME 200

void senseTouch(){
    static unsigned long lastISRTime = 0;
    static unsigned long count = 0;
    unsigned long currentISRTime = millis();
    if(currentISRTime - lastISRTime > DEBOUNCE_TIME){
        cout << "Touch detected: " << count++ << endl;
    }
    lastISRTime = currentISRTime;
}

int main(int argc, char *argv[]){
    wiringPiSetupGpio();
    pinMode(PIN_TOUCH, INPUT);

    wiringPiISR(PIN_TOUCH, INT_EDGE_RISING, &senseTouch);

    for(;;) {

    }

    cout << "Finished" << endl;
    return 0;
}
