#include <stdio.h>
#include <curl/curl.h>

/* Since this program depends on libcurl, you may need to 
 * install libcurl4-gnutls-dev.
 *
 * On Linux Mint, this can be done by running:
 * sudo apt-get install libcurl4-gnutls-dev
 */

void HTTP_GET(const char* url){
	CURL *curl;
	curl = curl_easy_init();
	if(curl) {
		curl_easy_setopt(curl, CURLOPT_URL, url);
		curl_easy_perform(curl);
		curl_easy_cleanup(curl);
	}
}



int main(void){
	const char* hostname="localhost";
	const int   port=8000;
	const int   id=1;
	const char* password="password";
	const char* name="Team+Awesome";
	const int   adcval=123;
	const char* status="Very+good+thank+you";
	const char* timestamp="20141116-09:12:34";

	char buf[1024];
	snprintf(buf, 1024, "http://%s:%d/update?id=%d&password=%s&name=%s&data=%d&status=%s&timestamp=%s",
			hostname,
			port,
			id,
			password,
			name,
			adcval,
			status,
            timestamp);
	HTTP_GET(buf);
	return 0;
}
