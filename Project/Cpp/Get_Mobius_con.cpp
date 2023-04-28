#include <curl/curl.h>
#include <string>
#include <iostream>

size_t write_callback(char *ptr, size_t size, size_t nmemb, std::string *data)
{
    size_t realsize = size * nmemb;
    data->append(ptr, realsize);
    return realsize;
}

int main(void) {
  CURL *curl;
  CURLcode res;
  std::string url = "http://203.253.128.177:7579/Mobius/20191546/data/la";
 
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
    std::string response_data;
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
      std::cout << "Content-Type: " << data << std::endl;

      // Parse the response data to get the "con" value
      std::string con_value;
      size_t start_pos = response_data.find("\"con\":\"");
      if (start_pos != std::string::npos) {
        start_pos += 7; // move past the "\"con\":\"" prefix
        size_t end_pos = response_data.find("\"", start_pos);
        if (end_pos != std::string::npos) {
          con_value = response_data.substr(start_pos, end_pos - start_pos);
        }
      }
      std::cout << "con value: " << con_value << std::endl;
    }

    curl_easy_cleanup(curl);
  }
  return 0;
}
