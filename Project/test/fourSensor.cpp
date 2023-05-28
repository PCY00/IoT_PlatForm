/*
인체감지센서와 초음파센서를 이용하여 사람이 다가와 거리가 30cm이내이고 인체감지센서가 감지되면 personcheck컨테이너에 post로 1의 값을 보낸다.
그리고 personcheck컨테이너에서 get으로 값을 받아온다. 받아온 값이 00이면 사람이 지나갔다는 뜻이므로 다시 초음파센서로 거리를 측정한다.
00은 rfid에서 결제시 00의 값을 보낼것이다. 00의 값이 오기전까진 초음파센서의 측정을 멈춘다.
00의 값을 받아오면 다시 초음파센서로 거리를 측정한다. 이렇게 하는 이유는 키오스크에서 사람이 메뉴를 선택하고 결제하기 전까진 한사람만 계속 있기 때문에 
결제하고 나면 다시 초음파센서로 거리를 측정하여 다음 사람이 다가오면 다시 결제할 수 있도록 한다.
딜레이사용 없이 5초마다 온습도를 측정하고 25도 이상이면 릴레이를 켜고 릴레이를 키면 선풍기가 돌아간다.
선풍기가 돌아가면 5초마다 온습도를 측정하고 25도 이하면 릴레이를 끄고 선풍기를 끈다.

-fopenmp -lcurl -lwiringPi
*/
#include <iostream>
#include <curl/curl.h>
#include <unistd.h>
#include <iomanip>
#include <string>
#include <wiringPi.h>
#include <omp.h>
using namespace std;

//pir
#define PIN_PIR 5
//ch
#define PIN_TRIG 23
#define PIN_ECHO 24
#define RANGE_MAX 30    //Set to 30cm
#define RANGE_MIN 0
//dht
#define USING_DHT11 true
#define DHT_GPIO 22
#define RELAY_GPIO 17
#define LH_THRESHOLD 26 //low=~14, high=~38 - pick avg.


//PIR state
int count = 0;

//function
void sensePIR();                                                            //PIR callback
size_t write_callback(char *ptr, size_t size, size_t nmemb, string *data);  //Get callback
string performGETRequest(const string& url);                                //HTTP Get
void performPOSTRequest(const string& url, const string& post_data);        //HTTP Post
unsigned int getDistance();                                                 //Distance measurement
bool runEvery(unsigned long interval);                                      //Delay milliseconds
void measureTemperatureAndHumidity(int& temp, int& humid);                  //Temperature and humidity measurement
void controlRelayAndFan(int temp);                                          //Relay and fan control
void T_Task();                                                              //Temperature and humidity task
void PNH_Task();                                                            //person Notice HTTP task


int main(int argc, char *argv[]){
    wiringPiSetupGpio();
    //Priority setting
    piHiPri(99);
    string url_get = "http://203.253.128.177:7579/Mobius/20191546/data/la";
    string url_post = "http://203.253.128.177:7579/Mobius/20191546/personcheck";
    string post_data = "{\"m2m:cin\": {\"con\": \"1\"}}";
    //PIR
    pinMode(PIN_PIR, INPUT);
    //ch
    pinMode(PIN_TRIG, OUTPUT);
    pinMode(PIN_ECHO, INPUT);
    //dht
    pinMode(RELAY_GPIO, OUTPUT);
    digitalWrite(RELAY_GPIO, LOW);
    //cout << "First Relay Off" << '\n';

    wiringPiISR(PIN_PIR, INT_EDGE_RISING, &sensePIR);

    while(1){
        //Create two threads and run parallel
        #pragma omp parallel num_threads(2)
	{
            #pragma omp sections
	    {
                //dht sensor task
                #pragma omp section
		{
                    T_Task();
                }
                //person notice task
                #pragma omp section
		{
                    PNH_Task();
                }
            }
        }
    }
    return 0;
}

void sensePIR(){
    count = 1;
    cout << "change 1" << '\n';
}

size_t write_callback(char *ptr, size_t size, size_t nmemb, string *data){
    size_t realsize = size * nmemb;
    data->append(ptr, realsize);
    return realsize;
}

string performGETRequest(const string& url) {
  CURL *curl;
  CURLcode res;
  string save_data;

  curl = curl_easy_init();
  if(curl) {
    // Set the URL to send the GET request
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());

    // Set the headers for the GET request
    struct curl_slist *headers = NULL;
    headers = curl_slist_append(headers, "Accept: application/json");
    headers = curl_slist_append(headers, "X-M2M-RI: 12345");
    headers = curl_slist_append(headers, "X-M2M-Origin: SI3oXROBJmB");
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

    // Set up the response data callback
    string response_data;
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response_data);

    // Perform the GET request
    res = curl_easy_perform(curl);
    if(res != CURLE_OK) {
      fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
    }

    // Output the response data content type
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &res);
    if(res == 200) {
      // Parse the response data to get the "con" value
      string con_value;
      size_t start_pos = response_data.find("\"con\":\"");
      if (start_pos != string::npos) {
        start_pos += 7; // move past the "\"con\":\"" prefix
        size_t end_pos = response_data.find("\"", start_pos);
        if (end_pos != string::npos) {
          con_value = response_data.substr(start_pos, end_pos - start_pos);
        }
      }
      save_data = con_value;
    }
    curl_easy_cleanup(curl);
  }
  return save_data;
}

void performPOSTRequest(const string& url, const string& post_data) {
    CURL *curl;
    CURLcode res;

    curl = curl_easy_init();
    if(curl) {
        // Set the URL to send the POST request
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());

        // Set the headers for the POST request
        struct curl_slist *headers = NULL;
        headers = curl_slist_append(headers, "Content-Type: application/vnd.onem2m-res+json; ty=4");
        headers = curl_slist_append(headers, "X-M2M-RI: 12345");
        headers = curl_slist_append(headers, "X-M2M-Origin: SI3oXROBJmB");
        headers = curl_slist_append(headers, "Accept: application/json");
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

        // Set up the POST data
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, post_data.c_str());

        // Perform the POST request
        res = curl_easy_perform(curl);
        if(res != CURLE_OK) {
            fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
        }

        curl_easy_cleanup(curl);
    }
}

unsigned int getDistance(){
    unsigned int T, L;

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

    return L;
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

void T_Task(){
    while(1){
        if(runEvery(5000)){
            int temp, humid;
            measureTemperatureAndHumidity(temp, humid);
            controlRelayAndFan(temp);
        }
    }
}

void PNH_Task(){
    while(1){
        unsigned int L = getDistance();
	    //cout << L << "cm\n"; //Output of ultrasonic sensor value

        //When the value of the human body detection and ultrasonic sensor is within 30 cm
        if((L >= RANGE_MIN && L <= RANGE_MAX) && count == 1){
            //Initialize to human detection sensor value 0
            count = 0;
            //post 1(someone coming)
            performPOSTRequest(url_post, post_data);
            //cout << "post 1(someone coming)" << '\n';

            while(1){
                //cout << "roop" << '\n';
                if(performGETRequest(url_get) == "00"){
                   	cout << "break" << '\n';
			        break;
                }
            }
        }
        delay(100);
    }
}
