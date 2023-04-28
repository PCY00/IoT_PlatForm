#include <curl/curl.h>
#include <string>
#include <iostream>

int main(void) {
  CURL *curl;
  CURLcode res;
  std::string url = "http://203.253.128.177:7579/Mobius/20191546/data";
 
  curl = curl_easy_init();
  if(curl) {
    // Set the URL to send the POST request
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());

    // Set the headers for the POST request
    struct curl_slist *headers = NULL;
    headers = curl_slist_append(headers, "Content-Type: application/vnd.onem2m-res+json; ty=4");
    headers = curl_slist_append(headers, "X-M2M-RI: 12345");
    headers = curl_slist_append(headers, "X-M2M-Origin: SOrigin");
    headers = curl_slist_append(headers, "Accept: application/json");
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

    // Set up the POST data
    std::string post_data = "{\"m2m:cin\": {\"con\": \"12\"}}";
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, post_data.c_str());

    // Perform the POST request
    res = curl_easy_perform(curl);
    if(res != CURLE_OK) {
      fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
    }

    curl_easy_cleanup(curl);
  }
  return 0;
}
