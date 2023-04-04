#include <wiringPi.h>
#include <iostream>
#include <unistd.h>

using namespace std;
#define LED_GPIO 17
#define BUTTON_GPIO 27

void lightLED(void){
    static int x = 1;
    digitalWrite(LED_GPIO, HIGH);
    cout << "Button pressed " << x++ << " times! LED on" << '\n';
}

int main(){
    wiringPiSetupGpio();
    pinMode(LED_GPIO, OUTPUT);
    pinMode(BUTTON_GPIO, INPUT);
    digitalWrite(LED_GPIO,LOW);
    cout << "Press the button on GPIO " << BUTTON_GPIO << '\n';
    
    wiringPiISR(BUTTON_GPIO, INT_EDGE_RISING, &lightLED);
    
    for(int i = 10; i >0; i--){
        cout << "You have " << i << " seconds remaining..." << '\n';
        sleep(1);
    }
    return 0;
}
