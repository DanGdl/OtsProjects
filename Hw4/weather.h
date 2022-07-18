
#ifndef WEATHER_
#define WEATHER_

#include <stdint.h>


typedef struct Wearther {
    char* main;
    char* description;
} Wearther_t;


typedef struct Main {
    float temp;
    float feels_like;
    float temp_min;
    float temp_max;
    int32_t pressure;
    int32_t humidity;
} Main_t;


typedef struct Wind {
    float speed;
    float deg;
    float gust;
} Wind_t;


typedef struct Clouds {
    int32_t all;
} Clouds_t;


typedef struct Sys {
    int32_t type;
    int64_t id;
    char* country;    
    int64_t sunrise;
    int64_t sunset;
} Sys_t;


typedef struct WeartherInfo {
    Wearther_t* weathers;
    int size_weathers;
    char* base;
    Main_t main;
    int32_t visibility;
    Wind_t wind;
    Clouds_t clouds;
    int32_t dt;
    Sys_t sys;
    int64_t timezone;
    int64_t id;
    char* name;
    int32_t cod;
} WeartherInfo_t;

#endif

/*
{
    "coord":{"lon":35.2163,"lat":31.769},
    "weather":[
        {
            "id":800,
            "main":"Clear",
            "description":"clear sky",
            "icon":"01d"
        }
    ],
    "base":"stations",
    "main":{
        "temp":301.32,
        "feels_like":301.11,
        "temp_min":297.26,
        "temp_max":301.54,
        "pressure":1002,
        "humidity":42
    },
    "visibility":10000,
    "wind":{
        "speed":2.24,
        "deg":261,
        "gust":4.92
    },
    "clouds":{
        "all":0
    },
    "dt":1656923797,
    "sys":{
        "type":2,
        "id":2004982,
        "country":"IL",
        "sunrise":1656902298,
        "sunset":1656953301
    },
    "timezone":10800,
    "id":281184,
    "name":"Jerusalem",
    "cod":200
}
*/
