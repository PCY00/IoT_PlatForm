#include <iostream>
#include <curl/curl.h>
#include <string>
#include <wiringPi.h>
using namespace std;

//pir
#define PIN_PIR 5
//ch
#define PIN_TRIG 23
#define PIN_ECHO 24
#define RANGE_MAX 50    //50cm
#define RANGE_MIN 0

int count = 0;

//function
void sensePIR();
size_t write_callback(char *ptr, size_t size, size_t nmemb, string *data);
string performGETRequest(const string& url);

int main(int argc, char *argv[]){
    wiringPiSetupGpio();
    unsigned int T, L;
    string url = "http://203.253.128.177:7579/Mobius/20191546/data/la";

    pinMode(PIN_PIR, INPUT);
    pinMode(PIN_TRIG, OUTPUT);
    pinMode(PIN_ECHO, INPUT);

    wiringPiISR(PIN_PIR, INT_EDGE_RISING, &sensePIR);

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

        if((L >= RANGE_MIN || L <= RANGE_MAX) && count == 1){
            count = 0;
            cout << "post 1(someone coming)" << '\n';
            //post 들어갈 자리
            while(1){
                
                if(performGETRequest(url) == "00"){
                    break;
                }
            }
        }
        delay(100);
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
    headers = curl_slist_append(headers, "X-M2M-Origin: SOrigin");
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
    char *data;
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &res);
    if(res == 200) {
      curl_easy_getinfo(curl, CURLINFO_CONTENT_TYPE, &data);
      cout << "Content-Type: " << data << endl;

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
      cout << "con value: " << con_value << endl;
      save_data = con_value;
    }
    curl_easy_cleanup(curl);
  }
  return save_data;
}
