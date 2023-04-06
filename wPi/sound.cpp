#include <iostream>
#include <wiringPi.h>
using namespace std;

#define PIN_SOUND 5

void senseSound(){
    static int count = 0;
    cout << "Somebody make noise " << count++ << '\n';
}

int main(int argc, char *argv[]){
    wiringPiSetupGpio();
    pinMode(PIN_SOUND, INPUT);

    wiringPiISR(PIN_SOUND, INT_EDGE_RISING, &senseSound);

    for(;;) {

    }
    return 0;
}
