
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <curl/curl.h>

#include "parser_json.h"
#include "weather.h"


#define URL_REQUEST "http://api.openweathermap.org/data/2.5/weather?appid=10ffe9e6913b2ac1529992c5618ca106&units=metric&q=%s"
#define BUFFER_SIZE 1024

typedef struct UrlData {
    size_t capacity;
    size_t size;
    char* data;
} UrlData_t;



size_t write_data(void* ptr, size_t size, size_t nmemb, UrlData_t *data) {
    const size_t index = data -> size;
    const size_t n = (size * nmemb);

    data -> size += (size * nmemb);
    if (data -> size >= data -> capacity) {
        const size_t new_capacity = data -> capacity + BUFFER_SIZE;
        char* tmp = realloc(data -> data, new_capacity);
        if (tmp == NULL) {
            fprintf(stderr, "Failed to allocate memory.\n");
            return 0;
        } else {
            data -> data = tmp;
            data -> capacity = new_capacity;
        }
    }

    memcpy((data -> data + index), ptr, n);
    data -> data[data->size] = '\0';

    return size * nmemb;
}


void print_weather(WeartherInfo_t* weather) {
    const Main_t mainInfo =  weather -> main;
    const Wind_t windInfo =  weather -> wind;
    printf(
        "Weather in %s: cloudiness %s, visibility %d, min temperature %f, max temperature %f, average temperature %f, humidity %d, pressure %d, wind direction %f, speed %f\n",
        weather -> name, weather -> weathers[0].description, weather -> visibility, mainInfo.temp_min, mainInfo.temp_max, mainInfo.temp, mainInfo.humidity,
        mainInfo.pressure, windInfo.deg, windInfo.speed
    );
}



int main(int argc, char* argv[]) {
    if (argc < 2) {
        printf("Please add a city name as parameters\n");
        return 0;
    }
    const int size = strlen(URL_REQUEST) + strlen(argv[1]);
    char* url = NULL;
    url = calloc(sizeof(*url), size + 1);
    if (url == NULL) {
       printf("Failed to allocate memory for url\n");
       return 0;
    }
    sprintf(url, URL_REQUEST, argv[1]);
    
    CURL* curl = curl_easy_init();
    if(curl == NULL) {
        printf("Failed to setup curl\n");
        free(url);
        return 0;
    }
    UrlData_t data;
    data.size = 0;
    data.capacity = BUFFER_SIZE;
    data.data = calloc(sizeof(data.data), BUFFER_SIZE);
    if (url == NULL) {
        printf("Failed to allocate memory for buffer (size %d)\n", BUFFER_SIZE);
        return 0;
    }

    curl_easy_setopt(curl, CURLOPT_URL, url);
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_data);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &data);
    
    CURLcode res = curl_easy_perform(curl);
    if(res == CURLE_OK) {
        // printf("Received json: %s\n", data.data);
        WeartherInfo_t weather;
        if (parse_weather_json(data.data, &weather) == 0) {
            print_weather(&weather);
        } else {
            printf("Failed to failed to parse json)\n");
        }
    } else {
        fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
    }
    curl_easy_cleanup(curl);
    free(data.data);
    free(url);
    return 0;
}
