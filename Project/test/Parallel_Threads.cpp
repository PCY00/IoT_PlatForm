/*
dht11Task() 함수는 무한 루프를 실행하며, runEvery() 함수를 사용하여 5초마다 "dht11"을 출력
countdownTask() 함수는 무한 루프를 실행하며, 1초마다 "1"을 출력
main() 함수에서 #pragma omp parallel num_threads(2)를 사용하여 2개의 스레드를 생성
#pragma omp sections를 사용하여 스레드들이 각자 별개의 섹션에서 작업을 수행
첫 번째 섹션에서는 dht11Task() 함수가 실행되고, 두 번째 섹션에서는 countdownTask() 함수가 실행
각 스레드는 독립적으로 작업을 수행하며, 출력 결과는 스레드별로 독립적으로 나타냄
따라서 이 코드는 병렬 처리를 사용하여 dht11Task()와 countdownTask() 함수를 동시에 실행하고, 각각의 작업 결과를 독립적으로 출력
*/

#include <iostream>
#include <wiringPi.h>
#include <omp.h>


using namespace std;

void dht11Task();
void countdownTask();
bool runEvery(unsigned long interval);

int main()
{
        while(1){
            #pragma omp parallel num_threads(2){
                #pragma omp sections{
                    #pragma omp section{
                        dht11Task();
                    }

                    #pragma omp section{
                        countdownTask();
                    }
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
