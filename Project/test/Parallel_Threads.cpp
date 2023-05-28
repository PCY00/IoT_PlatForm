#include <iostream>
#include <wiringPi.h>
#include <omp.h>


using namespace std;

void dht11Task();
void countdownTask();
bool runEvery(unsigned long interval);

int main()
{
    #pragma omp parallel num_threads(2)
    {
        #pragma omp sections
        {
            #pragma omp section
            {
                dht11Task();
            }

            #pragma omp section
            {
                countdownTask();
            }
        }
    }

    return 0;
}

void dht11Task(){
    while(1){
        if(runEvery(5000)){
            cout << "dht11" << '\n';
        }
    }
}

void countdownTask(){
    while(1){
        cout << "1" << '\n';
        delay(1000);
    }
}

bool runEvery(unsigned long interval){
	static unsigned long previousMillis = 0;
	unsigned long currentMillis = millis();

	if(currentMillis - previousMillis >= interval){
		previousMillis = currentMillis;
		return true;
	}
	return false;
}
