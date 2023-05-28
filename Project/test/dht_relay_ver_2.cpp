/*
    딜레이사용 없이 5초마다 온습도를 측정하고 25도 이상이면 릴레이를 켜고 릴레이를 키면 선풍기가 돌아간다.
    선풍기가 돌아가면 5초마다 온습도를 측정하고 25도 이하면 릴레이를 끄고 선풍기를 끈다.
*/
#include <iostream>
#include <unistd.h>
#include <wiringPi.h>
#include <iomanip>
using namespace std;

#define USING_DHT11 true
#define DHT_GPIO 22
#define RELAY_GPIO 17
#define LH_THRESHOLD 26 //low=~14, high=~38 - pick avg.

int temp = 0, humid = 0;

bool runEvery(unsigned long interval);
void measureTemperatureAndHumidity(int& temp, int& humid);
void controlRelayAndFan(int temp);

int main() {
    cout << "Starting..." << endl;
    wiringPiSetupGpio();
    piHiPri(99);
    pinMode(RELAY_GPIO, OUTPUT);
    digitalWrite(RELAY_GPIO, LOW);
    cout << "First Relay Off" << '\n';

    while (1) {
        if (runEvery(5000)) {
            measureTemperatureAndHumidity(temp, humid);
            controlRelayAndFan(temp);
        }
    }

    return 0;
}

bool runEvery(unsigned long interval) {
    static unsigned long previousMillis = 0;
    unsigned long currentMillis = millis();

    if (currentMillis - previousMillis >= interval) {
        previousMillis = currentMillis;
        return true;
    }
    return false;
}

void measureTemperatureAndHumidity(int& temp, int& humid) {
    unsigned char data[5] = {0, 0, 0, 0, 0};
    pinMode(DHT_GPIO, OUTPUT);
    digitalWrite(DHT_GPIO, LOW);
    usleep(18000);
    digitalWrite(DHT_GPIO, HIGH);
    pinMode(DHT_GPIO, INPUT);
    do { delayMicroseconds(1); } while (digitalRead(DHT_GPIO) == HIGH);
    do { delayMicroseconds(1); } while (digitalRead(DHT_GPIO) == LOW);
    do { delayMicroseconds(1); } while (digitalRead(DHT_GPIO) == HIGH);

    for (int d = 0; d < 5; d++) {
        for (int i = 0; i < 8; i++) {
            do { delayMicroseconds(1); } while (digitalRead(DHT_GPIO) == LOW);
            int width = 0;
            do {
                width++;
                delayMicroseconds(1);
                if (width > 1000) break;
            } while (digitalRead(DHT_GPIO) == HIGH);
            data[d] = data[d] | ((width > LH_THRESHOLD) << (7 - i));
        }
    }

    if (USING_DHT11) {
        humid = data[0] * 10;
        temp = data[2] * 10;
    } else {
        humid = data[0] << 8 | data[1];
        temp = data[2] << 8 | data[3];
    }

    unsigned char chk = 0;
    for (int i = 0; i < 4; i++) {
        chk += data[i];
    }

    if (chk != data[4]) {
        cout << "Checksum bad - data error - trying again" << endl;
        usleep(2000000);
        measureTemperatureAndHumidity(temp, humid); // Retry measurement
    }
}

void controlRelayAndFan(int temp) {
    if ((float)temp / 10 >= 25.0) {
        cout << "Relay On" << '\n';
        digitalWrite(RELAY_GPIO, HIGH);
    } else {
        cout << "Relay Off" << '\n';
        digitalWrite(RELAY_GPIO, LOW);
    }
}
