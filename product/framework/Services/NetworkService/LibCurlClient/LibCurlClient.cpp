#include "LibCurlClient.h"

#include <curl/curl.h>

namespace ucf::network::libcurl{
LibCurlClient::LibCurlClient()
{

}

void LibCurlClient::makeHttpRequest()
{
    
    
CURL *curl;
    CURLcode res;
    
    curl = curl_easy_init();
    if (curl) {
        curl_easy_setopt(curl, CURLOPT_URL, "http://www.example.com");
        /* example.com is redirected, so we tell libcurl to follow redirection */
        curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
        
        res = curl_easy_perform(curl);
        
        /* Check for errors */
        if (res != CURLE_OK) {
            fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
        }
        
        /* always cleanup */
        curl_easy_cleanup(curl);
    }

}
}