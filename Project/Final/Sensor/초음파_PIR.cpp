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
#define RANGE_MAX 30    //30cm
#define RANGE_MIN 0

int count = 0;

//function
void sensePIR();
size_t write_callback(char *ptr, size_t size, size_t nmemb, string *data);
string performGETRequest(const string& url);
void performPOSTRequest(const string& url, const string& post_data);
unsigned int getDistance();

int main(int argc, char *argv[]){
    wiringPiSetupGpio();
    string url_get = "http://203.253.128.177:7579/Mobius/20191546/personcheck/la";
    string url_post = "http://203.253.128.177:7579/Mobius/20191546/personcheck";
    string post_data = "{\"m2m:cin\": {\"con\": \"1\"}}";

    pinMode(PIN_PIR, INPUT);
    pinMode(PIN_TRIG, OUTPUT);
    pinMode(PIN_ECHO, INPUT);

    wiringPiISR(PIN_PIR, INT_EDGE_RISING, &sensePIR);

    while(1){
        unsigned int L = getDistance();
	cout << L << "cm\n";
        if((L >= RANGE_MIN && L <= RANGE_MAX) && count == 1){
            count = 0;
            performPOSTRequest(url_post, post_data);
            cout << "post 1(someone coming)" << '\n';

            while(1){
                //cout << "roop" << '\n';
                if(performGETRequest(url_get) == "00"){
                   	cout << "break" << '\n';
			break;
                }
            }
	    delay(3000);
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
