#include <stdio.h>
#include <stdlib.h>
#include <curl/curl.h>
#include <sys/stat.h>

/* Since this program depends on libcurl, you may need to 
 * install libcurl4-gnutls-dev.
 *
 * On Linux Mint, this can be done by running:
 * sudo apt-get install libcurl4-gnutls-dev
 */

void HTTP_POST(const char* url, const char* image, int size){
	CURL *curl;
	CURLcode res;

	curl = curl_easy_init();
	if(curl){
		curl_easy_setopt(curl, CURLOPT_URL, url);
                curl_easy_setopt(curl, CURLOPT_POST, 1);
		curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE,(long) size);
                curl_easy_setopt(curl, CURLOPT_POSTFIELDS, image);
		res = curl_easy_perform(curl);
		if(res != CURLE_OK)
      			fprintf(stderr, "curl_easy_perform() failed: %s\n",
              			curl_easy_strerror(res));
		curl_easy_cleanup(curl);
	}
}

int main(void){
	const char* hostname="kew.uml.edu";
	const int   port=8000;
	const int   id=1;
	const char* password="password";
	const char* name="Team+Awesome";
	const int   adcval=123;
	const char* status="Very+good+thank+you";
	const char* timestamp="20141116-09:12:34";
	const char* filename="smile.jpg";

	char buf[1024];
	snprintf(buf, 1024, "http://%s:%d/update?id=%d&password=%s&name=%s&data=%d&status=%s&timestamp=%s&filename=%s",
			hostname,
			port,
			id,
			password,
			name,
			adcval,
			status,
            		timestamp,
			filename);
	FILE *fp;
	
	struct stat num;
	stat(filename, &num);
	int size = num.st_size;
	printf("Image size: %dB\n", size);	

	char *buffer = (char*)malloc(size);

	fp = fopen(filename,"rb");
	int n = fread(buffer, 1, size, fp);

	HTTP_POST(buf, buffer, size);
	fclose(fp);
	return 0;
}
